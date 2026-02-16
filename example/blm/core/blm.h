/**
 * @file blm.h
 * @brief BLM Bootloader Core Module
 * 
 * Uses PERFC_PT state machine following perf_counter example conventions
 */
#ifndef __BLM_H__
#define __BLM_H__

/*============================ INCLUDES ======================================*/
#include "gmsi.h"
#include "../userconfig.h"
#include <perf_counter.h>

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/

/**
 * @brief Bootloader state machine states
 */
typedef enum {
    BLM_STATE_IDLE,             // Waiting for command
    BLM_STATE_WAIT_CONNECT,     // Waiting for host connection
    BLM_STATE_RECEIVING,        // Receiving firmware data
    BLM_STATE_VERIFY,           // Verifying firmware
    BLM_STATE_COMPLETE,         // Upgrade complete
    BLM_STATE_ERROR,            // Error state
    BLM_STATE_JUMP_APP          // Ready to jump to app
} blm_state_t;

/**
 * @brief Bootloader configuration structure
 */
typedef struct {
    uint32_t wAppAddr;          // Application start address
    uint32_t wAppMaxSize;       // Maximum application size
    uint32_t wSharedInfoAddr;   // Shared info region address
    uint32_t wTimeoutMs;        // Connection timeout
    uint8_t *pchRxBuffer;       // Receive buffer
    uint16_t hwRxBufferSize;    // Receive buffer size
} blm_cfg_t;

/**
 * @brief Bootloader Control Block (following pt_example convention)
 */
typedef struct {
    /* PT state machine (must be first for convention) */
    uint8_t chState;
    
    /* GMSI base */
    gmsi_base_t *ptBase;
    
    /* Configuration */
    uint32_t wAppAddr;
    uint32_t wAppMaxSize;
    uint32_t wSharedInfoAddr;
    uint32_t wTimeoutMs;
    uint8_t *pchRxBuffer;
    uint16_t hwRxBufferSize;
    
    /* Runtime state */
    blm_state_t tMainState;
    uint32_t wReceivedSize;
    uint32_t wFileSize;
    uint32_t wStartTime;
    uint8_t chPacketNum;
    uint16_t chRetryCount;
} blm_cb_t;

/* Alias for GMSI_DECLARE_OBJECT macro compatibility */
typedef blm_cb_t blm_t;

/*============================ PROTOTYPES ====================================*/

/* GMSI Interface */
extern int blm_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);
extern int blm_Run(uintptr_t wObjectAddr);
extern int blm_Clock(uintptr_t wObjectAddr);

/* PT State Machine API */
extern blm_cb_t *blm_task_init(blm_cb_t *ptThis);
extern fsm_rt_t blm_task(blm_cb_t *ptThis);

/* Utility API */
extern int blm_JumpToApp(blm_cb_t *ptThis);
extern blm_state_t blm_GetState(blm_cb_t *ptThis);
extern int blm_StartUpgrade(blm_cb_t *ptThis);

#endif  /* __BLM_H__ */
