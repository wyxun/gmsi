#ifndef __GMSI_GSHELL_H__
#define __GMSI_GSHELL_H__

#include <stdint.h>
#include <stdbool.h>
#include "global_define.h"

/*============================ MACROS ========================================*/

/* 命令行缓冲区大小（含 '\0'），可在 userconfig.h 中覆盖 */
#ifndef GSHELL_LINE_SIZE
#   define GSHELL_LINE_SIZE     32
#endif

/* 最大注册命令数（含内置命令），可在 userconfig.h 中覆盖 */
#ifndef GSHELL_MAX_CMDS
#   define GSHELL_MAX_CMDS      16
#endif

/* 为 0 时不编译 RTT 默认后端，需手动调用 gshell_SetIO() */
#ifndef GSHELL_USE_RTT_DEFAULT
#   define GSHELL_USE_RTT_DEFAULT   1
#endif

/* 自动注册命令宏：在任何 .c 文件中使用，无需手动初始化。
 * 它会在 init_infos 章节中产生一个初始化项，由 gmsi_Init() 自动触发注册。
 */
#define GMSI_SHELL_CMD(name, handler, help_str)                                 \
    static const gshell_cmd_t shell_cmd_##name = {                             \
        .pchName = #name,                                                       \
        .pfcnHandle = (handler),                                                \
        .pchHelp = (help_str),                                                  \
    };                                                                          \
    INIT_SECTION const gmsi_init_info_t shell_init_##name = {                   \
        .pfcnInitFunc = (init_func_t)gshell_RegisterCmd,                        \
        .wObjectAddr = (uintptr_t)&shell_cmd_##name,                            \
        .wConfigAddr = 0                                                        \
    }

/*============================ TYPES =========================================*/

/**
 * @brief gshell I/O 后端接口
 *   pfcnRead  — 非阻塞读，无数据返回 0
 *   pfcnWrite — 输出 hwSize 字节
 */
typedef struct {
    unsigned (*pfcnRead) (char *pchBuf, unsigned hwSize);
    void     (*pfcnWrite)(const char *pchBuf, unsigned hwSize);
} gshell_io_t;

/**
 * @brief 命令处理回调，args 为命令名后的参数字符串（已去前导空格），
 *        无参数时为空字符串 ""
 */
typedef void (*gshell_cmd_handler_t)(const char *args);

typedef struct {
    const char           *pchName;    /**< 命令名，如 "burn"           */
    gshell_cmd_handler_t  pfcnHandle; /**< 命令处理回调                 */
    const char           *pchHelp;    /**< 单行帮助文本                 */
} gshell_cmd_t;

/*============================ PROTOTYPES ====================================*/

/**
 * @brief 设置 I/O 后端（不调用则默认使用 RTT）
 * @param ptIO 指向静态/全局 gshell_io_t，生命周期须覆盖整个运行周期
 */
void gshell_SetIO(const gshell_io_t *ptIO);

/**
 * @brief 注册用户自定义命令
 *        须在 gmsi_Init() 后、gshell_Poll() 首次调用前调用
 * @return GMSI_SUCCESS 或 GMSI_ENOMEM（命令表已满）
 */
int gshell_RegisterCmd(uintptr_t wAddr, uintptr_t wUnused);

/**
 * @brief 轮询 I/O 并处理命令；由 gmsi_Run() 内部自动调用，用户无需手动调用
 */
void gshell_Poll(void);

#endif /* __GMSI_GSHELL_H__ */
