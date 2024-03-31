#ifndef __UTILITIES_H__
#define __UTILITIES_H__
#include <stdint.h>


#define PRIVILEGED_FUNCTION

// freertos相关定义
typedef uint32_t     TickType_t;
typedef unsigned long    UBaseType_t;

#define portMAX_DELAY              ( TickType_t ) 0xffffffffUL

#ifndef mtCOVERAGE_TEST_DELAY
    #define mtCOVERAGE_TEST_DELAY()
#endif
#ifndef mtCOVERAGE_TEST_MARKER
    #define mtCOVERAGE_TEST_MARKER()
#endif

#endif