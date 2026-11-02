#ifndef MQTT_LOG_COORD_H
#define MQTT_LOG_COORD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 沿用项目已有的错误码风格
// RET_OK = 0, RET_ERR = -1

/**
 * @brief 启动日志协同HTTP Server
 * @retval  RET_OK    成功
 * @retval  RET_ERR   HTTP Server启动失败
 */
int StartLogCoordinationHttpServer(void);

/**
 * @brief 停止日志协同HTTP Server
 */
void StopLogCoordinationHttpServer(void);

/**
 * @brief 校验batchNumber格式合法性
 * @param batchNumber 批次号
 * @retval  RET_OK    格式合法
 * @retval  RET_ERR   格式不合法
 */
int ValidateBatchNumber(const char *batchNumber);

/**
 * @brief 清除协同状态（比较batchNumber后调用，防止旧batchNumber被误用）
 */
void ClearCoordinationState(void);

/**
 * @brief 副模组收到MQTT日志上传指令后的处理入口
 *
 * @param batchNumber 批次号（会先校验格式合法性）
 * @param startTime 开始时间戳
 * @param endTime 结束时间戳
 * @retval  RET_OK    成功委托主模组上传
 * @retval  RET_ERR   失败（batchNumber格式不合法或其他错误）
 */
int SlaveHandleLogUpload(const char *batchNumber, uint64_t startTime, uint64_t endTime);

/**
 * @brief 在独立线程中执行副模组日志协同上传处理（不阻塞MQTTRecv线程）
 * @param batchNumber 批次号
 * @param startTime 开始时间戳
 * @param endTime 结束时间戳
 * @retval  RET_OK    成功创建线程
 * @retval  RET_ERR   创建线程失败
 */
int SlaveHandleLogUploadAsync(const char *batchNumber, uint64_t startTime, uint64_t endTime);

/**
 * @brief 主模组收到MQTT日志上传指令后的处理入口
 *
 * @param batchNumber 批次号（会先校验格式合法性）
 * @param startTime 开始时间戳
 * @param endTime 结束时间戳
 * @retval  RET_OK    成功
 * @retval  RET_ERR   失败（batchNumber格式不合法或其他错误）
 */
int MasterHandleLogUpload(const char *batchNumber, uint64_t startTime, uint64_t endTime);

/**
 * @brief 在独立线程中执行主模组日志协同上传处理（不阻塞MQTTRecv线程）
 * @param batchNumber 批次号
 * @param startTime 开始时间戳
 * @param endTime 结束时间戳
 * @retval  RET_OK    成功创建线程
 * @retval  RET_ERR   创建线程失败
 */
int MasterHandleLogUploadAsync(const char *batchNumber, uint64_t startTime, uint64_t endTime);

/**
 * @brief 查询主模组是否在线
 * @retval  1   在线
 * @retval  0   不在线
 */
int CheckMasterOnline(void);

/**
 * @brief 通知主模组上传日志
 * @param batchNumber 批次号
 * @param startTime 开始时间戳
 * @param endTime 结束时间戳
 * @retval  RET_OK    成功
 * @retval  RET_ERR   HTTP请求失败
 */
int NotifyMasterUpload(const char *batchNumber, uint64_t startTime, uint64_t endTime);

/**
 * @brief 获取委托上传状态
 * @param batchNumber 批次号输出
 * @param startTime 开始时间戳输出
 * @param endTime 结束时间戳输出
 * @retval  1   有委托任务
 * @retval  0   无委托任务
 */
int GetUploadDelegateStatus(char *batchNumber, uint64_t *startTime, uint64_t *endTime);

/**
 * @brief 获取额外打包任务状态
 * @param batchNumber 批次号输出
 * @param startTime 开始时间戳输出
 * @param endTime 结束时间戳输出
 * @retval  1   有额外任务
 * @retval  0   无额外任务
 */
int GetExtraUploadTask(char *batchNumber, uint64_t *startTime, uint64_t *endTime);

#ifdef __cplusplus
}
#endif

#endif /* MQTT_LOG_COORD_H */
