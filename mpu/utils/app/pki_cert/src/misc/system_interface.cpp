#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "sr_log.h"
#include "sr_param.h"
#include "time_status_api.h"
#include "time_status.h"
#include "sr_ipc.h"
#include "sr_sdk.h"



gboolean GetCarNum(uint8_t *carNum, size_t len)
{
    if ((NULL == carNum) || (len < 8))
    {
        LOG_Error("param is NULL or len is too short");
        return FALSE;  
    }
   
    ParamInfo_t paranInfo;


    memset(&paranInfo, 0, sizeof(ParamInfo_t));
    paranInfo.id = 102;
    if(GetParam(&paranInfo) < 0)
    {
        LOG_Error("Get vin error");
        return FALSE;
    }

    sprintf((char *)carNum, "%s", paranInfo.value); 
    return TRUE;

}

int CheckTimeSync()
{
    int ret = 0;
    MpuTimeSync_t timeSync;
    LOG_Debug("CheckTimeSync start");
  

    ret = InitGetTimeSyncStatusCallBack();
    if(ret < 0)
    {
        LOG_Error("InitGetTimeSyncStatusCallBack Fail");
        //return -1;
    }

    while (1)
    {
        ret = GetTimeSyncStatus(&timeSync);
        if(ret != 0)
        {
            LOG_Debug("GetTimeSyncStatus fail ret =%d",ret);
            continue;
        }
        
        if(timeSync.timeSyncState == TIME_SYNC_OK)
        {
            LOG_Debug("Time synchronization successful timeSync.timeSyncState = %d",timeSync.timeSyncState);
            DestoryGetTimeSyncStatusCallBack();
           
            return 0;
        }
        
        sleep(1);
    }
    return 0;
}


int IsNetworkAvailable() 
{
    int ret = 0;
    for(int i=0;i<3;i++)
    {
        ret = system("ping -c 1 -W 5 tls-gateway.zelostech.com.cn ");
        if (ret == 0) 
        {
            return 0; // 网络可用

        } else {
            LOG_Debug("网络不可用");
            sleep(1); // 网络不可用
        }

    }

    return -1; 
}



int GetSystemRetValue(const char *cmd)
{
    pid_t status;
    if(NULL == cmd)
    {
        return -1;
    }

    printf("%s exe start \n", cmd);
    LOG_Debug("%s exe start \n", cmd);
    status = system(cmd);
    if(-1 == status)
    {
        LOG_Debug("status:%d ", status);
        return -1;
    }

    if(!WIFEXITED(status))
    {
        LOG_Debug("WIFEXITED(status): error");
        return -1;
    }
    return WEXITSTATUS(status);
}


int DomainNameResolution()
{
    struct addrinfo hints, *res;
    int status;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo("tls-gateway.zelostech.com.cn", NULL, &hints, &res);
    if (status != 0) {
        LOG_Error("getaddrinfo: %s", gai_strerror(status));
        return 1;
    }
    
    freeaddrinfo(res);
    return 0;
}
