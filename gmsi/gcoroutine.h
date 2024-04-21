#ifndef __GCOROUTINE_H__
#define __GCOROUTINE_H__

#include "utilities/list.h"
#include "simple_fsm.h"
#include <stdbool.h>

typedef fsm_rt_t (*fcnCoroutine)(void *pvParam);

typedef struct{
    void *pvParam;
    fcnCoroutine pfcn;
    bool bIsRun;
    struct xLIST_ITEM tListItem;
}gcoroutine_handle_t;

void gcoroutine_Init(void);
int gcoroutine_Run(void);

int gcoroutine_Insert(gcoroutine_handle_t *ptHandle, void *pvParam, fcnCoroutine pfcn);
#endif