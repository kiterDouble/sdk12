#include <atomic>
#include "ota_master_api.h"
#include "ota_master_state_manage.h"
#include "ota_master_safety.h"
#include "ota_master_app_callback.h"
#include "ota_master_upgrade.h"
#include "ota_master_log.h"
#include "ota_master_ecu_brush.h"
#include "ota_master_config.h"
#include "ota_master_archive_manage.h"
#include "ota_master_utils.h"
#include "uds_tp.h"

namespace {
static std::atomic<bool> g_isOtaMasterInit(false); // OTA Master是否初始化
} // namespace
int OtaMasterInit(const char *configFilePath, const char *downloadFolder, const char *unzipFolder)
{
    OTA_LOG_E("Enter OTA Master OtaMasterInit...\n");
    if (g_isOtaMasterInit.load()) {
        OTA_LOG_E("OTA Master has already been initialized\n");
        return OTA_RET_ERR;
    }

    if (configFilePath == nullptr) {
        OTA_LOG_E("OtaMasterInit failed, configFilePath is nullptr\n");
        return OTA_RET_ERR;
    }

    if (downloadFolder == nullptr) {
        OTA_LOG_E("OtaMasterInit failed, downloadFolder is nullptr\n");
        return OTA_RET_ERR;
    }

    if (unzipFolder == nullptr) {
        OTA_LOG_E("OtaMasterInit failed, unzipFolder is nullptr\n");
        return OTA_RET_ERR;
    }

    // 先初始化日志模块
    OtaLogErrorCode logError = OtaMasterLogInit("otaMasterApi", downloadFolder);
    if (logError != OTA_LOG_OK && logError != OTA_LOG_ALREADY_INITIALIZED) {
        OTA_LOG_E("OtaMasterLogInit failed, error: %d\n", logError);
        return OTA_RET_ERR;
    }

    if (OtaStateManageInit(configFilePath, downloadFolder, unzipFolder) != OTA_RET_OK) {
        OTA_LOG_E("OtaMasterInit failed, OtaStateManageInit failed\n");
        return OTA_RET_ERR;

    }
    OTA_LOG_E("OtaStateManageInit success\n");
    g_isOtaMasterInit.store(true);
    return OTA_RET_OK;
}

void OtaMasterDeinit(void)
{
    if (!g_isOtaMasterInit.load()) {
        OTA_LOG_E("OTA Master has not been initialized\n");
        return;
    }
    OtaStateManageDeinit();
    OtaMasterLogFinish();
    g_isOtaMasterInit.store(false);
}

int OtaMasterRegisterReportCallback(const StOtaMasterReportCallback *callback)
{
    if (!g_isOtaMasterInit.load()) {
        OTA_LOG_E("Register callback failed, OTA Master has not been initialized\n");
        return OTA_RET_ERR;
    }

    return RegisterReportCallback(callback);
}

void OtaMasterUnregisterReportCallback(void)
{
    UnregisterReportCallback();
}

int OtaMasterRegisterQueryCallback(const StOtaMasterQueryCallback *callback)
{
    if (!g_isOtaMasterInit.load()) {
        OTA_LOG_E("Register callback failed, OTA Master has not been initialized\n");
        return OTA_RET_ERR;
    }

    return RegisterQueryCallback(callback);
}

void OtaMasterUnregisterQueryCallback(void)
{
    UnregisterQueryCallback();
}

int OtaMasterRegisterRequestCallback(const StOtaMasterRequestCallback *callback)
{
    if (!g_isOtaMasterInit.load()) {
        OTA_LOG_E("Register callback failed, OTA Master has not been initialized\n");
        return OTA_RET_ERR;
    }

    return RegisterRequestCallback(callback);
}

void OtaMasterUnregisterRequestCallback(void)
{
    UnregisterRequestCallback();
}

OtaMasterStatus OtaMasterGetOtaStatus(void)
{
    return GetOtaStatus();
}

int OtaMasterStart(void)
{
    if (!g_isOtaMasterInit.load()) {
        OTA_LOG_E("OtaMasterStart failed, libota_master has not been initialized\n");
        return OTA_RET_ERR;
    }
    return StartOtaStateMachine();
}

int OtaMasterStop(void)
{
    if (!g_isOtaMasterInit.load()) {
        OTA_LOG_E("OtaMasterStop failed, libota_master has not been initialized\n");
        return OTA_RET_ERR;
    }
    return StopOtaStateMachine();
}

int OtaMasterStartOtaTask(const StOtaTaskPolicyInfo *policyInfo)
{
    if (!g_isOtaMasterInit.load()) {
        OTA_LOG_E("OtaMasterStartOtaTask failed, libota_master has not been initialized\n");
        return OTA_RET_ERR;
    }
    return StartOtaTask(policyInfo);
}

