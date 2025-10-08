/**
  **************************************************************************
  * @file     mc_encoder.h
  * @brief    Declaration of encoder-related functions.
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

#ifndef __MC_ENCODER_H
#define __MC_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif


#include "mc_lib.h"

void encoder_count_reset(encoder_type *enc_handler, speed_type *spd_handler);
err_code_type encoder_alignment_index(voltage_type *volt_handler, encoder_type *enc_handler);
void encoder_alignment(voltage_type *volt_handler, encoder_type *enc_handler);
int16_t enc_rotor_angle_get(encoder_type *enc_handler, rotor_angle_type *rotor_angle_handler);
int32_t enc_rotor_speed_get(encoder_type *enc_handler, speed_type *spd_handler);
err_code_type enc_error_check(encoder_type *enc_handler, ramp_cmd_type *cmd_ramp_handler, current_type *curr_handler);

#ifdef __cplusplus
}
#endif

#endif
