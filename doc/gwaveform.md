# GMSI 实时波形采集 (gwaveform) 指南

`gwaveform` 模块是一个轻量级、高性能的实时数据可视化方案，专为高频嵌入式控制系统（如 FOC 电机控制、数控电源）设计。它通过 RTT 通道将 MCU 内部变量以二进制流的形式发送到上位机，并提供图形化显示。

---

## 1. 核心设计原理

### 分层架构
1. **`gringbuf` (底层支撑)**：基于 SPSC (Single Producer Single Consumer) 设计的高性能无锁环形缓冲区。通过 `volatile` 屏障和 2^n 长度掩码优化，确保在 ISR (中断服务程序) 中写入时无需关中断，性能极高且无竞态风险。
2. **`gwaveform` (业务层)**：负责数据帧打包、多通道掩码管理以及 RTT 搬运。支持动态描述帧发送，方便上位机自动重连与通道识别。
3. **`SuperWaveform` (Host 层 - 推荐)**：基于 C++/ImGui 开发的高性能可视化工具，支持实时测量、离线分析与数据录制。
4. **`viewer.py` (Host 层 - 备选)**：基于 Python/PyQtGraph 的轻量级查看器。

### 二进制协议
协议采用精简的二进制格式以节省带宽：
- **数据帧**：同步头 (2B) + 序列号 (1B) + 通道位图 (Variable) + 数据段 (int16_t) + CRC8 (1B)。
- **描述帧**：在启动时或 `wave start` 时发送，包含通道名称及物理量缩放系数 (Scale)，使上位机能自动识别通道含义。

---

## 2. 配置与启用

在项目的 `userconfig.h` 中进行如下配置：

| 宏 | 说明 | 默认值 |
|:---|:---|:---|
| `GWAVEFORM_ENABLE` | 总开关 (0/1) | 0 |
| `GWAVEFORM_MAX_CHANNELS` | 最大支持通道数 | 8 |
| `GWAVEFORM_RING_BUFFER_SIZE` | 内部缓存大小 (2^n) | 256 |
| `GWAVEFORM_DECIMATION` | 抽取比 (每 N 次 Commit 发送一次) | 10 |
| `GWAVEFORM_RTT_CHANNEL` | 使用的 RTT 通道号 | 1 |

---

## 3. MCU 侧用法

### 3.1 初始化与注册
`gwaveform` 利用 `INIT_SECTION` 段实现了自动发现，无需手动调用 `Init`。你只需在程序启动阶段注册通道：

```c
static uint8_t s_chIdU, s_chIdV;

void app_init(void) {
    /* 注册通道：名称最多 7 字节，缩放系数用于 float -> int16 转换 */
    s_chIdU = gwaveform_AddChannel("U_Phase", 100.0f); // 精度 0.01
    s_chIdV = gwaveform_AddChannel("V_Phase", 100.0f);
    
    /* 启动流控 */
    gwaveform_Start();
}
```

### 3.2 采样采集 (在 ISR 中)
在 PWM 或 ADC 中断等高频上下文中调用：

```c
void PWM_IRQHandler(void) {
    // ... 算法计算 ...
    
    gwaveform_Push(s_chIdU, fValU);
    gwaveform_Push(s_chIdV, fValV);
    
    /* 提交当前帧（内部会进行抽取比计数） */
    gwaveform_Commit();
}
```

---

## 4. 调试指令 (gshell)

如果同时启用了 `gshell`，可以使用以下命令进行运行时控制：

- `wave start`：开始流式传输并发送描述信息。
- `wave stop`：停止传输。
- `wave rate <n>`：运行时修改抽取比（例如 `wave rate 1` 为全速率下载）。
- `wave list`：列出当前已注册的所有通道。

---

## 5. 上位机工具

### 环境准备

根据您的开发环境选择以下一种方式安装依赖。`viewer.py` 需要 `numpy`, `pyqtgraph` 和一个 Qt 后端（`PySide6` 或 `PyQt6`）。

#### 方法 A: 标准 Python (推荐使用虚拟环境)
这是最通用的方法，适用于 Windows/Linux/macOS：
```bash
cd tools/gwaveform
# 创建并激活虚拟环境 (可选但推荐)
python -m venv venv
# Windows: venv\Scripts\activate  |  Linux/macOS: source venv/bin/activate

# 安装依赖
pip install -r requirements.txt
```

#### 方法 B: MSYS2 MINGW64 (推荐给 MSYS2 用户)
如果您在 Windows 上使用 MSYS2 并在 MINGW64 环境下工作，使用 `pacman` 安装预编译包最稳定：
```bash
pacman -S mingw-w64-x86_64-python-numpy \
          mingw-w64-x86_64-python-pyqt6 \
          mingw-w64-x86_64-python-pyqtgraph
```

#### 方法 C: Anaconda / Miniconda
```bash
conda install numpy pyqtgraph pyside6
```

### 运行
1. 确保 RTT 服务器（如 J-Link RTT Server 或 OpenOCD）已启动并在 `9091` 端口监听。
2. 运行查看器：
   ```bash
   python viewer.py --host localhost --port 9091
   ```

---

## 6. 开发者说明

- **同步机制**：`gwaveform_Poll()` 已挂载在 `gmsi_Run()` 中自动运行。
- **性能优化**：`Push` 操作仅涉及一次乘法和一次内存写入；`Commit` 在达到抽取比前仅执行一次计数自增，性能极高。

---

## 7. 高级分析工具: SuperWaveform (C++)

为了支持高频率、多通道的数据分析，GMSI 配套了基于 C++/ImGui 开发的 `SuperWaveform` 工具。

### 核心特性
1. **高性能交互**：相比 Python 版本，C++ 版支持更平滑的波形滚动和更低的 CPU 占用。
2. **Space 测量功能**：
   - 按下 `Space` 键记录参考点。
   - 实时显示当前鼠标点与参考点的 **Delta X (时间)**、**Delta Y (幅值)** 及 **Frequency (频率)**。
   - 虚线十字准心，提供非侵入式的视觉反馈。
3. **数据录制与回放**：
   - 录制文件名格式为 `rec_YYYYMMDD_HHMMSS.csv`，方便数据管理。
   - 支持 **多窗口离线查看器**，不同窗口自动应用独特颜色，方便横向对比。
4. **自适应同步**：
   - 内部集成虚拟时钟平滑算法（LPF），有效消除由于网络延迟（RTT）导致的波形抖动。

### 使用方法
1. 进入 `tools/superwaveform` 目录。
2. 执行 `make` 编译（需 MinGW64 环境）。
3. 运行 `./superwaveform.exe`，程序将自动尝试连接到本地 RTT 服务器。
