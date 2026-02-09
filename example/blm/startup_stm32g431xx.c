/**
 * @file startup_stm32g431xx.c
 * @brief STM32G431xx Startup Code (C-based)
 * 
 * Contains vector table and reset handler for Cortex-M4.
 * Compatible with LLVM Embedded Toolchain for Arm.
 */

#include <stdint.h>

/*============================ EXTERNAL SYMBOLS ==============================*/

/* From linker script */
extern uint32_t _sidata;    /* Start of .data in Flash */
extern uint32_t _sdata;     /* Start of .data in RAM */
extern uint32_t _edata;     /* End of .data in RAM */
extern uint32_t _sbss;      /* Start of .bss */
extern uint32_t _ebss;      /* End of .bss */
extern uint32_t _estack;    /* End of stack (top) */

/* Main function */
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
void SysTick_Handler(void)      __attribute__((weak, alias("Default_Handler")));

/* STM32G431 Peripheral Interrupts */
void WWDG_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void PVD_PVM_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void RTC_TAMP_LSECSS_IRQHandler(void)   __attribute__((weak, alias("Default_Handler")));
void RTC_WKUP_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void RCC_IRQHandler(void)               __attribute__((weak, alias("Default_Handler")));
void EXTI0_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void EXTI1_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void EXTI2_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void EXTI3_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void EXTI4_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel1_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel2_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel3_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel4_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel5_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel6_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void ADC1_2_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void USB_HP_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void USB_LP_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void FDCAN1_IT0_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void FDCAN1_IT1_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void EXTI9_5_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void TIM1_BRK_TIM15_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void TIM1_UP_TIM16_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void TIM1_TRG_COM_TIM17_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_CC_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void TIM2_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void TIM3_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void TIM4_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void I2C1_EV_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void I2C1_ER_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void I2C2_EV_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void I2C2_ER_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void SPI1_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void SPI2_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void USART1_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void USART3_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void EXTI15_10_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));
void RTC_Alarm_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));
void USBWakeUp_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));
void TIM8_BRK_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));
void TIM8_UP_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void TIM8_TRG_COM_IRQHandler(void)      __attribute__((weak, alias("Default_Handler")));
void TIM8_CC_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void LPTIM1_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void SPI3_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void UART4_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void TIM6_DAC_IRQHandler(void)          __attribute__((weak, alias("Default_Handler")));
void TIM7_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel1_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel2_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel3_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel4_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel5_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void UCPD1_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void COMP1_2_3_IRQHandler(void)         __attribute__((weak, alias("Default_Handler")));
void COMP4_IRQHandler(void)             __attribute__((weak, alias("Default_Handler")));
void CRS_IRQHandler(void)               __attribute__((weak, alias("Default_Handler")));
void SAI1_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));
void FPU_IRQHandler(void)               __attribute__((weak, alias("Default_Handler")));
void I2C3_EV_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void I2C3_ER_IRQHandler(void)           __attribute__((weak, alias("Default_Handler")));
void DMAMUX_OVR_IRQHandler(void)        __attribute__((weak, alias("Default_Handler")));
void DMA2_Channel6_IRQHandler(void)     __attribute__((weak, alias("Default_Handler")));
void CORDIC_IRQHandler(void)            __attribute__((weak, alias("Default_Handler")));
void FMAC_IRQHandler(void)              __attribute__((weak, alias("Default_Handler")));

/*============================ VECTOR TABLE ==================================*/

