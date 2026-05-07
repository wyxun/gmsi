/**
 * @file userconfig.h
 * @brief BLM Bootloader User Configuration
 */
#ifndef __USERCONFIG_H__
#define __USERCONFIG_H__

/* System clock - set to 120MHz (8MHz HEXT * 15) */
#define BLM_SYSCLK          120000000UL

#include "global_define.h"

/* Module ID */
#define BLM     ((MODUS_ID_CLASS<<8)+1)

/* Flash Layout */
#define BLM_BOOTLOADER_ADDR     0x08000000
#define BLM_BOOTLOADER_SIZE     0x10000     // 64KB
#define BLM_SHARED_INFO_ADDR    0x08010000
#define BLM_SHARED_INFO_SIZE    0x0800      // 2KB
#define BLM_APP_ADDR            0x08010800

#if defined(AT32F407xx)
#define BLM_APP_MAX_SIZE        0xEF800     // ~958KB (for 1MB flash)
#else  /* STM32G431 default */
#define BLM_APP_MAX_SIZE        0x2F800     // ~190KB (for 256KB flash)
#endif

/* UART Configuration */
#ifndef BLM_UART_BAUDRATE
#define BLM_UART_BAUDRATE       115200
#endif
/* Timing */
#define BLM_WAIT_TIMEOUT_MS     5000        // Wait for connection
#define BLM_PACKET_TIMEOUT_MS   1000        // Packet receive timeout
#define BLM_MAX_RETRY           1000        // Max retry count (very long for debug)

/* Version */
#define BLM_VERSION_MAJOR       1
#define BLM_VERSION_MINOR       0

/* Protocol */
#define BLM_PACKET_SIZE         128         // SOH packet size
#define BLM_PACKET_1K_SIZE      1024        // STX packet size
#define BLM_FRAME_SIZE          1029        // STX + SEQ + ~SEQ + DATA[1024] + CRC[2]

/* Events */
typedef enum {
    Event_BlmReceiveData = Modus_Event00,
    Event_BlmTimeout = Modus_Event01,
    Event_BlmComplete = Modus_Event02,
    Event_BlmError = Modus_Event03,
} BLM_Event;

#define MLOG_MASK_DEFAULT   (MLOG_MASK_E | MLOG_MASK_W | MLOG_MASK_I)

/* GWaveform Configuration */
#ifndef MWAVEFORM_ENABLE
#   define MWAVEFORM_ENABLE             1
#endif
#define MWAVEFORM_MAX_CHANNELS          8
#define MWAVEFORM_DECIMATION            1
#define MWAVEFORM_RTT_BUFFER_SIZE       8192    /* High bandwidth for 1kHz */
#define MWAVEFORM_FIFO_DEPTH            64      /* High jitter tolerance */



#endif  // __USERCONFIG_H__