int OtaMasterClearOtaTask(void)
{
    if (!g_isOtaMasterInit.load()) {
        OTA_LOG_E("OtaMasterClearOtaTask failed, libota_master has not been initialized\n");
        return OTA_RET_ERR;
    }
    return ClearOtaTask();
}

int OtaMasterConfirmStartUpgrading(const char *taskId)
{
    if (!g_isOtaMasterInit.load()) {
        OTA_LOG_E("OtaMasterConfirmStartUpgrading failed, libota_master has not been initialized\n");
        return OTA_RET_ERR;
    }

    if (taskId == nullptr) {
        OTA_LOG_E("OtaMasterConfirmStartUpgrading failed, taskId is nullptr\n");
        return OTA_RET_ERR;
    }
    return ConfirmStartUpgrading(taskId);
}

int OtaMasterAES128ECBEncrypt(const uint8_t *key, int keyLen, const uint8_t *input, int inputLen, uint8_t **output)
{
    if (key == nullptr) {
        OTA_LOG_E("AES128ECBEncrypt failed, key is nullptr\n");
        return 0;
    }

    if (keyLen <= 0) {
        OTA_LOG_E("AES128ECBEncrypt failed, keyLen:%d is invalid\n", keyLen);
        return 0;
    }

    if (input == nullptr) {
        OTA_LOG_E("AES128ECBEncrypt failed, input is nullptr\n");
        return 0;
    }

    if (inputLen <= 0) {
        OTA_LOG_E("AES128ECBEncrypt failed, inputLen:%d is invalid\n", inputLen);
        return 0;
    }

    std::string keyStr(reinterpret_cast<const char *>(key), keyLen);
    std::string inputStr(reinterpret_cast<const char *>(input), inputLen);
    return AES128ECBEncrypt(keyStr, inputStr, output);
}

int OtaMasterAES128ECBDecrypt(const uint8_t *key, int keyLen, const uint8_t *input, int inputLen, uint8_t **output)
{
    if (key == nullptr) {
        OTA_LOG_E("AES128ECBEncrypt failed, key is nullptr\n");
        return 0;
    }

    if (keyLen <= 0) {
        OTA_LOG_E("AES128ECBEncrypt failed, keyLen:%d is invalid\n", keyLen);
        return 0;
    }

    if (input == nullptr) {
        OTA_LOG_E("AES128ECBEncrypt failed, input is nullptr\n");
        return 0;
    }

    if (inputLen <= 0) {
        OTA_LOG_E("AES128ECBEncrypt failed, inputLen:%d is invalid\n", inputLen);
        return 0;
    }

    std::string keyStr(reinterpret_cast<const char *>(key), keyLen);
    std::string inputStr(reinterpret_cast<const char *>(input), inputLen);
    return AES128ECBDecrypt(keyStr, inputStr, output);
}

void OtaMasterNoticeMcuUdsDataAck(const uint8_t *data, uint16_t dataLen)
{
    ReceiveMcuAck(data, dataLen);
}

void OtaMasterNoticeEcuUdsData(const uint8_t *data, uint16_t dataLen)
{
    ReceiveUdsDataAndProcess(data, dataLen);
}

int OtaMasterCheckMD5(const char *filePath, const char *targetMD5)
{
    if (filePath == nullptr) {
        OTA_LOG_E("Check MD5 failed, filePath is nullptr\n");
        return OTA_RET_ERR;
    }

    if (targetMD5 == nullptr) {
        OTA_LOG_E("Check MD5 failed, targetMD5 is nullptr\n");
        return OTA_RET_ERR;
    }

    return CheckMD5(filePath, targetMD5);
}

void OtaMasterNoticeEcuVersionAsyncQueryResult(const StEcuVersionInfo *ecuVersions, uint8_t ecuCnt)
{
    NoticeEcuVersionAsyncQueryResult(ecuVersions, ecuCnt);
}

void OtaMasterNoticeUpgradePreconditionsAsyncQueryResult(const StUpgradePreconditions *preconditons)
{
    NoticeUpgradePreconditionsAsyncQueryResult(preconditons);
}

void OtaMasterNoticeTboxRollbackResult(int result, OtaMasterTboxRebootState rebootState)
{
    NoticeTboxRollbackResult(result, rebootState);
}

void OtaMasterNoticeTboxSelfUpgradeResult(int result, OtaMasterTboxRebootState rebootState)
{
    NoticeTboxSelfUpgradeResult(result, rebootState);
}

void OtaMasterNoticeReportDownloadResultHasACK(const char *taskId)
{
    std::string taskIdStr = (taskId == nullptr) ? "" : taskId;
    NoticeReportResultHasACK(taskIdStr);
}

