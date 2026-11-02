#include <unistd.h>
#include <string.h>
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <thread>
#include <unordered_set>
#include "ota_master_state_manage.h"
#include "ota_master_app_callback.h"
#include "ota_master_ecu_brush.h"
#include "ota_master_file_download.h"
#include "ota_master_inner_code.h"
#include "ota_master_config.h"
#include "ota_master_safety.h"
#include "ota_master_timer.h"
#include "ota_master_upgrade.h"
#include "ota_master_utils.h"
#include "ota_master_log.h"
#include "ota_master_pm_helper.h"

namespace {
static OtaMasterStatus g_otaStatus = OTA_NOT_READY;                          // OTA状态
static OtaMasterState g_otaState = OtaMasterState::OTA_STATE_INIT;           // OTA状态机状态
static std::string g_downloadRootFolder;                                     // OTA下载根目录
static std::string g_unzipRootFolder;                                        // OTA解压根目录
static std::mutex g_stateMachineMutex;                                       // OTA状态机互斥锁
static std::condition_variable g_stateMachineCond;                           // OTA状态机条件变量
static OtaTaskConfigInfo g_otaTaskConfigInfo;                                // OTA任务配置信息
static OtaPreconditionsConfigInfo g_otaPreconditionsConfigInfo;              // OTA前置条件配置信息
static StUpgradePreconditions g_upgradePreconditions;                        // 升级前置条件
static std::atomic<bool> g_isStateMachineInit(false);                        // OTA状态机是否初始化
static std::atomic<bool> g_isStateMachineStart(false);                       // OTA状态机处理线程是否运行
static std::atomic<bool> g_getOtaPrecondsAsyncResult(false);                 // 获取升级前置条件结果
static std::atomic<bool> g_getOtaEcuVerAsyncResult(false);                   // 获取ECU版本信息结果
static std::atomic<bool> g_isOtaTaskTimeout(false);                          // OTA任务是否超时
static std::atomic<bool> g_isResultReportHasAck(false);                      // 结果上报是否有ACK
static std::pair<OtaMasterUpgradePowerMode, bool> g_upDownPowerResult;       // 上下高压结果
static std::pair<CanSignalOtaMode, bool> g_otaModeOnOffSwResult;             // FOTA模式开关结果
static SupportOTAEcuConfigMap g_supportOtaEcuConfigInfo;                     // 可升级ECU配置信息
static std::unordered_map<uint16_t, OtaTaskEcuVersionInfo> g_ecuVersionInfo; // ECU版本信息
} // namespace

/**
 * @brief 获取已加载的ECU配置信息
 *
 * @param[out] config ECU配置信息
 *
 * @return
 *    OTA_RET_OK：获取成功
 *    OTA_RET_ERR：获取失败（未初始化）
 */
int GetLoadedSupportOtaEcuConfigInfo(SupportOTAEcuConfigMap &config)
{
    if (!g_isStateMachineInit.load()) {
        OTA_LOG_E("GetLoadedSupportOtaEcuConfigInfo failed, state machine not initialized");
        return OTA_RET_ERR;
    }
    config = g_supportOtaEcuConfigInfo;
    return OTA_RET_OK;
}
static const char *GetOtaMasterStateName(OtaMasterState state);

/**
 * @brief 查询OTA升级前置条件
 */
static int QueryUpgradePreconditions(void)
{
    OTA_LOG_D("Query upgrade preconditions");
    memset(&g_upgradePreconditions, 0, sizeof(StUpgradePreconditions));
    if (IsQueryUpgradePreconditionsCallbackAsync()) {
        g_getOtaPrecondsAsyncResult.store(false);
        QueryUpgradePreconditionsAsyncCallback();
        std::unique_lock<std::mutex> locker(g_stateMachineMutex);
        // 等待获取升级前置条件结果,最长等60s
        g_stateMachineCond.wait_for(locker, std::chrono::seconds(30));
        if (!g_getOtaPrecondsAsyncResult.load()) {
            OTA_LOG_E("Query upgrade preconditions async failed");
            return OTA_RET_ERR;
        }
        OTA_LOG_I("Query upgrade preconditions async success");
        return OTA_RET_OK;
    }

    if (QueryUpgradePreconditionsSyncCallback(&g_upgradePreconditions) != OTA_RET_OK) {
        OTA_LOG_E("Query upgrade preconditions sync failed");
        return OTA_RET_ERR;
    }
    OTA_LOG_I("Query upgrade preconditions sync success");
    return OTA_RET_OK;
}

/**
 * @brief 提取OTA任务配置信息中的ECU ID
 * 
 * @param[in] config OTA任务配置信息
 * @param[out] ecusId ECU ID列表
 * 
 * @return
 *    OTA_RET_OK：提取成功
 *    OTA_RET_ERR：提取失败
 */
static int ExtractOtaTaskEcuId(const OtaTaskConfigInfo &config, std::vector<StQueryEcuVerInfo> &queryEcusInfo)
{
    for (const auto &taskEcuConfig : config.highVoltEcuTasksInfo) {
        StQueryEcuVerInfo queryInfo;
        queryInfo.ecuId = taskEcuConfig.ecuId;
        queryInfo.canPort = taskEcuConfig.protocolInfo.canPort;
        queryInfo.physicalId = taskEcuConfig.protocolInfo.physicalId;
        queryInfo.functionId = taskEcuConfig.protocolInfo.functionId;;
        queryInfo.responseId = taskEcuConfig.protocolInfo.responseId;
        queryEcusInfo.push_back(queryInfo);
    }

    for (const auto &taskEcuConfig : config.lowVoltEcuTasksInfo) {
        StQueryEcuVerInfo queryInfo;
        queryInfo.ecuId = taskEcuConfig.ecuId;
        queryInfo.canPort = taskEcuConfig.protocolInfo.canPort;
        queryInfo.physicalId = taskEcuConfig.protocolInfo.physicalId;
        queryInfo.functionId = taskEcuConfig.protocolInfo.functionId;;
        queryInfo.responseId = taskEcuConfig.protocolInfo.responseId;
        queryEcusInfo.push_back(queryInfo);
    }
    return queryEcusInfo.empty() ? OTA_RET_ERR : OTA_RET_OK;
}

/**
 * @brief 设置ECU版本信息
 * 
 * @param[in] ecuVersion ECU版本信息
 * @param[in] size ECU数
 * 
 * @return 无
 */
static void SetEcuVersionInfo(const StEcuVersionInfo *ecuVersion, size_t size)
{
    OTA_LOG_D("Set ECU version info");
    if (ecuVersion == nullptr || size == 0) {
        OTA_LOG_E("Invalid version info [Ptr:%p][Size:%zu]", static_cast<const void *>(ecuVersion), size);
        return;
    }

    g_ecuVersionInfo.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        OtaTaskEcuVersionInfo dst;
        dst.ecuId = ecuVersion[i].ecuId;
        dst.swVersion = ecuVersion[i].swVersion ? ecuVersion[i].swVersion : "";
        StringToUpper(dst.swVersion);
        dst.hwVersion = ecuVersion[i].hwVersion ? ecuVersion[i].hwVersion : "";
        StringToUpper(dst.hwVersion);
        dst.supplierId = ecuVersion[i].supplierId ? ecuVersion[i].supplierId : "";
        StringToUpper(dst.supplierId);
        dst.partNumber = ecuVersion[i].partNum ? ecuVersion[i].partNum : "";
        StringToUpper(dst.partNumber);
        OTA_LOG_D("Set ECU[%04X] version info: SW:%s, HW:%s, SupplierId:%s, PartNum:%s", dst.ecuId,
                  dst.swVersion.c_str(), dst.hwVersion.c_str(), dst.supplierId.c_str(), dst.partNumber.c_str());
        // 插入ECU版本信息, 若ECU ID重复则不插入
        if (!g_ecuVersionInfo.emplace(dst.ecuId, std::move(dst)).second) {
            OTA_LOG_W("Duplicate ECU ID:%04X", ecuVersion[i].ecuId);
        }
    }
}

/**
 * @brief 查询指定ECU当前版本
 * 
 * @param[in] queryEcusInfo 查询版本的ECU信息
 * 
 * @return
 *    OTA_RET_OK：查询成功
 *    OTA_RET_ERR：查询失败
 */
static int QuerySpecifiedEcuCurrentVersion(const std::vector<StQueryEcuVerInfo> &queryEcusInfo)
{
    OTA_LOG_D("Query specified ECU current version");
    g_ecuVersionInfo.clear();
    if (IsQueryTaskECUVerCallbackAsync()) {
        g_getOtaEcuVerAsyncResult.store(false);
        QueryTaskECUVersionAsyncCallback(queryEcusInfo.data(), queryEcusInfo.size());
        std::unique_lock<std::mutex> locker(g_stateMachineMutex);
        // 等待获取ECU版本获取结果,最长等300s
        g_stateMachineCond.wait_for(locker, std::chrono::seconds(300));
        if (!g_getOtaEcuVerAsyncResult.load()) {
            OTA_LOG_E("Query all upgrade ECU current version async failed");
            return OTA_RET_ERR;
        }
        OTA_LOG_I("Query all upgrade ECU current version async success");
        return OTA_RET_OK;
    } else {
        const size_t ecuCount = queryEcusInfo.size();
        // RAII容器管理内存，回调应用查询升级件版本信息
        std::vector<StEcuVersionInfo> ecuVersions(ecuCount);
        if (QueryTaskECUVersionSyncCallback(queryEcusInfo.data(), ecuCount, ecuVersions.data()) != OTA_RET_OK) {
            OTA_LOG_E("Query all upgrade ECU current version sync failed");
            return OTA_RET_ERR;
        }
        SetEcuVersionInfo(ecuVersions.data(), ecuCount);
        OTA_LOG_I("Query all upgrade ECU current version sync success");
    }
    return OTA_RET_OK;
}

/**
 * @brief 二次检查ECU升级文件MD5
 * 
 * @param[in] taskConfig OTA任务配置信息
 * @param[out] errCode 错误码
 * 
 * @return
 *     OTA_RET_OK：文件完整
 *      OTA_RET_ERR：文件不完整
 */
