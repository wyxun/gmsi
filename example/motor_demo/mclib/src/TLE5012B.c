/**
  **************************************************************************
  * @file     TLE5012B.c
  * @brief    Magnetic encoder(TLE5012B) configuration with spi communication
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

/* ----------------------------------------------------------------------
** Include Files
** ------------------------------------------------------------------- */
#include "mc_lib.h"

/**
  * @brief  Initialization of SPI
  * @param  none
  * @retval none
  */
void mag_encoder_spi_init(void)
{
  gpio_init_type gpio_init_struct;
  // Config CS pin
  crm_periph_clock_enable(MAG_ENCODER_SPI_CS_CRM_CLK, TRUE);
  gpio_default_para_init(&gpio_init_struct);
  gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull           = GPIO_PULL_DOWN;
  gpio_init_struct.gpio_mode           = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_pins           = MAG_ENCODER_SPI_CS_GPIO_PIN;
  gpio_init(MAG_ENCODER_SPI_CS_GPIO_PORT, &gpio_init_struct);
  // Config CLK pin
  crm_periph_clock_enable(MAG_ENCODER_SPI_CLK_CRM_CLK, TRUE);
  gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull           = GPIO_PULL_DOWN;
  gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_pins           = MAG_ENCODER_SPI_CLK_GPIO_PIN;
  gpio_init(MAG_ENCODER_SPI_CLK_GPIO_PORT, &gpio_init_struct);
  // Config MISO pin
  crm_periph_clock_enable(MAG_ENCODER_SPI_MISO_CRM_CLK, TRUE);
  gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_OPEN_DRAIN;
  gpio_init_struct.gpio_pull           = GPIO_PULL_DOWN;
  gpio_init_struct.gpio_mode           = GPIO_MODE_INPUT;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_pins           = MAG_ENCODER_SPI_MISO_GPIO_PIN;
  gpio_init(MAG_ENCODER_SPI_MISO_GPIO_PORT, &gpio_init_struct);
// Config MOSI pin
  crm_periph_clock_enable(MAG_ENCODER_SPI_MOSI_CRM_CLK, TRUE);
  gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_OPEN_DRAIN;
  gpio_init_struct.gpio_pull           = GPIO_PULL_DOWN;
  gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_pins           = MAG_ENCODER_SPI_MOSI_GPIO_PIN;
  gpio_init(MAG_ENCODER_SPI_MOSI_GPIO_PORT, &gpio_init_struct);

  spi_init_type spi_init_struct;
  crm_periph_clock_enable(MAG_ENCODER_SPI_CRM_CLK, TRUE);
  spi_default_para_init(&spi_init_struct);
  spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
  spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
  spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_64;
  spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
  spi_init_struct.frame_bit_num = SPI_FRAME_16BIT;
  spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;
  spi_init_struct.clock_phase = SPI_CLOCK_PHASE_2EDGE;
  spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;
  spi_init(MAG_ENCODER_SPI_INTERFACE, &spi_init_struct);

  spi_enable(MAG_ENCODER_SPI_INTERFACE, TRUE);
}

/**
  * @brief  SPI write and read data byte function
  * @param  TxData: transmit data byte
  * @retval receive data byte
  */
uint16_t SPI_ReadWriteByte(uint16_t TxData)
{
  uint8_t retry = 0;

  while(spi_i2s_flag_get(SPI1, SPI_I2S_TDBE_FLAG) == RESET)
  {
    retry++;

    if (retry > 200)
    {
      return 0;
    }
  }

  spi_i2s_data_transmit(SPI1, TxData);
  retry = 0;

  while(spi_i2s_flag_get(SPI1, SPI_I2S_RDBF_FLAG) == RESET)
  {
    retry++;

    if (retry > 200)
    {
      return 0;
    }
  }

  return spi_i2s_data_receive(SPI1);
}

/**
  * @brief  read the register value.
  * @param  RegValue: register address
  * @retval register data
  */
int16_t ReadValue(uint16_t RegValue)
{
  int16_t data;

  MAG_ENCODER_SPI_CS_ENABLE;
  SPI_ReadWriteByte(RegValue);
  data = SPI_ReadWriteByte(0xFFFF);
  MAG_ENCODER_SPI_CS_DISABLE;
  return(data);
}

/**
  * @brief  read the angle value.
  * @param  none
  * @retval angle value
  */
int16_t ReadAngle(void)
{
  return(ReadValue(READ_ANGLE_VALUE) & 0x7FFF);
}
