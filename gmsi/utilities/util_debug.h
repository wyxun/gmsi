#ifndef __UTIL_DEBUG_H__
#define __UTIL_DEBUG_H__
#include <stdio.h>
#include <stdbool.h>
#include "trace.h"

//#include "global_define.h"

extern int utildebug_LedBreathe(void *ptVoid);
extern void utildebug_LedInit(void (*fcnLedSet)(bool bStatus));

#ifdef __NO_USE_LOG__
    #define LOG_OUT(...) 
#else
    #define LOG_OUT(...)         TRACE_TOSTR(__VA_ARGS__)
#endif

void assert_failed(char *file, uint32_t line);
#ifdef __NO_USE_ASSERT
    #define GMSI_ASSERT(...)
#else
    #define GMSI_ASSERT(expr)   ((expr)?(void)0:assert_failed((char *)__FILE__, __LINE__))       
#endif

#endif