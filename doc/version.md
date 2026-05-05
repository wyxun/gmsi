# Modus 版本历史

| 版本 | 日期 | 状态 | 核心变更 |
| :--- | :--- | :--- | :--- |
| **v0.3.0.4** | 2026-05-05 | 稳定 | gmsi.mk 统一构建系统，模块化编译开关 |
| **v0.3.0.3** | 2026-05-03 | 稳定 | 新增 -T 日志类别与 SuperWaveform INI 宏命令管理 |
| **v0.3.0.2** | 2026-04-23 | 稳定 | gwaveform 架构维护：回归 Ping-Pong 及文档补全 |
| **v0.3.0.1** | 2026-04-22 | 稳定 | gwaveform 架构重构：乒乓缓冲与 ISR 安全优化 |
| **v0.3.0.0** | 2026-04-18 | 稳定 | SuperWaveform 重构与测量系统；协议同步 with 稳定性优化 |
| **v0.2.1.0** | 2026-04-14 | 稳定 | 新增 gshell 调试 Shell；GLOG 运行期级别控制；GLOGF 补充 `%f` |
| **v0.2.0.1** | 2026-04-08 | 稳定 | GLOGF 支持 %lu 格式化打印 |
| **v0.2.0.0** | 2026-04-07 | 稳定 | 新增 GStorage 持久化存储模块 |

## [0.3.0.4] - 2026-05-05

### 新增功能
- **gmsi.mk 统一构建系统**:
  - 在仓库根目录新增 `gmsi.mk`，外部项目只需 `include $(GMSI_ROOT)/gmsi.mk` 即可引用 GMSI 框架。
  - 所有可选模块默认关闭（保守策略），外部项目按需通过 Make 变量显式开启。
- **模块化编译开关**:
  - `GSHELL_ENABLE` / `GWAVEFORM_ENABLE`: 控制 gshell、trace、SEGGER_RTT、gwaveform 等调试模块的编译。
  - `GSTORAGE_ENABLE` / `GBLINFO_ENABLE`: 控制 gstorage、gblinfo 功能模块的编译。
  - `GMSI_USE_LOG` / `GMSI_USE_ASSERT`: 控制 GLOG/GLOGF 日志宏和 GMSI_ASSERT 断言宏。
  - 关闭时零开销：对应源码不参与编译，ROM/RAM 完全不占用。
- **BLM 示例双模式构建**:
  - `make`（默认）: `-O0` + 全调试功能开启，支持 F5 一键调试。
  - `make release`: `-Oz` + 所有调试模块剥离，产出精简发布固件。

### 更新与优化
- **gmsi.c**: 在 `gmsi_Run()` 和 `gmsi_Clock()` 中增加了 `#if GSHELL_ENABLE` / `#if GWAVEFORM_ENABLE` 条件编译守卫，关闭时可被链接器垃圾回收。
- **gshell.h**: 增加了 `GSHELL_ENABLE` 编译期开关，当值为 `0` 时 `GMSI_SHELL_CMD` 宏展开为空，shell 命令自动废弃。
- **gwaveform.h**: 当 `GWAVEFORM_ENABLE == 0` 时提供轻量 API 桩结构体，避免调用方编译报错。
- **util_debug.h**: `__NO_USE_LOG__` 定义时 `GLOG`/`GLOGF` 宏展开为真正的空操作 `do {} while(0)`，彻底消除对 TRACE 符号的间接依赖。
- **BLM main.c**: 增加了 `#if GSHELL_ENABLE` / `#if GWAVEFORM_ENABLE` 守卫，保护 TRACE 初始化和波形测试相关调用。


## [0.3.0.3] - 2026-05-03

### 新增功能
- **GMSI 调试系统**:
  - **新增 -T (Timer) 日志类别**: 专门用于周期性/轮询函数中的状态验证。支持通过 `log -T` 独立控制开关，解决了高频定时打印干扰开发视线的问题。
  - **GLOG 架构优化**: 重构了编译期 Gate 逻辑，确保 T 类别在默认 INFO 级别下也可编译包含，且运行时 Mask 依然解耦。
- **SuperWaveform (Host)**:
  - **动态宏命令管理**: 引入 INI 驱动的宏按钮系统，支持用户手写配置文件自定义快捷指令。
  - **配置持久化**: 支持加载外部 INI/TOML 扩展配置，并自动保存/恢复最后一次使用的路径。

## [0.3.0.2] - 2026-04-23