static int SecondCheckDownloadedFileIntegrity(OtaTaskConfigInfo &taskConfig, OtaUpgradeErrorCode &errCode)
{
    //检测高压件升级包完整性
    for (auto &config : taskConfig.highVoltEcuTasksInfo) {
        OTA_LOG_D("Check ECU[0x%04x]:%s upgrade file:%s MD5", config.ecuId, config.ecuName.c_str(),
                  config.upgradeFile.downloadPath.c_str());
        // 检测升级文件MD5
        if (CheckMD5(config.upgradeFile.downloadPath, config.upgradeFile.downloadMD5) != OTA_RET_OK) {
            errCode = OtaUpgradeErrorCode::OTA_ERR_UPGRADE_FILE_MD5_CHECK_FAILED;
            return OTA_RET_ERR;
        }

        // 检测备份文件是否存在，不存在则不检测MD5
        if (config.backupFile.downloadPath.empty()) {
            OTA_LOG_W("ECU[0x%04x]:%s backup file is not exist, unable to check backup file MD5", config.ecuId,
                      config.ecuName.c_str());
            continue;
        }
        OTA_LOG_D("Check ECU[0x%04x]:%s backup file:%s MD5", config.ecuId, config.ecuName.c_str(),
                  config.backupFile.downloadPath.c_str());
        // 检测备份文件MD5
        if (CheckMD5(config.backupFile.downloadPath, config.backupFile.downloadMD5) != OTA_RET_OK) {
            // 二次校验备份包失败，若是本地备份包，也需要被删除
            config.isLocalBackup = 0;
            errCode = OtaUpgradeErrorCode::OTA_ERR_BACKUP_FILE_MD5_CHECK_FAILED;
            return OTA_RET_ERR;
        }
    }

    //检测低压件升级包完整性
    for (auto &config : taskConfig.lowVoltEcuTasksInfo) {
        // 检测升级文件MD5
        if (CheckMD5(config.upgradeFile.downloadPath, config.upgradeFile.downloadMD5) != OTA_RET_OK) {
            errCode = OtaUpgradeErrorCode::OTA_ERR_UPGRADE_FILE_MD5_CHECK_FAILED;
            return OTA_RET_ERR;
        }

        // 检测备份文件是否存在，不存在则不检测MD5
        if (config.backupFile.downloadPath.empty()) {
            OTA_LOG_W("ECU[0x%04x]:%s backup file is not exist, unable to check backup file MD5", config.ecuId,
                      config.ecuName.c_str());
            continue;
        }
        // 检测备份文件MD5
        if (CheckMD5(config.backupFile.downloadPath, config.backupFile.downloadMD5) != OTA_RET_OK) {
            // 二次校验备份包失败，若是本地备份包，也需要被删除
            config.isLocalBackup = 0;
            errCode = OtaUpgradeErrorCode::OTA_ERR_BACKUP_FILE_MD5_CHECK_FAILED;
            return OTA_RET_ERR;
        }
    }
    return OTA_RET_OK;
}

/**
 * @brief 检查所有ECU升级前的版本是否一致
 * 
 * @param[in] config OTA任务配置信息
 * 
 * @return
 *     OTA_RET_OK：版本一致
 *     OTA_RET_ERR：版本不一致
 */
static int CheckAllEcuPreUpgradeSwVersion(const OtaTaskConfigInfo &taskConfig)
{
    OTA_LOG_D("Check all ECU pre-upgraded software version");
    std::vector<StQueryEcuVerInfo> queryEcusInfo;
    if (ExtractOtaTaskEcuId(taskConfig, queryEcusInfo) != OTA_RET_OK) {
        OTA_LOG_E("Extract OTA task ECU id failed");
        return OTA_RET_ERR;
    }

    if (QuerySpecifiedEcuCurrentVersion(queryEcusInfo) != OTA_RET_OK) {
        OTA_LOG_E("Query all upgrade ECU current version failed");
        return OTA_RET_ERR;
    }

    const auto checkEcu = [&](const std::vector<OtaTaskEcuConfigInfo> &ecuTasks) {
        for (const auto &config : ecuTasks) {
            auto iter = g_ecuVersionInfo.find(config.ecuId);
            if (iter == g_ecuVersionInfo.end()) {
                OTA_LOG_E("ECU[0x%04X] not found in version info", config.ecuId);
                return false;
            }

            // 匹配零件号
            if (iter->second.partNumber != config.partNum) {
                OTA_LOG_E("ECU[0x%04X] part number mismatch. support: %s, actual: %s", config.ecuId,
                          config.partNum.c_str(), iter->second.partNumber.c_str());
                return false;
            }

            // 匹配软件版本
            if (iter->second.swVersion != config.supportVer) {
                OTA_LOG_E("ECU[0x%04X] support version mismatch. support: %s, actual: %s", config.ecuId,
                          config.supportVer.c_str(), iter->second.swVersion.c_str());
                return false;
            }

            // 匹配硬件版本
            if (iter->second.hwVersion != config.hwVer) {
                OTA_LOG_E("ECU[0x%04X] hardware version mismatch. support: %s, actual: %s", config.ecuId,
                          config.hwVer.c_str(), iter->second.hwVersion.c_str());
                return false;
            }
        }
        return true;
    };

    if (!checkEcu(taskConfig.highVoltEcuTasksInfo) || !checkEcu(taskConfig.lowVoltEcuTasksInfo)) {
        return OTA_RET_ERR;
    }
    OTA_LOG_I("All ECU preupgraded version is consistent");
    return OTA_RET_OK;
}

/**
 * @brief 查找TBox ECU配置
 * 
 * @param[in] highVolt 高压ECU配置列表
 * @param[in] lowVolt 低压ECU配置列表
 * 
 * @return TBox ECU配置指针；若未找到则返回nullptr
 */
static OtaTaskEcuConfigInfo *FindTBoxConfig(std::vector<OtaTaskEcuConfigInfo> &highVolt,
                                            std::vector<OtaTaskEcuConfigInfo> &lowVolt)
{
    auto predicate = [](const OtaTaskEcuConfigInfo &config) {
        return config.ecuType == static_cast<uint8_t>(OtaEcuType::E_OTA_ECU_TYPE_TBOX);
    };

    auto it = std::find_if(highVolt.begin(), highVolt.end(), predicate);
    if (it != highVolt.end()) {
        return &*it;
    }

    it = std::find_if(lowVolt.begin(), lowVolt.end(), predicate);
    return (it != lowVolt.end()) ? &*it : nullptr;
}

/**
 * @brief 检查TBox当前版本是否与期望版本一致
 * 
 * @param[in] config        OTA任务配置信息
 * @param[in] isTargetVer   是否检查目标版本；true:检查目标版本 false:检查支持版本
 * 
 * @return
 *      OTA_RET_OK：版本一致
 *      OTA_RET_ERR：版本不一致
 */
static int CheckTBoxCurrentSwVersion(OtaTaskConfigInfo &taskConfig, bool isTargetVer)
{
    OTA_LOG_D("Check TBox %s software version", isTargetVer ? "target" : "support");
    const auto *tboxConfig = FindTBoxConfig(taskConfig.highVoltEcuTasksInfo, taskConfig.lowVoltEcuTasksInfo);
    if (!tboxConfig) {
        OTA_LOG_E("TBox ECU not found in low or high volt ECU tasks");
        return OTA_RET_ERR;
    }
    std::vector<StQueryEcuVerInfo> queryEcusInfo;
    StQueryEcuVerInfo queryEcuInfo;
    queryEcuInfo.ecuId = tboxConfig->ecuId;
    queryEcuInfo.canPort = tboxConfig->protocolInfo.canPort;
    queryEcuInfo.physicalId = tboxConfig->protocolInfo.physicalId;
    queryEcuInfo.functionId = tboxConfig->protocolInfo.functionId;
    queryEcuInfo.responseId = tboxConfig->protocolInfo.responseId;
    queryEcusInfo.push_back(queryEcuInfo);

    // 查询TBox当前版本
    if (QuerySpecifiedEcuCurrentVersion(queryEcusInfo) != OTA_RET_OK) {
        OTA_LOG_E("Query all upgrade ECU current version failed");
        return OTA_RET_ERR;
    }

    // 检查TBox升级后的版本是否与目标版本一致
    auto iterVer = g_ecuVersionInfo.find(tboxConfig->ecuId);
    if (iterVer == g_ecuVersionInfo.end()) {
        OTA_LOG_E("TBox ECU[0x%04X] not found in version info", tboxConfig->ecuId);
        return OTA_RET_ERR;
    }

    std::string checkVersion = isTargetVer ? tboxConfig->targetVer : tboxConfig->supportVer;
    if (iterVer->second.swVersion != checkVersion) {
        OTA_LOG_E("Check TBox[0x%04X] version failed. expect: %s actual: %s", tboxConfig->ecuId, checkVersion.c_str(),
                  iterVer->second.swVersion.c_str());
        return OTA_RET_ERR;
    }
    OTA_LOG_I("TBox %s version is consistent with expected version[%s]", isTargetVer ? "target" : "support",
              checkVersion.c_str());
    return OTA_RET_OK;
}

/**
 * @brief 检测OTA升级前置田间是否满足
 * 
 * @param[in] preconditions 升级前置条件
 * 
 * @return 升级前置条件错误码
 */
static OtaPreconditionsErrorCode CheckUpgradePreconditions(void)
{
    if (QueryUpgradePreconditions() != OTA_RET_OK) {
        return OtaPreconditionsErrorCode::PRECONDS_ERR_UNKNOWN_REASON;
    }

    // 检测最低电压要求
    if (g_otaPreconditionsConfigInfo.minBatteryVoltageReq.first &&
        g_upgradePreconditions.batteryVoltage < g_otaPreconditionsConfigInfo.minBatteryVoltageReq.second) {
        OTA_LOG_E("Battery voltage is too low, current voltage: %.2f, request minimal voltage: %.2f",
                  g_upgradePreconditions.batteryVoltage, g_otaPreconditionsConfigInfo.minBatteryVoltageReq.second);
        return OtaPreconditionsErrorCode::PRECONDS_ERR_LOW_VOLTAGE_BATTERY_QUANTITY_INSUFFICIENT;
    }

    // 检测挡位要求
    if (g_otaPreconditionsConfigInfo.gearPositionReq.first &&
        g_upgradePreconditions.gearPosition != g_otaPreconditionsConfigInfo.gearPositionReq.second) {
        OTA_LOG_E("Gear position is not P or N, current gear position: %d", g_upgradePreconditions.gearPosition);
        return OtaPreconditionsErrorCode::PRECONDS_ERR_NOT_P_OR_N_GEAR;
    }

    // 检测动力系统就绪状态要求
    if (g_otaPreconditionsConfigInfo.vcuHvStatusNotReady.first &&
        g_upgradePreconditions.vcuHvStatus != g_otaPreconditionsConfigInfo.vcuHvStatusNotReady.second) {
        OTA_LOG_E("Vehicle is in ready state, failing to meet OTA requirements");
        return OtaPreconditionsErrorCode::PRECONDS_ERR_VEHICLE_IN_READY;
    }

    // 检测最高车速要求
    if (g_otaPreconditionsConfigInfo.maxVehicleSpeedReq.first &&
        g_upgradePreconditions.vehicleSpeed > g_otaPreconditionsConfigInfo.maxVehicleSpeedReq.second) {
        OTA_LOG_E("Vehicle speed is too high, current speed: %d, request max speed: %d",
                  g_upgradePreconditions.vehicleSpeed, g_otaPreconditionsConfigInfo.maxVehicleSpeedReq.second);
        return OtaPreconditionsErrorCode::PRECONDS_ERR_VEHICLE_IN_MOTION;
    }

    // 检测最低电池SOC要求
    if (g_otaPreconditionsConfigInfo.minBmsRealSocReq.first &&
        g_upgradePreconditions.bmsRealSoc < g_otaPreconditionsConfigInfo.minBmsRealSocReq.second) {
        OTA_LOG_E("BMS real SOC is too low, current SOC: %d, request minimal SOC: %d",
                  g_upgradePreconditions.bmsRealSoc, g_otaPreconditionsConfigInfo.minBmsRealSocReq.second);
        return OtaPreconditionsErrorCode::PRECONDS_ERR_POWER_BATTERY_QUANTITY_INSUFFICIENT;
    }

    // 检测整车故障要求
    if (g_otaPreconditionsConfigInfo.vehicleFaultReq.first &&
        g_upgradePreconditions.hasVehicleFault != g_otaPreconditionsConfigInfo.vehicleFaultReq.second) {
        OTA_LOG_E("Vehicle has fault");
        return OtaPreconditionsErrorCode::PRECONDS_ERR_VEHICLE_MALFUNCTION;
    }

    // 检测充电枪连接状态要求
    if (g_otaPreconditionsConfigInfo.chargePlugStateReq.first &&
        g_upgradePreconditions.chargePlugState != g_otaPreconditionsConfigInfo.chargePlugStateReq.second) {
        OTA_LOG_E("Charge plug is connected");
        return OtaPreconditionsErrorCode::PRECONDS_ERR_CHARGE_PLUG_CONN;
    }

    // 检测远程指令执行状态要求
    if (g_otaPreconditionsConfigInfo.remoteCtrlExecutingStateReq.first &&
        g_upgradePreconditions.hasRemoteCtrlExecuting != g_otaPreconditionsConfigInfo.remoteCtrlExecutingStateReq.second) {
        OTA_LOG_E("Remote control command is executing");
        return OtaPreconditionsErrorCode::PRECONDS_ERR_REMOTE_CTRL_EXECUTING;
    }

    // 检测OBD诊断设备连接状态要求
    if (g_otaPreconditionsConfigInfo.obdDiagDevConnStateReq.first &&
        g_upgradePreconditions.isObdDiagDevConn != g_otaPreconditionsConfigInfo.obdDiagDevConnStateReq.second) {
        OTA_LOG_E("Diagnostic device is connected");
        return OtaPreconditionsErrorCode::PRECONDS_ERR_DIAGNOSTIC_DEV_CONN;
    }

    return OtaPreconditionsErrorCode::PRECONDS_ERR_NONE;
}

