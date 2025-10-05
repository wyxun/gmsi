#include "lsm6dsl.h"
#include "userconfig.h"
#include "bsp_lsm6dsl.h"

int lsm6dsl_Clock(uintptr_t wObjectAddr);
int lsm6dsl_Run(uintptr_t wObjectAddr);

// Define a global lsm6dsl base of type gmsi_base_t
static gmsi_base_t s_tLsm6dslBase;

// Define and initialize a global lsm6dsl base configuration of type gmsi_base_cfg_t
gmsi_base_cfg_t s_tLsm6dslBaseCfg = {
    .wId = LSM6DSL,                 // Set the ID to LSM6DSL
    .wParent = 0,                   // Set the parent to 0
    .FcnInterface = {
        .Clock = lsm6dsl_Clock,     // Set the Clock function to lsm6dsl_Clock
        .Run = lsm6dsl_Run,         // Set the Run function to lsm6dsl_Run
    },
};

gcoroutine_handle_t tGcoroutineLsm6dslHandle = {
    .bIsRunning = false,
    .pfcn = NULL,
};

/**
 * Function: lsm6dsl_gcoroutine
 * ----------------------------
 * This function is a coroutine that handles different states of the lsm6dsl object. It receives 
 * a parameter, converts it to an lsm6dsl object, and uses a switch statement to handle different 
 * states. This function returns fsm_rt_on_going if it is still running, or fsm_rt_cpl if it has 
 * completed its task. Once the coroutine has completed its task, it will be removed from the 
 * coroutine list.
 *
 * Parameters: 
 * pvParam: The parameter to be converted to an lsm6dsl object.
 *
 * Returns: 
 * fsm_rt_on_going if the function is still running, or fsm_rt_cpl if the function has completed 
 * its task.
 */
fsm_rt_t lsm6dsl_gcoroutine(void *pvParam)
{
    static uint8_t s_eState = 0;
    fsm_rt_t tFsm = fsm_rt_on_going;
    lsm6dsl_t *ptThis = (lsm6dsl_t *)pvParam;

    // Check if ptThis is not NULL
    if (ptThis == NULL) {
        GLOG_PRINTF("Error: ptThis is NULL.\n");
        return fsm_rt_err;
    }

    switch(s_eState)
    {
        case 0:
            GLOG_PRINTF("get lsm6dsl event");
            s_eState++;
            break;
        case 1:
            GLOG_PRINTF("finish get lsm6dsl event handle");
            fsm_cpl();
            break;
        default:
            fsm_cpl();
        break;
    }
    fsm_on_going(); 
}

/**
 * Function: lsm6dsl_EventHandle
 * ----------------------------
 * This function handles the events for the lsm6dsl object. It checks for specific events and 
 * performs the corresponding actions. This function does not return a value, as its result is 
 * reflected in the state of the lsm6dsl object it operates on.
 *
 * Parameters: 
 * ptThis: The pointer to the lsm6dsl object.
 * wEvent: The events to be handled.
 *
 * Returns: 
 * None
 */
static void lsm6dsl_EventHandle(lsm6dsl_t *ptThis, uint32_t wEvent)
{
    // Check if ptThis is not NULL
    if (ptThis == NULL) {
        GLOG_PRINTF("ptThis is NULL.");
    
        return;
    }

}

/**
 * Function: lsm6dsl_Run
 * ----------------------------
 * This function is mounted in the GMSI run function list and is called within a while(1) loop. 
 * It receives events and messages from other objects. When a corresponding event is triggered, 
 * it executes the appropriate coroutine. The function gets the lsm6dsl object from the given 
 * address, retrieves the events for the object, and handles these events. This function returns 
 * GMSI_SUCCESS if it runs successfully.
 *
 * Parameters: 
 * wObjectAddr: The address of the lsm6dsl object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully.
 */
int lsm6dsl_Run(uintptr_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;
    uint32_t wEvent;
 
    // Get the lsm6dsl object from the given address
    lsm6dsl_t *ptThis = (lsm6dsl_t *)wObjectAddr;

    // Check if ptThis is not NULL
    if (ptThis == NULL) {
        GLOG_PRINTF("ptThis is NULL.");
        return GMSI_EFAIL;
    }

    // Get the events for the lsm6dsl object
    wEvent = gbase_EventPend(ptThis->ptBase);
    // If there are any events, handle them
    if(wEvent)
        lsm6dsl_EventHandle(ptThis, wEvent);

    // Check if the object is ready for update
    if (ptThis->chObjectStatus & LSM6DSL_UPDATE_READY) {
        LSM6DSL_ReadRawData((int16_t *)&ptThis->hwAccel, (int16_t *)&ptThis->hwGyro);
        ptThis->chObjectStatus &= ~LSM6DSL_UPDATE_READY;
        //GLOG_PRINTF("get lsm6dsl data");
    }

    // Logic or state machine programs

    return wRet;
}

/**
 * Function: lsm6dsl_Clock
 * ----------------------------
 * This function is called in a 1ms interrupt. It gets the lsm6dsl object from the given address 
 * and performs operations on it. This function returns GMSI_SUCCESS if it runs successfully.
 *
 * Parameters: 
 * wObjectAddr: The address of the lsm6dsl object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully.
 */
int lsm6dsl_Clock(uintptr_t wObjectAddr)
{
    
    // Get the lsm6dsl object from the given address
    lsm6dsl_t *ptThis = (lsm6dsl_t *)wObjectAddr;

    static uint16_t s_hwUpdateCounter = 0;
    
    if(s_hwUpdateCounter >= ptThis->hwUpdateRate)
    {
        ptThis->chObjectStatus |= LSM6DSL_UPDATE_READY;
        s_hwUpdateCounter = 0;
    }
    else
        s_hwUpdateCounter++;
    
    int wRet = GMSI_SUCCESS;
    
    // Perform operations on ptThis

    return wRet;
}

/**
 * Function: lsm6dsl_Init
 * ----------------------------
 * This function initializes an lsm6dsl object. It converts the given addresses to pointers, 
 * checks if the pointers are not NULL, copies the configuration members to the object, initializes 
 * the hardware, and registers the object in the GMSI list.
 *
 * Parameters: 
 * wObjectAddr: The address of the lsm6dsl object.
 * wObjectCfgAddr: The address of the lsm6dsl configuration object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully, GMSI_EAGAIN if the object cannot be registered 
 * in the GMSI list, or GMSI_FAILURE if any of the pointers is NULL.
 */
int lsm6dsl_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    // Convert the given addresses to pointers
    lsm6dsl_t *ptThis = (lsm6dsl_t *)wObjectAddr;
    lsm6dsl_cfg_t *ptCfg = (lsm6dsl_cfg_t *)wObjectCfgAddr;

    // Check if the pointers are not NULL
    if (ptThis == NULL || ptCfg == NULL) {
        GLOG_PRINTF("Error: ptThis or ptCfg is NULL.");
        return GMSI_EFAIL;
    }

    /* Copy the configuration members to the object */
    ptThis->hwUpdateRate = 1000/ptCfg->hwUpdateRateHz;
    /* Initialize the hardware */
    if(LSM6DSL_Init() != 1) {
        GLOG_PRINTF("Error: LSM6DSL_Init failed.");
        return GMSI_EFAIL;
    }

    // Register the object in the GMSI list
    ptThis->ptBase = &s_tLsm6dslBase;
    if (ptThis->ptBase == NULL) {
        return GMSI_EAGAIN;
    } else {
        s_tLsm6dslBaseCfg.wParent = wObjectAddr;
        return gbase_Init(ptThis->ptBase, &s_tLsm6dslBaseCfg);
    }
}