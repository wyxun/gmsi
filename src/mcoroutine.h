#ifndef __MCOROUTINE_H__
#define __MCOROUTINE_H__

#include "utilities/mlist.h"
#include "perf_counter.h"
#include <stdbool.h>

// Function pointer type for coroutine functions.
// These functions should take a void pointer as parameter and return a fsm_rt_t.
typedef fsm_rt_t (*fcnCoroutine)(void *pvParam);

typedef struct{
    uint8_t chState;
    void *pvParam;                      // Pointer to the coroutine parameters
    fcnCoroutine pfcn;                  // Pointer to the coroutine function
    bool bIsRunning;                    // Flag to indicate if the coroutine is running
    mlist_item_t tListItem;        // List item for the coroutine
}mcoroutine_handle_t;

// Function prototypes
int mcoroutine_Init(void);
int mcoroutine_Run(void);

int mcoroutine_Insert(mcoroutine_handle_t *ptHandle, void *pvParam, fcnCoroutine pfcn);
#endif  // __MCOROUTINE_H__