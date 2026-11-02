/**
 * tsp_app.c - 应用主控与状态机实现
 */

#include "tsp_app.h"
#include "tsp_log.h"
#include "tsp_protocol.h"
#include "tsp_http.h"
#include "tsp_offline_db.h"
#include "tsp_ota.h"
#include "tsp_device.h"
#include "tsp_upgrade_tbox.h"
#include "can_signal_header_tonly.h"
#include "tsp_ecu_version.h"
#include "tsp_remote_ctrl.h"
#include "tsp_common.h"
#include "tsp_can_data.h"
#include "tsp_vin.h"

/* PM Server SDK */
#include "pm_server_api.h"

/* OTA Master SDK */
#include "ota_master_api.h"
#include "ota_master_data.h"

/* MCU Service SDK */
#include "mcu_api.h"
#include "sr_ipc.h"
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <event2/thread.h>
#include <sys/stat.h>
#include <errno.h>
#include "sr_param.h"
#include "err_code.h"
#include "mcu_api.h"


static bool g_ipc_initialized = false;  /* IPC 初始化状态标志 */
static tly_app_t *g_app_ctx = NULL;     /* 全局app上下文，用于回调函数访问 */

/* CAN信号全局数据 */
static TonlyAllCanSignal_canVal_t g_canSignal = {0};
static pthread_rwlock_t g_canSignalRwlock = PTHREAD_RWLOCK_INITIALIZER;

/**
 * @brief CAN信号订阅回调
 */
static void CanSignalCallback(const uint8_t *recvData, const uint16_t dataLen, void *userdata)
{
    if (NULL == recvData) {
        TLY_LOG_E("CAN_SIGNAL: callback recvData is NULL");
        return;
    }
    if (dataLen != sizeof(TonlyAllCanSignal_canVal_t)) {
        TLY_LOG_E("CAN_SIGNAL: callback dataLen mismatch: %d != %zu", dataLen, sizeof(TonlyAllCanSignal_canVal_t));
        return;
    }

    const TonlyAllCanSignal_canVal_t *tmpRecvData = (const TonlyAllCanSignal_canVal_t *)recvData;
    pthread_rwlock_wrlock(&g_canSignalRwlock);
    memcpy(&g_canSignal, tmpRecvData, sizeof(TonlyAllCanSignal_canVal_t));
    pthread_rwlock_unlock(&g_canSignalRwlock);
}


/**
 * @brief 获取CAN信号数据（供tsp_device.c使用）
 */
int GetCanSignalData(TonlyAllCanSignal_canVal_t *outData)
{
    if (!outData) {
        return -1;
    }
    pthread_rwlock_rdlock(&g_canSignalRwlock);
    memcpy(outData, &g_canSignal, sizeof(TonlyAllCanSignal_canVal_t));
    pthread_rwlock_unlock(&g_canSignalRwlock);

    return 0;
}

/* ======================== 内部回调 ======================== */

static void on_tcp_packet(const tly_packet_t *pkt, void *ctx);
static void on_tcp_status(int connected, void *ctx);
static void on_heartbeat(evutil_socket_t fd, short events, void *ctx);
static void on_report_timer(evutil_socket_t fd, short events, void *ctx);
static void on_reissue_timer(evutil_socket_t fd, short events, void *ctx);
static void on_signal(evutil_socket_t fd, short events, void *ctx);
static void on_remove_alarm_check(evutil_socket_t fd, short events, void *ctx);
static void on_fault_check_timer(evutil_socket_t fd, short events, void *ctx);
static void start_fault_check_timer(tly_app_t *app);
static int  tly_app_send_vehicle_login(tly_app_t *app);
static int  tly_app_send_canid_request(tly_app_t *app);
static int  tly_app_send_version_upload(tly_app_t *app);
static int  tly_app_send_all_ecu_versions(tly_app_t *app);
static int  tly_app_send_wakeup_location(tly_app_t *app);
static int  tly_app_start_remove_alarm_check(tly_app_t *app);
static void tly_app_stop_remove_alarm_check(tly_app_t *app);


/* ======================== OTA Master 回调函数 ======================== */
/* ECU UDS响应数据订阅回调（MCU→MPU）*/
static void EcuUdsDataCallback(const uint8_t *data, const uint16_t dataLen, void *userdata)
{
    TLY_LOG_D("EcuUdsDataCallback dataLen: %d", dataLen);
    if (!data || dataLen == 0) {
        TLY_LOG_E("EcuUdsDataCallback invalid params");
        return;
    }
    /* 将ECU响应数据注入到OTA Master SDK */
    OtaMasterNoticeEcuUdsData(data, dataLen);
}

/* MCU ACK确认订阅回调（MPU→MCU发送确认）*/
static void McuUdsAckCallback(const uint8_t *data, const uint16_t dataLen, void *userdata)
{
    TLY_LOG_D("McuUdsAckCallback dataLen: %d", dataLen);
    if (!data || dataLen == 0) {
        TLY_LOG_E("McuUdsAckCallback invalid params");
        return;
    }
    /* 将MCU ACK注入到OTA Master SDK */
    OtaMasterNoticeMcuUdsDataAck(data, dataLen);
}

/* ======================== OTA Master Report 回调函数 ======================== */

/* 上报OTA任务清空结果回调 */
static int ReportOtaTaskClearResultCallback(const char *taskId, int result)
{
    TLY_LOG_I("ReportOtaTaskClearResultCallback taskId: %s, result: %d",
              taskId ? taskId : "NULL", result);
    return OTA_RET_OK;
}

/* UDS协议数据发送回调（MPU→MCU→CAN总线）*/
 
static int SendUdsDataCallback(const uint8_t *data, uint16_t dataLen)
{
    TLY_LOG_D("SendUdsDataCallback dataLen: %d", dataLen);
    if (!data || dataLen == 0) {
        TLY_LOG_E("SendUdsDataCallback invalid params");
        return OTA_RET_ERR;
    }
    
    McuMesg_t mesg;
    mesg.type = SYS_QUE_TYPE_MCU_CANTP_FLASH_MPU_TO_MCU;
    mesg.payloadLen = dataLen;
    mesg.payload = (uint8_t *)data;
    
    int32_t ret = McuSrvSendMesg(&mesg);
    if (ret != 0) {
        TLY_LOG_E("McuSrvSendMesg FLASH_MPU_TO_MCU failed, ret: %d", ret);
        return OTA_RET_ERR;
    }
    return OTA_RET_OK;
}

/* UDS ACK数据发送回调（SDK收到ECU响应后回复ACK给MCU）*/
static int SendUdsDataAckCallback(const uint8_t *data, uint16_t dataLen)
{
    TLY_LOG_D("SendUdsDataAckCallback dataLen: %d", dataLen);
    if (!data || dataLen == 0) {
        TLY_LOG_E("SendUdsDataAckCallback invalid params");
        return OTA_RET_ERR;
    }
    
    McuMesg_t mesg;
    mesg.type = SYS_QUE_TYPE_MCU_CANTP_MCU_TO_MPU_ACK;
    mesg.payloadLen = dataLen;
    mesg.payload = (uint8_t *)data;
    
    int32_t ret = McuSrvSendMesg(&mesg);
    if (ret != 0) {
        TLY_LOG_E("McuSrvSendMesg MCU_TO_MPU_ACK failed, ret: %d", ret);
        return OTA_RET_ERR;
    }
    return OTA_RET_OK;
}

/* ECU版本异步查询线程函数 */
static void *ecu_version_query_thread(void *arg)
{
    TLY_LOG_I("ECU version query thread started");
    
    /* 通过UDS诊断查询ECU版本信息 */
    tly_ecu_version_init();
    int ret = tly_ecu_version_read_all();
    if (ret == 0) {
        TLY_LOG_I("ECU versions read successfully in thread");
        tly_ecu_version_dump();

        /* 查询成功后上报版本到平台 */
        if (g_app_ctx && tly_tcp_is_connected(g_app_ctx->tcp)) {
            TLY_LOG_I("Reporting real ECU versions to platform");
            tly_app_send_all_ecu_versions(g_app_ctx);
        }
    } else {
        TLY_LOG_W("ECU version read failed in thread");
    }
    
    TLY_LOG_I("ECU version query thread finished");
    return NULL;
}

/* 启动ECU版本异步查询*/
static void start_ecu_version_query_async(void)
{
    pthread_t thread_id;
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    if (pthread_create(&thread_id, &attr, ecu_version_query_thread, NULL) != 0) {
        TLY_LOG_E("Failed to create ECU version query thread");
    } else {
        TLY_LOG_I("ECU version query thread created");
    }
    
    pthread_attr_destroy(&attr);
}

/* ECU版本异步查询回调（SDK请求查询ECU版本时可调用）*/
static void QueryTaskECUVersionAsyncCallback(const StQueryEcuVerInfo *queryEcusInfo, uint8_t ecuCnt)
{
    TLY_LOG_I("QueryTaskECUVersionAsyncCallback: ecuCnt=%u", ecuCnt);
    
    if (!queryEcusInfo || ecuCnt == 0) {
        TLY_LOG_E("QueryTaskECUVersionAsyncCallback invalid params");
        return;
    }    
    
    for (uint8_t i = 0; i < ecuCnt; i++) {
        TLY_LOG_I("Query ECU[%u]: ID=0x%04X, canPort=%u, phyId=0x%X, resId=0x%X",
                  i, queryEcusInfo[i].ecuId, queryEcusInfo[i].canPort,
                  queryEcusInfo[i].physicalId, queryEcusInfo[i].responseId);
    }    
    
    const tly_ecu_version_cache_t *cache = tly_ecu_version_get_cache();
    if (!cache || !cache->initialized) {
        TLY_LOG_W("ECU version cache not initialized, notify SDK with empty result");
        OtaMasterNoticeEcuVersionAsyncQueryResult(NULL, 0);
        return;
    }
    
    /* 构造SDK需要的版本信息结构 */
    StEcuVersionInfo *ecuVersions = (StEcuVersionInfo *)malloc(sizeof(StEcuVersionInfo) * ecuCnt);
    if (!ecuVersions) {
        TLY_LOG_E("Failed to allocate memory for ECU versions");
        OtaMasterNoticeEcuVersionAsyncQueryResult(NULL, 0);
        return;
    }
    
    /* 用于存储版本字符串的缓冲区 */
    static char swVerBufs[TLY_ECU_VER_MAX][TLY_ECU_VERSION_MAX_LEN + 1];
    uint8_t validCnt = 0;
    
    for (uint8_t i = 0; i < ecuCnt; i++) {
        /* 根据ECU ID在缓存中查找对应的版本信息 */
        for (int j = 0; j < TLY_ECU_VER_MAX; j++) {
            if (cache->ecus[j].valid && cache->ecus[j].version_len > 0) {
                /* 根据物理ID匹配ECU */
                if (cache->ecus[j].phy_id == queryEcusInfo[i].physicalId) {
                    ecuVersions[validCnt].ecuId = queryEcusInfo[i].ecuId;                   
                    memset(swVerBufs[validCnt], 0, sizeof(swVerBufs[validCnt]));
                    memcpy(swVerBufs[validCnt], cache->ecus[j].version,
                           cache->ecus[j].version_len < TLY_ECU_VERSION_MAX_LEN ?
                           cache->ecus[j].version_len : TLY_ECU_VERSION_MAX_LEN);
                    ecuVersions[validCnt].swVersion = swVerBufs[validCnt];
                    ecuVersions[validCnt].hwVersion = "";
                    ecuVersions[validCnt].supplierId = "";
                    ecuVersions[validCnt].partNum = "";
                    validCnt++;
                    TLY_LOG_I("Found ECU[0x%04X] version: %s",
                              queryEcusInfo[i].ecuId, swVerBufs[validCnt-1]);
                    break;
                }
            }
        }
    }
    
    /* 通知SDK查询结果 */
    if (validCnt > 0) {
        OtaMasterNoticeEcuVersionAsyncQueryResult(ecuVersions, validCnt);
        TLY_LOG_I("Notify OTA Master SDK with %u ECU versions", validCnt);
    } else {
        TLY_LOG_W("No valid ECU version found, notify SDK with empty result");
        OtaMasterNoticeEcuVersionAsyncQueryResult(NULL, 0);
    }
    
    free(ecuVersions);
}

