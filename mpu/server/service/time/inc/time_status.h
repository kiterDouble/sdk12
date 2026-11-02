/**
 * @file time_status.h
 * @author fengxiaolei (jiangze.jia@sirun.net)
 * @brief 
 * @version 1.0
 * @date 2024-02-29
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef __TIME_STATUS_H__
#define __TIME_STATUS_H__
#include <stdint.h>

#define TIME_SYNC_NOT_OK        (1)
#define TIME_SYNC_OK            (2)

typedef enum
{
    E_TIME_SYNC_SRC_NONE = 0x00,
    E_TIME_SYNC_SRC_NTP  = 0x01,
    E_TIME_SYNC_SRC_GNSS = 0x02,
}TimeSyncSource_e;

typedef struct 
{
    uint8_t timeSyncState;
    TimeSyncSource_e timeSyncSrc;
}MpuTimeSync_t;



#endif