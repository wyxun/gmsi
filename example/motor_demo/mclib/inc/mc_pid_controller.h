/**
  **************************************************************************
  * @file     mc_pid_controller.h
  * @brief    Declaration of pid control related functions.
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

#ifndef __MC_PID_CONTROLLER_H
#define __MC_PID_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif


#include "mc_lib.h"

void pid_set_kp(pid_ctrl_type *pid_handler, int16_t kp_gain);
void pid_set_ki(pid_ctrl_type *pid_handler, int16_t ki_gain);
void pid_set_kd(pid_ctrl_type *pid_handler, int16_t kd_gain);
void pid_set_integral(pid_ctrl_type *pid_handler, int32_t integral_value);
int16_t pid_get_kp(pid_ctrl_type *pid_handler);
int16_t pid_get_ki(pid_ctrl_type *pid_handler);
int16_t pid_get_kd(pid_ctrl_type *pid_handler);
void set_current_pid_param(i_auto_tune_type *i_tune_handler, pid_ctrl_type *pid_handler);
void command_ramp(ramp_cmd_type *cmd_ramp_handler);

#ifdef __cplusplus
}
#endif

#endif
