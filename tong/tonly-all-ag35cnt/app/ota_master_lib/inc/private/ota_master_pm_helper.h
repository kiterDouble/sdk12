/*************************************************
 * Copyright © 2025 SiRun (AnHui). All rights reserved.
 * File Name: ota_master_pm_helper.h
 * Author: 
 * Created Time: 2026-07-16
 * Description: OTA升级电源管理辅助模块头文件
 * Others: 
 *************************************************/

#ifndef _OTA_MASTER_PM_HELPER_H_
#define _OTA_MASTER_PM_HELPER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 休眠事件名称最大长度 */
#define PM_SLEEP_EVENT_NAME_MAX_LEN     (32)

/* 返回码定义 */
#ifndef RET_OK
#define RET_OK      (0)
#endif

#ifndef RET_ERR
#define RET_ERR     (-1)
#endif

/**
 * @brief ECU类型枚举
 * 
 * 定义支持的ECU升级类型，用于区分不同零件的休眠阻止事件
 */
typedef enum {
    ECU_TYPE_TBOX = 0,      /**< TBOX */
    ECU_TYPE_VCU,           /**< VCU (整车控制器) */
    ECU_TYPE_TMS,           /**< TMS (热管理系统) */
    ECU_TYPE_BMS,           /**< BMS (电池管理系统) */
    ECU_TYPE_MCU,           /**< MCU (微控制器) */
    ECU_TYPE_MPU,           /**< MPU (微处理器) */
    ECU_TYPE_OTHER          /**< 其他ECU */
} EcuType_e;

/**
 * @brief 获取指定ECU类型的休眠事件名称
 * 
 * @param ecuType ECU类型
 * @return const char* 对应的休眠事件名称，如果未知则返回默认事件
 */
const char* OtaMasterPmGetSleepEventName(EcuType_e ecuType);

/**
 * @brief 设置指定ECU类型的休眠阻止事件
 * 
 * 支持多个ECU类型同时设置休眠事件，不会互相覆盖。
 * 每个ECU类型独立维护自己的休眠阻止事件。
 * 
 * @param ecuType 需要升级的ECU类型
 * @return int8_t RET_OK成功，RET_ERR失败
 * 
 * @note 调用此函数后，系统将无法进入休眠，直到调用OtaMasterPmCleanSleepEvent清除事件
 */
int8_t OtaMasterPmSetSleepEvent(EcuType_e ecuType);

/**
 * @brief 清除所有已激活的休眠阻止事件
 * 
 * 遍历所有ECU类型，清除已设置的休眠事件。
 * 
 * @return int8_t RET_OK成功，RET_ERR失败
 * 
 * @note 升级结束时（无论成功或失败）必须调用此函数
 */
int8_t OtaMasterPmCleanSleepEvent(void);

/**
 * @brief 清除指定ECU类型的休眠阻止事件
 * 
 * @param ecuType ECU类型
 * @return int8_t RET_OK成功，RET_ERR失败
 */
int8_t OtaMasterPmCleanSleepEventByType(EcuType_e ecuType);

/**
 * @brief 获取ECU类型的字符串描述
 * 
 * @param ecuType ECU类型
 * @return const char* 类型名称字符串
 */
const char* OtaMasterPmGetEcuTypeString(EcuType_e ecuType);

/**
 * @brief 检查是否有休眠事件正在阻止系统休眠
 * 
 * @return int 1表示有事件阻止休眠，0表示没有
 */
int OtaMasterPmIsSleepBlocked(void);

/**
 * @brief 获取当前激活的休眠事件名称
 * 
 * 返回第一个激活的休眠事件名称（用于日志/诊断）
 * 
 * @return const char* 事件名称，如果没有则返回NULL
 */
const char* OtaMasterPmGetCurrentSleepEvent(void);

/**
 * @brief 清理所有可能的升级休眠事件（用于异常恢复）
 * 
 * 无论内部状态如何，强制清除PM服务器中所有已知的升级休眠事件。
 * 用于进程启动时的异常恢复，确保不会有残留事件阻止系统休眠。
 * 
 * @return void
 */
void OtaMasterPmCleanAllSleepEvents(void);

#ifdef __cplusplus
}
#endif

#endif /* _OTA_MASTER_PM_HELPER_H_ */
