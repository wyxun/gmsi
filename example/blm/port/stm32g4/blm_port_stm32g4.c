/**
 * @file blm_port_stm32g4.c
 * @brief BLM Port Implementation for STM32G4 Series
 * 
 * Uses direct register access for UART, Flash, and GPIO operations.
 * Based on STM32G431 reference manual (RM0440).
 */

/*============================ INCLUDES ======================================*/
#include "../blm_port.h"
#include "../../cmsis/stm32g431xx.h"
#include <perf_counter.h>

/*============================ MACROS ========================================*/

/* USART selection - default USART2 (PA2/PA3) */
#ifndef BLM_USART
#define BLM_USART           USART2
#define BLM_USART_GPIO      GPIOA
#define BLM_USART_TX_PIN    2
#define BLM_USART_RX_PIN    3
#define BLM_USART_AF        GPIO_AF7_USART2
#define BLM_USART_RCC_EN()  RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN
#endif

/* LED pin (optional) - default PC13 */
#ifndef BLM_LED_GPIO
#define BLM_LED_GPIO        GPIOC
#define BLM_LED_PIN         13
#endif

/* Upgrade button - default user button PA0 */
#ifndef BLM_BTN_GPIO
#define BLM_BTN_GPIO        GPIOA
#define BLM_BTN_PIN         0
#endif

/* System clock - default 16MHz (HSI) */
#ifndef BLM_SYSCLK
#define BLM_SYSCLK          16000000UL
#endif

/*============================ IMPLEMENTATION ================================*/

/* Define SystemCoreClock variable (used by perf_counter) */
uint32_t SystemCoreClock = BLM_SYSCLK;

/*----------------------------------------------------------------------------*
 * GPIO Configuration
 *----------------------------------------------------------------------------*/

/**
 * @brief Configure GPIO pin mode
 */
static void gpio_set_mode(GPIO_TypeDef *pGPIO, uint8_t chPin, uint8_t chMode)
{
    uint32_t wPos = chPin * 2;
    pGPIO->MODER &= ~(3UL << wPos);
    pGPIO->MODER |= ((uint32_t)chMode << wPos);
}

/**
 * @brief Configure GPIO alternate function
 */
static void gpio_set_af(GPIO_TypeDef *pGPIO, uint8_t chPin, uint8_t chAF)
{
    uint8_t chIdx = chPin / 8;
    uint8_t chPos = (chPin % 8) * 4;
    pGPIO->AFR[chIdx] &= ~(0x0FUL << chPos);
    pGPIO->AFR[chIdx] |= ((uint32_t)chAF << chPos);
}

/**
 * @brief Configure GPIO speed
 */
static void gpio_set_speed(GPIO_TypeDef *pGPIO, uint8_t chPin, uint8_t chSpeed)
{
    uint32_t wPos = chPin * 2;
    pGPIO->OSPEEDR &= ~(3UL << wPos);
    pGPIO->OSPEEDR |= ((uint32_t)chSpeed << wPos);
}

/**
 * @brief Configure GPIO pull-up/down
 */
static void gpio_set_pull(GPIO_TypeDef *pGPIO, uint8_t chPin, uint8_t chPull)
{
    uint32_t wPos = chPin * 2;
    pGPIO->PUPDR &= ~(3UL << wPos);
    pGPIO->PUPDR |= ((uint32_t)chPull << wPos);
}

/*----------------------------------------------------------------------------*
 * System Functions
 *----------------------------------------------------------------------------*/

/**
 * @brief Get system tick in milliseconds
 */
uint32_t blm_port_GetTickMs(void)
{
    return (uint32_t)get_system_ms();
}

/**
 * @brief Delay in milliseconds
 */
void blm_port_DelayMs(uint32_t wMs)
{
    int64_t lStart = get_system_ms();
    while ((get_system_ms() - lStart) < (int64_t)wMs) {
        /* Wait */
    }
}

/**
 * @brief System reset
 */
