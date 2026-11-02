/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-02-08
 * Description: 应用回调函数
 *********************************************************/
#ifndef _OTA_MASTER_APP_CALLBACK_H
#define _OTA_MASTER_APP_CALLBACK_H

#include <stdint.h>
#include <string>
#include "ota_master_data.h"

/**
 * 注册结果上报回调接口
 *
 * @param[in] callback  结果上报回调函数集合
 *
 * @return
 *      OTA_RET_OK：注册成功
 *      OTA_RET_ERR：注册失败
 */
int RegisterReportCallback(const StOtaMasterReportCallback *callback);

/**
 * 结果上报回调函数去注册
 *
 * @param 无
 *
 * @return 无
 */
void UnregisterReportCallback(void);

/**
 * 注册信息查询回调函数
 *
 * @param[in] callback  信息查询回调函数集合
 *
 * @return
 *      OTA_RET_OK：注册成功
 *      OTA_RET_ERR：注册失败
 */
int RegisterQueryCallback(const StOtaMasterQueryCallback *callback);

/**
 * 信息查询回调函数去注册
 *
 * @param 无
 *
 * @return 无
 */
void UnregisterQueryCallback(void);

/**
 * 注册请求类型回调函数
 *
 * @param[in] callback  请求类型回调函数集合
 *
 * @return
 *      OTA_RET_OK：注册成功
 *      OTA_RET_ERR：注册失败
 */
int RegisterRequestCallback(const StOtaMasterRequestCallback *callback);

/**
 * 请求类型回调函数去注册
 *
 * @param 无
 *
 * @return 无
 */
void UnregisterRequestCallback(void);

/**
 * 回调开始下载
 *
 * @param[in] taskId    升级任务ID
 *
 * @return
 *      OTA_RET_OK：开始下载上报成功
 *      OTA_RET_ERR：开始下载上报失败
 */
int ReportStartDownloadCallback(const std::string &taskId);

/**
 * 回调上报升级文件下载结果
 *
 * @param[in] taskId    OTA任务ID
 * @param[in] result    下载结果；OTA_RET_OK：下载成功、OTA_RET_ERR：下载失败
 * @param[in] errCode   下载失败原因
 *
 * @return
 *      OTA_RET_OK：下载结果上报成功
 *      OTA_RET_ERR：下载结果上报失败
 */
int ReportDownloadResultCallback(const std::string &taskId, int result, OtaUpgradeErrorCode errCode);

/**
 * 回调上报升级结果
 *
 * @param[in] taskId    OTA任务ID
 * @param[in] vehicleOtaVer 车辆OTA版本号
 * @param[in] result    升级结果；OTA_RET_OK：下载成功、OTA_RET_ERR：下载失败
 * @param[in] errCode   升级失败原因
 *
 * @return
 *      OTA_RET_OK：结果上报成功
 *      OTA_RET_ERR：结果上报失败
 */
int ReportUpgradeResultCallback(const std::string &taskId, const std::string &vehicleOtaVer, OtaUpgradeResult result,
                                OtaUpgradeErrorCode errCode);

/**
 * 回调上报升级任务清除结果
 *
 * @param[in] taskId    OTA任务ID
 * @param[in] result    升级任务清除结果；OTA_RET_OK：清除成功、OTA_RET_ERR：清除失败
 *
 * @return
 *      OTA_RET_OK：结果上报成功
 *      OTA_RET_ERR：结果上报失败
 */
int ReportOtaTaskClearResultCallback(const std::string &taskId, int result);

/**
 * 回调上报前置条件检测结果
 *
 * @param[in] taskId    OTA任务ID
 * @param[in] result    升级前置条件检测结果；OTA_RET_OK：检测通过、OTA_RET_ERR：检测不通过
 * @param[in] errCode   升级前置条件不通过原因
 *
 * @return
 *      OTA_RET_OK：上报成功
 *      OTA_RET_ERR：上报失败
 */
int ReportPreconditionsCheckResultCallback(const std::string &taskId, int result, OtaPreconditionsErrorCode errCode);

/**
 * 回调开始升级
 *
 * @param[in] taskId    升级任务ID
 *
 * @return
 *      OTA_RET_OK：开始升级成功
 *      OTA_RET_ERR：开始升级失败
 */
int ReportStartUpgradeCallback(const std::string &taskId);

/**
 * 回调通知APP开始回滚其他ECu
 *
 * @param[in] taskId    升级任务ID
 *
 * @return
 *      OTA_RET_OK：回调通知成功
 *      OTA_RET_ERR：回调通知失败
 */
