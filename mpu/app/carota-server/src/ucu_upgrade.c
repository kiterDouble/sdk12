#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include "sr_log.h"
#include "err_code.h"
#include "sdk/ota_api.h"
#include "sdk/pm_server_api.h"
#include "ucu_upgrade.h"
#include "carotaHttpClient.h"

#define UCU_UPGRADE_STATUS_IDLE (0x00)
#define UCU_UPGRADE_STATUS_DOING (0x01)
#define UCU_UPGRADE_STATUS_FINISH (0x02)

#define UCU_UPGRADE_FAIL (-1)
#define UCU_UPGRADE_SUCCESS (0)
#define UCU_UPGRADE_DOING (1)

#define UCU_UPGRADE_FILE_NAME_LENGTH (256)

G_LOCK_DEFINE_STATIC(g_upgradeLock);
static guint8 g_u8upgradeStatus;
static gint32 g_32upgradeResult;
static gchar g_u8updateFileName[UCU_UPGRADE_FILE_NAME_LENGTH] = {0x0,};
static gchar g_upgradelog[256] = {0};
static struct timespec g_resultTimeout;
typedef enum {
    SUCCESS = 0,
    ERR_FILE_OPEN,      // 版本文件打开失败
    ERR_FILE_READ,      // 版本文件读取失败
    ERR_CONTENT_EMPTY,  // 内容为空
    ERR_FORMAT_INVALID, // 格式错误（分割后部分不足）
    ERR_CMD_EXEC        // 命令执行失败
} Result;
static int GetSystemRetValue(const char *cmd)
{
	pid_t status;
	if(NULL == cmd)
	{
		return RET_ERR;
	}
	LOG_Debug("%s exe start", cmd);
	status = system(cmd);
	LOG_Debug("%s exe end:%d", cmd, status);
	if(-1 == status)
	{
		return RET_ERR;
	}
	if(!WIFEXITED(status))
	{
		return RET_ERR;
	}
	return WEXITSTATUS(status);
}
static gint GetResultFromFile(gchar filepath[])
{
    gchar buf[512] = {0x00};
    gint ret = RET_ERR;
    FILE *fp;
    if (!filepath)
    {
        goto error_exit; 
    }
    fp = fopen(filepath, "r");
    if (fp)
    {
        if (fgets(buf, sizeof(buf), fp) != NULL)
        {
            LOG_Debug("Read from File[%s]-[%s]", filepath, buf);
            if (strstr(buf, "success"))
            {
               ret = RET_OK; 
            }         
        }
        else
        {
            LOG_Warn("Read Error:[%s]", strerror(errno));
        }
        fclose(fp);
    }
error_exit:
    return ret;
}
static void WriteOkFile(void)
{
    GetSystemRetValue("echo \"success\" > /oemdata/tmp/ucu_upgrade_result.txt");
    GetSystemRetValue("echo \"success\" > /oemdata/flash/ucu_upgrade_result.txt");
    GetSystemRetValue("sync");
}
static void DeleteOkFile(void)
{
    GetSystemRetValue("rm -f /oemdata/tmp/ucu_upgrade_result.txt");
    GetSystemRetValue("rm -f /oemdata/flash/ucu_upgrade_result.txt");
    GetSystemRetValue("sync");
}
static gint CheckOkFile(void)
{
    gint ret = RET_ERR;
    if ((RET_OK == GetResultFromFile("/oemdata/tmp/ucu_upgrade_result.txt"))
        || (RET_OK == GetResultFromFile("/oemdata/flash/ucu_upgrade_result.txt")))
    {
        ret = RET_OK;
    }
    return ret;
}
static gint StartUpgradePkg(const gchar *realpath, gchar uflag, gchar *errlog)
{
    char cmd[512] = {0};
    int32_t retval = -1;
    GDir *dir = NULL;
    const gchar *filename = NULL;
    gchar *filename_copy = NULL;
    gchar *target_file = NULL;
    gboolean found = FALSE;
    char tarpack[512]={0};
    char tarpack_re[512]={0};
    // 导出库路径
    snprintf(cmd, sizeof(cmd), "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/oemapp/lib/");
    GetSystemRetValue(cmd);
    memset(cmd, 0, sizeof(cmd));
    memset(tarpack, 0, sizeof(tarpack));
    memset(tarpack_re, 0, sizeof(tarpack_re));
    // 打开realpath目录
    printf("realpath:%s\r\n",realpath);
    LOG_Debug("[%s]realpath:%s\r\n",__FUNCTION__,realpath);
    dir = g_dir_open(realpath, 0, NULL);
    if (!dir) {
        if (errlog) {
            snprintf(errlog, 256, "Failed to open directory: %s", realpath);
        }
        WriteSMainModemUpgradeStatus(3);
        return -1;
    }
    LOG_Debug("[%s]ECU upgrade g_dir_open\r\n",__FUNCTION__);
    printf("[%s]ECU upgrade g_dir_open\r\n",__FUNCTION__);
    // 遍历目录查找符合条件的文件
    while ((filename = g_dir_read_name(dir)) != NULL) {
        // 检查文件名是否以"TBOX"开头且以".tar.gz"结尾
        if (g_str_has_prefix(filename, "tbox")) {
            // 拼接完整路径
            target_file = g_build_filename(realpath, filename, NULL);
            LOG_Debug("[%s]target_file:%s\r\n",__FUNCTION__,filename);
            printf("target_file:%s\r\n",filename);
            found = TRUE;
            break;
        }
    }

    if (found && filename) {
        filename_copy = g_strdup(filename);
    }
    g_dir_close(dir); // 关闭目录
    LOG_Debug("[%s]ECU upgrade g_dir_close\r\n",__FUNCTION__);
    // 检查是否找到目标文件
    if (!found || !target_file || !filename_copy) {
        if (errlog) {
            snprintf(errlog, 256, "No TBOX*.tar.gz file found in %s", realpath);
            printf("errlog:%s\r\n",realpath);
        }
        g_free(target_file);
        g_free(filename_copy);
        WriteSMainModemUpgradeStatus(3);
        return -1;
    }
    LOG_Debug("[%s]ECU upgrade-1\r\n",__FUNCTION__);
    snprintf(tarpack, sizeof(tarpack), "/mnt/emmc/upgrade/ethernet/%s", filename_copy);//tarpack_re
    snprintf(tarpack_re, sizeof(tarpack_re), "%s/%s", realpath,filename_copy);
    int test_ret = rename(tarpack_re,tarpack);
    LOG_Debug("[%s]rename(filename,tarpack):%s||%s\r\n",__FUNCTION__,tarpack,tarpack_re);
    system("rm -rf /oemdata/upgrade");
    system("/oemapp/scripts/upgrade_link.sh");
    usleep(1000*3000);
    LOG_Debug("[%s]ECU upgrade:%d\r\n",__FUNCTION__,test_ret);
    snprintf(cmd, sizeof(cmd), "ln %s/%s /oemdata/upgrade/ethernet/tbox_target.tgz", realpath, filename_copy);
    LOG_Debug("[%s]cp cmd :%s\r\n",__FUNCTION__,cmd);
    retval = GetSystemRetValue(cmd);
    sleep(1);
    // 生成执行命令
    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "/oemapp/upgrade/ethernet_upgrade %s", filename_copy);//tarpack
    printf("ECU upgrade:%s\r\n",cmd);
    LOG_Debug("[%s]ECU upgrade2:%s\r\n",__FUNCTION__,cmd);
    g_free(target_file); // 释放内存
    g_free(filename_copy); // 释放复制的文件名
    
    // 执行命令并获取返回值
    retval = GetSystemRetValue(cmd);
    LOG_Debug("[%s]upgrade retval : %d", __FUNCTION__,retval);
    memset(tarpack, 0, sizeof(tarpack));
    return retval;
}

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
    const char *version_path = "/oemapp/upgrade/version.txt";
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
             "sqlite3 /oemdata/parameters/paramsDb \"update paramstbl set value='%s' where id=14\"", 
             software_ver);
    if (system(cmd) != 0) {
        fprintf(stderr, "Failed to execute command: %s\n", cmd);
        ret = ERR_CMD_EXEC;
    }

    // 更新零件号
    if (ret == SUCCESS) {
        snprintf(cmd, sizeof(cmd), 
                 "sqlite3 /oemdata/parameters/paramsDb \"update paramstbl set value='%s' where id=19\"", 
                 part_number);
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
        if (system(cmd) != 0) {
            fprintf(stderr, "Failed to execute command: %s\n", cmd);
            ret = ERR_CMD_EXEC;
        }
    }

    // 目前bootloader是固定的
    if (ret == SUCCESS) {
        snprintf(cmd, sizeof(cmd), 
                 "sqlite3 /oemdata/parameters/paramsDb \"update paramstbl set value='20231215' where id=16\"");
        if (system(cmd) != 0) {
            fprintf(stderr, "Failed to execute command: %s\n", cmd);
            ret = ERR_CMD_EXEC;
        }
    }
    return ret;
}

