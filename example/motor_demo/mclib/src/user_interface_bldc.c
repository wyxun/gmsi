/**
  **************************************************************************
  * @file     user_interface_bldc.c
  * @brief    Communication interface related functions
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
#include "user_interface_bldc.h"

int32_t flash_buf[SECTOR_SIZE / 4];
MC_Protocol_REG_t MonitorRegID_1, MonitorRegID_2;

/**
  * @brief  Check CRC of receving data, and save data buffer from DMA rx
  * @param  none
  * @retval none
  */
void ui_rx_receive_handler(void)
{
  uint16_t index;
  uint8_t bErrorCode;

  rx_data_command.Size = usart_rx_buffer[1];
  rx_data_command.Code = usart_rx_buffer[2];

  for(index = 0; index < (rx_data_command.Size - 1); index++)
  {
    rx_data_command.Buffer[index] = usart_rx_buffer[index + RCP_FRAME_START_ADRESS];
  }

  rx_data_command.FrameCRC = usart_rx_buffer[index + RCP_FRAME_START_ADRESS];

  if(RCP_CalcCRC(&rx_data_command) == rx_data_command.FrameCRC)
  {
    cmd_response_rdy = RESET;

    if(ui_receive_cmd_index > 0x0F)
    {
      ui_receive_cmd_index = 1;
    }

    /* decode received data */
    RCP_ReceivedFrameID(&rx_data_command);
    ui_receive_cmd_index++;
  }
  else
  {
    bErrorCode = ERROR_CODE_BAD_CRC;
    TCP_SendFrame(&tx_data_response, TCP_CODE_NACK, bErrorCode, 8);
  }
}

/**
  * @brief  save and packet extra data
  * @param  none
  * @retval none
  */
void save_extra_buffer(TCP_Frame_t * pFrame)
{
  uint16_t index = 0, len = 0;

  len = pFrame -> Size_LB + (pFrame -> Size_HB << 8) + 3;

  while(index <= len)
  {
    switch (index)
    {
    case 0:
      extra_data_buffer[index] = (uint8_t) pFrame -> Code;
      break;

    case 1:
      extra_data_buffer[index] = (uint8_t) pFrame -> Size_LB;
      break;

    case 2:
      extra_data_buffer[index] = (uint8_t) pFrame -> Size_HB;
      break;

    default:
      if(index < len)
      {
        extra_data_buffer[index] = (uint8_t) pFrame -> Buffer[index - 3];
      }
      else
      {
        extra_data_buffer[index] = (uint8_t) pFrame -> FrameCRC;
      }

      break;
    }

    index++;
  }

  cmd_response_rdy = SET;
}

/**
  * @brief  Function used to decode received data
  * @param  pFrame : receive data frame
  * @retval none
  */
void RCP_ReceivedFrameID( RCP_Frame_t * pFrame )
{
  uint8_t bErrorCode, bCmdID;
  MC_Protocol_REG_t bRegID;
  flag_status ch1_data_adress_set_rdy, ch2_data_adress_set_rdy;
  int32_t wData;

  switch(pFrame -> Code)
  {
  case MC_PROTOCOL_CODE_GET_BOARD_INFO:
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, FW_VER_LEN_SIZE);
    break;

  case MC_PROTOCOL_CODE_EXECUTE_CMD:
    bCmdID = (uint8_t)(pFrame -> Buffer[0]);
    RCP_ExecCmd(bCmdID, pFrame);
    break;

  case MC_PROTOCOL_CODE_GET_REG:
    bRegID = (MC_Protocol_REG_t)(pFrame -> Buffer[0]);
    RCP_GetReg(bRegID, pFrame);
    break;

  case MC_PROTOCOL_CODE_SET_REG:
    bRegID = (MC_Protocol_REG_t)(pFrame -> Buffer[0]);
    RCP_SetReg(bRegID, pFrame);
    break;

  case MC_PROTOCOL_CODE_SAVE_MONITOR_DATA:
    MonitorRegID_1 = (MC_Protocol_REG_t)(pFrame -> Buffer[0]);
    MonitorRegID_2 = (MC_Protocol_REG_t)(pFrame -> Buffer[1]);
    ch1_data_adress_set_rdy = RCP_SetMonitorDataAdress(MonitorRegID_1, 0);
    ch2_data_adress_set_rdy = RCP_SetMonitorDataAdress(MonitorRegID_2, 1);

    if((ch1_data_adress_set_rdy & ch2_data_adress_set_rdy) == 1)
    {
      TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    }
    else
    {
      bErrorCode = ERROR_CODE_GET_WRITE_ONLY;
      TCP_SendFrame(&tx_data_response, TCP_CODE_NACK, bErrorCode, 8);
    }

    break;

  case MC_PROTOCOL_CODE_LOAD_MONITOR_DATA:
    wData = MonitorRegID_1 + (MonitorRegID_2 << 8);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  default:
    bErrorCode = ERROR_BAD_FRAME_ID;
    TCP_SendFrame(&tx_data_response, TCP_CODE_NACK, bErrorCode, 8);
  }
}