/* 初始化 OTA Master SDK */
static int tly_app_init_ota_master(tly_app_t *app)
{
    /* 保存app上下文供回调函数使用 */
    g_app_ctx = app;
    
    if (g_ipc_initialized) {
        TLY_LOG_I("IPC already initialized, skip");
    } else if (InitSysQueTree() != 0) {
        TLY_LOG_W("InitSysQueTree failed, IPC will be initialized later when system service is ready");
        g_ipc_initialized = false;
    } else if (StartQueueMsgRcvThread("tsp_app") != 0) {
        TLY_LOG_W("StartQueueMsgRcvThread failed, IPC will be initialized later when system service is ready");
        g_ipc_initialized = false;
    } else {
        /* 订阅 ECU UDS 响应数据（MCU→MPU）- 远程诊断协议 */
        SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_CANTP_MCU_TO_MPU, EcuUdsDataCallback, NULL);
        /* 订阅 MCU ACK 确认消息（MPU→MCU发送确认）- 远程诊断协议 */
        SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_CANTP_MPU_TO_MCU_ACK, McuUdsAckCallback, NULL);
        /* 订阅刷写协议的 ECU UDS 响应数据（MCU→MPU）*/
        SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_CANTP_FLASH_MCU_TO_MPU, EcuUdsDataCallback, NULL);
        /* 订阅刷写协议的 MCU ACK 确认消息（MPU→MCU发送确认）*/
        SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_CANTP_FLASH_MPU_TO_MCU_ACK, McuUdsAckCallback, NULL);
        
        SubscriptionQueueMsg((SYSQueMsgType_e)TonlyAllCanSignal_canVal_t_msgid, CanSignalCallback, NULL);

        g_ipc_initialized = true;
        TLY_LOG_I("IPC initialized, UDS subscriptions registered");
    }
    
    /* 初始化 OTA Master SDK */
    int ret = OtaMasterInit(
        app->config.ota_master_config_dir,
        app->config.ota_save_dir,
        app->config.ota_unzip_dir
    );
    if (ret != OTA_RET_OK) {
        TLY_LOG_E("OtaMasterInit failed");
        return -1;
    }    
    
    /* 注册上报回调 - OTA状态上报回调 */
    StOtaMasterReportCallback reportCb = {
        .startDownloadCb = NULL,
        .downloadResultCb = NULL,
        .upgradeResultCb = NULL,
        .otaTaskClearResultCb = ReportOtaTaskClearResultCallback,
        .preconditionsCheckResultCb = NULL,
        .startUpgradeCb = NULL,
        .startRollbackCb = NULL,
    };
    OtaMasterRegisterReportCallback(&reportCb);
    
    /* 注册查询回调 - ECU版本异步查询回调 */
    StOtaMasterQueryCallback queryCb = {
        .queryTaskECUVerSyncCb = NULL,  /* 不使用同步查询 */
        .queryTaskECUVerAsyncCb = QueryTaskECUVersionAsyncCallback,  /* 使用异步查询 */
        .queryUpgradePrecondsSyncCb = NULL,
        .queryUpgradePrecondsAsyncCb = NULL,
    };
    OtaMasterRegisterQueryCallback(&queryCb);
    
    /* 注册请求回调 - UDS 数据发送回调 */
    StOtaMasterRequestCallback requestCb = {
        .sendUdsDataCb = SendUdsDataCallback,
        .sendUdsDataAckCb = SendUdsDataAckCallback,
        .reqUpDownHighVoltCb = NULL,
        .reqFotaModeOnOff = NULL,
        .reqTboxSelfUpgradeCb = NULL,
        .reqTboxSelfRollbackCb = NULL,
        .reqTboxRebootCb = NULL,
    };
    OtaMasterRegisterRequestCallback(&requestCb);
    
    /* 启动 OTA Master */
    ret = OtaMasterStart();
    if (ret != OTA_RET_OK) {
        TLY_LOG_E("OtaMasterStart failed");
        return -1;
    }
    
    TLY_LOG_I("OTA Master initialized successfully");
    return 0;
}

/* ======================== 生命周期 ======================== */

int tly_app_init(tly_app_t *app, const char *conf_file)
{
    if (!app || !conf_file) return TLY_ERR_PARAM;

    memset(app, 0, sizeof(*app));
    app->state = TLY_STATE_INIT;

    /* 加载配置 */
    int rc = tly_config_load(&app->config, conf_file);
    if (rc != 0) {
        fprintf(stderr, "Failed to load config: %s\n", conf_file);
        return rc;
    }

    /* 初始化日志 */
    // tly_log_level_t level = tly_log_level_from_str(app->config.log_level);
    // rc = tly_log_init(level, app->config.log_path);
    // if (rc != 0) {
    //     fprintf(stderr, "Failed to init log\n");
    //     return rc;
    // }

    tly_config_dump(&app->config);

    /* 初始化libevent线程支持 */
    (void)evthread_use_pthreads();

    /* 初始化libevent */
    app->evbase = event_base_new();
    if (!app->evbase) {
        TLY_LOG_E("Failed to create event base");
        return TLY_ERR_MEMORY;
    }

    tly_http_global_init();

    if (tly_offline_db_init(app->config.offline_dir, app->config.offline_max_days) != 0) {
        TLY_LOG_W("Offline storage db init failed, offline save disabled");
    }

    tly_offline_db_cleanup();

    tly_recorder_init(&app->recorder, app->evbase);

    /* 创建TCP连接管理器 */
    app->tcp = tly_tcp_create(app->evbase);
    if (!app->tcp) {
        TLY_LOG_E("Failed to create TCP connection");
        tly_app_destroy(app);
        return TLY_ERR_MEMORY;
    }

    tly_tcp_set_callbacks(app->tcp, on_tcp_packet, on_tcp_status, app);

    /* 注册信号处理 */
    app->ev_sigterm = evsignal_new(app->evbase, SIGTERM, on_signal, app);
    app->ev_sigint  = evsignal_new(app->evbase, SIGINT, on_signal, app);
    if (app->ev_sigterm) event_add(app->ev_sigterm, NULL);
    if (app->ev_sigint)  event_add(app->ev_sigint, NULL);
 
    /* 初始化 OTA Master SDK */
   if (tly_app_init_ota_master(app) != 0) {
       TLY_LOG_W("OTA Master initialization failed, OTA for other ECU disabled");
   }

    /* 初始化消息模块 */
    (void)CommonMsg_Subscribe(app);

    /* 初始化远控模块 */
    if (remote_ctrl_init(app) != 0) {
        TLY_LOG_W("Remote control module initialization failed");
    }
    
    /* 初始化CAN数据采集模块(先订阅队列, 筛选列表由0x7B/0x77响应后更新) */
    if (tsp_can_data_init() != 0) {
        TLY_LOG_W("CAN data module initialization failed, CAN raw data will not be available");
    }

    /* 初始化VIN数据模块 */
    if (tsp_vin_data_init(app) != 0) {
        TLY_LOG_W("VIN data module initialization failed, VIN data will not be available");
    }

    /* 启动故障检测定时器 */
    start_fault_check_timer(app);
    
    /* 初始化拆车报警检测（默认启用，2秒间隔） */
    if (tly_app_start_remove_alarm_check(app) != 0) {
        TLY_LOG_W("Remove alarm check initialization failed");
    }

    TLY_LOG_I("Application initialized");
    return 0;
}

int tly_app_run(tly_app_t *app)
{
    if (!app) return TLY_ERR_PARAM;

    /* 发起TCP连接 */
    app->state = TLY_STATE_TCP_CONNECTING;
    int rc = tly_tcp_connect(app->tcp,
                             app->config.tcp_host,
                             app->config.tcp_port,
                             app->config.tcp_reconnect_interval);
    if (rc != 0) {
        TLY_LOG_E("TCP connect failed immediately");
        return rc;
    }

    TLY_LOG_I("Entering event loop...");
    rc = event_base_dispatch(app->evbase);

    TLY_LOG_I("Event loop exited (rc=%d)", rc);
    app->state = TLY_STATE_INIT;
    return (rc == 0) ? 0 : -1;
}

void tly_app_stop(tly_app_t *app)
{
    if (!app) return;

    TLY_LOG_I("Application stopping...");
    app->state = TLY_STATE_STOPPING;

    /* 停止心跳 */
    if (app->ev_heartbeat) {
        event_del(app->ev_heartbeat);
        event_free(app->ev_heartbeat);
        app->ev_heartbeat = NULL;
    }

    /* 停止实时上报 */
    if (app->ev_report) {
        event_del(app->ev_report);
        event_free(app->ev_report);
        app->ev_report = NULL;
    }

    /* 停止补发 */
    if (app->ev_reissue) {
        event_del(app->ev_reissue);
        event_free(app->ev_reissue);
        app->ev_reissue = NULL;
    }

    /* 停止故障检测 */
    if (app->ev_fault_check) {
        event_del(app->ev_fault_check);
        event_free(app->ev_fault_check);
        app->ev_fault_check = NULL;
    }

    /* 停止拆车报警检测 */
    tly_app_stop_remove_alarm_check(app);

    tly_recorder_stop(&app->recorder);

    /* 断开TCP */
    tly_tcp_disconnect(app->tcp);

    /* 退出事件循环 */
    if (app->evbase) {
        event_base_loopexit(app->evbase, NULL);
    }
}

void tly_app_destroy(tly_app_t *app)
{
    if (!app) return;

    if (app->ev_heartbeat) {
        event_del(app->ev_heartbeat);
        event_free(app->ev_heartbeat);
        app->ev_heartbeat = NULL;
    }

    if (app->ev_report) {
        event_del(app->ev_report);
        event_free(app->ev_report);
        app->ev_report = NULL;
    }

    if (app->ev_reissue) {
        event_del(app->ev_reissue);
        event_free(app->ev_reissue);
        app->ev_reissue = NULL;
    }

    if (app->ev_sigterm) {
        event_del(app->ev_sigterm);
        event_free(app->ev_sigterm);
        app->ev_sigterm = NULL;
    }

    if (app->ev_sigint) {
        event_del(app->ev_sigint);
        event_free(app->ev_sigint);
        app->ev_sigint = NULL;
    }

    if (app->ev_fault_check) {
        event_del(app->ev_fault_check);
        event_free(app->ev_fault_check);
        app->ev_fault_check = NULL;
    }

    if (app->ev_remove_alarm_check) {
        event_del(app->ev_remove_alarm_check);
        event_free(app->ev_remove_alarm_check);
        app->ev_remove_alarm_check = NULL;
    }

    /* 反初始化消息模块 */
    CommonMsg_Unsubscribe();

    if (app->tcp) {
        tly_tcp_destroy(app->tcp);
        app->tcp = NULL;
    }

    if (app->evbase) {
        event_base_free(app->evbase);
        app->evbase = NULL;
    }

    tly_recorder_destroy(&app->recorder);

    /* 反初始化远控模块 */
    remote_ctrl_deinit();

    /* 反初始化CAN数据采集模块 */
    tsp_can_data_deinit();

    /* 关闭离线数据库 */
    tly_offline_db_deinit();

    /* 停止并清理 OTA Master */
    OtaMasterStop();
    OtaMasterUnregisterRequestCallback();
    OtaMasterUnregisterQueryCallback();
    OtaMasterUnregisterReportCallback();
    OtaMasterDeinit();   

    tly_http_global_cleanup();
    tly_log_close();

    TLY_LOG_I("Application destroyed (stats: recv=%u, send=%u, err=%u)",
              app->pkt_recv_count, app->pkt_send_count, app->err_count);
}

/* ======================== 车辆登入 ======================== */

static int tly_app_send_vehicle_login(tly_app_t *app)
{
    if (!app || !tly_tcp_is_connected(app->tcp)) return TLY_ERR_CLOSED;

    tly_vehicle_login_t login;
    memset(&login, 0, sizeof(login));

    tly_time_now(login.time);

    app->login_seq++;
    if (app->login_seq > 65531) app->login_seq = 1;
    login.seq = app->login_seq;

    snprintf(login.iccid, sizeof(login.iccid), "%s", app->config.iccid);

    uint8_t data_buf[32];
    uint32_t data_len = 0;
    int rc = tly_encode_vehicle_login(&login, data_buf, &data_len);
    if (rc != 0) {
        TLY_LOG_E("Failed to encode vehicle login: %d", rc);
        return rc;
    }

    tly_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd_id       = TLY_CMD_VEHICLE_LOGIN;
    snprintf(pkt.vin, sizeof(pkt.vin), "%s", app->config.vin);
    pkt.sw_version   = app->config.sw_version;
    pkt.encrypt_mode = TLY_ENCRYPT_NONE;
    pkt.data_len     = data_len;
    pkt.data         = data_buf;

    rc = tly_tcp_send_packet(app->tcp, &pkt);
    if (rc == 0) {
        app->pkt_send_count++;
        TLY_LOG_I("Vehicle login sent: seq=%u, ICCID=%.20s", login.seq, login.iccid);
    } else {
        TLY_LOG_E("Vehicle login send failed: %d", rc);
    }

    /* 打印完整登入报文原始hex */
    uint8_t raw_buf[TLY_PKT_MAX_SIZE];
    uint32_t raw_len = 0;
    if (tly_packet_encode(&pkt, raw_buf, &raw_len) == 0) {
        TLY_LOG_Dump("LOGIN PKT RAW:", raw_buf, raw_len);
    }

    return rc;
}

/* ======================== 版本号上传 ======================== */

