# GMSI (Generic MCU Software Infrastructure)

GMSI 是一个轻量级、面向对象且高度可移植的嵌入式软件框架。它致力于通过
抽象硬件接口（GDI）与业务逻辑，提供一套统一的对象管理机制，支持任务
的自动发现、初始化和事件驱动运行。
![framework](.assets/framework.jpg)

## 核心特性
- **面向对象设计**：支持多实例，每个模块（驱动、任务、应用）都封装为独立的对象。
- **自动初始化机制**：引入链接脚本段（Linker Section），利用 `GMSI_DECLARE_OBJECT`
  宏实现模块的零修改自动注册，无需手动更新 `main.c` 中的初始化列表。
- **非阻塞事件机制**：内置事件挂起（Pend）与发布（Post）机制，极大简化了异步任务逻辑。
- **协程/状态机支持**：无缝集成 `plooc` (Protected Low-overhead Object-Oriented 
  Programming in C) 与 `perf_counter` 的协程能力，让复杂的时序逻辑编写如同顺序代码。
- **跨平台一致性**：完美适配 **Keil (AC6)**, **LLVM (Clang)**, **GCC**，支持嵌入式裸机与 
  **POSIX (Linux/WSL)** 环境。

---

## 快速开发
### 1. POSIX (WSL/Linux) 环境
建议在 Windows 下安装 Ubuntu 子系统，使用以下流程：
```bash
# 安装基础编译环境
sudo apt-get update
sudo apt install clang make git

# 获取源码并运行示例
git clone https://gitee.com/wyxun/gmsi.git
cd gmsi/example/template
make
./build/example
```

### 2. Keil 环境配置
- **优化选项**：建议使用 `-Ofast` 以获得最佳性能，配置 `-g` 支持调试。
- **编译器支持**：必须使用 **Arm Compiler 6 (AC6)**，并开启 `GNU11` 与 `C11` 标准。
- **预定义宏**：
  - `__NO_USE_LOG__`：禁用日志输出。
  - `__NO_USE_ASSERT`：禁用断言检测。
- **语言扩展**：GMSI 强依赖 `plooc` 实现对象封装，请确保包含路径中含有 `lib/plooc`。

![image-20240514190553036](.assets/image-20240514190553036.png)

---

## 模块开发指南 (Object Template)

GMSI 将每个功能单元抽象为“对象”。参考 `example/template` 目录，一个标准的 
GMSI 对象由以下部分组成：

### 1. 结构定义 (`template.h`)
```c
/* 模块配置参数 */
typedef struct {
    uint8_t  *pchRingBuffer;
    uint16_t  hwRingSize;
} template_cfg_t;

/* 模块对象实体 */
typedef struct {
    gmsi_base_t *ptBase;   /* 必须包含 GMSI 基础类指针 */
    int          wFd;      /* 模块私有数据 */
} template_t;

int template_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);
```

### 2. 功能实现 (`template.c`)
模块物理行为通过 `Init`, `Clock`, `Run` 三个钩子函数挂载到框架：
```c
static gmsi_base_t s_tTemplateBase; // 静态分配基础类实例
static gmsi_base_cfg_t s_tTemplateBaseCfg = {
    .wId = TEMPLATE_ID, // 唯一标识符
    .FcnInterface = {
        .Clock = template_Clock, // 定时调用 (1ms)
        .Run   = template_Run,   // 主循环调用
    },
};

int template_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr) {
    template_t *ptThis = (template_t *)wObjectAddr;
    template_cfg_t *ptCfg = (template_cfg_t *)wObjectCfgAddr;
    
    ptThis->ptBase = &s_tTemplateBase;
    s_tTemplateBaseCfg.wParent = wObjectAddr; // 记录父对象指针
    
    // 初始化私有资源...
    
    return gbase_Init(ptThis->ptBase, &s_tTemplateBaseCfg);
}

int template_Run(uintptr_t wObjectAddr) {
    template_t *ptThis = (template_t *)wObjectAddr;
    // 获取事件并处理
    uint32_t wEvent = gbase_EventPend(ptThis->ptBase);
    if (wEvent) { /* handle event */ }
    return GMSI_SUCCESS;
}
```

### 3. 对象声明与注册 (`main.c`)
无需在主循环代码中手动添加。只需一行宏声明，框架启动时即可自动初始化并开始调
度所有对象。
```c
#include "gmsi.h"
#include "template.h"

/* 声明对象及初始参数 */
GMSI_DECLARE_OBJECT(template, MyTemplate, 
    .hwRingSize = 64,
    .pchRingBuffer = s_chBuffer
);
```

---

## 编码与命名规范
遵循以下前缀规则（匈牙利命名法变体），可确保代码意图一目了然：

| 前缀 | 原始含义 | 示例 | 备注 |
|:---|:---|:---|:---|
| `ch` | `char / uint8_t` | `chState` | 单字节状态或数据 |
| `hw` | `uint16_t` | `hwBufferSize` | 半字 (Half-Word), 16位长度 |
| `w` | `uint32_t` | `wEvent` | 字 (Word), 32位变量 |
| `b` | `bool` | `bIsRunning` | 布尔标志 |
| `pt` | `Pointer to Type`| `ptThis`, `ptMotor`| 指向结构体或自定义类型的指针 |
| `pch`| `uint8_t *` | `pchBuffer` | 指向字节流的指针 |
| `pfcn`| `Function Ptr` | `pfcnCallback` | 函数指针 |
| `s_` | `Static` | `s_tGmsiBase` | 静态变量 (文件作用域) |
| `g_` | `Global` | `g_hwCount` | 全局变量 |

