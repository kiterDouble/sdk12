#include "net_service_api.h"
#include "sr_sdk.h"
#include "sr_log.h"
#include "err_code.h"
#include <unistd.h>

int main()
{
    StrNetStatus_t netStatus = {0};
    StrSetNetRoute_t routeConfig[] = {
        {
            .priority = 100,                 // 优先级
            .route = "192.168.1.1",          // IP 地址或域名
            .routeDevice = NET_ROUTE_DEVICE_OTHER, // modem 网卡
            .deviceField = {
                .ethName = "eth0"           // 选择其他网卡，ethName 根据实际情况填写
            }
        },
        {
            .priority = 100,                 // 优先级
            .route = "doc.sirun.net",          // IP 地址或域名
            .routeDevice = NET_ROUTE_DEVICE_MODEM, // modem 网卡
            .deviceField = {
                .apnId = 1                    // 选择 modem 网卡，apnId 取值0-4
            }
        }
    };

    LOG_INIT("demo");
    
    InitSRSdk();
    
    int32_t ret = GetNetStatusInfo(&netStatus,50);
    if(RET_OK == ret)
    {
        for(int i = 0;i < netStatus.apnNum;i++)
        {
            LOG_Debug("profileIndex : %d , dialStatus : %d , privateNet : %d , ip : %s , ethName : %s .",
                netStatus.netStatusInfo[i].profileIndex,netStatus.netStatusInfo[i].dialStatus,netStatus.netStatusInfo[i].privateNet,netStatus.netStatusInfo[i].ip,netStatus.netStatusInfo[i].ethName);
        }
    }
    else
    {
       LOG_Debug("GetNetStatusInfo is err.");
    }


    for(int i = 0;i < sizeof(routeConfig) / sizeof(StrSetNetRoute_t);i++)
    {
        StrAddNetRouteResp_t routeResp = {0};
        int j;

        ret = AddNetRouteConfig(&routeConfig[i],&routeResp,0);
        if(RET_OK == ret)
        {
            LOG_Debug("AddNetRouteConfig is success. resolved ipCount: %d", routeResp.ipCount);
            for (j = 0; j < routeResp.ipCount; j++) {
                LOG_Debug("  resolved IP[%d]: %s", j, routeResp.ip[j]);
            }
            LOG_Debug("RouteConfig: Priority=%u, Route=%s, Device=%u, DeviceField={%u|%s}",
                   routeConfig[i].priority, routeConfig[i].route, routeConfig[i].routeDevice,
                   (routeConfig[i].routeDevice == NET_ROUTE_DEVICE_MODEM) ? routeConfig[i].deviceField.apnId : 0,
                   (routeConfig[i].routeDevice == NET_ROUTE_DEVICE_OTHER) ? routeConfig[i].deviceField.ethName : "");
        }
        else
        {
            LOG_Debug("AddNetRouteConfig is failed.");
        }
    }

    
    return 0;
    
}
