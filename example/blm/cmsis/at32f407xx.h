/**
 * @file at32f407xx.h
 * @brief AT32F407 Peripheral Register Definitions (Minimal for Bootloader)
 *
 * Based on AT32F403A/407 Reference Manual.
 * AT32F407 uses APB2 bus for GPIO (similar to STM32F1).
 * USART uses SR/DR register model (STM32F1-style).
 * Flash uses half-word (16-bit) programming.
 *
 * Core CM4 registers (SCB, SysTick, NVIC) are in core_cm4.h.
 */
#ifndef __AT32F407XX_H__
#define __AT32F407XX_H__

#include "core_cm4.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MEMORY MAP ====================================*/

#define FLASH_BASE          0x08000000UL
#define SRAM_BASE           0x20000000UL
#define PERIPH_BASE         0x40000000UL

#define APB1PERIPH_BASE     PERIPH_BASE
#define APB2PERIPH_BASE     (PERIPH_BASE + 0x00010000UL)
#define AHBPERIPH_BASE      (PERIPH_BASE + 0x00020000UL)

/* Peripheral base addresses */
#define USART1_BASE         (APB2PERIPH_BASE + 0x3800UL)
#define USART2_BASE         (APB1PERIPH_BASE + 0x4400UL)
#define USART3_BASE         (APB1PERIPH_BASE + 0x4800UL)
#define UART4_BASE          (APB1PERIPH_BASE + 0x4C00UL)
#define UART5_BASE          (APB1PERIPH_BASE + 0x5000UL)

#define CRM_BASE            (AHBPERIPH_BASE + 0x1000UL)   /* CRM (Clock/Reset Module) */
#define FLASH_R_BASE        (AHBPERIPH_BASE + 0x2000UL)

#define GPIOA_BASE          (APB2PERIPH_BASE + 0x0800UL)
#define GPIOB_BASE          (APB2PERIPH_BASE + 0x0C00UL)
#define GPIOC_BASE          (APB2PERIPH_BASE + 0x1000UL)
#define GPIOD_BASE          (APB2PERIPH_BASE + 0x1400UL)
#define GPIOE_BASE          (APB2PERIPH_BASE + 0x1800UL)

/*============================ CRM (RCC) REGISTERS ===========================*/

typedef struct {
    volatile uint32_t CTRL;         /* 0x00 Clock Control */
    volatile uint32_t CFG;          /* 0x04 Clock Configuration */
    volatile uint32_t CLKINT;       /* 0x08 Clock Interrupt */
    volatile uint32_t APB2RST;      /* 0x0C APB2 Reset */
    volatile uint32_t APB1RST;      /* 0x10 APB1 Reset */
    volatile uint32_t AHBEN;        /* 0x14 AHB Enable */
    volatile uint32_t APB2EN;       /* 0x18 APB2 Enable */
    volatile uint32_t APB1EN;       /* 0x1C APB1 Enable */
    volatile uint32_t BPDC;         /* 0x20 Battery Power Domain Control */
    volatile uint32_t CTRLSTS;      /* 0x24 Control/Status */
    volatile uint32_t RESERVED0[2]; /* 0x28-0x2C */
    volatile uint32_t MISC1;        /* 0x30 Misc1 */
    volatile uint32_t RESERVED1[4]; /* 0x34-0x40 */
    volatile uint32_t MISC2;        /* 0x44 Misc2 */
    volatile uint32_t MISC3;        /* 0x48 Misc3 */
    volatile uint32_t RESERVED2;    /* 0x4C */
    volatile uint32_t INTMAP;       /* 0x50 Interrupt remap */
} CRM_TypeDef;

/* Alias for compatibility */
typedef CRM_TypeDef RCC_TypeDef;

#define CRM                 ((CRM_TypeDef *)CRM_BASE)
#define RCC                 CRM

