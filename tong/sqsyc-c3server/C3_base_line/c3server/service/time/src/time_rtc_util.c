#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>

#include "sr_log.h"


/** 2026-01-01 00:00:00 UTC 的时间戳，用于 RTC 时间有效性校验 */
#define TIME_STAMP_20260101 1767196800ull


/**
 * @brief 设置 RTC 硬件时间
 *
 * 打开指定 RTC 设备节点，使用 gmtime_r 将 UTC 时间戳转换为 tm 结构，
 * 再通过 ioctl(RTC_SET_TIME) 写入硬件 RTC。
 *
 * @param[in] devRtc    RTC 设备节点路径（如 "/dev/rtc0"）
 * @param[in] timeStamp 要设置的 UTC 时间戳（秒）
 * @return int 0 表示成功；负值表示失败（-1:打开失败, -2:转换失败, -3:ioctl失败）
 */
int RtcTimeSet(const char* devRtc, time_t timeStamp)
{
    struct rtc_time rtcTm;
    struct tm utcTmBuf;
    struct tm *utcTm = NULL;
    int fd = -1;
    int ret = -1;

    do {
        if (devRtc == NULL || devRtc[0] == '\0') {
            LOG_Error("[%s] devRtc is NULL or empty", __func__);
            ret = -1;
            break;
        }

        fd = open(devRtc, O_RDWR);
        if (fd < 0) {
            LOG_Error("[%s] open %s error", __func__, devRtc);
            ret = -1;
            break;
        }

        /* 将时间戳转换为struct tm（UTC时间，避免时区影响）*/
        utcTm = gmtime_r(&timeStamp, &utcTmBuf);
        if (utcTm == NULL) {
            LOG_Error("[%s] timeStamp convert error", __func__);
            ret = -2;
            break;
        }

        /* 映射到struct rtc_time（RTC设备所需格式), 不需要设置 tm_wday（星期）和 tm_yday（年内天数），RTC 会自动计算 */
        memset(&rtcTm, 0, sizeof(rtcTm));
        rtcTm.tm_sec = utcTm->tm_sec;
        rtcTm.tm_min = utcTm->tm_min;
        rtcTm.tm_hour = utcTm->tm_hour;
        rtcTm.tm_mday = utcTm->tm_mday;
        rtcTm.tm_mon = utcTm->tm_mon;
        rtcTm.tm_year = utcTm->tm_year;

        /* 通过 ioctl 设置 RTC 时间 */
        if (ioctl(fd, RTC_SET_TIME, &rtcTm) == -1) {
            LOG_Error("[%s] set time error", __func__);
            ret = -3;
            break;
        }

        LOG_Warn("[%s] set rtc(%s) UTC+0 time: %d-%02d-%02d %02d:%02d:%02d (timestamp:%ld) success",
                __func__, devRtc,
                rtcTm.tm_year + 1900,  /* 转换为实际年份 */
                rtcTm.tm_mon + 1,      /* 转换为实际月份 */
                rtcTm.tm_mday,
                rtcTm.tm_hour,
                rtcTm.tm_min,
                rtcTm.tm_sec,
                (long)timeStamp);

        ret = 0;
    } while(0);

    if (fd >= 0) {
        close(fd);
    }

    return ret;
}

/**
 * @brief 读取 RTC 硬件时间
 *
 * 打开指定 RTC 设备节点，通过 ioctl(RTC_RD_TIME) 读取硬件时间，
 * 使用 timegm() 转换为 UTC 时间戳（非 mktime()，避免本地时区影响）。
 *
 * @param[in]  devRtc    RTC 设备节点路径
 * @param[out] timeStamp 输出读取到的 UTC 时间戳（秒）
 * @return int 0 表示成功，非 0 表示失败
 */
int RtcTimeGet(const char* devRtc, time_t* timeStamp)
{
    struct tm utcTm;
    struct rtc_time rtcTm;
    int fd = -1;
    int ret = -1;

    do {
        if (devRtc == NULL || devRtc[0] == '\0') {
            LOG_Error("[%s] devRtc is NULL or empty", __func__);
            break;
        }

        if (timeStamp == NULL) {
            LOG_Error("[%s] timeStamp is NULL", __func__);
            break;
        }

        fd = open(devRtc, O_RDONLY);
        if (fd < 0) {
            LOG_Error("[%s] open %s error", __func__, devRtc);
            break;
        }

        /* 读取RTC时间 */
        if (ioctl(fd, RTC_RD_TIME, &rtcTm) == -1) {
            LOG_Error("[%s] read rtc time error, dev:%s", __func__, devRtc);
            break;
        }

        /* 映射rtc_time到tm结构体（RTC存储的是UTC时间） */
        memset(&utcTm, 0, sizeof(utcTm));
        utcTm.tm_sec = rtcTm.tm_sec;
        utcTm.tm_min = rtcTm.tm_min;
        utcTm.tm_hour = rtcTm.tm_hour;
        utcTm.tm_mday = rtcTm.tm_mday;
        utcTm.tm_mon = rtcTm.tm_mon;
        utcTm.tm_year = rtcTm.tm_year;
        utcTm.tm_isdst = 0;               // 不考虑夏令时（RTC通常用UTC）
        /* 转换为时间戳（UTC）。RTC存储的是UTC时间，须用timegm而非mktime（后者按本地时区处理） */
        *timeStamp = timegm(&utcTm);

        LOG_Warn("[%s] get rtc(%s) UTC+0 time: %d-%02d-%02d %02d:%02d:%02d (timestamp:%ld) success",
            __func__, devRtc,
            rtcTm.tm_year + 1900,
            rtcTm.tm_mon + 1,
            rtcTm.tm_mday,
            rtcTm.tm_hour,
            rtcTm.tm_min,
            rtcTm.tm_sec,
            (long)*timeStamp);

        ret = 0;
    } while(0);

    if (fd >= 0) {
        close(fd);
    }

    return ret;
}

/**
 * @brief 将 RTC 时间加载到系统时间
 *
 * 读取 RTC 硬件时间，校验时间有效性（>= 2026-01-01），通过 settimeofday() 设置到系统。
 * 通常在系统从休眠唤醒到 NORMAL 状态时由 PmStateChangeCallback 调用。
 *
 * @param[in] devRtc RTC 设备节点路径
 * @return int 0 表示成功，负值表示失败
 */
int RtcTimeLoadToSystem(const char* devRtc)
{
    struct timeval rtcTvSysTime;
    time_t rtcTimeStamp = 0;
    int ret = -1;

    do {
        if (devRtc == NULL || devRtc[0] == '\0') {
            LOG_Error("[%s] devRtc is NULL or empty", __func__);
            ret = -1;
            break;
        }

        ret = RtcTimeGet(devRtc, &rtcTimeStamp);
        if (ret != 0) {
            LOG_Error("[%s] load Rtc Error", __func__);
            ret = -1;
            break;
        }

        if (rtcTimeStamp < TIME_STAMP_20260101) {
            LOG_Error("[%s] invalid Rtc Time", __func__);
            ret = -1;
            break;
        }

        rtcTvSysTime.tv_sec = rtcTimeStamp;
        rtcTvSysTime.tv_usec = 0;
        ret = settimeofday(&rtcTvSysTime, NULL);
        LOG_Warn("[%s] load Rtc Time to system ret:%d", __func__, ret);
    } while(0);

    return ret;
}
