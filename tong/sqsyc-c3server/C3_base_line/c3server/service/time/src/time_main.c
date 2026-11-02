#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <pthread.h>
#include <unistd.h>
#include <glib.h>

#include "sr_sdk.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "pm_server_api.h"

#include "time_rtc_util.h"
#include "time_status.h"
#include "time_server.h"

static GMainLoop* g_mainLoop = NULL;

/**
 * @brief 信号处理函数
 *
 * 处理 SIGINT（Ctrl+C）、SIGTERM（kill）和 SIGPIPE 信号。
 * SIGINT/SIGTERM 触发退出 GLib 主循环；SIGPIPE 忽略。
 *
 * @param[in] signum 信号编号
 */
static void SignalHandlerFunc(int signum)
{
    switch (signum) {
        case SIGINT:
            LOG_Info("SIGINT (Ctrl+C)");
            break;
        case SIGTERM:
            LOG_Info("SIGTERM (kill command)");
            break;
        case SIGPIPE:
            LOG_Info("SIGPIPE");
            return;
        default:
            return;
    }
    /* 退出GLib主循环 */
    if(g_mainLoop != NULL){
        g_main_loop_quit(g_mainLoop);
    }
}


/**
 * @brief 恢复信号默认处理
 *
 * 将 SIGINT、SIGTERM、SIGPIPE 恢复为默认行为。
 */
static void ReleaseSignalHandler(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);   /* 恢复SIGINT默认处理（Ctrl+C直接终止程序） */
    sigaction(SIGTERM, &sa, NULL);  /* 恢复SIGTERM默认处理 */
    sigaction(SIGPIPE, &sa, NULL);  /* 恢复SIGPIPE默认处理 */
}

/**
 * @brief 注册信号处理函数
 *
 * 注册 SIGINT、SIGTERM、SIGPIPE 的自定义处理函数。
 *
 * @return int 0 表示成功，-1 表示失败
 */
static int InitSignalHandler()
{
    struct sigaction sa;
    int ret = 0;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SignalHandlerFunc;
    sigemptyset(&sa.sa_mask);

    /* 注册SIGINT（Ctrl+C）处理函数 */
    ret = sigaction(SIGINT, &sa, NULL);
    if (ret != 0) {
        LOG_Error("sigaction SIGINT failed: %s", strerror(errno));
        return -1;
    }

    /* 注册SIGTERM（kill）处理函数 */
    ret = sigaction(SIGTERM, &sa, NULL);
    if (ret != 0) {
        LOG_Error("sigaction SIGTERM failed: %s", strerror(errno));
        ReleaseSignalHandler();         /* 恢复默认行为 */
        return -1;
    }

    /* 注册SIGPIPE（写操作时对端已关闭连接）处理函数 */
    ret = sigaction(SIGPIPE, &sa, NULL);
    if (ret != 0) {
        LOG_Error("sigaction SIGPIPE failed: %s", strerror(errno));
        ReleaseSignalHandler();         /* 恢复默认行为 */
        return -1;
    }

    return 0;
}

/**
 * @brief 电源管理状态变化回调函数
 *
 * 订阅 SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY 后触发。
 * 当检测到从非 NORMAL 状态切换到 NORMAL 状态时，调用 RtcTimeLoadToSystem 从 RTC 恢复系统时间。
 *
 * @param[in] recvData 接收到的 PM 状态数据指针（MPUPmStateChange_t）
 * @param[in] dataLen  数据长度
 * @param[in] userData 用户数据（指向 TimeSrvCtx_t）
 */
