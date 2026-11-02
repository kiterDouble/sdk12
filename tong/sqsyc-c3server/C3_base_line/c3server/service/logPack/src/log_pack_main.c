#include "stdint.h"
#include "stdio.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>

#include <glib.h>

#include "pm_server_api.h"
#include "sr_param.h"
#include "sr_log.h"
#include "sr_ipc.h"
#include "sr_sdk.h"
#include "err_code.h"

#include "log_pack_compress_and_crypto.h"
#include "sr_log.h"
#include "log_pack.h"
#include "log_pm.h"
#include "log_pack_cfg.h"
#include "log_pack_util.h"

static GMainLoop* g_mainLoop = NULL;

/**
 * 根据名称从全局接口注册表中查找加密接口。
 * 名称比较不区分大小写；未找到返回 NULL。
 */
static Crypto_interface_t* FindCryptoInterfaceByName(const char* name)
{
    uint32_t i = 0;

    if (name == NULL || g_cryptoInterfaces == NULL) {
        return NULL;
    }

    for (i = 0; g_cryptoInterfaces[i] != NULL; i++) {
        if (g_cryptoInterfaces[i]->name != NULL &&
            strcasecmp(g_cryptoInterfaces[i]->name, name) == 0) {
            return g_cryptoInterfaces[i];
        }
    }

    return NULL;
}

/**
 * 根据 [base] 中配置的全局加密方式，绑定 LogSrCtx_t 中的全局加密接口。
 * 所有 task 共用此接口，由 task->cryptoFlag 单独控制是否启用加密。
 */
static void InitCryptoInterface(LogSrCtx_t* ctx)
{
    uint32_t i = 0;

    if (ctx == NULL || ctx->tasks == NULL) {
        return;
    }

    if (ctx->cryptoName[0] == '\0' || strcasecmp(ctx->cryptoName, "none") == 0) {
        ctx->cryptoInterface = NULL;
        LOG_Info("[%s] global encryption disabled", __func__);
        return;
    }

    ctx->cryptoInterface = FindCryptoInterfaceByName(ctx->cryptoName);
    if (ctx->cryptoInterface != NULL) {
        LOG_Info("[%s] global crypto set to %s", __func__, ctx->cryptoInterface->name);
    } else {
        LOG_Warn("[%s] unknown global crypto %s, encryption disabled", __func__, ctx->cryptoName);
    }

    for (i = 0; i < ctx->taskCnt && i < LOG_PACK_TASK_MAX_NUM; i++) {
        LogSrPackTask_t* task = &ctx->tasks[i];
        if (task->cryptoFlag != 0 && ctx->cryptoInterface != NULL) {
            LOG_Info("[%s] task[%s] encryption enabled", __func__, task->name);
        } else {
            LOG_Info("[%s] task[%s] encryption disabled", __func__, task->name);
        }
    }
}

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

    if (g_mainLoop != NULL) {
        g_main_loop_quit(g_mainLoop);
    }
}

static void ReleaseSignalHandler(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);
}

static int InitSignalHandler(void)
{
    struct sigaction sa;
    int ret = 0;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SignalHandlerFunc;
    sigemptyset(&sa.sa_mask);

    ret = sigaction(SIGINT, &sa, NULL);
    if (ret != 0) {
        LOG_Error("sigaction SIGINT failed: %s", strerror(errno));
        return -1;
    }

    ret = sigaction(SIGTERM, &sa, NULL);
    if (ret != 0) {
        LOG_Error("sigaction SIGTERM failed: %s", strerror(errno));
        ReleaseSignalHandler();
        return -1;
    }

    ret = sigaction(SIGPIPE, &sa, NULL);
    if (ret != 0) {
        LOG_Error("sigaction SIGPIPE failed: %s", strerror(errno));
        ReleaseSignalHandler();
        return -1;
    }

    return 0;
}

int main(){
    int ret = 0;
    const char* cfgPath = LOG_PACK_CFG_PATH;
    LogSrCtx_t serverContext = {
        .cryptoInterface = NULL,
        .taskCnt = 0,
        .tasks = NULL,
        .checkInterval = 10,
        .bootUpDelay = 60,
        .wakeUpDelay = 30,
    };

    LOG_INIT("logServer");
    LOG_Warn("logServer main enter\n");

    /** init -- sr ipc */
    InitSRSdk();
    StartQueueMsgRcvThread("logServer_signal_handle");
    LOG_Info("InitSRSdk done");

    if (0 == access(LOG_PACK_CFG_PATH_PRIMARY, F_OK)) {
        cfgPath = LOG_PACK_CFG_PATH_PRIMARY;
    }

    do {
        if (InitSignalHandler() != 0) {
            LOG_Error("Failed to initialize signal handler");
            ret = -1;
            break;
        }

        /** init -- pm state monitor */
        if (LogPmStateMonitorInit(&serverContext) != RET_OK) {
            LOG_Error("LogPmStateMonitorInit failed");
            ret = -1;
            break;
        }

        if (LogPackLoadCfg(cfgPath, &serverContext) != 0) {
            LOG_Warn("LogPackLoadCfg failed");
            ret = -1;
            break;
        }

        /** bind global crypto interface */
        InitCryptoInterface(&serverContext);

        /** init crypto + timers + task thread */
        if (LogStoreTaskInit(&serverContext) != RET_OK) {
            LOG_Error("Failed to initialize log server resources");
            ret = -1;
            break;
        }        

        g_mainLoop = g_main_loop_new(NULL, FALSE);
        if (g_mainLoop == NULL) {
            LOG_Error("g_main_loop_new failed");
            ret = -1;
            break;
        }
            
        /** task -- main loop */
        LOG_Info("enter main loop");
        g_main_loop_run(g_mainLoop);
        LOG_Info("exit main loop");
    } while (0);

    /** deinit -- release glib loop */
    if (g_mainLoop != NULL) {
        g_main_loop_unref(g_mainLoop);
        g_mainLoop = NULL;
    }

    /** deinit -- release timers + crypto + thread */
    LogStoreTaskDeinit(&serverContext);

    /** release task array */
    if (serverContext.tasks != NULL) {
        free(serverContext.tasks);
        serverContext.tasks = NULL;
    }

    /** deinit -- release pm state monitor */
    LogPmStateMonitorDeinit(&serverContext);
    
    ReleaseSignalHandler();

    LOG_Warn("log_pack_server main exit");
    return ret;
}
