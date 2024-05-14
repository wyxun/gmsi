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
    vListInitialise(&tListCoroutine);
}

int gcoroutine_Insert(gcoroutine_handle_t *ptHandle, void *pvParam, fcnCoroutine pfcn)
{
    int wRet = GMSI_SUCCESS;
    if(false == ptHandle->bIsRun)
    {
        ptHandle->tListItem.pvOwner = ptHandle;
        ptHandle->pvParam = pvParam;
        ptHandle->pfcn = pfcn;
        vListInsertEnd(&tListCoroutine, &ptHandle->tListItem);
        ptHandle->bIsRun = true;
    }
    else {
        wRet = GMSI_EAGAIN;
    }

    return wRet;
}

int gcoroutine_Delete(gcoroutine_handle_t *ptHandle)
{
    int wRet = GMSI_SUCCESS;
    uxListRemove(&ptHandle->tListItem);
    ptHandle->bIsRun = false;
    return wRet;
}

int gcoroutine_Run(void)
{
    int wRet = GMSI_SUCCESS;
    gcoroutine_handle_t *ptHandle;
    fsm_rt_t tFsm;
    const struct xLIST_ITEM *ptListItemDes = tListCoroutine.xListEnd.pxPrevious;
    
    while(ptListItemDes != &tListCoroutine.xListEnd){
        ptHandle = (gcoroutine_handle_t *)ptListItemDes->pvOwner;
        GMSI_ASSERT(NULL != ptHandle->pfcn);
        tFsm = ptHandle->pfcn(ptHandle->pvParam);

        ptListItemDes = ptListItemDes->pxPrevious;
        if(fsm_rt_cpl == tFsm)
        {
            //uxListRemove(&ptHandle->tListItem);
            gcoroutine_Delete(ptHandle);
        }
    }

    return wRet;
}
