## GMSI框架

**Generic Mcu Software Infrastructure**

![framework](.assets/framework.jpg)

### 文件分层介绍

```c
// gmsi库实现文件
gmsi
--->>gmsi.c/h
--->>base.c/h
--->>global_define.h
// gmsi组件
--->>component/
----------->>gmsi_uart.c/h
--->>utilities/
----------->>utilities.h
----------->>list.c/h
----------->>util_queue.c/h
// 第三方库
ThirdParty/
--->>PLOOC/
// gmsi框架图
image/
-------->>Base.drawio
-------->>framework.drawio
// 不同平台下例子(注释满足Doxygen)
example/
// cortex_m平台下例子
--->>cortex_m/
// linux平台下例子
--->>posix_uart/
// gmsi通用模板
--->>template/
```

### GMSI理解与应用

#### object相互通信机制

```c
/***************************base.h实现******************************/
// object身份ID
typedef struct{
    uint32_t wId;
    /* ... */
}gmsi_base_cfg_t;
/***************************global_define.h实现**********************/
// object ID 基类划分
#define GMSI_ID_UART        1
#define GMSI_ID_IIC         2
#define GMSI_ID_SPI         3
#define GMSI_ID_CAN         4
#define GMSI_ID_INPUT       5
#define GMSI_ID_SENSOR      6
#define GMSI_ID_TIMER       7
// SOFTWARE
#define GMSI_ID_CLASS       100
#define GMSI_ID_MOCK        101

/***************************userconfig.h实现**************************/
// 定义一个具体object ID
#define OBJECT     (GMSI_ID_MOCK<<8+1)
#define OBJECT2    (GMSI_ID_MOCK<<8+2)

/***************************object.c实现******************************/
// object init id
gmsi_base_cfg_t s_tObjectBaseCfg = {
    .wId = EXAMPLE,
    /* ... */
};
// gmsi base object init
int object_Init(uint32_t wObjectAddr, uint32_t wObjectCfgAddr)
{
    wRet = gbase_Init(ptThis->ptBase, &s_tExampleBaseCfg);
    {
        ptBase->wId = ptCfg->wId;
    }
}
// object事件传递实现
int object_Run(uint32_t wObjectAddr)
{
    // wObjectAddr转换成object指针
    // 等待捕获事件处理
    wEvent = gbase_EventPend(ptThis->ptBase);
    if(wEvent)
        example_EventHandle(ptThis, wEvent);
}
int object_Clock(uint32_t wObject)
{
    // 定时向其他object发送事件
    gbase_EventPost(OBJECT2, Gmsi_Event00);
    {
        // 遍历链表，找到OBJECT2 id
        if(ptListItemDes->xItemValue == wId)
        	ptBaseDes = ptListItemDes->pvOwner;
        // 对OBJECT2的事件赋值
        ptBaseDes->wEvent |= wEvent;
    }
}
```

#### object自动运行实现机制

```c
/***************************object.c实现******************************/
// gmsi base object init
gmsi_base_cfg_t s_tExampleBaseCfg = {
    .wId = EXAMPLE,
    /* 获取父指针 */
    .wParent = 0,
    /* 将gmsi的接口函数地址写入到base_cfg_t */
    .FcnInterface = {
        .Clock = object_Clock,
        .Run = object_Run,
    },
};

// object init
int object_Init(uint32_t wObjectAddr, uint32_t wObjectCfgAddr)
{
    // 初始化Parent addr
    s_tObjectBaseCfg.wParent = wObjectAddr;
    wRet = gbase_Init(ptThis->ptBase, &s_tExampleBaseCfg);
}

/***************************gmsi.c实现******************************/
void gmsi_Run(void)
{
    // 遍历链表，执行对应object的Run函数
	ptBaseDes->pFcnInterface->Run(ptBaseDes->wParent);
}
void gmsi_Clock(void)
{
	// 遍历链表，执行对应object的Clock函数
	ptBaseDes->pFcnInterface->Clock(ptBaseDes->wParent);
}

```

#### LIST通信实现
##### object查找机制
```c
/* 
	1、根据目标id号遍历链表，提取对应的base
	2、把对应的event或message写入对应的base
*/    
/***************************base.c实现******************************/
	// 设置链表头部
struct xLIST        tListObject;

/*****************遍历链表函数段 start*****************/
    struct xLIST_ITEM *ptListItemDes = tListObject.xListEnd.pxPrevious;
    uint8_t chErgodicTime = 1;
    gmsi_base_t *ptBaseDes;
    // 遍历链表，确定目的id
    while(ptListItemDes != &tListObject.xListEnd){
        if(ptListItemDes->xItemValue == wId)
            break;
        // 不匹配继续遍历
        chErgodicTime++;
        ptListItemDes = ptListItemDes->pxPrevious;
    }
    if(chErgodicTime <= tListObject.uxNumberOfItems)
    {
        // 找到对应的object
        ptBaseDes = ptListItemDes->pvOwner;
        assert(NULL != ptBaseDes);
        // 操作对应的object
        ptBaseDes->tMessage.pchMessage= pchMessage;
        ptBaseDes->tMessage.hwLength = hwLength;
        ptBaseDes->wEvent |= Gmsi_Event_Transition;
    }
/*****************遍历链表函数段 end*****************/
```

#### GMSI通用

##### 通用函数

+ 异常处理机制

