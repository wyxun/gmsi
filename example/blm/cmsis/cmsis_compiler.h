/**
 * @file cmsis_compiler.h
 * @brief CMSIS Compiler Abstraction (Minimal for Bootloader)
 * 
 * Provides compiler-specific macros for ARM/Clang compilers.
 */
#ifndef __CMSIS_COMPILER_H__
#define __CMSIS_COMPILER_H__

#include <stdint.h>

/*============================ COMPILER DETECTION ============================*/

#if defined(__clang__)
/* LLVM/Clang Compiler */
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
/* GNU GCC Compiler */
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
/* ARM Compiler 5/6 */
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

/*============================ CORE INTRINSICS ===============================*/

/**
 * @brief No Operation
 */
__STATIC_FORCEINLINE void __NOP(void)
{
    __ASM volatile ("nop");
}

/**
 * @brief Wait For Interrupt
 */
__STATIC_FORCEINLINE void __WFI(void)
{
    __ASM volatile ("wfi":::"memory");
}

/**
 * @brief Wait For Event
 */
__STATIC_FORCEINLINE void __WFE(void)
{
    __ASM volatile ("wfe":::"memory");
}

/**
 * @brief Send Event
 */
__STATIC_FORCEINLINE void __SEV(void)
{
    __ASM volatile ("sev");
}

/**
 * @brief Instruction Synchronization Barrier
 */
__STATIC_FORCEINLINE void __ISB(void)
{
    __ASM volatile ("isb 0xF":::"memory");
}

/**
 * @brief Data Synchronization Barrier
 */
__STATIC_FORCEINLINE void __DSB(void)
{
    __ASM volatile ("dsb 0xF":::"memory");
}

/**
 * @brief Data Memory Barrier
 */
__STATIC_FORCEINLINE void __DMB(void)
{
    __ASM volatile ("dmb 0xF":::"memory");
}

/**
 * @brief Disable IRQ Interrupts
 */
__STATIC_FORCEINLINE void __disable_irq(void)
{
    __ASM volatile ("cpsid i" : : : "memory");
}

/**
 * @brief Enable IRQ Interrupts
 */
__STATIC_FORCEINLINE void __enable_irq(void)
{
    __ASM volatile ("cpsie i" : : : "memory");
}

/**
 * @brief Get PRIMASK
 */
__STATIC_FORCEINLINE uint32_t __get_PRIMASK(void)
{
    uint32_t result;
    __ASM volatile ("MRS %0, primask" : "=r" (result));
    return result;
}

/**
 * @brief Set PRIMASK
 */
__STATIC_FORCEINLINE void __set_PRIMASK(uint32_t priMask)
{
    __ASM volatile ("MSR primask, %0" : : "r" (priMask) : "memory");
}

/**
 * @brief Get Main Stack Pointer
 */
__STATIC_FORCEINLINE uint32_t __get_MSP(void)
{
    uint32_t result;
    __ASM volatile ("MRS %0, msp" : "=r" (result));
    return result;
}

/**
 * @brief Set Main Stack Pointer
 */
__STATIC_FORCEINLINE void __set_MSP(uint32_t topOfMainStack)
{
    __ASM volatile ("MSR msp, %0" : : "r" (topOfMainStack) : );
}

#endif /* __CMSIS_COMPILER_H__ */
