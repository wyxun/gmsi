/**
  **************************************************************************
  * @file     mc_hwio_v1.h
  * @brief    Definition and declaration of Hardware peripheral configuration
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#ifndef __MC_HWIO_V1_H
#define __MC_HWIO_V1_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

/**************** define Timer for PWM ******************/
/* 3-phase complementary pwm pin definition */
#define PWM_ADVANCE_TIMER                   TMR1
#define PWM_ADVANCE_TIMER_CRM_CLK           CRM_TMR1_PERIPH_CLOCK
#define ADVTMR_PWM_CYCLE_IRQ                TMR1_OVF_TMR10_IRQHandler
#define ADVTMR_PWM_CYCLE_IRQn               TMR1_OVF_TMR10_IRQn
#define ADVTMR_PWM_BRK_IRQ                  TMR1_BRK_TMR9_IRQHandler
#define ADVTMR_PWM_BRK_IRQn                 TMR1_BRK_TMR9_IRQn

/* timer for adc trigger source */
#define ADC_TIMER                           TMR1
#define ADC_TIMER_SELECT_CHANNEL            TMR_SELECT_CHANNEL_4
#define TMR_ADC_TRIG_SOURCE                 ADC12_PREEMPT_TRIG_TMR1CH4
/* define ADC TRIG OUTPUT PIN */
#define TMR_ADC_TRIG_GPIO_CRM_CLK           CRM_GPIOA_PERIPH_CLOCK
#define TMR_ADC_TRIG_PORT                   GPIOA
#define TMR_ADC_TRIG_GPIO_PIN               GPIO_PINS_11
#ifdef ONE_SHUNT
/* tmr adc dma definition */
#define TMR_ADC_DMA_CRM_CLK                 CRM_DMA1_PERIPH_CLOCK
#define TMR_ADC_DMA_CH                      DMA1_CHANNEL4
#define TMR_ADC_DMA                         DMA1
#define TMR_ADC_DMA_FLEX                    DMA_FLEXIBLE_TMR1_CH4
#define TMR_ADC_DMA_FLEX_CH                 FLEX_CHANNEL4
#define TMR_CH_ADC_DMA_REQUEST              TMR_C4_DMA_REQUEST
#define TMR_ADC_DMA_PERIPHERAL_ADDR         ADC_TIMER->c4dt
#endif

/**************** define GPIO for PWM *******************/
#define PWM_PHASE_A_HI_GPIO_CRM_CLK         CRM_GPIOA_PERIPH_CLOCK
#define PWM_PHASE_A_HI_PORT                 GPIOA
#define PWM_PHASE_A_HI_GPIO_PIN             GPIO_PINS_8
#define PWM_PHASE_B_HI_GPIO_CRM_CLK         CRM_GPIOA_PERIPH_CLOCK
#define PWM_PHASE_B_HI_PORT                 GPIOA
#define PWM_PHASE_B_HI_GPIO_PIN             GPIO_PINS_9
#define PWM_PHASE_C_HI_GPIO_CRM_CLK         CRM_GPIOA_PERIPH_CLOCK
#define PWM_PHASE_C_HI_PORT                 GPIOA
#define PWM_PHASE_C_HI_GPIO_PIN             GPIO_PINS_10
#define PWM_PHASE_A_LOW_GPIO_CRM_CLK        CRM_GPIOB_PERIPH_CLOCK
#define PWM_PHASE_A_LOW_PORT                GPIOB
#define PWM_PHASE_A_LOW_GPIO_PIN            GPIO_PINS_13
#define PWM_PHASE_B_LOW_GPIO_CRM_CLK        CRM_GPIOB_PERIPH_CLOCK
#define PWM_PHASE_B_LOW_PORT                GPIOB
#define PWM_PHASE_B_LOW_GPIO_PIN            GPIO_PINS_14
#define PWM_PHASE_C_LOW_GPIO_CRM_CLK        CRM_GPIOB_PERIPH_CLOCK
#define PWM_PHASE_C_LOW_PORT                GPIOB
#define PWM_PHASE_C_LOW_GPIO_PIN            GPIO_PINS_15
#define PWM_BRK_GPIO_CRM_CLK                CRM_GPIOB_PERIPH_CLOCK
#define PWM_BRK_PORT                        GPIOB
#define PWM_BRK_GPIO_PIN                    GPIO_PINS_12


