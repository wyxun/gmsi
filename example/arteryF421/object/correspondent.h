#ifndef __CORRESPONDENT_H__
#define __CORRESPONDENT_H__

#include "gmsi.h"
#include "userconfig.h"

typedef struct{
    pfcnWrite Write;
    pfcnRead Read;
    uint32_t wFd;
    
    uint8_t *pchRingBuffer;
    uint16_t hwRingSize;
}correspondent_cfg_t;

typedef struct{
    gmsi_base_t *ptBase;

    pfcnWrite Write;
    pfcnRead Read;
    uint32_t wFd;

}correspondent_t;

int correspondent_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

#endif