static int tly_app_send_version_upload(tly_app_t *app)
{
    if (!app || !tly_tcp_is_connected(app->tcp))
        return TLY_ERR_CLOSED;

    app->version_seq++;
    if (app->version_seq > 65531) app->version_seq = 1;

    tly_version_upload_t ver;
    memset(&ver, 0, sizeof(ver));
    tly_time_now(ver.time);

    ver.hw_version         = app->config.hw_version;
    ver.main_mcu_version   = app->config.main_mcu_version;
    ver.sub_mcu_version    = app->config.sub_mcu_version;
    ver.non_stage4_version = app->config.non_stage4_version;
    ver.version_seq        = app->version_seq;

    uint8_t data_buf[24];
    uint32_t data_len = 0;
    int rc = tly_encode_version_upload(&ver, data_buf, &data_len);
    if (rc != 0) {
        TLY_LOG_E("Failed to encode version upload: %d", rc);
        return rc;
    }

    tly_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd_id       = TLY_CMD_VERSION_UPLOAD;
    snprintf(pkt.vin, sizeof(pkt.vin), "%s", app->config.vin);
    pkt.sw_version   = app->config.sw_version;
    pkt.encrypt_mode = TLY_ENCRYPT_NONE;
    pkt.data_len     = data_len;
    pkt.data         = data_buf;

    rc = tly_tcp_send_packet(app->tcp, &pkt);
    if (rc == 0) {
        app->pkt_send_count++;
        TLY_LOG_I("Version upload sent: hw=%u, main_mcu=0x%08X, sub_mcu=%u, non4=%u, seq=%u",
                  ver.hw_version, ver.main_mcu_version,
                  ver.sub_mcu_version, ver.non_stage4_version, ver.version_seq);
    } else {
        TLY_LOG_E("Version upload send failed: %d", rc);
    }

    uint8_t raw_buf[TLY_PKT_MAX_SIZE];
    uint32_t raw_len = 0;
    if (tly_packet_encode(&pkt, raw_buf, &raw_len) == 0) {
        TLY_LOG_Dump("VERSION UPLOAD PKT RAW:", raw_buf, raw_len);
    }

    return rc;
}

/* ======================== VCU版本号上传 (0x73)  ======================== */

int tly_app_send_vcu_version(tly_app_t *app, const uint8_t *version, uint32_t version_len)
{
    if (!app || !tly_tcp_is_connected(app->tcp))
        return TLY_ERR_CLOSED;
    if (!version || version_len == 0 || version_len > 6)
        return TLY_ERR_PARAM;

    tly_vcu_version_t ver;
    memset(&ver, 0, sizeof(ver));
    tly_time_now(ver.time);
    /* VCU version is fixed 6 bytes */
    uint32_t copy_len = (version_len > 6) ? 6 : version_len;
    memcpy(ver.version, version, copy_len);

    uint8_t data_buf[16];
    uint32_t data_len = 0;
    int rc = tly_encode_vcu_version(&ver, data_buf, &data_len);
    if (rc != 0) {
        TLY_LOG_E("Failed to encode VCU version: %d", rc);
        return rc;
    }

    tly_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd_id       = TLY_CMD_VCU_VERSION;
    snprintf(pkt.vin, sizeof(pkt.vin), "%s", app->config.vin);
    pkt.sw_version   = app->config.sw_version;
    pkt.encrypt_mode = TLY_ENCRYPT_NONE;
    pkt.data_len     = data_len;
    pkt.data         = data_buf;

    rc = tly_tcp_send_packet(app->tcp, &pkt);
    if (rc == 0) {
        app->pkt_send_count++;
        TLY_LOG_I("VCU version sent: version_len=%u", version_len);
        TLY_LOG_Dump("VCU version data:", version, version_len);
    } else {
        TLY_LOG_E("VCU version send failed: %d", rc);
    }

    uint8_t raw_buf[TLY_PKT_MAX_SIZE];
    uint32_t raw_len = 0;
    if (tly_packet_encode(&pkt, raw_buf, &raw_len) == 0) {
        TLY_LOG_Dump("VCU VERSION PKT RAW:", raw_buf, raw_len);
    }

    return rc;
}

/* ======================== TCU版本号上传 (0x74) ======================== */

int tly_app_send_tcu_version(tly_app_t *app,
                              const uint8_t *version,
                              uint32_t version_len)
{
    if (!app || !tly_tcp_is_connected(app->tcp))
        return TLY_ERR_CLOSED;
    if (!version || version_len == 0 || version_len > TLY_TCU_VERSION_MAX_LEN)
        return TLY_ERR_PARAM;

    tly_tcu_version_t ver;
    memset(&ver, 0, sizeof(ver));
    memcpy(ver.version, version, version_len);
    ver.version_len = version_len;

    uint8_t data_buf[TLY_TCU_VERSION_MAX_LEN];
    uint32_t data_len = 0;
    int rc = tly_encode_tcu_version(&ver, data_buf, &data_len);
    if (rc != 0) {
        TLY_LOG_E("Failed to encode TCU version: %d", rc);
        return rc;
    }

    tly_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd_id       = TLY_CMD_TCM_VERSION;  /* 0x74 */
    snprintf(pkt.vin, sizeof(pkt.vin), "%s", app->config.vin);
    pkt.sw_version   = app->config.sw_version;
    pkt.encrypt_mode = TLY_ENCRYPT_NONE;
    pkt.data_len     = data_len;
    pkt.data         = data_buf;

    rc = tly_tcp_send_packet(app->tcp, &pkt);
    if (rc == 0) {
        app->pkt_send_count++;
        TLY_LOG_I("TCU version sent: version_len=%u", version_len);
        TLY_LOG_Dump("TCU version data:", version, version_len);
    } else {
        TLY_LOG_E("TCU version send failed: %d", rc);
    }

    uint8_t raw_buf[TLY_PKT_MAX_SIZE];
    uint32_t raw_len = 0;
    if (tly_packet_encode(&pkt, raw_buf, &raw_len) == 0) {
        TLY_LOG_Dump("TCU VERSION PKT RAW:", raw_buf, raw_len);
    }

    return rc;
}

/* ======================== MCU版本号上传 (0x75) ======================== */

int tly_app_send_mcu_version(tly_app_t *app, const tly_mcu_version_t *mcu_ver)
{
    if (!app || !tly_tcp_is_connected(app->tcp))
        return TLY_ERR_CLOSED;
    if (!mcu_ver || mcu_ver->count == 0)
        return TLY_ERR_PARAM;

    uint8_t data_buf[TLY_MCU_VERSION_MAX_LEN];
    uint32_t data_len = 0;
    int rc = tly_encode_mcu_version(mcu_ver, data_buf, &data_len);
    if (rc != 0) {
        TLY_LOG_E("Failed to encode MCU version: %d", rc);
        return rc;
    }

    tly_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd_id       = TLY_CMD_MCU_VERSION;  /* 0x75 */
    snprintf(pkt.vin, sizeof(pkt.vin), "%s", app->config.vin);
    pkt.sw_version   = app->config.sw_version;
    pkt.encrypt_mode = TLY_ENCRYPT_NONE;
    pkt.data_len     = data_len;
    pkt.data         = data_buf;

    rc = tly_tcp_send_packet(app->tcp, &pkt);
    if (rc == 0) {
        app->pkt_send_count++;
        TLY_LOG_I("MCU version sent: count=%u, data_len=%u", mcu_ver->count, data_len);
    } else {
        TLY_LOG_E("MCU version send failed: %d", rc);
    }

    return rc;
}

/* ======================== 所有ECU版本号上报 ======================== */

static int tly_app_send_all_ecu_versions(tly_app_t *app)
{
    if (!app) return TLY_ERR_PARAM;

    /* Send VCU version from cached ECU version data */
    uint8_t vcu_version[TLY_ECU_VERSION_MAX_LEN];
    uint32_t vcu_version_len = TLY_ECU_VERSION_MAX_LEN;
    if (tly_ecu_version_get(TLY_ECU_VER_VCU, vcu_version, &vcu_version_len) == 0) {
        TLY_LOG_I("Sending VCU version from diagnostic read: len=%u", vcu_version_len);
        tly_app_send_vcu_version(app, vcu_version, vcu_version_len);
    } else {
        /* Fallback to default if VCU version not read */
        uint8_t vcu_ver_buf[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x01};
        TLY_LOG_W("VCU version not available from diagnostic, using default");
        tly_app_send_vcu_version(app, vcu_ver_buf, sizeof(vcu_ver_buf));
    }
    
    /* Send TCU version from cached ECU version data */
    uint8_t tcu_version[TLY_ECU_VERSION_MAX_LEN];
    uint32_t tcu_version_len = TLY_ECU_VERSION_MAX_LEN;
    if (tly_ecu_version_get(TLY_ECU_VER_TCU, tcu_version, &tcu_version_len) == 0) {
        TLY_LOG_I("Sending TCU version from diagnostic read: len=%u", tcu_version_len);
        tly_app_send_tcu_version(app, tcu_version, tcu_version_len);
    } else {
        /* Fallback to default if TCU version not read */
        uint8_t tcu_ver_buf[] = "1001";
        TLY_LOG_W("TCU version not available from diagnostic, using default");
        tly_app_send_tcu_version(app, tcu_ver_buf, sizeof(tcu_ver_buf)-1);
    }

    /* Send MCU versions: 打包MCU1_TM1/PDU/EPS/ACM到一个0x75包 */
    {
        /* MCU版本类型与ECU版本缓存索引的映射表 */
        static const struct {
            uint8_t ver_type;             /* 0x75数据单元的mcu_type */
            tly_ecu_ver_type_t cache_idx; /* ECU版本缓存索引 */
            const char *name;
        } mcu_map[] = {
            { TLY_MCU_VER_TYPE_TM1, TLY_ECU_VER_MCU1_TM1, "MCU1_TM1" },
            { TLY_MCU_VER_TYPE_PDU, TLY_ECU_VER_MCU1_PDU, "MCU1_PDU" },
            { TLY_MCU_VER_TYPE_EPS, TLY_ECU_VER_MCU1_EPS, "MCU1_EPS" },
            { TLY_MCU_VER_TYPE_ACM, TLY_ECU_VER_MCU1_ACM, "MCU1_ACM" },
        };
        const uint8_t default_ver[] = "2002";
        const uint8_t default_len = sizeof(default_ver) - 1;  /* 4 */

        tly_mcu_version_t mcu_ver;
        memset(&mcu_ver, 0, sizeof(mcu_ver));

        for (size_t i = 0; i < sizeof(mcu_map)/sizeof(mcu_map[0]); i++) {
            uint8_t ver_data[TLY_ECU_VERSION_MAX_LEN];
            uint32_t ver_len = TLY_ECU_VERSION_MAX_LEN;
            mcu_ver.items[mcu_ver.count].mcu_type = mcu_map[i].ver_type;

            if (tly_ecu_version_get(mcu_map[i].cache_idx, ver_data, &ver_len) == 0) {
                /* 读取成功: 截取前10字节填入(不足补0x00) */
                uint32_t copy_len = (ver_len > TLY_MCU_VER_CONTENT_LEN) ? TLY_MCU_VER_CONTENT_LEN : ver_len;
                memcpy(mcu_ver.items[mcu_ver.count].version, ver_data, copy_len);
                TLY_LOG_I("Sending %s version from diagnostic: len=%u", mcu_map[i].name, ver_len);
            } else {
                /* 读取失败: 用默认值"2002"填充(不足补0x00) */
                memcpy(mcu_ver.items[mcu_ver.count].version, default_ver, default_len);
                TLY_LOG_W("%s version not available, using default", mcu_map[i].name);
            }
            mcu_ver.count++;
        }

        tly_app_send_mcu_version(app, &mcu_ver);
    }

    /* TODO: Add TMS_A, TMS_B, MCU2_TM1, MCU2_TM2 version report*/
    
    return 0;
}

/* ======================== CANID列表请求 ======================== */

static int tly_app_send_canid_request(tly_app_t *app)
{
    if (!app || !tly_tcp_is_connected(app->tcp)) return TLY_ERR_CLOSED;

    uint8_t data_buf[4];
    uint32_t data_len = 0;
    tly_encode_canid_request(1, data_buf, &data_len);

    tly_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd_id       = TLY_CMD_CANID_LIST_CH;
    snprintf(pkt.vin, sizeof(pkt.vin), "%s", app->config.vin);
    pkt.sw_version   = app->config.sw_version;
    pkt.encrypt_mode = TLY_ENCRYPT_NONE;
    pkt.data_len     = data_len;
    pkt.data         = data_buf;

    int rc = tly_tcp_send_packet(app->tcp, &pkt);
    if (rc == 0) {
        app->pkt_send_count++;
        TLY_LOG_I("CANID list request sent (0x7B)");
    }

    /* 打印完整周期上报报文原始hex */
    uint8_t raw_buf[TLY_PKT_MAX_SIZE];
    uint32_t raw_len = 0;
    if (tly_packet_encode(&pkt, raw_buf, &raw_len) == 0) {
        TLY_LOG_Dump("REALTIME PKT RAW:", raw_buf, raw_len);
    }

    return rc;
}

/* ======================== 实时信息上报 ======================== */

