#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include "ota_master_api.h"
#include "ota_master_log.h"
#include "ota_master_ecu_brush.h"
#include "ota_master_archive_manage.h"
#include "ota_master_config.h"
#include "ota_master_fota_ecu.h"
#include "ota_master_pm_helper.h"
#include "sr_ipc.h"
#include "sdk/mcu_api.h"

struct TestCase {
    int id;
    const char* description;
    void (*func)();
};

static constexpr const char *OTA_MASTER_LOG_SAVE_FOLDER = "/sdcard/ota_master/";
static constexpr const char *OTA_MASTER_CONFIG_FOLDER = "/sdcard/ota_master/config/";
static constexpr const char *OTA_MASTER_DOWNLOAD_FOLDER = "/sdcard/ota_master/";
static constexpr const char *OTA_MASTER_UNZIP_FOLDER = "/sdcard/ota_master/";

#if 1
// 下载链接中包含文件扩展名
static const StOtaTaskPolicyFileInfo upgradeFileInfo = {
    .fileSize = 4434354,
    .downloadUrl =
        "https://dev.whld.net:58082/api/offline-command/ota/file/v5/4de32d39-9e39-4fb5-9678-8810853361ff/"
        "46d340a8-b218-4afc-912d-2e0a16eee44c",
    .md5 = "0c15c2d7d9daaf9937c852569665a058"};
static const StOtaTaskPolicyFileInfo backupFileInfo = {
    .fileSize = 4434354,
    .downloadUrl =
        "https://dev.whld.net:58082/api/offline-command/ota/file/v5/4de32d39-9e39-4fb5-9678-8810853361ff/"
        "46d340a8-b218-4afc-912d-2e0a16eee44c",
    .md5 = "0c15c2d7d9daaf9937c852569665a058"};
#else
// 下载链接中不包含文件扩展名, 需要通过文件头信息获取
static const StOtaTaskPolicyFileInfo upgradeFileInfo = {
    .fileSize = 4251877,
    .downloadUrl =
        "https://dev.whld.net:58082/api/offline-command/ota/file/v5/bb546f3e-24aa-484f-af8a-be550a8e9627/d8e2a9c4-e772-456e-8f76-6f1ca33cf608",
    .md5 = "f162bef830f4551528a437ea33967d1a"};
static const StOtaTaskPolicyFileInfo backupFileInfo = {.fileSize = 0, .downloadUrl = "", .md5 = ""};
#endif

static const StOtaTaskPolicyFlashProtocol tboxFlashProto = {.canPort = 0,
                                                        .physicalId = 0x703,
                                                        .functionId = 0x7df,
                                                        .responseId = 0x783,
                                                        .secAccessLv1Mask = 0,
                                                        .secAccessLv2Mask = 0};


static const StOtaTaskPolicyVersionInfo tboxSupportVer = {.partNum = "1111",
                                                          .hwVer = "0001",
                                                          .oemSwVer = "0001",
                                                          .swVer = "0001"};

static const StOtaTaskPolicyVersionInfo tboxCheckVer = {.partNum = "1111",
                                                        .hwVer = "0001",
                                                        .oemSwVer = "0001",
                                                        .swVer = "0002"};

static const StOtaTaskPolicyFlashProtocol bcmFlashProto = {.canPort = 1,
                                                            .physicalId = 0x727,
                                                            .functionId = 0x7df,
                                                            .responseId = 0x7a7,
                                                            .secAccessLv1Mask = 0,
                                                            .secAccessLv2Mask = 0};

static const StOtaTaskPolicyVersionInfo bcmSupportVer = {.partNum = "1111",
                                                         .hwVer = "0001",
                                                         .oemSwVer = "0001",
                                                         .swVer = "0001"};

static const StOtaTaskPolicyVersionInfo bcmCheckVer = {.partNum = "1111",
                                                       .hwVer = "0001",
                                                       .oemSwVer = "0001",
                                                       .swVer = "0002"};

// VCU刷写协议参数（对应 support_ota_ecu_config.json 中的 VCU 配置）
static const StOtaTaskPolicyFlashProtocol vcuFlashProto = {.canPort = 1,
                                                           .physicalId = 0x7DC,
                                                           .functionId = 0x7DF,
                                                           .responseId = 0x7D4,
                                                           .secAccessLv1Mask = 0,
                                                           .secAccessLv2Mask = 0};

static const StOtaTaskPolicyVersionInfo vcuSupportVer = {.partNum = "VCU_PART_001",
                                                         .hwVer = "0001",
                                                         .oemSwVer = "0001",
                                                         .swVer = "0001"};

static const StOtaTaskPolicyVersionInfo vcuCheckVer = {.partNum = "VCU_PART_001",
                                                       .hwVer = "0001",
                                                       .oemSwVer = "0001",
                                                       .swVer = "0002"};

static bool isOtaLogInit = false;
static bool isOtaMasterInit = false;
static bool isOtaMasterStart = false;
static bool isIpcInitialized = false;
static OtaMasterUpgradePowerMode g_upgradePowerMode = OtaMasterUpgradePowerMode::OTA_LOW_VOLTAGE_UPGRADE;
static CanSignalOtaMode g_otaMode = CanSignalOtaMode::CAN_SIG_OTA_MODE_OFF;
static std::string g_taskId;

/**
 * @brief ECU UDS响应数据订阅回调（MCU→MPU）
 *        MCU收到ECU的CAN响应后转发给MPU，此处注入到OTA Master SDK
 */
static void EcuUdsDataCallback(const uint8_t *data, const uint16_t dataLen, void *userdata)
{
    if (!data || dataLen == 0) {
        OTA_LOG_E("EcuUdsDataCallback invalid params");
        return;
    }
    OtaMasterNoticeEcuUdsData(data, dataLen);
}

/**
 * @brief MCU ACK确认订阅回调（MPU→MCU发送确认）
 *        MCU确认已收到MPU发送的UDS数据
 */
static void McuUdsAckCallback(const uint8_t *data, const uint16_t dataLen, void *userdata)
{
    if (!data || dataLen == 0) {
        OTA_LOG_E("McuUdsAckCallback invalid params");
        return;
    }
    OtaMasterNoticeMcuUdsDataAck(data, dataLen);
}

/**
 * @brief 初始化平台IPC通信并订阅UDS消息
 */
static void InitIpcIfNeeded()
{
    if (isIpcInitialized) {
        return;
    }
    printf("[IPC] 初始化平台IPC通信...\n");
    if (InitSysQueTree() != 0) {
        OTA_LOG_E("InitSysQueTree failed");
        printf("[IPC] InitSysQueTree failed!\n");
        return;
    }
    if (StartQueueMsgRcvThread("otaDemo") != 0) {
        OTA_LOG_E("StartQueueMsgRcvThread failed");
        printf("[IPC] StartQueueMsgRcvThread failed!\n");
        return;
    }
    // 订阅ECU UDS响应数据（MCU→MPU）- 远程诊断协议
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_CANTP_MCU_TO_MPU, EcuUdsDataCallback, NULL);
    // 订阅MCU ACK确认消息（MPU→MCU发送确认）- 远程诊断协议
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_CANTP_MPU_TO_MCU_ACK, McuUdsAckCallback, NULL);
    // 订刷写协议的ECU UDS响应数据（MCU→MPU）
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_CANTP_FLASH_MCU_TO_MPU, EcuUdsDataCallback, NULL);
    // 订阅刷写协议的MCU ACK确认消息（MPU→MCU发送确认）
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_CANTP_FLASH_MPU_TO_MCU_ACK, McuUdsAckCallback, NULL);

    isIpcInitialized = true;
    printf("[IPC] IPC初始化成功, UDS消息订阅已注册\n");
    OTA_LOG_I("IPC initialized and UDS subscriptions registered");
}

static int ReportStartDownloadCallback(const char *taskId)
{
    OTA_LOG_D("ReportStartDownloadCallback taskId: %s", taskId);
    printf("ReportStartDownloadCallback taskId: %s\n", taskId);
    return OTA_RET_OK;
}

static int ReportDownloadResultCallback(const char *taskId, int reslut, OtaUpgradeErrorCode errorCode)
{
    g_taskId = (taskId == nullptr) ? "" : taskId;
    OTA_LOG_D("ReportDownloadResultCallback taskId:%s reslut: %d, errorCode: %d", g_taskId.c_str(), reslut, errorCode);
    printf("ReportDownloadResultCallback taskId:%s reslut: %d, errorCode: %d\n", g_taskId.c_str(), reslut, errorCode);
    return OTA_RET_OK;
}

static int ReportUpgradeResultCallback(const char *taskId, const char *vehicleOtaVer, OtaUpgradeResult reslut,
                                       OtaUpgradeErrorCode errorCode)
{
    OTA_LOG_D("ReportUpgradeResultCallback taskId:%s vehicleOtaVer:%s reslut: %d, errorCode: %d",
              taskId ? taskId : "NULL", vehicleOtaVer ? vehicleOtaVer : "NULL", reslut, errorCode);
    printf("ReportUpgradeResultCallback taskId:%s vehicleOtaVer:%s reslut: %d, errorCode: %d\n",
           taskId ? taskId : "NULL", vehicleOtaVer ? vehicleOtaVer : "NULL", reslut, errorCode);
    g_taskId = (taskId == nullptr) ? "" : taskId;
    return OTA_RET_OK;
}

