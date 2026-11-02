
#include <stdio.h>
#include<iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "sr_log.h"
#include "file_interface.h"
#include "log_interface.h"
#include "mtd_interface.h"
#include "system_interface.h"



long GetAvailableSpaceMb(const char *path) 
{
    struct statvfs vfs;
    
    if (statvfs(path, &vfs) == -1) {
        LOG_Error("statvfs failed");
        return -1;
    }
    
    unsigned long available_blocks = vfs.f_bavail;
    unsigned long block_size = vfs.f_frsize;
    unsigned long available_bytes = available_blocks * block_size;
    long available_mb = available_bytes / (1024 * 1024);
    
    return available_mb;
}

long SpatialProcessingFunction(long size)
{
    int ret = 0;
    const char *mount_point = "/usrdata";

    LogProcessingFunction(TIME_DIFF_20);
    long free_mb = GetAvailableSpaceMb(mount_point);
    if(free_mb < 0)
    {
        LOG_Error("Fifth unable to get /usrdata partition space information\n");
        return -1;
    }
    RemoveMiscLog();
    LOG_Debug("/usrdata partition remaining available space: %ld MB\n", GetAvailableSpaceMb(mount_point));
    return GetAvailableSpaceMb(mount_point);
}

