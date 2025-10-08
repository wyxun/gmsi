/**
  **************************************************************************
  * @file     mc_foc_sensorless.h
  * @brief    Declaration of foc sensorless related functions.
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

#ifndef __MC_FOC_SENSORLESS_H
#define __MC_FOC_SENSORLESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

void observer_pll_clear(state_observer_type *state_obs_handler);
void voltage_offset_init(abc_type*, motor_volt_type*);
void motor_volt_calc(motor_volt_type *motor_volt_handler);
void motor_volt_sense(motor_volt_type *motor_volt_handler, motor_emf_type *motor_emf_handler);

#ifdef __cplusplus
}
#endif

#endif
