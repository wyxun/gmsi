# 从 GMSI 到 MODUS 迁移升级指南

本指南用于帮助现有使用 `gmsi` 库的项目平滑迁移升级至全新的 `modus` 命名空间。本次升级主要移除了对特定项目（GMSI）的强绑定语义，采用更加通用的 `modus` (模式/方法) 命名，并将核心库的内部目录标准化为 `src`。

> **请注意**：本次迁移仅涉及重命名，**未改变任何核心功能、API 入参和运行逻辑**。带有 `g_` 前缀的普通全局变量（如 `g_hwSystemDataArrary`）因为属于通用全局变量命名习惯，予以保留，无需修改为 `m_`。

## 1. 物理结构与构建系统的修改

### 1.1 构建文件名称与路径
- 核心库文件夹从 `gmsi/` 重命名为 `src/`。
- 构建系统脚本从 `gmsi.mk` 重命名为 `modus.mk`。

在你的业务工程 `makefile` 中：
```diff
- GMSI_ROOT = ../../gmsi
+ MODUS_ROOT = ../..

- include $(GMSI_ROOT)/gmsi.mk
+ include $(MODUS_ROOT)/modus.mk

- C_SOURCES += $(GMSI_SRCS)
+ C_SOURCES += $(MODUS_SRCS)
```
编译选项相关的宏也会发生改变：
```diff
- GMSI_CFLAGS    -> MODUS_CFLAGS
- GMSI_INCLUDES  -> MODUS_INCLUDES
- GMSI_USE_LOG   -> MODUS_USE_LOG
- GMSI_USE_ASSERT-> MODUS_USE_ASSERT
```

### 1.2 头文件包含
包含核心入口文件的名字发生改变：
```diff
- #include "gmsi.h"
+ #include "modus.h"
```

## 2. 代码级标识符与宏替换映射表

为了保持统一的代码风格，原先所有以 `GMSI_` 为前缀的宏，以及代表库特性的 `g` 前缀全部改为了 `m`。你可以使用 IDE 的全局搜索与替换功能（区分大小写）按以下列表进行替换：

### 2.1 全局与基础类型
| 原名称 | 新名称 | 说明 |
|:---|:---|:---|
| `gmsi_t` | `modus_t` | 核心调度上下文结构体 |
| `gmsi_Init()` | `modus_Init()` | 核心初始化函数 |
| `gmsi_Run()` | `modus_Run()` | 主循环调度函数 |
| `gmsi_Clock()` | `modus_Clock()` | 时钟滴答心跳函数 |
| `GMSI_DECLARE_OBJECT` | `MODUS_DECLARE_OBJECT` | 静态对象声明宏 |
| `GMSI_SUCCESS` | `MODUS_SUCCESS` | 返回值定义 |
| `GMSIVersion` | `ModusVersion` | 版本信息变量名 |
| `gbase` / `GBASE_` | `mbase` / `MBASE_` | 基础派生对象相关 |

### 2.2 具体模块前缀映射
原先所有的内部组件都以 `g` 开头（如 `gstorage`, `gshell`），现在统一更名为 `m` 开头：

| 原模块/前缀 | 新模块/前缀 | 说明 |
|:---|:---|:---|
| `gstorage` | `mstorage` | 存储与配置模块 (如 `mstorage_Init`, `mstorage_t`) |
| `gdebug` | `mdebug` | 调试相关模块名 |
| `gshell` | `mshell` | 命令行 shell (如 `mshell_RegisterCmd`) |
| `gwaveform` | `mwaveform` | 波形上位机通信模块 |
| `glog` | `mlog` | 日志模块文件 |
| `GLOG` / `GLOGF` | `MLOG` / `MLOGF` | 打印宏指令 |
| `gcoroutine` | `mcoroutine` | 协程相关 |
| `gdi` / `GDI_` | `mdi` / `MDI_` | 驱动接口层 (如 `MDI_Write`, `mdi_hw.h`) |
| `gringbuf` | `mringbuf` | 环形缓冲区实现 |
| `gblinfo` | `mblinfo` | Bootloader 交互信息模块 |

### 2.3 工具类与底层组件 (Utilities)
本次更新移除了冗余的 `util_queue`，统一合并至高性能的 `mringbuf`。同时，为了剥离对 FreeRTOS 的依赖，自研了 `mlist` 链表。

| 原名称/组件 | 新名称/组件 | 说明 |
|:---|:---|:---|
| `util_queue.h/c` | **已移除** | 请迁移至 `mringbuf.h/c` |
| `util_queue_t` | `mringbuf_t` | 类型替换 |
| `queue_init()` | `mringbuf_Init()` | 初始化（注意：size 必须为 2 的幂） |
| `queue_write()` | `mringbuf_Write()` | 写入数据 |
| `queue_read()` | `mringbuf_Read()` | 读取数据 |
| `queue_isEmpty()` | `mringbuf_GetUsed() == 0` | 状态判断 |
| `List_t` / `ListItem_t` | `mlist_t` / `mlist_item_t` | 内部链表结构重命名 |
| `vListInitialisexxx()` | `mlist_Init()` / `mlist_ItemInit()` | 链表操作接口更新 |

## 3. 外围工具及第三方代码修改

- 如果你使用了 **superwaveform** 上位机，其底层的包含路径（如 `utils/gringbuf`）已经变更为 `utils/mringbuf`。
- 如果你自己实现了硬件抽象层移植（如原来的 `port/gdi_hw.h`），请将文件名同步修改为 `port/mdi_hw.h`，并且把里面的 `GDI_GPIO_HIGH` 等宏替换为 `MDI_GPIO_HIGH`。

## 4. 升级操作建议流程

1. **备份现有工程**，确保能够随时回滚。
2. 更改 `makefile` 或 `CMakeLists.txt`，将 `gmsi` 的引用变更为 `modus`。
3. 执行大小写敏感的全局字符串替换：
   - 替换 `GMSI` -> `MODUS`
   - 替换 `Gmsi` -> `Modus`
   - 替换 `gmsi` -> `modus`
   - 替换 `gstorage` -> `mstorage`, `gshell` -> `mshell`, `gdi_` -> `mdi_` 等等。
4. 排除掉全局变量带来的误伤（如确认 `g_xxxx` 全局变量没有被误替换掉）。
5. `make clean` 后重新编译。如果有遗漏的 `#include "gdebug/..."` 或者 `gdi/gdi.h`，请针对性将其修正为 `mdebug/...` 或 `mdi/mdi.h`。
