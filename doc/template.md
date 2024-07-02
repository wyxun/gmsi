### 创建对象

#### Base对象

+ 基类初始化

```c
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
```

+ 基类所需指针函数

```c
int template_Clock(uintptr_t wObjectAddr)
{
    // Get the template object from the given address
    template_t *ptThis = (template_t *)wObjectAddr;

    int wRet = GMSI_SUCCESS;
    
    // Perform operations on ptThis

    return wRet;
}

int template_Run(uintptr_t wObjectAddr)
{
    int wRet = GMSI_SUCCESS;
    uint32_t wEvent;
    GMSI_MSG_DECLARE(TemplateBufferGet, 20);
    // Get the template object from the given address
    template_t *ptThis = (template_t *)wObjectAddr;

    // Check if ptThis is not NULL
    if (ptThis == NULL) {
        GLOG_PRINTF("ptThis is NULL.");
        return GMSI_EFAIL;
    }

    // Get the events for the template object
    wEvent = gbase_EventPend(ptThis->ptBase);

    // If there are any events, handle them
    if(wEvent)
        template_EventHandle(ptThis, wEvent);
    
    if(gbase_MessagePend(ptThis->ptBase, GMSI_MSG_GET_HANDLE(TemplateBufferGet)) > 0)
    {
        // Handle the message
        GLOG_PRINTF("get example message");
    }
    // Logic or state machine programs

    return wRet;
}
```

#### template对象

+ 在头文件创建实际对象，其必须继承基类**base_t**

```c
// The configuration structure for the template object.
typedef struct{

}template_cfg_t;

// The structure for the template object.
typedef struct{
    gmsi_base_t *ptBase;

}template_t;
```

### 挂载GMSI

```c
GMSI_MSG_ITEM_DECLARE(TEMPLATE, TemplateBuffer, 20);

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

    /* Copy the configuration members to the object */
    GMSI_MSG_ITEM_INITIALISE_LIST(TemplateBuffer);

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
```

### 编写功能

```c
int template_Run(uintptr_t wObjectAddr)
{
    // If there are any events, handle them
    if(wEvent)
        template_EventHandle(ptThis, wEvent);
}

// 主函数Run事件响应处理
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
        if(GMSI_SUCCESS != gcoroutine_Insert(&tGcoroutineTemplateHandle, 	\
                                             (void *)ptThis, 				\
                                             template_gcoroutine))
        {
            GLOG_PRINTF("Error: gcoroutine_Insert failed.");
        }
    }

    // Check if the event Event_PacketReceived is set
    if(wEvent & Event_PacketReceived)
    {
        // Handle the event Event_PacketReceived
        GMSI_MSG_UPDATE(TemplateTestBuffer, chTemplateBufferTest, 20);
        GMSI_MSG_ITEM_UPDATE(TemplateBuffer, GMSI_MSG_GET_BUFFER(TemplateTestBuffer),   \
                                                GMSI_MSG_GET_LENGTH(TemplateTestBuffer));
        gbase_MessagePost(EXAMPLE, GMSI_MSG_ITEM_GET_HANDLE(TemplateBuffer));
    }
}

// 等待消息响应处理
if(gbase_MessagePend(ptThis->ptBase, GMSI_MSG_GET_HANDLE(TemplateBufferGet)) > 0)
{
    // Handle the message
    GLOG_PRINTF("get example message");
}
```



### 协程

#### 创建

```c
// 在某个合适的触发位置启动协程功能
if(GMSI_SUCCESS != gcoroutine_Insert(&tGcoroutineTemplateHandle, 	\
                                     (void *)ptThis, 				\
                                     template_gcoroutine))
{
    GLOG_PRINTF("Error: gcoroutine_Insert failed.");
}
```

#### 功能处理

```c
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
```