/**
 * @brief 检测BCM防盗认证是否通过
 * 
 * @return
 *    OTA_RET_OK：通过
 *    OTA_RET_ERR：未通过
 */
static int CheckTboxBcmAntiTheftAuthIsPassed(void)
{
    if (!g_otaPreconditionsConfigInfo.bcmAntiTheftAuthPassedReq.first) {
        return OTA_RET_OK;
    }
    return (g_upgradePreconditions.isBcmAntiTheftAuthPassed == 
            g_otaPreconditionsConfigInfo.bcmAntiTheftAuthPassedReq.second) ? OTA_RET_OK : OTA_RET_ERR;
}

/**
 * @brief 检测是否有不支持升级的ECU
 * 
 * @param[in] config OTA任务配置信息
 * 
 * @return
 *      true：有不支持升级的ECU
 *      false：无不支持升级的ECU
 */
static bool HasUnsupportedUpgradeECU(const OtaTaskConfigInfo &taskConfig)
{
    const auto hasUnsupportedEcu = [&](const std::vector<OtaTaskEcuConfigInfo> &ecuTasks) {
        for (const auto &config : ecuTasks) {
            if (g_supportOtaEcuConfigInfo.find(config.ecuId) == g_supportOtaEcuConfigInfo.end()) {
                OTA_LOG_E("ECU ID: %d is not in support OTA ECU config", config.ecuId);
                return true;
            }
        }
        return false;
    };

    return hasUnsupportedEcu(taskConfig.highVoltEcuTasksInfo) || hasUnsupportedEcu(taskConfig.lowVoltEcuTasksInfo);
}

/**
 * @brief 请求上下高压
 * 
 * @param[in] mode 电源模式
 * @param[in] ifNeedAck 是否需要应答
 * 
 * @return
 *      OTA_RET_OK：成功
 *      OTA_RET_ERR：失败
 */
static int RequestUpDownHighVoltage(OtaMasterUpgradePowerMode mode, bool ifNeedAck)
{
    OTA_LOG_D("Request %s voltage", (mode == OtaMasterUpgradePowerMode::OTA_HIGH_VOLTAGE_UPGRADE) ? "high" : "low");
    g_upDownPowerResult = std::make_pair(mode, false);
    // 回调请求上下高压
    if (RequestUpDownHighVoltageCallback(mode) != OTA_RET_OK) {
        OTA_LOG_E("Request %s voltage failed",
                  (mode == OtaMasterUpgradePowerMode::OTA_HIGH_VOLTAGE_UPGRADE) ? "high" : "low");
        return OTA_RET_ERR;
    }

    // 等待上下高压设置结果应答，等待20s
    if (ifNeedAck) {
        {
            std::unique_lock<std::mutex> locker(g_stateMachineMutex);
            if (!g_upDownPowerResult.second) {
                g_stateMachineCond.wait_for(locker, std::chrono::seconds(20));
            }
        }

        if (g_upDownPowerResult.second == false) {
            OTA_LOG_E("Request %s voltage timeout or failed",
                      (mode == OtaMasterUpgradePowerMode::OTA_HIGH_VOLTAGE_UPGRADE) ? "high" : "low");
            return OTA_RET_ERR;
        }
    }
    OTA_LOG_I("Current power mode is set to %s",
              (mode == OtaMasterUpgradePowerMode::OTA_HIGH_VOLTAGE_UPGRADE) ? "high" : "low");
    return OTA_RET_OK;
}

/**
 * @brief 请求进入或退出Fota模式，允许或禁止电源模式切换
 * 
 * @param[in] mode OTA模式
 * @param[in] ifNeedAck 是否需要应答
 * 
 * @return
 *     OTA_RET_OK：成功
 *    OTA_RET_ERR：失败
 */
static int RequestSwitchFotaModeOnOff(CanSignalOtaMode mode, bool ifNeedAck)
{
    OTA_LOG_D("Request switch fota mode %s", (mode == CanSignalOtaMode::CAN_SIG_OTA_MODE_ON) ? "ON" : "OFF");
    g_otaModeOnOffSwResult = std::make_pair(mode, false);
    // 回调请求设置OTA模式开关
    if (RequestFotaModeOnOffSwitchCallback(mode) != OTA_RET_OK) {
        OTA_LOG_E("Request set OTA mode %s failed", (mode == CanSignalOtaMode::CAN_SIG_OTA_MODE_ON) ? "on" : "off");
        return OTA_RET_ERR;
    }

    // 等待OTA模式切换结果应答，等待20s
    if (ifNeedAck) {
        {
            std::unique_lock<std::mutex> locker(g_stateMachineMutex);
            if (!g_otaModeOnOffSwResult.second) {
                g_stateMachineCond.wait_for(locker, std::chrono::seconds(20));
            }
        }

        if (g_otaModeOnOffSwResult.second == false) {
            OTA_LOG_E("Request switch OTA mode %s timeout or failed",
                      (mode == CanSignalOtaMode::CAN_SIG_OTA_MODE_ON) ? "on" : "off");
            return OTA_RET_ERR;
        }
    }
    return OTA_RET_OK;
}

/**
 * @brief 上报OTA升级结果
 * 
 * @param[in] taskId OTA任务ID
 * @param[in] vehicleOtaVer 车辆OTA版本
 * @param[in] result 升级结果
 * @param[in] errorCode 错误码
 * 
 * @return
 *      OTA_RET_OK：成功
 *      OTA_RET_ERR：失败
 */
static int ReportOtaUpgradeResult(const std::string &taskId, const std::string &vehicleOtaVer, OtaUpgradeResult result,
                                  OtaUpgradeErrorCode errorCode)
{
    g_isResultReportHasAck.store(false);
    if (ReportUpgradeResultCallback(taskId, vehicleOtaVer, result, errorCode) != OTA_RET_OK) {
        OTA_LOG_E("Report upgrade %s result failed",
                  (result == OtaUpgradeResult::OTA_UPGRADE_SUCC) ? "success" : "failed");
        std::unique_lock<std::mutex> locker(g_stateMachineMutex);
        g_stateMachineCond.wait_for(locker, std::chrono::seconds(30));
        return OTA_RET_ERR;
    }

    {
        std::unique_lock<std::mutex> locker(g_stateMachineMutex);
        // 等待下载失败结果上报完成并等待应答，超时时间30s
        g_stateMachineCond.wait_for(locker, std::chrono::seconds(30), [] { return g_isResultReportHasAck.load(); });
        if (!g_isResultReportHasAck.load()) {
            OTA_LOG_E("Report upgrade %s result ack timeout",
                      (result == OtaUpgradeResult::OTA_UPGRADE_SUCC) ? "success" : "failed");
            return OTA_RET_ERR;
        }
        g_isResultReportHasAck.store(false);
    }
    return OTA_RET_OK;
}

/**
 * @brief 文件下载错误是否是网络异常
 * 
 * @param[in] error 下载错误码
 * 
 * @return
 *      true：是网络异常
 *      false：不是网络异常
 */
static bool IsDownloadNetworkError(OtaFileDownloadErrorCode error)
{
    return error == OTA_DOWNLOAD_NETWORK_ABNORMAL || error == OTA_DOWNLOAD_TASK_TIMEOUT || error == OTA_DOWNLOAD_STOPED;
}

/**
 * @brief 下载ECU升级文件
 * 
 * @param[inout] tasksEcuConfig ECU OTA任务配置信息
 * @param[out] upgradeError 升级错误码
 * 
 * @return 下载错误码
 */
static OtaFileDownloadErrorCode ExecuteUpgradeFileDownload(std::vector<OtaTaskEcuConfigInfo> &tasksEcuConfig,
                                                           OtaUpgradeErrorCode &upgradeError)
{
    auto DonwnloadingCallback = [&](OtaTaskEcuUpgradeFileInfo &fileInfo) {
        OTA_LOG_D("Start downloading upgrade file:%s", fileInfo.downloadPath.c_str());
        fileInfo.downloadState = static_cast<uint8_t>(OtaFileDownloadStatus::OTA_FILE_DOWNLOADING);
        UpdateOtaTaskConfig(g_otaTaskConfigInfo);
    };

    auto UpdateDownloadState = [](OtaTaskEcuUpgradeFileInfo &fileInfo, OtaFileDownloadStatus status) {
        fileInfo.downloadState = static_cast<uint8_t>(status);
        UpdateOtaTaskConfig(g_otaTaskConfigInfo);
    };

    for (auto &config : tasksEcuConfig) {
        if (g_isOtaTaskTimeout.load()) {
            OTA_LOG_E("OTA task timeout, interrupt download");
            return OtaFileDownloadErrorCode::OTA_DOWNLOAD_TASK_TIMEOUT;
        }

        // 获取ECU升级文件下载目录
        config.dlFolder = std::move(GenerateFolderPath(g_downloadRootFolder, config.ecuName));
        if (config.dlFolder.empty()) {
            OTA_LOG_E("Get ECU[%d] download folder failed", config.ecuId);
            return OtaFileDownloadErrorCode::OTA_DOWNLOAD_FOLDER_EMPTY;;
        }

        // 下载升级文件
        auto error = StartDownloadOtaUpgradeFile(DonwnloadingCallback, config);
        if (error != OtaFileDownloadErrorCode::OTA_DOWNLOAD_OK) {
            OTA_LOG_E("Download OTA upgrade file failed, error code: %d", error);
            // 非网络异常错误，更新下载状态为下载失败
            if (!IsDownloadNetworkError(error)) {
                upgradeError = OtaUpgradeErrorCode::OTA_ERR_UPGRADE_FILE_DOWNLOAD_FAILED;
                UpdateDownloadState(config.upgradeFile, OtaFileDownloadStatus::OTA_FILE_DOWNLOAD_FAILED);
            }
            return error;
        }
        UpdateDownloadState(config.upgradeFile, OtaFileDownloadStatus::OTA_FILE_DOWNLOAD_SUCCESS);
        // 检查MD5
        if (CheckMD5(config.upgradeFile.downloadPath, config.upgradeFile.downloadMD5) != OTA_RET_OK) {
            OTA_LOG_E("Check OTA upgrade file MD5 failed");
            upgradeError = OtaUpgradeErrorCode::OTA_ERR_UPGRADE_FILE_MD5_CHECK_FAILED;
            return OtaFileDownloadErrorCode::OTA_DOWNLAOD_MD5_CHECK_FAILED;
        }

        // 下载备份文件
        error = StartDownloadOtaBackupFile(DonwnloadingCallback, config);
        if (error != OtaFileDownloadErrorCode::OTA_DOWNLOAD_OK) {
            OTA_LOG_E("Download OTA backup file failed, error code: %d", error);
            // 非网络异常错误，更新下载状态为下载失败
            if (!IsDownloadNetworkError(error)) {
                upgradeError = OtaUpgradeErrorCode::OTA_ERR_BACKUP_FILE_DOWNLOAD_FAILED;
                UpdateDownloadState(config.backupFile, OtaFileDownloadStatus::OTA_FILE_DOWNLOAD_FAILED);
            }
            return error;
        }
        if (config.backupFile.downloadPath.empty()) {
            OTA_LOG_W("ECU[0x%04x]:%s backup file is not exist", config.ecuId, config.ecuName.c_str());
            continue;
        }
        UpdateDownloadState(config.backupFile, OtaFileDownloadStatus::OTA_FILE_DOWNLOAD_SUCCESS);
        // 检查MD5
        if (CheckMD5(config.backupFile.downloadPath, config.backupFile.downloadMD5) != OTA_RET_OK) {
            OTA_LOG_E("Check OTA backup file MD5 failed");
            upgradeError = OtaUpgradeErrorCode::OTA_ERR_BACKUP_FILE_MD5_CHECK_FAILED;
            return OtaFileDownloadErrorCode::OTA_DOWNLAOD_MD5_CHECK_FAILED;
        }
    }
    return OtaFileDownloadErrorCode::OTA_DOWNLOAD_OK;
}

/**
 * @brief 升级ECU
 * 
 * @param[in] taskConfig ECU OTA任务配置信息
 * @param[out] upgradeError 升级失败错误码
 * @param[out] isTboxNeedReboot TBOX是否需要重启
 * 
 * @return 无
 */
static void ExecuteUpgradeEcu(std::vector<OtaTaskEcuConfigInfo> &taskEcuConfig, OtaUpgradeErrorCode &error,
                              bool &isTboxNeedReboot)
{
    for (auto &config : taskEcuConfig) {
        if (g_isOtaTaskTimeout.load()) {
            OTA_LOG_E("OTA task timeout, interrupt upgrade");
            error = OtaUpgradeErrorCode::OTA_ERR_TASK_TIMEOUT;
            break;
        }

        // 获取ECU升级文件解压目录
        config.unzipFolder = std::move(GenerateFolderPath(g_unzipRootFolder, config.ecuName));
        if (config.unzipFolder.empty()) {
            OTA_LOG_E("Get ECU[%d]:%s unzip folder failed", config.ecuId, config.ecuName.c_str());
            error = OtaUpgradeErrorCode::OTA_ERR_UPGRADE_FILE_DECOMPRESS_FAILED;
            break;
        }

        // 升级ECU
        config.upgradeState =
            UpgradeTargetEcu(config, error, isTboxNeedReboot, []() { return g_isOtaTaskTimeout.load(); });
        // 更新ECU升级结果到OTA任务配置信息
        UpdateOtaTaskConfig(g_otaTaskConfigInfo);
        if (config.upgradeState != OtaEcuUpgradeStatus::OTA_ECU_UPGRADE_SUCCESS || error != OtaUpgradeErrorCode::OTA_OK) {
            OTA_LOG_E("Upgrade ECU[0x%04x]:[%s] error, upgradeStatus:%d, error:%d", config.ecuId,
                      config.ecuName.c_str(), config.upgradeState, error);
            break;
        }
        OTA_LOG_I("Upgrade ECU[0x%04x]:[%s] success", config.ecuId, config.ecuName.c_str());
    }
}

/**
 * @brief 回滚升级任务ECU
 * 
 * @param[out] isTboxNeedReboot TBOX是否需要重启
 * 
 * @return 无
 */
static void ExecuteRollbackEcu(bool &isTboxNeedReboot)
{
    OTA_LOG_D("Execute rollback ECU...");
    // 通用回滚函数
    auto rollbackEcus = [&](OtaMasterUpgradePowerMode mode, std::vector<OtaTaskEcuConfigInfo> &ecuTasks) -> bool {
        std::string modeStr = (mode == OtaMasterUpgradePowerMode::OTA_HIGH_VOLTAGE_UPGRADE) ? "high" : "low";
        OTA_LOG_I("Start rollback %s ECU tasks", modeStr.c_str());
        if (ecuTasks.empty())
            return true;
 
        // 检测高/低压任务队列中的升级任务是否都没有执行，多都没有，则不做上/下高压动作
        auto iter = std::find_if(ecuTasks.begin(), ecuTasks.end(), [](const OtaTaskEcuConfigInfo &config) {
            return config.upgradeState != static_cast<uint8_t>(OtaEcuUpgradeStatus::OTA_ECU_NOT_UPGRADE);
        });
        if (iter == ecuTasks.end()) {
            OTA_LOG_I("No upgraded task in %s ECU tasks, skip rollback", modeStr.c_str());
            return true;
        }

        // 当前不是上高压状态，高压件回滚需要上高压，上高压前需要退出FOTA模式
        if (mode == OtaMasterUpgradePowerMode::OTA_HIGH_VOLTAGE_UPGRADE) {
            if (RequestSwitchFotaModeOnOff(CanSignalOtaMode::CAN_SIG_OTA_MODE_OFF, true) != OTA_RET_OK) {
                return false;
            }
        }

        // 请求上下高压
        if (RequestUpDownHighVoltage(mode, true) != OTA_RET_OK) {
            return false;
        }

        // 若当前不在FOTA模式，进入FOTA模式
        if (RequestSwitchFotaModeOnOff(CanSignalOtaMode::CAN_SIG_OTA_MODE_ON, true) != OTA_RET_OK) {
            return false;
        }

        // 批量处理ECU回滚
        for (auto &config : ecuTasks) {
            config.rollbackState = RollbackTargetEcu(config, isTboxNeedReboot);
        }
        // 批量更新配置
        UpdateOtaTaskConfig(g_otaTaskConfigInfo);
        return true;
    };
    // 回滚高压件
    if (!rollbackEcus(OtaMasterUpgradePowerMode::OTA_HIGH_VOLTAGE_UPGRADE, g_otaTaskConfigInfo.highVoltEcuTasksInfo)) {
        OTA_LOG_E("Rollback high voltage ECU tasks failed");
    }

    // 回滚低压件
    if (!rollbackEcus(OtaMasterUpgradePowerMode::OTA_LOW_VOLTAGE_UPGRADE, g_otaTaskConfigInfo.lowVoltEcuTasksInfo)) {
        OTA_LOG_E("Rollback low voltage ECU tasks failed");
    }
}

/**
 * @brief 上传OTA Master日志
 * 
 * @param 无
 * 
 * @return 无
 */
static void UploadOtaMasterLog(void)
{
    OTA_LOG_I("Upload OTA master log...");
    // 日志上传功能已移除，使用zlog
    OTA_LOG_I("OTA master log upload is handled by zlog");
}

/**
 * @brief 重置OTA Master日志
 * 
 * @param 无
 * 
 * @return 无
 */
static void ResetOtaMasterLog(void)
{
    // 日志重置功能已移除，使用zlog
    OTA_LOG_I("OTA master log reset is handled by zlog");
}

/**
 * @brief OTA升级任务超时回调
 * 
 * @return 无
 */
static void OtaTaskTimeoutCallback(void)
{
    OTA_LOG_E("OTA task timeout");
    g_isOtaTaskTimeout.store(true);
    g_stateMachineCond.notify_all();
}

/**
 * @brief 获取OTA状态机状态
 * 
 * @param 无
 * 
 * @return OTA状态机状态
 */
static OtaMasterState GetOtaMasterState(void)
{
    std::lock_guard<std::mutex> lock(g_stateMachineMutex);
    return g_otaState;
}

/**
 * @brief 设置OTA状态机状态
 * 
 * @param state OTA状态机状态
 * 
 * @return 无
 */
static void SetOtaMasterState(OtaMasterState state)
{
    OTA_LOG_I("Set OTA master state to -> %s", GetOtaMasterStateName(state));
    std::lock_guard<std::mutex> lock(g_stateMachineMutex);
    g_otaState = state;
}

/**
 * @brief 根据回滚结果，获取OTA升级失败结果
 * 
 * @return OTA升级结果
 */
static OtaUpgradeResult GetUpgradeFailedResult(void)
{
    // 回滚失败的ECU ID
    std::vector<uint16_t> rollbackFailedEcuIds;
    std::vector<uint16_t> rollbackSucceededEcuIds;

    for (const auto &config : g_otaTaskConfigInfo.highVoltEcuTasksInfo) {
        // 结果分类记录
        if (config.rollbackState == OtaEcuRollbackStatus::E_ROLLBACK_STATUS_FAILED ||
            (config.rollbackState == E_ROLLBACK_STATUS_NOT_ROLLBACK &&
             config.upgradeState != OtaEcuUpgradeStatus::OTA_ECU_NOT_UPGRADE)) {
            rollbackFailedEcuIds.push_back(config.ecuId);
        } else if (config.rollbackState == OtaEcuRollbackStatus::E_ROLLBACK_STATUS_SUCCESS) {
            rollbackSucceededEcuIds.push_back(config.ecuId);
        }
    }

    for (const auto &config : g_otaTaskConfigInfo.lowVoltEcuTasksInfo) {
        // 结果分类记录
        if (config.rollbackState == OtaEcuRollbackStatus::E_ROLLBACK_STATUS_FAILED ||
            (config.rollbackState == E_ROLLBACK_STATUS_NOT_ROLLBACK &&
             config.upgradeState != OtaEcuUpgradeStatus::OTA_ECU_NOT_UPGRADE)) {
            rollbackFailedEcuIds.push_back(config.ecuId);
        } else if (config.rollbackState == OtaEcuRollbackStatus::E_ROLLBACK_STATUS_SUCCESS) {
            rollbackSucceededEcuIds.push_back(config.ecuId);
        }
    }

    if (!rollbackFailedEcuIds.empty()) {
        return OtaUpgradeResult::OTA_UPGRADE_ROLLBACK_FAILED;
    } else if (!rollbackSucceededEcuIds.empty()) {
        return OtaUpgradeResult::OTA_UPGRADE_ROLLBACK_SUCC;
    }
    return OtaUpgradeResult::OTA_UPGRADE_FAILED;
}

/**
 * @brief OTA状态机-初始化状态
 */
static void HandleOtaMasterStateInit(void)
{
    g_isOtaTaskTimeout.store(false);
    if (LoadOtaTaskConfigInfo(g_otaTaskConfigInfo) != OTA_RET_OK) {
        OTA_LOG_E("Load OTA task config info failed");
        g_otaTaskConfigInfo.Clear();
        g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_IDLE);
        SetOtaMasterState(OtaMasterState::OTA_STATE_IDLE);
        return;
    }

    auto state = static_cast<OtaMasterState>(g_otaTaskConfigInfo.otaState);
    if (state >= OtaMasterState::OTA_STATE_MAX) {
        OTA_LOG_E("Invalid OTA state:%d", state);
        g_otaTaskConfigInfo.Clear();
        ResetOtaTaskConfig();
        g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_IDLE);
        SetOtaMasterState(OtaMasterState::OTA_STATE_IDLE);
        return;
    }

    // 恢复任务超时监听的OTA状态机状态
    const std::unordered_set<OtaMasterState> activeStates = {
        OtaMasterState::OTA_STATE_PRE_DOWNLOAD,
        OtaMasterState::OTA_STATE_DOWNLOADING,
        OtaMasterState::OTA_STATE_DOWNLOAD_SUCCESS,
        OtaMasterState::OTA_STATE_WAITING_FOR_USER_CONFIRM,
        OtaMasterState::OTA_STATE_PRE_UPGRADE,
        OtaMasterState::OTA_STATE_UPGRADING
    };
    // 检测是否做整个任务超时监听
    if (g_otaTaskConfigInfo.taskDuration > 0 && activeStates.count(state)) {
        // 启动OTA任务超时监听计时器
        StartOtaTaskTimeoutMonitor(g_otaTaskConfigInfo.taskStartTime, g_otaTaskConfigInfo.taskDuration,
                                   OtaTaskTimeoutCallback);
    }
    // 恢复OTA状态机状态
    SetOtaMasterState(state);
}

/**
 * @brief OTA状态机-空闲状态
 * 
 * @param[out] upgradeError  升级失败错误码
 * 
 * @return 无
 */
static void HandleOtaMasterStateIdle(void)
{
    std::unique_lock<std::mutex> locker(g_stateMachineMutex);
    g_stateMachineCond.wait_for(locker, std::chrono::seconds(300));
}

/**
 * @brief OTA状态机-预升级状态
 * 
 * @return 无
 */
static void HandleOtaMasterStatePreDownload(void)
{
    //检测升级任务中是否有不支持升级的ECU
    if (HasUnsupportedUpgradeECU(g_otaTaskConfigInfo)) {
        g_otaTaskConfigInfo.errorCode = static_cast<uint8_t>(OtaUpgradeErrorCode::OTA_ERR_PART_NUM_OR_HW_VER_NOT_MATCH);
        g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_DOWNLOAD_FAILED);
        UpdateOtaTaskConfig(g_otaTaskConfigInfo);
        SetOtaMasterState(OtaMasterState::OTA_STATE_DOWNLOAD_FAILED);
        return;
    }

    g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_DOWNLOADING);
    UpdateOtaTaskConfig(g_otaTaskConfigInfo);
    SetOtaMasterState(OtaMasterState::OTA_STATE_DOWNLOADING);

    //回调上报开始下载
    ReportStartDownloadCallback(g_otaTaskConfigInfo.taskId);
}

/**
 * @brief OTA状态机-下载中状态
 * 
 * @return 无
 */
static void HandleOtaMasterStateDownloading(void)
{
    OtaUpgradeErrorCode errCode = OtaUpgradeErrorCode::OTA_OK;
    // 下载高压件升级包
    auto downloadError = ExecuteUpgradeFileDownload(g_otaTaskConfigInfo.highVoltEcuTasksInfo, errCode);
    // 下载低压件升级包
    if (downloadError == OtaFileDownloadErrorCode::OTA_DOWNLOAD_OK) {
        downloadError = ExecuteUpgradeFileDownload(g_otaTaskConfigInfo.lowVoltEcuTasksInfo, errCode);
    }

    g_otaTaskConfigInfo.errorCode = static_cast<uint8_t>(errCode);
    if (downloadError == OtaFileDownloadErrorCode::OTA_DOWNLOAD_OK) {
        g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_DOWNLOAD_SUCCESS);
        UpdateOtaTaskConfig(g_otaTaskConfigInfo);
        SetOtaMasterState(OtaMasterState::OTA_STATE_DOWNLOAD_SUCCESS);
    } else if (IsDownloadNetworkError(downloadError)) {
        auto currentState = GetOtaMasterState();
        std::unique_lock<std::mutex> locker(g_stateMachineMutex);
        g_stateMachineCond.wait_for(locker, std::chrono::seconds(20), [&]() {
            return !g_isStateMachineStart.load() || (currentState != OtaMasterState::OTA_STATE_DOWNLOADING);
        });
    } else {
        g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_DOWNLOAD_FAILED);
        UpdateOtaTaskConfig(g_otaTaskConfigInfo);
        SetOtaMasterState(OtaMasterState::OTA_STATE_DOWNLOAD_FAILED);
        //停止OTA任务超时监听
        StopOtaTaskTimeoutMonitor();
    }
}

/**
 * @brief OTA状态机-下载成功状态
 */
static void HandleOtaMasterStateDownloadSuccess(void)
{
    if (ReportDownloadResultCallback(g_otaTaskConfigInfo.taskId.c_str(), OTA_RET_OK, OtaUpgradeErrorCode::OTA_OK) !=
        OTA_RET_OK) {
        OTA_LOG_E("Report download success result failed");
        std::unique_lock<std::mutex> locker(g_stateMachineMutex);
        g_stateMachineCond.wait_for(locker, std::chrono::seconds(30));
        return;
    }

    if (g_otaTaskConfigInfo.upgradeMode == OTA_UPGRADE_MODE_SILENT) {
        g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_PRE_UPGRADE);
        UpdateOtaTaskConfig(g_otaTaskConfigInfo);
        SetOtaMasterState(OtaMasterState::OTA_STATE_PRE_UPGRADE);
    } else {
        g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_WAITING_FOR_USER_CONFIRM);
        UpdateOtaTaskConfig(g_otaTaskConfigInfo);
        SetOtaMasterState(OtaMasterState::OTA_STATE_WAITING_FOR_USER_CONFIRM);
    }
}

/**
 * @brief OTA状态机-等待用户确认升级状态
 */
static void HandleOtaMasterStateWaitingForUserConfirm(void)
{
    if (g_isOtaTaskTimeout.load()) {
        OTA_LOG_E("OTA task timeout, interrupt download");
        g_otaTaskConfigInfo.errorCode = OtaUpgradeErrorCode::OTA_ERR_TASK_TIMEOUT;
        g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_UPGRADE_FAILED);
        UpdateOtaTaskConfig(g_otaTaskConfigInfo);
        SetOtaMasterState(OtaMasterState::OTA_STATE_UPGRADE_FAILED);
        return;
    }
    std::unique_lock<std::mutex> locker(g_stateMachineMutex);
    g_stateMachineCond.wait_for(locker, std::chrono::seconds(30));
}

/**
 * @brief OTA状态机-清除升级任务状态
 */
static void HandleOtaMasterStateClearTask(void)
{
    if (ReportOtaTaskClearResultCallback(g_otaTaskConfigInfo.taskId.c_str(), OTA_RET_OK) != OTA_RET_OK) {
        OTA_LOG_E("Report clear task result failed");
        std::unique_lock<std::mutex> locker(g_stateMachineMutex);
        g_stateMachineCond.wait_for(locker, std::chrono::seconds(30));
        return;
    }
    //删除升级文件
    DeleteDownloadedOtaFile(g_otaTaskConfigInfo, false);
    g_otaTaskConfigInfo.Clear();
    ResetOtaTaskConfig();
    SetOtaMasterState(OtaMasterState::OTA_STATE_IDLE);
    // 重置日志
    ResetOtaMasterLog();
}

/**
 * @brief OTA状态机-预升级状态
 * 
 */
static void HandleOtaMasterStatePreUpgrade(void)
{
    // 统一错误处理函数
    auto HandleError = [&](OtaUpgradeErrorCode code, const std::string &logMsg,
                           OtaPreconditionsErrorCode precondErr = OtaPreconditionsErrorCode::PRECONDS_ERR_NONE) {
        OTA_LOG_E("%s", logMsg.c_str());
        //停止OTA任务超时监听
        StopOtaTaskTimeoutMonitor();
        if (precondErr != OtaPreconditionsErrorCode::PRECONDS_ERR_NONE) {
            ReportPreconditionsCheckResultCallback(g_otaTaskConfigInfo.taskId.c_str(), OTA_RET_ERR, precondErr);
        }
        g_otaTaskConfigInfo.errorCode = code;
        g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_UPGRADE_FAILED);
        UpdateOtaTaskConfig(g_otaTaskConfigInfo);
        SetOtaMasterState(OtaMasterState::OTA_STATE_UPGRADE_FAILED);
    };
    // 检查升级前置条件
    auto precondsError = CheckUpgradePreconditions();
    if (precondsError != OtaPreconditionsErrorCode::PRECONDS_ERR_NONE) {
        std::string logMsg = "Check upgrade preconditions failed";
        HandleError(OtaUpgradeErrorCode::OTA_ERR_NOT_SATISFIED_UPGRADE_CONDITION, logMsg, precondsError);
        return;
    }
    // 上报升级前置条件检测结果
    ReportPreconditionsCheckResultCallback(g_otaTaskConfigInfo.taskId.c_str(), OTA_RET_OK,
                                           OtaPreconditionsErrorCode::PRECONDS_ERR_NONE);

    // 二次校验所有已下载的文件完整性
    OtaUpgradeErrorCode errCode = OtaUpgradeErrorCode::OTA_OK;
    if (SecondCheckDownloadedFileIntegrity(g_otaTaskConfigInfo, errCode) != OTA_RET_OK) {
        std::string logMsg = "Check all downloaded file integrity failed";
        HandleError(errCode, logMsg);
        return;
    }

    // 校验所有ECU版本一致性
    if (CheckAllEcuPreUpgradeSwVersion(g_otaTaskConfigInfo) != OTA_RET_OK) {
        std::string logMsg = "Check all ECU version consistency failed";
        HandleError(OtaUpgradeErrorCode::OTA_ERR_OFFLINE_UPGRADE_OCCUR, logMsg);
        return;
    }

    //检测TBox&BCM防盗认证是否通过
    if (CheckTboxBcmAntiTheftAuthIsPassed() != OTA_RET_OK) {
        std::string logMsg = "Check TBOX BCM anti-theft authentication failed";
        HandleError(OtaUpgradeErrorCode::OTA_ERR_ANTI_THEFT_AUTH_FAILED, logMsg);
        return;
    }

    // 请求上高压
    if (RequestUpDownHighVoltage(OtaMasterUpgradePowerMode::OTA_HIGH_VOLTAGE_UPGRADE, true) != OTA_RET_OK) {
        std::string logMsg = "Request up high voltage failed";
        HandleError(OtaUpgradeErrorCode::OTA_ERR_APPLY_HIGH_VOLTAGE_FAILED, logMsg);
        return;
    }

    // 进入FOTA模式
    if (RequestSwitchFotaModeOnOff(CanSignalOtaMode::CAN_SIG_OTA_MODE_ON, true) != OTA_RET_OK) {
        std::string logMsg = "Set OTA mode on failed";
        HandleError(OtaUpgradeErrorCode::OTA_ERR_PROHIBIT_POWER_SWITCH_FAILED, logMsg);
        return;
    }

    // 切换到升级中状态
    g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_UPGRADING);
    UpdateOtaTaskConfig(g_otaTaskConfigInfo);
    SetOtaMasterState(OtaMasterState::OTA_STATE_UPGRADING);
}

