#include <unistd.h>
#include <atomic>
#include <array>
#include <mutex>
#include <condition_variable>
#include "ota_master_upgrade.h"
#include "ota_master_app_callback.h"
#include "ota_master_archive_manage.h"
#include "ota_master_ecu_brush.h"
#include "ota_master_inner_code.h"
#include "ota_master_utils.h"
#include "ota_master_log.h"

namespace {
static constexpr const char *SUB_FOLDER_UPGRADE = "upgrade";            // 升级子目录
static constexpr const char *SUB_FOLDER_BACKUP = "backup";              // 备份子目录
static constexpr uint8_t UDS_BRUSH_MAX_TIMES = 3;                       // UDS刷写最大次数

static std::mutex g_upgradeMutex;                                       // 升级互斥锁
static std::condition_variable g_upgradeCond;                           // 升级条件变量
static std::pair<int, OtaMasterTboxRebootState> g_tboxRollbackResult;   // TBOX回滚结果
static std::pair<int, OtaMasterTboxRebootState> g_tboxUpgradeResult;    // TBOX升级结果
} // namespace

/**
 * @brief 升级TBox
 * 
 * @param[in] taskConfig TBox OTA任务配置信息
 * @param[out] upgradeError 升级失败错误码
 * @param[out] isTboxNeedReboot TBox是否需要重启
 * 
 * @return TBox升级结果
 */
static OtaEcuUpgradeStatus UpgradeTbox(const OtaTaskEcuConfigInfo &taskConfig, OtaUpgradeErrorCode &upgradeError,
                                       bool &isTboxNeedReboot)
{
    g_tboxUpgradeResult = std::make_pair(OTA_RET_ERR, OtaMasterTboxRebootState::OTA_TBOX_NO_REBOOT);
    // 升级TBox
    if (RequestTboxSelfUpgradeCallback(taskConfig.upgradeFile.decompressFolder) != OTA_RET_OK) {
        OTA_LOG_E("Request TBox self upgrade failed");
        upgradeError = OtaUpgradeErrorCode::OTA_ERR_UPGRADING_MPU_NEGATIVE_RESPONSE;
        return OtaEcuUpgradeStatus::OTA_ECU_NOT_UPGRADE;
    }
    {
        // 等待TBox升级结果，最大等待15分钟
        std::unique_lock<std::mutex> locker(g_upgradeMutex);
        if (g_tboxUpgradeResult.first == OTA_RET_ERR) {
            g_upgradeCond.wait_for(locker, std::chrono::seconds(900));
        }
    }
    if (g_tboxUpgradeResult.first == OTA_RET_ERR) {
        isTboxNeedReboot = false;
        upgradeError = OtaUpgradeErrorCode::OTA_ERR_UPGRADING_MPU_NEGATIVE_RESPONSE;
        OTA_LOG_E("Upgrade TBox failed");
        return OtaEcuUpgradeStatus::OTA_ECU_UPGRADE_FAILED;
    }
    isTboxNeedReboot = (g_tboxUpgradeResult.second == OtaMasterTboxRebootState::OTA_TBOX_NEED_REBOOT);
    OTA_LOG_D("Upgrade TBox success");
    return OtaEcuUpgradeStatus::OTA_ECU_UPGRADE_SUCCESS;
}

/**
 * @brief 升级其他ECU
 * 
 * @param[in] ecuConfig ECU配置信息
 * @param[out] error 升级失败错误码
 * 
 * @return 其他ECU升级结果
 */
