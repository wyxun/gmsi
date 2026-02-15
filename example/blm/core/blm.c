/**
 * @file blm.c
 * @brief BLM Bootloader Core Implementation
 * 
 * Uses PERFC_PT state machine following perf_counter pt_example.c conventions
 */

/*============================ INCLUDES ======================================*/
#include "blm.h"
#include "blm_protocol.h"
#include "../port/blm_port.h"
#include "../cmsis/cmsis_compiler.h"
#include "gblinfo.h"
#include <string.h>

#if defined(__IS_COMPILER_ARM_COMPILER_5__)
#   pragma diag_suppress 550, 546, 111
#endif

/*============================ MACROS ========================================*/
#undef this
#define this    (*ptThis)

/*============================ GLOBAL VARIABLES ==============================*/
static gmsi_base_t s_tBlmBase;
static blm_protocol_cb_t s_tProtocol;

/* Buffer for receive operations */
static uint8_t s_achRecvData[YMODEM_DATA_SIZE];
static uint16_t s_hwRecvLen;

static gmsi_base_cfg_t s_tBlmBaseCfg = {
    .wId = BLM,
    .wParent = 0,
    .FcnInterface = {
        .Clock = blm_Clock,
        .Run = blm_Run,
    },
};

/*============================ IMPLEMENTATION ================================*/

/**
 * @brief Check if should enter upgrade mode
 */
static int blm_ShouldEnterUpgrade(blm_cb_t *ptThis)
{
    /* Check upgrade button */
    if (blm_port_IsUpgradeButtonPressed()) {
        return 1;
    }
    
    /* Check upgrade flag from shared info */
    gblinfo_shared_t *ptShared = (gblinfo_shared_t *)(this.wSharedInfoAddr);
    if (ptShared->wMagic == GBLINFO_MAGIC && ptShared->chUpgradeFlag != 0) {
        /* Clear the flag */
        ptShared->chUpgradeFlag = 0;
        return 1;
    }
    
    return 0;
}

/**
 * @brief Check if valid application exists
 */
static int blm_IsAppValid(blm_cb_t *ptThis)
{
    /* Check if vector table looks valid */
    uint32_t wStackPtr = *(volatile uint32_t *)(this.wAppAddr);
    uint32_t wResetHandler = *(volatile uint32_t *)(this.wAppAddr + 4);
    
    /* Stack pointer should be in RAM range */
    if ((wStackPtr & 0xFFF00000) != 0x20000000) {
        return 0;
    }
    
    /* Reset handler should be in flash range */
    if ((wResetHandler & 0xFF000000) != 0x08000000) {
        return 0;
    }
    
    return 1;
}

/**
 * @brief Initialize bootloader task control block
 */
blm_cb_t *blm_task_init(blm_cb_t *ptThis)
{
    if (NULL == ptThis) {
        return NULL;
    }
    
    this.chState = 0;
    this.tMainState = BLM_STATE_IDLE;
    this.chRetryCount = 0;
    
    blm_protocol_Init(&s_tProtocol, this.pchRxBuffer, this.hwRxBufferSize);
    
    return ptThis;
}

/**
 * @brief Main bootloader task using PERFC_PT state machine
 * 
 * Following perf_counter pt_example.c convention
 */
