#include "example.h"
#include "userconfig.h"

int example_Clock(uintptr_t wObjectAddr);
int example_Run(uintptr_t wObjectAddr);

static gmsi_base_t s_tExampleBase;
gmsi_base_cfg_t s_tExampleBaseCfg = {
    .wId = EXAMPLE,
    .wParent = 0,
    .FcnInterface = {
        .Clock = example_Clock,
        .Run = example_Run,
    },
};

static void example_EventHandle(example_t *ptThis, uint32_t wEvent)
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
int example_Run(uintptr_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;
    uint32_t wEvent;
 
    example_t *ptThis = (example_t *)wObjectAddr;
    GMSI_ASSERT(NULL != ptThis);

    // event handler
    wEvent = gbase_EventPend(ptThis->ptBase);
    if(wEvent)
        example_EventHandle(ptThis, wEvent);
    
    // Logic or state machine programs

    return wRet;
}

int example_Clock(uintptr_t wObjectAddr)
{
    // clock in 1ms
    return 0;
}

int example_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    int wRet = GMSI_SUCCESS;
    // pointer conversion
    example_t *ptThis = (example_t *)wObjectAddr;
    example_cfg_t *ptCfg = (example_cfg_t *)wObjectCfgAddr;
    // check pointer
    GMSI_ASSERT(NULL != ptThis);
    GMSI_ASSERT(NULL != ptCfg);
    
    /* cfg members to object --start * / 

    /* cfg members to object --end */ 

    /* hardware init --start */

    /* hardware init --end */
    
    // regist object in gmsi list
    ptThis->ptBase = &s_tExampleBase;
    if(NULL == ptThis->ptBase)
        wRet = GMSI_EAGAIN;
    else
    {
        s_tExampleBaseCfg.wParent = wObjectAddr;
        wRet = gbase_Init(ptThis->ptBase, &s_tExampleBaseCfg);
    }

    /* end of add object in gmsi_lib*/ 
    return wRet;
}