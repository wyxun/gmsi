#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

// 引用gmsi库
#include "gmsi.h"

// 定义对象信息
typedef struct{
    uint8_t chExampleData;
}example_cfg_t;

typedef struct{
    /*第一个成员固定是base*/
    gmsi_base_t *ptBase;
    uint8_t chExampleData;
}example_t;

int example_Init(uint32_t wObjectAddr, uint32_t wObjectCfgAddr);
#endif