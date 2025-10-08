/**
  **************************************************************************
  * @file     mc_flash_data_table_foc.c
  * @brief    Writing parameters table into Flash
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

/* ----------------------------------------------------------------------
** Include Files
** ------------------------------------------------------------------- */
#include "mc_lib.h"

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)) // Keil V5/V6
const int32_t intCoeffs32[INT32_SIZE_ARRAY1] __attribute__((section(".ARM.__at_"MC_VectStoreAddr1))) =
#elif defined ( __ICCARM__ ) // IAR
const int32_t intCoeffs32[INT32_SIZE_ARRAY1] __attribute__((section(".ARM.__at_"MC_VectStoreAddr1))) =
#elif defined __GNUC__ // AT32IDE
__attribute__((section(".MC_VectStoreAddr1")))
const int32_t intCoeffs32[INT32_SIZE_ARRAY1]=
#endif
{
  FIRMWARE_ID,
  (CURRENT_BASE * 100),
  (int32_t)VOLTAGE_BASE,
  FIRMWARE_ID,
  TUNE_TARGET_CURRENT_PU,
  TUNE_CURRENT_TOTAL_PERIOD,
  TUNE_CURRENT_STEP_PERIOD,
  ESC_STATE_IDLE,
  MOTOR_CONTROL_MODE,
  CTRL_SOURCE,
  0x0A,
  PID_SPD_KP_DEFUALT,
  PID_SPD_KI_DEFUALT,
  PID_SPD_KP_GAIN_DIV_LOG,
  PID_SPD_KI_GAIN_DIV_LOG,
  0x0F,
  PID_IQ_KP_DEFUALT,
  PID_IQ_KI_DEFUALT,
  PID_IQ_KP_GAIN_DIV_LOG,
  PID_IQ_KI_GAIN_DIV_LOG,
  0x14,
  PID_ID_KP_DEFUALT,
  PID_ID_KI_DEFUALT,
  PID_ID_KP_GAIN_DIV_LOG,
  PID_ID_KI_GAIN_DIV_LOG,
  0x19,
  0x1A,
  0x1B,
  0x1C,
  0x1D,
  0x1E,
  0x1F,
  MAX_SPEED_RPM,
  MIN_CONTROL_SPEED,
  ACC_SPD_SLOPE,
  DEC_SPD_SLOPE,
  MAX_CURRENT_PU,
  MIN_CURRENT_PU,
  0x26,
  0x27,
  0x28,
  0x29,
  FW_KP_GAIN,
  FW_KI_GAIN,
  FW_MAX_ID_CURR_PU,
  0x2D,
  OBS_GAIN1,
  OBS_GAIN2,
  PLL_KP_GAIN,
  PLL_KI_GAIN,
  PLL_KP_GAIN_DIV_LOG,
  PLL_KI_GAIN_DIV_LOG,
  OLC_VOLT_CMD,
  OLC_ANGLE_INC,
  ALIGN_VOLT_CMD,
  0x37,
  MC_NO_ERROR,
  0x39,
  0x3A,
  0x3B,
  0x3C,
  0x3D,
  0x3E,
  0x3F,
  0x40,
  0x41,
  0x42,
  0x43,
  0x44,
  0x45,
  0x46,
  0x47,
  0x48,
  0x49,
  0x4A,
  0x4B,
  0x4C,
  0x4D,
  STARTUP_MAX_SPD,
  0x50,
  STARTUP_OL_SLOPE,
  STARTUP_ALIGN_TIME,
  0x52,
  STARTUP_START_TIME,
  FW_KP_GAIN_DIV_LOG,
  FW_KI_GAIN_DIV_LOG,
  0x56,
  0x57,
  STARTUP_CURRENT_PU,
  0x59,
  0x5A,
  0x5B,
  0x5C,
  ENC_CPR_NBR,
  MAX_POSITION_ANGLE,
  MIN_POSITION_ANGLE,
  0x60,
  0x61,
  0x62,
  PID_POS_KP_DEFUALT,
  PID_POS_KI_DEFUALT,
  PID_POS_KI_DEFUALT_STABLE,
  PID_POS_KD_DEFUALT,
  PID_POS_KP_GAIN_DIV_LOG,
  PID_POS_KI_GAIN_DIV_LOG,
  PID_POS_KD_GAIN_DIV_LOG,
  0x6A,
  UNDO,
  UNDO,
  0x6D,
  0x6E,
  0x6F,
  0x70,
  0x71,
  0x72,
  HALL_LEARN_0_STATE,
  HALL_LEARN_1_STATE,
  HALL_LEARN_2_STATE,
  HALL_LEARN_3_STATE,
  HALL_LEARN_4_STATE,
  HALL_LEARN_5_STATE,
  PROCESS_0_LOCK,
  HALL_LEARN_DIR,
  UNDO,
  HALL_LEARN_CHECK_FLAG,
};

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)) // Keil V5/V6
const float floatCoeffs32[INT32_SIZE_ARRAY2] __attribute__((section(".ARM.__at_"MC_VectStoreAddr2))) =
#elif defined ( __ICCARM__ ) // IAR
const float floatCoeffs32[INT32_SIZE_ARRAY2] __attribute__((section(".ARM.__at_"MC_VectStoreAddr2))) =
#elif defined __GNUC__ // AT32IDE
__attribute__((section(".MC_VectStoreAddr2")))
const float floatCoeffs32[INT32_SIZE_ARRAY2]=
#endif
{
  RS,
  LS
};
