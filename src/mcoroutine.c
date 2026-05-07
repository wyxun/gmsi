#include "mcoroutine.h"
#include "global_define.h"
#include "utilities/mlist.h"
#include "mdebug/util_debug.h"

#ifdef LINUX_POSIX
#include <stdio.h>
#endif

static mlist_t tListCoroutine;

/**
 * Function: mcoroutine_Init
 * ----------------------------
 * This function initializes a list named tListCoroutine.
 *
 * Parameters: 
 * None
 *
 * Returns: 
 * None
 */
void mcoroutine_Init(void)
{
    // Initialize the list tListCoroutine
    mlist_Init(&tListCoroutine);
}

/**
 * Function: mcoroutine_Insert
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
 * A status code indicating the result of the function. MODUS_SUCCESS if the function succeeds, 
 * MODUS_EAGAIN if the coroutine is already running, MODUS_EINVAL if ptHandle is NULL, 
 * MODUS_EFAIL if mlist_InsertEnd fails.
 */
int mcoroutine_Insert(mcoroutine_handle_t *ptHandle, void *pvParam, fcnCoroutine pfcn)
{
    // Check if ptHandle is NULL
    if(ptHandle == NULL) {
        return MODUS_EINVAL;
    }

    int wRet = MODUS_SUCCESS;
    if(false == ptHandle->bIsRunning)
    {
        // Initialize the list item and set its owner, parameters, and function
        // ptHandle->chState = 0;
        ptHandle->tListItem.pvOwner = ptHandle;
        ptHandle->pvParam = pvParam;
        ptHandle->pfcn = pfcn;
 
        mlist_InsertEnd(&tListCoroutine, &ptHandle->tListItem);
        ptHandle->bIsRunning = true;
    }
    else {
        wRet = MODUS_EAGAIN;
    }

    return wRet;
}

/**
 * Function: mcoroutine_Delete
 * ----------------------------
 * This function deletes a coroutine from the tListCoroutine list and sets its running state to false.
 *
 * Parameters: 
 * ptHandle: A pointer to the coroutine handle.
 *
 * Returns: 
 * A status code indicating the result of the function. MODUS_SUCCESS if the function succeeds.
 */
int mcoroutine_Delete(mcoroutine_handle_t *ptHandle)
{
    int wRet = MODUS_SUCCESS;
    // Remove the coroutine from the list
    mlist_Remove(&ptHandle->tListItem);
    // Set the running state to false
    ptHandle->bIsRunning = false;
    return wRet;
}

/**
 * Function: mcoroutine_Run
 * ----------------------------
 * This function runs all coroutines in the tListCoroutine list. For each coroutine, it calls its function 
 * with its parameters. If the coroutine's function returns fsm_rt_cpl, it deletes the coroutine from the list.
 *
 * Parameters: 
 * None
 *
 * Returns: 
 * A status code indicating the result of the function. MODUS_SUCCESS if the function succeeds.
 */
int mcoroutine_Run(void)
{
    int wRet = MODUS_SUCCESS;
    mcoroutine_handle_t *ptHandle;
    fsm_rt_t tFsm;
    const mlist_item_t *ptListItemDes;

    for(ptListItemDes = tListCoroutine.tListEnd.ptPrevious;                     \
                    ptListItemDes != &tListCoroutine.tListEnd;                  \
                    ptListItemDes = ptListItemDes->ptPrevious)
    {
        if(ptListItemDes->pvOwner == NULL) {
            continue;
        }

        ptHandle = (mcoroutine_handle_t *)ptListItemDes->pvOwner;
        MODUS_ASSERT(NULL != ptHandle->pfcn);
        tFsm = ptHandle->pfcn(ptHandle->pvParam);

        if(fsm_rt_cpl == tFsm)
        {
            mcoroutine_Delete(ptHandle);
        }
    }

    return wRet;
}