- **行宽限制**：所有源代码行的最大长度不得超过 **86个字符**。

---

## GStorage — 持久化存储模块

`gstorage` 提供自动 CRC 校验的数据持久化能力：当 RAM 中的数据发生变化时，
定时检测并将数据写入 Flash；系统启动时自动从 Flash 恢复数据。

### 功能特性
- **定时自动保存**：以 CRC 变化检测为触发，避免频繁擦写
- **CRC-16 完整性校验**：读取时自动验证，损坏时打印告警
- **Erase-before-Write**：写入前自动执行 Unlock → Erase → Write → Lock
- **GDI 接口解耦**：框架层与芯片无关，芯片端只需实现 `gdi_flash_t`

### Flash 移植步骤

**1. 在 port 文件中实现 `gdi_flash_t`（以 AT32 为例）：**
```c
static int32_t my_flash_erase(void *p, uint32_t addr, uint32_t size) {
    (void)p;
    return (int32_t)blm_port_FlashErase(addr, size);
}
static int32_t my_flash_write(void *p, uint32_t addr,
                              const uint8_t *data, uint32_t len) {
    (void)p;
    return (int32_t)blm_port_FlashWrite(addr, data, len);
}
static int32_t my_flash_read(void *p, uint32_t addr,
                             uint8_t *buf, uint32_t len) {
    (void)p;
    return (int32_t)blm_port_FlashRead(addr, buf, len);
}
static int32_t my_flash_unlock(void *p) { (void)p; return blm_port_FlashUnlock(); }
static int32_t my_flash_lock  (void *p) { (void)p; return blm_port_FlashLock(); }

static gdi_flash_t s_tFlashApp = {
    .pPriv    = NULL,
    .fnErase  = my_flash_erase,
    .fnWrite  = my_flash_write,
    .fnRead   = my_flash_read,
    .fnUnlock = my_flash_unlock,
    .fnLock   = my_flash_lock,
};

const gdi_hardware_t HW = {
    /* ... 其他外设 ... */
    .ptAppFlash = &s_tFlashApp,
};
```

**2. 在 `gdi_hw.h` 中声明 Flash 资源：**
```c
typedef struct {
    gdi_gpio_t   *ptLedStatus;
    gdi_flash_t  *ptAppFlash;   /* ← 添加此字段 */
} gdi_hardware_t;
```

**3. 在 `main.c` 中注册 GStorage 对象：**
```c
/* 存储描述符，ptFlash 留 NULL — 由 gmsi_Init 框架内部自动绑定 */
static gstorage_data_t s_tStorageData = {
    .ptFlash             = NULL,
    .wFlashAddr          = 0x0800F800,     /* Flash 存储地址 */
    .pchStorageStartAddr = (uint8_t *)&tAppData,
    .hwStorageLength     = sizeof(tAppData) - 2, /* 末尾 2 字节留给 CRC */
};

GMSI_DECLARE_OBJECT(gstorage, GStorage,
    .ptStorageObject  = &s_tStorageData,
    .hwStorageTimeOut = 5000,              /* 5 秒检测一次变化 */
);

/* main.c 只需在 gmsi_Init 前一行赋值，框架内部完成绑定 */
static gmsi_t s_tGmsi;

int main(void) {
    System_Init();
    s_tGmsi.ptAppFlash = HW.ptAppFlash;   /* ← 唯一需要的绑定操作 */
    gmsi_Init(&s_tGmsi);
    /* ... */
}
```

> [!NOTE]
> `hwStorageLength` 必须比数据结构小 2 字节（CRC 占用末尾 2 字节）。
> `wFlashAddr` 必须是 Flash 页面对齐地址，Erase 以此为起点擦除覆盖数据所在的页。

---

## 系统集成 (System Integration)

### 核心初始化流
`gmsi_Init()` 会解析链接器生成的 `init_infos` 数据段，动态执行所有已声明对象的
初始化函数。
```c
int main(void) {
    gmsi_t tGmsi = { 0 };
    
    gmsi_Init(&tGmsi); // 自动初始化所有模块
    while (1) {
        gmsi_Run();    // 轮训调度所有模块的 Run() 与协程
    }
}

// 滴答定时器中断处理
void SysTick_Handler(void) {
    gmsi_Clock();      // 驱动各模块内部时钟（1ms 精度）
}
```

---

## Git 子模块管理 (Git Submodules Configuration)
由于外设底层、核心 CMSIS、GMSI 均属于独立外链依赖仓库。
**首次克隆本仓库架构：**
务必带上 `--recursive` 参数，拉取所有依赖：
```bash
git clone --recursive https://your-repo-url/template-project.git
```
**忘记带参的自救指令：**
```bash
git submodule update --init --recursive
```