/**
  * @brief  Allow to execute a command coming from the user.
  * @param  CmdID : Code of command to execute.
  *         See MC_PROTOCOL_CMD_xxx for code definition.
  * @param  pFrame : receive data frame
  * @retval none
  */
void RCP_ExecCmd(uint8_t CmdID, RCP_Frame_t * pFrame)
{
  uint8_t bErrorCode;

  switch(CmdID)
  {
  case MC_PROTOCOL_CMD_START_MOTOR:
    start_stop_btn_flag = SET;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_CMD_STOP_MOTOR:
    start_stop_btn_flag = RESET;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_CMD_WRITE_FLASH:
    write_flash_cmd();
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_CMD_FAULT_ACK:
    error_code = MC_NO_ERROR;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_CMD_WINDING_IDENTIFY:
#ifdef MOTOR_PARAM_IDENTIFY
    if (esc_state == ESC_STATE_SAFETY_READY)
    {
      motor_param_ident.state_flag = PROCESSING;
      esc_state = ESC_STATE_WINDING_PARAM_ID;
    }
#endif
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_CMD_AUTO_TUNE_CURR_PI:
    if (esc_state == ESC_STATE_SAFETY_READY)
    {
      I_auto_tune.state_flag = PROCESSING;
    }
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_CMD_HALL_LEARN:
#ifdef HALL_SENSORS
    if (esc_state == ESC_STATE_SAFETY_READY)
    {
      hall_learn.start_flag = SET;
    }
#endif
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  default:
    bErrorCode = ERROR_CODE_WRONG_CMD;
    TCP_SendFrame(&tx_data_response, TCP_CODE_NACK, bErrorCode, 8);
    break;
  }
}

/**
  * @brief  Allow to execute a GetReg command coming from the user.
  * @param  reg_id : Code of register to update.
  *         See MC_PROTOCOL_REG_xxx for code definition.
  * @param  pFrame : receive data frame
  * @retval none
  */
void RCP_GetReg(MC_Protocol_REG_t RegID, RCP_Frame_t * pFrame)
{
  uint8_t bErrorCode;
  int32_t wData;

  switch(RegID)
  {
  case MC_PROTOCOL_REG_CURRENT_BASE:
    wData = (int32_t)(intCoeffs32[MC_PROTOCOL_REG_CURRENT_BASE]);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_VOLTAGE_BASE:
    wData = (int32_t)(intCoeffs32[MC_PROTOCOL_REG_VOLTAGE_BASE]);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_FIRMWARE_ID:
    wData = (int32_t)(intCoeffs32[MC_PROTOCOL_REG_FIRMWARE_ID]);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 8);
    break;

  case MC_PROTOCOL_REG_CURRENT_TUNE_TARGET_I:
    wData = (int32_t)current_tune_target_current;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_CURRENT_TUNE_TOTAL_PERIOD:
    wData = (int32_t)current_tune_total_period;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_CURRENT_TUNE_STEP_PERIOD:
    wData = (int32_t)current_tune_step_period;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_ESC_STATUS:
    wData = (int32_t)esc_state;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 8);
    break;

  case MC_PROTOCOL_REG_CONTROL_MODE:
    wData = (int32_t)ctrl_mode;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 8);
    break;

  case MC_PROTOCOL_REG_CTRL_SOURCE:
    wData = (int32_t)ctrl_source;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 8);
    break;

  case MC_PROTOCOL_REG_SPEED_REF:
    wData = (int32_t)speed_ramp.cmd_final;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 32);
    break;

  case MC_PROTOCOL_REG_SPEED_KP:
    wData = (int32_t)pid_spd.kp_gain;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_SPEED_KI:
    wData = (int32_t)pid_spd.ki_gain;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_SPEED_KP_DIV:
    wData = (int32_t)pid_spd.kp_shift;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_SPEED_KI_DIV:
    wData = (int32_t)pid_spd.ki_shift;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_TORQUE_REF:
    wData = (int32_t)current.Ibus.Iref;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_TORQUE_KP:
    wData = (int32_t)pid_is.kp_gain;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_TORQUE_KI:
    wData = (int32_t)pid_is.ki_gain;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_TORQUE_KP_DIV:
    wData = (int32_t)pid_is.kp_shift;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_TORQUE_KI_DIV:
    wData = (int32_t)pid_is.ki_shift;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_BUS_VOLTAGE_MEAS:
    wData = (int32_t)ui_wave_param.iBusVoltage_meas;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_MOS_TEMP_MEAS:
    wData = (int32_t)ui_wave_param.iMosTemperature_meas;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_MOTOR_POWER_MEAS:
    wData = (int32_t)0;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_MAX_MOTOR_POWER:
    wData = (int32_t)0;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_SPEED_MEAS:
    wData = (int32_t)rotor_speed.filtered;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 32);
    break;

  case MC_PROTOCOL_REG_TORQUE_MEAS:
    wData = (int32_t)current.Ibus.Ireal_pu;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_MAX_APP_SPEED:
    wData = (int32_t)(intCoeffs32[MC_PROTOCOL_REG_MAX_APP_SPEED]);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 32);
    break;

  case MC_PROTOCOL_REG_MIN_APP_SPEED:
    wData = (int32_t)(intCoeffs32[MC_PROTOCOL_REG_MIN_APP_SPEED]);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 32);
    break;

  case MC_PROTOCOL_REG_SPEED_ACC:
    wData = (int32_t)speed_ramp.acc_slope;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_SPEED_DEC:
    wData = (int32_t)speed_ramp.dec_slope;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_MAX_CURRENT:
    wData = (int32_t)max_current_pu;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_MIN_CURRENT:
    wData = (int32_t)min_current_pu;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_START_CURRENT:
    wData = (int32_t)start_current_cmd;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_START_PERIOD:
    wData = (int32_t)start_period;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_EMF_OFF_OFFSET_RISE:
    wData = (int32_t)zcp_lowspd_rise;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_EMF_OFF_OFFSET_FALL:
    wData = (int32_t)(zcp_lowspd_fall);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_ERROR_CODE:
    wData = (int32_t)error_code;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 8);
    break;

  case MC_PROTOCOL_REG_EMF_ON_OFFSET_RISE:
    wData = (int32_t)adc_sample.emf.emf_high_spd_offset_rising;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_EMF_ON_OFFSET_FALL:
    wData = (int32_t)adc_sample.emf.emf_high_spd_offset_falling;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_OLC_INIT_VOLT:
    wData = (int32_t)openloop.olc_init_volt;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_OLC_INIT_SPD:
    wData = (int32_t)openloop.olc_init_spd;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_OLC_VOLT_INC:
    wData = (int32_t)openloop.olc_volt_inc;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_OLC_FINAL_SPD:
    wData = (int32_t)openloop.olc_final_spd;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_OLC_TIMES:
    wData = (int32_t)openloop.olc_times;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_START_VOLT:
    wData = (int32_t)start_volt_cmd;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_HALL_LEARN_0_STATE:
    wData = (int32_t)hall_learn_state_table[0];
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_HALL_LEARN_1_STATE:
    wData = (int32_t)hall_learn_state_table[1];
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_HALL_LEARN_2_STATE:
    wData = (int32_t)hall_learn_state_table[2];
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_HALL_LEARN_3_STATE:
    wData = (int32_t)hall_learn_state_table[3];
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_HALL_LEARN_4_STATE:
    wData = (int32_t)hall_learn_state_table[4];
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_HALL_LEARN_5_STATE:
    wData = (int32_t)hall_learn_state_table[5];
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_HALL_LEARN_PROCESS_STATE:
    wData = (int32_t)hall_learn.process_state;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 8);
    break;

  case MC_PROTOCOL_REG_HALL_LEARN_DIR:
    wData = (int32_t)hall_learn.dir;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 8);
    break;

  case MC_PROTOCOL_REG_IDENT_PROCESS_STATE:
    wData = (int32_t)motor_param_ident.state_flag;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 8);
    break;

  case MC_PROTOCOL_REG_RS:
    wData = (int32_t)motor_param_ident.Rs.u32;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 32);
    break;

  case MC_PROTOCOL_REG_LS:
    wData = (int32_t)motor_param_ident.Ls.u32;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 32);
    break;

  default:
    bErrorCode = ERROR_CODE_GET_WRITE_ONLY;
    TCP_SendFrame(&tx_data_response, TCP_CODE_NACK, bErrorCode, 8);
    break;
  }
}

/**
  * @brief  Allow to execute a SetReg command coming from the user.
  * @param  reg_id : Code of register to update.
  *         See MC_PROTOCOL_REG_xxx for code definition.
  * @param  pFrame : receive data frame
  * @retval none
  */
