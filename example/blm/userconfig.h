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
#define BLM     ((GMSI_ID_CLASS<<8)+1)

/* Flash Layout */
#define BLM_BOOTLOADER_ADDR     0x08000000
#define BLM_BOOTLOADER_SIZE     0x7800      // 30KB
#define BLM_SHARED_INFO_ADDR    0x08007800
#define BLM_SHARED_INFO_SIZE    0x0800      // 2KB
#define BLM_APP_ADDR            0x08008000

#if defined(AT32F407xx)
#define BLM_APP_MAX_SIZE        0xFDC00     // ~1015KB (for 1MB flash)
#else  /* STM32G431 default */
#define BLM_APP_MAX_SIZE        0x3DC00     // ~247KB (for 256KB flash)
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
    Event_BlmReceiveData = Gmsi_Event00,
    Event_BlmTimeout = Gmsi_Event01,
    Event_BlmComplete = Gmsi_Event02,
    Event_BlmError = Gmsi_Event03,
} BLM_Event;

#define GLOG_MASK_DEFAULT   (GLOG_MASK_E | GLOG_MASK_W | GLOG_MASK_I)

/* GWaveform Configuration */
#ifndef GWAVEFORM_ENABLE
#   define GWAVEFORM_ENABLE             1
#endif
#define GWAVEFORM_MAX_CHANNELS          8
#define GWAVEFORM_DECIMATION            2

#endif  // __USERCONFIG_H__