static OtaEcuUpgradeStatus UpgradeOtherEcu(const OtaTaskEcuConfigInfo &ecuConfig, OtaUpgradeErrorCode &error,
                                           IsOtaUpgradeTimeoutFunc isUpgradeTimeout)
{
    // 从解压目录中找到升级文件和驱动文件
    std::string driverFile;
    std::vector<std::string> upgradeFiles;
    if (FindEcuDriverAndUpgradeFile(ecuConfig.upgradeFile.decompressFolder, driverFile, upgradeFiles) != OTA_RET_OK) {
        OTA_LOG_E("Find upgrade file and driver file failed");
        error = OtaUpgradeErrorCode::OTA_ERR_UPGRADE_FILE_DECOMPRESS_FAILED;
        return OtaEcuUpgradeStatus::OTA_ECU_NOT_UPGRADE;
    }

    // 刷写其他ECU（带重试机制）
    uint8_t retryCount = 0;
    do {
        ++retryCount;
        OTA_LOG_I("Attempt %d/%d: Execute ECU brush, target ECU[0x%04X]:%s", retryCount, UDS_BRUSH_MAX_TIMES,
                  ecuConfig.ecuId, ecuConfig.ecuName.c_str());
        // 判断是否超时
        if (isUpgradeTimeout && isUpgradeTimeout()) {
            error = OtaUpgradeErrorCode::OTA_ERR_TASK_TIMEOUT;
            OTA_LOG_E("OTA task timeout, interrupt upgrade target ECU[0x%04x]:[%s]", ecuConfig.ecuId,
                      ecuConfig.ecuName.c_str());
            return OtaEcuUpgradeStatus::OTA_ECU_UPGRADE_FAILED;
        }

        // 执行ECU刷写
        error = ExecEcuBrush(ecuConfig.ecuId, ecuConfig.protocolInfo, driverFile, upgradeFiles, ecuConfig.targetVer);
        if (error == OtaUpgradeErrorCode::OTA_OK) {
            OTA_LOG_D("Upgrade target ECU[0x%04x]:[%s] success", ecuConfig.ecuId, ecuConfig.ecuName.c_str());
            return OtaEcuUpgradeStatus::OTA_ECU_UPGRADE_SUCCESS;
        }
        OTA_LOG_E("Execute brush target ECU[0x%04x]:[%s] failed, error code[%d]", ecuConfig.ecuId,
                  ecuConfig.ecuName.c_str(), static_cast<int>(error));

        // 失败时记录日志（最后一次失败才记录错误级别日志）
        if (error != OtaUpgradeErrorCode::OTA_ERR_UPGRADE_TARGET_ECU_FAILED || retryCount >= UDS_BRUSH_MAX_TIMES) {
            break;
        }
        OTA_LOG_E("ECU brush failed, %d retries remaining. Retrying...", UDS_BRUSH_MAX_TIMES - retryCount);
        sleep(50);
    } while (retryCount < UDS_BRUSH_MAX_TIMES);

    // 所有刷写重试失败
    OTA_LOG_E("Upgrade target ECU[0x%04x]:[%s] failed after %d attempts, error code[%d]", ecuConfig.ecuId,
              ecuConfig.ecuName.c_str(), retryCount, static_cast<int>(error));
    if (error == OtaUpgradeErrorCode::OTA_ERR_PROGRAM_FILE_CHECK_FAILED) {
        return OtaEcuUpgradeStatus::OTA_ECU_NOT_UPGRADE;
    } else {
        return (error == OtaUpgradeErrorCode::OTA_ERR_BRUSH_VERSION_INCONSISTENT)
                   ? OtaEcuUpgradeStatus::OTA_ECU_UPGRADE_SUCCESS
                   : OtaEcuUpgradeStatus::OTA_ECU_UPGRADE_FAILED;
    }
}

/**
 * @brief 获取ECU回滚方式
 * 
 * @param[in] ecuConfig ECU配置信息
 * 
 * @return 回滚类型
 */
