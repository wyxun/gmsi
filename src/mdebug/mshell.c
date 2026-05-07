#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "mshell.h"
#include "mbase.h"
#include "global_define.h"
#include "mdebug/util_debug.h"

/*============================ RTT 默认后端 ==================================*/
#if MSHELL_USE_RTT_DEFAULT
#   include "segger_rtt/SEGGER_RTT.h"

static unsigned s_rtt_read(char *pchBuf, unsigned hwSize)
{
    return SEGGER_RTT_Read(0, pchBuf, hwSize);
}

static void s_rtt_write(const char *pchBuf, unsigned hwSize)
{
    SEGGER_RTT_Write(0, pchBuf, hwSize);
}

static const mshell_io_t s_tRttIO = {
    .pfcnRead  = s_rtt_read,
    .pfcnWrite = s_rtt_write,
};
#endif /* MSHELL_USE_RTT_DEFAULT */

/*============================ PRIVATE DATA ==================================*/

typedef struct {
    char    achLine[MSHELL_LINE_SIZE]; /* 行缓冲区                       */
    uint8_t chLen;                     /* 当前已缓存字节数                 */
} mshell_cb_t;

static const mshell_cmd_t *s_aptCmds[MSHELL_MAX_CMDS];
static uint8_t             s_chCmdCount = 0;
static mshell_cb_t         s_tShell;
static const mshell_io_t  *s_ptIO = NULL; /* 当前 I/O 后端               */

/*============================ PRIVATE HELPERS ===============================*/

static void shell_puts(const char *pchStr)
{
    if (s_ptIO == NULL || s_ptIO->pfcnWrite == NULL || pchStr == NULL) {
        return;
    }
    s_ptIO->pfcnWrite(pchStr, (unsigned)strlen(pchStr));
}

/* 去除字符串前导空格/Tab */
static const char *trim_leading(const char *pch)
{
    while (*pch == ' ' || *pch == '\t') {
        pch++;
    }
    return pch;
}

/* 解析十六进制无符号整数，pp 指向当前解析位置（解析后前移） */
static unsigned long parse_hex(const char **pp)
{
    unsigned long wVal = 0;
    const char *p = *pp;
    while ((*p >= '0' && *p <= '9') ||
           (*p >= 'a' && *p <= 'f') ||
           (*p >= 'A' && *p <= 'F')) {
        char c = *p++;
        wVal <<= 4;
        if      (c >= '0' && c <= '9') wVal |= (unsigned long)(c - '0');
        else if (c >= 'a' && c <= 'f') wVal |= (unsigned long)(c - 'a' + 10);
        else                           wVal |= (unsigned long)(c - 'A' + 10);
    }
    *pp = p;
    return wVal;
}

/*============================ BUILTIN: help =================================*/

static void cmd_help(const char *args)
{
    (void)args;
    shell_puts("\r\nAvailable commands:\r\n");
    for (uint8_t i = 0; i < s_chCmdCount; i++) {
        shell_puts("  ");
        shell_puts(s_aptCmds[i]->pchName);
        shell_puts("\t\t");
        shell_puts(s_aptCmds[i]->pchHelp);
        shell_puts("\r\n");
    }
}

/*============================ BUILTIN: ver ==================================*/

static void cmd_ver(const char *args)
{
    (void)args;
    extern const struct {
        uint8_t chPurpose;
        uint8_t chInterface;
        uint8_t chMajor;
        uint8_t chMinor;
    } MODUSVersion;

    MLOGF(I, "MODUS v%d.%d.%d.%d\r\n", 
                MODUSVersion.chPurpose,
                MODUSVersion.chInterface,
                MODUSVersion.chMajor,
                MODUSVersion.chMinor);
}

/*============================ BUILTIN: list =================================*/

