/**
  **************************************************************************
  * @file     mc_bldc_sensorless.c
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
#include "mc_lib.h"
const int16_t is_emf_rise_or_fall[7] = {0, -1, -1, 1, -1, 1, 1};

#if defined SPECIFIC_EACH_EMF_PIN
const gpio_type *read_gpio_port_table[7] = {NULL, HALL_B_PORT, HALL_A_PORT, HALL_C_PORT, HALL_C_PORT, HALL_A_PORT, HALL_B_PORT};
const int16_t read_gpio_pin_table[7] = {-1, HALL_B_GPIO_PIN, HALL_A_GPIO_PIN, HALL_C_GPIO_PIN, HALL_C_GPIO_PIN, HALL_A_GPIO_PIN, HALL_B_GPIO_PIN};
#else
const int16_t read_gpio_table[7] = {-1, 1, 0, 2, 2, 0, 1};
#endif

const int16_t emf_comp_hall_change_state[7] = {-1, 0, 0, 1, 0, 1, 1};
const adc_channel_select_type emf_detect_adc_channel[7] =
{
  BEMF_B_ADC_CH, /*<! 0 */
  BEMF_B_ADC_CH,
  BEMF_A_ADC_CH,
  BEMF_C_ADC_CH,
  BEMF_C_ADC_CH,
  BEMF_A_ADC_CH,
  BEMF_B_ADC_CH
};

int16_t *ZeroCrossPointMapping[7][2] =
{
  {0, 0},
  {&(zcp_lowspd_fall), &(zcp_highspd_fall)},
  {&(zcp_lowspd_fall), &(zcp_highspd_fall)},
  {&(zcp_lowspd_rise), &(zcp_highspd_rise)},
  {&(zcp_lowspd_fall), &(zcp_highspd_fall)},
  {&(zcp_lowspd_rise), &(zcp_highspd_rise)},
  {&(zcp_lowspd_rise), &(zcp_highspd_rise)}
};

/**
* @brief  Configuration of motor start-up
* @param  none
* @retval none
*/
void start_up_config(void)
{
#if defined SENSORLESS
#if defined INIT_ANGLE_STARTUP     /* method 1: Initial angle detection */
  bldc_angle_init_config(&angle_init, &adc_sample);
#elif defined ALIGN_AND_GO_STARTUP /* method 2:  fixed alpha axis and force to beta axis */
  hall.state = HALL_LEARN_4_STATE;
  align_bldc();
#elif defined OPENLOOP_STARTUP     /* method 3: open loop start-up */
  hall.state = HALL_LEARN_4_STATE;
  bldc_rdy = SET;
  /* clear tmr counter */
  tmr_counter_value_set(ADC_TIMER, 0);
  tmr_counter_value_set(PWM_ADVANCE_TIMER, 0);
  /* set pwm output mode in shadow buffer of timer cctrl reg. */
  bldc_output_config(output_hall_state[rotor_speed.dir][hall.state]);
  /* update pwm output mode from shadow buffer of timer cctrl reg. */
  tmr_event_sw_trigger(PWM_ADVANCE_TIMER, TMR_HALL_SWTRIG);
  /* enable pwm timer */
  tmr_counter_enable(PWM_ADVANCE_TIMER, TRUE);
  /* output enable */
  tmr_output_enable(PWM_ADVANCE_TIMER, TRUE);
#endif
#endif
}
/**
  * @brief  Set next hall state for detecting
  * @param  none
  * @retval none
  */
void set_next_detect_hall_state(void)
{
#if defined INIT_ANGLE_STARTUP
  crm_periph_clock_enable(CRM_TMR1_PERIPH_CLOCK,FALSE);
  crm_periph_reset(CRM_TMR1_PERIPH_RESET,TRUE);
  mc_delay_us(50);
  crm_periph_reset(CRM_TMR1_PERIPH_RESET,FALSE);
  /* enable pwm timer/high side gpio/low side gpio clock */
  tmr_pwm_init();
#elif defined ALIGN_AND_GO_STARTUP
  /* change to next hall state */
  if(rotor_speed.dir == CW)
  {
    hall.state = next_hall_state[0][hall.state];
  }
  else
  {
    hall.state = next_hall_state[0][hall.state];
    hall.state = next_hall_state[0][hall.state];
  }
#elif defined OPENLOOP_STARTUP
  /* change to next hall state */
  hall.state = next_hall_state[rotor_speed.dir][hall.state];
  hall.state = next_hall_state[rotor_speed.dir][hall.state];
#endif
  rotor_speed.speed_count = 0;
}

