#include <error.h>
#include <json/json.h>
#include <algorithm>
#include <ctime>
#include <fstream>
#include <mutex>
#include <sstream>
#include "ota_master_inner_code.h"
#include "ota_master_config.h"
#include "ota_master_utils.h"
#include "ota_master_log.h"

namespace OtaTask {
static constexpr const char *KEY_TASK_ID = "task_id";
static constexpr const char *KEY_TASK_NAME = "task_name";
static constexpr const char *KEY_TASK_INFO = "task_info";
static constexpr const char *KEY_UPGRADE_MODE = "upgrade_mode";
static constexpr const char *KEY_IS_UPLOAD_LOG = "is_upload_log";
static constexpr const char *KEY_LOG_UPLOAD_ADDR = "log_upload_addr";
static constexpr const char *KEY_VEHICLE_OTA_VER = "vehicle_ota_ver";
static constexpr const char *KEY_TASK_DURATION = "task_duration";
static constexpr const char *KEY_UPGRADE_DURATION = "upgrade_duration";
static constexpr const char *KEY_TASK_START_TIME = "task_start_time";
static constexpr const char *KEY_OTA_STATE = "ota_state";
static constexpr const char *KEY_UPGRADE_ERR_CODE = "upgrade_err_code";
static constexpr const char *KEY_HIGH_VOLT_ECUS = "high_volt_ecus";
static constexpr const char *KEY_LOW_VOLT_ECUS = "low_volt_ecus";
static constexpr const char *KEY_ECU_ID = "ecu_id";
static constexpr const char *KEY_ECU_TYPE = "ecu_type";
static constexpr const char *KEY_ECU_NAME = "ecu_name";
static constexpr const char *KEY_SUPPLIER_ID = "supplier_id";
static constexpr const char *KEY_PART_NUM = "part_num";
static constexpr const char *KEY_CAN_PORT = "can_port";
static constexpr const char *KEY_UPGRADE_ORDER = "upgrade_order";
static constexpr const char *KEY_RB_TIMEOUT = "rollback_timeout";
static constexpr const char *KEY_SUCC_RB_MODE = "succ_rollback_mode";
static constexpr const char *KEY_FAIL_RB_MODE = "failed_rollback_mode";
static constexpr const char *KEY_PHYSICAL_ID = "physical_id";
static constexpr const char *KEY_FUNCTION_ID = "function_id";
static constexpr const char *KEY_RESPONSE_ID = "response_id";
static constexpr const char *KEY_DOWNLOAD_FOLDER = "download_folder";
static constexpr const char *KEY_UNZIP_FOLDER = "unzip_folder";
static constexpr const char *KEY_UPGRADE = "upgrade";
static constexpr const char *KEY_BACKUP = "backup";
static constexpr const char *KEY_PACK_SIZE = "package_size";
static constexpr const char *KEY_PACK_URL = "package_url";
static constexpr const char *KEY_PACK_MD5 = "package_md5";
static constexpr const char *KEY_PACK_DL_PATH = "package_dl_ptah";
static constexpr const char *KEY_DECOMPESS_PATH = "decompress_folder";
static constexpr const char *KEY_DOWNLOAD_STATE = "download_state";
static constexpr const char *KEY_IS_LOCAL_BACKUP = "is_local_backup";
static constexpr const char *KEY_HW_VER = "hw_ver";
static constexpr const char *KEY_TARGET_VER = "target_ver";
static constexpr const char *KEY_CURRENT_VER = "support_ver";
static constexpr const char *KEY_UPGRADE_STATE = "upgrade_state";
static constexpr const char *KEY_ROLL_BACK_STATE = "rollback_state";
} // namespace OtaTask

namespace OtaEcu {
static constexpr const char *KEY_FOTA_ECU = "fota_ecu";
static constexpr const char *KEY_NAME = "name";
static constexpr const char *KEY_ECU_ID = "ecu_id";
static constexpr const char *KEY_ECU_TYPE = "ecu_type";
static constexpr const char *KEY_CAN_PORT = "can_port";
static constexpr const char *KEY_PHYSICAL_ID = "physical_id";
static constexpr const char *KEY_FUNCTION_ID = "function_id";
static constexpr const char *KEY_REPONSE_ID = "response_id";
static constexpr const char *KEY_POWER_MODE = "power_mode";
static constexpr const char *KEY_ROLLBACK = "rollback";
static constexpr const char *KEY_SUPPLIER_ID = "supplier_id";
static constexpr const char *KEY_SUCC_ROLLBACK_MODE = "succ_roll_back_mode";
static constexpr const char *KEY_FAIL_ROLLBACK_MODE = "failed_roll_back_mode";
static constexpr const char *KEY_ROLLBACK_TIMEOUT = "rollback_timeout";
} // namespace OtaEcu

namespace OtaPreconditions {
static constexpr const char *KEY_BATTERY_VOLT = "battery_voltage";
static constexpr const char *KEY_GEAR_POSITION = "gear_position";
static constexpr const char *KEY_VEHICLE_SPEED = "vehicle_speed";
static constexpr const char *KEY_VCU_HV_NOT_READY = "vcu_hv_not_ready";
static constexpr const char *KEY_BMS_REAL_SOC = "bms_real_soc";
static constexpr const char *KEY_REQUIRED_VEHICLE_NO_FAULT = "required_vehicle_no_fault";
static constexpr const char *KEY_REQUIRED_PLUG_OFF = "required_plug_off";
static constexpr const char *KEY_REQUIRED_NO_REMOTE_CTRL_EXEC = "required_no_remote_ctrl_executing";
static constexpr const char *KEY_NO_OBD_DIAG_DEV_CONN = "no_obd_diagnostic_dev_conn";
static constexpr const char *KEY_BCM_ANTI_THEFT_AUTH_PASSED = "bcm_anti_theft_auth_passed";
static constexpr const char *KEY_ENABLED = "enabled";
static constexpr const char *KEY_MIN_VOLTAGE = "min_voltage";
static constexpr const char *KEY_REQUIRED_GEAR = "required_gear";
static constexpr const char *KEY_REQUIRED_SPEED = "required_speed";
static constexpr const char *KEY_CANNOT_BE = "cannot_be";
static constexpr const char *KEY_MIN_SOC = "min_soc";
} // namespace OtaPreconditions

namespace {
static constexpr const char *SUPPORT_OTA_ECU_CONFIG_FILE = "support_ota_ecu_config.json"; //支持OTA升级的ECU配置文件
static constexpr const char *OTA_TASK_CONFIG_FILE = "ota_task_config.json";               // OTA任务配置文件
static constexpr const char *OTA_PRECONDITIONS_CONFIG_FILE = "ota_preconditions_config.json"; // OTA前置条件配置文件

static std::string g_configFolderPath;       //配置文件所在文件夹路径
static std::string g_supportOtaConfigPath;   //支持OTA升级的ECU配置文件路径
static std::string g_otaTaskConfigPath;      // OTA任务配置文件路径
static std::string g_otaPreocondsConfigPath; // OTA前置条件配置文件路径
static std::mutex g_otaTaskConfigMutex;      // OTA任务配置文件锁
} // namespace

/**
 * 加载配置文件，获取Root Json::Value
 * 
 * @param[in] configFile 配置文件绝对路径
 * @param[out] root Root Json::Value
 *
 * @return
 *      OTA_RET_OK：加载获取成功
 *      OTA_RET_ERR：加载获取失败
 */
