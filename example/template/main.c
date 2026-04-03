#include <stdio.h>
#include "example.h"
#include "template.h"
#include "utilities/util_debug.h"

#define EXAMPLE_RING_BUFFER_SIZE 256
uint8_t gchExampleBuffer[EXAMPLE_RING_BUFFER_SIZE] = {0};

GMSI_DECLARE_OBJECT(example, Example, 
    .hwRingSize = EXAMPLE_RING_BUFFER_SIZE,
    .pchRingBuffer = gchExampleBuffer,
);

GMSI_DECLARE_OBJECT(template, Template, 
    .hwRingSize = 0,
    .pchRingBuffer = NULL,
);

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
    // example hardware init
    GLOG(I, "Starting Template Example...\n");
    
    gmsi_Init(&tGmsi);
    GLOG(I, "Entering main loop.\n");
    while (1)
    {
        gmsi_Run();
    }

    return GMSI_SUCCESS;
}

void timer_handler(int signum)
{
    gmsi_Clock();
}