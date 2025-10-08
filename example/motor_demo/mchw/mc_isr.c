/**
  **************************************************************************
  * @file     mc_isr.c
  * @brief    Relevant motor control interrupt functions
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

int32_t spd_err;
trig_components_type trig_components;
int16_t Iq_cmd;
int8_t Iq_cmd_count;

/**
  * @brief  PWM timer interrupt handler for update event
  * @param  none
  * @retval none
  */
void ADVTMR_PWM_CYCLE_IRQ(void)
{
  static int8_t ui_count = 0;

  if (tmr_flag_get(PWM_ADVANCE_TIMER, TMR_OVF_FLAG) != RESET)
  {
    tmr_flag_clear(PWM_ADVANCE_TIMER, TMR_OVF_FLAG);

#ifdef ONE_SHUNT
    if ((PWM_ADVANCE_TIMER->ctrl1_bit.cnt_dir & TMR_COUNT_DOWN) == RESET)  /* underflow interrupt */
    {
#endif
      monitoring_signal_adc_trigger(); /* adc orydinary software trigger */

#ifdef MOTOR_PARAM_IDENTIFY
      if (motor_param_ident.state_flag == PROCESSING)
      {
        if (motor_param_ident.id_flag == SET)
        {
          param_identify(&motor_param_ident);
        }
#ifdef ONE_SHUNT
        if (esc_state == ESC_STATE_WINDING_PARAM_ID)
        {
          tmr_channel_value_set(ADC_TIMER, ADC_TIMER_SELECT_CHANNEL, motor_param_ident.duty); /* trigger adc current sensing */
        }
#endif
      }
#endif

      if (curr_offset_rdy & foc_rdy)
      {
        if(ctrl_mode == OPEN_LOOP_CTRL)
        {
          foc_open_loop_ctrl(&volt_cmd, &openloop);

          elec_angle_val = openloop.theta;

          /* trigonometric functions transformation */
          trig_components = trig_functions(elec_angle_val);

          /* Clarke transformation */
          foc_clarke_trans(&current.Iabc, &current.Ialphabeta);

          /* Park transformation */
          foc_park_trans(&current, &trig_components);

          if (volt_cmd.Vqd.q >= 0)
          {
            startup.dir = CW;
          }
          else
          {
            startup.dir = CCW;
          }
#if VOLT_SENSE
          motor_volt_sense(&motor_voltage, &motor_emf);
          motor_voltage.Vpu = motor_emf.emf_alphabeta_voltage;
#else
          motor_volt_calc(&motor_voltage);
#endif

          state_observer.elec_angle = rotor_angle_sensorless(&state_observer, &motor_voltage);

          /* low pass filter for speed by observer */
          state_observer.motor_speed.filtered = lowpass_filtering(&obs_speed_LPF, state_observer.motor_speed.val);

          /* foc circle limitation */
          foc_circle_limitation(&volt_cmd);

          /* Inverse Park transformation */
          foc_inver_park_trans(&volt_cmd, &trig_components);

          /* svpwm function */
          svpwm_func();

          /* pwm output */
          pwm_duty_update();

        }
        else
        {
          if (ctrl_mode == SPEED_CTRL || ctrl_mode == TORQUE_CTRL || ctrl_mode == VF_CTRL)
          {
            if (startup.closeloop_rdy != SET)
            {
              if (ctrl_mode == SPEED_CTRL && speed_ramp.cmd_final >= 0)
              {
                startup.dir = CW;
              }
              else if (ctrl_mode == TORQUE_CTRL && current.Iqdref.q >= 0)
              {
                startup.dir = CW;
              }
              else
              {
                startup.dir = CCW;
              }

#if defined OPENLOOP_STARTUP    /* method 1: open loop start-up */
              startup.closeloop_rdy = startup_openloop(&startup, &Iref);

#elif defined ALIGN_AND_GO_STARTUP    /* method 2:  fixed alpha axis and force to beta axis */
              startup.closeloop_rdy = startup_alpha_axis(&startup, &Iref);

#elif defined INIT_ANGLE_STARTUP    /* method 3: Initial angle detection */
              startup.closeloop_rdy = startup_angle_init2(&startup, &Iref);
#endif
            }

            if (startup.closeloop_rdy_old != RESET)
            {
              elec_angle_val = state_observer.elec_angle;
            }
            else if (startup.closeloop_rdy != RESET)
            {
              elec_angle_val = state_observer.elec_angle;
              startup.closeloop_rdy_old = startup.closeloop_rdy;

              if (ctrl_mode == SPEED_CTRL)
              {
                if (startup.dir == CW)
                {
                  current.Iqdref.q = (startup.start_current >> 1);
                }
                else
                {
                  current.Iqdref.q = -(startup.start_current >> 1);
                }

                pid_set_integral(&pid_spd, (int32_t)(current.Iqdref.q << pid_spd.ki_shift));
              }
            }
            else
            {
              elec_angle_val = startup.elec_angle;
            }
          }

          /* trigonometric functions transformation */
          trig_components = trig_functions(elec_angle_val);

          /* Clarke transformation */
          foc_clarke_trans(&current.Iabc, &current.Ialphabeta);

          /* Park transformation */
          foc_park_trans(&current, &trig_components);

#if VOLT_SENSE
          motor_volt_sense(&motor_voltage, &motor_emf);
          motor_voltage.Vpu = motor_emf.emf_alphabeta_voltage;
#else
          motor_volt_calc(&motor_voltage);
#endif

          state_observer.elec_angle = rotor_angle_sensorless(&state_observer, &motor_voltage);

          /* low pass filter for speed by observer */
          state_observer.motor_speed.filtered = lowpass_filtering(&obs_speed_LPF, state_observer.motor_speed.val);

#if CURRENT_LP_FILTER
          /* low pass filter for q/d-axis current */
          current.Iqd_LPF.d = lowpass_filtering(&d_current_LPF, current.Iqd.d);
          current.Iqd_LPF.q = lowpass_filtering(&q_current_LPF, current.Iqd.q);
          Ival.d = current.Iqd_LPF.d;
          Ival.q = current.Iqd_LPF.q;
#else
          Ival.d = current.Iqd.d;
          Ival.q = current.Iqd.q;
#endif

          if (startup.closeloop_rdy == SET || ctrl_mode == ID_MANUAL_TUNE || ctrl_mode == IQ_MANUAL_TUNE)
          {
            Iref.d = current.Iqdref.d;
            Iref.q = current.Iqdref.q;
          }

          /* Torque control */
          if (ctrl_mode == POSITION_CTRL || ctrl_mode == SPEED_CTRL || ctrl_mode == TORQUE_CTRL || ctrl_mode == ID_MANUAL_TUNE || ctrl_mode == IQ_MANUAL_TUNE)
          {
            volt_cmd.Vqd.d = pid_controller(&pid_id, (Iref.d - Ival.d));

            /* foc Vq limitation */
            foc_vq_limitation(&volt_cmd, &pid_iq);

            volt_cmd.Vqd.q = pid_controller(&pid_iq, (Iref.q - Ival.q));
          }
          else if (ctrl_mode == VF_CTRL)
          {
            /* foc circle limitation */
            foc_circle_limitation(&volt_cmd);
          }

          /* Inverse Park transformation */
          foc_inver_park_trans(&volt_cmd, &trig_components);

          /* svpwm function */
          svpwm_func();

          /* pwm output */
          pwm_duty_update();

#if defined FIELD_WEAKENING
          field_weakening.fw_err_filt = ma_filter(pid_iq.upper_limit_output - abs(pid_iq.out_start), field_weakening.fw_err_filt, FW_MA_NBR_LOG);
#endif
        }
      }
      /* UI sample */
      if(++ui_count >= ui_wave_param.sample_cycle)
      {
        ui_wave_param.user_define_a = (int16_t)elec_angle_val;
        ui_wave_param.user_define_b = (int16_t)rotor_speed_val_filt;
        ui_save_monitor_data();
        ui_count = 0;
      }

#ifdef ONE_SHUNT
    }
    else		/* overflow interrupt */
    {
      /* pwm output */
      pwm_duty_extra_update();
    }

#endif
  }
}

