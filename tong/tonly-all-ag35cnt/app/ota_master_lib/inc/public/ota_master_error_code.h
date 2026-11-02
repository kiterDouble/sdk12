/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-02-07
 * Description: 错误码定义
 *********************************************************/
#ifndef _OTA_MASTER_ERROR_CODE_H
#define _OTA_MASTER_ERROR_CODE_H

//执行结果
#define OTA_RET_OK              (0)     //成功
#define OTA_RET_ERR             (-1)    //失败
#define OTA_RET_TIMEOUT         (-99)   //超时


// ==================== ECU刷写流程定义 ====================
// 支持的ECU ID定义
#define ECU_ID_VCU    0x7DC    // VCU
#define ECU_ID_TMS    0x67C    // TMS
#define ECU_ID_TMS_B  0x68C    // TMS_B (新增)
#define ECU_ID_TCU    0x03F1   // TCU
// MCU1 系列
#define ECU_ID_MCU1_TM1  0xD2FA   // MCU1_TM1 (站号2, CAN3动力)
#define ECU_ID_MCU1_TM2  0xD9FA   // MCU1_TM2 (站号9, CAN3动力)
#define ECU_ID_MCU1_PDU  0xD6FA   // MCU1_PDU (站号6, CAN1整车)
#define ECU_ID_MCU1_EPS  0xD4FA   // MCU1_EPS (站号4, CAN1整车)
#define ECU_ID_MCU1_ACM  0xD5FA   // MCU1_ACM (站号5, CAN1整车)
#define ECU_ID_MCU1_DCDC 0xD7FA   // MCU1_DCDC (站号7, CAN1整车)
#define ECU_ID_MCU1      ECU_ID_MCU1_TM1  // 兼容旧名,默认TM1
// MCU2 系列
#define ECU_ID_MCU2_TM1  0xD2FB   // MCU2_TM1 (站号2, CAN3动力)
#define ECU_ID_MCU2_TM2  0xD9FB   // MCU2_TM2 (站号9, CAN3动力)
#define ECU_ID_MCU2_PDU  0xD6FB   // MCU2_PDU (站号6, CAN1整车)
#define ECU_ID_MCU2_EPS  0xD4FB   // MCU2_EPS (站号4, CAN1整车)
#define ECU_ID_MCU2_ACM  0xD5FB   // MCU2_ACM (站号5, CAN1整车)
#define ECU_ID_MCU2_DCDC 0xD7FB   // MCU2_DCDC (站号7, CAN1整车)
#define ECU_ID_MCU2      ECU_ID_MCU2_TM1  // 兼容旧名,默认TM1
#define ECU_ID_MCU3      0xD2FC   // MCU3_TM1

/**
 * 升级结果
 */
typedef enum {
    OTA_UPGRADE_SUCC = 0x01,           // 升级成功
    OTA_UPGRADE_FAILED = 0x02,         // 升级失败
    OTA_UPGRADE_ROLLBACK_SUCC = 0x03,  // 升级失败, 回滚成功
    OTA_UPGRADE_ROLLBACK_FAILED = 0x04 // 升级失败, 回滚失败
} OtaUpgradeResult;