#define TLY_REPORT_INTERVAL_SEC  10
#if 0
static int tly_app_send_realtime_report(tly_app_t *app)
{
    if (!app || !tly_tcp_is_connected(app->tcp)) return TLY_ERR_CLOSED;
    if (!app->canid_received) return TLY_ERR_PARAM;

    /* 数据单元: 时间(6B) + 信息项(0xA0电车快包) */
    uint8_t *data_buf = (uint8_t *)malloc(TLY_PKT_MAX_SIZE);
    if (!data_buf) return TLY_ERR_MEMORY;
    uint32_t data_pos = 0;

    /* 时间 */
    tly_time_now(data_buf);
    data_pos += 6;

    /* 信息标识 0xA0 (电车快包) */
    data_buf[data_pos++] = TLY_INFO_EV_FAST;

    /* 电车快包载荷: 帧数量 + 各帧 */
    tly_canid_list_resp_t *can = &app->canid_cache;
    uint8_t *ev_start = data_buf + data_pos;

    /* 用快包CANID列表构造帧, 每帧发送采集到的CAN数据 */
    uint8_t frame_count = (uint8_t)(can->fast_count > TLY_EV_FRAME_MAX
                                    ? TLY_EV_FRAME_MAX : can->fast_count);
    ev_start[0] = frame_count;
    uint32_t ev_pos = 1;

    for (int i = 0; i < frame_count; i++) {
        /* CANID(4B, 小端) */
        uint32_t cid = can->fast_ids[i].can_id;
        ev_start[ev_pos++] = (uint8_t)(cid);
        ev_start[ev_pos++] = (uint8_t)(cid >> 8);
        ev_start[ev_pos++] = (uint8_t)(cid >> 16);
        ev_start[ev_pos++] = (uint8_t)(cid >> 24);

        /* 报文状态: len=8, channel 从CANID列表取 */
        uint8_t ch = can->fast_ids[i].channel;
        uint8_t dlc = 8;
        uint8_t can_data[8] = {0};

        /* 从CAN数据缓存查找真实报文 */
        int ret = tsp_can_data_lookup(cid, ch, can_data, &dlc);
        if (ret != 0) {
            /* 查找失败，说明该CAN ID未在缓存中找到或发生错误，DLC设为0 */
            dlc = 0;
        }
        dlc = (dlc > 8) ? 8 : dlc;

        ev_start[ev_pos++] = (uint8_t)(dlc | (ch << 4));

        memcpy(ev_start + ev_pos, can_data, dlc);
        /* 不足8字节补0 */
        if (dlc < 8) {
            memset(ev_start + ev_pos + dlc, 0, 8 - dlc);
        }
        ev_pos += 8;
    }

    data_pos += ev_pos;

    /* 组包发送 */
    tly_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd_id       = TLY_CMD_REALTIME_REPORT;
    snprintf(pkt.vin, sizeof(pkt.vin), "%s", app->config.vin);
    pkt.sw_version   = app->config.sw_version;
    pkt.encrypt_mode = TLY_ENCRYPT_NONE;
    pkt.data_len     = data_pos;
    pkt.data         = data_buf;

    int rc = tly_tcp_send_packet(app->tcp, &pkt);
    if (rc == 0) {
        app->pkt_send_count++;
        TLY_LOG_D("Realtime report sent: %u frames, %u bytes", frame_count, data_pos);
    }
    free(data_buf);
    return rc;
}
#endif
static void on_report_timer(evutil_socket_t fd, short events, void *ctx)
{
    (void)fd;
    (void)events;
    tly_app_t *app = (tly_app_t *)ctx;

    if (!app->canid_received) return;

    /* 先组包: 构造数据单元 (时间 + 信息项) */
    uint8_t *data_buf = (uint8_t *)malloc(TLY_PKT_MAX_SIZE);
    if (!data_buf) return;

    uint32_t data_pos = 0;
    tly_time_now(data_buf);
    data_pos += 6;

    data_buf[data_pos++] = TLY_INFO_EV_FAST;
    tly_canid_list_resp_t *can = &app->canid_cache;
    uint8_t frame_count = (uint8_t)(can->fast_count > TLY_CANID_MAX
                                    ? TLY_CANID_MAX : can->fast_count);
    uint8_t *ev_start = data_buf + data_pos;
    // ev_start[0] = frame_count;
    ev_start[0] = 1;  // 一次上报一帧
    uint32_t ev_pos = 1;

    /* 获取当前时间（只获取一次），用于判断数据是否过期 */
    uint64_t current_time_ms = tly_monotonic_time_ms();

    for (int i = 0; i < frame_count; i++) {
        uint32_t cid = can->fast_ids[i].can_id;
        /* CAN ID 使用大端序（高字节在前） */
        ev_start[ev_pos++] = (uint8_t)(cid >> 24);
        ev_start[ev_pos++] = (uint8_t)(cid >> 16);
        ev_start[ev_pos++] = (uint8_t)(cid >> 8);
        ev_start[ev_pos++] = (uint8_t)(cid);
        uint8_t ch = can->fast_ids[i].channel;
        uint8_t dlc = 8;

        /* 先获取数据到临时缓冲区，传入当前时间用于超时检查 */
        uint8_t can_data[8] = {0};
        int ret = tsp_can_data_lookup(cid, ch, can_data, &dlc, current_time_ms);
        if (ret != 0) {
            /* 查找失败，说明该CAN ID未在缓存中找到或发生错误，DLC设为0 */
            dlc = 0;
        }
        dlc = (dlc > 8) ? 8 : dlc;

        /* DLC 字段只包含纯 DLC 值，不含 channel */
        ev_start[ev_pos++] = dlc;

        /* 根据 DLC 值写入数据，DLC=0 时不写入数据 */
        if (dlc > 0) {
            memcpy(&ev_start[ev_pos], can_data, dlc);
            ev_pos += dlc;
        }
    }

    /* ======================== 终端故障码信息 ======================== */

    /* 终端状态帧: CANID(4B) + 报文状态(1B) + 报文数据(8B) */
    uint32_t device_can_id = 0x18FED8FC;
    ev_start[ev_pos++] = (uint8_t)(device_can_id >> 24);  /* 高字节 */
    ev_start[ev_pos++] = (uint8_t)(device_can_id >> 16);
    ev_start[ev_pos++] = (uint8_t)(device_can_id >> 8);
    ev_start[ev_pos++] = (uint8_t)(device_can_id);        /* 低字节 */

    /* 报文状态: 0=未采集到报文, 8=采集到有效值, 固定为8 */
    ev_start[ev_pos++] = 8;

    /* 原始报文8字节，按位表示各故障状态 */
    uint8_t *dev_frame = ev_start + ev_pos;
    memset(dev_frame, 0, 8);

    /*
     * bit0: 蓄电池连接状态故障 (默认1=故障，等待后续传递)
     * bit1: 全球导航卫星系统模块工作故障
     * bit2: 全球导航卫星系统天线故障
     * bit3: 全球导航卫星系统定位故障
     * bit4: 移动通讯模块工作状态故障
     * bit5: 移动通讯模块主天线故障
     * bit6: 物联网卡工作故障 (默认0=正常)
     * bit7: 证书存储故障 (默认0=正常)
     * bit8: 设备证书有效状态故障 (默认0=正常)
     * bit9-11: VIN锁存状态 (默认0，等待后续传递)
     * bit12-63: 默认为0
     */
    uint64_t status_bits = 0;

    /* bit0: 蓄电池连接状态故障 (默认1) */
    status_bits |= (1ULL << 0);

    /* bit9-11: VIN锁存状态 (默认0) */
    status_bits |= ((uint64_t)0 << 9);

    /* bit1: 全球导航卫星系统模块工作故障 - 从GNSS获取 */
    /* bit2: 全球导航卫星系统天线故障 - 从GNSS获取 */
    /* bit3: 全球导航卫星系统定位故障 - 从GNSS获取 */
    GnssLocation_t gnss_loc;
    if (CommonMsg_GetGnssLocation(&gnss_loc) == 0) {
        /* 模块状态 0：工作正常 1：工作异常 */
        if (gnss_loc.status != 0) {
            status_bits |= (1ULL << 1);  /* 模块工作故障 */
        }
        /* 天线状态 0: 正常 非0: 故障 */
        if (gnss_loc.antStatus != 0) {
            status_bits |= (1ULL << 2);  /* 定位天线故障 */
        }
        /* 定位状态Bit0-5: 0=未定位 非0=已定位 */
        if ((gnss_loc.gpsState & 0x3F) == 0) {
            status_bits |= (1ULL << 3);  /* 定位故障 */
        }
    }

    /* bit4: 移动通讯模块工作状态故障 - 从Modem获取 */
    /* bit5: 移动通讯模块主天线故障 - 从Modem获取 */
    ModemSrvNotify_t modem_info;
    if (CommonMsg_GetModemSrvInfo(&modem_info) == 0) {
        /* 驻网状态 0：驻网成功 非0：驻网失败 */
        if (modem_info.dataRegStatus != 0) {
            status_bits |= (1ULL << 4);  /* 驻网失败 */
        }
        /* 主天线状态 0: 正常 非0: 故障 */
        if (modem_info.antennaStatus != 0) {
            status_bits |= (1ULL << 5);  /* 主天线故障 */
        }
    }

    /* 大端序写入8字节 */
    for (int i = 0; i < 8; i++) {
        dev_frame[i] = (uint8_t)(status_bits >> (56 - i * 8));
    }

    ev_pos += 8;  /* 原始报文8字节 */

    data_pos += ev_pos;

    /* 组包完成后判断连接状态 */
    if (app->logged_in && tly_tcp_is_connected(app->tcp)) {
        /* 已连接: 发送实时报文 */
        tly_packet_t pkt;
        memset(&pkt, 0, sizeof(pkt));
        pkt.cmd_id       = TLY_CMD_REALTIME_REPORT;
        snprintf(pkt.vin, sizeof(pkt.vin), "%s", app->config.vin);
        pkt.sw_version   = app->config.sw_version;
        pkt.encrypt_mode = TLY_ENCRYPT_NONE;
        pkt.data_len     = data_pos;
        pkt.data         = data_buf;

        int rc = tly_tcp_send_packet(app->tcp, &pkt);
        if (rc == 0) {
            app->pkt_send_count++;
            TLY_LOG_D("Realtime report sent: %u frames, %u bytes", frame_count, data_pos);
        }

        /* 打印周期上报原始hex */
        uint8_t raw_buf[TLY_PKT_MAX_SIZE];
        uint32_t raw_len = 0;
        if (tly_packet_encode(&pkt, raw_buf, &raw_len) == 0) {
            TLY_LOG_Dump("REALTIME PKT RAW:", raw_buf, raw_len);
        }
    } else {
        /* 未连接: 存储到本地补发 */
        tly_offline_db_save(TLY_DB_TYPE_REALTIME, data_buf, data_pos, TLY_CMD_REALTIME_REPORT);
    }
    free(data_buf);
}

static void tly_app_start_report_timer(tly_app_t *app)
{
    if (app->ev_report) {
        event_del(app->ev_report);
        event_free(app->ev_report);
        app->ev_report = NULL;
    }

    struct timeval tv = {TLY_REPORT_INTERVAL_SEC, 0};
    app->ev_report = event_new(app->evbase, -1,
                               EV_TIMEOUT | EV_PERSIST,
                               on_report_timer, app);
    if (app->ev_report) {
        evtimer_add(app->ev_report, &tv);
        TLY_LOG_I("Realtime report timer started (%ds)", TLY_REPORT_INTERVAL_SEC);
    }
}

/* ======================== 故障检测 ======================== */

#define TLY_FAULT_CHECK_INTERVAL_SEC  1  /* 故障检测周期：1秒 */
#define TLY_FAULT_LEVEL_THRESHOLD     2  /* 故障等级阈值：≥2触发 */

/** @brief 故障状态：0=正常，1=故障中（已触发存储，等待恢复） */
static uint8_t g_fault_active = 0;

static void on_fault_check_timer(evutil_socket_t fd, short events, void *ctx)
{
    (void)fd;
    (void)events;
    tly_app_t *app = (tly_app_t *)ctx;

    /* 获取CAN信号数据 */
    TonlyAllCanSignal_canVal_t can_signal;
    if (GetCanSignalData(&can_signal) != 0) {
        return;
    }

    /* 检查故障等级 */
    if (can_signal.max_error_level >= TLY_FAULT_LEVEL_THRESHOLD) {
        /* 故障等级≥2 */
        if (!g_fault_active && tsp_can_data_get_fault_storage_state() == FAULT_STORAGE_IDLE) {
            /* 故障刚发生，且未在存储中，触发故障存储 */
            /* 注意：目录创建和文件名生成已在tsp_can_data.c中自动处理，支持512M上限和自动清理 */
            
            TLY_LOG_I("FAULT: detected fault level %u, starting fault storage",
                      can_signal.max_error_level);
            
            int ret = tsp_can_data_start_fault_storage(NULL);  /* 传入NULL，使用内部自动生成的文件名 */
            if (ret == 0) {
                g_fault_active = 1;  /* 标记故障状态 */
            } else {
                TLY_LOG_E("FAULT: failed to start fault storage, ret=%d", ret);
            }
        }
        /* 如果 g_fault_active=1 或正在存储中，不重复触发 */
    } else {
        /* 故障等级<2，故障恢复 */
        if (g_fault_active) {
            TLY_LOG_I("FAULT: fault cleared, level=%u", can_signal.max_error_level);
            g_fault_active = 0;  /* 清除故障状态，允许下次触发 */
        }
    }
}

