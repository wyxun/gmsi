/**
  **************************************************************************
  * @file     mc_hall.c
  * @brief    hall related functions
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
* @brief  read the states of three hall sensors
* @param  hall_handler: hall sensors related variables
* @retval error code (MC_NO_ERROR or MC_HALL_ERROR)
*/
err_code_type read_hall_state(hall_sensor_type *hall_handler)
{
  int16_t hall_a, hall_b, hall_c;
  err_code_type error_code_handler = MC_NO_ERROR;

  hall_a = gpio_input_data_bit_read(hall_handler->H1_port, hall_handler->H1_pin);
  hall_b = gpio_input_data_bit_read(hall_handler->H2_port, hall_handler->H2_pin);
  hall_c = gpio_input_data_bit_read(hall_handler->H3_port, hall_handler->H3_pin);

  hall_handler -> state = hall_c + (hall_b << 1) + (hall_a << 2);

  if(hall_handler -> state == 0 || hall_handler -> state == 7)
  {
    error_code_handler = MC_HALL_ERROR;
  }
  else
  {
    error_code_handler = MC_NO_ERROR;
  }

  return (error_code_handler);
}

#if defined SIX_STEP_CONTROL && defined HALL_SENSORS
/**
  * @brief  timer interrupt handler for hall signals capturing
  * @param  none
  * @retval none
  */
void hall_isr_handler(void)
{
  int16_t dir, expect_hall_state;
  static uint8_t power = 1;

  if (tmr_flag_get(HALL_CAPTURE_TIMER, TMR_TRIGGER_FLAG | TMR_C1_FLAG))
  {
    /* clear flags of trigger/ch1 and overflow events of hall timer */
    tmr_flag_clear(HALL_CAPTURE_TIMER, TMR_TRIGGER_FLAG | TMR_C1_FLAG | TMR_OVF_FLAG);

    if(closeloop_rdy != SET)
    {
      if(hall_learn.start_flag != RESET)
      {
        error_code |= error_code_mask & read_hall_state(&hall_learn.hall_state);

        if(hall_learn.hall_state.state != hall_learn.hall_state.pre_state)
        {
          if(hall_learn.step < HALL_LEARN_TABLE_LENGTH)
          {
            next_hall_learn_state_table[hall_learn.hall_state.pre_state] = hall_learn.hall_state.state;
            hall_learn_sequence_table[hall_learn.step] = hall_learn.hall_state.state;
            hall_learn.step ++;
          }
          else
          {
            if(hall_learn.hall_state.state == next_hall_learn_state_table[hall_learn.hall_state.pre_state])
            {
              hall_learn.step ++;
            }
            else
            {
              hall_learn.step = 1;
            }
          }
          hall_learn.hall_state.pre_state = hall_learn.hall_state.state;
        }
      }
    }
    else
    {
      /* update pwm output mode from shadow buffer of timer cctrl reg. */
      tmr_event_sw_trigger(PWM_ADVANCE_TIMER, TMR_HALL_SWTRIG);

      /* get hall state */
      error_code |= error_code_mask & read_hall_state(&hall);

      if(error_code != MC_HALL_ERROR)
      {
        /* get expected hall state */
        expect_hall_state = next_hall_state[rotor_speed.dir][hall.pre_state];

        if (hall.state == expect_hall_state)
        {
          hall.pre_state = hall.state;
          current.volt_sign = current.volt_sign_coming;
        }
        else
        {
          dir = rotor_speed.dir ^ 0x01;
          expect_hall_state = next_hall_state[dir][hall.pre_state];

          if (hall.state == expect_hall_state)
          {
            rotor_speed.dir = dir;
            hall.pre_state = hall.state;

            if (volt_cmd < 0) /* CW change to CCW*/
            {
              bldc_output_config((hall.state ^ 0x07));
              current.volt_sign = -1;
            }
            else              /* CCW change to CW*/
            {
              bldc_output_config(hall.state);
              current.volt_sign = 1;
            }

            /* update pwm output mode from shadow buffer of timer cctrl reg. */
            tmr_event_sw_trigger(PWM_ADVANCE_TIMER, TMR_HALL_SWTRIG);
          }
          else
          {
            hall.offset += hall.hall_interval;
            return;
          }
        }

        /* find next hall state */
        hall.next_state = next_hall_state[rotor_speed.dir][hall.state];

        /* set pwm output control mode in shadow buffer for next hall state */
        if (volt_cmd < 0 || (abs(rotor_speed.filtered) <= MIN_CONTROL_SPEED && volt_cmd == 0 && (speed_ramp.cmd_final < 0 || current.Ibus.Iref < 0)))
        {
          hall.next_state ^= 0x07;
          current.volt_sign_coming = -1;
        }
        else
        {
          current.volt_sign_coming = 1;
        }

        bldc_output_config(hall.next_state);

        /* read hall interval count */
        hall.hall_interval = tmr_channel_value_get(HALL_CAPTURE_TIMER, TMR_SELECT_CHANNEL_1);

        /* compensate hall interval with hall offset  */
        hall.hall_interval += hall.offset;
        hall.offset = 0;

        if (hall.hall_interval > MAX_CAP_COUNT)
        {
          hall.hall_interval = MAX_CAP_COUNT;
        }

        /* motor speed calculation  */
        rotor_speed.interval_filter.long_word = moving_average_update(interval_moving_average_fliter, hall.hall_interval);
        rotor_speed.filtered = (int16_t)(MIN_SPD_HALL_INR / rotor_speed.interval_filter.long_word) * (1 - 2 * rotor_speed.dir);

        /* to check motor slow down, set double hall interval to hall timer period */
        hall.double_interval = hall.hall_interval << 1;

        if (hall.double_interval > MAX_CAP_COUNT)
        {
          hall.double_interval = MAX_CAP_COUNT;
        }

        tmr_period_value_set(HALL_CAPTURE_TIMER, hall.double_interval);
      }
    }
  }
  else if (tmr_flag_get(HALL_CAPTURE_TIMER, TMR_OVF_FLAG))
  {
    /* clear overflow flag of hall timer */
    tmr_flag_clear(HALL_CAPTURE_TIMER, TMR_OVF_FLAG);

    if(closeloop_rdy == SET)
    {
      /* hall interval accumulation */
      hall.offset += hall.double_interval;

      if (hall.offset > MAX_CAP_COUNT)
      {
        hall.offset = MAX_CAP_COUNT;
      }

      /* set motor speed in half */
      rotor_speed.filtered >>= power;
      power++;

      /* negative speed process */
      if(abs(rotor_speed.filtered) <= MIN_CONTROL_SPEED)
      {
        rotor_speed.filtered = 0;
        current.volt_sign = current.volt_sign_coming;
        power = 1;
      }

      /* update hall timer period */
      hall.double_interval <<= power;

      if (hall.double_interval > MAX_CAP_COUNT)
      {
        hall.double_interval = MAX_CAP_COUNT;
      }

      tmr_period_value_set(HALL_CAPTURE_TIMER, hall.double_interval);
    }
  }
}
/**
  * @brief  hall learn process
  * @param  none
  * @retval none
  */
