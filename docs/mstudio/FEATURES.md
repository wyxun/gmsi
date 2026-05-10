# MStudio 功能全集

MStudio 是一款基于 ImGui + ImPlot + SDL2 开发的 MODUS 统一调试工作台，集成波形分析、Shell 终端、寄存器查看、变量监控与 Map 内存分析六大功能面板。

## 核心功能

### 1. 实时波形监控 (Dashboard + Waveform)
*   **高性能渲染**：支持数万点级别的波形平滑刷新。
*   **自适应时钟**：内置虚拟时钟平滑算法，解决网络抖动导致的波形跳变问题。
*   **时间窗口调节**：支持 0.1s 至 60s 的动态时间窗口缩放（通过 Window 滑块）。
*   **波形锁定**：一键 `Pause` 锁定波形，方便在静态状态下进行精细观察。
*   **交互式测量**：`Space` 锁定参考点，实时显示 dX/dY/Hz 差值。

### 2. Shell 终端 (Terminal)
*   **双向通信**：通过 RTT Ch0 (TCP 9090) 与 MCU mshell 交互。
*   **日志过滤**：支持 `incl,-excl` 模式的关键字过滤。
*   **自动滚动**：可选的自动跟随最新输出。
*   **宏管理**：通过 `.ini` 文件定义快捷指令按钮，支持会话持久化。

### 3. 寄存器面板 (Registers)
*   **OpenOCD Telnet**：通过 TCP 4444 连接 OpenOCD，执行 `reg` 命令读取全部寄存器。
*   **分组显示**：Core (R0-R12/SP/LR/PC/xPSR)、Special (MSP/PSP/PRIMASK/...)、FPU (D0-D15/FPSCR) 三组。
*   **变更高亮**：寄存器值变化时绿色闪烁提示。
*   **自动刷新**：连接后每 1s 自动刷新；支持手动 Halt & Read / Resume 控制。

### 4. 变量面板 (Variables)
*   **ELF 符号解析**：加载 `.elf` 文件，提取全局/静态变量符号表（名称、地址、类型、大小）。
*   **实时读取**：通过 OpenOCD `mrw` 命令按地址读取内存值。
*   **收藏夹**：星标常用变量，支持一键 `Read All` 批量刷新。
*   **搜索过滤**：按变量名模糊搜索。
*   **自动刷新**：可选 1s 间隔自动更新收藏变量。

### 5. Map 分析器 (Map Analyzer)
*   **Map 文件解析**：加载 GNU linker 生成的 `.map` 文件。
*   **Flash/RAM 进度条**：可视化展示 Flash 与 RAM 占用比例。
*   **Section 明细**：左侧表格列出所有段（VMA、Size、Type 分类）。
*   **文件贡献排行**：点击 Section 查看该段内各 .o 文件的 Top 10 大小排行。
*   **符号搜索**：全量符号跨引用搜索，显示符号所在 Section 和引用文件列表。

### 6. 离线分析 (Offline Viewer)
*   **多窗口对比**：支持同时打开多个 CSV 文件，每个窗口独立运行。
*   **窗口调色**：不同离线窗口自动分配不同色系标题栏。
*   **智能缩放与平移**：`Ctrl + 滚轮` 缩放、`左键拖拽` 平移。

### 7. 实验数据录制 (CSV Recording)
*   **一键录制**：`Start CSV` 按钮，文件名自动格式化为 `rec_YYYYMMDD_HHMMSS.csv`。
*   **安全状态反馈**：录制中按钮变红 `Stop CSV`。

## 操作速查

| 操作 | 功能 |
| :--- | :--- |
| **Space** (在波形上) | 开启/关闭 差值测量 |
| **Ctrl + Scroll** | X 轴缩放（离线模式） |
| **Left Drag** | 波形平移（离线模式） |
| **Pause / Resume** | 锁定/恢复 实时波形 |
| **Clear** | 清空实时缓存 |

## 技术特性
*   **蓝黑科技格调**：深度定制的 ImGui Premium Dark 主题。
*   **非阻塞 I/O**：OpenOCD Telnet 客户端使用 select + 非阻塞 socket，不冻结 UI。
*   **鲁棒性**：严格的 ImPlot API 调用序列管理，杜绝状态机断言崩溃。
*   **跨平台基础**：基于 SDL2 + OpenGL3，兼容 Windows/Linux。