static OtaRollbackMode GetEcuRollbackMode(const OtaTaskEcuConfigInfo &ecuConfig)
{
    OTA_LOG_D("Get ECU[0x%04x]:%s rollback mode, upgrade state: %d", ecuConfig.ecuId, ecuConfig.ecuName.c_str(),
              ecuConfig.upgradeState);
    auto upgradeState = static_cast<OtaEcuUpgradeStatus>(ecuConfig.upgradeState);
    if (upgradeState == OtaEcuUpgradeStatus::OTA_ECU_UPGRADE_SUCCESS) {
        auto rollbackMode = ecuConfig.rollbackInfo.succRollbackMode;
        if (rollbackMode > static_cast<uint8_t>(OtaRollbackMode::E_ROLLBACK_MODE_NO_NEED)) {
            OTA_LOG_E("ECU ID[0x%04x]:%s upgrade success rollback mode:%d is invalid, no need rollback version",
                      ecuConfig.ecuId, ecuConfig.ecuName.c_str(), rollbackMode);
            return OtaRollbackMode::E_ROLLBACK_MODE_NO_NEED;
        }
        OTA_LOG_D("ECU ID[0x%04x]:%s upgrade success rollback mode:%d", ecuConfig.ecuId, ecuConfig.ecuName.c_str(),
                  rollbackMode);
        return static_cast<OtaRollbackMode>(rollbackMode);
    } else if (upgradeState == OtaEcuUpgradeStatus::OTA_ECU_UPGRADE_FAILED) {
        auto rollbackMode = ecuConfig.rollbackInfo.failRollbackMode;
        if (rollbackMode > static_cast<uint8_t>(OtaRollbackMode::E_ROLLBACK_MODE_NO_NEED)) {
            OTA_LOG_E("ECU ID[0x%04x]:%s upgrade failed rollback mode:%d is invalid, no need rollback version",
                      ecuConfig.ecuId, ecuConfig.ecuName.c_str(), rollbackMode);
            return OtaRollbackMode::E_ROLLBACK_MODE_NO_NEED;
        }
        OTA_LOG_D("ECU ID[0x%04x]:%s upgrade failed rollback mode:%d", ecuConfig.ecuId, ecuConfig.ecuName.c_str(),
                  rollbackMode);
        return static_cast<OtaRollbackMode>(rollbackMode);
    }
    OTA_LOG_E("ECU ID[0x%04x]:%s not upgraded, no need rollback version", ecuConfig.ecuId, ecuConfig.ecuName.c_str());
    return OtaRollbackMode::E_ROLLBACK_MODE_NO_NEED;
}

/**
 * @brief 请求TBox回滚
 * 
 * @param[in] ecuConfig ECU配置信息
 * @param[out] isTboxNeedReboot TBox是否需要重启
 * 
 * @return
 *      OTA_RET_OK：回滚成功
 *      OTA_RET_ERR：回滚失败
 */
static int RollbackTbox(const OtaTaskEcuConfigInfo &ecuConfig, bool &isTboxNeedReboot)
{
    OTA_LOG_D("Rollback TBox");
    // 获取TBox回滚超时时间
    uint16_t timeout = (ecuConfig.rollbackInfo.rollbackTimeout == 0) ? 900 : ecuConfig.rollbackInfo.rollbackTimeout;
    OTA_LOG_D("TBox rollback timeout duration: %d", timeout);

    // 请求TBox回滚
    g_tboxRollbackResult = std::make_pair(OTA_RET_ERR, OtaMasterTboxRebootState::OTA_TBOX_NO_REBOOT);
    if (RequestTboxSelfRollbackCallback(ecuConfig.backupFile.decompressFolder) != OTA_RET_OK) {
        OTA_LOG_E("Request TBox self rollback failed");
        return OTA_RET_ERR;
    }

    OTA_LOG_D("Request TBox self rollback success, waiting for TBox rollback result");
    {
        // 等待TBox升级结果，最大等待15分钟
        std::unique_lock<std::mutex> locker(g_upgradeMutex);
        if (g_tboxRollbackResult.first == OTA_RET_ERR) {
            g_upgradeCond.wait_for(locker, std::chrono::seconds(timeout));
        }
    }
    // 设置TBox重启状态
    isTboxNeedReboot = (g_tboxRollbackResult.second == OtaMasterTboxRebootState::OTA_TBOX_NEED_REBOOT);
    // 检查TBox回滚结果
    if (g_tboxRollbackResult.first == OTA_RET_ERR) {
        OTA_LOG_E("TBox rollback failed");
        return OTA_RET_ERR;
    }
    OTA_LOG_D("TBox rollback success");
    return OTA_RET_OK;
}

