#include <stdint.h>
#include "pc_clock.h"
#include "pc_uart.h"
#include "utilities/util_debug.h"

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
void StorageWrite(uint16_t *phwStorageStartAddr, uint16_t hwStorageLength)
{

}
void StorageRead(uint16_t *phwStorageStartAddr, uint16_t hwStorageLength)
{
    
}
uint16_t g_hwSystemDataArrary[16];

gstorage_data_t tSysData = {
    .phwStorageStartAddr = g_hwSystemDataArrary,
    .hwStorageLength = 16,
    .hwCrcFlag = 0,
    .fcnWrite = StorageWrite,
    .fcnRead = StorageRead,
};

gmsi_t tGmsi = {&tSysData};

int main()
{   
    int16_t hwLength;
    //printf("pcclock addr is %d\n", (uint32_t)&tClock);
    pcclock_Init((uint32_t)&tClock, (uint32_t)&tClockCfg);
    pcuart_Init((uint32_t)&tPcUart, (uint32_t)&tPcUartCfg);
    GVAL_PRINTF((uint32_t)g_hwSystemDataArrary);
    gmsi_Init(&tGmsi);

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
