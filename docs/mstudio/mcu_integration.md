# 下位机集成指南

本指南介绍如何在您的嵌入式固件中集成 `mwaveform` 组件，以便将实时数据推送到 MStudio 上位机。

## 1. 软件依赖

在您的工程中，需要包含以下 MODUS 组件：
- `modus/mdebug/mwaveform.c/h`：波形协议核心。
- `modus/mdebug/mwaveform_protocol.c/h`：波形协议编解码（帧同步、校验）。
- `modus/mdebug/segger_rtt/`：底层 RTT 传输通道。
- `modus/utilities/mringbuf.c/h`：环形缓冲区支持。

## 2. 快速集成示例

以下是一个典型的集成流程，建议在 1ms 频率的定时器中断中推送数据。

### 初始化阶段

在系统启动时，配置 RTT 通道并定义波形参数：

```c
#include "mwaveform.h"

void board_init(void) {
    // 1. 初始化波形组件
    // 它会自动配置 SEGGER_RTT 的 Channel 1 作为数据上传通道
    mwaveform_Init();

    // 2. 注册波形通道
    // 参数 1: 通道名称 (最多 8 字符)
    // 参数 2: 缩放因子 (Scale)。上位机显示值 = 下位机原始值 / Scale
    // 示例：如果推送电压，单位为 mV，希望显示 V，则 Scale 设为 1000.0f
    mwaveform_AddChannel("Sine", 100.0f);
    mwaveform_AddChannel("Cos", 100.0f);
    mwaveform_AddChannel("Voltage", 1000.0f);
}
```

### 数据推送阶段

在您的控制回路或定时器（如 `SysTick_Handler`）中调用：

```c
void SysTick_Handler(void) {
    // 模拟一些数据
    static int16_t angle = 0;
    int16_t sine_val = sin_table[angle];
    int16_t cos_val = cos_table[angle];

    // 3. 压入原始数据 (按注册顺序，索引从 0 开始)
    mwaveform_PushRaw(0, sine_val);
    mwaveform_PushRaw(1, cos_val);
    
    // 4. 提交当前帧
    // 该操作会将当前这一时刻的所有通道数据打包并写入 RTT 缓冲区
    mwaveform_Commit();

    angle = (angle + 1) % 360;
}
```

## 3. 调试器与端口配置

MStudio 通过 TCP 端口与 OpenOCD/JLink 通信，请确保您的调试器脚本已开启 RTT 服务。

### OpenOCD 配置参考
在您的 OpenOCD 配置文件或启动命令中，应包含：
- **9090 端口**：映射到 RTT Channel 0 (Shell)
- **9091 端口**：映射到 RTT Channel 1 (Waveform)

例如在 `makefile` 中的配置：
```makefile
OPENOCD_CMD += -c "rtt server start 9090 0" -c "rtt server start 9091 1"
```

## 4. 协议格式简述 (可选)
如果您希望通过串口或其他自定义通道传输数据，协议帧格式如下：
- **同步头**：`0xAA 0x55`
- **帧序号**：1 字节 (Seq)
- **掩码**：N 字节 (标志哪些通道有数据)
- **数据**：2 字节 * 通道数 (小端)
- **校验**：1 字节 (CRC8)
- **详情参考**：`mwaveform.c` 中的 `_mwaveform_SendFrame` 实现。
