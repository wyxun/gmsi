#include "pc_clock.h"
#include "userconfig.h"
#include <assert.h>

timer_t timerid;
extern void timer_handler(int signum);
int pcclock_Run(uint32_t wObjectAddr);
int pcclock_Clock(uint32_t wObjectAddr);

gmsi_base_cfg_t tTimerBaseCfg = {
    .wId = PC_CLOCK,
    /* 获取父指针 */
    .wParent = 5,
    .FcnInterface = {
        .Clock = pcclock_Clock,
        .Run = pcclock_Run,
    },
};
gmsi_base_t tBase;
//int pcclock_Init(pc_clock_t *ptThis, pc_clock_cfg_t *ptCfg)
int pcclock_Init(uint32_t wObjectAddr, uint32_t wObjectCfgAddr)
{
    struct sigaction sa;
    struct itimerspec its;
    pc_clock_t *ptThis = (pc_clock_t *)wObjectAddr;
    pc_clock_cfg_t* ptCfg = (pc_clock_cfg_t *)wObjectCfgAddr;

    // 设置SIGALRM信号的处理函数
    sa.sa_handler = timer_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);

    // 创建定时器
    timer_create(CLOCK_REALTIME, NULL, &timerid);

    // 设置定时器的初始值和间隔
    its.it_value.tv_sec = INTERVAL_MS / 1000;
    its.it_value.tv_nsec = (INTERVAL_MS % 1000) * 1000000;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    // 启动定时器
    timer_settime(timerid, 0, &its, NULL);
    ptThis->ptBase = &tBase;
    tTimerBaseCfg.wParent = wObjectAddr;

    if(GMSI_SUCCESS != gbase_Init(ptThis->ptBase, &tTimerBaseCfg))
        printf("pcclock_Init fail\n");
    return 0;
}

int pcclock_Run(uint32_t wObjectAddr)
{
    uint32_t wEvent;
    pc_clock_t *ptThis = (pc_clock_t *)wObjectAddr;

    assert(NULL != ptThis);
    wEvent = gbase_EventPend(ptThis->ptBase);
    if(wEvent & Gmsi_Event_Transition)
    {
        printf("get message, length is %d\n", ptThis->ptBase->tMessage.hwLength);
    }
}

int pcclock_Clock(uint32_t wObjectAddr)
{
    static sig_atomic_t timeoutcount = 0;
    #if 1
    if(!timeoutcount)
    {
        // 定时器触发时执行的操作
        //printf("Timer expired\n");
        timeoutcount = 999;
        // 发送给串口一个事件
        gbase_EventPost(PC_UART, Gmsi_Event00);
    }
    else
        timeoutcount--;
    #endif
}
int pcclock_Delete(void)
{
    return timer_delete(timerid);
}


