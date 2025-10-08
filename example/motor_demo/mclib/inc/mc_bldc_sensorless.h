/**
  **************************************************************************
  * @file     mc_bldc_sensorless.h
  * @brief
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

#ifndef __MC_BLDC_SENSORLESS_H
#define __MC_BLDC_SENSORLESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

/**************** extern parameter ******************/
extern const uint16_t hall_init_state[6];
extern const int16_t is_emf_rise_or_fall[7];

#if defined SPECIFIC_EACH_EMF_PIN
extern const gpio_type *read_gpio_port_table[7];
extern const int16_t read_gpio_pin_table[7];
#else
extern const int16_t read_gpio_table[7];
#endif

extern const int16_t emf_comp_hall_change_state[7];

/**************** define function ******************/
void start_up_config(void);
void set_next_detect_hall_state(void);
void bldc_detectEMF_param_init(adc_sample_type *adc_sample);
void angle_init_func(void);
void set_starting_volt(void);
void rdy_to_close_loop_param(void);
void detect_zero_cross_point(void);
flag_status is_find_zero_cross_COMP(hall_sensor_type *hall_handler, adc_sample_type *adc_sample);
flag_status is_find_zero_cross_ADC(hall_sensor_type *hall_handler, adc_sample_type *adc_sample);
void bldc_sensorless_change_phase(hall_sensor_type *hall_handler, adc_sample_type *adc_sample, speed_type *rotor_speed);

#ifdef __cplusplus
}
#endif

#endif

