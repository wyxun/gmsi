#ifndef __PC_UART_H__
#define __PC_UART_H__

#include "base.h"
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

    gmsi_interface_t impl;
}pc_uart_t;

//int pcuart_Init(pc_uart_t *ptThis, pc_uart_cfg_t *ptCfg);
int pcuart_Init(uint32_t wObjectAddr, uint32_t wObjectCfgAddr);
//int pcuart_Read(pc_uart_t *ptThis, uint8_t *pchData, uint16_t hwMaxLength);
//int pcuart_Write(pc_uart_t *ptThis, uint8_t *pchData, uint16_t hwLength);
//int pcuart_Run(pc_uart_t *ptThis);
//int pcuart_Run(uint32_t wObjectAddr);
//int pcuart_close(pc_uart_t *ptThis);

//void pcuart_clock(uint32_t addr);
#endif