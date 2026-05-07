# mwaveform — MODUS 实时波形采集与可视化模块设计

## 目标

在 MODUS 库的 `utilities` 层新增 `mwaveform` 模块，为外部项目提供实时波形数据采集与传输服务。典型场景：FOC 电机控制中对三相电压、电流、角度等高频信号的实时可视化调试。

模块遵循 MODUS 一贯原则：
- 编译期可完全关闭（零代码、零 RAM 开销）
- 仅提供服务 API，不耦合具体业务逻辑
- ISR 安全，主循环负责 RTT 搬运
- 代码量极小，不引入臃肿依赖

---

## 模块定位

```
modus/utilities/
├── mshell.h / mshell.c        ← 已有，调试 shell
├── mwaveform.h / mwaveform.c  ← 新增，波形采集服务
└── ...

tools/mwaveform/               ← Host 侧工具（不参与 MCU 编译）
├── viewer.py                  ← pyqtgraph 示波器主程序
└── protocol.py                ← 二进制帧解析
```

`mwaveform` 与 `mshell` 相互独立，均可单独开关。若两者同时启用，mshell 会自动注册 `wave` 命令用于运行时控制。

---

## 配置项（userconfig.h）

| 宏 | 默认值 | 说明 |
|----|--------|------|
| `MWAVEFORM_ENABLE` | `0` | 总开关，0=完全关闭 |
| `MWAVEFORM_MAX_CHANNELS` | `16` | 最大通道数（影响静态数组大小） |
| `MWAVEFORM_RTT_BUFFER_SIZE` | `512` | 字节，RTT up-buffer |
| `MWAVEFORM_RTT_CHANNEL` | `1` | RTT 通道号（0 留给 mshell/terminal） |
| `MWAVEFORM_DECIMATION` | `1` | 抽取比，ISR 每 N 次 Step 才实际发送一帧 |

当 `MWAVEFORM_ENABLE=0` 时，所有 API 宏展开为空语句，编译器完全裁剪。

---

## 公开 API

### 初始化与生命周期

```c
// 初始化模块，注册 RTT up-buffer，清空状态
// 在 modus_Init() 流程中自动调用（通过 init_infos 段），无需手动调用
void mwaveform_Init(void);

// 主循环轮询：将 ring buffer 中的帧搬运到 RTT
// 在 modus_Run() 或 mshell_Poll() 后调用，或由 modus 框架自动调度
void mwaveform_Poll(void);

// 开始/停止流式传输（可由 mshell wave 命令调用，也可由业务代码调用）
void mwaveform.Start(void);
void mwaveform.Stop(void);
```

### 通道注册

```c
// 注册一个波形通道，返回通道 ID（0-based）
uint8_t mwaveform.AddChannel(const char *pchName, float fScale);
```

通道注册应在初始化之后、第一次 `Step()` 之前完成。

### ISR 侧采样（在 FOC 中断中调用）

```c
// 写入一个浮点采样值（内部自动 × fScale 转为 int16）
void mwaveform.Push(uint8_t chID, float fValue);

// 写入原始 int16 值
void mwaveform.PushRaw(uint8_t chID, int16_t hwValue);

// 触发采样（在本 ISR 周期所有 Push 完成后调用一次）
void mwaveform.Step(void);
```

### 诊断与控制

```c
// 设置速率：0=外部驱动，n=内部 1/n kHz 驱动
void mwaveform.SetRate(uint32_t wHz);

// 获取丢帧统计
uint32_t mwaveform.GetDropCount(void);
void mwaveform.ClearDropCount(void);
```

### 典型调用示例（外部 FOC 项目）

```c
// 项目初始化阶段
static uint8_t s_chUa, s_chUb, s_chUc, s_chIa, s_chIb, s_chIc, s_chTheta;

void foc_Init(void) {
    s_chUa    = mwaveform.AddChannel("Ua",    100.0f);  // 单位 0.01V
    s_chUb    = mwaveform.AddChannel("Ub",    100.0f);
    s_chUc    = mwaveform.AddChannel("Uc",    100.0f);
    s_chIa    = mwaveform.AddChannel("Ia",   1000.0f);  // 单位 0.001A
    s_chIb    = mwaveform.AddChannel("Ib",   1000.0f);
    s_chIc    = mwaveform.AddChannel("Ic",   1000.0f);
    s_chTheta = mwaveform.AddChannel("Theta", 100.0f);  // 单位 0.01rad
    mwaveform.Start();
}

// FOC PWM/ADC 中断（10~20kHz）
void PWM_IRQHandler(void) {
    // ... FOC 计算 ...
    mwaveform.Push(s_chUa, fUa);
    mwaveform.Push(s_chUb, fUb);
    mwaveform.Push(s_chUc, fUc);
    mwaveform.Push(s_chIa, fIa);
    mwaveform.Push(s_chIb, fIb);
    mwaveform.Push(s_chIc, fIc);
    mwaveform.Push(s_chTheta, fTheta);
    mwaveform.Step();  
}
```

---

## 内部架构

