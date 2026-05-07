#ifndef __UTIL_DEBUG_H__
#define __UTIL_DEBUG_H__
#include <stdio.h>
#include <stdbool.h>
#include "trace.h"

#ifdef MODUS_CFG_USER_CONFIG_INCLUSION
#   include MODUS_CFG_USER_CONFIG_INCLUSION
#endif

//#include "global_define.h"

extern int utildebug_LedBreathe(void *ptVoid);
extern void utildebug_LedInit(void (*fcnLedSet)(bool bStatus));

/*============================ MACROS ========================================*/
#define MODUS_LOG_LEVEL_NONE    0
#define MODUS_LOG_LEVEL_ERROR   1
#define MODUS_LOG_LEVEL_WARN    2
#define MODUS_LOG_LEVEL_INFO    3
#define MODUS_LOG_LEVEL_DEBUG   4

#ifdef __NO_USE_LOG__
    #define MODUS_LOG_LEVEL     MODUS_LOG_LEVEL_NONE
#elif !defined(MODUS_LOG_LEVEL)
    #define MODUS_LOG_LEVEL     MODUS_LOG_LEVEL_INFO
#endif

#define _MLOG_LVL_E            MODUS_LOG_LEVEL_ERROR
#define _MLOG_LVL_W            MODUS_LOG_LEVEL_WARN
#define _MLOG_LVL_I            MODUS_LOG_LEVEL_INFO
#define _MLOG_LVL_D            MODUS_LOG_LEVEL_DEBUG
/* T is a category, not a severity level.
 * Use INFO as compile-time gate so T compiles in at the default log level.
 * Runtime visibility is controlled independently by MLOG_MASK_T (bit4). */
#define _MLOG_LVL_T            MODUS_LOG_LEVEL_INFO

/*--- 运行期日志级别掩码 (Runtime Log Mask) --------------------------------*/
/* 每个 bit 对应一个级别，bit0=E bit1=W bit2=I bit3=D bit4=T */
#define MLOG_MASK_E     (1u << 0)
#define MLOG_MASK_W     (1u << 1)
#define MLOG_MASK_I     (1u << 2)
#define MLOG_MASK_D     (1u << 3)
#define MLOG_MASK_T     (1u << 4)
#define MLOG_MASK_ALL   (0x1Fu)

/* 启动默认掩码，可在 userconfig.h 中覆盖（如仅开 E+W: 0x03u） */
#ifndef MLOG_MASK_DEFAULT
#   define MLOG_MASK_DEFAULT     MLOG_MASK_ALL
#endif

/* Per-level mask bit lookup (decoupled from level numbers).
 * This allows T to have its own bit(4) independent of its compile-time gate. */
#define _MLOG_MASK_BIT_E   MLOG_MASK_E
#define _MLOG_MASK_BIT_W   MLOG_MASK_W
#define _MLOG_MASK_BIT_I   MLOG_MASK_I
#define _MLOG_MASK_BIT_D   MLOG_MASK_D
#define _MLOG_MASK_BIT_T   MLOG_MASK_T
#define _MLOG_MASK_BIT(LVL)     (_MLOG_MASK_BIT_##LVL)

/** 运行期掩码变量（定义于 util_debug.c），mshell log 命令可动态修改 */
extern uint8_t g_chGLogMask;

extern void util_debug_Printf(const char *format, ...);

#ifdef __NO_USE_LOG__
    #define MLOG(LEVEL, ...)        do {} while(0)
    #define MLOGF(LEVEL, fmt, ...)  do {} while(0)
#else
#define MLOG(LEVEL, ...)                                                        \
    do {                                                                        \
        if ((MODUS_LOG_LEVEL >= _MLOG_LVL_##LEVEL) &&                           \
            (g_chGLogMask & _MLOG_MASK_BIT(LEVEL))) {                          \
            TRACE_TOSTR("[" #LEVEL "] ");                                       \
            TRACE_TOSTR(__VA_ARGS__);                                           \
        }                                                                       \
    } while(0)

#define MLOGF(LEVEL, fmt, ...)                                                  \
    do {                                                                        \
        if ((MODUS_LOG_LEVEL >= _MLOG_LVL_##LEVEL) &&                           \
            (g_chGLogMask & _MLOG_MASK_BIT(LEVEL))) {                          \
            util_debug_Printf("[" #LEVEL "] " fmt, ##__VA_ARGS__);             \
        }                                                                       \
    } while(0)
#endif

#define LOG_OUT(...)           TRACE_TOSTR(__VA_ARGS__)

void assert_failed(char *file, uint32_t line);
#ifdef __NO_USE_ASSERT
    #define MODUS_ASSERT(...)
#else
    #define MODUS_ASSERT(expr)   ((expr)?(void)0:assert_failed((char *)__FILE__, __LINE__))       
#endif

#endif