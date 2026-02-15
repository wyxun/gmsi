/**
 * @file startup_at32f4.c
 * @brief AT32F407 Startup Code (C-based)
 *
 * Contains vector table and reset handler for Cortex-M4F.
 * Compatible with LLVM Embedded Toolchain for Arm and GCC.
 */

#include <stdint.h>

/*============================ EXTERNAL SYMBOLS ==============================*/

extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;

extern int main(void);

/*============================ PROTOTYPES ====================================*/

void Reset_Handler(void);
void Default_Handler(void);

/* Cortex-M4 System Exceptions */
void NMI_Handler(void)          __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)          __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)       __attribute__((weak, alias("Default_Handler")));
extern void SysTick_Handler(void);   /* Extern declaration to allow --wrap */

/* AT32F407 Peripheral Interrupts */
void WWDT_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void PVM_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void TAMP_STAMP_IRQHandler(void)       __attribute__((weak, alias("Default_Handler")));
void ERTC_WKUP_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void CRM_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void EXINT0_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void EXINT1_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void EXINT2_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void EXINT3_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void EXINT4_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel1_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel2_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel3_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel4_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel5_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel6_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel7_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void ADC1_2_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void USBFS_H_CAN1_TX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USBFS_L_CAN1_RX0_IRQHandler(void)__attribute__((weak, alias("Default_Handler")));
void CAN1_RX1_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void CAN1_SE_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));
void EXINT9_5_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));
void TMR1_BRK_TMR9_IRQHandler(void)   __attribute__((weak, alias("Default_Handler")));
void TMR1_OVF_TMR10_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void TMR1_TRG_HALL_TMR11_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TMR1_CH_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));
void TMR2_GLOBAL_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void TMR3_GLOBAL_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void TMR4_GLOBAL_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void I2C1_EVT_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void I2C1_ERR_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void I2C2_EVT_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void I2C2_ERR_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void SPI1_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void SPI2_I2S2EXT_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void USART1_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));
void USART3_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));
void EXINT15_10_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void ERTCAlarm_IRQHandler(void)       __attribute__((weak, alias("Default_Handler")));
void USBFSWakeUp_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void TMR8_BRK_TMR12_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void TMR8_OVF_TMR13_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void TMR8_TRG_HALL_TMR14_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TMR8_CH_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));
void ADC3_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void XMC_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void SDIO1_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void TMR5_GLOBAL_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void SPI3_I2S3EXT_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void UART4_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void UART5_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void TMR6_GLOBAL_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void TMR7_GLOBAL_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel1_IRQHandler(void)   __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel2_IRQHandler(void)   __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel3_IRQHandler(void)   __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel4_5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SDIO2_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void I2C3_EVT_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void I2C3_ERR_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void SPI4_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void CAN2_TX_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));
void CAN2_RX0_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void CAN2_RX1_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void CAN2_SE_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));
void ACC_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void USBFS_MAPH_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void USBFS_MAPL_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel6_7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART6_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));
void UART7_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void UART8_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void EMAC_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void EMAC_WKUP_IRQHandler(void)       __attribute__((weak, alias("Default_Handler")));

/*============================ VECTOR TABLE ==================================*/

