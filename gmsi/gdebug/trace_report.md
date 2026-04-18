# Trace 模块统一重构报告

> 日期：2026-03-08  
> 目标：将 blm 示例中独立的 trace 实现合并回 gmsi 库，通过条件编译实现 PC / MCU 双平台兼容，消除冗余并剥离 MCU 端对 `printf`（stdio）的依赖。

---

## 一、修改点总览

| # | 文件 | 操作 | 路径 |
|---|------|------|------|
| 1 | `trace_fmt.h` | **新增** | `gmsi/utilities/trace_fmt.h` |
| 2 | `trace_fmt.c` | **新增** | `gmsi/utilities/trace_fmt.c` |
| 3 | `trace.c` | **重写** | `gmsi/utilities/trace.c` |
| 4 | `makefile` | **修改** | `example/blm/makefile` |
| 5 | `trace_stub.c` | **删除** | `example/blm/core/trace_stub.c` |

---

## 二、各修改点详细说明

### 1. [新增] `trace_fmt.h` / `trace_fmt.c` — 轻量级数字转字符串

**原因**：MCU 端需要将 `uint32_t`、`int32_t`、`float` 等数值转换为字符串来输出，但标准库的 `snprintf` / `printf` 会拉入整个格式化解析引擎（尤其是 `%f` 浮点转换），导致固件膨胀数 KB。

**方案**：手写三类极简转换函数，零外部依赖：

| 函数 | 算法 | 说明 |
|------|------|------|
| `trace_fmt_hex32/16/8` | 位移 + 查表 | `(val >> (i*4)) & 0x0F` 索引 `"0123456789ABCDEF"`，无任何除法 |
| `trace_fmt_int32` | `% 10` + `/ 10` 循环 | 经典 itoa，特殊处理 `INT32_MIN` 溢出 |
| `trace_fmt_float` | 整数拆分法 | 分离整数部分和小数部分，各自用整数方式打印，默认 4 位小数 |

**关键设计**：整个 `trace_fmt.c` 被 `#if !TRACE_USE_LIBC_PRINTF ... #endif` 包裹。PC 端编译时文件体为空，不产生任何符号。

---

### 2. [重写] `trace.c` — 条件编译双模式

**原因**：之前 `trace.c` 硬编码使用 `printf`，PC 端可用但 MCU 端会带来不必要的库体积。blm 工程被迫单独维护 `trace_stub.c`，造成代码冗余。

**方案**：在文件顶部增加平台自动检测：

```c
#ifndef TRACE_USE_LIBC_PRINTF
#   if defined(_MSC_VER) || defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
#       define TRACE_USE_LIBC_PRINTF    1   // PC 端：自动走 printf
#   else
#       define TRACE_USE_LIBC_PRINTF    0   // MCU 端：自动走轻量级路径
#   endif
#endif
```

每个 `__trace_xxx` 函数内部用 `#if TRACE_USE_LIBC_PRINTF / #else` 分流：
- **PC 路径**（`=1`）：原封不动保留所有 `printf(...)` 调用，行为完全不变
- **MCU 路径**（`=0`）：调用 `trace_fmt_*` 转到 `char buf[]`，再通过 `TRACE_MCU_WRITE_STRING(buf)` 输出

**MCU 输出后端注入**：通过编译期宏 `TRACE_MCU_WRITE_STRING(s)` 由用户指定具体实现（如 `SEGGER_RTT_WriteString(0, s)`），零运行时开销。

---

### 3. [修改] `example/blm/makefile`

**原因**：blm 工程之前编译自己的 `core/trace_stub.c`，现在要切换到统一的 `trace.c`。

**改动**：
- `C_SOURCES` 中移除 `core/trace_stub.c`，添加 `$(GMSI_DIR)/utilities/trace.c` 和 `$(GMSI_DIR)/utilities/trace_fmt.c`
- `C_DEFS` 中新增两个宏定义：
  - `-DTRACE_USE_LIBC_PRINTF=0` — 显式指定 MCU 模式
  - `-DTRACE_MCU_WRITE_STRING(s)=SEGGER_RTT_WriteString(0,(s))` — 绑定 RTT 输出

---

### 4. [删除] `example/blm/core/trace_stub.c`

**原因**：该文件的全部功能（RTT 初始化、各类型转字符串、流式 hex dump）已合并进统一的 `trace.c` MCU 分支，保留会造成符号重复定义和维护冗余。

---

## 三、其他工程接入指南

### Keil 工程

在 **Options → C/C++ → Preprocessor Symbols → Define** 中添加：

```
TRACE_USE_LIBC_PRINTF=0
TRACE_MCU_WRITE_STRING(s)=SEGGER_RTT_WriteString(0,(s))
```

同时确保工程包含 `SEGGER_RTT.c` 和 `trace_fmt.c` 的编译。

### PC 端示例（template 等）

**无需任何修改**。`TRACE_USE_LIBC_PRINTF` 会自动检测到 `__linux__` / `_WIN32` 而设为 `1`，走原始 `printf` 路径。`trace_fmt.c` 被条件编译排空，不影响链接。
