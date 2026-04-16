# gwaveform — GMSI 实时波形采集与可视化模块设计

## 目标

在 GMSI 库的 `utilities` 层新增 `gwaveform` 模块，为外部项目提供实时波形数据采集与传输服务。典型场景：FOC 电机控制中对三相电压、电流、角度等高频信号的实时可视化调试。

模块遵循 GMSI 一贯原则：
- 编译期可完全关闭（零代码、零 RAM 开销）
- 仅提供服务 API，不耦合具体业务逻辑
- ISR 安全，主循环负责 RTT 搬运
- 代码量极小，不引入臃肿依赖

---

## 模块定位

```
gmsi/utilities/
├── gshell.h / gshell.c        ← 已有，调试 shell
├── gwaveform.h / gwaveform.c  ← 新增，波形采集服务
└── ...

tools/gwaveform/               ← Host 侧工具（不参与 MCU 编译）
├── viewer.py                  ← pyqtgraph 示波器主程序
└── protocol.py                ← 二进制帧解析
```

`gwaveform` 与 `gshell` 相互独立，均可单独开关。若两者同时启用，gshell 会自动注册 `wave` 命令用于运行时控制。

---

## 配置项（userconfig.h）

| 宏 | 默认值 | 说明 |
|----|--------|------|
| `GWAVEFORM_ENABLE` | `0` | 总开关，0=完全关闭 |
| `GWAVEFORM_MAX_CHANNELS` | `8` | 最大通道数（影响静态数组大小） |
| `GWAVEFORM_RING_BUFFER_SIZE` | `256` | 字节，ISR→主循环 ring buffer |
| `GWAVEFORM_RTT_BUFFER_SIZE` | `512` | 字节，RTT up-buffer |
| `GWAVEFORM_RTT_CHANNEL` | `1` | RTT 通道号（0 留给 gshell/terminal） |
| `GWAVEFORM_DECIMATION` | `10` | 抽取比，ISR 每 N 次 Commit 才实际发送一帧 |

当 `GWAVEFORM_ENABLE=0` 时，所有 API 宏展开为空语句，编译器完全裁剪。

---

## 公开 API

### 初始化与生命周期

```c
// 初始化模块，注册 RTT up-buffer，清空状态
// 在 gmsi_Init() 流程中自动调用（通过 init_infos 段），无需手动调用
void gwaveform_Init(void);

// 主循环轮询：将 ring buffer 中的帧搬运到 RTT
// 在 gmsi_Run() 或 gshell_Poll() 后调用，或由 gmsi 框架自动调度
void gwaveform_Poll(void);

// 开始/停止流式传输（可由 gshell wave 命令调用，也可由业务代码调用）
void gwaveform_Start(void);
void gwaveform_Stop(void);
```

### 通道注册

```c
// 注册一个波形通道，返回通道 ID（0-based）
// pchName : 通道名称，最长 7 字节（用于 Host 侧显示）
// fScale  : 浮点值 × fScale = 存储的 int16，例如电压单位 V，fScale=100 → 精度 0.01V
uint8_t gwaveform_AddChannel(const char *pchName, float fScale);
```

通道注册应在 `gwaveform_Init()` 之后、第一次 `gwaveform_Commit()` 之前完成。

### ISR 侧采样（在 FOC 中断中调用）

```c
// 写入一个浮点采样值（内部自动 × fScale 转为 int16）
void gwaveform_Push(uint8_t chID, float fValue);

// 写入原始 int16 值（无缩放，性能最优，适合已量化的数据）
void gwaveform_PushRaw(uint8_t chID, int16_t hwValue);

// 提交当前帧到 ring buffer（在本 ISR 周期所有 Push 完成后调用一次）
// 内部执行抽取计数，未到抽取比时直接返回
void gwaveform_Commit(void);
```

### 典型调用示例（外部 FOC 项目）