static gpointer UcuUpgradeHandler(gpointer pArg)
{
    gint32 ret = -1;
    struct timespec curTime; 
    while(1)
    {
        LOG_Debug("g_u8upgradeStatus:%x\r\n",g_u8upgradeStatus);
        switch(g_u8upgradeStatus)
        {
            case UCU_UPGRADE_STATUS_IDLE:
                sleep(1);
                break;
            case UCU_UPGRADE_STATUS_DOING:
                // 初始化
                system("echo 0 > /oemdata/parameters/MainModemUpProgress");
                if(GetVehMisModStatus() == 1)
                {
                    WriteSMainModemUpgradeStatus(4);
                    ret = CheckVehicleAllowUpgrade();
                    sleep(3);
                    LOG_Debug("CheckVehicleAllowUpgrade ret: %d", ret);
                    if( ret == 1)
                    {
                        WriteSMainModemUpgradeStatus(3);
                        g_u8upgradeStatus = UCU_UPGRADE_STATUS_IDLE;
                        break;
                    }
                }
                WriteSMainModemUpgradeStatus(1);
                G_LOCK(g_upgradeLock);
                
                g_32upgradeResult = UCU_UPGRADE_DOING;
                memset(g_upgradelog, 0x0, sizeof(g_upgradelog));

                ret = StartUpgradePkg(g_u8updateFileName, UPGRADE_ALL_FLAG, g_upgradelog);           

                LOG_Debug("g_u8updateFileName log: %d,%s",ret,g_u8updateFileName);
                
                if(ret == 0)
                {
                    GetSystemRetValue("echo \"2\" > /oemdata/tmp/tbox_upgrade_result.txt");
                    WriteOkFile();
                }
                else
                {
                    g_32upgradeResult = UCU_UPGRADE_FAIL;
                }
                g_u8upgradeStatus = UCU_UPGRADE_STATUS_FINISH;
                clock_gettime(CLOCK_MONOTONIC, &g_resultTimeout);
                g_resultTimeout.tv_sec += 60;
                G_UNLOCK(g_upgradeLock);
                break;    
            case UCU_UPGRADE_STATUS_FINISH:
                LOG_Debug("waitint reboot !!!");
                sleep(3);
                break;
            default:
                break;
        }
    }
    return NULL;
}

