#ifndef __GSTORAGE_H__
#define __GSTORAGE_H__

#include <stdint.h>
#include "gbase.h"

typedef void (*fcnStorage)(uint16_t *phwStorageStartAddr, uint16_t hwStorageLength);

typedef struct{
    uint16_t *phwStorageStartAddr;
    uint16_t hwStorageLength;
    uint16_t hwCrcFlag;

    fcnStorage fcnWrite;
    fcnStorage fcnRead;
}gstorage_data_t;

typedef struct{
    gstorage_data_t *ptData;
    uint16_t hwStorageTimeOut;
}gstorage_cfg_t;

typedef struct{
    /*��һ����Ա�̶���base*/
    gmsi_base_t *ptBase;

    gstorage_data_t *ptData;
    uint16_t hwStorageTimeOut;

    uint8_t chStorageFlag;
}gstorage_t;

int gstorage_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

#endif