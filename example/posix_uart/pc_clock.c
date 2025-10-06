#include "pc_clock.h"
#include "userconfig.h"
#include <stdint.h>
#include "gcoroutine.h"
#include <perf_counter.h>

#include <signal.h>
#include <time.h>
#include <stdio.h>

timer_t timerid;
extern void timer_handler(int signum);
int pcclock_Run(uintptr_t wObjectAddr);
int pcclock_Clock(uintptr_t wObjectAddr);

gmsi_base_cfg_t tTimerBaseCfg = {
    .wId = PC_CLOCK,
    .wParent = 0,
    .FcnInterface = {
        .Clock = pcclock_Clock,
        .Run = pcclock_Run,
    },
};
gmsi_base_t tBase;

GMSI_MSG_ITEM_DECLARE(PC_CLOCK, Clockbuffer, 30);
gcoroutine_handle_t tGcoroutineHandle = {
    .bIsRunning = false,
    .pfcn = NULL,
};

fsm_rt_t pcclock_gcoroutine(void *pvParam)
{
    gcoroutine_handle_t *ptThis = (gcoroutine_handle_t *)&tGcoroutineHandle;
    pc_clock_t *ptObject = (pc_clock_t *)pvParam;
    uint8_t *pchMessage = NULL;

PERFC_PT_BEGIN(this.chState)
    do {
    PERFC_PT_WAIT_FOR_RES_UNTIL( 
        (pchMessage != NULL),               /* quit condition */
        pchMessage = malloc(100);          /* try to allocate memory */
    )

        printf("LED ON  [%lld]\r\n", get_system_ms());

    PERFC_PT_DELAY_MS(1000);
        
        printf("LED OFF [%lld]\r\n", get_system_ms());


    PERFC_PT_DELAY_MS(1000);
        
        free(pchMessage);
    } while(0);

PERFC_PT_END()

    return fsm_rt_cpl;
}

int pcclock_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    struct sigaction sa;
    struct itimerspec its;
    struct sigevent sev;
    pc_clock_t *ptThis = (pc_clock_t *)wObjectAddr;
    pc_clock_cfg_t* ptCfg = (pc_clock_cfg_t *)wObjectCfgAddr;

    sa.sa_handler = timer_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);

    /* Create a signal-based timer that delivers SIGALRM */
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    sev.sigev_value.sival_ptr = &timerid;

    if (timer_create(CLOCK_REALTIME, &sev, &timerid) != 0) {
        perror("timer_create failed\n");
        return -1;
    }

    its.it_value.tv_sec = INTERVAL_MS / 1000;
    its.it_value.tv_nsec = (INTERVAL_MS % 1000) * 1000000;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    if (timer_settime(timerid, 0, &its, NULL) != 0) {
        perror("timer_settime failed\n");
        timer_delete(timerid);
        return -1;
    }
    ptThis->ptBase = &tBase;
    tTimerBaseCfg.wParent = wObjectAddr;

    if(GMSI_SUCCESS != gbase_Init(ptThis->ptBase, &tTimerBaseCfg))
        printf("pcclock base Init failed\n");
    return 0;
}

uint8_t chClockBuffer[30];
message_t tMessage = {
    .pchMessage = chClockBuffer,
    .hwLength = 0,
    .hwMaxSize = 30,
};
GMSI_MSG_DECLARE(TestBuffer, 30);

int pcclock_Run(uintptr_t wObjectAddr)
{
    uint32_t wEvent;
    pc_clock_t *ptThis = (pc_clock_t *)wObjectAddr;
    
    GMSI_ASSERT(NULL != ptThis);
    wEvent = gbase_EventPend(ptThis->ptBase);
    if(wEvent & Gmsi_Event_Transition)
    {
        //printf("get message, length is %d\n", ptThis->ptBase->tMessage.hwLength);
        //GLOG_PRINTF(ptThis->ptBase->tMessage.pchMessage);
    }

    if(perfc_is_time_out_ms(5000))
    {
        gcoroutine_Insert(&tGcoroutineHandle, (void *)wObjectAddr, pcclock_gcoroutine);
    }
    if(perfc_is_time_out_ms(1000))
    {
        GLOG_PRINTF("insert coroutine");
    }
    return 0;
}

int pcclock_Clock(uintptr_t wObjectAddr)
{
    static sig_atomic_t timeoutcount = 0;
    #if 1
    if(!timeoutcount)
    {
        timeoutcount = 999;
        // gbase_EventPost(PC_UART, Gmsi_Event00);
        GLOG_PRINTF("post event to pc_uart");
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


