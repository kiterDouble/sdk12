#include "carotaFilehandle.h"
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <unistd.h>
#include "sr_log.h"


#include "err_code.h"
#include "ucu_status.h"

/**
 * @brief 从文件 /mnt/emmc/upgrade/status 获取UCU状态
 * @param status [out] 用于存储读取到的状态值的指针
 * @note 如果文件不存在或读取失败，将状态设置为默认值 UCU_STATUS_NORMAL
 * @return RET_OK 成功, RET_ERR 失败
 */
gint32 UcuStatusFileGetStatus(int *status)
{
    const char *file_path = "/mnt/emmc/upgrade/status";

    if (status == NULL) {
        LOG_Debug("UcuStatusGetStatus: status pointer is NULL");
        return RET_ERR;
    }

    // 1. 打开文件 (O_RDONLY)
    int fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        // 如果文件不存在，这是正常情况，返回默认状态
        if (errno == ENOENT) {
            LOG_Debug("UcuStatusGetStatus: File %s not found, returning default status NORMAL.", file_path);
            *status = UCU_STATUS_NORMAL;
            return RET_OK;
        } else {
            LOG_Debug("UcuStatusGetStatus: Failed to open file %s, errno=%d", file_path, errno);
            return RET_ERR;
        }
    }

    // 2. 读取文件内容
    char buffer[32];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1); // 留一个字节给'\0'
    if (bytes_read < 0) {
        LOG_Debug("UcuStatusGetStatus: Failed to read from file %s, errno=%d", file_path, errno);
        close(fd);
        return RET_ERR;
    }

    // 3. 将读取的内容转换为整数
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0'; // 添加字符串结束符
        char *endptr;
        long val = strtol(buffer, &endptr, 10);

        // 检查转换是否成功
        if (endptr > buffer) {
            *status = (int)val;
            LOG_Debug("UcuStatusGetStatus: Successfully read status %d from %s", *status, file_path);
            close(fd);
            return RET_OK;
        }
    }

    // 如果文件为空或内容不是有效数字，返回默认状态
    LOG_Debug("UcuStatusGetStatus: File %s is empty or contains invalid data, returning default status NORMAL.", file_path);
    *status = UCU_STATUS_NORMAL;
    close(fd);
    return RET_OK;
}

/**
 * @brief 创建目录路径 (递归创建)
 * @param path 要创建的目录路径，例如 "/mnt/emmc/upgrade"
 * @return RET_OK 成功, RET_ERR 失败
 */
gint32 create_directory_if_not_exists(const char *path)
{
    // 复制一份，因为我们会修改它
    char *dir_path = strdup(path);
    if (dir_path == NULL) {
        LOG_Debug("create_directory_if_not_exists: strdup failed");
        return RET_ERR;
    }

    char *p = dir_path;
    // 跳过开头的 '/'
    if (*p == '/') p++;

    // 逐个目录创建
    while ((p = strchr(p, '/')) != NULL) {
        *p = '\0'; // 临时截断字符串
        if (mkdir(dir_path, 0755) != 0 && errno != EEXIST) {
            LOG_Debug("create_directory_if_not_exists: mkdir failed for %s, errno=%d", dir_path, errno);
            free(dir_path);
            return RET_ERR;
        }
        *p = '/'; // 恢复 '/'
        p++; // 移动到下一个字符
    }

    // 创建最后一级目录
    if (mkdir(dir_path, 0755) != 0 && errno != EEXIST) {
        LOG_Debug("create_directory_if_not_exists: mkdir failed for %s, errno=%d", dir_path, errno);
        free(dir_path);
        return RET_ERR;
    }

    free(dir_path);
    return RET_OK;
}
/**
 * @brief 设置UCU状态到文件 /mnt/emmc/upgrade/status
 * @param status 要设置的状态值 (例如 UCU_STATUS_FOTA)
 * @return RET_OK 成功, RET_ERR 失败
 */
gint32 UcuStatusFileSetStatus(int status)
{
    const char *file_path = "/mnt/emmc/upgrade/status";
    const char *dir_path = "/mnt/emmc/upgrade";

    // 1. 确保目录存在
    if (create_directory_if_not_exists(dir_path) != RET_OK) {
        LOG_Debug("UcuStatusSetStatus: Failed to create directory %s", dir_path);
        return RET_ERR;
    }
    int fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        LOG_Debug("UcuStatusSetStatus: Failed to open file %s, errno=%d", file_path, errno);
        return RET_ERR;
    }

    // 3. 将状态转换为字符串并写入
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "%d\n", status);
    if (len < 0) {
        LOG_Debug("UcuStatusSetStatus: snprintf failed");
        close(fd);
        return RET_ERR;
    }

    if (write(fd, buffer, len) != len) {
        LOG_Debug("UcuStatusSetStatus: Failed to write to file %s, errno=%d", file_path, errno);
        close(fd);
        return RET_ERR;
    }

    // 4. 确保数据写入磁盘
    fsync(fd);

    // 5. 关闭文件
    close(fd);

    LOG_Debug("UcuStatusSetStatus: Successfully set status to %d in %s", status, file_path);
    return RET_OK;
}

