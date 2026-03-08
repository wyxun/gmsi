# BLM (Bootloader Manager)

BLM 是一个基于 [GMSI](https://github.com/wyxun/gmsi) 和 [perf_counter](https://github.com/GorgonMeducer/perf_counter) 的轻量级 Cortex-M Bootloader。它实现了通过 **Ymodem** 协议进行串口固件升级的功能，代码体积极小（< 16KB）。

> 💡 **开发者注意**：关于环境搭建、编译烧录、调试及芯片移植的详细指南，请移步 [开发与移植指南](doc/porting_guide.md)。

## 🔥 特性

- **极简设计**: 核心代码量少，依赖清晰。
- **协议支持**: 标准 Ymodem 协议 (128字节包)，支持常见的串口工具 (SecureCRT, TeraTerm, Xshell 等)。
- **安全可靠**: 包含 CRC16 校验，支持超时重传与错误处理。
- **共享机制**: 通过 Flash 固定区域与 APP 共享版本信息和升级标志。

## � Flash 分区布局

```
+-------------------+ 0x08000000
|   Bootloader      |  16KB (0x4000)
+-------------------+ 0x08004000
|   Shared Info     |  1KB  (gblinfo)
+-------------------+ 0x08004400
|   Application     |  剩余空间
|                   |
+-------------------+ Flash End
```

| 区域 | 起始地址 | 大小 | 说明 |
|:-----|:---------|:-----|:-----|
| Bootloader | `0x08000000` | 16KB | BLM 固件 |
| Shared Info | `0x08004000` | 1KB | 版本信息 / 升级标志 (gblinfo) |
| Application | `0x08004400` | 视芯片而定 | 用户应用固件 |

> ⚠️ **APP 固件编译时，链接脚本的 FLASH 起始地址必须设为 `0x08004400`**，否则 Bootloader 无法正确跳转。

## 📖 固件升级教程

### 1. 进入 Bootloader 升级模式

Bootloader 启动后按以下优先级决定行为：

```
复位
 ├─ 升级按键按下？ ──────── YES ──→ 进入升级模式
 ├─ APP 设置了升级标志？ ── YES ──→ 进入升级模式（自动清除标志）
 ├─ APP 区域有效？ ──────── YES ──→ 直接跳转 APP
 └─ APP 区域无效 ─────────────────→ 进入升级模式（等待烧写）
```

**三种触发方式**：

| 方式 | 操作 | 适用场景 |
|:-----|:-----|:---------|
| **按键触发** | 按住升级按键（默认 PA0，低电平触发），然后复位 | 手动升级、开发调试 |
| **APP 请求** | APP 中设置 `ptShared->chUpgradeFlag = 1` 并执行 `NVIC_SystemReset()` | 远程 OTA、菜单触发 |
| **无有效固件** | 首次烧写 Bootloader 后，APP 区域为空白 | 初始化烧写 |

**进入升级模式的标志**：串口持续输出字符 **`C`**（约每 1.5 秒一次）。

### 2. WSL 环境下的 USB 设备绑定 (Windows 用户必读)

如果您在 WSL (Windows Subsystem for Linux) 下进行开发，虚拟机默认无法直接访问主机的 USB 物理外设（如下载器）。您需要在 Windows 主机端的 **PowerShell（管理员模式）** 下，通过 `usbipd` 工具将烧录器挂接到 WSL 中。

**绑定流程：**

1. **列出可用设备**：
   在 PowerShell 中运行以下命令，找到您的下载器 (如 AT-Link 或 CMSIS-DAP)：
   ```powershell
   usbipd list
   ```
   > *输出示例*：
   > ```text
   > BUSID  VID:PID    DEVICE                                                        STATE
   > 3-4    2e3c:f000  ATLink-USART (COM14), USB 输入设备, AT-Link-EZ(WinUSB)...     Not attached
   > ```
   > 这里记下游离的对应设备的 **BUSID**（这里是 `3-4`）。

2. **绑定并挂载设备**：
   继续在 PowerShell 中运行下面两条指令（请将 `<设备ID>` 替换为您查到的实际 BUSID, 比如 `3-4`）：
   ```powershell
   # 绑定设备 (第一次使用时需要执行，后续可能不需要)
   usbipd bind --busid <设备ID>

   # 挂载设备到默认的 WSL 发行版中
   usbipd attach --wsl --busid <设备ID>
   ```
   > 挂载成功后，原来状态会变成 `Attached`。此时在您的 WSL Linux 终端中执行 `lsusb` 命令即可看到该对应的调试器设备，随后便可畅跑 `make flash` 与 `make rtt` 了。

### 3. 编译与调试 (Make 快捷命令)

本工程的 `makefile` 中内置了用于烧录和 RTT 调试的快捷命令：

- **烧录 Bootloader**：
  ```bash
  make flash
  ```
  该命令会通过 OpenOCD 和 CMSIS-DAP 将编译好的 `blm.hex` 烧录进芯片。

- **开启 RTT 调试服务**：
  若想通过 SEGGER RTT 查看 Bootloader 运行日志（例如 `BLM Bootloader Started`）：
  1. 在终端中运行：
     ```bash
     make rtt
     ```
     此时 OpenOCD 会查找到 `_SEGGER_RTT` 控制块地址，并开启 RTT Server（默认监听 **9090** 端口）。
  2. **新开一个终端容器/窗口**，使用 `nc` (netcat) 或 `telnet` 命令连接以实时查看日志：
     ```bash
     nc localhost 9090
     # 或者使用: telnet localhost 9090
     ```

### 3. 准备应用固件文件
- 固件必须是 **`.bin` 格式**（原始二进制），不是 `.hex` 或 `.elf`。
- APP 链接脚本中 `FLASH` 起始地址必须设为 `0x08004400`。
- 文件大小不得超过 `BLM_APP_MAX_SIZE`（STM32G431: ~247KB，AT32F407: ~1015KB）。

**生成 .bin 文件示例**（以 LLVM 工具链为例）：
```bash
llvm-objcopy -O binary your_app.elf your_app.bin
```

### 3. 使用串口工具发送固件

#### SecureCRT

1. 连接串口：波特率 **115200**，8N1（8 数据位，无校验，1 停止位）。
2. 等待终端持续显示 `C` 字符。
3. 点击菜单 `Transfer` → `Send Ymodem...`，选择 `.bin` 文件。
4. 等待进度条完成，Bootloader 自动校验并跳转。

#### TeraTerm

1. 连接串口，配置同上。
2. `File` → `Transfer` → `YMODEM` → `Send...`，选择文件。

#### Xshell

1. 连接串口后，等待 `C` 字符出现。
2. 在终端右键 → `传输` → `用 YMODEM 发送...`，选择文件。

#### 命令行 (lrzsz)

```bash
# 安装 lrzsz
sudo apt install lrzsz

# 通过串口发送 (需要在看到 'C' 字符后执行)
sb --ymodem your_app.bin > /dev/ttyUSB0 < /dev/ttyUSB0
```

### 4. 升级过程状态指示

| 阶段 | 串口输出 | LED 状态 | 说明 |
|:-----|:---------|:---------|:-----|
| 等待连接 | 持续打印 `C` | 闪烁 (0.5s/0.5s) | 等待上位机发送固件 |
| 接收数据 | (无输出) | 闪烁 | 正在接收并写入 Flash |
| 校验成功 | (无输出) | - | 自动跳转 APP |
| 超时 | | 闪烁 | 重试 10 次后进入错误状态 |
| 错误 | | - | 等待复位后重试 |

### 5. APP 端触发升级 (代码示例)

在应用程序中通过写入升级标志来触发 Bootloader 升级：

```c
#include "gblinfo.h"

void app_request_upgrade(void)
{
    /* 获取共享信息区指针 */
    gblinfo_shared_t *ptShared = (gblinfo_shared_t *)GBLINFO_SHARED_ADDR;
    
    /* 设置升级标志 */
    ptShared->chUpgradeFlag = 1;
    
    /* 复位，Bootloader 启动后会检测到此标志 */
    NVIC_SystemReset();
}
```

> Bootloader 检测到标志后会自动清除 `chUpgradeFlag`，无需 APP 额外处理。

### 6. 注意事项与故障排查

| 问题 | 可能原因 | 解决方法 |
|:-----|:---------|:---------|
| 看不到 `C` 字符 | 串口未连接 / 波特率不对 / 已跳转 APP | 检查串口配置；按住按键复位强制进入 |
| `C` 只打印一次就停了 | Bootloader 版本过旧，ReceivePacket 缺少超时 | 更新 Bootloader 固件 |
| 传输开始后中断 | 串口线松动 / 电源不稳 | 检查硬件连接 |
| 传输完成但 APP 未运行 | APP 链接地址不是 `0x08004400` | 检查 APP 链接脚本 FLASH 起始地址 |
| 文件太大被拒绝 | 固件超过 `BLM_APP_MAX_SIZE` | 优化 APP 体积或检查 Flash 布局 |
| 按键无法进入升级模式 | 引脚配置不匹配 | 确认按键引脚和有效电平与 port 实现一致 |

## 📡 协议规范 (Protocol Specifications)

本 Bootloader 实现 Ymodem 协议的一个子集，专为嵌入式环境优化。

### 帧格式 (Frame Format)

仅支持 **128字节** 数据帧 (SOH)，不支持 1024字节 扩展帧 (STX)。

```
+-----+-----+-------+------------+---------+
| SOH | SEQ | ~SEQ  | DATA (128) | CRC (2) |
+-----+-----+-------+------------+---------+
```
- **SOH**: 0x01
- **SEQ**: 包序号，从 0x00 (文件信息帧) 开始，数据帧从 0x01 开始。
- **~SEQ**: 包序号按位取反。
- **DATA**: 128字节载荷。不足处填充 0x1A (CPMEOF)。
- **CRC**: 16位 CRC-CCITT (Poly: 0x1021)，大端序。

### 通信流程 (Interaction)

```
Bootloader                          Host (PC)
    |                                   |
    |------- 'C' (0x43) -------------->|  握手：请求 CRC 模式
    |                                   |
    |<------ SOH + Packet0 ------------|  文件信息：文件名 + 大小
    |------- ACK --------------------->|
    |------- 'C' --------------------->|  请求数据
    |                                   |
    |<------ SOH + Packet1 ------------|  数据帧 #1
    |------- ACK --------------------->|
    |<------ SOH + Packet2 ------------|  数据帧 #2
    |------- ACK --------------------->|
    |         ...                       |
    |                                   |
    |<------ EOT ----------------------|  第一次 EOT
    |------- NAK --------------------->|
    |<------ EOT ----------------------|  第二次 EOT
    |------- ACK --------------------->|
    |------- 'C' --------------------->|  请求结束包
    |<------ SOH + Packet0 (empty) ----|  空包，结束传输
    |------- ACK --------------------->|
    |                                   |
    |  [校验 → 跳转 APP]                |
```

### 参数配置 (Configuration)

所有参数在 `userconfig.h` 中定义：

| 参数 | 宏名 | 默认值 | 描述 |
|:-----|:-----|:-------|:-----|
| 波特率 | `BLM_UART_BAUDRATE` | 115200 | 通信波特率 |
| 包大小 | `BLM_PACKET_SIZE` | 128 Bytes | 固定支持 SOH 帧 |
| 包接收超时 | `BLM_PACKET_TIMEOUT_MS` | 1000 ms | 单包接收超时 |
| 连接等待超时 | `BLM_WAIT_TIMEOUT_MS` | 3000 ms | 整体连接等待 |
| 最大重试 | `BLM_MAX_RETRY` | 10 | 错误/超时重试次数 |
| APP 起始地址 | `BLM_APP_ADDR` | 0x08004400 | 应用程序起始地址 |
| APP 最大体积 | `BLM_APP_MAX_SIZE` | 视芯片而定 | 最大可烧写固件大小 |