void RCP_SetReg(MC_Protocol_REG_t RegID, RCP_Frame_t * pFrame)
{
  uint8_t bErrorCode;
  int32_t wData;

  switch(RegID)
  {
  case MC_PROTOCOL_REG_CURRENT_TUNE_TARGET_I:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    current_tune_target_current = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_CURRENT_TUNE_TOTAL_PERIOD:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    current_tune_total_period = (uint16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_CURRENT_TUNE_STEP_PERIOD:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    current_tune_step_period = (uint16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_CONTROL_MODE:
    wData = pFrame -> Buffer[1];
    ctrl_mode = (motor_control_mode)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_CTRL_SOURCE:
    wData = pFrame -> Buffer[1];
    ctrl_source = (ctrl_source_type)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_SPEED_REF:
    wData = (int32_t)(pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8) + ((pFrame -> Buffer[3]) << 16) + ((pFrame -> Buffer[4]) << 24));
    speed_ramp.cmd_final = (int32_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_SPEED_KP:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_spd.kp_gain = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_SPEED_KI:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_spd.ki_gain = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_SPEED_KP_DIV:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_spd.kp_shift = (uint16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_SPEED_KI_DIV:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_spd.ki_shift = (uint16_t)wData;
    pid_spd.upper_limit_integral = (int32_t)(pid_spd.upper_limit_output << pid_spd.ki_shift);
    pid_spd.lower_limit_integral = (int32_t)(-(-pid_spd.lower_limit_output << pid_spd.ki_shift));
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_TORQUE_REF:
    wData = (int16_t)(pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8));

    /* current cmd limitation */
    if(wData > MAX_CURRENT_PU || wData < MIN_CURRENT_PU)
    {
      bErrorCode = ERROR_CODE_WRONG_SET;
      TCP_SendFrame(&tx_data_response, TCP_CODE_NACK, bErrorCode, 8);
    }
    else
    {
      current.Ibus.Iref = (int16_t)wData;
      TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    }

    break;

  case MC_PROTOCOL_REG_TORQUE_KP:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_set_kp(&pid_is, (int16_t)wData);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_TORQUE_KI:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_set_ki(&pid_is, (int16_t)wData);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_TORQUE_KP_DIV:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_is.kp_shift = (uint16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_TORQUE_KI_DIV:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_is.ki_shift = (uint16_t)wData;
    pid_is.upper_limit_integral = (int32_t)(pid_is.upper_limit_output << pid_is.ki_shift);
    pid_is.lower_limit_integral = (int32_t)(-(-pid_is.lower_limit_output << pid_is.ki_shift));
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_MAX_MOTOR_POWER:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    break;

  case MC_PROTOCOL_REG_SPEED_ACC:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    speed_ramp.acc_slope = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_SPEED_DEC:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    speed_ramp.dec_slope = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_MAX_CURRENT:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_spd.upper_limit_output = (int16_t)wData;
    pid_spd.upper_limit_integral = (int32_t)(wData << pid_spd.ki_shift);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_MIN_CURRENT:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_spd.lower_limit_output = (int16_t)wData;
    pid_spd.lower_limit_integral = (int32_t)(-(-pid_spd.lower_limit_output << pid_spd.ki_shift));
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_START_CURRENT:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    start_current_cmd = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_START_PERIOD:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    start_period = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_EMF_OFF_OFFSET_RISE:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    zcp_lowspd_rise = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_EMF_OFF_OFFSET_FALL:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    zcp_lowspd_fall = (int16_t)(wData);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_EMF_ON_OFFSET_RISE:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    adc_sample.emf.emf_high_spd_offset_rising = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_EMF_ON_OFFSET_FALL:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    adc_sample.emf.emf_high_spd_offset_falling = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_OLC_INIT_VOLT:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    openloop.olc_init_volt = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_OLC_INIT_SPD:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    openloop.olc_init_spd = (int16_t)wData;
    openloop.olc_init_period = ((int16_t)10 * PWM_FREQ / openloop.olc_init_spd / POLE_PAIRS);

    if(openloop.olc_times != 0)
    {
      openloop.olc_period_dec = ((int16_t)(openloop.olc_init_period - openloop.olc_final_period) / openloop.olc_times);
    }
    else
    {
      openloop.olc_period_dec = 0;
    }

    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_OLC_VOLT_INC:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    openloop.olc_volt_inc = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_OLC_FINAL_SPD:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    openloop.olc_final_spd = (int16_t)wData;
    openloop.olc_final_period = ((int16_t)10 * PWM_FREQ / openloop.olc_final_spd / POLE_PAIRS);

    if(openloop.olc_times != 0)
    {
      openloop.olc_period_dec = ((int16_t)(openloop.olc_init_period - openloop.olc_final_period) / openloop.olc_times);
    }
    else
    {
      openloop.olc_period_dec = 0;
    }

    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_OLC_TIMES:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    openloop.olc_times = (int16_t)wData;

    if(openloop.olc_times != 0)
    {
      openloop.olc_period_dec = ((int16_t)(openloop.olc_init_period - openloop.olc_final_period) / openloop.olc_times);
    }
    else
    {
      openloop.olc_period_dec = 0;
    }

    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_START_VOLT:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    start_volt_cmd = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  default:
    bErrorCode = ERROR_CODE_GET_WRITE_ONLY;
    TCP_SendFrame(&tx_data_response, TCP_CODE_NACK, bErrorCode, 8);
    break;
  }
}

/**
  * @brief  Save monitor data adress
  * @param  reg_id : register ID
  * @param  channel : waveform drawing channel(0 or 1)
  * @retval status of saving data adress, the returned value can be:SET or RESET.
  */
flag_status RCP_SetMonitorDataAdress(MC_Protocol_REG_t reg_id, uint8_t channel)
{
  uint8_t bErrorCode;

  switch(reg_id)
  {
  case MC_PROTOCOL_REG_TORQUE_REF:
    monitor_data[channel] = &current.Ibus.Iref;
    break;

  case MC_PROTOCOL_REG_BUS_VOLTAGE_MEAS:
    monitor_data[channel] = &ui_wave_param.iBusVoltage_meas;
    break;

  case MC_PROTOCOL_REG_MOS_TEMP_MEAS:
    monitor_data[channel] = &ui_wave_param.iMosTemperature_meas;
    break;

  case MC_PROTOCOL_REG_TORQUE_MEAS:
    monitor_data[channel] = &current.Ibus.Ireal_pu;
    break;

  case MC_PROTOCOL_REG_SPD_REF_PU:
    monitor_data[channel] = &ui_wave_param.speed_reference_pu;
    break;

  case MC_PROTOCOL_REG_SPD_MEAS_PU:
    monitor_data[channel] = &ui_wave_param.speed_meas_filter_pu;
    break;

  case MC_PROTOCOL_REG_USER_DEFINED_A:
    /* modified here to change monitor parameter 1 */
    monitor_data[channel] = &ui_wave_param.user_define_a;
    break;

  case MC_PROTOCOL_REG_USER_DEFINED_B:
    /* modified here to change monitor parameter 2 */
    monitor_data[channel] = &ui_wave_param.user_define_b;
    break;

  default:
    bErrorCode = ERROR_CODE_GET_WRITE_ONLY;
    TCP_SendFrame(&tx_data_response, TCP_CODE_NACK, bErrorCode, 8);
    return (RESET);
  }

  return (SET);
}

/**
  * @brief  Save data in the flash memory
  * @param  none
  * @retval none
  */
void write_flash_cmd()
{
  save_param_buffer[MC_PROTOCOL_REG_NONE] = 0;
  save_param_buffer[MC_PROTOCOL_REG_CURRENT_BASE] = (int32_t)(CURRENT_BASE * 100);
  save_param_buffer[MC_PROTOCOL_REG_VOLTAGE_BASE] = (int32_t) VOLTAGE_BASE;
  save_param_buffer[MC_PROTOCOL_REG_FIRMWARE_ID] = firmware_id;
  save_param_buffer[MC_PROTOCOL_REG_CURRENT_TUNE_TARGET_I] = current_tune_target_current;
  save_param_buffer[MC_PROTOCOL_REG_CURRENT_TUNE_TOTAL_PERIOD] = current_tune_total_period;
  save_param_buffer[MC_PROTOCOL_REG_CURRENT_TUNE_STEP_PERIOD] = current_tune_step_period;
  save_param_buffer[MC_PROTOCOL_REG_ESC_STATUS] = esc_state;
  save_param_buffer[MC_PROTOCOL_REG_CONTROL_MODE] = ctrl_mode;
  save_param_buffer[MC_PROTOCOL_REG_CTRL_SOURCE] = ctrl_source;
  save_param_buffer[MC_PROTOCOL_REG_SPEED_REF] = 0;
  save_param_buffer[MC_PROTOCOL_REG_SPEED_KP] = pid_spd.kp_gain;
  save_param_buffer[MC_PROTOCOL_REG_SPEED_KI] = pid_spd.ki_gain;
  save_param_buffer[MC_PROTOCOL_REG_SPEED_KP_DIV] = pid_spd.kp_shift;
  save_param_buffer[MC_PROTOCOL_REG_SPEED_KI_DIV] = pid_spd.ki_shift;
  save_param_buffer[MC_PROTOCOL_REG_TORQUE_REF] = current.Ibus.Iref;
  save_param_buffer[MC_PROTOCOL_REG_TORQUE_KP] = pid_is.kp_gain;
  save_param_buffer[MC_PROTOCOL_REG_TORQUE_KI] = pid_is.ki_gain;
  save_param_buffer[MC_PROTOCOL_REG_TORQUE_KP_DIV] = pid_is.kp_shift;
  save_param_buffer[MC_PROTOCOL_REG_TORQUE_KI_DIV] = pid_is.ki_shift;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_REF] = 0;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_KP] = 0;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_KI] = 0;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_KP_DIV] = 0;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_KI_DIV] = 0;
  save_param_buffer[MC_PROTOCOL_REG_BUS_VOLTAGE_MEAS] = 0;
  save_param_buffer[MC_PROTOCOL_REG_MOS_TEMP_MEAS] = 0;
  save_param_buffer[MC_PROTOCOL_REG_MOTOR_POWER_MEAS] = 0;
  save_param_buffer[MC_PROTOCOL_REG_MAX_MOTOR_POWER] = 0;
  save_param_buffer[MC_PROTOCOL_REG_SPEED_MEAS] = 0;
  save_param_buffer[MC_PROTOCOL_REG_TORQUE_MEAS] = 0;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_MEAS] = 0;
  save_param_buffer[MC_PROTOCOL_REG_MAX_APP_SPEED] = MAX_SPEED_RPM;
  save_param_buffer[MC_PROTOCOL_REG_MIN_APP_SPEED] = MIN_SPEED_RPM;
  save_param_buffer[MC_PROTOCOL_REG_SPEED_ACC] = speed_ramp.acc_slope;
  save_param_buffer[MC_PROTOCOL_REG_SPEED_DEC] = speed_ramp.dec_slope;
  save_param_buffer[MC_PROTOCOL_REG_MAX_CURRENT] = MAX_CURRENT_PU;
  save_param_buffer[MC_PROTOCOL_REG_MIN_CURRENT] = MIN_CURRENT_PU;
  save_param_buffer[MC_PROTOCOL_REG_START_CURRENT] = start_current_cmd;
  save_param_buffer[MC_PROTOCOL_REG_START_PERIOD] = start_period;
  save_param_buffer[MC_PROTOCOL_REG_EMF_OFF_OFFSET_RISE] = zcp_lowspd_rise;
  save_param_buffer[MC_PROTOCOL_REG_EMF_OFF_OFFSET_FALL] = zcp_lowspd_fall;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_WEK_KP] = 0;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_WEK_KI] = 0;
  save_param_buffer[MC_PROTOCOL_REG_MAX_FLUX_WEK_I] = 0;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_WEK_BUS_V_PERCENT] = 0;
  save_param_buffer[MC_PROTOCOL_REG_OBS_C1] = 0;
  save_param_buffer[MC_PROTOCOL_REG_OBS_C2] = 0;
  save_param_buffer[MC_PROTOCOL_REG_PLL_KP] = 0;
  save_param_buffer[MC_PROTOCOL_REG_PLL_KI] = 0;
  save_param_buffer[MC_PROTOCOL_REG_PLL_KP_DIV] = 0;
  save_param_buffer[MC_PROTOCOL_REG_PLL_KI_DIV] = 0;
  save_param_buffer[MC_PROTOCOL_REG_OPEN_LOOP_VOLT] = 0;
  save_param_buffer[MC_PROTOCOL_REG_OPEN_LOOP_ANGLE_INCRE] = 0;
  save_param_buffer[MC_PROTOCOL_REG_ENCODER_ALIGN_VOLT] = 0;
  save_param_buffer[MC_PROTOCOL_REG_ENCODER_OFFSET] = 0;
  save_param_buffer[MC_PROTOCOL_REG_ERROR_CODE] = 0;
  save_param_buffer[MC_PROTOCOL_REG_IA] = 0;
  save_param_buffer[MC_PROTOCOL_REG_IB] = 0;
  save_param_buffer[MC_PROTOCOL_REG_IC] = 0;
  save_param_buffer[MC_PROTOCOL_REG_I_ALPHA] = 0;
  save_param_buffer[MC_PROTOCOL_REG_I_BETA] = 0;
  save_param_buffer[MC_PROTOCOL_REG_IQ_FILTER] = 0;
  save_param_buffer[MC_PROTOCOL_REG_ID_FILTER] = 0;
  save_param_buffer[MC_PROTOCOL_REG_VQ] = 0;
  save_param_buffer[MC_PROTOCOL_REG_VD] = 0;
  save_param_buffer[MC_PROTOCOL_REG_V_ALPHA] = 0;
  save_param_buffer[MC_PROTOCOL_REG_V_BETA] = 0;
  save_param_buffer[MC_PROTOCOL_REG_ELEC_ANGLE_MEAS] = 0;
  save_param_buffer[MC_PROTOCOL_REG_ROTOR_SPEED_MEAS_FILTER] = 0;
  save_param_buffer[MC_PROTOCOL_REG_OBS_ELEC_ANGLE] = 0;
  save_param_buffer[MC_PROTOCOL_REG_OBS_ROTOR_SPEED] = 0;
  save_param_buffer[MC_PROTOCOL_REG_OBS_I_ALPHA] = 0;
  save_param_buffer[MC_PROTOCOL_REG_OBS_I_BETA] = 0;
  save_param_buffer[MC_PROTOCOL_REG_OBS_BEMF_ALPHA] = 0;
  save_param_buffer[MC_PROTOCOL_REG_OBS_BEMF_BETA] = 0;
  save_param_buffer[MC_PROTOCOL_REG_EMF_ON_OFFSET_RISE] = adc_sample.emf.emf_high_spd_offset_rising;
  save_param_buffer[MC_PROTOCOL_REG_EMF_ON_OFFSET_FALL] = adc_sample.emf.emf_high_spd_offset_falling;
  save_param_buffer[MC_PROTOCOL_REG_OLC_INIT_VOLT] = openloop.olc_init_volt;
  save_param_buffer[MC_PROTOCOL_REG_OLC_INIT_SPD] = openloop.olc_init_spd;
  save_param_buffer[MC_PROTOCOL_REG_OLC_VOLT_INC] = openloop.olc_volt_inc;
  save_param_buffer[MC_PROTOCOL_REG_OLC_TIMES] = openloop.olc_times;
  save_param_buffer[MC_PROTOCOL_REG_OLC_FINAL_SPD] = openloop.olc_final_spd;
  save_param_buffer[MC_PROTOCOL_REG_START_VOLT] = start_volt_cmd;
  save_param_buffer[MC_PROTOCOL_REG_HALL_LEARN_0_STATE] = hall_learn_state_table[0];
  save_param_buffer[MC_PROTOCOL_REG_HALL_LEARN_1_STATE] = hall_learn_state_table[1];
  save_param_buffer[MC_PROTOCOL_REG_HALL_LEARN_2_STATE] = hall_learn_state_table[2];
  save_param_buffer[MC_PROTOCOL_REG_HALL_LEARN_3_STATE] = hall_learn_state_table[3];
  save_param_buffer[MC_PROTOCOL_REG_HALL_LEARN_4_STATE] = hall_learn_state_table[4];
  save_param_buffer[MC_PROTOCOL_REG_HALL_LEARN_5_STATE] = hall_learn_state_table[5];
  save_param_buffer[MC_PROTOCOL_REG_HALL_LEARN_DIR] = hall_learn.dir;
  save_param_buffer[MC_PROTOCOL_REG_HALL_LEARN_CHECK_FLAG] = hall_learn.check_flag;

  save_param_buffer[MC_PROTOCOL_REG_IDENT_PROCESS_STATE] = motor_param_ident.state_flag;
  save_param_buffer[MC_PROTOCOL_REG_RS] = motor_param_ident.Rs.u32;
  save_param_buffer[MC_PROTOCOL_REG_LS] = motor_param_ident.Ls.u32;
  /* write data to flash */
  flash_write(MC_VectStoreAddr_UINT32, save_param_buffer, INT32_SIZE_ARRAY);
}

