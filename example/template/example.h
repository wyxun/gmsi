#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

#include "gmsi.h"

typedef struct{

}example_cfg_t;

typedef struct{
    gmsi_base_t *ptBase;

}example_t;

int example_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

#endif