/**
 * @brief 回滚目标ECU
 * 
 * @param[in] ecuConfig ECU配置信息
 * @param[out] isTboxNeedReboot TBox是否需要重启
 * 
 * @return
 *      OTA_RET_OK：回滚成功
 *      OTA_RET_ERR：回滚失败
 */
static int RollbackTargetEcuByTbox(OtaTaskEcuConfigInfo &ecuConfig, bool &isTboxNeedReboot)
{
    // 检测ECU回滚备份文件解压目录是否存在，不存在则创建
    if (CheckAndCreateDirectory(ecuConfig.unzipFolder) == false) {
        OTA_LOG_E("OTA backup file unzip directory[%s] is not exist", ecuConfig.unzipFolder.c_str());
        return OTA_RET_ERR;
    }

    // 生成解压文件夹路径
    std::string outputDir = std::move(GenerateFolderPath(ecuConfig.unzipFolder, SUB_FOLDER_BACKUP));
    // 解压之前删除旧文件夹，避免旧文件污染
    OTA_LOG_I("Delete old backup decompress folder: %s", outputDir.c_str());
    DeleteFolder(outputDir);
    sleep(1);
    // 解压文件
    if (DecompressFile(ecuConfig.backupFile.downloadPath, outputDir) != OTA_RET_OK) {
        OTA_LOG_E("Decompress backup file:%s failed", ecuConfig.backupFile.downloadPath.c_str());
        return OTA_RET_ERR;
    }
    // 保存解压目录
    ecuConfig.backupFile.decompressFolder = std::move(outputDir);
    OTA_LOG_I("Decompress backup file:%s to %s success", ecuConfig.backupFile.downloadPath.c_str(),
              ecuConfig.backupFile.decompressFolder.c_str());

    // 检测ECU类型是TBox
    if (ecuConfig.ecuType == OtaEcuType::E_OTA_ECU_TYPE_TBOX) {
        // 回滚TBox
        return RollbackTbox(ecuConfig, isTboxNeedReboot);
    }

    // 非TBox，从备份备份包解压目录中找到升级文件和驱动文件
    std::string driverFile;
    std::vector<std::string> upgradeFiles;
    if (FindEcuDriverAndUpgradeFile(ecuConfig.backupFile.decompressFolder, driverFile, upgradeFiles) != OTA_RET_OK) {
        OTA_LOG_E("Find backup upgrade file and driver file failed");
        return OTA_RET_ERR;
    }

    // 回滚刷写其他ECU（带重试机制）
    uint8_t retryCount = 0;
    do {
        ++retryCount;
        OTA_LOG_I("Attempt %d/%d: Execute ECU rollback, target ECU[0x%04X]:%s", retryCount, UDS_BRUSH_MAX_TIMES,
                  ecuConfig.ecuId, ecuConfig.ecuName.c_str());
        auto error = ExecEcuBrush(ecuConfig.ecuId, ecuConfig.protocolInfo, driverFile, upgradeFiles, ecuConfig.supportVer);
        if (error == OtaUpgradeErrorCode::OTA_OK) {
            OTA_LOG_D("Rollback ECU[0x%04x]:[%s] success", ecuConfig.ecuId, ecuConfig.ecuName.c_str());
            return OTA_RET_OK;
        }

        // 失败时记录日志（最后一次失败才记录错误级别日志）
        if (error != OtaUpgradeErrorCode::OTA_ERR_UPGRADE_TARGET_ECU_FAILED || retryCount >= UDS_BRUSH_MAX_TIMES) {
            break;
        }
        OTA_LOG_E("ECU brush failed, %d retries remaining. Retrying...", UDS_BRUSH_MAX_TIMES - retryCount);
        sleep(50);
    } while (retryCount < UDS_BRUSH_MAX_TIMES);

    // 所有回滚刷写重试失败
    OTA_LOG_E("Rollback ECU[0x%04x]:[%s] failed after %d attempts", ecuConfig.ecuId, ecuConfig.ecuName.c_str(),
              UDS_BRUSH_MAX_TIMES);
    return OTA_RET_ERR;
}

