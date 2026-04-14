# gshell — GMSI 极简调试 Shell 使用指南

> **适用版本：** GMSI v0.2.1.0+

## 概述

gshell 是 GMSI 框架内置的轻量级调试 Shell。以 `gshell_io_t` 抽象 I/O 后端，
默认对接 SEGGER RTT，也可通过 `gshell_SetIO()` 替换为 UART 或任意字节流接口。
Shell 在 `gmsi_Run()` 中自动轮询，**无需修改主循环**，开箱即用。

---

## 快速上手

连接 J-Link RTT Viewer 后，目标启动即可看到提示符：

```
[gshell] ready. Type 'help'.
>
```

在任何 `.c` 文件中定义命令（零代码初始化）：

```c
#include "utilities/gshell.h"

static void cmd_burn(const char *args) {
    (void)args;
    GLOGF(I, "Burn-in test started.\r\n");
}

/* 自动注册：无需在 main 中调用任何函数 */
GMSI_SHELL_CMD(burn, cmd_burn, "Start burn-in aging test");
```

---

## 内置命令

| 命令 | 用法 | 说明 |
|------|------|------|
| `help` | `help` | 列出所有已注册命令及帮助文本 |
| `ver`  | `ver`  | 打印 GMSI 版本号 |
| `list` | `list` | 遍历 gbase 对象链表，打印每个对象的 id 与当前 event |
| `post` | `post <id_hex> <event_hex>` | 向指定 id 的对象投递事件（十六进制） |
| `log`  | `log [-E][-W][-I][-D]` | 运行期切换日志级别；无参数 = 显示当前状态 |

---

## log 命令 — 运行期日志控制

GLOG / GLOGF 宏采用**两级过滤**：

1. **编译期** `GMSI_LOG_LEVEL`（上界）：超过此级别的代码被完全剔除，零运行时开销
2. **运行期** `g_chGLogMask`（位掩码）：在编译期允许的范围内，动态开关各级别

| 位 | 级别 | 掩码宏 |
|----|------|--------|
| bit0 | E (Error) | `GLOG_MASK_E` (0x01) |
| bit1 | W (Warn)  | `GLOG_MASK_W` (0x02) |
| bit2 | I (Info)  | `GLOG_MASK_I` (0x04) |
| bit3 | D (Debug) | `GLOG_MASK_D` (0x08) |

**交互示例：**

```
> log
Log mask:  [E][W][I][D]         ← 全开（默认状态）

> log -I                        ← 关闭 INFO
Log mask:  [E][W]( )[D]

> log -I                        ← 再次切换，恢复 INFO
Log mask:  [E][W][I][D]

> log -D -W                     ← 同时关闭 DEBUG 和 WARN
Log mask:  [E]( )[I]( )
```

**启动时默认值：** 由 `GLOG_MASK_DEFAULT` 控制，默认 `GLOG_MASK_ALL`（0x0F，全开）。
可在 `userconfig.h` 中覆盖，例如仅保留 Error + Warn：

```c
// userconfig.h
#define GLOG_MASK_DEFAULT   (GLOG_MASK_E | GLOG_MASK_W)  /* 0x03u */
```

---

## 可配置宏

在项目的 `userconfig.h`（或编译器 `-D` 定义）中覆盖：

| 宏 | 默认值 | 说明 |
|----|--------|------|
| `GSHELL_LINE_SIZE` | 32 | 命令行缓冲区字节数（含末尾 `\0`） |
| `GSHELL_MAX_CMDS` | 16 | 最大注册命令数（含内置 5 条） |
| `GSHELL_USE_RTT_DEFAULT` | 1 | 0 = 不编译 RTT 后端代码，需手动调用 `gshell_SetIO()` |
| `GLOG_MASK_DEFAULT` | `GLOG_MASK_ALL` | 启动时日志掩码初始值 |

---

## 替换为 UART 后端

当目标硬件无 J-Link 调试器时，将 I/O 接口替换为 UART：

```c
#include "utilities/gshell.h"

/* 非阻塞读：无数据时立即返回 0 */
static unsigned uart_read(char *pchBuf, unsigned hwSize) {
    return my_uart_receive_nonblock(pchBuf, hwSize);
}
static void uart_write(const char *pchBuf, unsigned hwSize) {
    my_uart_send(pchBuf, hwSize);
}
static const gshell_io_t s_tUartIO = {
    .pfcnRead  = uart_read,
    .pfcnWrite = uart_write,
};

// 在 gmsi_Init() 前或后（Poll 首次调用前）均可：
gshell_SetIO(&s_tUartIO);
```

> **重要：** `pfcnRead` 必须是**非阻塞**的。若无数据应立即返回 0，
> 阻塞等待会卡死 `gmsi_Run()` 的整个调度循环。

---

## 与 GLOGF 的 %f 支持协同

自 v0.2.1.0 起，`GLOGF` / `util_debug_Printf` 支持 `%f` 浮点格式化：

```c
static void cmd_status(const char *args) {
    (void)args;
    float fVbat = read_vbat();
    GLOGF(I, "VBAT = %f V\r\n", fVbat);
}
```

实现路径：`TRACE.ToString.Float()` → `trace_fmt_float()`，无需 `snprintf`。

---

## 注意事项

- **命令注册顺序**：用户命令须在 `gshell_Poll()` 首次调用前（`gmsi_Init()` 之后）通过
  `gshell_RegisterCmd()` 注册。内置命令（help/ver/list/post/log）在 Poll 首次调用时
  自动注册，排在用户命令之后，因此用户命令优先显示在 `help` 列表中。

- **缓冲区大小**：`GSHELL_LINE_SIZE` 含末尾 `\0`，实际可输入命令长度为 `GSHELL_LINE_SIZE - 1`。

- **命令表满**：`gshell_RegisterCmd()` 返回 `GMSI_ENOMEM`，增大 `GSHELL_MAX_CMDS` 即可。

- **`log` 命令与编译期的关系**：`log -D` 只能关闭运行期 DEBUG 打印，
  若编译期 `GMSI_LOG_LEVEL < GMSI_LOG_LEVEL_DEBUG`，则 DEBUG 代码已被裁剪，
  运行期无论如何都不会输出。
