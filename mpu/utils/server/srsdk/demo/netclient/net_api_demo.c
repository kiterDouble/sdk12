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
            .routeType = 0,                  // 0 表示 IP
            .priority = 100,                 // 优先级
            .route = "192.168.1.1",          // IP 地址或域名
            .routeDevice = 0,                // 0 表示 modem
            .deviceField = {
                .apnId = 0                    // 选择 modem 网卡，apnId 取值0-4
                // 或
                // .ethName = "eth0"           // 选择其他网卡，ethName 根据实际情况填写
            }
        },
        {
            .routeType = 1,                  // 0 表示 IP
            .priority = 100,                 // 优先级
            .route = "doc.sirun.net",          // IP 地址或域名
            .routeDevice = 0,                // 0 表示 modem
            .deviceField = {
                .apnId = 0                    // 选择 modem 网卡，apnId 取值0-4
                // 或
                // .ethName = "eth0"           // 选择其他网卡，ethName 根据实际情况填写
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
            LOG_Debug("profileIndex : %d , dialStatus : %d , ip : %s , ethName : %s .",
                netStatus.netStatusInfo[i].profileIndex,netStatus.netStatusInfo[i].dialStatus,netStatus.netStatusInfo[i].ip,netStatus.netStatusInfo[i].ethName);
        }
    }
    else
    {
       LOG_Debug("GetNetStatusInfo is err.");
    }


    for(int i = 0;i < sizeof(routeConfig) / sizeof(StrSetNetRoute_t);i++)
    {
        ret = AddNetRouteConfig(&routeConfig[i],0);
        if(RET_OK == ret)
        {
            LOG_Debug("AddNetRouteConfig is success.");
            LOG_Debug("RouteConfig: Type=%u, Priority=%u, Route=%s, Device=%u, DeviceField={%u|%s}",
                   routeConfig[i].routeType, routeConfig[i].priority, routeConfig[i].route, routeConfig[i].routeDevice,
                   (routeConfig[i].routeDevice == 0) ? routeConfig[i].deviceField.apnId : 0,
                   (routeConfig[i].routeDevice == 1) ? routeConfig[i].deviceField.ethName : "");
        }
        else
        {
            LOG_Debug("AddNetRouteConfig is failed.");
        }
    }

    
    return 0;
    
}
