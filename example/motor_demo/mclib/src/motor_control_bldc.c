/**
  **************************************************************************
  * @file     motor_control_bldc.c
  * @brief    motor control related function for BLDC
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
  pid_is.integral = 0;
  pid_spd.integral = 0;
#ifdef LOW_SPEED_VOLT_CTRL
  pid_spd_volt.integral = 0;
#endif
  speed_ramp.cmd_final = 0;
  speed_ramp.command = 0;
  current.Ibus.Iref = 0;

  reset_ma_buffer(interval_moving_average_fliter);

  volt_cmd = 0;

  hall.pre_state = 0;

  /* open loop parameter initial*/
  openloop.olc_count = 0;

  openloop.olc_init_period = (int16_t)(10.0 * PWM_FREQ / openloop.olc_init_spd / POLE_PAIRS);
  openloop.olc_final_period = (int16_t)(10.0 * PWM_FREQ / openloop.olc_final_spd / POLE_PAIRS);

  if(openloop.olc_times != 0)
  {
    openloop.olc_period_dec = (int16_t)((openloop.olc_init_period - openloop.olc_final_period) / openloop.olc_times);
  }
  else
  {
    openloop.olc_period_dec = 0;
  }

  openloop.period_ref = openloop.olc_init_period;
  openloop.volt_ref = openloop.olc_init_volt;
}

/**
  * @brief  initialization of motor control parameters
  * @param  none
  * @retval none
  */
void param_init(void)
{
  hall.offset = 0xFFFF;
  interval_moving_average_fliter = moving_average(SPEED_FILTER_TIMES);
  if(hall_learn.check_flag == SET)
  {
    hall_to_tmr_register_setting();
  }
  else
  {
    hall_learn_register_setting();
  }
  ui_wave_param.sample_cycle = (uint8_t)(((DATA_BUFFER_FRAME_SIZE * 10.0 / UI_UART_BAUDRATE) + 0.002) * PWM_FREQ / (DATA_BUFFER_SIZE / 4)) + 1;
}

/**
  * @brief  PWM control table of TMRx_CM1,TMRx_CM2 register in different Hall state
  * @param  none
  * @retval none
  */
void hall_to_tmr_register_setting(void)
{
  uint16_t table[7] = {0};
  for(uint8_t i=0; i<7 ; i++)
  {
    table[i] = hall_learn_state_table[i%6];
  }

  for(int16_t i=1; i<6; i++)
  {
    next_hall_state[0][table[i]] = table[i+1];
    next_hall_state[1][table[i]] = table[i-1];
  }
  next_hall_state[0][table[0]] = table[1];
  next_hall_state[1][table[0]] = table[5];

  tmr_pwm_channel_mode[table[pwm_pattern[hall_learn.dir][1]]][0] = BH_CL_PWM_MODE_CM1;
  tmr_pwm_channel_mode[table[pwm_pattern[hall_learn.dir][1]]][1] = BH_CL_PWM_MODE_CM2;
  tmr_pwm_output_mode[table[pwm_pattern[hall_learn.dir][1]]] = BH_CL_PWM_OUT_CCTRL;

  tmr_pwm_channel_mode[table[pwm_pattern[hall_learn.dir][2]]][0] = BH_AL_PWM_MODE_CM1;
  tmr_pwm_channel_mode[table[pwm_pattern[hall_learn.dir][2]]][1] = BH_AL_PWM_MODE_CM2;
  tmr_pwm_output_mode[table[pwm_pattern[hall_learn.dir][2]]] = BH_AL_PWM_OUT_CCTRL;

  tmr_pwm_channel_mode[table[pwm_pattern[hall_learn.dir][3]]][0] = CH_AL_PWM_MODE_CM1;
  tmr_pwm_channel_mode[table[pwm_pattern[hall_learn.dir][3]]][1] = CH_AL_PWM_MODE_CM2;
  tmr_pwm_output_mode[table[pwm_pattern[hall_learn.dir][3]]] = CH_AL_PWM_OUT_CCTRL;

  tmr_pwm_channel_mode[table[pwm_pattern[hall_learn.dir][4]]][0] = CH_BL_PWM_MODE_CM1;
  tmr_pwm_channel_mode[table[pwm_pattern[hall_learn.dir][4]]][1] = CH_BL_PWM_MODE_CM2;
  tmr_pwm_output_mode[table[pwm_pattern[hall_learn.dir][4]]] = CH_BL_PWM_OUT_CCTRL;

  tmr_pwm_channel_mode[table[pwm_pattern[hall_learn.dir][5]]][0] = AH_BL_PWM_MODE_CM1;
  tmr_pwm_channel_mode[table[pwm_pattern[hall_learn.dir][5]]][1] = AH_BL_PWM_MODE_CM2;
  tmr_pwm_output_mode[table[pwm_pattern[hall_learn.dir][5]]] = AH_BL_PWM_OUT_CCTRL;

  tmr_pwm_channel_mode[table[pwm_pattern[hall_learn.dir][6]]][0] = AH_CL_PWM_MODE_CM1;
  tmr_pwm_channel_mode[table[pwm_pattern[hall_learn.dir][6]]][1] = AH_CL_PWM_MODE_CM2;
  tmr_pwm_output_mode[table[pwm_pattern[hall_learn.dir][6]]] = AH_CL_PWM_OUT_CCTRL;
}