void hall_learn_process(void)
{
  static uint8_t start_index = 0;
  switch(hall_learn.process_state)
  {
  case PROCESS_0_LOCK:
    /* disable overflow and trigger interrup of hall timer */
    tmr_interrupt_enable(HALL_CAPTURE_TIMER, TMR_OVF_INT | TMR_TRIGGER_INT, FALSE);

    bldc_rdy = RESET;
    hall_learn.step = 0;
    hall.state = 0;
    align_bldc();
    if(sys_counter >= hall_learn.align_time)
    {
      hall_learn.process_state = PROCESS_1_FREE_RUN;
    }
    break;

  case PROCESS_1_FREE_RUN:
    /* get hall state */
    error_code |= error_code_mask & read_hall_state(&hall_learn.hall_state);
    hall_learn_state_table[0] = hall_learn.hall_state.state;
    hall_learn.hall_state.pre_state = hall_learn.hall_state.state;

    /* disable pwm output*/
    disable_mosfet(PWM_ADVANCE_TIMER);
    hall_learn.process_state = PROCESS_2_OPENLOOP;
    hall_learn.step = 0;
    break;

  case PROCESS_2_OPENLOOP:
    volt_cmd = hall_learn.learn_volt;
    /*hall state = OUTPUT_BH_CL_HALL_STATE;*/
    hall.state = 2;
    hall.next_state = 2;
    /* set pwm output mode in shadow buffer of timer cctrl reg. */
    bldc_output_config(output_hall_state[hall_learn.dir][hall.next_state]);
    /* update pwm output mode from shadow buffer of timer cctrl reg. */
    tmr_event_sw_trigger(PWM_ADVANCE_TIMER, TMR_HALL_SWTRIG);
    /* reset counter value */
    bldc_rdy = SET;
    /* clear PWM timer cval */
    PWM_ADVANCE_TIMER->cval = 0;
    /* clear interrupt flags of pwm timer */
    tmr_flag_clear(HALL_CAPTURE_TIMER, TMR_OVF_FLAG | TMR_TRIGGER_FLAG | TMR_HALL_FLAG);
    /* enable overflow and trigger interrup of hall timer */
    tmr_interrupt_enable(HALL_CAPTURE_TIMER, TMR_OVF_INT | TMR_TRIGGER_INT, TRUE);
    /* enable pwm timer output */
    tmr_output_enable(PWM_ADVANCE_TIMER, TRUE);
    hall_learn.process_state = PROCESS_3_LEARNING;
    break;

  case PROCESS_3_LEARNING:
    if(sys_counter >= LEARN_TIME)
    {
      if(hall_learn.step >= LEARN_CHECK_TIMES)
      {
        hall_learn.process_state = PROCESS_4_FINISH;
      }
      else
      {
        hall_learn.process_state = PROCESS_5_ERROR;
      }
      disable_mosfet(PWM_ADVANCE_TIMER);
    }
    break;

  case PROCESS_4_FINISH:

    for(uint8_t i=0; i< HALL_LEARN_TABLE_LENGTH; i++)
    {
      if(hall_learn_sequence_table[i] == hall_learn_state_table[0])
      {
        start_index = i;
      }
    }
    for(uint8_t i = 0; i < HALL_LEARN_TABLE_LENGTH; i++)
    {
      hall_learn_state_table[i] = hall_learn_sequence_table[(i+start_index)%6];
    }
    hall_learn.check_flag = SET;
    hall_learn.start_flag = RESET;
    esc_state = ESC_STATE_FREE_RUN;
    break;

  case PROCESS_5_ERROR:
    hall_learn.check_flag = RESET;
    error_code |= error_code_mask & MC_HALL_LEARN_ERROR;
    hall_learn.start_flag = RESET;
    esc_state = ESC_STATE_FREE_RUN;
    break;

  default:
    break;
  }
}
#endif

