
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <dirent.h>

#include <unistd.h>
#include <dirent.h>
#include "sr_log.h"
#include "file_interface.h"
#include "tmier_interface.h"
#include "system_interface.h"
#include "completion_timeout.h"

// 查找最新的 can_YYYY_MM_DD_HH_MM_SS.asc 文件
char* FindLatestCanFile(const char *directory) 
{
    struct dirent *entry;
    struct statx file_stat;
    
    char full_path[512];
    char *latest_file = NULL; // 存储最新文件路径
    long long  latest_time = 0; // 存储最新文件的修改时间

    // 打开指定目录
    DIR *dir = opendir(directory);
    if (dir == NULL) {
        LOG_Error("无法打开目录");
        return NULL;
    }

    // 遍历目录下的文件
    while ((entry = readdir(dir)) != NULL) {
        // 检查文件是否以 "can_" 开头且以 ".asc" 结尾
        if (strncmp(entry->d_name, FILE_PREFIX, strlen(FILE_PREFIX)) == 0 &&
            strstr(entry->d_name, FILE_SUFFIX) != NULL) {
            
            // 构建文件完整路径
            snprintf(full_path, sizeof(full_path), "%s%s", directory, entry->d_name);
            
            // 获取文件状态  file_statx.stx_btime.tv_sec
            if (statx(AT_FDCWD, full_path, AT_STATX_SYNC_AS_STAT, STATX_ALL, &file_stat) == 0) {
                // 如果当前文件的修改时间大于当前最新时间，更新
                //long long mtime_nsec = (long long)file_stat.stx_mtime.tv_sec * 1000000000 + file_stat.stx_mtime.tv_nsec;
                long long mtime_nsec = 0;
                  // 检查是否支持创建时间
                if ((file_stat.stx_mask & STATX_BTIME)) 
                {
                    mtime_nsec = (long long)file_stat.stx_btime.tv_sec * 1000000000 + file_stat.stx_btime.tv_nsec;  
//                    LOG_Debug("The creation time is supported by the file system "); 
                }else{

                    LOG_Error("The creation time is not supported by the file system");
                }
                
//                LOG_Debug("mtime_nsec: %lld    latest_time: %lld",mtime_nsec,latest_time);
                if (mtime_nsec > latest_time) {
                    latest_time = mtime_nsec;
                    free(latest_file); // 释放之前的内存
                    latest_file = strdup(entry->d_name); // 复制路径
                }
            }
        }
    }

    closedir(dir);
    return latest_file; // 返回最新文件路径
}

static int HasValidFormat(const char *filename) 
{
    //if (strstr(filename, FILE_EXTENSION))
    if (strlen(filename) >= 27 && strstr(filename, FILE_EXTENSION)) 
    {
        return 1;
    }
    return 0;
}

int TarGzCompress(const char *filename)
{
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char cmd[256] = {0};
    char syscmd[256] = {0};
    char filepath[512] = {0};
    int ret = 0;
    dir = opendir(DIRECTORY);
    if (!dir) {
        LOG_Error("opendir FAIL");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) 
    {
        memset(filepath,0,sizeof(filepath));
        if (entry->d_type == DT_DIR || !HasValidFormat(entry->d_name)) {
            continue;
        }

        
        if(filename)
        {
            if((!strcmp(filename,entry->d_name)))
            {
//                LOG_Debug("Skip this file: %s",filename);
                continue;
            }
        }
        memset(cmd,0,sizeof(cmd));
        snprintf(filepath, sizeof(filepath), "%s", entry->d_name);
        memcpy(cmd,filepath,strlen(filepath) -4);
        LOG_Debug("TarGzCompress %s",filepath);
        sprintf(syscmd," cd  %s ; tar -zcvf %s.tgz %s & echo $! > /sys/fs/cgroup/cpu,cpuacct/limit_cpu/tasks; wait $!",DIRECTORY,cmd,filepath);
        

        ret =  GetSystemRetValue(syscmd);
        if(ret == -1)
        {
            LOG_Error("%s fail ret = %d",syscmd,ret);

        }else{
            LOG_Debug("TarGzCompress %s exec ret = %d", filepath, ret);
            memset(syscmd,0,sizeof(syscmd));
            sprintf(syscmd," cd  %s ;rm -rf  %s;",DIRECTORY,entry->d_name);
            
            ret =  GetSystemRetValue(syscmd);
            if(ret == -1)
            {
                LOG_Error("%s fail ",filepath);
            }
        }

    }

    closedir(dir);
    return 0;
}