/**
 * 升级结果错误码
*/
typedef enum {
    OTA_OK = 0x00,                                  // 无错误
    OTA_ERR_UPGRADE_FILE_DOWNLOAD_FAILED = 0x01,    // 升级文件下载失败
    OTA_ERR_UPGRADE_FILE_MD5_CHECK_FAILED = 0x02,   // 升级文件MD5校验失败
    OTA_ERR_UPGRADE_FILE_DECOMPRESS_FAILED = 0x03,  // 升级文件解压失败
    OTA_ERR_PROGRAM_FILE_CHECK_FAILED = 0x04,       // 程序文件效验不通过
    OTA_ERR_PROGRAM_FILE_TOO_LARGE = 0x05,          // 程序文件过大无法存储
    OTA_ERR_UPGRADING_MPU_NO_RESPONSE = 0x06,       // 刷写MPU过程无应答
    OTA_ERR_UPGRADING_MPU_NEGATIVE_RESPONSE = 0x07, // 刷写MPU过程否定应答
    OTA_ERR_VER_VERIFY_FAILE_AFTER_UPGRADE = 0x08,  // 刷写目标升级后版本校验失败
    OTA_ERR_NOT_SATISFIED_UPGRADE_CONDITION = 0x09, // 不满足刷写条件
    OTA_ERR_VEHICLE_LOW_VOLTAGE = 0x0A,             // 车辆低电压
    OTA_ERR_UPGRADE_IN_PROGRESS = 0x0B,             // 刷写正在执行
    OTA_ERR_NEW_TASK = 0x0C,                        // 平台下发新任务，当前任务失败
    OTA_ERR_POLICY_FILE_DOWNLOAD_FAILED = 0x0D,     // 升级策略文件下载失败
    OTA_ERR_POLICY_FILE_CHECK_FAILED = 0x0E,        // 升级策略文件校验不通过
    OTA_ERR_INTERRUPTED_BY_ECU = 0x0F,              // 被ECU升级中断
    OTA_ERR_CURRENT_VERSION_NOT_SUPPORT = 0x10,     // 当前版本不是支持版本
    OTA_ERR_OFFLINE_UPGRADE_OCCUR = 0x11,           // 发生线下刷写
    OTA_ERR_UPGRADE_FILE_NOT_EXIST = 0x12,          // 升级包不存在
    OTA_ERR_BACKUP_FILE_NOT_EXIST = 0x13,           // 备份包不存在
    OTA_ERR_PROHIBIT_POWER_SWITCH_FAILED = 0x14,    // BCM进入OTA模式失败（禁止电源切换）
    OTA_ERR_APPLY_HIGH_VOLTAGE_FAILED = 0x15,       // 上高压失败
    OTA_ERR_BRUSH_VERSION_INCONSISTENT = 0x16,      // 实际刷写版本与目标刷写版本不一致
    OTA_ERR_BACKUP_FILE_DOWNLOAD_FAILED = 0x17,     // 备份文件下载失败
    OTA_ERR_BACKUP_FILE_MD5_CHECK_FAILED = 0x18,    // 备份文件MD5校验失败
    OTA_ERR_BACKUP_FILE_DECOMPRESS_FAILED = 0x19,   // 备份文件解压失败
    OTA_ERR_HIGH_VOLTAGE_DOWN_FAILED = 0x1A,        // 高压下电失败
    OTA_ERR_ANTI_THEFT_AUTH_FAILED = 0x1B,          // 防盗认证不通过
    OTA_ERR_UPGRADE_TARGET_ECU_FAILED = 0x1C,       // 升级目标ECU失败
    OTA_ERR_PART_NUM_OR_HW_VER_NOT_MATCH = 0x1D,    // 零件号或硬件版本不匹配
    OTA_ERR_UPGRADE_POLICY_FILE_FORMAT_ERR = 0x1E,  // 升级策略文件格式错误
    OTA_ERR_BIN_FILE_NUM_NOT_MATCH = 0x1F,          // bin文件数量不匹配
    OTA_ERR_BIN_FILE_MD5_CHECK_FAILED = 0x20,       // bin文件MD5校验失败
    OTA_ERR_MPU_UPGRADE_FAILED = 0x21,              // bin文件解压失败
    OTA_ERR_TASK_TIMEOUT = 0x22                     // 任务超时
} OtaUpgradeErrorCode;

/**
 * 升级前置条件不满足错误码
*/
typedef enum {
    PRECONDS_ERR_NONE = 0x00,                                      // 无错误
    PRECONDS_ERR_VEHICLE_IN_READY = 0x01,                          // 车辆处于READY状态
    PRECONDS_ERR_VEHICLE_IN_MOTION = 0x02,                         // 车辆非静止状态
    PRECONDS_ERR_POWER_BATTERY_QUANTITY_INSUFFICIENT = 0x03,       // 车辆动力电池电量不足
    PRECONDS_ERR_LOW_VOLTAGE_BATTERY_QUANTITY_INSUFFICIENT = 0x04, // 车辆低压蓄电池电量不足
    PRECONDS_ERR_NOT_P_OR_N_GEAR = 0x05,                           // 车辆挡位不是P/N挡
    PRECONDS_ERR_VEHICLE_MALFUNCTION = 0x06,                       // 整车有故障
    PRECONDS_ERR_CHARGE_PLUG_CONN = 0x07,                          // 充电枪连接
    PRECONDS_ERR_REMOTE_CTRL_EXECUTING = 0x08,                     // 远程指令正在执行
    PRECONDS_ERR_DIAGNOSTIC_DEV_CONN = 0x09,                       // 有诊断设备连接
    PRECONDS_ERR_UNKNOWN_REASON = 0xFE                             // 未知原因
} OtaPreconditionsErrorCode;

/**
 * OTA日志错误码
*/
typedef enum {
    OTA_LOG_OK = 0x0,                      // 无错误
    OTA_LOG_NOT_INITIALIZES = 0x01,        // OTA Log未初始化
    OTA_LOG_ALREADY_INITIALIZED = 0x02,    // OTA Log已经初始化
    OTA_LOG_FILE_CREATION_FAILED = 0x03,   // OTA日志文件创建失败
    OTA_LOG_FILE_BACKUP_FAILED = 0x04,     // OTA日志文件备份失败
    OTA_LOG_NULL_POINTER_PARAMETER = 0x05, // 空指针参数
    OTA_LOG_INVALID_PARAMETER = 0x06       // 无效参数
} OtaLogErrorCode;

#endif // _OTA_MASTER_ERROR_CODE_H