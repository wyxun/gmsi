#ifndef __UPPER_H__
#define __UPPER_H__

#include "gmsi.h"

typedef struct{

}upper_cfg_t;

typedef struct{
    gmsi_base_t *ptBase;

}upper_t;

int upper_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

#endif