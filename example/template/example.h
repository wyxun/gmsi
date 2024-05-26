#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

// Include necessary libraries
#include "gmsi.h"

// The configuration structure for the example object.
typedef struct{

}example_cfg_t;

// The structure for the example object.
typedef struct{
    gmsi_base_t *ptBase;

}example_t;

// Function prototypes
int example_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

#endif // __EXAMPLE_H__