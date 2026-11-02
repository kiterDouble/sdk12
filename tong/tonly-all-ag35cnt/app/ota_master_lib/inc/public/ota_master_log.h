/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-02-07
 * Description: ota_master日志模块
 *********************************************************/
#ifndef _OTA_MASTER_LOG_H
#define _OTA_MASTER_LOG_H

#include <stdint.h>
#include "ota_master_error_code.h"
#include "sr_log.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * OTA日志输出等级 
 */
typedef enum {
    OTA_LOG_ERROR = 0,
    OTA_LOG_WARN,
    OTA_LOG_INFO,
    OTA_LOG_DEBUG
} OtaMasterLogLevel;

/**
 * OTA Master Log初始化
 *
 * @param tag       日志标签
 * @param logDir    日志文件存储目录
 *
 * @return OtaLogErrorCode
 *      OTA_LOG_OK：初始化成功
 *      OTA_LOG_ALREADY_INITIALIZED：已初始化
 *      其他：初始化失败
 */
OtaLogErrorCode OtaMasterLogInit(const char* tag, const char *logDir);

/**
 * 结束OTA Master日志
 *
 * @param 无
 *
 * @return 无
 */
void OtaMasterLogFinish(void);

/**
 * 备份OTA Master日志文件并重置当前日志
 *
 * 该函数需要在'OtaMasterLogInit'调用后方可调用，该函数调用后会将当前OTA日志进行备份，并重置当前日志文件从头开始记录；
 *
 * @param 无
 *
 * @return OtaLogErrorCode
 *      OTA_LOG_OK：重置成功
 *      其他：重置失败
 */
OtaLogErrorCode OtaMasterLogReset(void);

/**
 * 上传OTA Master日志
 *
 * @param uploadUrl 日志上传URL;
 * @param taskId    任务ID
 *
 * @return 无
 */
void OtaMasterUploadLog(const char *uploadUrl, const char *taskId);

/**
 * 输出OTA Master日志
 *
 * @param level 日志输出等级
 * @param file  日志输出文件
 * @param line  日志输出行号
 * @param format 日志输出格式
 * @param ...    日志输出参数
 *
 * @return 无
 */
void OtaMasterLogPrint(OtaMasterLogLevel level, const char *file, int line, const char *format, ...);

/**
 * OTA Master日志宏定义
 * 使用 zlog 库提供的 LOG_Debug、LOG_Info、LOG_Warn、LOG_Error 接口
 */
#define OTA_LOG_D(format, ...)  LOG_Debug(format, ##__VA_ARGS__)
#define OTA_LOG_I(format, ...)  LOG_Info(format, ##__VA_ARGS__)
#define OTA_LOG_W(format, ...)  LOG_Warn(format, ##__VA_ARGS__)
#define OTA_LOG_E(format, ...)  LOG_Error(format, ##__VA_ARGS__)
#define OTA_LOG_HEX(data, len, text)  LOG_DebugHex(data, len, text)

#ifdef __cplusplus
}
#endif
#endif // _OTA_MASTER_LOG_H