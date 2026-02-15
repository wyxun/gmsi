/**
 * @file blm_port_at32f4.c
 * @brief BLM Port Implementation for AT32F407 Series
 *
 * Uses direct register access for UART, Flash, and GPIO operations.
 * Based on AT32F403A/407 Reference Manual.
 *
 * Key differences from STM32G4:
 *   - USART uses STS/DT registers (STM32F1-style SR/DR)
 *   - Flash uses half-word (16-bit) programming
 *   - GPIO uses CRL/CRH configuration model
 *   - GPIO/USART clocks on APB2 bus
 */

/*============================ INCLUDES ======================================*/
#include "../blm_port.h"
#include "../../cmsis/at32f407xx.h"
#include <perf_counter.h>

/*============================ MACROS ========================================*/

/* USART selection - default USART1 (PA9-TX, PA10-RX) */
#ifndef BLM_USART
#define BLM_USART           USART1
#define BLM_USART_GPIO      GPIOA
#define BLM_USART_TX_PIN    9
#define BLM_USART_RX_PIN    10
#define BLM_USART_RCC_EN()  CRM->APB2EN |= CRM_APB2EN_USART1EN
#endif

/* LED pin (optional) - default PD13 */
#ifndef BLM_LED_GPIO
#define BLM_LED_GPIO        GPIOD
#define BLM_LED_PIN         13
#endif

/* Upgrade button - default PA0 */
#ifndef BLM_BTN_GPIO
#define BLM_BTN_GPIO        GPIOA
#define BLM_BTN_PIN         0
#endif

/* System clock - default 8MHz (HICK) */
#ifndef BLM_SYSCLK
#define BLM_SYSCLK          8000000UL
#endif

/*============================ IMPLEMENTATION ================================*/

/* Define SystemCoreClock variable (used by perf_counter) */
uint32_t SystemCoreClock = BLM_SYSCLK;

/*----------------------------------------------------------------------------*
 * GPIO Configuration (AT32F407 CRL/CRH model)
 *----------------------------------------------------------------------------*/

/**
 * @brief Configure GPIO pin mode and configuration
 * @param pGPIO GPIO port
 * @param chPin Pin number (0-15)
 * @param chModeCnf Combined MODE[1:0] | CNF[3:2] value (4 bits per pin)
 */
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

/*----------------------------------------------------------------------------*
 * System Functions
 *----------------------------------------------------------------------------*/

uint32_t blm_port_GetTickMs(void)
{
    return (uint32_t)get_system_ms();
}

void blm_port_DelayMs(uint32_t wMs)
{
    int64_t lStart = get_system_ms();
    while ((get_system_ms() - lStart) < (int64_t)wMs) {
        /* Wait */
    }
}

void blm_port_SystemReset(void)
{
    __DSB();
    SCB->AIRCR = SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESETREQ;
    __DSB();
    while (1) {
        __NOP();
    }
}

void blm_port_JumpToApp(uint32_t wAppAddr)
{
    typedef void (*pFunction)(void);

    uint32_t wStackPtr = *(volatile uint32_t *)wAppAddr;
    uint32_t wResetHandler = *(volatile uint32_t *)(wAppAddr + 4);

    __disable_irq();

    SCB->VTOR = wAppAddr;

    __set_MSP(wStackPtr);

    pFunction JumpToApplication = (pFunction)wResetHandler;
    JumpToApplication();
}

int blm_port_IsUpgradeButtonPressed(void)
{
    /* Enable GPIOA clock */
    CRM->APB2EN |= CRM_APB2EN_GPIOAEN;

    /* Configure button pin as input with pull-up */
    gpio_set_mode_cnf(BLM_BTN_GPIO, BLM_BTN_PIN,
                      GPIO_MODE_INPUT | GPIO_CNF_PULL);

    /* Enable pull-up via ODR */
    BLM_BTN_GPIO->ODT |= (1UL << BLM_BTN_PIN);

    /* Button pressed when low (active low) */
    return (BLM_BTN_GPIO->IDT & (1UL << BLM_BTN_PIN)) == 0;
}