static int ReportOtaTaskClearResultCallback(const char *taskId, int reslut)
{
    OTA_LOG_D("ReportOtaTaskClearResultCallback taskId:%s reslut: %d", taskId, reslut);
    printf("ReportOtaTaskClearResultCallback taskId:%s reslut: %d\n", taskId, reslut);
    return OTA_RET_OK;
}

static int ReportPreconditionsCheckResultCallback(const char *taskId, int result, OtaPreconditionsErrorCode errCode)
{
    OTA_LOG_D("ReportPreconditionsCheckResultCallback taskId:%s result: %d, errCode: %d", taskId, result, errCode);
    printf("ReportPreconditionsCheckResultCallback taskId:%s result: %d, errCode: %d\n", taskId, result, errCode);
    return OTA_RET_OK;
}

static int ReportStartUpgradeCallback(const char *taskId)
{
    OTA_LOG_D("ReportStartUpgradeCallback taskId: %s", taskId);
    printf("ReportStartUpgradeCallback taskId: %s\n", taskId);
    return OTA_RET_OK;
}

/**
 * @brief UDS协议数据发送回调（MPU→MCU）
 *        通过MCU服务将UDS请求数据发送到MCU，MCU转发到CAN总线
 *        使用刷写协议（mid=0x20），MCU端从偏移11开始解析UDS数据
 */
static int SendUdsDataCallback(const uint8_t *data, uint16_t dataLen)
{
    OTA_LOG_D("SendUdsDataCallback dataLen: %d", dataLen);
    if (!data || dataLen == 0) {
        OTA_LOG_E("SendUdsDataCallback invalid params");
        return OTA_RET_ERR;
    }
    McuMesg_t mesg;
    mesg.type = SYS_QUE_TYPE_MCU_CANTP_FLASH_MPU_TO_MCU;  // 使用刷写协议（mid=0x20）
    mesg.payloadLen = dataLen;
    mesg.payload = const_cast<uint8_t *>(data);
    int32_t ret = McuSrvSendMesg(&mesg);
    if (ret != 0) {
        OTA_LOG_E("McuSrvSendMesg FLASH_MPU_TO_MCU failed, ret: %d", ret);
        return OTA_RET_ERR;
    }
    return OTA_RET_OK;
}

/**
 * @brief UDS ACK数据发送回调（MCU→MPU ACK）
 *        通过MCU服务将ACK确认发送给MCU
 */
static int SendUdsDataAckCallback(const uint8_t *data, uint16_t dataLen)
{
    OTA_LOG_D("SendUdsDataAckCallback dataLen: %d", dataLen);
    if (!data || dataLen == 0) {
        OTA_LOG_E("SendUdsDataAckCallback invalid params");
        return OTA_RET_ERR;
    }
    McuMesg_t mesg;
    mesg.type = SYS_QUE_TYPE_MCU_CANTP_MCU_TO_MPU_ACK;
    mesg.payloadLen = dataLen;
    mesg.payload = const_cast<uint8_t *>(data);
    int32_t ret = McuSrvSendMesg(&mesg);
    if (ret != 0) {
        OTA_LOG_E("McuSrvSendMesg MCU_TO_MPU_ACK failed, ret: %d", ret);
        return OTA_RET_ERR;
    }
    return OTA_RET_OK;
}

static int QueryTaskECUVersionSyncCallback(const StQueryEcuVerInfo *queryEcusInfo, uint8_t ecuCnt, StEcuVersionInfo *ecuVersion)
{
    OTA_LOG_D("QueryTaskECUVersionSyncCallback ecuCnt: %d", ecuCnt);
    printf("QueryTaskECUVersionSyncCallback ecuCnt: %d\n", ecuCnt);
    if (!queryEcusInfo || !ecuVersion) {
        OTA_LOG_E("Invalid params [ECUIds:%p][VersionInfo:%p]", static_cast<const void *>(queryEcusInfo),
                  static_cast<void *>(ecuVersion));
        return OTA_RET_ERR;
    }

    for (uint8_t i = 0; i < ecuCnt; ++i) {
        ecuVersion[i].ecuId = queryEcusInfo[i].ecuId;
        ecuVersion[i].swVersion = "0001";
        ecuVersion[i].hwVersion = "0001";
        if (queryEcusInfo[i].ecuId == 0x703) {
            ecuVersion[i].supplierId = "DL03005";
            ecuVersion[i].partNum = "1111";
        } else if (queryEcusInfo[i].ecuId == 0x7DC) {
            ecuVersion[i].supplierId = "DEFAULT";
            ecuVersion[i].partNum = "VCU_PART_001";
        } else if (queryEcusInfo[i].ecuId == 0x727) {
            ecuVersion[i].supplierId = "DL03024";
            ecuVersion[i].partNum = "1111";
        } else {
            ecuVersion[i].supplierId = "DL00000";
            ecuVersion[i].partNum = "FFFF";
        }
    }
    return OTA_RET_OK;
}

static int QueryUpgradePreconditionsSyncCallback(StUpgradePreconditions *preconditions)
{
    OTA_LOG_D("QueryUpgradePreconditionsSyncCallback");
    printf("QueryUpgradePreconditionsSyncCallback\n");
    if (preconditions == nullptr) {
        OTA_LOG_E("preconditions is nullptr");
        return OTA_RET_ERR;
    }

#if 1
    preconditions->batteryVoltage = 12.5;
    preconditions->bmsRealSoc = 30;
    preconditions->chargePlugState = 0;
    preconditions->gearPosition = 0;
    preconditions->hasRemoteCtrlExecuting = 0;
    preconditions->hasVehicleFault = 0;
    preconditions->isBcmAntiTheftAuthPassed = 1;
    preconditions->isObdDiagDevConn = 0;
    preconditions->vcuHvStatus =0;
    preconditions->vehicleSpeed = 0;
#else
    // 通过终端交互获取参数
    printf("Enter battery voltage (0.0~25.0): ");
    scanf("%lf", &preconditions->batteryVoltage);

    printf("Enter BMS real SOC (0-100): ");
    scanf("%d", &preconditions->bmsRealSoc);

    printf("Enter charge plug state (0:未连接, 1:已连接): ");
    scanf("%d", &preconditions->chargePlugState);

    printf("Enter gear position (0:P档, 1:R档, 2:N档, 3:D档): ");
    scanf("%d", &preconditions->gearPosition);

    printf("Is remote control executing? (0:无, 1:有): ");
    scanf("%d", &preconditions->hasRemoteCtrlExecuting);

    printf("Has vehicle fault? (0:无, 1:有): ");
    scanf("%d", &preconditions->hasVehicleFault);

    printf("Is BCM anti-theft auth passed? (0:未通过, 1:通过): ");
    scanf("%d", &preconditions->isBcmAntiTheftAuthPassed);

    printf("Is OBD diagnostic device connected? (0:无, 1:有): ");
    scanf("%d", &preconditions->isObdDiagDevConn);

    printf("Enter VCU HV status (0: Not ready, 1: Ready): ");
    scanf("%d", &preconditions->vcuHvStatus);

    printf("Enter vehicle speed (km/h, 0~300): ");
    scanf("%d", &preconditions->vehicleSpeed);

    // 清空输入缓冲区（防止换行符影响后续输入）
    while(getchar() != '\n');
#endif
    return OTA_RET_OK;
}

static int RequestLowOrHighVoltageCallback(OtaMasterUpgradePowerMode powerMode)
{
    OTA_LOG_D("RequestLowOrHighVoltageCallback powerMode: %d", powerMode);
    printf("RequestLowOrHighVoltageCallback powerMode: %d\n", powerMode);
    g_upgradePowerMode = powerMode;
    return OTA_RET_OK;
}

static int RequestFotaModeOnOffCallback(CanSignalOtaMode isModeOn)
{
    OTA_LOG_D("RequestFotaModeOnOffCallback isModeOn: %d", isModeOn);
    printf("RequestFotaModeOnOffCallback isModeOn: %d\n", isModeOn);
    g_otaMode = isModeOn;
    return OTA_RET_OK;
}

static int RequestTboxSelfUpgradeCallback(const char *upgradePath)
{
    OTA_LOG_D("RequestTboxSelfUpgradeCallback upgradePath: %s", upgradePath);
    printf("RequestTboxSelfUpgradeCallback upgradePath: %s\n", upgradePath);
    return OTA_RET_OK;
}

static int RequestTboxSelfRollbackCallback(const char *backupPath)
{
    OTA_LOG_D("RequestTboxSelfRollbackCallback backupPath: %s", backupPath);
    printf("RequestTboxSelfRollbackCallback backupPath: %s\n", backupPath);
    return OTA_RET_OK;
}