fsm_rt_t blm_task(blm_cb_t *ptThis)
{
    fsm_rt_t tResult;

PERFC_PT_BEGIN(this.chState)

    do {
        /* ============ STATE: IDLE ============ */
        if (blm_ShouldEnterUpgrade(ptThis)) {
            /* Enter upgrade mode */
            this.tMainState = BLM_STATE_WAIT_CONNECT;
            this.wStartTime = blm_port_GetTickMs();
            this.chRetryCount = 0;
            blm_protocol_Init(&s_tProtocol, this.pchRxBuffer, this.hwRxBufferSize);
        } else if (blm_IsAppValid(ptThis)) {
            /* Jump to application */
            this.tMainState = BLM_STATE_JUMP_APP;
            goto label_jump_app;
        } else {
            /* No valid app, wait for upgrade */
            this.tMainState = BLM_STATE_WAIT_CONNECT;
            this.wStartTime = blm_port_GetTickMs();
            blm_protocol_Init(&s_tProtocol, this.pchRxBuffer, this.hwRxBufferSize);
        }

        /* ============ STATE: WAIT_CONNECT ============ */
        this.chRetryCount = 0;
        do {
            /* Send 'C' to initiate connection */
            blm_protocol_SendC();
            
            /* Wait for packet 0 (file info) with PT */
        PERFC_PT_WAIT_UNTIL(
            (tResult == fsm_rt_cpl),
            tResult = blm_protocol_ReceivePacket(&s_tProtocol, s_achRecvData, &s_hwRecvLen);
        )
            
            /* Check result */
            if (s_tProtocol.tResult == PROTO_OK) {
                /* Parse file info */
                blm_protocol_ParseFileInfo(&s_tProtocol, s_achRecvData);
                
                if (s_tProtocol.wFileSize == 0) {
                    /* Empty file - end session */
                    blm_protocol_SendAck();
                    this.tMainState = BLM_STATE_IDLE;
                    break;
                } else if (s_tProtocol.wFileSize > this.wAppMaxSize) {
                    /* File too large */
                    blm_protocol_SendCancel();
                    this.tMainState = BLM_STATE_ERROR;
                    goto label_error;
                } else {
                    /* Start receiving */
                    blm_protocol_SendAck();
                    
                PERFC_PT_DELAY_MS(10);
                    
                    blm_protocol_SendC();  /* Request first data packet */
                    
                    /* Erase app region */
                    blm_port_FlashUnlock();
                    blm_port_FlashErase(this.wAppAddr, s_tProtocol.wFileSize);
                    
                    this.wReceivedSize = 0;
                    this.wFileSize = s_tProtocol.wFileSize;
                    this.tMainState = BLM_STATE_RECEIVING;
                    s_tProtocol.chExpectedSeq = 1;
                    break;  /* Exit WAIT_CONNECT loop */
                }
            } else if (s_tProtocol.tResult == PROTO_TIMEOUT) {
                this.chRetryCount++;
                if (this.chRetryCount >= BLM_MAX_RETRY) {
                    if (blm_IsAppValid(ptThis)) {
                        this.tMainState = BLM_STATE_JUMP_APP;
                        goto label_jump_app;
                    } else {
                        this.tMainState = BLM_STATE_ERROR;
                        goto label_error;
                    }
                }
            PERFC_PT_DELAY_MS(500);
            } else if (s_tProtocol.tResult == PROTO_CANCEL) {
                this.tMainState = BLM_STATE_IDLE;
                break;
            } else {
                blm_protocol_SendNak();
                this.chRetryCount++;
            PERFC_PT_DELAY_MS(100);
            }
        } while (this.tMainState == BLM_STATE_WAIT_CONNECT);
        
        /* Check if we should exit */
        if (this.tMainState != BLM_STATE_RECEIVING) {
            continue;
        }

        /* ============ STATE: RECEIVING ============ */
        this.chRetryCount = 0;
        do {
            /* Receive packet with PT */
        PERFC_PT_WAIT_UNTIL(
            (tResult == fsm_rt_cpl),
            tResult = blm_protocol_ReceivePacket(&s_tProtocol, s_achRecvData, &s_hwRecvLen);
        )
            
            switch (s_tProtocol.tResult) {
                case PROTO_OK:
                    /* Write data to flash */
                    {
                        uint32_t wWriteLen = s_hwRecvLen;
                        if (this.wReceivedSize + wWriteLen > this.wFileSize) {
                            wWriteLen = this.wFileSize - this.wReceivedSize;
                        }
                        if (wWriteLen > 0) {
                            blm_port_FlashWrite(this.wAppAddr + this.wReceivedSize, 
                                                s_achRecvData, wWriteLen);
                            this.wReceivedSize += wWriteLen;
                        }
                    }
                    blm_protocol_SendAck();
                    this.chRetryCount = 0;
                    break;
                    
                case PROTO_EOT:
                    /* First EOT - NAK it */
                    blm_protocol_SendNak();
                    
                    /* Wait for second EOT */
                PERFC_PT_WAIT_UNTIL(
                    (tResult == fsm_rt_cpl),
                    tResult = blm_protocol_ReceivePacket(&s_tProtocol, s_achRecvData, &s_hwRecvLen);
                )
                    
                    if (s_tProtocol.tResult == PROTO_EOT) {
                        blm_protocol_SendAck();
                        blm_port_FlashLock();
                        
                    PERFC_PT_DELAY_MS(10);
                        
                        /* Wait for empty packet 0 */
                        blm_protocol_SendC();
                        
                    PERFC_PT_WAIT_UNTIL(
                        (tResult == fsm_rt_cpl),
                        tResult = blm_protocol_ReceivePacket(&s_tProtocol, s_achRecvData, &s_hwRecvLen);
                    )
                        
                        if (s_tProtocol.tResult == PROTO_OK) {
                            blm_protocol_SendAck();
                        }
                        
                        this.tMainState = BLM_STATE_VERIFY;
                    }
                    break;
                    
                case PROTO_TIMEOUT:
                case PROTO_CRC_ERROR:
                case PROTO_SEQ_ERROR:
                    blm_protocol_SendNak();
                    this.chRetryCount++;
                    if (this.chRetryCount >= BLM_MAX_RETRY) {
                        blm_protocol_SendCancel();
                        blm_port_FlashLock();
                        this.tMainState = BLM_STATE_ERROR;
                    }
                PERFC_PT_DELAY_MS(50);
                    break;
                    
                case PROTO_CANCEL:
                    blm_port_FlashLock();
                    this.tMainState = BLM_STATE_IDLE;
                    break;
                    
                default:
                    blm_protocol_SendNak();
                    break;
            }
        } while (this.tMainState == BLM_STATE_RECEIVING);

        /* ============ STATE: VERIFY ============ */
        if (this.tMainState == BLM_STATE_VERIFY) {
            if (blm_IsAppValid(ptThis)) {
                gblinfo_UpdateAppInfo(0, 1, 0, this.wReceivedSize, 0);
                gblinfo_IncrementBootCount();
                this.tMainState = BLM_STATE_COMPLETE;
            } else {
                this.tMainState = BLM_STATE_ERROR;
                goto label_error;
            }
        }

        /* ============ STATE: COMPLETE ============ */
        if (this.tMainState == BLM_STATE_COMPLETE) {
        PERFC_PT_DELAY_MS(100);
            this.tMainState = BLM_STATE_JUMP_APP;
        }

label_jump_app:
        /* ============ STATE: JUMP_APP ============ */
        if (this.tMainState == BLM_STATE_JUMP_APP) {
            blm_JumpToApp(ptThis);
            /* Should not return */
        }

label_error:
        /* ============ STATE: ERROR ============ */
        /* Stay in error, wait for reset */
    PERFC_PT_DELAY_MS(1000);
        
    } while (1);

PERFC_PT_END()

    return fsm_rt_cpl;
}

