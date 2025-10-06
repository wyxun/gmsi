#include "example.h"
#include "userconfig.h"

int example_Clock(uintptr_t wObjectAddr);
int example_Run(uintptr_t wObjectAddr);

// Define a global example base of type gmsi_base_t
static gmsi_base_t s_tExampleBase;

example_share_mem_t tExampleShareMem;
share_mem_t s_tExampleShareMem = {
    .pchBuffer = (uint8_t *)&tExampleShareMem,  // Initialize the pointer to the shared memory buffer
    .hwSize = sizeof(tExampleShareMem)          // Initialize the size of the shared memory
};

// Define and initialize a global example base configuration of type gmsi_base_cfg_t
gmsi_base_cfg_t s_tExampleBaseCfg = {
    .wId = EXAMPLE,                 // Set the ID to EXAMPLE
    .wParent = 0,                   // Set the parent to 0
    .FcnInterface = {
        .Clock = example_Clock,     // Set the Clock function to example_Clock
        .Run = example_Run,         // Set the Run function to example_Run
    },
    .ptShareMem = &s_tExampleShareMem // Set the shared memory configuration
};

gcoroutine_handle_t tGcoroutineExampleHandle = {
    .bIsRunning = false,
    .pfcn = NULL,
};

/**
 * Function: example_gcoroutine
 * ----------------------------
 * This function is a coroutine that handles different states of the example object. It receives 
 * a parameter, converts it to an example object, and uses a switch statement to handle different 
 * states. This function returns fsm_rt_on_going if it is still running, or fsm_rt_cpl if it has 
 * completed its task. Once the coroutine has completed its task, it will be removed from the 
 * coroutine list.
 *
 * Parameters: 
 * pvParam: The parameter to be converted to an example object.
 *
 * Returns: 
 * fsm_rt_on_going if the function is still running, or fsm_rt_cpl if the function has completed 
 * its task.
 */
fsm_rt_t example_gcoroutine(void *pvParam)
{
    static uint8_t s_eState = 0;
    example_t *ptObject = (example_t *)pvParam;
    gcoroutine_handle_t *ptThis = (gcoroutine_handle_t *)&tGcoroutineExampleHandle;

PERFC_PT_BEGIN(this.chState)
    // Example coroutine logic
    do {
    PERFC_PT_WAIT_FOR_RES_UNTIL( 
        (ptObject != NULL),               /* quit condition */
        ptObject = (example_t *)pvParam; /* try to allocate memory */
    )
        printf("Template Coroutine Running\r\n");
    PERFC_PT_DELAY_MS(1000);

    PERFC_PT_DELAY_MS(1000);

    } while(0);
PERFC_PT_END()

    return fsm_rt_cpl;

}

/**
 * Function: example_EventHandle
 * ----------------------------
 * This function handles the events for the example object. It checks for specific events and 
 * performs the corresponding actions. This function does not return a value, as its result is 
 * reflected in the state of the example object it operates on.
 *
 * Parameters: 
 * ptThis: The pointer to the example object.
 * wEvent: The events to be handled.
 *
 * Returns: 
 * None
 */
static void example_EventHandle(example_t *ptThis, uint32_t wEvent)
{
    // Check if ptThis is not NULL
    if (ptThis == NULL) {
        GLOG_PRINTF("ptThis is NULL.");
        return;
    }

    // Check if the event Event_SyncMissed is set
    if(wEvent & Event_SyncMissed)
    {
        GLOG_PRINTF("get event Event_SyncMissed");
    }

    // Check if the event Event_SyncButtonPushed is set
    if(wEvent & Event_SyncButtonPushed)
    {
        if(GMSI_SUCCESS != gcoroutine_Insert(&tGcoroutineExampleHandle, (void *)ptThis, example_gcoroutine))
        {
            GLOG_PRINTF("Error: gcoroutine_Insert failed.");
        }
    }
}

/**
 * Function: example_Run
 * ----------------------------
 * This function is mounted in the GMSI run function list and is called within a while(1) loop. 
 * It receives events and messages from other objects. When a corresponding event is triggered, 
 * it executes the appropriate coroutine. The function gets the example object from the given 
 * address, retrieves the events for the object, and handles these events. This function returns 
 * GMSI_SUCCESS if it runs successfully.
 *
 * Parameters: 
 * wObjectAddr: The address of the example object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully.
 */
