/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-02-25
 * Description: OTA Master计时器
 *********************************************************/
#ifndef _OTA_MASTER_TIMER_H
#define _OTA_MASTER_TIMER_H

#include <stdint.h>

/**
 * @brief OTA Master计时器回调函数
 * 
 * @param 无
 * 
 * @return 无
 */
typedef void (*OtaTaskTimeoutCallbackFunc)(void);

/**
 * @brief 启动OTA任务超时监控
 * 
 * @param[in] startTime 开始时间（unix时间戳，单位秒）
 * @param[in] duration 任务持续时间（单位秒）
 * @param[in] callback OTA任务超时回调函数
 * 
 * @return 无
 */
void StartOtaTaskTimeoutMonitor(uint32_t startTime, uint32_t duration, const OtaTaskTimeoutCallbackFunc callback);

/**
 * @brief 停止OTA任务超时监控
 * 
 * @param 无
 * 
 * @return 无
 */
void StopOtaTaskTimeoutMonitor(void);

#endif // _OTA_MASTER_TIMER_H