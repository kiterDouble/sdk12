/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-02-07
 * Description: 应用层接口
 *********************************************************/
#ifndef _OTA_MASTER_API_H
#define _OTA_MASTER_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "ota_master_data.h"

/**
 * OTA Master SDK初始化
 *
 * @param[in] configFilePath    配置文件路径
 * @param[in] downloadFolder    下载目录
 * @param[in] unzipFolder       解压目录
 *
 * @return
 *      OTA_RET_OK：初始化成功
 *      OTA_RET_ERR：初始化失败
 */
int OtaMasterInit(const char *configFilePath, const char *downloadFolder, const char *unzipFolder);

/**
 * OTA Master SDK去初始化
 *
 * @param 无 
 *
 * @return 无
 */
void OtaMasterDeinit(void);

/**
 * 注册结果上报回调接口
 *
 * @param[in] callback  结果上报回调函数集合
 *
 * @return
 *      OTA_RET_OK：注册成功
 *      OTA_RET_ERR：注册失败
 */
int OtaMasterRegisterReportCallback(const StOtaMasterReportCallback *callback);

/**
 * 结果上报回调函数去注册
 *
 * @param 无
 *
 * @return 无
 */
void OtaMasterUnregisterReportCallback(void);

/**
 * 注册信息查询回调函数
 *
 * @param[in] callback  信息查询回调函数集合
 *
 * @return
 *      OTA_RET_OK：注册成功
 *      OTA_RET_ERR：注册失败
 */
int OtaMasterRegisterQueryCallback(const StOtaMasterQueryCallback *callback);

/**
 * 信息查询回调函数去注册
 *
 * @param 无
 *
 * @return 无
 */
void OtaMasterUnregisterQueryCallback(void);

/**
 * 注册请求类型回调函数
 *
 * @param[in] callback  请求类型回调函数集合
 *
 * @return
 *      OTA_RET_OK：注册成功
 *      OTA_RET_ERR：注册失败
 */
int OtaMasterRegisterRequestCallback(const StOtaMasterRequestCallback *callback);

/**
 * 请求类型回调函数去注册
 *
 * @param 无
 *
 * @return 无
 */
void OtaMasterUnregisterRequestCallback(void);

/**
 * 获取OTA状态
 *
 * @param 无
 *
 * @return OtaMasterStatus
 */
OtaMasterStatus OtaMasterGetOtaStatus(void);

/**
 * 启动OTA Master
 *
 * @param 无
 *
 * @return
 *      OTA_RET_OK：OTA Master启动成功
 *      OTA_RET_ERR：OTA Master启动失败
 */
int OtaMasterStart(void);

/**
 * 停止OTA Master
 *
 * @param 无
 *
 * @return
 *      OTA_RET_OK：OTA Master停止成功
 *      OTA_RET_ERR：OTA Master停止失败
 */
int OtaMasterStop(void);

/**
 * 开始OTA任务
 *
 * @param[in] policyInfo    OTA任务策略信息
 *
 * @return
 *      OTA_RET_OK：OTA 任务启动成功
 *      OTA_RET_ERR：OTA 任务启动失败
 */
int OtaMasterStartOtaTask(const StOtaTaskPolicyInfo *policyInfo);

/**
 * 清除OTA任务
 *
 * @param 无
 *
 * @return
 *      OTA_RET_OK：OTA 任务清除成功
 *      OTA_RET_ERR：OTA 任务清除失败
 */
int OtaMasterClearOtaTask(void);

/**
 * 确认开始执行升级
 *
 * 标准升级模式下，OTA升级文件下载成功并成功上报下载结果到平台，平台下发确认升级指令后通过此接口通知libota_master开始执行刷写；
 *
 * @param[in] taskId    OTA任务ID
 *
 * @return
 *      OTA_RET_OK：执行升级成功
 *      OTA_RET_ERR：执行升级失败
 */
int OtaMasterConfirmStartUpgrading(const char *taskId);

