/**
  **************************************************************************
  * @file     motor_control_foc.c
  * @brief    motor control related funciton for Field Oriented Control(FOC)
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
  * @brief  clear the control parameters
  * @param  none
  * @retval none
  */
void param_clear(void)
{
  abc_type null_abc = {(int16_t)0, (int16_t)0, (int16_t)0};
  qd_type null_qd = {(int16_t)0, (int16_t)0};
  alphabeta_type null_alphabeta =  {(int16_t)0, (int16_t)0};

  charge_boot_count = 0;

  speed_ramp.cmd_final = 0;
  speed_ramp.command = 0;

  current.Iabc = null_abc;
  current.Iabc_shunt = null_abc;
  current.Ialphabeta = null_alphabeta;
  current.Iqd = null_qd;
  current.Iqd_LPF = null_qd;
  current.Iqdref = null_qd;

  volt_cmd.Vqd = null_qd;
  volt_cmd.Valphabeta = null_alphabeta;

  pid_id.integral = 0;
  pid_iq.integral = 0;
  pid_spd.integral = 0;
  pid_pos.integral = 0;

#ifdef LOW_SPEED_VOLT_CTRL
  pid_spd_volt.integral = 0;
#endif

  pwm_duty.OF.a = pwm_duty.half_duty;
  pwm_duty.OF.b = pwm_duty.half_duty;
  pwm_duty.OF.c = pwm_duty.half_duty;
  pwm_duty.UF.a = pwm_duty.half_duty;
  pwm_duty.UF.b = pwm_duty.half_duty;
  pwm_duty.UF.c = pwm_duty.half_duty;

  PWM_ADVANCE_TIMER->c1dt = pwm_duty.half_duty;
  PWM_ADVANCE_TIMER->c2dt = pwm_duty.half_duty;
  PWM_ADVANCE_TIMER->c3dt = pwm_duty.half_duty;

#ifdef SENSORLESS
  observer_pll_clear(&state_observer);
  startup.closeloop_rdy = RESET;
  startup.closeloop_rdy_old = RESET;
#endif

#ifdef FIELD_WEAKENING
  fw_clear(&field_weakening, &volt_cmd);
#endif

#ifdef E_BIKE_SCOOTER
  I_ref = null_qd;
  Iq_ref_cmd = 0;
  pid_spd.upper_limit_integral = 0;
  pid_spd.lower_limit_integral = 0;
  pid_spd.upper_limit_output = 0;
  pid_spd.lower_limit_output = 0;
  current.Idc.val = 0;
  current.Idc.filtered = 0;
#endif

  /* clear output temp data in low pass filter */
#if defined SENSORLESS
  obs_speed_LPF.output_temp = 0;
#endif
#if CURRENT_LP_FILTER
  q_current_LPF.output_temp = 0;
  d_current_LPF.output_temp = 0;
#endif
#if RDS_AUTO_CALIBRATION
  Rds_Cali.Iq_LPF.output_temp = 0;
  Rds_Cali.Id_LPF.output_temp = 0;
#endif
}

/**
  * @brief  initialization of motor control parameters
  * @param  none
  * @retval none
  */
