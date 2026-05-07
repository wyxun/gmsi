#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "modus.h"

#define EXAMPLE     ((MODUS_ID_MOCK<<8)+1)
#define TEMPLATE    ((MODUS_ID_MOCK<<8)+2)

typedef struct{
    uint32_t value1;  // Example shared memory value
    uint32_t value2;  // Another example shared memory value
}example_share_mem_t;

typedef enum {
    Event_SyncButtonPushed = Modus_Event00,
    Event_PacketReceived = Modus_Event01,
    Event_SyncMissed = Modus_Event02
} EXAMPLE_Event;


#endif
