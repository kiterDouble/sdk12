#include "ota_master_app_callback.h"
#include "ota_master_log.h"
#include <mutex>

namespace {
static ReportStartDownloadCallbackFunc g_reportStartDownload = nullptr;
static ReportDownloadResultCallbackFunc g_reportDownloadResult = nullptr;
static ReportUpgradeResultCallbackFunc g_reportUpgradeResult = nullptr;
static ReportOtaTaskClearResultCallbackFunc g_reportOtaTaskClearResult = nullptr;
static ReportPreconditionsCheckResultCallbackFunc g_reportPreconditionsCheckResult = nullptr;
static ReportStartUpgradeCallbackFunc g_reportStartUpgrade = nullptr;
static ReportStartRollbackCallbackFunc g_reportStartRollback = nullptr;
static QueryTaskECUVersionSyncCallbackFunc g_queryTaskECUVersionSync = nullptr;
static QueryTaskECUVersionAsyncCallbackFunc g_queryTaskECUVersionAsync = nullptr;
static QueryUpgradePreconditionsSyncCallbackFunc g_queryUpgradePreconditionsSync = nullptr;
static QueryUpgradePreconditionsAsyncCallbackFunc g_queryUpgradePreconditionsAsync = nullptr;
static SendUdsDataCallbackFunc g_sendUdsData = nullptr;
static SendUdsDataAckCallbackFunc g_sendUdsDataAck = nullptr;
static RequestUpDownHighVoltageCallbackFunc g_requestUpDownHighVoltage = nullptr;
static RequestFotaModeOnOffSwCallbackFunc g_requestFotaModeOnOff = nullptr;
static RequestTboxSelfUpgradeCallbackFunc g_requestTboxSelfUpgrade = nullptr;
static RequestTboxSelfRollbackCallbackFunc g_requestTboxSelfRollback = nullptr;
static RequestTboxRebootCallbackFunc g_requestTboxReboot = nullptr;
static std::mutex g_callbackMutex;
} // namespace

int RegisterReportCallback(const StOtaMasterReportCallback *callback)
{
    if (callback == nullptr) {
        OTA_LOG_E("Register report type callback failed, callback is nullpter");
        return OTA_RET_ERR;
    }
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    g_reportStartDownload = callback->startDownloadCb;
    g_reportDownloadResult = callback->downloadResultCb;
    g_reportUpgradeResult = callback->upgradeResultCb;
    g_reportOtaTaskClearResult = callback->otaTaskClearResultCb;
    g_reportPreconditionsCheckResult = callback->preconditionsCheckResultCb;
    g_reportStartUpgrade = callback->startUpgradeCb;
    g_reportStartRollback = callback->startRollbackCb;
    return OTA_RET_OK;
}

void UnregisterReportCallback(void)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    g_reportStartDownload = nullptr;
    g_reportDownloadResult = nullptr;
    g_reportUpgradeResult = nullptr;
    g_reportOtaTaskClearResult = nullptr;
    g_reportPreconditionsCheckResult = nullptr;
    g_reportStartUpgrade = nullptr;
    g_reportStartRollback = nullptr;
}

int RegisterQueryCallback(const StOtaMasterQueryCallback *callback)
{
    if (callback == nullptr) {
        OTA_LOG_E("Register query type callback failed, callback is nullpter");
        return OTA_RET_ERR;
    }
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    g_queryTaskECUVersionSync = callback->queryTaskECUVerSyncCb;
    g_queryTaskECUVersionAsync = callback->queryTaskECUVerAsyncCb;
    g_queryUpgradePreconditionsSync = callback->queryUpgradePrecondsSyncCb;
    g_queryUpgradePreconditionsAsync = callback->queryUpgradePrecondsAsyncCb;
    return OTA_RET_OK;
}

void UnregisterQueryCallback(void)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    g_queryTaskECUVersionSync = nullptr;
    g_queryTaskECUVersionAsync = nullptr;
    g_queryUpgradePreconditionsSync = nullptr;
    g_queryUpgradePreconditionsAsync = nullptr;
}

