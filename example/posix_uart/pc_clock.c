#include "pc_clock.h"


time_t timer;
extern void timer_handler(int signum);
int pcclock_Init(void)
{
    struct sigevent sev;
    struct itimerspec its;

    // 创建定时器
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    sev.sigev_value.sival_ptr = &timer;
    if (timer_create(CLOCK_REALTIME, &sev, &timer) == -1)
    {
        perror("Failed to create timer");
        return 1;
    }

    // 设置定时器间隔
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = INTERVAL_MS * 1000000; // 将毫秒转换为纳秒
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = INTERVAL_MS * 1000000;

    // 启动定时器
    if (timer_settime(timer, 0, &its, NULL) == -1)
    {
        perror("Failed to set timer");
        return 1;
    }

    // 注册定时器处理函数
    signal(SIGALRM, timer_handler);
}

int pcclock_Delete(void)
{
    timer_delete(timer);
}