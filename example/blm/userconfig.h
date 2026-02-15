/**
 * @file userconfig.h
 * @brief BLM Bootloader User Configuration
 */
#ifndef __USERCONFIG_H__
#define __USERCONFIG_H__

#include "gmsi.h"

/* Module ID */
#define BLM     ((GMSI_ID_CLASS<<8)+1)

/* Flash Layout */
#define BLM_BOOTLOADER_ADDR     0x08000000
#define BLM_BOOTLOADER_SIZE     0x4000      // 16KB
#define BLM_SHARED_INFO_ADDR    GBLINFO_SHARED_ADDR
#define BLM_SHARED_INFO_SIZE    GBLINFO_SHARED_SIZE
#define BLM_APP_ADDR            0x08004400

#if defined(AT32F407xx)
#define BLM_APP_MAX_SIZE        0xFDC00     // ~1015KB (for 1MB flash)
#else  /* STM32G431 default */
#define BLM_APP_MAX_SIZE        0x3DC00     // ~247KB (for 256KB flash)
#endif

/* UART Configuration */
#define BLM_UART_BAUDRATE       115200

/* Timing */
#define BLM_WAIT_TIMEOUT_MS     3000        // Wait for connection
#define BLM_PACKET_TIMEOUT_MS   1000        // Packet receive timeout
#define BLM_MAX_RETRY           10          // Max retry count

/* Version */
#define BLM_VERSION_MAJOR       1
#define BLM_VERSION_MINOR       0

/* Protocol */
#define BLM_PACKET_SIZE         128         // SOH packet size
#define BLM_FRAME_SIZE          133         // SOH + SEQ + ~SEQ + DATA[128] + CRC[2]

/* Events */
typedef enum {
    Event_BlmReceiveData = Gmsi_Event00,
    Event_BlmTimeout = Gmsi_Event01,
    Event_BlmComplete = Gmsi_Event02,
    Event_BlmError = Gmsi_Event03,
} BLM_Event;

#endif  // __USERCONFIG_H__