static void start_fault_check_timer(tly_app_t *app)
{
    if (app->ev_fault_check) {
        event_del(app->ev_fault_check);
        event_free(app->ev_fault_check);
        app->ev_fault_check = NULL;
    }

    struct timeval tv = {TLY_FAULT_CHECK_INTERVAL_SEC, 0};
    app->ev_fault_check = event_new(app->evbase, -1,
                                    EV_TIMEOUT | EV_PERSIST,
                                    on_fault_check_timer, app);
    if (app->ev_fault_check) {
        evtimer_add(app->ev_fault_check, &tv);
        TLY_LOG_I("Fault check timer started (%ds)", TLY_FAULT_CHECK_INTERVAL_SEC);
    }
}

/* ======================== 离线补发 ======================== */

#define TLY_REISSUE_INTERVAL_SEC  2
#define TLY_OFFLINE_CLEANUP_INTERVAL_SEC  3600  /* 每小时清理一次过期数据 */

static void on_reissue_timer(evutil_socket_t fd, short events, void *ctx)
{
    (void)fd;
    (void)events;
    tly_app_t *app = (tly_app_t *)ctx;
    static time_t last_cleanup_time = 0;

    if (!app->logged_in || !tly_tcp_is_connected(app->tcp)) return;

    /* 定期清理过期数据(每小时一次)，使用单调时间避免系统时间跳变影响 */
    time_t now = tly_monotonic_time_sec();
    if (last_cleanup_time == 0) {
        last_cleanup_time = now;
    } else if (now - last_cleanup_time >= TLY_OFFLINE_CLEANUP_INTERVAL_SEC) {
        int deleted = tly_offline_db_cleanup();
        if (deleted > 0) {
            TLY_LOG_I("Periodic cleanup: deleted %d expired offline records", deleted);
        }
        last_cleanup_time = now;
    }

    tly_offline_db_record_t record;
    tly_db_type_t db_type;

    uint8_t *buf = (uint8_t *)malloc(TLY_PKT_MAX_SIZE);
    if (!buf) return;

    int rc = tly_offline_db_pop_oldest(&record, buf, TLY_PKT_MAX_SIZE, &db_type);
    if (rc != 0) {
        /* 无待发记录，停止补发定时器 */
        free(buf);
        if (app->ev_reissue) {
            event_del(app->ev_reissue);
            event_free(app->ev_reissue);
            app->ev_reissue = NULL;
        }
        TLY_LOG_I("Reissue complete: all offline records sent");
        return;
    }

    tly_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd_id       = TLY_CMD_REISSUE_REPORT;
    snprintf(pkt.vin, sizeof(pkt.vin), "%s", app->config.vin);
    pkt.sw_version   = app->config.sw_version;
    pkt.encrypt_mode = TLY_ENCRYPT_NONE;
    pkt.data_len     = record.data_len;
    pkt.data         = buf;

    rc = tly_tcp_send_packet(app->tcp, &pkt);
    if (rc == 0) {
        app->pkt_send_count++;
        TLY_LOG_I("Reissue sent: %s db id=%lld (%u bytes)",
                  db_type == TLY_DB_TYPE_ALARM ? "alarm" : "realtime",
                  (long long)record.id, record.data_len);
        tly_offline_db_delete(db_type, record.id);
    } else {
        TLY_LOG_W("Reissue send failed: %s db id=%lld",
                  db_type == TLY_DB_TYPE_ALARM ? "alarm" : "realtime",
                  (long long)record.id);
    }

    free(buf);
}
/* ======================== 拆车报警检测 ======================== */

/* 拆车报警检测阈值 */
#define REMOVE_ALARM_KL30_THRESHOLD     100     /* KL30电压阈值，单位：mV */
#define REMOVE_ALARM_CONFIRM_TIMES      2       /* 需要连续检测次数 */

/**
 * 拆车报警检测回调函数
 * 周期性检测拆车条件：KL15==2（1：下电  2：上电） && KL30==1
 * KL30==1需要连续两次检测（4秒）才触发报警
 */
static void on_remove_alarm_check(evutil_socket_t fd, short events, void *ctx)
{
    (void)fd;
    (void)events;
    tly_app_t *app = (tly_app_t *)ctx;
    if (!app) return;

    /* 读取硬线状态（KL15和KL30） */
    HardLineSyncInfo_t hardline_info;
    int kl15_status = 2;
    int kl30_status = 2;
    
    if (CommonMsg_GetHardLineInfo(&hardline_info) == 0) {
        kl15_status = hardline_info.kl15status;
        kl30_status = hardline_info.kl30status;
    } else {
        TLY_LOG_W("Failed to get hardline info");
    }

    // TLY_LOG_D("Remove alarm check: KL15=%d, KL30=%d, confirm_cnt=%d, detected=%d",
    //           kl15_status, kl30_status, app->remove_alarm_confirm_cnt, app->remove_alarm_detected);

    /* 检测条件：KL15==2 && KL30==1 */
    if (kl15_status == 2 && kl30_status == 1)
    {
        /* 条件满足，如果之前已触发过，说明是新的触发周期，重置状态 */
        if (app->remove_alarm_detected) {
            /* 已经在触发状态，等待条件恢复后再触发 */
            return;
        }

        /* KL30==1时需要连续两次确认 */
        if (kl30_status == 1)
        {
            app->remove_alarm_confirm_cnt++;
            TLY_LOG_W("KL30 low voltage detected (status=%d), confirm count: %d/%d",
                      kl30_status, app->remove_alarm_confirm_cnt, REMOVE_ALARM_CONFIRM_TIMES);

            /* 连续两次检测都满足条件（4秒） */
            if (app->remove_alarm_confirm_cnt >= REMOVE_ALARM_CONFIRM_TIMES) {
                TLY_LOG_W("Tamper confirmed! Sending remove alarm...");
                
                /* 获取当前位置 */
                GnssLocation_t gnss_loc;
                uint32_t longitude = 0;
                uint32_t latitude = 0;
                uint8_t status = 0x00;
                
                if (CommonMsg_GetGnssLocation(&gnss_loc) == 0) {
                    /* 经度纬度单位是度，精度10^-7，转换为协议格式 */
                    longitude = gnss_loc.longtitudeValue / 10; /* 转换为10^-6度 */
                    latitude = gnss_loc.latitudeValue / 10;   /* 转换为10^-6度 */
                    /* 构建定位状态位（拆除报警协议格式） */
                    /* bit0: 0=有效定位, 1=无效定位（与直觉相反！） */
                    /* bit1: 0=东经, 1=西经 */
                    /* bit2: 0=北纬, 1=南纬 */
                    /* bit3: 0=连接, 1=断开（拆车报警应设为1） */
                    /* GNSS gpsState: Bit0-5=定位状态, Bit6=西经, Bit7=南纬 */
                    
                    /* bit0: 定位状态（0=有效, 1=无效） */
                    if (!(gnss_loc.gpsState & 0x3F)) {
                        status |= 0x01;  // 未定位时设为1（无效）
                    }
                    /* bit1: 东西经（1=西经） */
                    if (gnss_loc.gpsState & 0x40) {
                        status |= 0x02;  // 西经
                    }
                    /* bit2: 南北纬（1=南纬） */
                    if (gnss_loc.gpsState & 0x80) {
                        status |= 0x04;  // 南纬
                    }
                    /* bit3: 连接状态（1=断开） */
                    status |= 0x08;  // 拆车报警，设为断开状态
                } else {
                    TLY_LOG_W("Failed to get GNSS location, using default values");
                }
                
                /* 构造拆车报警数据 */
                uint8_t data_buf[20];
                uint32_t data_len = 0;
                tly_remove_alarm_t alarm;
                memset(&alarm, 0, sizeof(alarm));
                tly_time_now(alarm.time);
                app->alarm_seq++;
                if (app->alarm_seq > 65531) app->alarm_seq = 1;
                alarm.seq       = app->alarm_seq;
                alarm.status    = status;
                alarm.longitude = longitude;
                alarm.latitude  = latitude;
                
                int encode_rc = tly_encode_remove_alarm(&alarm, data_buf, &data_len);
                if (encode_rc != 0) {
                    TLY_LOG_E("Failed to encode remove alarm: %d", encode_rc);
                } else {
                    /* 发送拆车报警 */
                    int rc = tly_app_send_remove_alarm(app, status, longitude, latitude);
                    if (rc == 0) {
                        TLY_LOG_I("Remove alarm sent successfully");
                    } else {
                        TLY_LOG_E("Failed to send remove alarm: %d", rc);
                        /* 发送失败，存储已构造好的数据(触发报警数据库，高优先级) */
                        tly_offline_db_save(TLY_DB_TYPE_ALARM, data_buf, data_len, TLY_CMD_REMOVE_ALARM);
                        TLY_LOG_I("Remove alarm saved to alarm db (high priority)");
                    }
                    /* 无论发送成功还是失败，都标记为已触发，避免重复上报 */
                    app->remove_alarm_detected = 1;
                }
            }
        }
    } else {
        /* 条件不满足，重置所有状态，允许下次触发 */
        if (app->remove_alarm_confirm_cnt > 0 || app->remove_alarm_detected) {
            TLY_LOG_D("Condition not met, reset alarm state");
            app->remove_alarm_confirm_cnt = 0;
            app->remove_alarm_detected = 0;  // 重置触发标志，允许再次触发
        }
    }
}

/**
 * 启动拆车报警检测定时器
 * @return 0成功, 负数失败
 */
static int tly_app_start_remove_alarm_check(tly_app_t *app)
{
    if (!app || !app->evbase) return TLY_ERR_PARAM;
    
    /* 如果已经在运行，先停止 */
    if (app->ev_remove_alarm_check) {
        tly_app_stop_remove_alarm_check(app);
    }
    
    /* 创建定时器，固定2秒间隔 */
    struct timeval tv = {2, 0};
    app->ev_remove_alarm_check = event_new(app->evbase, -1,
                                           EV_TIMEOUT | EV_PERSIST,
                                           on_remove_alarm_check, app);
    if (!app->ev_remove_alarm_check) {
        TLY_LOG_E("Failed to create remove alarm check timer");
        return TLY_ERR_MEMORY;
    }
    
    /* 添加到事件循环 */
    if (evtimer_add(app->ev_remove_alarm_check, &tv) != 0) {
        TLY_LOG_E("Failed to add remove alarm check timer");
        event_free(app->ev_remove_alarm_check);
        app->ev_remove_alarm_check = NULL;
        return TLY_ERR_PARAM;
    }
    
    app->remove_alarm_detected = 0;      // 重置检测状态
    app->remove_alarm_confirm_cnt = 0;   // 重置确认计数器
    TLY_LOG_I("Remove alarm check started (interval=2s)");
    return 0;
}

/**
 * 停止拆车报警检测定时器
 */
static void tly_app_stop_remove_alarm_check(tly_app_t *app)
{
    if (!app) return;
    
    if (app->ev_remove_alarm_check) {
        event_del(app->ev_remove_alarm_check);
        event_free(app->ev_remove_alarm_check);
        app->ev_remove_alarm_check = NULL;
    }
    
    TLY_LOG_I("Remove alarm check stopped");
}

static void tly_app_start_reissue(tly_app_t *app)
{
    tly_offline_db_stats_t stats;
    if (tly_offline_db_get_stats(&stats) != 0) {
        return;
    }

    uint32_t total_count = stats.alarm_count + stats.realtime_count;
    if (total_count == 0) {
        TLY_LOG_I("No offline records to reissue");
        return;
    }

    TLY_LOG_I("Starting reissue: %u pending records (alarm=%u, realtime=%u)",
              total_count, stats.alarm_count, stats.realtime_count);

    if (app->ev_reissue) {
        event_del(app->ev_reissue);
        event_free(app->ev_reissue);
    }

    struct timeval tv = {TLY_REISSUE_INTERVAL_SEC, 0};
    app->ev_reissue = event_new(app->evbase, -1,
                                EV_TIMEOUT | EV_PERSIST,
                                on_reissue_timer, app);
    if (app->ev_reissue) {
        evtimer_add(app->ev_reissue, &tv);
    }
}

/* ======================== 车辆登出 ======================== */

