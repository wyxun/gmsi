/**
 * @file blm_shell_test.c
 * @brief gshell 调试命令测试模块（blm 工程专用）
 *
 * 提供 burn（老化测试）和 peek（内存读取）两条调试命令，
 * 通过 gshell RTT Shell 触发，用于 v0.2.1.0 功能验证。
 */

#include "blm_shell_test.h"
#include "utilities/gshell.h"
#include "utilities/util_debug.h"
#include <stdint.h>

/*============================ PRIVATE FUNCTIONS =============================*/

/**
 * @brief burn 命令处理：启动老化测试
 *        用法: burn
 */
static void cmd_burn(const char *args)
{
    (void)args;
    GLOGF(I, "Burn-in test started.\r\n");
    /* TODO: 替换为实际老化程序入口，例如 blm_StartBurnIn() */
}

/**
 * @brief peek 命令处理：读取并打印指定内存地址的 uint32 值
 *        用法: peek <hex_addr>   例：peek 20000000
 */
static void cmd_peek(const char *args)
{
    unsigned long wAddr = 0;
    const char *p = args;

    /* 跳过前导空格 */
    while (*p == ' ' || *p == '\t') p++;

    /* 解析十六进制地址 */
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
        GLOGF(W, "Usage: peek <hex_addr> (e.g. peek 0x20000000)\r\n");
        return;
    }

    uint32_t wVal = *(volatile uint32_t *)wAddr;
    GLOGF(I, "@0x%08X = 0x%08X (%d)\r\n", 
          (unsigned)wAddr, (unsigned)wVal, (int)wVal);
}

/*============================ PRIVATE DATA ==================================*/

GMSI_SHELL_CMD(burn, cmd_burn, "Start burn-in aging test");
GMSI_SHELL_CMD(peek, cmd_peek, "Read memory word: peek <hex_addr>");

/*============================ PUBLIC FUNCTIONS ==============================*/

/* 架构更新：现在使用 GMSI_SHELL_CMD 宏自动注册，无需显式 Init 函数 */
