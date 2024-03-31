/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED0_Pin GPIO_PIN_0
#define LED0_GPIO_Port GPIOA
#define MAX3485_Pin GPIO_PIN_1
#define MAX3485_GPIO_Port GPIOA
#define FAN_Pin GPIO_PIN_4
#define FAN_GPIO_Port GPIOA
#define KEY_Pin GPIO_PIN_5
#define KEY_GPIO_Port GPIOA
#define LED_G_Pin GPIO_PIN_6
#define LED_G_GPIO_Port GPIOA
#define LED_R_Pin GPIO_PIN_7
#define LED_R_GPIO_Port GPIOA
#define LED_B_Pin GPIO_PIN_4
#define LED_B_GPIO_Port GPIOC
#define MAX31855_SCK_Pin GPIO_PIN_13
#define MAX31855_SCK_GPIO_Port GPIOB
#define MAX31855_MISO_Pin GPIO_PIN_14
#define MAX31855_MISO_GPIO_Port GPIOB
#define MAX31855_CS_Pin GPIO_PIN_15
#define MAX31855_CS_GPIO_Port GPIOB
#define RIGHT_IO_Pin GPIO_PIN_6
#define RIGHT_IO_GPIO_Port GPIOC
#define LIFT_IO_Pin GPIO_PIN_7
#define LIFT_IO_GPIO_Port GPIOC
#define CAN_S_Pin GPIO_PIN_8
#define CAN_S_GPIO_Port GPIOA
#define ULN2003_IND_Pin GPIO_PIN_2
#define ULN2003_IND_GPIO_Port GPIOD
#define ULN2003_INC_Pin GPIO_PIN_3
#define ULN2003_INC_GPIO_Port GPIOB
#define ULN2003_INB_Pin GPIO_PIN_4
#define ULN2003_INB_GPIO_Port GPIOB
#define ULN2003_INA_Pin GPIO_PIN_5
#define ULN2003_INA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
