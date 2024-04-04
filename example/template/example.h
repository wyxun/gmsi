#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

// ����gmsi��
#include "gmsi.h"

// ���������Ϣ
typedef struct{
    uint8_t chExampleData;
}example_cfg_t;

typedef struct{
    /*��һ����Ա�̶���base*/
    gmsi_base_t *ptBase;
    uint8_t chExampleData;
}example_t;

int example_Init(uint32_t wObjectAddr, uint32_t wObjectCfgAddr);
#endif