#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <mutex>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "tmier_interface.h"
#include "file_interface.h"

static long long headertime;

long long  GetHeaderTime()
{
    return headertime;
}


void SetHeaderTime(long long ms_timestamp)
{
    headertime = ms_timestamp;
}


// 获取当前时间的 ASC 格式字符串
void WriteAscHeader(FILE *file,long long ms_timestamp) 
{
    char buffer[256] = {0};

    time_t seconds = ms_timestamp / 1000;
    int milliseconds = ms_timestamp % 1000;
    struct tm *timeinfo = localtime(&seconds);
    SetHeaderTime(ms_timestamp);
    strftime(buffer, sizeof(buffer), "date %a %b %d %I:%M:%S", timeinfo);
    //printf("%s ....\n",buffer);
    snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ".%03ld %s %d", milliseconds, (timeinfo->tm_hour >= 12) ? "pm" : "am",timeinfo->tm_year + 1900);
    fprintf(file, "%s\n",buffer);
    fprintf(file, "base hex timestamps absolute\n");
    fprintf(file, "internal events logged\n");
    fprintf(file, "// version 18.3.0");
    fflush(file);
}


// 计算从基准时间到当前 CAN 帧的相对时间（秒）
double CalculateRelativeTime(long long start_time,long long current_time) 
{
    //printf("start_time:%lld  current_time:%lld \n",start_time,current_time);
    return (double)(current_time - start_time) /1000;
}


void GenerateNewFilename(long long ms_timestamp,char *buffer, size_t size) 
{
    char buf[256] = {0};
    time_t seconds = ms_timestamp / 1000;
    //printf("ms_timestamp: %lld \n",seconds);
    struct tm *tm = localtime(&seconds);
    strftime(buf, size, "can_%Y_%m_%d_%H_%M_%S.asc", tm);
    sprintf(buffer,"%s%s",DIRECTORY,buf);
    //printf("GenerateNewFilename :%s\n",buffer);
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
