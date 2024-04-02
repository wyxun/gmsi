#include "global_define.h"
#include "utilities/list.h"
#include "utilities/util_debug.h"
#include "base.h"

#ifdef PC_DEBUG
#include <stdio.h>
#endif
#define GMSI_BASE_LENGTH    20
//static gmsi_base_t g_tBase[GMSI_BASE_LENGTH];
// 定义根节点
struct xLIST        tListClass;

int gbase_Init(gmsi_base_t *ptBase, gmsi_base_cfg_t *ptCfg)
{
    static uint8_t chInitCount = 0;
    // 初始化类链表根节点
    if(!chInitCount)
        vListInitialise(&tListClass);
    chInitCount++;
    // 初始化类id
    ptBase->wId = ptCfg->wId;

    /* 事件初始化为0 */
    ptBase->wEvent = 0;
    /* 链表节点初始化 */ 
    vListInitialiseItem(&ptBase->tListItem);
    ptBase->tListItem.xItemValue = ptBase->wId;
    ptBase->tListItem.pvOwner = ptBase;
    // 节点插入链表
    vListInsert(&tListClass, &ptBase->tListItem);
    #ifdef PC_DEBUG
    printf("init base %d\n", ptBase->wId );
    #endif
    return 0;
}

// 事件发送
int gbase_EventSend(uint32_t wId, uint32_t wEvent)
{
    gmsi_base_t *ptBaseDes;
    struct xLIST_ITEM *ptListDes;
    
    // 遍历链表，确定目的id
    for(ptListDes = listGET_HEAD_ENTRY(&tListClass);        \
        ptListDes != NULL;                                  \
        ptListDes = listGET_NEXT(ptListDes)
        )
    {
        if(ptListDes->xItemValue == wId)
        {
            #ifdef PC_DEBUG
            //printf("gbase_EventSend to %d", gbase_EventSend);
            #endif
            break;
        }   
    }
    // 找到对应的类实体
    if(NULL != ptListDes)
    {
        ptBaseDes = ptListDes->pvOwner;
        // 根据目的id设置事件值
        ptBaseDes->wEvent |= wEvent;
    }

    return 0;
}

// 事件处理
uint32_t gbase_EventGet(gmsi_base_t *ptBase)
{
    uint32_t wEvent = ptBase->wEvent;
    if(0 != wEvent)
    {
        ptBase->wEvent = 0;
    }
        return wEvent;
}

int gbase_MessageSet(uint32_t wId, uint8_t *pchMessage, uint16_t hwLength)
{
    int wRet = 0;
    struct xLIST_ITEM *ptListDes;
    gmsi_base_t *ptBaseDes;
    printf("entry gbase_MessageSet");
    // 遍历链表，确定目的id
    for(ptListDes = listGET_HEAD_ENTRY(&tListClass);        \
        ptListDes != NULL;                                  \
        ptListDes = listGET_NEXT(ptListDes)
        )
    {
        if(ptListDes->xItemValue == wId)
        {
            break;
        }
    }
    
    // 找到对应的类实体
    if(NULL != ptListDes)
    {
        ptBaseDes = ptListDes->pvOwner;
        ptBaseDes->tMessage.pchMessage= pchMessage;
        ptBaseDes->tMessage.hwLength = hwLength;
        ptBaseDes->wEvent |= Gmsi_Event_Transition;
    }
    else
        wRet = GMSI_EAGAIN;
    return wRet;
}

void* gbase_MessageGet(gmsi_base_t *ptBase)
{
    uint16_t hwLength = ptBase->tMessage.hwLength;
    if(hwLength)
        return ptBase->tMessage.pchMessage; 
    else 
        return NULL;
}


void gbase_DegugListBase(void)
{
    //gmsi_base_t *ptBase;
    struct xLIST_ITEM *ptList;
    // 遍历链表
    for(ptList = listGET_HEAD_ENTRY(&tListClass);        \
        ptList != NULL;                                  \
        ptList = listGET_NEXT(ptList)
        )
    {
        LOG_OUT("\r\n");
        LOG_OUT(ptList->xItemValue);
    }
}

struct xLIST* gbase_GetBaseList(void)
{
    return &tListClass;
}