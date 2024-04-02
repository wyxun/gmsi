#include <stdio.h>
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
int main()
{   
    int16_t hwLength;

    pcclock_Init(&tClock, &tClockCfg);
    pcuart_Init(&tPcUart, &tPcUartCfg);

    while (1)
    {
        pcuart_Run(&tPcUart);
        pcclock_Run(&tClock);
    }

    // Close the serial port device
    pcuart_close(&tPcUart);
    return 0;
}