static void RequestTboxRebootCallback(void)
{
    OTA_LOG_D("RequestTboxRebootCallback");
    printf("RequestTboxRebootCallback\n");
    int status = system("reboot");
    if (WIFEXITED(status) && (WEXITSTATUS(status) == 0)) {
        OTA_LOG_D("TBox reboot success");
    } else {
        OTA_LOG_E("TBox reboot failed");
    }
}

static void TestOtaMasterLogInit()
{
    OtaLogErrorCode logError = OtaMasterLogInit("otaMasterApi", OTA_MASTER_LOG_SAVE_FOLDER);
    if (logError != OTA_LOG_OK) {
        printf("OtaMasterLogInit failed, error: %d\n", logError);
        if (logError == OTA_LOG_ALREADY_INITIALIZED) {
            OTA_LOG_W("OtaMaster log already initialized\n");
            return;
        }
        isOtaLogInit = false;
        return;
    }
    printf("OtaMasterLogInit success\n");    
    isOtaLogInit = true;
}

static void TestOtaMasterLogFinish()
{
    if (!isOtaLogInit) {
        printf("OtaMaster log uninitialized\n");
        return;
    }

    OtaMasterLogFinish();
    printf("OtaMasterLogFinish success\n");
}

static void TestOtaMasterDeinit()
{
    if (!isOtaMasterInit) {
        printf("OtaMaster uninitialized\n");
        return;
    }

    OtaMasterDeinit();
    printf("OtaMasterDeinit success\n");
}

static void TestOtaMasterLogReset()
{
    if (!isOtaLogInit) {
        printf("OtaMaster log uninitialized\n");
        return;
    }

    OtaLogErrorCode logError = OtaMasterLogReset();
    if (logError != OTA_LOG_OK) {
        printf("OtaMasterLogReset failed, error: %d\n", logError);
        return;
    }
    printf("OtaMasterLogReset success\n");
}

static void TestOtaMasterInit()
{
    auto error = OtaMasterInit(OTA_MASTER_CONFIG_FOLDER, OTA_MASTER_DOWNLOAD_FOLDER, OTA_MASTER_UNZIP_FOLDER);
    if (error != OTA_RET_OK) {
        printf("OtaMasterInit failed\n");
        OTA_LOG_E("OtaMasterInit failed");
        isOtaMasterInit = false;
        return;
    }
    isOtaMasterInit = true;
    printf("OtaMasterInit success\n");
    OTA_LOG_D("OtaMasterInit success");
}

static void TestOtaMasterRegisterReportCallback()
{
    if (!isOtaMasterInit) {
        printf("OtaMaster uninitialized\n");
        OTA_LOG_E("OtaMaster uninitialized");
        return;
    }

    StOtaMasterReportCallback otaMasterReportCb = {.startDownloadCb = ReportStartDownloadCallback,
                                                   .downloadResultCb = ReportDownloadResultCallback,
                                                   .upgradeResultCb = ReportUpgradeResultCallback,
                                                   .otaTaskClearResultCb = ReportOtaTaskClearResultCallback,
                                                   .preconditionsCheckResultCb = ReportPreconditionsCheckResultCallback,
                                                   .startUpgradeCb = ReportStartUpgradeCallback};
    OtaMasterRegisterReportCallback(&otaMasterReportCb);
    printf("OtaMasterRegisterReportCallback success\n");
    OTA_LOG_D("OtaMasterRegisterReportCallback success");
}

static void TestOtaMasterRegisterQueryCallback()
{
    if (!isOtaMasterInit) {
        printf("OtaMaster uninitialized\n");
        OTA_LOG_E("OtaMaster uninitialized");
        return;
    }

    StOtaMasterQueryCallback otaMasterQueryCb = {.queryTaskECUVerSyncCb = QueryTaskECUVersionSyncCallback,
                                                 .queryTaskECUVerAsyncCb = nullptr,
                                                 .queryUpgradePrecondsSyncCb = QueryUpgradePreconditionsSyncCallback,
                                                 .queryUpgradePrecondsAsyncCb = nullptr};
    OtaMasterRegisterQueryCallback(&otaMasterQueryCb);
    printf("OtaMasterRegisterQueryCallback success\n");
    OTA_LOG_D("OtaMasterRegisterQueryCallback success");
}

static void TestOtaMasterRegisterRequestCallback()
{
    if (!isOtaMasterInit) {
        printf("OtaMaster uninitialized\n");
        OTA_LOG_E("OtaMaster uninitialized");
        return;
    }

    StOtaMasterRequestCallback otaMasterRequestCallback = {.sendUdsDataCb = SendUdsDataCallback,
                                                           .sendUdsDataAckCb = SendUdsDataAckCallback,
                                                           .reqUpDownHighVoltCb = RequestLowOrHighVoltageCallback,
                                                           .reqFotaModeOnOff = RequestFotaModeOnOffCallback,
                                                           .reqTboxSelfUpgradeCb = RequestTboxSelfUpgradeCallback,
                                                           .reqTboxSelfRollbackCb = RequestTboxSelfRollbackCallback,
                                                           .reqTboxRebootCb = RequestTboxRebootCallback};
    OtaMasterRegisterRequestCallback(&otaMasterRequestCallback);
    printf("OtaMasterRegisterRequestCallback success\n");
    OTA_LOG_D("OtaMasterRegisterRequestCallback success");
}

static void TestOtaMasterGetOtaStatus()
{
    OtaMasterStatus status = OtaMasterGetOtaStatus();
    printf("OtaMasterGetOtaStatus: %d\n", status);
    OTA_LOG_D("OtaMasterGetOtaStatus: %d", status);
}

static void TestOtaMasterStart()
{
    if (!isOtaMasterInit) {
        printf("OtaMaster uninitialized\n");
        OTA_LOG_E("OtaMaster uninitialized");
        return;
    }

    if (OtaMasterStart() != OTA_RET_OK) {
        printf("OtaMasterStart failed\n");
        OTA_LOG_E("OtaMasterStart failed");
        isOtaMasterStart = false;
        return;
    }
    isOtaMasterStart = true;
    printf("OtaMasterStart success\n");
    OTA_LOG_D("OtaMasterStart success");
}

static void TestOtaMasterStop()
{
    if (!isOtaMasterStart) {
        printf("OtaMaster not started\n");
        OTA_LOG_E("OtaMaster not started");
        return;
    }

    if (OtaMasterStop() != OTA_RET_OK) {
        printf("OtaMasterStop failed\n");
        OTA_LOG_E("OtaMasterStop failed");
        return;
    }
    isOtaMasterStart = false;
    printf("OtaMasterStop success\n");
    OTA_LOG_D("OtaMasterStop success");
}

static void TestOtaMasterStartOtaTask()
{
    if (!isOtaMasterInit) {
        printf("OtaMaster uninitialized\n");
        OTA_LOG_E("OtaMaster uninitialized");
        return;
    }

    if (!isOtaMasterStart) {
        printf("OtaMaster not started\n");
        OTA_LOG_E("OtaMaster not started");
        return;
    }

    // VCU升级任务配置
    StOtaTaskPolicyEcuInfo otaTaskPolicyEcuInfo[1] = {
        {.ecuId = 0x7DC,
         .upgradeOrder = 1,
         .supplierId = "DEFAULT",
         .upgradeFileInfo = upgradeFileInfo,
         .backupFileInfo = backupFileInfo,
         .flashProtocol = vcuFlashProto,
         .supportVer = vcuSupportVer,
         .checkVer = vcuCheckVer},
    };

    StOtaTaskPolicyInfo otaTaskPolicyInfo = {.taskId = "bd26d188-3234-434b-1c0b-d9930f3fc0aa",
                                             .taskName = "ota_task_test",
                                             .taskInfo = "ota_task_test_info",
                                             .logUploadAddr = "",
                                             .vehicleOtaVer = "CV01.V1.0.1",
                                             .taskStartTime = 0,
                                             .taskDuration = 0,
                                             .upgradeDuration = 1,
                                             .isUploadLog = 0,
                                             .upgradeMode = 1,
                                             .taskCount = sizeof(otaTaskPolicyEcuInfo) / sizeof(StOtaTaskPolicyEcuInfo),
                                             .taskEcuInfo = otaTaskPolicyEcuInfo};
    if (OtaMasterStartOtaTask(&otaTaskPolicyInfo) != OTA_RET_OK) {
        printf("OtaMasterStartOtaTask failed\n");
        OTA_LOG_E("OtaMasterStartOtaTask failed");
        return;
    }
    printf("OtaMasterStartOtaTask success\n");
    OTA_LOG_D("OtaMasterStartOtaTask success");
}

static void TestOtaMasterClearOtaTask()
{
    if (!isOtaMasterStart) {
        printf("OtaMaster not started\n");
        return;
    }

    OtaMasterClearOtaTask();
}

static void TestOtaMasterConfirmStartUpgrading()
{
    if (!isOtaMasterStart) {
        printf("OtaMaster not started\n");
        return;
    }

    if (OtaMasterConfirmStartUpgrading(g_taskId.c_str()) != OTA_RET_OK) {
        printf("OtaMasterConfirmStartUpgrading failed\n");
        return;
    }
    printf("OtaMasterConfirmStartUpgrading success\n");
}

