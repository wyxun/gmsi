#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

// Include necessary libraries
#include "gmsi.h"
#include "userconfig.h"

// The configuration structure for the rfid object.
typedef struct {
    pfcnWrite Write;
    pfcnRead Read;
    void (*pfcnCallback)(uint8_t chCmd, uint8_t *pchData, uint8_t chLength);
} rfid_cfg_t;

typedef struct {
    gmsi_base_t *ptBase;
    rfid_cfg_t *ptCfg;
    uint8_t chAddr;
    pfcnWrite Write;
    pfcnRead Read;
    void (*pfcnCallback)(uint8_t chCmd, uint8_t *pchData, uint8_t chLength);
} rfid_t;

// Function prototypes
int rfid_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);
void rfid_callback(uint8_t chCmd, uint8_t *pchData, uint8_t chLength);
// 新增功能接口
int rfid_Configure(rfid_t *ptThis, uint8_t chSettingType, uint8_t chValue);
int rfid_GetCardInfo(rfid_t *ptThis);

#endif // __RFID_H__