int ReportStartRollbackCallback(const std::string &taskId);

/**
 * 回调发送UDS数据到MCU
 *
 * @param[in] data      UDS数据地址
 * @param[in] dataLen   UDS数据长度
 *
 * @return
 *      OTA_RET_OK：UDS数据发送成功
 *      OTA_RET_ERR：UDS数据发送失败
 */
int SendUdsDataCallback(const uint8_t *data, uint16_t dataLen);

/**
 * 回调发送UDS数据ACK到MCU
 *
 * @param[in] data      UDS数据地址
 * @param[in] dataLen   UDS数据长度
 *
 * @return
 *      OTA_RET_OK：数据发送成功
 *      OTA_RET_ERR：数据发送失败
 */
int SendUdsDataAckCallback(const uint8_t *data, uint16_t dataLen);

/**
 * 查询升级任务ECU版本信息回调函数是否是异步的
 *
 * @param[in] 无
 *
 * @return
 *      true：异步回调
 *      false：同步回调
 */
bool IsQueryTaskECUVerCallbackAsync(void);

/**
 * 回调同步查询升级任务ECU版本信息
 *
 * @param[in]   queryEcusInfo   查询的ECU信息集合
 * @param[in]   ecuCnt          查询的ECU数
 * @param[out]  ecuVersion      ECU版本信息
 *
 * @return
 *      OTA_RET_OK：查询成功
 *      OTA_RET_ERR：查询失败
 */
int QueryTaskECUVersionSyncCallback(const StQueryEcuVerInfo *queryEcusInfo, uint8_t ecuCnt,
                                    StEcuVersionInfo *ecuVersion);

/**
 * 回调异步查询升级任务ECU版本信息
 *
 * @param[in] queryEcusInfo 查询的ECU信息集合
 * @param[in] ecuCnt        查询的ECU数，表示 `ecuIds` 数组中的元素个数。
 *
 * @return 无
 */
void QueryTaskECUVersionAsyncCallback(const StQueryEcuVerInfo *queryEcusInfo, uint8_t ecuCnt);

/**
 * 查询升级前置条件回调函数是否是异步的
 *
 * @param[in] 无
 *
 * @return
 *      true：异步回调
 *      false：同步回调
 */
bool IsQueryUpgradePreconditionsCallbackAsync(void);

/**
 * 回调同步查询升级前置条件
 *
 * @param[out] preconditons   升级前置条件当前状态
 *
 * @return
 *      OTA_RET_OK：查询成功
 *      OTA_RET_ERR：查询失败
 */
int QueryUpgradePreconditionsSyncCallback(StUpgradePreconditions *preconditons);

/**
 * 回调异步查询升级前置条件
 *
 * @param 无
 *
 * @return 无
 */
void QueryUpgradePreconditionsAsyncCallback(void);

/**
 * 回调请求上/下高压
 * 
 * 上下高压结果通过`OtaMasterNoticeUpDownHighVoltageResult`异步通知libota_master
 *
 * @param[in] powerMode   0：下高压请求 1：上高压请求
 *
 * @return
 *      OTA_RET_OK：请求成功
 *      OTA_RET_ERR：请求失败
 */
int RequestUpDownHighVoltageCallback(OtaMasterUpgradePowerMode powerMode);

/**
 * 回调请求设置FOTA模式开关
 * 
 * 模式切换结果通过`OtaMasterNoticeFotaModeOnOffResult`异步通知libota_master
 *
 * @param[in] isModeOn
 * 
 * @return
 *      OTA_RET_OK：设置成功
 *      OTA_RET_ERR：设置失败
 */
int RequestFotaModeOnOffSwitchCallback(CanSignalOtaMode isModeOn);

/**
 * 回调请求TBox自升级
 *
 * @param[in] upgradeFilePath   TBox升级文件存储路径
 *
 * @return
 *      OTA_RET_OK：请求成功
 *      OTA_RET_ERR：请求失败
 */
int RequestTboxSelfUpgradeCallback(const std::string &upgradeFilePath);

/**
 * 回调请求TBox自回滚
 * 
 * @param[in] backupFilePath   TBox备份文件存储路径
 * 
 * @return
 *      OTA_RET_OK：请求成功
 *      OTA_RET_ERR：请求失败
 */
int RequestTboxSelfRollbackCallback(const std::string &backupFilePath);

/**
 * 回调请求TBox重启
 * 
 * @param 无
 * 
 * @return 无
 */
void RequestTboxRebootCallback(void);

#endif // _OTA_MASTER_APP_CALLBACK_H