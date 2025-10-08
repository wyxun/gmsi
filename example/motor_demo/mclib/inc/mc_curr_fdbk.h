/**
  **************************************************************************
  * @file     mc_curr_fdbk.h
  * @brief    Declaration of current offset and sensing related functions.
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

#ifndef __MC_CURR_FDBK_H
#define __MC_CURR_FDBK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

flag_status current_offset_init(current_type *curr_handler, shunt_nbr_type shunt_nbr);
void current_read_foc_3shunt(current_type *curr_handler, pwm_duty_type *pwm_duty_handler);
void current_read_foc_2shunt(current_type *curr_handler);
void current_read_bldc(current_type *curr_handler);
int16_t Idc_offset_init(uint16_t *adc_value);
int16_t dc_current_read(current_type *curr_handler, uint16_t *adc_value);
void current_read_1shunt_ID(current_type *curr_handler);

#ifdef __cplusplus
}
#endif

#endif
