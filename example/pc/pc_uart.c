#include "pc_uart.h"

#define RECEIVE_TIMEOUT 4

uint8_t pcuart_rxdata[100];

VOID CALLBACK ReadCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
    for(int i=0;i<dwNumberOfBytesTransfered;i++)
    {
        printf("%c", pcuart_rxdata[i]);
    }
    //ReadFileEx(ptThis->hSerial, pcuart_rxdata, 100, &ptThis->overlappedRead, &ReadCompletionRoutine);

    fflush(stdout);

}
VOID CALLBACK WriteCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
    if (dwErrorCode == 0 && dwNumberOfBytesTransfered > 0)
    {
        printf("WriteCompletionRoutine ok :%lu\n", dwNumberOfBytesTransfered);
    }
    else
    {
        printf("WriteCompletionRoutine fail :%lu\n", dwErrorCode);
    }
    fflush(stdout);
}

int pcuart_Init(pc_uart_t *ptThis, pc_uart_cfg_t *ptCfg)
{
    BOOL bRet = 0;
    DCB wParam;
    // 打开串口
    ptThis->hSerial = CreateFile(ptCfg->pchCom, ptCfg->dwAttribute, ptCfg->dwShareMode, NULL, \
                        ptCfg->dwCreationDisposition, ptCfg->dwFlagsAndAttributes, NULL);
    if (ptThis->hSerial == INVALID_HANDLE_VALUE)
    {
        printf("can not open uart\n");
        return 1;
    }
    GetCommState(ptThis->hSerial, &wParam);
    wParam.BaudRate = ptCfg->dcbSerialParams.BaudRate;
    wParam.ByteSize = ptCfg->dcbSerialParams.ByteSize;
    wParam.StopBits = ptCfg->dcbSerialParams.StopBits;
    wParam.Parity = ptCfg->dcbSerialParams.Parity;
    if (!SetCommState(ptThis->hSerial, &wParam))
    {
        printf("can not config params\n");
        CloseHandle(ptThis->hSerial);
        return 1;
    }
    // 设置串口超时
    if (!SetCommTimeouts(ptThis->hSerial, &ptCfg->timeouts))
    {
        printf("can not set timeout\n");
        CloseHandle(ptThis->hSerial);
        return 1;
    }

    // 设置串口接收事件
    memset(&ptThis->overlappedRead, 0, sizeof(ptThis->overlappedRead));
    DWORD RXeventMask = EV_RXCHAR | EV_RXFLAG;
    
    ptThis->overlappedRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(ptThis->overlappedRead.hEvent == NULL)
    {
        printf("overlappedRead.hEvent fail\n");
        return 1;
    }
    SetCommMask(ptThis->hSerial, RXeventMask);
    bRet = SetupComm(ptThis->hSerial, 100, 100);
    if(bRet == FALSE)
    {
        printf("SetupComm fail\n");
        return 1;
    }
    bRet = ReadFileEx(ptThis->hSerial, pcuart_rxdata, 100, &ptThis->overlappedRead, ReadCompletionRoutine);
    if(bRet == FALSE)
    {
        printf("ReadFileEx fail\n");
        return 1;
    }
    // 设置串口发送事件
    DWORD TXeventMask = EV_TXEMPTY;
    ptThis->overlappedWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ptThis->chTimeoutCount = 0;
    return 0;
}

int pcuart_write(pc_uart_t *ptThis, uint8_t *pchData, uint16_t hwLength)
{
/*
BOOL WriteFile(
  HANDLE       hFile,// 设备句柄
  LPCVOID      lpBuffer,//数据缓存
  DWORD        nNumberOfBytesToWrite,// 告诉设备需要写入多少字节
  LPDWORD      lpNumberOfBytesWritten,真实写入的字节
  LPOVERLAPPED lpOverlapped// 同步I/O 此参数应该为NULL，异步I/O时需要传入LPOVERLAPPED  
);
*/
    WriteFile(ptThis->hSerial, pchData, hwLength, &ptThis->bytesWritten, &ptThis->overlappedWrite);
    //GetOverlappedResult(ptThis->hSerial, &ptThis->overlappedWrite, &ptThis->dwBytesTransferred, TRUE);
    return 0;
}

int pcuart_read(pc_uart_t *ptThis, uint8_t *pchData)
{
    LPDWORD dwBytesTransferred;
    DWORD dwEvent;
    LPDWORD dwRet = 0;
    //printf("pcuart_read\n");
    #if 1

    #endif
    fflush(stdout);
    return dwRet;
}

void pcuart_Clock(pc_uart_t *ptThis)
{
    if(ptThis->chTimeoutCount)
        ptThis->chTimeoutCount--;
}