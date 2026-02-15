# BLM (Bootloader Manager)

BLM 是一个基于 [GMSI](https://github.com/wyxun/gmsi) 和 [perf_counter](https://github.com/GorgonMeducer/perf_counter) 的轻量级 Cortex-M Bootloader 示例。它实现了通过 Ymodem 协议进行串口固件升级的功能，并且代码体积极小（< 16KB）。

## 🔥 特性

- **多芯片支持**: 目前支持 **STM32G431** 和 **AT32F407**。
- **极简设计**: 核心代码量少，依赖清晰。
- **高性能**: 集成 `perf_counter` 提供高精度计时与状态机调度。
- **协议支持**: 标准 Ymodem 协议，支持常见的串口工具（SecureCRT, TeraTerm, Xshell）。
- **共享机制**: 通过 `gblinfo` (Flash 固定区域) 与 APP 共享版本信息和升级标志。
- **工具链友善**: 支持 LLVM Embedded Toolchain for Arm (Clang) 和 Arm GNU Toolchain (GCC)。
- **调试友好**: 提供完整的 Makefile 调试目标和 GDB 脚本支持。
- **资源占用**: Flash < 8KB (Release -Oz), RAM < 2KB.

## 📂 目录结构

```
example/blm/
├── core/               # 核心逻辑 (MCU无关)
│   ├── blm.c           # 主状态机 (App 跳转/升级流程)
│   └── blm_protocol.c  # Ymodem 协议解析
├── port/               # 移植层 (MCU相关)
│   ├── stm32g4/        # STM32G4 系列移植 (C文件, Linker, Startup)
│   └── at32f4/         # AT32F4 系列移植
├── cmsis/              # CMSIS 头文件 (重构版)
│   ├── core_cm4.h      # Cortex-M4 通用内核定义
│   ├── stm32g431xx.h   # STM32G431 外设定义
│   └── at32f407xx.h    # AT32F407 外设定义
├── main.c              # 入口函数
├── makefile            # 多芯片支持编译脚本
└── debug.gdb           # GDB 自动调试脚本
```

## 🛠️ 环境搭建 (Environment Setup)

### 2. OpenOCD (ArteryTek 分支)

由于 AT32F4 系列需要特定的 OpenOCD 支持，推荐编译安装 ArteryTek 官方分支。

**源码仓库**: [https://github.com/ArteryTek/openocd](https://github.com/ArteryTek/openocd)

#### 编译步骤 (Linux / WSL2)

```bash
# 1. 安装依赖
sudo apt update
sudo apt install build-essential pkg-config autoconf automake libtool libusb-1.0-0-dev libgpiod-dev texinfo

# 2. 克隆仓库
git clone https://github.com/ArteryTek/openocd.git
cd openocd

# 3. 生成配置
./bootstrap

# 4. 配置 (确保启用 cmsis-dap)
./configure --enable-cmsis-dap

# 5. 编译与安装
make -j4
sudo make install
```

#### 💡 常见问题与处理 (Troubleshooting)

**问题 1: `configure: error: libusb-1.x is required`**
- **原因**: 缺少 libusb 开发包。
- **解决**: `sudo apt install libusb-1.0-0-dev`

**问题 2: `Makefile: No such file or directory` (make 失败)**
- **原因**: `./configure` 这一步出错或未执行。
- **解决**: 检查 `./configure` 的输出，确保没有 Error。通常是缺少依赖工具 (如 `pkg-config` 或 `libtool`)。

**问题 3: WSL2 下 `Error: CMSIS-DAP command mismatch` / `unable to open CMSIS-DAP device`**
- **原因**: WSL2 的 USBPassthrough (usbipd) 对 CMSIS-DAP v2 的 Bulk 接口支持不稳定，或者被 Windows 侧驱动占用。
- **解决**:
  1.  确保使用了 `usbipd attach --wsl --busid <X-X>` 挂载设备。
  2.  **强制使用 HID 后端**: 在 OpenOCD 启动参数中添加 `-c "cmsis_dap_backend hid"`。本项目 Makefile 默认已包含此配置。

**问题 4: `Error: dsp563xx.c: ... variable ‘move_cmd’ set but not used`**
- **原因**: 某些新版 GCC/Clang 检查严格，导致将 Warning 视为 Error。
- **解决**: 在 Configure 时禁用 Werror: `./configure --enable-cmsis-dap --disable-werror`

### 3. 工具链 (Compiler)

推荐使用 **LLVM Embedded Toolchain for Arm** (Clang) 或 **Arm GNU Toolchain** (GCC)。
请确保编译器路径已添加到系统 PATH，或者修改 `makefile` 中的 `LLVM_PATH` 变量。

### 2. OpenOCD & 调试器

本项目使用 OpenOCD 连接 CMSIS-DAP 调试器。

#### Windows / Linux (Native)
直接安装 OpenOCD 即可。

#### WSL2 (Windows Subsystem for Linux) ⚠️ 注意
在 WSL2 中使用 USB 调试器需要特殊配置：

1.  **安装 usbipd-win**: 在 Windows 主机安装 `usbipd-win` (>= 4.0.0)。
2.  **连接设备**:
    ```powershell
    # Windows PowerShell (管理员)
    usbipd list
    usbipd bind --busid <BUSID>
    usbipd attach --wsl --busid <BUSID>
    ```
3.  **配置 OpenOCD (HID Backend)**:
    Linux 默认的 `libusb` 在 WSL2 下可能无法识别某些 CMSIS-DAP适配器的 Bulk 接口。**必须强制使用 HID 后端**。
    
    本项目 Makefile 已内置支持：
    ```makefile
    OPENOCD_CMD = openocd -f interface/cmsis-dap.cfg -c "cmsis_dap_backend hid" ...
    ```

## 🚀 编译与烧录 (Build & Flash)

使用 `CHIP` 变量选择目标芯片。默认为 `stm32g4`。

### 命令列表

| 目标 (Target) | 命令 | 说明 |
| :--- | :--- | :--- |
| **STM32G431** | `make` / `make CHIP=stm32g4` | 编译 STM32G4 版 |
| **AT32F407** | `make CHIP=at32f4` | 编译 AT32F4 版 |
| **清理** | `make clean` | 清理编译产物 |
| **烧录** | `make CHIP=... flash` | 编译并烧录到芯片 |
| **检查大小** | `make CHIP=... check-size` | 检查是否超过 Flash 限制 |

### 编译选项 (Build Options)

Makefile 会根据目标自动调整优化等级：

- **Release (默认)**: `make` ->使用 `-Oz` (最小体积优化)。
- **Debug**: `make debug` -> 使用 `-O0` (无优化) + `-g` (调试符号)。

## 🐛 调试指南 (Debugging)

本项目提供了便捷的 GDB 调试工作流，无需 VSCode 插件也可轻松调试。
(Debug 模式下代码体积会膨胀至 ~12KB，请确保 Flash 空间足够)

### 方法：双终端调试 (Two-Terminal Workflow)

**终端 1: 启动 OpenOCD Server**
此命令会启动 OpenOCD 并监听 3333 端口。它会一直运行直到你手动结束 (Ctrl+C)。
```bash
make CHIP=at32f4 debug-server
```

**终端 2: 启动 GDB Client**
此命令会自动连接到 localhost:3333，复位芯片，加载符号，并停在 `main` 函数。
```bash
make CHIP=at32f4 debug
```

### 调试命令速查 (GDB)

进入 GDB 界面后，你可以使用以下命令：

- `c` (continue): 继续运行
- `n` (next): 单步执行 (不进入函数)
- `s` (step): 单步执行 (进入函数)
- `b <func>`: 设置断点 (例: `b led_blink_task`)
- `p <var>`: 打印变量 (例: `p s_tLedBlink`)
- `x/10w 0x...`: 查看内存
- `Ctrl + C`: 暂停运行
- `q`: 退出 GDB

### 编写自动化调试脚本

你可以编写 `.gdb` 脚本来自动化测试流程。例如 `verify_led.gdb`:

```gdb
# 连接并复位
target remote :3333
monitor reset halt
load

# 设置断点
break led_blink_task
break blm_port_LedSet

# 运行并观察
continue
# (Hit led_blink_task)
info registers pc

continue
# (Hit blm_port_LedSet)
print/x $r0  # 查看 LED 状态参数

quit
```

运行脚本：
```bash
gdb-multiarch -batch -x verify_led.gdb build/at32f4/blm.elf
```

## � RTT 虚拟串口 (SEGGER RTT)

本项目集成了 [SEGGER RTT](https://www.segger.com/products/debug-probes/j-link/technology/about-real-time-transfer/)，通过 SWD 调试接口实现高速日志输出，**无需额外串口线**。

### 原理

RTT 在 RAM 中维护一个环形缓冲区（控制块 `_SEGGER_RTT`），固件通过 `LOG_OUT()` 宏写入数据，OpenOCD 通过调试接口读取并转发到 TCP 端口。

```
LOG_OUT(...) → snprintf → SEGGER_RTT_WriteString() → RAM 环形缓冲区
                                                          ↓
                                            OpenOCD RTT → TCP:9090 → nc / telnet
```

### 使用方法（三终端工作流）

**终端 1：启动 OpenOCD + RTT Server**
```bash
make rtt                  # 自动提取 RTT 地址，启动 RTT server (端口 9090)
```

**终端 2：查看 RTT 输出**
```bash
nc localhost 9090         # 持续显示 RTT 打印
```

**终端 3：编译 + 烧录（每次修改代码后）**
```bash
make CHIP=at32f4          # 编译
make flash-rtt            # 通过已运行的 OpenOCD 烧录，自动重启 RTT
```

### Makefile 命令

| 命令 | 说明 |
| :--- | :--- |
| `make rtt` | 启动 OpenOCD + RTT server（端口 9090），自动从 map 文件提取控制块地址 |
| `make flash-rtt` | 通过已运行的 OpenOCD 烧录固件并自动重启 RTT 轮询 |
| `make rtt-addr` | 仅打印当前 RTT 控制块地址（调试用） |

### 代码中使用

```c
#include "utilities/util_debug.h"

// 初始化（main 函数中调用一次）
TRACE.Init(NULL);

// 日志输出
LOG_OUT("Hello RTT!\r\n");

// 支持多种类型
uint32_t val = 42;
LOG_OUT("value = ");
LOG_OUT(val);
LOG_OUT("\r\n");
```

### ⚠️ 注意事项

1. **每次 `make flash-rtt` 后**，RTT 会自动重启。如果手动烧录（`make flash`），需要重启 OpenOCD。
2. **RTT 控制块地址会随编译变化**，`make rtt` 每次自动提取最新地址，无需手动查找。
3. **WSL2 环境**需先通过 `usbipd attach --wsl` 将调试器透传到 WSL。

## 📖 移植指南 (Porting)

若要支持新芯片：

1.  在 `port/` 下新建目录 `port/your_chip/`。
2.  实现 `blm_port_your_chip.c` (参考 `port/blm_port_template.c`)。
3.  提供 `startup_your_chip.c` 和 `linker.ld`。
4.  在 `cmsis/` 下添加外设头文件 `your_chip.h`。
5.  修改 `makefile` 添加新的 `CHIP` 分支。
