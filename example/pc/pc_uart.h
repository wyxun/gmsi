#ifndef __PC_UART_H__
#define __PC_UART_H__

#include <windows.h>
#include <stdio.h>
#include "base.h"

typedef struct{
    const char* pchCom;
    DWORD dwAttribute;
    DWORD dwShareMode;
    DWORD dwCreationDisposition;
    DWORD dwFlagsAndAttributes;
    DCB dcbSerialParams;
    COMMTIMEOUTS timeouts;
}pc_uart_cfg_t;
typedef struct{
    gmsi_base_t tBase;
    HANDLE hSerial;
    DWORD bytesWritten;
    DWORD bytesRead;
    DWORD dwRxEvent;
    OVERLAPPED overlappedRead;
    OVERLAPPED overlappedWrite;
    DWORD dwBytesTransferred;
    uint8_t chTimeoutCount;
}pc_uart_t;

int pcuart_Init(pc_uart_t *ptThis, pc_uart_cfg_t *ptCfg);
int pcuart_write(pc_uart_t *ptThis, uint8_t *pchData, uint16_t hwLength);
int pcuart_read(pc_uart_t *ptThis, uint8_t *pchData);
void pcuart_Clock(pc_uart_t *ptThis);
#endif