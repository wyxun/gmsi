#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "gmsi.h"

#define EXAMPLE         ((GMSI_ID_MOCK<<8)+1)
#define RFID            ((GMSI_ID_CLASS<<8)+1)
#define STEP4988        ((GMSI_ID_CLASS<<8)+2)

typedef uint16_t (*pfcnWrite)(uint8_t chUartNum, uint8_t *pchSendData, uint16_t hwLength);
typedef uint16_t (*pfcnRead)(uint8_t chUartNum, uint8_t *pchReceiveData);

typedef enum {
    // Individually controlled photo taking
    RFID_Event1 = 0,
    Rfid_Event_PacketReceived = 1,
    RFID_Event_Max
} twoInOneHandle_Event;

#endif