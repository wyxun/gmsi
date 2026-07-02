#include <stdint.h>
#include "debug_transport.h"

#if defined(__riscv)
/* RISC-V: RTT 暂未验证，仅占位 */
#else
#   include "segger_rtt/SEGGER_RTT.h"
#endif

/**
 * @brief 初始化调试传输层 (仅 RTT，不含任何硬件 MDI 操作)
 *
 * RTT 是纯共享内存机制，不依赖任何外设，在 Cortex-M / RISC-V 上通用。
 * UART 双输出由项目层 (TRACE_MCU_WRITE_STRING) 自行组合，不在此处处理。
 */
void debug_transport_init(void)
{
#if !defined(__riscv)
    SEGGER_RTT_Init();
#endif
}

/**
 * @brief 通过 RTT 输出字符串 (架构层，不耦合项目硬件命名)
 *
 * 如需同时输出到 UART，在项目层的 TRACE_MCU_WRITE_STRING 回调中先调用此函数，
 * 再通过 MDI_Write 写入 HW.ptSerial。
 */
void debug_transport_write_string(const char *str)
{
#if !defined(__riscv)
    SEGGER_RTT_WriteString(0, str);
#else
    (void)str;
#endif
}
