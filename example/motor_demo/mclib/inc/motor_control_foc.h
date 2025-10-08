/**
  **************************************************************************
  * @file     motor_control_foc.h
  * @brief    Declaration of motor control related funciton for Field Oriented Control(FOC)
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

#ifndef __MOTOR_CONTROL_FOC_H
#define __MOTOR_CONTROL_FOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

void param_clear(void);
void param_init(void);
void charge_boot_cap(void);
void enable_pwm_timer_channel_buffer(void);
void enable_pwm_timer(void);
void disable_pwm_timer(void);
void pwm_switch_off(void);
void pwm_switch_on(void);
void I_offset_init(void);
void I_tune_manual(void);
void hall_cw_ctrl_para(void);
void hall_ccw_ctrl_para(void);
void curr_cmd_handler_ebike(void);
void dc_current_limit(void);
void position_control_handler(void);
void speed_pid_param(void);
void monitoring_signal_adc_trigger(void);
void svpwm_func(void);
void pwm_duty_update(void);
void pwm_duty_extra_update(void);
void set_normal_pwm_mode(void);
void lock_rotor(void);

#ifdef __cplusplus
}
#endif

#endif
