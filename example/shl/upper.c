#include "upper.h"
#include "userconfig.h"
#include <errno.h>
#include <sysexits.h>
#include <unistd.h>
#include <fcntl.h>

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

gcoroutine_handle_t tGcoroutineUpperConnectClientHandle = {
    .bIsRunning = false,
    .pfcn = NULL,
};

void upper_MsgDataHandle(upper_t *ptThis)
{
    // Send data downwards based on the instruction
    gbase_EventPost(ptThis->ptBase, Event_PacketReceived);
    gbase_MessagePost(ptThis->ptBase, ptThis->chBuffer, ptThis->wAddrLength);
}

fsm_rt_t upper_GcoroutineConnectClient(void *pvParam)
{
    static uint8_t s_eState = 0;
    fsm_rt_t tFsm = fsm_rt_on_going;
    upper_t *ptThis = (upper_t *)pvParam;
    socklen_t tClientLen;
    int16_t hwLength = 0;
    // Check if ptThis is not NULL
    if (ptThis == NULL) {
        GLOG_PRINTF("Error: ptThis is NULL.\n");
        return fsm_rt_err;
    }

    switch(s_eState)
    {
        case 0:
            tClientLen = sizeof(ptThis->tClinetAddr);
            ptThis->wClinetFd = accept(ptThis->wServerFd, (struct sockaddr *)&ptThis->tClinetAddr, &tClientLen);

            if(ptThis->wClinetFd < 0)
            {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    //GLOG_PRINTF("Error EINTR");
                }else if(errno == EINTR){
                    GLOG_PRINTF("Error EINTR");
                } else {
                    GLOG_PRINTF("Error accepting connection");
                }
                fsm_on_going(); 
            }
            else {
                s_eState++;
                GLOG_PRINTF("Client connected");
            }
            break;
        case 1:
            memset(ptThis->chBuffer, 0, sizeof(ptThis->chBuffer));
            //ssize_t num_bytes = read(client_sockfd, buffer, sizeof(buffer) - 1);
            hwLength = read(ptThis->wClinetFd, ptThis->chBuffer, sizeof(ptThis->chBuffer));
            if (hwLength < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    GLOG_PRINTF("Error EINTR");
                } else if(errno == EINTR){
                }else {
                    perror("Error reading from socket");
                }
                fsm_on_going(); 
            } else if (hwLength == 0) {
                printf("Client disconnected\n");
                s_eState = 0;
                break;
            }
            upper_MsgDataHandle(ptThis);
            printf("Received message from client: %s\n", ptThis->chBuffer);
            // Echo the message back to the client
            if (write(ptThis->wClinetFd, ptThis->chBuffer, hwLength) < 0) {
                GLOG_PRINTF("Error writing to socket");
            }
            break;
        default:
            fsm_cpl();
        break;
    }
    fsm_on_going(); 
}

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
    int16_t hwLength = 0;
    upper_t *ptThis = (upper_t *)wObjectAddr;
    GMSI_ASSERT(NULL != ptThis);
 
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

    // set socket nonblock
    int flags = fcntl(ptThis->wServerFd, F_GETFL);
    if (flags < 0) {
        perror("Error getting socket flags");
        exit(1);
    }
    if (fcntl(ptThis->wServerFd, F_SETFL, flags | SOCK_NONBLOCK) < 0) {
        perror("Error setting socket flags");
        exit(1);
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
    ptThis->hwTestCount = 4999;
    // start coroutine
    if(GMSI_SUCCESS != gcoroutine_Insert(&tGcoroutineUpperConnectClientHandle, \
                        (void *)ptThis, upper_GcoroutineConnectClient))
    {
        //GLOG_PRINTF("Error: gcoroutine_Insert failed.");
    }
    /* end of add object in gmsi_lib*/ 
    return wRet;
}