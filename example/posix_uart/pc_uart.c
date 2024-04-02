#include "pc_uart.h"

gmsi_base_cfg_t tUartBaseCfg = {
    .wId = GMSI_ID_UART << 8 + 1,
};

static gmsi_base_t tBase;

uint8_t chReceiveData[100];
int pcuart_Init(pc_uart_t *ptThis, pc_uart_cfg_t *ptCfg)
{
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
    ptThis->ptBase = &tBase;
    gbase_Init(ptThis->ptBase, &tUartBaseCfg);
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

int pcuart_Run(pc_uart_t *ptThis)
{
    int16_t hwLength = 0;
    uint32_t wEvent;
    hwLength = pcuart_Read(ptThis, chReceiveData, 100);
    if(hwLength > 0)
    {
        gbase_MessageSet(GMSI_ID_TIMER<<8+1, chReceiveData, hwLength);
    }

    wEvent = gbase_EventGet(ptThis->ptBase);
    if(wEvent & Gmsi_Event00)
    {
        // 收到事件Gmsi_Event00
        printf("get clock event00\n");
    }
    if(wEvent & Gmsi_Event_Transition)
    {
        // 收到消息
    }

}
int pcuart_close(pc_uart_t *ptThis)
{
    return close(ptThis->fd);
}