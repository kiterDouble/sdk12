#ifndef __OTA_MASTER_FOTA_ECU_H__
#define __OTA_MASTER_FOTA_ECU_H__
#include <stdint.h>
#include <thread>
#include <vector>
#include <string>

#define SUPPORT_MAX_BLOCK_LEN (2048)

typedef struct {
    uint16_t ecuId;  // ECU ID，用于区分不同ECU类型
    uint32_t phyId;  // 物理CANID
    uint32_t funcId; // 功能CANID
    uint32_t resId;  // 响应CANID
    uint8_t channel; // CAN通道
    std::string driverFilePath; // 驱动路径
    std::vector<std::string> softwareFilePaths; // 软件文件路径列表
} FotaEcuInfo_t;

typedef void* FotaEcuHandle_t; //  FotaEcu句柄

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief 初始化FOTA ECU模块
 * @param hdl FOTA ECU句柄指针
 * @param info ECU信息结构体指针
 * @return int 返回初始化结果
 *         OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
int FotaEcuInit(FotaEcuHandle_t *hdl, const FotaEcuInfo_t *info);

/**
 * @brief 执行FOTA ECU升级
 * @param hdl FOTA ECU句柄
 * @param targetVer 目标版本号
 * @return int 返回执行结果
 *         OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
int FotaEcuExec(FotaEcuHandle_t hdl, const std::string &targetVer);

/**
 * @brief 执行FOTA ECU清除故障码
 * @param hdl FOTA ECU句柄
 * @return int 返回执行结果
 *         OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
int FotaEcuClearDtc(FotaEcuHandle_t hdl);

/**
 * @brief 执行回滚ECU
 * @param hdl FOTA ECU句柄
 * @param timeOutMs 回滚超时时间，单位ms
 * @param rollbackVer 回滚版本号
 * @return int 返回执行结果
 *         OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
int FotaEcuRollback(FotaEcuHandle_t hdl, uint16_t timeOutMs, const std::string &rollbackVer);

/**
 * @brief 验证软件版本
 * @param hdl FOTA ECU句柄
 * @param targetVer 目标版本号
 * @return int 返回执行结果
 *         OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
int VerifySoftwareVersion(FotaEcuHandle_t hdl, const std::string &targetVer);

/**
 * @brief 获取FOTA ECU错误码
 * @param hdl FOTA ECU句柄
 * @param errorCode 错误码EcuFirmwareBrushStatus
 * @return int 返回执行结果
 *         OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
int FotaEcuGetErrorCode(FotaEcuHandle_t hdl, uint32_t &errorCode);

/**
 * @brief 反初始化FOTA ECU模块
 * @param hdl FOTA ECU句柄
 * @return int 返回执行结果
 *         OTA_RET_OK 成功
 *         OTA_RET_ERR 失败
 */
int FotaEcuDeinit(FotaEcuHandle_t hdl);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif