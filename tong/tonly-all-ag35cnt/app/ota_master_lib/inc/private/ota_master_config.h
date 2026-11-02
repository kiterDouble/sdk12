/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-02-11
 * Description: OTA Master配置管理
 *********************************************************/
#ifndef _OTA_MASTER_CONFIG_H
#define _OTA_MASTER_CONFIG_H

#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "ota_master_data.h"

/**
 * OTA任务ECU刷写协议信息
 */
struct OtaTaskEcuProtocolInfo {
    uint8_t canPort = 0;     // CAN端口
    uint32_t physicalId = 0; // 物理寻址ID
    uint32_t functionId = 0; // 功能寻址ID
    uint32_t responseId = 0; // 应答ID
};

/**
 * OTA任务ECU回滚方式及超时信息
 */
struct  OtaTaskEcuRollbackInfo {
    uint16_t rollbackTimeout = 0;   // 回滚超时时间（单位：秒）
    uint8_t succRollbackMode = 0;   // 组合升级成功时的回滚方式；0：由Tbox刷写；1：通知ECU自行回滚；2：自行回滚，不需要通知
    uint8_t failRollbackMode = 0;   // 组合升级失败时的回滚方式；0：由Tbox刷写；1：通知ECU自行回滚；2：自行回滚，不需要通知
};
using OtaTaskEcuRollbackInfoMap = std::unordered_map<std::string, OtaTaskEcuRollbackInfo>;

/**
 * 支持OTA升级的ECU配置信息
 */
struct SupportOTAEcuConfigInfo {
    std::string name;                    // ECU名称
    uint16_t ecuId = 0;                  // ECU ID
    uint8_t powerMode = 0;               // 升级电源模式
    uint8_t ecuType = 0;                 // ECU类型
    OtaTaskEcuProtocolInfo protocolInfo; // 刷写协议信息
    OtaTaskEcuRollbackInfoMap rollbackInfo; // 回滚信息
};
using SupportOTAEcuConfigMap = std::unordered_map<uint16_t, SupportOTAEcuConfigInfo>;

/**
 * OTA任务ECU升级包/备份包信息
 */
struct OtaTaskEcuUpgradeFileInfo {
    uint32_t downloadSize = 0;  // 下载总大小
    std::string downloadUrl;    // 下载地址
    std::string downloadMD5;    // MD5
    std::string downloadPath;   // 存储路径
    std::string decompressFolder; // 解压路径
    uint8_t downloadState = 0;  // 下载状态：0：未下载, 1：下载中，2：下载完成
};

/**
 * OTA任务ECU件配置信息
 */
struct OtaTaskEcuConfigInfo {
    std::string ecuName;                   // ECU名称
    std::string sullpierId;                // 供应商ID
    std::string partNum;                   // 零件号
    uint16_t ecuId = 0;                    // 升级ECU ID
    uint8_t ecuType = 0;                   // ECU类型
    uint8_t upgradeOrder = 0;              // 升级顺序
    uint8_t isLocalBackup = 0;             // 是否本地备份 0：否 1：是
    OtaTaskEcuRollbackInfo rollbackInfo;   // 回滚信息
    OtaTaskEcuProtocolInfo protocolInfo;   // 刷写协议信息
    OtaTaskEcuUpgradeFileInfo upgradeFile; // 升级包信息
    OtaTaskEcuUpgradeFileInfo backupFile;  // 备份包信息
    std::string dlFolder;                  // 下载文件夹
    std::string unzipFolder;               // 解压文件夹
    std::string hwVer;                     // 硬件版本
    std::string targetVer;                 // 升级目标版本
    std::string supportVer;                // 支持升级的版本
    uint8_t upgradeState = 0;              // 升级状态：0：未升级 1：升级成功 2：升级失败
    uint8_t rollbackState = 0; // 回滚状态：0：不需回滚，1：未回滚，2：回滚成功，3：回滚失败
};

/**
 * OTA任务总配置信息
 */
struct OtaTaskConfigInfo {
    std::string taskId;           // OTA任务ID
    std::string taskName;         // OTA任务名
    std::string taskInfo;         // OTA任务信息
    std::string vehicleOtaVer;    // 整车OTA版本
    std::string logUploadAddr;    // OTA日志上传地址
    uint8_t isUploadLog = 0;      // 是否上传OTA日志
    uint8_t upgradeMode = 0;      // 升级模式（0：静默升级，1：标准升级）
    uint8_t otaState = 0;         // OTA状态，参考 OtaMasterState
    uint8_t errorCode = 0;        // OTA错误码，参考 OtaUpgradeErrorCode，仅回滚完TBox需要重启前做持久化
    uint32_t taskDuration = 0;    // OTA任务最长持续时间（单位：秒），若为0，则只考虑升级任务持续时间；
    uint32_t upgradeDuration = 0; // OTA升级持续时间（单位：秒），若taskDuration不为0，只考虑OTA任务最长持续时间
    uint32_t taskStartTime = 0;   // OTA任务开始时间（UTC时间戳，单位秒）
    std::vector<OtaTaskEcuConfigInfo> highVoltEcuTasksInfo; // 高压ECU件升级任务信息
    std::vector<OtaTaskEcuConfigInfo> lowVoltEcuTasksInfo;  // 低压ECU件升级任务信息

