/**
 * @file stm32g431xx.h
 * @brief STM32G431 Register Definitions (Minimal for Bootloader)
 * 
 * This is a minimal CMSIS-style header for bootloader use.
 * Contains only the registers needed for UART, Flash, and RCC.
 */
#ifndef __STM32G431XX_H__
#define __STM32G431XX_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MEMORY MAP ====================================*/

#define FLASH_BASE          0x08000000UL
#define SRAM1_BASE          0x20000000UL
#define PERIPH_BASE         0x40000000UL

#define APB1PERIPH_BASE     PERIPH_BASE
#define APB2PERIPH_BASE     (PERIPH_BASE + 0x00010000UL)
#define AHB1PERIPH_BASE     (PERIPH_BASE + 0x00020000UL)
#define AHB2PERIPH_BASE     (PERIPH_BASE + 0x08000000UL)

/* Peripheral base addresses */
#define USART1_BASE         (APB2PERIPH_BASE + 0x3800UL)
#define USART2_BASE         (APB1PERIPH_BASE + 0x4400UL)
#define USART3_BASE         (APB1PERIPH_BASE + 0x4800UL)
#define LPUART1_BASE        (APB1PERIPH_BASE + 0x8000UL)

#define RCC_BASE            (AHB1PERIPH_BASE + 0x1000UL)
#define FLASH_R_BASE        (AHB1PERIPH_BASE + 0x2000UL)
#define PWR_BASE            (APB1PERIPH_BASE + 0x7000UL)

#define GPIOA_BASE          (AHB2PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE          (AHB2PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE          (AHB2PERIPH_BASE + 0x0800UL)

#define SCB_BASE            0xE000ED00UL
#define NVIC_BASE           0xE000E100UL
#define SYSTICK_BASE        0xE000E010UL

/*============================ RCC REGISTERS =================================*/

typedef struct {
    volatile uint32_t CR;           /* 0x00 */
    volatile uint32_t ICSCR;        /* 0x04 */
    volatile uint32_t CFGR;         /* 0x08 */
    volatile uint32_t PLLCFGR;      /* 0x0C */
    volatile uint32_t RESERVED0[2]; /* 0x10-0x14 */
    volatile uint32_t CIER;         /* 0x18 */
    volatile uint32_t CIFR;         /* 0x1C */
    volatile uint32_t CICR;         /* 0x20 */
    volatile uint32_t RESERVED1;    /* 0x24 */
    volatile uint32_t AHB1RSTR;     /* 0x28 */
    volatile uint32_t AHB2RSTR;     /* 0x2C */
    volatile uint32_t AHB3RSTR;     /* 0x30 */
    volatile uint32_t RESERVED2;    /* 0x34 */
    volatile uint32_t APB1RSTR1;    /* 0x38 */
    volatile uint32_t APB1RSTR2;    /* 0x3C */
    volatile uint32_t APB2RSTR;     /* 0x40 */
    volatile uint32_t RESERVED3;    /* 0x44 */
    volatile uint32_t AHB1ENR;      /* 0x48 */
    volatile uint32_t AHB2ENR;      /* 0x4C */
    volatile uint32_t AHB3ENR;      /* 0x50 */
    volatile uint32_t RESERVED4;    /* 0x54 */
    volatile uint32_t APB1ENR1;     /* 0x58 */
    volatile uint32_t APB1ENR2;     /* 0x5C */
    volatile uint32_t APB2ENR;      /* 0x60 */
} RCC_TypeDef;

/* RCC peripheral */
#define RCC                 ((RCC_TypeDef *)RCC_BASE)

/* RCC bit definitions */
#define RCC_CR_HSION        (1UL << 8)
#define RCC_CR_HSIRDY       (1UL << 10)
#define RCC_CR_PLLON        (1UL << 24)
#define RCC_CR_PLLRDY       (1UL << 25)

