/**
  **************************************************************************
  * @file     mc_bldc.h
  * @brief
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

#ifndef __MC_BLDC_H
#define __MC_BLDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

void bldc_output_config(uint8_t);
void start_bldc(void);
void align_bldc(void);
void bldc_open_loop_ctrl(hall_sensor_type *hall_handler, olc_type *openloop_handler, speed_type *rotor_speed);
void bldc_hall_learning(hall_sensor_type *hall_handler, hall_learn_type *hall_learn);

#ifdef __cplusplus
}
#endif

#endif