/**************** define Timer for Hall ******************/
/* hall sensor pin definition */
#define HALL_CAPTURE_TIMER                  TMR3
#define HALL_CAPTURE_CRM_CLK                CRM_TMR3_PERIPH_CLOCK
#define HALL_CAPTURE_IOMUX                  TMR3_GMUX_0010
#define HALL_CAPTURE_IRQ                    TMR3_GLOBAL_IRQHandler
#define HALL_CAPTURE_IRQn                   TMR3_GLOBAL_IRQn
#define TMR_HALL_IN_FILTER                  0x6												/* 0x0 ~ 0xF */

/**************** define GPIO for Hall *******************/
#define HALL_A_GPIO_CRM_CLK                 CRM_GPIOB_PERIPH_CLOCK
#define HALL_A_PORT                         GPIOB
#define HALL_A_GPIO_PIN                     GPIO_PINS_4
#define HALL_B_GPIO_CRM_CLK                 CRM_GPIOB_PERIPH_CLOCK
#define HALL_B_PORT                         GPIOB
#define HALL_B_GPIO_PIN                     GPIO_PINS_5
#define HALL_C_GPIO_CRM_CLK                 CRM_GPIOB_PERIPH_CLOCK
#define HALL_C_PORT                         GPIOB
#define HALL_C_GPIO_PIN                     GPIO_PINS_0

/**************** define GPIO for Encoder *******************/
/* encoder sensor pin definition */
#define ENCODER_MODE_TIMER                  TMR5
#define ENCODER_MODE_CRM_CLK                CRM_TMR5_PERIPH_CLOCK
#define ENCODER_MODE_IOMUX                  TMR5_GMUX_001
#define ENCODER_A_GPIO_CRM_CLK              CRM_GPIOF_PERIPH_CLOCK
#define ENCODER_A_PORT                      GPIOF
#define ENCODER_A_GPIO_PIN                  GPIO_PINS_4
#define ENCODER_B_GPIO_CRM_CLK              CRM_GPIOF_PERIPH_CLOCK
#define ENCODER_B_PORT                      GPIOF
#define ENCODER_B_GPIO_PIN                  GPIO_PINS_5

#ifdef ABZ
/* encoder index */
#define ENCODER_Z_GPIO_CRM_CLK              CRM_GPIOC_PERIPH_CLOCK
#define ENCODER_Z_PORT                      GPIOC
#define ENCODER_Z_GPIO_PIN                  GPIO_PINS_4
#define ENCODER_Z_PORT_SOURCE               GPIO_PORT_SOURCE_GPIOC
#define ENCODER_Z_GPIO_PIN_SOURCE           GPIO_PINS_SOURCE4
#define ENCODER_Z_EXINT_LINE                EXINT_LINE_4
#define EXINT_ENCODER_IDX_IRQ               EXINT4_IRQHandler
#define EXINT_ENCODER_IDX_IRQn              EXINT4_IRQn
#endif

/**************** define Timer and GPIO for ENCODER CAPTURE *******************/
#define ENCODER_CAPTURE_TIMER               TMR3
#define ENCODER_CAPTURE_CRM_CLK             CRM_TMR3_PERIPH_CLOCK
#define ENCODER_CAPTURE_IOMUX               TMR3_GMUX_0010
#define ENCODER_CAPTURE_IRQ                 TMR3_GLOBAL_IRQHandler
#define ENCODER_CAPTURE_IRQn                TMR3_GLOBAL_IRQn
#define TMR_ENCODER_IN_FILTER               0x2												/* 0x0 ~ 0xF */
#define ENC_A_GPIO_CRM_CLK                  CRM_GPIOB_PERIPH_CLOCK
#define ENC_A_PORT                          GPIOB
#define ENC_A_GPIO_PIN                      GPIO_PINS_4
#define ENC_B_GPIO_CRM_CLK                  CRM_GPIOB_PERIPH_CLOCK
#define ENC_B_PORT                          GPIOB
#define ENC_B_GPIO_PIN                      GPIO_PINS_5

/**************** define Timer for speed control loop *******************/
#define SPEED_LOOP_TIMER                    TMR11
#define SPEED_LOOP_TIMER_CRM_CLK            CRM_TMR11_PERIPH_CLOCK
#define SPEED_LOOP_TIMER_IRQ                TMR1_TRG_HALL_TMR11_IRQHandler
#define SPEED_LOOP_TIMER_IRQn               TMR1_TRG_HALL_TMR11_IRQn

