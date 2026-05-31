#include "mdebug_riscv.h"
#include "mdebug/mshell.h"
#include "mdebug/util_debug.h"
#include <stdint.h>
#include <stdbool.h>

#if defined(__riscv)

/*============================ INLINE ASM HELPERS ==============================*/

static uintptr_t get_sp(void)
{
    uintptr_t r;
    __asm__ volatile ("mv %0, sp" : "=r" (r));
    return r;
}

static uintptr_t get_ra(void)
{
    uintptr_t r;
    __asm__ volatile ("mv %0, ra" : "=r" (r));
    return r;
}

static uintptr_t get_fp(void)
{
    uintptr_t r;
    __asm__ volatile ("mv %0, s0" : "=r" (r));
    return r;
}

static uint32_t get_mstatus(void)
{
    uint32_t r;
    __asm__ volatile ("csrr %0, mstatus" : "=r" (r));
    return r;
}

static uint32_t get_mcause(void)
{
    uint32_t r;
    __asm__ volatile ("csrr %0, mcause" : "=r" (r));
    return r;
}

static uint32_t get_mepc(void)
{
    uint32_t r;
    __asm__ volatile ("csrr %0, mepc" : "=r" (r));
    return r;
}

static uint32_t get_mtval(void)
{
    uint32_t r;
    __asm__ volatile ("csrr %0, mtval" : "=r" (r));
    return r;
}

static uint32_t get_mie(void)
{
    uint32_t r;
    __asm__ volatile ("csrr %0, mie" : "=r" (r));
    return r;
}

static uint32_t get_mip(void)
{
    uint32_t r;
    __asm__ volatile ("csrr %0, mip" : "=r" (r));
    return r;
}

/*============================ PRIVATE FUNCTIONS ==============================*/

/*-- regs command: dump core registers ----------------------------------------*/

static void cmd_regs(const char *args)
{
    (void)args;

    uintptr_t wSp      = get_sp();
    uintptr_t wRa      = get_ra();
    uintptr_t wFp      = get_fp();
    uint32_t  wMstatus = get_mstatus();
    uint32_t  wMcause  = get_mcause();
    uint32_t  wMepc    = get_mepc();
    uint32_t  wMtval   = get_mtval();
    uint32_t  wMie     = get_mie();
    uint32_t  wMip     = get_mip();

    MLOG(I, "\r\n=== RISC-V Core Registers ===\r\n");
    MLOGF(I, "SP (x2)  = 0x%08X\r\n", (unsigned)wSp);
    MLOGF(I, "RA (x1)  = 0x%08X\r\n", (unsigned)wRa);
    MLOGF(I, "FP (x8)  = 0x%08X\r\n", (unsigned)wFp);
    MLOGF(I, "MSTATUS  = 0x%08X (MIE=%u, MPIE=%u)\r\n",
          (unsigned)wMstatus,
          (unsigned)((wMstatus >> 3) & 1),
          (unsigned)((wMstatus >> 7) & 1));
    MLOGF(I, "MCAUSE   = 0x%08X (Interrupt=%u, ExceptionCode=%u)\r\n",
          (unsigned)wMcause,
          (unsigned)((wMcause >> 31) & 1),
          (unsigned)(wMcause & 0x7FFFFFFF));
    MLOGF(I, "MEPC     = 0x%08X\r\n", (unsigned)wMepc);
    MLOGF(I, "MTVAL    = 0x%08X\r\n", (unsigned)wMtval);
    MLOGF(I, "MIE      = 0x%08X\r\n", (unsigned)wMie);
    MLOGF(I, "MIP      = 0x%08X\r\n", (unsigned)wMip);
}

/*-- peek command: read uint32 at any memory address ---------------------------*/

static void cmd_peek(const char *args)
{
    unsigned long wAddr = 0;
    const char *p = args;

    while (*p == ' ' || *p == '\t') p++;
    if (*p == '0' && (*(p+1) == 'x' || *(p+1) == 'X')) p += 2;

    const char *pStart = p;
    while ((*p >= '0' && *p <= '9') ||
           (*p >= 'a' && *p <= 'f') ||
           (*p >= 'A' && *p <= 'F')) {
        char c = *p++;
        wAddr <<= 4;
        if      (c >= '0' && c <= '9') wAddr |= (unsigned long)(c - '0');
        else if (c >= 'a' && c <= 'f') wAddr |= (unsigned long)(c - 'a' + 10);
        else                           wAddr |= (unsigned long)(c - 'A' + 10);
    }

    if (p == pStart) {
        MLOGF(W, "Usage: peek <hex_addr>\r\n");
        return;
    }

    uint32_t wVal = *(volatile uint32_t *)wAddr;
    MLOGF(I, "@0x%08X = 0x%08X (%u)\r\n",
          (unsigned)wAddr, (unsigned)wVal, (unsigned)wVal);
}

