/*============================ INCLUDES ======================================*/
#include "template.h"
#include "perf_counter.h"
#include "userconfig.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ PROTOTYPES ====================================*/

int template_Clock(uintptr_t wObjectAddr);
int template_Run(uintptr_t wObjectAddr);

/*============================ GLOBAL VARIABLES ==============================*/

mcoroutine_handle_t tGcoroutineTemplateHandle = {
    .bIsRunning = false,
    .pfcn = NULL,
};

/*============================ LOCAL VARIABLES ===============================*/

static modus_base_t s_tTemplateBase;

static modus_base_cfg_t s_tTemplateBaseCfg = {
    .wId    = TEMPLATE,
    .wParent = 0,
    .FcnInterface = {
        .Clock = template_Clock,
        .Run   = template_Run,
    },
};

/*============================ IMPLEMENTATION ================================*/

/* coroutine triggered by Event_SyncButtonPushed */
fsm_rt_t template_mcoroutine(void *pvParam)
{
    template_t *ptObject = (template_t *)pvParam;
    mcoroutine_handle_t *ptThis = (mcoroutine_handle_t *)&tGcoroutineTemplateHandle;

PERFC_PT_BEGIN(this.chState)
    do {
    PERFC_PT_WAIT_FOR_RES_UNTIL(
        (ptObject != NULL),
        ptObject = (template_t *)pvParam;
    )
    MLOG(I, "Template Coroutine Running");
    PERFC_PT_DELAY_MS(1000);
    MLOG(D, "Template Coroutine delay 1s");
    PERFC_PT_DELAY_MS(500);
    MLOG(D, "Template Coroutine delay 0.5s");
    } while(0);
PERFC_PT_END()

    return fsm_rt_cpl;
}

/* handle events posted to the template object */
static void template_EventHandle(template_t *ptThis, uint32_t wEvent)
{
    MODUS_MSG_DECLARE(TemplateTestBuffer, 20);

    if (ptThis == NULL) {
        MLOG(E, "ptThis is NULL.");
        return;
    }

    if (wEvent & Event_SyncMissed) {
        MLOG(D, "get event Event_SyncMissed");
    }

    if (wEvent & Event_SyncButtonPushed) {
        if (MODUS_SUCCESS != mcoroutine_Insert(&tGcoroutineTemplateHandle,
                                              (void *)ptThis,
                                              template_mcoroutine)) {
            MLOG(E, "Error: mcoroutine_Insert failed.");
        }
    }

    if (wEvent & Event_PacketReceived) {
        /* handle Event_PacketReceived */
    }
}

/* called in the MODUS while(1) main loop */
int template_Run(uintptr_t wObjectAddr)
{
    int      wRet = MODUS_SUCCESS;
    uint32_t wEvent;
    uint8_t  chTemplateBufferTest[] = {
        11, 22, 33, 44, 55, 66, 77, 88, 99,
        0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14
    };

    template_t *ptThis = (template_t *)wObjectAddr;
    if (ptThis == NULL) {
        MLOG(E, "ptThis is NULL.");
        return MODUS_EFAIL;
    }

    if (perfc_is_time_out_ms(3000)) {
        mbase_MessagePostToRing(EXAMPLE,
                                (uint8_t *)chTemplateBufferTest,
                                sizeof(chTemplateBufferTest));
    }

    share_mem_t *ptShareMem = mbase_ShareMemRead(EXAMPLE);
    if (ptShareMem != NULL) {
        const example_share_mem_t *ptGetExampleShareData =
            (example_share_mem_t *)ptShareMem->pchBuffer;
        if (ptGetExampleShareData->value2 != 5) {
            //MLOG(E, "Error: Shared memory values are not as expected.");
        } else {
            MLOG(I, "Shared memory values are correct.");
        }
    } else {
        MLOG(E, "Error: ptShareMem is NULL.");
    }

    wEvent = mbase_EventPend(ptThis->ptBase);
    if (wEvent) {
        template_EventHandle(ptThis, wEvent);
    }

    /* logic or state machine programs */

    return wRet;
}

/* called in the 1ms system tick interrupt */
int template_Clock(uintptr_t wObjectAddr)
{
    template_t *ptThis = (template_t *)wObjectAddr;
    int wRet = MODUS_SUCCESS;

    /* perform periodic operations on ptThis */

    return wRet;
}

/* initialize template object and register it in the MODUS list */
int template_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    template_t     *ptThis = (template_t *)wObjectAddr;
    template_cfg_t *ptCfg  = (template_cfg_t *)wObjectCfgAddr;

    if (ptThis == NULL || ptCfg == NULL) {
        MLOG(E, "Error: ptThis or ptCfg is NULL.");
        return MODUS_EFAIL;
    }

    /* initialize the hardware */

    ptThis->ptBase = &s_tTemplateBase;
    if (ptThis->ptBase == NULL) {
        return MODUS_EAGAIN;
    }

    s_tTemplateBaseCfg.wParent = wObjectAddr;
    if (ptCfg->pchRingBuffer != NULL && ptCfg->hwRingSize != 0) {
        perfc_with(ptThis->ptBase) {
            _->tRingBuffer.buffer      = ptCfg->pchRingBuffer;
            _->tRingBuffer.hwBufferSize = ptCfg->hwRingSize;
            _->tRingBuffer.hwWriteIndex = 0;
            _->tRingBuffer.hwReadIndex  = 0;
        };
    }

    return mbase_Init(ptThis->ptBase, &s_tTemplateBaseCfg);
}

MODUS_DECLARE_OBJECT(template, Template, 
    .hwRingSize = 0,
    .pchRingBuffer = NULL,
);