/*----------------------------------------------------------------------------*
 * UART Functions (AT32F407 STS/DT model)
 *----------------------------------------------------------------------------*/

int blm_port_UartInit(uint32_t wBaudrate)
{
    /* Enable GPIO clock */
    CRM->APB2EN |= CRM_APB2EN_GPIOAEN;

    /* Enable USART clock */
    BLM_USART_RCC_EN();

    /* Configure TX pin: AF push-pull, 50MHz */
    gpio_set_mode_cnf(BLM_USART_GPIO, BLM_USART_TX_PIN,
                      GPIO_MODE_OUT_50M | GPIO_CNF_AF_PP);

    /* Configure RX pin: input with pull-up */
    gpio_set_mode_cnf(BLM_USART_GPIO, BLM_USART_RX_PIN,
                      GPIO_MODE_INPUT | GPIO_CNF_PULL);
    /* Enable pull-up via ODR */
    BLM_USART_GPIO->ODT |= (1UL << BLM_USART_RX_PIN);

    /* Disable USART before configuration */
    BLM_USART->CTRL1 = 0;

    /* Set baud rate: BAUDR = fCK / baud */
    BLM_USART->BAUDR = BLM_SYSCLK / wBaudrate;

    /* Configure: 8N1, no parity */
    BLM_USART->CTRL2 = USART_CTRL2_STOPBN_1;   /* 1 stop bit */
    BLM_USART->CTRL3 = 0;

    /* Enable USART, TX, RX */
    BLM_USART->CTRL1 = USART_CTRL1_UEN | USART_CTRL1_TEN | USART_CTRL1_REN;

    return 0;
}

int blm_port_UartSend(const uint8_t *pchData, uint16_t hwLen)
{
    for (uint16_t i = 0; i < hwLen; i++) {
        /* Wait until TDBE (TX buffer empty) */
        while (!(BLM_USART->STS & USART_STS_TDBE)) {
            /* Wait */
        }
        BLM_USART->DT = pchData[i];
    }

    /* Wait for transmission complete */
    while (!(BLM_USART->STS & USART_STS_TDC)) {
        /* Wait */
    }

    return hwLen;
}

int blm_port_UartRecv(uint8_t *pchData, uint16_t hwLen, uint32_t wTimeoutMs)
{
    int64_t lStart = get_system_ms();
    uint16_t hwReceived = 0;

    while (hwReceived < hwLen) {
        if ((get_system_ms() - lStart) >= (int64_t)wTimeoutMs) {
            break;
        }

        /* Check for errors and clear them (read STS then DT) */
        if (BLM_USART->STS & (USART_STS_ROERR | USART_STS_FERR | USART_STS_NERR)) {
            (void)BLM_USART->STS;
            (void)BLM_USART->DT;
        }

        /* Check if data available */
        if (BLM_USART->STS & USART_STS_RDBF) {
            pchData[hwReceived++] = (uint8_t)BLM_USART->DT;
            lStart = get_system_ms();
        }
    }

    return hwReceived;
}

int blm_port_UartAvailable(void)
{
    return (BLM_USART->STS & USART_STS_RDBF) ? 1 : 0;
}

void blm_port_UartFlush(void)
{
    /* Clear by reading STS then DT */
    (void)BLM_USART->STS;
    (void)BLM_USART->DT;
}

/*----------------------------------------------------------------------------*
 * Flash Functions (AT32F407 half-word programming)
 *----------------------------------------------------------------------------*/

static int flash_wait_done(uint32_t wTimeoutMs)
{
    int64_t lStart = get_system_ms();

    while (FLASH_REG->STS & FLASH_STS_OBF) {
        if ((get_system_ms() - lStart) >= (int64_t)wTimeoutMs) {
            return -1;
        }
    }

    return 0;
}

static void flash_clear_errors(void)
{
    /* Write 1 to clear error flags */
    FLASH_REG->STS = FLASH_STS_PRGMERR | FLASH_STS_EPPERR | FLASH_STS_ODF;
}

int blm_port_FlashInit(void)
{
    flash_clear_errors();
    return 0;
}

