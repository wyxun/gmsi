#ifndef __GMSI_BASE_H__
#define __GMSI_BASE_H__

// Include necessary libraries
#include "global_define.h"
#include "utilities/list.h"

// Define types
typedef struct {
    int (*Clock)(uintptr_t wObjectAddr);
    int (*Run)(uintptr_t wObjectAddr);
} gmsi_interface_t;

typedef struct {
    uint8_t *pchMessage;
    uint16_t hwLength;
} message_t;

typedef struct {
    uint32_t wId;
    uintptr_t wParent;
    gmsi_interface_t FcnInterface;
} gmsi_base_cfg_t;

typedef struct {
    uint32_t wId;
    uint32_t wEvent;
    message_t tMessage;
    gmsi_interface_t *pFcnInterface;
    uintptr_t wParent;
    struct xLIST_ITEM tListItem;
} gmsi_base_t;

// Function prototypes
int gbase_Init(gmsi_base_t *ptBase, gmsi_base_cfg_t *ptCfg);
int gbase_EventPost(uint32_t wId, uint32_t wEvent);
uint32_t gbase_EventPend(gmsi_base_t *ptBase);
int gbase_MessagePost(uint32_t wId, uint8_t *pchMessage, uint16_t hwLength);
struct xLIST* gbase_GetBaseList(void);
void gbase_DegugListBase(void);

#endif // __GMSI_BASE_H__