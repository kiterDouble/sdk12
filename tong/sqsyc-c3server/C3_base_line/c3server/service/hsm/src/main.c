#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "sr_log.h"
#include "sr_sdk.h"
#include "sr_ipc.h"
#include "err_code.h"
#include "queue_msg.h"

#include "hsm_api.h"
#include "hsm_rpc_handler.h"
#include "hsm_pm_manager.h"
#include "ctx.h"

static GMainLoop* g_mainLoop = NULL;

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

    if(g_mainLoop != NULL){
        g_main_loop_quit(g_mainLoop);
    }
}

/* 释放信号处理 */
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

/* 初始化信号处理 */
static int InitSignalHandler()
{
    int ret = 0;
    struct sigaction sa;

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

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    Ctx_t ctx = {
        .hsmWorkState = E_HSM_WORK_STATE_UNKNOWN,
        .pmState = PM_STATE_SLEEP,
        .hsmApi = (HsmApi_t*)&g_hsmCIU98BV2_Api,
        .hsmUserData = NULL,
        .config = { .type = E_HSM_TYPE_CIU98BV2 },
    };

    LOG_INIT("hsmServer");
    LOG_Info("HSM RPC Server starting...");

    /** 初始化进程通讯IPC队列 **/
    InitSRSdk();
    StartQueueMsgRcvThread("hsm_thread");

    do {
        /* 初始化信号处理 */
        if (InitSignalHandler() != 0) {
            LOG_Error("Failed to initialize signal handler");
            break;
        }

        /* 解析HSM相关配置 */
        if (HsmConfigParser(&ctx) != 0) {
            LOG_Error("Failed to parse HSM configuration");
            break;
        }

        /* 根据配置选择 HSM 后端 */
        switch (ctx.config.type) {
            case E_HSM_TYPE_SIMULATE:
                ctx.hsmApi = (HsmApi_t*)&g_hsmSimulate_Api;
                ctx.hsmUserData = NULL;
                LOG_Info("HSM backend selected: simulate");
                break;
#ifdef HSM_TEE_SUPPORTED
            case E_HSM_TYPE_TEE:
                ctx.hsmApi = (HsmApi_t*)&g_hsmTee_Api;
                ctx.hsmUserData = NULL;
                LOG_Info("HSM backend selected: tee");
                break;
#endif
            case E_HSM_TYPE_CIU98BV2:
            default:
                ctx.hsmApi = (HsmApi_t*)&g_hsmCIU98BV2_Api;
                ctx.hsmUserData = NULL;
                LOG_Info("HSM backend selected: ciu98bv2");
                break;
        }

        g_mutex_init(&ctx.hsmApiMutex);

        g_mainLoop = g_main_loop_new(NULL, FALSE);
        if (g_mainLoop == NULL) {
            break;
        }

        /* 初始化电源状态 */
        if (HsmPmStateManagerInit(&ctx) != 0) {
            break;
        }

        RpcStartMsgServer(RPCSERVER_TYPE_HSM_OPENSSL_ENGINE_SERVER, hsmRpcCallback, 1, &ctx);

        /* 运行主循环 */
        g_main_loop_run(g_mainLoop);
    } while (0);

    /* 释放电源状态 */
    HsmPmStateManagerRelease(&ctx);

    /* 释放主循环资源 */
    if (g_mainLoop != NULL) {
        g_main_loop_unref(g_mainLoop);
        g_mainLoop = NULL;
    }

    /* 释放信号处理 */
    ReleaseSignalHandler();

    g_mutex_clear(&ctx.hsmApiMutex);

    return RET_OK;
}
