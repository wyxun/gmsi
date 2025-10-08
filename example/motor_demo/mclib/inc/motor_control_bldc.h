/**
  **************************************************************************
  * @file     motor_control_bldc.h
  * @brief    Declaration of motor control related funciton for BLDC
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

#ifndef __MOTOR_CONTROL_BLDC_H
#define __MOTOR_CONTROL_BLDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

void param_clear(void);
void param_init(void);
void hall_to_tmr_register_setting(void);
void hall_learn_register_setting(void);
void disable_mosfet(tmr_type *tmr_x);
void I_offset_init(void);
void I_tune_manual(void);
void hall_cw_ctrl_para(void);
void hall_ccw_ctrl_para(void);
void pwm_duty_update(void);
void set_adc_sample_point(adc_sample_type *adc_sample);
void calc_motor_speed(speed_type *rotor_speed);
void open_loop_cmd_ramp(olc_type *openloop_handler);
void enable_pwm_timer(void);
void disable_pwm_timer(void);
#ifdef __cplusplus
}
#endif

#endif