/**
  * @brief  write data using word mode without checking
  * @param  write_addr: the address of writing
  * @param  p_buffer: the buffer of writing data
  * @param  num_write: the number of writing data
  * @retval none
  */
void flash_write_nocheck(uint32_t write_addr, int32_t *p_buffer, uint16_t num_write)
{
  uint16_t i;

  for(i = 0; i < num_write; i++)
  {
    flash_word_program(write_addr, p_buffer[i]);
    write_addr += 4;
  }
}

/**
  * @brief  write data using halfword mode with checking
  * @param  write_addr: the address of writing
  * @param  p_buffer: the buffer of writing data
  * @param  num_write: the number of writing data
  * @retval none
  */
void flash_write(uint32_t write_addr, int32_t *p_buffer, uint16_t num_write)
{
  uint32_t offset_addr;
  uint32_t sector_position;
  uint16_t sector_offset;
  uint16_t sector_remain;
  uint16_t i;

  flash_unlock();
  offset_addr = write_addr - FLASH_BASE;
  sector_position = offset_addr / SECTOR_SIZE;
  sector_offset = (offset_addr % SECTOR_SIZE) / 4;
  sector_remain = SECTOR_SIZE / 4 - sector_offset;

  if(num_write <= sector_remain)
  {
    sector_remain = num_write;
  }

  while(1)
  {
    for(i = 0; i < sector_remain; i++)
    {
      if(flash_buf[sector_offset + i] != 0xFFFFFFFF)
      {
        break;
      }
    }

    if(i < sector_remain)
    {
      flash_sector_erase(sector_position * SECTOR_SIZE + FLASH_BASE);

      for(i = 0; i < sector_remain; i++)
      {
        flash_buf[i + sector_offset] = p_buffer[i];
      }

      flash_write_nocheck(sector_position * SECTOR_SIZE + FLASH_BASE, flash_buf, SECTOR_SIZE / 4);
    }
    else
    {
      flash_write_nocheck(write_addr, p_buffer, sector_remain);
    }

    if(num_write == sector_remain)
    {
      break;
    }
    else
    {
      sector_position++;
      sector_offset = 0;
      p_buffer += sector_remain;
      write_addr += (sector_remain * 4);
      num_write -= sector_remain;

      if(num_write > (SECTOR_SIZE / 4))
      {
        sector_remain = SECTOR_SIZE / 4;
      }
      else
      {
        sector_remain = num_write;
      }
    }
  }

  flash_lock();
}