static int GetConfigFileRootJsonValue(const std::string &configFile, Json::Value &root)
{
    std::ifstream is(configFile, std::ios::binary);
    if (!is.is_open()) {
        OTA_LOG_E("Open %s failed, error[%d]: %s", configFile.c_str(), errno, strerror(errno));
        return OTA_RET_ERR;
    }

    // 检查文件是否为空
    is.seekg(0, std::ios::end);
    std::streampos fileSize = is.tellg();
    if (fileSize <= 0) {
        OTA_LOG_E("Config file %s is empty", configFile.c_str());
        is.close();
        return OTA_RET_ERR;
    }
    is.seekg(0, std::ios::beg);

    Json::Reader reader;
    if (!reader.parse(is, root)) {
        OTA_LOG_E("Parse %s as json data failed", configFile.c_str());
        is.close();
        return OTA_RET_ERR;
    }

    is.close();
    return OTA_RET_OK;
}

/**
 * 通过指定Key从root中提取对应value
 *
 * @param[in] root Root Json::Value
 * @param[in] key key
 * @param[out] value Json::value
 *
 * @return
 *      OTA_RET_OK：获取成功
 *      OTA_RET_ERR：获取失败
 */
static int GetValueFromRoot(const Json::Value &root, const char *key, Json::Value &value)
{
    if (key == nullptr) {
        OTA_LOG_E("Get value from json root failed, key is nullptr");
        return OTA_RET_ERR;
    }

    if (!root.isMember(key)) {
        OTA_LOG_E("Json::value does not have member %s", key);
        return OTA_RET_ERR;
    }

    value = root[key];
    return OTA_RET_OK;
}

/**
 * 通过指定Key从root中提取Array类型Json::Value
 *
 * @param[in] root Root Json::Value
 * @param[in] key key
 * @param[out] arrayValue Array类型Json::Value
 *
 * @return
 *      OTA_RET_OK：获取成功
 *      OTA_RET_ERR：获取失败
 */
static int GetArrayValueFromRoot(const Json::Value &root, const char *key, Json::Value &arrayValue)
{
    if (GetValueFromRoot(root, key, arrayValue) != OTA_RET_OK) {
        return OTA_RET_ERR;
    }

    if (!root[key].isArray()) {
        OTA_LOG_E("Json::value member %s is not of type Array", key);
        return OTA_RET_ERR;
    }
    arrayValue = root[key];
    return OTA_RET_OK;
}

/**
 * 通过指定Key从Json::Value提取uint类型值
 * 
 * @param[in] value Json::value
 * @param[in] key 成员key
 *
 * @return uint32_t类型值
*/
template <typename T>
static T GetUIntFromJsonValue(const Json::Value &root, const char *key)
{
    static_assert(std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value ||
                      std::is_same<T, uint32_t>::value || std::is_same<T, uint64_t>::value,
                  "T must be an unsigned integer");
    if (key == nullptr) {
        OTA_LOG_E("Get int from json value failed, key is nullptr");
        return 0;
    }

    if (!root.isMember(key)) {
        OTA_LOG_E("Json::value does not have member %s", key);
        return 0;
    }

    if (!root[key].isUInt()) {
        OTA_LOG_E("Json::value member %s is not of type UInt", key);
        return 0;
    }

    return static_cast<T>(root[key].asUInt());
}

/**
 * 通过指定Key从Json::Value提取double类型值
 * 
 * @param[in] value Json::value
 * @param[in] key 成员key
 *
 * @return double类型值
*/
static double GetDoubleFromJsonValue(const Json::Value &root, const char *key)
{
    if (key == nullptr) {
        OTA_LOG_E("Get int from json value failed, key is nullptr");
        return 0;
    }

    if (!root.isMember(key)) {
        OTA_LOG_E("Json::value does not have member %s", key);
        return 0;
    }

    if (!root[key].isDouble()) {
        OTA_LOG_E("Json::value member %s is not of type Double", key);
        return 0;
    }

    return static_cast<double>(root[key].asDouble());
}

/**
 * 通过指定Key从Json::Value提取bool类型值
 * 
 * @param[in] value Json::value
 * @param[in] key 成员key
 *
 * @return bool类型值
 */
static bool GetBoolFromJsonValue(const Json::Value &root, const char *key)
{
    if (key == nullptr) {
        OTA_LOG_E("Get int from json value failed, key is nullptr");
        return 0;
    }

    if (!root.isMember(key)) {
        OTA_LOG_E("Json::value does not have member %s", key);
        return 0;
    }

    if (!root[key].isBool()) {
        OTA_LOG_E("Json::value member %s is not of type Bool", key);
        return 0;
    }

    return root[key].asBool();
}

/**
 * 通过指定Key从Json::Value提取String类型值
 * 
 * @param[in] value Json::value
 * @param[in] key 成员key
 * @param[out] value 提取到的string value
 *
 * @return 无
 */
static void GetStringFromJsonValue(const Json::Value &root, const char *key, std::string &value)
{
    if (key == nullptr) {
        OTA_LOG_E("Get int from json value failed, key is nullptr");
        return;
    }

    if (!root.isMember(key)) {
        OTA_LOG_E("Json::value does not have member %s", key);
        return;
    }

    if (!root[key].isString()) {
        OTA_LOG_E("Json::value member %s is not of type String", key);
        return;
    }

    value = root[key].asString();
}

/**
 * @brief 从json文件中提取高压/低压升级任务配置信息
 * 
 * @param[in] root Json::Value
 * @param[in] poserMode 电源模式
 * @param[out] config 升级任务配置信息
 * 
 * @return
 *     OTA_RET_OK：提取成功
 *     OTA_RET_ERR：提取失败
 */
