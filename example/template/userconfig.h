#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "gmsi.h"

#define EXAMPLE     (GMSI_ID_MOCK<<8+1)
typedef enum {
    Event_SyncButtonPushed = Gmsi_Event00,
    Event_PacketReceived = Gmsi_Event01,
    Event_SyncMissed = Gmsi_Event02
} EXAMPLE_Event;


#endif
