#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "err_code.h"
#include "sr_log.h"
#include "net_rpc.h"
#include "net_service.h"
#include "queue_msg.h"
#include "sr_ipc.h"
#include "sr_sdk.h"
#include "net_route_cache.h"

#define RPCSERVER_TYPE_NETSERVER_SUBCMD_GET_INFO        (0x10)
#define RPCSERVER_TYPE_NETSERVER_SUBCMD_ADD_ROUTE       (0x20)
#define RPCSERVER_TYPE_NETSERVER_SUBCMD_GET_ROUTE       (0x30)
#define ROUTE_IP_DOMAIN_LEN                             (64)
#define MAX_BIND_ETH_NUM                                (10)

void NetIpcSendNetState(void)
{
    StrNetStatus_t NetStatus = {0};

    NetStatus = FillNetStatusInfo();
	LOG_Debug("%s : pub message : SYS_QUE_TYPE_NET_STATUS_NOTIFY: %d ",__FUNCTION__,SYS_QUE_TYPE_NET_STATUS_NOTIFY);
	SendMsgToAllQueue(SYS_QUE_TYPE_NET_STATUS_NOTIFY, (void *)&NetStatus, sizeof(StrNetStatus_t));
}

static void RpcNetServerCallBack(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
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
        NetStatus = FillNetStatusInfo();
        memcpy(respData,&NetStatus,sizeof(StrNetStatus_t));
    }
    else if(RPCSERVER_TYPE_NETSERVER_SUBCMD_ADD_ROUTE == subcmd)
    {
        if(NULL == reqData)
        {
            *respData = RET_ERR;
            *respLen = sizeof(uint8_t);
            return;
        }
        memcpy(&setNetRoute, reqData, sizeof(StrSetNetRoute_t));
        if(0 == setNetRoute.routeType)   //set ip
        {
            *respData = SetIpRoute(&setNetRoute);
        }
        else if(1 == setNetRoute.routeType) // set domain
        {
            *respData = SetDomainRoute(&setNetRoute);
        }
        else
        {
            *respData = RET_ERR;
        }
        *respLen = sizeof(uint8_t);
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
    else
    {
        *respData = RET_ERR;
        *respLen = sizeof(uint8_t);
    }
}
void InitIpcRpcModules(void)
{
    InitSRSdk();
    InitRouteCache();
    RpcStartMsgServer(RPCSERVER_TYPE_NETSERVER,RpcNetServerCallBack,2,NULL);
    StartQueueMsgRcvThread("netServerThread");
    SubscriptionQueueMsg(SYS_QUE_TYPE_MODEM_SIGNAL_NOFIFY,NetModemSrvInfoCallback,NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY,NetPmChangeCallback,NULL);
    // SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_MODEM_NET_REQ,NetGetMcuChangeModemCmd,NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_PING_NET, NetMonitorStatusCallback, NULL);   //订阅TSP监控网络状态变化
}
