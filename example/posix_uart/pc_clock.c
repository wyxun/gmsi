#include "pc_clock.h"

timer_t timerid;
extern void timer_handler(int signum);

gmsi_base_cfg_t tTimerBaseCfg = {
    .wId = GMSI_ID_TIMER << 8 + 1,
};
gmsi_base_t tBase;
int pcclock_Init(pc_clock_t *ptThis, pc_clock_cfg_t *ptCfg)
{
    struct sigaction sa;
    struct itimerspec its;

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
    gbase_Init(ptThis->ptBase, &tTimerBaseCfg);
    return 0;
}

int pcclock_Run(pc_clock_t *ptThis)
{
    uint32_t wEvent;
    wEvent = gbase_EventGet(ptThis->ptBase);
    if(wEvent & Gmsi_Event_Transition)
    {
        printf("get message, length is %d\n", ptThis->ptBase->tMessage.hwLength);
    }

}
int pcclock_Delete(void)
{
    return timer_delete(timerid);
}


void timer_handler(int signum)
{
    static sig_atomic_t timeoutcount = 0;
    #if 1
    if(!timeoutcount)
    {
        // 定时器触发时执行的操作
        //printf("Timer expired\n");
        timeoutcount = 999;
        // 发送给串口一个事件
        gbase_EventSend(GMSI_ID_UART << 8 + 1, Gmsi_Event00);
    }
    else
        timeoutcount--;
    #endif
}