__attribute__((used, section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
    (void (*)(void))(&_estack),

    /* Cortex-M4 System Exceptions */
    Reset_Handler,              /* -15 Reset */
    NMI_Handler,                /* -14 NMI */
    HardFault_Handler,          /* -13 Hard Fault */
    MemManage_Handler,          /* -12 MPU Fault */
    BusFault_Handler,           /* -11 Bus Fault */
    UsageFault_Handler,         /* -10 Usage Fault */
    0,                          /* -9  Reserved */
    0,                          /* -8  Reserved */
    0,                          /* -7  Reserved */
    0,                          /* -6  Reserved */
    SVC_Handler,                /* -5  SVC */
    DebugMon_Handler,           /* -4  Debug Monitor */
    0,                          /* -3  Reserved */
    PendSV_Handler,             /* -2  PendSV */
    SysTick_Handler,            /* -1  SysTick */

    /* AT32F407 Peripheral Interrupts */
    WWDT_IRQHandler,            /* 0   Window Watchdog */
    PVM_IRQHandler,             /* 1   PVM */
    TAMP_STAMP_IRQHandler,      /* 2   Tamper/Timestamp */
    ERTC_WKUP_IRQHandler,       /* 3   ERTC Wakeup */
    FLASH_IRQHandler,           /* 4   Flash */
    CRM_IRQHandler,             /* 5   CRM */
    EXINT0_IRQHandler,          /* 6   EXINT Line 0 */
    EXINT1_IRQHandler,          /* 7   EXINT Line 1 */
    EXINT2_IRQHandler,          /* 8   EXINT Line 2 */
    EXINT3_IRQHandler,          /* 9   EXINT Line 3 */
    EXINT4_IRQHandler,          /* 10  EXINT Line 4 */
    DMA1_Channel1_IRQHandler,   /* 11  DMA1 Channel 1 */
    DMA1_Channel2_IRQHandler,   /* 12  DMA1 Channel 2 */
    DMA1_Channel3_IRQHandler,   /* 13  DMA1 Channel 3 */
    DMA1_Channel4_IRQHandler,   /* 14  DMA1 Channel 4 */
    DMA1_Channel5_IRQHandler,   /* 15  DMA1 Channel 5 */
    DMA1_Channel6_IRQHandler,   /* 16  DMA1 Channel 6 */
    DMA1_Channel7_IRQHandler,   /* 17  DMA1 Channel 7 */
    ADC1_2_IRQHandler,          /* 18  ADC1 & ADC2 */
    USBFS_H_CAN1_TX_IRQHandler,/* 19  USB FS High / CAN1 TX */
    USBFS_L_CAN1_RX0_IRQHandler,/* 20 USB FS Low / CAN1 RX0 */
    CAN1_RX1_IRQHandler,       /* 21  CAN1 RX1 */
    CAN1_SE_IRQHandler,        /* 22  CAN1 SE */
    EXINT9_5_IRQHandler,        /* 23  EXINT Lines 5-9 */
    TMR1_BRK_TMR9_IRQHandler,  /* 24  TMR1 Break / TMR9 */
    TMR1_OVF_TMR10_IRQHandler, /* 25  TMR1 Overflow / TMR10 */
    TMR1_TRG_HALL_TMR11_IRQHandler, /* 26  TMR1 Trigger / TMR11 */
    TMR1_CH_IRQHandler,         /* 27  TMR1 Channel */
    TMR2_GLOBAL_IRQHandler,     /* 28  TMR2 */
    TMR3_GLOBAL_IRQHandler,     /* 29  TMR3 */
    TMR4_GLOBAL_IRQHandler,     /* 30  TMR4 */
    I2C1_EVT_IRQHandler,       /* 31  I2C1 Event */
    I2C1_ERR_IRQHandler,       /* 32  I2C1 Error */
    I2C2_EVT_IRQHandler,       /* 33  I2C2 Event */
    I2C2_ERR_IRQHandler,       /* 34  I2C2 Error */
    SPI1_IRQHandler,           /* 35  SPI1 */
    SPI2_I2S2EXT_IRQHandler,   /* 36  SPI2 / I2S2EXT */
    USART1_IRQHandler,         /* 37  USART1 */
    USART2_IRQHandler,         /* 38  USART2 */
    USART3_IRQHandler,         /* 39  USART3 */
    EXINT15_10_IRQHandler,     /* 40  EXINT Lines 10-15 */
    ERTCAlarm_IRQHandler,      /* 41  ERTC Alarm */
    USBFSWakeUp_IRQHandler,    /* 42  USB FS Wakeup */
    TMR8_BRK_TMR12_IRQHandler, /* 43  TMR8 Break / TMR12 */
    TMR8_OVF_TMR13_IRQHandler, /* 44  TMR8 Overflow / TMR13 */
    TMR8_TRG_HALL_TMR14_IRQHandler, /* 45  TMR8 Trigger / TMR14 */
    TMR8_CH_IRQHandler,        /* 46  TMR8 Channel */
    ADC3_IRQHandler,           /* 47  ADC3 */
    XMC_IRQHandler,            /* 48  XMC */
    SDIO1_IRQHandler,          /* 49  SDIO1 */
    TMR5_GLOBAL_IRQHandler,    /* 50  TMR5 */
    SPI3_I2S3EXT_IRQHandler,   /* 51  SPI3 / I2S3EXT */
    UART4_IRQHandler,          /* 52  UART4 */
    UART5_IRQHandler,          /* 53  UART5 */
    TMR6_GLOBAL_IRQHandler,    /* 54  TMR6 */
    TMR7_GLOBAL_IRQHandler,    /* 55  TMR7 */
    DMA2_Channel1_IRQHandler,  /* 56  DMA2 Channel 1 */
    DMA2_Channel2_IRQHandler,  /* 57  DMA2 Channel 2 */
    DMA2_Channel3_IRQHandler,  /* 58  DMA2 Channel 3 */
    DMA2_Channel4_5_IRQHandler,/* 59  DMA2 Channel 4-5 */
    SDIO2_IRQHandler,          /* 60  SDIO2 */
    I2C3_EVT_IRQHandler,       /* 61  I2C3 Event */
    I2C3_ERR_IRQHandler,       /* 62  I2C3 Error */
    SPI4_IRQHandler,           /* 63  SPI4 */
    0,                          /* 64  Reserved */
    0,                          /* 65  Reserved */
    0,                          /* 66  Reserved */
    0,                          /* 67  Reserved */
    CAN2_TX_IRQHandler,        /* 68  CAN2 TX */
    CAN2_RX0_IRQHandler,       /* 69  CAN2 RX0 */
    CAN2_RX1_IRQHandler,       /* 70  CAN2 RX1 */
    CAN2_SE_IRQHandler,        /* 71  CAN2 SE */
    ACC_IRQHandler,            /* 72  ACC */
    USBFS_MAPH_IRQHandler,     /* 73  USB FS MAP H */
    USBFS_MAPL_IRQHandler,     /* 74  USB FS MAP L */
    DMA2_Channel6_7_IRQHandler,/* 75  DMA2 Channel 6-7 */
    USART6_IRQHandler,         /* 76  USART6 */
    UART7_IRQHandler,          /* 77  UART7 */
    UART8_IRQHandler,          /* 78  UART8 */
    EMAC_IRQHandler,           /* 79  EMAC */
    EMAC_WKUP_IRQHandler,      /* 80  EMAC Wakeup */
};

/*============================ RESET HANDLER =================================*/

__attribute__((noreturn))
void Reset_Handler(void)
{
    /* Copy .data from Flash to RAM */
    uint32_t *pSrc = &_sidata;
    uint32_t *pDest = &_sdata;
    while (pDest < &_edata) {
        *pDest++ = *pSrc++;
    }

    /* Zero .bss section */
    pDest = &_sbss;
    while (pDest < &_ebss) {
        *pDest++ = 0;
    }

    /* Enable FPU (Cortex-M4 with FPU) */
    *(volatile uint32_t *)0xE000ED88 |= ((3UL << 20) | (3UL << 22));
    __asm volatile ("dsb");
    __asm volatile ("isb");

    /* Call main */
    main();

    /* Should never reach here */
    while (1) {
        __asm volatile ("wfi");
    }
}

/*============================ DEFAULT HANDLER ===============================*/

void Default_Handler(void)
{
    while (1) {
        __asm volatile ("bkpt #0");
    }
}