/**
 * AES128 ECB加密
 *
 * @param[in] key     加密KEY
 * @param[in] keyLen  加密KEY长度
 * @param[in] input   加密源数据
 * @param[in] inputLen  加密源数据长度
 * @param[out] output  加密后输出数据
 *
 * @return 加密后的数据长度（失败返回0）
 */
int OtaMasterAES128ECBEncrypt(const uint8_t *key, int keyLen, const uint8_t *input, int inputLen, uint8_t **output);

/**
 * AES128 ECB解密
 *
 * @param[in] key     加密KEY
 * @param[in] keyLen  加密KEY长度
 * @param[in] input   加密源数据
 * @param[in] inputLen  加密源数据长度
 * @param[out] output  解密后输出数据
 *
 * @return 解密后的数据长度（失败返回0）
 */
int OtaMasterAES128ECBDecrypt(const uint8_t *key, int keyLen, const uint8_t *input, int inputLen, uint8_t **output);

/**
 * 通知MasterSDK MCU收到MPU发送的UDS数据后的ACK数据
 * 
 * MCU回复ACK数据格式
 * 
 * +-----------------------------------------------------------------------------------------------------------------+
 * | 字节索引 | 字段名称             | 值范围/类型         | 描述                                                         |
 * |----------|--------------------|-------------------|-------------------------------------------------------------|
 * | [0]      | 协议类型标识         | 0x00              | 不使用。                                                     |
 * | [1]      | ACK类型标识         | 0x00               | 不使用。                                                     |
 * | [2]      | 计数器索引           | 0x00~0xFF (uint8) | 必须与SendUdsDataCallbackFunc中的data[2]一致，用于匹配请求与响应。|
 * +-----------------------------------------------------------------------------------------------------------------+
 * 
 * @note 数据总长度固定为3字节。
 * @example MCU ACK示例：
 *   buffer = {0x00, 0x00, 0x5A}; // 确认接收counterIdx=0x5A的报文
 *
 * @param[in] data      数据地址
 * @param[in] dataLen   数据长度
 *
 * @return 无
 */
void OtaMasterNoticeMcuUdsDataAck(const uint8_t *data, uint16_t dataLen);

/**
 * 检查MD5
 * 
 * @param[in] filePath    文件路径
 * @param[in] targetMD5   目标MD5
 * 
 * @return
 *      OTA_RET_OK：MD5校验成功
 *      OTA_RET_ERR：MD5校验失败
 */
int OtaMasterCheckMD5(const char *filePath, const char *targetMD5);

/**
 * 通知Master SDK ECU的UDS响应数据
 * 
 * ECU响应报文数据格式
 * 
 * +--------------------------------------------------------------------------------------------------------------------+
 * | 字节索引 | 字段名称             | 值范围/类型        | 描述                                                             |
 * |----------|--------------------|-------------------|----------------------------------------------------------------|
 * | [0]      | 协议类型标识         | 0x00              | 标识为ECU响应协议。                                               |
 * | [1]      | 发送模式标志         | 0x00 或 0x01       | 0x00: 首次响应；0x01: 重复响应（如超时重传）。                       |
 * | [2]      | 计数器索引           | 0x00~0xFF (uint8) | 必须与SendUdsDataCallbackFunc中的data[2]一致，用于匹配请求与响应。   |
 * | [3]-[6]  | 响应CAN ID          | uint8_t[4]        | ECU的响应ID。                                                   |
 * | [7]-N    | 报文载荷             | uint8_t[]         | ECU返回的实际数据。                                              |
 * +--------------------------------------------------------------------------------------------------------------------+
 * 
 * @example ECU响应示例：
 *   buffer = {0x00, 0x00, 0x5A, 0x00,0x00,0x07,0xA7,0x05,0xAA,0xBB,0xCC};
 *
 * @param[in] data      数据地址
 * @param[in] dataLen   数据长度
 *
 * @return 无
 */
void OtaMasterNoticeEcuUdsData(const uint8_t *data, uint16_t dataLen);