/**
 * @brief TBox通知目标ECU回滚
 * 
 * @param[in] ecuConfig ECU配置信息
 * 
 * @return 回滚结果
 */
static OtaEcuRollbackStatus TBoxNotifyTargetECURollback(const OtaTaskEcuConfigInfo &ecuConfig)
{
    // 检测ECU类型是否是TBox，避免配置错误误误发诊断回退
    if (ecuConfig.ecuType == OtaEcuType::E_OTA_ECU_TYPE_TBOX) {
        OTA_LOG_D("TBox no need rollback");
        return OtaEcuRollbackStatus::E_ROLLBACK_STATUS_NO_NEED;
    }

    constexpr uint16_t DEFAULT_TIMEOUT = 300;   // 默认回滚超时时间
    auto rollbackTimout =
        (ecuConfig.rollbackInfo.rollbackTimeout == 0) ? DEFAULT_TIMEOUT : ecuConfig.rollbackInfo.rollbackTimeout;

    OTA_LOG_I("Execute ECU rollback, target ECU[0x%04X]:%s, timeout: %d",
              ecuConfig.ecuId, ecuConfig.ecuName.c_str(), rollbackTimout);
    // 执行ECU回滚（只执行一次，无重试）
    if (ExecEcuRollback(ecuConfig.protocolInfo, rollbackTimout, ecuConfig.supportVer) == OTA_RET_OK) {
        OTA_LOG_D("Rollback ECU[0x%04x]:[%s] success", ecuConfig.ecuId, ecuConfig.ecuName.c_str());
        return OtaEcuRollbackStatus::E_ROLLBACK_STATUS_SUCCESS;
    }

    // 回滚ECU失败
    OTA_LOG_E("Rollback ECU[0x%04x]:[%s] failed", ecuConfig.ecuId, ecuConfig.ecuName.c_str());
    return OtaEcuRollbackStatus::E_ROLLBACK_STATUS_FAILED;
}

/**
 * @brief ECU自行回滚
 * 
 * @param[in] ecuConfig ECU配置信息
 * 
 * @return 回滚结果
 */
static OtaEcuRollbackStatus RollbackTargetEcuByThemself(const OtaTaskEcuConfigInfo &ecuConfig)
{
    // 检测ECU类型是否是TBox，避免配置错误误误发诊断回退
    if (ecuConfig.ecuType == OtaEcuType::E_OTA_ECU_TYPE_TBOX) {
        OTA_LOG_D("TBox no need rollback");
        return OtaEcuRollbackStatus::E_ROLLBACK_STATUS_NO_NEED;
    }

    // 每隔10s，查询目标件当前版本与回滚前版本是否一致，确认回滚是否完成，总计查询6次
    const int VER_CHECK_INTEVAL = 10; // 版本查询间隔
    uint8_t verCheckTimes = 6;        // 版本查询次数
    do {
        verCheckTimes--;
        sleep(VER_CHECK_INTEVAL);

        // 查询ECU件版本号与目标版本是否一致
        if (VerifySelfRollback(ecuConfig.protocolInfo, ecuConfig.supportVer) == OTA_RET_OK) {
            OTA_LOG_D("Rollback ECU[0x%04x]:[%s] success", ecuConfig.ecuId, ecuConfig.ecuName.c_str());
            return OtaEcuRollbackStatus::E_ROLLBACK_STATUS_SUCCESS;
        }
    } while (verCheckTimes > 0);

    // 回滚ECU失败
    OTA_LOG_E("Rollback ECU[0x%04x]:[%s] failed", ecuConfig.ecuId, ecuConfig.ecuName.c_str());
    return OtaEcuRollbackStatus::E_ROLLBACK_STATUS_FAILED;
}

