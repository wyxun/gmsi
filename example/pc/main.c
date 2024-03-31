#include <stdio.h>
#include "gmsi.h"
#include "pc_uart.h"

pc_uart_cfg_t tPcUartCfg = {
    .pchCom = "com7",
    .dwAttribute = GENERIC_READ | GENERIC_WRITE,
    .dwShareMode = 0,
    .dwCreationDisposition = OPEN_EXISTING,
    .dwFlagsAndAttributes = FILE_FLAG_OVERLAPPED,
    //.dwFlagsAndAttributes = 0,

    .dcbSerialParams.BaudRate = CBR_9600,
    .dcbSerialParams.ByteSize = 8,
    .dcbSerialParams.StopBits = ONESTOPBIT,
    .dcbSerialParams.Parity = NOPARITY,
    .timeouts.ReadIntervalTimeout = 50,
    .timeouts.ReadTotalTimeoutConstant = 50,
    .timeouts.ReadTotalTimeoutMultiplier = 10,
    .timeouts.WriteTotalTimeoutConstant = 50,
    .timeouts.WriteTotalTimeoutMultiplier = 10,
};
pc_uart_t tPcUart;
uint8_t  *chTestSendData = "hello gmsi\r\n";
uint8_t chTestReadData[100];

VOID CALLBACK TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
#define TIMER_ID 1

int main(int argc, char* argv[])
{
    int wLength = 0;
    //gmsi_base_t 
    UINT_PTR timerId = SetTimer(NULL, TIMER_ID, 1, TimerCallback);
    printf("init pcuart\n");
    #if 1
    if(pcuart_Init(&tPcUart, &tPcUartCfg))
    {
        printf("open uart fail\n");
        return 0;
    }
    else
    {
        if(pcuart_write(&tPcUart, chTestSendData, sizeof(chTestSendData)))
        {
            printf("send fail\n");
        }
    }
    #endif
    fflush(stdout);
    //Sleep(5000);
    #if 1
    while(1)
    {
        wLength = pcuart_read(&tPcUart, chTestReadData);
        if(wLength)
        {
            pcuart_write(&tPcUart, chTestReadData, wLength);
            memset(chTestReadData, 0, 100);
        }
    }
    #endif
    return 0;
}

// 定时器回调函数
VOID CALLBACK TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    printf("Timer tick!\n");
    static uint16_t hwSecondCount = 1000;
    if(!hwSecondCount)
    {
        hwSecondCount = 1000;
        printf("clock \r\n");
    }
    else
    {
        hwSecondCount--;
    }

    pcuart_Clock(&tPcUart);
}