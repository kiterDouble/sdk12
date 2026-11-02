/*********************************************************
 * Copyright © 2025 SiRun (HeFei) . All rights reserved.
 *
 * Author: byf
 * Created Time: 2025-02-07
 * Description: OTA Master数据结构定义
 *********************************************************/
#ifndef _OTA_MASTER_DATA_H
#define _OTA_MASTER_DATA_H

#include <stdint.h>
#include "ota_master_error_code.h"


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


typedef enum {
    CAN_SIG_OTA_MODE_OFF = 0,  // OTA模式关闭
    CAN_SIG_OTA_MODE_ON = 1,   // OTA模式开启
} CanSignalOtaMode;

/**
 * OTA状态
 */
typedef enum {
    OTA_NOT_READY = 0,   // OTA未准备
    OTA_IN_PROGRESS = 1, // OTA中
    OTA_READY = 2,       // OTA准备好，可OTA
} OtaMasterStatus;

/**
 * OTA升级电源模式
 */
typedef enum {
    OTA_LOW_VOLTAGE_UPGRADE = 0,      // 低压升级
    OTA_HIGH_VOLTAGE_UPGRADE = 1,     // 高压升级
    STOP_HIGH_VOLTAGE_POWER_DOWN = 2, // 下高压后置零
    OTA_NORMAL_POWER_DOWN = 3,        // 正常断电
} OtaMasterUpgradePowerMode;

/**
 * OTA升级模式
 */
typedef enum {
    OTA_UPGRADE_MODE_SILENT = 0,   // 静默升级
    OTA_UPGRADE_MODE_STANDARD = 1, // 标准升级
} OtaMasterUpgradeMode;

/**
 * OTA重启状态
 */
typedef enum {
    OTA_TBOX_NO_REBOOT = 0,   // TBox不重启
    OTA_TBOX_NEED_REBOOT = 1, // TBox需要重启
} OtaMasterTboxRebootState;

struct UpgradePreconditions_st;
struct EcuVersionInfo_st;
struct QueryEcuVerInfo_st;

/**
 * OTA升级件下载开始上报回调
 *
 * @param[in] taskId    OTA任务ID
 *
 * @return
 *      OTA_RET_OK：上报成功
 *      OTA_RET_ERR：上报失败
 */
typedef int (*ReportStartDownloadCallbackFunc)(const char *taskId);

/**
 * 升级包下载结果上报回调
 * 
 * @param[in] taskId    OTA任务ID
 * @param[in] reslut    下载结果 OTA_RET_OK：成功 其他：失败
 * @param[in] errorCode 失败原因
 *
 * @return
 *      OTA_RET_OK：上报成功
 *      OTA_RET_ERR：上报失败
 */
typedef int (*ReportDownloadResultCallbackFunc)(const char *taskId, int result, OtaUpgradeErrorCode errorCode);

/**
 * 升级结果上报回调
 * 
 * @param[in] taskId    OTA任务ID
 * @param[in] vehicleOtaVer 整车OTA版本
 * @param[in] reslut    升级结果
 * @param[in] errorCode 失败原因
 *
 * @return
 *      OTA_RET_OK：上报成功
 *      OTA_RET_ERR：上报失败
 */
typedef int (*ReportUpgradeResultCallbackFunc)(const char *taskId, const char *vehicleOtaVer, OtaUpgradeResult result,
                                               OtaUpgradeErrorCode errorCode);

/**
 * OTA任务清除结果上报回调
 *
 * @param[in] taskId    OTA任务ID
 * @param[in] reslut    任务清除结果 OTA_RET_OK：成功 其他：失败
 *
 * @return
 *      OTA_RET_OK：上报成功
 *      OTA_RET_ERR：上报失败
 */
typedef int (*ReportOtaTaskClearResultCallbackFunc)(const char *taskId, int result);

