#ifndef __GMSI_BASE_H__
#define __GMSI_BASE_H__
/*============================ INCLUDES ======================================*/
#include "global_define.h"
#include "utilities/list.h"
/*============================ MACROS ========================================*/


/*============================ MACROFIED FUNCTIONS ===========================*/                   
/*============================ TYPES =========================================*/

#if 1
typedef struct{
    //void (*Init)(uint32_t wObjectAddr, uint32_t wObjectCfgAddr);
    int (*Clock)(uintptr_t wObjectAddr);
    int (*Run)(uintptr_t wObjectAddr);
}gmsi_interface_t;
#endif

typedef struct{

    uint8_t *pchMessage;

    uint16_t hwLength;
}message_t;

typedef struct{
    uint32_t wId;
    uint32_t wParent;
    gmsi_interface_t FcnInterface;
}gmsi_base_cfg_t;
typedef struct {
    uint32_t wId;
    uint32_t wEvent;
    message_t tMessage;

    gmsi_interface_t *pFcnInterface;
    uint32_t wParent;

    struct xLIST_ITEM   tListItem;
}gmsi_base_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/
int gbase_Init(gmsi_base_t *ptBase, gmsi_base_cfg_t *ptCfg);

int gbase_EventPost(uint32_t wId, uint32_t wEvent);
uint32_t gbase_EventPend(gmsi_base_t *ptBase);

int gbase_MessagePost(uint32_t wId, uint8_t *pchMessage, uint16_t hwLength);
struct xLIST* gbase_GetBaseList(void);
#endif

