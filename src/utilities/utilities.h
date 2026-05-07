#ifndef __UTILITIES_H__
#define __UTILITIES_H__
#include <stdint.h>


#define PRIVILEGED_FUNCTION

#ifndef pdTRUE
    #define pdTRUE                     1
#endif
#ifndef pdFALSE
    #define pdFALSE                    0
#endif

typedef uint32_t     TickType_t;
typedef unsigned long    UBaseType_t;

#define portMAX_DELAY              ( TickType_t ) 0xffffffffUL

#ifndef mtCOVERAGE_TEST_DELAY
    #define mtCOVERAGE_TEST_DELAY()
#endif
#ifndef mtCOVERAGE_TEST_MARKER
    #define mtCOVERAGE_TEST_MARKER()
#endif

#ifndef container_of
#define container_of(pointer, type, member)                                     \
    ((type *)((char *)(pointer) - (unsigned long)(&((type *)0)->member)))
#endif

#include "mdebug/mshell.h"

#endif