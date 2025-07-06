#ifndef __BSP_STEPMOTOR_H__
#define __BSP_STEPMOTOR_H__

#include "at32f421_gpio.h"
#include "util_queue.h"

// step motor
typedef struct{
    ggpio_t tMs1;
    ggpio_t tMs2;
    ggpio_t tMs3;
    ggpio_t tStep;
    ggpio_t tDir;
    ggpio_t tNsleep;
    ggpio_t tNenable;
    ggpio_t tNreset;
}step4988io_t;


#endif /* __BSP_STEPMOTOR_H__ */