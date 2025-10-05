#include "step4988.h"
#include "userconfig.h"
#include "bsp_stepmotor.h"

int step4988_Clock(uintptr_t wObjectAddr);
int step4988_Run(uintptr_t wObjectAddr);

// Define a global step4988 base of type gmsi_base_t
static gmsi_base_t s_tStep4988Base;

// Define and initialize a global step4988 base configuration of type gmsi_base_cfg_t
gmsi_base_cfg_t s_tStep4988BaseCfg = {
    .wId = STEP4988,                 // Set the ID to STEP4988
    .wParent = 0,                   // Set the parent to 0
    .FcnInterface = {
        .Clock = step4988_Clock,     // Set the Clock function to step4988_Clock
        .Run = step4988_Run,         // Set the Run function to step4988_Run
    },
};

gcoroutine_handle_t tGcoroutineStep4988Handle = {
    .bIsRunning = false,
    .pfcn = NULL,
};

/**
 * Function: step4988_gcoroutine
 * ----------------------------
 * This function is a coroutine that handles different states of the step4988 object. It receives 
 * a parameter, converts it to an step4988 object, and uses a switch statement to handle different 
 * states. This function returns fsm_rt_on_going if it is still running, or fsm_rt_cpl if it has 
 * completed its task. Once the coroutine has completed its task, it will be removed from the 
 * coroutine list.
 *
 * Parameters: 
 * pvParam: The parameter to be converted to an step4988 object.
 *
 * Returns: 
 * fsm_rt_on_going if the function is still running, or fsm_rt_cpl if the function has completed 
 * its task.
 */
fsm_rt_t step4988_gcoroutine(void *pvParam)
{
    static uint8_t s_eState = 0;
    fsm_rt_t tFsm = fsm_rt_on_going;
    step4988_t *ptThis = (step4988_t *)pvParam;

    // Check if ptThis is not NULL
    if (ptThis == NULL) {
        GLOG_PRINTF("Error: ptThis is NULL.\n");
        return fsm_rt_err;
    }

    switch(s_eState)
    {
        case 0:
            GLOG_PRINTF("get step4988 event");
            s_eState++;
            break;
        case 1:
            GLOG_PRINTF("finish get step4988 event handle");
            fsm_cpl();
            break;
        default:
            fsm_cpl();
        break;
    }
    fsm_on_going(); 
}

/**
 * Function: step4988_EventHandle
 * ----------------------------
 * This function handles the events for the step4988 object. It checks for specific events and 
 * performs the corresponding actions. This function does not return a value, as its result is 
 * reflected in the state of the step4988 object it operates on.
 *
 * Parameters: 
 * ptThis: The pointer to the step4988 object.
 * wEvent: The events to be handled.
 *
 * Returns: 
 * None
 */
static void step4988_EventHandle(step4988_t *ptThis, uint32_t wEvent)
{
    // Check if ptThis is not NULL
    if (ptThis == NULL) {
        GLOG_PRINTF("ptThis is NULL.");
        return;
    }
    
    if(wEvent & twoInOneHandle_Event_StartStepMotor)
    {
        bsp_StepMotorEnable(true);
    }
    
    if(wEvent & twoInOneHandle_Event_StopStepMotor)
    {
        bsp_StepMotorEnable(false);
    }
    if(wEvent & twoInOneHandle_Event_MotorDirCCW)
        bsp_StepMotorSetDirection(STEP_MOTOR_DIRECTION_CCW);
    if(wEvent & twoInOneHandle_Event_MotorDirCW)
        bsp_StepMotorSetDirection(STEP_MOTOR_DIRECTION_CW);
    
#if 0
    // Check if the event Event_SyncMissed is set
    if(wEvent & Event_SyncMissed)
    {
        GLOG_PRINTF("get event Event_SyncMissed");
    }

    // Check if the event Event_SyncButtonPushed is set
    if(wEvent & Event_SyncButtonPushed)
    {
        if(GMSI_SUCCESS != gcoroutine_Insert(&tGcoroutineStep4988Handle, (void *)ptThis, step4988_gcoroutine))
        {
            GLOG_PRINTF("Error: gcoroutine_Insert failed.");
        }
    }

    // Check if the event Event_PacketReceived is set
    if(wEvent & Event_PacketReceived)
    {
        // Handle the event Event_PacketReceived
    }
#endif
}

/**
 * Function: step4988_Run
 * ----------------------------
 * This function is mounted in the GMSI run function list and is called within a while(1) loop. 
 * It receives events and messages from other objects. When a corresponding event is triggered, 
 * it executes the appropriate coroutine. The function gets the step4988 object from the given 
 * address, retrieves the events for the object, and handles these events. This function returns 
 * GMSI_SUCCESS if it runs successfully.
 *
 * Parameters: 
 * wObjectAddr: The address of the step4988 object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully.
 */
int step4988_Run(uintptr_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;
    uint32_t wEvent;
 
    // Get the step4988 object from the given address
    step4988_t *ptThis = (step4988_t *)wObjectAddr;

    // Check if ptThis is not NULL
    if (ptThis == NULL) {
        GLOG_PRINTF("ptThis is NULL.");
        return GMSI_EFAIL;
    }

    // Get the events for the step4988 object
    wEvent = gbase_EventPend(ptThis->ptBase);

    // If there are any events, handle them
    if(wEvent)
        step4988_EventHandle(ptThis, wEvent);
    
    //gbase_MessagePostToRing(RFID, "HHHHH", 5);
    // Logic or state machine programs

    return wRet;
}

/**
 * Function: step4988_Clock
 * ----------------------------
 * This function is called in a 1ms interrupt. It gets the step4988 object from the given address 
 * and performs operations on it. This function returns GMSI_SUCCESS if it runs successfully.
 *
 * Parameters: 
 * wObjectAddr: The address of the step4988 object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully.
 */
int step4988_Clock(uintptr_t wObjectAddr)
{
    volatile static bool bStatus = true;
    // Get the step4988 object from the given address
    step4988_t *ptThis = (step4988_t *)wObjectAddr;

    int wRet = GMSI_SUCCESS;

    // Perform operations on ptThis

    return wRet;
}

/**
 * Function: step4988_Init
 * ----------------------------
 * This function initializes an step4988 object. It converts the given addresses to pointers, 
 * checks if the pointers are not NULL, copies the configuration members to the object, initializes 
 * the hardware, and registers the object in the GMSI list.
 *
 * Parameters: 
 * wObjectAddr: The address of the step4988 object.
 * wObjectCfgAddr: The address of the step4988 configuration object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully, GMSI_EAGAIN if the object cannot be registered 
 * in the GMSI list, or GMSI_FAILURE if any of the pointers is NULL.
 */
int step4988_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    // Convert the given addresses to pointers
    step4988_t *ptThis = (step4988_t *)wObjectAddr;
    step4988_cfg_t *ptCfg = (step4988_cfg_t *)wObjectCfgAddr;

    // Check if the pointers are not NULL
    if (ptThis == NULL || ptCfg == NULL) {
        GLOG_PRINTF("Error: ptThis or ptCfg is NULL.");
        return GMSI_EFAIL;
    }

    /* Copy the configuration members to the object */

    /* Initialize the hardware */
    bsp_StepMotorInit();
    
    // Register the object in the GMSI list
    ptThis->ptBase = &s_tStep4988Base;
    if (ptThis->ptBase == NULL) {
        return GMSI_EAGAIN;
    } else {
        s_tStep4988BaseCfg.wParent = wObjectAddr;
        return gbase_Init(ptThis->ptBase, &s_tStep4988BaseCfg);
    }
}