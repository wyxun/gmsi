# BLM Porting & Development Guide

本指引包含环境搭建、编译烧录、调试方法以及芯片移植指南。

## 🛠️ 环境搭建 (Environment Setup)

### 1. OpenOCD (ArteryTek 分支)

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

### 2. 工具链 (Compiler)

推荐使用 **LLVM Embedded Toolchain for Arm** (Clang) 或 **Arm GNU Toolchain** (GCC)。
请确保编译器路径已添加到系统 PATH，或者修改 `makefile` 中的 `LLVM_PATH` 变量。

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

本项目提供了便捷的 GDB 调试工作流。
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

- `c`: 继续运行
- `n`: 单步执行 (不进入函数)
- `s`: 单步执行 (进入函数)
- `b <func>`: 设置断点
- `p <var>`: 打印变量
- `Ctrl + C`: 暂停运行
- `q`: 退出 GDB

##  RTT 虚拟串口 (SEGGER RTT)

本项目集成了 [SEGGER RTT](https://www.segger.com/products/debug-probes/j-link/technology/about-real-time-transfer/)，通过 SWD 调试接口实现高速日志输出。

### 原理
```
LOG_OUT(...) → snprintf → SEGGER_RTT_WriteString() → RAM 环形缓冲区
                                                          ↓
                                            OpenOCD RTT → TCP:9090 → nc / telnet
```

### 使用方法

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

## 📖 移植指南 (Porting)

若要支持新芯片：

1.  在 `port/` 下新建目录 `port/your_chip/`。
2.  实现 `blm_port_your_chip.c` (参考 `port/blm_port_template.c`)。
    - 实现 `blm_port_Init`
    - 实现 `blm_port_UartInit`, `blm_port_UartSend`, `blm_port_UartRecv`
    - 实现 `blm_port_FlashUnlock`, `blm_port_FlashErase`, `blm_port_FlashWrite`, `blm_port_FlashLock`
    - 实现 `blm_port_JumpToApp`
    - 实现 `blm_port_GetTickMs`
3.  提供 `startup_your_chip.c` 和 `linker.ld`。
4.  在 `cmsis/` 下添加外设头文件 `your_chip.h`。
5.  修改 `makefile` 添加新的 `CHIP` 分支。
