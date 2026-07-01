#include "mdebug_cm.h"
#include "mdebug/util_debug.h"

#if defined(__ARM_ARCH)

#ifndef CORE_DEBUG_OVERRIDE_FAULT_HANDLER

typedef struct {
    uint32_t wR0;
    uint32_t wR1;
    uint32_t wR2;
    uint32_t wR3;
    uint32_t wR12;
    uint32_t wLR;
    uint32_t wPC;
    uint32_t wxPSR;
} exception_frame_t;

#define SCB_CFSR  (*(volatile uint32_t *)0xE000ED28u)
#define SCB_HFSR  (*(volatile uint32_t *)0xE000ED2Cu)
#define SCB_MMAR  (*(volatile uint32_t *)0xE000ED34u)
#define SCB_BFAR  (*(volatile uint32_t *)0xE000ED38u)

static void dump_exception_frame(const exception_frame_t *ptFrame,
                                 const char *pchFaultType)
{
    uint32_t wCfsr = SCB_CFSR;
    uint32_t wHfsr = SCB_HFSR;
    uint32_t wMmar = SCB_MMAR;
    uint32_t wBfar = SCB_BFAR;

    MLOG(E, "\r\n");
    MLOG(E, "========================================\r\n");
    MLOGF(E, "  !! %s !! \r\n", pchFaultType);
    MLOG(E, "========================================\r\n");

    MLOGF(E, "R0   = 0x%08X    R1  = 0x%08X\r\n",
          (unsigned)ptFrame->wR0,  (unsigned)ptFrame->wR1);
    MLOGF(E, "R2   = 0x%08X    R3  = 0x%08X\r\n",
          (unsigned)ptFrame->wR2,  (unsigned)ptFrame->wR3);
    MLOGF(E, "R12  = 0x%08X    LR  = 0x%08X\r\n",
          (unsigned)ptFrame->wR12, (unsigned)ptFrame->wLR);
    MLOGF(E, "PC   = 0x%08X    xPSR= 0x%08X\r\n",
          (unsigned)ptFrame->wPC,  (unsigned)ptFrame->wxPSR);

    MLOGF(E, "\r\nCFSR = 0x%08X  HFSR = 0x%08X\r\n",
          (unsigned)wCfsr, (unsigned)wHfsr);
    MLOGF(E, "MMAR = 0x%08X  BFAR = 0x%08X\r\n",
          (unsigned)wMmar, (unsigned)wBfar);

    uint8_t chMmf = (uint8_t)(wCfsr & 0xFF);
    uint8_t chBf  = (uint8_t)((wCfsr >> 8) & 0xFF);
    uint8_t chUf  = (uint8_t)((wCfsr >> 16) & 0xFF);

    if (chMmf) {
        MLOG(E, "\r\n[MEM MANAGE FAULT]\r\n");
        if (chMmf & 1)  MLOGF(E, "  IACCVIOL: instruction access violation\r\n");
        if (chMmf & 2)  MLOGF(E, "  DACCVIOL: data access violation, MMAR=0x%08X\r\n", (unsigned)wMmar);
        if (chMmf & 0x80) MLOG(E, "  UNSTKERR: unstacking error\r\n");
    }
    if (chBf) {
        MLOG(E, "\r\n[BUS FAULT]\r\n");
        if (chBf & 1)  MLOG(E, "  IBUSERR: bus fault on instruction fetch\r\n");
        if (chBf & 2)  MLOGF(E, "  PRECISERR: precise data bus fault, BFAR=0x%08X\r\n", (unsigned)wBfar);
        if (chBf & 4)  MLOG(E, "  IMPRECISERR: imprecise data bus fault\r\n");
    }
    if (chUf) {
        MLOG(E, "\r\n[USAGE FAULT]\r\n");
        if (chUf & 1)  MLOG(E, "  UNDEFINSTR: undefined instruction\r\n");
        if (chUf & 2)  MLOG(E, "  INVSTATE: invalid state (e.g. ARM/Thumb mismatch)\r\n");
        if (chUf & 4)  MLOG(E, "  INVPC: invalid PC load\r\n");
        if (chUf & 8)  MLOG(E, "  NOCP: no coprocessor\r\n");
        if (chUf & 0x100) MLOG(E, "  DIVBYZERO: divide by zero\r\n");
    }
    if (wHfsr & 0x40000000) MLOG(E, "\r\n[HARD FAULT] FORCED: escalated from another fault\r\n");

    unsigned wExNum = ptFrame->wxPSR & 0x1FFu;
    if (wExNum > 0) {
        MLOGF(E, "\r\nException number in xPSR: %u\r\n", wExNum);
    }

    MLOG(E, "========================================\r\n");
}

/* Naked wrapper to capture the true exception LR before C prologue */
__attribute__((naked))
void HardFault_Handler(void)
{
    __asm__ volatile (
        "mov r0, lr\n"
        "b   fault_common\n"
    );
}

__attribute__((naked))
void MemManage_Handler(void)
{
    __asm__ volatile (
        "mov r0, lr\n"
        "b   fault_common\n"
    );
}

__attribute__((naked))
void BusFault_Handler(void)
{
    __asm__ volatile (
        "mov r0, lr\n"
        "b   fault_common\n"
    );
}

__attribute__((naked))
void UsageFault_Handler(void)
{
    __asm__ volatile (
        "mov r0, lr\n"
        "b   fault_common\n"
    );
}

/* Common C handler: r0 = exception LR (EXC_RETURN) */
__attribute__((used))
void fault_common(uint32_t wExcLR)
{
    uint32_t wMsp, wPsp;
    __asm__ volatile ("mrs %0, msp" : "=r" (wMsp));
    __asm__ volatile ("mrs %0, psp" : "=r" (wPsp));
    const exception_frame_t *ptFrame;

    if ((wExcLR & 0x4u) != 0) {
        ptFrame = (const exception_frame_t *)wPsp;
    } else {
        ptFrame = (const exception_frame_t *)wMsp;
    }

    uint32_t wHfsr = SCB_HFSR;
    const char *pchType;
    if (wHfsr & 0x40000000) {
        pchType = "HARD FAULT";
    } else if (SCB_CFSR & 0xFF) {
        pchType = "MEM MANAGE FAULT";
    } else if ((SCB_CFSR >> 8) & 0xFF) {
        pchType = "BUS FAULT";
    } else {
        pchType = "USAGE FAULT";
    }

    dump_exception_frame(ptFrame, pchType);

    while (1) {
        __asm__ volatile ("wfi");
    }
}

#endif /* CORE_DEBUG_OVERRIDE_FAULT_HANDLER */
#endif /* defined(__ARM_ARCH) */
