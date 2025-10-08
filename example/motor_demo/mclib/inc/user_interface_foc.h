/**
  **************************************************************************
  * @file     user_interface_foc.h
  * @brief    Declaration of communication interface related functions
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

#ifndef __USER_INTERFACE_FOC_H
#define __USER_INTERFACE_FOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

/** @brief Not initialized pointer */
#define MC_NULL    (uint16_t)(0x0000u)

/** @brief Size of the Header of an Communication Protocol frame */
#define CP_HEADER_SIZE       1
/** @brief Size of the payload length of an Communication Protocol frame */
#define CP_LENGTH_SIZE       1
/** @brief Maximum size of the payload of an Communication Protocol frame(RX) */
#define CP_RX_MAX_PAYLOAD_SIZE  10
/** @brief Size of the Code of an Communication Protocol frame */
#define CP_CODE_SIZE         1
/** @brief Size of the Control Sum (actually not a CRC) of an Communication Protocol frame */
#define CP_CRC_SIZE          1
/** @brief Maximum size of an Communication Protocol frame, all inclusive */
#define RCP_MAX_FRAME_SIZE   (CP_HEADER_SIZE + CP_RX_MAX_PAYLOAD_SIZE + CP_CRC_SIZE)

#define DATA_BUFFER_SYNC_SIZE    (10)
#define DATA_BUFFER_SIZE         (2048)
#define DATA_BUFFER_EXTRA_SIZE   (36)
#define DATA_BUFFER_FRAME_SIZE   (DATA_BUFFER_SYNC_SIZE + DATA_BUFFER_SIZE + DATA_BUFFER_EXTRA_SIZE)

#define FW_VER_SIZE              (32)
#define FW_VER_LEN_SIZE          (32*8)

#define TCP_CODE_ACK             (0xF0)
#define TCP_CODE_NACK            (0xE0)

#define TCP_LEN_NONE             (0)
#define TCP_LEN_BYTE             (8)
#define TCP_LEN_HALF_WORD        (16)
#define TCP_LEN_WORD             (32)

#define RCP_FRAME_START_ADRESS   (CP_HEADER_SIZE + CP_LENGTH_SIZE + CP_CODE_SIZE)
/* Exported types ------------------------------------------------------------*/
/**
 * @brief Motor control protocol registers
 *
 * Enumerates the variables and information that can be exchanged across the Motor Control Protocol
 */
