#include "template.h"
#include "userconfig.h"

#ifdef TEMPLATE_ITEM_MESSAGE
GMSI_MSG_ITEM_DECLARE(TEMPLATE, g_tTemplatePost, sizeof(template_msg_t));

#endif

int template_Clock(uintptr_t wObjectAddr);
int template_Run(uintptr_t wObjectAddr);

// Define a global template base of type gmsi_base_t
static gmsi_base_t s_tTemplateBase;

// Define and initialize a global template base configuration of type gmsi_base_cfg_t
gmsi_base_cfg_t s_tTemplateBaseCfg = {
    .wId = TEMPLATE,                        // Set the ID to TEMPLATE
    .wParent = 0,                           // Set the parent to 0
    .FcnInterface = {
        .Clock = template_Clock,            // Set the Clock function to template_Clock
        .Run = template_Run,                // Set the Run function to template_Run
    },
};

gcoroutine_handle_t tGcoroutineTemplateHandle = {
    .bIsRunning = false,
    .pfcn = NULL,
};

uint8_t chTemplateBufferTest[20] = {11, 22, 33, 44, 55, 66, 77, 88, 99, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14};
/**
 * Function: template_gcoroutine
 * ----------------------------
 * This function is a coroutine that handles different states of the template object. It receives 
 * a parameter, converts it to an template object, and uses a switch statement to handle different 
 * states. This function returns fsm_rt_on_going if it is still running, or fsm_rt_cpl if it has 
 * completed its task. Once the coroutine has completed its task, it will be removed from the 
 * coroutine list.
 *
 * Parameters: 
 * pvParam: The parameter to be converted to an template object.
 *
 * Returns: 
 * fsm_rt_on_going if the function is still running, or fsm_rt_cpl if the function has completed 
 * its task.
 */
fsm_rt_t template_gcoroutine(void *pvParam)
{
    static uint8_t s_eState = 0;
    fsm_rt_t tFsm = fsm_rt_on_going;
    template_t *ptThis = (template_t *)pvParam;

    // Check if ptThis is not NULL
    if (ptThis == NULL) {
        GLOG_PRINTF("Error: ptThis is NULL.\n");
        return fsm_rt_err;
    }

    switch(s_eState)
    {
        case 0:
            GLOG_PRINTF("get template event");
            s_eState++;
            break;
        case 1:
            GLOG_PRINTF("finish get template event handle");
            fsm_cpl();
            break;
        default:
            fsm_cpl();
        break;
    }
    fsm_on_going(); 
}

/**
 * Function: template_EventHandle
 * ----------------------------
 * This function handles the events for the template object. It checks for specific events and 
 * performs the corresponding actions. This function does not return a value, as its result is 
 * reflected in the state of the template object it operates on.
 *
 * Parameters: 
 * ptThis: The pointer to the template object.
 * wEvent: The events to be handled.
 *
 * Returns: 
 * None
 */
static void template_EventHandle(template_t *ptThis, uint32_t wEvent)
{
    GMSI_MSG_DECLARE(TemplateTestBuffer, 20);
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
        if(GMSI_SUCCESS != gcoroutine_Insert(&tGcoroutineTemplateHandle, (void *)ptThis, template_gcoroutine))
        {
            GLOG_PRINTF("Error: gcoroutine_Insert failed.");
        }
    }

    // Check if the event Event_PacketReceived is set
    if(wEvent & Event_PacketReceived)
    {
        // Handle the event Event_PacketReceived
    }
}

/**
 * Function: template_Run
 * ----------------------------
 * This function is mounted in the GMSI run function list and is called within a while(1) loop. 
 * It receives events and messages from other objects. When a corresponding event is triggered, 
 * it executes the appropriate coroutine. The function gets the template object from the given 
 * address, retrieves the events for the object, and handles these events. This function returns 
 * GMSI_SUCCESS if it runs successfully.
 *
 * Parameters: 
 * wObjectAddr: The address of the template object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully.
 */
int template_Run(uintptr_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;
    uint32_t wEvent;
    #ifdef TEMPLATE_ITEM_MESSAGE
    GMSI_MSG_DECLARE(tTemplatePend, PEND_BUFFER_SIZE);
    #endif
    // Get the template object from the given address
    template_t *ptThis = (template_t *)wObjectAddr;

    // Check if ptThis is not NULL
    if (ptThis == NULL) {
        GLOG_PRINTF("ptThis is NULL.");
        return GMSI_EFAIL;
    }

    template_msg_t *ptMsg = (template_msg_t *)GMSI_MSG_ITEM_GET_BUFFER(g_tTemplatePost);
    ptMsg->hwLength = ptThis->read(ptThis->wFd, ptMsg->chData);
    if(ptMsg->hwLength)
    {
        ptMsg->chStatus = 0;

        gbase_MessagePost(EXAMPLE, GMSI_MSG_ITEM_GET_HANDLE(g_tTemplatePost));
    }
    // Get the events for the template object
    wEvent = gbase_EventPend(ptThis->ptBase);
    // If there are any events, handle them
    if(wEvent)
        template_EventHandle(ptThis, wEvent);

    #ifdef TEMPLATE_ITEM_MESSAGE
    if(gbase_MessagePend(ptThis->ptBase, GMSI_MSG_GET_HANDLE(tTemplatePend)) > 0)
    {
        // Handle the message
        GLOG_PRINTF("get example message");
    }
    #endif
    // Logic or state machine programs

    return wRet;
}

/**
 * Function: template_Clock
 * ----------------------------
 * This function is called in a 1ms interrupt. It gets the template object from the given address 
 * and performs operations on it. This function returns GMSI_SUCCESS if it runs successfully.
 *
 * Parameters: 
 * wObjectAddr: The address of the template object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully.
 */
int template_Clock(uintptr_t wObjectAddr)
{
    // Get the template object from the given address
    template_t *ptThis = (template_t *)wObjectAddr;

    int wRet = GMSI_SUCCESS;
    
    // Perform operations on ptThis

    return wRet;
}

/**
 * Function: template_Init
 * ----------------------------
 * This function initializes an template object. It converts the given addresses to pointers, 
 * checks if the pointers are not NULL, copies the configuration members to the object, initializes 
 * the hardware, and registers the object in the GMSI list.
 *
 * Parameters: 
 * wObjectAddr: The address of the template object.
 * wObjectCfgAddr: The address of the template configuration object.
 *
 * Returns: 
 * GMSI_SUCCESS if the function runs successfully, GMSI_EAGAIN if the object cannot be registered 
 * in the GMSI list, or GMSI_FAILURE if any of the pointers is NULL.
 */
int template_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    // Convert the given addresses to pointers
    template_t *ptThis = (template_t *)wObjectAddr;
    template_cfg_t *ptCfg = (template_cfg_t *)wObjectCfgAddr;

    // Check if the pointers are not NULL
    if (ptThis == NULL || ptCfg == NULL) {
        GLOG_PRINTF("Error: ptThis or ptCfg is NULL.");
        return GMSI_EFAIL;
    }

#ifdef TEMPLATE_ITEM_MESSAGE
    /* Copy the configuration members to the object */
    GMSI_MSG_ITEM_INITIALISE_LIST(g_tTemplatePost);
#endif
    /* Initialize the hardware */

    // Register the object in the GMSI list
    ptThis->ptBase = &s_tTemplateBase;
    if (ptThis->ptBase == NULL) {
        return GMSI_EAGAIN;
    } else {
        s_tTemplateBaseCfg.wParent = wObjectAddr;
        return gbase_Init(ptThis->ptBase, &s_tTemplateBaseCfg);
    }
}