/**
 * 上报升级前置条件检测结果回调
 * 
 * libota_master根据获取到的前置条件实际信息与目标状态对比并确认是否满足升级条件后，将升级前置条件是否满足结果回调通知到注册方；
 *
 * @param[in] taskId    OTA任务ID
 * @param[in] result    检测结果：OTA_RET_OK：升级前置条件满足；OTA_RET_ERR：升级前置条件不满足；
 * @param[in] errCode   升级前置条件不满足的原因
 *
 * @return
 *      OTA_RET_OK：上报成功
 *      OTA_RET_ERR：上报失败
 */
typedef int (*ReportPreconditionsCheckResultCallbackFunc)(const char *taskId, int result,
                                                          OtaPreconditionsErrorCode errCode);

/**
 * OTA刷写开始上报回调
 *
 * @param[in] taskId    OTA任务ID
 *
 * @return
 *      OTA_RET_OK：上报成功
 *      OTA_RET_ERR：上报失败
 */
typedef int (*ReportStartUpgradeCallbackFunc)(const char *taskId);

/**
 * 通知APP开始回滚其他ECU
 *
 * @param[in] taskId    OTA任务ID
 *
 * @return
 *      OTA_RET_OK：通知成功
 *      OTA_RET_ERR：通知失败
 */
typedef int (*ReportStartRollbackCallbackFunc)(const char *taskId);

/**
 * UDS协议数据发送回调函数
 * 
 * 协议结构（按字节索引顺序）：
 * 
 * +-------------------------------------------------------------------------------------------------------+
 * | 字节索引 | 字段名称             | 值范围/类型        | 描述                                                |
 * |----------|----------------------|-------------------|-------------------------------------------------|
 * | [0]      | 协议起始标识         | 固定值 0x00        | 标识数据包的开始，固定为0。                           |
 * | [1]      | 发送模式标志         | 0x00 或 0x01       | 0x00: 首次发送（新会话）；0x01: 重复发送（重传或续传）   |
 * | [2]      | 计数器索引           | 0x00~0xFF (uint8) | 发送计数器，每次发送自增1，溢出后自动回绕为0。           |
 * | [3]-[6]  | 目标CAN ID          | uint32_t          | 目标设备CAN标识符（物理寻址或功能寻址），以大端格式存储。   |
 * | [7]-[10] | 响应CAN ID          | uint32_t          | ECU回复的响应CAN标识符，存储格式同目标CAN ID。          |
 * | [11]     | CAN通道号           | 0x00~0xFF         | 指定CAN总线通道（如0x00:CAN0, 0x01:CAN1）。            |
 * | [12]-N   | 报文载荷            | uint8_t[]         | 实际传输的CAN报文数据，长度可变。                       |
 * +--------------------------------------------------------------------------------------------------------+
 * 
 * @note 使用说明：
 * 1. **字节序**：多字节字段（如CAN ID）按小端格式存储（低字节在前，高字节在后）。
 * 2. **计数器回绕**：计数器索引达到0xFF后，下一次发送应回绕为0x00。
 * 3. **报文长度**：总长度 = 12字节头部 + 报文载荷长度（需确保缓冲区足够大）。
 * 4. **重传机制**：发送模式标志为0x01时，接收方应忽略计数器索引的变化，视为同一数据包的重传。
 * 
 * @example 示例数据：
 *   uint8_t buffer[] = {
 *       0x00,               // [0] 起始标识
 *       0x00,               // [1] 首次发送
 *       0x5A,               // [2] 计数器索引=90
 *       0x00, 0x00, 0x07, 0x27, // [3]-[6] 目标CAN ID=0x00000727
 *       0x00, 0x00, 0x07, 0xA7, // [7]-[10] 响应CAN ID=0x000007A7
 *       0x00,               // [11] CAN通道0
 *       0xAA, 0xBB, 0xCC    // [12]-[14] 报文载荷（3字节）
 *   };
 *
 * @param[in] data      数据起始地址
 * @param[in] dataLen   数据长度
 *
 * @return
 *      OTA_RET_OK：发送成功
 *      OTA_RET_ERR：发送失败
 */
