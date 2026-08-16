---
name: embedded-coding
description: 嵌入式 C 编码规则包：MISRA C:2012 安全子集 15 条、代码风格（78字符/Doxygen）、
             modus 状态机策略（perfc-PT/裸机/PLOOC）、modus 库映射、TDD 要求
whenToUse: 编写或审查 STM32/ARM/RISC-V 嵌入式 C 代码、状态机、HAL/MDI 适配、
           涉及 modus/grblhal 的代码生成与修改时
---

# Embedded Coding Rules (modus / grblhal)

编写或审查嵌入式 C 代码时必须遵守本规则包。四层规则按优先级执行：
L0 MISRA（不可违背）→ L1 风格 → L2 状态机 → L3 库映射 → TDD 验证。

## L0: MISRA C:2012 安全子集（硬规则，不可违背）

| 规则 | 要求 | bad | good |
|---|---|---|---|
| 10.3 | 禁止隐式整数转换 | `uint8_t b = a;` | `uint8_t b = (uint8_t)(a & 0xFF);` |
| 10.4 | 禁止有符号/无符号混合比较 | `if (s < u)` | `if (s < (int32_t)u)` |
| 13.1 | if 中禁止赋值 | `if ((ret = f()) == OK)` | `ret = f(); if (ret == OK)` |
| 13.2 | 禁止同一变量多次修改（未定义行为） | `i = i++ + 1;` | 分开写 |
| 13.5 | 复杂表达式中禁止自增/自减 | `d = buf[i++];` | `d = buf[i]; i++;` |
| 15.1 | if-else-if 链必须 else 结尾 | `if (x==1){} else if (x==2){}` | `... else {}` |
| 16.1 | switch 必有 default | `switch(x){case 1:break;}` | 加 `default:` |
| 16.3 | case 不能 fall-through | `case 1: f1(); case 2: f2();` | 每个 case 后 break |
| 8.2 | 函数原型显式参数 | `void process();` | `void process(void);` |
| 8.13 | 只读指针必须 const | `void read(uint8_t* buf)` | `void read(const uint8_t* buf)` |
| 9.1 | 局部变量声明即初始化 | `uint32_t cnt; if(f) cnt=10;` | `uint32_t cnt = 0;` |
| 17.7 | 返回值必须检查 | `hal_init();` | `ret = hal_init(); if (ret != OK) {...}` |
| 20.1 | #include 必须在文件开头 | `#define M\n#include <h>` | include 先于宏 |
| 20.7 | 禁止 #undef | `#undef SIZE` | 用 const/enum |

**可选规则**（默认不启用，启用需在 AGENTS.inject.md 打开）：
- 15.5 单一出口：嵌入式错误处理常用多 return，默认允许；启用时用单出口变量模式。

**偏差许可**：
- 标准库内存操作允许：`memset/memcpy/memcmp/strlen`（modus/grblhal 依赖）。
- DMA 缓冲区 `void*` 运算允许：`(uint8_t*)(void_ptr) + offset`。
- grblhal 内部代码（third_party/）不适用本规则包；只约束业务代码（src/、class/、peripheral/driver/、foc/）。

## L1: 代码风格

- **78 字符硬限制**：超出必须用 `\` 续行，续行缩进 4 空格；优先在运算符后（`&&`/`||`/`=`/`+`）或逗号后断行。
- **文件头 Doxygen（必须）**：
  ```c
  /**
   * @file    module.c
   * @brief   一句话功能
   * @author  <作者>
   * @date    YYYY-MM-DD
   */
  ```
- **函数头 Doxygen（必须）**：`@brief`（做什么，不是怎么做）、`@param`（含义/范围）、`@return`（含义，尤其错误码）、`@note`（可选：ISR 限制等）。
- **行内注释只写"为什么"**：硬件限制、性能考量、副作用、失败条件。禁止"增加变量/循环/返回/赋值"类废话注释。
- 命名：函数 `module_Action()`（如 `motor_start()`）、类型 `_t` 后缀、宏全大写。

## L2: 状态机策略（modus 真实能力）

库选择优先级：**perfc-PT 协程 > 裸机 switch > PLOOC**。

### perfc-PT 协程（首选：任务级状态机/并发流程）

- 头文件：`perfc_task_pt.h`（Protothreads，开关 `__C_LANGUAGE_EXTENSIONS_PERFC_PT__`）+ `perf_counter.h`。
- 适用：多状态、需要等待/超时/挂起的流程型状态机。
- 模板：
  ```c
  #include "perf_counter.h"       /* delay_ms / perfc_is_time_out_ms */
  #include "perfc_task_pt.h"      /* PT_BEGIN / PT_WAIT_UNTIL / PT_YIELD / PT_END */

  typedef struct {
      struct pt pt;               /* Protothreads 上下文（首个成员） */
      uint32_t   timestamp;       /* 超时时间戳 */
      uint8_t    count;
  } pump_task_t;

  static PT_THREAD(pump_run(struct pt *pt, pump_task_t *sm)) {
      PT_BEGIN(pt);
      for (;;) {
          PT_WAIT_UNTIL(pt, sm->count >= 10);   /* 等待条件 */
          delay_ms(500);                        /* 非阻塞协作延时 */
          sm->count = 0;
          PT_YIELD(pt);
      }
      PT_END(pt);
  }

  void pump_init(pump_task_t *sm) {
      PT_INIT(&sm->pt);
      sm->count = 0;
  }
  ```

### 裸机 switch（简单状态机，<8 状态）

```c
typedef enum { SM_IDLE, SM_RUN, SM_ERR } sm_state_t;
typedef struct { sm_state_t cur, next; uint32_t timeout; } my_sm_t;

