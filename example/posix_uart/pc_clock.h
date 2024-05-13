#ifndef __PC_CLOCK_H__
#define __PC_CLOCK_H__

#define INTERVAL_MS 1

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "gmsi.h"

typedef struct{
    uint8_t chClockbase;
}pc_clock_cfg_t;

typedef struct{
    gmsi_base_t *ptBase;
}pc_clock_t;
//int pcclock_Init(pc_clock_t *ptThis, pc_clock_cfg_t *ptCfg);
int pcclock_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);
int pcclock_Run(uintptr_t wObjectAddr);
#endif