/**
  * @brief  initial PWM control table of TMRx_CM1,TMRx_CM2 register in hall learning mode
  * @param  none
  * @retval none
  */
void hall_learn_register_setting(void)
{
  for(int16_t i = 0; i<7; i++)
  {
    tmr_pwm_channel_mode[i][0] = init_tmr_pwm_channel_mode[i][0];
    tmr_pwm_channel_mode[i][1] = init_tmr_pwm_channel_mode[i][1];
    tmr_pwm_output_mode[i] = init_tmr_pwm_output_mode[i];
    next_hall_state[0][i] = init_next_hall_state[0][i];
    next_hall_state[1][i] = init_next_hall_state[1][i];
  }
}

/**
* @brief  disable_mosfet
* @param  tmr_x: select the tmr peripheral.
  *       The timer is used for output PWM.
* @retval none
*/
void disable_mosfet(tmr_type *tmr_x)
{
  /* disable pwm timer output */
  tmr_output_enable(tmr_x, FALSE);

  /* clear pwm timer PWM control mode */
  tmr_x->cm1 &= (~TMR_PWM_MODE_CM1_MASK);
  tmr_x->cm2 &= (~TMR_PWM_MODE_CM2_MASK);

  /* update pwm output mode from shadow buffer of timer cctrl reg. */
  tmr_event_sw_trigger(tmr_x, TMR_HALL_SWTRIG);

  /* set channel dt */
  tmr_x->c1dt = 0;
  tmr_x->c2dt = 0;
  tmr_x->c3dt = 0;
  tmr_x->c4dt = 0;
}

/**
  * @brief  current offset initialize function
  * @param  none
  * @retval none
  */
void I_offset_init(void)
{
  current_offset_tmr_setting(PWM_ADVANCE_TIMER);
  curr_offset_rdy = current_offset_init(&current, ONESHUNT);
  adc_flag_clear(ADC_CONVERTER, ADC_PCCE_FLAG);
  /* Enable current sensing interrupt */
  adc_interrupt_enable(ADC_CONVERTER, ADC_PCCE_INT, TRUE);
}


/**
  * @brief  current mamual tuning function
  * @param  none
  * @retval none
  */
void I_tune_manual(void)
{
  /* Ibus pulse */
  I_tune_count++;

  if ((I_tune_count % current_tune_total_period) == 0)
  {
    I_tune_count = 0;
  }

  if (I_tune_count < current_tune_step_period)
  {
    current.Ibus.Iref = current_tune_target_current;

    if(current.Ibus.Iref > 0)
    {
      current.volt_sign = 1;
      pid_is.lower_limit_output = 0;
      pid_is.upper_limit_output = INT16_MAX;
    }
    else if(current.Ibus.Iref < 0)
    {
      current.volt_sign = -1;
      pid_is.lower_limit_output = -INT16_MAX;
      pid_is.upper_limit_output = 0;
    }
  }
  else
  {
    current.Ibus.Iref = 0;
    pid_is.integral = 0;
    volt_cmd = 0;
  }
}

/**
  * @brief  hall control parameters cofiguration(dir:CW)
  * @param  none
  * @retval none
  */
