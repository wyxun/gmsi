#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "gmsi.h"

#define EXAMPLE     ((GMSI_ID_MOCK<<8)+1)
#define TEMPLATE    ((GMSI_ID_MOCK<<8)+2)

#define TEMPLATE_ITEM_MESSAGE
#ifdef TEMPLATE_ITEM_MESSAGE
typedef struct{
    uint8_t chStatus;
    uint8_t chData[20];
    uint16_t hwLength;
}template_msg_t;
#endif

typedef enum {
    Event_SyncButtonPushed = Gmsi_Event00,
    Event_PacketReceived = Gmsi_Event01,
    Event_SyncMissed = Gmsi_Event02
} EXAMPLE_Event;


#endif