typedef enum
{
  MC_PROTOCOL_REG_NONE,
  MC_PROTOCOL_REG_CURRENT_BASE,              /* 1   */
  MC_PROTOCOL_REG_VOLTAGE_BASE,              /* 2   */
  MC_PROTOCOL_REG_FIRMWARE_ID,               /* 3   */
  MC_PROTOCOL_REG_CURRENT_TUNE_TARGET_I,     /* 4   */
  MC_PROTOCOL_REG_CURRENT_TUNE_TOTAL_PERIOD, /* 5   */
  MC_PROTOCOL_REG_CURRENT_TUNE_STEP_PERIOD,  /* 6   */
  MC_PROTOCOL_REG_ESC_STATUS,                /* 7   */
  MC_PROTOCOL_REG_CONTROL_MODE,              /* 8   */
  MC_PROTOCOL_REG_CTRL_SOURCE,               /* 9   */
  MC_PROTOCOL_REG_SPEED_REF,                 /* 10  */
  MC_PROTOCOL_REG_SPEED_KP,                  /* 11  */
  MC_PROTOCOL_REG_SPEED_KI,                  /* 12  */
  MC_PROTOCOL_REG_SPEED_KP_DIV,              /* 13  */
  MC_PROTOCOL_REG_SPEED_KI_DIV,              /* 14  */
  MC_PROTOCOL_REG_TORQUE_REF,                /* 15  */
  MC_PROTOCOL_REG_TORQUE_KP,                 /* 16  */
  MC_PROTOCOL_REG_TORQUE_KI,                 /* 17  */
  MC_PROTOCOL_REG_TORQUE_KP_DIV,             /* 18  */
  MC_PROTOCOL_REG_TORQUE_KI_DIV,             /* 19  */
  MC_PROTOCOL_REG_FLUX_REF,                  /* 20  */
  MC_PROTOCOL_REG_FLUX_KP,                   /* 21  */
  MC_PROTOCOL_REG_FLUX_KI,                   /* 22  */
  MC_PROTOCOL_REG_FLUX_KP_DIV,               /* 23  */
  MC_PROTOCOL_REG_FLUX_KI_DIV,               /* 24  */
  MC_PROTOCOL_REG_BUS_VOLTAGE_MEAS,          /* 25  */
  MC_PROTOCOL_REG_MOS_TEMP_MEAS,             /* 26  */
  MC_PROTOCOL_REG_MOTOR_POWER_MEAS,          /* 27  */
  MC_PROTOCOL_REG_MAX_MOTOR_POWER,           /* 28  */
  MC_PROTOCOL_REG_SPEED_MEAS,                /* 29  */
  MC_PROTOCOL_REG_TORQUE_MEAS,               /* 30  */
  MC_PROTOCOL_REG_FLUX_MEAS,                 /* 31  */
  MC_PROTOCOL_REG_MAX_APP_SPEED,             /* 32  */
  MC_PROTOCOL_REG_MIN_APP_SPEED,             /* 33  */
  MC_PROTOCOL_REG_SPEED_ACC,                 /* 34  */
  MC_PROTOCOL_REG_SPEED_DEC,                 /* 35  */
  MC_PROTOCOL_REG_MAX_CURRENT,               /* 36  */
  MC_PROTOCOL_REG_MIN_CURRENT,               /* 37  */
  MC_PROTOCOL_REG_START_CURRENT,             /* 38  */
  MC_PROTOCOL_REG_START_PERIOD,              /* 39  */
  MC_PROTOCOL_REG_EMF_OFF_OFFSET_RISE,       /* 40  */
  MC_PROTOCOL_REG_EMF_OFF_OFFSET_FALL,       /* 41  */
  MC_PROTOCOL_REG_FLUX_WEK_KP,               /* 42  */
  MC_PROTOCOL_REG_FLUX_WEK_KI,               /* 43  */
  MC_PROTOCOL_REG_MAX_FLUX_WEK_I,            /* 44  */
  MC_PROTOCOL_REG_FLUX_WEK_BUS_V_PERCENT,    /* 45  */
  MC_PROTOCOL_REG_OBS_C1,                    /* 46  */
  MC_PROTOCOL_REG_OBS_C2,                    /* 47  */
  MC_PROTOCOL_REG_PLL_KP,                    /* 48  */
  MC_PROTOCOL_REG_PLL_KI,                    /* 49  */
  MC_PROTOCOL_REG_PLL_KP_DIV,                /* 50  */
  MC_PROTOCOL_REG_PLL_KI_DIV,                /* 51  */
  MC_PROTOCOL_REG_OPEN_LOOP_VOLT,            /* 52  */
  MC_PROTOCOL_REG_OPEN_LOOP_ANGLE_INCRE,     /* 53  */
  MC_PROTOCOL_REG_ENCODER_ALIGN_VOLT,        /* 54  */
  MC_PROTOCOL_REG_ENCODER_OFFSET,            /* 55  */
  MC_PROTOCOL_REG_ERROR_CODE,                /* 56  */
  MC_PROTOCOL_REG_IA,                        /* 57  */
  MC_PROTOCOL_REG_IB,                        /* 58  */
  MC_PROTOCOL_REG_IC,                        /* 59  */
  MC_PROTOCOL_REG_I_ALPHA,                   /* 60  */
  MC_PROTOCOL_REG_I_BETA,                    /* 61  */
  MC_PROTOCOL_REG_IQ_FILTER,                 /* 62  */
  MC_PROTOCOL_REG_ID_FILTER,                 /* 63  */
  MC_PROTOCOL_REG_VQ,                        /* 64  */
  MC_PROTOCOL_REG_VD,                        /* 65  */
  MC_PROTOCOL_REG_V_ALPHA,                   /* 66  */
  MC_PROTOCOL_REG_V_BETA,                    /* 67  */
  MC_PROTOCOL_REG_ELEC_ANGLE_MEAS,           /* 68  */
  MC_PROTOCOL_REG_ROTOR_SPEED_MEAS_FILTER,   /* 69  */
  MC_PROTOCOL_REG_OBS_ELEC_ANGLE,            /* 70  */
  MC_PROTOCOL_REG_OBS_ROTOR_SPEED,           /* 71  */
  MC_PROTOCOL_REG_OBS_I_ALPHA,               /* 72  */
  MC_PROTOCOL_REG_OBS_I_BETA,                /* 73  */
  MC_PROTOCOL_REG_OBS_BEMF_ALPHA,            /* 74  */
  MC_PROTOCOL_REG_OBS_BEMF_BETA,             /* 75  */
  MC_PROTOCOL_REG_VQ_REF,                    /* 76  */
  MC_PROTOCOL_REG_VD_REF,                    /* 77  */
  MC_PROTOCOL_REG_STARTUP_MAX_SPD,           /* 78  */
  MC_PROTOCOL_REG_STARTUP_OPEN_LOOP_VOLT,    /* 79  */
  MC_PROTOCOL_REG_STARTUP_OPEN_LOOP_SLOPE,   /* 80  */
  MC_PROTOCOL_REG_STARTUP_ALIGN_TIME,        /* 81  */
  MC_PROTOCOL_REG_STARTUP_ALIGN_VOLT,        /* 82  */
  MC_PROTOCOL_REG_STARTUP_START_TIME,        /* 83  */
  MC_PROTOCOL_REG_FLUX_WEK_KP_DIV,           /* 84  */
  MC_PROTOCOL_REG_FLUX_WEK_KI_DIV,           /* 85  */
  MC_PROTOCOL_REG_EMF_ON_OFFSET_RISE,        /* 86  */
  MC_PROTOCOL_REG_EMF_ON_OFFSET_FALL,        /* 87  */
  MC_PROTOCOL_REG_STARTUP_START_CURRENT,     /* 88  */
  MC_PROTOCOL_REG_SPD_REF_PU,                /* 89  */
  MC_PROTOCOL_REG_SPD_MEAS_PU,               /* 90  */
  MC_PROTOCOL_REG_USER_DEFINED_A,            /* 91  */
  MC_PROTOCOL_REG_USER_DEFINED_B,            /* 92  */
  MC_PROTOCOL_REG_ENCODER_RESOLUTION,        /* 93  */
  MC_PROTOCOL_REG_MAX_APP_ANGLE,             /* 94  */
  MC_PROTOCOL_REG_MIN_APP_ANGLE,             /* 95  */
  MC_PROTOCOL_REG_POS_REF_PU,                /* 96  */
  MC_PROTOCOL_REG_POS_MEAS_PU,               /* 97  */
  MC_PROTOCOL_REG_POSITION_REF,              /* 98  */
  MC_PROTOCOL_REG_POSITION_KP,               /* 99  */
  MC_PROTOCOL_REG_POSITION_KI,               /* 100 */
  MC_PROTOCOL_REG_POSITION_KI_STABLE,        /* 101 */
  MC_PROTOCOL_REG_POSITION_KD,               /* 102 */
  MC_PROTOCOL_REG_POSITION_KP_DIV,           /* 103 */
  MC_PROTOCOL_REG_POSITION_KI_DIV,           /* 104 */
  MC_PROTOCOL_REG_POSITION_KD_DIV,           /* 105 */
  MC_PROTOCOL_REG_POSITION_MEAS,             /* 106 */
  MC_PROTOCOL_REG_ENCODER_ADJUST,            /* 107 */
  MC_PROTOCOL_REG_ENCODER_ALIGN,             /* 108 */
  MC_PROTOCOL_REG_OLC_INIT_VOLT,             /* 109 */
  MC_PROTOCOL_REG_OLC_INIT_SPD,              /* 110 */
  MC_PROTOCOL_REG_OLC_VOLT_INC,              /* 111 */
  MC_PROTOCOL_REG_OLC_FINAL_SPD,             /* 112 */
  MC_PROTOCOL_REG_OLC_TIMES,                 /* 113 */
  MC_PROTOCOL_REG_START_VOLT,                /* 114 */
  MC_PROTOCOL_REG_HALL_LEARN_0_STATE,        /* 115 */
  MC_PROTOCOL_REG_HALL_LEARN_1_STATE,        /* 116 */
  MC_PROTOCOL_REG_HALL_LEARN_2_STATE,        /* 117 */
  MC_PROTOCOL_REG_HALL_LEARN_3_STATE,        /* 118 */
  MC_PROTOCOL_REG_HALL_LEARN_4_STATE,        /* 119 */
  MC_PROTOCOL_REG_HALL_LEARN_5_STATE,        /* 120 */
  MC_PROTOCOL_REG_HALL_LEARN_PROCESS_STATE,  /* 121 */
  MC_PROTOCOL_REG_HALL_LEARN_DIR,            /* 122 */
  MC_PROTOCOL_REG_IDENT_PROCESS_STATE,       /* 123 */
  MC_PROTOCOL_REG_HALL_LEARN_CHECK_FLAG,     /* 124 */
  MC_PROTOCOL_REG_RS = 200,                  /* 200 */
  MC_PROTOCOL_REG_LS                         /* 201 */
} MC_Protocol_REG_t;