gint32 UcuUpgradeStart(const gchar filepath[])
{
    LOG_Debug("UcuUpgradeStart g_u8upgradeStatus: %d", g_u8upgradeStatus);
    if ((NULL == filepath) || (0 == strlen(filepath)))
    {
        printf("param filepath illegal, UcuUpgradeStart call fail!");
        return RET_ERR;
    }
    if (UCU_UPGRADE_STATUS_DOING == g_u8upgradeStatus)
    {
        return RET_OK; 
    }

    G_LOCK(g_upgradeLock);
    memset(g_u8updateFileName, 0x0, sizeof(g_u8updateFileName));
    memcpy(g_u8updateFileName, filepath, strlen(filepath));
    
    g_u8upgradeStatus = UCU_UPGRADE_STATUS_DOING;
    G_UNLOCK(g_upgradeLock);
    return RET_OK;
}

int ReadTboxUpgradeResult(gchar errlog[])
{
    // 初始化错误日志（若传入非空缓冲区）
    if (errlog)
    {
        memset(errlog, 0, strlen(errlog)); // 清空缓冲区
    }

    // 打开文件
    FILE *fp = fopen("/oemdata/tmp/tbox_upgrade_result.txt", "r");
    if (fp == NULL)
    {
        if (errlog)
        {
            snprintf(errlog, 128, "打开文件失败: %s", strerror(errno));
        }
        return -1; // 打开失败返回-1
    }

    // 读取文件内容（最多31字节，留1字节给终止符）
    char num_str[32] = {0};
    if (fgets(num_str, sizeof(num_str), fp) == NULL)
    {
        if (errlog)
        {
            if (feof(fp))
            {
                snprintf(errlog, 128, "文件为空（无内容）");
            }
            else
            {
                snprintf(errlog, 128, "读取文件失败: %s", strerror(errno));
            }
        }
        fclose(fp);
        return -1; // 读取失败返回-1
    }
    fclose(fp); // 及时关闭文件

    // 移除字符串末尾的换行符（若存在）
    size_t str_len = strlen(num_str);
    if (str_len > 0 && num_str[str_len - 1] == '\n')
    {
        num_str[str_len - 1] = '\0';
    }

    // 转换字符串为int（严格校验是否为纯数字）
    char *end_ptr = NULL;
    long num = strtol(num_str, &end_ptr, 10); // 使用strtol支持范围检查

    // 校验转换结果：是否完全转换（无多余字符）、是否有有效转换
    if (*end_ptr != '\0' || end_ptr == num_str)
    {
        if (errlog)
        {
            snprintf(errlog, 128, "文件内容不是有效数字: %s", num_str);
        }
        return -1; // 转换失败返回-1
    }

    // 校验数值是否在int范围内（防止溢出）
    if (num < INT_MIN || num > INT_MAX)
    {
        if (errlog)
        {
            snprintf(errlog, 128, "数字超出int范围: %ld", num);
        }
        return -1;
    }

    // 成功：更新错误日志为成功信息
    if (errlog)
    {
        snprintf(errlog, 128, "读取成功，数值: %d", (int)num);
    }

    return (int)num;
}

