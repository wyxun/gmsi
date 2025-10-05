#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "gmsi.h"

#define RFID            ((GMSI_ID_CLASS<<8)+1)
#define STEP4988        ((GMSI_ID_CLASS<<8)+2)
#define LSM6DSL         ((GMSI_ID_CLASS<<8)+3)
#define CORRESPONDENT   ((GMSI_ID_CLASS<<8)+4)

#define PROTOCOL_BOARD_ADDR     0x02

#define USER_PROTOCOL_CRC

typedef uint16_t (*pfcnWrite)(uint8_t chUartNum, uint8_t *pchSendData, uint16_t hwLength);
typedef uint16_t (*pfcnRead)(uint8_t chUartNum, uint8_t *pchReceiveData);

typedef enum {
    // Individually controlled photo taking
    RFID_Event1 = 1 << 0,
    twoInOneHandle_Event_PacketReceived = 1 << 1,
    twoInOneHandle_Event_GetLsm6dsl = 1 << 2,
    twoInOneHandle_Event_StartStepMotor = 1 << 3,
    twoInOneHandle_Event_StopStepMotor = 1 << 4,
    twoInOneHandle_Event_MotorDirCCW = 1 << 5,
    twoInOneHandle_Event_MotorDirCW = 1 << 6,
    twoInOneHandle_Event_ReadNfcData = 1 << 7, 
    twoInOneHandle_Event_Max
} twoInOneHandle_Event;

#define USART1_ENABLE       TRUE
#define USART2_ENABLE       TRUE
#define USART2_RS485_ENABLE TRUE
#endif