typedef struct RCP_Frame_s
{
  uint8_t Code;                           /**< Identifier of the Frame. States the nature of the Frame. */
  uint8_t Size;                           /**< Size of the Payload of the frame in bytes. */
  uint8_t Buffer[CP_RX_MAX_PAYLOAD_SIZE]; /**< buffer containing the Payload of the frame. */
  uint8_t FrameCRC;                       /**< "CRC" of the Frame. Computed on the whole frame (Code,
                                           */
} RCP_Frame_t;

typedef struct TCP_Frame_s
{
  uint8_t Code;                            /**< Identifier of the Frame. States the nature of the Frame. */
  uint8_t Size_LB;                         /**< Size of the Payload of the frame in bytes.(High Byte) */
  uint8_t Size_HB;                         /**< Size of the Payload of the frame in bytes.(Low Byte) */
  uint8_t Buffer[DATA_BUFFER_EXTRA_SIZE];  /**< buffer containing the Payload of the frame. */
  uint8_t FrameCRC;                        /**< "CRC" of the Frame. Computed on the whole frame (Code,
                                            */
} TCP_Frame_t;

/* List of error codes */
typedef enum ERROR_CODE_e
{
  ERROR_NONE = 0,             /**<  0x00 - No error */
  ERROR_BAD_FRAME_ID,         /**<  0x01 - BAD Frame ID. The Frame ID has not been recognized by the firmware. */
  ERROR_CODE_SET_READ_ONLY,   /**<  0x02 - Write on read-only. The master wants to write on a read-only register. */
  ERROR_CODE_GET_WRITE_ONLY,  /**<  0x03 - Read not allowed. The value cannot be read. */
  ERROR_CODE_NO_TARGET_DRIVE, /**<  0x04 - Bad target drive. The target motor is not supported by the firmware. */
  ERROR_CODE_WRONG_SET,       /**<  0x05 - Value used in the frame is out of range expected by the FW. */
  ERROR_CODE_WRONG_CMD,       /**<  0x06 - Bad command ID. The command ID has not been recognized. */
  ERROR_CODE_OVERRUN,         /**<  0x07 - Overrun error. Transmission speed too fast, frame not received correctly */
  ERROR_CODE_TIMEOUT,         /**<  0x08 - Timeout error. Received frame corrupted or unrecognized by the FW. */
  ERROR_CODE_BAD_CRC,         /**<  0x09 - The computed CRC is not equal to the received CRC byte. */
  ERROR_BAD_MOTOR_SELECTED,   /**<  0x0A - Bad target drive. The target motor is not supported by the firmware. */
  ERROR_BAD_START_BYTE,       /**<  0x0B - Bad Start byte. The start byte has not been recognized. */
  ERROR_CODE_INCOMPLETE       /**<  0x0C - Receive command is incomplete.*/
} ERROR_CODE;