#define RCC_CFGR_SW_HSI     (0UL << 0)
#define RCC_CFGR_SW_PLL     (3UL << 0)
#define RCC_CFGR_SWS_HSI    (0UL << 2)
#define RCC_CFGR_SWS_PLL    (3UL << 2)

#define RCC_APB1ENR1_USART2EN   (1UL << 17)
#define RCC_APB1ENR1_USART3EN   (1UL << 18)
#define RCC_APB1ENR1_PWREN      (1UL << 28)

#define RCC_APB2ENR_USART1EN    (1UL << 14)

#define RCC_AHB2ENR_GPIOAEN     (1UL << 0)
#define RCC_AHB2ENR_GPIOBEN     (1UL << 1)
#define RCC_AHB2ENR_GPIOCEN     (1UL << 2)

#define RCC_AHB1ENR_FLASHEN     (1UL << 8)

/*============================ FLASH REGISTERS ===============================*/

typedef struct {
    volatile uint32_t ACR;          /* 0x00 Access Control */
    volatile uint32_t PDKEYR;       /* 0x04 */
    volatile uint32_t KEYR;         /* 0x08 Key */
    volatile uint32_t OPTKEYR;      /* 0x0C Option Key */
    volatile uint32_t SR;           /* 0x10 Status */
    volatile uint32_t CR;           /* 0x14 Control */
    volatile uint32_t ECCR;         /* 0x18 */
    volatile uint32_t RESERVED0;    /* 0x1C */
    volatile uint32_t OPTR;         /* 0x20 */
    volatile uint32_t PCROP1SR;     /* 0x24 */
    volatile uint32_t PCROP1ER;     /* 0x28 */
    volatile uint32_t WRP1AR;       /* 0x2C */
    volatile uint32_t WRP1BR;       /* 0x30 */
} FLASH_TypeDef;

/* FLASH peripheral */
#define FLASH_REG           ((FLASH_TypeDef *)FLASH_R_BASE)

/* Flash key values */
#define FLASH_KEY1          0x45670123UL
#define FLASH_KEY2          0xCDEF89ABUL

/* Flash SR bits */
#define FLASH_SR_EOP        (1UL << 0)  /* End of operation */
#define FLASH_SR_OPERR      (1UL << 1)  /* Operation error */
#define FLASH_SR_PROGERR    (1UL << 3)  /* Programming error */
#define FLASH_SR_WRPERR     (1UL << 4)  /* Write protection error */
#define FLASH_SR_PGAERR     (1UL << 5)  /* Programming alignment error */
#define FLASH_SR_SIZERR     (1UL << 6)  /* Size error */
#define FLASH_SR_PGSERR     (1UL << 7)  /* Programming sequence error */
#define FLASH_SR_MISERR     (1UL << 8)  /* Fast programming miss */
#define FLASH_SR_FASTERR    (1UL << 9)  /* Fast programming error */
#define FLASH_SR_BSY        (1UL << 16) /* Busy */

/* Flash CR bits */
#define FLASH_CR_PG         (1UL << 0)  /* Programming */
#define FLASH_CR_PER        (1UL << 1)  /* Page erase */
#define FLASH_CR_MER1       (1UL << 2)  /* Mass erase bank 1 */
#define FLASH_CR_PNB_Pos    3           /* Page number position */
#define FLASH_CR_PNB_Msk    (0x7FUL << FLASH_CR_PNB_Pos)
#define FLASH_CR_STRT       (1UL << 16) /* Start */
#define FLASH_CR_LOCK       (1UL << 31) /* Lock */

/* Flash page size: 2KB for STM32G431 */
#define FLASH_PAGE_SIZE     2048UL
#define FLASH_PAGE_NB       64          /* 128KB / 2KB */

/*============================ USART REGISTERS ===============================*/