int blm_port_FlashUnlock(void)
{
    if (FLASH_REG->CTRL & FLASH_CTRL_OPLK) {
        FLASH_REG->UNLOCK = FLASH_KEY1;
        FLASH_REG->UNLOCK = FLASH_KEY2;
    }

    flash_clear_errors();

    return (FLASH_REG->CTRL & FLASH_CTRL_OPLK) ? -1 : 0;
}

int blm_port_FlashLock(void)
{
    FLASH_REG->CTRL |= FLASH_CTRL_OPLK;
    return 0;
}

int blm_port_FlashErase(uint32_t wAddr, uint32_t wLen)
{
    uint32_t wEndAddr = wAddr + wLen;

    /* Erase page by page */
    for (uint32_t wPageAddr = wAddr; wPageAddr < wEndAddr; wPageAddr += FLASH_PAGE_SIZE) {
        if (flash_wait_done(1000) != 0) {
            return -1;
        }

        flash_clear_errors();

        /* Set sector erase mode */
        FLASH_REG->CTRL |= FLASH_CTRL_SECERS;

        /* Set page address */
        FLASH_REG->ADDR = wPageAddr;

        /* Start erase */
        FLASH_REG->CTRL |= FLASH_CTRL_ERSTR;

        /* Wait for completion */
        if (flash_wait_done(1000) != 0) {
            return -1;
        }

        /* Clear sector erase bit */
        FLASH_REG->CTRL &= ~FLASH_CTRL_SECERS;
    }

    return 0;
}

int blm_port_FlashWrite(uint32_t wAddr, const uint8_t *pchData, uint32_t wLen)
{
    /* AT32F407 uses half-word (16-bit) programming */
    uint32_t wWriteAddr = wAddr;
    uint32_t wIdx = 0;

    while (wIdx < wLen) {
        if (flash_wait_done(100) != 0) {
            return -1;
        }

        flash_clear_errors();

        /* Enable programming */
        FLASH_REG->CTRL |= FLASH_CTRL_FPRGM;

        /* Prepare half-word (little-endian: low byte first) */
        uint16_t hwData;
        hwData = pchData[wIdx++];
        if (wIdx < wLen) {
            hwData |= (uint16_t)pchData[wIdx++] << 8;
        } else {
            hwData |= 0xFF00;  /* Pad with 0xFF for odd length */
        }

        /* Write half-word */
        *(volatile uint16_t *)wWriteAddr = hwData;

        /* Wait for completion */
        if (flash_wait_done(100) != 0) {
            return -1;
        }

        /* Check for errors */
        if (FLASH_REG->STS & (FLASH_STS_PRGMERR | FLASH_STS_EPPERR)) {
            return -1;
        }

        /* Clear programming bit */
        FLASH_REG->CTRL &= ~FLASH_CTRL_FPRGM;

        wWriteAddr += 2;
    }

    return wLen;
}

int blm_port_FlashRead(uint32_t wAddr, uint8_t *pchData, uint32_t wLen)
{
    const uint8_t *pchSrc = (const uint8_t *)wAddr;

    for (uint32_t i = 0; i < wLen; i++) {
        pchData[i] = pchSrc[i];
    }

    return wLen;
}

uint32_t blm_port_FlashGetPageSize(uint32_t wAddr)
{
    (void)wAddr;
    return FLASH_PAGE_SIZE;
}

/*----------------------------------------------------------------------------*
 * LED Functions (optional)
 *----------------------------------------------------------------------------*/

void blm_port_LedSet(int nState)
{
    /* Enable GPIOD clock */
    CRM->APB2EN |= CRM_APB2EN_GPIODEN;

    /* Configure LED pin as output push-pull, 2MHz */
    gpio_set_mode_cnf(BLM_LED_GPIO, BLM_LED_PIN,
                      GPIO_MODE_OUT_2M | GPIO_CNF_GP_PP);

    if (nState) {
        BLM_LED_GPIO->SCR = (1UL << BLM_LED_PIN);  /* Set */
    } else {
        BLM_LED_GPIO->CLR = (1UL << BLM_LED_PIN);  /* Reset */
    }
}