// 检查文件大小并决定是否创建新文件
FILE *CheckRotateFile(FILE *current_file, const char *current_path,long long ms_timestamp) 
{
    char cmd[256] = {0};
    int ret = 0;
    if (!current_file) return NULL;

    // 获取当前文件大小
    struct stat st;
    if (fstat(fileno(current_file), &st) != 0) {
        LOG_Error("fstat failed");
        return current_file;
    }

    // 如果未超过限制，继续使用当前文件
    if (st.st_size < MAX_FILE_SIZE) {
        return current_file;
    }

    // 关闭当前文件
    fclose(current_file);
    current_file = NULL;
 
    // 生成新文件名
    char new_path[256];
    GenerateNewFilename(ms_timestamp,new_path, sizeof(new_path));

    // 创建新文件
    FILE *new_file = fopen(new_path, "w");
    if (!new_file) {
        LOG_Error("Failed to create new file");
        return NULL;
    }
    WriteAscHeader(new_file,ms_timestamp);
    Complete(GetComp());
    //LOG_Debug("已创建新文件: %s", new_path);
    return new_file;
}

// 主写入函数
int WriteDataWithRotation(CAN_Frame *frames,int count) 
{
    static FILE *current_file = NULL;
    static char current_path[256] = {0};
    int ret = 0;
    long pos = 0;
    CAN_Frame frame = {0};
    
    GenerateNewFilename(frames[0].timestamp,current_path, sizeof(current_path));
    
    if (!current_file) {
        current_file = fopen(current_path, "a");
        if (!current_file) {
            LOG_Error("Initial file creation failed");
            return -1;
        }
        WriteAscHeader(current_file,frames[0].timestamp);
    }

    // 检查文件大小并轮转
    current_file =  current_file = CheckRotateFile(current_file, current_path,frames[0].timestamp);
    if (!current_file) 
    {
        LOG_Error("Check Rotate File current_file = NULL");
        return -1;
    }
        
   
    // 写入数据
    char *buffer = (char *)malloc(1000000*(sizeof(char)));
    if(!buffer)
    {
        LOG_Error("malloc buffer fail");
	return -1;
    }

    memset(buffer,0,1000000);
    for (int i = 0; i < count; i++) {
        CAN_Frame *f = &frames[i];
        double time = CalculateRelativeTime(GetHeaderTime(),f->timestamp);
        long n = snprintf(buffer + pos, 1000000 - pos,
                        "\n%.6f\t\t%d\t%X\t%s\t%c\t%u\t%02X %02X %02X %02X %02X %02X %02X %02X",
                        time,
                        f->channel,
                        f->id,
                        f->direction == 'R' ? "RX" : "TX",
                        f->dataflag,
                        f->dlc,
                        f->data[0], f->data[1], f->data[2], f->data[3],
                        f->data[4], f->data[5], f->data[6], f->data[7]);
        if (n < 0 || (pos + n) >= 1000000) {
            LOG_Error( "错误：格式化失败或缓冲区不足 (帧 %d) pos: %lld n:%lld count:%d", i,pos,n,count);
            break;
        }

        pos += n;  // 更新写入位置
    }

    if (pos > 0) {
        //ssize_t bytes_written = fprintf(current_file, buffer);
        ssize_t bytes_written = fwrite(buffer,sizeof(char),pos,current_file);
        if (bytes_written == -1) {
            LOG_Error("write 失败");
        } else if ((size_t)bytes_written != (size_t)pos) {
            LOG_Error( "警告：未完全写入所有数据（请求 %d 字节，实际写入 %zd 字节）\n", pos, bytes_written);
        }
        
    }

    if(buffer)
    {
        free(buffer);
    }
    
    ret = fflush(current_file);
    if(ret < 0)
    {
        LOG_Error("%s fflush fail ret = %d",__FUNCTION__,ret);
    }
    return 0;
}


