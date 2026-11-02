#include <iostream>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <mutex>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "timer_interface.h"
#include "file_interface.h"
#include "sr_log.h"



// 计算从基准时间到当前 CAN 帧的相对时间（秒）
double CalculateRelativeTime(long long start_time,long long current_time) 
{
    //printf("start_time:%lld  current_time:%lld \n",start_time,current_time);
    return (double)(current_time - start_time) /1000;
}


void GenerateNewFilename(int64_t timestamp, char *buffer, size_t size)
{
    if (!buffer || size == 0) return;

    int64_t ms_timestamp = timestamp / 1000;
    time_t seconds = ms_timestamp / 1000;
    int msec = ms_timestamp % 1000;

    /* 获取系统当前时间用于对比 */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t sys_seconds = tv.tv_sec;

    struct tm tm_info;
    localtime_r(&seconds, &tm_info);  // 线程安全

    struct tm sys_tm_info;
    localtime_r(&sys_seconds, &sys_tm_info);

    char time_buf[64];
    char sys_time_buf[64];

    // 格式化 MCU 时间
    strftime(time_buf, sizeof(time_buf),
             "can_%Y_%m_%d_%H_%M_%S",
             &tm_info);

    // 格式化系统时间
    strftime(sys_time_buf, sizeof(sys_time_buf),
             "%Y-%m-%d %H:%M:%S",
             &sys_tm_info);

    LOG_Debug("GenerateNewFilename: MCU timestamp=%ld (%s), sys time=%s, diff=%ld sec",
              (long)seconds, time_buf, sys_time_buf, (long)(seconds - sys_seconds));

    // 👉 拼接毫秒 + 路径（安全写法）
    snprintf(buffer, size, "%s%s_%03d.bin",
             DIRECTORY,
             time_buf,
             msec);
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


long long GetCurrentTime() 
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long current_time = (tv.tv_sec)*(1000) + (tv.tv_usec / 1000);
    return current_time;
}


uint64_t GetMonoUs()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000000ULL +
           ts.tv_nsec / 1000ULL;
}

void FormatTimestamp(uint64_t timestamp_us, char *buffer, size_t size)
{
    if (!buffer || size == 0) return;

    time_t seconds = timestamp_us / 1000000ULL;
    uint32_t usec = timestamp_us % 1000000ULL;

    struct tm tm_info;
    localtime_r(&seconds, &tm_info);  // 线程安全

    // 格式化为: 2026-06-01 10:25:15.123456
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm_info);

    // 追加微秒部分
    char usec_buf[16];
    snprintf(usec_buf, sizeof(usec_buf), ".%06u", usec);
    strncat(buffer, usec_buf, size - strlen(buffer) - 1);
}

