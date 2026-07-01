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
 * RISC-V (QingKe V4C) 底层移植实现
 *
 * 部分 RISC-V 芯片(如 CH592)的 mcycle CSR 硬件未实现, 因此使用软件方案:
 *
 * 1. SysTick_Handler 每 1ms 将 g_perfc_systick_overflow 递增
 * 2. perfc_port_get_system_timer_elapsed() 返回:
 *    g_perfc_systick_overflow + SysTick->CNT
 *    这是一个真正的 64 位单调递增计数器, 永不回绕
 * 3. 溢出检测和 top 均返回 "永不回绕" 值
 *
 * SysTick_Handler 中必须调用:
 *   perfc_port_insert_to_system_timer_insert_ovf_handler();
 *
 * 若芯片正确实现了 mcycle CSR, 可定义 MODUS_PERFC_USE_MCYCLE 切换到硬件方案。
 *===========================================================================*/

#if defined(MODUS_PERFC_USE_MCYCLE)
/* 硬件 mcycle 方案 — 用于实现了 mcycle CSR 的 RISC-V 芯片 */

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
    return true;
}

int64_t perfc_port_get_system_timer_elapsed(void)
{
    return get_riscv_cycle64();
}

#else
/* 默认: 软件 SysTick 64 位拓展计数 — 兼容所有 RISC-V 芯片 */

typedef struct
{
    volatile uint32_t CTLR;
    volatile uint32_t SR;
    volatile uint64_t CNT;
    volatile uint64_t CMP;
} perfc_SysTick_Type;

#define perfc_SysTick                 ((perfc_SysTick_Type *)0xE000F000)

/*
 * 64 位软件溢出计数器, 由 SysTick_Handler 递增
 * 每次中断加 (SystemCoreClock / 1000), 即每 1ms 的 tick 数
 */
volatile uint64_t g_perfc_systick_overflow = 0;

static inline int64_t get_riscv_systick_elapsed64(void)
{
    uint64_t ovf_before, ovf_after;
    uint64_t cnt;
    do {
        ovf_before = g_perfc_systick_overflow;
        cnt        = (uint64_t)perfc_SysTick->CNT;
        ovf_after  = g_perfc_systick_overflow;
    } while (ovf_before != ovf_after);
    return (int64_t)(ovf_before + cnt);
}

bool perfc_port_init_system_timer(bool bIsTimeOccupied)
{
    (void)bIsTimeOccupied;
    return true;
}

int64_t perfc_port_get_system_timer_elapsed(void)
{
    return get_riscv_systick_elapsed64();
}

#endif /* MODUS_PERFC_USE_MCYCLE */

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

void perfc_port_clear_system_timer_ovf_pending(void)
{
}

void perfc_port_stop_system_timer_counting(void)
{
    perfc_SysTick->CTLR = 0;
}

void perfc_port_clear_system_timer_counter(void)
{
    perfc_SysTick->CNT = 0;
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
