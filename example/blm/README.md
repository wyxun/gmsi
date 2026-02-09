# BLM (Bootloader Manager)

BLM 是一个基于 [GMSI](https://github.com/GorgonMeducer/Generic-Microcontroller-Software-Infrastructure) 和 [perf_counter](https://github.com/GorgonMeducer/perf_counter) 的轻量级 Cortex-M Bootloader 示例。它实现了通过 Ymodem 协议进行串口固件升级的功能，并且代码体积极小（< 8KB）。

## 🔥 特性

- **极简设计**: 核心代码量少，依赖清晰。
- **高性能**: 集成 `perf_counter` 提供高精度计时与状态机调度。
- **协议支持**: 标准 Ymodem 协议，支持常见的串口工具（SecureCRT, TeraTerm, Xshell）。
- **共享机制**: 通过 `gblinfo` (0x08002000) 与 APP 共享版本信息和升级标志。
- **工具链友善**: 支持 LLVM Embedded Toolchain for Arm (Clang) 和 GCC。
- **易于移植**: 硬件抽象层 (Port Layer) 分离，仅需实现 UART 和 Flash 接口。

## 📂 目录结构

```
example/blm/
├── core/               # 核心逻辑
│   ├── blm.c           # 主状态机 (App 跳转/升级流程)
│   └── blm_protocol.c  # Ymodem 协议解析
├── port/               # 移植层
│   ├── blm_port_template.c    # 移植模板
│   └── blm_port_stm32g431.c   # STM32G431 参考实现
├── cmsis/              # CMSIS 头文件 (精简版)
├── doc/                # 文档 (设计与验证)
├── main.c              # 入口函数
├── makefile            # 编译脚本
├── linker.ld           # 链接脚本
├── perfc_port_user.c   # perf_counter 移植
└── startup_*.c         # 启动文件
```

## 🚀 快速开始 (STM32G431)

本项目默认提供了 **STM32G431** 的完整移植。

### 1. 硬件连接
- **UART**: USART2 (PA2-TX, PA3-RX), 115200 bps
- **Button**: PA0 (高电平有效，上电按住进入升级模式)
- **LED**: PC13 (低电平点亮)

### 2. 编译与烧录

确保已安装 `llvm_for_arm` 或 `arm-none-eabi-gcc`。

```bash
# 编译
make

# 烧录 (需要连接 pwlink/CMSIS-DAP)
make flash

# 查看代码大小
make size
```

### 3. 使用方法
1.  **正常启动**: 复位后 LED 以 1Hz 闪烁，串口输出 Bootloader Banner，随后跳转到 App (若存在)。
2.  **强制升级**: 按住 PA0 键复位，LED 快闪/常亮，串口输出 'C' 字符请求文件。
3.  **发送固件**: 在串口终端选择 "Send Ymodem"，选择你的 App `.bin` 文件发送。

## 🛠️ 移植教程 (Porting Guide)

若要移植到其他芯片（如 AT32, GD32 等），请按照以下步骤操作：

### 第一步：准备文件
1.  复制 `port/blm_port_template.c` 为 `port/blm_port_your_mcu.c`。
2.  准备你的芯片的 CMSIS 头文件（或使用精简版）。
3.  准备启动文件 `startup_your_mcu.c` (或汇编 `.s`)。

### 第二步：实现硬件接口
在 `blm_port_your_mcu.c` 中实现以下函数：

- **`blm_platform_init()`**: 初始化时钟、GPIO、UART。
- **`blm_uart_config()`**: 配置波特率 (115200)。
- **`blm_uart_write_byte()`**: 发送一个字节（轮询模式）。
- **`blm_uart_read_byte()`**: 读取一个字节（非阻塞，无数据返回 -1）。
- **`blm_flash_erase_page()`**: 擦除指定页。
- **`blm_flash_write_word()`**: 写入数据。
- **`blm_jump_to_app()`**: 跳转到 App 地址 (通常是 0x08000000 + Bootloader Size)。

### 第三步：修改配置
1.  **makefile**:
    -   修改 `TARGET_TRIPLE` (如 `armv7m-none-eabi`)。
    -   修改 `CPU_FLAGS` (如 `-mcpu=cortex-m3`)。
    -   将 `blm_port_stm32g431.c` 替换为你的移植文件。
2.  **linker.ld**:
    -   调整 `FLASH` 和 `RAM` 的大小与起始地址。
    -   确保 `SHARED` 区域 (gblinfo) 地址与 App 一致。

### 第四步：调整 perf_counter
修改 `perfc_port_user.c`:
- 确保 `SysTick_Handler` 被正确调用。
- 确认 `SystemCoreClock` 设置正确。

## 🐛 调试 (Debugging)

推荐使用 VSCode + Cortex-Debug 或 OpenOCD。

1.  **启动 GDB Server**: `make debug-server`
2.  **VSCode**: 使用 `.vscode/launch.json` 中的 "Debug BLM (OpenOCD)" 配置启动调试。

## 📦 资源占用
- **Flash**: ~3.2 KB (基于 LLVM -Oz 优化)
- **RAM**: ~1 KB (主要用于 Ymodem 缓冲)
