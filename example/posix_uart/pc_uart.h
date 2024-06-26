#ifndef __PC_UART_H__
#define __PC_UART_H__

#include "gmsi.h"
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
}pc_uart_cfg_t;
typedef struct{
    gmsi_base_t *ptBase;
    int fd;
    uint8_t chBufferData[100];
    uint16_t hwBufferLength;
    gmsi_interface_t impl;
}pc_uart_t;

int pcuart_Init(uintptr_t wObjectAddr, uintptr_t wObjectCfgAddr);

#endif