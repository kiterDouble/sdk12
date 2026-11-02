/* ========================================================================== */
/*  文件名称  : main.c                                                         */
/*  模块名称  : UDS 网关 - 程序入口                                            */
/*  版本      : v1.0.1                                                         */
/*  作者      : <SIRUN HF / Jaxsen>                                           */
/*  日期      : 2025-09-09                                                     */
/* -------------------------------------------------------------------------- */
/*  摘要说明  :                                                                */
/*    - 初始化网关配置                                                         */
/*    - 注册回调：当 MCU 服务收到下行诊断请求时，转调 UdsGatewayOnMcuMessage     */
/*    - 运行 GLib 主循环，等待信号退出                                          */
/*    - 启动阶段与异常处加入 sr_log.h 日志（正常流程不刷日志）                  */
/* ========================================================================== */

#include <glib.h>
#include <string.h>
#ifdef G_OS_UNIX
#include <glib-unix.h>
#endif

#include "pm_server_api.h"
#include "modem_ipc_msg.h"
#include "gnss_ipc_msg.h"
#include "queue_msg.h"
#include "sirunMdlSafety.h"
#include "udsGateway.h"
#include "didRegistry.h"
#include "mcu_api.h"
#include "sr_log.h"  /* 日志：仅启动与异常打印 */
#include "err_code.h"
#include "appRpcType.h"
#include "canSignal.h"
#include "udsCallbackEntry.h"
#include "dtcRpcApi.h"
#include "fty.h"
#include "srTimer.h"
#include "dbDriver.h"
#include "dbBackupService.h"

/* ========================== 可配置宏（可由编译器 -D 覆盖） ================== */
#ifndef DTC_MSG_QUEUE_NAME
#define DTC_MSG_QUEUE_NAME        "dtc_msg_queue"
#endif

/* ---------------- MCU 服务事件派发（回调中不刷日志，避免频繁输出） ---------- */

static UdsGatewayHandle_t gGw;
static timer_t g_tboxRebootTimer;
static void OnMcuServiceMessage(const McuMesg_t *msg)
{
    (void)UdsGatewayOnMcuMessage(&gGw, msg);
}

void McuSrvReadDtcCallback(const uint8_t *data, const uint16_t len, void* userdata)
{
    (void)userdata;

    /* 异常保护：长度越界时记录错误并丢弃本帧，避免栈溢出 */
    if (len > 1024U)
    {
        LOG_Error("DTC callback payload too long: %u (>1024)", (unsigned)len);
        return;
    }

    uint8_t dtcData[1024U] = {0};
    (void)memcpy(dtcData, data, len);

    McuMesg_t msg = {
        .type       = (SYSQueMsgType_e)APP_RPC_QUE_TYPE_MCU_DTC_MCU_TO_MPU,
        .payload    = dtcData,
        .payloadLen = len,
    };

    OnMcuServiceMessage(&msg);
}

/**
 * @brief   RPC 应用层 DTC 回调函数
 *
 * @details 该回调函数用于处理 DTC（Diagnostic Trouble Code，诊断故障码）
 *          子命令请求，包括 DID 读写两类操作。
 *          - 当 subcmd == RPC_APP_DTC_SUBCMD_READ_DID_E 时：解析 DID 读请求，
 *            调用 UDS DID 读取接口，组装并返回响应。
 *          - 当 subcmd == RPC_APP_DTC_SUBCMD_WRITE_DID_E 时：解析 DID 写请求，
 *            调用 UDS DID 写接口，组装并返回响应。
 *          - 其它 subcmd 值：返回错误。
 *
 * @param[in]   subcmd    子命令类型（读 DID / 写 DID / 其它）
 * @param[in]   reqData   输入请求数据指针（上层透传的缓冲区，需解析）
 * @param[in]   reqLen    输入请求数据长度（字节数）
 * @param[out]  respData  输出响应数据指针（回填响应结构体内容）
 * @param[out]  respLen   输出响应数据长度（字节数）
 * @param[in]   userdata  用户自定义上下文指针（未使用，可为 NULL）
 *
 * @return  无返回值。通过 respData / respLen 返回响应。
 *
 * @note
 *  1. reqData、respData 在调用方必须保证非空、有效。
 *  2. respData 缓冲区应保证足够容纳最大响应结构体。
 *  3. UdsDidRead/UdsDidWrite 为下层 UDS DID 访问接口。
 */
