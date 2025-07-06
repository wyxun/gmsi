#ifndef __STEP4988_H__
#define __STEP4988_H__

// Include necessary libraries
#include "gmsi.h"

// The configuration structure for the step4988 object.
typedef struct{

}step4988_cfg_t;

// The structure for the step4988 object.
typedef struct{
    gmsi_base_t *ptBase;

}step4988_t;

// Function prototypes
int step4988_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

#endif // __STEP4988_H__