int RegisterRequestCallback(const StOtaMasterRequestCallback *callback)
{
    if (callback == nullptr) {
        OTA_LOG_E("Register request type callback failed, callback is nullpter");
        return OTA_RET_ERR;
    }
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    g_sendUdsData = callback->sendUdsDataCb;
    g_sendUdsDataAck = callback->sendUdsDataAckCb;
    g_requestUpDownHighVoltage = callback->reqUpDownHighVoltCb;
    g_requestFotaModeOnOff = callback->reqFotaModeOnOff;
    g_requestTboxSelfUpgrade = callback->reqTboxSelfUpgradeCb;
    g_requestTboxSelfRollback = callback->reqTboxSelfRollbackCb;
    g_requestTboxReboot = callback->reqTboxRebootCb;
    return OTA_RET_OK;
}

void UnregisterRequestCallback(void)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    g_sendUdsData = nullptr;
    g_sendUdsDataAck = nullptr;
    g_requestUpDownHighVoltage = nullptr;
    g_requestFotaModeOnOff = nullptr;
    g_requestTboxSelfUpgrade = nullptr;
    g_requestTboxSelfRollback = nullptr;
    g_requestTboxReboot = nullptr;
}

int ReportStartDownloadCallback(const std::string &taskId)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_reportStartDownload == nullptr) {
        OTA_LOG_E("ReportStartDownloadCallback failed, g_reportStartDownload is nullptr");
        return OTA_RET_ERR;
    }
    return g_reportStartDownload(taskId.c_str());
}

int ReportDownloadResultCallback(const std::string &taskId, int result, OtaUpgradeErrorCode errCode)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_reportDownloadResult == nullptr) {
        OTA_LOG_E("ReportDownloadResultCallback failed, g_reportDownloadResult is nullptr");
        return OTA_RET_ERR;
    }
    return g_reportDownloadResult(taskId.c_str(), result, errCode);
}

int ReportUpgradeResultCallback(const std::string &taskId, const std::string &vehicleOtaVer, OtaUpgradeResult result,
                                OtaUpgradeErrorCode errCode)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_reportUpgradeResult == nullptr) {
        OTA_LOG_E("ReportUpgradeResultCallback failed, g_reportUpgradeResult is nullptr");
        return OTA_RET_ERR;
    }
    return g_reportUpgradeResult(taskId.c_str(), vehicleOtaVer.c_str(), result, errCode);
}

int ReportOtaTaskClearResultCallback(const std::string &taskId, int reslut)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_reportOtaTaskClearResult == nullptr) {
        OTA_LOG_E("ReportOtaTaskClearResultCallback failed, g_reportOtaTaskClearResult is nullptr");
        return OTA_RET_ERR;
    }
    return g_reportOtaTaskClearResult(taskId.c_str(), reslut);
}

int ReportPreconditionsCheckResultCallback(const std::string &taskId, int result, OtaPreconditionsErrorCode errCode)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_reportPreconditionsCheckResult == nullptr) {
        OTA_LOG_E("ReportPreconditionsCheckResultCallback failed, g_reportPreconditionsCheckResult is nullptr");
        return OTA_RET_ERR;
    }
    return g_reportPreconditionsCheckResult(taskId.c_str(), result, errCode);
}

int ReportStartUpgradeCallback(const std::string &taskId)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_reportStartUpgrade == nullptr) {
        OTA_LOG_E("ReportStartUpgradeCallback failed, g_reportStartUpgrade is nullptr");
        return OTA_RET_ERR;
    }
    return g_reportStartUpgrade(taskId.c_str());
}

int ReportStartRollbackCallback(const std::string &taskId)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_reportStartRollback == nullptr) {
        OTA_LOG_E("ReportStartRollbackOtherEcuCallback failed, g_reportStartRollback is nullptr");
        return OTA_RET_ERR;
    }
    return g_reportStartRollback(taskId.c_str());
}

int SendUdsDataCallback(const uint8_t *data, uint16_t dataLen)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_sendUdsData == nullptr) {
        OTA_LOG_E("SendUdsDataCallback failed, g_sendUdsData is nullptr");
        return OTA_RET_ERR;
    }
    return g_sendUdsData(data, dataLen);
}

