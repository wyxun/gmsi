/**
  **************************************************************************
  * @file     mc_math.c
  * @brief    the filter related functions.
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
  * @brief  moving average filter initialize function
  * @param  order: order of moving average
  * @retval moving average filter related variables
  */
moving_average_type* moving_average(uint16_t order)
{
  moving_average_type* filter = malloc(sizeof(moving_average_type));
  int32_t* buffer = calloc(order, sizeof(int32_t));

  filter->order = order;
  filter->index = 0;
  filter->sum = 0;
  filter->buffer = buffer;

  return filter;
}

/**
  * @brief  moving average filter function
  * @param  filter: moving average filter related variables
  * @param  data: raw data
  * @retval filtered data
  */
int32_t moving_average_update(moving_average_type* filter, int32_t data)
{
  int32_t avg_value;

  if (filter->index >= (filter->order)) /* reset buffer index, ring buffer mechanism.*/
  {
    filter->index = 0;
    filter->full_flag = 1;
  }

  if (filter->full_flag > 0)
  {
    filter->sum = filter->sum - filter->buffer[filter->index] + data;
    avg_value = filter->sum / filter->order;
  }
  else
  {
    filter->sum = filter->sum + data;
    avg_value = filter->sum / (filter->index + 1);
  }

  filter->buffer[filter->index++] = data;

  return (avg_value);
}

/**
  * @brief  reset moving average filter function
  * @param  filter: moving average filter related variables
  * @retval none
  */
void reset_ma_buffer(moving_average_type* filter)
{
  filter->full_flag = 0;
  filter->index = 0;
  filter->sum = 0;
}

/**
  * @brief  fast moving average filter function
  * @param  input_handler: raw data
  * @param  average_handler: previous filtered data
  * @param  PowOf2: raw data
  * @retval next filtered data
  */
int32_t ma_filter(int32_t input_handler, int32_t average_handler, uint16_t PowOf2)
{
  int64_t cal_temp;

  cal_temp = (int64_t)average_handler << PowOf2;
  cal_temp = cal_temp - average_handler + input_handler;

  average_handler = (int32_t) (cal_temp >> PowOf2);

  return ( average_handler );
}

/**
  * @brief  lowpass filter initialize function
  * @param  lowpass_handler: lowpass filter related variables
  * @retval none
  */
void lowpass_filter_init(lowpass_filter_type *lowpass_handler)
{
  double cal_temp;

  cal_temp = (double) lowpass_handler->bandwidth / lowpass_handler->sample_freq;

  lowpass_handler->coef1 = (int16_t)(0x7FFF / (1 + cal_temp));
  lowpass_handler->coef2 = (int16_t)(0x7FFF * cal_temp / (1 + cal_temp));
}

/**
  * @brief  lowpass filter function
  * @param  lowpass_handler: lowpass filter related variables
  * @param  input_handler: raw data
  * @retval filtered data
  */
int32_t lowpass_filtering(lowpass_filter_type *lowpass_handler, int32_t input_handler)
{
  int64_t cal_temp;
  cal_temp = (int64_t)lowpass_handler->coef1 * lowpass_handler->output_temp + (int64_t)lowpass_handler->coef2 * input_handler;
  lowpass_handler->output_temp = cal_temp >> 15;

  return ( lowpass_handler->output_temp);
}