typedef int (*SendUdsDataCallbackFunc)(const uint8_t *data, uint16_t dataLen);

/**
 * 接收到ECU UDS数据后响应接口回调
 *
 * MPU回复ACK数据格式 (类型4)
 * 
 * +----------------------------------------------------------------------------------------------+
 * | 字节索引 | 字段名称             | 值范围/类型        | 描述                                       |
 * |----------|-------------------|-------------------|-------------------------------------------|
 * | [0]      | 协议类型标识        | 0x00              | 标识为ACK回复协议。                          |
 * | [1]      | ACK类型标识        | 0x00              | 标识为MPU的ACK。                            |
 * | [2]      | 计数器索引          | 0x00~0xFF (uint8) | 必须与SendUdsDataCallbackFunc中的data[2]:  |
 * |          |                   |                   | counterIdx一致，表示对应请求已处理完成。       |
 * +----------------------------------------------------------------------------------------------+
 * 
 * @note 数据总长度固定为3字节。
 * @example MPU ACK示例：
 *   buffer = {0x00, 0x00, 0x5A}; // 确认处理完成counterIdx=0x5A的报文
 * 
 * @param[in] data      数据起始地址
 * @param[in] dataLen   数据长度
 *
 * @return
 *      OTA_RET_OK：发送成功
 *      OTA_RET_ERR：发送失败
 */
typedef int (*SendUdsDataAckCallbackFunc)(const uint8_t *data, uint16_t dataLen);

/**
 * 同步查询目标ECU软硬件版本号回调函数
 *
 * 当注册该同步查询回调函数时，`libota_master`在需要查询ECU版本信息时会调用此回调函数，
 * 注册方同步查询ECU版本信息后更新到ecuVersion中，并返回查询结果；
 *
 * @param[in]   queryEcusInfo   查询的ECU信息集合
 * @param[in]   ecuCnt          查询的ECU数
 * @param[out]  ecuVersion      ECU版本信息
 *
 * @return
 *      OTA_RET_OK：查询成功
 *      OTA_RET_ERR：查询失败
 */
typedef int (*QueryTaskECUVersionSyncCallbackFunc)(const struct QueryEcuVerInfo_st *queryEcusInfo, uint8_t ecuCnt,
                                                   struct EcuVersionInfo_st *ecuVersion);

/**
 * 异步查询目标ECU软硬件版本号回调函数
 *
 * 当未注册 `QueryTaskECUVersionSyncCallbackFunc` 函数，而仅注册本函数时，`libota_master`
 * 在需要查询ECU版本信息时会调用此回调函数。 注册方不能阻塞该函数，且异步方式获取目标ECU的版本信息后通过
 * `OtaMasterNoticeEcuVersionAsyncQueryResult` 告知 `libota_master` 查询结果。
 *
 * @param[in] queryEcusInfo 查询的ECU信息集合
 * @param[in] ecuCnt        查询的ECU数，表示 `ecuIds` 数组中的元素个数。
 *
 * @return 无
 */
typedef void (*QueryTaskECUVersionAsyncCallbackFunc)(const struct QueryEcuVerInfo_st *queryEcusInfo, uint8_t ecuCnt);

/**
 * 同步查询升级前置条件回调函数
 *
 * 当注册该同步查询回调函数时，`libota_master`在准备升级前会调用此回调函数同步查询升级前置条件是否满足，
 * 注册方同步查询所有要求的前置条件具体信息后更新到preconditons中，并返回查询结果；
 *
 * @param[out] preconditons 所有前置条件
 *
 * @return
 *      OTA_RET_OK：查询成功
 *      OTA_RET_ERR：查询失败
 */
typedef int (*QueryUpgradePreconditionsSyncCallbackFunc)(struct UpgradePreconditions_st *preconditions);

