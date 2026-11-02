/**
 * @file time_gnss.h
 * @author fengxiaolei (jiangze.jia@sirun.net)
 * @brief 
 * @version 1.0
 * @date 2024-02-29
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef __TIME_GNSS_H__
#define __TIME_GNSS_H__
#include <stdint.h>
#include <time.h>

/**
 * @name StartGnssTimeSync
 * @brief  开启Gnss时间同步
 * @param 
 * @return 成功返回0，失败返回小于0 
 */
int32_t StartGnssTimeSync();

/**
 * @name StopGnssTimeSync
 * @brief  关闭Gnss时间同步
 * @param 
 * @return 成功返回0，失败返回小于0 
 */
int32_t StopGnssTimeSync();

/**
 * @name GetGnssTimeSyncStatus
 * @brief  获取GNSS时间同步的状态
 * @param syncStatus 存储时间同步状态的指针
 * @return 成功返回0，失败返回小于0 
 */
int32_t GetGnssTimeSyncStatus(uint8_t *syncStatus);

/**
 * @name InitGnssIpcCallback
 * @brief  初始化GNSS IPC 回调函数
 * @param 
 * @return 成功返回0，失败返回小于0 
 */
int32_t InitGnssIpcCallback();

/**
 * @name DestoryGnssIpcCallback
 * @brief  销毁GNSS IPC 回调函数
 * @param 
 * @return 成功返回0，失败返回小于0 
 */
int32_t DestoryGnssIpcCallback();

/**
 * @name GnssTimeManageThread
 * @brief  GNSS时间管理线程函数
 * @param 
 * @return
 */
void *GnssTimeManageThread(void *arg);


#endif