/**
  * @brief  timer interrupt handler for break event
  * @param  none
  * @retval none
  */
void ADVTMR_PWM_BRK_IRQ(void)
{
  if (tmr_flag_get(PWM_ADVANCE_TIMER, TMR_BRK_FLAG) != RESET)
  {
    tmr_flag_clear(PWM_ADVANCE_TIMER, TMR_BRK_FLAG);

    /* Over current protection */
    error_code |= error_code_mask & MC_OVER_CURRENT_ERROR;
  }
}

/**
  * @brief  adc interrupt handler for end of preempted channel conversion
  * @param  none
  * @retval none
  */
void ADC_SHUNT_SAMP_READY_IRQ(void)
{
  if (adc_flag_get(ADC_CONVERTER, ADC_PCCE_FLAG) != RESET)
  {
    adc_flag_clear(ADC_CONVERTER, ADC_PCCE_FLAG);

    if (esc_state != ESC_STATE_ANGLE_INIT)
    {
#if defined THREE_SHUNT
      current_read_foc_3shunt(&current, &pwm_duty);
#elif defined TWO_SHUNT
      current_read_foc_2shunt(&current);
#elif defined ONE_SHUNT
#ifdef MOTOR_PARAM_IDENTIFY
      if (esc_state == ESC_STATE_WINDING_PARAM_ID)
      {
        current_read_1shunt_ID(&current);
      }
      else
#endif
        current_read_foc_1shunt(&current, &volt_cmd);
#endif
    }
    else
    {
#if defined THREE_SHUNT
      current_angle_init_3shunt(&angle_detector, &current);
#elif defined TWO_SHUNT || defined ONE_SHUNT
      current_angle_init_2_1shunt(&angle_detector, &current);
#endif
    }
  }
}


