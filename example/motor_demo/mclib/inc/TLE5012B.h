/**
  **************************************************************************
  * @file     TLE5012B.h
  * @brief    Definition and declaration of magnetic encoder(TLE5012B) peripheral configuration
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

#ifndef __TLE5012B_H
#define __TLE5012B_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

/* spi interface */
#define MAG_ENCODER_SPI_INTERFACE           SPI1
#define MAG_ENCODER_SPI_CRM_CLK             CRM_SPI1_PERIPH_CLOCK
#define MAG_ENCODER_SPI_CS_CRM_CLK          CRM_GPIOA_PERIPH_CLOCK
#define MAG_ENCODER_SPI_CS_GPIO_PORT        GPIOA
#define MAG_ENCODER_SPI_CS_GPIO_PIN         GPIO_PINS_4
#define MAG_ENCODER_SPI_CLK_CRM_CLK         CRM_GPIOA_PERIPH_CLOCK
#define MAG_ENCODER_SPI_CLK_GPIO_PORT       GPIOA
#define MAG_ENCODER_SPI_CLK_GPIO_PIN        GPIO_PINS_5
#define MAG_ENCODER_SPI_MISO_CRM_CLK        CRM_GPIOA_PERIPH_CLOCK
#define MAG_ENCODER_SPI_MISO_GPIO_PORT      GPIOA
#define MAG_ENCODER_SPI_MISO_GPIO_PIN       GPIO_PINS_6
#define MAG_ENCODER_SPI_MOSI_CRM_CLK        CRM_GPIOA_PERIPH_CLOCK
#define MAG_ENCODER_SPI_MOSI_GPIO_PORT      GPIOA
#define MAG_ENCODER_SPI_MOSI_GPIO_PIN       GPIO_PINS_7
#define MAG_ENCODER_SPI_CS_ENABLE           gpio_bits_write(MAG_ENCODER_SPI_CS_GPIO_PORT, MAG_ENCODER_SPI_CS_GPIO_PIN, FALSE)
#define MAG_ENCODER_SPI_CS_DISABLE		        gpio_bits_write(MAG_ENCODER_SPI_CS_GPIO_PORT, MAG_ENCODER_SPI_CS_GPIO_PIN, TRUE)

/* Magnetic encoder command */
#define READ_ANGLE_VALUE 0x8021

void mag_encoder_spi_init(void);
uint16_t SPI_ReadWriteByte(uint16_t TxData);
int16_t ReadValue(uint16_t RegValue);
int16_t ReadAngle(void);

#ifdef __cplusplus
}
#endif

#endif
