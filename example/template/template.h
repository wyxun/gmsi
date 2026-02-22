#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__

/*============================ INCLUDES ======================================*/
#include "gmsi.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

/* configuration structure for the template object */
typedef struct {
    uint8_t  *pchRingBuffer;
    uint16_t  hwRingSize;
} template_cfg_t;

/* template object */
typedef struct {
    gmsi_base_t *ptBase;

    int    wFd;
    void (*write)(int wFd, uint8_t *pchBuffer, uint16_t hwLength);
    int  (*read)(int wFd, uint8_t *pchBuffer);
} template_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

int template_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

/*============================ IMPLEMENTATION ================================*/

#endif /* __TEMPLATE_H__ */