/**
 * 异步查询升级前置条件回调函数
 *
 * 当未注册 `QueryUpgradePreconditionsSyncCallbackFunc`函数，而仅注册本函数时，`libota_master`
 * 在准备升级前会调用此回调函数异步查询升级前置条件是否满足。 注册方不能阻塞该函数，异步方式查询所有要求的前置条件具体信息后通过
 * `OtaMasterNoticeUpgradePreconditionsAsyncQueryResult` 告知 `libota_master` 查询结果。
 *
 * @param 无
 * 
 * @return 无
 */
typedef void (*QueryUpgradePreconditionsAsyncCallbackFunc)(void);

/**
 * 上、下高压请求回调函数
 * 
 * OTA_HIGH_VOLTAGE_UPGRADE：上高压请求
 * a. TBOX发送远程上电控制命令TBOX_PowerMode_Req=0x1: ON给BCM，BCM执行低压上电，吸合KL15继电器，发出电源模式信号为BCM_PowerMode =0x3: Remote ON；
 * b. VCU检测到低压上电后，执行上高压，若TBOX检测到高压状态VCU_HV_Sts =0x2: HV ON，则高压上电成功，高压上电通过；
 * 
 * OTA_LOW_VOLTAGE_UPGRADE：下高压请求
 * a. TBOX发送下高压请求TBOX_HV_Req=01:ON，VCU执行高压下电后返回VCU_HV_Sts=0x1: Standby，则高压下电请求通过;
 * b. VCU执行高压下电后返回VCU_HV_Sts！=0x1: Standby，表示下高压失败；
 * 
 * 上下高压结果通过`OtaMasterNoticeUpDownHighVoltageResult`异步通知libota_master
 *
 * @param[in] powerMode 0：下高压；1：上高压；
 *
 * @return
 *      OTA_RET_OK：请求成功
 *      OTA_RET_ERR：请求失败
 */
typedef int (*RequestUpDownHighVoltageCallbackFunc)(OtaMasterUpgradePowerMode powerMode);

/**
 * FOTA Mode ON/OFF切换请求回调函数
 *
 * 通知MCU周期性发送
 * 0x00：FOTA mode OFF -> FOTAModReq = 0x02，所有ECU控制器接收到此信号后，退出OTA模式，BCM正常退出禁止电源模式切换；
 * 0x01：FOTA mode ON -> FOTAModReq = 0x01，所有ECU控制器接收到此信号后，进入OTA模式，BCM执行禁止电源模式切换；
 * 
 * 模式切换结果通过`OtaMasterNoticeFotaModeOnOffResult`异步通知libota_master
 *
 * @param[in] isModeOn 升级模式状态；0：FOTA mode OFF；1：FOTA mode ON
 *
 * @return
 *      OTA_RET_OK：请求成功
 *      OTA_RET_ERR：请求失败
 */
typedef int (*RequestFotaModeOnOffSwCallbackFunc)(CanSignalOtaMode isModeOn);

/**
 * 请求TBox自升级回调函数
 *
 * @param[in] upgradePath T-box升级包路径
 *
 * @return
 *      OTA_RET_OK：请求成功
 *      OTA_RET_ERR：请求失败
 */
typedef int (*RequestTboxSelfUpgradeCallbackFunc)(const char *upgradePath);

/**
 * 请求TBox自回滚回调函数
 * 
 * @param[in] backupPath T-box备份路径
 * 
 * @return
 *      OTA_RET_OK：请求成功
 *      OTA_RET_ERR：请求失败
 */
typedef int (*RequestTboxSelfRollbackCallbackFunc)(const char *backupPath);

/**
 * 请求TBox重启回调函数
 */
typedef void (*RequestTboxRebootCallbackFunc)(void);

#pragma pack(1)
/**
 * OTA任务策略升级文件下载信息
 */
typedef struct {
    uint32_t fileSize;       // 文件大小
    const char *downloadUrl; // 下载地址
    const char *md5;         // 下载文件MD5
} StOtaTaskPolicyFileInfo;

/**
 * OTA任务策略Flash协议信息
 */
