#ifndef __PERFC_PORT_H__
#define __PERFC_PORT_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 声明全局统一的时钟获取接口 */
uint32_t get_system_core_clock_hz(void);

/* 定义 perf_counter 期待的通用 inline 宏 */
#ifndef __STATIC_INLINE
#define __STATIC_INLINE   static inline
#endif

typedef uint32_t perfc_global_interrupt_status_t;

#if defined(__riscv)
/*============================ RISC-V 架构全局开关中断 ============================*/

static inline uint32_t perfc_port_disable_global_interrupt(void)
{
    uint32_t status;
    /* 清零 mstatus 中的 MIE (bit 3) 并回传原值 */
    __asm__ volatile ("csrrc %0, mstatus, %1" : "=r"(status) : "i"(8));
    return status & 8;
}

static inline void perfc_port_resume_global_interrupt(uint32_t status)
{
    if (status & 8) {
        __asm__ volatile ("csrrs zero, mstatus, %0" :: "i"(8));
    }
}

#elif defined(__ARM_ARCH) || defined(__CORTEX_M) || defined(__arm__)
/*=========================== Cortex-M 架构全局开关中断 ============================*/

static inline uint32_t perfc_port_disable_global_interrupt(void)
{
    uint32_t status;
    __asm__ volatile ("mrs %0, primask" : "=r"(status));
    __asm__ volatile ("cpsid i" ::: "memory");
    return status;
}

static inline void perfc_port_resume_global_interrupt(uint32_t status)
{
    __asm__ volatile ("msr primask, %0" :: "r"(status) : "memory");
}

#else
/*============================= 其他架构空实现 ==================================*/
static inline uint32_t perfc_port_disable_global_interrupt(void) { return 0; }
static inline void perfc_port_resume_global_interrupt(uint32_t status) { (void)status; }
#endif

#ifdef __cplusplus
}
#endif

#endif /* __PERFC_PORT_H__ */
