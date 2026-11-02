#include <glib.h> 
#include <string.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <signal.h> 
#include <sys/time.h>
#include "sr_ipc.h" 
#include "sr_log.h"
#include "net_service_api.h"
#include "err_code.h"

#define RPCSERVER_TYPE_NETSERVER_SUBCMD_GET_INFO        (0x10)
#define RPCSERVER_TYPE_NETSERVER_SUBCMD_ADD_ROUTE       (0x20)
#define RPCSERVER_TYPE_NETSERVER_SUBCMD_GET_ROUTE       (0x30)
#define RPCSERVER_TYPE_NETSERVER_SUBCMD_NET_RESET       (0x40)

uint8_t g_IpcInitFlag = 0;


int32_t GetNetStatusInfo(StrNetStatus_t *netStatus,int32_t timeOut)
{
    StrNetStatus_t recvdata = {0};
    uint16_t recvlen = 0;
    
    memset(&recvdata,0, sizeof(StrNetStatus_t));
    if(!g_IpcInitFlag)
    {    
        CreatRpcQueue();
        g_IpcInitFlag = 1;
    }
    RpcGetMsgSync(RPCSERVER_TYPE_NETSERVER, RPCSERVER_TYPE_NETSERVER_SUBCMD_GET_INFO, timeOut, NULL, 0, (uint8_t *)&recvdata, &recvlen);
    if(recvlen == sizeof(StrNetStatus_t))
    {
        *netStatus = recvdata;
        return RET_OK;
    }
    return RET_ERR;
}

int32_t AddNetRouteConfig(StrSetNetRoute_t *setNetRoute, StrAddNetRouteResp_t *resp, int32_t timeOut)
{
    StrAddNetRouteResp_t recvdata = {0};
    uint16_t recvlen = 0;

    if(!g_IpcInitFlag)
    {    
        CreatRpcQueue();
        g_IpcInitFlag = 1;
    }
    RpcGetMsgSync(RPCSERVER_TYPE_NETSERVER, RPCSERVER_TYPE_NETSERVER_SUBCMD_ADD_ROUTE, timeOut, (uint8_t *)setNetRoute,sizeof(StrSetNetRoute_t), (uint8_t *)&recvdata, &recvlen);
    if(recvlen == sizeof(StrAddNetRouteResp_t))
    {
        if(NULL != resp)
        {
            memcpy(resp, &recvdata, sizeof(StrAddNetRouteResp_t));
        }
        return recvdata.result;
    }
    else
    {
        return RET_ERR;
    }
}

int32_t QueryNetRouteBind(StrNetRouteBind_t *routeBind,char IP[ROUTE_IP_DOMAIN_LEN],int32_t timeOut)
{
    StrNetRouteBind_t recvdata = {0};
    uint16_t recvlen = 0;

    if(!g_IpcInitFlag)
    {    
        CreatRpcQueue();
        g_IpcInitFlag = 1;
    }
    RpcGetMsgSync(RPCSERVER_TYPE_NETSERVER, RPCSERVER_TYPE_NETSERVER_SUBCMD_GET_ROUTE, timeOut, (uint8_t *)IP,ROUTE_IP_DOMAIN_LEN , (uint8_t *)&recvdata, &recvlen);
    if(recvlen == sizeof(StrNetRouteBind_t))
    {
        *routeBind = recvdata;
        return RET_OK;
    }
    else
    {
        return RET_ERR;
    }
}

int32_t NetReset(int32_t timeOut)
{
    if(!g_IpcInitFlag)
    {    
        CreatRpcQueue();
        g_IpcInitFlag = 1;
    }
    int32_t ret = RpcGetMsgSync(RPCSERVER_TYPE_NETSERVER, RPCSERVER_TYPE_NETSERVER_SUBCMD_NET_RESET, timeOut, NULL, 0, NULL, 0);
    if (ret != 0)
    {
        LOG_Debug("net reset failed");
    }
    
    return ret;
}
