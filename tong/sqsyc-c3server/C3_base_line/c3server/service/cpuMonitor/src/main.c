#include <errno.h>
#include <signal.h>
#include <glib.h>

#include "cpu_info.h"
#include "cpu_pm.h"
#include "pm_server_api.h"
#include "sr_log.h"
#include "sr_ipc.h"
#include "sr_sdk.h"
#include "err_code.h"

static GMainLoop *volatile g_mainLoop = NULL;

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


int main()
{
    int retVal = 0;
    CpuMonitorTaskData_t taskData = {
        .pmState = PM_STATE_NORMAL,
    };

    LOG_INIT("cpuInfo");
    LOG_Warn("cpuInfoServer main enter\n");

    InitSRSdk();
    StartQueueMsgRcvThread("cpuInfo_signal_handle");
    LOG_Info("InitSRSdk done");

    do {
        if(InitSignalHandler() != 0){
            retVal = -1;
            break;
        }

        if (CpuPmStateMonitorInit(&taskData) != RET_OK) {
            LOG_Error("CpuPmStateMonitorInit failed");
            retVal = -1;
            break;
        }

        if (InitCpuMonitorTask(&taskData) != 0) {
            retVal = -1;
            break;
        }

        g_mainLoop = g_main_loop_new(NULL, FALSE);
        if (g_mainLoop == NULL) {
            retVal = -1;
            break;
        }

        g_main_loop_run(g_mainLoop);
    } while (0);

    if (g_mainLoop != NULL) {
        g_main_loop_unref(g_mainLoop);
        g_mainLoop = NULL;
    }

    ReleaseCpuMonitorTask(&taskData);

    CpuPmStateMonitorDeinit(&taskData);

    ReleaseSignalHandler();


    return retVal;
}