int read_status(void) 
{
    FILE *fp = fopen("/oemdata/tmp/tbox_upgrade_status.txt", "r");
    if (fp == NULL) {
        return -2; // 文件打开失败
    }

    // 读取文件内容（最多读取15字节，留一个给终止符）
    char buf[16] = {0};
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        fclose(fp);
        return -2; // 读取内容失败
    }
    fclose(fp);

    // 去除可能的换行符（处理echo命令自动添加的换行）
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }

    // 判断读取到的内容
    if (strcmp(buf, "1") == 0) {
        return 1;
    } else if (strcmp(buf, "2") == 0) {
        return 0;
    } else if (strcmp(buf, "-1") == 0) {
        return -1;
    } else {
        return -2; // 内容不匹配
    }
}

/**
 * @brief 读取主模组升级状态从文件 /oemdata/parameters/MainModemUpgradeStatus
 * @return int 状态值 (0-正常，1-升级中，2-升级成功，3-升级失败，4-其他)
 *         如果文件不存在、读取失败或值不在 0-4 范围内，返回默认值 0
 */
int ReadSMainModemUpgradeStatus(void)
{
    const char *file_path = "/oemdata/parameters/MainModemUpgradeStatus";
    char cmd[512] = {0};
    char buffer[32] = {0};
    FILE *fp = NULL;
    
    // 使用 cat 命令读取文件
    snprintf(cmd, sizeof(cmd), "cat %s 2>/dev/null", file_path);
    
    fp = popen(cmd, "r");
    if (fp == NULL) {
        LOG_Debug("ReadMainModemUpgradeStatus: Failed to execute command, returning default status 0");
        return 0;
    }
    
    // 读取输出内容
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // 去除换行符和空格
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r' || buffer[len - 1] == ' ')) {
            buffer[--len] = '\0';
        }
        LOG_Debug("ReadSMainModemUpgradeStatus: Read status %s ", buffer);
        // 使用 atoi 转换
        int status = atoi(buffer);
        pclose(fp);
        
        // 验证读取的值是否在有效范围内 (0-4)
        if (status >= 0 && status <= 4) {
            LOG_Debug("ReadSMainModemUpgradeStatus: Successfully read valid status %d from %s", status, file_path);
            
        } else {
            LOG_Debug("ReadSMainModemUpgradeStatus: Invalid status %d (not in 0-4 range), returning default status 0", status);
            return 0;
        }
        if(status == 1 || status == 4 )
        {
            return 1;
        }else if(status == 2)
        {
            return 2;
        }else{
            return -1;
        }
    }
    
    pclose(fp);
    LOG_Debug("ReadSMainModemUpgradeStatus: File not found or empty, returning default status 0");
    return 0;
}
/**
 * @brief 读取主模组升级状态从文件 /oemdata/parameters/MainModemUpgradeStatus
 * @return int 状态值 (0-正常，1-升级中，2-升级成功，3-升级失败，4-其他)
 *         如果文件不存在、读取失败或值不在 0-4 范围内，返回默认值 0
 */
int ReadMqttUpgradeStatus(void)
{
    const char *file_path = "/oemdata/parameters/MainModemUpgradeStatus";
    char cmd[512] = {0};
    char buffer[32] = {0};
    FILE *fp = NULL;
    
    // 使用 cat 命令读取文件
    snprintf(cmd, sizeof(cmd), "cat %s 2>/dev/null", file_path);
    
    fp = popen(cmd, "r");
    if (fp == NULL) {
        LOG_Debug("ReadMainModemUpgradeStatus: Failed to execute command, returning default status 0");
        return 0;
    }
    
    // 读取输出内容
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // 去除换行符和空格
        size_t len = strlen(buffer);
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r' || buffer[len - 1] == ' ')) {
            buffer[--len] = '\0';
        }
        LOG_Debug("ReadSMainModemUpgradeStatus: Read status %s ", buffer);
        // 使用 atoi 转换
        int status = atoi(buffer);
        pclose(fp);
        
        // 验证读取的值是否在有效范围内 (0-4)
        if (status >= 0 && status <= 4) {
            LOG_Debug("ReadSMainModemUpgradeStatus: Successfully read valid status %d from %s", status, file_path);
            
        } else {
            LOG_Debug("ReadSMainModemUpgradeStatus: Invalid status %d (not in 0-4 range), returning default status 0", status);
            return 0;
        }
        if(status == 1 )
        {
            return 1;
        }else if(status == 2)
        {
            return 2;
        }else if(status == 4)
        {
            return 4;
        }
        else{
            return -1;
        }
    }
    
    pclose(fp);
    LOG_Debug("ReadSMainModemUpgradeStatus: File not found or empty, returning default status 0");
    return 0;
}

/**
 * @brief 设置主模组升级状态到文件 /oemdata/parameters/MainModemUpgradeStatus
 * @param status 要设置的状态值 (只能是 0,1,2,3,4)
 * @return RET_OK 成功，RET_ERR 失败
 */