void hall_cw_ctrl_para(void)
{
#if defined WITHOUT_CURRENT_CTRL
  pid_spd.upper_limit_integral = (INT16_MAX << pid_spd.ki_shift);
  pid_spd.lower_limit_integral = 0;

  pid_spd.upper_limit_output = INT16_MAX;
  pid_spd.lower_limit_output = 0;
#else
  pid_is.upper_limit_integral = (INT16_MAX << pid_is.ki_shift);
  pid_is.lower_limit_integral = 0;

  pid_is.upper_limit_output = INT16_MAX;
  pid_is.lower_limit_output = 0;
#endif

#if defined LOW_SPEED_VOLT_CTRL || defined WITHOUT_CURRENT_CTRL
  pid_spd_volt.upper_limit_integral = INT16_MAX << pid_spd_volt.ki_shift;
  pid_spd_volt.lower_limit_integral = 0;
  pid_spd_volt.upper_limit_output = INT16_MAX;
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
#if defined WITHOUT_CURRENT_CTRL
  pid_spd.upper_limit_integral = 0;
  pid_spd.lower_limit_integral = -(INT16_MAX << pid_spd.ki_shift);

  pid_spd.upper_limit_output = 0;
  pid_spd.lower_limit_output = -INT16_MAX;
#else
  pid_is.upper_limit_integral = 0;
  pid_is.lower_limit_integral = -(INT16_MAX << pid_is.ki_shift);

  pid_is.upper_limit_output = 0;
  pid_is.lower_limit_output = -INT16_MAX;
#endif

#if defined LOW_SPEED_VOLT_CTRL || defined WITHOUT_CURRENT_CTRL
  pid_spd_volt.upper_limit_integral = 0;
  pid_spd_volt.lower_limit_integral = -(INT16_MAX << pid_spd_volt.ki_shift);
  pid_spd_volt.upper_limit_output = 0;
  pid_spd_volt.lower_limit_output = -INT16_MAX;
#endif
}

/**
  * @brief  update pwm duty in overflow
  * @param  none
  * @retval none
  */
void pwm_duty_update(void)
{
  PWM_ADVANCE_TIMER->c1dt = pwm_comp_value;
  PWM_ADVANCE_TIMER->c2dt = pwm_comp_value;
  PWM_ADVANCE_TIMER->c3dt = pwm_comp_value;
}

/**
  * @brief  Set ADC sample point
  * @param  adc_sample : ADC sampling related structure variables
  * @retval none
  */
void set_adc_sample_point(adc_sample_type *adc_sample)
{
#if defined HALL_SENSORS
  /* current sample point */
  PWM_ADVANCE_TIMER->c4dt = adc_sample->current_sampling_point;
#else
  adc_sample->adc_sample_page ^= 0x01;
  adc_sample->adc_sample_point[adc_sample->adc_sample_page][0] = adc_sample->current_sampling_point;
  adc_sample->adc_sample_point[adc_sample->adc_sample_page][1] = adc_sample->emf_sampling_point;
#endif

#if defined EMF_CONTINOUS_SAMPLE
  /* current sample point */
  PWM_ADVANCE_TIMER->c4dt = adc_sample->current_sampling_point;

  READ_EMF_TIMER->c3dt = pwm_comp_value + GATE_DELAY_COUNT + EMF_SIG_FALLING_COUNT;
  READ_EMF_TIMER->c4dt = adc_sample->current_sampling_point + GATE_DELAY_COUNT;

  lowspd_sample_end = EMF_LOW_SPD_CONT_SAMPLE_END;
  highspd_sample_end = pwm_comp_value - EMF_SAMPLE_INTERVAL - SENSE_GPIN_DELAY;
#endif
}

/**
  * @brief  Calculate motor speed function
  * @param  rotor_speed : Speed-related structure variables
  * @retval none
  */
void calc_motor_speed(speed_type *rotor_speed)
{
  if(rotor_speed->speed_count < MAX_SPD_CNT)
  {
    rotor_speed->speed_count++;

    if (rotor_speed->speed_count > rotor_speed->val_temp)
    {
      rotor_speed->interval_filter.long_word = (rotor_speed->speed_count) * PWM_PERIOD;

      /* set flag to calculate motor speed in main function */
      calc_spd_rdy = SET;
    }
  }
  else
  {
    rotor_speed->speed_count = MAX_SPD_CNT;
    rotor_speed->val_temp = MAX_SPD_CNT;
    rotor_speed->filtered = 0;
    reset_ma_buffer(interval_moving_average_fliter);
  }
}
int16_t ramp_cnt = 0;
/**
  * @brief  Open-loop control command ramp
  * @param  openloop_handler : Open-loop related structure variables
  * @retval none
  */
void open_loop_cmd_ramp(olc_type *openloop_handler)
{
  int16_t period_err;

  if(ramp_cnt % 5 == 0)
  {
    period_err = openloop_handler->period_ref - openloop_handler->olc_final_period;

    if (period_err > 0)
    {
      if (period_err < openloop_handler->olc_period_dec)
      {
        openloop_handler->period_ref = openloop_handler->olc_final_period;
      }
      else
      {
        openloop_handler->period_ref -= openloop_handler->olc_period_dec;
      }
      openloop_handler->volt_ref += openloop_handler->olc_volt_inc;
    }
    ramp_cnt = 0;
  }
  ramp_cnt++;
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