OtaEcuUpgradeStatus UpgradeTargetEcu(OtaTaskEcuConfigInfo &ecuConfig, OtaUpgradeErrorCode &error,
                                     bool &isTboxNeedReboot, IsOtaUpgradeTimeoutFunc isOtaUpgradeTimeout)
{
    OTA_LOG_I("Start upgrade ECU[0x%04x]:[%s]", ecuConfig.ecuId, ecuConfig.ecuName.c_str());
    auto state = static_cast<OtaEcuUpgradeStatus>(ecuConfig.upgradeState);
    // 检测ECU是否已升级
    if (state == OtaEcuUpgradeStatus::OTA_ECU_UPGRADE_SUCCESS || state == OtaEcuUpgradeStatus::OTA_ECU_UPGRADE_FAILED) {
        OTA_LOG_D("ECU ID: %d has been upgraded", ecuConfig.ecuId);
        return state;
    }

    // 检测ECU升级文件解压目录是否存在，不存在则创建
    if (CheckAndCreateDirectory(ecuConfig.unzipFolder) == false) {
        OTA_LOG_E("OTA upgrade file unzip directory[%s] is not exist", ecuConfig.unzipFolder.c_str());
        error = OtaUpgradeErrorCode::OTA_ERR_UPGRADE_FILE_DECOMPRESS_FAILED;
        return OtaEcuUpgradeStatus::OTA_ECU_NOT_UPGRADE;
    }

    // 生成解压文件夹路径
    std::string outputDir = std::move(GenerateFolderPath(ecuConfig.unzipFolder, SUB_FOLDER_UPGRADE));
    // 解压之前删除旧文件夹，避免旧文件污染
    OTA_LOG_I("Delete old upgrade decompress folder: %s", outputDir.c_str());
    DeleteFolder(outputDir);
    sleep(1);
    // 解压文件
    if (DecompressFile(ecuConfig.upgradeFile.downloadPath, outputDir) != OTA_RET_OK) {
        OTA_LOG_E("Decompress file:%s failed", ecuConfig.upgradeFile.downloadPath.c_str());
        error = OtaUpgradeErrorCode::OTA_ERR_UPGRADE_FILE_DECOMPRESS_FAILED;
        return OtaEcuUpgradeStatus::OTA_ECU_NOT_UPGRADE;
    }
    ecuConfig.upgradeFile.decompressFolder = std::move(outputDir);
    OTA_LOG_I("Decompress upgrade file:%s to %s success", ecuConfig.upgradeFile.downloadPath.c_str(),
              ecuConfig.upgradeFile.decompressFolder.c_str());

    OTA_LOG_I("Start upgrade ECU[0x%04x]:[%s]", ecuConfig.ecuId, ecuConfig.ecuName.c_str());

    OtaEcuUpgradeStatus upgradeResult;
    // 检测ECU类型
    if (ecuConfig.ecuType == OtaEcuType::E_OTA_ECU_TYPE_TBOX) {
        upgradeResult = UpgradeTbox(ecuConfig, error, isTboxNeedReboot);
    } else {
        upgradeResult = UpgradeOtherEcu(ecuConfig, error, isOtaUpgradeTimeout);
    }

    return upgradeResult;
}

