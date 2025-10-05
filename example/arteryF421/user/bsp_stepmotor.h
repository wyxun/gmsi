#ifndef __BSP_STEPMOTOR_H__
#define __BSP_STEPMOTOR_H__

#include "at32f421_gpio.h"
#include "at32f421_bsp.h"

#define STEP_MOTOR_DIRECTION_CW  1
#define STEP_MOTOR_DIRECTION_CCW 0

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

// Ï¸·ÖÊý
#define FULL_STEP       0
#define HALF_STEP       1
#define QUARTER_STEP    2
#define EIGHTH_STEP     3
#define SIXTEENTH_STEP  7

void bsp_StepMotorInit(void);
void bsp_StepMotorEnable(bool bEnable);
void bsp_StepMotorSetMs(uint8_t chMs);
void bsp_StepMotorSetDirection(uint8_t chDirection);
void bsp_StepMotorStepControl(bool bEnable);

#endif /* __BSP_STEPMOTOR_H__ */