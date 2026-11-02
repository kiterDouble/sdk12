/**
 * @file time_status.h
 * @brief 时间同步状态定义头文件
 * @version 1.0
 * @date 2024-02-29
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef __TIME_STATUS_H__
#define __TIME_STATUS_H__
#include <stdint.h>

/** 时间未同步状态 */
#define TIME_SYNC_NOT_OK        (1)
/** 时间已同步状态 */
#define TIME_SYNC_OK            (2)

/**
 * @brief 时间同步源枚举
 */
typedef enum
{
    E_TIME_SYNC_SRC_NONE = 0x00,    /**< 无授时源 */
    E_TIME_SYNC_SRC_NTP  = 0x01,    /**< NTP 网络授时 */
    E_TIME_SYNC_SRC_GNSS = 0x02,    /**< GNSS 卫星授时 */
    E_TIME_SYNC_SRC_TSP  = 0x03,    /**< TSP 平台授时 */
}TimeSyncSource_e;

/**
 * @brief 时间同步结果枚举（保留兼容）
 */
typedef enum
{
    E_TIME_SYNC_FAIL         = 0x00,    /**< 授时失败 */
    E_TIME_SYNC_NTP_SUCCESS  = 0x01,    /**< NTP 授时成功 */
    E_TIME_SYNC_GNSS_SUCCESS = 0x02,    /**< GNSS 授时成功 */
}TimeSyncResult_e;

/**
 * @brief MPU 时间同步状态结构体
 *
 * 用于广播时间同步状态到 MCU 和 IPC 订阅者。
 */
typedef struct
{
    uint8_t timeSyncState;          /**< 时间同步状态：TIME_SYNC_NOT_OK(1) 或 TIME_SYNC_OK(2) */
    TimeSyncSource_e timeSyncSrc;   /**< 当前授时源类型 */
    uint8_t priority;                /**< 当前授时源优先级 */
}MpuTimeSync_t;



#endif
