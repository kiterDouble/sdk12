#include <glib.h>
#include <gio/gio.h>
#include "sr_log.h"
#include "sr_ipc.h"
#include "stdint.h"
#include "param_sync.h"
#include "param_database.h"
#include "param_rpc_handle.h"
#include "param_backup.h"

int32_t main(int32_t argc, char *argv[])
{

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    GThread *syncMcuThread = NULL;
    GThread *syncModemThread = NULL;
    GThread *backupThread = NULL;

    LOG_INIT("paramServer");
    /**
     * @brief 从数据库进行初始化
     * 
     */
    ParamsDbInit();

    /**
     * @brief 数据加密初始化
     * 
     */
    ParamsEncryptInit();

    /**
     * @brief 启动IPC服务
     * 
     */
    StartQueueMsgRcvThread("paramServer_ipc");

    /**
     * @brief 启动RPC服务
     * GetParam UpdateParam AddUserParam DeleteUserParam
     * 
     */
    RpcStartMsgServer(RPCSERVER_TYPE_PARAMSERVER, paramServerCallBack, 1, NULL);

    /**
     * @brief 订阅消息 MCU向MPU请求参数回调 a01m03s01
     * 
     */
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_TBOX_READ_REQ, HandleMcuReadParamReq, NULL);
    /**
     * @brief 订阅消息 MCU向MPU同步参数 a01m03s03
     * 
     */
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_TBOX_MCU_WRITE_REQ, HandleMcuSyncParam, NULL);
    /**
     * @brief 订阅消息 MCU向MPU回复参数查询 a01m03s02
     * 
     */
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_TBOX_READ_RES, HandleMcuRespParam, NULL);
    /**
     * @brief 订阅消息 MCU向MPU回复参数同步结果 a01m03s04
     * 
     */
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_TBOX_MCU_WRITE_RES, HandleMcuRespSyncParam, NULL);

    /**
     * @brief 加载并更新MPU版本到数据库
     */
    LoadMpuVersionToParam();

    /**
     * @brief 定时器 使用主线程资源
     * 当MPU参数更新后，向MCU同步参数，但是MCU没有回复应答，使用此定时器检查应答情况，没有应答重新向MCU同步
     * 
     */
    g_timeout_add(5000, (GSourceFunc)CheckParamSyncAck, NULL);

    /**
     * @brief 线程 从mcu同步参数如vin 同步完退出
     * 
     */
    syncMcuThread = g_thread_new("syncMcuThread", UpdateParamFromMcu, NULL);

    /**
     * @brief 线程 从modem同步imei、imsi、iccid
     * 
     */
    syncModemThread = g_thread_new("syncModemThread", UpdateParamFromModem, NULL);

    /**
     * @brief 线程 参数文件备份管理
     * 
     */
    backupThread = g_thread_new("backupThread", paramBackupCallBack, NULL);

    /**
     * @brief 阻塞主线程
     * 
     */
    g_main_loop_run(loop);


    g_main_loop_unref(loop);
    g_thread_unref(syncMcuThread);
    g_thread_unref(syncModemThread);
    g_thread_unref(backupThread);
    return 0;
}