static void cmd_list(const char *args)
{
    (void)args;
    mlist_t *ptList = mbase_GetBaseList();
    if (ptList == NULL || ptList->wNumberOfItems == 0) {
        shell_puts("No objects registered.\r\n");
        return;
    }
    shell_puts("MODUS objects:\r\n");
    for (mlist_item_t *ptItem = ptList->tListEnd.ptPrevious;
         ptItem != &ptList->tListEnd;
         ptItem = ptItem->ptPrevious) {
        modus_base_t *ptBase = (modus_base_t *)ptItem->pvOwner;
        if (ptBase == NULL) continue;
        util_debug_Printf(
            "  id=0x%x  event=0x%x\r\n",
            (unsigned)ptBase->wId,
            (unsigned)ptBase->wEvent
        );
    }
}

/*============================ BUILTIN: post =================================*/

static void cmd_post(const char *args)
{
    const char *p = trim_leading(args);
    unsigned long wId    = parse_hex(&p);
    p = trim_leading(p);
    unsigned long wEvent = parse_hex(&p);

    if (wId == 0 || wEvent == 0) {
        shell_puts("Usage: post <id_hex> <event_hex>\r\n");
        return;
    }
    int wRet = mbase_EventPost((uint32_t)wId, (uint32_t)wEvent);
    if (wRet == MODUS_SUCCESS) {
        util_debug_Printf(
            "Event 0x%x posted to id=0x%x\r\n",
            (unsigned)wEvent, (unsigned)wId
        );
    } else {
        util_debug_Printf("post failed: err=%d\r\n", wRet);
    }
}

/*============================ BUILTIN: log ==================================*/
/*
 * 用法:
 *   log          — 显示各级别当前开关状态
 *   log -E/-W/-I/-D/-T  — 切换对应级别（可组合，空格分隔），例：log -I -D
 *
 * g_chGLogMask 各位：bit0=E  bit1=W  bit2=I  bit3=D  bit4=T
 */

static void print_log_status(void)
{
    shell_puts("Log mask:  ");
    shell_puts((g_chGLogMask & MLOG_MASK_E) ? "[E]" : "( )");
    shell_puts((g_chGLogMask & MLOG_MASK_W) ? "[W]" : "( )");
    shell_puts((g_chGLogMask & MLOG_MASK_I) ? "[I]" : "( )");
    shell_puts((g_chGLogMask & MLOG_MASK_D) ? "[D]" : "( )");
    shell_puts((g_chGLogMask & MLOG_MASK_T) ? "[T]" : "( )");
    shell_puts("\r\n");
}

static void cmd_log(const char *args)
{
    const char *p = trim_leading(args);

    if (*p == '\0') {
        /* 无参数：仅显示当前状态 */
        print_log_status();
        return;
    }

    /* 解析一个或多个 -E/-W/-I/-D 参数，每个 toggle 对应 bit */
    while (*p != '\0') {
        if (*p == '-' && *(p + 1) != '\0') {
            p++; /* skip '-' */
            uint8_t chBit = 0;
            switch (*p) {
                case 'E': case 'e': chBit = MLOG_MASK_E; break;
                case 'W': case 'w': chBit = MLOG_MASK_W; break;
                case 'I': case 'i': chBit = MLOG_MASK_I; break;
                case 'D': case 'd': chBit = MLOG_MASK_D; break;
                case 'T': case 't': chBit = MLOG_MASK_T; break;
                default: break;
            }
            if (chBit) {
                g_chGLogMask ^= chBit; /* toggle 对应位 */
            }
            p++;
        } else {
            p++;
        }
        /* 跳至下一个 token */
        while (*p && *p != ' ' && *p != '\t') p++;
        p = trim_leading(p);
    }
    print_log_status();
}

/*============================ BUILTIN COMMAND TABLE =========================*/

static const mshell_cmd_t s_tCmdHelp = {
    "help", cmd_help, "Show all commands"
};
static const mshell_cmd_t s_tCmdVer = {
    "ver",  cmd_ver,  "Show MODUS version"
};
static const mshell_cmd_t s_tCmdList = {
    "list", cmd_list, "List MODUS mbase objects (id / event)"
};
static const mshell_cmd_t s_tCmdPost = {
    "post", cmd_post, "Post event: post <id_hex> <event_hex>"
};
static const mshell_cmd_t s_tCmdLog = {
    "log",  cmd_log,
    "Toggle log levels: log [-E][-W][-I][-D][-T] | no arg=show status"
};

/*============================ DISPATCH ======================================*/

