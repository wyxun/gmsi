/**
 * @file blm_port_at32f4.c
 * @brief BLM Port Implementation for AT32F407 Series (HEXT 8MHz version)
 */

#include "../blm_port.h"
#include "../../cmsis/at32f407xx.h"
#include "../userconfig.h"
#include <perf_counter.h>
#include <string.h>
#include "../mdi_hw.h"

/*============================ MACROS ========================================*/

/* USART selection - default USART1 (PA9-TX, PA10-RX) */
#ifndef BLM_USART
#define BLM_USART           USART1
#define BLM_USART_GPIO      GPIOA
#define BLM_USART_TX_PIN    9
#define BLM_USART_RX_PIN    10
#define BLM_USART_RCC_EN()  CRM->APB2EN |= CRM_APB2EN_USART1EN
#endif

/* LED pin - default PD13 */
#ifndef BLM_LED_GPIO
#define BLM_LED_GPIO        GPIOD
#define BLM_LED_PIN         13
#endif

/* Upgrade button - default PA0 */
#ifndef BLM_BTN_GPIO
#define BLM_BTN_GPIO        GPIOA
#define BLM_BTN_PIN         0
#endif

#define BLM_RX_BUF_SIZE         512  /* Reduced from 4096 for size optimization */

#define CRM_CFG_PLLMULT_H   (1UL << 29)
#define CRM_CFG_AHBPSC_Pos  4
#define CRM_CFG_AHBPSC_Msk  (0x0FUL << CRM_CFG_AHBPSC_Pos)
#define CRM_CFG_APB1PSC_Pos 8
#define CRM_CFG_APB1PSC_Msk (0x07UL << CRM_CFG_APB1PSC_Pos)
#define CRM_CFG_APB2PSC_Pos 11
#define CRM_CFG_APB2PSC_Msk (0x07UL << CRM_CFG_APB2PSC_Pos)

/*============================ IMPLEMENTATION ================================*/

uint32_t SystemCoreClock = BLM_SYSCLK;
#include "utilities/mringbuf.h"
static mringbuf_t s_tRxQueue;
static uint8_t s_achRxBuf[512];  /* Reduced from 2048 for size optimization */

static void gpio_set_mode_cnf(GPIO_TypeDef *pGPIO, uint8_t chPin, uint8_t chModeCnf)
{
    if (chPin < 8) {
        uint32_t wPos = chPin * 4;
        pGPIO->CFGLR &= ~(0x0FUL << wPos);
        pGPIO->CFGLR |= ((uint32_t)(chModeCnf & 0x0F) << wPos);
    } else {
        uint32_t wPos = (chPin - 8) * 4;
        pGPIO->CFGHR &= ~(0x0FUL << wPos);
        pGPIO->CFGHR |= ((uint32_t)(chModeCnf & 0x0F) << wPos);
    }
}

uint32_t blm_port_GetTickMs(void)
{
    return (uint32_t)get_system_ms();
}

void blm_port_DelayMs(uint32_t wMs)
{
    perfc_delay_ms(wMs);
}

void blm_port_SystemReset(void)
{
    SCB->AIRCR = SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESETREQ;
    while (1);
}

void blm_port_SystemClockConfig(void)
{
    /* Ensure HICK is on and switch to it first */

    CRM->CTRL |= CRM_CTRL_HICKEN;
    while (!(CRM->CTRL & CRM_CTRL_HICKSTBL));
    
    /* Switch to HICK */
    CRM->CFG &= ~0x03UL;
    
    /* Turn off PLL and HEXT to allow configuration */
    CRM->CTRL &= ~(CRM_CTRL_PLLEN | CRM_CTRL_HEXTEN);
    
    /* Enable HEXT (8MHz crystal) */
    CRM->CTRL |= CRM_CTRL_HEXTEN;
    while (!(CRM->CTRL & CRM_CTRL_HEXTSTBL));

    /* Config Flash Latency = 3 for 120MHz */
    FLASH_REG->PSR &= ~0x07UL;
    FLASH_REG->PSR |= 0x03UL;

    /* Config PLL: SOURCE=HEXT(8MHz), MULT=15 (8*15=120MHz) */
    /* MULT[3:0] = (15-2) & 0xF = 13 (0x0D) */
    /* MULT[4]   = 0 */
    CRM->CFG &= ~((0x0FUL << 18) | (1UL << 17) | (1UL << 16) | CRM_CFG_PLLMULT_H);
    CRM->CFG |= (1UL << 16);               /* Source = HEXT */
    CRM->CFG |= (0x0DUL << 18);            /* MULT[3:0] = 0x0D */
    /* CRM_CFG_PLLMULT_H is left 0 */
    /* PLLHSCFG (bit 17) is left 0 (HEXT directly) */
    
    CRM->CTRL |= CRM_CTRL_PLLEN;
    while (!(CRM->CTRL & CRM_CTRL_PLLSTBL));
    
    /* Config APB1 = 60MHz, APB2 = 120MHz, AHB = 120MHz */
    CRM->CFG &= ~(CRM_CFG_APB1PSC_Msk | CRM_CFG_APB2PSC_Msk);
    CRM->CFG |= (0x04UL << CRM_CFG_APB1PSC_Pos); /* APB1 = DIV2 */

    /* Select PLL as system clock */
    CRM->CFG &= ~0x03UL;
    CRM->CFG |= 0x02UL;
    while ((CRM->CFG & 0x0CUL) != 0x08UL);
}