void param_init(void)
{
#if defined INCREM_ENCODER
  encoder.mech_to_elect_angle_shift = 16-ceil(log2(encoder.pole_pairs));
  encoder.mech_to_elect_angle = ENC_MECH_TO_ELECT_ANGLE * pow(2,encoder.mech_to_elect_angle_shift);
#endif
#if defined HALL_SENSORS
  foc_hall_table_mapping();
  hall_interval_moving_average = moving_average(6);
  hall_next_state_table = hall_cw_next_state_table;
  hall_theta_table = hall_cw_theta_table;

  error_code |= error_code_mask & hall_at_zero_speed(&hall);
  hall.slick_speed = SLICK_SPEED_RPM_SHIFT;
  rotor_angle_hall_old.elec_angle_val = rotor_angle_hall.elec_angle_val;
  rotor_angle_hall.elec_angle_pre_val = rotor_angle_hall.elec_angle_val;
#endif
#if defined SENSORLESS
  lowpass_filter_init(&obs_speed_LPF);
  state_observer.c1 = (int16_t)(0x7FFF*EXP_TAYLOR(-(*state_observer.Rs)/((*state_observer.Ls)*PWM_FREQ)));
  state_observer.c3 = (int32_t)(0x7FFF*((ZS_BASE/(*state_observer.Rs))*(1.0 - EXP_TAYLOR(-(*state_observer.Rs)/((*state_observer.Ls)*PWM_FREQ)))));
#endif
#if CURRENT_LP_FILTER
  lowpass_filter_init(&q_current_LPF);
  lowpass_filter_init(&d_current_LPF);
#endif
#if DC_CURRENT_LIMIT
  Idc_ma_fliter = moving_average(DC_CURR_MA_NBR);
#endif
#if RDS_AUTO_CALIBRATION
  lowpass_filter_init(&Rds_Cali.Iq_LPF);
  lowpass_filter_init(&Rds_Cali.Id_LPF);
#endif
  speed_ma_fliter = moving_average(10);
  foc_rdy = RESET;
  pwm_switch_off();
  param_clear();
  ui_wave_param.sample_cycle = (uint8_t)(((DATA_BUFFER_FRAME_SIZE*10.0/UI_UART_BAUDRATE)+0.002)*PWM_FREQ/(DATA_BUFFER_SIZE/4)+1);
}

/**
  * @brief  gate drive capacitor precharge function
  * @param  none
  * @retval none
  */
void charge_boot_cap(void)
{
  tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_1, 0);
  tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_2, 0);
  tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_3, 0);
  mc_delay_ms(1);
  tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_1, pwm_duty.half_duty);
  tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_2, pwm_duty.half_duty);
  tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_3, pwm_duty.half_duty);
}

/**
  * @brief  enable pwm timer channel mode buffer
  * @param  none
  * @retval none
  */
void enable_pwm_timer_channel_buffer(void)
{
  tmr_channel_buffer_enable(PWM_ADVANCE_TIMER, TRUE);
}

/**
  * @brief  enable pwm timer counter
  * @param  none
  * @retval none
  */
void enable_pwm_timer(void)
{
  tmr_counter_enable(PWM_ADVANCE_TIMER, TRUE);
}

/**
  * @brief  disable pwm timer counter
  * @param  none
  * @retval none
  */
void disable_pwm_timer(void)
{
  tmr_counter_enable(PWM_ADVANCE_TIMER, FALSE);
}

#if defined AT32F423xx || defined AT32F425xx
/**
  * @brief  disable pwm output
  * @param  none
  * @retval none
  */
void pwm_switch_off(void)
{
  tmr_output_enable(PWM_ADVANCE_TIMER, FALSE);
  tmr_output_enable(ADC_TIMER, FALSE);
  tmr_channel_enable(ADC_TIMER, ADC_TIMER_SELECT_CHANNEL, FALSE);
}

/**
  * @brief  enable pwm output
  * @param  none
  * @retval none
  */
void pwm_switch_on(void)
{
  tmr_output_enable(PWM_ADVANCE_TIMER, FALSE);
  tmr_counter_enable(PWM_ADVANCE_TIMER, FALSE);
  tmr_channel_enable(ADC_TIMER, ADC_TIMER_SELECT_CHANNEL, FALSE);
  tmr_output_enable(ADC_TIMER, FALSE);
  tmr_counter_enable(ADC_TIMER, FALSE);
#ifdef ONE_SHUNT
  dma_channel_enable(TMR_ADC_DMA_CH, FALSE);
  /* reset adc preempt conversion order */
  ADC_CONVERTER->psq_bit.pclen = 0;
  ADC_CONVERTER->psq_bit.pclen = 1;
  dma_channel_enable(TMR_ADC_DMA_CH, TRUE);
  pwm_duty.adc_trig.first_pos = (PWM_PERIOD/2);
  pwm_duty.adc_trig.second_pos[0] = (PWM_PERIOD/4);

  tmr_counter_value_set(ADC_TIMER, PWM_PERIOD);
  tmr_counter_value_set(PWM_ADVANCE_TIMER, PWM_PERIOD);

  tmr_channel_value_set(ADC_TIMER, ADC_TIMER_SELECT_CHANNEL, pwm_duty.adc_trig.first_pos);
#endif
  tmr_counter_enable(ADC_TIMER, TRUE);
  tmr_counter_enable(PWM_ADVANCE_TIMER, TRUE);
  tmr_output_enable(PWM_ADVANCE_TIMER, TRUE);
  tmr_channel_enable(ADC_TIMER, ADC_TIMER_SELECT_CHANNEL, TRUE);
  tmr_output_enable(ADC_TIMER, TRUE);
}

