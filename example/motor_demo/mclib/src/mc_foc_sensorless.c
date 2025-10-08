/**
  **************************************************************************
  * @file     mc_foc_sensorless.c
  * @brief    foc sensorless related functions.
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

#include "mc_lib.h"

/**
  * @brief  clear sensorless related variables
  * @param  state_obs_handler: sensorless related variables
  * @retval none
  */
void observer_pll_clear(state_observer_type *state_obs_handler)
{
  state_obs_handler->wIalpha_est = ( int32_t )0;
  state_obs_handler->wIbeta_est = ( int32_t )0;
  state_obs_handler->hIalpha_est = ( int16_t )0;
  state_obs_handler->hIbeta_est = ( int16_t )0;
  state_obs_handler->wBemf_alpha_est = ( int32_t )0;
  state_obs_handler->wBemf_beta_est = ( int32_t )0;
  state_obs_handler->hBemf_alpha_est = ( int16_t )0;
  state_obs_handler->hBemf_beta_est = ( int16_t )0;
  state_obs_handler->elec_angle = ( int16_t )0;
  state_obs_handler->motor_speed.val = ( int32_t )0;
  state_obs_handler->motor_speed.filtered = ( int32_t )0;

  pid_set_integral(&state_obs_handler->pid_pll, ( int32_t )0);
}

/**
  * @brief  read EMF voltage offset value
  * @param  emf_abc_offset_handler: adc value of emf voltage offset
  * @param  motor_volt_handler: motor voltage related variables
  * @retval none
  */
void voltage_offset_init(abc_type* emf_abc_offset_handler, motor_volt_type* motor_volt_handler)
{
  static uint8_t icount = 0;

  emf_abc_offset_handler->a = 0;
  emf_abc_offset_handler->b = 0;
  emf_abc_offset_handler->c = 0;

  while (icount < 16)
  {
    if (dma_flag_get(ADC_ORDINARY_DMA_FT_STS_FLAG) != RESET)
    {
      dma_flag_get(ADC_ORDINARY_DMA_FT_STS_FLAG);

      emf_abc_offset_handler->a += *(motor_volt_handler->Va);
      emf_abc_offset_handler->b += *(motor_volt_handler->Vb);
      emf_abc_offset_handler->c += *(motor_volt_handler->Vc);

      icount ++;
    }
  }
  emf_abc_offset_handler->a >>= 4;
  emf_abc_offset_handler->b >>= 4;
  emf_abc_offset_handler->c >>= 4;
}

/**
  * @brief  calculate motor voltage values
  * @param  motor_volt_handler: motor voltage related variables
  * @retval none
  */
void motor_volt_calc(motor_volt_type *motor_volt_handler)
{
  int32_t MulTemp;
  int16_t Vbus_handler;

  Vbus_handler = *(motor_volt_handler->Vbus) << 3;

  MulTemp = (int32_t) motor_volt_handler->Valphabeta->alpha * Vbus_handler;
  motor_volt_handler->Vpu.alpha = MulTemp >> 15;

  MulTemp = (int32_t) motor_volt_handler->Valphabeta->beta * Vbus_handler;
  motor_volt_handler->Vpu.beta = MulTemp >> 15;
}

/**
  * @brief  read motor EMF voltage values
  * @param  motor_volt_handler: motor voltage related variables
  * @param  motor_emf_handler: motor EMF voltage related variables
  * @retval none
  */
void motor_volt_sense(motor_volt_type *motor_volt_handler, motor_emf_type *motor_emf_handler)
{
  abc_type *emf_abc_volt_handler = &(motor_emf_handler->emf_abc_voltage);
  abc_type *emf_abc_offset_handler = &(motor_emf_handler->emf_abc_offset);
  alphabeta_type *emf_alphabeta_volt_handler = &(motor_emf_handler->emf_alphabeta_voltage);

  emf_abc_volt_handler->a = (*(motor_volt_handler->Va) - emf_abc_offset_handler->a) << 4;
  emf_abc_volt_handler->b = (*(motor_volt_handler->Vb) - emf_abc_offset_handler->b) << 4;
  emf_abc_volt_handler->c = (*(motor_volt_handler->Vc) - emf_abc_offset_handler->c) << 4;

  foc_clarke_transform(emf_abc_volt_handler, emf_alphabeta_volt_handler);

  emf_alphabeta_volt_handler->alpha = (motor_volt_handler->emf_factor * emf_alphabeta_volt_handler->alpha) >> 15;
  emf_alphabeta_volt_handler->beta = (motor_volt_handler->emf_factor * emf_alphabeta_volt_handler->beta) >> 15;
}

