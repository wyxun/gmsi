#include "mcoroutine.h"
#include <stddef.h>

__attribute__((weak)) int mcoroutine_Init(void) {
    return 0; 
}

__attribute__((weak)) int mcoroutine_Run(void) {
    return 0; 
}

__attribute__((weak)) int mcoroutine_Insert(mcoroutine_handle_t *ptHandle, void *pvParam, fcnCoroutine pfcn) {
    (void)ptHandle;
    (void)pvParam;
    (void)pfcn;
    return 0;
}