/**
  * @brief  Packet tansmit data frame
  * @param  pFrame : tansmit data frame
  * @param  code : feedback Code (ACK or NACK)
  * @param  wData : payload data
  * @param  len : payload data length
  * @retval none
  */
void TCP_SendFrame( TCP_Frame_t * pFrame, uint8_t code, int32_t wData, uint16_t len)
{
  uint16_t len_bytes;

  len_bytes = len >> 3;

  pFrame -> Code = code + ui_receive_cmd_index;
  pFrame -> Size_LB = (uint8_t)(len_bytes & 0xFF);
  pFrame -> Size_HB = (uint8_t)(len_bytes >> 8);

  switch(len)
  {
  case TCP_LEN_NONE:
    break;

  case TCP_LEN_BYTE:
    pFrame -> Buffer[0] = (uint8_t)wData;
    break;

  case TCP_LEN_HALF_WORD:
    pFrame -> Buffer[0] = (uint8_t)(wData & 0xFF);
    pFrame -> Buffer[1] = (uint8_t)(wData >> 8);
    break;

  case TCP_LEN_WORD:
    pFrame -> Buffer[0] = (uint8_t)(wData & 0xFF);
    pFrame -> Buffer[1] = (uint8_t)(wData >> 8);
    pFrame -> Buffer[2] = (uint8_t)(wData >> 16);
    pFrame -> Buffer[3] = (uint8_t)(wData >> 24);
    break;

  default:

    /* GetBoardInfo */
    for (uint8_t i = 0; (i < 29) && (s_fwVer[i] != '\t'); i++)
    {
      pFrame -> Buffer[i] = s_fwVer[i];
    }

    break;
  }

  pFrame -> FrameCRC = TCP_CalcCRC(pFrame);
  save_extra_buffer(pFrame);
}

