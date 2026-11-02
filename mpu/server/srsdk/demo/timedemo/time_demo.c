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
        LOG_Debug("time status :%d, %d", timeSync.timeSyncState, timeSync.timeSyncSrc);
        sleep(1);
    }

    DestoryGetTimeSyncStatusCallBack();

    return RET_OK;
}