### 数据流

```
FOC ISR (10~20kHz)
  mwaveform_Push() × N
  mwaveform_Commit()
       │  抽取计数
       │  打包帧到 frame_buf[]
       ▼
  ring_buffer[] (RAM, lock-free SPSC)
       │
       │  主循环 mwaveform_Poll()
       ▼
  SEGGER_RTT_Write(channel 1)
       │
       │  OpenOCD RTT TCP server
       ▼
  TCP:9091
       │
       ▼
  tools/mwaveform/viewer.py (Host)
```

- ISR 为唯一写入方，主循环为唯一读取方
- 采用 **Ping-Pong 双缓冲** 配合 **wWriteCount / wReadCount** 锁无关计数器进行握手
- 这种方式比环形缓冲区更节省内存，且天然支持“最新值优先”策略
- 当 Poll 速度慢于 Step 时，Step 会增加 `wDropCount` 并覆盖旧帧，确保 Poll 读到的是当前最实时的数据

### 帧格式（二进制，小端）

**数据帧（Data Frame）：**

| 偏移 | 大小 | 字段 | 说明 |
|------|------|------|------|
| 0 | 1 | `SYNC_H` = 0xAA | 同步头 |
| 1 | 1 | `SYNC_L` = 0x55 | 同步头 |
| 2 | 1 | `seq` | 滚动序列号，Host 侧检测丢帧 |
| 3 | 1 | `ch_mask` | 通道位图（bit N=1 表示通道 N 有数据） |
| 4 | 2×N | `values[N]` | 各活跃通道的 int16 值，按通道 ID 升序 |
| 4+2N | 1 | `crc8` | XOR 校验（覆盖 seq 到 values 末尾） |

7 通道示例帧大小：2+1+1+14+1 = **19 字节**

**描述帧（Descriptor Frame，启动时发送一次）：**

| 偏移 | 大小 | 字段 | 说明 |
|------|------|------|------|
| 0 | 1 | `SYNC_H` = 0xAA | |
| 1 | 1 | `SYNC_L` = 0x55 | |
| 2 | 1 | `TYPE` = 0xFD | 描述帧标识 |
| 3 | 1 | `ch_count` | 通道总数 |
| 4 | 12×N | `ch_desc[N]` | 每通道：name(7B)+'\0'(1B)+scale_f32(4B) |
| last | 1 | `crc8` | |

Host 收到描述帧后即可知道各通道名称和缩放系数，用于坐标轴标注。

---

## mshell 集成（可选）

当 `MWAVEFORM_ENABLE=1` 且 `MSHELL_ENABLE=1` 时，自动注册 `wave` 命令：

| 命令 | 说明 |
|------|------|
| `wave start` | 开始流式传输 |
| `wave stop` | 停止流式传输 |
| `wave rate <n>` | 运行时修改抽取比（0=外部驱动，n=内部 1/n kHz 驱动） |
| `wave drop` | 显示丢帧数及丢帧率 |
| `wave drop clear` | 清零统计计数 |
| `wave list` | 列出已注册通道及 ID |

实现方式：在 `mwaveform.c` 中用 `MODUS_SHELL_CMD()` 宏注册，与 mshell 零耦合。

---

## Host 工具（tools/mwaveform/）

### 依赖

```
pip install pyqtgraph pyserial
```

### 启动方式

```bash
# 默认连接 OpenOCD RTT TCP:9091
python tools/mwaveform/viewer.py

# 指定地址和端口
python tools/mwaveform/viewer.py --host localhost --port 9091
```

### 功能

- 实时多通道波形显示（pyqtgraph，OpenGL 加速）
- 鼠标滚轮缩放 / 拖拽平移（时间轴和幅值轴独立）
- 通道显示开关（点击图例）
- 序列号丢帧检测与提示
- 描述帧自动解析通道名称和单位

### protocol.py 职责

- 从 TCP 字节流中同步帧边界（搜索 0xAA 0x55）
- 解析描述帧和数据帧
- CRC8 校验，丢弃损坏帧
- 输出标准化的 `(timestamp, {ch_name: float_value})` 数据结构供 viewer 消费

---

## 内存估算

| 项目 | 大小 |
|------|------|
| `mwaveform_cb_t` 结构体（16通道） | ~500 字节 RAM |
| 乒乓帧缓冲 | ~80 字节 RAM |
| RTT up-buffer（默认） | 512 字节 RAM |
| 代码段（估算） | ~600 字节 Flash |
| **合计** | **~1.1 KB RAM + ~600 字节 Flash** |

`MWAVEFORM_ENABLE=0` 时以上全部为零。

---

## 与现有模块的关系

- `mshell`：独立，可选集成 `wave` 命令
- `util_debug`（MLOG/MLOGF）：无依赖，两者使用不同 RTT 通道
- `modus_Run()`：`mwaveform_Poll()` 可挂入主循环，也可由用户手动调用
- `segger_rtt`：直接依赖，调用 `SEGGER_RTT_Write()` 和 `SEGGER_RTT_AllocUpBuffer()`
