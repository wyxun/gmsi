#include "global_define.h"
#include "utilities/list.h"
#include "utilities/util_debug.h"
#include "base.h"

#ifdef PC_DEBUG
#include <stdio.h>
#include <assert.h>
#endif
#define GMSI_BASE_LENGTH    20
//static gmsi_base_t g_tBase[GMSI_BASE_LENGTH];
// 定义根节点
struct xLIST        tListObject;

int gbase_Init(gmsi_base_t *ptBase, gmsi_base_cfg_t *ptCfg)
{
    int wRet = GMSI_SUCCESS;
    static uint8_t chInitCount = 0;
    // 初始化对象链表根节点
    if(!chInitCount)
        vListInitialise(&tListObject);
    chInitCount++;
    // 初始化对象id
    ptBase->wId = ptCfg->wId;

    /* 事件初始化为0 */
    ptBase->wEvent = 0;
    /* 链表节点初始化 */ 
    vListInitialiseItem(&ptBase->tListItem);
    ptBase->tListItem.xItemValue = ptBase->wId;
    ptBase->tListItem.pvOwner = ptBase;
    // 节点插入链表
    vListInsert(&tListObject, &ptBase->tListItem);

    // 对象操作接口
    if(ptCfg->wParent)
        ptBase->wParent = ptCfg->wParent;
    else
        wRet = GMSI_EAGAIN;
    ptBase->pFcnInterface = &ptCfg->FcnInterface;
    #ifdef PC_DEBUG
    //printf("init base %d\n", ptBase->wId);
    //printf("ptCfg wParent %d\n", ptCfg->wParent);
    #endif
    return wRet;
}

// 事件发送
int gbase_EventPost(uint32_t wId, uint32_t wEvent)
{
    int wRet = GMSI_SUCCESS;
    uint8_t chErgodicTime = 1;
    struct xLIST_ITEM *ptListItemDes = tListObject.xListEnd.pxPrevious;;
    gmsi_base_t *ptBaseDes;
    // 遍历链表，确定目的id
    while(ptListItemDes != &tListObject.xListEnd){
        if(ptListItemDes->xItemValue == wId)
            break;
        // 不匹配继续遍历
        ptListItemDes = ptListItemDes->pxPrevious;
        chErgodicTime++;
    }
    if(chErgodicTime <= tListObject.uxNumberOfItems)
    {
        // 找到对应的object
        ptBaseDes = ptListItemDes->pvOwner;
        assert(NULL != ptBaseDes);
        // 根据目的id设置事件值
        ptBaseDes->wEvent |= wEvent;
    }
    else
        wRet = GMSI_ENODEV;
    
    return wRet;
}

// 事件处理
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
    // 遍历链表，确定目的id
    while(ptListItemDes != &tListObject.xListEnd){
        if(ptListItemDes->xItemValue == wId)
            break;
        // 不匹配继续遍历
        chErgodicTime++;
        ptListItemDes = ptListItemDes->pxPrevious;
    }
    if(chErgodicTime <= tListObject.uxNumberOfItems)
    {
        // 找到对应的object
        ptBaseDes = ptListItemDes->pvOwner;
        assert(NULL != ptBaseDes);
        // 操作对应的object
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
    printf("List all object:\n");
    // 遍历链表
    while(ptListItemDes != &tListObject.xListEnd){
        printf("    itme id: %d\n", ptListItemDes->xItemValue);
        ptListItemDes = ptListItemDes->pxPrevious;
    }
}

struct xLIST* gbase_GetBaseList(void)
{
    return &tListObject;
}