int blm_port_UartInit(uint32_t wBaudrate)
{
    blm_port_SystemClockConfig();
    SystemCoreClock = BLM_SYSCLK;

    mringbuf_Init(&s_tRxQueue, s_achRxBuf, sizeof(s_achRxBuf));


    CRM->APB2EN |= CRM_APB2EN_GPIOAEN | CRM_APB2EN_IOMUXEN;
    BLM_USART_RCC_EN();

    /* TX: AF push-pull, 10MHz */
    gpio_set_mode_cnf(BLM_USART_GPIO, BLM_USART_TX_PIN,
                      GPIO_MODE_OUT_10M | GPIO_CNF_AF_PP);

    /* RX: input with pull-up */
    gpio_set_mode_cnf(BLM_USART_GPIO, BLM_USART_RX_PIN,
                      GPIO_MODE_INPUT | GPIO_CNF_PULL);
    BLM_USART_GPIO->ODT |= (1UL << BLM_USART_RX_PIN);

    BLM_USART->CTRL1 = 0;
    BLM_USART->BAUDR = (SystemCoreClock) / wBaudrate;
    BLM_USART->CTRL2 = USART_CTRL2_STOPBN_1; /* 1 Stop Bit */
    BLM_USART->CTRL3 = 0;

    BLM_USART->CTRL1 = USART_CTRL1_UEN | USART_CTRL1_TEN | USART_CTRL1_REN | USART_CTRL1_RDBFIEN;

    /* NVIC settings */
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, 1);


    return 0;
}

void USART1_IRQHandler(void)
{
    uint32_t wStatus = BLM_USART->STS;
    if (wStatus & USART_STS_RDBF) {
        uint8_t chByte = (uint8_t)BLM_USART->DT;
        if (mringbuf_GetFree(&s_tRxQueue) > 0) {
            mringbuf_Write(&s_tRxQueue, chByte);
        }
    }
    if (wStatus & (USART_STS_ROERR | USART_STS_FERR | USART_STS_NERR)) {
        volatile uint32_t tmpreg = BLM_USART->STS;
        tmpreg = BLM_USART->DT;
        (void)tmpreg;
    }
}

int blm_port_UartSend(const uint8_t *pchData, uint16_t hwLen)
{
    for (uint16_t i = 0; i < hwLen; i++) {
        while (!(BLM_USART->STS & USART_STS_TDBE));
        BLM_USART->DT = pchData[i];
    }
    while (!(BLM_USART->STS & USART_STS_TDC));
    return hwLen;
}

int blm_port_UartRecv(uint8_t *pchData, uint16_t hwLen, uint32_t wTimeoutMs)
{
    int64_t lStart = get_system_ms();
    uint16_t hwReceived = 0;

    /* Force clear hardware status to recover from early errors */
    if (BLM_USART->STS & (USART_STS_ROERR | USART_STS_FERR | USART_STS_NERR)) {
        volatile uint32_t tmp = BLM_USART->STS;
        tmp = BLM_USART->DT;
        (void)tmp;
    }
    
    while (hwReceived < hwLen) {
        if ((get_system_ms() - lStart) >= (int64_t)wTimeoutMs) break;
        
        uint32_t wStatus = BLM_USART->STS;
        if (wStatus & (USART_STS_ROERR | USART_STS_FERR | USART_STS_NERR)) {
            volatile uint8_t ch = (uint8_t)BLM_USART->DT; (void)ch;
        }

        uint8_t chByte;
        /* ISR Queue Read */
        if (mringbuf_Read(&s_tRxQueue, &chByte) > 0) {
            pchData[hwReceived++] = chByte;
            lStart = get_system_ms();
            continue;
        }
        
        /* Polling Fallback: Check hardware directly in case ISR is not firing */
        if (wStatus & USART_STS_RDBF) {
            chByte = (uint8_t)BLM_USART->DT;
            pchData[hwReceived++] = chByte;
            lStart = get_system_ms();
        }
    }
    return hwReceived;
}

