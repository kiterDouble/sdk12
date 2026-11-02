
#include <stdio.h>
#include<iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "sr_log.h"
#include "system_interface.h"
#include <arpa/inet.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <signal.h>
#include <openssl/md5.h>
#include "err_code.h"

int WriteFile(FILE *file , char *data,size_t datalen)
{
    if(!data || !file)
    {
        printf(" OpenWriteFile data NULL\n");
        LOG_Error(" OpenWriteFile data NULL\n");
        return -1;
    }

    size_t bytes_written = fwrite(data, sizeof(char), datalen, file);
    if (bytes_written < 0) {
        printf("Error writing to file \n");
        LOG_Error("Error writing to file \n");
        fclose(file);
        return -1;
    }

    printf("Successfully wrote %zu \n", bytes_written);
    LOG_Error("Successfully wrote %zu \n", bytes_written);


    return 0;
}

int SwtMainModemUpgradeStatus(int status)
{
    char cmd[512] = {0};
    int ret = 0;
    
    // 验证状态值范围 (只允许 0,1,2,3,4)
    if (status < 0 || status > 4) {
        LOG_Error("WriteSMainModemUpgradeStatus: Invalid status value %d, must be 0-4", status);
        return RET_ERR;
    }
    
    // 使用 echo 写入文件
    snprintf(cmd, sizeof(cmd), "echo %d > /oemdata/parameters/MainModemUpgradeStatus", status);
    ret = system(cmd);
    if (ret != 0) {
        LOG_Error("WriteSMainModemUpgradeStatus: Failed to write status %d, ret=%d", status, ret);
        return -1;
    }
    
    LOG_Debug("WriteSMainModemUpgradeStatus: Successfully wrote status %d", status);
    return 0;
}

double GetFileSize(const char *filename) 
{
    struct stat st;
    if (stat(filename, &st) != 0) {
        LOG_Error("stat error ");
        printf("stat error \n");
        return -1;
    }

    double size = (double)st.st_size;

    size /= (1024 * 1024);

    printf("文件 %s 的大小: %.2f \n", filename, size);
    LOG_Debug("文件 %s 的大小: %.2f \n", filename, size);

    return size;
}

int ReadSpecificLine(const char *filename, int lineNumber, char *lineContent, size_t maxLen) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("ReadSpecificLine 无法打开文件 \n");
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
int GetSysRetValue(const char *cmd)
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
        return -1;
    }

    if(!WIFEXITED(status))
    {
        return -1;
    }
    return WEXITSTATUS(status);
}
void HexStringToBytes(const char *hex_string, unsigned char *bytes, size_t length) {
    for (size_t i = 0; i < length; i++) {
        sscanf(hex_string + 2 * i, "%2hhx", &bytes[i]);
    }
}