/**
 * @brief 根据OTA ECU类型获取PM休眠事件ECU类型
 *
 * @param ecuType OTA ECU类型
 * @return EcuType_e PM休眠事件ECU类型
 */
static EcuType_e GetPmEcuTypeFromOtaEcuType(uint8_t ecuType)
{
    switch (ecuType) {
        case 0: // E_OTA_ECU_TYPE_TBOX
            return ECU_TYPE_TBOX;
        case 1: // E_OTA_ECU_TYPE_BCM
            return ECU_TYPE_OTHER;
        case 2: // E_OTA_ECU_TYPE_BMS
            return ECU_TYPE_BMS;
        case 3: // E_OTA_ECU_TYPE_VCU
            return ECU_TYPE_VCU;
        case 4: // E_OTA_ECU_TYPE_OTHER
        default:
            return ECU_TYPE_OTHER;
    }
}

/**
 * @brief 设置升级任务中所有ECU的休眠事件
 *
 * @param tasks ECU任务列表
 */
static void SetSleepEventForEcuTasks(const std::vector<OtaTaskEcuConfigInfo> &tasks)
{
    for (const auto &task : tasks) {
        EcuType_e pmEcuType = GetPmEcuTypeFromOtaEcuType(task.ecuType);
        if (OtaMasterPmSetSleepEvent(pmEcuType) != 0) {
            OTA_LOG_E("Failed to set sleep event for ECU[%s], type[%d]", task.ecuName.c_str(), task.ecuType);
        } else {
            OTA_LOG_I("Set sleep event for ECU[%s], type[%d] success", task.ecuName.c_str(), task.ecuType);
        }
    }
}

/**
 * @brief OTA状态机-升级中状态
 *
 */
static void HandleOtaMasterStateUpgrading(void)
{
    // 设置所有待升级ECU的休眠阻塞事件
    OTA_LOG_I("Setting sleep events for all ECU upgrade tasks...");
    SetSleepEventForEcuTasks(g_otaTaskConfigInfo.highVoltEcuTasksInfo);
    SetSleepEventForEcuTasks(g_otaTaskConfigInfo.lowVoltEcuTasksInfo);
    
    // 检测启动OTA刷写超时监听，当全任务超时时间不为0 ，不启用刷写超时监听
    if (g_otaTaskConfigInfo.taskDuration == 0 && g_otaTaskConfigInfo.upgradeDuration > 0) {
        g_otaTaskConfigInfo.taskStartTime =
            (g_otaTaskConfigInfo.taskStartTime == 0) ? time(nullptr) : g_otaTaskConfigInfo.taskStartTime;
        // 更新OTA任务刷写开始时间到配置文件
        UpdateOtaTaskConfig(g_otaTaskConfigInfo);
        g_isOtaTaskTimeout.store(false);
        // 启动OTA任务超时监听计时器
        StartOtaTaskTimeoutMonitor(g_otaTaskConfigInfo.taskStartTime, g_otaTaskConfigInfo.upgradeDuration,
                                   OtaTaskTimeoutCallback);
    }

    bool isTboxNeedReboot = false;
    // 高压/低压ECU升级具体处理函数
    auto ProcessVoltageTasks = [&](std::vector<OtaTaskEcuConfigInfo> &tasks, OtaMasterUpgradePowerMode mode,
                                   OtaUpgradeErrorCode failCode) {
        std::string modeStr = (mode == OtaMasterUpgradePowerMode::OTA_HIGH_VOLTAGE_UPGRADE) ? "high volt" : "low volt";
        OTA_LOG_D("Process %s ECU tasks", modeStr.c_str());
        if (tasks.empty() || g_otaTaskConfigInfo.errorCode != static_cast<uint8_t>(OtaUpgradeErrorCode::OTA_OK)) {
            OTA_LOG_E("Previous error:%d occurred, skip %s upgrade", g_otaTaskConfigInfo.errorCode, modeStr.c_str());
            return;
        }

        // 前置条件判断中已提前山高压，若是高压件升级，不需要再请求上高压
        // 如是低压件升级，请求下高压
        if (mode == OtaMasterUpgradePowerMode::OTA_LOW_VOLTAGE_UPGRADE) {
            if (RequestUpDownHighVoltage(mode, true) != OTA_RET_OK) {
                g_otaTaskConfigInfo.errorCode = static_cast<uint8_t>(failCode);
                return;
            }
        }

        // 升级ECU
        auto upgradeError = OtaUpgradeErrorCode::OTA_OK;
        ExecuteUpgradeEcu(tasks, upgradeError, isTboxNeedReboot);
        g_otaTaskConfigInfo.errorCode = static_cast<uint8_t>(upgradeError);
    };

    // 满足升级条件，上报平台开始升级
    ReportStartUpgradeCallback(g_otaTaskConfigInfo.taskId);

    g_otaTaskConfigInfo.errorCode = OtaUpgradeErrorCode::OTA_OK;
    // 升级高压件
    ProcessVoltageTasks(g_otaTaskConfigInfo.highVoltEcuTasksInfo, OtaMasterUpgradePowerMode::OTA_HIGH_VOLTAGE_UPGRADE,
                        OtaUpgradeErrorCode::OTA_ERR_APPLY_HIGH_VOLTAGE_FAILED);
    // 升级低压件
    ProcessVoltageTasks(g_otaTaskConfigInfo.lowVoltEcuTasksInfo, OtaMasterUpgradePowerMode::OTA_LOW_VOLTAGE_UPGRADE,
                        OtaUpgradeErrorCode::OTA_ERR_HIGH_VOLTAGE_DOWN_FAILED);

    if (g_otaTaskConfigInfo.errorCode == static_cast<uint8_t>(OtaUpgradeErrorCode::OTA_OK)) {
        // 全部ECU升级成功且包含TBox,判断进入重启TBox状态还是校验升级后的版本状态
        g_otaTaskConfigInfo.otaState = isTboxNeedReboot
                                           ? static_cast<uint8_t>(OtaMasterState::OTA_STATE_REBOOT_TBOX)
                                           : static_cast<uint8_t>(OtaMasterState::OTA_STATE_UPGRADE_SUCCESS);
    } else {
        //升级失败，进入失败回滚状态
        g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_ROLLBACK);
    }
    UpdateOtaTaskConfig(g_otaTaskConfigInfo);
    SetOtaMasterState(static_cast<OtaMasterState>(g_otaTaskConfigInfo.otaState));
    //停止OTA任务超时监听
    StopOtaTaskTimeoutMonitor();
}

/**
 * @brief OTA状态机-重启TBox状态
 */
static void HandleOtaMasterStateRebootTbox(void)
{
    // 更新升级状态为：OTA_STATE_VERIFY_TBOX_VERSION
    g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_VERIFY_TBOX_VERSION);
    UpdateOtaTaskConfig(g_otaTaskConfigInfo);
    SetOtaMasterState(static_cast<OtaMasterState>(g_otaTaskConfigInfo.otaState));
    // 通知TBox重启
    RequestTboxRebootCallback();
    {
        // 锁等待TBox重启完成后继续执行OtaMasterState::OTA_STATE_VERIFY_TBOX_VERSION状态机
        std::unique_lock<std::mutex> locker(g_stateMachineMutex);
        g_stateMachineCond.wait(locker, []{ return false; });
    }
}

/**
 * @brief OTA状态机-校验TBox升级后的版本状态
 */
static void HandleOtaMasterStateVerifyTboxVersion(void)
{
    if (CheckTBoxCurrentSwVersion(g_otaTaskConfigInfo, true) != OTA_RET_OK) {
        OTA_LOG_E("Check T-Box upgraded version consistency failed");
        g_otaTaskConfigInfo.errorCode = OtaUpgradeErrorCode::OTA_ERR_BRUSH_VERSION_INCONSISTENT;
        g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_ROLLBACK);
        UpdateOtaTaskConfig(g_otaTaskConfigInfo);
        SetOtaMasterState(OtaMasterState::OTA_STATE_ROLLBACK);
        return;
    }

    g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_UPGRADE_SUCCESS);
    UpdateOtaTaskConfig(g_otaTaskConfigInfo);
    SetOtaMasterState(OtaMasterState::OTA_STATE_UPGRADE_SUCCESS);
}

/**
 * @brief OTA状态机-失败回滚状态
 * 
 * @param[out] upgradeResult 升级失败结果
 * 
 * @return 无
 */
static void HandleOtaMasterStateRollback(void)
{
    // 防御性设置：确保回滚阶段也有休眠保护（幂等操作，已设置则跳过）
    // 正常情况下休眠事件从 HandleOtaMasterStateUpgrading 入口就已设置并保持至此
    OTA_LOG_I("Setting sleep events for rollback phase...");
    SetSleepEventForEcuTasks(g_otaTaskConfigInfo.highVoltEcuTasksInfo);
    SetSleepEventForEcuTasks(g_otaTaskConfigInfo.lowVoltEcuTasksInfo);

    bool isTboxNeedReboot = false;
    // 回调通知APP开始回滚
    ReportStartRollbackCallback(g_otaTaskConfigInfo.taskId.c_str());
    // 优先回滚控制高压电的ECU（BCM、VCU、IPU、BMS），这些ECU升级失败时无需上下高压直接回滚
    auto status = RollbackPowerControlEcusFirst(g_otaTaskConfigInfo);
    UpdateOtaTaskConfig(g_otaTaskConfigInfo);
    if (status == OtaEcuRollbackStatus::E_ROLLBACK_STATUS_FAILED) {
        OTA_LOG_E("Rollback BCM or VCU or IPU or BMS failed");
    } else {
        ExecuteRollbackEcu(isTboxNeedReboot);
    }

    //切换到升级失败状态
    g_otaTaskConfigInfo.otaState = isTboxNeedReboot ? static_cast<uint8_t>(OtaMasterState::OTA_STATE_TBOX_RBK_REBOOT)
                                                    : static_cast<uint8_t>(OtaMasterState::OTA_STATE_UPGRADE_FAILED);
    UpdateOtaTaskConfig(g_otaTaskConfigInfo);
    SetOtaMasterState(static_cast<OtaMasterState>(g_otaTaskConfigInfo.otaState));
}

/**
 * @brief OTA状态机-TBox回滚后重启状态
 * 
 */
