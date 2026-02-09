/**
 * @file blm_port_template.c
 * @brief BLM Port Layer Template Implementation
 * 
 * This is a template file for porting the bootloader to different MCU platforms.
 * Users should copy this file and implement the functions for their target MCU.
 * 
 * Supported platforms:
 * - STM32F1/F4/G4 (define USE_STM32)
 * - AT32F421/F425 (define USE_AT32)
 * 
 * This implementation uses CMSIS register definitions where possible.
 */
#include "blm_port.h"
#include "../userconfig.h"
#include <perf_counter.h>

/* Platform selection - uncomment one */
// #define USE_STM32
// #define USE_AT32

/*============================================================================
 * Platform-specific includes
 *===========================================================================*/
#if defined(USE_STM32)
    /* STM32 CMSIS headers */
    // #include "stm32f4xx.h"  // or stm32g4xx.h etc.
#elif defined(USE_AT32)
    /* AT32 headers */
    // #include "at32f421.h"
#else
    /* Template mode - for reference only */
    #warning "No platform defined, using template stubs"
#endif

/*============================================================================
 * Internal variables
 *===========================================================================*/
/* Tick is now managed by perfc_port_user.c */
extern uint32_t perfc_port_get_tick_ms(void);

/*============================================================================
 * UART Implementation
 *===========================================================================*/

int blm_port_UartInit(uint32_t wBaudrate)
{
    (void)wBaudrate;
    
#if defined(USE_STM32)
    /* TODO: STM32 UART init using CMSIS registers
     * 1. Enable GPIO clock
     * 2. Configure TX/RX pins
     * 3. Enable USART clock
     * 4. Configure baud rate: USART->BRR = SystemCoreClock / wBaudrate
     * 5. Enable USART: USART->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE
     */
#elif defined(USE_AT32)
    /* TODO: AT32 UART init */
#endif
    
    return 0;
}

int blm_port_UartSend(const uint8_t *pchData, uint16_t hwLen)
{
    if (pchData == NULL) {
        return -1;
    }
    
#if defined(USE_STM32) || defined(USE_AT32)
    /* TODO: Implement UART transmit
     * for (uint16_t i = 0; i < hwLen; i++) {
     *     while (!(USART->SR & USART_SR_TXE));
     *     USART->DR = pchData[i];
     * }
     * while (!(USART->SR & USART_SR_TC));
     */
#endif
    
    return hwLen;
}

int blm_port_UartRecv(uint8_t *pchData, uint16_t hwLen, uint32_t wTimeoutMs)
{
    if (pchData == NULL) {
        return -1;
    }
    
    uint16_t hwReceived = 0;
    uint32_t wStart = blm_port_GetTickMs();
    
#if defined(USE_STM32) || defined(USE_AT32)
    /* TODO: Implement UART receive with timeout
     * while (hwReceived < hwLen) {
     *     if (USART->SR & USART_SR_RXNE) {
     *         pchData[hwReceived++] = USART->DR;
     *     }
     *     if ((blm_port_GetTickMs() - wStart) >= wTimeoutMs) {
     *         return -2;  // Timeout
     *     }
     * }
     */
#endif
    (void)wStart;
    (void)wTimeoutMs;
    
    return hwReceived;
}

int blm_port_UartAvailable(void)
{
#if defined(USE_STM32) || defined(USE_AT32)
    /* TODO: Check RXNE flag
     * return (USART->SR & USART_SR_RXNE) ? 1 : 0;
     */
#endif
    return 0;
}

void blm_port_UartFlush(void)
{
#if defined(USE_STM32) || defined(USE_AT32)
    /* TODO: Read and discard any pending data */
#endif
}

/*============================================================================
 * Flash Implementation
 *===========================================================================*/

int blm_port_FlashUnlock(void)
{
#if defined(USE_STM32)
    /* STM32 Flash unlock sequence
     * FLASH->KEYR = 0x45670123;
     * FLASH->KEYR = 0xCDEF89AB;
     */
#elif defined(USE_AT32)
    /* AT32 Flash unlock sequence */
#endif
    return 0;
}

int blm_port_FlashLock(void)
{
#if defined(USE_STM32)
    /* FLASH->CR |= FLASH_CR_LOCK; */
#elif defined(USE_AT32)
    /* AT32 Flash lock */
#endif
    return 0;
}

int blm_port_FlashErase(uint32_t wAddr, uint32_t wSize)
{
    (void)wAddr;
    (void)wSize;
    
#if defined(USE_STM32)
    /* TODO: Page/Sector erase
     * 1. Wait for BSY
     * 2. Set PER (page erase) or SER (sector erase)
     * 3. Write address
     * 4. Set STRT
     * 5. Wait for BSY
     * 6. Check for errors
     */
#elif defined(USE_AT32)
    /* AT32 Flash erase */
#endif
    
    return 0;
}

int blm_port_FlashWrite(uint32_t wAddr, const uint8_t *pchData, uint32_t wLen)
{
    (void)wAddr;
    (void)pchData;
    (void)wLen;
    
#if defined(USE_STM32)
    /* TODO: Flash program
     * 1. Wait for BSY
     * 2. Set PG
     * 3. Write halfword/word
     * 4. Wait for BSY
     * 5. Check for errors
     */
#elif defined(USE_AT32)
    /* AT32 Flash program */
#endif
    
    return 0;
}

int blm_port_FlashRead(uint32_t wAddr, uint8_t *pchData, uint32_t wLen)
{
    if (pchData == NULL) {
        return -1;
    }
    
    /* Direct memory read */
    const uint8_t *pchSrc = (const uint8_t *)wAddr;
    for (uint32_t i = 0; i < wLen; i++) {
        pchData[i] = pchSrc[i];
    }
    
    return 0;
}

uint32_t blm_port_FlashGetPageSize(uint32_t wAddr)
{
    (void)wAddr;
    
#if defined(USE_STM32)
    /* STM32F1: 1KB or 2KB pages
     * STM32F4: Variable sector sizes
     * STM32G4: 2KB pages
     */
    return 2048;
#elif defined(USE_AT32)
    /* AT32F421: 1KB pages */
    return 1024;
#else
    return 1024;
#endif
}

/*============================================================================
 * System Implementation
 *===========================================================================*/

void blm_port_SystemReset(void)
{
    /* CMSIS NVIC_SystemReset */
    __asm volatile ("dsb");
    /* SCB->AIRCR = (0x5FA << 16) | (1 << 2); */
    __asm volatile ("dsb");
    while(1);
}

void blm_port_JumpToApp(uint32_t wAppAddr)
{
    typedef void (*pFunction)(void);
    
    /* Get reset handler address from vector table */
    uint32_t wJumpAddr = *((volatile uint32_t *)(wAppAddr + 4));
    pFunction JumpToApp = (pFunction)wJumpAddr;
    
    /* Set MSP from vector table */
    __asm volatile ("msr msp, %0" : : "r" (*((volatile uint32_t *)wAppAddr)));
    
    /* Jump to application */
    JumpToApp();
}

uint32_t blm_port_GetTickMs(void)
{
    return perfc_port_get_tick_ms();
}

void blm_port_DelayMs(uint32_t wMs)
{
    perfc_delay_ms(wMs);
}

int blm_port_IsUpgradeButtonPressed(void)
{
    /* TODO: Check upgrade button GPIO
     * return !(GPIOA->IDR & GPIO_IDR_0);  // Active low
     */
    return 0;
}

/* SysTick handling is now done by perfc_port_user.c */