/*-- poke command: write uint32 to any memory address -------------------------*/

static void cmd_poke(const char *args)
{
    unsigned long wAddr = 0;
    unsigned long wVal  = 0;

    while (*args == ' ' || *args == '\t') args++;

    const char *p = args;
    if (*p == '0' && (*(p+1) == 'x' || *(p+1) == 'X')) p += 2;
    while ((*p >= '0' && *p <= '9') ||
           (*p >= 'a' && *p <= 'f') ||
           (*p >= 'A' && *p <= 'F')) {
        char c = *p++;
        wAddr <<= 4;
        if      (c >= '0' && c <= '9') wAddr |= (unsigned long)(c - '0');
        else if (c >= 'a' && c <= 'f') wAddr |= (unsigned long)(c - 'a' + 10);
        else                           wAddr |= (unsigned long)(c - 'A' + 10);
    }

    while (*p == ' ' || *p == '\t') p++;

    if (*p == '0' && (*(p+1) == 'x' || *(p+1) == 'X')) p += 2;
    const char *pVal = p;
    while ((*p >= '0' && *p <= '9') ||
           (*p >= 'a' && *p <= 'f') ||
           (*p >= 'A' && *p <= 'F')) {
        char c = *p++;
        wVal <<= 4;
        if      (c >= '0' && c <= '9') wVal |= (unsigned long)(c - '0');
        else if (c >= 'a' && c <= 'f') wVal |= (unsigned long)(c - 'a' + 10);
        else                           wVal |= (unsigned long)(c - 'A' + 10);
    }

    if (p == args || p == pVal) {
        MLOGF(W, "Usage: poke <hex_addr> <hex_val>\r\n");
        return;
    }

    *(volatile uint32_t *)wAddr = (uint32_t)wVal;
    MLOGF(I, "@0x%08X <- 0x%08X\r\n", (unsigned)wAddr, (unsigned)wVal);
}

/*-- stack command: dump stack memory around current SP ------------------------*/

static bool is_likely_code_addr(uint32_t wAddr)
{
    /* CH592 code space: 0x00000000 - 0x00080000 */
    return (wAddr <= 0x00080000u);
}

static void cmd_stack(const char *args)
{
    uintptr_t wSp = get_sp();
    unsigned wDepth = 16;
    const char *p = args;

    while (*p == ' ' || *p == '\t') p++;
    if (*p >= '0' && *p <= '9') {
        wDepth = 0;
        while (*p >= '0' && *p <= '9') {
            wDepth = wDepth * 10 + (unsigned)(*p - '0');
            p++;
        }
        if (wDepth > 256) wDepth = 256;
    }

    MLOGF(I, "\r\nSP(now) = 0x%08X, dump %u words:\r\n",
          (unsigned)wSp, (unsigned)wDepth);

    for (unsigned i = 0; i < wDepth; i++) {
        uintptr_t wAddr = wSp + i * 4;
        uint32_t wVal  = *(volatile uint32_t *)wAddr;

        if (i == 0) {
            MLOGF(I, "  +0x%04X  0x%08X  0x%08X <-- SP\r\n",
                  (unsigned)(i * 4), (unsigned)wAddr, (unsigned)wVal);
        } else if (is_likely_code_addr(wVal)) {
            MLOGF(I, "  +0x%04X  0x%08X  0x%08X <-- RA/PC?\r\n",
                  (unsigned)(i * 4), (unsigned)wAddr, (unsigned)wVal);
        } else {
            MLOGF(I, "  +0x%04X  0x%08X  0x%08X\r\n",
                  (unsigned)(i * 4), (unsigned)wAddr, (unsigned)wVal);
        }
    }
}

/*-- trap command: Configurable Trap/Fault Status -----------------------------*/