/**
 * 通知Master SDK ECU版本异步查询结果
 *
 * @param[in] ecuVersions   ECU版本信息
 * @param[in] ecuCnt        ECU数
 *
 * @return 无
 */
void OtaMasterNoticeEcuVersionAsyncQueryResult(const StEcuVersionInfo *ecuVersions, uint8_t ecuCnt);

/**
 * 通知Master SDK升级前置条件异步查询结果
 *
 * @param[in] preconditons   升级前置条件当前实际状态
 *
 * @return 无
 */
void OtaMasterNoticeUpgradePreconditionsAsyncQueryResult(const StUpgradePreconditions *preconditons);

/**
 * 通知Master SDK TBox回滚结果
 *
 * @param[in] result    回滚结果：OTA_RET_OK：回滚成功；OTA_RET_ERR：回滚失败
 * @param[in] rebootState 若TBox回滚成功，告知Master SDK是否需要重启TBox
 *
 * @return 无
 */
void OtaMasterNoticeTboxRollbackResult(int result, OtaMasterTboxRebootState rebootState);

/**
 * 通知Master SDK TBox自升级结果
 *
 * @param[in] result    升级结果：OTA_RET_OK：升级成功；OTA_RET_ERR：升级失败
 * @param[in] rebootState 若TBox升级成功，告知Master SDK是否需要重启TBox
 *
 * @return 无
 */
void OtaMasterNoticeTboxSelfUpgradeResult(int result, OtaMasterTboxRebootState rebootState);

/**
 * 通知Master SDK 上报下载结果已经ACK
 * 
 * @param[int] taskId 任务ID
 * 
 * @return 无
 */
void OtaMasterNoticeReportDownloadResultHasACK(const char *taskId);

/**
 * 通知Master SDK 上报升级结果已经ACK
 * 
 * @param[int] taskId 任务ID
 * 
 * @return 无
 */
void OtaMasterNoticeReportUpgradeResultHasACK(const char *taskId);

/**
 * 通知Master SDK 上下高压请求结果
 * 
 * @param[in] result    请求结果：OTA_RET_OK：请求成功；OTA_RET_ERR：请求失败
 * @param[in] powerMode 0：下高压请求 1：上高压请求
 * 
 * @return 无
 */
void OtaMasterNoticeUpDownHighVoltageResult(int result, OtaMasterUpgradePowerMode powerMode);

/**
 * 通知Master SDK FOTA模式开关结果
 * 
 * @param[in] result    请求结果：OTA_RET_OK：请求成功；OTA_RET_ERR：请求失败
 * @param[in] mode      OTA模式
 * 
 * @return 无
 */
void OtaMasterNoticeFotaModeOnOffSwResult(int result, CanSignalOtaMode mode);

/**
 * ECU刷写进度回调函数类型
 *
 * @param[in] percent  刷写进度百分比（0-100）
 * @param[in] userdata 用户数据指针
 */
typedef void (*OtaMasterEcuBrushProgressCallback)(uint8_t percent, void *userdata);

/**
 * 直接执行ECU刷写(带进度回调)
 *
 * 该接口直接执行ECU刷写，不经过完整的OTA任务流程。
 * 协议信息从 support_ota_ecu_config.json 配置文件中获取。
 *
 * @param[in] ecuId            ECU ID
 * @param[in] driverFilePath   驱动文件路径（可为NULL）
 * @param[in] softwareFilePath 软件文件路径
 * @param[in] targetVer        目标版本号（可为NULL）
 * @param[in] progressCb       进度回调函数（可为NULL）
 * @param[in] userdata         用户数据指针（传递给回调函数）
 *
 * @return
 *      OTA_RET_OK：刷写成功
 *      OTA_RET_ERR：刷写失败
 */
int OtaMasterExecEcuBrush(uint16_t ecuId, const char *driverFilePath, const char *softwareFilePath,
                          const char *targetVer, OtaMasterEcuBrushProgressCallback progressCb, void *userdata);

#ifdef __cplusplus
}
#endif
#endif // _OTA_MASTER_API_H