#if defined FOC_CONTROL && defined HALL_SENSORS
/**
  * @brief  timer interrupt handler for hall signals capturing
  * @param  none
  * @retval none
  */
void hall_isr_handler(void)
{
  uint8_t hall_state_temp;
  static uint8_t bit_shift = 1;

  if (tmr_flag_get(HALL_CAPTURE_TIMER, TMR_TRIGGER_FLAG | TMR_C1_FLAG) != RESET)
  {
    /* clear flags of trigger/ch1 and overflow events of hall timer */
    tmr_flag_clear(HALL_CAPTURE_TIMER, TMR_TRIGGER_FLAG | TMR_C1_FLAG | TMR_OVF_FLAG);

    if(hall_learn.start_flag != RESET)
    {
      /* get hall state */
      error_code |= error_code_mask & read_hall_state(&hall_learn.hall_state);

      if(hall_learn.hall_state.state != hall_learn.hall_state.pre_state)
      {
        if(hall_learn.step < HALL_LEARN_TABLE_LENGTH)
        {
          next_hall_learn_state_table[hall_learn.hall_state.pre_state] = hall_learn.hall_state.state;
          hall_learn_sequence_table[hall_learn.step] = hall_learn.hall_state.state;
          hall_learn.step ++;
        }
        else
        {
          if(hall_learn.hall_state.state == next_hall_learn_state_table[hall_learn.hall_state.pre_state])
          {
            hall_learn.step ++;
          }
          else
          {
            hall_learn.step = 1;
          }
        }
        hall_learn.hall_state.pre_state = hall_learn.hall_state.state;
      }
    }
    else
    {
      /* get hall state */
      error_code |= error_code_mask & read_hall_state(&hall);

      /* read hall interval count */
      hall.hall_interval = tmr_channel_value_get(HALL_CAPTURE_TIMER, TMR_SELECT_CHANNEL_1);

      /* read hall next state */
      hall.next_state = hall_next_state_table[hall.pre_state];

      if ((hall.state != hall.pre_state) && (hall.hall_interval > HALF_MAX_SPD_HALL_INR))
      {
        if (hall.state == hall.next_state)  // in same direction
        {
          /* compensate hall interval with hall offset  */
          hall.hall_interval += hall.offset;
          hall.offset = 0;

          if (hall.hall_interval > MAX_CAP_COUNT)
            hall.hall_interval = MAX_CAP_COUNT;

          if (abs(rotor_speed_hall.filtered) < MIN_SPEED_RPM_SHIFT)
          {
            reset_ma_buffer(hall_interval_moving_average);
          }

          /* rotor speed calculation  */
          hall.hall_interval_filt = moving_average_update(hall_interval_moving_average, hall.hall_interval);
          rotor_speed_hall.filtered = (int32_t) ( MIN_SPD_HALL_INR / hall.hall_interval_filt ) * rotor_speed_hall.dir;

          /* to check motor slow down, set double hall interval to hall timer period */
          hall.double_interval = hall.hall_interval << 1;
          if (hall.double_interval > MAX_CAP_COUNT)
          {
            hall.double_interval = MAX_CAP_COUNT;
          }

          tmr_period_value_set(HALL_CAPTURE_TIMER, hall.double_interval);

          /** Delta Theta Calculation */
          hall.theta_inc = hall_delta_theta_calculation(&rotor_speed_hall, &rotor_angle_hall, &hall);

          hall.pre_state = hall.state;    //update hall sensor state
        }
        else   //abnormal
        {
          if (hall.next_state > 0)  //in reverse direction
          {
            hall_state_temp = hall.state;

            /* get hall state */
            error_code |= error_code_mask & read_hall_state(&hall);

            if ((abs(rotor_speed_hall.filtered) < STABLE_SPEED_RPM_SHIFT) && (hall.state == hall_state_temp))
            {
              hall.theta_inc = 0;
              rotor_speed_hall.filtered = 0;
              hall.pre_state = hall.state;    //update hall sensor state
              rotor_angle_hall.elec_angle_val = hall_startup_theta_table[hall.state];

              reset_ma_buffer(hall_interval_moving_average);

              hall.double_interval = MAX_CAP_COUNT;
              tmr_period_value_set(HALL_CAPTURE_TIMER, hall.double_interval);
            }
            else
              hall.offset += hall.hall_interval;
          }
          else   // hall state error
          {
            error_code |= error_code_mask & MC_HALL_ERROR;
          }
        }

        bit_shift = 1;
      }
      else
      {
        hall.offset += hall.hall_interval;
      }
    }
  }
  else if (tmr_flag_get(HALL_CAPTURE_TIMER, TMR_OVF_FLAG) != RESET)
  {
    /* clear overflow flag of hall timer */
    tmr_flag_clear(HALL_CAPTURE_TIMER, TMR_OVF_FLAG);

    /* hall interval accumulation */
    hall.offset += hall.double_interval;

    if (hall.offset > MAX_CAP_COUNT)
    {
      hall.offset = MAX_CAP_COUNT;
    }

    /* set motor speed and theta error in half or zero*/
    if (abs(rotor_speed_hall.filtered) < MIN_SPEED_RPM_SHIFT)
    {
      rotor_speed_hall.filtered = 0;
      hall.theta_inc = 0;
      error_code |= error_code_mask & hall_at_zero_speed(&hall);
      reset_ma_buffer(hall_interval_moving_average);
      hall.offset = 0;
    }
    else
    {
      hall.theta_inc >>= bit_shift;
      rotor_speed_hall.filtered >>= bit_shift;
      bit_shift *= 2;
    }

    if (rotor_speed_hall.filtered != 0)
      hall.hall_interval_filt = moving_average_update(hall_interval_moving_average, hall.offset); //update the moving average speed

    /* update hall timer period */
    hall.double_interval <<= 1;

    if (hall.double_interval > MAX_CAP_COUNT)
    {
      hall.double_interval = MAX_CAP_COUNT;
    }

    tmr_period_value_set(HALL_CAPTURE_TIMER, hall.double_interval);
  }
}