typedef struct {
    uint8_t canPort;           // CAN端口，云端不下发则默认为0xFF
    uint32_t physicalId;       // 物理寻址ID，云端不下发则默认为0xFFFFFFFF
    uint32_t functionId;       // 功能寻址ID，云端不下发则默认为0xFFFFFFFF
    uint32_t responseId;       // 响应ID，云端不下发则默认为0xFFFFFFFF
    uint32_t secAccessLv1Mask; // 安全访问级别1掩码
    uint32_t secAccessLv2Mask; // 安全访问级别2掩码
} StOtaTaskPolicyFlashProtocol;

/**
 * OTA任务策略版本信息
 */
typedef struct {
    const char *partNum;    // 零部件号
    const char *hwVer;      // 硬件版本
    const char *oemSwVer;   // OEM软件号
    const char *swVer;      // 软件版本
} StOtaTaskPolicyVersionInfo;

/**
 * OTA任务策略升级ECU件信息
 */
typedef struct {
    uint16_t ecuId;                             // 升级ECU ID
    uint8_t upgradeOrder;                       // 升级顺序
    const char *supplierId;                     // 供应商ID
    StOtaTaskPolicyFileInfo upgradeFileInfo;    // 主文件信息
    StOtaTaskPolicyFileInfo backupFileInfo;     // 备份文件信息
    StOtaTaskPolicyFlashProtocol flashProtocol; // 刷写协议
    StOtaTaskPolicyVersionInfo supportVer;      // 支持版本
    StOtaTaskPolicyVersionInfo checkVer;        // 校验版本
} StOtaTaskPolicyEcuInfo;

/**
 * OTA任务策略信息
 *
 * OTA任务策略信息包含OTA任务ID、任务名、任务信息、日志上传地址、任务开始时间、任务持续时间、是否上传OTA日志、升级模式、升级件总数、升级件信息等;
 * taskStartTime - OTA任务开始时间，为UTC时间戳，单位秒；当taskDuration不为0时，该时间由应用层从接收到OTA任务开始指令开始计时；若taskDuration为0时，该值默认传0，由master
 * sdk在刷写开始时自行获取；
 * taskDuration - OTA全任务持续时间，包含升级包下载、解压、前置条件判断、刷写、等待用户确认等时间；当该值为0时，默认全任务升级时长无限大，只考虑upgradeDuration；
 * upgradeDuration - 表示升级文件刷写时长，当taskDuration为0时，该值有效，仅进行刷写计时；若upgradeDuration也为0时，表示刷写时长无限大；
 */
typedef struct {
    const char *taskId;                  // OTA任务ID
    const char *taskName;                // OTA任务名
    const char *taskInfo;                // OTA任务信息
    const char *logUploadAddr;           // OTA日志上传地址
    const char *vehicleOtaVer;           // 整车OTA版本
    uint32_t taskStartTime;              // OTA任务开始时间（UTC时间戳，单位秒）
    uint8_t taskDuration;                // OTA全任务持续时间（单位：分）
    uint8_t upgradeDuration;             // 升级文件刷写时长（单位：分）
    uint8_t isUploadLog;                 // 是否上传OTA日志 0：不上传 1：上传
    uint8_t upgradeMode;                 // 升级模式（0：静默升级，1：标准升级）
    uint8_t taskCount;                   // 升级件总数
    StOtaTaskPolicyEcuInfo *taskEcuInfo; // 升级件信息
} StOtaTaskPolicyInfo;

/**
 * ECU版本查询信息
 */
typedef struct QueryEcuVerInfo_st {
    uint16_t ecuId;      // ECU ID
    uint8_t canPort;     // CAN端口 1：CAN BD; 2：CAN PT; 3：CAN FD;
    uint32_t physicalId; // 物理寻址ID
    uint32_t functionId; // 功能寻址ID
    uint32_t responseId; // 应答ID
} StQueryEcuVerInfo;

