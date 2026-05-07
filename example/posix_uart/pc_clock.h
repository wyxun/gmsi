#ifndef __PC_CLOCK_H__
#define __PC_CLOCK_H__

#define INTERVAL_MS 1

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "modus.h"

typedef struct{
    uint8_t chClockbase;
}pcclock_cfg_t;

typedef struct{
    modus_base_t *ptBase;
}pcclock_t;
//int pcclock_Init(pcclock_t *ptThis, pcclock_cfg_t *ptCfg);
int pcclock_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);
int pcclock_Run(uintptr_t wObjectAddr);
#endif