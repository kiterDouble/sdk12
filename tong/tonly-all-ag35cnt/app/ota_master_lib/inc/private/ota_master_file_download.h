/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-02-19
 * Description: OTA Master升级文件下载
 *********************************************************/
#ifndef _OTA_MASTER_FILE_DOWNLOAD_H
#define _OTA_MASTER_FILE_DOWNLOAD_H

#include <stdint.h>
#include <string>
#include <functional>
#include "ota_master_config.h"
#include "ota_master_inner_code.h"

using DownloadingCallback = std::function<void(OtaTaskEcuUpgradeFileInfo &fileInfo)>;

/**
 * 下载OTA升级文件
 * 
 * @param[in] downloadingCb 下载中回调
 * @param[out] config       ECU配置信息
 * 
 * @return 下载错误码
 */
OtaFileDownloadErrorCode StartDownloadOtaUpgradeFile(DownloadingCallback downloadingCb, OtaTaskEcuConfigInfo &config);

/**
 * 下载OTA备份文件
 *
 * @param[in] downloadingCb 下载中回调
 * @param[out] config       ECU配置信息
 *
 * @return 下载错误码
 */
OtaFileDownloadErrorCode StartDownloadOtaBackupFile(DownloadingCallback downloadingCb, OtaTaskEcuConfigInfo &config);

/**
 * 停止OTA文件下载
 */
void StopOtaFileDownload(void);

/**
 * OTA任务升级完成后，将升级文件进行备份，用于下次OTA失败回滚用
 * 
 * @param[in] taskConfig OTA任务配置信息
 */
void BackupDownloadedOtaFile(const OtaTaskConfigInfo &taskConfig);

/**
 * 删除已下载的文件
 * 
 * @param[in] taskConfig OTA任务配置信息
 * @param[in] isDelLocalBackupFile 是否删除本地备份文件
 */
void DeleteDownloadedOtaFile(const OtaTaskConfigInfo &taskConfig, bool isDelLocalBackupFile);
#endif // _OTA_MASTER_FILE_DOWNLOAD_H