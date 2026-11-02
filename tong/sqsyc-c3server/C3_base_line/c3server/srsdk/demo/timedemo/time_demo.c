#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include "err_code.h"
#include "sr_log.h"
#include "time_status_api.h"
#include "sr_ipc.h"

int main(int argc, char *argv[])
{

    LOG_INIT("timeServer");
    MpuTimeSync_t timeSync;

    StartQueueMsgRcvThread("timeDemo");

    InitGetTimeSyncStatusCallBack();

    while(1)
    {
        GetTimeSyncStatus(&timeSync);
        LOG_Debug("time status :%d, Src:%d, Priority:%d, TSPPriority:%d", timeSync.timeSyncState, timeSync.timeSyncSrc, timeSync.priority, GetTSPPriority());
        if (GetTSPPriority() > timeSync.priority)
        {
            time_t tspTime = 1783872000;    // 2026-07-13 00:00:00
            LOG_Debug("report tsp time :%ld", tspTime);
            SetTspSyncTime(tspTime);
        }
        sleep(1);
    }

    DestoryGetTimeSyncStatusCallBack();

    return RET_OK;
}