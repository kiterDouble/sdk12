#ifndef _GNU_SOURCE
#define _GNU_SOURCE  /* CLOCK_BOOTTIME */
#endif

#include "net_service.h"
#include "net_rpc.h"
#include "err_code.h"
#include "mcu_api.h"
#include "modem_api.h"
#include "modem_ipc_msg.h"
#include "net_config.h"
#include "net_bewinner_sim_sw_task.h"
#include "net_service_api.h"
#include "pm_server_api.h"
#include "sr_log.h"
#include "net_route_cache.h"
#include "net_host_resolve.h"
#include "hal_modem_dial.h" 
#include "hal_modem_dms.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/reboot.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>


#include "net_dial_record_db_api.h"
#include "net_interface_utils.h"
#include "net_route_utils.h"
#include "net_shell_utils.h"
#include "net_route_domain.h"
#include "common_utils.h"

typedef struct {
    uint8_t profileIndex;                      // 对应的profileIndex(callId)
    uint8_t dialStarted;                       // 是否已经执行拨号
    uint8_t dialStatus;                        // apn拨号状态
    uint8_t firstDialSuccessFlag;              // 本次启动是否已成功过
    struct timespec lastApnDisconnectTime;     // 记录apn拨号断开时间
} DialApnTaskState_t;

extern NetSrvCtx_t g_srvCtx;

static int8_t NetInit(NetSrvCtx_t *ctx)
{
    if (strcmp(ctx->userCfg->dnsPathName, "NULL") == 0 || strlen(ctx->userCfg->dnsPathName) == 0)
    {
        LOG_Debug("ctx->userCfg->dnsPathName is NULL");
    }
    else
    {
        char cmd[256] = {0};
        snprintf(cmd, sizeof(cmd), "echo \"\" > %s", ctx->userCfg->dnsPathName);
        system(cmd);
    }
    
    // system("iptables -t filter -F");
    // system("iptables -t nat -F");

    // 根据 ctx->userCfg->apnNum 的值添加相应数量的 MASQUERADE 规则
#if 0
    int8_t isFailed = 0;
    for (uint8_t i = 0; i < ctx->userCfg->apnNum; ++i)
    {
        char command[128] = {0};
        snprintf(command, sizeof(command), "iptables -t nat -A POSTROUTING -o %s -j MASQUERADE", ctx->strNetInfo[i].ethName);
        int status = system(command);
        if (status == 0)
        {
            LOG_Debug("%s : exec cmd[%s] success.",__FUNCTION__, command);
        }
        else
        {
            LOG_Debug("%s : exec cmd[%s] failed.",__FUNCTION__, command);
            isFailed = 1;
        }
    }

    if (isFailed == 1)
    {
        return RET_ERR;
    }
    else
    {
        return RET_OK;
    }
#else
    return RET_OK;
#endif
}