/* CRM CTRL bits (HSI/HSE/PLL) */
#define CRM_CTRL_HICKEN     (1UL << 0)      /* High speed internal clock enable */
#define CRM_CTRL_HICKSTBL   (1UL << 1)      /* HICK stable */
#define CRM_CTRL_HEXTEN     (1UL << 16)     /* High speed external crystal enable */
#define CRM_CTRL_HEXTSTBL   (1UL << 17)     /* HEXT stable */
#define CRM_CTRL_PLLEN      (1UL << 24)     /* PLL enable */
#define CRM_CTRL_PLLSTBL    (1UL << 25)     /* PLL stable */

/* CRM APB2EN bits */
#define CRM_APB2EN_IOMUXEN  (1UL << 0)      /* IO MUX clock enable */
#define CRM_APB2EN_GPIOAEN  (1UL << 2)      /* GPIOA clock enable */
#define CRM_APB2EN_GPIOBEN  (1UL << 3)      /* GPIOB clock enable */
#define CRM_APB2EN_GPIOCEN  (1UL << 4)      /* GPIOC clock enable */
#define CRM_APB2EN_GPIODEN  (1UL << 5)      /* GPIOD clock enable */
#define CRM_APB2EN_GPIOEEN  (1UL << 6)      /* GPIOE clock enable */
#define CRM_APB2EN_USART1EN (1UL << 14)     /* USART1 clock enable */

/* CRM APB1EN bits */
#define CRM_APB1EN_USART2EN (1UL << 17)     /* USART2 clock enable */
#define CRM_APB1EN_USART3EN (1UL << 18)     /* USART3 clock enable */
#define CRM_APB1EN_UART4EN  (1UL << 19)     /* UART4 clock enable */
#define CRM_APB1EN_UART5EN  (1UL << 20)     /* UART5 clock enable */
#define CRM_APB1EN_PWCEN    (1UL << 28)     /* Power controller clock enable */

/* CRM AHBEN bits */
#define CRM_AHBEN_DMA1EN    (1UL << 0)
#define CRM_AHBEN_DMA2EN    (1UL << 1)
#define CRM_AHBEN_FLASHEN   (1UL << 4)

/*============================ FLASH REGISTERS ===============================*/

typedef struct {
    volatile uint32_t PSR;          /* 0x00 Performance Select */
    volatile uint32_t UNLOCK;       /* 0x04 Unlock */
    volatile uint32_t USD_UNLOCK;   /* 0x08 USD Unlock */
    volatile uint32_t STS;          /* 0x0C Status */
    volatile uint32_t CTRL;         /* 0x10 Control */
    volatile uint32_t ADDR;         /* 0x14 Address */
    volatile uint32_t RESERVED0;    /* 0x18 */
    volatile uint32_t USD;          /* 0x1C User System Data */
    volatile uint32_t EPPS;         /* 0x20 Erase/Program Protection Status */
    volatile uint32_t RESERVED1[20];/* 0x24-0x70 */
    volatile uint32_t SLIB_STS0;    /* 0x74 sLib Status 0 */
    volatile uint32_t SLIB_STS1;    /* 0x78 sLib Status 1 */
    volatile uint32_t SLIB_PWD_CLR; /* 0x7C sLib Password Clear */
    volatile uint32_t SLIB_MISC_STS;/* 0x80 sLib Misc Status */
    volatile uint32_t CRC_ADDR;     /* 0x84 CRC address */
    volatile uint32_t CRC_CTRL;     /* 0x88 CRC Control */
    volatile uint32_t CRC_CHKR;     /* 0x8C CRC Check Result */
    volatile uint32_t RESERVED2[52];/* 0x90-0x15C */
    volatile uint32_t SLIB_SET_PWD; /* 0x160 sLib Set Password */
    volatile uint32_t SLIB_SET_RANGE;/* 0x164 sLib Set Range */
    volatile uint32_t EM_SLIB_SET;  /* 0x168 Extension Memory sLib Set */
    volatile uint32_t BTM_MODE_SET; /* 0x16C Boot Memory Mode Set */
    volatile uint32_t SLIB_UNLOCK;  /* 0x170 sLib Unlock */
} FLASH_TypeDef;

