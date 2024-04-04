#ifndef __GMSI_H__
#define __GMSI_H__

// gmsi库所有头文件
#include "state_machine.h"
#include "base.h"
#include "global_define.h"

typedef int (*gmsi_fn_t)(void);
#define SECTION(x)                  __attribute__((section(x)))
#define GMSI_UNUSED                   __attribute__((unused))
#define GMSI_USED                     __attribute__((used))
#define ALIGN(n)                    __attribute__((aligned(n)))
#define INIT_EXPORT(fn, level)                                                       \
    GMSI_USED const gmsi_fn_t __gmsi_init_##fn SECTION(".rti_fn."level) = fn
    
/* board init routines will be called in board_init() function */
#define INIT_BOARD_EXPORT(fn)           INIT_EXPORT(fn, "1")
 
/* pre/device/component/env/app init routines will be called in init_thread */
/* components pre-initialization (pure software initilization) */
#define INIT_PREV_EXPORT(fn)            INIT_EXPORT(fn, "2")
/* device initialization */
#define INIT_DEVICE_EXPORT(fn)          INIT_EXPORT(fn, "3")
/* components initialization (dfs, lwip, ...) */
#define INIT_COMPONENT_EXPORT(fn)       INIT_EXPORT(fn, "4")
/* environment initialization (mount disk, ...) */
#define INIT_ENV_EXPORT(fn)             INIT_EXPORT(fn, "5")
/* appliation initialization (rtgui application etc ...) */
#define INIT_APP_EXPORT(fn)             INIT_EXPORT(fn, "6")

void gmsi_Clock(void);
void gmsi_Run(void);
#endif