static void TestOtaMasterNoticeEcuVersionRollbackResult()
{

}

static void TestOtaMasterNoticeTboxSelfUpgradeSuccess()
{
    printf("TestOtaMasterNoticeTboxSelfUpgradeSuccess\n");
    if (!isOtaMasterStart) {
        printf("OtaMaster not started\n");
        return;
    }
    OtaMasterNoticeTboxSelfUpgradeResult(OTA_RET_OK, OtaMasterTboxRebootState::OTA_TBOX_NEED_REBOOT);
}

static void TestOtaMasterNoticeTboxSelfUpgradeFailed()
{
    printf("TestOtaMasterNoticeTboxSelfUpgradeFailed\n");
    if (!isOtaMasterStart) {
        printf("OtaMaster not started\n");
        return;
    }
    OtaMasterNoticeTboxSelfUpgradeResult(OTA_RET_ERR, OtaMasterTboxRebootState::OTA_TBOX_NO_REBOOT);
}

static void TestOtaMasterNoticeTboxRollbackSuccess()
{
    printf("TestOtaMasterNoticeTboxRollbackSuccess\n");
    if (!isOtaMasterStart) {
        printf("OtaMaster not started\n");
        return;
    }
    OtaMasterNoticeTboxRollbackResult(OTA_RET_OK, OtaMasterTboxRebootState::OTA_TBOX_NEED_REBOOT);
}

static void TestOtaMasterNoticeTboxRollbackFailed()
{
    printf("TestOtaMasterNoticeTboxRollbackFailed\n");
    if (!isOtaMasterStart) {
        printf("OtaMaster not started\n");
        return;
    }
    OtaMasterNoticeTboxRollbackResult(OTA_RET_ERR, OtaMasterTboxRebootState::OTA_TBOX_NO_REBOOT);
}

static void TestOtaMasterNoticeReportDownloadResultHasACK()
{
    if (!isOtaMasterStart) {
        printf("OtaMaster not started\n");
        return;
    }
    OtaMasterNoticeReportDownloadResultHasACK(g_taskId.c_str());
}

static void TestOtaMasterNoticeReportUpgradeResultHasACK()
{
    if (!isOtaMasterStart) {
        printf("OtaMaster not started\n");
        return;
    }
    OtaMasterNoticeReportUpgradeResultHasACK(g_taskId.c_str());
}

static void TestOtaMasterNoticeUpDownHighVoltageSucc()
{
    if (!isOtaMasterStart) {
        printf("OtaMaster not started\n");
        return;
    }
    OtaMasterNoticeUpDownHighVoltageResult(OTA_RET_OK, g_upgradePowerMode);
}

static void TestOtaMasterNoticeUpDownHighVoltageFailed()
{
    if (!isOtaMasterStart) {
        printf("OtaMaster not started\n");
        return;
    }
    OtaMasterNoticeUpDownHighVoltageResult(OTA_RET_ERR, g_upgradePowerMode);
}

static void TestOtaMasterNoticeOtaModeOnOffSucc()
{
    if (!isOtaMasterStart) {
        printf("OtaMaster not started\n");
        return;
    }
    OtaMasterNoticeFotaModeOnOffSwResult(OTA_RET_OK, g_otaMode);
}

static void TestOtaMasterNoticeOtaModeOnOffFailed()
{
    if (!isOtaMasterStart) {
        printf("OtaMaster not started\n");
        return;
    }
    OtaMasterNoticeFotaModeOnOffSwResult(OTA_RET_ERR, g_otaMode);
}

static void TestOtaMasterCheckMD5()
{
    const char *filePath = "/oemdata/ota_master/download/vcu_update.zip";
    const char *md5 = "d38e272f447c95fee85de4fa36af8dcd";
    if (OtaMasterCheckMD5(filePath, md5) != OTA_RET_OK) {
        OTA_LOG_E("OtaMasterCheckMD5 failed");
        return;
    }
    OTA_LOG_D("OtaMasterCheckMD5 success");
}

/**
 * @brief 直接执行VCU刷写（完成初始化+解压+刷写）
 *
 * 前置条件：
 *   1. 已将VCU固件zip包放到 /sdcard/ota_master/VCU/ 目录
 *   2. 配置文件 support_ota_ecu_config.json 已放到 /oemapp/config/
 *
 * 使用方式：直接执行测试用例26
 */
static void TestDirectVcuBrush()
{
    printf("====== 直接执行VCU刷写 ======\n");

    // ========== 0. 自动初始化IPC + OTA Master ==========
    // 必须先初始化IPC，否则UDS数据无法收发
    if (!isIpcInitialized) {
        printf("[自动初始化] InitIpc...\n");
        InitIpcIfNeeded();
        if (!isIpcInitialized) {
            printf("IPC初始化失败!UDS数据无法收发\n");
            return;
        }
    }

    if (!isOtaLogInit) {
        printf("[自动初始化] OtaMasterLogInit...\n");
        TestOtaMasterLogInit();
        if (!isOtaLogInit) {
            printf("日志初始化失败，继续执行（不影响刷写）\n");
        }
    }

    if (!isOtaMasterInit) {
        printf("[自动初始化] OtaMasterInit...\n");
        TestOtaMasterInit();
        if (!isOtaMasterInit) {
            printf("OTA Master初始化失败！请检查配置文件目录: /oemapp/config/\n");
            return;
        }
    }

    if (!isOtaMasterStart) {
        printf("[自动初始化] 注册回调 + OtaMasterStart...\n");
        TestOtaMasterRegisterReportCallback();
        TestOtaMasterRegisterQueryCallback();
        TestOtaMasterRegisterRequestCallback();  // 必须注册，UDS数据发送依赖此回调
        TestOtaMasterStart();
        if (!isOtaMasterStart) {
            printf("OTA Master启动失败！\n");
            return;
        }
    }
    printf("[初始化完成]\n");

    // ========== 1. 配置参数 ==========
    // VCU固件zip包路径（根据实际文件名修改）
    const std::string zipFilePath = "/sdcard/ota_master/VCU/tlota_vcu_sirun_test.hex";
    // 解压目标目录
    const std::string outputDir = "/sdcard/ota_master/VCU/upgrade/";
    // VCU目标版本号（刷写后校验用，需与固件实际版本一致）
    const std::string targetVer = "0002";

    // VCU刷写协议参数（对应 support_ota_ecu_config.json，can_port=3）
    OtaTaskEcuProtocolInfo protocolInfo;
    protocolInfo.physicalId = 0x7DC;
    protocolInfo.functionId = 0x7DF;
    protocolInfo.responseId = 0x7D4;
    protocolInfo.canPort = 3;

    // ========== 2. 删除旧解压目录并解压 ==========
    // printf("\n[步骤1] 删除旧解压目录: %s\n", outputDir.c_str());
    // std::string rmCmd = "rm -rf " + outputDir;
    // (void)system(rmCmd.c_str());
    // sleep(1);

    printf("[步骤2] 解压固件包: %s -> %s\n", zipFilePath.c_str(), outputDir.c_str());
    if (DecompressFile(zipFilePath, outputDir) != OTA_RET_OK) {
        OTA_LOG_E("解压失败，请检查文件路径是否正确: %s", zipFilePath.c_str());
        printf("解压失败！请确认文件存在: %s\n", zipFilePath.c_str());
        return;
    }
    printf("解压成功\n");

    // ========== 3. 查找驱动文件和升级文件 ==========
    std::string driverFile;
    std::vector<std::string> upgradeFiles;
    printf("[步骤3] 扫描解压目录查找固件文件: %s\n", outputDir.c_str());
    if (FindEcuDriverAndUpgradeFile(outputDir, driverFile, upgradeFiles) != OTA_RET_OK) {
        OTA_LOG_E("未找到固件文件(.hex/.s19)，请检查zip包内容");
        printf("未找到固件文件! 请确认zip包中包含 .hex 或 .s19 文件\n");
        return;
    }

    printf("驱动文件: %s\n", driverFile.empty() ? "(无)" : driverFile.c_str());
    printf("升级文件数量: %zu\n", upgradeFiles.size());
    for (size_t i = 0; i < upgradeFiles.size(); i++) {
        printf("  升级文件[%zu]: %s\n", i, upgradeFiles[i].c_str());
    }

    // ========== 4. 执行ECU刷写 ==========
    printf("\n[步骤4] 开始执行VCU刷写(预编程→编程→后编程）...\n");
    OTA_LOG_I("开始执行VCU[0x7DC]刷写，目标版本: %s", targetVer.c_str());

    // VCU刷写前设置休眠阻止事件，避免刷写期间PM事件链表为空导致MPU提前休眠
    const EcuType_e pmEcuType = ECU_TYPE_VCU;
    if (OtaMasterPmSetSleepEvent(pmEcuType) != 0) {
        OTA_LOG_E("VCU刷写: 设置休眠阻止事件失败");
    } else {
        OTA_LOG_I("VCU刷写: 设置休眠阻止事件成功");
    }

    OtaUpgradeErrorCode error = ExecEcuBrush(0x7DC, protocolInfo, driverFile, upgradeFiles, targetVer);

    // 刷写结束清除休眠阻止事件
    if (OtaMasterPmCleanSleepEventByType(pmEcuType) != 0) {
        OTA_LOG_E("VCU刷写: 清除休眠阻止事件失败");
    } else {
        OTA_LOG_I("VCU刷写: 清除休眠阻止事件成功");
    }

    if (error == OtaUpgradeErrorCode::OTA_OK) {
        printf("\n====== VCU刷写成功!======\n");
        OTA_LOG_I("VCU刷写成功");
    } else {
        printf("\n====== VCU刷写失败!errorCode: 0x%x ======\n", static_cast<int>(error));
        OTA_LOG_E("VCU刷写失败, errorCode: 0x%x", static_cast<int>(error));
    }
}