static int GetOtaTaskEcuConfigInfo(const Json::Value &root, const std::string &key,
                                   std::vector<OtaTaskEcuConfigInfo> &config)
{
    Json::Value taskArray;
    if (GetArrayValueFromRoot(root, key.c_str(), taskArray) != OTA_RET_OK) {
        OTA_LOG_E("Get array Json::Value failed");
        return OTA_RET_ERR;
    }

    for (int i = 0; i < taskArray.size(); ++i) {
        OtaTaskEcuConfigInfo taskEcuConfig;
        taskEcuConfig.ecuId = GetUIntFromJsonValue<uint16_t>(taskArray[i], OtaTask::KEY_ECU_ID);
        taskEcuConfig.ecuType = GetUIntFromJsonValue<uint8_t>(taskArray[i], OtaTask::KEY_ECU_TYPE);
        GetStringFromJsonValue(taskArray[i], OtaTask::KEY_ECU_NAME, taskEcuConfig.ecuName);
        GetStringFromJsonValue(taskArray[i], OtaTask::KEY_SUPPLIER_ID, taskEcuConfig.sullpierId);
        GetStringFromJsonValue(taskArray[i], OtaTask::KEY_PART_NUM, taskEcuConfig.partNum);
        taskEcuConfig.protocolInfo.canPort = GetUIntFromJsonValue<uint8_t>(taskArray[i], OtaTask::KEY_CAN_PORT);
        taskEcuConfig.upgradeOrder = GetUIntFromJsonValue<uint8_t>(taskArray[i], OtaTask::KEY_UPGRADE_ORDER);
        taskEcuConfig.rollbackInfo.rollbackTimeout = GetUIntFromJsonValue<uint16_t>(taskArray[i], OtaTask::KEY_RB_TIMEOUT);
        taskEcuConfig.rollbackInfo.succRollbackMode =
            GetUIntFromJsonValue<uint8_t>(taskArray[i], OtaTask::KEY_SUCC_RB_MODE);
        taskEcuConfig.rollbackInfo.failRollbackMode = GetUIntFromJsonValue<uint8_t>(taskArray[i], OtaTask::KEY_FAIL_RB_MODE);
        taskEcuConfig.protocolInfo.physicalId = GetUIntFromJsonValue<uint32_t>(taskArray[i], OtaTask::KEY_PHYSICAL_ID);
        taskEcuConfig.protocolInfo.functionId = GetUIntFromJsonValue<uint32_t>(taskArray[i], OtaTask::KEY_FUNCTION_ID);
        taskEcuConfig.protocolInfo.responseId = GetUIntFromJsonValue<uint32_t>(taskArray[i], OtaTask::KEY_RESPONSE_ID);
        Json::Value upgradeValue, backupValue;
        // 读取upgrade文件下载/解压信息
        if (GetValueFromRoot(taskArray[i], OtaTask::KEY_UPGRADE, upgradeValue) == OTA_RET_OK) {
            taskEcuConfig.upgradeFile.downloadSize =
                GetUIntFromJsonValue<uint32_t>(upgradeValue, OtaTask::KEY_PACK_SIZE);
            GetStringFromJsonValue(upgradeValue, OtaTask::KEY_PACK_URL, taskEcuConfig.upgradeFile.downloadUrl);
            GetStringFromJsonValue(upgradeValue, OtaTask::KEY_PACK_MD5, taskEcuConfig.upgradeFile.downloadMD5);
            GetStringFromJsonValue(upgradeValue, OtaTask::KEY_PACK_DL_PATH, taskEcuConfig.upgradeFile.downloadPath);
            GetStringFromJsonValue(upgradeValue, OtaTask::KEY_DECOMPESS_PATH,
                                   taskEcuConfig.upgradeFile.decompressFolder);
            taskEcuConfig.upgradeFile.downloadState =
                GetUIntFromJsonValue<uint8_t>(upgradeValue, OtaTask::KEY_DOWNLOAD_STATE);
            taskEcuConfig.upgradeState = GetUIntFromJsonValue<uint8_t>(upgradeValue, OtaTask::KEY_UPGRADE_STATE);
        }
        // 读取backup文件下载/解压信息
        if (GetValueFromRoot(taskArray[i], OtaTask::KEY_BACKUP, backupValue) == OTA_RET_OK) {
            taskEcuConfig.backupFile.downloadSize = GetUIntFromJsonValue<uint32_t>(backupValue, OtaTask::KEY_PACK_SIZE);
            GetStringFromJsonValue(backupValue, OtaTask::KEY_PACK_URL, taskEcuConfig.backupFile.downloadUrl);
            GetStringFromJsonValue(backupValue, OtaTask::KEY_PACK_MD5, taskEcuConfig.backupFile.downloadMD5);
            GetStringFromJsonValue(backupValue, OtaTask::KEY_PACK_DL_PATH, taskEcuConfig.backupFile.downloadPath);
            GetStringFromJsonValue(backupValue, OtaTask::KEY_DECOMPESS_PATH, taskEcuConfig.backupFile.decompressFolder);
            taskEcuConfig.backupFile.downloadState =
                GetUIntFromJsonValue<uint8_t>(backupValue, OtaTask::KEY_DOWNLOAD_STATE);
            taskEcuConfig.isLocalBackup = GetUIntFromJsonValue<uint8_t>(backupValue, OtaTask::KEY_IS_LOCAL_BACKUP);
            taskEcuConfig.rollbackState = GetUIntFromJsonValue<uint8_t>(backupValue, OtaTask::KEY_ROLL_BACK_STATE);
        }
        GetStringFromJsonValue(taskArray[i], OtaTask::KEY_DOWNLOAD_FOLDER, taskEcuConfig.dlFolder);
        GetStringFromJsonValue(taskArray[i], OtaTask::KEY_UNZIP_FOLDER, taskEcuConfig.unzipFolder);
        GetStringFromJsonValue(taskArray[i], OtaTask::KEY_HW_VER, taskEcuConfig.hwVer);
        GetStringFromJsonValue(taskArray[i], OtaTask::KEY_TARGET_VER, taskEcuConfig.targetVer);
        GetStringFromJsonValue(taskArray[i], OtaTask::KEY_CURRENT_VER, taskEcuConfig.supportVer);

        // 根据 upgradeOrder 排序
        auto pos = std::lower_bound(config.begin(), config.end(), taskEcuConfig,
                                    [](const OtaTaskEcuConfigInfo &lhs, const OtaTaskEcuConfigInfo &rhs) {
                                        return lhs.upgradeOrder < rhs.upgradeOrder;
                                    });
        OTA_LOG_D("Insert task[%d] into vector pos: %d", i, pos);
        config.insert(pos, taskEcuConfig);
    }
    return OTA_RET_OK;
}

/**
 * @brief 提取默认配置的OTA任务刷写协议
 * 
 * @param[in] supportConfig 支持OTA升级的ECU配置信息
 * @param[in] flashProtocol OTA任务刷写协议
 * @param[inout] config ECU配置信息
 * 
 * @return 无
 */
static void GetEcuFlashProtocolAndRollbackInfo(const SupportOTAEcuConfigMap &supportConfig,
                                               const StOtaTaskPolicyFlashProtocol &flashProtocol,
                                               OtaTaskEcuConfigInfo &config)
{
    const auto &configIter = supportConfig.find(config.ecuId);
    if (configIter == supportConfig.end()) {
        OTA_LOG_E("ECU ID[0x%04x] is not supported", config.ecuId);
        return;
    }

    // 提取刷写协议信息
    config.ecuType = configIter->second.ecuType;
    config.protocolInfo.canPort =
        flashProtocol.canPort == 0xFF ? configIter->second.protocolInfo.canPort : flashProtocol.canPort;
    config.protocolInfo.physicalId =
        flashProtocol.physicalId == 0xFFFFFFFF ? configIter->second.protocolInfo.physicalId : flashProtocol.physicalId;
    config.protocolInfo.functionId =
        flashProtocol.functionId == 0xFFFFFFFF ? configIter->second.protocolInfo.functionId : flashProtocol.functionId;
    config.protocolInfo.responseId =
        flashProtocol.responseId == 0xFFFFFFFF ? configIter->second.protocolInfo.responseId : flashProtocol.responseId;

    // 提取回滚配置信息
    const auto &infoIter = configIter->second.rollbackInfo.find(config.sullpierId);
    if (infoIter == configIter->second.rollbackInfo.end()) {
        OTA_LOG_E("ECU ID[0x%04x] does not have rollback info for supplier ID[%s]", config.ecuId,
                    config.sullpierId.c_str());
        return;
    }
    config.rollbackInfo = infoIter->second;
}

/**
 * @brief 将ECU配置信息插入到对应的vector中
 * 
 * @param[in] supportConfig 支持OTA升级的ECU配置信息
 * @param[in] config ECU配置信息
 * @param[out] otaTaskConfig OTA任务配置信息
 * 
 * @return 无
 */
static void InsertOtaTaskEcuConfigIntoVec(const SupportOTAEcuConfigMap &supportConfig, OtaTaskEcuConfigInfo &config,
                                          OtaTaskConfigInfo &otaTaskConfig)
{
    auto iter = supportConfig.find(config.ecuId);
    if (iter == supportConfig.end()) {
        OTA_LOG_E("ECU ID[%d] is not supported", config.ecuId);
        return;
    }
    // 设置ECU名称
    config.ecuName = iter->second.name;
    auto &targetList = (iter->second.powerMode == OTA_LOW_VOLTAGE_UPGRADE) ? otaTaskConfig.lowVoltEcuTasksInfo
                                                                           : otaTaskConfig.highVoltEcuTasksInfo;
    // 根据 upgradeOrder 从小到大排序
    auto pos = std::lower_bound(targetList.begin(), targetList.end(), config,
                                [](const OtaTaskEcuConfigInfo &lhs, const OtaTaskEcuConfigInfo &rhs) {
                                    return lhs.upgradeOrder < rhs.upgradeOrder;
                                });
    targetList.insert(pos, config);
}

/**
 * @brief 打印OTA任务ECU配置信息
 * 
 * @param[in] config OTA任务ECU配置信息
 * 
 * @return 无
 */