void blm_port_SystemReset(void)
{
    SCB->AIRCR = SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESETREQ;
    while (1) {
        __asm volatile ("nop");
    }
}

/**
 * @brief Jump to application
 */
void blm_port_JumpToApp(uint32_t wAppAddr)
{
    typedef void (*pFunction)(void);
    
    /* Get stack pointer and reset handler from vector table */
    uint32_t wStackPtr = *(volatile uint32_t *)wAppAddr;
    uint32_t wResetHandler = *(volatile uint32_t *)(wAppAddr + 4);
    
    /* Disable all interrupts */
    __disable_irq();
    
    /* Set vector table offset */
    SCB->VTOR = wAppAddr;
    
    /* Set stack pointer */
    __asm volatile ("MSR MSP, %0" : : "r" (wStackPtr));
    
    /* Jump to reset handler */
    pFunction JumpToApplication = (pFunction)wResetHandler;
    JumpToApplication();
}

/**
 * @brief Check if upgrade button pressed
 */
int blm_port_IsUpgradeButtonPressed(void)
{
    /* Enable GPIOA clock */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    
    /* Configure button pin as input with pull-up */
    gpio_set_mode(BLM_BTN_GPIO, BLM_BTN_PIN, GPIO_MODE_INPUT);
    gpio_set_pull(BLM_BTN_GPIO, BLM_BTN_PIN, GPIO_PULL_UP);
    
    /* Button pressed when low (active low) */
    return (BLM_BTN_GPIO->IDR & (1UL << BLM_BTN_PIN)) == 0;
}

/*----------------------------------------------------------------------------*
 * UART Functions
 *----------------------------------------------------------------------------*/

/**
 * @brief Initialize UART
 */
int blm_port_UartInit(uint32_t wBaudrate)
{
    /* Enable GPIO clock */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    
    /* Enable USART clock */
    BLM_USART_RCC_EN();
    
    /* Configure TX pin: AF, push-pull, high speed */
    gpio_set_mode(BLM_USART_GPIO, BLM_USART_TX_PIN, GPIO_MODE_AF);
    gpio_set_af(BLM_USART_GPIO, BLM_USART_TX_PIN, BLM_USART_AF);
    gpio_set_speed(BLM_USART_GPIO, BLM_USART_TX_PIN, GPIO_SPEED_HIGH);
    
    /* Configure RX pin: AF, pull-up */
    gpio_set_mode(BLM_USART_GPIO, BLM_USART_RX_PIN, GPIO_MODE_AF);
    gpio_set_af(BLM_USART_GPIO, BLM_USART_RX_PIN, BLM_USART_AF);
    gpio_set_pull(BLM_USART_GPIO, BLM_USART_RX_PIN, GPIO_PULL_UP);
    
    /* Disable USART before configuration */
    BLM_USART->CR1 = 0;
    
    /* Set baud rate: BRR = fCK / USARTDIV */
    BLM_USART->BRR = BLM_SYSCLK / wBaudrate;
    
    /* Configure: 8N1, no parity */
    BLM_USART->CR2 = USART_CR2_STOP_1;  /* 1 stop bit */
    BLM_USART->CR3 = 0;
    
    /* Enable USART, TX, RX */
    BLM_USART->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
    
    /* Wait for TE and RE acknowledge */
    while ((BLM_USART->ISR & (USART_ISR_TEACK | USART_ISR_REACK)) 
            != (USART_ISR_TEACK | USART_ISR_REACK)) {
        /* Wait */
    }
    
    return 0;
}

/**
 * @brief Send data via UART
 */
int blm_port_UartSend(const uint8_t *pchData, uint16_t hwLen)
{
    for (uint16_t i = 0; i < hwLen; i++) {
        /* Wait until TXE is set */
        while (!(BLM_USART->ISR & USART_ISR_TXE)) {
            /* Wait */
        }
        BLM_USART->TDR = pchData[i];
    }
    
    /* Wait for transmission complete */
    while (!(BLM_USART->ISR & USART_ISR_TC)) {
        /* Wait */
    }
    
    return hwLen;
}

