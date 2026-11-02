
/**
 * @file time_status_api.h
 * @brief 时间状态外部 API 接口头文件
 * @version 1.0
 * @date 2024-02-29
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef __TIME_STATUS_API_H__
#define __TIME_STATUS_API_H__
#include <stdint.h>
#include "time_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name InitGetTimeSyncStatusCallBack
 * 初始化获取时间同步状态回调函数
 * 
 * 该函数用于初始化并注册一个用于接收时间同步状态通知的消息订阅回调。
 * 
 * @return int32_t 返回订阅操作的结果状态码。成功返回正值，失败返回负值。
 */

int32_t InitGetTimeSyncStatusCallBack();

/**
 * @name GetTimeSyncStatus
 * 获取时间同步状态
 * 
 * 本函数用于获取当前时间同步的状态信息，包括时间同步的状态和来源。
 * 
 * @param timeSync 指向MpuTimeSync_t结构体的指针，用于存放时间同步的状态和来源信息。
 * @return 返回函数执行状态，如果执行成功返回RET_OK，如果timeSync参数为NULL返回RET_ERR。
 */
int32_t GetTimeSyncStatus(MpuTimeSync_t *timeSync);

/**
 * @name SetTspSyncTime
 * @brief 向时间管理服务上报 TSP 时间戳
 *
 * 通过同步 RPC 调用将 TSP 模块获取的时间戳推送给 timeServer。
 * timeServer 校验优先级条件后决定是否采纳该时间进行系统授时。
 *
 * @param[in] timeStamp TSP UTC 时间戳（秒）
 * @return int32_t 返回操作结果：0 表示 RPC 调用成功（不代表时间被采纳），负值或 RET_ERR 表示失败
 */
int32_t SetTspSyncTime(time_t timeStamp);

/**
 * @name DestoryGetTimeSyncStatusCallBack
 * 销毁获取时间同步状态回调函数
 * 
 * 该函数用于取消订阅系统时间状态通知的消息队列消息。执行该操作后，将不再接收时间同步状态的更新通知。
 * 
 * @return int32_t 返回操作的结果状态码。成功返回0，失败返回非0值。
 */
int32_t DestoryGetTimeSyncStatusCallBack();

/**
 * @brief 获取TSP校时源优先级
 *
 * 通过同步 RPC 调用获取 TSP 模块优先级。
 *
 * @return int TSP校时源优先级
 */
int GetTSPPriority(void);

#ifdef __cplusplus
}
#endif

#endif