static void *DialCBThread(void *dialCBInfo)
{
    sleep_clock(3);
    DialCB_t *tmp = (DialCB_t *)dialCBInfo;
    NetSrvCtx_t *ctx = (NetSrvCtx_t *)tmp->ctx;
    HalModemNetInfo_t *netInfo = &tmp->netInfo;
    int32_t callId = tmp->callId;
    uint8_t netInfoIndex = 0xFF;

    if (ctx == NULL)
    {
        LOG_Error("%s : ctx is NULL", __FUNCTION__);
        free(dialCBInfo);
        return NULL;
    }

    LOG_Debug("%s : [net] profile id %d",__FUNCTION__, callId);
    // if(callId < 0 || callId > ctx->userCfg->apnNum)
    // {
    //     LOG_Error("%s : wrong callId value : %d",__FUNCTION__, callId);
    //     free(dialCBInfo);
    //     return NULL;
    // }

    for(uint8_t i = 0; i < ctx->userCfg->apnNum; i++)
    {
        if(ctx->strNetInfo[i].profileIndex == callId)
        {
            netInfoIndex = i;
            break;
        }
    }

    if (netInfoIndex >= ctx->userCfg->apnNum)
    {
        LOG_Error("%s : wrong netInfoIndex value : %d",__FUNCTION__, netInfoIndex);
        free(dialCBInfo);
        dialCBInfo = NULL;
        return NULL;
    }

    // 检查是否是有效的拨号状态
    if (netInfo->dialStatus == 1)
    {
        LOG_Debug("[net] profile id %d, netInfoIndex:%d", callId, netInfoIndex);
        LOG_Debug("[net] [%d]eth name:            %s", callId, netInfo->name);
        LOG_Debug("[net] [%d]IP address:          %s", callId, netInfo->ip);
        LOG_Debug("[net] [%d]Gateway address:     %s", callId, netInfo->gateway);
        LOG_Debug("[net] [%d]Primary DNS address: %s", callId, netInfo->priDns);
        LOG_Debug("[net] [%d]Second DNS address:  %s", callId, netInfo->secDns);
        
        // 获取IP地址和网关到ctx->strNetInfo
        strncpy(ctx->strNetInfo[netInfoIndex].ip, (char *)netInfo->ip, MODEM_IPADDR_LEN);
        ctx->strNetInfo[netInfoIndex].ip[MODEM_IPADDR_LEN - 1] = '\0';
        strncpy(ctx->strNetInfo[netInfoIndex].gateway, (char *)netInfo->gateway, MODEM_IPADDR_LEN);
        ctx->strNetInfo[netInfoIndex].gateway[MODEM_IPADDR_LEN - 1] = '\0';
        strncpy(ctx->strNetInfo[netInfoIndex].ethName, (char *)netInfo->name, ETH_NAME_LENGTH);
        ctx->strNetInfo[netInfoIndex].ethName[ETH_NAME_LENGTH - 1] = '\0';
        strncpy(ctx->strNetInfo[netInfoIndex].priDns, (char *)netInfo->priDns, MODEM_IPADDR_LEN);
        ctx->strNetInfo[netInfoIndex].priDns[MODEM_IPADDR_LEN - 1] = '\0';
        strncpy(ctx->strNetInfo[netInfoIndex].secDns, (char *)netInfo->secDns, MODEM_IPADDR_LEN);
        ctx->strNetInfo[netInfoIndex].secDns[MODEM_IPADDR_LEN - 1] = '\0';

        if (WaitInterfaceReady((const char *)netInfo->name, (const char *)netInfo->ip, 2000U) != RET_OK)
        {
            LOG_Error("%s : interface %s not ready after dial success, ip=%s",
                      __FUNCTION__,
                      netInfo->name,
                      netInfo->ip);
        }

        // 添加DNS到/etc/resolv.conf
        SetDnsAddr(ctx->userCfg->dnsPathName, ctx->strNetInfo[netInfoIndex].nameserverPri, netInfo->priDns, netInfo->secDns);

        // 添加DNS到路由表
        AddNameServerToRouteTable(ctx->strNetInfo[netInfoIndex].dnsRouteSet, netInfo->priDns, netInfo->secDns, netInfo->name);

        // 添加路由IP到网卡
        AddRouteIpToInterface(ctx->strNetInfo[netInfoIndex].privateRoute, ctx->strNetInfo[netInfoIndex].ethName);

        // 添加域名IP到网卡
        AddDomainIpToInterface(ctx->strNetInfo[netInfoIndex].domain, ctx->strNetInfo[netInfoIndex].ethName,
                               ctx->strNetInfo[netInfoIndex].priDns, ctx->strNetInfo[netInfoIndex].secDns);

        // 设置当前Apn拨号状态为连接状态
        ctx->strNetInfo[netInfoIndex].dialStatus = 1;

        LOG_Info("apn%d dail is success,dialStatus is connect.",(netInfoIndex));

        /** 拨号成功后写入数据库 */
        (void)g_dialRecordDbApi.insert(ctx->dialRecordDb, callId);
        if (g_dialRecordDbApi.queryCount(ctx->dialRecordDb, callId) >= (NET_DIAL_RECORD_KEEP_NUM + 30))
        {
            LOG_Info("apn%d dail is success, trim dial record to keep %d record.",(netInfoIndex), NET_DIAL_RECORD_KEEP_NUM);
            (void)g_dialRecordDbApi.trim(ctx->dialRecordDb, callId, NET_DIAL_RECORD_KEEP_NUM);
        }
    }
    else
    {
        // 设置当前Apn拨号状态为断开状态
        ctx->strNetInfo[netInfoIndex].dialStatus = 0;
        LOG_Info("apn%d dail is failed,dialStatus is disconnect.",(netInfoIndex));
    }
    NetIpcSendNetState(ctx);
    free(dialCBInfo);
    dialCBInfo = NULL;
    return NULL;
}