/**
  * @brief  timer interrupt handler for speed control loop
  * @param  none
  * @retval none
  */
void SPEED_LOOP_TIMER_IRQ(void)
{
  if (tmr_flag_get(SPEED_LOOP_TIMER, TMR_OVF_FLAG) != RESET)
  {
    /* clear flags of overflow events */
    tmr_flag_clear(SPEED_LOOP_TIMER, TMR_OVF_FLAG);

    if (esc_state == ESC_STATE_RUNNING)
    {
      if (ctrl_mode == SPEED_CTRL)
      {
        spd_err = speed_ramp.command - rotor_speed_val;
        current.Iqdref.q = pid_controller(&pid_spd, spd_err);

#ifdef FIELD_WEAKENING
        current.Iqdref.d = pid_controller(&(field_weakening.pid_fw), field_weakening.fw_err_filt);
#endif
      }
    }
  }
}

/**
  * @brief  this function handles systick handler.
  * @param  none
  * @retval none
  */
void SysTick_Handler(void)
{
  int16_t sp_value;

  rotor_speed_val = state_observer.motor_speed.filtered;

  rotor_speed_val_filt = moving_average_update(speed_ma_fliter, rotor_speed_val);

  sp_value = adc_in_tab[ADC_POTENTIO_IDX] - SP_OFFSET;

  switch(esc_state_old)
  {
  case ESC_STATE_IDLE:
    if(ctrl_source == CTRL_SOURCE_EXTERNAL)
    {
      if((sp_value <= 0) && (param_initial_rdy != RESET) && (curr_offset_rdy != RESET))
      {
        esc_state = ESC_STATE_SAFETY_READY;
      }
      else
      {
        esc_state = ESC_STATE_IDLE;
      }
    }
    else
    {
      if((curr_offset_rdy != RESET) && (param_initial_rdy != RESET))
      {
        esc_state = ESC_STATE_SAFETY_READY;
      }
      else
      {
        esc_state = ESC_STATE_IDLE;
      }
    }

    break;

  case ESC_STATE_SAFETY_READY:
    if(start_stop_btn_flag != RESET)
    {
      if(ctrl_mode == ID_MANUAL_TUNE || ctrl_mode == IQ_MANUAL_TUNE)
      {
        esc_state = ESC_STATE_I_TUNE;
      }
      else if(ctrl_mode == OPEN_LOOP_CTRL)
      {
        esc_state = ESC_STATE_RUNNING;
      }
      else
      {
#if  defined OPENLOOP_STARTUP || defined ALIGN_AND_GO_STARTUP
        esc_state = ESC_STATE_STARTING;
#elif defined INIT_ANGLE_STARTUP
        esc_state = ESC_STATE_ANGLE_INIT;
#endif
      }
    }

    if(I_auto_tune.state_flag == PROCESSING)
    {
      current_auto_tuning(&I_auto_tune);
      set_current_pid_param(&I_auto_tune, &pid_iq);
      set_current_pid_param(&I_auto_tune, &pid_id);
      I_auto_tune.state_flag = SUCCEED;
    }

    break;

  case ESC_STATE_ANGLE_INIT:
#if defined INIT_ANGLE_STARTUP
    if(angle_detector.step_count < STEP_8_MAX)
    {
      foc_sensorless_angle_init(&angle_detector, &current);
      angle_detector.step_count++;
      esc_state = ESC_STATE_ANGLE_INIT;
    }
    else
    {
      esc_state = ESC_STATE_STARTING;
    }

#endif
    break;

  case ESC_STATE_STARTING:
    if (startup.closeloop_rdy != RESET)
    {
      speed_ramp.command = rotor_speed_val;
      esc_state = ESC_STATE_RUNNING;
    }
    break;

  case ESC_STATE_ENC_ALIGN:
    if (ctrl_mode != OPEN_LOOP_CTRL)
    {
      if (encoder.align == SUCCEED)
      {
        esc_state = ESC_STATE_RUNNING;
      }
    }
    else
    {
      esc_state = ESC_STATE_RUNNING;
    }

    break;

  case ESC_STATE_RUNNING:

    if(ctrl_source == CTRL_SOURCE_EXTERNAL)
    {
      if(ctrl_mode == SPEED_CTRL)
      {
        if (sp_value >= SP_RUN_POINT)
        {
          speed_ramp.cmd_final = ((sp_value << 1) * SP_TO_SPD_CMD) >> 15;
        }
        else
        {
          speed_ramp.cmd_final = MIN_CONTROL_SPEED;
        }

        command_ramp(&speed_ramp);
      }
      else if(ctrl_mode == TORQUE_CTRL)
      {
        if (sp_value >= SP_RUN_POINT)
        {
          current.Iqdref.q = (sp_value * SP_TO_I_CMD) >> 15;
        }
      }
    }
    else if(ctrl_source == CTRL_SOURCE_SOFTWARE)
    {
      if (ctrl_mode == SPEED_CTRL)
      {
        command_ramp(&speed_ramp);
      }
    }

    if (start_stop_btn_flag == RESET)
    {
      esc_state = ESC_STATE_FREE_RUN;
    }

    break;

  case ESC_STATE_FREE_RUN:
    if (rotor_speed_val == 0)
    {
      esc_state = ESC_STATE_SAFETY_READY;
    }
    else
    {
      esc_state = ESC_STATE_FREE_RUN;
    }

    break;

  case ESC_STATE_BRAKING:

    break;

  case ESC_STATE_ERROR:
    if (error_code == MC_NO_ERROR)
    {
      esc_state = ESC_STATE_IDLE;
    }

    break;

  case ESC_STATE_I_TUNE:
    if (start_stop_btn_flag == SET)
    {
      I_tune_manual();
    }
    else
    {
      current.Iqdref.d = 0;
      current.Iqdref.q = 0;
      esc_state = ESC_STATE_FREE_RUN;
    }

    if(ctrl_mode != ID_MANUAL_TUNE && ctrl_mode != IQ_MANUAL_TUNE)
    {
      esc_state = ESC_STATE_SAFETY_READY;
    }

    break;

  case ESC_STATE_AUTO_LEARN:
    break;
#ifdef MOTOR_PARAM_IDENTIFY
  case ESC_STATE_WINDING_PARAM_ID:
    if (motor_param_ident.state_flag == PROCESSING)
    {
      param_id_process(&motor_param_ident);
    }
    else
    {
      if (motor_param_ident.Ls.f <= 0 || motor_param_ident.Rs.f <= 0)
      {
        motor_param_ident.state_flag = FAILED;
        error_code |= error_code_mask & MC_PARAM_IDENT_ERROR;
        motor_param_ident.Rs.f = motor_param_ident.Rs_Old.f;
        motor_param_ident.Ls.f = motor_param_ident.Ls_Old.f;
      }
      else
      {
        motor_param_ident.state_flag = SUCCEED;
        motor_param_ident.Rs_Old.f = motor_param_ident.Rs.f;
        motor_param_ident.Ls_Old.f = motor_param_ident.Ls.f;
      }
      pwm_switch_off();
      disable_pwm_timer();
      tmr_pwm_init();
      adc_preempt_config();
      enable_pwm_timer();
      esc_state = ESC_STATE_FREE_RUN;
    }
    break;
#endif
  case ESC_STATE_NONE:
    break;
  }

  fMosTemperature = (((adc_in_tab[ADC_MOS_TEMP_IDX] * ADC_REFERENCE_VOLT / ADC_DIGITAL_SCALE_12BITS) - V0_V) / dV_dT) + T0_C;
  ui_wave_param.iMosTemperature_meas = (int16_t)(fMosTemperature * 100);
  ui_wave_param.iBusVoltage_meas = (int16_t)(adc_in_tab[ADC_BUS_VOLT_IDX]);
  ui_wave_param.speed_meas_filter_pu = (int16_t)((rotor_speed_val_filt * RPM_TO_SPEED_PU)>>15);
  ui_wave_param.speed_reference_pu = (int16_t)((speed_ramp.command * RPM_TO_SPEED_PU)>>15);
  ui_wave_param.position_meas_pu = (int16_t)((int32_t)(angle.val * DEGREE_TO_POS_PU)>>15);
  ui_wave_param.position_reference_pu = (int16_t)((int32_t)(angle.command * DEGREE_TO_POS_PU)>>15);


  /* Over/under voltage protection */
  if (ui_wave_param.iBusVoltage_meas < UNDERVOLTAGE_THRESHOLD_d)
  {
    error_code |= error_code_mask & MC_UNDER_VOLT_ERROR;
  }
  else if (ui_wave_param.iBusVoltage_meas > OVERVOLTAGE_THRESHOLD_d)
  {
    error_code |= error_code_mask & MC_OVER_VOLT_ERROR;
  }

  /* MOS Temperature protection */
  if (adc_in_tab[ADC_MOS_TEMP_IDX] > TEMPERATURE_THRESHOLD_d)
  {
    error_code |= error_code_mask & MC_OVER_TEMP_ERROR;
  }

  /* Enter error state handler */
  if (error_code != MC_NO_ERROR)
  {
    esc_state = ESC_STATE_ERROR;
  }

  /* step Iq current test */
#if 0

  if(ctrl_mode == TORQUE_CTRL)
  {
    if (Iq_cmd_count >= 0)
    {
      current.Iqdref.q = Iq_cmd;
    }
    else
    {
      current.Iqdref.q = -Iq_cmd;
    }

    Iq_cmd_count++;
  }

#endif

}

/**
  * @brief  button interrupt handler
  * @param  none
  * @retval none
  */
void BUTTON_EXINT_IRQHandler(void)
{
  if (exint_flag_get(BUTTON_EXINT_LINE) != RESET)
  {
    /* delay 30ms for debouncing */
    mc_delay_ms(30);

    /* clear interrupt pending bit */
    exint_flag_clear(BUTTON_EXINT_LINE);

    if(gpio_input_data_bit_read(USER_BUTTON_PORT, USER_BUTTON_PIN) == SET)
    {
      if(start_stop_btn_flag != RESET)
      {
        start_stop_btn_flag = RESET;
      }
      else
      {
        start_stop_btn_flag = SET;
      }
    }
  }
}