int example_Run(uintptr_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;
    uint32_t wEvent;
    // Get the example object from the given address
    example_t *ptThis = (example_t *)wObjectAddr;
    uint8_t chRingBufferMsg[16];
    // Check if ptThis is not NULL
    if (ptThis == NULL) {
        GLOG_PRINTF("ptThis is NULL.");
        return GMSI_EFAIL;
    }

    // Get the events for the example object
    wEvent = gbase_EventPend(ptThis->ptBase);
    // If there are any events, handle them
    if(wEvent)
        example_EventHandle(ptThis, wEvent);
    
    // If the ring buffer is enabled, check for messages in the ring buffer
    uint16_t hwLength = gbase_MessagePendFromRing(ptThis->ptBase, chRingBufferMsg, sizeof(chRingBufferMsg));
    if(hwLength > 0)
    {

        // Process the messages received from the ring buffer
        GLOG_PRINTF("get chRingBufferMsg");
        GVAL_PRINTF(hwLength);
        tExampleShareMem.value2 = hwLength; // Reset the shared memory value
    }

    // Logic or state machine programs

    return wRet;
}

/**
 * Function: example_Clock
 * ----------------------------
 * This function is called in a 1ms interrupt. It gets the example object from the given address 
 * and performs operations on it. This function returns GMSI_SUCCESS if it runs successfully.
 *
 * Parameters: 
 * wObjectAddr: The address of the example object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully.
 */
int example_Clock(uintptr_t wObjectAddr)
{
    // Get the example object from the given address
    example_t *ptThis = (example_t *)wObjectAddr;
    uint16_t hwExampleTestCount = 2000;        // 2000ms send a message
    uint16_t hwExampleTestCount2 = 5000;       // 5000ms send a Event_PacketReceived event
    int wRet = GMSI_SUCCESS;

    if(!hwExampleTestCount2)
    {
        gbase_EventPost(EXAMPLE, Event_PacketReceived);
        hwExampleTestCount2 = 5000;
    }
    else
    {
        hwExampleTestCount2--;
    }

    return wRet;
}

/**
 * Function: example_Init
 * ----------------------------
 * This function initializes an example object. It converts the given addresses to pointers, 
 * checks if the pointers are not NULL, copies the configuration members to the object, initializes 
 * the hardware, and registers the object in the GMSI list.
 *
 * Parameters: 
 * wObjectAddr: The address of the example object.
 * wObjectCfgAddr: The address of the example configuration object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully, GMSI_EAGAIN if the object cannot be registered 
 * in the GMSI list, or GMSI_FAILURE if any of the pointers is NULL.
 */
int example_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    // Convert the given addresses to pointers
    example_t *ptThis = (example_t *)wObjectAddr;
    example_cfg_t *ptCfg = (example_cfg_t *)wObjectCfgAddr;

    // Check if the pointers are not NULL
    if (ptThis == NULL || ptCfg == NULL) {
        GLOG_PRINTF("Error: ptThis or ptCfg is NULL.");
        return GMSI_EFAIL;
    }

    /* Initialize the hardware */

    // Register the object in the GMSI list
    ptThis->ptBase = &s_tExampleBase;
    if (ptThis->ptBase == NULL) {
        return GMSI_EAGAIN;
    } else {
        s_tExampleBaseCfg.wParent = wObjectAddr;
        if(ptCfg->pchRingBuffer != NULL && ptCfg->hwRingSize != 0) {
            // Initialize the ring buffer in the example object
            with(ptThis->ptBase){
                _->tRingBuffer.buffer = ptCfg->pchRingBuffer;
                _->tRingBuffer.hwBufferSize = ptCfg->hwRingSize;
                _->tRingBuffer.hwWriteIndex = 0;
                _->tRingBuffer.hwReadIndex = 0;
            };
        }
        return gbase_Init(ptThis->ptBase, &s_tExampleBaseCfg);
    }   
}