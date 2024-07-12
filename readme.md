## GMSI框架

**Generic Mcu Software Infrastructure**

![framework](.assets/framework.jpg)

### Quick Start

#### WSL子系统

+ 启动windows系统的ubuntu子系统（**该教程看网上**）
+ 更新wsl系统环境

```sh
# 更新源
sudo apt-get update
# 更新软件
sudo apt-get upgrade
# 安装相关工具make clang git
sudo apt install clang make git
# 配置git
git config --global user.name "xxx"
git config --global user.email "xxx@yyy.com"
```

+ 拉取GMSI库代码

```sh
# 在用户home目录下创建个人文件夹（看个人喜好）
mkdir code_dir
cd code_dir
# 拉取代码
git clone https://gitee.com/wyxun/gmsi.git
# 测试编译确认环境是否有缺失
cd gmsi/example/posix_uart
make
# 正常情况可看到下面信息，正常生成可执行文件
# clang -std=c11 -W -Ofast -Wno-implicit-function-declaration -Wno-unused-parameter -Wno-int-to-pointer-cast -Wno-sign-compare -# Wno-compare-distinct-pointer-types -I../../gmsi -I/usr/include -I../../gmsi/utilities -I../../ThirdParty/plooc -#D_XOPEN_SOURCE=700 -DLINUX_POSIX -c main.c -o main.o
# clang -std=c11 -W -Ofast -Wno-implicit-function-declaration -Wno-unused-parameter -Wno-int-to-pointer-cast -Wno-sign-compare -# Wno-compare-distinct-pointer-types -I../../gmsi -I/usr/include -I../../gmsi/utilities -I../../ThirdParty/plooc -#D_XOPEN_SOURCE=700 -DLINUX_POSIX -c pc_uart.c -o pc_uart.o
# clang -std=c11 -W -Ofast -Wno-implicit-function-declaration -Wno-unused-parameter -Wno-int-to-pointer-cast -Wno-sign-compare -# Wno-compare-distinct-pointer-types -I../../gmsi -I/usr/include -I../../gmsi/utilities -I../../ThirdParty/plooc -fno-autolink -# Wall -lrt ../../gmsi/glog.o ../../gmsi/gcoroutine.o ../../gmsi/gmsi.o ../../gmsi/gstorage.o ../../gmsi/gbase.o # ../../gmsi/ginput.o ../../gmsi/utilities/trace.o ../../gmsi/utilities/util_queue.o ../../gmsi/utilities/list.o # ../../gmsi/utilities/util_debug.o main.o pc_uart.o pc_clock.o -o demo
# 执行程序,可看到程序正常执行
./demo
```

#### keil环境

+ 拉取库代码

```sh
git clone https://gitee.com/wyxun/gmsi.git
```

+ 创建项目

```sh
cd gmsi
# 创建project文件夹
# 在project文件下创建keil项目
```

+ 配置项目
  + optimization -o0：方便调试，项目后期可改成**-fast**且**关闭debug**：__NO_USE_LOG__
  + **GMSI**：根据截图上的源文件进行包含
  + **Device**：根据芯片选择对应的库文件
  + **Language Extension**：启用PLOOC，扩展使用**LOG_OUT**调试接口
  + 

![image-20240514190553036](.assets/image-20240514190553036.png)