static void HandleOtaMasterStateTBoxRollbackReboot(void)
{
    // 更新升级状态为：OOTA_STATE_TBOX_RBK_VERIFY，重启后校验TBox版本是否回退成功
    g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_TBOX_RBK_VERIFY);
    UpdateOtaTaskConfig(g_otaTaskConfigInfo);
    SetOtaMasterState(static_cast<OtaMasterState>(g_otaTaskConfigInfo.otaState));
    // 通知TBox重启
    RequestTboxRebootCallback();
    {
        // 锁等待TBox重启完成后继续执行OtaMasterState::OTA_STATE_TBOX_RBK_VERIFY状态机
        std::unique_lock<std::mutex> locker(g_stateMachineMutex);
        g_stateMachineCond.wait(locker, []{ return false; });
    }
}

/**
 * @brief OTA状态机-TBox回退版本重启后状态校验
 */
static void HandleOtaMasterStateTBoxRollbackVersionVerify(void)
{
    if (CheckTBoxCurrentSwVersion(g_otaTaskConfigInfo, false) != OTA_RET_OK) {
        OTA_LOG_E("Check T-Box rollback version consistency failed");
        auto *cfg = FindTBoxConfig(g_otaTaskConfigInfo.highVoltEcuTasksInfo, g_otaTaskConfigInfo.lowVoltEcuTasksInfo);
        if (!cfg) {
            OTA_LOG_E("TBox ECU not found in low or high volt ECU tasks");
            return;
        }
        cfg->rollbackState = OtaEcuRollbackStatus::E_ROLLBACK_STATUS_FAILED;
    }

    g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_UPGRADE_FAILED);
    UpdateOtaTaskConfig(g_otaTaskConfigInfo);
    SetOtaMasterState(OtaMasterState::OTA_STATE_UPGRADE_FAILED);
}

/**
 * @brief OTA状态机-下载失败状态
 * 
 */
static void HandleOtaMasterStateDownloadFailed(void)
{
    OTA_LOG_I("Download failed, force cleaning all sleep events for recovery");
    OtaMasterPmCleanAllSleepEvents();

    g_isResultReportHasAck.store(false);
    auto errCode = static_cast<OtaUpgradeErrorCode>(g_otaTaskConfigInfo.errorCode);
    if (ReportDownloadResultCallback(g_otaTaskConfigInfo.taskId.c_str(), OTA_RET_ERR, errCode) != OTA_RET_OK) {
        OTA_LOG_E("Report download failed result failed");
        std::unique_lock<std::mutex> locker(g_stateMachineMutex);
        g_stateMachineCond.wait_for(locker, std::chrono::seconds(30));
        return;
    }

    {
        std::unique_lock<std::mutex> locker(g_stateMachineMutex);
        // 等待下载失败结果上报完成并等待应答，超时时间30s
        g_stateMachineCond.wait_for(locker, std::chrono::seconds(30), [] { return g_isResultReportHasAck.load(); });
        if (!g_isResultReportHasAck.load()) {
            OTA_LOG_E("Report download failed result ack timeout");
            return;
        }
        g_isResultReportHasAck.store(false);
    }

    // 删除升级文件
    DeleteDownloadedOtaFile(g_otaTaskConfigInfo, false);
    // 重置OTA任务配置文件
    ResetOtaTaskConfig();
    // 上传OTA日志
    UploadOtaMasterLog();
    // 重置日志
    ResetOtaMasterLog();
    g_otaTaskConfigInfo.Clear();
    // 设置状态机状态为空闲
    SetOtaMasterState(OtaMasterState::OTA_STATE_IDLE);
}

/**
 * @brief OTA状态机-升级失败状态
 * 
 */
static void HandleOtaMasterStateUpgradeFailed(void)
{
    OTA_LOG_I("Upgrade failed, force cleaning all sleep events for recovery");
    OtaMasterPmCleanAllSleepEvents();

    // 获取升级失败结果
    auto upgradeResult = GetUpgradeFailedResult();
    // 失败错误码
    auto errorCode = static_cast<OtaUpgradeErrorCode>(g_otaTaskConfigInfo.errorCode);
    // 上报升级失败结果
    if (ReportOtaUpgradeResult(g_otaTaskConfigInfo.taskId, g_otaTaskConfigInfo.vehicleOtaVer, upgradeResult,
                               errorCode) != OTA_RET_OK) {
        return;
    }

    // 退出FOTA模式
    RequestSwitchFotaModeOnOff(CanSignalOtaMode::CAN_SIG_OTA_MODE_OFF, true);

    // 若回滚失败，则需要删除本地备份包
    auto isDelLocalBackupFile = (upgradeResult == OTA_UPGRADE_ROLLBACK_FAILED);
    // 删除升级文件
    DeleteDownloadedOtaFile(g_otaTaskConfigInfo, isDelLocalBackupFile);
    // 重置OTA任务配置文件
    ResetOtaTaskConfig();
    // 上传OTA日志
    UploadOtaMasterLog();
    // 重置日志
    ResetOtaMasterLog();
    g_otaTaskConfigInfo.Clear();
    // 设置状态机状态为空闲
    SetOtaMasterState(OtaMasterState::OTA_STATE_IDLE);
}

/**
 * @brief OTA状态机-升级成功状态
 */
static void HandleOtaMasterStateUpgradeSuccess(void)
{
    OTA_LOG_I("Upgrade success, force cleaning all sleep events");
    OtaMasterPmCleanAllSleepEvents();

    // 上报升级成功结果
    if (ReportOtaUpgradeResult(g_otaTaskConfigInfo.taskId, g_otaTaskConfigInfo.vehicleOtaVer,
                               OtaUpgradeResult::OTA_UPGRADE_SUCC, OtaUpgradeErrorCode::OTA_OK) != OTA_RET_OK) {
        return;
    }

    // 退出FOTA模式
    RequestSwitchFotaModeOnOff(CanSignalOtaMode::CAN_SIG_OTA_MODE_OFF, true);

    // 备份升级包
    BackupDownloadedOtaFile(g_otaTaskConfigInfo);
    // 重置OTA任务配置文件
    ResetOtaTaskConfig();
    // 上传OTA日志
    UploadOtaMasterLog();
    // 重置日志
    ResetOtaMasterLog();
    g_otaTaskConfigInfo.Clear();
    // 设置状态机状态为空闲
    SetOtaMasterState(OtaMasterState::OTA_STATE_IDLE);
}

/**
 * @brief 获取OTA状态机状态名称
 * 
 * @param state OTA状态机状态
 * 
 * @return OTA状态机状态名称
 */
static const char *GetOtaMasterStateName(OtaMasterState state)
{
    switch (state) {
        case OtaMasterState::OTA_STATE_INIT:
            return "OTA_STATE_INIT";
        case OtaMasterState::OTA_STATE_IDLE:
            return "OTA_STATE_IDLE";
        case OtaMasterState::OTA_STATE_PRE_DOWNLOAD:
            return "OTA_STATE_PRE_DOWNLOAD";
        case OtaMasterState::OTA_STATE_DOWNLOADING:
            return "OTA_STATE_DOWNLOADING";
        case OtaMasterState::OTA_STATE_DOWNLOAD_SUCCESS:
            return "OTA_STATE_DOWNLOAD_SUCCESS";
        case OtaMasterState::OTA_STATE_WAITING_FOR_USER_CONFIRM:
            return "OTA_STATE_WAITING_FOR_USER_CONFIRM";
        case OtaMasterState::OTA_STATE_CLEAR_TASK:
            return "OTA_STATE_CLEAR_TASK";
        case OtaMasterState::OTA_STATE_PRE_UPGRADE:
            return "OTA_STATE_PRE_UPGRADE";
        case OtaMasterState::OTA_STATE_UPGRADING:
            return "OTA_STATE_UPGRADING";
        case OtaMasterState::OTA_STATE_REBOOT_TBOX:
            return "OTA_STATE_REBOOT_TBOX";
        case OtaMasterState::OTA_STATE_VERIFY_TBOX_VERSION:
            return "OTA_STATE_VERIFY_TBOX_VERSION";
        case OtaMasterState::OTA_STATE_ROLLBACK:
            return "OTA_STATE_ROLLBACK";
        case OtaMasterState::OTA_STATE_TBOX_RBK_REBOOT:
            return "OTA_STATE_TBOX_RBK_REBOOT";
        case OtaMasterState::OTA_STATE_TBOX_RBK_VERIFY:
            return "OTA_STATE_TBOX_RBK_VERIFY";
        case OtaMasterState::OTA_STATE_DOWNLOAD_FAILED:
            return "OTA_STATE_DOWNLOAD_FAILED";
        case OtaMasterState::OTA_STATE_UPGRADE_FAILED:
            return "OTA_STATE_UPGRADE_FAILED";
        case OtaMasterState::OTA_STATE_UPGRADE_SUCCESS:
            return "OTA_STATE_UPGRADE_SUCCESS";
        default:
            return "UNKNOWN_STATE";
    }
}

/**
 * @brief OTA状态机处理线程
 * 
 * @param 无
 * 
 * @return 无
 */
static void OtaMasterStateMachineThread(void)
{
    while (g_isStateMachineInit.load()) {
        // 等待OTA状态机启动
        if (!g_isStateMachineStart.load()) {
            OTA_LOG_D("OTA master state machine is stopped, waiting for start...");
            std::unique_lock<std::mutex> locker(g_stateMachineMutex);
            g_stateMachineCond.wait_for(locker, std::chrono::seconds(600),
                                        []() { return g_isStateMachineStart.load(); });
            continue;
        }
        OtaMasterState state = GetOtaMasterState();
        OTA_LOG_D("----------> %s <----------", GetOtaMasterStateName(state));
        switch (state) {
            case OtaMasterState::OTA_STATE_INIT:
                HandleOtaMasterStateInit();
                break;
            case OtaMasterState::OTA_STATE_IDLE:
                HandleOtaMasterStateIdle();
                break;
            case OtaMasterState::OTA_STATE_PRE_DOWNLOAD:
                HandleOtaMasterStatePreDownload();
                break;
            case OtaMasterState::OTA_STATE_DOWNLOADING:
                HandleOtaMasterStateDownloading();
                break;
            case OtaMasterState::OTA_STATE_DOWNLOAD_SUCCESS:
                HandleOtaMasterStateDownloadSuccess();
                break;
            case OtaMasterState::OTA_STATE_WAITING_FOR_USER_CONFIRM:
                HandleOtaMasterStateWaitingForUserConfirm();
                break;
            case OtaMasterState::OTA_STATE_CLEAR_TASK:
                HandleOtaMasterStateClearTask();
                break;
            case OtaMasterState::OTA_STATE_PRE_UPGRADE:
                HandleOtaMasterStatePreUpgrade();
                break;
            case OtaMasterState::OTA_STATE_UPGRADING:
                HandleOtaMasterStateUpgrading();
                break;
            case OtaMasterState::OTA_STATE_REBOOT_TBOX:
                HandleOtaMasterStateRebootTbox();
                break;
            case OtaMasterState::OTA_STATE_VERIFY_TBOX_VERSION:
                HandleOtaMasterStateVerifyTboxVersion();
                break;
            case OtaMasterState::OTA_STATE_ROLLBACK:
                HandleOtaMasterStateRollback();
                break;
            case OtaMasterState::OTA_STATE_TBOX_RBK_REBOOT:
                HandleOtaMasterStateTBoxRollbackReboot();
                break;
            case OtaMasterState::OTA_STATE_TBOX_RBK_VERIFY:
                HandleOtaMasterStateTBoxRollbackVersionVerify();
                break;
            case OtaMasterState::OTA_STATE_DOWNLOAD_FAILED:
                HandleOtaMasterStateDownloadFailed();
                break;
            case OtaMasterState::OTA_STATE_UPGRADE_FAILED:
                HandleOtaMasterStateUpgradeFailed();
                break;
            case OtaMasterState::OTA_STATE_UPGRADE_SUCCESS:
                HandleOtaMasterStateUpgradeSuccess();
                break;
            default:
                break;
        }
    }
    OTA_LOG_D("OTA state machine thread exit");
}