gint32 UcuUpgradeGetResult(gchar errlog[])
{
    gchar eerlog[300];
    int result = ReadTboxUpgradeResult(eerlog);
    printf("result:%d\r\n",result);
    gint32 ret = UCU_UPGRADE_FAIL;
    switch (g_u8upgradeStatus)
    {
    case UCU_UPGRADE_STATUS_IDLE:
        if (errlog)
        {
            strcpy(errlog, "upgrade not start!");
        }
        ret = UCU_UPGRADE_FAIL;
        break;
    case UCU_UPGRADE_STATUS_DOING:
        if (errlog)
        {
            strcpy(errlog, "upgrade not finished!");
        }
        ret = UCU_UPGRADE_DOING;
        break;
    case UCU_UPGRADE_STATUS_FINISH:
        if (errlog)
        {
            strcpy(errlog, g_upgradelog);
        }
        ret = g_32upgradeResult;
        break;
    default:
        if (errlog)
        {
            strcpy(errlog, "upgrade status illegal!");
        }
        if(result ==UCU_UPGRADE_STATUS_FINISH )
        {
            ret = UCU_UPGRADE_SUCCESS;
        }
        else
        {
            ret = UCU_UPGRADE_FAIL;
        }
        break;
    }

    return ret;
}

gint32 UcuUpgradeinit(void)
{
    gint32 ret = -1;

    if (RET_OK == CheckOkFile())
    {
        LOG_Debug("get upgrade result success.reset timeout.");
        //DeleteOkFile();
        clock_gettime(CLOCK_MONOTONIC, &g_resultTimeout);
        g_resultTimeout.tv_sec += 90;
        g_u8upgradeStatus = UCU_UPGRADE_STATUS_FINISH;
        g_32upgradeResult = UCU_UPGRADE_SUCCESS;
    }
    else
    {    
    g_u8upgradeStatus = UCU_UPGRADE_STATUS_IDLE;
    g_32upgradeResult = UCU_UPGRADE_FAIL;
    }
    memset(g_u8updateFileName, 0x0, sizeof(g_u8updateFileName));
    memset(g_upgradelog, 0x0, sizeof(g_upgradelog));
    pthread_attr_t attr;
    pthread_t pthreadId;
    
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	ret = pthread_create(&pthreadId, &attr, UcuUpgradeHandler, NULL);
	if(ret != 0)
	{
		LOG_Error("pthread_create UcuUpgradeHandler %s", strerror(errno));
        return RET_ERR;
	}

    return RET_OK;
}
