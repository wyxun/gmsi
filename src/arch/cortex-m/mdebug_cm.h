#ifndef __MDEBUG_CM_H__
#define __MDEBUG_CM_H__

#include <stdint.h>

/* Sentinel: when defined, IT files skip their stub fault handlers
 * because fault_cm.c provides real ones. */
#define MDEBUG_CM_FAULT_HANDLERS_ACTIVE

/**
 * @brief Register all Cortex-M debug shell commands (regs, peek, poke, stack, cfsr).
 *        Called automatically via MODUS_SHELL_CMD macro; no manual init needed.
 */
#endif /* __MDEBUG_CM_H__ */