int tly_app_send_vehicle_logout(tly_app_t *app)
{
    if (!app || !tly_tcp_is_connected(app->tcp)) return TLY_ERR_CLOSED;

    tly_vehicle_logout_t logout;
    memset(&logout, 0, sizeof(logout));

    tly_time_now(logout.time);

    app->login_seq++;
    if (app->login_seq > 65531) app->login_seq = 1;
    logout.seq = app->login_seq;

    uint8_t data_buf[16];
    uint32_t data_len = 0;
    int rc = tly_encode_vehicle_logout(&logout, data_buf, &data_len);
    if (rc != 0) {
        TLY_LOG_E("Failed to encode vehicle logout: %d", rc);
        return rc;
    }

    tly_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd_id       = TLY_CMD_VEHICLE_LOGOUT;
    snprintf(pkt.vin, sizeof(pkt.vin), "%s", app->config.vin);
    pkt.sw_version   = app->config.sw_version;
    pkt.encrypt_mode = TLY_ENCRYPT_NONE;
    pkt.data_len     = data_len;
    pkt.data         = data_buf;

    rc = tly_tcp_send_packet(app->tcp, &pkt);
    if (rc == 0) {
        app->pkt_send_count++;
        TLY_LOG_I("Vehicle logout sent: seq=%u", logout.seq);
    } else {
        TLY_LOG_E("Vehicle logout send failed: %d", rc);
    }

    uint8_t raw_buf[TLY_PKT_MAX_SIZE];
    uint32_t raw_len = 0;
    if (tly_packet_encode(&pkt, raw_buf, &raw_len) == 0) {
        TLY_LOG_Dump("LOGOUT PKT RAW:", raw_buf, raw_len);
    }

    return rc;
}

/* ======================== 心跳 ======================== */

int tly_app_send_heartbeat(tly_app_t *app)
{
    if (!app || !tly_tcp_is_connected(app->tcp))
        return TLY_ERR_CLOSED;

    tly_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd_id       = TLY_CMD_HEARTBEAT;
    snprintf(pkt.vin, sizeof(pkt.vin), "%s", app->config.vin);
    pkt.sw_version   = app->config.sw_version;
    pkt.encrypt_mode = TLY_ENCRYPT_NONE;
    pkt.data_len     = 0;
    pkt.data         = NULL;

    int rc = tly_tcp_send_packet(app->tcp, &pkt);
    if (rc == 0) {
        app->pkt_send_count++;
        TLY_LOG_D("Heartbeat sent");
    }
    return rc;
}

/* ======================== 位置心跳 ======================== */

static int tly_app_send_wakeup_location(tly_app_t *app)
{
    if (!app || !tly_tcp_is_connected(app->tcp))
        return TLY_ERR_CLOSED;

    /* 获取GNSS和Modem数据 */
    GnssLocation_t gnss_loc;
    memset(&gnss_loc, 0, sizeof(gnss_loc));
    (void)CommonMsg_GetGnssLocation(&gnss_loc);

    ModemSrvNotify_t modem_info;
    memset(&modem_info, 0, sizeof(modem_info));
    (void)CommonMsg_GetModemSrvInfo(&modem_info);

    /* 构造数据单元: 21字节
     * [0]     GPS定位状态(1B): bit0=定位状态, bit1=南北纬, bit2=东西经
     * [1-4]   经度(4B, BE): ×10^7, gnss.longtitudeValue/10
     * [5-8]   纬度(4B, BE): ×10^7, gnss.latitudeValue/10
     * [9-10]  高程(2B, BE): 米, gnss.seaHeight/10
     * [11]    速度(1B): km/h, gnss.speed/10
     * [12-13] 航向(2B, BE): 度, gnss.direction/10
     * [14-15] X轴加速度(2B, BE): 默认0
     * [16-17] Y轴加速度(2B, BE): 默认0
     * [18-19] Z轴加速度(2B, BE): 默认0
     * [20]    天线状态(1B): gnss.antStatus
     */
    tly_wakeup_location_t loc;
    memset(&loc, 0, sizeof(loc));

    /* [0] GPS定位状态: bit0=无效, bit1=南纬, bit2=西经 */
    if ((gnss_loc.gpsState & 0x3F) == 0) {
        loc.gps_status |= 0x01;  /* 未定位 */
    }
    if (gnss_loc.gpsState & 0x80) loc.gps_status |= 0x02;  /* 南纬 */
    if (gnss_loc.gpsState & 0x40) loc.gps_status |= 0x04;  /* 西经 */

    /* gnss_loc中的longtitudeValue、latitudeValue精度为10的-7次方 */
    loc.longitude = gnss_loc.longtitudeValue / 10;
    loc.latitude  = gnss_loc.latitudeValue / 10;
    
    /* gnss_loc中的seaHeight精度为0.1 */
    loc.altitude  = gnss_loc.seaHeight / 10;

    /* gnss_loc中的speed精度为0.1 */
    loc.speed     = gnss_loc.speed / 10;

    /* gnss_loc中的direction精度为0.1 */
    loc.heading   = gnss_loc.direction / 10;

    loc.accel_x  = 0;
    loc.accel_y  = 0;
    loc.accel_z  = 0;

    uint8_t signal = modem_info.csq;
    uint8_t ant_status = 0;
    if (gnss_loc.antStatus & 0x01) 
        ant_status |= 0x02;
    else if ((gnss_loc.antStatus & 0x02) || (gnss_loc.antStatus & 0x03)) 
        ant_status |= 0x01;
    
    if (signal > 31) signal = 31;
    loc.dev_status = (ant_status & 0x07) | (signal << 3);

    /* 调用协议层编码 */
    uint8_t data_buf[32];
    uint32_t data_len = 0;
    int rc = tly_encode_wakeup_location(&loc, data_buf, &data_len);
    if (rc != 0) {
        TLY_LOG_E("Failed to encode wakeup location: %d", rc);
        return rc;
    }

    /* 构造报文并发送 */
    tly_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd_id       = TLY_CMD_WAKEUP_LOCATION;
    snprintf(pkt.vin, sizeof(pkt.vin), "%s", app->config.vin);
    pkt.sw_version   = app->config.sw_version;
    pkt.encrypt_mode = TLY_ENCRYPT_NONE;
    pkt.data_len     = data_len;
    pkt.data         = data_buf;

    rc = tly_tcp_send_packet(app->tcp, &pkt);
    if (rc == 0) {
        app->pkt_send_count++;
        TLY_LOG_D("Wakeup location sent: gpsStatus=0x%02X lon=%u lat=%u",
                  loc.gps_status, loc.longitude, loc.latitude);
    }
    return rc;
}

int tly_app_get_vin(tly_app_t *app)
{
    if (!app || !tly_tcp_is_connected(app->tcp))
        return TLY_ERR_CLOSED;

    tly_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd_id       = TLY_CMD_VIN_REQUEST;
    snprintf(pkt.vin, sizeof(pkt.vin), "%s", app->config.vin);
    pkt.sw_version   = app->config.sw_version;
    pkt.encrypt_mode = TLY_ENCRYPT_NONE;
    pkt.data_len     = 0;
    pkt.data         = NULL;

    int rc = tly_tcp_send_packet(app->tcp, &pkt);
    if (rc == 0) {
        app->pkt_send_count++;
        TLY_LOG_D("VIN request sent");
    }
    return rc;
}

int tly_app_send_remove_alarm(tly_app_t *app,
                              uint8_t status,
                              uint32_t longitude,
                              uint32_t latitude)
{
    if (!app || !tly_tcp_is_connected(app->tcp))
        return TLY_ERR_CLOSED;

    tly_remove_alarm_t alarm;
    memset(&alarm, 0, sizeof(alarm));

    tly_time_now(alarm.time);

    app->alarm_seq++;
    if (app->alarm_seq > 65531) app->alarm_seq = 1;
    alarm.seq       = app->alarm_seq;
    alarm.status    = status;
    alarm.longitude = longitude;
    alarm.latitude  = latitude;

    uint8_t data_buf[20];
    uint32_t data_len = 0;
    int rc = tly_encode_remove_alarm(&alarm, data_buf, &data_len);
    if (rc != 0) {
        TLY_LOG_E("Failed to encode remove alarm: %d", rc);
        return rc;
    }

    tly_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.cmd_id       = TLY_CMD_REMOVE_ALARM;
    snprintf(pkt.vin, sizeof(pkt.vin), "%s", app->config.vin);
    pkt.sw_version   = app->config.sw_version;
    pkt.encrypt_mode = TLY_ENCRYPT_NONE;
    pkt.data_len     = data_len;
    pkt.data         = data_buf;

    rc = tly_tcp_send_packet(app->tcp, &pkt);
    if (rc == 0) {
        app->pkt_send_count++;
        double lon = (double)longitude / 1000000.0;
        double lat = (double)latitude  / 1000000.0;
        TLY_LOG_W("Remove alarm sent: seq=%u, status=0x%02X, lon=%.6f, lat=%.6f",
                  alarm.seq, status, lon, lat);
    } else {
        TLY_LOG_E("Remove alarm send failed: %d", rc);
    }

    uint8_t raw_buf[TLY_PKT_MAX_SIZE];
    uint32_t raw_len = 0;
    if (tly_packet_encode(&pkt, raw_buf, &raw_len) == 0) {
        TLY_LOG_Dump("REMOVE ALARM PKT RAW:", raw_buf, raw_len);
    }

    return rc;
}

int tly_app_send_response(tly_app_t *app, uint8_t cmd_id,
                          uint8_t resp_flag,
                          const uint8_t *data, uint32_t data_len)
{
    if (!app)
        return TLY_ERR_PARAM;

    uint8_t buf[TLY_PKT_MAX_SIZE];
    uint32_t len = 0;

    int rc = tly_encode_response(cmd_id, app->config.vin,
                                 resp_flag, app->config.sw_version,
                                 TLY_ENCRYPT_NONE,
                                 data, data_len, buf, &len);
    if (rc != 0) return rc;

    rc = tly_tcp_send(app->tcp, buf, len);
    if (rc == 0) app->pkt_send_count++;
    return rc;
}

/* ======================== 远控业务处理 ======================== */

static int tly_app_handle_lock_unlock(tly_app_t *app, const tly_lock_control_t *lock)
{
    TLY_LOG_I("Remote UNLOCK: engine=0x%02X", lock->ctrl_cmd);
    send_remote_cmd(app, lock);
    return 0;
}

static int tly_app_handle_lock_lock(tly_app_t *app, const tly_lock_control_t *lock)
{
    TLY_LOG_I("Remote LOCK: engine=0x%02X, rpm=%u", lock->ctrl_cmd, lock->rpm);
    send_remote_cmd(app, lock);
    return 0;
}

static int tly_app_handle_lock_limit_torque(tly_app_t *app, const tly_lock_control_t *lock)
{
    TLY_LOG_I("Remote LIMIT_TORQUE: engine=0x%02X, torque=%u, rpm=%u",
              lock->ctrl_cmd, lock->torque, lock->rpm);
    send_remote_cmd(app, lock);
    return 0;
}

static int tly_app_handle_lock_activate(tly_app_t *app, const tly_lock_control_t *lock)
{
    TLY_LOG_I("Remote ACTIVATE: engine=0x%02X", lock->ctrl_cmd);
    send_remote_cmd(app, lock);
    return 0;
}

static int tly_app_handle_lock_deactivate(tly_app_t *app, const tly_lock_control_t *lock)
{
    TLY_LOG_I("Remote DEACTIVATE: engine=0x%02X", lock->ctrl_cmd);
    send_remote_cmd(app, lock);
    return 0;
}

static int tly_app_handle_battery_heat(tly_app_t *app, const tly_battery_heat_t *heat)
{
    int8_t ret;
    
    if (!app || !heat) {
        TLY_LOG_E("Battery heat: invalid params");
        return -1;
    }
    
    if (heat->time_second < 3600 || heat->time_second > 18000) {
        TLY_LOG_W("Battery heat time_second %u out of range [3600-18000]", heat->time_second);
        return -1;
    }
    
    app->battery_heat_alarm_on = 1; /* 标记已接收到远控 */
    app->battery_heat_alarm_time_second = heat->time_second;
#if 0
    /* 创建RTC休眠闹钟 */
    if (app->battery_heat_alarm_id != 0) {
        /* 销毁已存在的闹钟 */
        ret = PmDestroySleepAlarm(app->battery_heat_alarm_id);
        TLY_LOG_D("Battery heat: destroy existing alarm, alarmId=%u, ret=%d",
                  app->battery_heat_alarm_id, ret);
        app->battery_heat_alarm_id = 0;
    }
    
    /* 创建RTC闹钟，使用APP层预留ID范围(10-30) */
    app->battery_heat_alarm_id = CPU_APP_TIMER_BATTERY_HEAT;
    ret = PmCreateSleepAlarm(app->battery_heat_alarm_id, heat->time_second, 0);
    if (ret != 0) {
        TLY_LOG_E("Battery heat: failed to create RTC alarm, alarmId=%u, duration=%u, ret=%d",
                  app->battery_heat_alarm_id, heat->time_second, ret);
        app->battery_heat_alarm_id = 0;
        return -1;
    }
    
    TLY_LOG_I("Battery heat scheduled: RTC alarmId=%u, duration=%u seconds",
              app->battery_heat_alarm_id, heat->time_second);

#endif
    return 0;
}

 /**
 * @brief 删除电池加热RTC闹钟
 * @param app 应用上下文
 */