/* adc reading pin definition */
#define ADC_CONVERTER                       ADC1
#define ADC_CONVERTER_CRM_CLK               CRM_ADC1_PERIPH_CLOCK
#define ADC_CONVERTER_CRM_CLK_DIV           CRM_ADC_DIV_8
#define ADC_SHUNT_SAMP_READY_IRQ            ADC1_2_IRQHandler
#define ADC_SHUNT_SAMP_READY_IRQn           ADC1_2_IRQn
#define ADC_ORDINARY_CH_LEN                 ADC_IDX_MAX

/* dma1 ch1 for adc ordinary conversion */
#define ADC_ORDINARY_DMA_CRM_CLK            CRM_DMA1_PERIPH_CLOCK
#define ADC_ORDINARY_DMA_CHANNEL            DMA1_CHANNEL1
#define ADC_ORDINARY_DMA                    DMA1
#define ADC_ORDINARY_DMA_FLEX               DMA_FLEXIBLE_ADC1
#define ADC_ORDINARY_DMA_FLEX_CH            FLEX_CHANNEL1
#define ADC_ORDINARY_DMA_FT_STS_FLAG        DMA1_FDT1_FLAG

#define CURR_PHASE_A_ADC_CH                 ADC_CHANNEL_0
#define CURR_PHASE_A_ADC_GPIO_CRM_CLK       CRM_GPIOA_PERIPH_CLOCK
#define CURR_PHASE_A_ADC_PORT               GPIOA
#define CURR_PHASE_A_ADC_GPIO_PIN           GPIO_PINS_0

#define CURR_PHASE_B_ADC_CH                 ADC_CHANNEL_1
#define CURR_PHASE_B_ADC_GPIO_CRM_CLK       CRM_GPIOA_PERIPH_CLOCK
#define CURR_PHASE_B_ADC_PORT               GPIOA
#define CURR_PHASE_B_ADC_GPIO_PIN           GPIO_PINS_1

#define CURR_PHASE_C_ADC_CH                 ADC_CHANNEL_2
#define CURR_PHASE_C_ADC_GPIO_CRM_CLK       CRM_GPIOA_PERIPH_CLOCK
#define CURR_PHASE_C_ADC_PORT               GPIOA
#define CURR_PHASE_C_ADC_GPIO_PIN           GPIO_PINS_2

#define CURR_BUS_ADC_CH                     ADC_CHANNEL_3
#define CURR_BUS_ADC_GPIO_CRM_CLK           CRM_GPIOA_PERIPH_CLOCK
#define CURR_BUS_ADC_PORT                   GPIOA
#define CURR_BUS_ADC_GPIO_PIN               GPIO_PINS_3

#define VOLT_BUS_ADC_CH                     ADC_CHANNEL_7
#define VOLT_BUS_ADC_GPIO_CRM_CLK           CRM_GPIOA_PERIPH_CLOCK
#define VOLT_BUS_ADC_PORT                   GPIOA
#define VOLT_BUS_ADC_GPIO_PIN               GPIO_PINS_7

#define MOS_TEMP_ADC_CH                     ADC_CHANNEL_9
#define MOS_TEMP_ADC_GPIO_CRM_CLK           CRM_GPIOB_PERIPH_CLOCK
#define	MOS_TEMP_ADC_PORT                   GPIOB
#define MOS_TEMP_ADC_GPIO_PIN               GPIO_PINS_1

#define POTENTIO_ADC_CH                     ADC_CHANNEL_10
#define POTENTIO_ADC_GPIO_CRM_CLK           CRM_GPIOC_PERIPH_CLOCK
#define POTENTIO_ADC_PORT                   GPIOC
#define POTENTIO_ADC_GPIO_PIN               GPIO_PINS_0

#define IBUS_AVG_ADC_CH                     ADC_CHANNEL_3
#define IBUS_AVG_ADC_GPIO_CRM_CLK           CRM_GPIOA_PERIPH_CLOCK
#define IBUS_AVG_ADC_PORT                   GPIOA
#define IBUS_AVG_ADC_GPIO_PIN               GPIO_PINS_3

#define BEMF_A_ADC_CH                       ADC_CHANNEL_4
#define BEMF_A_ADC_GPIO_CRM_CLK             CRM_GPIOA_PERIPH_CLOCK
#define BEMF_A_ADC_PORT                     GPIOA
#define BEMF_A_ADC_GPIO_PIN                 GPIO_PINS_4

