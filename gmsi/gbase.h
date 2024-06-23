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
}message_t;

typedef struct {
    uint8_t *pchMessage;
    uint16_t hwLength;
    struct xLIST_ITEM tListItem;
} message_item_t;

typedef struct {
    uint32_t wId;
    uintptr_t wParent;
    gmsi_interface_t FcnInterface;
} gmsi_base_cfg_t;

typedef struct {
    uint32_t wId;
    uint32_t wEvent;
    //message_t tMessage;
    struct xLIST tListMessage;
    gmsi_interface_t *pFcnInterface;
    uintptr_t wParent;
    struct xLIST_ITEM tListItem;
} gmsi_base_t;

// Function prototypes
int gbase_Init(gmsi_base_t *ptBase, gmsi_base_cfg_t *ptCfg);
int gbase_EventPost(uint32_t wId, uint32_t wEvent);
uint32_t gbase_EventPend(gmsi_base_t *ptBase);
int gbase_MessagePost(uint32_t wId, message_item_t *ptMsgItem);
int gbase_MessagePend(gmsi_base_t *ptBase, message_t *ptMsg);
struct xLIST* gbase_GetBaseList(void);
void gbase_DegugListBase(void);

#define GMSI_MSG_ITEM_DECLARE(NAME,SIZE)                                    \
        uint8_t ch##NAME##_Buffer[SIZE] = {0};                              \
        static message_item_t t##NAME##item = {                             \
            .tListItem = {0},                                               \
            .pchMessage = ch##NAME##_Buffer,                                \
            .hwLength = SIZE                                                \
        };                                                                  \

#define GMSI_MSG_ITEM_INITIALISE_LIST(NAME)                                 \
    do{                                                                     \
        vListInitialiseItem(&(t##NAME##item).tListItem);                    \
        (t##NAME##item).tListItem.xItemValue = (t##NAME##item).hwLength;    \
        (t##NAME##item).hwLength = 0;                                       \
        (t##NAME##item).tListItem.pvOwner = &(t##NAME##item);               \
    }while(0)

#define GMSI_MSG_GET_HANDLE(NAME) &(t##NAME##item)   

#define GMSI_MSG_UPDATE(NAME, MESSAGE, LENGTH)                              \
    do{                                                                     \
        memcpy((t##NAME##item).pchMessage, MESSAGE, LENGTH);                \
        (t##NAME##item).hwLength = LENGTH;                                  \
    }while(0)

#define GMSI_MSG_GET_BUFFER(NAME) (t##NAME##item).pchMessage
#define GMSI_MSG_GET_LENGTH(NAME) (t##NAME##item).hwLength        

#endif // __GMSI_BASE_H__