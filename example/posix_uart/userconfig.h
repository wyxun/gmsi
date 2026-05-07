#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "global_define.h"

#define PC_UART     ((MODUS_ID_UART<<8)+1)
typedef enum {
    Event_SyncButtonPushed = Modus_Event00,
    Event_PacketReceived = Modus_Event01,
    Event_SyncMissed = Modus_Event02
} PC_UART_Event;


#define PC_CLOCK    ((MODUS_ID_TIMER<<8)+1)
typedef enum {
    Event_SyncButtonPushed2 = Modus_Event00,
    Event_PacketReceived2 = Modus_Event01,
    Event_SyncMissed2 = Modus_Event02
} PC_CLOCK_Event;

// example
/*
#define EXAMPLE     (MODUS_ID_EXAMPLE<<8+1)
typedef enum {
    Event_SyncButtonPushed2 = Modus_Event00,
    Event_PacketReceived2 = Modus_Event01,
    Event_Clock = Modus_Event_Transition
} PC_CLOCK_Event;
*/
#endif