gint32 WriteSMainModemUpgradeStatus(int status)
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
        return RET_ERR;
    }
    
    LOG_Debug("WriteSMainModemUpgradeStatus: Successfully wrote status %d", status);
    return RET_OK;
}

gint32 CarotaFileVersionGetVersion(gchar *version_buffer, gsize buffer_size)
{
    const char *file_path = "/oemapp/upgrade/version_uds.txt";
    gchar buffer[256];  // 足够大的缓冲区来存放整个文件内容
    gchar *buffer_copy; // 用于分割操作的副本
    FILE *fp;

    if (version_buffer == NULL || buffer_size < 14) {
        LOG_Debug("CarotaFileVersionGetVersion: Invalid input parameters");
        return RET_ERR;
    }

    // 检查文件是否存在
    fp = fopen(file_path, "r");
    if (fp == NULL) {
        LOG_Debug("CarotaFileVersionGetVersion: Failed to open file %s", file_path);
        return RET_ERR;
    }

    // 读取文件内容
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        LOG_Debug("CarotaFileVersionGetVersion: Failed to read from file %s", file_path);
        fclose(fp);
        return RET_ERR;
    }
    fclose(fp);

    // 清理换行符
    g_strstrip(buffer);

    // 创建buffer的副本用于strtok_r操作，避免修改原buffer
    buffer_copy = g_strdup(buffer);
    if (buffer_copy == NULL) {
        LOG_Debug("CarotaFileVersionGetVersion: Failed to duplicate buffer");
        return RET_ERR;
    }

    gchar *tokens[10]; // 假设最多有10个部分
    gint32 token_count = 0;
    gchar *save_ptr = NULL;
    gchar *token = strtok_r(buffer_copy, "_", &save_ptr);

    // 分割字符串并存储各部分
    while (token != NULL && token_count < 10) {
        tokens[token_count] = token;
        token_count++;
        token = strtok_r(NULL, "_", &save_ptr);
    }

    // 检查是否有足够的部分
    if (token_count >= 3) {
        // 第三个部分 (索引为2) 是版本号
        gchar *version_part = tokens[2];
        
        // 确保版本号长度为14位
        if (strlen(version_part) >= 14) {
            strncpy(version_buffer, version_part, 14);
            version_buffer[14] = '\0';
            LOG_Debug("CarotaFileVersionGetVersion: Successfully extracted version %s from %s", version_buffer, file_path);
            g_free(buffer_copy);
            return RET_OK;
        } else {
            LOG_Debug("CarotaFileVersionGetVersion: Version string length is less than 14: %s", version_part);
            g_free(buffer_copy);
            return RET_ERR;
        }
    }

    LOG_Debug("CarotaFileVersionGetVersion: Could not find version string in expected format in %s", file_path);
    g_free(buffer_copy);
    return RET_ERR;
}

// 从文件读取kl15值
int ReadKl15StatusFromFile(uint8_t *kl15_value)
{
    FILE *file = NULL;
    uint8_t temp_value = 0;
    int result = RET_OK;
    
    file = fopen("/oemdata/parameters/kl15_status", "r");
    if (file == NULL) {
        LOG_Error("Failed to open file /oemdata/parameters/kl15_status for reading, error: %d", errno);
        return RET_ERR;
    }
    
    if (fscanf(file, "%"SCNu8, &temp_value) != 1) {
        LOG_Error("Failed to read from file /oemdata/parameters/kl15_status");
        result = RET_ERR;
        temp_value = 0; // 默认值
    } else {
        *kl15_value = temp_value;
    }
    
    fclose(file);
    if (result == RET_OK) {
        LOG_Debug("Successfully read kl15 status %"PRIu8" from file /oemdata/parameters/kl15_status", *kl15_value);
    }
    return result;
}
// 保存kl15值到文件
int SaveKl15StatusToFile(uint8_t kl15_value)
{
    FILE *file = NULL;
    int result = RET_OK;
    int dir_result;
    
    // 确保目录存在
    dir_result = mkdir("/oemdata/parameters", 0755);
    if (dir_result != 0 && errno != EEXIST) {
        LOG_Error("Failed to create directory /oemdata/parameters, error: %d", errno);
        return RET_ERR;
    }
    
    // 检查文件是否存在，如果存在则删除
    if (access("/oemdata/parameters/kl15_status", F_OK) == 0) {
        if (remove("/oemdata/parameters/kl15_status") != 0) {
            LOG_Error("Failed to remove existing file /oemdata/parameters/kl15_status, error: %d", errno);
        }
    }
    
    file = fopen("/oemdata/parameters/kl15_status", "w");
    if (file == NULL) {
        LOG_Error("Failed to open file /oemdata/parameters/kl15_status for writing, error: %d", errno);
        return RET_ERR;
    }
    
    if (fprintf(file, "%"PRIu8, kl15_value) < 0) {
        LOG_Error("Failed to write to file /oemdata/parameters/kl15_status, error: %d", errno);
        result = RET_ERR;
    }
    
    fclose(file);
    if (result == RET_OK) {
        LOG_Debug("Successfully saved kl15 status %"PRIu8" to file /oemdata/parameters/kl15_status", kl15_value);
    }
    return result;
}

