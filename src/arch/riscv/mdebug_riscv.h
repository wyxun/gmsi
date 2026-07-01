#ifndef __MDEBUG_RISCV_H__
#define __MDEBUG_RISCV_H__

#include <stdint.h>

#if defined(__riscv)

/**
 * @brief Dump RISC-V exception registers and state.
 * @param sp Stack pointer at exception entry.
 * @param mepc Machine exception program counter.
 * @param mcause Machine exception cause.
 * @param mtval Machine trap value.
 */
void mdebug_riscv_DumpException(uintptr_t sp, uint32_t mepc, uint32_t mcause, uint32_t mtval);

#endif /* defined(__riscv) */

#endif /* __MDEBUG_RISCV_H__ */
