#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

int main()
{
    int fd;
    struct termios serialAttr;
    char readBuffer[BUFFER_SIZE];
    char writeBuffer[] = "Hello, serial port!";

    // Open the serial port device
    fd = open("/dev/ttyS7", O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1)
    {
        perror("Failed to open the serial port device");
        return 1;
    }

    // Configure the serial port parameters
    if (tcgetattr(fd, &serialAttr) == -1)
    {
        perror("Failed to get the serial port parameters");
        close(fd);
        return 1;
    }
    cfmakeraw(&serialAttr);
    cfsetspeed(&serialAttr, B9600);
    serialAttr.c_cflag |= CLOCAL | CREAD;
    serialAttr.c_cc[VMIN] = 1;
    serialAttr.c_cc[VTIME] = 0;
    if (tcsetattr(fd, TCSANOW, &serialAttr) == -1)
    {
        perror("Failed to set the serial port parameters");
        close(fd);
        return 1;
    }

    // Asynchronously read data
    fcntl(fd, F_SETFL, O_RDWR | O_NONBLOCK);
    while (1)
    {
        ssize_t bytesRead = read(fd, readBuffer, BUFFER_SIZE);
        if (bytesRead > 0)
        {
            printf("Read success, bytes read: %ld, data received: %.*s\n", bytesRead, (int)bytesRead, readBuffer);
            write(fd, readBuffer, bytesRead);
        }
        else if (bytesRead < 0 && errno != EAGAIN)
        {
            perror("Error while reading from the serial port");
            break;
        }

        usleep(100000); // 100ms delay
#if 0
        // Asynchronously write data
        ssize_t bytesWritten = write(fd, writeBuffer, strlen(writeBuffer));
        if (bytesWritten > 0)
        {
            printf("Write success, bytes written: %ld, data sent: %.*s\n", bytesWritten, (int)bytesWritten, writeBuffer);
        }
        else if (bytesWritten < 0 && errno != EAGAIN)
        {
            perror("Error while writing to the serial port");
            break;
        }

        usleep(100000); // 100ms delay
#endif
    }

    // Close the serial port device
    close(fd);

    return 0;
}