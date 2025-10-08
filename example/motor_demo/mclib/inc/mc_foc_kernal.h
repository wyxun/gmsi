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

flag_status RECOGNIZE_ID(void);
firmware_id_type get_fw_id(void);
flag_status mc_param_init(firmware_id_type fw_id);
void foc_circle_limitation (voltage_type *volt_handler);
void foc_vq_limitation(voltage_type *volt_handler, pid_ctrl_type *pid_handler);
void current_read_foc_1shunt(current_type *curr_handler, voltage_type *volt_handler);
void rds_auto_calibration(rds_cali_type *rds_cali_handler, current_type *curr_handler, voltage_type *volt_handler);
int32_t enc_speed_get_MTmethod(encoder_type *enc_handler, speed_type *spd_handler);
void position_cmd_ramp(position_type *pos_handler, ramp_cmd_type *cmd_ramp_handler, pid_ctrl_type *pid_handler);
int16_t pid_controller_static_clamp(pid_ctrl_type *pid_handler, int32_t var_err);
int16_t pid_controller_dyna_clamp(pid_ctrl_type *pid_handler, int32_t var_err);
void svpwm_3shunt(voltage_type *volt_handler, pwm_duty_type *pwm_duty_handler);
void svpwm_2shunt(voltage_type *volt_handler, pwm_duty_type *pwm_duty_handler);
void svpwm_1shunt(voltage_type *volt_handler, pwm_duty_type *pwm_duty_handler);
adc_trigger_type pwm_shift(int16_t *dTa, int16_t *dTb, int16_t *dTc, int16_t Ta, int16_t Tb, int16_t Tc, pwm_duty_type *pwm_duty_handler);
int16_t atan2_fixed(int32_t y, int32_t x);
void sqrt_fixed(int32_t input, int16_t *out_ptr);
//qd_type fw_curr_ref(field_weakening_type *fw_handler, qd_type Iqdref);
int16_t hall_rotor_angle_get(hall_sensor_type *hall_handler, rotor_angle_type *rotor_angle_handler);
int16_t hall_delta_theta_calculation(speed_type *rotor_speed_handler, rotor_angle_type *rotor_angle_handler, hall_sensor_type *hall_handler);
flag_status startup_openloop(sensorless_startup_type *startup_handler, qd_type *Iqdref_handler);
flag_status startup_alpha_axis(sensorless_startup_type *startup_handler, qd_type *Iqdref_handler);
flag_status startup_angle_init(sensorless_startup_type *startup_handler, qd_type *Iqdref_handler);
flag_status startup_angle_init2(sensorless_startup_type *startup_handler, qd_type *Iqdref_handler);
void foc_sensorless_angle_init(foc_angle_init_type *angle_detect_handler, current_type *curr_handler);
int16_t obs_pll_execute(state_observer_type *state_obs_handler, int16_t hBemf_alfa_est, int16_t hBemf_beta_est);
int16_t rotor_angle_sensorless(state_observer_type *state_obs_handler, motor_volt_type *motor_volt_handler);
void current_angle_init_3shunt(foc_angle_init_type *angle_detect_handler, current_type *curr_handler);
void current_angle_init_2_1shunt(foc_angle_init_type *angle_detect_handler, current_type *curr_handler);
void current_auto_tuning(i_auto_tune_type *i_tune_handler);
void param_identify(motor_param_id_type *mot_param_id_handler);
void param_id_process(motor_param_id_type *mot_param_id_handler);
#ifdef __cplusplus
}
#endif

#endif
