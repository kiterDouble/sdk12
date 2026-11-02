#include <iostream>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <string.h>
#include <algorithm>
#include "ota_master_log.h"
#include "ota_master_error_code.h"
#include "ota_master_ecu_brush.h"
#include "ota_master_fota_ecu.h"

// 全局进度回调函数和用户数据
static std::atomic<bool> g_progressCallbackEnabled(false);
static OtaMasterEcuBrushProgressCallback g_progressCallback = nullptr;
static void *g_progressUserdata = nullptr;

// 进度上报线程
static std::thread g_progressThread;
static std::atomic<bool> g_progressThreadRunning(false);
static std::atomic<uint8_t> g_currentProgress(0);
static std::condition_variable g_progressCond;
static std::mutex g_progressMutex;

// 进度上报线程函数
static void ProgressReportThread()
{
    while (g_progressThreadRunning.load()) {
        std::unique_lock<std::mutex> locker(g_progressMutex);
        g_progressCond.wait_for(locker, std::chrono::milliseconds(500));
        
        if (g_progressCallback && g_progressCallbackEnabled.load()) {
            uint8_t progress = g_currentProgress.load();
            g_progressCallback(progress, g_progressUserdata);
        }
    }
}

// 设置当前进度
void SetEcuBrushProgress(uint8_t percent)
{
    g_currentProgress.store(percent);
    if (g_progressCallbackEnabled.load()) {
        g_progressCond.notify_one();
    }
}

static FotaEcuInfo_t InitFotaEcuInfo(uint16_t ecuId, const OtaTaskEcuProtocolInfo &protocolInfo,
                                     const std::string &driverFilePath,
                                     const std::vector<std::string> &softwareFilePaths)
{
    FotaEcuInfo_t info = {
        .ecuId = ecuId,
        .phyId = protocolInfo.physicalId,
        .funcId = protocolInfo.functionId,
        .resId = protocolInfo.responseId,
        .channel = protocolInfo.canPort,
        .driverFilePath = driverFilePath,
        .softwareFilePaths = softwareFilePaths
    };
    return info;
}

OtaUpgradeErrorCode ExecEcuBrush(uint16_t ecuId, const OtaTaskEcuProtocolInfo &protocolInfo, const std::string &driverFilePath,
                 const std::vector<std::string> &softwareFilePaths, const std::string &targetVer)
{
    return ExecEcuBrushWithProgress(ecuId, protocolInfo, driverFilePath, softwareFilePaths, targetVer, nullptr, nullptr);
}

OtaUpgradeErrorCode  ExecEcuBrushWithProgress(uint16_t ecuId, const OtaTaskEcuProtocolInfo &protocolInfo,
                 const std::string &driverFilePath, const std::vector<std::string> &softwareFilePaths,
                 const std::string &targetVer, OtaMasterEcuBrushProgressCallback progressCb, void *userdata)
{
    OTA_LOG_D("ECU ID: 0x%04X, Driver File Path: %s", ecuId, driverFilePath.c_str());
    for (size_t i = 0; i < softwareFilePaths.size(); ++i) {
        OTA_LOG_D("Software File Path %zu: %s", i, softwareFilePaths[i].c_str());
    }

    // 设置进度回调
    g_progressCallback = progressCb;
    g_progressUserdata = userdata;
    g_progressCallbackEnabled.store(progressCb != nullptr);
    g_currentProgress.store(0);

    // 启动进度上报线程
    if (progressCb != nullptr) {
        g_progressThreadRunning.store(true);
        g_progressThread = std::thread(ProgressReportThread);
    }

    int ret = OTA_ERR_UPGRADE_TARGET_ECU_FAILED;
    FotaEcuHandle_t ecuHdl;
    FotaEcuInfo_t info = InitFotaEcuInfo(ecuId, protocolInfo, driverFilePath, softwareFilePaths);

    if (OTA_RET_OK != FotaEcuInit(&ecuHdl, &info)) {
        OTA_LOG_E("%s FotaEcuInit failed, phyId: 0x%x", __FUNCTION__, info.phyId);
        // 停止进度上报线程
        g_progressThreadRunning.store(false);
        if (g_progressThread.joinable()) {
            g_progressThread.join();
        }
        g_progressCallbackEnabled.store(false);
        return OTA_ERR_UPGRADE_TARGET_ECU_FAILED;
    }
    OTA_LOG_D("info_phyId: 0x%04X", info.phyId);
    
    // 设置初始进度
    SetEcuBrushProgress(5);

    if (OTA_RET_OK != FotaEcuExec(ecuHdl, targetVer)) {
        uint32_t errorCode = OTA_OK;
        FotaEcuGetErrorCode(ecuHdl, errorCode);
        OTA_LOG_E("%s FotaEcuExec failed, errorCode: 0x%x", __FUNCTION__, errorCode);
        if (errorCode != OTA_OK) {
            ret = errorCode;
        }
    } else {
        ret = OTA_OK;
        SetEcuBrushProgress(100);
    }

    FotaEcuDeinit(ecuHdl);

    // 停止进度上报线程
    g_progressThreadRunning.store(false);
    g_progressCond.notify_all();
    if (g_progressThread.joinable()) {
        g_progressThread.join();
    }
    g_progressCallbackEnabled.store(false);
    g_progressCallback = nullptr;
    g_progressUserdata = nullptr;

    return OtaUpgradeErrorCode(ret);
}

int ExecEcuRollback(const OtaTaskEcuProtocolInfo &protocolInfo, uint16_t timeOutMs, const std::string &rollbackVer)
{
    int ret = OTA_RET_ERR;
    FotaEcuHandle_t ecuHdl;
    FotaEcuInfo_t info = InitFotaEcuInfo(0, protocolInfo, std::string(), std::vector<std::string>());

    if (OTA_RET_OK != FotaEcuInit(&ecuHdl, &info)) {
        OTA_LOG_E("%s ECU rollback failed, phyId: 0x%x", __FUNCTION__, info.phyId);
        return OTA_RET_ERR;
    }

    if (OTA_RET_OK == FotaEcuRollback(ecuHdl, timeOutMs, rollbackVer)) {
        ret = OTA_RET_OK;
    } else {
        OTA_LOG_E("%s ECU rollback failed, phyId: 0x%x", __FUNCTION__, info.phyId);
    }

    FotaEcuDeinit(ecuHdl);
    return ret;
}

int VerifySelfRollback(const OtaTaskEcuProtocolInfo &protocolInfo , const std::string &version)
{
    int ret = OTA_RET_ERR;
    FotaEcuHandle_t ecuHdl;
    FotaEcuInfo_t info = InitFotaEcuInfo(0, protocolInfo, std::string(), std::vector<std::string>());

    if (OTA_RET_OK != FotaEcuInit(&ecuHdl, &info)) {
        OTA_LOG_E("%s ECU rollback failed, phyId: 0x%x", __FUNCTION__, info.phyId);
        return OTA_RET_ERR;
    }

    if (OTA_RET_OK == VerifySoftwareVersion(ecuHdl, version)) {
        ret = OTA_RET_OK;
    }

    FotaEcuDeinit(ecuHdl);
    return ret;
}