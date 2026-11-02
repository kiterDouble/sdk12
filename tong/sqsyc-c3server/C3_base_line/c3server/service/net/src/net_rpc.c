/**
 * @file net_rpc.c
 * @author 
 * @brief 
 * @version 0.1
 * @date 2025-05-30
 * 
 * @copyright Copyright (c) 2025 SiRun (HeFei).
 * 
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "err_code.h"
#include "sr_log.h"
#include "net_rpc.h"
#include "net_service.h"
#include "queue_msg.h"
#include "sr_ipc.h"
#include "sr_sdk.h"
#include "net_route_cache.h"
// add by mia.zhong @20250529
#include "net_timer.h" 
#include "modem_ipc_msg.h"
#include "modem_api.h"

#define RPCSERVER_TYPE_NETSERVER_SUBCMD_GET_INFO        (0x10)
#define RPCSERVER_TYPE_NETSERVER_SUBCMD_ADD_ROUTE       (0x20)
#define RPCSERVER_TYPE_NETSERVER_SUBCMD_GET_ROUTE       (0x30)
#define RPCSERVER_TYPE_NETSERVER_SUBCMD_NET_RESET       (0x40) // add by mia.zhong @20250529

#define ROUTE_IP_DOMAIN_LEN                             (64)
#define MAX_BIND_ETH_NUM                                (10)
#define NET_RESET_MIN_PERIOD                            (5 * 60) // 秒，网络重置定时器最小周期-5分钟
#define NET_RESET_MAX_CNT                               (5) // 网络最大重置次数-5次

// add by mia.zhong @20250529 ------- [start]
/**
 * 重置网络 - oos机制
 * max reset times：5次
 * oos time：
 *     (0-3) : reset times * 5（min）
 *      4    ：30（min）
 *      5    ：60（min）
 * 超过最大reset次数后，强制重启
 */
static pthread_mutex_t g_netRpcMutex = PTHREAD_MUTEX_INITIALIZER;
static timer_t g_netTimerId;
static bool g_netResetEnable = true;
static int g_netResetCnt = 1;


/* ++++++++++++++++++++++++++++++++ net rpc call  ++++++++++++++++++++++++++++++++*/

static StrNetStatus_t FillNetStatusInfo(NetSrvCtx_t *ctx)
{
    StrNetStatus_t NetStatus = {0};
    if (ctx->userCfg->apnNum > NETWORK_APN_NUM)
    {
        NetStatus.apnNum = NETWORK_APN_NUM;
    }
    else
    {
        NetStatus.apnNum = ctx->userCfg->apnNum;
    }
    for (int loopNum = 0; loopNum < NetStatus.apnNum; loopNum++)
    {
        NetStatus.netStatusInfo[loopNum].profileIndex = ctx->strNetInfo[loopNum].profileIndex;
        NetStatus.netStatusInfo[loopNum].dialStatus = ctx->strNetInfo[loopNum].dialStatus;
        NetStatus.netStatusInfo[loopNum].privateNet = ctx->strNetInfo[loopNum].privateNet;

        // 复制 ip
        strncpy(NetStatus.netStatusInfo[loopNum].ip, ctx->strNetInfo[loopNum].ip, NET_ADDR_LENGTH - 1);
        NetStatus.netStatusInfo[loopNum].ip[NET_ADDR_LENGTH - 1] = '\0';

        // 复制 ethName
        strncpy(NetStatus.netStatusInfo[loopNum].ethName, ctx->strNetInfo[loopNum].ethName, ETH_NAME_LENGTH - 1);
        NetStatus.netStatusInfo[loopNum].ethName[ETH_NAME_LENGTH - 1] = '\0';

        // 复制 priDns
        strncpy(NetStatus.netStatusInfo[loopNum].priDns, ctx->strNetInfo[loopNum].priDns, MODEM_IPADDR_LEN - 1);
        NetStatus.netStatusInfo[loopNum].priDns[MODEM_IPADDR_LEN - 1] = '\0';

        
        // 复制 secDns
        strncpy(NetStatus.netStatusInfo[loopNum].secDns, ctx->strNetInfo[loopNum].secDns, MODEM_IPADDR_LEN - 1);
        NetStatus.netStatusInfo[loopNum].secDns[MODEM_IPADDR_LEN - 1] = '\0';

        
        // 复制 gateway
        strncpy(NetStatus.netStatusInfo[loopNum].gateway, ctx->strNetInfo[loopNum].gateway, MODEM_IPADDR_LEN - 1);
        NetStatus.netStatusInfo[loopNum].gateway[MODEM_IPADDR_LEN - 1] = '\0';

        LOG_Debug("Profile Index: %d, Dial Status: %d, PrivateNet: %d, IP: %s, EthName: %s, PriDns : %s, SecDns : %s, GateWay : %s",
               NetStatus.netStatusInfo[loopNum].profileIndex,
               NetStatus.netStatusInfo[loopNum].dialStatus,
               NetStatus.netStatusInfo[loopNum].privateNet,
               NetStatus.netStatusInfo[loopNum].ip,
               NetStatus.netStatusInfo[loopNum].ethName,
               NetStatus.netStatusInfo[loopNum].priDns,
               NetStatus.netStatusInfo[loopNum].secDns,
               NetStatus.netStatusInfo[loopNum].gateway);
    }  
    
    return NetStatus;
}

