#include "main.h"
#include "gmsi.h"
#include "at32f421_bsp.h"
#include "gmsi.h"
//#include "platform.h"
#include "rfid.h"
#include "step4988.h"
#include "lsm6dsl.h"
#include "correspondent.h"

// object
// 初始化rfid
rfid_cfg_t tRfidCfg = {
    .Write = usart_sendData,        // 实际串口写函数
    .Read = usart_receiveData,      // 实际串口读函数
    .wFd = 0,                       // 串口号
    //.pfcnCallback = rfid_callback
};
rfid_t tRfid;
// 初始化步进电机驱动
step4988_cfg_t tStep4988Cfg ={

};
step4988_t tStep4988;

// 初始化lsm6dsl
lsm6dsl_cfg_t tLsm6dslCfg = {
    .hwUpdateRateHz = 50,
};
lsm6dsl_t tLsm6dsl;

#define CORRESPONDENT_RING_BUFFER_SIZE 50
uint8_t gchCorrespondentBuffer[CORRESPONDENT_RING_BUFFER_SIZE] = {0};
// 初始化上位机通信模块
correspondent_cfg_t tCorrespondentCfg = {
    .wFd = 1,                       // 串口号
    .Write = usart_sendData,        // 实际串口写函数
    .Read = usart_receiveData,      // 实际串口读函数
    .hwRingSize = CORRESPONDENT_RING_BUFFER_SIZE,
    .pchRingBuffer = gchCorrespondentBuffer,
};
correspondent_t tCorrespondent;

// 初始化GMSI
gmsi_t tGmsi = {
    .ptData = NULL,
    //.pfcnLedSet = bsp_LedSet
};


int main(void)
{
    system_clock_config();
    bsp_Init();
    //platform_init();
    utildebug_LedInit(bsp_LedSet);

    // 加载模块
    rfid_Init((uintptr_t)&tRfid, (uintptr_t)&tRfidCfg);
    step4988_Init((uintptr_t)&tStep4988, (uintptr_t)&tStep4988Cfg);
    lsm6dsl_Init((uintptr_t)&tLsm6dsl, (uintptr_t)&tLsm6dslCfg);
    correspondent_Init((uintptr_t)&tCorrespondent, (uintptr_t)&tCorrespondentCfg);
    // 初始化库
    gmsi_Init(&tGmsi);

    for(;;)
    {
        utildebug_LedBreathe(NULL);
        gmsi_Run();
    }
    return 0;
}

void SysTick_Handler(void)
{
    gmsi_Clock();
    
    USART1_TimeOutCounter();
    USART2_TimeOutCounter();
}