static void cmd_trap(const char *args)
{
    (void)args;
    uint32_t wMcause = get_mcause();
    uint32_t wMepc   = get_mepc();
    uint32_t wMtval  = get_mtval();

    MLOGF(I, "\r\n=== RISC-V Exception CSR Status ===\r\n");
    MLOGF(I, "MCAUSE = 0x%08X  MEPC = 0x%08X  MTVAL = 0x%08X\r\n",
          (unsigned)wMcause, (unsigned)wMepc, (unsigned)wMtval);

    bool bIsInterrupt = (wMcause >> 31) & 1;
    uint32_t wCode = wMcause & 0x7FFFFFFF;

    if (bIsInterrupt) {
        MLOGF(I, "Type: Asynchronous Interrupt\r\n");
        switch (wCode) {
            case 3:  MLOG(I, "  Cause: Machine Software Interrupt (MSI)\r\n"); break;
            case 7:  MLOG(I, "  Cause: Machine Timer Interrupt (MTI)\r\n"); break;
            case 11: MLOG(I, "  Cause: Machine External Interrupt (MEI)\r\n"); break;
            default: MLOGF(I, "  Cause: Reserved Interrupt (%u)\r\n", (unsigned)wCode); break;
        }
    } else {
        MLOGF(E, "Type: Synchronous Exception (Fault)\r\n");
        switch (wCode) {
            case 0:  MLOG(E, "  Instruction address misaligned (指令地址非对齐错误)\r\n"); break;
            case 1:  MLOG(E, "  Instruction access fault (指令访问错误 - PMP限制或无效地址)\r\n"); break;
            case 2:  MLOG(E, "  Illegal instruction (非法指令 - 译码失败或特权级冲突)\r\n"); break;
            case 3:  MLOG(E, "  Breakpoint (断点异常 - ebreak)\r\n"); break;
            case 4:  MLOG(E, "  Load address misaligned (数据加载地址非对齐错误)\r\n"); break;
            case 5:  MLOG(E, "  Load access fault (数据加载错误 - PMP限制或无效物理地址)\r\n"); break;
            case 6:  MLOG(E, "  Store/AMO address misaligned (数据存储地址非对齐错误)\r\n"); break;
            case 7:  MLOG(E, "  Store/AMO access fault (数据存储错误 - PMP限制或无效物理地址)\r\n"); break;
            case 8:  MLOG(E, "  Environment call from U-mode (U模式环境系统调用 - ecall)\r\n"); break;
            case 11: MLOG(E, "  Environment call from M-mode (M模式环境系统调用 - ecall)\r\n"); break;
            case 12: MLOG(E, "  Instruction page fault (指令虚存页故障)\r\n"); break;
            case 13: MLOG(E, "  Load page fault (加载数据虚存页故障)\r\n"); break;
            case 15: MLOG(E, "  Store/AMO page fault (存储数据虚存页故障)\r\n"); break;
            default: MLOGF(E, "  Reserved / Custom Fault (%u)\r\n", (unsigned)wCode); break;
        }
    }
}

/*============================ PUBLIC FUNCTIONS ==============================*/

