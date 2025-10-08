/**
  **************************************************************************
  * @file     mc_bldc.c
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

/**
* @brief  configuration of pwm timer outputs
* @param  hall_states : hall state
* @retval none
*/
void bldc_output_config(uint8_t hall_states)
{
  uint16_t temp;

  temp = PWM_ADVANCE_TIMER->cm1;
  temp &= (~TMR_PWM_MODE_CM1_MASK);
  temp |= tmr_pwm_channel_mode[hall_states][0];
  PWM_ADVANCE_TIMER->cm1 = temp;

  temp = PWM_ADVANCE_TIMER->cm2;
  temp &= (~TMR_PWM_MODE_CM2_MASK);
  temp |= tmr_pwm_channel_mode[hall_states][1];
  PWM_ADVANCE_TIMER->cm2 = temp;

  temp = PWM_ADVANCE_TIMER->cctrl;
  temp &= (~TMR_PWM_OUT_MODE_MASK);
  temp |= tmr_pwm_output_mode[hall_states];
  PWM_ADVANCE_TIMER->cctrl = temp;
}

/**
  * @brief  start bldc setting
  * @param  none
  * @retval none
  */
void start_bldc(void)
{
#if defined HALL_SENSORS
  error_code |= error_code_mask & read_hall_state(&hall);

  /* clear interrupt flags of hall timer */
  tmr_flag_clear(HALL_CAPTURE_TIMER, TMR_OVF_FLAG | TMR_TRIGGER_FLAG | TMR_HALL_FLAG);

  /* set dir: CW or CCW */
  if(ctrl_source == CTRL_SOURCE_EXTERNAL)
  {
    rotor_speed.dir = gpio_input_data_bit_read(MODE1_BUTTON_PORT, MODE1_BUTTON_PIN);
  }
  else if(ctrl_source == CTRL_SOURCE_SOFTWARE)
  {
    if(ctrl_mode == SPEED_CTRL)
    {
      if(speed_ramp.cmd_final > 0)
      {
        rotor_speed.dir = CW;
      }
      else
      {
        rotor_speed.dir = CCW;
      }
    }
    else if(ctrl_mode == TORQUE_CTRL)
    {
      if(current.Ibus.Iref > 0)
      {
        rotor_speed.dir = CW;
      }
      else
      {
        rotor_speed.dir = CCW;
      }
    }
  }

  /* set first hall state output */
  bldc_output_config(output_hall_state[rotor_speed.dir][hall.state]);

  if(rotor_speed.dir == CW)
  {
    current.volt_sign = 1;
    current.volt_sign_coming = 1;
    hall_cw_ctrl_para();
  }
  else
  {
    current.volt_sign = -1;
    current.volt_sign_coming = -1;
    hall_ccw_ctrl_para();
  }

  /* update pwm output mode from shadow buffer of timer cctrl reg. */
  tmr_event_sw_trigger(PWM_ADVANCE_TIMER, TMR_HALL_SWTRIG);

  /* set hall pre_state */
  hall.pre_state = hall.state;

  /* find next hall state */
  hall.next_state = next_hall_state[rotor_speed.dir][hall.state];

  /* set pwm output mode in shadow buffer of timer cctrl reg. */
  bldc_output_config(output_hall_state[rotor_speed.dir][hall.next_state]);

  /* reset counter value */
  PWM_ADVANCE_TIMER->cval = 0;

  /* enable overflow and trigger interrup of hall timer */
  tmr_interrupt_enable(HALL_CAPTURE_TIMER, TMR_OVF_INT | TMR_TRIGGER_INT, TRUE);

  /* enable hall timer */
  tmr_counter_enable(HALL_CAPTURE_TIMER, TRUE);

  /* enable pwm timer output */
  tmr_output_enable(PWM_ADVANCE_TIMER, TRUE);

#elif defined SENSORLESS
  /* reset counter value */
  PWM_ADVANCE_TIMER->cval = 0;

  /* set pwm output mode in shadow buffer of timer cctrl reg. */
  bldc_output_config(output_hall_state[rotor_speed.dir][hall.state]);

  /* update pwm output mode from shadow buffer of timer cctrl reg. */
  tmr_event_sw_trigger(PWM_ADVANCE_TIMER, TMR_HALL_SWTRIG);

  /* enable pwm timer output */
  tmr_output_enable(PWM_ADVANCE_TIMER, TRUE);

  /* enable adc timer */
  tmr_counter_enable(ADC_TIMER, TRUE);

  /* enable pwm timer */
  tmr_counter_enable(PWM_ADVANCE_TIMER, TRUE);

#endif
}

