#include "pc_clock.h"
#include "userconfig.h"
#include <stdint.h>
#include "gcoroutine.h"

timer_t timerid;
extern void timer_handler(int signum);
int pcclock_Run(uintptr_t wObjectAddr);
int pcclock_Clock(uintptr_t wObjectAddr);

gmsi_base_cfg_t tTimerBaseCfg = {
    .wId = PC_CLOCK,
    /* ��ȡ��ָ�� */
    .wParent = 0,
    .FcnInterface = {
        .Clock = pcclock_Clock,
        .Run = pcclock_Run,
    },
};
gmsi_base_t tBase;


fsm_rt_t pcclock_gcoroutine(void *pvParam)
{
    static uint8_t s_eState = 0;
    fsm_rt_t tFsm = fsm_rt_on_going;
    pc_clock_t *ptThis = (pc_clock_t *)pvParam;
    switch(s_eState)
    {
        case 0:
            //printf("entry");
            GLOG_PRINTF("ENTRY Gcoroutine");
            s_eState++;
        break;
        case 1:
            GLOG_PRINTF("finish gcoroutine");
            fsm_cpl();
        default:
        break;
    }
    return tFsm;
    
}
gcoroutine_handle_t tGcoroutineHandle = {
    .bIsRun = false,
    .pfcn = NULL,
};
int pcclock_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    struct sigaction sa;
    struct itimerspec its;
    pc_clock_t *ptThis = (pc_clock_t *)wObjectAddr;
    pc_clock_cfg_t* ptCfg = (pc_clock_cfg_t *)wObjectCfgAddr;

    // ����SIGALRM�źŵĴ�������
    sa.sa_handler = timer_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);

    // ������ʱ��
    timer_create(CLOCK_REALTIME, NULL, &timerid);

    // ���ö�ʱ���ĳ�ʼֵ�ͼ��
    its.it_value.tv_sec = INTERVAL_MS / 1000;
    its.it_value.tv_nsec = (INTERVAL_MS % 1000) * 1000000;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    // ������ʱ��
    timer_settime(timerid, 0, &its, NULL);
    ptThis->ptBase = &tBase;
    tTimerBaseCfg.wParent = wObjectAddr;

    if(GMSI_SUCCESS != gbase_Init(ptThis->ptBase, &tTimerBaseCfg))
        printf("pcclock base Init fail\n");
    return 0;
}

int pcclock_Run(uintptr_t wObjectAddr)
{
    uint32_t wEvent;
    pc_clock_t *ptThis = (pc_clock_t *)wObjectAddr;

    GMSI_ASSERT(NULL != ptThis);
    wEvent = gbase_EventPend(ptThis->ptBase);
    if(wEvent & Gmsi_Event_Transition)
    {
        printf("get message, length is %d\n", ptThis->ptBase->tMessage.hwLength);
        //GLOG_PRINTF(ptThis->ptBase->tMessage.pchMessage);
    }
    return 0;
}

int pcclock_Clock(uintptr_t wObjectAddr)
{
    static sig_atomic_t timeoutcount = 0;
    #if 1
    if(!timeoutcount)
    {
        // ��ʱ������ʱִ�еĲ���
        //printf("Timer expired\n");
        timeoutcount = 999;
        // ���͸�����һ���¼�
        gbase_EventPost(PC_UART, Gmsi_Event00);

        gcoroutine_Insert(&tGcoroutineHandle, (void *)wObjectAddr, pcclock_gcoroutine);
    }
    else
        timeoutcount--;
    #endif
    return 0;
}
int pcclock_Delete(void)
{
    return timer_delete(timerid);
}


