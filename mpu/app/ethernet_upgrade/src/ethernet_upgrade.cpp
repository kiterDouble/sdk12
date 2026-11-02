
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <openssl/md5.h>
#include "ota_api.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "ethernet_upgrade.h"
#include "md5_interface.h"
#include "upgrade_interface.h"
#include "system_interface.h"
#include <poll.h>
#include <ctype.h>
#include <time.h>

#include "mcuinte.h"

//新增修改数据库中的版本号
typedef enum {
    SUCCESS = 0,
    ERR_FILE_OPEN,      // 版本文件打开失败
    ERR_FILE_READ,      // 版本文件读取失败
    ERR_CONTENT_EMPTY,  // 内容为空
    ERR_FORMAT_INVALID, // 格式错误（分割后部分不足）
    ERR_CMD_EXEC        // 命令执行失败
} Result;

static void trimWhitespace(char *str) {
    if (str == NULL) return;
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    char *end = str + strlen(str) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

int  updateVersionWithCmd(void) {
    // 1. 读取版本文件内容
    const char *version_path = "/oemapp/upgrade/version_uds.txt";
    FILE *fp = fopen(version_path, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open version file: %s\n", version_path);
        return ERR_FILE_OPEN;
    }

    char content[512] = {0};
    if (!fgets(content, sizeof(content)-1, fp)) { // 留一个字节给终止符
        fclose(fp);
        fprintf(stderr, "Failed to read version file: %s\n", version_path);
        return ERR_FILE_READ;
    }
    fclose(fp);

    // 去除空白字符
    trimWhitespace(content);
    if (strlen(content) == 0) {
        fprintf(stderr, "Version file content is empty\n");
        return ERR_CONTENT_EMPTY;
    }

    // 2. 按下划线拆分字符串
    char *parts[10] = {NULL}; // 存储拆分后的部分
    int part_count = 0;
    char *token = strtok(content, "_"); // 第一次拆分

    while (token && part_count < 10) {
        parts[part_count++] = token;
        token = strtok(NULL, "_"); // 后续拆分
    }

    // 检查格式（至少需要6个部分：TBOX_L5_软件版本_零件号_供应商代码_日期）
    if (part_count < 6) {
        fprintf(stderr, "Invalid version format (need at least 6 parts)\n");
        return ERR_FORMAT_INVALID;
    }

    // 提取目标字段
    char *software_ver = parts[2];  // 软件版本
    char *part_number = parts[3];   // 零件号
    char *supplier_code = parts[4]; // 供应商代码
    char cmd[1024] = {0};
    int ret = SUCCESS;

    // 更新软件版本
    snprintf(cmd, sizeof(cmd), 
             "sqlite3 /oemdata/parameters/paramsDb \"update paramstbl set value='%s' where id=208\"", 
             software_ver);
    LOG_Debug("[%s]%s",__FUNCTION__,cmd);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to execute command: %s\n", cmd);
        ret = ERR_CMD_EXEC;
    }

    

    // 更新零件号
    if (ret == SUCCESS) {
        snprintf(cmd, sizeof(cmd), 
                 "sqlite3 /oemdata/parameters/paramsDb \"update paramstbl set value='%s' where id=19\"", 
                 part_number);
        LOG_Debug("[%s]%s",__FUNCTION__,cmd);
        if (system(cmd) != 0) {
            fprintf(stderr, "Failed to execute command: %s\n", cmd);
            ret = ERR_CMD_EXEC;
        }
    }

    // 更新供应商代码
    if (ret == SUCCESS) {
        snprintf(cmd, sizeof(cmd), 
                 "sqlite3 /oemdata/parameters/paramsDb \"update paramstbl set value='%s' where id=20\"", 
                 supplier_code);
        LOG_Debug("[%s]%s",__FUNCTION__,cmd);
        if (system(cmd) != 0) {
            fprintf(stderr, "Failed to execute command: %s\n", cmd);
            ret = ERR_CMD_EXEC;
        }
    }

    // 目前bootloader是固定的
    if (ret == SUCCESS) {
        snprintf(cmd, sizeof(cmd), 
                 "sqlite3 /oemdata/parameters/paramsDb \"update paramstbl set value='20231215' where id=16\"");
        LOG_Debug("[%s]%s",__FUNCTION__,cmd);
        if (system(cmd) != 0) {
            fprintf(stderr, "Failed to execute command: %s\n", cmd);
            ret = ERR_CMD_EXEC;
        }
    }
    return ret;
}

void handle_signal(int sig) 
{
    int ret = 0;
    ret = GetSysRetValue(RMFILEETHERNET);
    if(ret == -1)
    {
        //LOG_Error("%s fail",RMFILEETHERNET);
        printf("%s fail \n",RMFILEETHERNET);
    }

    printf("recv signal:%d\n", sig);
    if (sig == SIGINT || sig == SIGTSTP)
    {
        //LOG_Debug("KILL -9 %d\n",getpid());
        printf("KILL -9 %d\n",getpid());
        kill(getpid(), SIGKILL); 
    }

}
// void CloseSocket(int sockfd)
// {
//     close(sockfd);
//     sockfd = -1;
// }