#define FLASH_REG           ((FLASH_TypeDef *)FLASH_R_BASE)

/* Flash key values */
#define FLASH_KEY1          0x45670123UL
#define FLASH_KEY2          0xCDEF89ABUL

/* Flash STS (Status) bits */
#define FLASH_STS_OBF       (1UL << 0)      /* Operation busy flag */
#define FLASH_STS_PRGMERR   (1UL << 2)      /* Program error */
#define FLASH_STS_EPPERR    (1UL << 4)      /* Erase/program protection error */
#define FLASH_STS_ODF       (1UL << 5)      /* Operation done flag */

/* Flash CTRL bits */
#define FLASH_CTRL_FPRGM    (1UL << 0)      /* Flash program */
#define FLASH_CTRL_SECERS   (1UL << 1)      /* Sector erase */
#define FLASH_CTRL_BANKERS  (1UL << 2)      /* Bank erase */
#define FLASH_CTRL_ERSTR    (1UL << 6)      /* Erase start */
#define FLASH_CTRL_OPLK     (1UL << 7)      /* Operation lock */
#define FLASH_CTRL_USDPRGM  (1UL << 9)      /* USD program */
#define FLASH_CTRL_USDERS   (1UL << 10)     /* USD erase */

/* Flash page size: 2KB for AT32F407 (>512KB variant) */
#define FLASH_PAGE_SIZE     2048UL

/*============================ USART REGISTERS ===============================*/
/* AT32F407 USART uses SR/DR model (similar to STM32F1) */

typedef struct {
    volatile uint32_t STS;          /* 0x00 Status Register */
    volatile uint32_t DT;           /* 0x04 Data Register */
    volatile uint32_t BAUDR;        /* 0x08 Baud Rate Register */
    volatile uint32_t CTRL1;        /* 0x0C Control Register 1 */
    volatile uint32_t CTRL2;        /* 0x10 Control Register 2 */
    volatile uint32_t CTRL3;        /* 0x14 Control Register 3 */
    volatile uint32_t GDT;          /* 0x18 Guard Time and Division */
} USART_TypeDef;

#define USART1              ((USART_TypeDef *)USART1_BASE)
#define USART2              ((USART_TypeDef *)USART2_BASE)
#define USART3              ((USART_TypeDef *)USART3_BASE)

/* USART STS bits */
#define USART_STS_PERR      (1UL << 0)      /* Parity error */
#define USART_STS_FERR      (1UL << 1)      /* Framing error */
#define USART_STS_NERR      (1UL << 2)      /* Noise error */
#define USART_STS_ROERR     (1UL << 3)      /* Receiver overflow error */
#define USART_STS_IDLEF     (1UL << 4)      /* IDLE flag */
#define USART_STS_RDBF      (1UL << 5)      /* Receive data buffer full */
#define USART_STS_TDC       (1UL << 6)      /* Transmit data complete */
#define USART_STS_TDBE      (1UL << 7)      /* Transmit data buffer empty */

/* USART CTRL1 bits */
#define USART_CTRL1_SBF     (1UL << 0)      /* Send break frame */
#define USART_CTRL1_RM      (1UL << 1)      /* Receiver mute */
#define USART_CTRL1_REN     (1UL << 2)      /* Receiver enable */
#define USART_CTRL1_TEN     (1UL << 3)      /* Transmitter enable */
#define USART_CTRL1_IDLEIEN (1UL << 4)      /* IDLE interrupt enable */
#define USART_CTRL1_RDBFIEN (1UL << 5)      /* RDBF interrupt enable */
#define USART_CTRL1_TDCIEN  (1UL << 6)      /* TDC interrupt enable */
#define USART_CTRL1_TDBEIEN (1UL << 7)      /* TDBE interrupt enable */
#define USART_CTRL1_DBN     (1UL << 12)     /* Data bit number (0=8bit) */
#define USART_CTRL1_UEN     (1UL << 13)     /* USART enable */

