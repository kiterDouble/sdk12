
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

static AppContext_t g_app_context;
static int g_context_initialized = 0;
#define CAN_COUNT (2 * 60 * 4100)

/**
 * 获取全局应用上下文实例
 */
AppContext_t *GetAppContext(void)
{
    return &g_app_context;
}

/**
 * 初始化应用上下文
 */
int AppContextInit(void)
{
    if (g_context_initialized)
    {
        return 0;
    }
    memset(&g_app_context, 0, sizeof(AppContext_t));

    /* 初始化CAN FIFO */
    size_t fifo_len = CAN_COUNT * sizeof(Can_frame_t);
    size_t fifo_size = roundup_pow_of_two_u64(fifo_len);

    if (kfifo_init_elem(&g_app_context.can_fifo, fifo_size,sizeof(Can_frame_t)) != 0)
    {
        return -1;
    }

    /* 初始化状态 */
    SetCollectStatus(COLLECT_STATUS_RUNNING); /* 默认开启采集 */
    SetBackupFinishFlag(BACKUP_NOT_FINISH);

    g_context_initialized = 1;
    return 0;
}

/**
 * 设置备份完成标志
 * 注意：单线程写场景，volatile 保证可见性，无需加锁
 */
void SetBackupFinishFlag(int value)
{
    g_app_context.backup_finish_flag = value;
}

/**
 * 获取备份完成标志
 * 注意：单线程读场景，volatile 保证可见性，无需加锁
 */
int GetBackupFinishFlag(void)
{
    return g_app_context.backup_finish_flag;
}

/**
 * 设置数据采集状态
 * 注意：单线程写场景，volatile 保证可见性，无需加锁
 */
void SetCollectStatus(int value)
{
    g_app_context.collect_status = value;
}

/**
 * 获取数据采集状态
 * 注意：单线程读场景，volatile 保证可见性，无需加锁
 */
int GetCollectStatus(void)
{
    return g_app_context.collect_status;
}

void AppContextDeinit(void)
{
    if (!g_context_initialized)
    {
        return;
    }

    kfifo_free(&g_app_context.can_fifo);
    /* 清理文件上下文 */
    if (g_app_context.main_file_ctx.current_file)
    {
        fclose(g_app_context.main_file_ctx.current_file);
        g_app_context.main_file_ctx.current_file = NULL;
    }

    if (g_app_context.backup_file_ctx.current_file)
    {
        fclose(g_app_context.backup_file_ctx.current_file);
        g_app_context.backup_file_ctx.current_file = NULL;
    }

    memset(&g_app_context, 0, sizeof(AppContext_t));
    g_context_initialized = 0;
}

int GetSystemRetValue(const char *cmd)
{
    pid_t status;
    if (NULL == cmd)
    {
        return -1;
    }

    status = system(cmd);
    //    LOG_Debug("%s exe start status = %d", cmd,status);
    if (-1 == status)
    {
        return -1;
    }

    if (!WIFEXITED(status))
    {
        return -1;
    }
    return WEXITSTATUS(status);
}

int GetEmmcSize()
{
    const char *device = "/dev/mmcblk0p1";
    int fd;
    unsigned long long size; // 用于存储设备大小（以字节为单位）

    fd = open(device, O_RDONLY);
    if (fd < 0)
    {
        LOG_Error("Failed to open device");
        return -1;
    }

    if (ioctl(fd, BLKGETSIZE64, &size) == -1)
    {
        LOG_Error("Failed to get device size");
        close(fd);
        return -1;
    }

    LOG_Debug("Device size: %llu bytes (%.2f GB)\n", size, size / (1024.0 * 1024 * 1024));
    close(fd);
    return ((int)(size / (1024.0 * 1024 * 1024)));
}