#else

/**
  * @brief  disable pwm output
  * @param  none
  * @retval none
  */
void pwm_switch_off(void)
{
  tmr_output_enable(PWM_ADVANCE_TIMER, FALSE);
}

/**
  * @brief  enable pwm output
  * @param  none
  * @retval none
  */
void pwm_switch_on(void)
{
  tmr_output_enable(PWM_ADVANCE_TIMER, FALSE);
  tmr_counter_enable(PWM_ADVANCE_TIMER, FALSE);
#ifdef ONE_SHUNT
  dma_channel_enable(TMR_ADC_DMA_CH, FALSE);
  TMR_ADC_DMA_CH->dtcnt = 2;
  /* reset adc preempt conversion order */
  ADC_CONVERTER->psq_bit.pclen = 0;
  ADC_CONVERTER->psq_bit.pclen = 1;
  dma_channel_enable(TMR_ADC_DMA_CH, TRUE);
  pwm_duty.adc_trig.first_pos = (PWM_PERIOD/2);
  pwm_duty.adc_trig.second_pos[0] = (PWM_PERIOD/4);
  pwm_duty.adc_trig.second_pos[1] = PWM_PERIOD;
  tmr_counter_value_set(PWM_ADVANCE_TIMER, PWM_PERIOD);
  tmr_channel_value_set(PWM_ADVANCE_TIMER, ADC_TIMER_SELECT_CHANNEL, pwm_duty.adc_trig.first_pos);
#endif
  tmr_counter_enable(PWM_ADVANCE_TIMER, TRUE);
  tmr_output_enable(PWM_ADVANCE_TIMER, TRUE);
}

#endif


/**
  * @brief  current offset initialize function
  * @param  none
  * @retval none
  */
void I_offset_init(void)
{
#if defined THREE_SHUNT
  curr_offset_rdy = current_offset_init(&current, THREESHUNT);
#elif defined TWO_SHUNT
  curr_offset_rdy = current_offset_init(&current, TWOSHUNT);
#elif defined ONE_SHUNT
  curr_offset_rdy = current_offset_init(&current, ONESHUNT);
#endif
  adc_flag_clear(current.ADCx, ADC_PCCE_FLAG);
  adc_interrupt_enable(current.ADCx, ADC_PCCE_INT, TRUE); /* Enable current sensing interrupt */
}

/**
  * @brief  current mamual tuning function
  * @param  none
  * @retval none
  */
void I_tune_manual(void)
{
  /* Id-axis or Iq-axis pulse */
  I_tune_count++;

  if (I_tune_count % current_tune_total_period == 0)
  {
    I_tune_count = 0;
  }

  if (I_tune_count < current_tune_step_period)
  {
    if (ctrl_mode == ID_MANUAL_TUNE)
    {
#ifdef E_BIKE_SCOOTER
      I_ref.d= current_tune_target_current;
#else
      current.Iqdref.d = current_tune_target_current;
#endif
    }
    else
    {
#ifdef E_BIKE_SCOOTER
      I_ref.q = current_tune_target_current;
#else
      current.Iqdref.q = current_tune_target_current;
#endif
    }
  }
  else
  {
    if (ctrl_mode == ID_MANUAL_TUNE)
    {
#ifdef E_BIKE_SCOOTER
      I_ref.d = 0;
#else
      current.Iqdref.d = 0;
#endif
    }
    else
    {
#ifdef E_BIKE_SCOOTER
      I_ref.q = 0;
#else
      current.Iqdref.q = 0;
#endif
    }
  }
}

/**
  * @brief  hall control parameters cofiguration(dir:CW)
  * @param  none
  * @retval none
  */