static int8_t SetIpRoute(NetSrvCtx_t *ctx, StrSetNetRoute_t *SetNetRoute, StrAddNetRouteResp_t *resp)
{
    if (NULL == SetNetRoute)
    {
        LOG_Error("SetNetRoute is NULL");
        return RET_ERR;
    }

    char route[ROUTE_IP_DOMAIN_LEN] = {0};
    uint32_t priority = SetNetRoute->priority;

    if (SetNetRoute->routeDevice == NET_ROUTE_DEVICE_MODEM)
    {
        uint8_t pidExistFlag = 0;
        uint8_t i = 0;

        for (i = 0; i < ctx->userCfg->apnNum; i++)
        {
            if (SetNetRoute->deviceField.apnId == ctx->strNetInfo[i].profileIndex)
            {
                pidExistFlag = 1;
                break;
            }
        }

        if (pidExistFlag == 0)
        {
            LOG_Error("Invalid pid");
            return RET_ERR;
        }

        {
            const char *ethName = ctx->strNetInfo[i].ethName;
            LOG_Debug("ethName: %s", ethName);

            snprintf(route, sizeof(route), "%s", SetNetRoute->route);

            LOG_Debug("Setting route for modem device %s: %s, Priority: %u",
                      ethName, route, priority);

            {
            return DoSetIpRoute(route, ethName, priority, resp);
        }
        }
    }
    else if (SetNetRoute->routeDevice == NET_ROUTE_DEVICE_OTHER)
    {
        const char *ethName = SetNetRoute->deviceField.ethName;

        snprintf(route, sizeof(route), "%s", SetNetRoute->route);

        LOG_Debug("Setting route for other device %s: %s, Priority: %u",
                  ethName, route, priority);

        {
            return DoSetIpRoute(route, ethName, priority, resp);
        }
    }
    else
    {
        LOG_Error("Invalid route device type");
    }

    return RET_ERR;
}

