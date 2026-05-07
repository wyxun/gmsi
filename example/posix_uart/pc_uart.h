#ifndef __PC_UART_H__
#define __PC_UART_H__

#include "modus.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

typedef struct{
    const char* pchCom;
    struct termios serialAttr;
    int wOflag;
}pcuart_cfg_t;
typedef struct{
    modus_base_t *ptBase;
    int fd;
    uint8_t chBufferData[100];
    uint16_t hwBufferLength;
    modus_interface_t impl;
}pcuart_t;

int pcuart_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

#endif