static void PmStateChangeCallback(const uint8_t *recvData, const uint16_t dataLen, void *userData)
{
    TimeSrvCtx_t *ctx = NULL;
    MPUPmStateChange_t *pmStatus = NULL;

    do
    {
        if (recvData == NULL || userData == NULL) {
            LOG_Warn("param is NULL");
            break;
        }

        if (dataLen != sizeof(MPUPmStateChange_t)) {
            LOG_Warn("dataLen[%d] is invalid", dataLen);
            break;
        }

        ctx = (TimeSrvCtx_t*)userData;
        pmStatus = (MPUPmStateChange_t *)recvData;

        if (g_atomic_int_get(&ctx->pmState) != pmStatus->PmState) {
            if (g_atomic_int_get(&ctx->pmState) != PM_STATE_NORMAL  && pmStatus->PmState == PM_STATE_NORMAL){
                (void)RtcTimeLoadToSystem(ctx->config.rtcDev);
            }

            g_atomic_int_set(&ctx->pmState, pmStatus->PmState);
            LOG_Warn("pmState is changed to %d", g_atomic_int_get(&ctx->pmState));
        }

    } while (0);
}

/**
 * @brief 主进程入口
 *
 * timeServer 进程入口函数。初始化 SDK、配置解析、信号处理、PM 状态订阅、
 * 各工作线程（Manage/GNSS/NTP/Broadcast）、RPC Server，进入 GLib 主循环。
 * 退出时按逆序释放所有资源。
 *
 * @param[in] argc 命令行参数个数
 * @param[in] argv 命令行参数数组
 * @return int 进程退出码（始终返回 0）
 */
int main(int argc,char *argv[])
{

    const char *confPath = TIME_SYNC_CFG_PATH;

    TimeSrvCtx_t ctx = {
        .pmState = PM_STATE_NORMAL,
    };

	LOG_INIT("timeServer");
    LOG_Warn("timeServer Main enter");


    if (0 == access(TIME_SYNC_CFG_PATH_PRIMARY, F_OK))
    {
        confPath = TIME_SYNC_CFG_PATH_PRIMARY;
    }


    InitSRSdk();
    StartQueueMsgRcvThread("timeServer_signal_handle");

    (void) SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, PmStateChangeCallback, &ctx);

    do {

        g_mainLoop = g_main_loop_new(NULL, FALSE);
        if (g_mainLoop == NULL) {
            break;
        }
        
        /* 初始化加载授时配置 */
        if (TimeCfgParse(confPath, &ctx.config) != 0) {
            break;
        }
            
        /* 初始化信号处理 */
        if (InitSignalHandler() != 0) {
            break;
        }

        /* 初始化 同步时间管理 线程 */
        if (InitTimeSyncManageTask(&ctx) != 0) {
            break;
        }

        /* 初始化 Gnss同步时间 线程 */
        if (ctx.config.gnssCfg.priorityActivation.priority>0 && InitTimeSyncGnssTask(&ctx) != 0) {
            break;
        }
        /* 初始化 Ntp同步时间 线程 */
        if (ctx.config.ntpCfg.priorityActivation.priority>0 && InitTimeSyncNtpTask(&ctx) != 0) {
            break;
        }
        /* 初始化 MCU时间同步 线程 */
        if (InitTimeSyncBroadcastTask(&ctx) != 0) {
            break;
        }

        (void) RpcStartMsgServer(RPCSERVER_TYPE_TIMESERVER, TimeRpcBusServerCallBack, 2, &ctx);

        g_main_loop_run(g_mainLoop);

    }while(0);

    /* 释放 MCU时间同步 线程 */
    ReleaseTimeSyncBroadcastTask(&ctx);
    
    /* 释放 Ntp同步时间 线程 */
    if (ctx.config.ntpCfg.priorityActivation.priority>0){
        ReleaseTimeSyncNtpTask(&ctx);
    }    
    
    /* 释放 Gnss同步时间 线程 */
    if (ctx.config.gnssCfg.priorityActivation.priority>0){
        ReleaseTimeSyncGnssTask(&ctx);
    }

    /* 释放 同步时间管理 线程 */
    ReleaseTimeSyncManageTask(&ctx);

    /* 释放信号处理 */
    ReleaseSignalHandler();
    
    (void) UnSubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY);

    if (g_mainLoop != NULL) {
        g_main_loop_unref(g_mainLoop);
        g_mainLoop = NULL;
    }

    return 0;
}