/**
 * @brief Jump to application
 */
int blm_JumpToApp(blm_cb_t *ptThis)
{
    if (!blm_IsAppValid(ptThis)) {
        return -1;
    }
    
    /* Disable interrupts */
    __disable_irq();
    
    /* Update boot status */
    gblinfo_UpdateBootStatus(1);
    
    /* Jump to app */
    blm_port_JumpToApp(this.wAppAddr);
    
    /* Should never reach here */
    return 0;
}

/**
 * @brief Get current state
 */
blm_state_t blm_GetState(blm_cb_t *ptThis)
{
    if (NULL == ptThis) {
        return BLM_STATE_ERROR;
    }
    return this.tMainState;
}

/**
 * @brief Start upgrade manually
 */
int blm_StartUpgrade(blm_cb_t *ptThis)
{
    if (NULL == ptThis) {
        return -1;
    }
    
    this.tMainState = BLM_STATE_WAIT_CONNECT;
    this.wStartTime = blm_port_GetTickMs();
    this.chRetryCount = 0;
    this.chState = 0;  /* Reset PT state */
    
    blm_protocol_Init(&s_tProtocol, this.pchRxBuffer, this.hwRxBufferSize);
    
    return 0;
}

/**
 * @brief GMSI Run function - calls PT task
 */
int blm_Run(uintptr_t wObjectAddr)
{
    blm_cb_t *ptThis = (blm_cb_t *)wObjectAddr;
    
    if (NULL == ptThis) {
        return GMSI_EFAIL;
    }
    
    blm_task(ptThis);
    
    return GMSI_SUCCESS;
}

/**
 * @brief GMSI Clock function
 */
int blm_Clock(uintptr_t wObjectAddr)
{
    (void)wObjectAddr;
    return GMSI_SUCCESS;
}

/**
 * @brief Initialize bootloader (GMSI interface)
 */
int blm_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    int wRet = GMSI_SUCCESS;
    
    blm_cb_t *ptThis = (blm_cb_t *)wObjectAddr;
    blm_cfg_t *ptCfg = (blm_cfg_t *)wObjectCfgAddr;
    
    if (NULL == ptThis || NULL == ptCfg) {
        return GMSI_EFAIL;
    }
    
    /* Copy configuration */
    this.wAppAddr = ptCfg->wAppAddr;
    this.wAppMaxSize = ptCfg->wAppMaxSize;
    this.wSharedInfoAddr = ptCfg->wSharedInfoAddr;
    this.wTimeoutMs = ptCfg->wTimeoutMs;
    this.pchRxBuffer = ptCfg->pchRxBuffer;
    this.hwRxBufferSize = ptCfg->hwRxBufferSize;
    
    /* Initialize task */
    blm_task_init(ptThis);
    
    /* Initialize UART */
    blm_port_UartInit(BLM_UART_BAUDRATE);
    
    /* Update bootloader info in shared region */
    gblinfo_UpdateBlInfo(BLM_VERSION_MAJOR, BLM_VERSION_MINOR, 0);
    
    /* Initialize perf_counter */
    perfc_init(true);
    
    /* Register with GMSI */
    this.ptBase = &s_tBlmBase;
    s_tBlmBaseCfg.wParent = wObjectAddr;
    wRet = gbase_Init(this.ptBase, &s_tBlmBaseCfg);
    
    return wRet;
}
