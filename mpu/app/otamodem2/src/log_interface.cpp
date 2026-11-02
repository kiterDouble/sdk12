#include <stdio.h>
#include<iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
//#include <openssl/md5.h>
//#include <signal.h>
//#include <sys/statvfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "sr_log.h"
#include "log_interface.h"
#include "system_interface.h"



int HasValidFormat(const char *filename) {

    //if (strlen(filename) == 23 && strstr(filename, FILE_EXTENSION)) 
    if (strstr(filename, FILE_EXTENSION))
    {
        return 1;
    }
    return 0;
}


int LogProcessingFunction(long timediff)
{
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    time_t current_time = time(NULL);
    LOG_Debug("timediff = %ld\n",timediff);
    dir = opendir(DIRECTORY);
    if (!dir) {
        printf("opendir FAIL \n");
        LOG_Error("opendir FAIL \n");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) 
    {
        char filepath[512];
        if (entry->d_type == DT_DIR || !HasValidFormat(entry->d_name)) {
            continue;
        }

        snprintf(filepath, sizeof(filepath), "%s/%s", DIRECTORY, entry->d_name);

        if (stat(filepath, &file_stat) == -1) {
            LOG_Error("stat fail\n");
            continue;
        }

        if (difftime(current_time, file_stat.st_mtime) > timediff) 
        {    
            LOG_Debug("filepath = %s \n",filepath);
            if (remove(filepath) == 0) {
                LOG_Debug("Deleted: %s\n", filepath);
            } else {
                LOG_Error("remove %s fail\n",filepath);
            }

        }
    }

    closedir(dir);
    return 0;
}


void RemoveMiscLog()
{
    int ret = 0;
    char cmd[64] = {0};
    
    if(access(BOOTUP_LOG,F_OK) == 0)
    {
        sprintf(cmd, "rm -rf %s", BOOTUP_LOG);
        ret = GetSystemRetValue(cmd);
        if(-1 == ret)
        {
            LOG_Error("ret %d, cmd : %s", ret, cmd);

        }
    }


    if(access(DNS_LOG,F_OK) == 0)
    {
        sprintf(cmd, "rm -rf %s", DNS_LOG);
        ret = GetSystemRetValue(cmd);
        if(-1 == ret)
        {
            LOG_Error("ret %d, cmd : %s", ret, cmd);

        }
    }

    if(access(MEASUREMENTS_LOG,F_OK) == 0)
    {
        sprintf(cmd, "rm -rf %s", MEASUREMENTS_LOG);
        ret = GetSystemRetValue(cmd);
        if(-1 == ret)
        {
            LOG_Error("ret %d, cmd : %s", ret, cmd);

        }
    }

    if(access(STATISTICS_LOG,F_OK) == 0)
    {
        sprintf(cmd, "rm -rf %s", STATISTICS_LOG);
        ret = GetSystemRetValue(cmd);
        if(-1 == ret)
        {
            LOG_Error("ret %d, cmd : %s", ret, cmd);

        }
    }

    if(access(TRACKING_LOG,F_OK) == 0)
    {
        sprintf(cmd, "rm -rf %s", TRACKING_LOG);
        ret = GetSystemRetValue(cmd);
        if(-1 == ret)
        {
            LOG_Error("ret %d, cmd : %s", ret, cmd);

        }
    }

    if(access(SR_LOG,F_OK) == 0)
    {
        sprintf(cmd, "rm -rf %s", SR_LOG);
        ret = GetSystemRetValue(cmd);
        if(-1 == ret)
        {
            LOG_Error("ret %d, cmd : %s", ret, cmd);

        }
    }
    if(access(LOGSTORE,F_OK) == 0)
    {
        sprintf(cmd, "rm -rf %s", LOGSTORE);
        ret = GetSystemRetValue(cmd);
        if(-1 == ret)
        {
            LOG_Error("ret %d, cmd : %s", ret, cmd);

        }
    }

}