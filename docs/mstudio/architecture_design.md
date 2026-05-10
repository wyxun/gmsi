# MStudio 架构设计

MStudio 采用多层异步处理架构，确保在处理高频串行数据流时保持 UI 的响应性。

## 1. 系统层次结构

*   **数据接入层 (NetworkMgr + OcdClient)**：
    *   `NetworkMgr`：独立双线程设计，Ch0 (Shell) 与 Ch1 (Waveform) 互不干扰。基于 Socket 的 TCP 客户端，连接 RTT 转发服务（端口 9090/9091）。
    *   `OcdClient`：同步式 OpenOCD Telnet 客户端（端口 4444），使用非阻塞 socket + select 实现 UI 线程安全。支持 `reg`、`mrw`、`mdw`、`halt`、`resume` 等命令。
*   **协议解析层 (ProtocolParser + ElfParser + MapParser)**：
    *   `ProtocolParser`：轻量级字节状态机，识别帧同步、校验和通道映射。支持动态通道描述帧。
    *   `ElfParser`：ARM ELF 文件符号表解析器，提取全局/静态变量名称、地址、类型、大小。
    *   `MapParser`：GNU linker `.map` 文件解析器，提取 Section 布局、文件贡献量、符号地址、交叉引用表。
*   **业务逻辑层 (GuiLayer + Panels)**：
    *   **虚拟时钟同步 (Adaptive Clocking)**：利用 LPF 平滑包到达时间，解决 RTT 抖动问题。
    *   **Session 管理**：通过 `OfflineSession` 结构支持并行的离线分析窗口。
    *   **六面板架构**：Dashboard、Waveform、Terminal、Registers、Variables、Map Analyzer。
*   **渲染层 (ImGui/ImPlot)**：
    *   **DockSpace 布局**：所有面板停靠在一个无标题栏的全屏 DockSpace 中。
    *   **循环采样渲染**：直接从环形缓冲区 (`ScrollingBuffer`) 映射坐标。
    *   **UI 隔离**：每个 Plot 窗口拥有独立的交互状态（测量位、缩放级）。

## 2. 核心技术解决点

### 2.1 鲁棒的状态机管理 (ImPlot Stability)
为了防止 ImPlot 断言失败，渲染逻辑严格遵循以下序列：
`BeginPlot` -> `SetupAxes` -> `SetupAxisLimits` -> `Interactions` -> `PlotLines` -> `EndPlot`。

### 2.2 非阻塞 OpenOCD 通信
`OcdClient::Connect()` 使用 `ioctlsocket(FIONBIO)` + `select()` 实现 500ms 超时的非阻塞连接，确保 UI 线程不会因网络阻塞而冻结。所有命令发送/接收均带超时保护。

### 2.3 离线查看器的多实例架构
每个离线窗口拥有独立的 `ImGui::ID`。通过 `OfflineSession` 存储文件的采样数据、当前 X 轴显示范围和测量状态，实现了完全隔离的对比分析能力。
