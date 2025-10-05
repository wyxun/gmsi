#ifndef __LSM6DSL_H__
#define __LSM6DSL_H__

// Include necessary libraries
#include "gmsi.h"

// chObjectStatus
#define LSM6DSL_UPDATE_READY       (1 << 0)
// #define LSM6DSL_OBJECT_RUNNING     (1 << 1)
// #define LSM6DSL_OBJECT_ERROR       (1 << 2)

// The configuration structure for the lsm6dsl object.
typedef struct{
    uint16_t hwUpdateRateHz;
}lsm6dsl_cfg_t;

// The structure for the lsm6dsl object.
typedef struct{
    gmsi_base_t *ptBase;
    
    uint16_t hwUpdateRate;
    uint8_t chObjectStatus;
    // 六轴参数
    int16_t hwAccel[3];
    int16_t hwGyro[3];
}lsm6dsl_t;

// Function prototypes
int lsm6dsl_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

#endif // __LSM6DSL_H__