void hall_cw_ctrl_para(void)
{
  hall_next_state_table = hall_cw_next_state_table;
  hall_theta_table = hall_cw_theta_table;
  rotor_speed_hall.dir = CW;
  pid_iq.upper_limit_integral = VQ_MAX << pid_iq.ki_shift;
  pid_iq.lower_limit_integral = 0;
  pid_iq.upper_limit_output = VQ_MAX;
  pid_iq.lower_limit_output = 0;

#ifdef LOW_SPEED_VOLT_CTRL
  pid_spd_volt.upper_limit_integral = VQ_MAX << pid_spd_volt.ki_shift;
  pid_spd_volt.lower_limit_integral = 0;
  pid_spd_volt.upper_limit_output = VQ_MAX;
  pid_spd_volt.lower_limit_output = 0;
#endif

}

/**
  * @brief  hall control parameters cofiguration(dir:CCW)
  * @param  none
  * @retval none
  */
void hall_ccw_ctrl_para(void)
{
  hall_next_state_table = hall_ccw_next_state_table;
  hall_theta_table = hall_ccw_theta_table;
  rotor_speed_hall.dir = CCW;
  pid_iq.upper_limit_integral = 0;
  pid_iq.lower_limit_integral = -VQ_MAX << pid_iq.ki_shift;
  pid_iq.upper_limit_output = 0;
  pid_iq.lower_limit_output = -VQ_MAX;

#ifdef LOW_SPEED_VOLT_CTRL
  pid_spd_volt.upper_limit_integral = 0;
  pid_spd_volt.lower_limit_integral = -VQ_MAX << pid_spd_volt.ki_shift;
  pid_spd_volt.upper_limit_output = 0;
  pid_spd_volt.lower_limit_output = -VQ_MAX;
#endif
}

/**
  * @brief  control parameters cofiguration in torque control(E_BIKE_SCOOTER Mode)
  * @param  none
  * @retval none
  */
void curr_cmd_handler_ebike(void)
{
  if (Iq_ref_cmd >= 0)
  {
    pid_spd.upper_limit_output = Iq_ref_cmd;
    pid_spd.lower_limit_output = -Iq_ref_cmd;
    pid_spd.upper_limit_integral = (Iq_ref_cmd << pid_spd.ki_shift);
    pid_spd.lower_limit_integral = -(Iq_ref_cmd << pid_spd.ki_shift);
    speed_ramp.cmd_final = MAX_SPEED_RPM;
    speed_ramp.command = MAX_SPEED_RPM;
  }
  else
  {
    pid_spd.upper_limit_output = -Iq_ref_cmd;
    pid_spd.lower_limit_output = Iq_ref_cmd;
    pid_spd.upper_limit_integral = -(Iq_ref_cmd << pid_spd.ki_shift);
    pid_spd.lower_limit_integral = (Iq_ref_cmd << pid_spd.ki_shift);
    speed_ramp.cmd_final = REVERSE_MAX_SPEED_RPM;
    speed_ramp.command = REVERSE_MAX_SPEED_RPM;
  }
}

/**
  * @brief  dc current limitation function(E_BIKE_SCOOTER Mode)
  * @param  none
  * @retval none
  */
void dc_current_limit(void)
{
  if (Iq_ref_cmd >= 0)
  {
    current.Idc.val = dc_current_read(&current, &adc_in_tab[ADC_IBUS_AVE_IDX]);
    current.Idc.filtered = moving_average_update(Idc_ma_fliter, current.Idc.val);

    if (current.Idc.filtered >= DC_MAX_CURRENT_PU)
    {
      if (Iq_ref_cmd > DC_MAX_CURRENT_PU)
        I_ref.q -= REDUCE_IQ;

      if (I_ref.q < 0)
        I_ref.q = 0;
    }
    else
    {
      I_ref.q += RECOVER_IQ;

      if (I_ref.q >= Iq_ref_cmd)
        I_ref.q = Iq_ref_cmd;

    }

    pid_spd.upper_limit_output = I_ref.q;
    pid_spd.lower_limit_output = -I_ref.q;
    pid_spd.upper_limit_integral = (I_ref.q << pid_spd.ki_shift);
    pid_spd.lower_limit_integral = -(I_ref.q << pid_spd.ki_shift);
  }
}

/**
  * @brief  position controller and command configuration
  * @param  none
  * @retval none
  */
