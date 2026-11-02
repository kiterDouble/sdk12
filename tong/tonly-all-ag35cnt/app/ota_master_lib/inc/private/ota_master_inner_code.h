/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-02-07
 * Description: OTA Master内部错误码
 *********************************************************/
#ifndef _OTA_MASTER_INNER_ERROR_CODE_H
#define _OTA_MASTER_INNER_ERROR_CODE_H

#include <stdint.h>

/**
 * OTA文件下载状态
 */
enum OtaFileDownloadStatus : uint8_t {
    OTA_FILE_NOT_DOWNLOAD = 0,      // 升级文件未下载
    OTA_FILE_DOWNLOADING = 1,       // 升级文件下载中
    OTA_FILE_DOWNLOAD_SUCCESS = 2,  // 升级文件下载成功
    OTA_FILE_DOWNLOAD_FAILED = 3,   // 升级文件下载失败
};

/**
 * OTA文件下载错误码
 */
enum OtaFileDownloadErrorCode : uint8_t {
    OTA_DOWNLOAD_OK = 0x0,                         // 车辆处于READY状态
    OTA_DOWNLOAD_NETWORK_ABNORMAL = 0x01,          // 网络异常
    OTA_DOWNLOAD_TIMEOUT = 0x02,                   // 下载超时
    OTA_DOWNLOAD_CURL_ERROR = 0x03,                // CURL异常
    OTA_DOWNLOAD_SPACE_NOT_ENOUGH = 0x04,          // 存储空间不足
    OTA_DOWNLOAD_INVALID_ADDRESS = 0x05,           // 无效地址
    OTA_DOWNLOAD_TASK_TIMEOUT = 0x06,              // 任务超时
    OTA_DOWNLOAD_FOLDER_EMPTY = 0x07,              // 文件夹为空
    OTA_DOWNLOAD_FILE_PATH_ERROR = 0x08,           // 文件路径错误
    OTA_DOWNLOAD_STOPED = 0x09,                    // 下载被停止
    OTA_DOWNLAOD_MD5_CHECK_FAILED = 0x0A,          // MD5校验失败
    OTA_DOWNLOAD_UNKNOWN_ERROR = 0xFF              // 未知错误
};

/**
 * OTA文件升级状态
 */
enum OtaEcuUpgradeStatus : uint8_t {
    OTA_ECU_NOT_UPGRADE = 0,      // 升级文件未升级
    OTA_ECU_UPGRADE_SUCCESS = 1,  // 升级文件升级成功
    OTA_ECU_UPGRADE_FAILED = 2,   // 升级文件升级失败
};

/**
 * OTA ECU回滚状态
 */
enum OtaEcuRollbackStatus : uint8_t {
    E_ROLLBACK_STATUS_NOT_ROLLBACK = 0, // ECU未回滚
    E_ROLLBACK_STATUS_SUCCESS = 1,      // ECU回滚成功
    E_ROLLBACK_STATUS_FAILED = 2,       // ECU回滚失败
    E_ROLLBACK_STATUS_NO_NEED = 3       // ECU无需回滚
};

/**
 * ECU版本回滚方式
 */
enum OtaRollbackMode : uint8_t {
    E_ROLLBACK_MODE_BY_TBOX = 0,  // TBox触发回滚
    E_ROLLBACK_MODE_BY_AB_SW = 1, // 发31通知自行回滚
    E_ROLLBACK_MODE_NO_NEED = 2   // 不需要执行回滚，由ECU内部处理
};

/**
 * OTA ECU类型
 */
enum OtaEcuType : uint8_t {
    E_OTA_ECU_TYPE_TBOX = 0, // TBox
    E_OTA_ECU_TYPE_BCM = 1,  // BCM
    E_OTA_ECU_TYPE_BMS = 2,  // BMS
    E_OTA_ECU_TYPE_VCU = 3,  // VCU
    E_OTA_ECU_TYPE_OTHER = 4 // 其他ECU
};

#endif // _OTA_MASTER_INNER_ERROR_CODE_H