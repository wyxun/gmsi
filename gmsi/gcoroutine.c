#include "gcoroutine.h"
#include "global_define.h"
#include "utilities/list.h"
#include "utilities/util_debug.h"

#ifdef LINUX_POSIX
#include <stdio.h>
#endif

static struct xLIST tListCoroutine;

/**
 * Function: gcoroutine_Init
 * ----------------------------
 * This function initializes a list named tListCoroutine.
 *
 * Parameters: 
 * None
 *
 * Returns: 
 * None
 */
void gcoroutine_Init(void)
{
    // Initialize the list tListCoroutine
    vListInitialise(&tListCoroutine);
}

/**
 * Function: gcoroutine_Insert
 * ----------------------------
 * This function inserts a coroutine into the tListCoroutine list. If the coroutine is already running, 
 * it returns an error code.
 *
 * Parameters: 
 * ptHandle: A pointer to the coroutine handle.
 * pvParam: A pointer to the coroutine parameters.
 * pfcn: A pointer to the coroutine function.
 *
 * Returns: 
 * A status code indicating the result of the function. GMSI_SUCCESS if the function succeeds, 
 * GMSI_EAGAIN if the coroutine is already running, GMSI_EINVAL if ptHandle is NULL, 
 * GMSI_EFAIL if vListInsertEnd fails.
 */
int gcoroutine_Insert(gcoroutine_handle_t *ptHandle, void *pvParam, fcnCoroutine pfcn)
{
    // Check if ptHandle is NULL
    if(ptHandle == NULL) {
        return GMSI_EINVAL;
    }

    int wRet = GMSI_SUCCESS;
    if(false == ptHandle->bIsRunning)
    {
        ptHandle->tListItem.pvOwner = ptHandle;
        ptHandle->pvParam = pvParam;
        ptHandle->pfcn = pfcn;
 
        vListInsertEnd(&tListCoroutine, &ptHandle->tListItem);
        ptHandle->bIsRunning = true;
    }
    else {
        wRet = GMSI_EAGAIN;
    }

    return wRet;
}

/**
 * Function: gcoroutine_Delete
 * ----------------------------
 * This function deletes a coroutine from the tListCoroutine list and sets its running state to false.
 *
 * Parameters: 
 * ptHandle: A pointer to the coroutine handle.
 *
 * Returns: 
 * A status code indicating the result of the function. GMSI_SUCCESS if the function succeeds.
 */
int gcoroutine_Delete(gcoroutine_handle_t *ptHandle)
{
    int wRet = GMSI_SUCCESS;
    // Remove the coroutine from the list
    uxListRemove(&ptHandle->tListItem);
    // Set the running state to false
    ptHandle->bIsRunning = false;
    return wRet;
}

/**
 * Function: gcoroutine_Run
 * ----------------------------
 * This function runs all coroutines in the tListCoroutine list. For each coroutine, it calls its function 
 * with its parameters. If the coroutine's function returns fsm_rt_cpl, it deletes the coroutine from the list.
 *
 * Parameters: 
 * None
 *
 * Returns: 
 * A status code indicating the result of the function. GMSI_SUCCESS if the function succeeds.
 */
int gcoroutine_Run(void)
{
    int wRet = GMSI_SUCCESS;
    gcoroutine_handle_t *ptHandle;
    fsm_rt_t tFsm;
    const struct xLIST_ITEM *ptListItemDes;

    for(ptListItemDes = tListCoroutine.xListEnd.pxPrevious;     \
                    ptListItemDes != &tListCoroutine.xListEnd;  \
                    ptListItemDes = ptListItemDes->pxPrevious)
    {
        if(ptListItemDes->pvOwner == NULL) {
            continue;
        }

        ptHandle = (gcoroutine_handle_t *)ptListItemDes->pvOwner;
        GMSI_ASSERT(NULL != ptHandle->pfcn);
        tFsm = ptHandle->pfcn(ptHandle->pvParam);

        if(fsm_rt_cpl == tFsm)
        {
            gcoroutine_Delete(ptHandle);
        }
    }

    return wRet;
}