void position_control_handler(void)
{
  int32_t pos_err;
#if defined HALL_SENSORS && defined LOW_SPEED_VOLT_CTRL
  static flag_status lock_flag = RESET;
#endif

  pos.cmd_new = (int32_t)(angle.cmd_final * ANGLE_TO_PULSE);   /* if angle.cmd_final = 36010, which mean 36010/100 = 360.1 degree */

  if (pos.cmd_new != pos.cmd_final)
  {
    if (pos.stable == TRUE)
    {
      pos.cmd_final = pos.cmd_new;
#if defined HALL_SENSORS && defined LOW_SPEED_VOLT_CTRL
      set_normal_pwm_mode();
      lock_flag = RESET;
#endif
      if (pos.cmd_final >= pos.val)
      {
        pid_pos.lower_limit_output = -pos.min_pos_ctrl_spd;
        pid_pos.lower_limit_integral = -pos.min_pos_ctrl_spd << (pid_pos.ki_shift);
        pid_pos.upper_limit_output = intCoeffs32[MC_PROTOCOL_REG_MAX_APP_SPEED];
        pid_pos.upper_limit_integral = intCoeffs32[MC_PROTOCOL_REG_MAX_APP_SPEED] << (pid_pos.ki_shift);
      }
      else
      {
        pid_pos.upper_limit_output = pos.min_pos_ctrl_spd;
        pid_pos.upper_limit_integral = pos.min_pos_ctrl_spd << (pid_pos.ki_shift);
        pid_pos.lower_limit_output = -intCoeffs32[MC_PROTOCOL_REG_MAX_APP_SPEED];
        pid_pos.lower_limit_integral = -(intCoeffs32[MC_PROTOCOL_REG_MAX_APP_SPEED] << (pid_pos.ki_shift));
      }
    }
    else
    {
      pos.cmd_new = pos.cmd_final;
      angle.cmd_final = (int32_t)(pos.cmd_new * PULSE_TO_ANGLE);
    }
  }
  position_cmd_ramp(&pos, &speed_ramp, &pid_pos);

  pos_err = pos.command - pos.val;

#if defined HALL_SENSORS && defined LOW_SPEED_VOLT_CTRL
  if (pos.stable == TRUE)
  {
    if (abs(pos_err) < ROTOR_LOCK_GAP)
    {
      lock_rotor();
      lock_flag = SET;
    }
  }
  else
  {
    if (lock_flag == SET)
    {
      set_normal_pwm_mode();
      lock_flag = RESET;
    }
    speed_ramp.cmd_final = pid_controller(&pid_pos, pos_err);
  }
#else
  speed_ramp.cmd_final = pid_controller(&pid_pos, pos_err);
#endif
  angle.command = (int32_t)(pos.command * PULSE_TO_ANGLE);
}

/**
  * @brief  fine-tune speed pi parameters at low speed control
  * @param  none
  * @retval none
  */
void speed_pid_param(void)
{
  if (speed_ramp.command >= 0)
  {
    if (speed_ramp.command == 0)
    {
      pid_spd.kp_gain = 400;
      pid_spd.ki_gain = 1500;
    }
    else if (speed_ramp.command > 0 && speed_ramp.command < 20)
    {
      pid_spd.kp_gain = 400;
      pid_spd.ki_gain = 1500;
    }
    else if (speed_ramp.command >= 20 && speed_ramp.command < 100)
    {
      pid_spd.kp_gain = 2000;
      pid_spd.ki_gain = 1000;
    }
    else if (speed_ramp.command >= 100)
    {
      pid_spd.kp_gain = 2000;
      pid_spd.ki_gain = 50;
    }
  }
  else
  {
    if (speed_ramp.command > -20)
    {
      pid_spd.kp_gain = 400;
      pid_spd.ki_gain = 1500;
    }
    else if (speed_ramp.command <= -20 && speed_ramp.command > -100)
    {
      pid_spd.kp_gain = 2000;
      pid_spd.ki_gain = 1000;
    }
    else if (speed_ramp.command <= -100)
    {
      pid_spd.kp_gain = 2000;
      pid_spd.ki_gain = 50;
    }
  }
}

/**
  * @brief  enable adc trigger for monitoring signals
  * @param  none
  * @retval none
  */
void monitoring_signal_adc_trigger(void)
{
  adc_ordinary_software_trigger_enable(ADC_CONVERTER, TRUE);
}

