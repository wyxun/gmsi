## Embedded Coding Rules (managed by deploy-embedded-rules.ps1)

- 编码规则权威入口：`.agents/skills/embedded-coding/SKILL.md`。写/改嵌入式 C 代码前必须加载并遵守。
- MISRA 核心：显式强转；禁止符号混合比较；switch 必有 default、case 必 break；
  if-else-if 必以 else 收尾；局部变量声明即初始化；函数返回值必须检查；
  只读指针加 const；#include 在文件顶部。
- 风格：行宽 78 字符；文件/函数头 Doxygen（@brief/@param/@return）；注释只写"为什么"。
- 状态机：优先 perfc-PT（perfc_task_pt.h），简单状态机用裸机 switch，复杂对象用 PLOOC；
  禁止阻塞延时（用 perfc_delay_ms / perfc_is_time_out_ms）；状态切换中断保护；
  状态枚举含 IDLE/ERROR；每状态超时跳转（默认 500ms）。
- 库：对象注册用 MODUS_DECLARE_OBJECT；日志用 MODUS_SHELL_CMD/mdebug；
  延时计时用 perf_counter；硬件访问走 MDI 层，禁止业务代码直接 include vendor HAL。
- TDD：每个模块必须有独立验证入口（tests/ 或可单独编译的测试 target）。