static void PrintTaskEcuConfigInfo(const OtaTaskEcuConfigInfo &config)
{
    OTA_LOG_D("        {");
    OTA_LOG_D("            \"ecu_id\": 0x%04x,", config.ecuId);
    OTA_LOG_D("            \"ecu_type\": %d,", config.ecuType);
    OTA_LOG_D("            \"ecu_name\": %s,", config.ecuName.c_str());
    OTA_LOG_D("            \"supplier_id\": %s,", config.sullpierId.c_str());
    OTA_LOG_D("            \"part_num\": %s,", config.partNum.c_str());
    OTA_LOG_D("            \"can_port\": %d,", config.protocolInfo.canPort);
    OTA_LOG_D("            \"upgrade_order\": %d,", config.upgradeOrder);
    OTA_LOG_D("            \"rollback_timeout\": %d,", config.rollbackInfo.rollbackTimeout);
    OTA_LOG_D("            \"succ_rollback_mode\": %d,", config.rollbackInfo.succRollbackMode);
    OTA_LOG_D("            \"failed_rollback_mode\": %d,", config.rollbackInfo.failRollbackMode);
    OTA_LOG_D("            \"physical_id\": 0x%04x,", config.protocolInfo.physicalId);
    OTA_LOG_D("            \"function_id\": 0x%04x,", config.protocolInfo.functionId);
    OTA_LOG_D("            \"response_id\": 0x%04x,", config.protocolInfo.responseId);
    OTA_LOG_D("            \"download_folder\": %s,", config.dlFolder.c_str());
    OTA_LOG_D("            \"unzip_folder\": %s,", config.unzipFolder.c_str());
    OTA_LOG_D("            \"upgrade\": {");
    OTA_LOG_D("                \"package_size\": %d,", config.upgradeFile.downloadSize);
    OTA_LOG_D("                \"package_url\": %s,", config.upgradeFile.downloadUrl.c_str());
    OTA_LOG_D("                \"package_md5\": %s,", config.upgradeFile.downloadMD5.c_str());
    OTA_LOG_D("                \"package_dl_ptah\": %s,", config.upgradeFile.downloadPath.c_str());
    OTA_LOG_D("                \"decompress_folder\": %s,", config.upgradeFile.decompressFolder.c_str());
    OTA_LOG_D("                \"download_state\": %d,", config.upgradeFile.downloadState);
    OTA_LOG_D("                \"upgrade_state\": %d,", config.upgradeState);
    OTA_LOG_D("            },");
    OTA_LOG_D("            \"backup\": {");
    OTA_LOG_D("                \"package_size\": %d,", config.backupFile.downloadSize);
    OTA_LOG_D("                \"package_url\": %s,", config.backupFile.downloadUrl.c_str());
    OTA_LOG_D("                \"package_md5\": %s,", config.backupFile.downloadMD5.c_str());
    OTA_LOG_D("                \"package_dl_ptah\": %s,", config.backupFile.downloadPath.c_str());
    OTA_LOG_D("                \"decompress_folder\": %s,", config.backupFile.decompressFolder.c_str());
    OTA_LOG_D("                \"download_state\": %d,", config.backupFile.downloadState);
    OTA_LOG_D("                \"is_local_backup\": %d,", config.isLocalBackup);
    OTA_LOG_D("                \"rollback_state\": %d,", config.rollbackState);
    OTA_LOG_D("            },");
    OTA_LOG_D("            \"hw_ver\": %s,", config.hwVer.c_str());
    OTA_LOG_D("            \"target_ver\": %s,", config.targetVer.c_str());
    OTA_LOG_D("            \"support_ver\": %s", config.supportVer.c_str());
    OTA_LOG_D("        }");
}

/**
 * @brief 打印全部OTA任务配置信息
 * 
 * @param[in] taskConfig OTA任务配置信息
 * 
 * @return 无
 */
static void PrintOtaTaskConfigInfo(const OtaTaskConfigInfo &taskConfig){
    OTA_LOG_D("OTA task config info --->");
    OTA_LOG_D("{");
    OTA_LOG_D("    \"task_id\": %s,", taskConfig.taskId.c_str());
    OTA_LOG_D("    \"task_name\": %s,", taskConfig.taskName.c_str());
    OTA_LOG_D("    \"task_info\": %s,", taskConfig.taskInfo.c_str());
    OTA_LOG_D("    \"upgrade_mode\": %d,", taskConfig.upgradeMode);
    OTA_LOG_D("    \"is_upload_log\": %d,", taskConfig.isUploadLog);
    OTA_LOG_D("    \"log_upload_addr\": %s,", taskConfig.logUploadAddr.c_str());
    OTA_LOG_D("    \"vehicle_ota_ver\": %s,", taskConfig.vehicleOtaVer.c_str());
    OTA_LOG_D("    \"task_start_time\": %d,", taskConfig.taskStartTime);
    OTA_LOG_D("    \"task_duration\": %d,", taskConfig.taskDuration);
    OTA_LOG_D("    \"upgrade_duration\": %d,", taskConfig.upgradeDuration);
    OTA_LOG_D("    \"ota_state\": %d,", taskConfig.otaState);
    OTA_LOG_D("    \"upgrade_err_code\": %d,", taskConfig.errorCode);
    OTA_LOG_D("    \"high_volt_ecus\":[");
    for (const auto &config : taskConfig.highVoltEcuTasksInfo) {
        PrintTaskEcuConfigInfo(config);
        OTA_LOG_D("");
    }
    OTA_LOG_D("    ],");

    OTA_LOG_D("    \"low_volt_ecus\":[");
    for (const auto &config : taskConfig.lowVoltEcuTasksInfo) {
        PrintTaskEcuConfigInfo(config);
        OTA_LOG_D("");
    }
    OTA_LOG_D("    ]");
    OTA_LOG_D("}");
}

/**
 * @brief 打印支持OTA ECU配置信息
 * 
 * @param[in] config 支持OTA ECU配置信息
 * 
 * @return 无
 */
static void PrintSupportOtaEcuConfigInfo(const SupportOTAEcuConfigMap &config)
{
    OTA_LOG_D("\"fota_ecu\": [");
    for (const auto &iter : config) {
        OTA_LOG_D("    {");
        OTA_LOG_D("        \"ecu_id\": 0x%04x,", iter.first);
        OTA_LOG_D("        \"name\": %s,", iter.second.name.c_str());
        OTA_LOG_D("        \"ecu_type\": %d,", iter.second.ecuType);
        OTA_LOG_D("        \"can_port\": %d,", iter.second.protocolInfo.canPort);
        OTA_LOG_D("        \"physical_id\": 0x%04x,", iter.second.protocolInfo.physicalId);
        OTA_LOG_D("        \"function_id\": 0x%04x,", iter.second.protocolInfo.functionId);
        OTA_LOG_D("        \"response_id\": 0x%04x,", iter.second.protocolInfo.responseId);
        OTA_LOG_D("        \"power_mode\": %d", iter.second.powerMode);
        OTA_LOG_D("        \"rollback\": [ ");
        for (const auto &rollback : iter.second.rollbackInfo) {
            OTA_LOG_D("            {");
            OTA_LOG_D("                \"supplier_id\": %s,", rollback.first.c_str());
            OTA_LOG_D("                \"succ_roll_back_mode\": %d,", rollback.second.succRollbackMode);
            OTA_LOG_D("                \"failed_roll_back_mode\": %d,", rollback.second.failRollbackMode);
            OTA_LOG_D("                \"rollback_timeout\": %d", rollback.second.rollbackTimeout);
            OTA_LOG_D("            },");
        }
        OTA_LOG_D("        ]");
        OTA_LOG_D("    },");
    }
    OTA_LOG_D("]");
}

