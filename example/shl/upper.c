#include "upper.h"
#include "userconfig.h"

#define PORT    5001

#define SOCKET_PATH "/tmp/ipc_socket"

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
    socklen_t tClientLen;
    upper_t *ptThis = (upper_t *)wObjectAddr;
    GMSI_ASSERT(NULL != ptThis);
    tClientLen = sizeof(ptThis->tClinetAddr);
    // accept connect
    ptThis->wClinetFd = accept(ptThis->wServerFd, (struct sockaddr *)&ptThis->tClinetAddr, &tClientLen);

    if(ptThis->wClinetFd > 0)
    {
        memset(ptThis->chBuffer, 0, sizeof(ptThis->chBuffer));
        uint16_t hwLength = read(ptThis->wClinetFd, ptThis->chBuffer, sizeof(ptThis->chBuffer));
        if(hwLength)
        {
            printf("Received message from client: %s\n", ptThis->chBuffer);
            gbase_EventPost(GMSI_STORAGE, Event_Storage);
            write(ptThis->wClinetFd, ptThis->chBuffer, hwLength);
        }
    }  
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
        ptThis->hwTestCount = 4999;
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
    char buf[30];
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
    ptThis->wServerFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (ptThis->wServerFd < 0) {
        GVAL_PRINTF(ptThis->wServerFd);
        GLOG_PRINTF("socket failed");
    }
    // band socket
    memset(&ptThis->tServerAddr, 0, sizeof(ptThis->tServerAddr));
    ptThis->tServerAddr.sun_family = AF_UNIX;
    strncpy(ptThis->tServerAddr.sun_path, SOCKET_PATH, sizeof(ptThis->tServerAddr.sun_path) - 1);
    // makesure path ok
    unlink(SOCKET_PATH);    
    if(bind(ptThis->wServerFd, (struct sockaddr *)&ptThis->tServerAddr, sizeof(ptThis->tServerAddr)) < 0)
    {
        GLOG_PRINTF("bind failed");
        exit(1);
    }

    // listen
    if(listen(ptThis->wServerFd, 5) < 0)
    {
        GLOG_PRINTF("listen failed");
        exit(1);
    }
    printf("Server is running and waiting for connections...\n");
    /* hardware init --end */

    /* end of add object in gmsi_lib*/ 
    return wRet;
}