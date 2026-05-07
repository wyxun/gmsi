#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

/*============================ INCLUDES ======================================*/
#include "modus.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

/* configuration structure for the example object */
typedef struct {
    uint8_t  *pchRingBuffer;
    uint16_t  hwRingSize;
} example_cfg_t;

/* example object */
typedef struct {
    modus_base_t *ptBase;
} example_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

int example_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

/*============================ IMPLEMENTATION ================================*/

#endif /* __EXAMPLE_H__ */