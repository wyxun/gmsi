# BLM Bootloader 实现完成

## 实现概览

为modus库实现了一个Cortex-M系列MCU的Bootloader示例，支持Ymodem协议串口固件升级。

## 目录结构

```
modus/
├── modus/
│   ├── mblinfo.h/.c      ✅ 共享信息模块 (新增)
│   └── global_define.h   ✅ 添加MBLINFO_SHARED_ADDR
│
└── example/blm/
    ├── main.c            ✅ 入口(MODUS_DECLARE_OBJECT)
    ├── userconfig.h      ✅ 用户配置
    ├── makefile          ✅ 通用Cortex-M makefile
    ├── linker.ld         ✅ 链接脚本(8KB布局)
    ├── core/
    │   ├── blm.h/.c      ✅ 核心状态机
    │   └── blm_protocol.h/.c ✅ Ymodem协议
    └── port/
        ├── blm_port.h    ✅ 移植接口定义
        └── blm_port_template.c ✅ 移植模板
```

## 模块说明

| 模块 | 位置 | 说明 |
|------|------|------|
| mblinfo | modus/ | 共享信息模块，App/BL都可访问 |
| blm | example/blm/core/ | Bootloader核心状态机 |
| blm_protocol | example/blm/core/ | Ymodem协议实现 |
| blm_port | example/blm/port/ | 硬件抽象层接口 |

## 使用方法

### 1. 移植到目标芯片

编辑 `port/blm_port_template.c`：
- 定义 `USE_STM32` 或 `USE_AT32`
- 实现UART和Flash接口

### 2. 编译

```bash
cd example/blm
make
make check-size  # 验证 < 8KB
```

### 3. 固件端使用mblinfo

```c
MODUS_DECLARE_OBJECT(mblinfo, Gblinfo,
    .wSharedInfoAddr = MBLINFO_SHARED_ADDR,
);

// 读取版本
uint8_t chMajor, chMinor;
mblinfo_GetBlVersion(&chMajor, &chMinor);

// 请求升级
mblinfo_SetUpgradeFlag(1);
NVIC_SystemReset();
```

## STM32G431 实现详情

本示例已为 STM32G431 (Cortex-M4F) 完成完整移植。

### 1. 硬件配置
- **UART**: USART2 (PA2-TX, PA3-RX), 115200bps
- **LED**: PC13 (低电平点亮)
- **Button**: PA0 (高电平有效，按下进入升级模式)
- **Clock**: HSI (16MHz)

### 2. 软件组件
- **Toolchain**: LLVM for Arm (Clang + LLD)
- **Startup**: 纯C语言启动文件 (`startup_stm32g431xx.c`)
- **Port Layer**: 寄存器直接操作 (`port/blm_port_stm32g431.c`)
- **Debug**: OpenOCD (CMSIS-DAP) + GDB Multiarch

### 3. 编译与烧录

**编译命令：**
```bash
make            # 编译
make clean      # 清理
make flash      # 烧录 (需要连接 pwlink)
make debug-server # 启动 OpenOCD 服务
```

**VSCode 调试：**
- 使用 `.vscode/launch.json` 中的 "Debug BLM (OpenOCD)" 配置。
- 需要安装 `gdb-multiarch`。

## 验证步骤

1.  **烧录**：连接硬件，执行 `make flash`。
2.  **LED指示**：复位后，LED (PC13) 应以 1Hz 频率闪烁（表示运行中）。
3.  **串口打印**：连接 USART2 到 PC，波特率 115200，应看到 Banner 信息。
4.  **进入升级**：按住 PA0 键复位，LED 快闪（或常亮），串口输出 'C' 字符请求 Ymodem 发送。
5.  **发送固件**：使用支持 Ymodem 的终端（如 TeraTerm/SecureCRT）发送 App固件。