+ 编写object程序

  + 参看**example**下的**template**例子

  ```c
  // 挂载对象到gmsi内部调用
  static gmsi_base_t s_tExampleBase;
  gmsi_base_cfg_t s_tExampleBaseCfg = {
      .wId = EXAMPLE,
      .wParent = 0,
      .FcnInterface = {
          .Clock = example_Clock,
          .Run = example_Run,
      },
  };
  
  int example_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
  {
      int wRet = GMSI_SUCCESS;
      example_t *ptThis = (example_t *)wObjectAddr;
      example_cfg_t *ptCfg = (example_cfg_t *)wObjectCfgAddr;
      GMSI_ASSERT(NULL != ptThis);
      GMSI_ASSERT(NULL != ptCfg);
      
      ptThis->ptBase = &s_tExampleBase;
      ptThis->chExampleData = ptCfg->chExampleData;
  
      if(NULL == ptThis->ptBase)
          wRet = GMSI_EAGAIN;
      else
      {
          s_tExampleBaseCfg.wParent = wObjectAddr;
          wRet = gbase_Init(ptThis->ptBase, &s_tExampleBaseCfg);
      }
      return wRet;
  }
  ```

  + 快速调用方法

    + copy example.c  example.h到项目目录

    ```sh
    cp ../template/example.c .
    cp ../template/example.h .
    ```

    + 全局替换example名字，注意**大小写匹配**

    ```c
    example  --> object
    Example  --> Objeect
    EXAMPLE  --> OBJECT_ID
    ```

    + 将object.c添加到makefile文件

    ```makefile
    SOURCES += object.c
    ```

    + 在main.c文件初始化object

    ```c
    example_cfg_t tEexampleCfg = {
        //.chExampleData = 0,
    };
    example_t tExample;
    
    int main(void)
    {
        example_Init((uintptr_t)&tExample, (uintptr_t)&tEexampleCfg);
        while(1);
    }
    ```

    

+ 编写**main**主程序

