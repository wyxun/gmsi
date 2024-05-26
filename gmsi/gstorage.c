#include "global_define.h"
#include "glog.h"
#include <stdio.h>
#include <stdint.h>
#include "gstorage.h"
#include "utilities/util_debug.h"

int gstorage_Run(uintptr_t wObjectAddr);
int gstorage_Clock(uintptr_t wObjectAddr);

// Define a global storage base of type gmsi_base_t
static gmsi_base_t s_tStorageBase;
// Define and initialize a global storage base configuration of type gmsi_base_cfg_t
gmsi_base_cfg_t s_tStorageBaseCfg = {
    // Set the ID to GMSI_STORAGE
    .wId = GMSI_STORAGE,
    .wParent = 0,
    // Set the interface functions
    .FcnInterface = {
        .Clock = gstorage_Clock,
        .Run = gstorage_Run,
    },
};

/**
 * Function: gstorage_EventHandle
 * ----------------------------
 * This function handles storage events. If the event includes Event_Storage, it writes data to the storage.
 * If the event includes Event_GetData, it reads data from the storage.
 *
 * Parameters: 
 * ptThis: A pointer to the gstorage_t object.
 * wEvent: The event to be handled.
 *
 * Returns: 
 * None
 */
void gstorage_EventHandle(gstorage_t *ptThis, uint32_t wEvent)
{
    // Check if ptThis and ptThis->ptData are not NULL
    if (ptThis == NULL || ptThis->ptData == NULL) {
        GLOG_PRINTF("Error: ptThis or ptThis->ptData is NULL.\n");
        return;
    }

    if(wEvent & Event_Storage)
    {
        GLOG_PRINTF("get Event_Storage");
        ptThis->ptData->fcnWrite(ptThis->ptData->phwStorageStartAddr, ptThis->ptData->hwStorageLength);
    }
    if(wEvent & Event_GetData)
    {
        GLOG_PRINTF("get Event_GetData");
        ptThis->ptData->fcnRead(ptThis->ptData->phwStorageStartAddr, ptThis->ptData->hwStorageLength);
    }
}

/**
 * Function: gstorage_Run
 * ----------------------------
 * This function runs the storage object. It gets the events of the object and handles them.
 *
 * Parameters: 
 * wObjectAddr: The address of the storage object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully.
 */
int gstorage_Run(uintptr_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;
    uint32_t wEvent;

    gstorage_t *ptThis = (gstorage_t *)wObjectAddr;

    // Check if ptThis and ptThis->ptBase are not NULL
    if (ptThis == NULL || ptThis->ptBase == NULL) {
        GLOG_PRINTF("Error: ptThis or ptThis->ptBase is NULL.\n");
        return GMSI_EFAIL;
    }

    wEvent = gbase_EventPend(ptThis->ptBase);
    if(wEvent)
        gstorage_EventHandle(ptThis, wEvent);

    return wRet;
}

/**
 * Function: gstorage_Clock
 * ----------------------------
 * This function is the clock function of the storage object.
 *
 * Parameters: 
 * wObjectAddr: The address of the storage object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully.
 */
int gstorage_Clock(uintptr_t wObjectAddr)
{
    gstorage_t *ptThis = (gstorage_t *)wObjectAddr;

    // Check if ptThis is not NULL
    if (ptThis == NULL) {
        GLOG_PRINTF("Error: ptThis is NULL.\n");
        return GMSI_EFAIL;
    }

    return GMSI_SUCCESS;
}

/**
 * Function: gstorage_Init
 * ----------------------------
 * This function initializes the storage object.
 *
 * Parameters: 
 * wObjectAddr: The address of the storage object.
 * wObjectCfgAddr: The address of the storage configuration object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully.
 */
int gstorage_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    int wRet = GMSI_SUCCESS;

    gstorage_t *ptThis = (gstorage_t *)wObjectAddr;
    gstorage_cfg_t *ptCfg = (gstorage_cfg_t *)wObjectCfgAddr;

    // Check if ptThis and ptCfg are not NULL
    if (ptThis == NULL || ptCfg == NULL) {
        GLOG_PRINTF("Error: ptThis or ptCfg is NULL.");
        return GMSI_EFAIL;
    }

    ptThis->ptData = ptCfg->ptData;
    ptThis->ptBase = &s_tStorageBase;

    if(NULL == ptThis->ptBase)
    {
        GLOG_PRINTF("Error: ptThis->ptBase is NULL.");
        return GMSI_EFAIL;
    }
    else
    {
        s_tStorageBaseCfg.wParent = wObjectAddr;
        wRet = gbase_Init(ptThis->ptBase, &s_tStorageBaseCfg);
        if(wRet < 0)
        {
            GERR_PRINTF(wRet);
            return wRet;
        }
    }

    return wRet;
}