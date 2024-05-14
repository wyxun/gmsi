#include <stdio.h>
#include "example.h"

typedef int (*InitObject)(uint32_t, uint32_t);
typedef struct
{
    uint32_t wObjectAddr;
    uint32_t wObjectCfgAddr;
    InitObject pFcn;
}init_object_t;

example_cfg_t tEexampleCfg = {
    .chExampleData = 0,
};
example_t tExample;

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

    // example object init
    example_Init((uintptr_t)&tExample, (uintptr_t)&tEexampleCfg);
    
    gmsi_Init(&tGmsi);
    
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