/**
  * @brief  Calcultion CRC of receive data
  * @param  pFrame : receive data frame
  * @retval none
  */
uint8_t RCP_CalcCRC(RCP_Frame_t * pFrame)
{
  uint8_t nCRC = 0;
  uint16_t nSum = 0;

  if( pFrame == NULL )
  {
  }
  else
  {
    nSum += pFrame->Size;
    nSum += pFrame->Code;

    for (uint8_t idx = 0; idx < (pFrame->Size - 1); idx++ )
    {
      nSum += pFrame->Buffer[idx];
    }

    nCRC = (uint8_t)(nSum & 0xFF) ;
    nCRC += (uint8_t) (nSum >> 8) ;
  }

  return nCRC ;
}

/**
  * @brief  Calcultion CRC of transmit data
  * @param  pFrame : tansmit data frame
  * @retval none
  */
uint8_t TCP_CalcCRC(TCP_Frame_t * pFrame)
{
  uint8_t nCRC = 0, idx = 0;
  uint16_t nSum = 0, buffer_size = 0;

  if( pFrame == NULL )
  {
  }
  else
  {
    buffer_size = pFrame->Size_LB + (pFrame->Size_HB << 8);

    nSum += pFrame->Size_LB;
    nSum += pFrame->Size_HB;
    nSum += pFrame->Code;

    for ( idx = 0; idx < buffer_size; idx++ )
    {
      nSum += pFrame -> Buffer[idx];
    }

    nCRC = (uint8_t)(nSum & 0xFF) ;
    nCRC += (uint8_t) (nSum >> 8) ;
  }

  return nCRC;
}

