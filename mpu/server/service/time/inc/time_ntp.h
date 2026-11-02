/**
 * @file time_ntp.h
 * @author fengxiaolei (jiangze.jia@sirun.net)
 * @brief 
 * @version 1.0
 * @date 2024-02-29
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef __TIME_NTP_H__
#define __TIME_NTP_H__
#include <stdint.h>

/**
 * @name GetNtpSyncStatus
 * @brief  获取NTP时间同步状态
 * @param ntpStatus  用于存储NTP状态的指针
 * @return 成功返回0，失败返回小于0 
 */
int32_t GetNtpSyncStatus(uint8_t *ntpStatus);

/**
 * @name ChronyManageThread
 * @brief  NTP时间同步管理线程函数
 * @param 
 * @return 
 */
void *ChronyManageThread(void *arg);
/**
 * @name TimeGetPmPowerState
 * @brief 从给定数据中获取PM电源状态
 * @param data 指向包含PM电源状态数据的缓冲区的指针。
 * @param len 数据缓冲区的长度。
 * @param userdata 用户自定义数据的指针，本函数中未使用。
 * @return 无返回值 
 */
void TimeGetPmPowerState(const uint8_t* data, const uint16_t len, void* userdata);
#endif