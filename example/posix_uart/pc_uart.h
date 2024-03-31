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
    gmsi_base_t tBase;
    int fd;
}pc_uart_t;

int pcuart_Init(pc_uart_t *ptThis, pc_uart_cfg_t *ptCfg);
int pcuart_Read(pc_uart_t *ptThis, uint8_t *pchData, uint16_t hwMaxLength);
int pcuart_Write(pc_uart_t *ptThis, uint8_t *pchData, uint16_t hwLength);

int pcuart_close(pc_uart_t *ptThis);
#endif