static int8_t SetDomainRoute(NetSrvCtx_t *ctx, StrSetNetRoute_t *SetNetRoute, StrAddNetRouteResp_t *resp)
{

    if (NULL == SetNetRoute)
    {
        LOG_Error("SetNetRoute is NULL");
        return RET_ERR;
    }

    uint32_t priority = SetNetRoute->priority;
    
    
    if (SetNetRoute->routeDevice == NET_ROUTE_DEVICE_MODEM) {
        // modem设备
        // if (SetNetRoute->deviceField.apnId < ctx->userCfg->apnNum)
        {
            // 获取对应全局变量ctx->strNetInfo中ethName
            // const char *ethName = ctx->strNetInfo[SetNetRoute->deviceField.apnId].ethName;
            uint8_t pidExistFlag = 0;
            uint8_t i = 0;
            LOG_Debug("pid: %d", SetNetRoute->deviceField.apnId);

            for(i = 0; i < ctx->userCfg->apnNum; i++)
            {
                if (SetNetRoute->deviceField.apnId == ctx->strNetInfo[i].profileIndex)
                {
                    pidExistFlag = 1;
                    break;
                }
            }
            if (pidExistFlag == 0)
            {
                LOG_Error("Invalid pid");
                return RET_ERR;
            }
            
            LOG_Debug("ctx->strNetInfo[%d].profileIndex: %d", i, ctx->strNetInfo[i].profileIndex);
            const char *ethName = ctx->strNetInfo[i].ethName;
            LOG_Debug("ethName: %s", ethName);

            // 给网卡设置路由
            LOG_Debug("Setting route for modem device %s: %s, Priority: %u", ethName, SetNetRoute->route, priority);
            
            return DoSetDomainRoute(SetNetRoute->route, ethName, priority, ctx->strNetInfo[i].priDns, ctx->strNetInfo[i].secDns, resp);
        }
    }
    else if (SetNetRoute->routeDevice == NET_ROUTE_DEVICE_OTHER)
    {
        // 其他设备
        const char *ethName = SetNetRoute->deviceField.ethName;
        // 给网卡设置路由
        LOG_Debug("Setting route for other device %s: %s, Priority: %u", ethName, SetNetRoute->route, priority);

        return DoSetDomainRoute(SetNetRoute->route, ethName, priority, NULL, NULL, resp);
    }
    else
    {
        LOG_Error("Invalid route device type");
        return RET_ERR;
    }

    return RET_OK;
}

/* -------------------------------- net rpc call -------------------------------- */

static void NetRpcTimerCallback(union sigval val)
{
    (void)pthread_mutex_lock(&g_netRpcMutex);
    g_netResetEnable = true;
    (void)pthread_mutex_unlock(&g_netRpcMutex);
}
// add by mia.zhong @20250603 ------- [end]

static int IsIpRouteString(const char *route)
{
    struct in_addr addr;
    const char *slash = NULL;
    char ipPart[INET_ADDRSTRLEN] = {0};
    size_t ipLen = 0;

    if ((route == NULL) || (route[0] == '\0'))
    {
        return 0;
    }

    if (strcmp(route, "default") == 0)
    {
        return 1;
    }

    slash = strchr(route, '/');
    if (slash != NULL)
    {
        ipLen = (size_t)(slash - route);
        if (ipLen >= sizeof(ipPart))
        {
            return 0;
        }
        memcpy(ipPart, route, ipLen);
        ipPart[ipLen] = '\0';
        return (inet_pton(AF_INET, ipPart, &addr) == 1);
    }

    return (inet_pton(AF_INET, route, &addr) == 1);
}

void NetIpcSendNetState(void* arg)
{
    NetSrvCtx_t *ctx = (NetSrvCtx_t *)arg;
    if (ctx == NULL)
    {
        LOG_Error("%s : ctx is NULL", __FUNCTION__);
        return;
    }

    StrNetStatus_t NetStatus = {0};

    NetStatus = FillNetStatusInfo(ctx);
	LOG_Debug("%s : pub message : SYS_QUE_TYPE_NET_STATUS_NOTIFY: %d ",__FUNCTION__,SYS_QUE_TYPE_NET_STATUS_NOTIFY);
	SendMsgToAllQueue(SYS_QUE_TYPE_NET_STATUS_NOTIFY, (void *)&NetStatus, sizeof(StrNetStatus_t));
}