int OtaStateManageInit(const std::string &configPath, const std::string &downloadFolder, const std::string &unzipFolder)
{
    OTA_LOG_D("----------> OTA state machine init");
    if (g_isStateMachineInit.load()) {
        OTA_LOG_E("OTA Master state machine has already been initialized\n");
        return OTA_RET_ERR;
    }

    g_downloadRootFolder = downloadFolder;
    g_unzipRootFolder = unzipFolder;
    OtaConfigInit(configPath);
    if (LoadSupportOtaEcuConfigInfo(g_supportOtaEcuConfigInfo) != OTA_RET_OK) {
        OTA_LOG_E("Load support OTA ECU config info failed");
        OtaConfigDeinit();
        return OTA_RET_ERR;
    }

    if (LoadOtaEcuPreconditionsConfigInfo(g_otaPreconditionsConfigInfo) != OTA_RET_OK) {
        OTA_LOG_E("Load OTA ECU preconditions config info failed");
        OtaConfigDeinit();
        return OTA_RET_ERR;
    }

    g_isStateMachineInit.store(true);
    SetOtaMasterState(OtaMasterState::OTA_STATE_INIT);
    std::thread([]() { OtaMasterStateMachineThread(); }).detach();
    return OTA_RET_OK;
}

void OtaStateManageDeinit(void)
{
    OTA_LOG_D("----------> OTA state machine deinit");
    OtaMasterPmCleanAllSleepEvents();

    if (!g_isStateMachineInit.load()) {
        OTA_LOG_E("OTA Master state machine has not been initialized\n");
        return;
    }
    g_downloadRootFolder.clear();
    g_unzipRootFolder.clear();
    OtaConfigDeinit();
    g_supportOtaEcuConfigInfo.clear();
    g_otaTaskConfigInfo.Clear();
    g_otaPreconditionsConfigInfo = {};
    g_isStateMachineInit.store(false);
    g_stateMachineCond.notify_all();
}

int StartOtaStateMachine(void)
{
    OTA_LOG_D("----------> Start OTA state machine");
    std::lock_guard<std::mutex> lock(g_stateMachineMutex);
    if (g_isStateMachineStart.load()) {
        OTA_LOG_E("OTA state machine is already started");
        return OTA_RET_ERR;
    }
    g_isStateMachineStart.store(true);
    g_stateMachineCond.notify_all();
    return OTA_RET_OK;
}

int StopOtaStateMachine(void)
{
    OTA_LOG_D("----------> Stop OTA state machine");
    auto state = GetOtaMasterState();
    if (state >= OtaMasterState::OTA_STATE_PRE_UPGRADE && state <= OtaMasterState::OTA_STATE_ROLLBACK) {
        OTA_LOG_E("Caurrent OTA state[%d]: %s, can't stop state machine.", state, GetOtaMasterStateName(state));
        return OTA_RET_ERR;
    }

    // 若正在下载升级文件，停止下载
    StopOtaFileDownload();
    {
        std::lock_guard<std::mutex> lock(g_stateMachineMutex);
        g_isStateMachineStart.store(false);
        g_stateMachineCond.notify_all();
    }
    return OTA_RET_OK;
}

int StartOtaTask(const StOtaTaskPolicyInfo *policyInfo)
{
    OTA_LOG_D("----------> Start OTA task");
    auto state = GetOtaMasterState();
    if (state != OtaMasterState::OTA_STATE_IDLE) {
        OTA_LOG_E("OTA state is not idle, can't start OTA task");
        return OTA_RET_ERR;
    }

    if (ExtractOtaTaskPolicyInfo(policyInfo, g_supportOtaEcuConfigInfo, g_otaTaskConfigInfo) != OTA_RET_OK) {
        OTA_LOG_E("Extract OTA task policy info failed");
        return OTA_RET_ERR;
    }

    // 启动OTA任务超时监听计时器
    g_isOtaTaskTimeout.store(false);
    if (g_otaTaskConfigInfo.taskDuration > 0) {
        StartOtaTaskTimeoutMonitor(g_otaTaskConfigInfo.taskStartTime, g_otaTaskConfigInfo.taskDuration,
                                   OtaTaskTimeoutCallback);
    }

    //切换OTA状态
    g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_PRE_DOWNLOAD);
    UpdateOtaTaskConfig(g_otaTaskConfigInfo);
    SetOtaMasterState(OtaMasterState::OTA_STATE_PRE_DOWNLOAD);
    g_stateMachineCond.notify_all();
    return OTA_RET_OK;
}

int ClearOtaTask(void)
{
    OTA_LOG_D("----------> Clear OTA task");
    auto state = GetOtaMasterState();
    if (state > OtaMasterState::OTA_STATE_WAITING_FOR_USER_CONFIRM) {
        OTA_LOG_E("OTA state is not waiting for user confirm, can't clear OTA task");
        ReportOtaTaskClearResultCallback(g_otaTaskConfigInfo.taskId.c_str(), OTA_RET_ERR);
        return OTA_RET_ERR;
    }
    //停止OTA任务超时监听
    StopOtaTaskTimeoutMonitor();
    //若正在下载升级文件，停止下载
    StopOtaFileDownload();
    g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_CLEAR_TASK);
    UpdateOtaTaskConfig(g_otaTaskConfigInfo);
    SetOtaMasterState(OtaMasterState::OTA_STATE_CLEAR_TASK);
    g_stateMachineCond.notify_all();
    //停止ota下载任务
    g_isOtaTaskTimeout.store(false);
    return OTA_RET_OK;
}

int ConfirmStartUpgrading(const std::string &taskId)
{
    OTA_LOG_D("----------> Confirm start upgrading");
    auto state = GetOtaMasterState();
    if (state != OtaMasterState::OTA_STATE_WAITING_FOR_USER_CONFIRM) {
        OTA_LOG_E("OTA state is not waiting for user confirm, can't start upgrading");
        return OTA_RET_ERR;
    }

    if (taskId != g_otaTaskConfigInfo.taskId) {
        OTA_LOG_E("Start upgrading failed, Task id[%s] is not match, current task id: %s", taskId.c_str(),
                  g_otaTaskConfigInfo.taskId.c_str());
        return OTA_RET_ERR;
    }

    g_otaTaskConfigInfo.otaState = static_cast<uint8_t>(OtaMasterState::OTA_STATE_PRE_UPGRADE);
    UpdateOtaTaskConfig(g_otaTaskConfigInfo);
    SetOtaMasterState(OtaMasterState::OTA_STATE_PRE_UPGRADE);
    g_stateMachineCond.notify_all();
    return OTA_RET_OK;
}

void NoticeEcuVersionAsyncQueryResult(const StEcuVersionInfo *ecuVersion, uint8_t ecuCnt)
{
    OTA_LOG_D("----------> Notice ECU version async query result");
    std::lock_guard<std::mutex> lock(g_stateMachineMutex);
    if (ecuVersion == nullptr) {
        OTA_LOG_E("ECU version is null");
        g_getOtaEcuVerAsyncResult.store(false);
        g_stateMachineCond.notify_all();
        return;
    }
    SetEcuVersionInfo(ecuVersion, ecuCnt);
    g_getOtaEcuVerAsyncResult.store(true);
    g_stateMachineCond.notify_all();
}

void NoticeUpgradePreconditionsAsyncQueryResult(const StUpgradePreconditions *preconditons)
{
    OTA_LOG_D("----------> Notice upgrade preconditions async query result");
    std::lock_guard<std::mutex> lock(g_stateMachineMutex);
    if (preconditons == nullptr) {
        OTA_LOG_E("preconditons is null");
        g_getOtaPrecondsAsyncResult.store(false);
        g_stateMachineCond.notify_all();
        return;
    }

    memcpy(&g_upgradePreconditions, preconditons, sizeof(StUpgradePreconditions));
    g_getOtaPrecondsAsyncResult.store(true);
    g_stateMachineCond.notify_all();
}

void NoticeReportResultHasACK(const std::string &taskId)
{
    OTA_LOG_D("----------> Notice task:%s report download or upgrade result has ACK", taskId.c_str());
    std::lock_guard<std::mutex> lock(g_stateMachineMutex);
    if (taskId != g_otaTaskConfigInfo.taskId) {
        OTA_LOG_E("Task id is not match, current task id: %s", g_otaTaskConfigInfo.taskId.c_str());
        g_isResultReportHasAck.store(false);
        return;
    }
    OTA_LOG_D("Task:%s report download or upgrade result has ACK", taskId.c_str());
    g_isResultReportHasAck.store(true);
    g_stateMachineCond.notify_all();
}

void NoticeUpDownHighVoltageResult(int result, OtaMasterUpgradePowerMode powerMode)
{
    OTA_LOG_D("----------> Notice up or down high voltage result, result: %d, powerMode: %d", result, powerMode);
    std::lock_guard<std::mutex> lock(g_stateMachineMutex);
    if (g_upDownPowerResult.first != powerMode) {
        OTA_LOG_E("Power mode is not match, current power mode: %d", g_upDownPowerResult.first);
        return;
    }
    if (result != OTA_RET_OK) {
        OTA_LOG_E("Up or down high voltage failed, powerMode: %d", powerMode);
        g_upDownPowerResult = std::make_pair(powerMode, false);
    } else {
        g_upDownPowerResult = std::make_pair(powerMode, true);
    }
    g_stateMachineCond.notify_all();
}

void NoticeFotaModeOnOffSwResult(int result, CanSignalOtaMode mode)
{
    OTA_LOG_D("----------> Notice OTA mode on or off result, result: %d, mode: %d", result, mode);
    std::lock_guard<std::mutex> lock(g_stateMachineMutex);
    if (g_otaModeOnOffSwResult.first != mode) {
        OTA_LOG_E("OTA mode is not match, current mode: %d", g_otaModeOnOffSwResult.first);
        return;
    }
    if (result != OTA_RET_OK) {
        OTA_LOG_E("Set OTA mode on or off failed, mode: %d", mode);
        g_otaModeOnOffSwResult = std::make_pair(mode, false);
    } else {
        g_otaModeOnOffSwResult = std::make_pair(mode, true);
    }
    g_stateMachineCond.notify_all();
}

OtaMasterStatus GetOtaStatus(void)
{
    auto state = GetOtaMasterState();
    OTA_LOG_D("----------> Get OTA status, current ota state: %d", state);
    if (state == OtaMasterState::OTA_STATE_INIT) {
        return OTA_NOT_READY;
    } else if (state == OtaMasterState::OTA_STATE_IDLE) {
        return OTA_READY;
    } else {
        return OTA_IN_PROGRESS;
    }
}