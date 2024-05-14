#include "global_define.h"
#include "utilities/list.h"
#include "utilities/util_debug.h"
#include "gbase.h"

#ifdef LINUX_POSIX
#include <stdio.h>
#include <assert.h>
#endif

static struct xLIST tListObject;

int gbase_Init(gmsi_base_t *ptBase, gmsi_base_cfg_t *ptCfg)
{
    int wRet = GMSI_SUCCESS;
    static uint8_t chInitCount = 0;

    if(!chInitCount)
        vListInitialise(&tListObject);
    chInitCount++;

    ptBase->wId = ptCfg->wId;

    ptBase->wEvent = 0;

    vListInitialiseItem(&ptBase->tListItem);
    ptBase->tListItem.xItemValue = ptBase->wId;
    ptBase->tListItem.pvOwner = ptBase;

    vListInsert(&tListObject, &ptBase->tListItem);

    if(ptCfg->wParent)
        ptBase->wParent = ptCfg->wParent;
    else
        wRet = GMSI_EAGAIN;
    ptBase->pFcnInterface = &ptCfg->FcnInterface;
    return wRet;
}

int gbase_EventPost(uint32_t wId, uint32_t wEvent)
{
    int wRet = GMSI_SUCCESS;
    uint8_t chErgodicTime = 1;
    struct xLIST_ITEM *ptListItemDes = tListObject.xListEnd.pxPrevious;;
    gmsi_base_t *ptBaseDes;

    while(ptListItemDes != &tListObject.xListEnd){
        if(ptListItemDes->xItemValue == wId)
            break;

        ptListItemDes = ptListItemDes->pxPrevious;
        chErgodicTime++;
    }
    if(chErgodicTime <= tListObject.uxNumberOfItems)
    {

        ptBaseDes = ptListItemDes->pvOwner;
        GMSI_ASSERT(NULL != ptBaseDes);

        ptBaseDes->wEvent |= wEvent;
    }
    else
        wRet = GMSI_ENODEV;
    
    return wRet;
}


uint32_t gbase_EventPend(gmsi_base_t *ptBase)
{
    uint32_t wEvent = ptBase->wEvent;
    if(0 != wEvent)
    {
        ptBase->wEvent = 0;
    }
    return wEvent;
}

int gbase_MessagePost(uint32_t wId, uint8_t *pchMessage, uint16_t hwLength)
{
    int wRet = 0;
    struct xLIST_ITEM *ptListItemDes = tListObject.xListEnd.pxPrevious;
    uint8_t chErgodicTime = 1;
    gmsi_base_t *ptBaseDes;

    while(ptListItemDes != &tListObject.xListEnd){
        if(ptListItemDes->xItemValue == wId)
            break;

        chErgodicTime++;
        ptListItemDes = ptListItemDes->pxPrevious;
    }
    if(chErgodicTime <= tListObject.uxNumberOfItems)
    {

        ptBaseDes = ptListItemDes->pvOwner;
        GMSI_ASSERT(NULL != ptBaseDes);

        ptBaseDes->tMessage.pchMessage= pchMessage;
        ptBaseDes->tMessage.hwLength = hwLength;
        ptBaseDes->wEvent |= Gmsi_Event_Transition;
    }
    else
        wRet = GMSI_ENODEV;

    return wRet;
}

void gbase_DegugListBase(void)
{
    struct xLIST_ITEM *ptListItemDes = tListObject.xListEnd.pxPrevious;
    LOG_OUT("List all object:\n");

    while(ptListItemDes != &tListObject.xListEnd){
        LOG_OUT("    itme id:");
        LOG_OUT((uint32_t)ptListItemDes->xItemValue);
        LOG_OUT("\n");
        ptListItemDes = ptListItemDes->pxPrevious;
    }
}

struct xLIST* gbase_GetBaseList(void)
{
    return &tListObject;
}