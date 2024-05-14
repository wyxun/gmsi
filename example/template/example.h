#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

#include "gmsi.h"

typedef struct{
    uint8_t chExampleData;
}example_cfg_t;

typedef struct{
    gmsi_base_t *ptBase;
    uint8_t chExampleData;
}example_t;

int example_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

#endif