int SendUdsDataAckCallback(const uint8_t *data, uint16_t dataLen)
{
    {
        std::lock_guard<std::mutex> locker(g_callbackMutex);
        if (g_sendUdsDataAck == nullptr) {
            OTA_LOG_E("SendUdsDataAckCallback failed, g_sendUdsDataAck is nullptr");
            return OTA_RET_ERR;
        }
        return g_sendUdsDataAck(data, dataLen);
    }
}

bool IsQueryTaskECUVerCallbackAsync(void)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_queryTaskECUVersionSync) {
        return false;
    }
    return true;
}

int QueryTaskECUVersionSyncCallback(const StQueryEcuVerInfo *queryEcusInfo, uint8_t ecuCnt, StEcuVersionInfo *ecuVersion)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_queryTaskECUVersionSync == nullptr) {
        OTA_LOG_E("QueryTaskECUVersionSyncCallback failed, g_queryTaskECUVersionSync is nullptr");
        return OTA_RET_ERR;
    }
    return g_queryTaskECUVersionSync(queryEcusInfo, ecuCnt, ecuVersion);
}

void QueryTaskECUVersionAsyncCallback(const StQueryEcuVerInfo *queryEcusInfo, uint8_t ecuCnt)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_queryTaskECUVersionAsync == nullptr) {
        OTA_LOG_E("QueryTaskECUVersionAsyncCallback failed, g_queryTaskECUVersionAsync is nullptr");
        return;
    }
    g_queryTaskECUVersionAsync(queryEcusInfo, ecuCnt);
}

bool IsQueryUpgradePreconditionsCallbackAsync(void)
{
    if (g_queryUpgradePreconditionsSync) {
        return false;
    }
    return true;
}

int QueryUpgradePreconditionsSyncCallback(StUpgradePreconditions *preconditons)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_queryUpgradePreconditionsSync == nullptr) {
        OTA_LOG_E("QueryUpgradePreconditionsSyncCallback failed, g_queryUpgradePreconditionsSync is nullptr");
        return OTA_RET_ERR;
    }
    return g_queryUpgradePreconditionsSync(preconditons);
}

void QueryUpgradePreconditionsAsyncCallback(void)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_queryUpgradePreconditionsAsync == nullptr) {
        OTA_LOG_E("QueryUpgradePreconditionsAsyncCallback failed, g_queryUpgradePreconditionsAsync is nullptr");
        return;
    }
    g_queryUpgradePreconditionsAsync();
}

int RequestUpDownHighVoltageCallback(OtaMasterUpgradePowerMode lowOrHigh)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_requestUpDownHighVoltage == nullptr) {
        OTA_LOG_E("RequestUpDownHighVoltageCallback failed, g_requestUpDownHighVoltage is nullptr");
        return OTA_RET_ERR;
    }
    return g_requestUpDownHighVoltage(lowOrHigh);
}

int RequestFotaModeOnOffSwitchCallback(CanSignalOtaMode isModeOn)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_requestFotaModeOnOff == nullptr) {
        OTA_LOG_E("RequestFotaModeOnOffSwitchCallback failed, g_requestFotaModeOnOff is nullptr");
        return OTA_RET_ERR;
    }
    return g_requestFotaModeOnOff(isModeOn);
}

int RequestTboxSelfUpgradeCallback(const std::string &upgradeFilePath)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_requestTboxSelfUpgrade == nullptr) {
        OTA_LOG_E("RequestTboxSelfUpgradeCallback failed, g_requestTboxSelfUpgrade is nullptr");
        return OTA_RET_ERR;
    }
    return g_requestTboxSelfUpgrade(upgradeFilePath.c_str());
}

int RequestTboxSelfRollbackCallback(const std::string &rollbackFilePath)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_requestTboxSelfRollback == nullptr) {
        OTA_LOG_E("RequestTboxSelfRollbackCallback failed, g_requestTboxSelfRollback is nullptr");
        return OTA_RET_ERR;
    }
    return g_requestTboxSelfRollback(rollbackFilePath.c_str());
}

void RequestTboxRebootCallback(void)
{
    std::lock_guard<std::mutex> locker(g_callbackMutex);
    if (g_requestTboxReboot == nullptr) {
        OTA_LOG_E("RequestTboxRebootCallback failed, g_requestTboxReboot is nullptr");
        return;
    }
    g_requestTboxReboot();
}