/**
 * @brief Receive data via UART with timeout
 */
int blm_port_UartRecv(uint8_t *pchData, uint16_t hwLen, uint32_t wTimeoutMs)
{
    int64_t lStart = get_system_ms();
    uint16_t hwReceived = 0;
    
    while (hwReceived < hwLen) {
        /* Check timeout */
        if ((get_system_ms() - lStart) >= (int64_t)wTimeoutMs) {
            break;
        }
        
        /* Check for errors and clear them */
        if (BLM_USART->ISR & (USART_ISR_ORE | USART_ISR_FE | USART_ISR_NE)) {
            BLM_USART->ICR = USART_ICR_ORECF | USART_ICR_FECF | USART_ICR_NCF;
        }
        
        /* Check if data available */
        if (BLM_USART->ISR & USART_ISR_RXNE) {
            pchData[hwReceived++] = (uint8_t)BLM_USART->RDR;
            lStart = get_system_ms();  /* Reset timeout for next byte */
        }
    }
    
    return hwReceived;
}

/**
 * @brief Check if UART data available
 */
int blm_port_UartAvailable(void)
{
    return (BLM_USART->ISR & USART_ISR_RXNE) ? 1 : 0;
}

/**
 * @brief Flush UART receive buffer
 */
void blm_port_UartFlush(void)
{
    /* Clear overrun and read RDR to clear RXNE */
    BLM_USART->ICR = USART_ICR_ORECF;
    (void)BLM_USART->RDR;
}

/*----------------------------------------------------------------------------*
 * Flash Functions
 *----------------------------------------------------------------------------*/

/**
 * @brief Wait for flash operation to complete
 */
static int flash_wait_bsy(uint32_t wTimeoutMs)
{
    int64_t lStart = get_system_ms();
    
    while (FLASH_REG->SR & FLASH_SR_BSY) {
        if ((get_system_ms() - lStart) >= (int64_t)wTimeoutMs) {
            return -1;
        }
    }
    
    return 0;
}

/**
 * @brief Clear flash error flags
 */
static void flash_clear_errors(void)
{
    FLASH_REG->SR = FLASH_SR_OPERR | FLASH_SR_PROGERR | FLASH_SR_WRPERR |
                    FLASH_SR_PGAERR | FLASH_SR_SIZERR | FLASH_SR_PGSERR |
                    FLASH_SR_MISERR | FLASH_SR_FASTERR | FLASH_SR_EOP;
}

/**
 * @brief Initialize Flash
 * 
 * Clears error flags and prepares flash for operation.
 */
int blm_port_FlashInit(void)
{
    flash_clear_errors();
    return 0;
}

/**
 * @brief Unlock flash for programming
 */
int blm_port_FlashUnlock(void)
{
    if (FLASH_REG->CR & FLASH_CR_LOCK) {
        FLASH_REG->KEYR = FLASH_KEY1;
        FLASH_REG->KEYR = FLASH_KEY2;
    }
    
    flash_clear_errors();
    
    return (FLASH_REG->CR & FLASH_CR_LOCK) ? -1 : 0;
}

/**
 * @brief Lock flash
 */
int blm_port_FlashLock(void)
{
    FLASH_REG->CR |= FLASH_CR_LOCK;
    return 0;
}

/**
 * @brief Erase flash pages covering given size
 */
int blm_port_FlashErase(uint32_t wAddr, uint32_t wLen)
{
    /* Calculate page range */
    uint32_t wStartPage = (wAddr - FLASH_BASE) / FLASH_PAGE_SIZE;
    uint32_t wEndAddr = wAddr + wLen - 1;
    uint32_t wEndPage = (wEndAddr - FLASH_BASE) / FLASH_PAGE_SIZE;
    
    for (uint32_t wPage = wStartPage; wPage <= wEndPage; wPage++) {
        /* Wait for any ongoing operation */
        if (flash_wait_bsy(1000) != 0) {
            return -1;
        }
        
        flash_clear_errors();
        
        /* Set page erase mode and page number */
        FLASH_REG->CR = FLASH_CR_PER | (wPage << FLASH_CR_PNB_Pos);
        
        /* Start erase */
        FLASH_REG->CR |= FLASH_CR_STRT;
        
        /* Wait for completion */
        if (flash_wait_bsy(1000) != 0) {
            return -1;
        }
        
        /* Clear PER bit */
        FLASH_REG->CR &= ~FLASH_CR_PER;
    }
    
    return 0;
}

