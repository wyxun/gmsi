### object

![](gbase.assets/base_t.png)

#### register

```c
gmsi_base_cfg_t s_tTemplateBaseCfg = {
    .wId = TEMPLATE,                        // Set the ID to TEMPLATE
    .wParent = 0,                           // Set the parent to 0
    .FcnInterface = {
        .Clock = template_Clock,            // Set the Clock function to template_Clock
        .Run = template_Run,                // Set the Run function to template_Run
    },
};

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

### message

![](gbase.assets/message.png)

#### create

```c
// 创建一个有限长度的消息项
#define GMSI_MSG_ITEM_DECLARE(OBJECT,NAME,SIZE)                             \
        uint8_t ch##NAME##_Buffer[SIZE] = {0};                              \
        message_item_t t##NAME##item = {                                    \
            .tListItem.xItemValue = OBJECT,                                 \
            .pchMessage = ch##NAME##_Buffer,                                \
            .hwLength = 0,                                                  \
            .hwMaxSize = SIZE                                               \
        };                                                                  \
```

#### update data

```c
#define GMSI_MSG_ITEM_UPDATE(NAME, MESSAGE, LENGTH)                         \
    do{                                                                     \
        if((t##NAME##item).hwMaxSize >= LENGTH)                             \
        {                                                                   \
            memcpy((t##NAME##item).pchMessage, MESSAGE, LENGTH);            \
            (t##NAME##item).hwLength = LENGTH;                              \
        }                                                                   \
        else                                                                \
        {                                                                   \
            (t##NAME##item).hwLength = 0;                                   \
        }                                                                   \
    }while(0)
```

#### rev_send data

```c
// 发送消息
int gbase_MessagePost(uint32_t wId, message_item_t *ptMsgItem)
{
    // 根据id遍历object链表
    
    // 找到对应id的object后，将ptMsgItem挂在目标object上；
    /* 注意：该ptMsgItem所有权在post方源object，目标object只有读权限*/
    
    // 同时置位公共事件Gmsi_Event_Transition；这个是保留设计，当前不对该事件处理
}
// 接收消息
if(gbase_MessagePend(ptThis->ptBase, GMSI_MSG_GET_HANDLE(TemplateBufferGet)) > 0)
{
    uint16_t hwLength = GMSI_MSG_ITEM_GET_LENGTH(TemplateBufferGet);
    uint8_t *pchData = GMSI_MSG_ITEM_GET_BUFFER(TemplateBufferGet);
    // 处理数据
}
```



### event

![img](gbase.assets/event-17200177187851.png)

#### post

```c
int gbase_EventPost(uint32_t wId, uint32_t wEvent)
{
    // 根据id遍历对象链表
    
    // 找到对象节点
    
    // 将事件值写入到改对象节点的wEvent变量
}
```

#### pend

```C
uint32_t gbase_EventPend(gmsi_base_t *ptBase)
{
    // 读取自身的wEvent是否有值
    
    // 获取事件后清空wEvent
}

// 对事件值做响应处理
wEvent = gbase_EventPend(ptThis->ptBase);
if(wEvent & Event_SyncButtonPushed)
{}
if(wEvent & Event_SyncMissed)
{}
```

### coroutine

![coroutine](gbase.assets/coroutine.png)

#### create

```c
gcoroutine_handle_t tGcoroutineTemplateHandle = {
    // 记录运行状态，当其运行时不可再次挂入协程链表
    .bIsRunning = false,
    .pfcn = NULL,
};
```

#### run

```c
// 将协程函数挂载到协程链表运行
if(GMSI_SUCCESS != gcoroutine_Insert(&tGcoroutineTemplateHandle, \
                                     (void *)ptThis, template_gcoroutine))
{
    GLOG_PRINTF("Error: gcoroutine_Insert failed.");
}

```

#### delete

```c
int gcoroutine_Run(void)
{
    // ...
    if(fsm_rt_cpl == tFsm)
    {
        // 当其运行结束时删除节点
        gcoroutine_Delete(ptHandle);
    }
}
```



