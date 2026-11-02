#include <time.h>
#include <errno.h>
#include "common_utils.h"

void sleep_clock(double seconds) {
    struct timespec req = {
        .tv_sec = (time_t)seconds,
        .tv_nsec = (long)((seconds - (time_t)seconds) * 1e9)
    };
    
    while (clock_nanosleep(CLOCK_MONOTONIC, 0, &req, &req) == EINTR) {
        //被中断继续
    }
}