static void RpcNetServerCallBack(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
    NetSrvCtx_t *ctx = (NetSrvCtx_t *)userdata;
    StrNetStatus_t NetStatus = {0};
    StrSetNetRoute_t setNetRoute = {0};
    char IP[ROUTE_IP_DOMAIN_LEN] = {0};
    StrNetRouteBind_t routeBind = {0};
    
    if(NULL == respData || NULL == respLen)
    {
        LOG_Debug("parameter is err.");
        return;
    }
    if(RPCSERVER_TYPE_NETSERVER_SUBCMD_GET_INFO == subcmd)
    {
        *respLen = sizeof(StrNetStatus_t);
        NetStatus = FillNetStatusInfo(ctx);
        memcpy(respData,&NetStatus,sizeof(StrNetStatus_t));
    }
    else if(RPCSERVER_TYPE_NETSERVER_SUBCMD_ADD_ROUTE == subcmd)
    {
        StrAddNetRouteResp_t addRouteResp = {0};

        if(NULL == reqData)
        {
            addRouteResp.result = RET_ERR;
            *respLen = sizeof(StrAddNetRouteResp_t);
            memcpy(respData, &addRouteResp, sizeof(StrAddNetRouteResp_t));
            return;
        }
        memcpy(&setNetRoute, reqData, sizeof(StrSetNetRoute_t));
        if(IsIpRouteString(setNetRoute.route))
        {
            addRouteResp.result = SetIpRoute(ctx, &setNetRoute, &addRouteResp);
        }
        else
        {
            addRouteResp.result = SetDomainRoute(ctx, &setNetRoute, &addRouteResp);
        }
        *respLen = sizeof(StrAddNetRouteResp_t);
        memcpy(respData, &addRouteResp, sizeof(StrAddNetRouteResp_t));
    }
    else if(RPCSERVER_TYPE_NETSERVER_SUBCMD_GET_ROUTE == subcmd)
    {
        if(NULL == reqData)
        {
            LOG_Error("parameter is err.");
            return;
        }
        memcpy(IP, reqData, sizeof(IP));
        QueryRoute(IP,routeBind.devs,&routeBind.count, MAX_BIND_ETH_NUM);
        *respLen = sizeof(StrNetRouteBind_t);
        memcpy(respData,&routeBind, sizeof(StrNetRouteBind_t));
    }
    else if(RPCSERVER_TYPE_NETSERVER_SUBCMD_NET_RESET == subcmd)
    {
        // add by mia.zhong @20250529
        (void)pthread_mutex_lock(&g_netRpcMutex);
        if (g_netResetEnable) // reset enable
        {
            int timeout = 0;

            // 1、更改标志位
            g_netResetEnable = false;

            // 2、net reset
            if (g_netResetCnt > NET_RESET_MAX_CNT) // reboot
            {
                LOG_Debug("g_netResetCnt > 5, reboot tbox after 2s");
                sleep(2);
                system("reboot");
            }
            else
            {
                ModemSrvSetAirplaneMode(E_MODEM_SRV_DMS_AIRPLANE_ON); // at+cfun=4
                sleep(2);
                ModemSrvSetAirplaneMode(E_MODEM_SRV_DMS_AIRPLANE_OFF); // at+cfun=1
                // sleep(5);
            }

            // 3、start timer
            if (g_netResetCnt >= 5) // 60min
            {
                timeout = 60 * 60;
            }
            else if (g_netResetCnt == 4) // 30min
            {
                timeout = 30 * 60;
            }
            else if (g_netResetCnt > 0 && g_netResetCnt <= 3) // reset times * 5min
            {
                timeout = g_netResetCnt * NET_RESET_MIN_PERIOD;
            }
            else
            {
                LOG_Debug("g_netResetCnt is invalid!");
                (void)pthread_mutex_unlock(&g_netRpcMutex);
                return;
            }

            g_netResetCnt++;

            if (TimerXSecTimer(&g_netTimerId, timeout, 0) != 0)
            {
                g_netResetEnable = true;
            }
        }
        (void)pthread_mutex_unlock(&g_netRpcMutex);
    }
    else
    {
        *respData = RET_ERR;
        *respLen = sizeof(uint8_t);
    }
}
void InitIpcRpcModules(void* arg)
{
    InitSRSdk();
    InitRouteCache();
    RpcStartMsgServer(RPCSERVER_TYPE_NETSERVER,RpcNetServerCallBack,2,arg);
    StartQueueMsgRcvThread("netServerThread");
    SubscriptionQueueMsg(SYS_QUE_TYPE_MODEM_SIGNAL_NOFIFY,NetModemSrvInfoCallback,arg);
    SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY,NetPmChangeCallback,arg);
    // add by mia.zhong &20250529
    InitTimer(&g_netTimerId, NetRpcTimerCallback);
}