OtaEcuRollbackStatus RollbackPowerControlEcusFirst(OtaTaskConfigInfo &taskEcuConfig)
{
    OTA_LOG_D("First rollback power control ECUs (BCM/VCU/IPU/BMS) without high voltage...");
    // 若升级任务中有BCM、VCU、IPU、BMS且刷写失败，由于这些ECU控制上下高压电，最优先回滚（无需上下高压）
    OtaEcuRollbackStatus bcmRollbackStatus = OtaEcuRollbackStatus::E_ROLLBACK_STATUS_NO_NEED;
    OtaEcuRollbackStatus vcuRollbackStatus = OtaEcuRollbackStatus::E_ROLLBACK_STATUS_NO_NEED;
    OtaEcuRollbackStatus ipuRollbackStatus = OtaEcuRollbackStatus::E_ROLLBACK_STATUS_NO_NEED;
    OtaEcuRollbackStatus bmsRollbackStatus = OtaEcuRollbackStatus::E_ROLLBACK_STATUS_NO_NEED;
    // 统一处理高压/低压ECU中的BCM、VCU、IPU、BMS回滚
    auto rollbackEcu = [&](std::vector<OtaTaskEcuConfigInfo> &ecuTasksInfo) {
        bool isTboxNeedReboot = false; // TBox是否需要重启
        for (auto &config : ecuTasksInfo) {
            if (config.ecuType == OtaEcuType::E_OTA_ECU_TYPE_BCM &&
                config.upgradeState == OtaEcuUpgradeStatus::OTA_ECU_UPGRADE_FAILED) {
                OTA_LOG_D("Start rollback BCM...");
                bcmRollbackStatus = RollbackTargetEcu(config, isTboxNeedReboot);
                config.rollbackState = static_cast<uint8_t>(bcmRollbackStatus);
            } else if (config.ecuType == OtaEcuType::E_OTA_ECU_TYPE_VCU &&
                       config.upgradeState == OtaEcuUpgradeStatus::OTA_ECU_UPGRADE_FAILED) {
                OTA_LOG_D("Start rollback VCU...");
                vcuRollbackStatus = RollbackTargetEcu(config, isTboxNeedReboot);
                config.rollbackState = static_cast<uint8_t>(vcuRollbackStatus);
            } else if (config.ecuType == OtaEcuType::E_OTA_ECU_TYPE_BMS &&
                       config.upgradeState == OtaEcuUpgradeStatus::OTA_ECU_UPGRADE_FAILED) {
                OTA_LOG_D("Start rollback BMS...");
                bmsRollbackStatus = RollbackTargetEcu(config, isTboxNeedReboot);
                config.rollbackState = static_cast<uint8_t>(bmsRollbackStatus);
            } else if (config.ecuType == OtaEcuType::E_OTA_ECU_TYPE_OTHER &&
                       config.upgradeState == OtaEcuUpgradeStatus::OTA_ECU_UPGRADE_FAILED) {
                // 检查是否为IPU类型的ECU（通过ECU ID判断）
                if (config.ecuId == 0x716) { // IPU物理ID
                    OTA_LOG_D("Start rollback IPU...");
                    ipuRollbackStatus = RollbackTargetEcu(config, isTboxNeedReboot);
                    config.rollbackState = static_cast<uint8_t>(ipuRollbackStatus);
                }
            }
        }
    };

    // 遍历高压升级件，若有BCM或VCU则回滚
    rollbackEcu(taskEcuConfig.highVoltEcuTasksInfo);
    // 遍历低压升级件，若有BCM或VCU则回滚
    rollbackEcu(taskEcuConfig.lowVoltEcuTasksInfo);

    // BCM、VCU、IPU、BMS任意回滚失败，返回回滚失败
    if (bcmRollbackStatus == OtaEcuRollbackStatus::E_ROLLBACK_STATUS_FAILED ||
        vcuRollbackStatus == OtaEcuRollbackStatus::E_ROLLBACK_STATUS_FAILED ||
        ipuRollbackStatus == OtaEcuRollbackStatus::E_ROLLBACK_STATUS_FAILED ||
        bmsRollbackStatus == OtaEcuRollbackStatus::E_ROLLBACK_STATUS_FAILED) {
        OTA_LOG_E("BCM or VCU or IPU or BMS rollback failed");
        return OtaEcuRollbackStatus::E_ROLLBACK_STATUS_FAILED;
    }

    // BCM和VCU、IPU、BMS回滚均不需要，返回BCM、VCU、IPU、BMS无须回滚
    if (bcmRollbackStatus == OtaEcuRollbackStatus::E_ROLLBACK_STATUS_NO_NEED &&
        vcuRollbackStatus == OtaEcuRollbackStatus::E_ROLLBACK_STATUS_NO_NEED &&
        ipuRollbackStatus == OtaEcuRollbackStatus::E_ROLLBACK_STATUS_NO_NEED &&
        bmsRollbackStatus == OtaEcuRollbackStatus::E_ROLLBACK_STATUS_NO_NEED) {
        OTA_LOG_D("BCM and VCU and IPU and BMS rollback no need");
        return OtaEcuRollbackStatus::E_ROLLBACK_STATUS_NO_NEED;
    }
    return OtaEcuRollbackStatus::E_ROLLBACK_STATUS_SUCCESS;
}

