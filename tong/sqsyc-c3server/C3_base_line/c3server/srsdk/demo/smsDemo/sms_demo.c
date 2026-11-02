#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>

#include "sr_log.h"
#include "sr_ipc.h"
#include "modem_api.h"
#include "modem_ipc_msg.h"
#include "pm_server_api.h"
#include "err_code.h"

static volatile int g_running = 1;
static MPUPmStateChange_t g_pmState = {0};

/**
 * @brief 信号处理函数，响应 Ctrl+C 和 kill 信号
 */
static void signal_handler(int signum)
{
    (void)signum;
    g_running = 0;
}

/**
 * @brief 电源状态变化回调函数
 */
static void pm_state_callback(const uint8_t* data, const uint16_t len, void* userdata)
{
    (void)userdata;

    if (data == NULL || len != sizeof(MPUPmStateChange_t))
    {
        printf("[%s] invalid pm notify, len:%d\n", __FUNCTION__, len);
        return;
    }

    (void)memcpy((void*)&g_pmState, data, sizeof(MPUPmStateChange_t));

    printf("[%s] PM state changed: WorkMode=%u, PmState=%u, WakeupSrc=%u\n",
           __FUNCTION__, g_pmState.WorkMode, g_pmState.PmState, g_pmState.WakeupSrc);
}

/**
 * @brief 短信接收回调函数
 */
static void sms_recv_callback(const uint8_t* data, const uint16_t len, void* userdata)
{
    (void)userdata;
    ModemSrvSms_t recvSms;

    if (data == NULL || len != sizeof(ModemSrvSms_t))
    {
        printf("[%s] invalid sms notify, len:%d\n", __FUNCTION__, len);
        return;
    }

    (void)memcpy(&recvSms, data, sizeof(ModemSrvSms_t));

    printf("\n========== 收到短信 ==========\n");
    printf("来源号码: %s\n", recvSms.sourceAddr);
    printf("内容长度: %d\n", recvSms.msgLen);
    if (recvSms.msgLen > 0 && recvSms.msgLen <= (int32_t)sizeof(recvSms.msgContent))
    {
        /* 按文本打印，内容可能为 ASCII/GSM7/UCS2 编码 */
        printf("内容: %.*s\n", (int)recvSms.msgLen, (const char*)recvSms.msgContent);
    }

    /* 若当前处于休眠状态，触发短信唤醒 */
    if (g_pmState.PmState == PM_STATE_SLEEP)
    {
        printf("系统处于休眠状态，触发短信唤醒...\n");
        if (PmSyncDataWakeSourceAndWakeupSystem(PM_WAKEUP_SOURCE_SMS) != 0)
        {
            printf("短信唤醒调用失败\n");
        }
        else
        {
            printf("短信唤醒调用成功\n");
        }
    }

    printf("==============================\n");
}

/**
 * @brief 打印用法说明
 */
static void usage(const char *argv0)
{
    printf("用法: %s [选项]\n", argv0);
    printf("选项:\n");
    printf("  -h              显示帮助信息\n");
    printf("  -s <手机号>     发送短信，需配合 -c 指定内容\n");
    printf("  -c <内容>       短信内容\n");
    printf("  -r              进入接收等待模式（可与其他选项组合使用）\n");
    printf("\n示例:\n");
    printf("  %s -s 13800138000 -c hello          # 发送短信\n", argv0);
    printf("  %s -r                                  # 等待接收短信\n", argv0);
    printf("  %s -s 13800138000 -c hello -r        # 发送后等待接收\n", argv0);
}

/**
 * @brief 发送短信
 */
static int send_sms(const char *phoneNum, const char *content)
{
    int32_t ret = 0;

    if (phoneNum == NULL || content == NULL)
    {
        printf("[%s] 手机号或内容为空\n", __FUNCTION__);
        return -1;
    }

    printf("\n========== 发送短信 ==========\n");
    printf("目标号码: %s\n", phoneNum);
    printf("内容: %s\n", content);

    ret = ModemSrvSendSmsMesg((uint8_t*)phoneNum, (uint8_t*)content, (uint32_t)strlen(content));
    if (ret != RET_OK)
    {
        printf("发送短信失败! ret:%d\n", ret);
        return -1;
    }

    printf("发送短信成功!\n");
    printf("==============================\n");
    return 0;
}

int main(int argc, char *argv[])
{
    int opt;
    int sendMode = 0;
    int recvMode = 0;
    const char *phoneNum = NULL;
    const char *content = NULL;

    LOG_INIT("smsDemo");

    /* 注册信号处理：Ctrl+C 和 kill */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* 解析命令行参数 */
    while ((opt = getopt(argc, argv, "hs:c:r")) != -1)
    {
        switch (opt)
        {
            case 'h':
                usage(argv[0]);
                return 0;
            case 's':
                phoneNum = optarg;
                sendMode = 1;
                break;
            case 'c':
                content = optarg;
                break;
            case 'r':
                recvMode = 1;
                break;
            default:
                printf("错误: 无效选项 -%c\n", optopt);
                usage(argv[0]);
                return -1;
        }
    }

    if (!sendMode && !recvMode)
    {
        printf("错误: 未指定发送或接收模式\n");
        usage(argv[0]);
        return -1;
    }

    if (sendMode && content == NULL)
    {
        printf("错误: 发送短信需指定内容 -c <内容>\n");
        usage(argv[0]);
        return -1;
    }

    /* 启动 IPC 消息接收线程 */
    if (StartQueueMsgRcvThread("smsDemo_thread") != 0)
    {
        LOG_Error("[%s] StartQueueMsgRcvThread failed", __FUNCTION__);
        return -1;
    }

    /* 订阅电源状态通知，用于判断是否在休眠时收到短信 */
    if (SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, pm_state_callback, NULL) != 0)
    {
        LOG_Error("[%s] SubscriptionQueueMsg PM failed", __FUNCTION__);
        return -1;
    }

    /* 订阅短信通知 */
    if (recvMode)
    {
        if (SubscriptionQueueMsg(SYS_QUE_TYPE_MODEM_SMS_NOTIFY, sms_recv_callback, NULL) != 0)
        {
            LOG_Error("[%s] SubscriptionQueueMsg SMS failed", __FUNCTION__);
            return -1;
        }
        printf("已订阅短信接收通知，等待短信...\n");
    }

    /* 发送短信 */
    if (sendMode)
    {
        if (send_sms(phoneNum, content) != 0)
        {
            return -1;
        }
    }

    /* 主循环：保持进程运行，等待信号退出 */
    while (g_running)
    {
        sleep(1);
    }

    /* 取消订阅 */
    if (recvMode)
    {
        (void)UnSubscriptionQueueMsg(SYS_QUE_TYPE_MODEM_SMS_NOTIFY);
    }
    (void)UnSubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY);

    printf("smsDemo 退出\n");
    return 0;
}
