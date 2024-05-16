#include "lower.h"
#include "userconfig.h"

int lower_Clock(uintptr_t wObjectAddr);
int lower_Run(uintptr_t wObjectAddr);

static gmsi_base_t s_tLowerBase;
gmsi_base_cfg_t s_tLowerBaseCfg = {
    .wId = LOWER,
    .wParent = 0,
    .FcnInterface = {
        .Clock = lower_Clock,
        .Run = lower_Run,
    },
};

static void lower_EventHandle(lower_t *ptThis, uint32_t wEvent)
{
    if(wEvent & Event_PacketReceived)
    {
        GLOG_PRINTF("Event_PacketReceived");
    }
}
int lower_Run(uintptr_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;
    uint32_t wEvent;
 
    lower_t *ptThis = (lower_t *)wObjectAddr;
    GMSI_ASSERT(NULL != ptThis);

    wEvent = gbase_EventPend(ptThis->ptBase);
    if(wEvent)
        lower_EventHandle(ptThis, wEvent);
    
    // Logic or state machine programs

    return wRet;
}

int lower_Clock(uintptr_t wObjectAddr)
{
    return 0;
}

int lower_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    int wRet = GMSI_SUCCESS;
    lower_t *ptThis = (lower_t *)wObjectAddr;
    lower_cfg_t *ptCfg = (lower_cfg_t *)wObjectCfgAddr;
    GMSI_ASSERT(NULL != ptThis);
    GMSI_ASSERT(NULL != ptCfg);

    // Open the serial port device
    ptThis->fd = open(ptCfg->pchCom, ptCfg->wOflag);
    if (ptThis->fd == -1)
    {
        wRet = GMSI_EAGAIN;
    }
    // Configure the serial port parameters
    if (tcgetattr(ptThis->fd, &ptCfg->serialAttr) == -1)
    {
        close(ptThis->fd);
        wRet = GMSI_EAGAIN;
    }

    cfmakeraw(&ptCfg->serialAttr);
    cfsetspeed(&ptCfg->serialAttr, B9600);
    ptCfg->serialAttr.c_cflag |= CLOCAL | CREAD;
    ptCfg->serialAttr.c_cc[VMIN] = 1;
    ptCfg->serialAttr.c_cc[VTIME] = 0;
    if (tcsetattr(ptThis->fd, TCSANOW, &ptCfg->serialAttr) == -1)
    {
        close(ptThis->fd);
        return 1;
    }
    // Asynchronously read data
    fcntl(ptThis->fd, F_SETFL, O_RDWR | O_NONBLOCK);

    // regist object in gmsi list
    ptThis->ptBase = &s_tLowerBase;
    if(NULL == ptThis->ptBase)
        wRet = GMSI_EAGAIN;
    else
    {
        s_tLowerBaseCfg.wParent = wObjectAddr;
        wRet = gbase_Init(ptThis->ptBase, &s_tLowerBaseCfg);
    }

    return wRet;
}