/**
  * @brief  space vector modulation function(1/2/3-shunt)
  * @param  none
  * @retval none
  */
void svpwm_func(void)
{
#if defined THREE_SHUNT
  svpwm_3shunt(&volt_cmd, &pwm_duty);
#elif defined TWO_SHUNT
  svpwm_2shunt(&volt_cmd, &pwm_duty);
#elif defined ONE_SHUNT
  svpwm_1shunt(&volt_cmd, &pwm_duty);
  tmr_channel_value_set(ADC_TIMER, ADC_TIMER_SELECT_CHANNEL, pwm_duty.adc_trig.first_pos); /* trigger adc current sensing */
#endif
}

/**
  * @brief  update pwm duty in overflow
  * @param  none
  * @retval none
  */
void pwm_duty_update(void)
{
  tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_1, pwm_duty.UF.a);
#ifdef HALL_SENSORS
  if (hall_learn.dir == 1)
  {
    tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_2, pwm_duty.UF.c);
    tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_3, pwm_duty.UF.b);
  }
  else
  {
    tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_2, pwm_duty.UF.b);
    tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_3, pwm_duty.UF.c);
  }
#else
  tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_2, pwm_duty.UF.b);
  tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_3, pwm_duty.UF.c);
#endif
}

/**
  * @brief  update pwm duty in underflow
  * @param  none
  * @retval none
  */
void pwm_duty_extra_update(void)
{
  tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_1, pwm_duty.OF.a);
#ifdef HALL_SENSORS
  if (hall_learn.dir == 1)
  {
    tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_2, pwm_duty.OF.c);
    tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_3, pwm_duty.OF.b);
  }
  else
  {
    tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_2, pwm_duty.OF.b);
    tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_3, pwm_duty.OF.c);
  }
#else
  tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_2, pwm_duty.OF.b);
  tmr_channel_value_set(PWM_ADVANCE_TIMER, TMR_SELECT_CHANNEL_3, pwm_duty.OF.c);
#endif
}

#if defined HALL_SENSORS && defined LOW_SPEED_VOLT_CTRL
/**
  * @brief  set pwm output mode to normal pwm control mode
  * @param  none
  * @retval none
  */
void set_normal_pwm_mode(void)
{
  /* clear pwm timer PWM control mode */
  PWM_ADVANCE_TIMER->cm1 &= (~TMR_PWM_MODE_CM1_MASK);
  PWM_ADVANCE_TIMER->cm2 &= (~TMR_PWM_MODE_CM2_MASK);
  /* set pwm timer PWM control mode */
  PWM_ADVANCE_TIMER->cm1 |= TMR_PWM_1PWMA_2PWMA;
  PWM_ADVANCE_TIMER->cm2 |= TMR_PWM_3PWMA;

  /* change tmr channel mode */
  PWM_ADVANCE_TIMER->swevt |= TMR_HALL_SWTRIG;
}

/**
  * @brief  set pwm output mode to turn on the low side power switches of three phase
  * @param  none
  * @retval none
  */
void lock_rotor(void)
{
  /* clear pwm timer PWM control mode */
  PWM_ADVANCE_TIMER->cm1 &= (~TMR_PWM_MODE_CM1_MASK);
  PWM_ADVANCE_TIMER->cm2 &= (~TMR_PWM_MODE_CM2_MASK);
  /* set pwm timer PWM control mode */
  PWM_ADVANCE_TIMER->cm1 |= TMR_PWM_1LOW_2LOW;
  PWM_ADVANCE_TIMER->cm2 |= TMR_PWM_3LOW;

  /* change tmr channel mode */
  PWM_ADVANCE_TIMER->swevt |= TMR_HALL_SWTRIG;

  hall.theta_inc = 0;
  rotor_speed_hall.filtered = 0;
  reset_ma_buffer(hall_interval_moving_average);
  hall.offset = 0;
  speed_ramp.cmd_final = 0;
  speed_ramp.command = 0;
  pid_spd.integral = 0;
  pid_spd_volt.integral = 0;
  pid_iq.integral = 0;
  pid_id.integral = 0;
  error_code |= error_code_mask & hall_at_zero_speed(&hall);
}
#endif