```c
// 错误打印
void gmsi_errorlog(int wErrorNum)
{
    switch(wErrorNum)
    {
        case GMSI_SUCCESS:
        break;
        
        default:
        break;
    }
}
// 遍历所有对象
void gbase_DegugListBase(void)
{
    struct xLIST_ITEM *ptListItemDes = tListObject.xListEnd.pxPrevious;
    printf("List all object:\n");
    // 遍历链表
    while(ptListItemDes != &tListObject.xListEnd){
        printf("    itme id: %d\n", ptListItemDes->xItemValue);
        ptListItemDes = ptListItemDes->pxPrevious;
    }
}
```



##### 返回值

```c
// GMSI返回值，基于LINUX
#define GMSI_SUCCESS        0
#define GMSI_EPERM          -1
#define GMSI_ENOENT         -2
#define GMSI_ESRCH          -3  
#define GMSI_EINTR          -4
```



### 使用方法

**参考example/template实现**



## 编码要求

### 编码格式

+ 使用四个空格替代TAB

+ 关于指针

  + 指针使用前要判断是非为空（NULL）

+ 关于布尔值

  + 尽可能使用stdbool.h提供的布尔型

  + 不允许直接使用0和1表示false和true。应该直接使用false和true。

  + 如果对应的C系统未提供stdbool.h可以自行定义，通常定义如下

    + ```c
      typedef enum {
          false = 0,
          true = !false
      } bool;
      ```

  + **绝对不允许在逻辑表达式中与true进行比较**。因为在布尔量中只有false是确定的值（0），true可以用任何非零值表示，是不确定的，因而要避免和true进行比较。

+ 关于命名规则

  + 变量

    + 变量使用名词性短语构成

    + 单词首字母大写

    + 单词与单词之间**不允许**用下划线隔开

    + 使用改良的匈牙利命名法

      + 使用改良的匈牙利命名法；用前缀表示变量类型；

      | 类型     | 前缀 | 注释         |
      | -------- | ---- | ------------ |
      | uint8_t  | ch   | byte         |
      | int8_t   | c    |              |
      | uint16_t | hw   | half-word    |
      | int16_t  | i    |              |
      | uint32_t | w    | word         |
      | int32_t  | n    |              |
      | uint64_t | dw   | double-word  |
      | int64_t  | l    |              |
      | float    | f    |              |
      | double   | df   | double-float |
      | bool     | b    | boolean      |
      | 函数指针 | fcn  |              |

      + 指针的前缀是“p”，指向指针的指针是“pp”，以此类推，一般用不到“pp”以上的。如果是函数指针，则用fcn

      + 所有自定义的变量类型前缀都是“t”

      + 对于特殊修饰的变量再加入不同的前缀

        + static型变量追加“s_”

        + 全局变量追加“g_”

        + const修饰的变量追加“c_”，当const和static同时出现使用“c_”

          ```c
          bool g_bFlag = false;			//!< 全局变量
          static uint16_t s_hwValue;		//!< 静态变量
          
          typedef struct {
              …
          }example_t;
          
          {
              example_t tExample ;		//!< 局部变量
          …
          }
          ```

      + 宏（Macro）与枚举（enum）

        + 由短语构成且一律**大写**
        + 单词与单词之间用下划线隔开
        + 有返回值的宏，请使用括号包裹起来（常数/常量除外）
        + 没有返回值的宏，请使用 do {} while(0) 包裹起来

      + 函数（Function）

        + 由短语构成且原则上一律**小写**
        + 模块名与描述单词之间用下划线隔开，其中描述单词首字母大写

  + 其他

    + 如果和常量进行“==”运算，常量应该放到表达式的左边

### 注释模板

**支持Doxygen**

+ 文件头部

```c
/**
* @file         
* @brief		This is a brief description.
* @details	    This is the detail description.
* @author		author
* @date		    date
* @version	    v1.0
* @par Copyright(c): 	abc corporation
* @par History:         
*	version: author, date, desc\n
*/
```

+ 函数注释

```c
 /**
 * @brief		This is a brief description.
 * @details	    This is the detail description. 
 * @param[in]	inArgName input argument description.
 * @param[out]	outArgName output argument description. 
 * @retval		0		成功
 * @retval		ERROR	错误 
 * @par 标识符
 * 		保留
 * @par 其它
 * 		无
 * @par 修改日志
 * 		XXX于2023-10-03创建
 */
greturn_error_t gmsi_xxxInit(gmsi_xxx_t *ptxxx, gmsi_xxx_cfg_t *ptCfg);
```

+ 数据结构

```c
/** 
 * @brief		This is a brief description.
 * @details	    This is the detail description. 
 */
typedef struct
{
	int wVar1; /*!< Detailed description of the member var1 */
	int wVar2; /*!< Detailed description of the member var2*/
	int wVar3; /*!< Detailed description of the member var3 */
} gmsi_xxx_t;

```

+ 宏注释

```c
// 上方或者右方皆可
/** Description of the macro a */
#define a		0

#define b		0  /*!< Description of the macro b */

```

+ 全局和静态变量注释

```c
/**  Description of global variable  */
int g_qwe = 0;
 
int static  s_asd = 0; /*!< Description of static variable */
```

+ 常用标签命令关键字
  + 文件信息：
    + @file--> 文件声明，即当前文件名
    + @author --> 作者
    + @todo --> 改进，可以指定针对的版本
  + 模块信息：
    + @var --> 模块变量说明
    + @typedef --> 模块变量类型说明
  + 函数信息：
    + @param --> 参数说明
    + @arg --> 列表说明参数信息
    + @return --> 返回值说明
    + @retval --> 返回值类型说明
    + @note --> 注解
  + 提醒信息：
    + @brief --> 摘要，即当前文件说明
    + @see --> 参看
    + @attention --> 注意
    + @bug --> 问题
    + @warning --> 警告
    + @sa --> 参考资料