/**
 * @brief 打印OTA前置条件配置信息
 * 
 * @param[in] config OTA前置条件配置信息
 * 
 * @return 无
 */
static void PrintOtaPreconditionsConfigInfo(const OtaPreconditionsConfigInfo &config)
{
    OTA_LOG_D("OTA preconditions config info --->");
    OTA_LOG_D("{");
    OTA_LOG_D("    \"battery_voltage\": {");
    OTA_LOG_D("        \"enabled\": %s,", config.minBatteryVoltageReq.first ? "true" : "false");
    OTA_LOG_D("        \"min_voltage\": %f", config.minBatteryVoltageReq.second);
    OTA_LOG_D("    },");
    OTA_LOG_D("    \"gear_position\": {");
    OTA_LOG_D("        \"enabled\": %s,", config.gearPositionReq.first ? "true" : "false");
    OTA_LOG_D("        \"required_gear\": %d", config.gearPositionReq.second);
    OTA_LOG_D("    },");
    OTA_LOG_D("    \"vehicle_speed\": {");
    OTA_LOG_D("        \"enabled\": %s,", config.maxVehicleSpeedReq.first ? "true" : "false");
    OTA_LOG_D("        \"required_speed\": %d", config.maxVehicleSpeedReq.second);
    OTA_LOG_D("    },");
    OTA_LOG_D("    \"vcu_hv_not_ready\": {");
    OTA_LOG_D("        \"enabled\": %s,", config.vcuHvStatusNotReady.first ? "true" : "false");
    OTA_LOG_D("    },");
    OTA_LOG_D("    \"bms_real_soc\": {");
    OTA_LOG_D("        \"enabled\": %s,", config.minBmsRealSocReq.first ? "true" : "false");
    OTA_LOG_D("        \"min_soc\": %d", config.minBmsRealSocReq.second);
    OTA_LOG_D("    },");
    OTA_LOG_D("    \"required_vehicle_no_fault\": {");
    OTA_LOG_D("        \"enabled\": %s", config.vehicleFaultReq.first ? "true" : "false");
    OTA_LOG_D("    },");
    OTA_LOG_D("    \"required_plug_off\": {");
    OTA_LOG_D("        \"enabled\": %s", config.chargePlugStateReq.first ? "true" : "false");
    OTA_LOG_D("    },");
    OTA_LOG_D("    \"required_no_remote_ctrl_executing\": {");
    OTA_LOG_D("        \"enabled\": %s", config.remoteCtrlExecutingStateReq.first ? "true" : "false");
    OTA_LOG_D("    },");
    OTA_LOG_D("    \"no_obd_diagnostic_dev_conn\": {");
    OTA_LOG_D("        \"enabled\": %s", config.obdDiagDevConnStateReq.first ? "true" : "false");
    OTA_LOG_D("    },");
    OTA_LOG_D("    \"bcm_anti_theft_auth_passed\": {");
    OTA_LOG_D("        \"enabled\": %s", config.bcmAntiTheftAuthPassedReq.first ? "true" : "false");
    OTA_LOG_D("    }");
    OTA_LOG_D("}");
}

void OtaConfigInit(const std::string &configPath)
{
    OTA_LOG_D("OTA config init");
    g_configFolderPath = configPath;
    // 处理路径末尾可能有的斜杠，避免双斜杠
    std::string normalizedPath = configPath;
    if (!normalizedPath.empty() && normalizedPath.back() == '/') {
        normalizedPath.pop_back();
    }
    g_supportOtaConfigPath = normalizedPath + "/" + SUPPORT_OTA_ECU_CONFIG_FILE;
    g_otaTaskConfigPath = normalizedPath + "/" + OTA_TASK_CONFIG_FILE;
    g_otaPreocondsConfigPath = normalizedPath + "/" + OTA_PRECONDITIONS_CONFIG_FILE;
}

void OtaConfigDeinit(void)
{
    OTA_LOG_D("OTA config deinit");
    g_configFolderPath = "";
    g_supportOtaConfigPath = "";
    g_otaTaskConfigPath = "";
    g_otaPreocondsConfigPath = "";
}

int LoadSupportOtaEcuConfigInfo(SupportOTAEcuConfigMap &config)
{
    OTA_LOG_D("Load support OTA ECU config info");
    Json::Value root;
    if (GetConfigFileRootJsonValue(g_supportOtaConfigPath, root) != OTA_RET_OK) {
        OTA_LOG_E("Get %s root Json::Value failed", g_supportOtaConfigPath.c_str());
        return OTA_RET_ERR;
    }

    Json::Value ecuArray;
    if (GetArrayValueFromRoot(root, OtaEcu::KEY_FOTA_ECU, ecuArray) != OTA_RET_OK) {
        OTA_LOG_E("Get array Json::Value failed");
        return OTA_RET_ERR;
    }

    for (int i = 0; i < ecuArray.size(); ++i) {
        SupportOTAEcuConfigInfo ecuConfig;
        std::string ecuIdStr, physicalIdStr, funcIdStr, respIdStr;
        Json::Value rollbackArray;
        if (GetArrayValueFromRoot(ecuArray[i], OtaEcu::KEY_ROLLBACK, rollbackArray) != OTA_RET_OK) {
            OTA_LOG_E("Get array Json::Value failed");
            return OTA_RET_ERR;
        }

        GetStringFromJsonValue(ecuArray[i], OtaEcu::KEY_NAME, ecuConfig.name);
        GetStringFromJsonValue(ecuArray[i], OtaEcu::KEY_ECU_ID, ecuIdStr);
        GetStringFromJsonValue(ecuArray[i], OtaEcu::KEY_PHYSICAL_ID, physicalIdStr);
        GetStringFromJsonValue(ecuArray[i], OtaEcu::KEY_FUNCTION_ID, funcIdStr);
        GetStringFromJsonValue(ecuArray[i], OtaEcu::KEY_REPONSE_ID, respIdStr);
        ecuConfig.ecuId = static_cast<uint16_t>(HexStringToUint32(ecuIdStr));
        ecuConfig.protocolInfo.canPort = GetUIntFromJsonValue<uint8_t>(ecuArray[i], OtaEcu::KEY_CAN_PORT);
        ecuConfig.protocolInfo.physicalId = HexStringToUint32(physicalIdStr);
        ecuConfig.protocolInfo.functionId = HexStringToUint32(funcIdStr);
        ecuConfig.protocolInfo.responseId = HexStringToUint32(respIdStr);
        ecuConfig.powerMode = GetUIntFromJsonValue<uint8_t>(ecuArray[i], OtaEcu::KEY_POWER_MODE);
        ecuConfig.ecuType = GetUIntFromJsonValue<uint8_t>(ecuArray[i], OtaEcu::KEY_ECU_TYPE);

        for (int j = 0; j < rollbackArray.size(); ++j) {
            OtaTaskEcuRollbackInfo rollbackInfo;
            std::string supplierId;
            rollbackInfo.rollbackTimeout = GetUIntFromJsonValue<uint16_t>(rollbackArray[j], OtaEcu::KEY_ROLLBACK_TIMEOUT);
            rollbackInfo.succRollbackMode = GetUIntFromJsonValue<uint8_t>(rollbackArray[j], OtaEcu::KEY_SUCC_ROLLBACK_MODE);
            rollbackInfo.failRollbackMode = GetUIntFromJsonValue<uint8_t>(rollbackArray[j], OtaEcu::KEY_FAIL_ROLLBACK_MODE);
            GetStringFromJsonValue(rollbackArray[j], OtaEcu::KEY_SUPPLIER_ID, supplierId);
            ecuConfig.rollbackInfo.emplace(supplierId, rollbackInfo);
        }
        config.emplace(ecuConfig.ecuId, ecuConfig);
    }
    // 打印支持OTA ECU配置信息
    PrintSupportOtaEcuConfigInfo(config);
    return OTA_RET_OK;
}

