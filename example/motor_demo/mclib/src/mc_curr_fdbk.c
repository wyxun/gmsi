/**
  **************************************************************************
  * @file     mc_curr_fdbk.c
  * @brief    current offset and sensing related functions.
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
  * @brief  read current offset values
  * @param  curr_handler: voltage vector d-axis and q-axis
  * @param  shunt_nbr: shunt number (ONESHUNT, TWOSHUNT or THREESHUNT)
  * @retval the state of initialize offset current (SET or RESET)
  */
//flag_status current_offset_init(current_type* curr_handler, shunt_nbr_type shunt_nbr)
//{
//  static flag_status adc_flag = SET;
//  static uint8_t icount = 0;
//  static uint16_t temp_a = 0, temp_b = 0, temp_c = 0;

//  while (icount < 17)
//  {
//    if ( (adc_flag_get(curr_handler->ADCx, ADC_PCCE_FLAG) != RESET) && (adc_flag == SET) )
//    {
//      adc_flag_clear(curr_handler->ADCx, ADC_PCCE_FLAG);

//      if (icount > 0)
//      {
//        if (shunt_nbr > 0)
//        {
//          temp_a += adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_1);
//        }

//        if (shunt_nbr > 1)
//        {
//          temp_b += adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_2);
//        }

//        if (shunt_nbr > 2)
//        {
//          temp_c += adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_3);
//        }
//      }

//      adc_flag = RESET;
//      icount++;
//    }
//    else if ( (adc_flag_get(curr_handler->ADCx, ADC_PCCE_FLAG) == RESET) && (adc_flag != SET) )
//    {
//      adc_flag = SET;
//    }
//  }

//  if (shunt_nbr == 1)
//  {
//    curr_handler->offset.bus = temp_a / 16;
//  }

//  if (shunt_nbr > 1)
//  {
//    curr_handler->offset.a = temp_a / 16;
//    curr_handler->offset.b = temp_b / 16;
//  }

//  if (shunt_nbr > 2)
//  {
//    curr_handler->offset.c = temp_c / 16;
//  }

//  return (SET);
//}

flag_status current_offset_init(current_type* curr_handler, shunt_nbr_type shunt_nbr)
{
  static flag_status adc_flag = SET;
  static uint8_t icount = 0;
  static uint16_t temp1 = 0, temp2 = 0, temp3 = 0;

  while (icount < 17)
  {
    if ( (adc_flag_get(curr_handler->ADCx, ADC_PCCE_FLAG) != RESET) && (adc_flag == SET) )
    {
      adc_flag_clear(curr_handler->ADCx, ADC_PCCE_FLAG);

      if (icount > 0)
      {
        if (shunt_nbr > 0)
        {
          temp1 += adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_1);
        }

        if (shunt_nbr > 1)
        {
          temp2 += adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_2);
        }

        if (shunt_nbr > 2)
        {
          temp3 += adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_3);
        }
      }

      adc_flag = RESET;
      icount++;
    }
    else if ( (adc_flag_get(curr_handler->ADCx, ADC_PCCE_FLAG) == RESET) && (adc_flag != SET) )
    {
      adc_flag = SET;
    }
  }

  if (shunt_nbr == 1)
  {
    curr_handler->offset.bus = temp1 / 16;
  }
  else if(shunt_nbr == 2)
  {
#if defined U_V_SHUNT
    curr_handler->offset.a = temp1 / 16;
    curr_handler->offset.b = temp2 / 16;
#elif defined V_W_SHUNT
    curr_handler->offset.b = temp1 / 16;
    curr_handler->offset.c = temp2 / 16;
#elif defined U_W_SHUNT
    curr_handler->offset.a = temp1 / 16;
    curr_handler->offset.c = temp2 / 16;
#endif
  }
  else if (shunt_nbr == 3)
  {
    curr_handler->offset.a = temp1 / 16;
    curr_handler->offset.b = temp2 / 16;
    curr_handler->offset.c = temp3 / 16;
  }

  return (SET);
}