/**
 * @brief 验证ECU软件版本并清除DTC（第27步测试）
 *        只执行版本号验证和清除故障码，不执行完整刷写流程
 */
static void TestVerifySoftwareVersion()
{
    printf("====== 验证ECU版本号 + 清除DTC ======\n");

    // ========== 0. 自动初始化IPC + OTA Master（复用TestDirectVcuBrush的初始化逻辑）==========
    if (!isIpcInitialized) {
        printf("[自动初始化] InitIpc...\n");
        InitIpcIfNeeded();
        if (!isIpcInitialized) {
            printf("IPC初始化失败!UDS数据无法收发\n");
            return;
        }
    }

    if (!isOtaLogInit) {
        printf("[自动初始化] OtaMasterLogInit...\n");
        TestOtaMasterLogInit();
        if (!isOtaLogInit) {
            printf("日志初始化失败，继续执行（不影响验证）\n");
        }
    }

    if (!isOtaMasterInit) {
        printf("[自动初始化] OtaMasterInit...\n");
        TestOtaMasterInit();
        if (!isOtaMasterInit) {
            printf("OTA Master初始化失败！\n");
            return;
        }
    }

    if (!isOtaMasterStart) {
        printf("[自动初始化] 注册回调 + OtaMasterStart...\n");
        TestOtaMasterRegisterReportCallback();
        TestOtaMasterRegisterQueryCallback();
        TestOtaMasterRegisterRequestCallback();
        TestOtaMasterStart();
        if (!isOtaMasterStart) {
            printf("OTA Master启动失败！\n");
            return;
        }
    }
    printf("[初始化完成]\n");

    // ========== 1. 配置参数 ==========
    // VCU刷写协议参数
    FotaEcuInfo_t ecuInfo = {
        .ecuId = 0x7DC,
        .phyId = 0x7DC,
        .funcId = 0x7DF,
        .resId = 0x7D4,
        .channel = 1,
        .driverFilePath = "",
        .softwareFilePaths = {}
    };

    // 目标版本号（用于验证）
    const std::string targetVer = "0002";

    FotaEcuHandle_t ecuHdl = nullptr;

    // ========== 2. 初始化FOTA ECU模块 ==========
    printf("\n[步骤1] 初始化FOTA ECU模块...\n");
    if (FotaEcuInit(&ecuHdl, &ecuInfo) != OTA_RET_OK) {
        printf("FotaEcuInit失败！phyId: 0x%x\n", ecuInfo.phyId);
        OTA_LOG_E("FotaEcuInit failed, phyId: 0x%x", ecuInfo.phyId);
        return;
    }
    printf("FotaEcuInit成功\n");
    OTA_LOG_D("FotaEcuInit success, phyId: 0x%04X", ecuInfo.phyId);

    // ========== 3. 验证软件版本 ==========
    printf("\n[步骤2] 验证ECU软件版本: %s\n", targetVer.c_str());
    int verifyRet = VerifySoftwareVersion(ecuHdl, targetVer);
    if (verifyRet == OTA_RET_OK) {
        printf("版本验证成功！ECU版本匹配: %s\n", targetVer.c_str());
        OTA_LOG_I("VerifySoftwareVersion success, version: %s", targetVer.c_str());
    } else {
        printf("版本验证失败！返回值: %d\n", verifyRet);
        OTA_LOG_E("VerifySoftwareVersion failed, ret: %d", verifyRet);
    }

    // ========== 4. 反初始化 ==========
    printf("\n[步骤4] 反初始化FOTA ECU模块...\n");
    FotaEcuDeinit(ecuHdl);
    printf("FotaEcuDeinit成功\n");
    OTA_LOG_D("FotaEcuDeinit success");

    printf("\n====== 测试完成 ======\n");
    printf("版本验证结果: %s\n", verifyRet == OTA_RET_OK ? "成功" : "失败");    
}

/**
 * @brief 直接执行TCU刷写测试（第28步测试）
 *        跳过下载流程，直接使用本地固件文件执行TCU刷写
 *
 * 前置条件：
 *   1. 已将TCU固件.hex包放到 /sdcard/ota_master/TCU/ 目录
 *   2. 配置文件 support_ota_ecu_config.json 已放到 /oemapp/config/
 *
 * 使用方式：直接执行测试用例28
 */
static void TestDirectTcuBrush()
{
    printf("====== 直接执行TCU刷写 ======\n");

    // ========== 0. 自动初始化IPC + OTA Master ==========
    // 必须先初始化IPC，否则UDS数据无法收发
    if (!isIpcInitialized) {
        printf("[自动初始化] InitIpc...\n");
        InitIpcIfNeeded();
        if (!isIpcInitialized) {
            printf("IPC初始化失败!UDS数据无法收发\n");
            return;
        }
    }

    if (!isOtaLogInit) {
        printf("[自动初始化] OtaMasterLogInit...\n");
        TestOtaMasterLogInit();
        if (!isOtaLogInit) {
            printf("日志初始化失败，继续执行（不影响刷写）\n");
        }
    }

    if (!isOtaMasterInit) {
        printf("[自动初始化] OtaMasterInit...\n");
        TestOtaMasterInit();
        if (!isOtaMasterInit) {
            printf("OTA Master初始化失败！请检查配置文件目录: /oemapp/config/\n");
            return;
        }
    }

    if (!isOtaMasterStart) {
        printf("[自动初始化] 注册回调 + OtaMasterStart...\n");
        TestOtaMasterRegisterReportCallback();
        TestOtaMasterRegisterQueryCallback();
        TestOtaMasterRegisterRequestCallback();  // 必须注册，UDS数据发送依赖此回调
        TestOtaMasterStart();
        if (!isOtaMasterStart) {
            printf("OTA Master启动失败!\n");
            return;
        }
    }
    printf("[初始化完成]\n");

    // ========== 1. 配置参数 ==========
    // TCU固件文件路径（支持 .hex/.s19/.zip 格式）
    const std::string firmwareFilePath = "/sdcard/ota_master/TCU/tlota_tcu_sirun_test.hex";
    // TCU目标版本号（刷写后校验用，需与固件实际版本一致）
    const std::string targetVer = "0002";

    // TCU刷写协议参数（对应 support_ota_ecu_config.json，can_port=3）
    OtaTaskEcuProtocolInfo protocolInfo;
    protocolInfo.physicalId = 0x18DA03F1;
    protocolInfo.functionId = 0x18DBFFF1;
    protocolInfo.responseId = 0x18DAF103;
    protocolInfo.canPort = 3;

    // ========== 2. 检查固件文件是否存在 ==========
    printf("\n[步骤1] 检查固件文件是否存在: %s\n", firmwareFilePath.c_str());
    
    // 检查文件是否存在（使用 stat 检查是否为普通文件）
    struct stat fileStat;
    if (stat(firmwareFilePath.c_str(), &fileStat) != 0) {
        OTA_LOG_E("固件文件不存在: %s", firmwareFilePath.c_str());
        printf("固件文件不存在！请确认文件路径: %s\n", firmwareFilePath.c_str());
        return;
    }    
    
    // 检查文件扩展名是否为 .hex 或 .s19
    std::string fileExt;
    size_t pos = firmwareFilePath.find_last_of(".");
    if (pos != std::string::npos) {
        fileExt = firmwareFilePath.substr(pos);
        // 转换为小写进行比较
        std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::tolower);
    }
    
    if (fileExt != ".hex" && fileExt != ".s19") {
        OTA_LOG_E("固件文件格式不支持: %s，仅支持 .hex 或 .s19 格式", fileExt.c_str());
        printf("固件文件格式不支持！仅支持 .hex 或 .s19 格式，当前格式: %s\n", fileExt.c_str());
        return;
    }
    
    printf("[步骤2] 固件文件检查通过，格式: %s，大小: %ld bytes\n", fileExt.c_str(), fileStat.st_size);

    // ========== 3. 直接使用固件文件 ==========
    std::string driverFile;  // TCU通常无驱动文件
    std::vector<std::string> upgradeFiles;
    upgradeFiles.push_back(firmwareFilePath);  // 直接使用固件文件路径
    
    printf("驱动文件: (无)\n");
    printf("升级文件数量: 1\n");
    printf("  升级文件[0]: %s\n", firmwareFilePath.c_str());

    // ========== 4. 执行ECU刷写 ==========
    printf("\n[步骤4] 开始执行TCU刷写(预编程→编程→后编程）...\n");
    OTA_LOG_I("开始执行TCU[0x03F1]刷写，目标版本: %s", targetVer.c_str());

    // TCU刷写前设置休眠阻止事件，避免刷写期间PM事件链表为空导致MPU提前休眠
    const EcuType_e pmEcuType = ECU_TYPE_OTHER;
    if (OtaMasterPmSetSleepEvent(pmEcuType) != 0) {
        OTA_LOG_E("TCU刷写: 设置休眠阻止事件失败");
    } else {
        OTA_LOG_I("TCU刷写: 设置休眠阻止事件成功");
    }

    OtaUpgradeErrorCode error = ExecEcuBrush(0x03F1, protocolInfo, driverFile, upgradeFiles, targetVer);

    // 刷写结束清除休眠阻止事件
    if (OtaMasterPmCleanSleepEventByType(pmEcuType) != 0) {
        OTA_LOG_E("TCU刷写: 清除休眠阻止事件失败");
    } else {
        OTA_LOG_I("TCU刷写: 清除休眠阻止事件成功");
    }

    if (error == OtaUpgradeErrorCode::OTA_OK) {
        printf("\n====== TCU刷写成功!======\n");
        OTA_LOG_I("TCU刷写成功");
    } else {
        printf("\n====== TCU刷写失败!errorCode: 0x%x ======\n", static_cast<int>(error));
        OTA_LOG_E("TCU刷写失败, errorCode: 0x%x", static_cast<int>(error));
    }
}

