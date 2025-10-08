/**
  **************************************************************************
  * @file     mc_foc.h
  * @brief    Declaration of Field Oriented Control-related functions.
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

#ifndef __MC_FOC_H
#define __MC_FOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

trig_components_type trig_functions (int16_t angle_handler);
void foc_clarke_transform(abc_type *abc_handler, alphabeta_type *alphabeta_handler);
void foc_clarke_trans(abc_type *abc_handler, alphabeta_type *alphabeta_handler);
void foc_park_trans(current_type *curr_handler, trig_components_type *theta_handler);
void foc_inver_park_trans(voltage_type *volt_handler, trig_components_type *theta_handler);
void foc_open_loop_ctrl(voltage_type *volt_handler, open_loop_type *openloop_handler);

#ifdef __cplusplus
}
#endif

#endif
