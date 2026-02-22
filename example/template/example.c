/*============================ INCLUDES ======================================*/
#include "example.h"
#include "userconfig.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

int example_Clock(uintptr_t wObjectAddr);
int example_Run(uintptr_t wObjectAddr);

/*============================ GLOBAL VARIABLES ==============================*/

example_share_mem_t tExampleShareMem;

share_mem_t s_tExampleShareMem = {
    .pchBuffer = (uint8_t *)&tExampleShareMem,
    .hwSize    = sizeof(tExampleShareMem),
};

gcoroutine_handle_t tGcoroutineExampleHandle = {
    .bIsRunning = false,
    .pfcn = NULL,
};

/*============================ LOCAL VARIABLES ===============================*/

static gmsi_base_t s_tExampleBase;

static gmsi_base_cfg_t s_tExampleBaseCfg = {
    .wId    = EXAMPLE,
    .wParent = 0,
    .FcnInterface = {
        .Clock = example_Clock,
        .Run   = example_Run,
    },
    .ptShareMem = &s_tExampleShareMem,
};

/*============================ IMPLEMENTATION ================================*/

/* coroutine triggered by Event_SyncButtonPushed */
fsm_rt_t example_gcoroutine(void *pvParam)
{
    static uint8_t s_eState = 0;
    example_t *ptObject = (example_t *)pvParam;
    gcoroutine_handle_t *ptThis = (gcoroutine_handle_t *)&tGcoroutineExampleHandle;

PERFC_PT_BEGIN(this.chState)
    do {
    PERFC_PT_WAIT_FOR_RES_UNTIL(
        (ptObject != NULL),
        ptObject = (example_t *)pvParam;
    )
        printf("Template Coroutine Running\r\n");
    PERFC_PT_DELAY_MS(1000);

    PERFC_PT_DELAY_MS(1000);

    } while(0);
PERFC_PT_END()

    return fsm_rt_cpl;
}

/* handle events posted to the example object */
static void example_EventHandle(example_t *ptThis, uint32_t wEvent)
{
    if (ptThis == NULL) {
        GLOG_PRINTF("ptThis is NULL.");
        return;
    }

    if (wEvent & Event_SyncMissed) {
        GLOG_PRINTF("get event Event_SyncMissed");
    }

    if (wEvent & Event_SyncButtonPushed) {
        if (GMSI_SUCCESS != gcoroutine_Insert(&tGcoroutineExampleHandle,
                                              (void *)ptThis,
                                              example_gcoroutine)) {
            GLOG_PRINTF("Error: gcoroutine_Insert failed.");
        }
    }
}

/* called in the GMSI while(1) main loop */
int example_Run(uintptr_t wObjectAddr)
{
    int      wRet = GMSI_SUCCESS;
    uint32_t wEvent;
    uint8_t  chRingBufferMsg[16];

    example_t *ptThis = (example_t *)wObjectAddr;
    if (ptThis == NULL) {
        GLOG_PRINTF("ptThis is NULL.");
        return GMSI_EFAIL;
    }

    wEvent = gbase_EventPend(ptThis->ptBase);
    if (wEvent) {
        example_EventHandle(ptThis, wEvent);
    }

    uint16_t hwLength = gbase_MessagePendFromRing(ptThis->ptBase,
                                                  chRingBufferMsg,
                                                  sizeof(chRingBufferMsg));
    if (hwLength > 0) {
        GLOG_PRINTF("get chRingBufferMsg");
        GVAL_PRINTF(hwLength);
        tExampleShareMem.value2 = hwLength;
    }

    /* logic or state machine programs */

    return wRet;
}

/* called in the 1ms system tick interrupt */
int example_Clock(uintptr_t wObjectAddr)
{
    example_t *ptThis = (example_t *)wObjectAddr;
    uint16_t hwExampleTestCount  = 2000;    /* 2000ms send a message */
    uint16_t hwExampleTestCount2 = 5000;    /* 5000ms send Event_PacketReceived */
    int wRet = GMSI_SUCCESS;

    if (!hwExampleTestCount2) {
        gbase_EventPost(EXAMPLE, Event_PacketReceived);
        hwExampleTestCount2 = 5000;
    } else {
        hwExampleTestCount2--;
    }

    return wRet;
}

/* initialize example object and register it in the GMSI list */
int example_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    example_t     *ptThis = (example_t *)wObjectAddr;
    example_cfg_t *ptCfg  = (example_cfg_t *)wObjectCfgAddr;

    if (ptThis == NULL || ptCfg == NULL) {
        GLOG_PRINTF("Error: ptThis or ptCfg is NULL.");
        return GMSI_EFAIL;
    }

    /* initialize the hardware */

    ptThis->ptBase = &s_tExampleBase;
    if (ptThis->ptBase == NULL) {
        return GMSI_EAGAIN;
    }

    s_tExampleBaseCfg.wParent = wObjectAddr;
    if (ptCfg->pchRingBuffer != NULL && ptCfg->hwRingSize != 0) {
        with(ptThis->ptBase) {
            _->tRingBuffer.buffer      = ptCfg->pchRingBuffer;
            _->tRingBuffer.hwBufferSize = ptCfg->hwRingSize;
            _->tRingBuffer.hwWriteIndex = 0;
            _->tRingBuffer.hwReadIndex  = 0;
        };
    }

    return gbase_Init(ptThis->ptBase, &s_tExampleBaseCfg);
}