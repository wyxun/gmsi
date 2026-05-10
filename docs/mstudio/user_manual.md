# MStudio 用户操作手册

## 1. 快速入门
1. 启动 OpenOCD RTT 服务（`.\make.bat rtt` 或手动启动）。
2. 运行 `mstudio.exe`。
3. 如果连接成功，Dashboard 会显示 `[Ch0 Shell: ON]` 和 `[Ch1 Wave: ON]`。

## 2. 界面布局
MStudio 采用可停靠（Dockable）的六面板布局，所有面板可在主窗口内自由拖拽排列：
*   **Dashboard** — 连接状态、采样率、波形控制、CSV 录制、Shell 宏管理。
*   **Waveform** — 实时多通道波形显示与交互式测量。
*   **Shell Terminal** — RTT Ch0 双向 Shell 终端，支持日志过滤。
*   **Registers** — OpenOCD 寄存器查看器（Core/Special/FPU）。
*   **Variables** — ELF 符号变量实时监控。
*   **Map Analyzer** — 链接器 Map 文件内存分析。

## 3. 实时波形操作
*   **Window (滑块)**：调节当前显示的波形时长（0.1s - 60s）。
*   **Pause**：点击暂停波形刷新，此时测量功能依然有效。
*   **Clear**：清除当前所有通道的内存缓冲区。

## 4. 测量功能 (核心)
在任何 Plot 窗口（实时或离线）中：
1. **坐标查看**：鼠标悬停，黄色虚线十字准心跟随，浮窗显示 X (s) 和 Y 值。
2. **差值测量 (Space)**：
   - 移动鼠标到起始点，按下 **Space** 键锁定红色参考线。
   - 移动鼠标，浮窗实时显示 `dX`、`Hz`（1/dX）、`dY`。
   - 再次按下 **Space** 清除参考点。

## 5. 寄存器面板
1. 点击 **Connect to OpenOCD** 连接调试器（默认 127.0.0.1:4444）。
2. 连接成功后自动读取全部寄存器，分三组显示：
   - **Core Registers**: R0-R12, SP, LR, PC, xPSR
   - **Special Registers**: MSP, PSP, PRIMASK, BASEPRI, FAULTMASK, CONTROL
   - **FPU Registers**: D0-D15, FPSCR（仅 STM32G431 等带 FPU 的芯片）
3. 按钮操作：
   - **Halt & Read** — 暂停 CPU 并刷新寄存器
   - **Resume** — 恢复 CPU 运行
   - **Refresh** — 手动刷新
   - **Disconnect** — 断开连接
4. 连接状态下每 1s 自动刷新；值变化时绿色高亮提示。

## 6. 变量面板
1. 在 **ELF** 输入框中输入 ELF 文件路径（如 `build/template.elf`），点击 **Load** 加载符号表。
2. 点击 **Connect OCD** 连接 OpenOCD。
3. 变量表操作：
   - 点击星标列切换收藏（黄色星号）。
   - 点击 **Read** 按钮读取单个变量值。
   - 勾选 **Auto** 后每 1s 自动刷新所有收藏变量。
   - 点击 **Read All** 批量读取全部收藏变量。
   - 使用 **Search** 输入框按名称过滤。
4. 变量值以 Hex 和 Decimal 双格式显示，带符号解析。

## 7. Map 分析器
1. 在路径输入框中输入 `.map` 文件路径（默认 `build/template.map`），点击 **Load**。
2. 显示内容：
   - **Flash/RAM 进度条**：可视化内存占用比例及 KB 数值。
   - **左侧 Section 表**：列出所有段（名称、VMA、Size、Type 分类）。
   - **右侧 File 表**：点击 Section 行查看该段内各 .o 文件的 Top 10 大小排行；未选中时显示全局 Top 10。
   - **底部搜索表**：输入符号名搜索，显示 Symbol、Size、Section、被引用文件列表。

## 8. 离线分析
*   **打开文件**：在 Dashboard 点击 **Open CSV Viewer** 选择 CSV 文件。
*   **缩放**：按住 `Ctrl` 键并滚动鼠标滚轮。
*   **平移**：在波形区域按住鼠标左键并拖拽。
*   **对比**：可同时打开多个 CSV 窗口，自动分配不同标题栏颜色。

## 9. 数据录制
*   **Start CSV**：点击 Dashboard 右侧绿色按钮开始录制。
*   **保存位置**：文件保存在程序运行目录下，名为 `rec_年月日_时分秒.csv`。
*   **停止**：录制时按钮变红，点击 `Stop CSV` 结束录制。

## 10. Shell 宏命令
通过 `.ini` 文件可定义快捷按钮，向 Shell 通道 (Ch0) 发送预设指令。

### 配置文件格式
```ini
# 这是注释
wave start = wave start
wave stop  = wave stop
wave speed = wave rate 4
```

### 操作流程
1. 点击 Dashboard 中的 **Load Macros...** 按钮，选择 `.ini` 文件。
2. 加载成功后，文件名显示在按钮右侧，按钮栏生成对应快捷按钮。
3. 点击按钮即可向 Shell 发送指令（自动追加换行符）。
4. 程序退出时记住上次加载的文件，下次启动自动恢复。
