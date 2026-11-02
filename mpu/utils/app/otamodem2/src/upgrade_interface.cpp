
#include <stdio.h>
#include<iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "ota_api.h"
#include "sr_ipc.h"

#include "sr_log.h"
#include "file_interface.h"
#include "log_interface.h"
#include "system_interface.h"
#include "md5_interface.h"
#include "upgrade_interface.h"

void HexStringToBytes(const char *hex_string, unsigned char *bytes, size_t length) 
{
    for (size_t i = 0; i < length; i++) {
        sscanf(hex_string + 2 * i, "%2hhx", &bytes[i]);
    }
}


int DoUpgrade()
{
    int ret = 0;
    char cmd[512] = {0};
    UpgradeStatus_E upgradeSta = UPGRADE_INIT;
    unsigned char output[MD5_DIGEST_LENGTH +1];
    FILE *file =NULL;
    char line[64]= {0};
    unsigned char outbytes[64] = {0};

    LOG_Debug("DoUpgrade Start...\n");
    printf("DoUpgrade Start...\n");

    StartQueueMsgRcvThread("upgrade");
    ret = OtaStartUpgrade("/oemdata/upgrade/", UPGRADE_MPU_FLAG);
    if(ret < 0)
    {
        LOG_Error("OtaStartUpgrade return error ret = %d",ret);
        printf("OtaStartUpgrade return error ret = %d\n",ret);
        return UPGRADE_MPU_FAILED;
    }
    
    // 添加超时逻辑 - 15分钟超时
    time_t startTime = time(NULL);
    time_t currentTime = startTime;
    const int TIMEOUT_SECONDS = 15 * 60; // 15分钟 = 900秒
    system("echo 65 > /oemdata/parameters/SlaveModemUpProgress");
    while(1)
    {
        printf("upgrade status : %d\n", upgradeSta);
        OtaGetUpgradeStatus("/oemdata/upgrade/", &upgradeSta);
        if((UPGRADE_MPU_FAILED == upgradeSta || UPGRADE_MPU_NEED_BACKUP == upgradeSta || UPGRADE_IS_BUSY == upgradeSta))
        {
            LOG_Debug("upgrade status : %d", upgradeSta);
            if(UPGRADE_MPU_FAILED == upgradeSta)
            {
                memset(cmd, 0, sizeof(cmd));
                sprintf(cmd, "ql_cmd sysinfo /oemdata/log;");
                int ret = GetSystemRetValue(cmd);
                if(-1 == ret)
                {
                    LOG_Error("ql_cmd sysinfo %d, cmd : %s", ret, cmd);
                }
            }
            break;
        }

        // 检查是否超时
        currentTime = time(NULL);
        if ((currentTime - startTime) >= TIMEOUT_SECONDS) {
            LOG_Error("upgrade wait timeout after %d seconds, current status: %d", TIMEOUT_SECONDS, upgradeSta);
            upgradeSta = UPGRADE_MPU_FAILED; // 设置为失败状态
            break;
        }

        sleep(1);
        LOG_Debug("upgrade status : %d", upgradeSta);
    }
    sprintf(cmd, "rm -rf %s; sync ;", MPUBINFILEPATHMPUBIN);
    ret = GetSystemRetValue(cmd);
    if(-1 == ret)
    {
        LOG_Error("ret %d, cmd : %s", ret, cmd);

    }
    LOG_Debug("upgrade status : %d", upgradeSta);
    printf("upgrade status : %d\n", upgradeSta);
    return upgradeSta;
}
