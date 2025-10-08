/**
  **************************************************************************
  * @file     delay.c
  * @brief    Delay-related functions
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

/** @addtogroup delay
  * @{
  */

/** @defgroup delay
  * @brief delay function
  * @{
  */


/* delay variable */
static __IO uint32_t fac_us;
static __IO uint32_t fac_ms;



/**
  * @brief  initialize systick interrupt every 1ms
  * @param  none
  * @retval none
  */
void systick_init(void)
{
  systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
  SysTick->LOAD = (uint32_t)(SystemCoreClock / 1000);
  SysTick->VAL = 0x00;
  SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
  NVIC_SetPriority(SysTick_IRQn, 3);
}

/**
  * @brief  initialize delay function
  * @param  none
  * @retval none
  */
void mc_delay_init()
{
  /* configure systick */
  fac_us = SystemCoreClock / (1000000U);
  fac_ms = fac_us * (1000U);
}

/**
  * @brief  inserts a delay time.
  * @param  nus: specifies the delay time length, in microsecond.
  * @retval none
  */
void mc_delay_us(uint32_t nus)
{
  uint32_t tick_value, start_value;
  uint32_t total_cnt, check_value;

  start_value = SysTick->VAL;
  total_cnt = nus * fac_us;

  do
  {
    tick_value = SysTick->VAL;
    check_value = start_value - tick_value;

    if (check_value <= 0)
    {
      check_value += fac_ms;
    }
  }
  while (check_value < total_cnt);
}

/**
  * @brief  inserts a delay time.
  * @param  nms: specifies the delay time length, in milliseconds.
  * @retval none
  */
void mc_delay_ms(uint16_t nms)
{
  uint32_t temp = 0;
  uint32_t start_value, tick_value;

  start_value = SysTick->VAL;
  /*clear count flag*/
  temp = SysTick->CTRL;

  while (nms)
  {
    do
    {
      temp = SysTick->CTRL;
    }
    while(!(temp & (1 << 16)));

    nms--;
  }

  do
  {
    tick_value = SysTick->VAL;
  }
  while (tick_value > start_value);
}

/**
  * @brief  inserts a delay time.
  * @param  sec: specifies the delay time, in seconds.
  * @retval none
  */
void mc_delay_sec(uint16_t sec)
{
  uint16_t index;

  for(index = 0; index < sec; index++)
  {
    mc_delay_ms(500);
    mc_delay_ms(500);
  }
}

/**
  * @}
  */

/**
  * @}
  */

