#ifndef __UTIL_DEBUG_H__
#define __UTIL_DEBUG_H__
#include <stdio.h>
#include <stdbool.h>
#include "trace.h"

#ifdef GMSI_CFG_USER_CONFIG_INCLUSION
#   include GMSI_CFG_USER_CONFIG_INCLUSION
#endif

//#include "global_define.h"

extern int utildebug_LedBreathe(void *ptVoid);
extern void utildebug_LedInit(void (*fcnLedSet)(bool bStatus));

/*============================ MACROS ========================================*/
#define GMSI_LOG_LEVEL_NONE    0
#define GMSI_LOG_LEVEL_ERROR   1
#define GMSI_LOG_LEVEL_WARN    2
#define GMSI_LOG_LEVEL_INFO    3
#define GMSI_LOG_LEVEL_DEBUG   4

#ifdef __NO_USE_LOG__
    #define GMSI_LOG_LEVEL     GMSI_LOG_LEVEL_NONE
#elif !defined(GMSI_LOG_LEVEL)
    #define GMSI_LOG_LEVEL     GMSI_LOG_LEVEL_INFO
#endif

#define _GLOG_LVL_E            GMSI_LOG_LEVEL_ERROR
#define _GLOG_LVL_W            GMSI_LOG_LEVEL_WARN
#define _GLOG_LVL_I            GMSI_LOG_LEVEL_INFO
#define _GLOG_LVL_D            GMSI_LOG_LEVEL_DEBUG
/* T is a category, not a severity level.
 * Use INFO as compile-time gate so T compiles in at the default log level.
 * Runtime visibility is controlled independently by GLOG_MASK_T (bit4). */
#define _GLOG_LVL_T            GMSI_LOG_LEVEL_INFO

/*--- 运行期日志级别掩码 (Runtime Log Mask) --------------------------------*/
/* 每个 bit 对应一个级别，bit0=E bit1=W bit2=I bit3=D bit4=T */
#define GLOG_MASK_E     (1u << 0)
#define GLOG_MASK_W     (1u << 1)
#define GLOG_MASK_I     (1u << 2)
#define GLOG_MASK_D     (1u << 3)
#define GLOG_MASK_T     (1u << 4)
#define GLOG_MASK_ALL   (0x1Fu)

/* 启动默认掩码，可在 userconfig.h 中覆盖（如仅开 E+W: 0x03u） */
#ifndef GLOG_MASK_DEFAULT
#   define GLOG_MASK_DEFAULT     GLOG_MASK_ALL
#endif

/* Per-level mask bit lookup (decoupled from level numbers).
 * This allows T to have its own bit(4) independent of its compile-time gate. */
#define _GLOG_MASK_BIT_E   GLOG_MASK_E
#define _GLOG_MASK_BIT_W   GLOG_MASK_W
#define _GLOG_MASK_BIT_I   GLOG_MASK_I
#define _GLOG_MASK_BIT_D   GLOG_MASK_D
#define _GLOG_MASK_BIT_T   GLOG_MASK_T
#define _GLOG_MASK_BIT(LVL)     (_GLOG_MASK_BIT_##LVL)

/** 运行期掩码变量（定义于 util_debug.c），gshell log 命令可动态修改 */
extern uint8_t g_chGLogMask;

extern void util_debug_Printf(const char *format, ...);

#ifdef __NO_USE_LOG__
    #define GLOG(LEVEL, ...)        do {} while(0)
    #define GLOGF(LEVEL, fmt, ...)  do {} while(0)
#else
#define GLOG(LEVEL, ...)                                                        \
    do {                                                                        \
        if ((GMSI_LOG_LEVEL >= _GLOG_LVL_##LEVEL) &&                           \
            (g_chGLogMask & _GLOG_MASK_BIT(LEVEL))) {                          \
            TRACE_TOSTR("[" #LEVEL "] ");                                       \
            TRACE_TOSTR(__VA_ARGS__);                                           \
        }                                                                       \
    } while(0)

#define GLOGF(LEVEL, fmt, ...)                                                  \
    do {                                                                        \
        if ((GMSI_LOG_LEVEL >= _GLOG_LVL_##LEVEL) &&                           \
            (g_chGLogMask & _GLOG_MASK_BIT(LEVEL))) {                          \
            util_debug_Printf("[" #LEVEL "] " fmt, ##__VA_ARGS__);             \
        }                                                                       \
    } while(0)
#endif

#define LOG_OUT(...)           TRACE_TOSTR(__VA_ARGS__)

void assert_failed(char *file, uint32_t line);
#ifdef __NO_USE_ASSERT
    #define GMSI_ASSERT(...)
#else
    #define GMSI_ASSERT(expr)   ((expr)?(void)0:assert_failed((char *)__FILE__, __LINE__))       
#endif

#endif