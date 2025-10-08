/**
  **************************************************************************
  * @file     mc_comm_uart.c
  * @brief    Peripherals configuration related to communication interface
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

#include "stdarg.h"
#include "mc_lib.h"

/* global variables definition */
uint8_t sync_frame[10] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB};
uint8_t usart_rx_buffer[RCP_MAX_FRAME_SIZE];

/**
  * @brief  config dma for usart
  * @param  none
  * @retval none
  */
void dma_uart_configuration()
{
  dma_init_type dma_init_struct;

  /* dma_uart channel for usart tx configuration */
  dma_reset(DMA_UART_TX_CHANNEL);
  dma_default_para_init(&dma_init_struct);
  dma_init_struct.buffer_size = DATA_BUFFER_FRAME_SIZE;
  dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
  dma_init_struct.memory_base_addr = (uint32_t) &monitor_data_buffer[0][0];
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.peripheral_base_addr = (uint32_t)&COMM_UART->dt;
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(DMA_UART_TX_CHANNEL, &dma_init_struct);

#ifndef AT32F421xx
  /* config flexible dma for usart1 tx */
  dma_flexible_config(DMA_UART, DMA_UART_TX_FLEX_CHANNEL, DMA_UART_TX_FLEX);
#endif

  /* disable transfer full data interrupt */
  dma_interrupt_enable(DMA_UART_TX_CHANNEL, DMA_FDT_INT, FALSE);

  /* dma1 channel for usart rx configuration */
  dma_reset(DMA_UART_RX_CHANNEL);
  dma_default_para_init(&dma_init_struct);
  dma_init_struct.buffer_size = RCP_MAX_FRAME_SIZE;
  dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
  dma_init_struct.memory_base_addr = (uint32_t)usart_rx_buffer;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.peripheral_base_addr = (uint32_t)&COMM_UART->dt;
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(DMA_UART_RX_CHANNEL, &dma_init_struct);

#ifndef AT32F421xx
  /* config flexible dma for usart1 rx */
  dma_flexible_config(DMA_UART, DMA_UART_RX_FLEX_CHANNEL, DMA_UART_RX_FLEX);
#endif
  /* disable transfer full data interrupt */
  dma_interrupt_enable(DMA_UART_RX_CHANNEL, DMA_FDT_INT, FALSE);
}

/**
  * @brief  Initialization of UART RX
  * @param  none
  * @retval none
  */
void uart_rx_init(void)
{
  /* Initialization of UART index */
  usart_data_idx.data_len = 0;
  usart_data_idx.frame_len = 0;
  usart_data_idx.no_error = TRUE;

  /* set sync frame*/
  for(uint8_t i = 0; i < 10; i++)
  {
    monitor_data_buffer[0][i] = sync_frame[i];
    monitor_data_buffer[1][i] = sync_frame[i];
  }

  monitor_data_buffer[0][DATA_BUFFER_SYNC_SIZE + DATA_BUFFER_SIZE] = (uint8_t)(0xFF);
  monitor_data_buffer[1][DATA_BUFFER_SYNC_SIZE + DATA_BUFFER_SIZE] = (uint8_t)(0xFF);

  /* usart2 rx begin DMA receiving */
  dma_channel_enable(DMA_UART_RX_CHANNEL, TRUE);
}

/**
  * @brief  UART IDLE interrupt handler for receiving data form MotorMonitor(UI)
  * @param  none
  * @retval none
  */
void COMM_UART_IRQHandler(void)
{
  uint8_t bErrorCode, receive_frame_len;

  if (usart_flag_get(COMM_UART, USART_IDLEF_FLAG))
  {
    /* clear USART1 IDLE interupt flags */
    usart_flag_clear(COMM_UART, USART_IDLEF_FLAG);
    /* disable DMA UART RX */
    dma_channel_enable(DMA_UART_RX_CHANNEL, FALSE);
    /* count data length */
    usart_data_idx.data_len = RCP_MAX_FRAME_SIZE - dma_data_number_get(DMA_UART_RX_CHANNEL);

    /* check start byte */
    if(usart_rx_buffer[0] != MC_PROTOCOL_START_BYTE)
    {
      bErrorCode = ERROR_BAD_START_BYTE;
      TCP_SendFrame(&tx_data_response, TCP_CODE_NACK, bErrorCode, 8);
    }
    else
    {
      receive_frame_len = usart_rx_buffer[1];
      usart_data_idx.frame_len = usart_data_idx.data_len - 3;

      /* check data length */
      if(usart_data_idx.frame_len == receive_frame_len)
      {
        ui_rx_receive_handler();
      }
      else
      {
        bErrorCode = ERROR_CODE_INCOMPLETE;
        TCP_SendFrame(&tx_data_response, TCP_CODE_NACK, bErrorCode, 8);
      }
    }

    /* reset DMA UART RX length */
    dma_data_number_set(DMA_UART_RX_CHANNEL, RCP_MAX_FRAME_SIZE);
    /* enable DMA UART RX */
    dma_channel_enable(DMA_UART_RX_CHANNEL, TRUE);
  }
}