static void DataCallStateCallback(int32_t callId, const HalModemNetInfo_t *netInfo)
{
    int ret;
    pthread_t threadId = 0;
    pthread_attr_t attr;
    DialCB_t *dialCBInfo = NULL;

    LOG_Debug("[net] profile id %d\n", callId);

    if (netInfo == NULL)
    {
        LOG_Error("%s: netInfo is NULL", __FUNCTION__);
        return;
    }

    dialCBInfo = (DialCB_t *)malloc(sizeof(DialCB_t));
    if (dialCBInfo == NULL)
    {
        LOG_Error("%s: malloc failed", __FUNCTION__);
        return;
    }

    memset(dialCBInfo, 0, sizeof(DialCB_t));
    dialCBInfo->callId = callId;
    dialCBInfo->ctx = &g_srvCtx;
    memmove(&dialCBInfo->netInfo, netInfo, sizeof(HalModemNetInfo_t));

    ret = pthread_attr_init(&attr);
    if (ret != 0)
    {
        LOG_Error("%s: pthread_attr_init failed, ret=%d", __FUNCTION__, ret);
        free(dialCBInfo);
        return;
    }

    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (ret != 0)
    {
        LOG_Error("%s: pthread_attr_setdetachstate failed, ret=%d", __FUNCTION__, ret);
        pthread_attr_destroy(&attr);
        free(dialCBInfo);
        return;
    }

    ret = pthread_create(&threadId, &attr, DialCBThread, (void *)dialCBInfo);
    pthread_attr_destroy(&attr);
    if (ret != 0)
    {
        LOG_Error("%s: pthread_create failed, ret=%d", __FUNCTION__, ret);
        free(dialCBInfo);
        return;
    }
}

// 拨号初始化（只需执行一次）
static int32_t DialInit(NetSrvCtx_t *ctx)
{
    HalModemDialParam_t dialParam = {0};
    dialParam.apnNum = ctx->userCfg->apnNum;

    for (uint8_t i = 0; i < ctx->userCfg->apnNum; ++i)
    {
        dialParam.apnInfo[i].callId = ctx->strNetInfo[i].profileIndex;
        strncpy((char *)dialParam.apnInfo[i].apnName, ctx->strNetInfo[i].apnName, MODEM_TEXT_LEN - 1);
        strncpy((char *)dialParam.apnInfo[i].userName, ctx->strNetInfo[i].usrName, MODEM_TEXT_LEN - 1);
        strncpy((char *)dialParam.apnInfo[i].userPasswd, ctx->strNetInfo[i].password, MODEM_TEXT_LEN - 1);
        dialParam.apnInfo[i].authType = ctx->strNetInfo[i].authProto;
    }

    // 初始化拨号模块
    if (HalModemDialInit() != RET_OK)
    {
        LOG_Error("%s : HalModemDialInit is err.",__FUNCTION__);
        return RET_ERR;
    }

    // 设置拨号参数
    if (HalModemDialSetParam(&dialParam) != RET_OK)
    {
        LOG_Error("%s : HalModemDialSetParam is err.",__FUNCTION__);
        HalModemDialDeinit();
        return RET_ERR;
    }

    // 设置拨号状态回调
    if (HalModemDialSetStatusCallback(DataCallStateCallback) != RET_OK) {
        LOG_Error("%s : HalModemDialSetStatusCallback is err.",__FUNCTION__);
        HalModemDialDeinit();
        return RET_ERR;
    }

    return RET_OK;
}

