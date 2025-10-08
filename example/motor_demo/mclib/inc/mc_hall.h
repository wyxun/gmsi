/**
  **************************************************************************
  * @file     mc_hall.h
  * @brief    Declaration of hall related functions
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

#ifndef __MC_HALL_H
#define __MC_HALL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

#define HALL_INVERSE_MASK					0xFFF8

err_code_type read_hall_state(hall_sensor_type *hall_handler);

#if defined SIX_STEP_CONTROL && defined HALL_SENSORS
void hall_isr_handler(void);
void hall_learn_process(void);
#endif

#ifdef FOC_CONTROL
void hall_isr_handler(void);
err_code_type hall_at_zero_speed(hall_sensor_type *hall_handler);
void hall_learn_process(void);
void foc_hall_table_mapping(void);
#endif

#ifdef __cplusplus
}
#endif

#endif