/**
  * @brief  Send full data to user
  * @param  none
  * @retval none
  */
void tx_send_monitor_data()
{
  int16_t index = 0;

  /* disable DMA UART TX*/
  dma_channel_enable(DMA_UART_TX_CHANNEL, FALSE);

  /* set transmit table */
  DMA_UART_TX_CHANNEL->maddr = (uint32_t) &monitor_data_buffer[monitor_data_buffer_num][0];

  /* put extra data response */
  if(cmd_response_rdy != RESET)
  {
    while(index < DATA_BUFFER_EXTRA_SIZE)
    {
      monitor_data_buffer[monitor_data_buffer_num][DATA_BUFFER_SYNC_SIZE + DATA_BUFFER_SIZE + index] = extra_data_buffer[index];
      index++;
    }
  }

  /* set transmission data number */
  dma_data_number_set(DMA_UART_TX_CHANNEL, DATA_BUFFER_FRAME_SIZE);
  /* enable DMA UART TX*/
  dma_channel_enable(DMA_UART_TX_CHANNEL, TRUE);
}

/**
  * @brief  Save monitor data(for dual-channel waveform drawing)
  * @param  none
  * @retval none
  */
void ui_save_monitor_data(void)
{
  monitor_data_buffer[monitor_data_buffer_num][buffer_index++] = (uint8_t)(*monitor_data[0] & 0xFF);
  monitor_data_buffer[monitor_data_buffer_num][buffer_index++] = (uint8_t)(*monitor_data[0] >> 8);
  monitor_data_buffer[monitor_data_buffer_num][buffer_index++] = (uint8_t)(*monitor_data[1] & 0xFF);
  monitor_data_buffer[monitor_data_buffer_num][buffer_index++] = (uint8_t)(*monitor_data[1] >> 8);

  if(buffer_index > (DATA_BUFFER_SYNC_SIZE + DATA_BUFFER_SIZE - 2))
  {
    tx_send_monitor_data();
    monitor_data_buffer_num ^= 1;
    buffer_index = 10;
  }
}