static void FreeFileList(file_info_t *files, off_t count) 
{
    for (off_t i = 0; i < count; i++) 
    {
        if (files[i].path) {
            free(files[i].path);
            (files[i].path) = NULL;
        }
    }

    if(files)
    {
        free(files);
        files = NULL;
    }
    
}

static int compare_files_by_time(const void *a, const void *b) {
    const file_info_t *fa = (const file_info_t *)a;
    const file_info_t *fb = (const file_info_t *)b;
    
    if (fa->mtime < fb->mtime) return -1;
    if (fa->mtime > fb->mtime) return 1;
    return 0;
}

int GetDirectoryInfo(const char *dir_path, off_t *total_size, file_info_t **files, off_t *file_count) 
{
    DIR *dir=NULL;
    struct dirent *entry = {0};
    struct stat statbuf={0};
    char full_path[1024] = {0};
    *total_size = 0;
    *file_count = 0;
    dir = opendir(dir_path);
    if (dir == NULL) {
        LOG_Error("无法打开目录 %s: %s\n", dir_path, strerror(errno));
        return -1;
    }
    
    // 第一次遍历：统计文件数量
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        if (stat(full_path, &statbuf) == -1) {
            LOG_Error("无法获取文件信息 %s: %s\n", full_path, strerror(errno));
            continue;
        }
        
        if (S_ISREG(statbuf.st_mode)) {
            (*file_count)++;
        }
    }
    
    rewinddir(dir);
    LOG_Debug("file_count:%lu",(*file_count));
    // 分配内存
    *files = (file_info_t *)calloc((*file_count) + 5 , sizeof(file_info_t));;
    if (*files == NULL) {
        LOG_Error("calloc 内存分配失败");
        closedir(dir);
        return -1;
    }
   
    int index = 0;
    // 第二次遍历：收集文件信息
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        memset(full_path,0,sizeof(full_path));
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        if (stat(full_path, &statbuf) == -1) {
            continue;
        }
        
        if (S_ISREG(statbuf.st_mode)) {
            //strncpy((*files)[index].path, full_path, sizeof((*files)[index].path) - 1);
            (*files)[index].path = strdup(full_path);
            if (((*files)[index].path) == NULL) {
                LOG_Error("strdup 内存分配失败");
                FreeFileList((*files), index);
                closedir(dir);
                return -1;
            }

            //(*files)[index].path[sizeof((*files)[index].path) - 1] = '\0';
            (*files)[index].mtime = statbuf.st_mtime;
            (*files)[index].size = statbuf.st_size;
            (*total_size) += statbuf.st_size;
            index++;
        }
    }
    LOG_Debug("index:%lu",index);
    *file_count = index;
    closedir(dir);
    return 0;
}


static unsigned int RemoveFile(file_info_t *files, off_t file_count, off_t *current_size,off_t target_size) 
{
    unsigned int deleted_count = 0;
    qsort(files, file_count, sizeof(file_info_t), compare_files_by_time);
    for (off_t i = 0; (i < file_count) && ((*current_size) > target_size); i++) 
    {
        LOG_Debug("删除文件释放空间: %s (大小: %.2f MB, 修改时间: %s)", files[i].path, (double)files[i].size / (1024 * 1024), ctime(&files[i].mtime));
        if (unlink(files[i].path) == 0) {
            *current_size -= files[i].size;
            deleted_count++;
            free(files[i].path);
            files[i].path = NULL;
        } else {
            LOG_Debug("删除文件失败 %s: %s", files[i].path, strerror(errno));
        }
    }
    
    if (deleted_count > 0) {
        LOG_Debug("已删除 %d 个文件，当前目录大小: %.2f GB\n", deleted_count, (double)*current_size / (1024 * 1024 * 1024));
    }
    
    return deleted_count;
}