/**
 * @name DialApnTaskHandle
 * @brief 手动APN拨号任务函数
 * @param dialApnTaskState  手动APN拨号任务状态
 * @param realDialStatus   APN实时拨号状态
 * @param firstDial        服务启动第一次拨号*
 */
static void DialApnTaskHandle(NetSrvCtx_t *ctx, DialApnTaskState_t* dialApnTaskState, uint8_t realDialStatus)
{
    /* 统计“最近一小时”拨号成功次数的窗口时长 */
    const uint32_t recentDialDurationSeconds = 3600;

    struct timespec currentUpTime = {0};
    int32_t recentDialSuccCount = 0;
    int32_t dialInterval = 0;
    int32_t ret = 0;

    /*** 记录APN拨号断开时间点 ***/
    if (dialApnTaskState->dialStatus != realDialStatus)
    {
        dialApnTaskState->dialStatus = realDialStatus;
        if (realDialStatus == 0)
        {
            /** 记录APN拨号断开时间点 **/
            (void)clock_gettime(CLOCK_BOOTTIME, &dialApnTaskState->lastApnDisconnectTime);
            LOG_Info("[%s] apn callId(%d) disconnected record", __FUNCTION__, dialApnTaskState->profileIndex);
        }
        else
        {
            dialApnTaskState->firstDialSuccessFlag = 1; // 标记本次启动成功拨号过
        }
    }

    do{
        if (dialApnTaskState->profileIndex <= 0)
        {
            LOG_Error("[%s] Invalid callId=%d , skip", __FUNCTION__, dialApnTaskState->profileIndex);
            break;
        }

        if (ctx->modemSrvNotify->dataRegStatus != E_MODEM_SRV_NW_REG_STATUS_REGISTERED)
        {
            LOG_Warn("[%s] net is not registered", __FUNCTION__);
            break;
        }

        /** APN 拨号已经成功 **/
        if (realDialStatus == 1)
        {
            break;
        }

        /** 第一次拨号成功前，忽视拨号间隔 **/
        if (dialApnTaskState->firstDialSuccessFlag)
        {
            /* 查询该 APN 在最近一小时内拨号成功的次数 */
            recentDialSuccCount = g_dialRecordDbApi.queryRecentCount(ctx->dialRecordDb, dialApnTaskState->profileIndex, recentDialDurationSeconds);
            if (recentDialSuccCount >= ctx->userCfg->apnDialSuccOneHourThreadHoldCount)
            {
                /* 超过阈值，使用较长的二级间隔，防止运营商锁卡 */
                dialInterval = ctx->userCfg->apnDialSecInterval;
                LOG_Info("[%s] apn callId(%d) interval(%d)s ", __FUNCTION__, dialApnTaskState->profileIndex, dialInterval);
            }
            else
            {
                /* 未超过阈值，使用较短的一级间隔  */
                dialInterval = ctx->userCfg->apnDialPriInterval;
            }
    
            /** 获取当前启动时间 **/
            (void)clock_gettime(CLOCK_BOOTTIME, &currentUpTime);
            /** 未满足拨号间隔 **/
            if ((currentUpTime.tv_sec - dialApnTaskState->lastApnDisconnectTime.tv_sec) < dialInterval)
            {
                break;
            }
        }

        LOG_Info("[%s] apn callId(%d) interval(%d)s prepare to dial", __FUNCTION__, dialApnTaskState->profileIndex, dialInterval);
        /** 如果已经成功执行过拨号，需要先DialStop **/
        if (dialApnTaskState->dialStarted)
        {
            (void) HalModemDialStopEx(dialApnTaskState->profileIndex);
            dialApnTaskState->dialStarted = 0;
        }

        /** 手动拨号，执行后不会自动重新拨号 **/
        ret = HalModemDialStartEx(dialApnTaskState->profileIndex, 0);
        if (ret == 0)
        {
            LOG_Info("[%s] apn callId(%d) DialStart interface call succeeded", __FUNCTION__, dialApnTaskState->profileIndex);
            dialApnTaskState->dialStarted = 1; // 标记已经拨号
        }
        else
        {
            LOG_Error("[%s] apn callId(%d)DialStart interface call failed, ret=%d, stopping this route", __FUNCTION__, dialApnTaskState->profileIndex, ret);
            (void) HalModemDialStopEx(dialApnTaskState->profileIndex); //DialStart接口调用失败，调用DialStop停止这一路
        }

    }while(0);
}