/* USART CTRL2 bits */
#define USART_CTRL2_STOPBN_Pos  12
#define USART_CTRL2_STOPBN_Msk  (3UL << USART_CTRL2_STOPBN_Pos)
#define USART_CTRL2_STOPBN_1    (0UL << USART_CTRL2_STOPBN_Pos)
#define USART_CTRL2_STOPBN_2    (2UL << USART_CTRL2_STOPBN_Pos)

/*============================ GPIO REGISTERS ================================*/
/* AT32F407 GPIO uses CRL/CRH model (similar to STM32F1) */

typedef struct {
    volatile uint32_t CFGLR;        /* 0x00 Configuration Low (pin 0-7) */
    volatile uint32_t CFGHR;        /* 0x04 Configuration High (pin 8-15) */
    volatile uint32_t IDT;          /* 0x08 Input Data */
    volatile uint32_t ODT;          /* 0x0C Output Data */
    volatile uint32_t SCR;          /* 0x10 Set/Clear Register */
    volatile uint32_t CLR;          /* 0x14 Clear Register */
    volatile uint32_t WPR;          /* 0x18 Write Protection */
} GPIO_TypeDef;

#define GPIOA               ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB               ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC               ((GPIO_TypeDef *)GPIOC_BASE)
#define GPIOD               ((GPIO_TypeDef *)GPIOD_BASE)
#define GPIOE               ((GPIO_TypeDef *)GPIOE_BASE)

/* GPIO CFGLR/CFGHR mode values (per pin: 4 bits = MODE[1:0] + CNF[1:0]) */
#define GPIO_MODE_INPUT     0x00UL      /* Input mode */
#define GPIO_MODE_OUT_10M   0x01UL      /* Output 10MHz */
#define GPIO_MODE_OUT_2M    0x02UL      /* Output 2MHz */
#define GPIO_MODE_OUT_50M   0x03UL      /* Output 50MHz */

#define GPIO_CNF_ANALOG     (0x00UL << 2)   /* Analog input */
#define GPIO_CNF_FLOATING   (0x01UL << 2)   /* Floating input */
#define GPIO_CNF_PULL       (0x02UL << 2)   /* Input with pull-up/down */
#define GPIO_CNF_AF_PP      (0x02UL << 2)   /* AF push-pull output */
#define GPIO_CNF_AF_OD      (0x03UL << 2)   /* AF open-drain output */
#define GPIO_CNF_GP_PP      (0x00UL << 2)   /* GP push-pull output */
#define GPIO_CNF_GP_OD      (0x01UL << 2)   /* GP open-drain output */

/*============================ IOMUX (AFIO) ==================================*/

#define IOMUX_BASE          (APB2PERIPH_BASE + 0x0000UL)

typedef struct {
    volatile uint32_t EVTOUT;       /* 0x00 Event output */
    volatile uint32_t REMAP;        /* 0x04 IO remap */
    volatile uint32_t EXINTC[4];    /* 0x08-0x14 EXINT configuration */
    volatile uint32_t RESERVED0;    /* 0x18 */
    volatile uint32_t REMAP2;       /* 0x1C IO remap 2 */
    volatile uint32_t REMAP3;       /* 0x20 IO remap 3 */
    volatile uint32_t REMAP4;       /* 0x24 IO remap 4 */
    volatile uint32_t REMAP5;       /* 0x28 IO remap 5 */
    volatile uint32_t REMAP6;       /* 0x2C IO remap 6 */
    volatile uint32_t REMAP7;       /* 0x30 IO remap 7 */
    volatile uint32_t REMAP8;       /* 0x34 IO remap 8 */
} IOMUX_TypeDef;

#define IOMUX               ((IOMUX_TypeDef *)IOMUX_BASE)

/*============================ SYSTEM ========================================*/

/* System core clock (defined in port file) */
extern uint32_t SystemCoreClock;

#ifdef __cplusplus
}
#endif

#endif /* __AT32F407XX_H__ */
