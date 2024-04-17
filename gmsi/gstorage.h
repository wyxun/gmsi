#ifndef __GSTORAGE_H__
#define __GSTORAGE_H__

#include <stdint.h>
#include "gbase.h"

typedef void (*fcnStorage)(uint16_t *phwStorageStartAddr, uint16_t hwStorageLength);

typedef struct{
    uint16_t *phwStorageStartAddr;
    uint16_t hwStorageLength;
    fcnStorage fcnWrite;
    fcnStorage fcnRead;
}gstorage_cfg_t;

typedef struct{
    /*第一个成员固定是base*/
    gmsi_base_t *ptBase;

    uint16_t *phwStorageStartAddr;
    uint16_t hwStorageLength;
    fcnStorage fcnWrite;
    fcnStorage fcnRead;

    uint8_t chStorageFlag;
}gstorage_t;

#endif