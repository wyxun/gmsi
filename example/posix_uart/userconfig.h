#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "global_define.h"

#define PC_UART     ((GMSI_ID_UART<<8)+1)
typedef enum {
    Event_SyncButtonPushed = Gmsi_Event00,
    Event_PacketReceived = Gmsi_Event01,
    Event_SyncMissed = Gmsi_Event02
} PC_UART_Event;


#define PC_CLOCK    ((GMSI_ID_TIMER<<8)+1)
typedef enum {
    Event_SyncButtonPushed2 = Gmsi_Event00,
    Event_PacketReceived2 = Gmsi_Event01,
    Event_SyncMissed2 = Gmsi_Event02
} PC_CLOCK_Event;

// example
/*
#define EXAMPLE     (GMSI_ID_EXAMPLE<<8+1)
typedef enum {
    Event_SyncButtonPushed2 = Gmsi_Event00,
    Event_PacketReceived2 = Gmsi_Event01,
    Event_Clock = Gmsi_Event_Transition
} PC_CLOCK_Event;
*/
#endif
