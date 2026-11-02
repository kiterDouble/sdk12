
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include "sr_log.h"
#include "log_pack_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 32 * ( EVENT_SIZE + 16 ) )

static int IsFileBeingWritten(const char *filename, int timeout_sec) {
    int fd, wd;
    char buffer[EVENT_BUF_LEN];
    fd_set readfds;
    struct timeval tv;
    int result = 0;
    int length = 0;

    // 初始化 inotify 实例
    fd = inotify_init();
    if (fd < 0) {
        LOG_Error("[%s] inotify_init", __func__);
        return -1;
    }

    // 添加对指定文件的监控
    wd = inotify_add_watch(fd, filename, IN_MODIFY);
    if (wd < 0) {
        LOG_Error("[%s] inotify_add_watch", __func__);
        close(fd);
        return -2;
    }

    // 初始化文件描述符集合和超时时间
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;

    result = select(fd + 1, &readfds, NULL, NULL, &tv);
    if (result > 0) {
        length = read(fd, buffer, EVENT_BUF_LEN);
        inotify_rm_watch(fd, wd);
        close(fd);
        return length;
    } else if (result == 0) {
        inotify_rm_watch(fd, wd);
        close(fd);
        return 0;
    } else {
        LOG_Error("[%s] select", __func__);
        inotify_rm_watch(fd, wd);
        close(fd);
        return -3;
    }
}

uint32_t GetUtcTimestamp() {
    time_t current_time;
    time(&current_time);
    return (uint32_t)current_time;
}

int GetUtcDateTimeStr(uint8_t* dateTimeStrBuffer, int32_t bufferSize){
    time_t rawtime;
    struct tm timeinfo = {0};
    size_t fillLen = 0;

    if (dateTimeStrBuffer == NULL || bufferSize <= 0) {
        return -1;
    }

    memset(dateTimeStrBuffer, 0, bufferSize);

    // 获取当前 UTC 时间
    time(&rawtime);
    if (gmtime_r(&rawtime, &timeinfo) == NULL) {
        // 失败时填充 12 个 '0'，并保证字符串结尾
        fillLen = (bufferSize > 12) ? 12 : (size_t)(bufferSize - 1);
        memset(dateTimeStrBuffer, '0', fillLen);
        dateTimeStrBuffer[fillLen] = '\0';
        return -1;
    }

    // 格式化时间
    strftime((char *)dateTimeStrBuffer, bufferSize, "%y%m%d%H%M%S", &timeinfo);

    return 0;
}

/*
* 判断文件存在并且空闲
* return: 0:空闲； else：非空闲
*/
int IsFileExistsAndIdel(const char *filename) {
    int ret = 0;

    ret = access(filename, F_OK);
    if(ret != 0){
        return -1;
    }

    ret = IsFileBeingWritten(filename, 2);
    LOG_Warn("[%s] IsFileBeingWritten:%d", __func__, ret);
    return ret;
}

uint64_t GetFileSize(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == -1) {
        return 0;
    }
    return (uint64_t)st.st_size;
}

// 检查目录是否存在，不存在则创建
void CheckAndCreateDirectory(const char *dir_path) {
    struct stat st;
    if (stat(dir_path, &st) != 0) {
        if (mkdir(dir_path, 0755) != 0) {
            LOG_Error("create directory failed: %s",dir_path);
        }
    } else if (!S_ISDIR(st.st_mode)) {
        LOG_Error("%s is not a valid directory", dir_path);
    }
}