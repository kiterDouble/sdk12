#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "srTspFWMqttClient.h"
#include "terminal_management_api.h"
#include "terminal_management_data_monitor.h"
#include "terminal_management_data_report.h"

#include "sr_log.h"
#include "sr_ipc.h"

using namespace std;

static constexpr const char *DATA_SAVE_FOLDER = "/mnt/sdcard/terminal_manager/data/";
static constexpr const char *TASK_CONFIG_FOLDER = "/mnt/sdcard/terminal_manager/config/";
static constexpr const char *CERT_FOLDER = "/mnt/sdcard/terminal_manager/cert";
static constexpr const char *DOWNLOAD_FOLDER = "/mnt/sdcard/terminal_manager/download/";
static constexpr const char *UNZIP_FOLDER = "/mnt/sdcard/terminal_manager/download/";
static StBaseInfoCallbacks_t g_baseInfo = {0};

/**
 * @brief 获取设备 SN 编号（序列号）
 *
 * 在初始化或上报阶段会调用本函数，用于获取设备的唯一序列号。
 *
 * @param[out] buffer 输出缓冲区，用于存放 SN 字符串
 * @param[in] len 缓冲区长度，必须大于实际 SN 长度
 * @return RET_OK 成功，RET_ERR 缓冲区不足或参数无效
 */
static int QuerySnCallback(char *buffer, int len)
{
    ParamInfo_t paramInfo = {.id = E_ParamId_SN};
    if (GetParam(&paramInfo) < 0)
    {
        LOG_Debug("Get SN error");
        return RET_ERR;
    }
    memcpy(buffer, paramInfo.value, strlen((char *)paramInfo.value));
    return RET_OK;
}

/**
 * @brief 获取设备的 SIM 卡号（MSISDN）
 */
static int QuerySimCallback(char *buffer, int len)
{
    ParamInfo_t paramInfo = {.id = E_ParamId_IMSI};
    if (GetParam(&paramInfo) < 0)
    {
        LOG_Debug("Get MSISDN error");
        return RET_ERR;
    }
    memcpy(buffer, paramInfo.value, strlen((char *)paramInfo.value));
    return RET_OK;
}

/**
 * @brief 获取设备 IMEI 编号（International Mobile Equipment Identity）
 */
static int QueryImeiCallback(char *buffer, int len)
{
    ParamInfo_t paramInfo = {.id = E_ParamId_IMEI};
    if (GetParam(&paramInfo) < 0)
    {
        LOG_Debug("Get IMEI error");
        return RET_ERR;
    }
    memcpy(buffer, paramInfo.value, strlen((char *)paramInfo.value));
    return RET_OK;
}

/**
 * @brief 获取 ICCID 编号（集成电路卡唯一识别码）
 */
static int QueryIccidCallback(char *buffer, int len)
{
    ParamInfo_t paramInfo = {.id = E_ParamId_ICCID};
    if (GetParam(&paramInfo) < 0)
    {
        LOG_Debug("Get ICCID error");
        return RET_ERR;
    }
    memcpy(buffer, paramInfo.value, strlen((char *)paramInfo.value));
    return RET_OK;
}

/**
 * @brief 获取当前项目名称（用于平台识别 TBOX 所属项目）
 */
static int QueryProjectNameCallback(char *buffer, int len)
{
    const char *projectName = "Tonly";
    size_t projectNameLen = strlen(projectName) + 1;
    if (!buffer || len < projectNameLen)
    {
        return RET_ERR;
    }
    strcpy(buffer, projectName);
    return RET_OK;
}

/**
 * @brief 获取车辆 VIN（Vehicle Identification Number）
 */
static int QueryVinCallback(char *buffer, int len)
{
    ParamInfo_t paramInfo = {.id = E_ParamId_VIN};
    if (GetParam(&paramInfo) < 0)
    {
        LOG_Debug("Get VIN error");
        return RET_ERR;
    }
    memcpy(buffer, paramInfo.value, strlen((char *)paramInfo.value));
    return RET_OK;
}

/**
 * @brief 获取设备硬件版本号（如主板版本）
 */
static int QueryHwVersionCallback(char *buffer, int len)
{
    ParamInfo_t paramInfo = {.id = E_ParamId_HW_Version};
    if (GetParam(&paramInfo) < 0)
    {
        LOG_Debug("Get srHwVersion error");
        return RET_ERR;
    }
    memcpy(buffer, paramInfo.value, strlen((char *)paramInfo.value));
    return RET_OK;
}

/**
 * @name QuerySwVersionMainCallback
 * @brief 获取主软件版本
 */
static int QuerySwVersionMainCallback(char *buffer, int len)
{
    ParamInfo_t paramInfo = {.id = E_ParamId_SW_Version_External};
    if (GetParam(&paramInfo) < 0)
    {
        LOG_Debug("Get SwVersion error");
        return RET_ERR;
    }
    memcpy(buffer, paramInfo.value, strlen((char *)paramInfo.value));
    return RET_OK;
}

