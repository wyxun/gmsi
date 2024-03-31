#ifndef __UTIL_DEBUG_H__
#define __UTIL_DEBUG_H__
#include <stdio.h>
#include <stdbool.h>
//#include "utilities/trace.h"
#include "global_define.h"
//extern void utildebug_LedBreathe(void);
extern int utildebug_LedBreathe(void *ptVoid);
extern void utildebug_LedInit(void (*fcnLedSet)(bool bStatus));

#ifdef __NO_USE_LOG__
    #define LOG_OUT(...) 
#else
    #define LOG_OUT(...)         TRACE_TOSTR(__VA_ARGS__)
#endif

#endif