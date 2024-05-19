### 创建对象

#### Base对象

+ 基类初始化

```c
// 创建gmsi_base_t对象，
static gmsi_base_t s_tExampleBase;
// 创建base的cfg对象，并进行初始化
gmsi_base_cfg_t s_tExampleBaseCfg = {
    .wId = EXAMPLE,
    /* 获取父指针，在init里初始化该变量 */
    .wParent = 0,
    // 对象的核心函数，自动调用的对象主程序和对象时钟函数
    .FcnInterface = {
        .Clock = example_Clock,
        .Run = example_Run,
    },
};
```

+ 基类所需指针函数

```c
// 时钟函数，该函数会自动挂载在硬件中断
int example_Clock(uint32_t wObjectAddr)
{
    return 0;
}

// 主程序，该函数自动挂载在while程序
int example_Run(uint32_t wObjectAddr)
{
    return 0;
}
```

#### example对象

+ 在头文件创建实际对象，其必须继承基类**base_t**

```c
// 定义对象信息
typedef struct{
    uint8_t chExampleData;
}example_cfg_t;

typedef struct{
    /*第一个成员固定是base*/
    gmsi_base_t *ptBase;
    uint8_t chExampleData;
}example_t;
```

### 挂载GMSI

```c
int example_Init(uint32_t wObjectAddr, uint32_t wObjectCfgAddr)
{
    int wRet = GMSI_SUCCESS;
    example_t *ptThis = (example_t *)wObjectAddr;
    example_cfg_t *ptCfg = (example_cfg_t *)wObjectCfgAddr;
    GMSI_ASSERT(NULL != ptThis);
    GMSI_ASSERT(NULL != ptCfg);
    // 初始化基类
    ptThis->ptBase = &s_tExampleBase;
    ptThis->chExampleData = ptCfg->chExampleData;

    // 资源不可用
    if(NULL == ptThis->ptBase)
        wRet = GMSI_EAGAIN;
    else
    {
        // 设定父指针
        s_tExampleBaseCfg.wParent = wObjectAddr;
        wRet = gbase_Init(ptThis->ptBase, &s_tExampleBaseCfg);
    }
    return wRet;
}
```

### 编写功能

### 创建协程