    void Clear()
    {
        taskId.clear();
        taskName.clear();
        taskInfo.clear();
        vehicleOtaVer.clear();
        logUploadAddr.clear();
        isUploadLog = false;
        taskDuration = 0;
        upgradeDuration = 0;
        upgradeMode = 0;
        otaState = 0;
        errorCode = 0;
        taskStartTime = 0;
        highVoltEcuTasksInfo.clear();
        lowVoltEcuTasksInfo.clear();
        highVoltEcuTasksInfo.shrink_to_fit();
        lowVoltEcuTasksInfo.shrink_to_fit();
    }
};

/**
 * 升级前置条件配置信息
 */
struct OtaPreconditionsConfigInfo {
    // 最低电压要求
    std::pair<bool, double> minBatteryVoltageReq; // first: 是否使用，second: 最低电压
    // 档位要求
    std::pair<bool, uint16_t> gearPositionReq; // first: 是否使用，second: 档位要求
    // 动力系统就绪状态要求
    std::pair<bool, uint16_t> vcuHvStatusNotReady; // first: 是否使用，second: 动力系统就绪状态为Not Ready（固定0）
    // 最高车速要求
    std::pair<bool, uint16_t> maxVehicleSpeedReq; // first: 是否使用，second: 最高车速
    // 最低电池SOC要求
    std::pair<bool, uint16_t> minBmsRealSocReq; // first: 是否使用，second: 最低动力电池SOC
    // 整车故障要求
    std::pair<bool, uint16_t> vehicleFaultReq; // first: 是否使用，second: 整车故障要求（固定0）
    // 充电枪连接状态要求
    std::pair<bool, uint16_t> chargePlugStateReq; // first: 是否使用，second: 充电枪连接状态要求（固定0）
    // 远程指令执行状态要求
    std::pair<bool, uint16_t> remoteCtrlExecutingStateReq; // first: 是否使用，second: 远程指令执行状态要求（固定0）
    // OBD诊断设备连接状态要求
    std::pair<bool, uint16_t> obdDiagDevConnStateReq; // first: 是否使用，second: OBD诊断设备连接状态要求（固定0）
    // BCM防盗认证状态要求
    std::pair<bool, uint16_t> bcmAntiTheftAuthPassedReq; // first: 是否使用，second: BCM防盗认证状态要求（固定1）
};

/**
 * OTA Master配置管理初始化
 *
 * @param[in] configPath   配置文件目录
 *
 * @return 无
 */
void OtaConfigInit(const std::string &configPath);

/**
 * OTA Master配置管理去初始化
 *
 * @param 无
 *
 * @return 无
 */
void OtaConfigDeinit(void);

/**
 * 加载可升级ECU配置信息
 *
 * @param[out] config   可升级ECU配置信息
 *
 * @return
 *      OTA_RET_OK：加载成功
 *      OTA_RET_ERR：加载失败
 */
int LoadSupportOtaEcuConfigInfo(SupportOTAEcuConfigMap &config);

/**
 * 获取已加载的可升级ECU配置信息（避免重复加载）
 *
 * @param[out] config   可升级ECU配置信息
 *
 * @return
 *      OTA_RET_OK：获取成功
 *      OTA_RET_ERR：获取失败（未初始化）
 */
int GetLoadedSupportOtaEcuConfigInfo(SupportOTAEcuConfigMap &config);

/**
 * 加载升级前置条件配置信息
 *
 * @param[out] config   升级前置条件配置信息
 *
 * @return
 *      OTA_RET_OK：加载成功
 *      OTA_RET_ERR：加载失败
 */
int LoadOtaEcuPreconditionsConfigInfo(OtaPreconditionsConfigInfo &config);

/**
 * 加载升级任务配置信息
 *
 * @param[out] config   升级任务配置信息
 *
 * @return
 *      OTA_RET_OK：加载成功
 *      OTA_RET_ERR：加载失败
 */
int LoadOtaTaskConfigInfo(OtaTaskConfigInfo &config);

/**
 * 更新并持久化升级任务配置信息
 *
 * @param[in] config   升级任务配置信息
 *
 * @return 无
 */
void UpdateOtaTaskConfig(const OtaTaskConfigInfo &config);

/**
 * 重置升级任务配置文件
 *
 * @param 无
 *
 * @return 无
 */
void ResetOtaTaskConfig(void);

/**
 * 提取OTA任务策略信息
 * 
 * @param[in] policyInfo   OTA任务策略信息
 * @param[in] supportConfig   支持OTA ECU配置信息
 * @param[out] taskConfig  OTA任务配置信息
 * 
 * @return
 *     OTA_RET_OK：成功
 *     OTA_RET_ERR：失败
 */
int ExtractOtaTaskPolicyInfo(const StOtaTaskPolicyInfo *policyInfo, const SupportOTAEcuConfigMap &supportConfig,
                             OtaTaskConfigInfo &taskConfig);
#endif // _OTA_MASTER_CONFIG_H