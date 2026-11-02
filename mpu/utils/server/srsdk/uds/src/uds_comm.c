#include <sys/time.h>
#include <time.h>
#include "uds_comm.h"

/**
 * @name UDSMillis
 * @brief 获取当前毫秒数(使用单调时钟)。
 * @param null
 * @return 返回当前时间的毫秒表示。返回值为uint32_t类型，即无符号32位整数。
 */
uint32_t UDSMillis(void)
{
    struct timespec outtime;
    long long milliseconds;
    clock_gettime(CLOCK_MONOTONIC, &outtime);
    milliseconds = (outtime.tv_sec * 1000LL) + (outtime.tv_nsec / 1000000);
    return milliseconds;
}

/**
 * @name SleepMillis
 * @brief 以毫秒为单位进行睡眠。
 * @param tms 指定睡眠的毫秒数。
 * @return 返回0表示成功，非0表示失败。 
 */
int32_t SleepMillis(uint32_t tms)
{
    struct timespec ts;
    int ret;

    ts.tv_sec = tms / 1000;
    ts.tv_nsec = (tms % 1000) * 1000000;
    do
    {
        ret = nanosleep(&ts, &ts);
    } 
    while (ret && (errno == EINTR));

    return ret;
}