/**
  * @brief  EMF detect initialize function
  * @param  none
  * @retval none
  */
void bldc_detectEMF_param_init(adc_sample_type *adc_sample)
{
  /* set first step state */
  start_state = START_STATE_FIRST_STEP;
  sense_hall_steps = 0;
  EMF_switch_sample_position = 0;
  emf_avoid_noise_counter = 0;
  rotor_speed.val = EMF_AVOID_NOISE_INIT_TIMES;
  rotor_speed.speed_count = 0;
  current.Ibus.Ireal_pu = 0;
  current.Ibus.Icalc = 0;

#if defined (BLDC_SENSORLESS_ADC) && defined (EMF_PULL_UP)
  adc_sample->emf.emf_pull_up = SET;
#elif defined (BLDC_SENSORLESS_COMP) && defined (EMF_CONTINOUS_SAMPLE)
  adc_sample->emf.emf_comp_continous_mode = SET;
#endif

  /* set adc timer CC4 cval */
  tmr_channel_value_set(adc_sample->adc_tmr_x, TMR_SELECT_CHANNEL_4, adc_sample->adc_sample_point[0][2]);

  /* set sample point */
  adc_sample->current_sampling_point = adc_sample->i_sample_min_cnt + adc_sample->i_sample_delay_count;
  adc_sample->emf_sampling_point = adc_sample->emf.emf_low_spd_sample_point;
  adc_sample->adc_sample_point[0][0] = adc_sample->current_sampling_point;     /* current ADC sample point */
  adc_sample->adc_sample_point[0][1] = adc_sample->emf_sampling_point;         /* BEMF ADC sample point */
  adc_sample->adc_sample_page = 0;

  /* To avoid EMF noise of first step */
  adc_sample->emf.emf_avoid_noise_times = EMF_AVOID_NOISE_INIT_TIMES;
}

/**
  * @brief  Determine the initial rotor position at the beginning
  * @param  none
  * @retval none
  */
void angle_init_func(void)
{
#if defined SENSORLESS && defined INIT_ANGLE_STARTUP

  if(angle_init.step_count < STEP_8_MAX)
  {
    bldc_sensorless_angle_init(&angle_init);
    angle_init.step_count++;
    esc_state = ESC_STATE_ANGLE_INIT;
  }
  else
  {
    hall.state = angle_init_estimation(&angle_init);
#if defined LARGE_COGGING
    hall.state = next_hall_state[1][hall.state];  /* find previous hall state */
#endif
    esc_state = ESC_STATE_STARTING;
  }

#elif defined SENSORLESS && defined ALIGN_AND_GO_STARTUP

  if(sys_counter < LOCK_PERIOD)
  {
    sys_counter++;
  }
  else
  {
    disable_mosfet(PWM_ADVANCE_TIMER);
    esc_state = ESC_STATE_STARTING;
  }

#elif defined SENSORLESS && defined OPENLOOP_STARTUP
  /* 5 ms inc openloop volt & spd */
  open_loop_cmd_ramp(&openloop);
  if(sys_counter < OLC_STARTUP_PERIOD)
  {
    sys_counter++;
  }
  else
  {
    disable_mosfet(PWM_ADVANCE_TIMER);
    esc_state = ESC_STATE_STARTING;
  }
#endif
}

/**
  * @brief  Set start-up voltage command
  * @param  none
  * @retval none
  */
void set_starting_volt(void)
{
#if defined CONST_CURRENT_START
  current.Ibus.Istart = start_current_cmd * current.volt_sign;
  const_current_ctrl = SET;
  current_loop_ctrl = SET;
  pid_is.integral = volt_cmd << (pid_is.ki_shift);
#elif defined CONST_VOLTAGE_START && (defined OPENLOOP_STARTUP)
  volt_cmd = openloop.volt_ref * current.volt_sign;
  current_loop_ctrl = RESET;
#elif defined CONST_VOLTAGE_START && (defined ALIGN_AND_GO_STARTUP || defined INIT_ANGLE_STARTUP)
  volt_cmd = start_volt_cmd * current.volt_sign;
  current_loop_ctrl = RESET;
#endif
}

