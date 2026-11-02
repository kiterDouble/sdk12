#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include <pthread.h>
#include <unistd.h>
#include <glib.h>

#include "sr_log.h"
#include "sr_ipc.h"
#include "sr_sdk.h"
#include "property_server.h"
#include "property_rpc_handler.h"

#define PROPERTY_RUN_FLAG "/tmp/PROPERTY_SERVER_BOOT_FIRST_RUNNING"

static GMainLoop* g_mainLoop = NULL;

/*
* brief: 判断Server 是否是本次开机第一次启动
*        如果存在 PROPERTY_RUN_FLAG 文件，则不是第一次启动，返回false
*        如果不存在 PROPERTY_RUN_FLAG 文件，则返回true
*        注意：创建标志文件延迟到 PropertyServerInit 成功后进行
*/
static uint8_t IsServiceBootFirstRun(void){
    if (access(PROPERTY_RUN_FLAG, F_OK) == 0)
    {
        return false;
    }

    return true;
}

/*
* brief: 创建 PROPERTY_RUN_FLAG 标志文件
*        在 PropertyServerInit 成功后调用，标记本次启动已完成初始化
*/
static int CreatePropertyRunFlag(void){
    FILE *flagFile = NULL;

    flagFile = fopen(PROPERTY_RUN_FLAG, "w");
    if (flagFile == NULL)
    {
        LOG_Error("[%s] failed to create %s: %s", __func__, PROPERTY_RUN_FLAG, strerror(errno));
        return -1;
    }

    fclose(flagFile);
    return 0;
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

/*
* main func
*/
int main(int argc,char *argv[]) {

    (void)argc;
    (void)argv;
    
    PropertyServerContext_t propertyCtx = {
        .propStoreApi = (PropStoreInterface_t*) &g_sqlite3_property_store_interface,
        .propStoreDev = 0,
        .isFirstRunning = 0,
        .propertys = {
            .map = NULL,
            .keyList = NULL
        }
    };

    LOG_INIT("propServer");
    LOG_Warn("PropertyServer Main enter\n");

    /** 初始化进程通讯IPC队列 **/
    InitSRSdk();
    LOG_Info("InitSRSdk done");

    propertyCtx.isFirstRunning = IsServiceBootFirstRun();
    LOG_Info("isFirstRunning: %d", propertyCtx.isFirstRunning);

    do {
        /* 初始化信号处理 */
        if (InitSignalHandler() != 0) {
            LOG_Error("Failed to initialize signal handler");
            break;
        }

        if (PropertyServerInit(&propertyCtx) != 0) {
            LOG_Error("Failed to initialize property server");
            break;
        } else {
            /* PropertyServerInit 成功后，才创建运行标志文件 */
            if (propertyCtx.isFirstRunning != 0)
            {
                (void) CreatePropertyRunFlag();
            }
        }
        LOG_Info("PropertyServerInit done");

        g_mainLoop = g_main_loop_new(NULL, FALSE);
        if (g_mainLoop == NULL) {
            break;
        }

        if (RpcStartMsgServer(RPCSERVER_TYPE_PROPERTYSERVER, propertyRpcCallback, 1, &propertyCtx) != 0) {
            LOG_Error("Failed to start rpc msg server");
            break;
        }

        /* 运行主循环 */
        LOG_Info("enter main loop");
        g_main_loop_run(g_mainLoop);
        LOG_Info("exit main loop");
    } while (0);

    /* 释放主循环资源 */
    if (g_mainLoop != NULL) {
        g_main_loop_unref(g_mainLoop);
        g_mainLoop = NULL;
    }

    PropertyServerDeInit(&propertyCtx);

    /* 释放信号处理 */
    ReleaseSignalHandler();

    return 0;
}