int LoadOtaEcuPreconditionsConfigInfo(OtaPreconditionsConfigInfo &config)
{
    OTA_LOG_D("Load OTA ECU preconditions config info");
    Json::Value root;
    if (GetConfigFileRootJsonValue(g_otaPreocondsConfigPath, root) != OTA_RET_OK) {
        OTA_LOG_E("Get %s root Json::Value failed", g_otaPreocondsConfigPath.c_str());
        return OTA_RET_ERR;
    }

    bool enable;
    uint8_t requiredValue;
    Json::Value conditions;
    //读取battery_voltage要求条件
    if (GetValueFromRoot(root, OtaPreconditions::KEY_BATTERY_VOLT, conditions) != OTA_RET_OK) {
        config.minBatteryVoltageReq = std::make_pair(false, 12.0f);
    } else {
        enable = GetBoolFromJsonValue(conditions, OtaPreconditions::KEY_ENABLED);
        double minVoltage = GetDoubleFromJsonValue(conditions, OtaPreconditions::KEY_MIN_VOLTAGE);
        config.minBatteryVoltageReq = std::make_pair(enable, minVoltage);
    }

    conditions.clear();
    //读取gear_position要求条件
    if (GetValueFromRoot(root, OtaPreconditions::KEY_GEAR_POSITION, conditions) != OTA_RET_OK) {
        config.gearPositionReq = std::make_pair(false, 0);
    } else {
        enable = GetBoolFromJsonValue(conditions, OtaPreconditions::KEY_ENABLED);
        requiredValue = GetUIntFromJsonValue<uint16_t>(conditions, OtaPreconditions::KEY_REQUIRED_GEAR);
        config.gearPositionReq = std::make_pair(enable, requiredValue);
    }

    conditions.clear();
    //读取vehicle_speed要求条件
    if (GetValueFromRoot(root, OtaPreconditions::KEY_VEHICLE_SPEED, conditions) != OTA_RET_OK) {
        config.vehicleFaultReq = std::make_pair(false, 0);
    } else {
        enable = GetBoolFromJsonValue(conditions, OtaPreconditions::KEY_ENABLED);
        requiredValue = GetUIntFromJsonValue<uint16_t>(conditions, OtaPreconditions::KEY_REQUIRED_SPEED);
        config.vehicleFaultReq = std::make_pair(enable, requiredValue);
    }

    conditions.clear();
    //读取vcu_hv_not_ready要求条件
    if (GetValueFromRoot(root, OtaPreconditions::KEY_VCU_HV_NOT_READY, conditions) != OTA_RET_OK) {
        config.vcuHvStatusNotReady = std::make_pair(false, 0);
    } else {
        enable = GetBoolFromJsonValue(conditions, OtaPreconditions::KEY_ENABLED);
        config.vcuHvStatusNotReady = std::make_pair(enable, 0);
    }

    conditions.clear();
    //读取bms_real_soc要求条件
    if (GetValueFromRoot(root, OtaPreconditions::KEY_BMS_REAL_SOC, conditions) != OTA_RET_OK) {
        config.gearPositionReq = std::make_pair(false, 0);
    } else {
        enable = GetBoolFromJsonValue(conditions, OtaPreconditions::KEY_ENABLED);
        requiredValue = GetUIntFromJsonValue<uint16_t>(conditions, OtaPreconditions::KEY_MIN_SOC);
        config.minBmsRealSocReq = std::make_pair(enable, requiredValue);
    }

    conditions.clear();
    //读取required_vehicle_no_fault要求条件
    if (GetValueFromRoot(root, OtaPreconditions::KEY_REQUIRED_VEHICLE_NO_FAULT, conditions) != OTA_RET_OK) {
        config.vehicleFaultReq = std::make_pair(false, 0);
    } else {
        enable = GetBoolFromJsonValue(conditions, OtaPreconditions::KEY_ENABLED);
        config.vehicleFaultReq = std::make_pair(enable, 0);
    }

    conditions.clear();
    //读取required_plug_off要求条件
    if (GetValueFromRoot(root, OtaPreconditions::KEY_REQUIRED_PLUG_OFF, conditions) != OTA_RET_OK) {
        config.chargePlugStateReq = std::make_pair(false, 0);
    } else {
        enable = GetBoolFromJsonValue(conditions, OtaPreconditions::KEY_ENABLED);
        config.chargePlugStateReq = std::make_pair(enable, 0);
    }

    conditions.clear();
    //读取required_no_remote_ctrl_executing要求条件
    if (GetValueFromRoot(root, OtaPreconditions::KEY_REQUIRED_NO_REMOTE_CTRL_EXEC, conditions) != OTA_RET_OK) {
        config.remoteCtrlExecutingStateReq = std::make_pair(false, 0);
    } else {
        enable = GetBoolFromJsonValue(conditions, OtaPreconditions::KEY_ENABLED);
        config.remoteCtrlExecutingStateReq = std::make_pair(enable, 0);
    }

    conditions.clear();
    //读取no_obd_diagnostic_dev_conn要求条件
    if (GetValueFromRoot(root, OtaPreconditions::KEY_NO_OBD_DIAG_DEV_CONN, conditions) != OTA_RET_OK) {
        config.obdDiagDevConnStateReq = std::make_pair(false, 0);
    } else {
        enable = GetBoolFromJsonValue(conditions, OtaPreconditions::KEY_ENABLED);
        config.obdDiagDevConnStateReq = std::make_pair(enable, 0);
    }

    conditions.clear();
    //读取bcm_anti_theft_auth_passed要求条件
    if (GetValueFromRoot(root, OtaPreconditions::KEY_BCM_ANTI_THEFT_AUTH_PASSED, conditions) != OTA_RET_OK) {
        config.bcmAntiTheftAuthPassedReq = std::make_pair(false, 0);
    } else {
        enable = GetBoolFromJsonValue(conditions, OtaPreconditions::KEY_ENABLED);
        config.bcmAntiTheftAuthPassedReq = std::make_pair(enable, 1);
    }
    OTA_LOG_D("Load OTA ECU preconditions config info success");
    // 打印OTA前置条件配置信息
    PrintOtaPreconditionsConfigInfo(config);
    return OTA_RET_OK;
}

int LoadOtaTaskConfigInfo(OtaTaskConfigInfo &config)
{
    OTA_LOG_D("Load Ota task config info");
    std::unique_lock<std::mutex> locker(g_otaTaskConfigMutex);
    Json::Value root;
    if (GetConfigFileRootJsonValue(g_otaTaskConfigPath, root) != OTA_RET_OK) {
        locker.unlock();
        OTA_LOG_E("Get %s root Json::Value failed", g_otaTaskConfigPath.c_str());
        ResetOtaTaskConfig();
        return OTA_RET_ERR;
    }

    GetStringFromJsonValue(root, OtaTask::KEY_TASK_ID, config.taskId);
    GetStringFromJsonValue(root, OtaTask::KEY_TASK_NAME, config.taskName);
    GetStringFromJsonValue(root, OtaTask::KEY_TASK_INFO, config.taskInfo);
    GetStringFromJsonValue(root, OtaTask::KEY_LOG_UPLOAD_ADDR, config.logUploadAddr);
    GetStringFromJsonValue(root, OtaTask::KEY_VEHICLE_OTA_VER, config.vehicleOtaVer);
    config.isUploadLog = GetUIntFromJsonValue<uint8_t>(root, OtaTask::KEY_IS_UPLOAD_LOG);
    config.taskDuration = GetUIntFromJsonValue<uint32_t>(root, OtaTask::KEY_TASK_DURATION);
    config.upgradeDuration = GetUIntFromJsonValue<uint32_t>(root, OtaTask::KEY_UPGRADE_DURATION);
    config.upgradeMode = GetUIntFromJsonValue<uint8_t>(root, OtaTask::KEY_UPGRADE_MODE);
    config.otaState = GetUIntFromJsonValue<uint8_t>(root, OtaTask::KEY_OTA_STATE);
    config.errorCode = GetUIntFromJsonValue<uint8_t>(root, OtaTask::KEY_UPGRADE_ERR_CODE);
    config.taskStartTime = GetUIntFromJsonValue<uint32_t>(root, OtaTask::KEY_TASK_START_TIME);

    if (GetOtaTaskEcuConfigInfo(root, OtaTask::KEY_LOW_VOLT_ECUS, config.lowVoltEcuTasksInfo) != OTA_RET_OK ||
        GetOtaTaskEcuConfigInfo(root, OtaTask::KEY_HIGH_VOLT_ECUS, config.highVoltEcuTasksInfo) != OTA_RET_OK) {
        return OTA_RET_ERR;
    }
    // 打印OTA任务信息
    PrintOtaTaskConfigInfo(config);
    return OTA_RET_OK;
}