/**
  * @brief  Set parameter for switching to closed-loop control
  * @param  none
  * @retval none
  */
void rdy_to_close_loop_param(void)
{
#if defined OPENLOOP_STARTUP
  pid_spd.integral = current.Ibus.Ireal_pu << (pid_spd.ki_shift);
  pid_is.integral = (openloop.volt_ref * current.volt_sign) << pid_is.ki_shift;
  pid_spd_volt.integral = (openloop.volt_ref * current.volt_sign) << pid_spd_volt.ki_shift;
  speed_ramp.command = openloop.olc_final_spd * current.volt_sign;
#else
#if defined CONST_CURRENT_START
  pid_is.integral = volt_cmd << pid_is.ki_shift;
  pid_spd_volt.integral = volt_cmd << pid_spd_volt.ki_shift;
#else
  pid_is.integral = volt_cmd << pid_is.ki_shift;
  pid_spd_volt.integral = volt_cmd << pid_spd_volt.ki_shift;
#endif
  pid_spd.integral = current.Ibus.Ireal_pu << pid_spd.ki_shift;
  speed_ramp.command = rotor_speed.filtered;
#endif
}

/**
  * @brief  Detect_zero_cross_point
  * @param  none
  * @retval none
  */
void detect_zero_cross_point(void)
{
  if (rotor_speed.speed_count > rotor_speed.val)
  {
#if defined BLDC_SENSORLESS_COMP
    if(is_find_zero_cross_COMP(&hall, &adc_sample))
#elif defined BLDC_SENSORLESS_ADC
    if(is_find_zero_cross_ADC(&hall, &adc_sample))
#endif
    {
      bldc_sensorless_change_phase(&hall, &adc_sample, &rotor_speed);
    }
  }
  /* To avoid PWM switch noise */
  emf_avoid_noise_counter++;
}

/**
  * @brief  Find zero crossing of the BEMF by using comparator input
  * @param  hall_handler : Hall sensor related structure variables
  * @param  adc_sample : ADC sampling related structure variables
  * @retval state of finding zero crossing flag
  */
flag_status is_find_zero_cross_COMP(hall_sensor_type *hall_handler, adc_sample_type *adc_sample)
{
#if defined SPECIFIC_EACH_EMF_PIN
  gpio_type *port_num;
  int16_t  pin_num, pre_pin_state, pin_state;

  if(emf_avoid_noise_counter > adc_sample->emf.emf_avoid_noise_times)
  {
    port_num = (gpio_type *) read_gpio_port_table[hall_handler->state];
    pin_num = read_gpio_pin_table[hall_handler->state];

    pre_pin_state = gpio_input_data_bit_read(port_num, pin_num);
    pin_state = gpio_input_data_bit_read(port_num, pin_num);

    while(pre_pin_state != pin_state)
    {
      pre_pin_state = pin_state;
      pin_state = gpio_input_data_bit_read(port_num, pin_num);
    }

    if(pin_state == emf_comp_hall_change_state[hall_handler->state])
    {
      return (SET);
    }
  }
  return (RESET);
#else
  emf_comp_hall[0] = gpio_input_data_bit_read(hall_handler->H1_port, hall_handler->H1_pin);
  emf_comp_hall[1] = gpio_input_data_bit_read(hall_handler->H2_port, hall_handler->H2_pin);
  emf_comp_hall[2] = gpio_input_data_bit_read(hall_handler->H3_port, hall_handler->H3_pin);

  if(emf_avoid_noise_counter > adc_sample->emf.emf_avoid_noise_times)
  {
    read_gpio_num = read_gpio_table[hall_handler->state];
    emf_comp_state = emf_comp_hall[read_gpio_num];

    if(emf_comp_state == emf_comp_hall_change_state[hall_handler->state])
    {
      return (SET);
    }
  }
  return (RESET);
#endif
}