static void PrintDirectoryInfo(const char *dir_path, off_t size,off_t max) {
    double size_gb = (double)size / (1024 * 1024 * 1024);
    double size_mb = (double)size / (1024 * 1024);
    double max_gb = (double)max / (1024 * 1024 * 1024);
    
    LOG_Debug("目录: %s", dir_path);
    LOG_Debug("大小: %.2f GB (%.2f MB)", size_gb, size_mb);
    LOG_Debug("限制: %.2f GB", max_gb);
    
    if (size > max) {
        LOG_Debug("状态: 超出限制 %.2f GB", (size_gb - max_gb));
    } else {
        LOG_Debug("状态: 正常 (剩余 %.2f GB)", max_gb - size_gb);
    }
    LOG_Debug("----------------------------------------\n");
}

static int FileProcesFunction(off_t target_size)
{
    int ret = 0;
    unsigned int deleted_count = 0;
    off_t total_size = 0;
    file_info_t *files = NULL;
    off_t file_count = 0;
    off_t current_size = 0;
    ret = GetDirectoryInfo(DIRECTORY, &total_size, &files, &file_count);
    if(ret < 0)
    {
        if(files)
        {
            free(files);
            files = NULL;
        }
        LOG_Error("GetDirectoryInfo Fail");
        return -1;
    }

    LOG_Debug("file_count:%lu",file_count);
    PrintDirectoryInfo(DIRECTORY, total_size,target_size);
    deleted_count = RemoveFile(files, file_count, &total_size,target_size);
    LOG_Debug("Remove File Count:%lu",deleted_count);
    if (files) 
    {
        FreeFileList(files, file_count);
    }

    LOG_Debug("Remove File Count:%lu end",deleted_count);
    return 0;
}

static int GetEmmcSizeInterface()
{
    int ret  = 0;
    for(int i = 0;i<20;i++)
    {
        ret = GetEmmcSize();
        LOG_Debug("EMMC Size:%d",ret);
        if(ret > 20)
        {
            ret = 0;
            break;
        }else if((ret > 0) && (ret < 20))
        {
            ret = 1;
            break;
        }
        sleep(2);
    }

    return ret;
}



void * CanFileHander(void *arg)
{
    int ret  = 0;
    //int flag = -1;
    
    //flag = GetEmmcSizeInterface();
    while (1)
    {

        // if(flag == 0)
        // {
        //     ret = FileProcesFunction(EMMC_32G_MAX_SIZE_BYTES);
        //     if(ret < 0 )
        //     {
        //         LOG_Error("FileProcesFunction error");
        //     }
        // }else if(flag == 1)
        // {
        //     ret = FileProcesFunction(EMMC_8G_MAX_SIZE_BYTES);
        //     if(ret < 0 )
        //     {
        //         LOG_Error("FileProcesFunction error");
        //     }
        // }else
        // {
        //     LOG_Error("flag:%d error",flag);
        //     flag = GetEmmcSizeInterface();
        // }
        ret = FileProcesFunction(EMMC_32G_MAX_SIZE_BYTES);
        if(ret < 0 )
        {
            LOG_Error("FileProcesFunction error");
        }
        sleep(60);
    }
    
}

void * TarFileHander(void *arg)
{
    int ret  = 0;
    char* latest_file = NULL;
    LOG_Debug("TarFileHander start...");
    while (1)
    {
        if(WaitForCompletionTimeout(GetComp(), -1))
        {
            //压缩文件
            latest_file =  FindLatestCanFile(DIRECTORY);
            if(latest_file)
            {
                ret = TarGzCompress(latest_file);
                if(ret < 0 )
                {
                    LOG_Error("Tar Gz Compress Error");
                }
                free(latest_file);
                latest_file = NULL;
            }else{

                LOG_Error("Find Latest Can File Error");
            }

        }
    }
    
}