// 拨号任务线程函数
static gpointer DialTaskThread(gpointer arg)
{
    NetSrvCtx_t *ctx = (NetSrvCtx_t *)arg;
    DialApnTaskState_t dialApnTaskState[NETWORK_APN_NUM] = {0};
    int32_t ret = 0;
    uint32_t cycleCount = 0;        // 主循环计数，用于控制 DialStart 调用周期

    LOG_Info("[net] DialTaskThread started, waiting for network registration...");

    /*** 初始化拨号记录数据库 ***/
    (void)g_dialRecordDbApi.init(ctx->dialRecordDb, NET_DIAL_RECORD_DB_PATH);

    // 初始化：记录每路APN的profileIndex
    for (uint8_t i = 0; i < ctx->userCfg->apnNum; i++)
    {
        dialApnTaskState[i].profileIndex = ctx->strNetInfo[i].profileIndex;
    }

    // 步骤1: 等待注网成功
    while (1)
    {
        // 检查注网状态
        if (ctx->modemSrvNotify->dataRegStatus == E_MODEM_SRV_NW_REG_STATUS_REGISTERED)
        {
            LOG_Info("[net] Network registered successfully, dataRegStatus=%d, csq=%d", ctx->modemSrvNotify->dataRegStatus, ctx->modemSrvNotify->csq);
            break;
        }

        LOG_Debug("[net] Waiting for network registration, dataRegStatus=%d, csq=%d", ctx->modemSrvNotify->dataRegStatus, ctx->modemSrvNotify->csq);
        sleep(5);
    }

    // 步骤2: 初始化拨号（失败则重试）
    while (1)
    {
        LOG_Info("[net] Initializing dial...");
        ret = DialInit(ctx);

        if (ret == RET_OK)
        {
            LOG_Info("[net] DialInit completed successfully");
            break;
        }
        else
        {
            LOG_Error("[net] DialInit failed, ret=%d, retrying in 5 seconds...", ret);
            sleep(5);
        }
    }

    while (1)
    {
        /**
         * 根据电源状态管理拨号记录数据库的初始化和释放。
         */
        if(ctx->pmState->PmState == PM_STATE_NORMAL)
        {
            if (!g_dialRecordDbApi.isInitialized(ctx->dialRecordDb))
            {
                (void)g_dialRecordDbApi.init(ctx->dialRecordDb, NET_DIAL_RECORD_DB_PATH);
            }
        }
        else
        {
            cycleCount = 0;
            if (g_dialRecordDbApi.isInitialized(ctx->dialRecordDb))
            {
                (void)g_dialRecordDbApi.deinit(ctx->dialRecordDb);
            }
            sleep(1);
            continue;
        }

        /**
         *  APN拨号
         */
        if ((cycleCount++)%5 == 0)
        {
            for (uint8_t i = 0; i < ctx->userCfg->apnNum; i++)
            {
                DialApnTaskHandle(ctx, &dialApnTaskState[i], ctx->strNetInfo[i].dialStatus);
            }
        }
        sleep(1);
    }

    return NULL;
}

// 创建拨号任务线程的辅助函数
static int32_t CreateDialTaskThread(NetSrvCtx_t *ctx)
{
    GThread *thread = g_thread_new("DialTask", DialTaskThread, ctx);
    if (thread == NULL)
    {
        LOG_Error("[net] Failed to create dial task thread");
        return RET_ERR;
    }

    // 分离线程，使其在完成后自动清理资源
    g_thread_unref(thread);
    LOG_Info("[net] Dial task thread created successfully");
    return RET_OK;
}

void StartNetService(NetSrvCtx_t *ctx)
{
    // 初始化网络配置
    NetInit(ctx);

    // 高速模式配置
    NetHighSpeedModeSet(ctx->userCfg->highSpeedModeEnable);
    
    // 发起拨号逻辑
    CreateDialTaskThread(ctx);
}
