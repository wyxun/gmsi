#ifndef __UPPER_H__
#define __UPPER_H__
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "gmsi.h"

typedef struct{

}upper_cfg_t;

typedef struct{
    gmsi_base_t *ptBase;

    int wServerFd;
    int wClinetFd;
    int wAddrLength;
    struct sockaddr_un tServerAddr;
    struct sockaddr_un tClinetAddr;
    uint8_t chBuffer[1024];

    uint16_t hwTestCount;
}upper_t;

int upper_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

#endif