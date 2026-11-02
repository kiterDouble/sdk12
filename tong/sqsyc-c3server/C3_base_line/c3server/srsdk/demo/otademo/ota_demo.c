#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include "err_code.h"
#include "sr_log.h"
#include "ota_api.h"
#include "sr_ipc.h"

int main(int argc, char *argv[])
{
    UpgradeStatus_E upgradeSta = -1;
    int upgradeFlag = 0;
    LOG_INIT("demo");

    if(argc < 2)
    {
        LOG_Error("uesage: otaDemo <1 :升级MCU, 2升级CPU, 3 全部升级>");
        return -1;
    }
    if(atoi(argv[1]) < UPGRADE_MCU_FLAG || atoi(argv[1]) > UPGRADE_ALL_FLAG)
    {
        LOG_Error("argv is invalid <1 :升级MCU, 2升级CPU, 3 全部升级>");
        return -1;
    }
    StartQueueMsgRcvThread("upgrade");
    OtaStartUpgrade("/oemdata/upgrade/", atoi(argv[1]));
    upgradeFlag = atoi(argv[1]);
    while(1)
    {
        OtaGetUpgradeStatus("/oemdata/upgrade/", &upgradeSta);

        if(UPGRADE_MCU_FLAG == (upgradeFlag) && \
          (UPGRADE_MCU_SUCCESS == upgradeSta || \
           UPGRADE_MCU_FAILED  == upgradeSta))
        {
            break;
        }

        if(UPGRADE_MPU_FLAG == (upgradeFlag & UPGRADE_MPU_FLAG) && \
           (UPGRADE_MPU_FAILED == upgradeSta || UPGRADE_MPU_NEED_BACKUP == upgradeSta))
        {
            break;
        }
        sleep(1);
        LOG_Debug("upgrade status : %d", upgradeSta);
    }

    if(UPGRADE_MPU_NEED_BACKUP == upgradeSta)
    {
        OtaSwitchAOrBPartition();
    }

    LOG_Debug("upgrade status : %d", upgradeSta);
    return upgradeSta;
}