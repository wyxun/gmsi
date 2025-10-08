/**
  **************************************************************************
  * @file     user_interface_foc.c
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
#include "user_interface_foc.h"

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

  case MC_PROTOCOL_CMD_ENCODER_ALIGN:
    if (esc_state == ESC_STATE_SAFETY_READY)
    {
      encoder.align = PROCESSING;
      esc_state = ESC_STATE_ENC_ALIGN;
    }

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
#ifdef E_BIKE_SCOOTER
    wData = (int32_t)Iq_ref_cmd;
#else
    wData = (int32_t)current.Iqdref.q;
#endif
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_TORQUE_KP:
    wData = (int32_t)pid_iq.kp_gain;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_TORQUE_KI:
    wData = (int32_t)pid_iq.ki_gain;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_TORQUE_KP_DIV:
    wData = (int32_t)pid_iq.kp_shift;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_TORQUE_KI_DIV:
    wData = (int32_t)pid_iq.ki_shift;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_FLUX_REF:
#ifdef E_BIKE_SCOOTER
    wData = (int32_t)I_ref.d;
#else
    wData = (int32_t)current.Iqdref.d;
#endif
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_FLUX_KP:
    wData = (int32_t)pid_id.kp_gain;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_FLUX_KI:
    wData = (int32_t)pid_id.ki_gain;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_FLUX_KP_DIV:
    wData = (int32_t)pid_id.kp_shift;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_FLUX_KI_DIV:
    wData = (int32_t)pid_id.ki_shift;
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
    wData = (int32_t)rotor_speed_val_filt;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 32);
    break;

  case MC_PROTOCOL_REG_TORQUE_MEAS:
    wData = (int32_t)current.Iqd.q;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_FLUX_MEAS:
    wData = (int32_t)current.Iqd.d;
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
    wData = (int32_t)pid_spd.upper_limit_output;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_MIN_CURRENT:
    wData = (int32_t)pid_spd.lower_limit_output;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_START_CURRENT:
    wData = (int32_t)0;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_START_PERIOD:
    wData = (int32_t)0;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_EMF_OFF_OFFSET_RISE:
    wData = (int32_t)0;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_EMF_OFF_OFFSET_FALL:
    wData = (int32_t)0;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_FLUX_WEK_KP:
    wData = (int32_t)field_weakening.pid_fw.kp_gain;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_FLUX_WEK_KI:
    wData = (int32_t)field_weakening.pid_fw.ki_gain;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_MAX_FLUX_WEK_I:
    wData = (int32_t)field_weakening.Id_max;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

//  case MC_PROTOCOL_REG_FLUX_WEK_BUS_V_PERCENT:
//    wData = (int32_t)(field_weakening.volt_limit * 1000 / field_weakening.max_module);
//    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
//    break;

  case MC_PROTOCOL_REG_OBS_C1:
    wData = (int32_t)state_observer.c2;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_OBS_C2:
    wData = (int32_t)state_observer.c4;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_PLL_KP:
    wData = (int32_t)state_observer.pid_pll.kp_gain;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_PLL_KI:
    wData = (int32_t)state_observer.pid_pll.ki_gain;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_PLL_KP_DIV:
    wData = (int32_t)state_observer.pid_pll.kp_shift;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_PLL_KI_DIV:
    wData = (int32_t)state_observer.pid_pll.ki_shift;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_OPEN_LOOP_VOLT:
    wData = (int32_t)openloop.volt.q;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_OPEN_LOOP_ANGLE_INCRE:
    wData = (int32_t)openloop.inc;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 8);
    break;

  case MC_PROTOCOL_REG_ENCODER_ALIGN_VOLT:
    wData = (int32_t)encoder.volt.q;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_ENCODER_OFFSET:
    wData = (int32_t)encoder.offset;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 32);
    break;

  case MC_PROTOCOL_REG_ERROR_CODE:
    wData = (int32_t)error_code;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 8);
    break;

  case MC_PROTOCOL_REG_IA:
    wData = (int32_t)current.Iabc.a;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_IB:
    wData = (int32_t)current.Iabc.b;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_IC:
    wData = (int32_t)current.Iabc.c;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_I_ALPHA:
    wData = (int32_t)current.Ialphabeta.alpha;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_I_BETA:
    wData = (int32_t)current.Ialphabeta.beta;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_IQ_FILTER:
    wData = (int32_t)current.Iqd_LPF.q;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_ID_FILTER:
    wData = (int32_t)current.Iqd_LPF.d;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_VQ:
    wData = (int32_t)volt_cmd.Vqd.q;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_VD:
    wData = (int32_t)volt_cmd.Vqd.d;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_V_ALPHA:
    wData = (int32_t)volt_cmd.Valphabeta.alpha;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_V_BETA:
    wData = (int32_t)volt_cmd.Valphabeta.beta;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_ELEC_ANGLE_MEAS:
    wData = (int32_t)elec_angle_val;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_ROTOR_SPEED_MEAS_FILTER:
    wData = (int32_t)rotor_speed_val_filt;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_OBS_ELEC_ANGLE:
    wData = (int32_t)state_observer.elec_angle;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_OBS_ROTOR_SPEED:
    wData = (int32_t)state_observer.motor_speed.filtered;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_OBS_I_ALPHA:
    wData = (int32_t)state_observer.hIalpha_est;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_OBS_I_BETA:
    wData = (int32_t)state_observer.hIbeta_est;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_OBS_BEMF_ALPHA:
    wData = (int32_t)state_observer.hBemf_alpha_est;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_OBS_BEMF_BETA:
    wData = (int32_t)state_observer.hBemf_beta_est;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_VQ_REF:
    wData = (int32_t)volt_cmd.Vqd.q;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_VD_REF:
    wData = (int32_t)volt_cmd.Vqd.d;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_STARTUP_MAX_SPD:
    wData = (int32_t)startup.max_speed;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

//  case MC_PROTOCOL_REG_STARTUP_OPEN_LOOP_VOLT:
//    wData = (int32_t)startup.ol_volt;
//    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
//    break;

  case MC_PROTOCOL_REG_STARTUP_OPEN_LOOP_SLOPE:
    wData = (int32_t)(startup.elec_spd_to_rpm * obs_speed_LPF.sample_freq / startup.ol_delay_count);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_STARTUP_ALIGN_TIME:
    wData = (int32_t)(startup.align_count * 1000 / obs_speed_LPF.sample_freq);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

//  case MC_PROTOCOL_REG_STARTUP_ALIGN_VOLT:
//    wData = (int32_t)startup.align_volt;
//    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
//    break;

  case MC_PROTOCOL_REG_STARTUP_START_TIME:
    wData = (int32_t)(startup.start_count * 1000 / obs_speed_LPF.sample_freq);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_FLUX_WEK_KP_DIV:
    wData = (int32_t)field_weakening.pid_fw.kp_shift;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_FLUX_WEK_KI_DIV:
    wData = (int32_t)field_weakening.pid_fw.ki_shift;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_EMF_ON_OFFSET_RISE:
    wData = (int32_t)0;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_EMF_ON_OFFSET_FALL:
    wData = (int32_t)0;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_STARTUP_START_CURRENT:
    wData = (int32_t)startup.start_current;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_ENCODER_RESOLUTION:
    wData = (int32_t)(intCoeffs32[MC_PROTOCOL_REG_ENCODER_RESOLUTION]);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 32);
    break;

  case MC_PROTOCOL_REG_MAX_APP_ANGLE:
    wData = (int32_t)(intCoeffs32[MC_PROTOCOL_REG_MAX_APP_ANGLE]);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 32);
    break;

  case MC_PROTOCOL_REG_MIN_APP_ANGLE:
    wData = (int32_t)(intCoeffs32[MC_PROTOCOL_REG_MIN_APP_ANGLE]);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 32);
    break;

  case MC_PROTOCOL_REG_POSITION_REF:
    wData = (int32_t) angle.cmd_final;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 32);
    break;

  case MC_PROTOCOL_REG_POSITION_KP:
    wData = (int32_t)pid_pos.kp_gain;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_POSITION_KI:
    wData = (int32_t)pid_pos.ki_gain_1st;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_POSITION_KI_STABLE:
    wData = (int32_t)pid_pos.ki_gain_2nd;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_POSITION_KD:
    wData = (int32_t)pid_pos.kd_gain;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_POSITION_KP_DIV:
    wData = (int32_t)pid_pos.kp_shift;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_POSITION_KI_DIV:
    wData = (int32_t)pid_pos.ki_shift;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_POSITION_KD_DIV:
    wData = (int32_t)pid_pos.kd_shift;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_POSITION_MEAS:
    wData = (int32_t) angle.val;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 32);
    break;

  case MC_PROTOCOL_REG_ENCODER_ADJUST:
    wData = (int32_t)0;//encoder.adjust;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, wData, 16);
    break;

  case MC_PROTOCOL_REG_ENCODER_ALIGN:
    wData = (int32_t)encoder.align;
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
    ctrl_mode_cmd = (motor_control_mode)wData;
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
    if(wData > MAX_CURRENT_PU)// || wData < -MIN_CURRENT_PU)
    {
      bErrorCode = ERROR_CODE_WRONG_SET;
      TCP_SendFrame(&tx_data_response, TCP_CODE_NACK, bErrorCode, 8);
    }
    else
    {
#ifdef E_BIKE_SCOOTER
      Iq_ref_cmd = (int16_t)wData;
      I_ref.q = Iq_ref_cmd;
#else
      current.Iqdref.q  = (int16_t)wData;
#endif
      TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    }

    break;

  case MC_PROTOCOL_REG_TORQUE_KP:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_iq.kp_gain = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_TORQUE_KI:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_iq.ki_gain = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_TORQUE_KP_DIV:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_iq.kp_shift = (uint16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_TORQUE_KI_DIV:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_iq.ki_shift = (uint16_t)wData;
    pid_iq.upper_limit_integral = (int32_t)(pid_iq.upper_limit_output << pid_iq.ki_shift);
    pid_iq.lower_limit_integral = (int32_t)(-(-pid_iq.lower_limit_output << pid_iq.ki_shift));
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_FLUX_REF:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
#ifdef E_BIKE_SCOOTER
    I_ref.d = (int16_t)wData;
#else
    current.Iqdref.d = (int16_t)wData;
#endif
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_FLUX_KP:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_id.kp_gain = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_FLUX_KI:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_id.ki_gain = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_FLUX_KP_DIV:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_id.kp_shift = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_FLUX_KI_DIV:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_id.ki_shift = (int16_t)wData;
    pid_id.upper_limit_integral = (int32_t)(pid_id.upper_limit_output << pid_id.ki_shift);
    pid_id.lower_limit_integral = (int32_t)(-(-pid_id.lower_limit_output << pid_id.ki_shift));
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

//    case MC_PROTOCOL_REG_MAX_MOTOR_POWER:
//        wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
//        break;
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
#ifndef E_BIKE_SCOOTER
    pid_spd.upper_limit_output = (int16_t)wData;
    pid_spd.upper_limit_integral = (int32_t)(wData << pid_spd.ki_shift);
#endif
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_MIN_CURRENT:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
#ifndef E_BIKE_SCOOTER
    pid_spd.lower_limit_output = (int16_t)wData;
    pid_spd.lower_limit_integral = (int32_t)(-(-pid_spd.lower_limit_output << pid_spd.ki_shift));
#endif
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_FLUX_WEK_KP:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    field_weakening.pid_fw.kp_gain = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_FLUX_WEK_KI:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    field_weakening.pid_fw.ki_gain = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_MAX_FLUX_WEK_I:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    field_weakening.Id_max = (int16_t)wData;
    field_weakening.pid_fw.lower_limit_output = (int16_t) - wData;
    field_weakening.pid_fw.lower_limit_integral = (int32_t) - (wData << field_weakening.pid_fw.ki_shift);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

//  case MC_PROTOCOL_REG_FLUX_WEK_BUS_V_PERCENT:
//    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
//    field_weakening.volt_limit = (int32_t)wData * field_weakening.max_module / 1000;
//    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
//    break;

  case MC_PROTOCOL_REG_OBS_C1:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    state_observer.c2 = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_OBS_C2:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    state_observer.c4 = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_PLL_KP:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    state_observer.pid_pll.kp_gain = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_PLL_KI:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    state_observer.pid_pll.ki_gain = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_PLL_KP_DIV:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    state_observer.pid_pll.kp_shift = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_PLL_KI_DIV:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    state_observer.pid_pll.ki_shift = (int16_t)wData;
    state_observer.pid_pll.upper_limit_integral = (int32_t)(INT16_MAX << state_observer.pid_pll.ki_shift);
    state_observer.pid_pll.lower_limit_integral = (int32_t)(-(INT16_MAX << state_observer.pid_pll.ki_shift));
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_OPEN_LOOP_VOLT:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    openloop.volt.q = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_OPEN_LOOP_ANGLE_INCRE:
    wData = pFrame -> Buffer[1];
    openloop.inc = (uint8_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_ENCODER_ALIGN_VOLT:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    encoder.volt.q = (int16_t)wData;
    encoder.volt.d = (int16_t)(wData * 2);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_VQ_REF:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    volt_cmd.Vqd.q = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_VD_REF:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    volt_cmd.Vqd.d = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_STARTUP_MAX_SPD:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    startup.max_speed = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

//  case MC_PROTOCOL_REG_STARTUP_OPEN_LOOP_VOLT:
//    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
//    startup.ol_volt = (int16_t)wData;
//    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
//    break;

  case MC_PROTOCOL_REG_STARTUP_OPEN_LOOP_SLOPE:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    startup.ol_delay_count = ((uint16_t)((uint32_t)startup.elec_spd_to_rpm * obs_speed_LPF.sample_freq / wData));
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_STARTUP_ALIGN_TIME:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    startup.align_count = ((uint32_t)wData * obs_speed_LPF.sample_freq / 1000);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

//  case MC_PROTOCOL_REG_STARTUP_ALIGN_VOLT:
//    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
//    startup.align_volt = (int16_t)wData;
//    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
//    break;

  case MC_PROTOCOL_REG_STARTUP_START_TIME:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    startup.start_count = ((uint32_t)wData * obs_speed_LPF.sample_freq / 1000);
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_FLUX_WEK_KP_DIV:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    field_weakening.pid_fw.kp_shift = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_FLUX_WEK_KI_DIV:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    field_weakening.pid_fw.ki_shift = (int16_t)wData;
    field_weakening.pid_fw.lower_limit_integral = (int32_t)(-(-field_weakening.pid_fw.lower_limit_output << field_weakening.pid_fw.ki_shift));
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_STARTUP_START_CURRENT:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    startup.start_current = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);

  case MC_PROTOCOL_REG_POSITION_REF:
    wData = (int32_t)(pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8) + ((pFrame -> Buffer[3]) << 16) + ((pFrame -> Buffer[4]) << 24));
    angle.cmd_final = (int32_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_POSITION_KP:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_pos.kp_gain = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_POSITION_KI:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_pos.ki_gain_1st = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_POSITION_KI_STABLE:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_pos.ki_gain_2nd = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_POSITION_KD:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_pos.kd_gain = (int16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_POSITION_KP_DIV:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_pos.kp_shift = (uint16_t)wData;
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_POSITION_KI_DIV:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_pos.ki_shift = (uint16_t)wData;
    pid_pos.upper_limit_integral = (int32_t)(pid_pos.upper_limit_output << pid_pos.ki_shift);
    pid_pos.lower_limit_integral = (int32_t)(-(-pid_pos.lower_limit_output << pid_pos.ki_shift));
    TCP_SendFrame(&tx_data_response, TCP_CODE_ACK, 0, 0);
    break;

  case MC_PROTOCOL_REG_POSITION_KD_DIV:
    wData = pFrame -> Buffer[1] + ((pFrame -> Buffer[2]) << 8);
    pid_pos.kd_shift = (uint16_t)wData;
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
#ifdef E_BIKE_SCOOTER
    monitor_data[channel] = &I_ref.q;
#else
    monitor_data[channel] = &current.Iqdref.q;
#endif
    break;

  case MC_PROTOCOL_REG_FLUX_REF:
#ifdef E_BIKE_SCOOTER
    monitor_data[channel] = &I_ref.d;
#else
    monitor_data[channel] = &current.Iqdref.d;
#endif
    break;

  case MC_PROTOCOL_REG_BUS_VOLTAGE_MEAS:
    monitor_data[channel] = &ui_wave_param.iBusVoltage_meas;
    break;

  case MC_PROTOCOL_REG_MOS_TEMP_MEAS:
    monitor_data[channel] = &ui_wave_param.iMosTemperature_meas;
    break;

  case MC_PROTOCOL_REG_TORQUE_MEAS:
    monitor_data[channel] = &current.Iqd.q;
    break;

  case MC_PROTOCOL_REG_FLUX_MEAS:
    monitor_data[channel] = &current.Iqd.d;
    break;

  case MC_PROTOCOL_REG_IA:
    monitor_data[channel] = &current.Iabc.a;
    break;

  case MC_PROTOCOL_REG_IB:
    monitor_data[channel] = &current.Iabc.b;
    break;

  case MC_PROTOCOL_REG_IC:
    monitor_data[channel] = &current.Iabc.c;
    break;

  case MC_PROTOCOL_REG_I_ALPHA:
    monitor_data[channel] = &current.Ialphabeta.alpha;
    break;

  case MC_PROTOCOL_REG_I_BETA:
    monitor_data[channel] = &current.Ialphabeta.beta;
    break;

  case MC_PROTOCOL_REG_IQ_FILTER:
    monitor_data[channel] = &current.Iqd_LPF.q;
    break;

  case MC_PROTOCOL_REG_ID_FILTER:
    monitor_data[channel] = &current.Iqd_LPF.d;
    break;

  case MC_PROTOCOL_REG_VQ:
    monitor_data[channel] = &volt_cmd.Vqd.q;
    break;

  case MC_PROTOCOL_REG_VD:
    monitor_data[channel] = &volt_cmd.Vqd.d;
    break;

  case MC_PROTOCOL_REG_V_ALPHA:
    monitor_data[channel] = &volt_cmd.Valphabeta.alpha;
    break;

  case MC_PROTOCOL_REG_V_BETA:
    monitor_data[channel] = &volt_cmd.Valphabeta.beta;
    break;

  case MC_PROTOCOL_REG_ELEC_ANGLE_MEAS:
    monitor_data[channel] = &elec_angle_val;
    break;

  case MC_PROTOCOL_REG_ROTOR_SPEED_MEAS_FILTER:
    monitor_data[channel] = (int16_t*)(&rotor_speed_val_filt);
    break;

  case MC_PROTOCOL_REG_OBS_ELEC_ANGLE:
    monitor_data[channel] = &state_observer.elec_angle;
    break;

  case MC_PROTOCOL_REG_OBS_ROTOR_SPEED:
    monitor_data[channel] = (int16_t*)(&state_observer.motor_speed.filtered);
    break;

  case MC_PROTOCOL_REG_OBS_I_ALPHA:
    monitor_data[channel] = &state_observer.hIalpha_est;
    break;

  case MC_PROTOCOL_REG_OBS_I_BETA:
    monitor_data[channel] = &state_observer.hIbeta_est;
    break;

  case MC_PROTOCOL_REG_OBS_BEMF_ALPHA:
    monitor_data[channel] = &state_observer.hBemf_alpha_est;
    break;

  case MC_PROTOCOL_REG_OBS_BEMF_BETA:
    monitor_data[channel] = &state_observer.hBemf_beta_est;
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

  case MC_PROTOCOL_REG_POS_REF_PU:
    monitor_data[channel] = &ui_wave_param.position_reference_pu;
    break;

  case MC_PROTOCOL_REG_POS_MEAS_PU:
    monitor_data[channel] = &ui_wave_param.position_meas_pu;
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
  save_param_buffer[MC_PROTOCOL_REG_CONTROL_MODE] = 0; //ctrl_mode;
  save_param_buffer[MC_PROTOCOL_REG_CTRL_SOURCE] = ctrl_source;
  save_param_buffer[MC_PROTOCOL_REG_SPEED_REF] = speed_ramp.cmd_final;
  save_param_buffer[MC_PROTOCOL_REG_SPEED_KP] = pid_spd.kp_gain;
  save_param_buffer[MC_PROTOCOL_REG_SPEED_KI] = pid_spd.ki_gain;
  save_param_buffer[MC_PROTOCOL_REG_SPEED_KP_DIV] = pid_spd.kp_shift;
  save_param_buffer[MC_PROTOCOL_REG_SPEED_KI_DIV] = pid_spd.ki_shift;
#ifdef E_BIKE_SCOOTER
  save_param_buffer[MC_PROTOCOL_REG_TORQUE_REF] = I_ref.q;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_REF] = I_ref.d;
#else
  save_param_buffer[MC_PROTOCOL_REG_TORQUE_REF] = current.Iqdref.q;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_REF] = current.Iqdref.d;
#endif
  save_param_buffer[MC_PROTOCOL_REG_TORQUE_KP] = pid_iq.kp_gain;
  save_param_buffer[MC_PROTOCOL_REG_TORQUE_KI] = pid_iq.ki_gain;
  save_param_buffer[MC_PROTOCOL_REG_TORQUE_KP_DIV] = pid_iq.kp_shift;
  save_param_buffer[MC_PROTOCOL_REG_TORQUE_KI_DIV] = pid_iq.ki_shift;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_KP] = pid_id.kp_gain;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_KI] = pid_iq.ki_gain;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_KP_DIV] = pid_id.kp_shift;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_KI_DIV] = pid_id.ki_shift;
  save_param_buffer[MC_PROTOCOL_REG_BUS_VOLTAGE_MEAS] = 0;
  save_param_buffer[MC_PROTOCOL_REG_MOS_TEMP_MEAS] = 0;
  save_param_buffer[MC_PROTOCOL_REG_MOTOR_POWER_MEAS] = 0;
  save_param_buffer[MC_PROTOCOL_REG_MAX_MOTOR_POWER] = 0;
  save_param_buffer[MC_PROTOCOL_REG_SPEED_MEAS] = rotor_speed_val_filt;
  save_param_buffer[MC_PROTOCOL_REG_TORQUE_MEAS] = current.Iqd.q;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_MEAS] = current.Iqd.d;
  save_param_buffer[MC_PROTOCOL_REG_MAX_APP_SPEED] = MAX_SPEED_RPM;
  save_param_buffer[MC_PROTOCOL_REG_MIN_APP_SPEED] = MIN_CONTROL_SPEED;
  save_param_buffer[MC_PROTOCOL_REG_SPEED_ACC] = speed_ramp.acc_slope;
  save_param_buffer[MC_PROTOCOL_REG_SPEED_DEC] = speed_ramp.dec_slope;
#ifdef E_BIKE_SCOOTER
  save_param_buffer[MC_PROTOCOL_REG_MAX_CURRENT] = MAX_CURRENT_PU;
  save_param_buffer[MC_PROTOCOL_REG_MIN_CURRENT] = MIN_CURRENT_PU;
#else
  save_param_buffer[MC_PROTOCOL_REG_MAX_CURRENT] = pid_spd.upper_limit_output;
  save_param_buffer[MC_PROTOCOL_REG_MIN_CURRENT] = pid_spd.lower_limit_output;
#endif
  save_param_buffer[MC_PROTOCOL_REG_START_CURRENT] = 0;
  save_param_buffer[MC_PROTOCOL_REG_START_PERIOD] = 0;
  save_param_buffer[MC_PROTOCOL_REG_EMF_OFF_OFFSET_RISE] = 0;
  save_param_buffer[MC_PROTOCOL_REG_EMF_OFF_OFFSET_FALL] = 0;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_WEK_KP] = field_weakening.pid_fw.kp_gain;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_WEK_KI] = field_weakening.pid_fw.ki_gain;
  save_param_buffer[MC_PROTOCOL_REG_MAX_FLUX_WEK_I] = field_weakening.Id_max;
  //save_param_buffer[MC_PROTOCOL_REG_FLUX_WEK_BUS_V_PERCENT] = (int32_t)(field_weakening.volt_limit * 1000 / field_weakening.max_module);
  save_param_buffer[MC_PROTOCOL_REG_OBS_C1] = state_observer.c2;
  save_param_buffer[MC_PROTOCOL_REG_OBS_C2] = state_observer.c4;
  save_param_buffer[MC_PROTOCOL_REG_PLL_KP] = state_observer.pid_pll.kp_gain;
  save_param_buffer[MC_PROTOCOL_REG_PLL_KI] = state_observer.pid_pll.ki_gain;
  save_param_buffer[MC_PROTOCOL_REG_PLL_KP_DIV] = state_observer.pid_pll.kp_shift;
  save_param_buffer[MC_PROTOCOL_REG_PLL_KI_DIV] = state_observer.pid_pll.ki_shift;
  save_param_buffer[MC_PROTOCOL_REG_OPEN_LOOP_VOLT] = openloop.volt.q;
  save_param_buffer[MC_PROTOCOL_REG_OPEN_LOOP_ANGLE_INCRE] = openloop.inc;
  save_param_buffer[MC_PROTOCOL_REG_ENCODER_ALIGN_VOLT] = encoder.volt.q;
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
  save_param_buffer[MC_PROTOCOL_REG_VQ_REF] = volt_cmd.Vqd.q;
  save_param_buffer[MC_PROTOCOL_REG_VD_REF] = volt_cmd.Vqd.d;
  save_param_buffer[MC_PROTOCOL_REG_STARTUP_MAX_SPD] = startup.max_speed ;
  //save_param_buffer[MC_PROTOCOL_REG_STARTUP_OPEN_LOOP_VOLT] = startup.ol_volt;
  save_param_buffer[MC_PROTOCOL_REG_STARTUP_OPEN_LOOP_SLOPE] = ((uint16_t)((uint32_t)startup.elec_spd_to_rpm * obs_speed_LPF.sample_freq / startup.ol_delay_count));
  save_param_buffer[MC_PROTOCOL_REG_STARTUP_ALIGN_TIME] = (int32_t)startup.align_count * 1000 / obs_speed_LPF.sample_freq;
  //save_param_buffer[MC_PROTOCOL_REG_STARTUP_ALIGN_VOLT] = startup.align_volt;
  save_param_buffer[MC_PROTOCOL_REG_STARTUP_START_TIME] = (int32_t)startup.start_count * 1000 / obs_speed_LPF.sample_freq;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_WEK_KP_DIV] = field_weakening.pid_fw.kp_shift;
  save_param_buffer[MC_PROTOCOL_REG_FLUX_WEK_KI_DIV] = field_weakening.pid_fw.ki_shift;
  save_param_buffer[MC_PROTOCOL_REG_EMF_ON_OFFSET_RISE] = 0;
  save_param_buffer[MC_PROTOCOL_REG_EMF_ON_OFFSET_FALL] = 0;
  save_param_buffer[MC_PROTOCOL_REG_STARTUP_START_CURRENT] = (int32_t)startup.start_current;
  save_param_buffer[MC_PROTOCOL_REG_SPD_REF_PU] = 0;
  save_param_buffer[MC_PROTOCOL_REG_SPD_MEAS_PU] = 0;
  save_param_buffer[MC_PROTOCOL_REG_USER_DEFINED_A] = 0;
  save_param_buffer[MC_PROTOCOL_REG_USER_DEFINED_B] = 0;
  save_param_buffer[MC_PROTOCOL_REG_ENCODER_RESOLUTION] = ENC_CPR_NBR;
  save_param_buffer[MC_PROTOCOL_REG_MAX_APP_ANGLE] = MAX_POSITION_ANGLE;
  save_param_buffer[MC_PROTOCOL_REG_MIN_APP_ANGLE] = MIN_POSITION_ANGLE;
  save_param_buffer[MC_PROTOCOL_REG_POS_REF_PU] = 0;
  save_param_buffer[MC_PROTOCOL_REG_POS_MEAS_PU] = 0;
  save_param_buffer[MC_PROTOCOL_REG_POSITION_REF] = angle.cmd_final;
  save_param_buffer[MC_PROTOCOL_REG_POSITION_KP] = pid_pos.kp_gain;
  save_param_buffer[MC_PROTOCOL_REG_POSITION_KI] = pid_pos.ki_gain_1st;
  save_param_buffer[MC_PROTOCOL_REG_POSITION_KI_STABLE] = pid_pos.ki_gain_2nd;
  save_param_buffer[MC_PROTOCOL_REG_POSITION_KD] = pid_pos.kd_gain;
  save_param_buffer[MC_PROTOCOL_REG_POSITION_KP_DIV] = pid_pos.kp_shift;
  save_param_buffer[MC_PROTOCOL_REG_POSITION_KI_DIV] = pid_pos.ki_shift;
  save_param_buffer[MC_PROTOCOL_REG_POSITION_KD_DIV] = pid_pos.kd_shift;
  save_param_buffer[MC_PROTOCOL_REG_POSITION_MEAS] = angle.val;
#if defined MAGNET_ENCODER_W_ABZ
  save_param_buffer[MC_PROTOCOL_REG_ENCODER_ADJUST] = 0;//encoder.adjust;
  save_param_buffer[MC_PROTOCOL_REG_ENCODER_ALIGN] = encoder.align;
  save_param_buffer[MC_PROTOCOL_REG_ENCODER_OFFSET] = encoder.offset;
#endif
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
  * @brief  read data using halfword mode
  * @param  read_addr: the address of reading
  * @param  p_buffer: the buffer of reading data
  * @param  num_read: the number of reading data
  * @retval none
  */
void flash_read(uint32_t read_addr, int32_t *p_buffer, uint16_t num_read)
{
  uint16_t i;
  for(i = 0; i < num_read; i++)
  {
    p_buffer[i] = *(int32_t*)(read_addr);
    read_addr += 4;
  }
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


