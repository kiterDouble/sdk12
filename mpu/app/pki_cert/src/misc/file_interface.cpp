#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <string.h>  
#include <fcntl.h>
#include <errno.h>
#include"sr_log.h"
#include "file_interface.h"

#if 0
// 函数：加锁
static int LockFile(int *fd_lock) 
{
    struct flock fl;
    *fd_lock = open(LOCK_FILE, O_CREAT | O_RDWR, 0666);

    if ((*fd_lock) == -1) {
        LOG_Error("Failed to open lock file");
        return -1;
    }
    // 设置文件锁的参数
    fl.l_type = F_WRLCK; // 写锁
    fl.l_whence = SEEK_SET;
    fl.l_start = 0; // 从文件开头开始
    fl.l_len = 0;   // 锁定整个文件

    // 尝试加锁
    if (fcntl(*fd_lock, F_SETLKW, &fl) == -1) {
        LOG_Error("Failed to lock file");
        close(*fd_lock);
        return -1;
    }
    return 0;
}

// 函数：解锁
static int UnLockFile(int fd_lock) 
{
    struct flock fl;
    if(fd_lock == -1)
    {
        LOG_Error("fd_lock -1");
        return -1;
    }
    // 设置文件锁的参数
    fl.l_type = F_UNLCK; // 解锁
    fl.l_whence = SEEK_SET;
    fl.l_start = 0; // 从文件开头开始
    fl.l_len = 0;   // 解锁整个文件

    // 解锁
    if (fcntl(fd_lock, F_SETLK, &fl) == -1) {
        LOG_Error("Failed to unlock file");
        return -1;
    }

    close(fd_lock);
    return 0;
}

#endif
int OpenWriteFile(const char *filepath , char *data,size_t datalen)
{
    if(!data || !filepath) 
    {
        LOG_Error(" OpenWriteFile data NULL\n");
        return -1;
    }
    // int fd_lock = -1;
    // int ret = LockFile(&fd_lock);
    // if(ret < 0 )
    // {
    //     LOG_Error("LockFile fail \n");
    //     return -1;
    // }
    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
        LOG_Error("Error opening or creating file \n");
        
        return -1;
    }

    size_t bytes_written = fwrite(data, sizeof(char), datalen, file);
    if (bytes_written < datalen) {
        LOG_Error("Error writing to file \n");
        fclose(file); 
        
        return -1;
    }
    fflush(file);
    LOG_Debug("Successfully wrote %zu \n", bytes_written);
    if (fclose(file) != 0) {
        LOG_Error("Error closing file \n");
        
        return -1;
    }

    return 0;
}

int OpenReadFile(const char *filepath , char *data, int datalen)
{
    
    if(!data || !filepath)
    {
        LOG_Error(" OpenReadFile data NULL\n");
        return -1;
    }

    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        LOG_Error("Error opening or creating file \n");
        return -1;
    }

    size_t bytes = fread(data, sizeof(char),datalen , file);
    if (bytes < 0) {
        LOG_Error("Error read to file \n");
        fclose(file);
        return -1;
    }
    LOG_Debug("Successfully read %zu \n", bytes);
    if (fclose(file) != 0) {
        LOG_Error("Error closing file \n");
        return -1;
    }

    return 0;
}



int WriteFile(FILE *file , unsigned char *data,size_t datalen)
{
    if(!data || !file)
    {
        LOG_Error(" OpenWriteFile data NULL\n");
        return -1;
    }

    size_t bytes_written = fwrite(data, sizeof(unsigned char), datalen, file);
    if (bytes_written < datalen) {
        LOG_Error("Error writing to file \n");
        fclose(file);
        return -1;
    }
    fflush(file);
    LOG_Debug("Successfully wrote %zu \n", bytes_written);
    return 0;
}


double GetFileSize(const char *filename) 
{
    struct stat st;
    if (stat(filename, &st) != 0) {
        LOG_Error("stat error ");
        return -1;
    }

    double size = (double)st.st_size;

    size /= (1024 * 1024);

    LOG_Debug("文件 %s 的大小: %.2f \n", filename, size);

    return size;
}


int writeIntToFile(const char *filepath, int data) 
{
    // 打开文件，"w"表示写模式，将覆盖原有内容
    FILE *file = fopen(filepath, "w");
    if (!file) {
        LOG_Error("Error opening file for writing");
        return -1;
    }

    // 写入整数数据
    fprintf(file, "%d\n", data);

    // 关闭文件
    fclose(file);
    return 0;
}


int readIntFromFile(const char *filepath, int *data) 
{
    // 打开文件，"r"表示读取模式
    FILE *file = fopen(filepath, "r");
    if (!file) {
        LOG_Error("Error opening file for reading");
        // 文件不存在，以写模式创建文件
        file = fopen(filepath, "w");
        if (file == NULL) 
        {
            LOG_Error("Failed to create file");
            return -1;
        }
        // 关闭后再以读模式打开
        fclose(file);
        file = fopen(filepath, "r");
        if(!file)
        {
            LOG_Error("Failed to open file");
            return -1;
        }
    }

    // 读取整数数据
    if (fscanf(file, "%d", data) != 1) {
        LOG_Error("Error reading integer from file");
        fclose(file);
        return -1;
    }

    // 关闭文件
    fclose(file);
    return 0;
}

int writeIntWithTimestampToFile(const char *filepath, int flag) 
{
    // 打开文件，"w"表示写模式，将覆盖原有内容
    FILE *file = fopen(filepath, "a");
    if (!file) {
        LOG_Error("Error opening file for writing");
        return -1;
    }

    // 获取当前时间
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    // 格式化当前时间
    char timeStr[64];
    if (strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", t) == 0) {
        LOG_Error("Failed to format time");
        fclose(file);
        return -1;
    }
    if(flag == 1)
    {
        // 写入整数数据和时间戳
        fprintf(file, "FAIL, Timestamp: %s\n", timeStr);
    }else{

        fprintf(file, "success, Timestamp: %s\n", timeStr);
    }
    

    // 关闭文件
    fclose(file);
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


static int FileProcessing(const char *path) 
{
    int ret = 0;
    ret = GetFileSize(path);
    if(ret < 0)
    {
        LOG_Error("GetFileSize fail");
        return -1;
    }

    if(ret > FILE_MAX_SIZE)
    {
        if (remove(path) == 0) {
            LOG_Debug("Deleted: %s\n", path);
        } else {
            LOG_Error("remove %s fail\n",path);
        }
    }
}