typedef struct {
    volatile uint32_t CR1;          /* 0x00 Control 1 */
    volatile uint32_t CR2;          /* 0x04 Control 2 */
    volatile uint32_t CR3;          /* 0x08 Control 3 */
    volatile uint32_t BRR;          /* 0x0C Baud Rate */
    volatile uint32_t GTPR;         /* 0x10 Guard Time/Prescaler */
    volatile uint32_t RTOR;         /* 0x14 Receiver Timeout */
    volatile uint32_t RQR;          /* 0x18 Request */
    volatile uint32_t ISR;          /* 0x1C Interrupt/Status */
    volatile uint32_t ICR;          /* 0x20 Interrupt Clear */
    volatile uint32_t RDR;          /* 0x24 Receive Data */
    volatile uint32_t TDR;          /* 0x28 Transmit Data */
    volatile uint32_t PRESC;        /* 0x2C Prescaler */
} USART_TypeDef;

/* USART peripherals */
#define USART1              ((USART_TypeDef *)USART1_BASE)
#define USART2              ((USART_TypeDef *)USART2_BASE)
#define USART3              ((USART_TypeDef *)USART3_BASE)

/* USART CR1 bits */
#define USART_CR1_UE        (1UL << 0)  /* USART enable */
#define USART_CR1_RE        (1UL << 2)  /* Receiver enable */
#define USART_CR1_TE        (1UL << 3)  /* Transmitter enable */
#define USART_CR1_IDLEIE    (1UL << 4)  /* IDLE interrupt */
#define USART_CR1_RXNEIE    (1UL << 5)  /* RXNE interrupt */
#define USART_CR1_TCIE      (1UL << 6)  /* TC interrupt */
#define USART_CR1_TXEIE     (1UL << 7)  /* TXE interrupt */
#define USART_CR1_OVER8     (1UL << 15) /* Oversampling 8 */
#define USART_CR1_M0        (1UL << 12) /* Word length bit 0 */
#define USART_CR1_M1        (1UL << 28) /* Word length bit 1 */
#define USART_CR1_FIFOEN    (1UL << 29) /* FIFO enable */

/* USART CR2 bits */
#define USART_CR2_STOP_Pos  12
#define USART_CR2_STOP_Msk  (3UL << USART_CR2_STOP_Pos)
#define USART_CR2_STOP_1    (0UL << USART_CR2_STOP_Pos)  /* 1 stop bit */
#define USART_CR2_STOP_2    (2UL << USART_CR2_STOP_Pos)  /* 2 stop bits */

/* USART CR3 bits */
#define USART_CR3_RTSE      (1UL << 8)  /* RTS enable */
#define USART_CR3_CTSE      (1UL << 9)  /* CTS enable */
#define USART_CR3_DMAT      (1UL << 7)  /* DMA transmit */
#define USART_CR3_DMAR      (1UL << 6)  /* DMA receive */

/* USART ISR bits */
#define USART_ISR_PE        (1UL << 0)  /* Parity error */
#define USART_ISR_FE        (1UL << 1)  /* Framing error */
#define USART_ISR_NE        (1UL << 2)  /* Noise error */
#define USART_ISR_ORE       (1UL << 3)  /* Overrun error */
#define USART_ISR_IDLE      (1UL << 4)  /* IDLE detected */
#define USART_ISR_RXNE      (1UL << 5)  /* RX not empty */
#define USART_ISR_TC        (1UL << 6)  /* TX complete */
#define USART_ISR_TXE       (1UL << 7)  /* TX empty */
#define USART_ISR_TEACK     (1UL << 21) /* TX enable ACK */
#define USART_ISR_REACK     (1UL << 22) /* RX enable ACK */

/* USART ICR bits */
#define USART_ICR_PECF      (1UL << 0)
#define USART_ICR_FECF      (1UL << 1)
#define USART_ICR_NCF       (1UL << 2)
#define USART_ICR_ORECF     (1UL << 3)
#define USART_ICR_IDLECF    (1UL << 4)
#define USART_ICR_TCCF      (1UL << 6)