/**
 * ECU软硬件版本信息
 */
typedef struct EcuVersionInfo_st {
    uint16_t ecuId;                         // ECU ID
    const char *swVersion;                  // 软件版本号
    const char *hwVersion;                  // 硬件版本号
    const char *supplierId;                 // 供应商ID
    const char *partNum;                    // 零部件号
} StEcuVersionInfo;

/**
 * 升级前置条件 
 */
typedef struct UpgradePreconditions_st {
    float batteryVoltage;                   // 蓄电池电压
    uint8_t gearPosition;                   // 当前档位; 0：P档，1：R档，2：N档，3：D档
    uint8_t vcuHvStatus;                    // 动力系统就绪状态; 0x0: Not ready，0x1: Ready
    uint16_t vehicleSpeed;                  // 车速
    uint8_t bmsRealSoc;                     // 动力电池 SOC
    uint8_t hasVehicleFault;                // 整车是否故障 0：无故障，1：有故障
    uint8_t chargePlugState;                // 充电枪连接状态 0：未连接，1：已连接
    uint8_t hasRemoteCtrlExecuting;         // 有无远程指令正在执行 0：无远控指令正在执行 1：有远控指令正在执行
    uint8_t isObdDiagDevConn;               // OBD诊断设备是否连接 0：未连接 1：已连接
    uint8_t isBcmAntiTheftAuthPassed;       // 二级防盗认证（BCM&TBOX）是否通过 0：未通过 1：已通过
} StUpgradePreconditions;

/**
 * OTA Master上报回调函数集合
 */
typedef struct {
    ReportStartDownloadCallbackFunc startDownloadCb;                       // 上报下载开始
    ReportDownloadResultCallbackFunc downloadResultCb;                     // 上报下载结果
    ReportUpgradeResultCallbackFunc upgradeResultCb;                       // 上报升级结果
    ReportOtaTaskClearResultCallbackFunc otaTaskClearResultCb;             // 上报OTA任务清空结果
    ReportPreconditionsCheckResultCallbackFunc preconditionsCheckResultCb; // 上报前置条件检测结果
    ReportStartUpgradeCallbackFunc startUpgradeCb;                         // 上报OTA刷写开始
    ReportStartRollbackCallbackFunc startRollbackCb;                       // 上报回滚其他ECU
} StOtaMasterReportCallback;

/**
 * OTA Master查询函数集合
 */
typedef struct {
    QueryTaskECUVersionSyncCallbackFunc queryTaskECUVerSyncCb;              // 同步查询ECU版本信息
    QueryTaskECUVersionAsyncCallbackFunc queryTaskECUVerAsyncCb;            // 异步查询ECU版本信息
    QueryUpgradePreconditionsSyncCallbackFunc queryUpgradePrecondsSyncCb;   // 同步查询前置条件
    QueryUpgradePreconditionsAsyncCallbackFunc queryUpgradePrecondsAsyncCb; // 异步查询前置条件
} StOtaMasterQueryCallback;

/**
 * OTA Master请求/发送回调函数集合
 */
typedef struct {
    SendUdsDataCallbackFunc sendUdsDataCb;                     // 发送UDS协议数据
    SendUdsDataAckCallbackFunc sendUdsDataAckCb;               // UDS协议数据ACK
    RequestUpDownHighVoltageCallbackFunc reqUpDownHighVoltCb;  // 上下高压请求
    RequestFotaModeOnOffSwCallbackFunc reqFotaModeOnOff;       // FOTA Mode ON/OFF请求
    RequestTboxSelfUpgradeCallbackFunc reqTboxSelfUpgradeCb;   // TBox自升级触发请求
    RequestTboxSelfRollbackCallbackFunc reqTboxSelfRollbackCb; // TBox自回滚触发请求
    RequestTboxRebootCallbackFunc reqTboxRebootCb;             // TBox重启请求
} StOtaMasterRequestCallback;
#pragma pack()

#endif // _OTA_MASTER_DATA_H