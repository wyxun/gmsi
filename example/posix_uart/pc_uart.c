#include "pc_uart.h"

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
}

int pcuart_Read(pc_uart_t *ptThis, uint8_t *pchData, uint16_t hwMaxLength)
{
    ssize_t bytesRead = read(ptThis->fd, pchData, hwMaxLength);
    if (bytesRead > 0)
    {
        printf("Read success, bytes read: %ld, data received: %.*s\n", bytesRead, (int)bytesRead, pchData);
        //write(ptThis->fd, readBuffer, bytesRead);
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

int pcuart_close(pc_uart_t *ptThis)
{
    close(ptThis->fd);
}