void UpdateOtaTaskConfig(const OtaTaskConfigInfo &taskConfig)
{
    OTA_LOG_D("Update Ota task config info");
    // 打印OTA任务信息
    PrintOtaTaskConfigInfo(taskConfig);
    std::lock_guard<std::mutex> locker(g_otaTaskConfigMutex);
    Json::Value root;
    if (GetConfigFileRootJsonValue(g_otaTaskConfigPath, root) != OTA_RET_OK) {
        OTA_LOG_E("Get %s root Json::Value failed", g_otaTaskConfigPath.c_str());
        return;
    }

    root[OtaTask::KEY_TASK_ID] = taskConfig.taskId;
    root[OtaTask::KEY_TASK_NAME] = taskConfig.taskName;
    root[OtaTask::KEY_TASK_INFO] = taskConfig.taskInfo;
    root[OtaTask::KEY_UPGRADE_MODE] = taskConfig.upgradeMode;
    root[OtaTask::KEY_IS_UPLOAD_LOG] = taskConfig.isUploadLog;
    root[OtaTask::KEY_LOG_UPLOAD_ADDR] = taskConfig.logUploadAddr;
    root[OtaTask::KEY_VEHICLE_OTA_VER] = taskConfig.vehicleOtaVer;
    root[OtaTask::KEY_TASK_DURATION] = taskConfig.taskDuration;
    root[OtaTask::KEY_UPGRADE_DURATION] = taskConfig.upgradeDuration;
    root[OtaTask::KEY_TASK_START_TIME] = taskConfig.taskStartTime;
    root[OtaTask::KEY_OTA_STATE] = taskConfig.otaState;
    root[OtaTask::KEY_UPGRADE_ERR_CODE] = taskConfig.errorCode;

    const auto setTaskInfo = [&](const OtaTaskEcuConfigInfo &config, Json::Value &taskInfo) {
        taskInfo[OtaTask::KEY_ECU_ID] = config.ecuId;
        taskInfo[OtaTask::KEY_ECU_TYPE] = config.ecuType;
        taskInfo[OtaTask::KEY_ECU_NAME] = config.ecuName;
        taskInfo[OtaTask::KEY_SUPPLIER_ID] = config.sullpierId;
        taskInfo[OtaTask::KEY_PART_NUM] = config.partNum;
        taskInfo[OtaTask::KEY_CAN_PORT] = config.protocolInfo.canPort;
        taskInfo[OtaTask::KEY_UPGRADE_ORDER] = config.upgradeOrder;
        taskInfo[OtaTask::KEY_RB_TIMEOUT] = config.rollbackInfo.rollbackTimeout;
        taskInfo[OtaTask::KEY_SUCC_RB_MODE] = config.rollbackInfo.succRollbackMode;
        taskInfo[OtaTask::KEY_FAIL_RB_MODE] = config.rollbackInfo.failRollbackMode;
        taskInfo[OtaTask::KEY_PHYSICAL_ID] = config.protocolInfo.physicalId;
        taskInfo[OtaTask::KEY_FUNCTION_ID] = config.protocolInfo.functionId;
        taskInfo[OtaTask::KEY_RESPONSE_ID] = config.protocolInfo.responseId;
        taskInfo[OtaTask::KEY_DOWNLOAD_FOLDER] = config.dlFolder;
        taskInfo[OtaTask::KEY_UNZIP_FOLDER] = config.unzipFolder;
        Json::Value upgradeInfo;
        upgradeInfo[OtaTask::KEY_PACK_SIZE] = config.upgradeFile.downloadSize;
        upgradeInfo[OtaTask::KEY_PACK_URL] = config.upgradeFile.downloadUrl;
        upgradeInfo[OtaTask::KEY_PACK_MD5] = config.upgradeFile.downloadMD5;
        upgradeInfo[OtaTask::KEY_PACK_DL_PATH] = config.upgradeFile.downloadPath;
        upgradeInfo[OtaTask::KEY_DECOMPESS_PATH] = config.upgradeFile.decompressFolder;
        upgradeInfo[OtaTask::KEY_DOWNLOAD_STATE] = config.upgradeFile.downloadState;
        upgradeInfo[OtaTask::KEY_UPGRADE_STATE] = config.upgradeState;
        taskInfo[OtaTask::KEY_UPGRADE] = upgradeInfo;
        Json::Value backupInfo;
        backupInfo[OtaTask::KEY_PACK_SIZE] = config.backupFile.downloadSize;
        backupInfo[OtaTask::KEY_PACK_URL] = config.backupFile.downloadUrl;
        backupInfo[OtaTask::KEY_PACK_MD5] = config.backupFile.downloadMD5;
        backupInfo[OtaTask::KEY_PACK_DL_PATH] = config.backupFile.downloadPath;
        backupInfo[OtaTask::KEY_DECOMPESS_PATH] = config.backupFile.decompressFolder;
        backupInfo[OtaTask::KEY_DOWNLOAD_STATE] = config.backupFile.downloadState;
        backupInfo[OtaTask::KEY_IS_LOCAL_BACKUP] = config.isLocalBackup;
        backupInfo[OtaTask::KEY_ROLL_BACK_STATE] = config.rollbackState;
        taskInfo[OtaTask::KEY_BACKUP] = backupInfo;
        taskInfo[OtaTask::KEY_HW_VER] = config.hwVer;
        taskInfo[OtaTask::KEY_TARGET_VER] = config.targetVer;
        taskInfo[OtaTask::KEY_CURRENT_VER] = config.supportVer;
    };

    //更新高压ECU升级任务信息
    Json::Value highVoltEcuArray(Json::arrayValue);
    for (const auto &config : taskConfig.highVoltEcuTasksInfo) {
        Json::Value taskInfo;
        setTaskInfo(config, taskInfo);
        highVoltEcuArray.append(taskInfo);
    }
    root[OtaTask::KEY_HIGH_VOLT_ECUS] = highVoltEcuArray;

    //更新低压ECU升级任务信息
    Json::Value lowVoltEcuArray(Json::arrayValue);
    for (const auto &config : taskConfig.lowVoltEcuTasksInfo) {
        Json::Value taskInfo;
        setTaskInfo(config, taskInfo);
        lowVoltEcuArray.append(taskInfo);
    }
    root[OtaTask::KEY_LOW_VOLT_ECUS] = lowVoltEcuArray;

    //将JSON数据写入配置文件中
    std::ofstream os(g_otaTaskConfigPath, std::ios::binary | std::ios::out);
    if (!os.is_open()) {
        OTA_LOG_E("Open %s failed, error[%d]: %s\n", g_otaTaskConfigPath.c_str(), errno, strerror(errno));
        return;
    }
    Json::StyledStreamWriter writer;
    writer.write(os, root);
    os.close();
}

