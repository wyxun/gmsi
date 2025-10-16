#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "gmsi.h"

#define EXAMPLE     ((GMSI_ID_MOCK<<8)+1)
#define TEMPLATE    ((GMSI_ID_MOCK<<8)+2)

typedef struct{
    uint32_t value1;  // Example shared memory value
    uint32_t value2;  // Another example shared memory value
}example_share_mem_t;

typedef enum {
    Event_SyncButtonPushed = Gmsi_Event00,
    Event_PacketReceived = Gmsi_Event01,
    Event_SyncMissed = Gmsi_Event02
} EXAMPLE_Event;


#endif
