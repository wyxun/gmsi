/**
  **************************************************************************
  * @file     mc_field_weakening.c
  * @brief    field weakening related functions.
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
  * @brief  clear parameters of field weakening
  * @param  fw_handler: field weakening related variables
  * @param  volt_handler: voltage related variables
  * @retval none
  */
void fw_clear(field_weakening_type *fw_handler, voltage_type *volt_handler)
{
  qd_type V_null = {( int16_t )0, ( int16_t )0};

  pid_set_integral(&fw_handler->pid_fw, ( int32_t )0);
  fw_handler->Idref_offset = 0;
  volt_handler->Vqd_filtered = V_null;
}
