/*************************************************
 * Copyright © 2025 SiRun (AnHui). All rights reserved.
 * File Name: ota_master_pm_helper.c
 * Author: 
 * Created Time: 2026-07-16
 * Description: OTA升级电源管理辅助模块 - 用于管理升级期间的休眠阻止事件
 * Others: 
 *************************************************/

#include <string.h>
#include "ota_master_pm_helper.h"
#include "pm_server_api.h"
#include "sr_log.h"

/* ECU类型总数 */
#define ECU_TYPE_MAX  (ECU_TYPE_OTHER + 1)

/* 每个ECU类型对应的休眠事件设置状态（支持多ECU同时跟踪） */
static int g_sleepEventSet[ECU_TYPE_MAX] = {0};

/* 当前已设置的休眠事件数量 */
static int g_activeEventCount = 0;

/**
 * @brief 获取指定ECU类型的休眠事件名称
 * 
 * @param ecuType ECU类型
 * @return const char* 对应的休眠事件名称，如果未知则返回默认事件
 */
const char* OtaMasterPmGetSleepEventName(EcuType_e ecuType)
{
    switch (ecuType) {
        case ECU_TYPE_TBOX:
            return PM_SLEEP_EVENT_UPGRADE_TBOX;
        case ECU_TYPE_VCU:
            return PM_SLEEP_EVENT_UPGRADE_VCU;
        case ECU_TYPE_TMS:
            return PM_SLEEP_EVENT_UPGRADE_TMS;
        case ECU_TYPE_BMS:
            return PM_SLEEP_EVENT_UPGRADE_BMS;
        case ECU_TYPE_MCU:
            return PM_SLEEP_EVENT_UPGRADE_MCU;
        case ECU_TYPE_MPU:
            return PM_SLEEP_EVENT_UPGRADE_MPU;
        case ECU_TYPE_OTHER:
        default:
            return PM_SLEEP_EVENT_UPGRADE_OTHER;
    }
}

/**
 * @brief 设置指定ECU类型的休眠阻止事件
 * 
 * 支持多个ECU类型同时设置休眠事件，不会互相覆盖。
 * 如果该ECU类型的事件已经设置，直接返回，不重复调用PM API。
 * 
 * @param ecuType 需要升级的ECU类型
 * @return int8_t RET_OK成功，RET_ERR失败
 */
int8_t OtaMasterPmSetSleepEvent(EcuType_e ecuType)
{
    if (ecuType < 0 || ecuType >= ECU_TYPE_MAX) {
        LOG_Error("OTA PM: Invalid ECU type: %d", ecuType);
        return RET_ERR;
    }

    /* 如果该ECU类型的事件已经设置，直接返回 */
    if (g_sleepEventSet[ecuType]) {
        LOG_Debug("OTA PM: Sleep event for %s already set, skip",
                  OtaMasterPmGetEcuTypeString(ecuType));
        return RET_OK;
    }

    const char* eventName = OtaMasterPmGetSleepEventName(ecuType);

    /* 调用PM API设置休眠事件 */
    int8_t ret = PmSetSleepEvent(eventName);
    if (ret == RET_OK) {
        g_sleepEventSet[ecuType] = 1;
        g_activeEventCount++;
        LOG_Info("OTA PM: Set sleep event for %s: %s (active count: %d)",
                 OtaMasterPmGetEcuTypeString(ecuType), eventName, g_activeEventCount);
    } else {
        LOG_Error("OTA PM: Failed to set sleep event for %s",
                  OtaMasterPmGetEcuTypeString(ecuType));
    }

    return ret;
}

/**
 * @brief 清除所有已激活的休眠阻止事件
 * 
 * 遍历所有ECU类型，清除已设置的休眠事件。
 * 
 * @return int8_t RET_OK成功，RET_ERR失败
 * 
 * @note 升级结束时（无论成功或失败）必须调用此函数
 */
int8_t OtaMasterPmCleanSleepEvent(void)
{
    int8_t ret = RET_OK;
    int cleanedCount = 0;

    for (int i = 0; i < ECU_TYPE_MAX; i++) {
        if (g_sleepEventSet[i]) {
            const char* eventName = OtaMasterPmGetSleepEventName((EcuType_e)i);
            int8_t cleanRet = PmCleanSleepEvent(eventName);
            if (cleanRet == RET_OK) {
                LOG_Info("OTA PM: Cleaned sleep event for %s: %s",
                         OtaMasterPmGetEcuTypeString((EcuType_e)i), eventName);
                cleanedCount++;
            } else {
                LOG_Error("OTA PM: Failed to clean sleep event for %s: %s",
                          OtaMasterPmGetEcuTypeString((EcuType_e)i), eventName);
                ret = RET_ERR;
            }
            g_sleepEventSet[i] = 0;
        }
    }
    g_activeEventCount = 0;

    if (cleanedCount == 0) {
        LOG_Debug("OTA PM: No active sleep events to clean");
    } else {
        LOG_Info("OTA PM: Cleaned %d sleep events", cleanedCount);
    }

    return ret;
}

