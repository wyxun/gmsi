# Modus 核心库大版本退役与演进路线图 (v0.5.0)

本指南面向后续开发此项目的开发者及 AI 工具。为了保障 Modus 核心库的极致性能、高可维护性与类型安全性，设计上对旧版部分过度工程化的组件进行了废弃。在下一个大版本（v0.5.0）中，这些被标记为 `deprecated` 的 API 将会被**彻底物理删除**。

在后续移植和开发新模块时，请严格遵守本路线图中的架构设计与替换原则，避免重复引入旧版设计隐患。

---

## 1. 废弃组件与退役路线

| 弃用组件 / API | 状态 | 拟退役版本 | 替代方案 | 演进核心理由 |
| :--- | :--- | :--- | :--- | :--- |
| **`mcoroutine`** | 虚化存根 (Stub) | `v0.5.0` | **MTimer + FSM 轮询状态机** | MCU 上过重的协程组件（动态栈或复杂的控制块检索）极易造成死机且调度低效。MTimer 配合 perf_counter 轮询机制完全能够以零额外内存开销实现全部业务需求，符合 KISS 原则。 |
| **`mbase_MessagePost`<br>`mbase_MessagePend`** | 标记废弃 (Deprecated) | `v0.5.0` | **RingBuffer 消息传递** | 链表消息（MessageList）在 MCU 并发中断或多任务环境下极易发生节点锁死与重入 BUG。使用被动 RingBuffer 作为通信管道能做到绝对无锁、极低开销与极高抗丢包率。 |
| **`mbase_ShareMemRead`<br>`share_mem_t`** | 标记废弃 (Deprecated) | `v0.5.0` | **全局/静态结构体指针 (DI 依赖注入)** | 查表式弱类型“共享内存”API 存在运行期哈希/ID检索损耗，且没有编译期类型安全保障。直接通过静态/全局强类型结构体并在初始化时将指针注入目标 Class，是最高效、直观且类型安全的最佳共享方案。 |

---

## 2. 核心迁移指南 (How-to-Upgrade)

### 2.1 环形缓冲区 (RingBuffer) 替代 链表消息 (MessageList)

在旧版本中，开发者通过向特定的 Class ID 发送 `message_item_t` 结构体来传递消息，接收方使用 `mbase_MessagePend` 从消息链表中提取。这种机制容易导致并发冲突和动态管理混乱。

**旧版实现 (不推荐)：**
```c
// 发送方
message_item_t tMsg;
tMsg.hwLength = sizeof(tData);
tMsg.pchBuffer = (uint8_t *)&tData;
mbase_MessagePost(TARGET_CLASS_ID, &tMsg);

// 接收方 (Poll)
message_t tRxMsg;
if (mbase_MessagePend(this.ptBase, &tRxMsg) == MODUS_SUCCESS) {
    // 处理 tRxMsg.pchBuffer
}
```

**新版黄金样板 (推荐)：**
利用 `mbase_Init` 内部的静默全自动绑定 RingBuffer，接收方不需要关心底层的 Ring 状态，直接使用非阻塞 API 检索：
```c
// 发送方：通过底层的物理/虚接口直接将数据推入目标 Class 的 RingBuffer 中
mringbuf_write(&ptTargetBase->tRingBuffer, pchData, hwLen);

// 接收方 (Poll)：直接且高效地从自身的 RingBuffer 中取出数据，无锁非阻塞
uint8_t chRxBuf[32];
int nLen = mbase_MessagePendFromRing(ptThis->ptBase, chRxBuf, sizeof(chRxBuf));
if (nLen > 0) {
    // 成功获取数据，进行非阻塞业务处理
}
```

---

## 3. 强类型依赖注入 (DI) 替代 查表式共享内存 (ShareMem)

为了让模块之间的共享关系在编译期清晰可见，避免 AI 工具或者开发者在查表时用错 ID，禁止再使用 `mbase_ShareMemRead`。新设计强要求显性进行数据依赖声明。

**旧版实现 (不推荐)：**
```c
// 某个模块试图在内部动态查找共享的系统 Tick，容易因 ID 匹配失败导致空指针崩溃
share_mem_t *ptShared = mbase_ShareMemRead(SHARE_MEM_SYS_TICK_ID);
if (ptShared != NULL) {
    uint32_t wTick = *(uint32_t *)(ptShared->pvAddr);
}
```

**新版黄金样板 (推荐)：**
在 Class 的配置结构体 `class_cfg_t` 中**显式声明**该模块运行所需的共享数据指针，在 `Init` 时由外部注入。

```c
// 1. 头文件显式声明依赖 (以 class/template_class.h 为例)
typedef struct {
    uint8_t  *pchRingBuffer;
    uint16_t  hwRingSize;
    
    /* === 显性共享数据依赖申报 (DI 依赖注入) === */
    const uint32_t *pwSharedSystemTick; // 显性依赖系统 Tick
} template_class_cfg_t;

typedef struct {
    modus_base_t *ptBase;
    const uint32_t *pwSharedSystemTick; // 保存被注入的依赖指针
    // ... 其他私有状态
} template_class_t;

// 2. 源文件内直接使用指针访问，零运行时开销，编译期安全 (class/template_class.c)
int template_class_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr)
{
    template_class_t     *ptThis = (template_class_t *)wObjectAddr;
    template_class_cfg_t *ptCfg  = (template_class_cfg_t *)wObjectCfgAddr;
    
    // 注入依赖
    ptThis->pwSharedSystemTick = ptCfg->pwSharedSystemTick;
    return mbase_Init(ptThis->ptBase, &s_tTemplateClassBaseCfg);
}

// 3. 业务逻辑中使用
if (this.pwSharedSystemTick != NULL) {
    uint32_t wCurrentTick = *(this.pwSharedSystemTick);
}
```

---

## 4. 后续 AI 开发与移植黄金法则

在后续利用 AI 编写 Modus 组件或进行老旧固件工程向 Modus 框架移植时，AI 工具必须遵守以下三条原则：

1. **原则一：先声明，后使用。** 所有的类模块都必须将其对外部其他类或全局的数据依赖，用强类型指针显式写在自己的配置 `_cfg_t` 结构体中，不得采用全局变量乱穿插或查表机制。
2. **原则二：无锁被动轮询。** 所有状态机应配合 `MTimer` 实现被动式的、非阻塞的 Poll。严禁使用 `mcoroutine` 或在状态机中使用阻塞式的 CPU `delay`。
3. **原则三：接口高度规整。** 新建 Class 必须严格按照 `template_class.c/h` 模板结构进行开发，包含统一的 `Clock` 和 `Run` 驱动入口。
