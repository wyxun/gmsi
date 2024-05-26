#ifndef __GCOROUTINE_H__
#define __GCOROUTINE_H__

#include "utilities/list.h"
#include "simple_fsm.h"
#include <stdbool.h>

// Function pointer type for coroutine functions.
// These functions should take a void pointer as parameter and return a fsm_rt_t.
typedef fsm_rt_t (*fcnCoroutine)(void *pvParam);

typedef struct{
    void *pvParam;                      // Pointer to the coroutine parameters                  
    fcnCoroutine pfcn;                  // Pointer to the coroutine function
    bool bIsRunning;                     // Flag to indicate if the coroutine is running
    struct xLIST_ITEM tListItem;        // List item for the coroutine
}gcoroutine_handle_t;

// Function prototypes
void gcoroutine_Init(void);
int gcoroutine_Run(void);

int gcoroutine_Insert(gcoroutine_handle_t *ptHandle, void *pvParam, fcnCoroutine pfcn);
#endif  // __GCOROUTINE_H__