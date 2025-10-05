#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

// Include necessary libraries
#include "gmsi.h"
#include "userconfig.h"

#define NFC_TYPE_UNKNOWN 0x00 // NFC type definition
#define NFC_TYPE_NTAG213 213 // NTAG213 type definition

#pragma pack(push, 1) // 强制1字节对齐
// NFC user data
typedef struct{
    uint8_t chNfcType;      // NFC type
    uint8_t chNfcUUID[7];   // NFC UUID
    uint8_t chData[16];     // NFC data buffer
} nfc_message_t;
#pragma pack(pop)

// The configuration structure for the rfid object.
typedef struct {
    pfcnWrite Write;
    pfcnRead Read;
    uint32_t wFd;
    //void (*pfcnCallback)(rfid_t *ptThis, uint8_t chCmd, uint8_t *pchData, uint8_t chLength);
} rfid_cfg_t;

typedef struct {
    gmsi_base_t *ptBase;
    uint8_t chAddr;
    uint32_t wFd;
    pfcnWrite Write;
    pfcnRead Read;
    nfc_message_t tNfcMessage; // NFC message data
    //void (*pfcnCallback)(rfid_t *ptThis, uint8_t chCmd, uint8_t *pchData, uint8_t chLength);
} rfid_t;

// Function prototypes
int rfid_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);
//void rfid_callback(uint8_t chCmd, uint8_t *pchData, uint8_t chLength);
// 新增功能接口
int rfid_Configure(rfid_t *ptThis, uint8_t chSettingType, uint8_t chValue);
int rfid_GetCardInfo(rfid_t *ptThis);

#endif // __RFID_H__