/**
 * @brief Write data to flash (must be 64-bit aligned, double-word programming)
 */
int blm_port_FlashWrite(uint32_t wAddr, const uint8_t *pchData, uint32_t wLen)
{
    /* STM32G4 requires double-word (64-bit) programming */
    uint32_t wWriteAddr = wAddr;
    uint32_t wIdx = 0;
    
    while (wIdx < wLen) {
        /* Wait for flash ready */
        if (flash_wait_bsy(100) != 0) {
            return -1;
        }
        
        flash_clear_errors();
        
        /* Enable programming */
        FLASH_REG->CR = FLASH_CR_PG;
        
        /* Prepare double-word data */
        uint32_t wWord1 = 0xFFFFFFFF;
        uint32_t wWord2 = 0xFFFFFFFF;
        
        /* Fill first word */
        for (int i = 0; i < 4 && wIdx < wLen; i++, wIdx++) {
            wWord1 &= ~(0xFFUL << (i * 8));
            wWord1 |= ((uint32_t)pchData[wIdx] << (i * 8));
        }
        
        /* Fill second word */
        for (int i = 0; i < 4 && wIdx < wLen; i++, wIdx++) {
            wWord2 &= ~(0xFFUL << (i * 8));
            wWord2 |= ((uint32_t)pchData[wIdx] << (i * 8));
        }
        
        /* Write first word (must write both words before BSY check) */
        *(volatile uint32_t *)wWriteAddr = wWord1;
        
        /* Write second word immediately */
        *(volatile uint32_t *)(wWriteAddr + 4) = wWord2;
        
        /* Wait for completion */
        if (flash_wait_bsy(100) != 0) {
            return -1;
        }
        
        /* Check for errors */
        if (FLASH_REG->SR & (FLASH_SR_PROGERR | FLASH_SR_WRPERR | FLASH_SR_PGAERR)) {
            return -1;
        }
        
        /* Clear PG bit */
        FLASH_REG->CR &= ~FLASH_CR_PG;
        
        wWriteAddr += 8;
    }
    
    return wLen;
}

/**
 * @brief Read data from flash
 */
int blm_port_FlashRead(uint32_t wAddr, uint8_t *pchData, uint32_t wLen)
{
    /* Direct memory read */
    const uint8_t *pchSrc = (const uint8_t *)wAddr;
    
    for (uint32_t i = 0; i < wLen; i++) {
        pchData[i] = pchSrc[i];
    }
    
    return wLen;
}

/**
 * @brief Get flash page size
 */
uint32_t blm_port_FlashGetPageSize(uint32_t wAddr)
{
    (void)wAddr;  /* STM32G431 has uniform 2KB page size */
    return FLASH_PAGE_SIZE;
}

/*----------------------------------------------------------------------------*
 * LED Functions (optional)
 *----------------------------------------------------------------------------*/

/**
 * @brief Set LED state
 */
void blm_port_LedSet(int nState)
{
    /* Enable GPIOC clock */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
    
    /* Configure LED pin as output */
    gpio_set_mode(BLM_LED_GPIO, BLM_LED_PIN, GPIO_MODE_OUTPUT);
    
    if (nState) {
        BLM_LED_GPIO->BSRR = (1UL << BLM_LED_PIN);  /* Set */
    } else {
        BLM_LED_GPIO->BRR = (1UL << BLM_LED_PIN);   /* Reset */
    }
}
