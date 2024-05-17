#include "upper.h"
#include "userconfig.h"

#define PORT    5001

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

    // accept connect
    if(ptThis->wSocket == accept(ptThis->wServerFd, (struct sockaddr *)&ptThis->tSocketAddr, (socklen_t *)&ptThis->wAddrLength) < 0)
    {
        GVAL_PRINTF(ptThis->wServerFd);
        GLOG_PRINTF("accept fail");
    }

    // clear buffer
    memset(ptThis->chBuffer, 0, sizeof(ptThis->chBuffer));
    // get ipc data
    read(ptThis->wSocket, ptThis->chBuffer, 1024);
    if(ptThis->chBuffer[0] != 0)
        printf("Received message: %s\n", ptThis->chBuffer);

    close(ptThis->wSocket);
    // event handler
    wEvent = gbase_EventPend(ptThis->ptBase);
    if(wEvent)
        upper_EventHandle(ptThis, wEvent);
    
    if(ptThis->hwTestCount == 1)
    {
        GLOG_PRINTF("timeout");
        ptThis->hwTestCount = 0;
    }
    // Logic or state machine programs

    return wRet;
}

int upper_Clock(uintptr_t wObjectAddr)
{
    upper_t *ptThis = (upper_t *)wObjectAddr;

    if(ptThis->hwTestCount > 1)
        ptThis->hwTestCount--;
    else if(!ptThis->hwTestCount)
    {
        ptThis->hwTestCount = 1000;
    }
    // clock in 1ms
    return 0;
}

int upper_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    int wRet = GMSI_SUCCESS;
    int wOpt = 1;
    // pointer conversion
    upper_t *ptThis = (upper_t *)wObjectAddr;
    upper_cfg_t *ptCfg = (upper_cfg_t *)wObjectCfgAddr;
    // check pointer
    GMSI_ASSERT(NULL != ptThis);
    GMSI_ASSERT(NULL != ptCfg);

    // regist object in gmsi list
    ptThis->ptBase = &s_tUpperBase;
    if(NULL == ptThis->ptBase)
        wRet = GMSI_EAGAIN;
    else
    {
        s_tUpperBaseCfg.wParent = wObjectAddr;
        wRet = gbase_Init(ptThis->ptBase, &s_tUpperBaseCfg);
    }

    /* cfg data to object --start */ 

    /* cfg data to object --end */ 

    /* hardware init --start */
    ptThis->wAddrLength = sizeof(ptThis->chBuffer);
    // creat socketfd
    if ((ptThis->wServerFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        GVAL_PRINTF(ptThis->wServerFd);
        GLOG_PRINTF("socket failed");
    }
    // setting socketfd
    #if 0
    if (setsockopt(ptThis->wServerFd, SOL_SOCKET, SO_REUSEADDR, &wOpt, sizeof(wOpt))) {
        GVAL_PRINTF(ptThis->wServerFd);
        GLOG_PRINTF("setsockopt fail");
    }
    #endif
    // setting address
    ptThis->tSocketAddr.sin_family = AF_INET;
    ptThis->tSocketAddr.sin_addr.s_addr = INADDR_ANY;
    ptThis->tSocketAddr.sin_port = htons(PORT);
    memset(&(ptThis->tSocketAddr.sin_zero), 0, sizeof(ptThis->tSocketAddr.sin_zero));
    // band fd
    if (bind(ptThis->wServerFd, (struct sockaddr *)&ptThis->tSocketAddr, sizeof(ptThis->tSocketAddr))<0) {
        GLOG_PRINTF("bind failed");
    }
    // listen
    if (listen(ptThis->wServerFd, 5) < 0) {
        GLOG_PRINTF("listen fail");
    }
    /* hardware init --end */

    /* end of add object in gmsi_lib*/ 
    return wRet;
}