```c
int main(void)
{
    system_clock_config();
    // 硬件初始化
    bsp_Init();

    // object初始化
    hvcontroler_Init((uintptr_t)&tHvControler, (uintptr_t)&tHvControlerCfg);
    correspondent_Init((uintptr_t)&tCorrespondent, (uintptr_t)&tCorrespondentCfg);
    example_Init((uintptr_t)&tExample, (uintptr_t)&tEexampleCfg);
    // 初始化gmsi内部模块
    gmsi_Init(&tGmsi);

    while(1)
    {
        // gmsi主程序挂载
        gmsi_Run();
    }
    return 0;
}

// 1ms中断定时器
void timer_handler(int signum)
{
    // gmsi时钟挂载
    gmsi_Clock();
}
```



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
gmsi_base_cfg_t tObjectBaseCfg = {
    .wId = OBJECT,
    /* ... */
};
// gmsi base object init
int object_Init(uint32_t wObjectAddr, uint32_t wObjectCfgAddr)
{
    wRet = gbase_Init(ptThis->ptBase, &tObjectBaseCfg);
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
int object_Clock(uint32_t wObjectAddr)
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
        GMSI_ASSERT(NULL != ptBaseDes);
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
    LOGOUT("List all object:\n");
    // 遍历链表
    while(ptListItemDes != &tListObject.xListEnd){
        LOGOUT("    itme id: %d\n", ptListItemDes->xItemValue);
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



## 使用方法

**参考example/template实现**

### 项目制作方法

#### llvm编译器+cubemx生成

+ 利用cubemx生成gcc项目

+ 修改makefile的编译器

  ```makefile
  CC = $(llvm_path)\bin\clang.exe
  AS = $(llvm_path)\bin\clang.exe
  OBJCOPY = $(llvm_path)\bin\llvm-objcopy
  SIZE = $(llvm_path)\bin\llvm-size
  ```

+ 修改CFLAG

  ```makefile
  CFLAGS = -target armv7em-none-eabi -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
  CFLAGS += -std=gnu11 -Wall -Wextra -Ofast
  CFLAGS += -fno-builtin -ffunction-sections -fdata-sections -fno-strict-aliasing
  CFLAGS += -fshort-enums -fomit-frame-pointer -c -fdata-sections -ffunction-sections
  ```

+ 指定ld文件

  ```makefile
  LDSCRIPT = STM32G431.lds
  ```

#### llvm编译器+vscode编辑

+ 在根目录下创建project文件夹

  ```sh
  mkdir project
  cd project
  ```

+ 拷贝example/template到project文件下，并将template重命名为工程名

  ```sh
  cp -r ../example/template .
  mv template myproject
  ```

+ 修改myproject的makefile文件

  ```makefile
  #TARGET := demo
  TARGET := myproject				# 名字自定义
  
  # 指定项目源文件（芯片库、第三方文件）
  # SOURCES += main.c example.c
  SOURCES += main.c object.c		# 另外补上该项目的对象文件OBJECT.c，泛指所有对象文件
  
  # 指定编译器，根据项目平台决定 嵌入式/x86
  CC = $(llvm_path)\bin\clang.exe
  ```

#### keil+cubemx

+ 利用cubemx生成keil项目
+ 在keil项目上增加gmsi库文件

## MAKEFILE说明

+ 指定编译器

```makefile
// 使用x86下的llvm
CC := clang
CXX := clang++
// 使用嵌入式平台的llvm
CC = $(llvm_path)\bin\clang.exe
AS = $(llvm_path)\bin\clang.exe
OBJCOPY = $(llvm_path)\bin\llvm-objcopy
SIZE = $(llvm_path)\bin\llvm-size
```

+ 编译flag

```makefile
# CFLAG:c文件标志
CFLAGS := -std=c11 -W -Ofast
# ASFLAG：汇编文件标志，仅在嵌入式平台需要
ASFLAGS = -target armv7em-none-eabi -mthumb
# DFLAG：宏标志
DFLAG ：= -D_XOPEN_SOURCE=700 -DLINUX_POSIX
# LDFLAG:链接标志
LDFLAGS += -T $(LDSCRIPT)			# 嵌入式需要ld文件，指定链接顺序
LDFLAGS +=-fno-autolink -Wall -lrt 	# 链接静态库rt
```

+ 源文件路径

```makefile
# 将选定的目录下所有的c文件添加到SOURCES变量
SOURCES += $(foreach dir, $(GMSI_DIR), $(wildcard $(dir)/*.c))
SOURCES += $(foreach dir, $(GMSI_UTL_DIR), $(wildcard $(dir)/*.c))
# 指定添加c文件
SOURCES += main.c pc_uart.c pc_clock.c
```

+ 编译依赖

```makefile
# 将文件链接成可执行文件
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
# 将源文件变异成汇编.o文件
%.o: %.c
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@
```

+ 标号

```makefile
# 删除编译垃圾文件
clean:
	rm -rf obj/$(COBJS) $(TARGET) $(OBJECTS)
# 打印makefile文件里的变量值
printf_value:
	@echo $(info source files is '$(SOURCES)')
	@echo $(info CFILENDIR files is '$(CFILENDIR)')
```

## VS环境准备
+ clangd
+ 跳转与补全
```sh
sudo apt install bear
// 在makefile目录下执行
bear make
```

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

      | 类型                | 前缀 | 注释                  |
      | ------------------- | ---- | --------------------- |
      | uint8_t             | ch   | byte（无符号）        |
      | int8_t              | c    | byte（有符号）        |
      | uint16_t            | hw   | half-word（无符号）   |
      | int16_t             | i    | half-word（有符号）   |
      | uint32_t            | w    | word（无符号）        |
      | int32_t             | n    | word（有符号）        |
      | uint64_t            | dw   | double-word（无符号） |
      | int64_t             | l    | double-word（有符号） |
      | float               | f    |                       |
      | double              | df   | double-float          |
      | bool                | b    | boolean               |
      | 函数指针            | fcn  |                       |
      | typedef struct xx_t | tXxx | 自定义结构体          |
      | typedef enum xx_e   | eXxx | 自定义枚举变量        |
      |                     |      |                       |
      
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
 * Function: gmsi_xxxInit
 * ----------------------------
 * This function initializes a gmsi_base_t structure. It sets the structure's ID and event, 
 * initializes its list item, and inserts the item into a list. If the parent of the configuration 
 * structure is not zero, it sets the parent of the base structure; otherwise, it returns GMSI_EAGAIN.
 *
 * Parameters: 
 * ptBase: A pointer to the gmsi_base_t structure to initialize.
 * ptCfg: A pointer to the configuration structure for the base structure.
 *
 * Returns: 
 * A status code indicating the result of the function. GMSI_SUCCESS if the function succeeds, 
 * GMSI_EINVAL if the parent of the configuration structure is zero.
 */
int gbase_Init(gmsi_base_t *ptBase, gmsi_base_cfg_t *ptCfg)
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
