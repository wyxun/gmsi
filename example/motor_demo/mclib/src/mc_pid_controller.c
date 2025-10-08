/**
  **************************************************************************
  * @file     mc_pid_controller.c
  * @brief    pid control related functions.
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

#include "mc_lib.h"

/**
  * @brief  set Kp value
  * @param  pid_handler: pid controller related variables
  * @param  kp_gain: proportional gain
  * @retval none
  */
void pid_set_kp(pid_ctrl_type *pid_handler, int16_t kp_gain)
{
  pid_handler->kp_gain = kp_gain;
}

/**
  * @brief  set Ki value
  * @param  pid_handler: pid controller related variables
  * @param  ki_gain: integral gain
  * @retval none
  */
void pid_set_ki(pid_ctrl_type *pid_handler, int16_t ki_gain)
{
  pid_handler->ki_gain = ki_gain;
}

/**
  * @brief  set Kd value
  * @param  pid_handler: pid controller related variables
  * @param  kd_gain: derivative gain
  * @retval none
  */
void pid_set_kd(pid_ctrl_type *pid_handler, int16_t kd_gain)
{
  pid_handler->kd_gain = kd_gain;
}

/**
  * @brief  set integral value
  * @param  pid_handler: pid controller related variables
  * @param  integral_value: integral value
  * @retval none
  */
void pid_set_integral(pid_ctrl_type *pid_handler, int32_t integral_value)
{
  pid_handler->integral = integral_value;
}

/**
  * @brief  get Kp value
  * @param  pid_handler: pid controller related variables
  * @retval proportional gain
  */
int16_t pid_get_kp(pid_ctrl_type *pid_handler)
{
  return ( pid_handler->kp_gain );
}

/**
  * @brief  get Ki value
  * @param  pid_handler: pid controller related variables
  * @retval integral gain
  */
int16_t pid_get_ki(pid_ctrl_type *pid_handler)
{
  return ( pid_handler->ki_gain );
}

/**
  * @brief  get Kd value
  * @param  pid_handler: pid controller related variables
  * @retval derivative gain
  */
int16_t pid_get_kd(pid_ctrl_type *pid_handler)
{
  return ( pid_handler->kd_gain );
}

/**
  * @brief  set auto-tune current pid parameters
  * @param  i_tune_handler: current pid auto-tune related variables
  * @param  pid_handler: pid controller related variables
  * @retval none
  */
void set_current_pid_param(i_auto_tune_type *i_tune_handler, pid_ctrl_type *pid_handler)
{
  pid_handler->kp_gain = i_tune_handler->kp;
  pid_handler->ki_gain = i_tune_handler->ki;
  pid_handler->kp_shift = i_tune_handler->kp_shift;
  pid_handler->ki_shift = i_tune_handler->ki_shift;

  pid_handler->upper_limit_integral = pid_handler->upper_limit_output << pid_handler->ki_shift;
  pid_handler->lower_limit_integral = pid_handler->lower_limit_output << pid_handler->ki_shift;
}

/**
  * @brief  convert step command to slope command
  * @param  cmd_ramp_handler: command and slope related variables
  * @retval none
  */
void command_ramp(ramp_cmd_type *cmd_ramp_handler)
{
  int32_t cmd_ramp_err;

  cmd_ramp_err = cmd_ramp_handler->cmd_final - cmd_ramp_handler->command;

  if (cmd_ramp_err >= cmd_ramp_handler->acc_slope)
  {
    cmd_ramp_handler->command += cmd_ramp_handler->acc_slope;

    if (cmd_ramp_handler->command >= cmd_ramp_handler->cmd_final)
    {
      cmd_ramp_handler->command = cmd_ramp_handler->cmd_final;
    }
  }
  else if (cmd_ramp_err < -cmd_ramp_handler->dec_slope)
  {
    cmd_ramp_handler->command -= cmd_ramp_handler->dec_slope;

    if (cmd_ramp_handler->command <= cmd_ramp_handler->cmd_final)
    {
      cmd_ramp_handler->command = cmd_ramp_handler->cmd_final;
    }
  }
  else
  {
    cmd_ramp_handler->command = cmd_ramp_handler->cmd_final;
  }
}

