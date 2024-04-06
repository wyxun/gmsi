#include "pc_uart.h"
#include "userconfig.h"
#include <stdint.h>

int pcuart_Run(uint32_t wObjectAddr);
int pcuart_Clock(uint32_t addr);
gmsi_base_cfg_t tUartBaseCfg = {
    .wId = PC_UART,
    .wParent = 0,
    .FcnInterface = {
        .Clock = pcuart_Clock,
        .Run = pcuart_Run,
    },
};

static gmsi_base_t tBase;

uint8_t chReceiveData[100];
//int pcuart_Init(pc_uart_t *ptThis, pc_uart_cfg_t *ptCfg)
int pcuart_Init(uint32_t wObjectAddr, uint32_t wObjectCfgAddr)
{
    pc_uart_t *ptThis = (pc_uart_t *)wObjectAddr;
    pc_uart_cfg_t *ptCfg = (pc_uart_cfg_t *)wObjectCfgAddr;

    // Open the serial port device
    ptThis->fd = open(ptCfg->pchCom, ptCfg->wOflag);
    if (ptThis->fd == -1)
    {
        perror("Failed to open the serial port device");
        return 1;
    }

    // Configure the serial port parameters
    if (tcgetattr(ptThis->fd, &ptCfg->serialAttr) == -1)
    {
        perror("Failed to get the serial port parameters");
        close(ptThis->fd);
        return 1;
    }

    cfmakeraw(&ptCfg->serialAttr);
    cfsetspeed(&ptCfg->serialAttr, B9600);
    ptCfg->serialAttr.c_cflag |= CLOCAL | CREAD;
    ptCfg->serialAttr.c_cc[VMIN] = 1;
    ptCfg->serialAttr.c_cc[VTIME] = 0;
    if (tcsetattr(ptThis->fd, TCSANOW, &ptCfg->serialAttr) == -1)
    {
        perror("Failed to set the serial port parameters");
        close(ptThis->fd);
        return 1;
    }
    // Asynchronously read data
    fcntl(ptThis->fd, F_SETFL, O_RDWR | O_NONBLOCK);

    tUartBaseCfg.wParent = wObjectAddr;
    ptThis->ptBase = &tBase;
    if(GMSI_SUCCESS != gbase_Init(ptThis->ptBase, &tUartBaseCfg))
        printf("pcuart_Init fail\n");
    return 0;
}

int pcuart_Read(pc_uart_t *ptThis, uint8_t *pchData, uint16_t hwMaxLength)
{
    ssize_t bytesRead = read(ptThis->fd, pchData, hwMaxLength);
    if (bytesRead > 0)
    {
        //printf("Read success, bytes read: %ld, data received: %.*s\n", bytesRead, (int)bytesRead, pchData);
        //write(ptThis->fd, pchData, bytesRead);
    }
    else if (bytesRead < 0 && errno != EAGAIN)
    {
        perror("Error while reading from the serial port");
    }
    return bytesRead;
}

int pcuart_Write(pc_uart_t *ptThis, uint8_t *pchData, uint16_t hwLength)
{
    int wRet = write(ptThis->fd, pchData, hwLength);

    return wRet;
}

int pcuart_Run(uint32_t wObjectAddr)
{
    int16_t hwLength = 0;
    uint32_t wEvent;
    pc_uart_t *ptThis = (pc_uart_t *)wObjectAddr;
    GMSI_ASSERT(NULL != ptThis);

    hwLength = pcuart_Read(ptThis, chReceiveData, 100);
    if(hwLength > 0)
    {
        gbase_MessagePost(PC_CLOCK, chReceiveData, hwLength);
    }

    wEvent = gbase_EventPend(ptThis->ptBase);
    if(wEvent & Gmsi_Event00)
    {
        // 收到事件Gmsi_Event00
        printf("get clock event00\n");
    }
    if(wEvent & Gmsi_Event_Transition)
    {
        // 收到消息
    }
    return 0;
}
int pcuart_close(pc_uart_t *ptThis)
{
    return close(ptThis->fd);
}

int pcuart_Clock(uint32_t wObjectAddr)
{
    static uint32_t s_count = 0;
    pc_uart_t *ptThis = (pc_uart_t *)wObjectAddr;

    s_count++;
    if(s_count > 5000)
    {
        printf("pc_uart clock\n");
        s_count = 0;
    }
    return 0;
}