/**
 * @brief 清除指定ECU类型的休眠阻止事件
 * 
 * 如果该ECU类型的事件已设置，则清除；如果未设置，直接返回。
 * 
 * @param ecuType ECU类型
 * @return int8_t RET_OK成功，RET_ERR失败
 */
int8_t OtaMasterPmCleanSleepEventByType(EcuType_e ecuType)
{
    if (ecuType < 0 || ecuType >= ECU_TYPE_MAX) {
        LOG_Error("OTA PM: Invalid ECU type: %d", ecuType);
        return RET_ERR;
    }

    /* 如果该ECU类型的事件未设置，直接返回 */
    if (!g_sleepEventSet[ecuType]) {
        LOG_Debug("OTA PM: No active sleep event for %s, skip clean", OtaMasterPmGetEcuTypeString(ecuType));
        return RET_OK;
    }

    const char* eventName = OtaMasterPmGetSleepEventName(ecuType);
    int8_t ret = PmCleanSleepEvent(eventName);
    if (ret == RET_OK) {
        g_sleepEventSet[ecuType] = 0;
        g_activeEventCount--;
        LOG_Info("OTA PM: Cleaned sleep event for %s: %s (active count: %d)",
                 OtaMasterPmGetEcuTypeString(ecuType), eventName, g_activeEventCount);
    } else {
        LOG_Error("OTA PM: Failed to clean sleep event for %s: %s",
                  OtaMasterPmGetEcuTypeString(ecuType), eventName);
    }

    return ret;
}

/**
 * @brief 获取ECU类型的字符串描述
 * 
 * @param ecuType ECU类型
 * @return const char* 类型名称字符串
 */
const char* OtaMasterPmGetEcuTypeString(EcuType_e ecuType)
{
    switch (ecuType) {
        case ECU_TYPE_TBOX:
            return "TBOX";
        case ECU_TYPE_VCU:
            return "VCU";
        case ECU_TYPE_TMS:
            return "TMS";
        case ECU_TYPE_BMS:
            return "BMS";
        case ECU_TYPE_MCU:
            return "MCU";
        case ECU_TYPE_MPU:
            return "MPU";
        case ECU_TYPE_OTHER:
            return "OTHER";
        default:
            return "UNKNOWN";
    }
}

/**
 * @brief 检查是否有休眠事件正在阻止系统休眠
 * 
 * @return int 1表示有事件阻止休眠，0表示没有
 */
int OtaMasterPmIsSleepBlocked(void)
{
    return g_activeEventCount > 0;
}

/**
 * @brief 获取当前激活的休眠事件名称
 * 
 * 返回第一个激活的休眠事件名称（用于日志/诊断）
 * 
 * @return const char* 事件名称，如果没有则返回NULL
 */
const char* OtaMasterPmGetCurrentSleepEvent(void)
{
    for (int i = 0; i < ECU_TYPE_MAX; i++) {
        if (g_sleepEventSet[i]) {
            return OtaMasterPmGetSleepEventName((EcuType_e)i);
        }
    }
    return NULL;
}

/**
 * @brief 清理所有可能的升级休眠事件（用于异常恢复）
 * 
 * 无论内部状态如何，强制清除PM服务器中所有已知的升级休眠事件。
 * 用于进程启动时的异常恢复，确保不会有残留事件阻止系统休眠。
 * 
 * @return void
 */
void OtaMasterPmCleanAllSleepEvents(void)
{
    LOG_Info("OTA PM: Cleaning all upgrade sleep events (force cleanup)");

    /* 清除所有已定义的升级休眠事件 */
    PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE_TBOX);
    PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE_VCU);
    PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE_TMS);
    PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE_BMS);
    PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE_MCU);
    PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE_MPU);
    PmCleanSleepEvent(PM_SLEEP_EVENT_UPGRADE_OTHER);

    /* 重置内部状态 */
    memset(g_sleepEventSet, 0, sizeof(g_sleepEventSet));
    g_activeEventCount = 0;

    LOG_Info("OTA PM: All upgrade sleep events cleaned");
}