OtaEcuRollbackStatus RollbackTargetEcu(OtaTaskEcuConfigInfo &ecuConfig, bool &isTboxNeedReboot)
{
    // 若ECU未升级或已回滚，无需回滚
    if (ecuConfig.upgradeState == OtaEcuUpgradeStatus::OTA_ECU_NOT_UPGRADE) {
        OTA_LOG_D("ECU[0x%04X]:[%s] not upgrade, no need rollback", ecuConfig.ecuId, ecuConfig.ecuName.c_str());
        return OtaEcuRollbackStatus::E_ROLLBACK_STATUS_NO_NEED;
    }

    // 若ECU已回滚或回滚失败，无需再次回滚
    if (ecuConfig.rollbackState == OtaEcuRollbackStatus::E_ROLLBACK_STATUS_SUCCESS ||
        ecuConfig.rollbackState == OtaEcuRollbackStatus::E_ROLLBACK_STATUS_FAILED) {
        return static_cast<OtaEcuRollbackStatus>(ecuConfig.rollbackState);
    }

    OTA_LOG_D("Rollback ECU[0x%04x]:[%s]", ecuConfig.ecuId, ecuConfig.ecuName.c_str());
    // 获取ECU回滚方式
    auto rollbackMode = GetEcuRollbackMode(ecuConfig);
    OTA_LOG_D("ECU[0x%04x]:[%s] rollback mode: %d", ecuConfig.ecuId, ecuConfig.ecuName.c_str(), rollbackMode);
    if (rollbackMode == OtaRollbackMode::E_ROLLBACK_MODE_BY_TBOX) {
        // 由TBox进行回滚
        if (RollbackTargetEcuByTbox(ecuConfig, isTboxNeedReboot) != OTA_RET_OK) {
            OTA_LOG_E("Rollback ECU[0x%04x]:[%s] failed", ecuConfig.ecuId, ecuConfig.ecuName.c_str());
            return OtaEcuRollbackStatus::E_ROLLBACK_STATUS_FAILED;
        }
        OTA_LOG_D("Rollback ECU[0x%04x]:[%s] success", ecuConfig.ecuId, ecuConfig.ecuName.c_str());
        return OtaEcuRollbackStatus::E_ROLLBACK_STATUS_SUCCESS;
    } else if (rollbackMode == OtaRollbackMode::E_ROLLBACK_MODE_BY_AB_SW) {
        // 目标ECU自行回滚
        return TBoxNotifyTargetECURollback(ecuConfig);
    } else {
        return RollbackTargetEcuByThemself(ecuConfig);
    }
    OTA_LOG_D("ECU[0x%04x]:[%s] no need rollback", ecuConfig.ecuId, ecuConfig.ecuName.c_str());
    // 无需回滚
    return OtaEcuRollbackStatus::E_ROLLBACK_STATUS_NO_NEED;
}

void NoticeTboxSelfUpgradeResult(int result, OtaMasterTboxRebootState rebootState)
{
    OTA_LOG_D("Notice TBox self upgrade result, result: %d, is need reboot: %d", result, rebootState);
    {
        std::lock_guard<std::mutex> locker(g_upgradeMutex);
        g_tboxUpgradeResult = std::make_pair(result, rebootState);
    }
    g_upgradeCond.notify_all();
}

void NoticeTboxRollbackResult(int result, OtaMasterTboxRebootState rebootState)
{
    OTA_LOG_D("Notice TBox rollback result: %d, reboot state: %d", result, rebootState);
    {
        std::lock_guard<std::mutex> locker(g_upgradeMutex);
        g_tboxRollbackResult = std::make_pair(result, rebootState);
    }
    g_upgradeCond.notify_all();
}