#include "at32f421_gpio.h"
#include "at32f421_tmr.h"
#include "at32f421_bsp.h"
#include "bsp_stepmotor.h"
#include "perf_counter.h"

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
//        .tGmux = {
//            .chSource = GPIO_PINS_SOURCE5,
//            .chMux = GPIO_MUX_5
//        },
    }
};

void bsp_StepMotorGpioInit(void)
{
    gpio_init_type gpio_init_struct;
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
    crm_clocks_freq_type crm_clocks_freq_struct = {0};
    crm_clocks_freq_get(&crm_clocks_freq_struct);
    
    gpio_init_type gpio_init_struct;
    // output
    gpio_init_struct.gpio_pins = c_tStepMotorIo.tStep.wPin;
    gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(c_tStepMotorIo.tStep.ptPort, &gpio_init_struct);
    
    crm_periph_clock_enable(CRM_TMR1_PERIPH_CLOCK, TRUE);
    //tmr_base_init(TMR1, 20000-1, 120-1);      // 8min  20mm  50hz/2
    tmr_base_init(TMR1, 25-1, 12000-1);       // 1min  20mm  400hz/2
    //tmr_base_init(TMR1, 50-1, 3000-1);        // 30s  20mm  800hz/2
    //tmr_base_init(TMR1, 78-1, 600-1);        // 10s  20mm  256hz/2
    //tmr_base_init(TMR1, 100-1, 100-1);       
    tmr_cnt_dir_set(TMR1, TMR_COUNT_UP);

    /* overflow interrupt enable */
    tmr_interrupt_enable(TMR1, TMR_OVF_INT, TRUE);

    /* tmr1 overflow interrupt nvic init */
    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    nvic_irq_enable(TMR1_BRK_OVF_TRG_HALL_IRQn, 0, 0);
    
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

void bsp_StepMotorStepControl(bool bEnable)
{
    gpio_bits_write(c_tStepMotorIo.tStep.ptPort, c_tStepMotorIo.tStep.wPin,
        bEnable ? 0 : 1);
}

void bsp_StepMotorSetMs(uint8_t chMs)
{
    switch(chMs)
    {
        case FULL_STEP:
            gpio_bits_write(c_tStepMotorIo.tMs1.ptPort, c_tStepMotorIo.tMs1.wPin, 0);
            gpio_bits_write(c_tStepMotorIo.tMs2.ptPort, c_tStepMotorIo.tMs2.wPin, 0);
            gpio_bits_write(c_tStepMotorIo.tMs3.ptPort, c_tStepMotorIo.tMs3.wPin, 0);
        break;
        case HALF_STEP:
            gpio_bits_write(c_tStepMotorIo.tMs1.ptPort, c_tStepMotorIo.tMs1.wPin, 1);
            gpio_bits_write(c_tStepMotorIo.tMs2.ptPort, c_tStepMotorIo.tMs2.wPin, 0);
            gpio_bits_write(c_tStepMotorIo.tMs3.ptPort, c_tStepMotorIo.tMs3.wPin, 0);
        break;
        case QUARTER_STEP:
            gpio_bits_write(c_tStepMotorIo.tMs1.ptPort, c_tStepMotorIo.tMs1.wPin, 0);
            gpio_bits_write(c_tStepMotorIo.tMs2.ptPort, c_tStepMotorIo.tMs2.wPin, 1);
            gpio_bits_write(c_tStepMotorIo.tMs3.ptPort, c_tStepMotorIo.tMs3.wPin, 0);
        break;
        case EIGHTH_STEP:
            gpio_bits_write(c_tStepMotorIo.tMs1.ptPort, c_tStepMotorIo.tMs1.wPin, 1);
            gpio_bits_write(c_tStepMotorIo.tMs2.ptPort, c_tStepMotorIo.tMs2.wPin, 1);
            gpio_bits_write(c_tStepMotorIo.tMs3.ptPort, c_tStepMotorIo.tMs3.wPin, 0);
        break;
        case SIXTEENTH_STEP:
            gpio_bits_write(c_tStepMotorIo.tMs1.ptPort, c_tStepMotorIo.tMs1.wPin, 1);
            gpio_bits_write(c_tStepMotorIo.tMs2.ptPort, c_tStepMotorIo.tMs2.wPin, 1);
            gpio_bits_write(c_tStepMotorIo.tMs3.ptPort, c_tStepMotorIo.tMs3.wPin, 1);
        break;
        default:
        break;
    }
}

void bsp_StepMotorInit(void)
{
    bsp_StepMotorGpioInit();
    bsp_StepMotorTimerInit();
    
    bsp_StepMotorEnable(false);
    bsp_StepMotorSetMs(SIXTEENTH_STEP);
    bsp_StepMotorSleep(false);
    bsp_StepMotorReset(true);
    delay_ms(100);
    bsp_StepMotorReset(false);
    tmr_counter_enable(TMR1, TRUE);
    
    //bsp_StepMotorEnable(false);
    gpio_bits_write(c_tStepMotorIo.tStep.ptPort, c_tStepMotorIo.tStep.wPin, 0);
}

void TMR1_BRK_OVF_TRG_HALL_IRQHandler(void)
{
    static bool bStatus = true;
    if(tmr_interrupt_flag_get(TMR1, TMR_OVF_FLAG) != RESET)
    {
        gpio_bits_write(c_tStepMotorIo.tStep.ptPort, c_tStepMotorIo.tStep.wPin, bStatus);
        bStatus = !bStatus;
        tmr_flag_clear(TMR1, TMR_OVF_FLAG);
    }
}