/**
  * @brief  Rotor alignment (BLDC)
  * @param  none
  * @retval none
  */
void align_bldc(void)
{
  /* set pwm output mode in shadow buffer of timer cctrl reg. */
  bldc_output_config(hall.state);
  /* update pwm output mode from shadow buffer of timer cctrl reg. */
  tmr_event_sw_trigger(PWM_ADVANCE_TIMER, TMR_HALL_SWTRIG);
  /* set align volt */
  volt_cmd = LOCK_VOLT_CMD;
  pwm_comp_value = (volt_cmd * (PWM_PERIOD + 1)) >> 15;

  PWM_ADVANCE_TIMER->c1dt = pwm_comp_value;
  PWM_ADVANCE_TIMER->c2dt = pwm_comp_value;
  PWM_ADVANCE_TIMER->c3dt = pwm_comp_value;

  /* enable pwm timer */
  tmr_counter_enable(PWM_ADVANCE_TIMER, TRUE);

  /* enable pwm timer output */
  tmr_output_enable(PWM_ADVANCE_TIMER, TRUE);
}

/**
  * @brief  Open-loop control function (BLDC)
  * @param  hall_handler : Hall sensor related structure variables
  * @param  openloop_handler : Open-loop related structure variables
  * @param  rotor_speed : Speed-related structure variables
  * @retval none
  */
void bldc_open_loop_ctrl(hall_sensor_type *hall_handler, olc_type *openloop_handler, speed_type *rotor_speed)
{
  volt_cmd = openloop_handler->volt_ref * current.volt_sign;

  if(openloop_handler->olc_count > openloop_handler->period_ref)
  {
    /* Set next state */
    hall_handler->next_state = next_hall_state[rotor_speed->dir][hall_handler->state];

    /* Change mode **/
    bldc_output_config(output_hall_state[rotor_speed->dir][hall_handler->next_state]);

    /* Change phase trigger **/
    tmr_event_sw_trigger(PWM_ADVANCE_TIMER, TMR_HALL_SWTRIG);

    /* Renew hall state */
    hall_handler->state = hall_handler->next_state;

    /* Speed estimation */
    rotor_speed->interval_filter.long_word = openloop_handler->period_ref * PWM_PERIOD* (1 - 2 * rotor_speed->dir);

    /* set flag to calculate motor speed in main function */
    calc_spd_rdy = SET;

    openloop_handler->olc_count = 0;
    rotor_speed->speed_count = 0;
  }
  else
    openloop_handler->olc_count++;
}

/**
  * @brief  bldc hall auto learning function (BLDC)
  * @param  hall_handler : Hall sensor related structure variables
  * @param  openloop_handler : Open-loop related structure variables
  * @param  rotor_speed : Speed-related structure variables
  * @retval none
  */
void bldc_hall_learning(hall_sensor_type *hall_handler, hall_learn_type *hall_learn)
{
  if(hall_learn->count > hall_learn->learn_period)
  {
    /* Set next state */
    hall_handler->next_state = next_hall_state[hall_learn->dir][hall_handler->next_state];

    /* Change mode **/
    bldc_output_config(output_hall_state[hall_learn->dir][hall_handler->next_state]);

    /* update pwm output mode from shadow buffer of timer cctrl reg. */
    tmr_event_sw_trigger(PWM_ADVANCE_TIMER, TMR_HALL_SWTRIG);

    hall_learn->count = 0;
  }
  else
  {
    hall_learn->count++;
  }
}
