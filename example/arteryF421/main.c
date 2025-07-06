#include "main.h"
#include "gmsi.h"
#include "at32f421_board.h"
#include "at32f421_bsp.h"
#include "gmsi.h"
//#include "platform.h"
#include "rfid.h"

// object
// 初始化配置
rfid_cfg_t tRfidCfg = {
    .Write = usart_sendData, // 实际串口写函数
    .Read = usart_receiveData,   // 实际串口读函数
    .pfcnCallback = rfid_callback
};

rfid_t tRfid;

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

    gmsi_Init(&tGmsi);

    // 修改波特率到115200
    rfid_Configure(&tRfid, 0x2C, 0x11); 

    // 修改模块地址到0x01
    rfid_Configure(&tRfid, 0x2B, 0x01);
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
