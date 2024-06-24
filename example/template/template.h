#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__

// Include necessary libraries
#include "gmsi.h"

// The configuration structure for the template object.
typedef struct{

}template_cfg_t;

// The structure for the template object.
typedef struct{
    gmsi_base_t *ptBase;

}template_t;

// Function prototypes
int template_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

#endif // __TEMPLATE_H__