static void sm_run(my_sm_t *sm) {
    switch (sm->cur) {
        case SM_IDLE:
            if (start_cond()) { sm->next = SM_RUN; sm->timeout = perfc_get() + 500; }
            break;
        case SM_RUN:
            if (perfc_is_time_out_ms(500, &sm->timeout, false)) sm->next = SM_ERR;
            break;
        default:
            sm->next = SM_ERR;
            break;
    }
    sm->cur = sm->next;
}
```

### PLOOC（复杂对象/继承/多实例）

- 头文件：`lib/plooc`（宏 `__IMPLEMENT`、`__new` 等）。
- 适用：需要封装、继承、多实例的领域对象（如电机、传感器管理器）。
- 规则：所有 Manager 结构体第一个成员是基类；用 `__new` 创建实例。

### 通用约束（跨所有档位）

1. 状态切换必须中断保护：`__disable_irq()` / `__enable_irq()` 包裹状态迁移（或等效关调度）。
2. 禁止在状态机中使用阻塞延时（HAL_Delay/vTaskDelay）；用 `perfc_delay_ms`（协程内）/ `perfc_is_time_out_ms`（轮询超时）。
3. 状态枚举必须包含 IDLE 和 ERROR。
4. 每个状态必须实现超时跳转（默认 500ms，`perfc_is_time_out_ms(500, &ts, false)`）。
5. 状态机函数命名 `xxx_run()`，初始化 `xxx_init()`；状态迁移只能发生在 `xxx_run()` 内。

## L3: modus 库映射（真实 API，禁止重复造轮子）

| 场景 | 必须使用 | 禁止 |
|---|---|---|
| 对象注册 | `MODUS_DECLARE_OBJECT` | 手写注册表/自造对象框架 |
| Shell/日志 | `MODUS_SHELL_CMD` + mdebug | 裸 printf 自造日志协议 |
| 延时 | `perfc_delay_us` / `perfc_delay_ms`（或 delay_us/delay_ms 宏） | 空循环延时、HAL_Delay |
| 超时判断 | `perfc_is_time_out_ms()` / `perfc_is_time_out_us()` | 手写毫秒计数比较 |
| 硬件访问 | MDI 层（`peripheral/<chip>/` 适配 + `peripheral/driver/` 芯片无关驱动） | 业务代码直接 include vendor HAL/寄存器 |
| 调试 | `tools/aitrace.exe` + RTT（见 aitrace skill） | 裸串口自造调试协议 |
| 运动控制 | grblhal（`mc_`/`protocol_`/`settings_` 前缀 API，经 grblhal_adapt 适配层） | 业务代码直接调 grblhal 内部 |

## TDD: 独立验证要求

- 每个业务模块必须有独立验证入口：`tests/` 目录 + 可单独编译/运行的测试 target。
- 模块通过接口解耦，测试不依赖真实硬件（HAL 抽象可注入）。
- 修改模块行为时，先补/改测试再改实现（红→绿），或提供可执行验证脚本证明行为。
- 不为测试引入过度设计：能独立验证即可，保持简单可靠。

## 使用流程（AI 生成代码时）

1. 读 AGENTS.md（浓缩硬规则已在上下文）。
2. 按任务选择状态机档位（L2）与库映射（L3）。
3. 生成代码后自查：MISRA 15 条逐条过一遍、行宽、Doxygen、返回值检查。
4. 提供测试/验证入口（TDD）。
