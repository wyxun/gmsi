/****************************************************************************
*  Copyright 2024 Gorgon Meducer (Email:embedded_zhuoran@hotmail.com)       *
*                                                                           *
*  Licensed under the Apache License, Version 2.0 (the "License");          *
*  you may not use this file except in compliance with the License.         *
*  You may obtain a copy of the License at                                  *
*                                                                           *
*     http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                           *
*  Unless required by applicable law or agreed to in writing, software      *
*  distributed under the License is distributed on an "AS IS" BASIS,        *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
*  See the License for the specific language governing permissions and      *
*  limitations under the License.                                           *
*                                                                           *
****************************************************************************/

/*============================ INCLUDES ======================================*/
#undef __PERF_COUNT_PLATFORM_SPECIFIC_HEADER__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#define __IMPLEMENT_PERF_COUNTER
#include "perf_counter.h"

#if defined(__IS_COMPILER_GCC__)
#   pragma GCC diagnostic ignored "-Wattributes"
#endif

#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wunknown-warning-option"
#   pragma clang diagnostic ignored "-Wreserved-identifier"
#   pragma clang diagnostic ignored "-Wconditional-uninitialized"
#   pragma clang diagnostic ignored "-Wcast-align"
#   pragma clang diagnostic ignored "-Wmissing-prototypes"
#   pragma clang diagnostic ignored "-Wimplicit-function-declaration"
#endif


/*============================ MACROS ========================================*/
#define PERFC_TIMER_FREQ_HZ     1000000000UL    /* 1 GHz (nanoseconds) */

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
static struct timespec s_tStartTime = {0};
static bool s_bTimerInitialized = false;

/*============================ PROTOTYPES ====================================*/
/* low level interface for porting */
extern
uint32_t perfc_port_get_system_timer_freq(void);
extern
int64_t perfc_port_get_system_timer_top(void);
extern
bool perfc_port_is_system_timer_ovf_pending(void);
extern
bool perfc_port_init_system_timer(bool bTimerOccupied);
extern
int64_t perfc_port_get_system_timer_elapsed(void);
extern
void perfc_port_clear_system_timer_ovf_pending(void);
extern
void perfc_port_stop_system_timer_counting(void);
extern
void perfc_port_clear_system_timer_counter(void);

/*============================ IMPLEMENTATION ================================*/

#if __PERFC_USE_USER_CUSTOM_PORTING__
 
bool perfc_port_init_system_timer(bool bIsTimeOccupied)
{
    if (!s_bTimerInitialized) {
        /* Get the initial timestamp */
        clock_gettime(CLOCK_MONOTONIC, &s_tStartTime);
        s_bTimerInitialized = true;
    }
    return true;
}

uint32_t perfc_port_get_system_timer_freq(void)
{
    /* Return 1 GHz (nanosecond resolution) */
    return PERFC_TIMER_FREQ_HZ;
}

bool perfc_port_is_system_timer_ovf_pending(void)
{
    /* POSIX monotonic clock doesn't overflow in practical use */
    return false;
}

int64_t perfc_port_get_system_timer_top(void)
{
    /* No top value for POSIX monotonic timer - return max int64 */
    return INT64_MAX;
}

int64_t perfc_port_get_system_timer_elapsed(void)
{
    struct timespec tNow;
    clock_gettime(CLOCK_MONOTONIC, &tNow);
    
    /* Calculate elapsed time in nanoseconds */
    int64_t lElapsedNs = (int64_t)(tNow.tv_sec - s_tStartTime.tv_sec) * 1000000000LL
                       + (int64_t)(tNow.tv_nsec - s_tStartTime.tv_nsec);
    
    return lElapsedNs;
}

void perfc_port_clear_system_timer_ovf_pending(void)
{
    /* No overflow to clear for POSIX monotonic clock */
}

void perfc_port_stop_system_timer_counting(void)
{
    /* Cannot stop POSIX system clock - no-op */
}

void perfc_port_clear_system_timer_counter(void)
{
    /* Reset the reference timestamp */
    if (s_bTimerInitialized) {
        clock_gettime(CLOCK_MONOTONIC, &s_tStartTime);
    }
}

__attribute__((noinline))
uintptr_t __perfc_port_get_sp(void)
{
#if defined(__x86_64__) || defined(__amd64__)
    uintptr_t result;
    __asm__ volatile ("mov %%rsp, %0" : "=r" (result));
    return result;
#elif defined(__i386__)
    uintptr_t result;
    __asm__ volatile ("mov %%esp, %0" : "=r" (result));
    return result;
#elif defined(__aarch64__)
    uintptr_t result;
    __asm__ volatile ("mov %0, sp" : "=r" (result));
    return result;
#elif defined(__arm__)
    uintptr_t result;
    __asm__ volatile ("mov %0, sp" : "=r" (result));
    return result;
#else
    /* Fallback: return 0 for unsupported architectures */
    return 0;
#endif
}

__attribute__((noinline))
void __perfc_port_set_sp(uintptr_t nSP)
{
#if defined(__x86_64__) || defined(__amd64__)
    __asm__ volatile ("mov %0, %%rsp" : : "r" (nSP));
#elif defined(__i386__)
    __asm__ volatile ("mov %0, %%esp" : : "r" (nSP));
#elif defined(__aarch64__)
    __asm__ volatile ("mov sp, %0" : : "r" (nSP));
#elif defined(__arm__)
    __asm__ volatile ("mov sp, %0" : : "r" (nSP));
#else
    /* Fallback: no-op for unsupported architectures */
    (void)nSP;
#endif
}

/* Override the weak function in perf_counter.c to reduce CPU usage */
bool perfc_delay_ms_user_code_in_loop(int64_t lRemainInMs)
{
    /* Sleep for a short time to reduce CPU usage during delay loops */
    if (lRemainInMs > 1) {
        struct timespec ts, remaining;
        /* Sleep for 1ms or remaining time, whichever is smaller */
        int64_t lSleepMs = (lRemainInMs > 10) ? 1 : 0;
        ts.tv_sec = lSleepMs / 1000;
        ts.tv_nsec = (lSleepMs % 1000) * 1000000L + 100000L; /* 0.1ms minimum */
        
        while (nanosleep(&ts, &remaining) == -1) {
            if (errno == EINTR) {
                ts = remaining;
            } else {
                break;
            }
        }
    }
    return true;
}
#endif