```c
// 项目初始化阶段
static uint8_t s_chUa, s_chUb, s_chUc, s_chIa, s_chIb, s_chIc, s_chTheta;

void foc_Init(void) {
    s_chUa    = gwaveform_AddChannel("Ua",    100.0f);  // 单位 0.01V
    s_chUb    = gwaveform_AddChannel("Ub",    100.0f);
    s_chUc    = gwaveform_AddChannel("Uc",    100.0f);
    s_chIa    = gwaveform_AddChannel("Ia",   1000.0f);  // 单位 0.001A
    s_chIb    = gwaveform_AddChannel("Ib",   1000.0f);
    s_chIc    = gwaveform_AddChannel("Ic",   1000.0f);
    s_chTheta = gwaveform_AddChannel("Theta", 100.0f);  // 单位 0.01rad
    gwaveform_Start();
}

// FOC PWM/ADC 中断（10~20kHz）
void PWM_IRQHandler(void) {
    // ... FOC 计算 ...
    gwaveform_Push(s_chUa, fUa);
    gwaveform_Push(s_chUb, fUb);
    gwaveform_Push(s_chUc, fUc);
    gwaveform_Push(s_chIa, fIa);
    gwaveform_Push(s_chIb, fIb);
    gwaveform_Push(s_chIc, fIc);
    gwaveform_Push(s_chTheta, fTheta);
    gwaveform_Commit();  // 每 GWAVEFORM_DECIMATION 次才真正写入 ring buffer
}
```

---

## 内部架构

### 数据流

```
FOC ISR (10~20kHz)
  gwaveform_Push() × N
  gwaveform_Commit()
       │  抽取计数
       │  打包帧到 frame_buf[]
       ▼
  ring_buffer[] (RAM, lock-free SPSC)
       │
       │  主循环 gwaveform_Poll()
       ▼
  SEGGER_RTT_Write(channel 1)
       │
       │  OpenOCD RTT TCP server
       ▼
  TCP:9091
       │
       ▼
  tools/gwaveform/viewer.py (Host)
```

### Ring Buffer（单生产者单消费者，无锁）

- ISR 为唯一写入方，主循环为唯一读取方，天然 SPSC，无需互斥锁
- 使用 `volatile` head/tail 索引，写入时先写数据再更新 head（内存屏障由 Cortex-M 架构保证）
- 大小为 `GWAVEFORM_RING_BUFFER_SIZE`，建议为帧大小的整数倍

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

## gshell 集成（可选）

当 `GWAVEFORM_ENABLE=1` 且 `GSHELL_ENABLE=1` 时，自动注册 `wave` 命令：

| 命令 | 说明 |
|------|------|
| `wave start` | 开始流式传输 |
| `wave stop` | 停止流式传输 |
| `wave rate <n>` | 运行时修改抽取比（n=1~255） |
| `wave list` | 列出已注册通道及 ID |

实现方式：在 `gwaveform.c` 中用 `GMSI_SHELL_CMD()` 宏注册，与 gshell 零耦合。

---

## Host 工具（tools/gwaveform/）

### 依赖

```
pip install pyqtgraph pyserial
```

### 启动方式

```bash
# 默认连接 OpenOCD RTT TCP:9091
python tools/gwaveform/viewer.py

# 指定地址和端口
python tools/gwaveform/viewer.py --host localhost --port 9091
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
| `gwaveform_t` 结构体（8通道） | ~80 字节 RAM |
| Ring buffer（默认） | 256 字节 RAM |
| RTT up-buffer（默认） | 512 字节 RAM |
| 代码段（估算） | ~400 字节 Flash |
| **合计** | **~850 字节 RAM + ~400 字节 Flash** |

`GWAVEFORM_ENABLE=0` 时以上全部为零。

---

## 与现有模块的关系

- `gshell`：独立，可选集成 `wave` 命令
- `util_debug`（GLOG/GLOGF）：无依赖，两者使用不同 RTT 通道
- `gmsi_Run()`：`gwaveform_Poll()` 可挂入主循环，也可由用户手动调用
- `segger_rtt`：直接依赖，调用 `SEGGER_RTT_Write()` 和 `SEGGER_RTT_AllocUpBuffer()`
