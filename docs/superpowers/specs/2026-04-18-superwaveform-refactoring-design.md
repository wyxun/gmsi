# GMSI & SuperWaveform 架构重构与功能设计方案

**日期**: 2026-04-18
**主题**: 框架解耦、协议抽象与上位机体验升级

## 1. 背景与目标
随着 GMSI 框架在项目中的深入应用，原本混杂在 `utilities` 中的调试工具（如 gwaveform, gshell 等）与纯粹的数据结构发生了职责耦合；同时，SuperWaveform 上位机在源码层面仍依赖 GMSI 目录，且缺乏离线数据分析及灵活交互的能力。
本次设计目标：实现纯粹的组件解耦、协议的抽象可替换化，并为上位机注入录制、导出及过滤等专业级功能。

## 2. 架构设计：GMSI 目录重构
现有 `utilities` 目录将严格收敛为“**通用数据结构与算法**”。
新增 `gdebug` 目录作为“**调试与可视化子系统**”。

- **gdebug/**:
  - `gshell.c/h`, `gwaveform.c/h`
  - `trace.c/h`, `trace_fmt.c/h`, `util_debug.c/h`
  - `segger_rtt/`
  - 新增 `gwaveform_protocol.h` (协议接口定义)
- **utilities/**:
  - `gringbuf.c/h`, `util_queue.c/h`, `list.c/h`, `utilities.h`

## 3. 设计方案：协议抽象与对象化控制

### 3.1 协议抽象 (方式 A)
采用接口类（函数指针）方式抽象协议。这允许用户完全自定义数据链路的封装格式。
```c
// gdebug/gwaveform_protocol.h
typedef struct {
    uint16_t (*pack_data)(uint8_t *buffer, const int16_t *samples, const uint8_t *mask, uint8_t count);
    uint16_t (*pack_desc)(uint8_t *buffer, const gwaveform_ch_desc_t *channels, uint8_t count);
    // ...
} gwaveform_protocol_t;

extern const gwaveform_protocol_t default_waveform_protocol;
```

### 3.2 面向对象化控制 (gwaveform)
移除 `INIT_SECTION` 的隐式自动注册，采用显式接口调用。
对外暴露统一的 API 对象：
```c
typedef struct {
    int  (*Init)(const gwaveform_protocol_t *protocol);
    void (*Step)(void);
    uint8_t (*AddChannel)(const char *name, float scale);
    void (*Push)(uint8_t chID, float value);
    void (*Start)(void);
    void (*Stop)(void);
} gwaveform_api_t;

extern const gwaveform_api_t gwaveform;
```
**弱函数调度机制**：在 `gmsi_Clock()` (1ms) 中调用弱函数 `gwaveform_Default_Step_Callback()`。默认该回调执行 `gwaveform.Step()`。若用户需在例如 10kHz PWM 中断中调度，只需重写该回调为空，并在中断中调用 `gwaveform.Step()`。

## 4. 上位机 (SuperWaveform) 功能设计

### 4.1 源码层级隔离
在 `superwaveform/src/` 下新建 `utils/` 目录，将 `gringbuf.c/h` 完整复制过去，彻底删除 Makefile 中对 `../../gmsi/utilities` 的依赖。

### 4.2 UI 与交互升级
1. **Time Window (时间窗口)**:
   - 动态调节限制：最小值为 20ms（背景划分 20 个 Grid，1ms/格），上限设定为 60s（基于内存缓冲区 depth 评估具体上限以保障流畅度）。
   - **交互覆盖**: 拦截 ImPlot 默认的滚轮缩放逻辑。
     - 单独滚动鼠标滚轮 -> 缩放 Y 轴数值。
     - `Ctrl` + 鼠标滚轮 -> 缩放 X 轴时间窗口范围。
2. **GSHELL 过滤器**:
   - 在 `ImGuiWindow` 顶部添加 `Filter` 文本框。
   - 绘图前，对收到的文本行执行 `strstr()`，实现实时过滤。

### 4.3 离线数据支持 (Record & CSV)
- **录制状态机**: 空闲 -> 录制中。录制期间将实时到达的数据按照 `std::vector<WaveDataPoint>` 追加到堆内存中。
- **导出**: 遍历 vector 按照 `Time(ms), Ch0, Ch1...` 格式保存为 `.csv`。
- **导入查看**: 创建独立的 `Offline Viewer` ImGui 视图，利用 ImPlot 加载 CSV 全量数据，用于无设备状态下的故障分析。

## 5. 验收标准
1. `example/blm` 在重构后成功编译运行。
2. 上位机不依赖框架代码独立编译。
3. 可通过用户代码在特定中断里调用 `.Step()` 以改变通信行为。
4. 滚轮与 Ctrl+滚轮 体验分离，CSV 录入导入功能完备且互不干扰。