/**
 * @name QuerySwVersionSecondaryCallback
 * @brief 获取次软件版本
 */
static int QuerySwVersionSecondaryCallback(char *buffer, int len)
{
    ParamInfo_t paramInfo = {.id = E_ParamId_SW_Version};
    if (GetParam(&paramInfo) < 0)
    {
        LOG_Debug("Get srSwVersion error");
        return RET_ERR;
    }
    memcpy(buffer, paramInfo.value, strlen((char *)paramInfo.value));
    return RET_OK;
}

/**
 * @name QuerySwVersionConfigurationCallback
 * @brief 获取配置软件版本
 */
static int QuerySwVersionConfigurationCallback(char *buffer, int len)
{
    const char *swVersionConfiguration = "SwVersionConfiguration123456";
    size_t swVersionConfigurationLen = strlen(swVersionConfiguration) + 1;
    if (len < swVersionConfigurationLen)
    {
        return RET_ERR;
    }
    strcpy(buffer, swVersionConfiguration);
    return RET_OK;
}

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
    int error = -1;
    LOG_INIT("terminalManagerApp");
    LOG_Debug("terminal_manager APP start !");
    StartQueueMsgRcvThread("monitor_thread"); // IPC初始化函数

    printf("xxx========== [SDK_DEMO] terminal_manager APP start ! ==========\n");

    // ---------------------------------------------------------------
    // Step 1: 配置地址初始化
    // 包括配置加载、证书加载、存储路径准备等
    // ---------------------------------------------------------------
    error = ConfigAddressInit(DATA_SAVE_FOLDER,
                              TASK_CONFIG_FOLDER,
                              CERT_FOLDER,
                              DOWNLOAD_FOLDER,
                              UNZIP_FOLDER);
    if (error != RET_OK)
    {
        printf("[SDK_DEMO] ConfigAddressInit failed (error=%d)\n", error);
        return -1;
    }
    printf("[SDK_DEMO] ConfigAddressInit success\n");

    // ---------------------------------------------------------------
    // Step 2: 注册设备基础信息接口
    // ---------------------------------------------------------------
    g_baseInfo.getSn = QuerySnCallback;                                         ///< 设备 SN
    g_baseInfo.getSim = QuerySimCallback;                                       ///< SIM 卡号
    g_baseInfo.getImei = QueryImeiCallback;                                     ///< IMEI 编号
    g_baseInfo.getIccid = QueryIccidCallback;                                   ///< ICCID 编号
    g_baseInfo.getProjectName = QueryProjectNameCallback;                       ///< 项目名
    g_baseInfo.getVin = QueryVinCallback;                                       ///< VIN 码
    g_baseInfo.getHwVersion = QueryHwVersionCallback;                           ///< 硬件版本
    g_baseInfo.getSwVersionMain = QuerySwVersionMainCallback;                   ///< 主软件版本
    g_baseInfo.getSwVersionSecondary = QuerySwVersionSecondaryCallback;         ///< 次软件版本
    g_baseInfo.getSwVersionConfiguration = QuerySwVersionConfigurationCallback; ///< 配置版本

    if (RegisterQueryBaseInfoCallback(&g_baseInfo) != RET_OK)
    {
        printf("[SDK_DEMO] Register base info callback failed\n");
        return -1;
    }

    // ---------------------------------------------------------------
    // Step 3: 初始化原始数据回调及资源数据的采集与更新
    // ---------------------------------------------------------------
    InitTriggerEventCallback();
    InitSignalIpcCallback();

    // ---------------------------------------------------------------
    // Step 4: 创建上行数据监控组包线程
    // ---------------------------------------------------------------
    if (DataPackagingStart() != RET_OK)
    {
        printf("[SDK_DEMO] DataPackagingStart failed\n");
        return -2;
    }
    printf("[SDK_DEMO] DataPackagingStart success\n");

    // ---------------------------------------------------------------
    // Step 5: 启动 平台状态机
    // ---------------------------------------------------------------
    if (TspClientStart() != RET_OK)
    {
        printf("[SDK_DEMO] TspClientStart failed\n");
        return -3;
    }
    printf("[SDK_DEMO] TspClientStart success\n");

    // ---------------------------------------------------------------
    // Step 6: 创建上下行数据处理线程
    // ---------------------------------------------------------------
    if (DataProcessingStart() != RET_OK)
    {
        printf("[SDK_DEMO] DataProcessingStart failed\n");
        return -2;
    }
    printf("[SDK_DEMO] DataProcessingStart success\n");

    // ---------------------------------------------------------------
    // Step 7: 主线程维持运行，通常可用于设备业务层状态上报、
    // 故障检测、Watchdog 喂狗、保活逻辑等；此处仅用于保活演示
    // ---------------------------------------------------------------
    while (1)
    {
        sleep(25);
        // 可在此添加调试打印或接口交互代码
    }

    if (ServiceStop() != RET_OK)
    {
        printf("ServiceStop failed.\n");
        return -4;
    }
    printf("[SDK_DEMO] ServiceStop success\n");

    return 0;
}