/**
 * @brief 直接执行MCU1(汇川)刷写测试（第29步测试）
 *        跳过下载流程，直接使用本地固件文件执行MCU1刷写
 *
 * 前置条件：
 *   1. 已将MCU1固件.hex/.s19包放到 /sdcard/ota_master/MCU1/ 目录
 *   2. 配置文件 support_ota_ecu_config.json 已放到 /oemapp/config/
 *
 * 使用方式：直接执行测试用例29
 * @note MCU1(汇川) ECU_ID = 0xD2FA, CAN3动力CAN, ISO-15765扩展帧
 *       协议参数对应 support_ota_ecu_config.json 中 MCU1_TM1 配置
 */
/**
 * @brief MCU1刷写公共实现 (汇川) - 由case 29~34分别调用
 * @param typeIdx MCU1子类型索引: 0=TM1, 1=TM2, 2=PDU, 3=EPS, 4=ACM, 5=DCDC
 * @note 也支持通过环境变量 TLY_MCU1_TYPE 选择 (优先级低于case参数)
 */
static void TestDirectMcu1BrushImpl(int typeIdx)
{
    /* 6个MCU1子类型表: 名称/ECU ID/物理寻址/响应ID/CAN通道 */
    static const struct {
        const char *name;
        uint16_t    ecuId;
        uint32_t    phyId;
        uint32_t    resId;
        uint8_t     canPort;
    } mcu1Map[] = {
        {"TM1",  0xD2FA, 0x18DAD2FA, 0x18DAFAD2, 3},  /* 站号2, CAN3 */
        {"TM2",  0xD9FA, 0x18DAD9FA, 0x18DAFAD9, 3},  /* 站号9, CAN3 */
        {"PDU",  0xD6FA, 0x18DAD6FA, 0x18DAFAD6, 1},  /* 站号6, CAN1 */
        {"EPS",  0xD4FA, 0x18DAD4FA, 0x18DAFAD4, 1},  /* 站号4, CAN1 */
        {"ACM",  0xD5FA, 0x18DAD5FA, 0x18DAFAD5, 1},  /* 站号5, CAN1 */
        {"DCDC", 0xD7FA, 0x18DAD7FA, 0x18DAFAD7, 1},  /* 站号7, CAN1 */
    };
    constexpr int kMcu1Count = sizeof(mcu1Map)/sizeof(mcu1Map[0]);

    if (typeIdx < 0 || typeIdx >= kMcu1Count) {
        printf("Invalid MCU1 typeIdx %d (valid range: 0~%d)\n", typeIdx, kMcu1Count - 1);
        return;
    }

    const char    *type_name = mcu1Map[typeIdx].name;
    const uint16_t ecu_id    = mcu1Map[typeIdx].ecuId;
    const uint32_t phy_id    = mcu1Map[typeIdx].phyId;
    const uint32_t res_id    = mcu1Map[typeIdx].resId;
    const uint8_t  can_port  = mcu1Map[typeIdx].canPort;

    printf("====== 直接执行MCU1_%s(汇川)刷写 [CAN%d] ======\n", type_name, can_port);

    // ========== 0. 自动初始化IPC + OTA Master ==========
    // 必须先初始化IPC，否则UDS数据无法收发
    if (!isIpcInitialized) {
        printf("[自动初始化] InitIpc...\n");
        InitIpcIfNeeded();
        if (!isIpcInitialized) {
            printf("IPC初始化失败!UDS数据无法收发\n");
            return;
        }
    }

    if (!isOtaLogInit) {
        printf("[自动初始化] OtaMasterLogInit...\n");
        TestOtaMasterLogInit();
        if (!isOtaLogInit) {
            printf("日志初始化失败，继续执行（不影响刷写）\n");
        }
    }

    if (!isOtaMasterInit) {
        printf("[自动初始化] OtaMasterInit...\n");
        TestOtaMasterInit();
        if (!isOtaMasterInit) {
            printf("OTA Master初始化失败！请检查配置文件目录: /oemapp/config/\n");
            return;
        }
    }

    if (!isOtaMasterStart) {
        printf("[自动初始化] 注册回调 + OtaMasterStart...\n");
        TestOtaMasterRegisterReportCallback();
        TestOtaMasterRegisterQueryCallback();
        TestOtaMasterRegisterRequestCallback();  // 必须注册，UDS数据发送依赖此回调
        TestOtaMasterStart();
        if (!isOtaMasterStart) {
            printf("OTA Master启动失败!\n");
            return;
        }
    }
    printf("[初始化完成]\n");

    // ========== 1. 配置参数 ==========
    // MCU1固件文件路径（支持 .hex/.s19 格式）
    char firmwarePathBuf[128];
    snprintf(firmwarePathBuf, sizeof(firmwarePathBuf),
             "/sdcard/ota_master/MCU1/tlota_mcu1_%s_sirun_test.hex", type_name);
    const std::string firmwareFilePath = firmwarePathBuf;
    // MCU1目标版本号（刷写后校验用，需与固件实际版本一致）
    const std::string targetVer = "0002";

    // MCU1刷写协议参数（TM1/TM2走CAN3动力, PDU/EPS/ACM/DCDC走CAN1整车）
    OtaTaskEcuProtocolInfo protocolInfo;
    protocolInfo.physicalId = phy_id;
    protocolInfo.functionId = 0x18DBFFFA;
    protocolInfo.responseId = res_id;
    protocolInfo.canPort = can_port;

    // ========== 2. 检查固件文件是否存在 ==========
    printf("\n[步骤1] 检查固件文件是否存在: %s\n", firmwareFilePath.c_str());

    struct stat fileStat;
    if (stat(firmwareFilePath.c_str(), &fileStat) != 0) {
        OTA_LOG_E("固件文件不存在: %s", firmwareFilePath.c_str());
        printf("固件文件不存在！请确认文件路径: %s\n", firmwareFilePath.c_str());
        return;
    }

    // 检查文件扩展名是否为 .hex 或 .s19
    std::string fileExt;
    size_t pos = firmwareFilePath.find_last_of(".");
    if (pos != std::string::npos) {
        fileExt = firmwareFilePath.substr(pos);
        std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::tolower);
    }

    if (fileExt != ".hex" && fileExt != ".s19") {
        OTA_LOG_E("固件文件格式不支持: %s，仅支持 .hex 或 .s19 格式", fileExt.c_str());
        printf("固件文件格式不支持！仅支持 .hex 或 .s19 格式，当前格式: %s\n", fileExt.c_str());
        return;
    }

    printf("[步骤2] 固件文件检查通过，格式: %s，大小: %ld bytes\n", fileExt.c_str(), fileStat.st_size);

    // ========== 3. 直接使用固件文件 ==========
    std::string driverFile;  // MCU1驱动文件（如有需填入路径，通常无）
    std::vector<std::string> upgradeFiles;
    upgradeFiles.push_back(firmwareFilePath);  // 直接使用固件文件路径

    printf("驱动文件: (无)\n");
    printf("升级文件数量: 1\n");
    printf("  升级文件[0]: %s\n", firmwareFilePath.c_str());

    // ========== 4. 执行ECU刷写 ==========
    printf("\n[步骤4] 开始执行MCU1(汇川)刷写(预编程→编程→后编程）...\n");
    OTA_LOG_I("开始执行MCU1_%s[0x%04X]刷写，目标版本: %s", type_name, ecu_id, targetVer.c_str());

    // MCU1刷写前设置休眠阻止事件，避免刷写期间PM事件链表为空导致MPU提前休眠
    const EcuType_e pmEcuType = ECU_TYPE_MCU;
    if (OtaMasterPmSetSleepEvent(pmEcuType) != 0) {
        OTA_LOG_E("MCU1_%s刷写: 设置休眠阻止事件失败", type_name);
    } else {
        OTA_LOG_I("MCU1_%s刷写: 设置休眠阻止事件成功", type_name);
    }

    OtaUpgradeErrorCode error = ExecEcuBrush(ecu_id, protocolInfo, driverFile, upgradeFiles, targetVer);

    // 刷写结束清除休眠阻止事件
    if (OtaMasterPmCleanSleepEventByType(pmEcuType) != 0) {
        OTA_LOG_E("MCU1_%s刷写: 清除休眠阻止事件失败", type_name);
    } else {
        OTA_LOG_I("MCU1_%s刷写: 清除休眠阻止事件成功", type_name);
    }

    if (error == OtaUpgradeErrorCode::OTA_OK) {
        printf("\n====== MCU1_%s刷写成功!======\n", type_name);
        OTA_LOG_I("MCU1_%s刷写成功", type_name);
    } else {
        printf("\n====== MCU1_%s刷写失败!errorCode: 0x%x ======\n", type_name, static_cast<int>(error));
        OTA_LOG_E("MCU1_%s刷写失败, errorCode: 0x%x", type_name, static_cast<int>(error));
    }
}

