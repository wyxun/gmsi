#include "gcoroutine.h"
#include "global_define.h"
#include "utilities/list.h"
#include "utilities/util_debug.h"

#ifdef LINUX_POSIX
#include <stdio.h>
#endif

static struct xLIST tListCoroutine;

void gcoroutine_Init(void)
{
    // 初始化链表头
    vListInitialise(&tListCoroutine);
}

int gcoroutine_Insert(gcoroutine_handle_t *ptHandle, fcnCoroutine *pfcn)
{
    int wRet = GMSI_SUCCESS;
    // 插入链表
    ptHandle->tListItem.pvOwner = ptHandle;
    vListInsertEnd(&tListCoroutine, &ptHandle->tListItem);

    return wRet;
}

int gcoroutine_Delete(gcoroutine_handle_t *ptHandle)
{
    int wRet = GMSI_SUCCESS;
    uxListRemove(&ptHandle->tListItem);

    return wRet;
}

int gcoroutine_Run(void)
{
    int wRet = GMSI_SUCCESS;
    gcoroutine_handle_t *ptHandle;
    fsm_rt_t tFsm;
    const struct xLIST_ITEM *ptListItemDes = tListCoroutine.xListEnd.pxPrevious;
    
    // 遍历链表
    while(ptListItemDes != &tListCoroutine.xListEnd){
        ptHandle = (gcoroutine_handle_t *)ptListItemDes->pvOwner;
        GMSI_ASSERT(NULL != ptHandle);
        tFsm = ptHandle->pfcn(ptHandle->pvParam);

        ptListItemDes = ptListItemDes->pxPrevious;
        if(fsm_rt_cpl == tFsm)
        {
            uxListRemove(&ptHandle->tListItem);
        }
    }

    return wRet;
}