/* START BYTE */
#define MC_PROTOCOL_START_BYTE                  0x41

/* PROTOCOL CODE */
#define MC_PROTOCOL_CODE_GET_BOARD_INFO         0x01
#define MC_PROTOCOL_CODE_EXECUTE_CMD            0x02
#define MC_PROTOCOL_CODE_GET_REG                0x03
#define MC_PROTOCOL_CODE_SET_REG                0x04
#define MC_PROTOCOL_CODE_SAVE_MONITOR_DATA      0x05
#define MC_PROTOCOL_CODE_LOAD_MONITOR_DATA      0x06

/* PROTOCOL COMMAND */
#define MC_PROTOCOL_CMD_START_MOTOR             0x01
#define MC_PROTOCOL_CMD_STOP_MOTOR              0x02
#define MC_PROTOCOL_CMD_ENCODER_ALIGN           0x03
#define MC_PROTOCOL_CMD_WRITE_FLASH             0x04
#define MC_PROTOCOL_CMD_FAULT_ACK               0x05
#define MC_PROTOCOL_CMD_WINDING_IDENTIFY        0x06
#define MC_PROTOCOL_CMD_AUTO_TUNE_CURR_PI       0x07
#define MC_PROTOCOL_CMD_HALL_LEARN              0x08

void ui_rx_receive_handler(void);
void save_extra_buffer(TCP_Frame_t * pFrame);
void RCP_ReceivedFrameID(RCP_Frame_t * pFrame);
void RCP_ExecCmd(uint8_t CmdID, RCP_Frame_t * pFrame);
void RCP_GetReg(MC_Protocol_REG_t RegID, RCP_Frame_t * pFrame);
void RCP_SetReg(MC_Protocol_REG_t RegID, RCP_Frame_t * pFrame);
flag_status RCP_SetMonitorDataAdress(MC_Protocol_REG_t reg_id, uint8_t channel);
void write_flash_cmd(void);
void flash_read(uint32_t read_addr, int32_t *p_buffer, uint16_t num_read);
void flash_write_nocheck(uint32_t write_addr, int32_t *p_buffer, uint16_t num_write);
void flash_write(uint32_t write_addr, int32_t *p_buffer, uint16_t num_write);
void TCP_SendFrame(TCP_Frame_t * pFrame, uint8_t code, int32_t wData, uint16_t len);
uint8_t RCP_CalcCRC(RCP_Frame_t * pFrame);
uint8_t TCP_CalcCRC(TCP_Frame_t * pFrame);
void tx_send_monitor_data(void);
void ui_save_monitor_data(void);

#ifdef __cplusplus
}
#endif

#endif