void tly_app_delete_battery_heat_alarm(tly_app_t *app)
{
    int8_t ret;
 
    /* 销毁已存在的闹钟 */
    if (app->battery_heat_alarm_id != 0) { 
        ret = PmDestroySleepAlarm(app->battery_heat_alarm_id);
        TLY_LOG_D("Battery heat: delete battery heat alarm, alarmId=%u, ret=%d",
                   app->battery_heat_alarm_id, ret);
        app->battery_heat_alarm_id = 0;
    }
}

/**
 * 设置电池加热RTC闹钟
 * @param app 应用上下文
 * @return 0成功, 负数失败
 */
int tly_app_set_battery_heat_alarm(tly_app_t *app)
{
    int ret;
    uint32_t time_second;
    
    if (!app) {
        TLY_LOG_E("Battery heat alarm: invalid app");
        return -1;
    }
    
    time_second = app->battery_heat_alarm_time_second;
    if (time_second < 3600 || time_second > 18000) {
        TLY_LOG_W("Battery heat alarm: time_second %u out of range [3600-18000]", time_second);
        return -1;
    }
    
    /* 销毁已存在的闹钟 */
    if (app->battery_heat_alarm_id != 0) {
        ret = PmDestroySleepAlarm(app->battery_heat_alarm_id);
        TLY_LOG_D("Battery heat: destroy existing alarm, alarmId=%u, ret=%d",
                  app->battery_heat_alarm_id, ret);
        app->battery_heat_alarm_id = 0;
    }
    
    /* 创建RTC闹钟，使用APP层预留ID范围(10-30) */
    app->battery_heat_alarm_id = CPU_APP_TIMER_BATTERY_HEAT;
    ret = PmCreateSleepAlarm(app->battery_heat_alarm_id, time_second, 0);
    if (ret != 0) {
        TLY_LOG_E("Battery heat: failed to create RTC alarm, alarmId=%u, duration=%u, ret=%d",
                  app->battery_heat_alarm_id, time_second, ret);
        app->battery_heat_alarm_id = 0;
        return -1;
    }
    
    TLY_LOG_I("Battery heat: RTC alarm created, alarmId=%u, duration=%u seconds",
              app->battery_heat_alarm_id, time_second);
    
    return 0;
}

static int tly_app_handle_sd_card(tly_app_t *app, const tly_sd_card_t *sd)
{
    (void)app;
    if (sd->time_count > 600) {
        TLY_LOG_W("SD card time_count %u out of range [0-600]", sd->time_count);
        return -1;
    }
    return tly_device_sd_card_upload(sd->username, sd->password, sd->url, sd->port,
                                     sd->begin_time, sd->time_count);
}

static int tly_app_handle_vin_unbind(tly_app_t *app, const tly_vin_unbind_t *vu)
{
    (void)app;
    return tly_device_vin_unbind(vu->target_vin);
}

static int tly_app_handle_vin_bind(tly_app_t *app, const tly_vin_bind_t *vb)
{
    (void)app;
    return tly_device_vin_bind(vb->target_vin);
}

static int tly_app_handle_time_sync(tly_app_t *app, const tly_time_sync_t *ts)
{
    (void)app;
    if (ts->time_second < 300 || ts->time_second > 604800) {
        TLY_LOG_W("Time sync time_second %u out of range [300-604800]", ts->time_second);
        return -1;
    }
    return tly_device_time_sync(ts->time_second);
}

static void tly_app_dispatch_lock(tly_app_t *app, const tly_lock_control_t *lock)
{
    if (!app || !lock) return;

    int rc = 0;
    switch (lock->lock_type) {
    case TLY_LOCK_UNLOCK:
        rc = tly_app_handle_lock_unlock(app, lock);
        break;
    case TLY_LOCK_LOCK:
        rc = tly_app_handle_lock_lock(app, lock);
        break;
    case TLY_LOCK_LIMIT_TORQUE:
        rc = tly_app_handle_lock_limit_torque(app, lock);
        break;
    case TLY_LOCK_ACTIVATE:
        rc = tly_app_handle_lock_activate(app, lock);
        break;
    case TLY_LOCK_DEACTIVATE:
        rc = tly_app_handle_lock_deactivate(app, lock);
        break;
    default:
        TLY_LOG_W("Unknown lock type: %u", lock->lock_type);
        rc = -1;
        break;
    }

    if (rc != 0) {
        TLY_LOG_E("Lock operation failed: type=%u, engine=0x%02X", lock->lock_type, lock->ctrl_cmd);
    }
}

/* ======================== 数据包分发 ======================== */

static void handle_vehicle_login(tly_app_t *app, const tly_packet_t *pkt)
{
    /* byte 2 在响应包中是应答标志(0x01=成功), 与命令标识0x01(车辆登入)重叠
     * 用 logged_in 状态区分:
     *   未登入(logged_in==0) → 平台对我们登入请求的应答, byte 2 = 应答标志
     *   已登入(logged_in==1) → 平台转发的终端登入数据, byte 2 = 命令标识
     */

    if (!app->logged_in) {
        uint8_t resp = pkt->cmd_id;
        if (resp == TLY_RESP_SUCCESS) {
            TLY_LOG_I("Vehicle login accepted by platform");
            app->logged_in = 1;

            if (app->tcp_keepalive_alarm_id == CPU_APP_TIMER_TCP_KEEPALIVE) 
            {
                /* TSP长连接唤醒场景，发送位置心跳包 */
                tly_app_send_wakeup_location(app);
            }
            else
            {
                /* 正常连接情况（区别TSP长连接闹钟唤醒场景） */
                if (app->ev_heartbeat) {
                    event_del(app->ev_heartbeat);
                    event_free(app->ev_heartbeat);
                }

                struct timeval tv = {app->config.heartbeat_interval, 0};
                app->ev_heartbeat = event_new(app->evbase, -1,
                                              EV_TIMEOUT | EV_PERSIST,
                                              on_heartbeat, app);
                if (app->ev_heartbeat) {
                    evtimer_add(app->ev_heartbeat, &tv);
                }

                TLY_LOG_I("Heartbeat started (%ds)", app->config.heartbeat_interval);

                tly_app_send_version_upload(app);
                //tly_app_send_all_ecu_versions(app);
                tly_app_start_reissue(app);
            }
        } else {
            TLY_LOG_E("Vehicle login rejected: resp=0x%02X", resp);
            app->logged_in = 0;
        }
        return;
    }

    /* 平台转发的终端登入包 */
    tly_vehicle_login_t login;
    int rc = tly_decode_vehicle_login(pkt->data, pkt->data_len, &login);
    if (rc != 0) {
        TLY_LOG_W("Failed to decode vehicle login");
        return;
    }

    struct tm tm;
    tly_time_decode(login.time, &tm);
    TLY_LOG_I("Vehicle login: VIN=%.17s, seq=%u, ICCID=%.20s, time=%04d-%02d-%02d %02d:%02d:%02d",
              pkt->vin, login.seq, login.iccid,
              tm.tm_year, tm.tm_mon + 1, tm.tm_mday,
              tm.tm_hour, tm.tm_min, tm.tm_sec);

    tly_app_send_response(app, pkt->cmd_id, TLY_RESP_SUCCESS, NULL, 0);
}

static void handle_vehicle_logout(tly_app_t *app, const tly_packet_t *pkt)
{
    tly_vehicle_logout_t logout;
    int rc = tly_decode_vehicle_logout(pkt->data, pkt->data_len, &logout);
    if (rc != 0) {
        TLY_LOG_W("Failed to decode vehicle logout");
        return;
    }

    struct tm tm;
    tly_time_decode(logout.time, &tm);
    TLY_LOG_I("Vehicle logout: VIN=%.17s, seq=%u", pkt->vin, logout.seq);

    /* 切换为未登录状态，保持程序运行，等待平台重新登录 */
    app->logged_in = 0;

    if (app->ev_heartbeat) {
        event_del(app->ev_heartbeat);
        event_free(app->ev_heartbeat);
        app->ev_heartbeat = NULL;
    }
    if (app->ev_report) {
        event_del(app->ev_report);
        event_free(app->ev_report);
        app->ev_report = NULL;
    }
    if (app->ev_reissue) {
        event_del(app->ev_reissue);
        event_free(app->ev_reissue);
        app->ev_reissue = NULL;
    }
    if (app->ev_fault_check) {
        event_del(app->ev_fault_check);
        event_free(app->ev_fault_check);
        app->ev_fault_check = NULL;
    }
    tly_app_stop_remove_alarm_check(app);

    if (app->tcp && app->tcp->ev_reconnect) {
        event_del(app->tcp->ev_reconnect);
        event_free(app->tcp->ev_reconnect);
        app->tcp->ev_reconnect = NULL;
    }

    // tly_app_send_response(app, pkt->cmd_id, TLY_RESP_SUCCESS, NULL, 0);
}

static void handle_realtime_report(tly_app_t *app, const tly_packet_t *pkt)
{
    tly_realtime_report_t rpt;
    int rc = tly_decode_realtime_report(pkt->data, pkt->data_len, &rpt);
    if (rc != 0) {
        TLY_LOG_W("Failed to decode realtime report");
        return;
    }

    struct tm tm;
    tly_time_decode(rpt.time, &tm);
    TLY_LOG_I("Realtime report: VIN=%.17s, items=%u, time=%04d-%02d-%02d %02d:%02d:%02d",
              pkt->vin, rpt.item_count,
              tm.tm_year, tm.tm_mon + 1, tm.tm_mday,
              tm.tm_hour, tm.tm_min, tm.tm_sec);

    /* 遍历信息项 */
    for (uint32_t i = 0; i < rpt.item_count; i++) {
        tly_info_item_t *item = &rpt.items[i];

        switch (item->info_id) {
        case TLY_INFO_LOCATION: {
            tly_location_t loc;
            if (tly_decode_location(item->info_data, item->info_len, &loc) == 0) {
                double lon = (double)loc.longitude / 1000000.0;
                double lat = (double)loc.latitude  / 1000000.0;
                TLY_LOG_I("  Location: lon=%.6f, lat=%.6f, alt=%dm, speed=%dkm/h, heading=%u",
                          lon, lat, loc.altitude, loc.gps_speed, loc.heading);
            }
            break;
        }
        case TLY_INFO_FAST:
            TLY_LOG_I("  Fast CAN data: %u bytes", item->info_len);
            break;
        case TLY_INFO_SLOW:
            TLY_LOG_I("  Slow CAN data: %u bytes", item->info_len);
            break;
        case TLY_INFO_EV_FAST: {
            tly_ev_fast_packet_t ev_pkt;
            if (tly_decode_ev_fast_packet(item->info_data, item->info_len, &ev_pkt) == 0) {
                tly_ev_fast_packet_dump(&ev_pkt);
            } else {
                TLY_LOG_W("  EV fast CAN decode failed, raw %u bytes", item->info_len);
            }
            break;
        }
        case TLY_INFO_EV_SLOW:
            TLY_LOG_I("  EV slow CAN data: %u bytes", item->info_len);
            break;
        case TLY_INFO_ALARM:
            TLY_LOG_I("  Alarm data: %u bytes", item->info_len);
            break;
        case TLY_INFO_FAULT_CODE:
            TLY_LOG_I("  Fault code: %u bytes", item->info_len);
            break;
        case TLY_INFO_TIRE_PRESSURE:
            TLY_LOG_I("  Tire pressure: %u bytes", item->info_len);
            break;
        case TLY_INFO_LOCK_STATUS:
            TLY_LOG_I("  Lock status: %u bytes", item->info_len);
            break;
        default:
            TLY_LOG_I("  Unknown info 0x%02X: %u bytes", item->info_id, item->info_len);
            break;
        }
    }

    tly_app_send_response(app, pkt->cmd_id, TLY_RESP_SUCCESS, NULL, 0);
    tly_realtime_report_free(&rpt);
}

static void handle_remove_alarm(tly_app_t *app, const tly_packet_t *pkt)
{
    tly_remove_alarm_t alarm;
    int rc = tly_decode_remove_alarm(pkt->data, pkt->data_len, &alarm);
    if (rc != 0) {
        TLY_LOG_W("Failed to decode remove alarm");
        return;
    }

    double lon = (double)alarm.longitude / 1000000.0;
    double lat = (double)alarm.latitude  / 1000000.0;
    TLY_LOG_W("REMOVE ALARM: VIN=%.17s, seq=%u, status=0x%02X, lon=%.6f, lat=%.6f",
              pkt->vin, alarm.seq, alarm.status, lon, lat);

    tly_app_send_response(app, pkt->cmd_id, TLY_RESP_SUCCESS, NULL, 0);
}

