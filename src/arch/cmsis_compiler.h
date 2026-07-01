/**
 * @file    cmsis_compiler.h
 * @brief   CMSIS Compiler Abstraction — architecture-aware minimal shim
 *
 * Provides compiler-specific macros (__STATIC_INLINE, __WEAK, etc.) and
 * architecture-specific intrinsics (__DSB, __ISB, __disable_irq, ...) for
 * both RISC-V and Cortex-M targets.
 *
 * Placed in src/arch/ so it is automatically available via the MODUS_INCLUDES
 * search path.  No per-chip copy needed.
 */
#ifndef __CMSIS_COMPILER_H__
#define __CMSIS_COMPILER_H__

#include <stdint.h>

/*============================================================================
 * Compiler detection — GCC / Clang (LLVM) / ARM Compiler
 *============================================================================*/
#if defined(__clang__)
  #ifndef   __ASM
    #define __ASM                     __asm
  #endif
  #ifndef   __INLINE
    #define __INLINE                  inline
  #endif
  #ifndef   __STATIC_INLINE
    #define __STATIC_INLINE           static inline
  #endif
  #ifndef   __STATIC_FORCEINLINE
    #define __STATIC_FORCEINLINE      __attribute__((always_inline)) static inline
  #endif
  #ifndef   __NO_RETURN
    #define __NO_RETURN               __attribute__((__noreturn__))
  #endif
  #ifndef   __USED
    #define __USED                    __attribute__((used))
  #endif
  #ifndef   __WEAK
    #define __WEAK                    __attribute__((weak))
  #endif
  #ifndef   __PACKED
    #define __PACKED                  __attribute__((packed, aligned(1)))
  #endif
  #ifndef   __PACKED_STRUCT
    #define __PACKED_STRUCT           struct __attribute__((packed, aligned(1)))
  #endif
  #ifndef   __ALIGNED
    #define __ALIGNED(x)              __attribute__((aligned(x)))
  #endif
  #ifndef   __RESTRICT
    #define __RESTRICT                __restrict
  #endif
  #ifndef   __COMPILER_BARRIER
    #define __COMPILER_BARRIER()      __ASM volatile("":::"memory")
  #endif

#elif defined(__GNUC__)
  #ifndef   __ASM
    #define __ASM                     __asm
  #endif
  #ifndef   __INLINE
    #define __INLINE                  inline
  #endif
  #ifndef   __STATIC_INLINE
    #define __STATIC_INLINE           static inline
  #endif
  #ifndef   __STATIC_FORCEINLINE
    #define __STATIC_FORCEINLINE      __attribute__((always_inline)) static inline
  #endif
  #ifndef   __NO_RETURN
    #define __NO_RETURN               __attribute__((__noreturn__))
  #endif
  #ifndef   __USED
    #define __USED                    __attribute__((used))
  #endif
  #ifndef   __WEAK
    #define __WEAK                    __attribute__((weak))
  #endif
  #ifndef   __PACKED
    #define __PACKED                  __attribute__((packed, aligned(1)))
  #endif
  #ifndef   __PACKED_STRUCT
    #define __PACKED_STRUCT           struct __attribute__((packed, aligned(1)))
  #endif
  #ifndef   __ALIGNED
    #define __ALIGNED(x)              __attribute__((aligned(x)))
  #endif
  #ifndef   __RESTRICT
    #define __RESTRICT                __restrict
  #endif
  #ifndef   __COMPILER_BARRIER
    #define __COMPILER_BARRIER()      __ASM volatile("":::"memory")
  #endif

#elif defined(__ARMCC_VERSION)
  #ifndef   __ASM
    #define __ASM                     __asm
  #endif
  #ifndef   __INLINE
    #define __INLINE                  __inline
  #endif
  #ifndef   __STATIC_INLINE
    #define __STATIC_INLINE           static __inline
  #endif
  #ifndef   __STATIC_FORCEINLINE
    #define __STATIC_FORCEINLINE      __attribute__((always_inline)) static __inline
  #endif
  #ifndef   __NO_RETURN
    #define __NO_RETURN               __attribute__((__noreturn__))
  #endif
  #ifndef   __USED
    #define __USED                    __attribute__((used))
  #endif
  #ifndef   __WEAK
    #define __WEAK                    __attribute__((weak))
  #endif
  #ifndef   __PACKED
    #define __PACKED                  __attribute__((packed))
  #endif
  #ifndef   __PACKED_STRUCT
    #define __PACKED_STRUCT           __packed struct
  #endif
  #ifndef   __ALIGNED
    #define __ALIGNED(x)              __attribute__((aligned(x)))
  #endif
  #ifndef   __RESTRICT
    #define __RESTRICT                __restrict
  #endif
  #ifndef   __COMPILER_BARRIER
    #define __COMPILER_BARRIER()      __ASM volatile("":::"memory")
  #endif

