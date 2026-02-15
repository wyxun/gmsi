/**
 * @file core_cm4.h
 * @brief Cortex-M4 Core Peripheral Definitions (Minimal for Bootloader)
 *
 * CMSIS-style header providing CM4 core registers:
 *   - SCB (System Control Block)
 *   - SysTick (System Timer)
 *
 * This is chip-independent and shared across all CM4 targets.
 */
#ifndef __CORE_CM4_H__
#define __CORE_CM4_H__

#include "cmsis_compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MEMORY MAP ====================================*/

#define SCS_BASE            (0xE000E000UL)
#define SysTick_BASE        (SCS_BASE + 0x0010UL)
#define NVIC_BASE           (SCS_BASE + 0x0100UL)
#define SCB_BASE            (SCS_BASE + 0x0D00UL)

/*============================ SCB ===========================================*/

typedef struct {
    volatile uint32_t CPUID;        /* 0x000 CPUID Base Register */
    volatile uint32_t ICSR;         /* 0x004 Interrupt Control and State */
    volatile uint32_t VTOR;         /* 0x008 Vector Table Offset */
    volatile uint32_t AIRCR;        /* 0x00C Application Interrupt/Reset Control */
    volatile uint32_t SCR;          /* 0x010 System Control Register */
    volatile uint32_t CCR;          /* 0x014 Configuration Control */
    volatile uint8_t  SHP[12U];     /* 0x018 System Handlers Priority (4-7, 8-11, 12-15) */
    volatile uint32_t SHCSR;        /* 0x024 System Handler Control and State */
    volatile uint32_t CFSR;         /* 0x028 Configurable Fault Status */
    volatile uint32_t HFSR;         /* 0x02C HardFault Status */
    volatile uint32_t DFSR;         /* 0x030 Debug Fault Status */
    volatile uint32_t MMFAR;        /* 0x034 MemManage Fault Address */
    volatile uint32_t BFAR;         /* 0x038 BusFault Address */
    volatile uint32_t AFSR;         /* 0x03C Auxiliary Fault Status */
    volatile const uint32_t PFR[2U];/* 0x040 Processor Feature */
    volatile const uint32_t DFR;    /* 0x048 Debug Feature */
    volatile const uint32_t ADR;    /* 0x04C Auxiliary Feature */
    volatile const uint32_t MMFR[4U];/* 0x050 Memory Model Feature */
    volatile const uint32_t ISAR[5U];/* 0x060 Instruction Set Attributes */
    uint32_t RESERVED0[5U];
    volatile uint32_t CPACR;        /* 0x088 Coprocessor Access Control */
} SCB_TypeDef;

#define SCB                 ((SCB_TypeDef *)SCB_BASE)

/* SCB AIRCR bits */
#define SCB_AIRCR_VECTKEY       (0x05FAUL << 16)
#define SCB_AIRCR_SYSRESETREQ   (1UL << 2)

/* SCB ICSR bits */
#define SCB_ICSR_PENDSTCLR      (1UL << 25)
#define SCB_ICSR_PENDSTSET      (1UL << 26)

/*============================ SysTick =======================================*/

typedef struct {
    volatile uint32_t CTRL;         /* 0x000 Control and Status */
    volatile uint32_t LOAD;         /* 0x004 Reload Value */
    volatile uint32_t VAL;          /* 0x008 Current Value */
    volatile const uint32_t CALIB;  /* 0x00C Calibration */
} SysTick_TypeDef;

#define SysTick             ((SysTick_TypeDef *)SysTick_BASE)

/* SysTick CTRL bits */
#define SysTick_CTRL_ENABLE     (1UL << 0)
#define SysTick_CTRL_TICKINT    (1UL << 1)
#define SysTick_CTRL_CLKSOURCE  (1UL << 2)
#define SysTick_CTRL_COUNTFLAG  (1UL << 16)

/* SysTick LOAD mask (24-bit) */
#define SysTick_LOAD_RELOAD_Msk 0x00FFFFFFUL

/*============================ NVIC (minimal) ================================*/

typedef struct {
    volatile uint32_t ISER[8U];     /* 0x000 Interrupt Set Enable */
    uint32_t RESERVED0[24U];
    volatile uint32_t ICER[8U];     /* 0x080 Interrupt Clear Enable */
    uint32_t RESERVED1[24U];
    volatile uint32_t ISPR[8U];     /* 0x100 Interrupt Set Pending */
    uint32_t RESERVED2[24U];
    volatile uint32_t ICPR[8U];     /* 0x180 Interrupt Clear Pending */
    uint32_t RESERVED3[24U];
    volatile uint32_t IABR[8U];     /* 0x200 Interrupt Active Bit */
    uint32_t RESERVED4[56U];
    volatile uint8_t  IP[240U];     /* 0x300 Interrupt Priority */
    uint32_t RESERVED5[644U];
    volatile uint32_t STIR;         /* 0xE00 Software Trigger Interrupt */
} NVIC_TypeDef;

#define NVIC                ((NVIC_TypeDef *)NVIC_BASE)

/*============================ FPU CPACR =====================================*/

/* FPU enable: set CP10 and CP11 to full access */
#define SCB_CPACR_CP10_Full     (3UL << 20)
#define SCB_CPACR_CP11_Full     (3UL << 22)
#define SCB_CPACR_FPU_ENABLE    (SCB_CPACR_CP10_Full | SCB_CPACR_CP11_Full)

#ifdef __cplusplus
}
#endif

#endif /* __CORE_CM4_H__ */