/**
 * @brief 6 个 MCU1 子类型测试函数 (汇川)
 * @note 每个子类型固定对应一个 ECU_ID/TM/PDU/EPS/ACM/DCDC, 共用 TestDirectMcu1BrushImpl
 */
static void TestDirectMcu1TM1Brush()  { TestDirectMcu1BrushImpl(0); }
static void TestDirectMcu1TM2Brush()  { TestDirectMcu1BrushImpl(1); }
static void TestDirectMcu1PDUBrush()  { TestDirectMcu1BrushImpl(2); }
static void TestDirectMcu1EPSBrush()  { TestDirectMcu1BrushImpl(3); }
static void TestDirectMcu1ACMBrush()  { TestDirectMcu1BrushImpl(4); }
static void TestDirectMcu1DCDCBrush() { TestDirectMcu1BrushImpl(5); }

static void TestDirectTmsBrush()
{
    printf("====== 直接执行TMS(热管理)刷写 ======\n");

    if (!isIpcInitialized) { printf("[自动初始化] InitIpc...\n"); InitIpcIfNeeded();
        if (!isIpcInitialized) { printf("IPC初始化失败!UDS数据无法收发\n"); return; } }
    if (!isOtaLogInit) { printf("[自动初始化] OtaMasterLogInit...\n"); TestOtaMasterLogInit();
        if (!isOtaLogInit) { printf("日志初始化失败，继续执行（不影响刷写）\n"); } }
    if (!isOtaMasterInit) { printf("[自动初始化] OtaMasterInit...\n"); TestOtaMasterInit();
        if (!isOtaMasterInit) { printf("OTA Master初始化失败！\n"); return; } }
    if (!isOtaMasterStart) { printf("[自动初始化] 注册回调 + OtaMasterStart...\n");
        TestOtaMasterRegisterReportCallback(); TestOtaMasterRegisterQueryCallback();
        TestOtaMasterRegisterRequestCallback(); TestOtaMasterStart();
        if (!isOtaMasterStart) { printf("OTA Master启动失败!\n"); return; } }
    printf("[初始化完成]\n");

    const std::string firmwareFilePath = "/sdcard/ota_master/TMS/tlota_tms_sirun_test.hex";
    const std::string targetVer = "0002";

    OtaTaskEcuProtocolInfo protocolInfo;
    protocolInfo.physicalId = 0x67C;
    protocolInfo.functionId = 0x7DF;
    protocolInfo.responseId = 0x674;
    protocolInfo.canPort = 1;

    printf("\n[步骤1] 检查固件文件是否存在: %s\n", firmwareFilePath.c_str());
    struct stat fileStat;
    if (stat(firmwareFilePath.c_str(), &fileStat) != 0) {
        printf("固件文件不存在！请确认文件路径: %s\n", firmwareFilePath.c_str());
        return;
    }
    std::string fileExt; size_t pos = firmwareFilePath.find_last_of(".");
    if (pos != std::string::npos) { fileExt = firmwareFilePath.substr(pos);
        std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::tolower); }
    if (fileExt != ".hex" && fileExt != ".s19") {
        printf("固件文件格式不支持！仅支持 .hex 或 .s19 格式\n"); return; }
    printf("[步骤2] 固件文件检查通过，格式: %s，大小: %ld bytes\n", fileExt.c_str(), fileStat.st_size);

    std::string driverFile;
    std::vector<std::string> upgradeFiles;
    upgradeFiles.push_back(firmwareFilePath);

    printf("驱动文件: (无)\n升级文件数量: 1\n  升级文件[0]: %s\n", firmwareFilePath.c_str());
    printf("\n[步骤4] 开始执行TMS刷写(预编程→编程→后编程）...\n");
    OTA_LOG_I("开始执行TMS[0x067C]刷写，目标版本: %s", targetVer.c_str());

    // TMS刷写前设置休眠阻止事件，避免刷写期间PM事件链表为空导致MPU提前休眠
    const EcuType_e pmEcuType = ECU_TYPE_TMS;
    if (OtaMasterPmSetSleepEvent(pmEcuType) != 0) {
        OTA_LOG_E("TMS刷写: 设置休眠阻止事件失败");
    } else {
        OTA_LOG_I("TMS刷写: 设置休眠阻止事件成功");
    }

    OtaUpgradeErrorCode error = ExecEcuBrush(0x67C, protocolInfo, driverFile, upgradeFiles, targetVer);

    // 刷写结束清除休眠阻止事件
    if (OtaMasterPmCleanSleepEventByType(pmEcuType) != 0) {
        OTA_LOG_E("TMS刷写: 清除休眠阻止事件失败");
    } else {
        OTA_LOG_I("TMS刷写: 清除休眠阻止事件成功");
    }

    if (error == OtaUpgradeErrorCode::OTA_OK) {
        printf("\n====== TMS刷写成功!======\n");
        OTA_LOG_I("TMS刷写成功");
    } else {
        printf("\n====== TMS刷写失败!errorCode: 0x%x ======\n", static_cast<int>(error));
        OTA_LOG_E("TMS刷写失败, errorCode: 0x%x", static_cast<int>(error));
    }
}

static void TestDirectMcu2BrushImpl(int typeIdx)
{
    static const struct {
        const char *name;
        uint16_t    ecuId;
        uint32_t    phyId;
        uint32_t    resId;
        uint8_t     canPort;
    } mcu2Map[] = {
        {"TM1",  0xD2FB, 0x18DAD2FB, 0x18DAFBD2, 3},  /* 站号2, CAN3 */
        {"TM2",  0xD9FB, 0x18DAD9FB, 0x18DAFBD9, 3},  /* 站号9, CAN3 */
        {"PDU",  0xD6FB, 0x18DAD6FB, 0x18DAFBD6, 1},  /* 站号6, CAN1 */
        {"EPS",  0xD4FB, 0x18DAD4FB, 0x18DAFBD4, 1},  /* 站号4, CAN1 */
        {"ACM",  0xD5FB, 0x18DAD5FB, 0x18DAFBD5, 1},  /* 站号5, CAN1 */
        {"DCDC", 0xD7FB, 0x18DAD7FB, 0x18DAFBD7, 1},  /* 站号7, CAN1 */
    };
    constexpr int kCount = sizeof(mcu2Map)/sizeof(mcu2Map[0]);
    if (typeIdx < 0 || typeIdx >= kCount) { printf("Invalid MCU2 typeIdx\n"); return; }

    const char *type_name = mcu2Map[typeIdx].name;
    printf("====== 直接执行MCU2_%s(汇川)刷写 [CAN%d] ======\n", type_name, mcu2Map[typeIdx].canPort);

    if (!isIpcInitialized) { printf("[自动初始化] InitIpc...\n"); InitIpcIfNeeded();
        if (!isIpcInitialized) { printf("IPC初始化失败!\n"); return; } }
    if (!isOtaLogInit) { TestOtaMasterLogInit(); isOtaLogInit = true; }
    if (!isOtaMasterInit) { TestOtaMasterInit(); isOtaMasterInit = true;
        if (!isOtaMasterInit) { printf("OTA Master初始化失败!\n"); return; } }
    if (!isOtaMasterStart) { TestOtaMasterRegisterReportCallback(); TestOtaMasterRegisterQueryCallback();
        TestOtaMasterRegisterRequestCallback(); TestOtaMasterStart(); isOtaMasterStart = true;
        if (!isOtaMasterStart) { printf("OTA Master启动失败!\n"); return; } }
    printf("[初始化完成]\n");

    char fwPath[128]; snprintf(fwPath, sizeof(fwPath), "/sdcard/ota_master/MCU2/tlota_mcu2_%s_sirun_test.hex", type_name);
    const std::string firmwareFilePath = fwPath;
    const std::string targetVer = "0002";

    OtaTaskEcuProtocolInfo pi;
    pi.physicalId = mcu2Map[typeIdx].phyId; pi.functionId = 0x18DBFFFB;
    pi.responseId = mcu2Map[typeIdx].resId; pi.canPort = mcu2Map[typeIdx].canPort;

    struct stat st;
    if (stat(firmwareFilePath.c_str(), &st) != 0) { printf("固件文件不存在: %s\n", firmwareFilePath.c_str()); return; }
    std::string ext; size_t p = firmwareFilePath.find_last_of(".");
    if (p != std::string::npos) { ext = firmwareFilePath.substr(p);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower); }
    if (ext != ".hex" && ext != ".s19") { printf("格式不支持: %s\n", ext.c_str()); return; }

    std::string driverFile;
    std::vector<std::string> upgradeFiles; upgradeFiles.push_back(firmwareFilePath);

    OTA_LOG_I("开始执行MCU2_%s[0x%04X]刷写", type_name, mcu2Map[typeIdx].ecuId);

    // MCU2刷写前设置休眠阻止事件，避免刷写期间PM事件链表为空导致MPU提前休眠
    const EcuType_e pmEcuType = ECU_TYPE_MCU;
    if (OtaMasterPmSetSleepEvent(pmEcuType) != 0) {
        OTA_LOG_E("MCU2_%s刷写: 设置休眠阻止事件失败", type_name);
    } else {
        OTA_LOG_I("MCU2_%s刷写: 设置休眠阻止事件成功", type_name);
    }

    OtaUpgradeErrorCode err = ExecEcuBrush(mcu2Map[typeIdx].ecuId, pi, driverFile, upgradeFiles, targetVer);

    // 刷写结束清除休眠阻止事件
    if (OtaMasterPmCleanSleepEventByType(pmEcuType) != 0) {
        OTA_LOG_E("MCU2_%s刷写: 清除休眠阻止事件失败", type_name);
    } else {
        OTA_LOG_I("MCU2_%s刷写: 清除休眠阻止事件成功", type_name);
    }

    if (err == OtaUpgradeErrorCode::OTA_OK) {
        printf("\n====== MCU2_%s刷写成功!======\n", type_name);
    } else {
        printf("\n====== MCU2_%s刷写失败!errorCode: 0x%x ======\n", type_name, static_cast<int>(err));
    }
}

