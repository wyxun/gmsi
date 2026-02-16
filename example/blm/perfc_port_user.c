/**
 * @file perfc_port_user.c
 * @brief perf_counter porting implementation (chip-agnostic)
 * 
 * Uses SysTick as the system timer with maximum reload value
 * for high-precision timing. Only depends on core_cm4.h.
 */

/*============================ INCLUDES ======================================*/
#undef __PERF_COUNT_PLATFORM_SPECIFIC_HEADER__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define __IMPLEMENT_PERF_COUNTER
#include "perf_counter.h"

#if defined(AT32F407xx)
#   include "cmsis/at32f407xx.h"
#elif defined(STM32G431xx)
#   include "cmsis/stm32g431xx.h"
#endif

#include "cmsis/core_cm4.h"

#if defined(__IS_COMPILER_GCC__) || defined(__clang__)
#   pragma GCC diagnostic ignored "-Wattributes"
#endif

#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wunknown-warning-option"
#   pragma clang diagnostic ignored "-Wreserved-identifier"
#   pragma clang diagnostic ignored "-Wconditional-uninitialized"
#   pragma clang diagnostic ignored "-Wcast-align"
#   pragma clang diagnostic ignored "-Wmissing-prototypes"
#   pragma clang diagnostic ignored "-Wimplicit-function-declaration"
#endif

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/

extern uint32_t SystemCoreClock;
/*============================ LOCAL VARIABLES ===============================*/

/* Overflow counter for extending SysTick to 64-bit */
static volatile uint32_t s_wSysTickOvfCnt = 0;

/*============================ PROTOTYPES ====================================*/
/* low level interface for porting */
extern uint32_t perfc_port_get_system_timer_freq(void);
extern int64_t perfc_port_get_system_timer_top(void);
extern bool perfc_port_is_system_timer_ovf_pending(void);
extern bool perfc_port_init_system_timer(bool bTimerOccupied);
extern int64_t perfc_port_get_system_timer_elapsed(void);
extern void perfc_port_clear_system_timer_ovf_pending(void);
extern void perfc_port_stop_system_timer_counting(void);
extern void perfc_port_clear_system_timer_counter(void);

/*============================ IMPLEMENTATION ================================*/

#if __PERFC_USE_USER_CUSTOM_PORTING__

/**
 * @brief Initialize system timer (SysTick) - called by perfc_init
 * @param bIsTimeOccupied true if timer is already used by application
 * @return true on success
 */
bool perfc_port_init_system_timer(bool bIsTimeOccupied)
{
    bool bResult = true;
    
    do {
        if (bIsTimeOccupied) {
            /* Timer already in use by application, just hook into it */
            break;
        }

        __IRQ_SAFE {
            /* Configure SysTick for 1ms interrupt period */
            SysTick->LOAD = SystemCoreClock / 1000 - 1;
            SysTick->VAL  = 0;
            SysTick->CTRL = SysTick_CTRL_CLKSOURCE |    /* Use processor clock */
                            SysTick_CTRL_TICKINT   |    /* Enable interrupt */
                            SysTick_CTRL_ENABLE;        /* Enable SysTick */
            
            s_wSysTickOvfCnt = 0;
        }
    } while(0);
    
    return bResult;
}

/**
 * @brief Get system timer frequency
 * @return Frequency in Hz
 */
uint32_t perfc_port_get_system_timer_freq(void)
{
    return SystemCoreClock;
}

/**
 * @brief Check if system timer overflow is pending
 * @return true if overflow pending
 */
bool perfc_port_is_system_timer_ovf_pending(void)
{
    return (SysTick->CTRL & SysTick_CTRL_COUNTFLAG) != 0;
}

/**
 * @brief Get the top (reload) value of the system timer
 * @return Top value
 */
int64_t perfc_port_get_system_timer_top(void)
{
    return (int64_t)SysTick->LOAD;
}

/**
 * @brief Get elapsed count since last overflow
 * @return Elapsed ticks (SysTick counts down)
 */
int64_t perfc_port_get_system_timer_elapsed(void)
{
    /* SysTick counts down, so elapsed = LOAD - VAL */
    return (int64_t)(SysTick->LOAD - SysTick->VAL);
}

/**
 * @brief Clear the overflow pending flag
 */
void perfc_port_clear_system_timer_ovf_pending(void)
{
    /* Reading CTRL clears the countflag */
    (void)SysTick->CTRL;
}

/**
 * @brief Stop system timer counting
 */
void perfc_port_stop_system_timer_counting(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE;
}

/**
 * @brief Clear system timer counter
 */
void perfc_port_clear_system_timer_counter(void)
{
    SysTick->VAL = 0;
    s_wSysTickOvfCnt = 0;
}

#endif  /* __PERFC_USE_USER_CUSTOM_PORTING__ */