static void handle_time_sync(tly_app_t *app, const tly_packet_t *pkt)
{
    if (pkt->data_len == 0) {
        /* 终端请求校时 → 返回当前时间 */
        TLY_LOG_I("Time sync request from VIN=%.17s", pkt->vin);
        uint8_t time_buf[6];
        tly_time_now(time_buf);
        tly_app_send_response(app, pkt->cmd_id, TLY_RESP_SUCCESS, time_buf, 6);
    } else {
        /* 平台返回校时结果 */
        struct tm tm;
        tly_time_decode(pkt->data, &tm);
        TLY_LOG_I("Time sync response: %04d-%02d-%02d %02d:%02d:%02d",
                  tm.tm_year, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
}

void tly_app_dispatch_packet(tly_app_t *app, const tly_packet_t *pkt)
{
    if (!app || !pkt) return;

    app->pkt_recv_count++;
    app->state = TLY_STATE_RUNNING;

    if (!app->logged_in) {
        TLY_LOG_I("Dispatch: resp_flag=0x%02X (response), VIN=%.17s, data_len=%u",
                  pkt->cmd_id, pkt->vin, pkt->data_len);
    } else {
        TLY_LOG_I("Dispatch: cmd=0x%02X, VIN=%.17s, data_len=%u",
                  pkt->cmd_id, pkt->vin, pkt->data_len);
    }

    if (pkt->data_len > 0 && pkt->data) {
        TLY_LOG_Dump("RECV DATA:", pkt->data, pkt->data_len);
    }

    /* byte 2 在应答标志范围(0x01-0x03)内且已登入 → 通用应答, 不走命令分发 */
    if (app->logged_in && pkt->cmd_id >= 0x01 && pkt->cmd_id <= 0x03) {
        TLY_LOG_D("Response ack: cmd=0x%02X, resp=%s",
                  pkt->cmd_id,
                  pkt->cmd_id == TLY_RESP_SUCCESS ? "SUCCESS" : "FAIL");
        return;
    }

    switch (pkt->cmd_id) {
    case TLY_CMD_VEHICLE_LOGIN:
        handle_vehicle_login(app, pkt);
        break;
    case TLY_CMD_REALTIME_REPORT:
        handle_realtime_report(app, pkt);
        break;
    case TLY_CMD_REISSUE_REPORT:
        TLY_LOG_I("Reissue report (same format as realtime)");
        handle_realtime_report(app, pkt);
        break;
    case TLY_CMD_VEHICLE_LOGOUT:
        handle_vehicle_logout(app, pkt);
        break;
    case TLY_CMD_REMOVE_ALARM:
        handle_remove_alarm(app, pkt);
        break;
    case TLY_CMD_TIME_SYNC:
        handle_time_sync(app, pkt);
        break;
    case TLY_CMD_HEARTBEAT:
        TLY_LOG_D("Heartbeat response received");
        break;
    case TLY_CMD_CONTROL: {
        TLY_LOG_I("Control command received: %u bytes", pkt->data_len);
        if (pkt->data_len >= 7 && pkt->data) {
            int type = tly_ctrl_classify(pkt->data, pkt->data_len);
            switch (type) {
            case TLY_CTRL_TYPE_LOCK: {
                tly_lock_control_t lock;
                if (tly_decode_lock_control(pkt->data, pkt->data_len, &lock) == 0) {
                    tly_lock_control_dump(&lock);
                    tly_app_dispatch_lock(app, &lock);
                } else {
                    TLY_LOG_W("Failed to decode lock control");
                }
                break;
            }
            case TLY_CTRL_TYPE_OTA: {
                tly_ota_info_t ota;
                if (tly_decode_ota_info(pkt->data, pkt->data_len, &ota) == 0) {
                    tly_ota_info_dump(&ota);
                    tly_ota_dispatch(app, &ota);
                } else {
                    TLY_LOG_W("Failed to decode OTA info");
                }
                break;
            }
            case TLY_CTRL_TYPE_BATTERY_HEAT: {
                tly_battery_heat_t heat;
                if (tly_decode_battery_heat(pkt->data, pkt->data_len, &heat) == 0) {
                    tly_battery_heat_dump(&heat);
                    tly_app_handle_battery_heat(app, &heat);
                } else {
                    TLY_LOG_W("Failed to decode battery heat");
                }
                break;
            }
            case TLY_CTRL_TYPE_SD_CARD: {
                tly_sd_card_t sd;
                if (tly_decode_sd_card(pkt->data, pkt->data_len, &sd) == 0) {
                    tly_sd_card_dump(&sd);
                    tly_app_handle_sd_card(app, &sd);
                } else {
                    TLY_LOG_W("Failed to decode SD card info");
                }
                break;
            }
            case TLY_CTRL_TYPE_VIN_UNBIND: {
                tly_vin_unbind_t vu;
                if (tly_decode_vin_unbind(pkt->data, pkt->data_len, &vu) == 0) {
                    tly_vin_unbind_dump(&vu);
                    tly_app_handle_vin_unbind(app, &vu);
                } else {
                    TLY_LOG_W("Failed to decode VIN unbind");
                }
                break;
            }
            case TLY_CTRL_TYPE_TIME_SYNC: {
                tly_time_sync_t ts;
                if (tly_decode_time_sync(pkt->data, pkt->data_len, &ts) == 0) {
                    tly_time_sync_dump(&ts);
                    tly_app_handle_time_sync(app, &ts);
                } else {
                    TLY_LOG_W("Failed to decode time sync");
                }
                break;
            }
            case TLY_CTRL_TYPE_VIN_BIND: {
                tly_vin_bind_t vb;
                if (tly_decode_vin_bind(pkt->data, pkt->data_len, &vb) == 0) {
                    tly_vin_bind_dump(&vb);
                    tly_app_handle_vin_bind(app, &vb);
                } else {
                    TLY_LOG_W("Failed to decode VIN bind");
                }
                break;
            }
            default:
                TLY_LOG_I("Control other: sub_cmd=0x%02X", pkt->data[6]);
                break;
            }
        }
        tly_app_send_response(app, pkt->cmd_id, TLY_RESP_SUCCESS, NULL, 0);
        break;
    }
    case TLY_CMD_VERSION_UPLOAD:
        TLY_LOG_I("Version upload: %u bytes", pkt->data_len);
        tly_app_send_canid_request(app);  // 收到版本上传的平台响应后，立即请求CANID列表
        break;
    case TLY_CMD_VIN_REQUEST:
        if (pkt->data_len > 0 && pkt->data) {
            char vin_buf[32] = {0};
            uint32_t copy_len = (pkt->data_len < sizeof(vin_buf) - 1) ?
                                pkt->data_len : sizeof(vin_buf) - 1;
            memcpy(vin_buf, pkt->data, copy_len - 1);
            TLY_LOG_I("VIN response: %s (%u bytes), real_len=%u", vin_buf, pkt->data_len, strlen(vin_buf));

            /* 获取当前数据库中的VIN，比较后再决定是否更新 */
            ParamInfo_t param = {0};
            param.id = E_ParamId_VIN;
            if (GetParam(&param) == RET_OK) {
                if (strncmp(vin_buf, (const char *)param.value, VIN_CAN1_VIN_LENGTH) != 0) {
                    TLY_LOG_I("VIN changed, update db: old=[%s] new=[%s]", param.value, vin_buf);
                    UpdateParam(E_ParamId_VIN, (uint8_t *)vin_buf, strlen(vin_buf));
                }
            } else {
                TLY_LOG_E("GetParam VIN failed, update anyway");
            }

            McuMesg_t McuMsg = {0};
            McuMsg.type = SYS_QUE_TYPE_MCU_SYNC_VIN_STATUS;
            McuMsg.payloadLen = 2;
            uint8_t payLoad[2] = {0};
            payLoad[0] = 0x01;  // 强制锁存
            payLoad[1] = 0x01;  // VIN不一致
            McuMsg.payload = payLoad;
            if (RET_ERR == McuSrvSendMesg(&McuMsg))
            {
                TLY_LOG_E(" %s:McuSrvSendMesg faild!", __func__);
            }
        }
        break;
    case TLY_CMD_UPGRADE_STATUS:
        tly_ota_handle_upgrade_status(pkt->data, pkt->data_len);
        break;
    case TLY_CMD_CANID_LIST: {
        TLY_LOG_I("CANID list response (no channel): %u bytes", pkt->data_len);
        if (pkt->data_len > 0 && pkt->data) {
            int rc = tly_decode_canid_response_no_ch(pkt->data, pkt->data_len,
                                                     &app->canid_cache);
            if (rc == 0) {
                tly_canid_list_resp_dump(&app->canid_cache);
                app->canid_received = 1;

                /* 更新CAN数据采集的筛选列表 */
                tsp_can_data_update_filter(&app->canid_cache);

                tly_app_start_report_timer(app);
#if 0
                if (app->config.recorder_enable) {
                    tly_recorder_start(&app->recorder,
                                       app->config.recorder_dir,
                                       (uint32_t)app->config.recorder_max_size_mb,
                                       (uint32_t)app->config.recorder_sample_ms,
                                       (uint8_t)app->config.recorder_channels);
                }
#endif
            } else {
                TLY_LOG_W("Failed to decode CANID list response (no ch): %d", rc);
            }
        }
        break;
    }
    case TLY_CMD_CANID_LIST_CH: {
        TLY_LOG_I("CANID list response (with channel): %u bytes", pkt->data_len);
        if (pkt->data_len > 0 && pkt->data) {
            int rc = tly_decode_canid_response(pkt->data, pkt->data_len,
                                               &app->canid_cache);
            if (rc == 0) {
                tly_canid_list_resp_dump(&app->canid_cache);
                app->canid_received = 1;

                /* 更新CAN数据采集的筛选列表 */
                tsp_can_data_update_filter(&app->canid_cache);

                tly_app_start_report_timer(app);
#if 0
                if (app->config.recorder_enable) {
                    tly_recorder_start(&app->recorder,
                                       app->config.recorder_dir,
                                       (uint32_t)app->config.recorder_max_size_mb,
                                       (uint32_t)app->config.recorder_sample_ms,
                                       (uint8_t)app->config.recorder_channels);
                }
#endif
            } else {
                TLY_LOG_W("Failed to decode CANID list response: %d", rc);
            }
        }
        break;
    }
    case TLY_CMD_LOCK_KEY_REPORT:
        TLY_LOG_I("Lock key report: %u bytes", pkt->data_len);
        break;
    case TLY_CMD_WAKEUP_LOCATION:
        tly_app_send_vehicle_logout(app);  // 收到唤醒位置上报的平台响应后，立即发送车辆登出
        TLY_LOG_I("Wakeup location report: %u bytes", pkt->data_len);
        break;
    case TLY_CMD_VCU_VERSION:
    case TLY_CMD_MCU_VERSION:
    case TLY_CMD_ECU_VERSION:
    case TLY_CMD_TCM_VERSION:
        TLY_LOG_I("ECU version report (cmd=0x%02X): %u bytes",
                  pkt->cmd_id, pkt->data_len);
        break;
    case TLY_CMD_EV_CMD_FEEDBACK:
        TLY_LOG_I("EV command feedback: %u bytes", pkt->data_len);
        break;
    case TLY_CMD_ECU_FAULT_CODE:
        TLY_LOG_I("ECU fault code: %u bytes", pkt->data_len);
        break;
    case TLY_CMD_FILING_RESULT:
        TLY_LOG_I("Filing result (non-stage4): %u bytes", pkt->data_len);
        break;
    default:
        TLY_LOG_W("Unknown command 0x%02X: %u bytes", pkt->cmd_id, pkt->data_len);
        app->err_count++;
        break;
    }
}

/* ======================== TCP回调 ======================== */

static void on_tcp_packet(const tly_packet_t *pkt, void *ctx)
{
    tly_app_t *app = (tly_app_t *)ctx;
    tly_app_dispatch_packet(app, pkt);
}

static void on_tcp_status(int connected, void *ctx)
{
    tly_app_t *app = (tly_app_t *)ctx;

    if (connected) {
        app->state = TLY_STATE_CONNECTED;
        app->logged_in = 0;
        TLY_LOG_I("TCP connected, starting ECU version query and sending login..."); 
        if (app->tcp_keepalive_alarm_id == 0) /* 正常连接情况才上报ECU版本信息 */
        {
            start_ecu_version_query_async();
        }
        tly_app_send_vehicle_login(app);
    } else {
        app->state = TLY_STATE_RECONNECTING;
        app->logged_in = 0;
        /* canid_received 不重置: 离线存储需要 CANID 列表构造报文 */

        /* 停止心跳 */
        if (app->ev_heartbeat) {
            event_del(app->ev_heartbeat);
            event_free(app->ev_heartbeat);
         app->ev_heartbeat = NULL;
         }

         /* 不停止 report timer: 断线时继续触发并走离线存储 */
     }
}

static void on_heartbeat(evutil_socket_t fd, short events, void *ctx)
{
    (void)fd;
    (void)events;
    tly_app_t *app = (tly_app_t *)ctx;

    if (tly_tcp_is_connected(app->tcp)) {
        tly_app_send_heartbeat(app);
    }
}

static void on_signal(evutil_socket_t fd, short events, void *ctx)
{
    (void)fd;
    (void)events;
    tly_app_t *app = (tly_app_t *)ctx;

    TLY_LOG_I("Signal received, stopping...");
    tly_app_stop(app);
}
