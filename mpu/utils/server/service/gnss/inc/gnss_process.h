/**
 * @file gnss_process.h
 * @author jiajiangze (jiangze.jia@sirun.net)
 * @brief gnssServer 进程头文件
 * @version 0.1
 * @date 2023-12-08
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __GNSS_PROCESS_H__
#define __GNSS_PROCESS_H__
#include <glib.h>
#include <time.h>
#include <stdint.h>
#include "gnss_ipc_msg.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "mcu_api.h"
#include "pm_server_api.h"
#include "stdbool.h"
#include <glib/gthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RMC_GPS_STATUS_INITIALIZING (0)
#define RMC_GPS_STATUS_SINGLE_POINT (1)
#define RMC_GPS_STATUS_CODE_DIFF (2)
#define RMC_GPS_STATUS_INVALID_PPS (3)
#define RMC_GPS_STATUS_FIXED_SOLUTION (4)
#define RMC_GPS_STATUS_FLOAT_SOLUTION (5)
#define RMC_GPS_STATUS_ESTIMATING (6)
#define RMC_GPS_STATUS_MANUAL_INPUT (7)
#define RMC_GPS_STATUS_SIMULATION_MODE (8)
#define RMC_GPS_STATUS_WAAS_DIFF (9)

#define GPS_STATUS_NOT_POSITIONED (0)
#define GPS_STATUS_SINGLE_POINT (1)
#define GPS_STATUS_FLOAT_POINT (2)
#define GPS_STATUS_RTK_POINT (3)

#define LATITUDE_FLAG_NORTH (0) // 北纬
#define LATITUDE_FLAG_SOUTH (1<<7) // 南纬

#define LONGITUDE_FLAG_EAST (0) // 东经
#define LONGITUDE_FLAG_WEST (1<<6) // 西经

#define GNSS_ERROR_RESET_TIME (300) //GNSS状态检查重置时间

/**
 * @brief Gnss进程初始化
 * 
 */
void GpsProcessInit(void);
/**
 * @brief Gnss重新初始化
 * 
 */
void *GpsProcessReset(void *pArg);
/**
 * @brief gnssServer RPC 回调Server
 * 
 * @param subcmd 接口ID
 * @param reqData 请求数据
 * @param reqLen 请求数据长度
 * @param respData 回复数据
 * @param respLen 回复数据长度
 * @param userdata 用户指针
 */
void gnssServerCallBack(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);

/**
 * @brief 订阅pmServer休眠控制函数 休眠时关闭gnss
 * 
 * @param data ipc接收到的数据
 * @param len 接收到的数据长度
 * @param userdata 用户数据指针
 */
void HandlePmSleepCmd(const uint8_t *data, const uint16_t len, void *userdata);

/**
 * @brief 定时器回调函数 检查gnss回调是否正常
 * 
 * @param user_data 用户数据指针
 * @return true 定时器到期后重新定时
 * @return false 定时器到期后结束定时
 */
bool CheckGnssCBTimer(void* user_data);
#ifdef __cplusplus
}
#endif
#endif