static void RpcAppDtcCallBack(const uint16_t subcmd,
                              const uint8_t *const reqData,
                              const uint16_t reqLen,
                              uint8_t *respData,
                              uint16_t *respLen,
                              void* userdata)
{
    if (RPC_APP_DTC_SUBCMD_READ_DID_E == subcmd)
    {
        /* -------------------- 处理 DID 读请求 -------------------- */
        if (NULL == reqData)
        {
            /* 安全检查：请求负载为空 */
            LOG_Error("DTC callback payload is NULL");
            return;
        }

        /* 解析请求结构体 */
        RpcAppDtcDidReadReq_t msgreq;
        (void)memset(&msgreq, 0, sizeof(RpcAppDtcDidReadReq_t));
        (void)memmove(&msgreq, reqData, reqLen);

        /* 准备响应结构体 */
        RpcAppDtcDidReadResp_t msgresp;
        (void)memset(&msgresp, 0, sizeof(RpcAppDtcDidReadResp_t));
        msgresp.value.len = sizeof(msgresp.value.data);

        /* 调用 UDS 层函数读取 DID */
        msgresp.nrc = UdsDidRead(msgreq.did, msgresp.value.data, &msgresp.value.len);

        /* 将响应数据拷贝到输出缓冲区 */
        (void)memcpy(respData, &msgresp, sizeof(RpcAppDtcDidReadResp_t));
        *respLen = sizeof(RpcAppDtcDidReadResp_t);
    }
    else if (RPC_APP_DTC_SUBCMD_WRITE_DID_E == subcmd)
    {
        /* -------------------- 处理 DID 写请求 -------------------- */
        RpcAppDtcDidWriteReq_t msgreq;
        (void)memset(&msgreq, 0, sizeof(RpcAppDtcDidWriteReq_t));
        (void)memmove(&msgreq, reqData, reqLen);

        RpcAppDtcDidWriteResp_t msgresp;
        (void)memset(&msgresp, 0, sizeof(RpcAppDtcDidWriteResp_t));

        /* 调用 UDS 层函数写 DID */
        msgresp.nrc = UdsDidWrite(msgreq.did, msgreq.value.data, msgreq.value.len);

        (void)memcpy(respData, &msgresp, sizeof(RpcAppDtcDidWriteResp_t));
        *respLen = sizeof(RpcAppDtcDidWriteResp_t);
    }
    else
    {
        /* -------------------- 处理非法子命令 -------------------- */
        LOG_Error("DTC callback subcmd error: %u", (unsigned)subcmd);

        /* 返回一个字节的错误标志 */
        *respData = RET_ERR;
        *respLen = sizeof(uint8_t);
    }
}

// #define     MCU_RESET_MPU_E            (0U) // 默认复位MPU
// #define     MCU_RESET_MCU_E            (1U) // 默认复位MCU, MPU 会重新启动
// #define     MPU_REQ_MSU_RESET_CMD_LEN  (1U)
// static void RebootTimerCallback(union sigval val)
// {
//     (void)val;
//     LOG_Debug("RebootTimerCallback, send reboot signal to mcu");

//     McuMesg_t mcumesg = { 0 };