#define BEMF_B_ADC_CH                       ADC_CHANNEL_5
#define BEMF_B_ADC_GPIO_CRM_CLK             CRM_GPIOA_PERIPH_CLOCK
#define BEMF_B_ADC_PORT                     GPIOA
#define BEMF_B_ADC_GPIO_PIN                 GPIO_PINS_5

#define BEMF_C_ADC_CH                       ADC_CHANNEL_6
#define BEMF_C_ADC_GPIO_CRM_CLK             CRM_GPIOA_PERIPH_CLOCK
#define BEMF_C_ADC_PORT                     GPIOA
#define BEMF_C_ADC_GPIO_PIN                 GPIO_PINS_6

/* i2c interface */
#define	I2C_INTERFACE                       I2C2
#define I2C_SCL_PORT                        GPIOB
#define I2C_SCL_GPIO_PIN                    GPIO_PINS_10
#define I2C_SDA_PORT                        GPIOB
#define I2C_SDA_GPIO_PIN                    GPIO_PINS_11

/**************** define uart1 Tx and Rx ******************/
#define COMM_UART                           USART1
#define COMM_UART_CRM_CLK                   CRM_USART1_PERIPH_CLOCK
#define COMM_UART_TX_GPIO_CRM_CLK           CRM_GPIOB_PERIPH_CLOCK
#define COMM_UART_TX_PORT                   GPIOB
#define COMM_UART_TX_PIN                    GPIO_PINS_6
#define COMM_UART_RX_GPIO_CRM_CLK           CRM_GPIOB_PERIPH_CLOCK
#define COMM_UART_RX_PORT                   GPIOB
#define COMM_UART_RX_PIN                    GPIO_PINS_7
#define COMM_UART_IOMUX                     USART1_GMUX_0001
#define COMM_UART_IRQn                      USART1_IRQn
#define COMM_UART_IRQHandler                USART1_IRQHandler

/**************** define DMA for uart Tx and Rx ******************/
#define DMA_UART                            DMA1
#define DMA_UART_CRM_CLK                    CRM_DMA1_PERIPH_CLOCK
#define DMA_UART_TX_CHANNEL                 DMA1_CHANNEL2
#define DMA_UART_TX_CHANNEL_IRQn            DMA1_Channel2_IRQn
#define DMA_UART_TX_IRQHandler              DMA1_Channel2_IRQHandler
#define DMA_UART_TX_FDT_FLAG                DMA1_FDT2_FLAG
#define DMA_UART_TX_FLEX_CHANNEL            FLEX_CHANNEL2
#define DMA_UART_TX_FLEX                    DMA_FLEXIBLE_UART1_TX
#define DMA_UART_RX_CHANNEL                 DMA1_CHANNEL3
#define DMA_UART_RX_CHANNEL_IRQn            DMA1_Channel3_IRQn
#define DMA_UART_RX_IRQHandler              DMA1_Channel3_IRQHandler
#define DMA_UART_RX_FDT_FLAG                DMA1_FDT3_FLAG
#define DMA_UART_RX_FLEX_CHANNEL            FLEX_CHANNEL3
#define DMA_UART_RX_FLEX                    DMA_FLEXIBLE_UART1_RX

/******************* define led *******************/
/* error led state */
#define	ERROR_LED_GPIO_CRM_CLK              CRM_GPIOB_PERIPH_CLOCK
#define	ERROR_LED_PORT                      GPIOB
#define	ERROR_LED_GPIO_PIN                  GPIO_PINS_9

/* adc trig led state */
#define	ADC_TRIG_LED_GPIO_CRM_CLK           CRM_GPIOA_PERIPH_CLOCK
#define	ADC_TRIG_LED_PORT                   GPIOA
#define	ADC_TRIG_LED_GPIO_PIN               GPIO_PINS_11

/* operating status LEDs */
#define	STATUS1_LED_GPIO_CRM_CLK            CRM_GPIOC_PERIPH_CLOCK
#define	STATUS1_LED_PORT                    GPIOC
#define	STATUS1_LED_GPIO_PIN                GPIO_PINS_13
#define	STATUS2_LED_GPIO_CRM_CLK            CRM_GPIOC_PERIPH_CLOCK
#define	STATUS2_LED_PORT                    GPIOC
#define	STATUS2_LED_GPIO_PIN                GPIO_PINS_14
#define	STATUS3_LED_GPIO_CRM_CLK            CRM_GPIOC_PERIPH_CLOCK
#define	STATUS3_LED_PORT                    GPIOC
#define	STATUS3_LED_GPIO_PIN                GPIO_PINS_15

