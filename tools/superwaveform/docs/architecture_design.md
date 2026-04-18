# SuperWaveform 架构设计

SuperWaveform 采用多层异步处理架构，确保在处理高频串行数据流时保持 UI 的响应性。

## 1. 系统层次结构

*   **数据接入层 (NetworkMgr)**：
    *   独立双线程设计：Ch0 (Shell) 与 Ch1 (Waveform) 互不干扰。
    *   基于 Socket 的抽象，支持 TCP (RTT) 转发数据流。
*   **协议解析层 (ProtocolParser)**：
    *   轻量级字节状态机，识别帧同步、校验和通道映射。
    *   支持动态通道描述帧，无需重启工具即可识别 MCU 通道变动。
*   **业务逻辑层 (GuiLayer)**：
    *   **虚拟时钟同步 (Adaptive Clocking)**：利用 LPF 平滑包到达时间，解决 RTT 抖动问题。
    *   **Session 管理**：通过 `OfflineSession` 结构支持并行的离线分析窗口。
*   **渲染层 (ImGui/ImPlot)**：
    *   **循环采样渲染**：直接从环形缓冲区 (`ScrollingBuffer`) 映射坐标。
    *   **UI 隔离**：每个 Plot 窗口拥有独立的交互状态（测量位、缩放级）。

## 2. 核心技术解决点

### 2.1 鲁棒的状态机管理 (ImPlot Stability)
为了防止 ImPlot 断言失败，渲染逻辑严格遵循以下序列：
`BeginPlot` -> `SetupAxes` -> `SetupAxisLimits` -> `Interactions` -> `PlotLines` -> `EndPlot`。
通过将交互（如缩放、平移）放置在 `Setup` 之后但 `PlotLines` 之前，确保了每一帧的变换状态是一致的。

### 2.2 离线查看器的多实例架构
每个离线窗口拥有独立的 `ImGui::ID`。通过 `OfflineSession` 存储文件的采样数据、当前 X 轴显示范围和测量状态，实现了完全隔离的对比分析能力。
