#include <stdio.h>
#include "example.h"
#include "template.h"

typedef int (*InitObject)(uint32_t, uint32_t);
typedef struct
{
    uint32_t wObjectAddr;
    uint32_t wObjectCfgAddr;
    InitObject pFcn;
}init_object_t;

#define EXAMPLE_RING_BUFFER_SIZE 256
uint8_t gchExampleBuffer[EXAMPLE_RING_BUFFER_SIZE] = {0};
// example_cfg_t tExampleCfg = {
//     //.chExampleData = 0,
//     .hwRingSize = EXAMPLE_RING_BUFFER_SIZE,
//     .pchRingBuffer = gchExampleBuffer,
// };
// example_t tExample;

// template_cfg_t tTemplateCfg = {
//     .hwRingSize = 0,
//     .pchRingBuffer = NULL,
// };
// template_t tTemplate;
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

    // example object init

    // example_Init((uintptr_t)&tExample, (uintptr_t)&tExampleCfg);
    // template_Init((uintptr_t)&tTemplate, (uintptr_t)&tTemplateCfg);
    
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