/******************* define button *******************/
typedef enum
{
  USER_BUTTON                               = 0,
  NO_BUTTON                                 = 1
} button_type;

#define USER_BUTTON_PIN                     GPIO_PINS_12
#define USER_BUTTON_PORT                    GPIOA
#define USER_BUTTON_CRM_CLK                 CRM_GPIOA_PERIPH_CLOCK

#define BUTTON_PORT_SOURCE                  GPIO_PORT_SOURCE_GPIOA
#define BUTTON_PIN_SOURCE                   GPIO_PINS_SOURCE12
#define BUTTON_EXINT_LINE                   EXINT_LINE_12
#define BUTTON_EXINT_IRQ                    EXINT15_10_IRQn
#define BUTTON_EXINT_IRQHandler             EXINT15_10_IRQHandler

#define HALL_LEARN_BUTTON_PIN               GPIO_PINS_10
#define HALL_LEARN_BUTTON_PORT              GPIOB
#define HALL_LEARN_BUTTON_CRM_CLK           CRM_GPIOB_PERIPH_CLOCK

#define HALL_LEARN_BUTTON_PORT_SOURCE       GPIO_PORT_SOURCE_GPIOB
#define HALL_LEARN_BUTTON_PIN_SOURCE        GPIO_PINS_SOURCE10
#define HALL_LEARN_BUTTON_EXINT_LINE        EXINT_LINE_10

#define MODE1_BUTTON_PIN                    GPIO_PINS_6
#define MODE1_BUTTON_PORT                   GPIOF
#define MODE1_BUTTON_CRM_CLK                CRM_GPIOF_PERIPH_CLOCK

#define MODE2_BUTTON_PIN                    GPIO_PINS_7
#define MODE2_BUTTON_PORT                   GPIOF
#define MODE2_BUTTON_CRM_CLK                CRM_GPIOF_PERIPH_CLOCK

/* pwm brake function */
#define BRAKE_PWM_TIMER                     TMR10
#define BRAKE_PWM_TIMER_CRM_CLK             CRM_TMR10_PERIPH_CLOCK
#define BRAKE_PWM_TIMER_CH                  TMR_SELECT_CHANNEL_1
#define BRAKE_PWM_PIN                       GPIO_PINS_8
#define BRAKE_PWM_PORT                      GPIOB
#define BRAKE_PWM_GPIO_CRM_CLK              CRM_GPIOB_PERIPH_CLOCK

typedef enum
{
  ADC_BEMF_A_IDX                            = 0x00,
  ADC_BEMF_B_IDX                            = 0x01,
  ADC_BEMF_C_IDX                            = 0x02,
  ADC_BUS_VOLT_IDX                          = 0x03,
  ADC_MOS_TEMP_IDX                          = 0x04,
  ADC_POTENTIO_IDX                          = 0x05,
  ADC_IBUS_AVE_IDX                          = 0x06,
  ADC_IDX_MAX                               = 0x07
} adc_in_idx;

extern uint16_t adc_in_tab[ADC_IDX_MAX];

void nvic_config(void);
void tmr_pwm_init(void);
void adc_ordinary_config(void);
void adc_preempt_config(void);
void encoder_timer_init(void);
void encoder_capture_timer_init(void);
void hall_timer_init(void);
void speed_timer_init(void);
void brake_pwm_init(void);
void uart_init(usart_config_type *usart_config);

/******************** functions ********************/
/* led operation function */
void led_init(void);
void led_on(gpio_type *led_gpio_port, uint16_t led_gpio_pin);
void led_off(gpio_type *led_gpio_port, uint16_t led_gpio_pin);
void led_toggle(gpio_type *led_gpio_port, uint16_t led_gpio_pin);
void led_config(void);
void led_blink(void);

/* mode switch configuration */
void mode_switch_init(void);

/* button operation function */
void button_exint_init(void);

/* initial angle detection configuration */
void foc_angle_init_config(void);

/* motor parameter identify configuration */
void motor_parameter_ID_config(void);

#ifdef __cplusplus
}
#endif

#endif