/*============================ GPIO REGISTERS ================================*/

typedef struct {
    volatile uint32_t MODER;        /* 0x00 Mode */
    volatile uint32_t OTYPER;       /* 0x04 Output type */
    volatile uint32_t OSPEEDR;      /* 0x08 Speed */
    volatile uint32_t PUPDR;        /* 0x0C Pull-up/down */
    volatile uint32_t IDR;          /* 0x10 Input data */
    volatile uint32_t ODR;          /* 0x14 Output data */
    volatile uint32_t BSRR;         /* 0x18 Bit set/reset */
    volatile uint32_t LCKR;         /* 0x1C Lock */
    volatile uint32_t AFR[2];       /* 0x20-0x24 Alternate function */
    volatile uint32_t BRR;          /* 0x28 Bit reset */
} GPIO_TypeDef;

/* GPIO peripherals */
#define GPIOA               ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB               ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC               ((GPIO_TypeDef *)GPIOC_BASE)

/* GPIO Mode values */
#define GPIO_MODE_INPUT     0UL
#define GPIO_MODE_OUTPUT    1UL
#define GPIO_MODE_AF        2UL
#define GPIO_MODE_ANALOG    3UL

/* GPIO Speed values */
#define GPIO_SPEED_LOW      0UL
#define GPIO_SPEED_MEDIUM   1UL
#define GPIO_SPEED_HIGH     2UL
#define GPIO_SPEED_VHIGH    3UL

/* GPIO Pull values */
#define GPIO_PULL_NONE      0UL
#define GPIO_PULL_UP        1UL
#define GPIO_PULL_DOWN      2UL

/* Alternate function numbers */
#define GPIO_AF7_USART1     7UL
#define GPIO_AF7_USART2     7UL
#define GPIO_AF7_USART3     7UL

/*============================ SCB REGISTERS =================================*/

typedef struct {
    volatile uint32_t CPUID;        /* 0x00 */
    volatile uint32_t ICSR;         /* 0x04 */
    volatile uint32_t VTOR;         /* 0x08 Vector Table Offset */
    volatile uint32_t AIRCR;        /* 0x0C Application Interrupt/Reset */
    volatile uint32_t SCR;          /* 0x10 */
    volatile uint32_t CCR;          /* 0x14 */
    volatile uint32_t SHPR[3];      /* 0x18-0x20 */
    volatile uint32_t SHCSR;        /* 0x24 */
} SCB_TypeDef;

/* SCB peripheral */
#define SCB                 ((SCB_TypeDef *)SCB_BASE)

/* SCB AIRCR bits */
#define SCB_AIRCR_VECTKEY       (0x05FAUL << 16)
#define SCB_AIRCR_SYSRESETREQ   (1UL << 2)

/*============================ SYSTICK REGISTERS =============================*/

typedef struct {
    volatile uint32_t CTRL;         /* 0x00 Control */
    volatile uint32_t LOAD;         /* 0x04 Reload */
    volatile uint32_t VAL;          /* 0x08 Current value */
    volatile uint32_t CALIB;        /* 0x0C Calibration */
} SysTick_TypeDef;

/* SysTick peripheral */
#define SysTick             ((SysTick_TypeDef *)SYSTICK_BASE)

/* SysTick CTRL bits */
#define SysTick_CTRL_ENABLE     (1UL << 0)
#define SysTick_CTRL_TICKINT    (1UL << 1)
#define SysTick_CTRL_CLKSOURCE  (1UL << 2)
#define SysTick_CTRL_COUNTFLAG  (1UL << 16)

/* SysTick max value (24-bit) */
#define SysTick_LOAD_RELOAD_Msk 0x00FFFFFFUL

/*============================ SYSTEM ========================================*/

/* System core clock (default HSI = 16MHz) */
#ifndef SystemCoreClock
#define SystemCoreClock     16000000UL
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STM32G431XX_H__ */