/**
  * @brief  Find zero crossing of the BEMF by using the Analog-to-Digital Converter (ADC)
  * @param  hall_handler : Hall sensor related structure variables
  * @param  adc_sample : ADC sampling related structure variables
  * @retval state of finding zero crossing flag
  */
flag_status is_find_zero_cross_ADC(hall_sensor_type *hall_handler, adc_sample_type *adc_sample)
{
  if(hall_handler->state != hall_handler->pre_state)
  {
    adc_preempt_channel_set(adc_sample->adc_x, emf_detect_adc_channel[hall_handler->state], 3, ADC_SAMPLETIME_7_5);
    hall_handler->pre_state = hall_handler->state;
  }

  if(emf_avoid_noise_counter >= adc_sample->emf.emf_avoid_noise_times)
  {
    zero_cross_point = *(ZeroCrossPointMapping[hall_handler->state][EMF_switch_sample_position]) * is_emf_rise_or_fall[hall_handler->state];
    emf_adc_value = adc_preempt_conversion_data_get(adc_sample->adc_x, ADC_PREEMPT_CHANNEL_3) * is_emf_rise_or_fall[hall_handler->state];

    if(emf_adc_value >= zero_cross_point && hall_handler->state != 0)
    {
      return (SET);
    }
  }

  return (RESET);
}

/**
  * @brief  Calculate commutation point and rotor speed
  * @param  hall_handler : Hall sensor related structure variables
  * @param  adc_sample : ADC sampling related structure variables
  * @param  rotor_speed : Speed-related structure variables
  * @retval none
  */
void bldc_sensorless_change_phase(hall_sensor_type *hall_handler, adc_sample_type *adc_sample, speed_type *rotor_speed)
{
  long_words_union emf_zcp_avg_interval_count;

  /* Set change phase flag */
  change_phase_flag = SET;

  if(ctrl_mode != OPEN_LOOP_CTRL)
  {
    /* Speed estimation */
    spd_cval = PWM_ADVANCE_TIMER->cval ;
    spd_total_cval = (rotor_speed->speed_count - 1) * PWM_PERIOD + spd_cval + (PWM_PERIOD - spd_last_cval);
    spd_last_cval = spd_cval;
    rotor_speed->speed_count = 0;
  }

  if(closeloop_rdy != RESET)
  {
    /* Set next state */
    hall_handler->next_state = next_hall_state[rotor_speed->dir][hall_handler->state];

    /* Change mode **/
    bldc_output_config(output_hall_state[rotor_speed->dir][hall_handler->next_state]);
  }

  emf_zcp_avg_interval_count.long_word = moving_average_update(interval_moving_average_fliter, spd_total_cval);
  rotor_speed->interval_filter.long_word = emf_zcp_avg_interval_count.long_word;

  /* clear tmr counter */
  tmr_counter_value_set(adc_sample->change_phase_tmr_x, 0);

#if defined PHASE_ADVANCE
  emf_zcp_avg_interval_count.long_word = rotor_speed->interval_filter.long_word - EMF_PHASE_ADV_SPD_PERIOD;

  if(emf_zcp_avg_interval_count.long_word < EMF_MIN_DELAY)
  {
    emf_zcp_avg_interval_count.long_word = EMF_MIN_DELAY;
  }
#endif

  /* disable change phase timer overflow interrupt */
  adc_sample->change_phase_tmr_x->iden_bit.ovfien = FALSE;

  /* set change phase timer period */
  adc_sample->change_phase_tmr_x->pr = (uint16_t)((((uint32_t)emf_zcp_avg_interval_count.long_word) >> 1) / (emf_zcp_avg_interval_count.high_word + 1));
  adc_sample->change_phase_tmr_x->div = emf_zcp_avg_interval_count.high_word;

  /* trigger the overflow event to immediately reload pr value and div value */
  adc_sample->change_phase_tmr_x->swevt_bit.ovfswtr = TRUE;
  adc_sample->change_phase_tmr_x->ists_bit.ovfif = FALSE;
  adc_sample->change_phase_tmr_x->iden_bit.ovfien = TRUE;

  /* Enable change phase timer */
  tmr_counter_enable(adc_sample->change_phase_tmr_x, TRUE);
}

