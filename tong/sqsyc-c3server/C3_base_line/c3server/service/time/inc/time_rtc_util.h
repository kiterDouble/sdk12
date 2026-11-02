#ifndef __TIME_RTC_UTIL__
#define __TIME_RTC_UTIL__

#include <stdint.h>
#include <time.h>

/**
 * @brief 设置 RTC 硬件时间
 *
 * 打开指定 RTC 设备节点，将 UTC 时间戳转换为 rtc_time 结构后通过 ioctl(RTC_SET_TIME) 写入硬件。
 *
 * @param[in] devRtc RTC 设备节点路径（如 "/dev/rtc0"）
 * @param[in] timeStamp 要设置的 UTC 时间戳（秒）
 * @return int 0 表示成功，负值表示失败（-1:打开失败, -2:转换失败, -3:ioctl失败）
 */
int RtcTimeSet(const char* devRtc, time_t timeStamp);

/**
 * @brief 读取 RTC 硬件时间
 *
 * 打开指定 RTC 设备节点，通过 ioctl(RTC_RD_TIME) 读取硬件时间，使用 timegm() 转换为 UTC 时间戳。
 * 注意：使用 timegm() 而非 mktime()，避免本地时区影响。
 *
 * @param[in]  devRtc    RTC 设备节点路径
 * @param[out] timeStamp 输出读取到的 UTC 时间戳（秒）
 * @return int 0 表示成功，非 0 表示失败
 */
int RtcTimeGet(const char* devRtc, time_t* timeStamp);

/**
 * @brief 将 RTC 时间加载到系统时间
 *
 * 读取 RTC 硬件时间，校验时间有效性（>= 2026-01-01），通过 settimeofday() 设置到系统。
 * 通常在系统从休眠唤醒到 NORMAL 状态时调用。
 *
 * @param[in] devRtc RTC 设备节点路径
 * @return int 0 表示成功，负值表示失败
 */
int RtcTimeLoadToSystem(const char* devRtc);

#endif