#else
  #error "Unknown compiler"
#endif

/*============================================================================
 * Architecture-specific intrinsics
 *============================================================================*/
#if defined(__riscv)

/* ---- RISC-V ---- */

__STATIC_FORCEINLINE void __NOP(void)
{
    __ASM volatile ("nop");
}

__STATIC_FORCEINLINE void __WFI(void)
{
    __ASM volatile ("wfi":::"memory");
}

__STATIC_FORCEINLINE void __ISB(void)
{
    __ASM volatile ("fence.i":::"memory");
}

__STATIC_FORCEINLINE void __DSB(void)
{
    __ASM volatile ("fence":::"memory");
}

__STATIC_FORCEINLINE void __DMB(void)
{
    __ASM volatile ("fence":::"memory");
}

__STATIC_FORCEINLINE void __disable_irq(void)
{
    __ASM volatile ("csrci mstatus, 8" ::: "memory");
}

__STATIC_FORCEINLINE void __enable_irq(void)
{
    __ASM volatile ("csrsi mstatus, 8" ::: "memory");
}

__STATIC_FORCEINLINE uint32_t __get_PRIMASK(void)
{
    uint32_t result;
    __ASM volatile ("csrr %0, mstatus" : "=r"(result));
    return (result & 8) ? 0 : 1;
}

__STATIC_FORCEINLINE void __set_PRIMASK(uint32_t priMask)
{
    if (priMask) {
        __disable_irq();
    } else {
        __enable_irq();
    }
}

__STATIC_FORCEINLINE uint32_t __get_MSP(void)
{
    uint32_t result;
    __ASM volatile ("mv %0, sp" : "=r"(result));
    return result;
}

__STATIC_FORCEINLINE void __set_MSP(uint32_t topOfMainStack)
{
    __ASM volatile ("mv sp, %0" :: "r"(topOfMainStack));
}

#else
/* ---- Cortex-M (default) ---- */

__STATIC_FORCEINLINE void __NOP(void)
{
    __ASM volatile ("nop");
}

__STATIC_FORCEINLINE void __WFI(void)
{
    __ASM volatile ("wfi":::"memory");
}

__STATIC_FORCEINLINE void __WFE(void)
{
    __ASM volatile ("wfe":::"memory");
}

__STATIC_FORCEINLINE void __SEV(void)
{
    __ASM volatile ("sev");
}

__STATIC_FORCEINLINE void __ISB(void)
{
    __ASM volatile ("isb 0xF":::"memory");
}

__STATIC_FORCEINLINE void __DSB(void)
{
    __ASM volatile ("dsb 0xF":::"memory");
}

__STATIC_FORCEINLINE void __DMB(void)
{
    __ASM volatile ("dmb 0xF":::"memory");
}

__STATIC_FORCEINLINE void __disable_irq(void)
{
    __ASM volatile ("cpsid i" ::: "memory");
}

__STATIC_FORCEINLINE void __enable_irq(void)
{
    __ASM volatile ("cpsie i" ::: "memory");
}

__STATIC_FORCEINLINE uint32_t __get_PRIMASK(void)
{
    uint32_t result;
    __ASM volatile ("MRS %0, primask" : "=r"(result));
    return result;
}

__STATIC_FORCEINLINE void __set_PRIMASK(uint32_t priMask)
{
    __ASM volatile ("MSR primask, %0" :: "r"(priMask) : "memory");
}

__STATIC_FORCEINLINE uint32_t __get_MSP(void)
{
    uint32_t result;
    __ASM volatile ("MRS %0, msp" : "=r"(result));
    return result;
}

__STATIC_FORCEINLINE void __set_MSP(uint32_t topOfMainStack)
{
    __ASM volatile ("MSR msp, %0" :: "r"(topOfMainStack));
}

#endif /* __riscv */

#endif /* __CMSIS_COMPILER_H__ */