/**
  * @brief  read three-phase current values
  * @param  curr_handler: three-phase current values and their offset
  * @param  pwm_duty_handler: three-phase pwm duty
  * @retval none
  */
void current_read_foc_3shunt(current_type *curr_handler, pwm_duty_type *pwm_duty_handler)
{
  int32_t mul_temp;
#ifdef HALL_SENSORS
  int16_t swap_temp;
#endif

  curr_handler->Iabc_shunt.a = -((adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_1) - curr_handler->offset.a) << 3);
  curr_handler->Iabc_shunt.b = -((adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_2) - curr_handler->offset.b) << 3);
  curr_handler->Iabc_shunt.c = -((adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_3) - curr_handler->offset.c) << 3);

#ifdef HALL_SENSORS
  if (hall_learn.dir == 1)
  {
    swap_temp = curr_handler->Iabc_shunt.c;
    curr_handler->Iabc_shunt.c = curr_handler->Iabc_shunt.b;
    curr_handler->Iabc_shunt.b = swap_temp;
  }
#endif

  mul_temp = ((int32_t)curr_handler->span * curr_handler->Iabc_shunt.a);
  curr_handler->Iabc_shunt.a = (mul_temp >> (15 - curr_handler->span_shift));

  mul_temp = ((int32_t)curr_handler->span * curr_handler->Iabc_shunt.b);
  curr_handler->Iabc_shunt.b = (mul_temp >> (15 - curr_handler->span_shift));

  mul_temp = ((int32_t)curr_handler->span * curr_handler->Iabc_shunt.c);
  curr_handler->Iabc_shunt.c = (mul_temp >> (15 - curr_handler->span_shift));

  if (pwm_duty_handler->UF.a > pwm_duty_handler->threshold)
  {
    curr_handler->Iabc.b = curr_handler->Iabc_shunt.b;
    curr_handler->Iabc.c = curr_handler->Iabc_shunt.c;
    curr_handler->Iabc.a = -(curr_handler->Iabc.b + curr_handler->Iabc.c);
  }
  else if (pwm_duty_handler->UF.b > pwm_duty_handler->threshold)
  {
    curr_handler->Iabc.a = curr_handler->Iabc_shunt.a;
    curr_handler->Iabc.c = curr_handler->Iabc_shunt.c;
    curr_handler->Iabc.b = -(curr_handler->Iabc.a + curr_handler->Iabc.c);
  }
  else if (pwm_duty_handler->UF.c > pwm_duty_handler->threshold)
  {
    curr_handler->Iabc.a = curr_handler->Iabc_shunt.a;
    curr_handler->Iabc.b = curr_handler->Iabc_shunt.b;
    curr_handler->Iabc.c = -(curr_handler->Iabc.a + curr_handler->Iabc.b);
  }
  else
  {
    curr_handler->Iabc.a = curr_handler->Iabc_shunt.a;
    curr_handler->Iabc.b = curr_handler->Iabc_shunt.b;
    curr_handler->Iabc.c = -(curr_handler->Iabc.a + curr_handler->Iabc.b);
  }
}

/**
  * @brief  read three-phase current values
  * @param  curr_handler: two-phase current values and their offset
  * @retval none
  */
void current_read_foc_2shunt(current_type *curr_handler)
{
  int32_t mul_temp;
#ifdef HALL_SENSORS
  int16_t swap_temp;
#endif

#if defined U_V_SHUNT
  curr_handler->Iabc.a = -((adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_1) - curr_handler->offset.a) << 3);
  curr_handler->Iabc.b = -((adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_2) - curr_handler->offset.b) << 3);
  curr_handler->Iabc.c = -(curr_handler->Iabc.a + curr_handler->Iabc.b);
