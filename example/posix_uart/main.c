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
int main()
{   
    int16_t hwLength;
    //pcclock_Init();
    pcuart_Init(&tPcUart, &tPcUartCfg);

    while (1)
    {
        hwLength = pcuart_Read(&tPcUart, ReceiveData, 100);
        if(hwLength > 0)
        {
            if(pcuart_Write(&tPcUart, ReceiveData, hwLength) > 0)
            {

            }
            else
            {
                perror("Error while writing to the serial port");
            }
        }
        //usleep(100000); // 100ms delay
    }

    // Close the serial port device
    pcuart_close(&tPcUart);
    return 0;
}

void timer_handler(int signum)
{
    static uint16_t timeoutcount = 999;
    if(!timeoutcount)
    {
        // 定时器触发时执行的操作
        printf("Timer expired\n");
        timeoutcount = 999;
    }
    else
        timeoutcount--;

}