

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <fcntl.h>
#include "system_interface.h"
#include "time_status_api.h"
#include "time_status.h"
#include "sr_log.h"
#include "sr_ipc.h"

int GetSystemRetValue(const char *cmd)
{
    pid_t status;
    if(NULL == cmd)
    {
        return -1;
    }

    status = system(cmd);
//    LOG_Debug("%s exe start status = %d", cmd,status);
    if(-1 == status)
    {
        return -1;
    }

    if(!WIFEXITED(status))
    {
        return -1;
    }
    return WEXITSTATUS(status);
}


int CheckTimeSync()
{
    int ret = 0;
    MpuTimeSync_t timeSync;
    LOG_Debug("CheckTimeSync start");
    StartQueueMsgRcvThread("can_file_save");
    InitGetTimeSyncStatusCallBack();
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


int GetEmmcSize()
{
    const char *device = "/dev/mmcblk0p1";
    int fd;
    unsigned long long size; // 用于存储设备大小（以字节为单位）

    fd = open(device, O_RDONLY);
    if (fd < 0) {
        LOG_Error("Failed to open device");
        return -1;
    }

    if (ioctl(fd, BLKGETSIZE64, &size) == -1) {
        LOG_Error("Failed to get device size");
        close(fd);
        return -1;
    }

    LOG_Debug("Device size: %llu bytes (%.2f GB)\n", size, size / (1024.0 * 1024 * 1024));
    close(fd);
    return ((int)(size / (1024.0 * 1024 * 1024)));
}