### 更新与优化
- **gwaveform (架构维护)**:
  - **稳定性回归**: 将驱动架构从实验性 FIFO 回归至 **Ping-Pong (最新优先)** 模型，确保高频采样下的低延迟特性。
  - **物理链路优化**: 经验证并保留了 8MHz 的高速 SWD 配置，显著提升 RTT 传输上限。
  - **文档补全**: 完成了 `doc/gdebug/gwaveform.md` 深度指南，包含协议细节说明并归档了无损 FIFO 演进思路。

## [0.3.0.1] - 2026-04-22

### 更新与优化
- **gwaveform (架构级重构)**:
  - **内存优化**: 舍弃了 8KB 的中间 Ring Buffer，改用双缓冲区 **Ping-Pong** 架构。总 RAM 开销从 ~16.5KB 降低至 **~1.1KB**，节省了约 94% 的内存。
  - **ISR 安全**: 实现了基于 `wWriteCount / wReadCount` 锁无关计数器的交接逻辑，确保在 FOC 等高频 ISR 中调用 `Step()` 是非阻塞且 100% 安全的。
  - **丢帧诊断**: 新增丢帧计数与丢帧率统计，支持通过 `wave drop` 实时监控传输健康度。
  - **控制增强**: 引入 `SetRate(0)` 外部驱动模式，支持用户在自定义高频中断（如 20kHz）中直接触发采样。
  - **API 统一**: 接口全面迁移至 `gwaveform.` 命名空间，并废弃了旧的 `gringbuf` 内部依赖。

## [0.3.0.0] - 2026-04-18

### 新增功能
- **SuperWaveform (Host)**: 彻底重构为高性能 C++ 版本，采用 ImGui/ImPlot 渲染引擎。
  - **测量系统**: 新增 `Space` 差值测量功能，支持 dX, dY 及频率实时计算。
  - **录制功能**: 实现本地 CSV 异步录制，支持 `rec_YYYYMMDD_HHMMSS` 自动命名。
  - **多窗口分析**: 支持多实例离线查看器，具备窗口色调自动区分与独立交互。
  - **视觉提升**: 引入蓝黑科技格调 UI 与虚线十字准心绘制。

### 更新与优化
- **架构优化 (Directory Refactoring)**:
  - **组件迁移**: 建立了 `gmsi/gdebug/` 统一调试目录。
  - **文件重组**: 将 `gshell`, `gwaveform`, `util_debug`, `trace` 等调试支撑组件迁移至该目录，使核心框架与调试工具链实现物理隔离，提高了模块化程度。
- **GWaveform**:
  - **同步优化**: 引入自适应虚拟时钟同步算法，消除 RTT 抖动导致的波形锯齿与阶梯感。
  - **协议稳定性**: 修复了通道掩码字节对齐（Mask Bytes Mismatch）导致的严重丢包 Bug。
  - **并发安全与性能**: 
    - 优化 `gringbuf` SPSC 环形缓冲区处理，消除采样中断与 Poll 循环之间的竞态冲突。
    - 通过扩大 RTT 单次读取块（从 128B 提升至 512B）减少了 RTT 操作频率，降低下位机 CPU 负载。
  - **接口升级**: `gshell` 的 `ver` 指令现在可以动态读回 v0.3.0.0 版本信息。

### 已修复
- **稳定性**: 修复了 ImPlot API 调用序列断言失败及 Tooltip 嵌套导致的闪退问题。
- **性能**: 修正了部分调试接口在极端带宽下的阻塞风险。

## [0.2.0.1] - 2026-04-08

### 新增功能

+ **GLOGF**：增加对`%lu`变量打印的支持，可以识别`long`型数值打印

## [0.2.0.0] - 2026-04-07

### 新增功能
- **GStorage**: 增加了持久化存储接口。基于 GDI (GMSI Driver Interface) 抽象层，支持自动 CRC 校验与 Flash 读写操作，实现了 RAM 数据的自动定时同步。

### 更新与优化
- **GLOG**: 更新了日志系统接口。采用了链式参数分发机制，支持更高效的多参数打印，降低了栈开销并移除了对 `snprintf` 的依赖。
- **GMSI 初始化**: 更新了 `gmsi_Init` 以支持全局默认 Flash 设备的注入。

### 已修复
- 修复了 `example/posix_uart` 和 `example/template` 在最新接口下的运行问题。
- 修复了 `gstorage` 的缓冲区溢出隐患。