static void dispatch(void)
{
    const char *pch = trim_leading(s_tShell.achLine);
    if (*pch == '\0') return; /* 空行忽略 */

    /* 定位命令名结束 */
    const char *pchEnd = pch;
    while (*pchEnd != ' ' && *pchEnd != '\t' && *pchEnd != '\0') {
        pchEnd++;
    }
    uint8_t chNameLen = (uint8_t)(pchEnd - pch);

    for (uint8_t i = 0; i < s_chCmdCount; i++) {
        if (strlen(s_aptCmds[i]->pchName) == chNameLen &&
            memcmp(s_aptCmds[i]->pchName, pch, chNameLen) == 0) {
            s_aptCmds[i]->pfcnHandle(trim_leading(pchEnd));
            return;
        }
    }
    shell_puts("Unknown command. Type 'help'.\r\n");
}

/*============================ PUBLIC API ====================================*/

void mshell_SetIO(const mshell_io_t *ptIO)
{
    s_ptIO = ptIO;
}

int mshell_RegisterCmd(uintptr_t wAddr, uintptr_t wUnused)
{
    (void)wUnused;
    const mshell_cmd_t *ptCmd = (const mshell_cmd_t *)wAddr;
    if (NULL == ptCmd)                   return MODUS_EINVAL;
    if (s_chCmdCount >= MSHELL_MAX_CMDS) return MODUS_ENOMEM;
    s_aptCmds[s_chCmdCount++] = ptCmd;
    return MODUS_SUCCESS;
}

void mshell_Poll(void)
{
    /* 延迟初始化（只执行一次） */
    static bool s_bInited = false;
    static char s_chPrev = 0; /* 用于处理 \r\n 原子性 */

    if (!s_bInited) {
        s_bInited = true;

        /* 若用户未设置 IO，使用 RTT 默认后端 */
#if MSHELL_USE_RTT_DEFAULT
        if (s_ptIO == NULL) {
            s_ptIO = &s_tRttIO;
        }
#endif
        /* 内置命令排在用户命令之后（用户命令优先显示在 help 列表中） */
        s_aptCmds[s_chCmdCount++] = &s_tCmdHelp;
        s_aptCmds[s_chCmdCount++] = &s_tCmdVer;
        s_aptCmds[s_chCmdCount++] = &s_tCmdList;
        s_aptCmds[s_chCmdCount++] = &s_tCmdPost;
        s_aptCmds[s_chCmdCount++] = &s_tCmdLog;

        shell_puts("\r\n[mshell] ready. Type 'help'.\r\n> ");
    }

    if (s_ptIO == NULL || s_ptIO->pfcnRead == NULL) return;

    char     achBuf[8];
    unsigned hwRead = s_ptIO->pfcnRead(achBuf, sizeof(achBuf));
    if (hwRead == 0) return;

    for (unsigned i = 0; i < hwRead; i++) {
        char c = achBuf[i];

        /* 处理 \r\n：如果当前是 \n 且上一个是 \r，则跳过 */
        if (c == '\n' && s_chPrev == '\r') {
            s_chPrev = c;
            continue;
        }
        s_chPrev = c;

        /* 回车/换行：执行命令 */
        if (c == '\r' || c == '\n') {
            if (s_tShell.chLen > 0) {
                shell_puts("\r\n");
                s_tShell.achLine[s_tShell.chLen] = '\0';
                dispatch();
                s_tShell.chLen = 0;
                shell_puts("> ");
            } else {
                /* 忽略空行 */
                shell_puts("\r\n> ");
            }
            continue;
        }

        /* 退格/DEL：删除最后一个字符 */
        if (c == '\b' || c == 0x7F) {
            if (s_tShell.chLen > 0) {
                s_tShell.chLen--;
                shell_puts("\b \b");
            }
            continue;
        }

        /* 普通可打印字符：入行缓冲 (已关闭回显) */
        if (c >= 0x20 && c < 0x7F) {
            if (s_tShell.chLen < (MSHELL_LINE_SIZE - 1)) {
                s_tShell.achLine[s_tShell.chLen++] = c;
            }
            /* 缓冲满时静默丢弃 */
        }
    }
}