#elif defined V_W_SHUNT
  curr_handler->Iabc.b = -((adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_1) - curr_handler->offset.b) << 3);
  curr_handler->Iabc.c = -((adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_2) - curr_handler->offset.c) << 3);
  curr_handler->Iabc.a = -(curr_handler->Iabc.b + curr_handler->Iabc.c);
#elif defined U_W_SHUNT
  curr_handler->Iabc.a = -((adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_1) - curr_handler->offset.a) << 3);
  curr_handler->Iabc.c = -((adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_2) - curr_handler->offset.c) << 3);
  curr_handler->Iabc.b = -(curr_handler->Iabc.a + curr_handler->Iabc.c);
#endif

#ifdef HALL_SENSORS
  if (hall_learn.dir == 1)
  {
    swap_temp = curr_handler->Iabc.c;
    curr_handler->Iabc.c = curr_handler->Iabc.b;
    curr_handler->Iabc.b = swap_temp;
  }
#endif

  mul_temp = ((int32_t)curr_handler->span * curr_handler->Iabc.a);
  curr_handler->Iabc.a = (mul_temp >> (15 - curr_handler->span_shift));

  mul_temp = ((int32_t)curr_handler->span * curr_handler->Iabc.b);
  curr_handler->Iabc.b = (mul_temp >> (15 - curr_handler->span_shift));

  mul_temp = ((int32_t)curr_handler->span * curr_handler->Iabc.c);
  curr_handler->Iabc.c = (mul_temp >> (15 - curr_handler->span_shift));
}

/**
  * @brief  read bus current value
  * @param  curr_handler: bus current value and its offset
  * @retval none
  */
#if defined SIX_STEP_CONTROL
void current_read_bldc(current_type *curr_handler)
{
  int32_t  mul_result;

  if (ADC_TIMER->c4dt > (I_SAMP_MIN_CNT + I_SAMP_DLY_CNT))
  {
    curr_handler->Ibus.Icalc = ((adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_2) - curr_handler->offset.bus) << 3);
  }
  else
  {
    mul_result = curr_handler->Ibus.Icalc * curr_handler->Ibus.decay_const;
    curr_handler->Ibus.Icalc = mul_result >> 15;
  }

  mul_result = ((int32_t)curr_handler->span * curr_handler->Ibus.Icalc);
  curr_handler->Ibus.Ireal_pu = (mul_result >> (15 - curr_handler->span_shift)) * curr_handler->volt_sign;
}
#endif

/**
  * @brief  read dc current offset value
  * @param  adc_value: adc value of dc current offset
  * @retval average of dc current offset values
  */
int16_t Idc_offset_init(uint16_t *adc_value)
{
  static uint8_t icount = 0;
  static uint16_t sum = 0;

  while (icount < 16)
  {
    sum += *adc_value;
    mc_delay_ms(1);
    icount++;
  }

  return (sum / 16);
}

/**
  * @brief  read dc current value
  * @param  curr_handler: dc current value and its offset
  * @param  adc_value: adc value of dc current
  * @retval none
  */
int16_t dc_current_read(current_type *curr_handler, uint16_t *adc_value)
{
  int16_t temp_i_dc;
  int32_t  mul_result;

  temp_i_dc = (*adc_value - curr_handler->Idc.offset) << 3;

  mul_result = ((int32_t)curr_handler->dc_span * temp_i_dc);

  return (mul_result >> (15 - curr_handler->span_shift));
}

/**
  * @brief  read 1-shunt current values for motor parameter identification
  * @param  curr_handler: bus current value and its offset
  * @retval none
  */
void current_read_1shunt_ID(current_type *curr_handler)
{
  int32_t mul_temp;

  curr_handler->ID_1shunt = (adc_preempt_conversion_data_get(curr_handler->ADCx, ADC_PREEMPT_CHANNEL_1) - curr_handler->offset.bus) << 3;
  mul_temp = ((int32_t)curr_handler->span * curr_handler->ID_1shunt);
  curr_handler->ID_1shunt = (mul_temp >> (15 - curr_handler->span_shift));
}