void OtaMasterNoticeReportUpgradeResultHasACK(const char *taskId)
{
    std::string taskIdStr = (taskId == nullptr) ? "" : taskId;
    NoticeReportResultHasACK(taskIdStr);
}

void OtaMasterNoticeUpDownHighVoltageResult(int result, OtaMasterUpgradePowerMode powerMode)
{
    NoticeUpDownHighVoltageResult(result, powerMode);
}

void OtaMasterNoticeFotaModeOnOffSwResult(int result, CanSignalOtaMode mode)
{
    NoticeFotaModeOnOffSwResult(result, mode);
}

int OtaMasterExecEcuBrush(uint16_t ecuId, const char *driverFilePath, const char *softwareFilePath,
                          const char *targetVer, OtaMasterEcuBrushProgressCallback progressCb, void *userdata)
{
    if (!g_isOtaMasterInit.load()) {
        OTA_LOG_E("ExecEcuBrush failed, OTA Master has not been initialized\n");
        return OTA_RET_ERR;
    }

    if (softwareFilePath == nullptr) {
        OTA_LOG_E("ExecEcuBrush failed, softwareFilePath is nullptr\n");
        return OTA_RET_ERR;
    }

    // 获取已加载的支持OTA升级的ECU配置信息,避免重复加载
    SupportOTAEcuConfigMap supportConfig;
    if (GetLoadedSupportOtaEcuConfigInfo(supportConfig) != OTA_RET_OK) {
        OTA_LOG_E("ExecEcuBrush failed, GetLoadedSupportOtaEcuConfigInfo failed\n");
        return OTA_RET_ERR;
    }

    // 查找ECU配置
    auto configIter = supportConfig.find(ecuId);
    if (configIter == supportConfig.end()) {
        OTA_LOG_E("ExecEcuBrush failed, ECU ID[0x%04x] is not supported\n", ecuId);
        return OTA_RET_ERR;
    }

    // 从配置中获取协议信息和ECU名称
    OtaTaskEcuProtocolInfo protocolInfo = configIter->second.protocolInfo;
    std::string ecuName = configIter->second.name;
    OTA_LOG_I("ExecEcuBrush: ECU ID=0x%04X, name=%s, canPort=%d, physicalId=0x%X, functionId=0x%X, responseId=0x%X\n",
              ecuId, ecuName.c_str(), protocolInfo.canPort, protocolInfo.physicalId, protocolInfo.functionId, protocolInfo.responseId);

    // 根据ECU类型构建解压/存放目录
    std::string ecuDir = "/sdcard/ota_master/" + ecuName;
    std::string actualSoftwarePath = softwareFilePath;
    OTA_LOG_I("softwareFilePath=%s, ecuDir: %s\n", softwareFilePath, ecuDir.c_str());

    // 解压/复制文件到ECU目录
    OTA_LOG_I("ExecEcuBrush: processing file %s to %s\n", softwareFilePath, ecuDir.c_str());
    
    if (DecompressFile(softwareFilePath, ecuDir) != OTA_RET_OK) {
        OTA_LOG_E("ExecEcuBrush failed, process file failed: %s\n", softwareFilePath);
        return OTA_RET_ERR;
    }
    
    // 从ECU目录中查找固件文件
    std::string driverFileFromZip;
    std::vector<std::string> upgradeFilesFromZip;
    if (FindEcuDriverAndUpgradeFile(ecuDir, driverFileFromZip, upgradeFilesFromZip) != OTA_RET_OK) {
        OTA_LOG_E("ExecEcuBrush failed, cannot find upgrade file in: %s\n", ecuDir.c_str());
        return OTA_RET_ERR;
    }
    
    // 使用解压/复制后的文件路径
    if (upgradeFilesFromZip.size() > 0) {
        actualSoftwarePath = upgradeFilesFromZip[0];
        OTA_LOG_I("ExecEcuBrush: using file: %s\n", actualSoftwarePath.c_str());
    }

    // 构造文件路径
    std::string driverFile = (driverFilePath == nullptr) ? "" : driverFilePath;
    std::vector<std::string> softwareFiles;
    softwareFiles.push_back(actualSoftwarePath);
    std::string targetVersion = (targetVer == nullptr) ? "" : targetVer;

    // 执行ECU刷写(带进度回调)
    OtaUpgradeErrorCode error = ExecEcuBrushWithProgress(ecuId, protocolInfo, driverFile, softwareFiles, targetVersion, progressCb, userdata);
    if (error == OtaUpgradeErrorCode::OTA_OK) {
        OTA_LOG_I("ExecEcuBrush success, ECU ID=0x%04X\n", ecuId);
        return OTA_RET_OK;
    } else {
        OTA_LOG_E("ExecEcuBrush failed, ECU ID=0x%04X, error=%d\n", ecuId, error);
        return OTA_RET_ERR;
    }
}