int blm_port_UartAvailable(void) { return (mringbuf_GetUsed(&s_tRxQueue) > 0); }

void blm_port_UartFlush(void)
{
    __disable_irq();
    s_tRxQueue.hwTail = s_tRxQueue.hwHead;
    __enable_irq();
}

int blm_port_IsUpgradeButtonPressed(void)
{
    CRM->APB2EN |= CRM_APB2EN_GPIOAEN;
    gpio_set_mode_cnf(BLM_BTN_GPIO, BLM_BTN_PIN, GPIO_MODE_INPUT | GPIO_CNF_PULL);
    BLM_BTN_GPIO->ODT |= (1UL << BLM_BTN_PIN);
    return (BLM_BTN_GPIO->IDT & (1UL << BLM_BTN_PIN)) == 0;
}

void blm_port_JumpToApp(uint32_t wAppAddr)
{
    typedef void (*pFunction)(void);
    uint32_t wStackPtr = *(volatile uint32_t *)wAppAddr;
    uint32_t wResetHandler = *(volatile uint32_t *)(wAppAddr + 4);
    __disable_irq();
    SCB->VTOR = wAppAddr;
    __set_MSP(wStackPtr);
    ((pFunction)wResetHandler)();
}

int blm_port_FlashInit(void) { FLASH_REG->STS = 0x34; return 0; }
int blm_port_FlashUnlock(void) {
    if (FLASH_REG->CTRL & FLASH_CTRL_OPLK) {
        FLASH_REG->UNLOCK = FLASH_KEY1; FLASH_REG->UNLOCK = FLASH_KEY2;
    }
    return (FLASH_REG->CTRL & FLASH_CTRL_OPLK) ? -1 : 0;
}
int blm_port_FlashLock(void) { FLASH_REG->CTRL |= FLASH_CTRL_OPLK; return 0; }
int blm_port_FlashErase(uint32_t wAddr, uint32_t wLen) {
    uint32_t wEnd = wAddr + wLen;
    for (uint32_t wP = wAddr; wP < wEnd; wP += FLASH_PAGE_SIZE) {
        while (FLASH_REG->STS & FLASH_STS_OBF);
        FLASH_REG->CTRL |= FLASH_CTRL_SECERS;
        FLASH_REG->ADDR = wP;
        FLASH_REG->CTRL |= FLASH_CTRL_ERSTR;
        while (FLASH_REG->STS & FLASH_STS_OBF);
        FLASH_REG->CTRL &= ~FLASH_CTRL_SECERS;
    }
    return 0;
}
int blm_port_FlashWrite(uint32_t wAddr, const uint8_t *pchData, uint32_t wLen) {
    uint32_t wW = wAddr; uint32_t wI = 0;
    while (wI < wLen) {
        while (FLASH_REG->STS & FLASH_STS_OBF);
        FLASH_REG->CTRL |= FLASH_CTRL_FPRGM;
        uint16_t hw = pchData[wI++];
        if (wI < wLen) hw |= (uint16_t)pchData[wI++] << 8; else hw |= 0xFF00;
        *(volatile uint16_t *)wW = hw;
        while (FLASH_REG->STS & FLASH_STS_OBF);
        FLASH_REG->CTRL &= ~FLASH_CTRL_FPRGM;
        wW += 2;
    }
    return wLen;
}
int blm_port_FlashRead(uint32_t wAddr, uint8_t *pchData, uint32_t wLen) {
    memcpy(pchData, (void*)wAddr, wLen); return wLen;
}
uint32_t blm_port_FlashGetPageSize(uint32_t wAddr) { return FLASH_PAGE_SIZE; }
void blm_port_LedSet(int n) {
    CRM->APB2EN |= CRM_APB2EN_GPIODEN;
    gpio_set_mode_cnf(BLM_LED_GPIO, BLM_LED_PIN, GPIO_MODE_OUT_2M | GPIO_CNF_GP_PP);
    if (n) BLM_LED_GPIO->SCR = (1UL << BLM_LED_PIN); else BLM_LED_GPIO->CLR = (1UL << BLM_LED_PIN);
}

