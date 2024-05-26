#ifndef __GSTORAGE_H__
#define __GSTORAGE_H__

// Include necessary libraries
#include <stdint.h>
#include "gbase.h"

// Define types
typedef void (*fcnStorage)(uint16_t *phwStorageStartAddr, uint16_t hwStorageLength);

// Define structures
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
    gmsi_base_t *ptBase;

    gstorage_data_t *ptData;
    uint16_t hwStorageTimeOut;

    uint8_t chStorageFlag;
}gstorage_t;

// Function prototypes
int gstorage_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

#endif      // __GSTORAGE_H__