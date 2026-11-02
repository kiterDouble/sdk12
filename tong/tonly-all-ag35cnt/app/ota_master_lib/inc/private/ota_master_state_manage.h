/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-02-07
 * Description: OTA Master状态机管理
 *********************************************************/
#ifndef _OTA_MASTER_STATE_MANAGE_H
#define _OTA_MASTER_STATE_MANAGE_H

#include <stdint.h>
#include <string>
#include "ota_master_data.h"

/**
 * 升级任务状态
 */
enum class OtaMasterState : uint8_t {
    OTA_STATE_INIT = 0xFF,                     // 初始化状态
    OTA_STATE_IDLE = 0x00,                     // 空闲状态
    OTA_STATE_PRE_DOWNLOAD = 0x01,             // 预下载状态
    OTA_STATE_DOWNLOADING = 0x02,              // 下载状态
    OTA_STATE_DOWNLOAD_SUCCESS = 0x03,         // 下载成功状态
    OTA_STATE_DOWNLOAD_FAILED = 0x04,          // 下载失败状态
    OTA_STATE_WAITING_FOR_USER_CONFIRM = 0x05, // 等待用户确认升级状态
    OTA_STATE_CLEAR_TASK = 0x06,               // 清空任务状态
    OTA_STATE_PRE_UPGRADE = 0x07,              // 预升级状态
    OTA_STATE_UPGRADING = 0x08,                // 升级中状态
    OTA_STATE_REBOOT_TBOX = 0x09,              // TBox升级后，重启
    OTA_STATE_VERIFY_TBOX_VERSION = 0x0A,      // TBox重启后，校验TBox升级后版本是否正确
    OTA_STATE_ROLLBACK = 0x0B,                 // 失败回滚状态
    OTA_STATE_TBOX_RBK_REBOOT = 0x0C,          // TBox回滚后，重启
    OTA_STATE_TBOX_RBK_VERIFY = 0x0D,          // TBox回滚后，校验TBox回滚后版本是否正确
    OTA_STATE_UPGRADE_FAILED = 0x0E,           // 升级失败状态
    OTA_STATE_UPGRADE_SUCCESS = 0x0F,          // 升级成功状态
    OTA_STATE_MAX
};

/**
 * OTA任务ECU版本信息
 */
struct OtaTaskEcuVersionInfo {
    uint16_t ecuId = 0;     // ECU ID
    std::string swVersion;  // 软件版本
    std::string hwVersion;  // 硬件版本
    std::string supplierId; // 供应商ID
    std::string partNumber; // 零件号
};

/**
 * OTA状态机管理初始化
 *
 * @param[in] downloadPath  升级文件下载存储目录
 * @param[in] downloadFolder  升级文件下载存储目录
 * @param[in] unzipFolder  升级文件解压存储目录
 *
 * @return
 *      OTA_RET_OK：初始化成功
 *      OTA_RET_ERR：初始化失败
 */
int OtaStateManageInit(const std::string &configPath, const std::string &downloadFolder,
                       const std::string &unzipFolder);

/**
 * OTA状态机管理去初始化
 */
void OtaStateManageDeinit(void);

/**
 * 启动OTA Master状态机
 *
 * @param   无
 *
 * @return
 *      OTA_RET_OK：启动成功
 *      OTA_RET_ERR：启动失败
 */
int StartOtaStateMachine(void);

/**
 * 停止OTA Master状态机
 *
 * @param   无
 *
 * @return
 *      OTA_RET_OK：停止成功
 *      OTA_RET_ERR：停止失败
 */
int StopOtaStateMachine(void);

/**
 * 开始OTA任务
 *
 * @param[in] policyInfo OTA任务策略信息
 *
 * @return
 *      OTA_RET_OK：开始OTA任务成功
 *      OTA_RET_ERR：开始OTA任务失败
 */
int StartOtaTask(const StOtaTaskPolicyInfo *policyInfo);

/**
 * 清除OTA任务
 *
 * @param  无
 *
 * @return
 *      OTA_RET_OK：OTA任务清除成功
 *      OTA_RET_ERR：OTA任务清除失败
 */
int ClearOtaTask(void);

/**
 * 确认开始执行升级
 *
 * @param[in] taskId OTA任务ID
 *
 * @return
 *      OTA_RET_OK：执行成功
 *      OTA_RET_ERR：执行失败
 */
int ConfirmStartUpgrading(const std::string &taskId);

/**
 * 通知Master SDK ECU版本异步查询结果
 *
 * @param[in] ecuVersions   ECU版本信息
 * @param[in] ecuCnt        ECU数
 *
 * @return 无
 */
void NoticeEcuVersionAsyncQueryResult(const StEcuVersionInfo *ecuVersion, uint8_t ecuCnt);

/**
 * 通知Master SDK升级前置条件异步查询结果
 *
 * @param[in] preconditons   升级前置条件当前实际状态
 *
 * @return 无
 */
void NoticeUpgradePreconditionsAsyncQueryResult(const StUpgradePreconditions *preconditons);

/**
 * 通知结果上报有ACK
 * 
 * @param[in] taskId 任务ID
 * 
 * @return 无
 */
void NoticeReportResultHasACK(const std::string &taskId);

/**
 * 通知上下高压请求结果
 * 
 * @param[in] result    请求结果：OTA_RET_OK：请求成功；OTA_RET_ERR：请求失败
 * @param[in] powerMode 0：下高压请求 1：上高压请求
 * 
 * @return 无
 */
void NoticeUpDownHighVoltageResult(int result, OtaMasterUpgradePowerMode powerMode);

/**
 * 通知OTA模式开关切换结果
 * 
 * @param[in] result    请求结果：OTA_RET_OK：请求成功；OTA_RET_ERR：请求失败
 * @param[in] mode      OTA模式
 * 
 * @return 无
 */
void NoticeFotaModeOnOffSwResult(int result, CanSignalOtaMode mode);

/**
 * 获取OTA状态
 *
 * @param   无
 *
 * @return OtaMasterStatus
 */
OtaMasterStatus GetOtaStatus(void);

#endif // _OTA_MASTER_STATE_MANAGE_H