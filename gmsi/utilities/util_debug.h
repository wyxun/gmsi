#ifndef __UTIL_DEBUG_H__
#define __UTIL_DEBUG_H__
#include <stdio.h>
#include <stdbool.h>
#include "trace.h"

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

#define __GLOG_ITEM_1(a)       TRACE_TOSTR(a)
#define __GLOG_ITEM_2(a, b)    __GLOG_ITEM_1(a); __GLOG_ITEM_1(b)
#define __GLOG_ITEM_3(a, b, c) __GLOG_ITEM_2(a, b); __GLOG_ITEM_1(c)
#define __GLOG_ITEM_4(a, b, c, d)  __GLOG_ITEM_3(a, b, c); __GLOG_ITEM_1(d)
#define __GLOG_ITEM_5(a, b, c, d, e) __GLOG_ITEM_4(a, b, c, d); __GLOG_ITEM_1(e)
#define __GLOG_ITEM_6(a, b, c, d, e, f) __GLOG_ITEM_5(a, b, c, d, e); __GLOG_ITEM_1(f)
#define __GLOG_ITEM_7(a, b, c, d, e, f, g) __GLOG_ITEM_6(a, b, c, d, e, f); __GLOG_ITEM_1(g)
#define __GLOG_ITEM_8(a, b, c, d, e, f, g, h) __GLOG_ITEM_7(a, b, c, d, e, f, g); __GLOG_ITEM_1(h)

extern void util_debug_Printf(const char *format, ...);

#define GLOG(LEVEL, ...)                                                        \
    do {                                                                        \
        if (GMSI_LOG_LEVEL >= _GLOG_LVL_##LEVEL) {                              \
            TRACE_TOSTR("[" #LEVEL "] ");                                       \
            __PLOOC_EVAL(__GLOG_ITEM_, ##__VA_ARGS__)(__VA_ARGS__);            \
        }                                                                       \
    } while(0)

#define GLOGF(LEVEL, fmt, ...)                                                  \
    do {                                                                        \
        if (GMSI_LOG_LEVEL >= _GLOG_LVL_##LEVEL) {                              \
            util_debug_Printf("[" #LEVEL "] " fmt, ##__VA_ARGS__);             \
        }                                                                       \
    } while(0)

#define LOG_OUT(...)           TRACE_TOSTR(__VA_ARGS__)

void assert_failed(char *file, uint32_t line);
#ifdef __NO_USE_ASSERT
    #define GMSI_ASSERT(...)
#else
    #define GMSI_ASSERT(expr)   ((expr)?(void)0:assert_failed((char *)__FILE__, __LINE__))       
#endif

#endif