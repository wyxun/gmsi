#undef __PERF_COUNT_PLATFORM_SPECIFIC_HEADER__
#include <stdint.h>
#include <stdbool.h>

#define __IMPLEMENT_PERF_COUNTER
#include "perf_counter.h"

#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wunknown-warning-option"
#   pragma clang diagnostic ignored "-Wreserved-identifier"
#   pragma clang diagnostic ignored "-Wmissing-prototypes"
#   pragma clang diagnostic ignored "-Wimplicit-function-declaration"
#   pragma clang diagnostic ignored "-Wcast-align"
#endif

/*============================ 全局统一弱时钟解耦接口 ============================*/

__attribute__((weak)) uint32_t get_system_core_clock_hz(void)
{
    return 60000000UL; /* 默认兜底频率 */
}

#if defined(__riscv)
/*============================================================================
 * RISC-V 架构通用底层移植实现
 *===========================================================================*/

static inline int64_t get_riscv_cycle64(void)
{
    uint32_t cycle_l, cycle_h1, cycle_h2;
    do {
        __asm__ __volatile__("csrr %0, cycleh" : "=r"(cycle_h1));
        __asm__ __volatile__("csrr %0, cycle" : "=r"(cycle_l));
        __asm__ __volatile__("csrr %0, cycleh" : "=r"(cycle_h2));
    } while (cycle_h1 != cycle_h2);
    return (((int64_t)cycle_h1) << 32) | cycle_l;
}

bool perfc_port_init_system_timer(bool bIsTimeOccupied)
{
    (void)bIsTimeOccupied;
    /* RISC-V 的 cycle CSR 属于硬件硬件自启动计数 */
    return true;
}

uint32_t perfc_port_get_system_timer_freq(void)
{
    return get_system_core_clock_hz();
}

bool perfc_port_is_system_timer_ovf_pending(void)
{
    return false;
}

int64_t perfc_port_get_system_timer_top(void)
{
    return 0x7FFFFFFFFFFFFFFFLL;
}

int64_t perfc_port_get_system_timer_elapsed(void)
{
    return get_riscv_cycle64();
}

void perfc_port_clear_system_timer_ovf_pending(void)
{
}

void perfc_port_stop_system_timer_counting(void)
{
}

void perfc_port_clear_system_timer_counter(void)
{
}

__attribute__((noinline)) uintptr_t __perfc_port_get_sp(void)
{
    uintptr_t result;
    __asm__ volatile ("mv %0, sp" : "=r" (result));
    return result;
}

__attribute__((noinline)) void __perfc_port_set_sp(uintptr_t nSP)
{
    uintptr_t nAlignSP = nSP;
    __asm__ volatile ("mv sp, %0" : : "r" (nAlignSP));
}

#elif defined(__ARM_ARCH) || defined(__CORTEX_M) || defined(__arm__)
/*============================================================================
 * Cortex-M 架构通用底层移植实现 (零芯片头文件依赖)
 *===========================================================================*/

typedef struct {
    volatile uint32_t CTRL;  /* 0x000 (R/W)  SysTick Control and Status Register */
    volatile uint32_t LOAD;  /* 0x004 (R/W)  SysTick Reload Value Register */
    volatile uint32_t VAL;   /* 0x008 (R/W)  SysTick Current Value Register */
    volatile uint32_t CALIB; /* 0x00C (R/ )  SysTick Calibration Register */
} SysTick_Type;

#define SysTick             ((SysTick_Type *) 0xE000E010UL)

typedef struct {
    volatile uint32_t CPUID;
    volatile uint32_t ICSR;  /* Interrupt Control and State Register */
    volatile uint32_t VTOR;
    volatile uint32_t AIRCR;
    volatile uint32_t SCR;
    volatile uint32_t CCR;
    volatile uint8_t  SHP[12];
    volatile uint32_t SHCSR;
    volatile uint32_t CFSR;
} SCB_Type;

#define SCB                 ((SCB_Type *) 0xE000ED00UL)

#define SysTick_CTRL_ENABLE_Msk     (1UL << 0)
#define SysTick_CTRL_TICKINT_Msk    (1UL << 1)
#define SysTick_CTRL_CLKSOURCE_Msk  (1UL << 2)
#define SysTick_CTRL_COUNTFLAG_Msk  (1UL << 16)
#define SCB_ICSR_PENDSTSET_Msk      (1UL << 26)
#define SCB_ICSR_PENDSTCLR_Msk      (1UL << 25)

bool perfc_port_init_system_timer(bool bIsTimeOccupied)
{
    if (!bIsTimeOccupied) {
        uint32_t wReload = get_system_core_clock_hz() / 1000U - 1U;
        SysTick->LOAD = wReload;
        SysTick->VAL  = 0UL;
        SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk
                      | SysTick_CTRL_TICKINT_Msk
                      | SysTick_CTRL_ENABLE_Msk;
    }
    return true;
}

uint32_t perfc_port_get_system_timer_freq(void)
{
    return get_system_core_clock_hz();
}

bool perfc_port_is_system_timer_ovf_pending(void)
{
    return (SCB->ICSR & SCB_ICSR_PENDSTSET_Msk) != 0U;
}

int64_t perfc_port_get_system_timer_top(void)
{
    return (int64_t)(SysTick->LOAD + 1U);
}

int64_t perfc_port_get_system_timer_elapsed(void)
{
    return (int64_t)(SysTick->LOAD - SysTick->VAL);
}

void perfc_port_clear_system_timer_ovf_pending(void)
{
    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk;
}

void perfc_port_stop_system_timer_counting(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void perfc_port_clear_system_timer_counter(void)
{
    SysTick->VAL = 0UL;
}

__attribute__((noinline)) uintptr_t __perfc_port_get_sp(void)
{
    uintptr_t result;
    __asm__ volatile ("mov %0, sp" : "=r" (result));
    return result;
}

__attribute__((noinline)) void __perfc_port_set_sp(uintptr_t nSP)
{
    uint32_t nAlign8Padding = nSP;
    __asm__ volatile ("mov sp, %0" : : "r" (nAlign8Padding));
}

#endif
