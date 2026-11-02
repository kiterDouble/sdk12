#include <stdio.h>
#include<iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "otamodem2.h"
#include "sr_log.h"
#include "file_interface.h"
#include "upgrade_interface.h"
#include "system_interface.h"
#include "err_code.h"

int SetsloveModemUpgradeStatus(int status)
{
    char cmd[512] = {0};
    int ret = 0;
    
    // 验证状态值范围 (只允许 0,1,2,3,4)
    if (status < 0 || status > 4) {
        LOG_Error("WriteSMainModemUpgradeStatus: Invalid status value %d, must be 0-4", status);
        return RET_ERR;
    }
    
    // 使用 echo 写入文件
    snprintf(cmd, sizeof(cmd), "echo %d > /oemdata/parameters/SlaveModemUpgradeStatus", status);
    ret = system(cmd);
    if (ret != 0) {
        LOG_Error("WriteSMainModemUpgradeStatus: Failed to write status %d, ret=%d", status, ret);
        return RET_ERR;
    }
    
    LOG_Debug("WriteSMainModemUpgradeStatus: Successfully wrote status %d", status);
    return RET_OK;
}

int WriteFile(FILE *file ,unsigned char *data,size_t datalen)
{
    if(!data || !file)
    {
        LOG_Error(" OpenWriteFile data NULL\n");
        return -1;
    }

    size_t bytes_written = fwrite(data, sizeof(unsigned char), datalen, file);
    if (bytes_written < datalen) {
        LOG_Error("Error writing to file \n");
        return -1;
    }

    fflush(file); 
    return 0;
}

int ReadSpecificLine(const char *filename, int lineNumber, char *lineContent, size_t maxLen) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        LOG_Error("ReadSpecificLine 无法打开文件 \n");
        return -1;
    }

    char buffer[maxLen];
    int currentLine = 0;
    int success = -1;

    while (fgets(buffer, maxLen, file) != NULL) {
        currentLine++;
        if (currentLine == lineNumber) {
            strncpy(lineContent, buffer, maxLen);
            success = 0;
            break;
        }
    }

    fclose(file);

    return success;
}


void RemoveUpgradeFile()
{
    int ret = 0;
    char cmd[64] = {0};
    if(access(MPUBINFILEPATHMPUBIN,F_OK) == 0)
    {
        sprintf(cmd, "rm -rf %s", MPUBINFILEPATHMPUBIN);
        ret = GetSystemRetValue(cmd);
        if(-1 == ret)
        {
            LOG_Error("ret %d, cmd : %s", ret, cmd);

        }
    }
}