__attribute__((used, section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
    /* Initial Stack Pointer */
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
    
    /* STM32G431 Peripheral Interrupts */
    WWDG_IRQHandler,            /* 0   Window Watchdog */
    PVD_PVM_IRQHandler,         /* 1   PVD/PVM */
    RTC_TAMP_LSECSS_IRQHandler, /* 2   RTC Tamper/LSE CSS */
    RTC_WKUP_IRQHandler,        /* 3   RTC Wakeup */
    FLASH_IRQHandler,           /* 4   Flash */
    RCC_IRQHandler,             /* 5   RCC */
    EXTI0_IRQHandler,           /* 6   EXTI Line 0 */
    EXTI1_IRQHandler,           /* 7   EXTI Line 1 */
    EXTI2_IRQHandler,           /* 8   EXTI Line 2 */
    EXTI3_IRQHandler,           /* 9   EXTI Line 3 */
    EXTI4_IRQHandler,           /* 10  EXTI Line 4 */
    DMA1_Channel1_IRQHandler,   /* 11  DMA1 Channel 1 */
    DMA1_Channel2_IRQHandler,   /* 12  DMA1 Channel 2 */
    DMA1_Channel3_IRQHandler,   /* 13  DMA1 Channel 3 */
    DMA1_Channel4_IRQHandler,   /* 14  DMA1 Channel 4 */
    DMA1_Channel5_IRQHandler,   /* 15  DMA1 Channel 5 */
    DMA1_Channel6_IRQHandler,   /* 16  DMA1 Channel 6 */
    0,                          /* 17  Reserved */
    ADC1_2_IRQHandler,          /* 18  ADC1 & ADC2 */
    USB_HP_IRQHandler,          /* 19  USB High Priority */
    USB_LP_IRQHandler,          /* 20  USB Low Priority */
    FDCAN1_IT0_IRQHandler,      /* 21  FDCAN1 IT0 */
    FDCAN1_IT1_IRQHandler,      /* 22  FDCAN1 IT1 */
    EXTI9_5_IRQHandler,         /* 23  EXTI Lines 5-9 */
    TIM1_BRK_TIM15_IRQHandler,  /* 24  TIM1 Break/TIM15 */
    TIM1_UP_TIM16_IRQHandler,   /* 25  TIM1 Update/TIM16 */
    TIM1_TRG_COM_TIM17_IRQHandler, /* 26  TIM1 Trigger/TIM17 */
    TIM1_CC_IRQHandler,         /* 27  TIM1 Capture Compare */
    TIM2_IRQHandler,            /* 28  TIM2 */
    TIM3_IRQHandler,            /* 29  TIM3 */
    TIM4_IRQHandler,            /* 30  TIM4 */
    I2C1_EV_IRQHandler,         /* 31  I2C1 Event */
    I2C1_ER_IRQHandler,         /* 32  I2C1 Error */
    I2C2_EV_IRQHandler,         /* 33  I2C2 Event */
    I2C2_ER_IRQHandler,         /* 34  I2C2 Error */
    SPI1_IRQHandler,            /* 35  SPI1 */
    SPI2_IRQHandler,            /* 36  SPI2 */
    USART1_IRQHandler,          /* 37  USART1 */
    USART2_IRQHandler,          /* 38  USART2 */
    USART3_IRQHandler,          /* 39  USART3 */
    EXTI15_10_IRQHandler,       /* 40  EXTI Lines 10-15 */
    RTC_Alarm_IRQHandler,       /* 41  RTC Alarm */
    USBWakeUp_IRQHandler,       /* 42  USB Wakeup */
    TIM8_BRK_IRQHandler,        /* 43  TIM8 Break */
    TIM8_UP_IRQHandler,         /* 44  TIM8 Update */
    TIM8_TRG_COM_IRQHandler,    /* 45  TIM8 Trigger/Commutation */
    TIM8_CC_IRQHandler,         /* 46  TIM8 Capture Compare */
    0,                          /* 47  Reserved */
    0,                          /* 48  Reserved */
    LPTIM1_IRQHandler,          /* 49  LPTIM1 */
    0,                          /* 50  Reserved */
    SPI3_IRQHandler,            /* 51  SPI3 */
    UART4_IRQHandler,           /* 52  UART4 */
    0,                          /* 53  Reserved */
    TIM6_DAC_IRQHandler,        /* 54  TIM6/DAC */
    TIM7_IRQHandler,            /* 55  TIM7 */
    DMA2_Channel1_IRQHandler,   /* 56  DMA2 Channel 1 */
    DMA2_Channel2_IRQHandler,   /* 57  DMA2 Channel 2 */
    DMA2_Channel3_IRQHandler,   /* 58  DMA2 Channel 3 */
    DMA2_Channel4_IRQHandler,   /* 59  DMA2 Channel 4 */
    DMA2_Channel5_IRQHandler,   /* 60  DMA2 Channel 5 */
    0,                          /* 61  Reserved */
    0,                          /* 62  Reserved */
    UCPD1_IRQHandler,           /* 63  UCPD1 */
    COMP1_2_3_IRQHandler,       /* 64  COMP1/2/3 */
    COMP4_IRQHandler,           /* 65  COMP4 */
    0,                          /* 66  Reserved */
    0,                          /* 67  Reserved */
    0,                          /* 68  Reserved */
    0,                          /* 69  Reserved */
    0,                          /* 70  Reserved */
    0,                          /* 71  Reserved */
    0,                          /* 72  Reserved */
    0,                          /* 73  Reserved */
    0,                          /* 74  Reserved */
    CRS_IRQHandler,             /* 75  CRS */
    SAI1_IRQHandler,            /* 76  SAI1 */
    0,                          /* 77  Reserved */
    0,                          /* 78  Reserved */
    0,                          /* 79  Reserved */
    0,                          /* 80  Reserved */
    FPU_IRQHandler,             /* 81  FPU */
    I2C3_EV_IRQHandler,         /* 82  I2C3 Event */
    I2C3_ER_IRQHandler,         /* 83  I2C3 Error */
    0,                          /* 84  Reserved */
    0,                          /* 85  Reserved */
    0,                          /* 86  Reserved */
    0,                          /* 87  Reserved */
    0,                          /* 88  Reserved */
    0,                          /* 89  Reserved */
    0,                          /* 90  Reserved */
    DMAMUX_OVR_IRQHandler,      /* 91  DMAMUX Overrun */
    0,                          /* 92  Reserved */
    0,                          /* 93  Reserved */
    DMA2_Channel6_IRQHandler,   /* 94  DMA2 Channel 6 */
    0,                          /* 95  Reserved */
    0,                          /* 96  Reserved */
    CORDIC_IRQHandler,          /* 97  CORDIC */
    FMAC_IRQHandler,            /* 98  FMAC */
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
    /* SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2)); */
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
