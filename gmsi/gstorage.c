#include "global_define.h"
#include "glog.h"
#include <stdio.h>
#include <stdint.h>
#include "gstorage.h"
#include "utilities/util_debug.h"

int gstorage_Run(uint32_t wObjectAddr);
int gstorage_Clock(uint32_t wObjectAddr);

static gmsi_base_t s_tStorageBase;
gmsi_base_cfg_t s_tStorageBaseCfg = {
    .wId = GMSI_STORAGE,
    /* 获取父指针 */
    .wParent = 0,
    .FcnInterface = {
        .Clock = gstorage_Clock,
        .Run = gstorage_Run,
    },
};

void gstorage_EventHandle(gstorage_t *ptThis, uint32_t wEvent)
{
    if(wEvent & Event_Storage)
    {
        GLOG_PRINTF("get Event_Storage");
        ptThis->fcnWrite(ptThis->phwStorageStartAddr, ptThis->hwStorageLength);
    }
    if(wEvent & Event_GetData)
    {
        GLOG_PRINTF("get Event_GetData");
        ptThis->fcnRead(ptThis->phwStorageStartAddr, ptThis->hwStorageLength);
    }
}

int gstorage_Run(uint32_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;
    uint32_t wEvent;
    // 指针检查
    gstorage_t *ptThis = (gstorage_t *)wObjectAddr;
    GMSI_ASSERT(NULL != ptThis);

    // 事件处理
    wEvent = gbase_EventPend(ptThis->ptBase);
    if(wEvent)
        gstorage_EventHandle(ptThis, wEvent);
    
    // 逻辑程序或状态机程序

    return wRet;
}

int gstorage_Clock(uint32_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;

    // 指针检查
    gstorage_t *ptThis = (gstorage_t *)wObjectAddr;
    GMSI_ASSERT(NULL != ptThis);

    return wRet;
}

int gstorage_Init(uint32_t wObjectAddr, uint32_t wObjectCfgAddr)
{
    int wRet = GMSI_SUCCESS;
    // 获取指针
    gstorage_t *ptThis = (gstorage_t *)wObjectAddr;
    gstorage_cfg_t *ptCfg = (gstorage_cfg_t *)wObjectCfgAddr;
    GMSI_ASSERT(NULL != ptThis);
    GMSI_ASSERT(NULL != ptCfg);

    ptThis->phwStorageStartAddr = ptCfg->phwStorageStartAddr;
    ptThis->hwStorageLength = ptCfg->hwStorageLength;

    GMSI_ASSERT(NULL != ptCfg->fcnRead);
    ptThis->fcnRead = ptCfg->fcnRead;

    GMSI_ASSERT(NULL != ptCfg->fcnWrite);
    ptThis->fcnWrite = ptCfg->fcnWrite;

    ptThis->ptBase = &s_tStorageBase;

    // 资源不可用
    if(NULL == ptThis->ptBase)
        wRet = GMSI_EAGAIN;
    else
    {
        // 设定父指针
        s_tStorageBaseCfg.wParent = wObjectAddr;
        wRet = gbase_Init(ptThis->ptBase, &s_tStorageBaseCfg);
    }
    GLOG_PRINTF("gstorage init ok");

    return wRet;
}