void mdebug_riscv_DumpException(uintptr_t sp, uint32_t mepc, uint32_t mcause, uint32_t mtval)
{
    MLOG(E, "\r\n");
    MLOG(E, "========================================\r\n");
    MLOG(E, "      !! RISC-V HARDWARE FAULT !!       \r\n");
    MLOG(E, "========================================\r\n");

    /* Exception core info */
    MLOGF(E, "MCAUSE   = 0x%08X  (Exception Code: %u)\r\n", (unsigned)mcause, (unsigned)(mcause & 0x7FFFFFFF));
    MLOGF(E, "MEPC(PC) = 0x%08X  (跑飞时的指令地址)\r\n", (unsigned)mepc);
    MLOGF(E, "MTVAL    = 0x%08X  (发生故障的内存地址/非法指令码)\r\n", (unsigned)mtval);
    MLOGF(E, "SP (x2)  = 0x%08X  (异常发生时的栈指针)\r\n", (unsigned)sp);

    /* Decode fault cause */
    uint32_t wCode = mcause & 0x7FFFFFFF;
    MLOG(E, "\r\n[故障可读性分析与定位提示]:\r\n");
    switch (wCode) {
        case 0:
            MLOG(E, "  * 指令地址非对齐错误 (Instruction address misaligned)\r\n");
            MLOG(E, "    原因: 分支跳转目标(PC)不是2字节或4字节对齐。请检查函数指针损坏或飞线。\r\n");
            break;
        case 1:
            MLOG(E, "  * 指令访问错误 (Instruction access fault)\r\n");
            MLOG(E, "    原因: 跳转到了无效的内存地址（如空指针、外设地址），或PMP（物理内存保护）触发安全阻断。\r\n");
            break;
        case 2:
            MLOG(E, "  * 非法指令 (Illegal instruction)\r\n");
            MLOGF(E, "    原因: 尝试执行未定义/不支持的指令码（指令值: 0x%08X）。\r\n", (unsigned)mtval);
            MLOG(E, "    常见于: 跑飞、内存溢出破坏了Flash里的代码段、或者执行了特权指令。\r\n");
            break;
        case 3:
            MLOG(E, "  * 软件断点 (Breakpoint)\r\n");
            MLOG(E, "    原因: 执行了 ebreak 断点指令。\r\n");
            break;
        case 4:
            MLOG(E, "  * 数据加载地址非对齐错误 (Load address misaligned)\r\n");
            MLOGF(E, "    原因: 对未对齐的地址(0x%08X)进行了多字节读取。\r\n", (unsigned)mtval);
            break;
        case 5:
            MLOG(E, "  * 数据加载错误 (Load access fault)\r\n");
            MLOGF(E, "    原因: 尝试从非法或受保护物理地址(0x%08X)读取数据，触发总线错误或PMP阻断。\r\n", (unsigned)mtval);
            break;
        case 6:
            MLOG(E, "  * 数据存储地址非对齐错误 (Store/AMO address misaligned)\r\n");
            MLOGF(E, "    原因: 对未对齐的地址(0x%08X)进行了多字节写操作。\r\n", (unsigned)mtval);
            break;
        case 7:
            MLOG(E, "  * 数据存储错误 (Store/AMO access fault)\r\n");
            MLOGF(E, "    原因: 尝试写入非法、受保护物理地址(0x%08X)或只读区域，触发总线错误或PMP阻断。\r\n", (unsigned)mtval);
            break;
        case 12: MLOG(E, "  * 指令虚存页故障 (Instruction page fault)\r\n"); break;
        case 13: MLOG(E, "  * 加载数据虚存页故障 (Load page fault)\r\n"); break;
        case 15: MLOG(E, "  * 存储数据虚存页故障 (Store/AMO page fault)\r\n"); break;
        default:
            MLOGF(E, "  * 未知/厂商自定义内核故障 (Exception Code: %u)\r\n", (unsigned)wCode);
            break;
    }

    MLOG(E, "\r\n[异常栈上下文转储 (Around SP)]:\r\n");
    /* Print 16 words from stack pointer */
    for (int i = 0; i < 16; i++) {
        uintptr_t wAddr = sp + i * 4;
        /* Simple check to avoid reading outside of SRAM */
        if (wAddr >= 0x20000000 && wAddr <= 0x20008000) {
            uint32_t wVal = *(volatile uint32_t *)wAddr;
            if (i == 0) {
                MLOGF(E, "  sp+0x%02X [0x%08X] = 0x%08X <-- 发生异常时的栈顶\r\n", i*4, (unsigned)wAddr, (unsigned)wVal);
            } else if (is_likely_code_addr(wVal)) {
                MLOGF(E, "  sp+0x%02X [0x%08X] = 0x%08X <-- 疑似代码地址/函数返回RA?\r\n", i*4, (unsigned)wAddr, (unsigned)wVal);
            } else {
                MLOGF(E, "  sp+0x%02X [0x%08X] = 0x%08X\r\n", i*4, (unsigned)wAddr, (unsigned)wVal);
            }
        }
    }

    MLOG(E, "========================================\r\n");
}

/*============================ PRIVATE DATA ===================================*/

MODUS_SHELL_CMD(regs,  cmd_regs,  "Dump RISC-V core registers (SP, RA, FP, CSRs)");
MODUS_SHELL_CMD(peek,  cmd_peek,  "Read memory word: peek <hex_addr>");
MODUS_SHELL_CMD(poke,  cmd_poke,  "Write memory word: poke <hex_addr> <hex_val>");
MODUS_SHELL_CMD(stack, cmd_stack, "Dump stack memory: stack [depth_words]");
MODUS_SHELL_CMD(trap,  cmd_trap,  "Show current trap cause and status registers");

#endif /* defined(__riscv) */
