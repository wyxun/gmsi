#include <unistd.h>
#include <stdint.h>
#include "utilities/util_debug.h"
#include <signal.h>
#include <time.h>
#include "gmsi.h"
#include "lower.h"
#include "upper.h"
#include "userconfig.h"

#define INTERVAL_MS 1

timer_t timerid;
void timer_handler(int signum);
void StorageWrite(uint16_t *phwStorageStartAddr, uint16_t hwStorageLength);
void StorageRead(uint16_t *phwStorageStartAddr, uint16_t hwStorageLength);

uint16_t g_hwSystemDataArrary[16];
gstorage_data_t tSysData = {
    .phwStorageStartAddr = g_hwSystemDataArrary,
    .hwStorageLength = 16,
    .hwCrcFlag = 0,
    .fcnWrite = StorageWrite,
    .fcnRead = StorageRead,
};

gmsi_t tGmsi = {&tSysData};

lower_cfg_t tLowerCfg = {
    .pchCom = "/dev/ttyS1",
    .wOflag = O_RDWR | O_NOCTTY | O_NONBLOCK,
};
lower_t tLower;

upper_cfg_t tUpperCfg = {

};
upper_t tUpper;

void systemclock_Init(void)
{
    struct sigaction sa;
    struct itimerspec its;

    sa.sa_handler = timer_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);

    timer_create(CLOCK_REALTIME, NULL, &timerid);

    its.it_value.tv_sec = INTERVAL_MS / 1000;
    its.it_value.tv_nsec = (INTERVAL_MS % 1000) * 1000000;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;

    timer_settime(timerid, 0, &its, NULL);
}
int main(int argc, char **argv)
{
    systemclock_Init();

    lower_Init((uintptr_t)&tLower, (uintptr_t)&tLowerCfg);
    upper_Init((uintptr_t)&tUpper, (uintptr_t)&tUpperCfg);

    gmsi_Init(&tGmsi);
    GLOG_PRINTF("OK");
    while(1)
    {
        gmsi_Run();
    }
    return 0;
}

void timer_handler(int signum)
{
    static uint16_t timeout = 0;
    if(!timeout)
    {
        timeout = 4999;
        //GLOG_PRINTF("timeout");
        gbase_EventPost(LOWER, Event_PacketReceived);
    }
    else {
        timeout--;
    }
    gmsi_Clock();
}

void StorageWrite(uint16_t *phwStorageStartAddr, uint16_t hwStorageLength)
{
    // Write flash save sys_data programme
}
void StorageRead(uint16_t *phwStorageStartAddr, uint16_t hwStorageLength)
{
    // Write flash read sys_data programme
}