//     uint8_t* reqData = (uint8_t *)g_malloc(MPU_REQ_MSU_RESET_CMD_LEN);
//     reqData[0] = (uint8_t)MCU_RESET_MCU_E;
//     mcumesg.type = APP_RPC_QUE_TYPE_MCU_RESET_MPU_TO_MCU;
//     mcumesg.payload = reqData;
//     mcumesg.payloadLen = MPU_REQ_MSU_RESET_CMD_LEN;
//     g_free(reqData);
//     (void)McuSrvSendMesg(&mcumesg);
// }
#if 0
/* -------------------------------------------------------------------------- */
/*  函数名称  : LoadAndApplyEthConfigFromIni                                  */
/*  功能描述  : 从配置文件读取以太网配置并写入 DID_FUNCTION_CONFIG              */
/*              同时设置全局变量 g_isEthernetVersion 供 fty.c 使用            */
/*  参数说明  :                                                                */
/*    configPath [in]  配置文件路径（如 "/oemapp/sr_config/ethConfig.ini"）    */
/*  返回值    : 0=成功；-1=失败                                                */
/*  执行步骤  :                                                                */
/*    STEP1: 使用 GKeyFile 加载 INI 配置文件；                                  */
/*    STEP2: 读取 [default] 节的 eth 参数（1=以太网，0=非以太网）；              */
/*    STEP3: 设置全局变量 g_isEthernetVersion；                                */
/*    STEP4: 构建 64 字节的 DID_FUNCTION_CONFIG 配置缓冲区；                    */
/*    STEP5: 调用 UdsDidWrite 写入配置到 DID_FUNCTION_CONFIG；                 */
/*    STEP6: 返回执行结果。                                                    */
/* -------------------------------------------------------------------------- */
static int LoadAndApplyEthConfigFromIni(const char *configPath)
{
    GKeyFile *keyFile = NULL;
    GError *error = NULL;
    int ret = -1;

    if (configPath == NULL)
    {
        LOG_Error("LoadAndApplyEthConfigFromIni: configPath is NULL");
        return -1;
    }

    keyFile = g_key_file_new();
    if (!keyFile)
    {
        LOG_Error("Failed to create key file for %s", configPath);
        return -1;
    }

    if (!g_key_file_load_from_file(keyFile, configPath, G_KEY_FILE_NONE, &error))
    {
        LOG_Error("Failed to load %s: %s", configPath, error ? error->message : "unknown error");
        if (error != NULL)
        {
            g_error_free(error);
        }
        g_key_file_free(keyFile);
        return -1;
    }

    // 读取以太网配置
    int ethValue = g_key_file_get_integer(keyFile, "default", "eth", &error);
    if (error != NULL)
    {
        LOG_Error("Failed to read eth value from %s: %s", configPath, error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return -1;
    }

    LOG_Info("Read ethernet config from %s: eth=%d", configPath, ethValue);

    // 设置全局变量 g_isEthernetVersion（供 fty.c 使用）
    SetEthernetVersion((ethValue == 1) ? TRUE : FALSE);

    // 构建DID_FUNCTION_CONFIG配置数据（64字节）
    uint8_t configBuf[64] = {0};
    uint8_t vehicleTypeForMqttTopic[10] = "T08-Y1";
    uint8_t vehicleTypeForPkiSDK[30] = "T08Y1-TBOX-SM2-021";
    uint8_t ecallAudioType = 0x02;
    uint8_t bcallAudioType = 0x02;
    uint8_t ethernetConfig = (ethValue == 1) ? 0x01 : 0x00;  // 1:以太网 0:非以太网
    uint8_t rtkConfig = 0x00;
    uint8_t srsPwmType = 0x00;
    uint8_t simCardNumber = 0x00;

    // 组装config40字节
    uint8_t config40 = (ecallAudioType & 0x03) |
                        ((bcallAudioType & 0x03) << 2) |
                        ((ethernetConfig & 0x01) << 4) |
                        ((rtkConfig & 0x01) << 5) |
                        ((srsPwmType & 0x01) << 6) |
                        ((simCardNumber & 0x01) << 7);

    // 填充配置缓冲区
    (void)memcpy(&configBuf[0], vehicleTypeForMqttTopic, 10);
    (void)memcpy(&configBuf[10], vehicleTypeForPkiSDK, 30);
    configBuf[40] = config40;
    configBuf[41] = 0x00;
    configBuf[42] = 0x00;
    configBuf[43] = 0x00;

    // 写入DID_FUNCTION_CONFIG
    UdsNrc_e nrc = UdsDidWrite(DID_FUNCTION_CONFIG, configBuf, 64);
    if (nrc == NRC_NONE_E)
    {
        LOG_Info("Successfully wrote DID_FUNCTION_CONFIG with ethernet config: %s",
                 ethernetConfig ? "Ethernet" : "Non-Ethernet");
        ret = 0;
    }
    else
    {
        LOG_Error("Failed to write DID_FUNCTION_CONFIG, NRC=0x%02X", nrc);
        ret = -1;
    }

    g_key_file_free(keyFile);
    return ret;
}
#endif
/* ---------------- 主入口 ---------------- */
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    /* 日志初始化（按需替换规则名，比如 "dtc"） */
    LOG_INIT("dtcApp");
    LOG_Info("UDS Gateway starting... queue=\"%s\", db=\"%s\"", DTC_MSG_QUEUE_NAME, UDS_DB_PATH);

    (void)SirunSignalCatchInit();
    
    /* STEP 0: 启动消息接收线程 */
    (void)StartQueueMsgRcvThread(DTC_MSG_QUEUE_NAME);

    (void)RpcStartMsgServer(RPC_APP_SERVER_TYPE_DTC, RpcAppDtcCallBack, 1, NULL);
    

    /*初始化检测备份还原，并且查看是否存在丢失的数据库， 如果有则进行备份还原*/
    DbBackupConfig_t dbBackupConfig = {.dbFilePath = UDS_DB_PATH, .backupFilePath = UDS_DB_BACKUP_PATH};
    (void)DbBackupServiceInit(&dbBackupConfig);
    (void)DbBackupCheckAndRestore();
    (void)DbBackupServiceStart();

    /* 初始化回调函数 */
    UdsCallbackEntryInit();

    /* STEP 0.5: DID 存储初始化（打开 DB / 建表 / 下沉默认 / 回灌） */
    (void)UdsStorageInit(UDS_DB_PATH);

    /* STEP 1: 初始化网关配置 */
    UdsGatewayConfig_t cfg;
    (void)memset(&cfg, 0, sizeof cfg);
    cfg.reqType               = (SYSQueMsgType_e)APP_RPC_QUE_TYPE_MCU_DTC_MCU_TO_MPU;        /* 下行诊断请求消息类型 */
    cfg.respType              = (SYSQueMsgType_e)APP_RPC_QUE_TYPE_MCU_DTC_MPU_TO_MCU_ACK;    /* 上行诊断响应消息类型 */
    cfg.useExtendedNegWithDid = FALSE;                                   /* 7F 不带 DID（外部标准） */
    cfg.maxServiceData        = 1024u;                                   /* 响应缓冲上限 */

    (void)UdsGatewayInit(&gGw, &cfg);
#if 1 //SQ
    AdbOpen();
#else
    // ADB状态初始化：第一次开机设置为关闭，后续根据存储的状态恢复
    {
        uint8_t adbStatus = 0;
        uint16_t adbStatusLen = sizeof(adbStatus);
        UdsNrc_e nrc = UdsDidRead(DID_ADB_STATUS, &adbStatus, &adbStatusLen);
        if (nrc == NRC_NONE_E && adbStatusLen == 1)
        {
            // 数据库中有记录，根据记录的状态恢复ADB
            if (adbStatus == 0x01)
            {
                LOG_Info("Restoring ADB status: OPEN");
                AdbOpen();
            }
            else
            {
                LOG_Info("Restoring ADB status: CLOSE");
                AdbClose();
            }
        }
        else
        {
            // 第一次开机，数据库中没有记录，设置为关闭状态
            LOG_Info("First boot detected, setting ADB to CLOSE");
            AdbClose();
            adbStatus = 0x00;
            (void)UdsDidWrite(DID_ADB_STATUS, &adbStatus, 1);
        }
    }   

    // 记录重启次数
    // E_ParamId_4G_HW_Reset_Count
    ParamInfo_t param = {0};
    uint32_t v = 0;
    param.id = E_ParamId_4G_HW_Reset_Count;
    int ret = GetParam(&param);
    if (ret == 0)
    {
        if (ParseUint32FromAscii(param.value, sizeof(param.value), 0U, 0xFFFFFFFFU, &v) == true)
        {
            v++;
            UintToAsciiIntoParamValue(param.value, sizeof(param.value), v);
            uint32_t wlen = (uint32_t)strnlen((const char*)param.value, sizeof(param.value));
            (void)UpdateParam(param.id, param.value, wlen);
        }
    }
#endif
    // ddr init
    // (void)DdrCheckAndUpdate();
    // ddr测试过程中不可下电，删除之前版本中可能存在的误报文件
    RemoteDdrTestFile();

    // 根据/oemapp/sr_config/ethConfig.ini文件内容，更新DID_FUNCTION_CONFIG的配置
    // 同时设置 g_isEthernetVersion 供 fty.c 使用
    // (void)LoadAndApplyEthConfigFromIni("/oemapp/sr_config/ethConfig.ini");

    /* STEP 2: 订阅 MCU 服务消息（异常才打日志） */
    (void)SubscriptionQueueMsg((SYSQueMsgType_e)APP_RPC_QUE_TYPE_MCU_DTC_MCU_TO_MPU, McuSrvReadDtcCallback, NULL);
    (void)SubscriptionQueueMsg(SYS_QUE_TYPE_GNSS_LOCATIONINFO, FtyGnssMsgCallback, NULL);
    (void)SubscriptionQueueMsg(SYS_QUE_TYPE_MODEM_SIGNAL_NOFIFY, FtyModemCallback, NULL);
    (void)SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, FtyPmCallback, NULL);
    (void)SubscriptionQueueMsg(SYS_QUE_TYPE_GNSS_NMEA, FtyGnssNmeaCallback, NULL);
    (void)SubscriptionQueueMsg(APP_RPC_QUE_TYPE_SYNC_DTC_MPU_FROM_MCU_ACK, FtySyncVehicleEolDataAckCallback, NULL);
    (void)SubscriptionQueueMsg(APP_RPC_QUE_TYPE_SYNC_DTC_MCU_FROM_MPU, FtyMcuReqVehicleEolDataCallback, NULL);

    /* STEP 3: 启动 GLib 主循环（启动/停止各打一条） */
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    GThread *sendTask = g_thread_new("ftyPeriod", SendInfoToMCU, (gpointer)loop);
    // GThread *SyncVehicleEolDataTask = g_thread_new("syncEOLData", SyncVehicleEolDataFromMcu, (gpointer)loop);

#ifdef G_OS_UNIX
    /* 可选：捕捉信号优雅退出（只在退出时打一条日志，不在回调里刷频繁日志） */
    (void)g_unix_signal_add(SIGINT,  (GSourceFunc)g_main_loop_quit, loop);
    (void)g_unix_signal_add(SIGTERM, (GSourceFunc)g_main_loop_quit, loop);
#endif

    LOG_Notice("UDS Gateway main loop running");
    (void)g_main_loop_run(loop);
    LOG_Notice("UDS Gateway main loop stopped");
    g_thread_unref(sendTask);
    // g_thread_unref(SyncVehicleEolDataTask);
    (void)g_main_loop_unref(loop);

    /* STEP 4: 资源清理（异常才打日志） */
    UdsStorageClose();

    return 0;
}