/*============================================================================
 * GDI 适配层
 *===========================================================================*/

/* ---- GPIO (LED) ---- */

static int32_t mdi_led_Set(void *pPriv, mdi_gpio_level_t eLevel)
{
    (void)pPriv;
    blm_port_LedSet((int)eLevel);
    return 0;
}

static int32_t mdi_led_Get(void *pPriv)
{
    (void)pPriv;
    return (BLM_LED_GPIO->ODT & (1UL << BLM_LED_PIN)) ? MDI_GPIO_HIGH : MDI_GPIO_LOW;
}

static int32_t mdi_led_Toggle(void *pPriv)
{
    (void)pPriv;
    BLM_LED_GPIO->ODT ^= (1UL << BLM_LED_PIN);
    return 0;
}

static mdi_gpio_t s_tLedGpio = {
    .pPriv    = NULL,
    .fnSet    = mdi_led_Set,
    .fnGet    = mdi_led_Get,
    .fnToggle = mdi_led_Toggle,
};

/* ---- Stream (UART) ---- */

static int32_t mdi_uart_Write(void *pPriv, const uint8_t *pchData, uint32_t wLen)
{
    (void)pPriv;
    return (int32_t)blm_port_UartSend(pchData, (uint16_t)wLen);
}

/**
 * @brief GDI UART Read — 固定 3ms timeout
 *
 * 协议层通过 PERFC_PT_WAIT_UNTIL 外层循环处理宏观超时，
 * 此处只需短暂尝试读取即可。
 */
static int32_t mdi_uart_Read(void *pPriv, uint8_t *pchBuf, uint32_t wLen)
{
    (void)pPriv;
    return (int32_t)blm_port_UartRecv(pchBuf, (uint16_t)wLen, 3);
}

static int32_t mdi_uart_IsBusy(void *pPriv)
{
    (void)pPriv;
    return (BLM_USART->STS & USART_STS_TDC) ? 0 : 1;
}

static mdi_stream_t s_tUartDebug = {
    .pPriv    = NULL,
    .fnWrite  = mdi_uart_Write,
    .fnRead   = mdi_uart_Read,
    .fnIsBusy = mdi_uart_IsBusy,
};

/* ---- Flash ---- */

static int32_t mdi_flash_Erase_wrapper(void *pPriv, uint32_t wAddr, uint32_t wSize)
{
    (void)pPriv;
    return (int32_t)blm_port_FlashErase(wAddr, wSize);
}

static int32_t mdi_flash_Write_wrapper(void *pPriv, uint32_t wAddr, const uint8_t *pchData, uint32_t wLen)
{
    (void)pPriv;
    return (int32_t)blm_port_FlashWrite(wAddr, pchData, wLen);
}

static int32_t mdi_flash_Read_wrapper(void *pPriv, uint32_t wAddr, uint8_t *pchBuf, uint32_t wLen)
{
    (void)pPriv;
    return (int32_t)blm_port_FlashRead(wAddr, pchBuf, wLen);
}

static int32_t mdi_flash_Unlock_wrapper(void *pPriv)
{
    (void)pPriv;
    return (int32_t)blm_port_FlashUnlock();
}

static int32_t mdi_flash_Lock_wrapper(void *pPriv)
{
    (void)pPriv;
    return (int32_t)blm_port_FlashLock();
}

static mdi_flash_t s_tFlashApp = {
    .pPriv    = NULL,
    .fnErase  = mdi_flash_Erase_wrapper,
    .fnWrite  = mdi_flash_Write_wrapper,
    .fnRead   = mdi_flash_Read_wrapper,
    .fnUnlock = mdi_flash_Unlock_wrapper,
    .fnLock   = mdi_flash_Lock_wrapper,
};

/* ---- 全局硬件资源池 ---- */

const mdi_hardware_t HW = {
    .ptLedStatus   = &s_tLedGpio,
    .ptSerialDebug = &s_tUartDebug,
    .ptAppFlash    = &s_tFlashApp,
};
