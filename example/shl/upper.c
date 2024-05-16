#include "upper.h"
#include "userconfig.h"

int upper_Clock(uintptr_t wObjectAddr);
int upper_Run(uintptr_t wObjectAddr);

static gmsi_base_t s_tUpperBase;
gmsi_base_cfg_t s_tUpperBaseCfg = {
    .wId = UPPER,
    .wParent = 0,
    .FcnInterface = {
        .Clock = upper_Clock,
        .Run = upper_Run,
    },
};

static void upper_EventHandle(upper_t *ptThis, uint32_t wEvent)
{
    if(wEvent & Event_SyncMissed)
    {
        GLOG_PRINTF("get event Event_SyncMissed");
    }
    if(wEvent & Event_SyncButtonPushed)
    {

    }
    if(wEvent & Event_PacketReceived)
    {

    }
}
int upper_Run(uintptr_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;
    uint32_t wEvent;
 
    upper_t *ptThis = (upper_t *)wObjectAddr;
    GMSI_ASSERT(NULL != ptThis);

    // event handler
    wEvent = gbase_EventPend(ptThis->ptBase);
    if(wEvent)
        upper_EventHandle(ptThis, wEvent);
    
    // Logic or state machine programs

    return wRet;
}

int upper_Clock(uintptr_t wObjectAddr)
{
    // clock in 1ms
    return 0;
}

int upper_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    int wRet = GMSI_SUCCESS;
    // pointer conversion
    upper_t *ptThis = (upper_t *)wObjectAddr;
    upper_cfg_t *ptCfg = (upper_cfg_t *)wObjectCfgAddr;
    // check pointer
    GMSI_ASSERT(NULL != ptThis);
    GMSI_ASSERT(NULL != ptCfg);
    
    /* cfg data to object --start*/ 

    /* cfg data to object --end*/ 

    // regist object in gmsi list
    ptThis->ptBase = &s_tUpperBase;
    if(NULL == ptThis->ptBase)
        wRet = GMSI_EAGAIN;
    else
    {
        s_tUpperBaseCfg.wParent = wObjectAddr;
        wRet = gbase_Init(ptThis->ptBase, &s_tUpperBaseCfg);
    }

    /* end of add object in gmsi_lib*/ 
    return wRet;
}