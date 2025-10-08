/**
  **************************************************************************
  * @file     mc_flash_data_table_bldc.c
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
#elif defined __GNUC__ // AT32IDE
__attribute__((section(".MC_VectStoreAddr1")))
const int32_t intCoeffs32[INT32_SIZE_ARRAY1]=
#endif
{
  FIRMWARE_ID,
  (CURRENT_BASE * 100),
  VOLTAGE_BASE,
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
  PID_SPD_KP_DIV_LOG,
  PID_SPD_KI_DIV_LOG,
  0x0F,
  PID_IS_KP_DEFUALT,
  PID_IS_KI_DEFUALT,
  PID_IS_KP_DIV_LOG,
  PID_IS_KI_DIV_LOG,
  0x14,
  0x15,
  0x16,
  0x17,
  0x18,
  0x19,
  0x1A,
  0x1B,
  0x1C,
  0x1D,
  0x1E,
  0x1F,
  MAX_SPEED_RPM,
  MIN_SPEED_RPM,
  ACC_SPD_SLOPE,
  DEC_SPD_SLOPE,
  MAX_CURRENT_PU,
  MIN_CURRENT_PU,
  START_CURRENT_PU,
  0x27,//START_PERIOD,
  EMF_LOW_SPD_OFFSET_RISING,
  EMF_LOW_SPD_OFFSET_FALLING,
  0x2A,
  0x2B,
  0x2C,
  0x2D,
  0x2E,
  0x2F,
  0x30,
  0x31,
  0x32,
  0x33,
  0x34,
  0x35,
  0x36,
  0x37,
  0x38,
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
  0x4E,
  0x4F,
  0x50,
  0x51,
  0x52,
  0x53,
  0x54,
  0x55,
  EMF_HIGH_SPD_OFFSET_RISING,
  EMF_HIGH_SPD_OFFSET_FALLING,
  0x58,
  0x59,
  0x5A,
  0x5B,
  0x5C,
  0x5D,
  0x5E,
  0x5F,
  0x60,
  0x61,
  0x62,
  0x63,
  0x64,
  0x65,
  0x66,
  0x67,
  0x68,
  0x69,
  0x6A,
  0x6B,
  0x6C,
  OLC_VOLT_CMD,
  OLC_INIT_SPD,
  OLC_VOLT_INC_CMD,
  OLC_FINAL_SPD,
  OLC_TIMES,
  START_VOLTAGE_DUTY_PU,
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
