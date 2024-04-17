#include <stdint.h>
#include "pc_clock.h"
#include "pc_uart.h"

#define BUFFER_SIZE 100
pc_uart_cfg_t tPcUartCfg = {
    .pchCom = "/dev/ttyS7",
    .wOflag = O_RDWR | O_NOCTTY | O_NONBLOCK,
};

pc_uart_t tPcUart;
uint8_t ReceiveData[100];

pc_clock_t tClock;
pc_clock_cfg_t tClockCfg = {
    .chClockbase = 5,
};

//gstorage_t tGstorage;
//gstorage_cfg_t tGstorageCfg = {
//}

int main()
{   
    int16_t hwLength;
    //printf("pcclock addr is %d\n", (uint32_t)&tClock);
    pcclock_Init((uint32_t)&tClock, (uint32_t)&tClockCfg);
    pcuart_Init((uint32_t)&tPcUart, (uint32_t)&tPcUartCfg);

    gbase_DegugListBase();
    gmsi_Init();
    //GMSI_ASSERT(2 == 1);
    while (1)
    {
        gmsi_Run();
    }

    return 0;
}

void timer_handler(int signum)
{
    gmsi_Clock();
}
