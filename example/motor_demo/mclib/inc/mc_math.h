/**
  **************************************************************************
  * @file     mc_math.h
  * @brief    Declaration of the filter related functions.
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

#ifndef __MC_MATH_H
#define __MC_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

moving_average_type* moving_average(uint16_t);
int32_t moving_average_update(moving_average_type*, int32_t);
void reset_ma_buffer(moving_average_type* filter);
int32_t ma_filter(int32_t input_handler, int32_t average_handler, uint16_t PowOf2);
void lowpass_filter_init(lowpass_filter_type *lowpass_handler);
int32_t lowpass_filtering(lowpass_filter_type *lowpass_handler, int32_t input_handler);

#ifdef __cplusplus
}
#endif

#endif
