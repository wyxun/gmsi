# MODUS 轻量级日志系统优化设计 (MLOG)

## 目标
优化 `utilities` 目录下的打印体系，弃用原有的基于 `__NO_USE_LOG__` 的粗粒度开关，淘汰容易引起性能开销和栈溢出风险的格式化代码（如重复调用 `snprintf`）。设计一套零缓存开销、按严重等级输出、语义清晰且具备极高可移植性的轻量级模块。

## 设计决策与思路

### 1. 命名隔离
宏定义由普通的 `LOG` 改为 `MLOG`。
- `G` 代表 MODUS，遵循项目一贯的命名风格 (类似 `mlog.c`, `mstorage` 等)。
- 能够完全避免与 `<math.h>` 等标准库（自带 `log()` 函数）或其它第三方库产生冲突。

### 2. 日志严重等级 (Severity Levels)
引入标准的五个过滤级别：
- `MODUS_LOG_LEVEL_NONE`  (0)
- `MODUS_LOG_LEVEL_ERROR` (1)
- `MODUS_LOG_LEVEL_WARN`  (2)
- `MODUS_LOG_LEVEL_INFO`  (3)
- `MODUS_LOG_LEVEL_DEBUG` (4)

全局默认定义 `MODUS_LOG_LEVEL` 为 `MODUS_LOG_LEVEL_INFO`，项目可以在外部 `userconfig.h` 等处覆盖重写，从编译期对非必要级别的输出直接进行“死代码裁剪 (Dead Code Elimination)”。

### 3. 解析策略：参数链式分发 (Option A-1)
为实现真正的嵌入式“零缓存”，本设计不涉及任何 `snprintf` 及标准格式化函数。借助引入的 `plooc.h` 变参遍历方案 (`__PLOOC_EVAL`)，将多参数转化为串行的变量提取，交予底层处理。

- **使用示例**：
  ```c
  MLOG(I, "chVar1: ", s_tAppData.chVar1, "\n");
  ```
- **等效展开原理**：
  ```c
  if (MODUS_LOG_LEVEL >= MODUS_LOG_LEVEL_INFO) {
      TRACE_TOSTR("[I] ");
      TRACE_TOSTR("chVar1: ");
      TRACE_TOSTR(s_tAppData.chVar1);
      TRACE_TOSTR("\n");
  }
  ```

### 4. 关键宏的具体实现

提供对各个等级的简写映射：
```c
#define _MLOG_LVL_E  MODUS_LOG_LEVEL_ERROR
#define _MLOG_LVL_W  MODUS_LOG_LEVEL_WARN
#define _MLOG_LVL_I  MODUS_LOG_LEVEL_INFO
#define _MLOG_LVL_D  MODUS_LOG_LEVEL_DEBUG
```

利用 `plooc` 为变参提供串行访问能力支持。定义多项元操作映射：
```c
#define __MLOG_ITEM_1(a)         TRACE_TOSTR(a)
#define __MLOG_ITEM_2(a, b)      __MLOG_ITEM_1(a); __MLOG_ITEM_1(b)
#define __MLOG_ITEM_3(a, b, c)   __MLOG_ITEM_2(a, b); __MLOG_ITEM_1(c)
// 依此类推支持1 ~ 8以上个参数...
```

顶层聚合宏：
```c
#define MLOG(LEVEL, ...) \
    do { \
        if (MODUS_LOG_LEVEL >= _MLOG_LVL_##LEVEL) { \
            TRACE_TOSTR("[" #LEVEL "] "); \
            __PLOOC_EVAL(__MLOG_ITEM_, ##__VA_ARGS__)(__VA_ARGS__); \
        } \
    } while(0)
```

## 测试与验证
1. 在 `d:\2_xundoc\project\modus\modus\utilities\util_debug.h` 中进行宏重写和升级。
2. 查找并替换 `example/blm/main.c` 中第202-203行：
   将 `snprintf(...) ; LOG_OUT(...)` 替换为 `MLOG(I, "chVar1: ", s_tAppData.chVar1, "\n");`。
3. 重新编译通过 `make`，并在 RTT 查看终端中确认相应的标签 `[I] chVar1: X` 正确展示在输出中。
