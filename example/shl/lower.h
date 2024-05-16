#ifndef __LOWER_H__
#define __LOWER_H__
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdint.h>
#include "gmsi.h"

typedef struct{
    const char* pchCom;
    struct termios serialAttr;
    int wOflag;
}lower_cfg_t;
typedef struct{
    gmsi_base_t *ptBase;
    int fd;
    uint8_t chBufferData[100];
    uint16_t hwBufferLength;
    gmsi_interface_t impl;
}lower_t;

int lower_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

#endif