/**
  **************************************************************************
  * @file     mc_foc_kernal.h
  * @brief    Declaration of the kernal functions for Field Oriented Control(FOC)
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

#ifndef __MC_FOC_KERNAL_H
#define __MC_FOC_KERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

#define STEP_1_AH_PWM_BL                 0x01
#define STEP_2_BH_PWM_AL                 0x02
#define STEP_3_CH_PWM_BL                 0x03
#define STEP_4_BH_PWM_CL                 0x04
#define STEP_5_CH_PWM_AL                 0x05
#define STEP_6_AH_PWM_CL                 0x06
#define STEP_7_ANGLE_EST                 0x07
#define STEP_8_MAX                       0x08

/**************** define pwm mode pattern for angle init ******************/
#define TMR_PWM_1PWMB_2PWMB              0x7070
#define TMR_PWM_1PWMB_2OFF               0x0070
#define TMR_PWM_1OFF_2PWMB               0x7000
#define TMR_PWM_3PWMB                    0x0070

flag_status RECOGNIZE_ID(void);
firmware_id_type get_fw_id(void);
flag_status mc_param_init(uint8_t fw_id);
void calc_adc_sample_point(adc_sample_type *adc_sample, int16_t pwm_duty);
//int16_t pi_controller(pid_ctrl_type *pid_handler, int32_t var_err);
int16_t pid_controller_static_clamp(pid_ctrl_type *pid_handler, int32_t var_err);
int16_t pid_controller_dyna_clamp(pid_ctrl_type *pid_handler, int32_t var_err);
void bldc_sensorless_angle_init(angle_init_type *current_init);
uint8_t angle_init_estimation(angle_init_type *current);
void current_auto_tuning(i_auto_tune_type *i_tune_handler);
void param_identify(motor_param_id_type *mot_param_id_handler);
void param_id_process(motor_param_id_type *mot_param_id_handler);
#ifdef __cplusplus
}
#endif

#endif
