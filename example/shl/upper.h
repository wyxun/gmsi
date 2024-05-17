#ifndef __UPPER_H__
#define __UPPER_H__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "gmsi.h"

typedef struct{

}upper_cfg_t;

typedef struct{
    gmsi_base_t *ptBase;

    int wServerFd;
    int wSocket;
    int wAddrLength;
    struct sockaddr_in  tSocketAddr;
    uint8_t chBuffer[1024];

    uint16_t hwTestCount;
}upper_t;

int upper_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

#endif