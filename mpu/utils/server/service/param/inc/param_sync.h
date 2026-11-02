/**
 * @file param_sync.h
 * @author jiajiangze (jiangze.jia@sirun.net)
 * @brief 参数同步逻辑实现
 * @version 0.1
 * @date 2023-12-27
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __PARAM_SYNC_H__
#define __PARAM_SYNC_H__
#ifdef __cplusplus
extern "C"
{
#endif
#include <glib.h>
#include "param_msg.h"
#include "sr_param.h"
#include "modem_api.h"
#include "mcu_api.h"
#include "stdint.h"
#include "stdbool.h"

/**
 * @brief 从mcu同步数据线程函数
 * 
 * @param pArg 
 * @return void* 
 */
void* UpdateParamFromMcu(void* pArg);

/**
 * @brief 从modem同步数据线程函数
 * 
 * @param pArg 
 * @return void* 
 */
void* UpdateParamFromModem(void* pArg);

/**
 * @brief 定时器回调函数 检查与mcu参数同步情况
 * 
 * @param user_data 用户数据指针
 * @return true 定时器到期后重新定时
 * @return false 定时器到期后结束定时
 */
bool CheckParamSyncAck(void* user_data);

/**
 * @brief 向mcu同步参数
 * 
 * @param paramIndex 参数id
 * @param paramValue 参数值
 */
void SyncParamToMcu(DeviceParamEnum paramIndex, void* paramValue);

/**
 * @brief 订阅消息 MCU向MPU请求参数回调 a01m03s01
 * 
 * @param data 
 * @param len 
 * @param userdata 
 */
void HandleMcuReadParamReq(const uint8_t* data, const uint16_t len, void* userdata);

/**
 * @brief 订阅消息 MCU向MPU同步参数 a01m03s03
 * 
 * @param data 
 * @param len 
 * @param userdata 
 */
void HandleMcuSyncParam(const uint8_t* data, const uint16_t len, void* userdata);

/**
 * @brief 订阅消息 MCU向MPU回复参数查询 a01m03s02
 * 
 * @param data 
 * @param len 
 * @param userdata 
 */
void HandleMcuRespParam(const uint8_t* data, const uint16_t len, void* userdata);

/**
 * @brief 订阅消息 MCU向MPU回复参数同步结果 a01m03s04
 * 
 * @param data 
 * @param len 
 * @param userdata 
 */
void HandleMcuRespSyncParam(const uint8_t* data, const uint16_t len, void* userdata);
#ifdef __cplusplus
}
#endif

#endif
