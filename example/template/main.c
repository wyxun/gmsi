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

int main()
{   
    // example hardware init

    // example object init
    example_Init((uintptr_t)&tExample, (uintptr_t)&tEexampleCfg);
    
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