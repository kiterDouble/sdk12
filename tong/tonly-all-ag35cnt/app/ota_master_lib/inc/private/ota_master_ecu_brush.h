#ifndef OTA_MASTER_ECU_BRUSH_H
#define OTA_MASTER_ECU_BRUSH_H

#include <string>
#include "ota_master_config.h"
#include "ota_master_api.h"

/**
 * @brief 执行ECU固件刷新
 *
 * @param ecuId ECU ID，用于区分不同ECU的刷写流程
 * @param protocolInfo ECU刷新协议信息
 * @param driverFilePath 驱动文件路径
 * @param softwareFilePath 软件文件路径
 * @param targetVer 目标版本号（对比版本号）
 * @return  OtaUpgradeErrorCode 执行结果
 */
OtaUpgradeErrorCode ExecEcuBrush(uint16_t ecuId, const OtaTaskEcuProtocolInfo &protocolInfo, const std::string &driverFilePath,
    const std::vector<std::string> &softwareFilePaths, const std::string &targetVer);

/**
 * @brief 执行ECU固件刷新(带进度回调)
 *
 * @param ecuId ECU ID，用于区分不同ECU的刷写流程
 * @param protocolInfo ECU刷新协议信息
 * @param driverFilePath 驱动文件路径
 * @param softwareFilePath 软件文件路径
 * @param targetVer 目标版本号（对比版本号）
 * @param progressCb 进度回调函数（可为NULL）
 * @param userdata 用户数据指针
 * @return  OtaUpgradeErrorCode 执行结果
 */
OtaUpgradeErrorCode ExecEcuBrushWithProgress(uint16_t ecuId, const OtaTaskEcuProtocolInfo &protocolInfo,
    const std::string &driverFilePath, const std::vector<std::string> &softwareFilePaths,
    const std::string &targetVer, OtaMasterEcuBrushProgressCallback progressCb, void *userdata);

/**
 * @brief 设置ECU刷写进度
 *
 * @param percent 进度百分比（0-100）
 */
void SetEcuBrushProgress(uint8_t percent);

/**
 * @brief 执行特定ECU回滚
 *
 * @param protocolInfo  ECU刷新协议信息
 * @param timeOutMs 回滚超时时间，单位ms
 * @param rollbackVer 回滚版本号
 * @return OTA_RET_OK 成功，OTA_RET_ERR 失败
 */
int ExecEcuRollback(const OtaTaskEcuProtocolInfo &protocolInfo, uint16_t timeOutMs, const std::string &rollbackVer);

/**
 * @brief 验证ECU回滚版本
 *
 * @param protocolInfo  ECU刷新协议信息
 * @param version 版本号
 * @return OTA_RET_OK 成功，OTA_RET_ERR 失败
 */
int VerifySelfRollback(const OtaTaskEcuProtocolInfo &protocolInfo , const std::string &version);

#endif // OTA_MASTER_ECU_BRUSH_H