void ResetOtaTaskConfig(void)
{
    OTA_LOG_D("Reset ota task config");
    std::lock_guard<std::mutex> locker(g_otaTaskConfigMutex);
    std::ofstream os(g_otaTaskConfigPath, std::ios::binary | std::ios::out);
    if (!os.is_open()) {
        OTA_LOG_E("Open %s failed, error[%d]: %s", g_otaTaskConfigPath.c_str(), errno, strerror(errno));
        return;
    }

    Json::Value root;
    root[OtaTask::KEY_TASK_ID] = "";
    root[OtaTask::KEY_TASK_NAME] = "";
    root[OtaTask::KEY_TASK_INFO] = "";
    root[OtaTask::KEY_UPGRADE_MODE] = 0;
    root[OtaTask::KEY_IS_UPLOAD_LOG] = 0;
    root[OtaTask::KEY_LOG_UPLOAD_ADDR] = "";
    root[OtaTask::KEY_VEHICLE_OTA_VER] = "";
    root[OtaTask::KEY_TASK_DURATION] = 0;
    root[OtaTask::KEY_UPGRADE_DURATION] = 0;
    root[OtaTask::KEY_TASK_START_TIME] = 0;
    root[OtaTask::KEY_OTA_STATE] = 0;
    root[OtaTask::KEY_HIGH_VOLT_ECUS] = Json::arrayValue;
    root[OtaTask::KEY_LOW_VOLT_ECUS] = Json::arrayValue;

    Json::StyledStreamWriter writer;
    writer.write(os, root);
    os.close();
}

int ExtractOtaTaskPolicyInfo(const StOtaTaskPolicyInfo *policyInfo, const SupportOTAEcuConfigMap &supportConfig,
                             OtaTaskConfigInfo &taskConfig)
{
    if (policyInfo == nullptr) {
        OTA_LOG_E("OTA task policy info is nullptr");
        return OTA_RET_ERR;
    }

    if (policyInfo->taskId == nullptr || strlen(policyInfo->taskId) == 0) {
        OTA_LOG_E("OTA task id is nullptr or empty");
        return OTA_RET_ERR;
    }

    if (policyInfo->taskEcuInfo == nullptr) {
        OTA_LOG_E("OTA task ecu info is nullptr");
        return OTA_RET_ERR;
    }

    OTA_LOG_D("Extract OTA task policy info -------------------> START");
    taskConfig.Clear();
    taskConfig.taskId = policyInfo->taskId;
    taskConfig.taskName = policyInfo->taskName == nullptr ? "" : policyInfo->taskName;
    taskConfig.taskInfo = policyInfo->taskInfo == nullptr ? "" : policyInfo->taskInfo;
    taskConfig.upgradeMode = policyInfo->upgradeMode;
    taskConfig.logUploadAddr = policyInfo->logUploadAddr == nullptr ? "" : policyInfo->logUploadAddr;
    taskConfig.vehicleOtaVer = policyInfo->vehicleOtaVer == nullptr ? "" : policyInfo->vehicleOtaVer;
    taskConfig.isUploadLog = taskConfig.logUploadAddr.empty() ? 0 : policyInfo->isUploadLog;
    taskConfig.taskDuration = static_cast<uint32_t>(policyInfo->taskDuration * 60);
    taskConfig.upgradeDuration = static_cast<uint32_t>(policyInfo->upgradeDuration * 60);
    // 若全任务持续时间为0，则任务开始时间默认为0，实际以刷写开始时间为准；否则为任务触发时间或者当前时间；
    if (taskConfig.taskDuration == 0) {
        taskConfig.taskStartTime = 0;
    } else {
        taskConfig.taskStartTime = (policyInfo->taskStartTime == 0) ? time(nullptr) : policyInfo->taskStartTime;
    }
    taskConfig.otaState = 0;
    // 校验每次OTA升级指令必须只包含一个ECU，由TSP指定升级哪个ECU
    if (policyInfo->taskCount != 1) {
        OTA_LOG_E("OTA task must contain exactly one ECU, but got %d", policyInfo->taskCount);
        return OTA_RET_ERR;
    }
    for (int i = 0; i < policyInfo->taskCount; ++i) {
        OtaTaskEcuConfigInfo config;
        config.ecuId = policyInfo->taskEcuInfo[i].ecuId;
        config.upgradeOrder = policyInfo->taskEcuInfo[i].upgradeOrder;
        config.upgradeFile.downloadSize = policyInfo->taskEcuInfo[i].upgradeFileInfo.fileSize;
        config.upgradeFile.downloadUrl = policyInfo->taskEcuInfo[i].upgradeFileInfo.downloadUrl == nullptr
                                             ? ""
                                             : policyInfo->taskEcuInfo[i].upgradeFileInfo.downloadUrl;
        config.upgradeFile.downloadMD5 = policyInfo->taskEcuInfo[i].upgradeFileInfo.md5 == nullptr
                                             ? ""
                                             : policyInfo->taskEcuInfo[i].upgradeFileInfo.md5;
        config.upgradeFile.downloadState = OTA_FILE_NOT_DOWNLOAD;
        config.backupFile.downloadSize = policyInfo->taskEcuInfo[i].backupFileInfo.fileSize;
        config.backupFile.downloadUrl = policyInfo->taskEcuInfo[i].backupFileInfo.downloadUrl == nullptr
                                            ? ""
                                            : policyInfo->taskEcuInfo[i].backupFileInfo.downloadUrl;
        config.backupFile.downloadMD5 = policyInfo->taskEcuInfo[i].backupFileInfo.md5 == nullptr
                                            ? ""
                                            : policyInfo->taskEcuInfo[i].backupFileInfo.md5;
        config.backupFile.downloadState = OTA_FILE_NOT_DOWNLOAD;
        config.isLocalBackup = 0;
        config.hwVer =
            policyInfo->taskEcuInfo[i].checkVer.hwVer == nullptr ? "" : policyInfo->taskEcuInfo[i].checkVer.hwVer;
        StringToUpper(config.hwVer);
        config.targetVer =
            policyInfo->taskEcuInfo[i].checkVer.swVer == nullptr ? "" : policyInfo->taskEcuInfo[i].checkVer.swVer;
        StringToUpper(config.targetVer);
        config.supportVer =
            policyInfo->taskEcuInfo[i].supportVer.swVer == nullptr ? "" : policyInfo->taskEcuInfo[i].supportVer.swVer;
        StringToUpper(config.supportVer);
        config.sullpierId =
            policyInfo->taskEcuInfo[i].supplierId == nullptr ? "" : policyInfo->taskEcuInfo[i].supplierId;
        StringToUpper(config.sullpierId);
        config.partNum = policyInfo->taskEcuInfo[i].supportVer.partNum == nullptr
                             ? ""
                             : policyInfo->taskEcuInfo[i].supportVer.partNum;
        StringToUpper(config.partNum);
        config.upgradeState = OTA_ECU_NOT_UPGRADE;
        config.rollbackState = E_ROLLBACK_STATUS_NOT_ROLLBACK;
        // 获取ECU刷写协议及回滚信息
        GetEcuFlashProtocolAndRollbackInfo(supportConfig, policyInfo->taskEcuInfo[i].flashProtocol, config);

        InsertOtaTaskEcuConfigIntoVec(supportConfig, config, taskConfig);
    }

    //将提取出的策略信息写入配置文件中
    UpdateOtaTaskConfig(taskConfig);
    OTA_LOG_D("Extract OTA task policy info -------------------> END");
    return OTA_RET_OK;
}