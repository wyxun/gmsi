#include <stdint.h>
#include <unistd.h>
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

void StorageWrite(uint16_t *phwStorageStartAddr, uint16_t hwStorageLength)
{
    // Write flash save sys_data programme
}
void StorageRead(uint16_t *phwStorageStartAddr, uint16_t hwStorageLength)
{
    // Write flash read sys_data programme
}

// Structures for sys_data access
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
    // Initialise the pcclock object
    pcclock_Init((uintptr_t)&tClock, (uintptr_t)&tClockCfg);
    // Initialise the pcuart object
    pcuart_Init((uintptr_t)&tPcUart, (uintptr_t)&tPcUartCfg);
    
    GVAL_PRINTF((uint32_t)g_hwSystemDataArrary);
    gmsi_Init(&tGmsi);

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
