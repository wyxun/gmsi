#include "at32f421_gpio.h"
#include "at32f421_tmr.h"
#include "at32f421_bsp.h"
#include "bsp_stepmotor.h"

#define STEP_MOTOR_DIRECTION_CW  1
#define STEP_MOTOR_DIRECTION_CCW 0

const step4988io_t c_tStepMotorIo = {
    .tDir = {
        .ptPort = GPIOA,
        .wPin = GPIO_PINS_4
    },
    .tMs1 = {
        .ptPort = GPIOB,
        .wPin = GPIO_PINS_2
    },
    .tMs2 = {
        .ptPort = GPIOB,
        .wPin = GPIO_PINS_1
    },
    .tMs3 = {
        .ptPort = GPIOB,
        .wPin = GPIO_PINS_0
    },
    .tNenable = {
        .ptPort = GPIOB,
        .wPin = GPIO_PINS_10
    },
    .tNreset = {
        .ptPort = GPIOA,
        .wPin = GPIO_PINS_7
    },
    .tNsleep = {
        .ptPort = GPIOA,
        .wPin = GPIO_PINS_6
    },
    .tStep = {
        .ptPort = GPIOA,
        .wPin = GPIO_PINS_5,
        .tGmux = {
            .chSource = GPIO_PINS_SOURCE5,
            .chMux = GPIO_MUX_5
        },
    }
};

void bsp_StepMotorGpioInit(void)
{
    gpio_init_type gpio_init_struct;
    uint8_t chOffset = 0;
    // output
    gpio_init_struct.gpio_pins = c_tStepMotorIo.tDir.wPin;
    gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(c_tStepMotorIo.tDir.ptPort, &gpio_init_struct);

    gpio_init_struct.gpio_pins = c_tStepMotorIo.tMs1.wPin;
    gpio_init(c_tStepMotorIo.tMs1.ptPort, &gpio_init_struct);

    gpio_init_struct.gpio_pins = c_tStepMotorIo.tMs2.wPin;
    gpio_init(c_tStepMotorIo.tMs2.ptPort, &gpio_init_struct);

    gpio_init_struct.gpio_pins = c_tStepMotorIo.tMs3.wPin;
    gpio_init(c_tStepMotorIo.tMs3.ptPort, &gpio_init_struct);

    gpio_init_struct.gpio_pins = c_tStepMotorIo.tNenable.wPin;
    gpio_init(c_tStepMotorIo.tNenable.ptPort, &gpio_init_struct);

    gpio_init_struct.gpio_pins = c_tStepMotorIo.tNreset.wPin;
    gpio_init(c_tStepMotorIo.tNreset.ptPort, &gpio_init_struct);

    gpio_init_struct.gpio_pins = c_tStepMotorIo.tNsleep.wPin;
    gpio_init(c_tStepMotorIo.tNsleep.ptPort, &gpio_init_struct);
}

void bsp_StepMotorTimerInit(void)
{
    // timer_init_type timer_init_struct;
    // timer_init_struct.timer_prescaler = 0;
    // timer_init_struct.timer_period = 1000 - 1;
    // timer_init_struct.timer_counter_mode = TIMER_COUNTER_UP;
    // timer_init_struct.timer_auto_reload = TIMER_AUTO_RELOAD_ENABLE;
    // timer_init_struct.timer_update_request = TIMER_UPDATE_REQUEST_ENABLE;
    // timer_init(c_tStepMotorIo.tStep.ptPort, &timer_init_struct);
}

void bsp_StepMotorSetSpeed(uint32_t speed)
{
    //timer_set_period(c_tStepMotorIo.tStep.ptPort, speed);
}

void bsp_StepMotorSetDirection(uint8_t chDirection)
{
    gpio_bits_write(c_tStepMotorIo.tDir.ptPort, c_tStepMotorIo.tDir.wPin,
        (chDirection == STEP_MOTOR_DIRECTION_CW) ? 0 : 1);
}

void bsp_StepMotorSleep(bool bSleep)
{
    gpio_bits_write(c_tStepMotorIo.tNsleep.ptPort, c_tStepMotorIo.tNsleep.wPin,
        bSleep ? 0 : 1);
}

void bsp_StepMotorReset(bool bReset)
{
    gpio_bits_write(c_tStepMotorIo.tNreset.ptPort, c_tStepMotorIo.tNreset.wPin,
        bReset ? 0 : 1);
}

void bsp_StepMotorEnable(bool bEnable)
{
    gpio_bits_write(c_tStepMotorIo.tNenable.ptPort, c_tStepMotorIo.tNenable.wPin,
        bEnable ? 0 : 1);
}