/**
  * @brief  hall initialize function at zero speed
  * @param  hall_handler: hall sensors related variables
  * @retval error code (MC_NO_ERROR or MC_HALL_ERROR)
  */
err_code_type hall_at_zero_speed(hall_sensor_type *hall_handler)
{
  int16_t hall_state_table_value;
  err_code_type error_code_handler = MC_NO_ERROR;

  error_code_handler = read_hall_state(hall_handler);
  hall_state_table_value = hall_next_state_table[hall_handler->state];

  if (hall_state_table_value > 0)
  {
    hall_handler->pre_state = hall_handler->state;
    rotor_angle_hall.elec_angle_val = hall_startup_theta_table[hall_handler->state];
    hall_handler->theta_inc = 0;
  }
  else
  {
    error_code_handler = MC_HALL_ERROR;
  }

  return (error_code_handler);
}

void hall_learn_process(void)
{
  static uint8_t start_index = 0;
  switch(hall_learn.process_state)
  {
  case PROCESS_0_LOCK:
    /* disable overflow and trigger interrup of hall timer */
    tmr_interrupt_enable(HALL_CAPTURE_TIMER, TMR_OVF_INT | TMR_TRIGGER_INT, FALSE);
    foc_rdy = SET;
    openloop.theta = 0;
    openloop.inc = 0;
    openloop.volt.d = hall_learn.learn_volt * 2;
    pwm_switch_on();
    if(sys_counter >= hall_learn.align_time)
    {
      hall_learn.process_state = PROCESS_1_FREE_RUN;
    }
    break;

  case PROCESS_1_FREE_RUN:
    /* get hall state */
    error_code |= error_code_mask & read_hall_state(&hall_learn.hall_state);
    hall_learn_state_table[0] = hall_learn.hall_state.state;
    hall_learn.hall_state.pre_state = hall_learn.hall_state.state;

    openloop.volt.d = 0;
    hall_learn.process_state = PROCESS_2_OPENLOOP;
    hall_learn.step = 0;
    break;

  case PROCESS_2_OPENLOOP:
    openloop.volt.d = hall_learn.learn_volt;
    openloop.inc = hall_learn.learn_angle_inc;
    /* clear interrupt flag of hall timer */
    tmr_flag_clear(HALL_CAPTURE_TIMER, TMR_TRIGGER_FLAG | TMR_OVF_FLAG);
    /* enable overflow flag of hall timer */
    tmr_interrupt_enable(HALL_CAPTURE_TIMER, TMR_TRIGGER_INT | TMR_OVF_INT, TRUE);
    /* enable hall timer */
    tmr_counter_enable(HALL_CAPTURE_TIMER, TRUE);
    hall_learn.process_state = PROCESS_3_LEARNING;
    break;

  case PROCESS_3_LEARNING:
    if(sys_counter >= LEARN_TIME)
    {
      if(hall_learn.step >= LEARN_CHECK_TIMES)
      {
        hall_learn.process_state = PROCESS_4_FINISH;
      }
      else
      {
        hall_learn.process_state = PROCESS_5_ERROR;
      }
      openloop.volt.d = 0;
      openloop.inc = 0;
      pwm_switch_off();
      /* disable overflow and trigger interrup of hall timer */
      tmr_interrupt_enable(HALL_CAPTURE_TIMER, TMR_OVF_INT | TMR_TRIGGER_INT, FALSE);
      /* disable hall timer */
      tmr_counter_enable(HALL_CAPTURE_TIMER, FALSE);
    }
    break;

  case PROCESS_4_FINISH:
    for(uint8_t i=0; i< HALL_LEARN_TABLE_LENGTH; i++)
    {
      if(hall_learn_sequence_table[i] == hall_learn_state_table[0])
      {
        start_index = i;
      }
    }
    for(uint8_t i = 0; i < HALL_LEARN_TABLE_LENGTH; i++)
    {
      hall_learn_state_table[i] = hall_learn_sequence_table[(i+start_index)%6];
    }
    hall_learn.check_flag = SET;
    hall_learn.start_flag = RESET;
    esc_state = ESC_STATE_FREE_RUN;
    break;

  case PROCESS_5_ERROR:
    hall_learn.check_flag = RESET;
    error_code |= error_code_mask & MC_HALL_LEARN_ERROR;
    hall_learn.start_flag = RESET;
    esc_state = ESC_STATE_FREE_RUN;
    break;

  default:
    break;
  }
}

/**
  * @brief  mapping hall state/angle table for foc control
  * @param  none
  * @retval none
  */
void foc_hall_table_mapping(void)
{
  uint8_t i,j;
  uint16_t table[7] = {0};

  for(uint8_t i=0; i<7 ; i++)
  {
    table[i] = hall_learn_state_table[i%6];
  }

  for (i=0; i<8; i++)
  {
    for (j=0; j<7; j++)
    {
      if (i == table[j])
      {
        if (j<6)
        {
          hall_cw_next_state_table[i] = table[j+1];
          hall_startup_theta_table[i] = j*60*0x7FFF/360;
        }
        if (j>0)
        {
          hall_ccw_next_state_table[i] = table[j-1];
        }
      }
    }
  }

  for (j=1; j<7; j++)
  {
    hall_cw_theta_table[j] = hall_startup_theta_table[j] - THIRTY_DEGREE;
    hall_ccw_theta_table[j] = hall_startup_theta_table[j] + THIRTY_DEGREE;
  }

}
#endif
