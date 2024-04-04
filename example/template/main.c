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


// initializer element is not a compile-time constant
/*
__attribute__((section("gmsi_init_table")))
init_object_t tInitTable[] = {
    {(uint32_t)&tExample,(uint32_t)&tEexampleCfg,example_Init},
};
    int table_size = sizeof(tInitTable) / sizeof(init_object_t);
    for(uint8_t i=0;i<table_size;i++)
    {
        tInitTable[i].pFcn(tInitTable[i].wObjectAddr, tInitTable[i].wObjectCfgAddr);
    }
*/
int main()
{   
    // Ó²¼þ³õÊ¼»¯
    example_Init((uint32_t)&tExample, (uint32_t)&tEexampleCfg);
    
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