static void TestDirectMcu2TM1Brush()  { TestDirectMcu2BrushImpl(0); }
static void TestDirectMcu2TM2Brush()  { TestDirectMcu2BrushImpl(1); }
static void TestDirectMcu2PDUBrush()  { TestDirectMcu2BrushImpl(2); }
static void TestDirectMcu2EPSBrush()  { TestDirectMcu2BrushImpl(3); }
static void TestDirectMcu2ACMBrush()  { TestDirectMcu2BrushImpl(4); }
static void TestDirectMcu2DCDCBrush() { TestDirectMcu2BrushImpl(5); }

// 集中管理所有测试用例
constexpr TestCase testCases[] = {
    { 1, "OtaMasterLogInit", TestOtaMasterLogInit},
    { 2, "OtaMasterInit", TestOtaMasterInit},
    { 3, "OtaMasterRegisterReportCallback", TestOtaMasterRegisterReportCallback},
    { 4, "OtaMasterRegisterQueryCallback", TestOtaMasterRegisterQueryCallback},
    { 5, "OtaMasterRegisterRequestCallback", TestOtaMasterRegisterRequestCallback},
    { 6, "OtaMasterStart", TestOtaMasterStart},
    { 7, "OtaMasterStartOtaTask", TestOtaMasterStartOtaTask},
    { 8, "OtaMasterClearOtaTask", TestOtaMasterClearOtaTask},
    { 9, "OtaMasterConfirmStartUpgrading", TestOtaMasterConfirmStartUpgrading},
    {10, "OtaMasterNoticeOtaModeOnOffSuccess", TestOtaMasterNoticeOtaModeOnOffSucc},
    {11, "OtaMasterNoticeOtaModeOnOffFailed", TestOtaMasterNoticeOtaModeOnOffFailed},
    {12, "OtaMasterNoticeUpDownHighVoltageSuccess", TestOtaMasterNoticeUpDownHighVoltageSucc},
    {13, "OtaMasterNoticeUpDownHighVoltageFailed", TestOtaMasterNoticeUpDownHighVoltageFailed},
    {14, "OtaMasterNoticeTboxSelfUpgradeSuccess", TestOtaMasterNoticeTboxSelfUpgradeSuccess},
    {15, "OtaMasterNoticeTboxSelfUpgradeFailed", TestOtaMasterNoticeTboxSelfUpgradeFailed},
    {16, "TestOtaMasterNoticeTboxRollbackSuccess", TestOtaMasterNoticeTboxRollbackSuccess},
    {17, "TestOtaMasterNoticeTboxRollbackFailed", TestOtaMasterNoticeTboxRollbackFailed},
    {18, "OtaMasterNoticeEcuVersionRollbackResult", TestOtaMasterNoticeEcuVersionRollbackResult},
    {19, "OtaMasterNoticeReportDownloadResultHasACK", TestOtaMasterNoticeReportDownloadResultHasACK},
    {20, "OtaMasterNoticeReportUpgradeResultHasACK", TestOtaMasterNoticeReportUpgradeResultHasACK},
    {21, "OtaMasterStop", TestOtaMasterStop},
    {22, "OtaMasterCheckMD5", TestOtaMasterCheckMD5},
    {23, "OtaMasterLogReset", TestOtaMasterLogReset},
    {24, "OtaMasterLogFinish", TestOtaMasterLogFinish},
    {25, "OtaMasterDeinit", TestOtaMasterDeinit},
    {26, "DirectVcuBrush (跳过下载直接刷写VCU)", TestDirectVcuBrush},
    {27, "VerifySoftwareVersionAndClearDtc (验证版本号)", TestVerifySoftwareVersion},
    {28, "DirectTcuBrush (跳过下载直接刷写TCU)", TestDirectTcuBrush},
    {29, "DirectMcu1TM1Brush (跳过下载直接刷写MCU1_TM1)", TestDirectMcu1TM1Brush},
    {30, "DirectMcu1TM2Brush (跳过下载直接刷写MCU1_TM2)", TestDirectMcu1TM2Brush},
    {31, "DirectMcu1PDUBrush (跳过下载直接刷写MCU1_PDU)", TestDirectMcu1PDUBrush},
    {32, "DirectMcu1EPSBrush (跳过下载直接刷写MCU1_EPS)", TestDirectMcu1EPSBrush},
    {33, "DirectMcu1ACMBrush (跳过下载直接刷写MCU1_ACM)", TestDirectMcu1ACMBrush},
    {34, "DirectMcu1DCDCBrush (跳过下载直接刷写MCU1_DCDC)", TestDirectMcu1DCDCBrush},
    {35, "DirectTmsBrush (跳过下载直接刷写TMS)", TestDirectTmsBrush},
    {36, "DirectMcu2TM1Brush (跳过下载直接刷写MCU2_TM1)", TestDirectMcu2TM1Brush},
    {37, "DirectMcu2TM2Brush (跳过下载直接刷写MCU2_TM2)", TestDirectMcu2TM2Brush},
    {38, "DirectMcu2PDUBrush (跳过下载直接刷写MCU2_PDU)", TestDirectMcu2PDUBrush},
    {39, "DirectMcu2EPSBrush (跳过下载直接刷写MCU2_EPS)", TestDirectMcu2EPSBrush},
    {40, "DirectMcu2ACMBrush (跳过下载直接刷写MCU2_ACM)", TestDirectMcu2ACMBrush},
    {41, "DirectMcu2DCDCBrush (跳过下载直接刷写MCU2_DCDC)", TestDirectMcu2DCDCBrush}
};

static void ExecuteTestCase(int32_t caseId)
{
    auto iter = std::find_if(std::begin(testCases), std::end(testCases),
                             [caseId](const TestCase &tc) { return tc.id == caseId; });

    if (iter != std::end(testCases)) {
        iter->func();
    } else {
        std::cerr << "Invalid test case: " << caseId << std::endl;
    }
}

static void ShowHelp()
{
    std::cout << "************************************************************" << std::endl;
    std::cout << "                      OtaMasterDemo                       " << std::endl;
    std::cout << "************************************************************" << std::endl;
    for (const auto& tc : testCases) {
        std::cout << std::setw(2) << tc.id << ". " << tc.description << "\n";
    }
    std::cout << "99. Exit\n";
}

int main(int argc, char *argv[])
{
    std::cout << std::unitbuf;  // 开启实时输出
    ShowHelp();
    while (1) {
        std::cout << "\nEnter test case ID (99 to exit): ";
        int input;
        if (std::cin >> input) {
            if (input == 99) {
                break;
            }
            ExecuteTestCase(input);
        } else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << "Invalid input!" << std::endl;
        }
    }

    printf("OtaMasterDemo exit\n");
    return 0;
}
