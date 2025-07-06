#ifndef __AT32F421_BSP_H__
#define __AT32F421_BSP_H__

#include "at32f421.h"
#include <stdbool.h>

#define TRIGGER_CHANNEL     8
#define INCLUSIVE_MAX       5
typedef struct{
    gpio_type *ptPort;
    uint16_t hwPin;
}io_t;

typedef struct{
    uint8_t chSource;
    uint8_t chMux;
}gmux_t;
typedef struct{
    uint32_t wPin;
    gpio_type *ptPort;
    
    gmux_t tGmux;
}ggpio_t;


// A:which
// B:status
#define TRIGGER(A,B)        gpio_bits_write(c_tTrigger[A].ptPort, c_tTrigger[A].wPin, B)

void system_clock_config(void);
void bsp_Init(void);

void bsp_LedSet(bool bStatus);
uint16_t usart_sendData(uint8_t chUsartNum, uint8_t *pchSendData, uint16_t hwLength);
uint16_t usart_receiveData(uint8_t chUsartNum, uint8_t *pchReceiveData);
void USART1_TimeOutCounter(void);
void USART2_TimeOutCounter(void);

void bsp_StepMotor(uint8_t chStepMotorNum, uint8_t chInputData);
uint8_t get_exposure_status(void);
#endif