#include <time.h>
#include <glib.h>
#include <errno.h>
#include <string.h>
#include"system_interface.h"
#include "timer_interface.h"


uint64_t GetCurrentTimeMS() 
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


void DelayMS(unsigned int time) 
{
    struct timespec req = {
        .tv_sec  = 0,          // 秒
        .tv_nsec = time * 1000000 // 纳秒 (10ms = 10,000,000ns)
    };
    struct timespec rem;
    
    while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
        req = rem; // 被信号中断后继续剩余延时
    }
}
