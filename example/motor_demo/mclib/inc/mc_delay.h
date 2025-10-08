/**
  **************************************************************************
  * @file     mc_delay.h
  * @brief    Declaration of Delay-related functions
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

#ifndef __MC_DELAY_H
#define __MC_DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mc_lib.h"

/** @addtogroup delay
  * @{
  */


/******************** functions ********************/
/* delay function */
void systick_init(void);
void mc_delay_init(void);
void mc_delay_us(uint32_t nus);
void mc_delay_ms(uint16_t nms);
void mc_delay_sec(uint16_t sec);

/**
  * @}
  */


#ifdef __cplusplus
}
#endif

#endif

