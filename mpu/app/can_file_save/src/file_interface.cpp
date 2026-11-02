#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include "crcTool.h"
#include <unistd.h>
#include "sr_log.h"
#include "file_interface.h"
#include "timer_interface.h"
#include "system_interface.h"
#include "completion_timeout.h"
#include "sr_ipc.h"
#include "time_status_api.h"
#include "time_status.h"
#include <archive.h>
#include <archive_entry.h>
#include <cinttypes>
#define TEMP_DIR "/tmp/data"
uint64_t ParseTimestampFromFileName(const char *fileName);
typedef struct
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int ms;
} FileTime_t;

char *FindLatestCanFile(const char *directory)
// 查找最新的 can_YYYY_MM_DD_HH_MM_SS.bin 文件
{
    struct dirent *entry;
    struct statx file_stat;

    char full_path[512];
    char *latest_file = NULL;  // 存储最新文件路径
    long long latest_time = 0; // 存储最新文件的修改时间

    // 打开指定目录
    DIR *dir = opendir(directory);
    if (dir == NULL)
    {
        LOG_Error("无法打开目录");
        return NULL;
    }

    // 遍历目录下的文件
    while ((entry = readdir(dir)) != NULL)
    {
        // 检查文件是否以 "can_" 开头且以 ".bin" 结尾
        if (strncmp(entry->d_name, FILE_PREFIX, strlen(FILE_PREFIX)) == 0 &&
            strstr(entry->d_name, FILE_SUFFIX) != NULL)
        {

            // 构建文件完整路径
            snprintf(full_path, sizeof(full_path), "%s%s", directory, entry->d_name);

            // 获取文件状态  file_statx.stx_btime.tv_sec
            if (statx(AT_FDCWD, full_path, AT_STATX_SYNC_AS_STAT, STATX_ALL, &file_stat) == 0)
            {
                // 如果当前文件的修改时间大于当前最新时间，更新
                // long long mtime_nsec = (long long)file_stat.stx_mtime.tv_sec * 1000000000 + file_stat.stx_mtime.tv_nsec;
                long long mtime_nsec = 0;
                // 检查是否支持创建时间
                if ((file_stat.stx_mask & STATX_BTIME))
                {
                    mtime_nsec = (long long)file_stat.stx_btime.tv_sec * 1000000000 + file_stat.stx_btime.tv_nsec;
                    //                    LOG_Debug("The creation time is supported by the file system ");
                }
                else
                {

                    LOG_Error("The creation time is not supported by the file system");
                }

                //                LOG_Debug("mtime_nsec: %lld    latest_time: %lld",mtime_nsec,latest_time);
                if (mtime_nsec > latest_time)
                {
                    latest_time = mtime_nsec;
                    free(latest_file);                   // 释放之前的内存
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
    // if (strstr(filename, FILE_EXTENSION))
    if (strlen(filename) >= 31 && strstr(filename, FILE_EXTENSION))
    {
        return 1;
    }
    LOG_Debug("filename this file: %s", filename);
    return 0;
}

int TarGzCompress(const char *filename)
{
    DIR *dir;
    struct dirent *entry;
    char cmd[256] = {0};
    char syscmd[512] = {0};
    char filepath[512] = {0};
    int ret = 0;

    LOG_Debug("TarGzCompress START: filename=%s", filename ? filename : "NULL (process all)");

    dir = opendir(DIRECTORY);
    if (!dir)
    {
        LOG_Error("opendir FAIL");
        return -1;
    }

    /* 获取应用上下文，用于检查文件是否正在被写入 */
    AppContext_t *ctx = GetAppContext();

    while ((entry = readdir(dir)) != NULL)
    {
        memset(filepath, 0, sizeof(filepath));
        if (entry->d_type == DT_DIR || !HasValidFormat(entry->d_name))
        {
            continue;
        }

        if (filename)
        {
            if ((!strcmp(filename, entry->d_name)))
            {
                continue;
            }
        }
        memset(cmd, 0, sizeof(cmd));
        snprintf(filepath, sizeof(filepath), "%s", entry->d_name);
        memcpy(cmd, filepath, strlen(filepath) - 4);
        LOG_Debug("TarGzCompress %s", filepath);
        if (filename)
        {
            snprintf(syscmd, sizeof(syscmd), " cd  %s ; tar -zcvf %s.tgz %s & echo $! > /sys/fs/cgroup/cpu,cpuacct/limit_cpu/tasks; wait $!", DIRECTORY, cmd, filepath);
        }
        else
        {
            snprintf(syscmd, sizeof(syscmd), " cd  %s ; tar -zcvf %s.tgz %s ", DIRECTORY, cmd, filepath);
        }

        ret = GetSystemRetValue(syscmd);
        if (ret != 0)
        {
            LOG_Error("%s fail ret = %d", syscmd, ret);
        }
        else
        {
            LOG_Debug("TarGzCompress %s exec ret = %d", filepath, ret);
            /* 提取文件名（不含路径）用于比较 */
            const char *base_name = entry->d_name;
        {
            char tgzPath[512];
            struct stat tgzSt;
            snprintf(tgzPath, sizeof(tgzPath), "%s%s.tgz", DIRECTORY, cmd);  /* cmd=去掉.bin的名字 */
            if (stat(tgzPath, &tgzSt) == 0 && tgzSt.st_size > 0)
            {
                memset(syscmd, 0, sizeof(syscmd));
                sprintf(syscmd, " cd  %s ;rm -rf  %s;", DIRECTORY, entry->d_name);
                ret = GetSystemRetValue(syscmd);
                if (ret != 0) 
                { 
                  LOG_Error("%s fail ", filepath);
                }
            }
            else
            {
                LOG_Error("TarGzCompress: tgz missing/empty, keep source bin: %s", filepath);
            }
        }
        }
    }
    closedir(dir);
    return 0;
}

// 检查文件大小并决定是否创建新文件
FILE *CheckRotateFile(FILE *current_file, const char *current_path, uint64_t timestamp)
{
    if (!current_file)
        return NULL;

    // 获取当前文件大小
    struct stat st;
    if (fstat(fileno(current_file), &st) != 0)
    {
        LOG_Error("fstat failed");
        return current_file;
    }

    // 如果未超过限制，继续使用当前文件
    if (st.st_size < MAX_FILE_SIZE)
    {
        
        LOG_Debug("CheckRotateFile st_size %u", st.st_size);
        return current_file;
    }
    // 关闭当前文件
    LOG_Debug("CheckRotateFile close file, st_size %u", st.st_size);
    fclose(current_file);
    current_file = NULL;

    // 生成新文件名
    char new_path[256];
    GenerateNewFilename(timestamp, new_path, sizeof(new_path));

    // 创建新文件
    FILE *new_file = fopen(new_path, "wb");
    if (!new_file)
    {
        LOG_Error("Failed to create new file");
        return NULL;
    }

    Complete(GetComp());
    LOG_Debug("已创建新文件: %s", new_path);
    return new_file;
}

// 主写入函数
int WriteDataWithRotation(FileWriterCtx_t *ctx, Can_frame_t *frames, int count)
{
    AppContext_t *app_ctx = GetAppContext();
    const char *ctx_type = (ctx == &app_ctx->main_file_ctx) ? "MAIN" : "BACKUP";

    int ret = 0;
    if (!ctx || !frames || count <= 0)
        return -1;

    LOG_Debug("WriteDataWithRotation [%s]: count=%d, timestamp=%lu us", ctx_type, count, frames[0].timestamp);


    // 初始化文件
    if (!ctx->current_file)
    {
        // 根据时间生成文件名（用第一帧）
        GenerateNewFilename(frames[0].timestamp, ctx->current_path, sizeof(ctx->current_path));
        ctx->current_file = fopen(ctx->current_path, "ab"); // 二进制追加
        memset(ctx->bkfilename,0,sizeof(ctx->bkfilename));
        memcpy(ctx->bkfilename,ctx->current_path,sizeof(ctx->bkfilename));
        if (!ctx->current_file)
        {
            LOG_Error("[%s] Initial file creation =%s failed", ctx_type, ctx->current_path);
            return -1;
        }
        LOG_Debug("[%s] Created new file: %s", ctx_type, ctx->current_path);

    }

    // 文件轮转
    ctx->current_file = CheckRotateFile(ctx->current_file, ctx->current_path, frames[0].timestamp);
    if (!ctx->current_file)
    {
        LOG_Error("CheckRotateFile failed");
        return -1;
    }

   
    size_t total = count;
    size_t written = fwrite(frames, sizeof(Can_frame_t), total, ctx->current_file);
    LOG_Debug("written: %d", count);
    if (written != total)
    {
        LOG_Error("fwrite failed: expect=%zu actual=%zu", total, written);

        if (ferror(ctx->current_file))
        {
            LOG_Error("fwrite failed:");
            return -1;
        }
    }
    ret = fflush(ctx->current_file);
    if (ret < 0)
    {
        LOG_Error("fflush fail ret=%d", ret);
    }

    return 0;
}

static void FreeFileList(file_info_t *files, off_t count)
{
    for (off_t i = 0; i < count; i++)
    {
        if (files[i].path)
        {
            free(files[i].path);
            (files[i].path) = NULL;
        }
    }

    if (files)
    {
        free(files);
        files = NULL;
    }
}

static int compare_files_by_time(const void *a, const void *b)
{
    const file_info_t *fa = (const file_info_t *)a;
    const file_info_t *fb = (const file_info_t *)b;

    if (fa->mtime < fb->mtime)
        return -1;
    if (fa->mtime > fb->mtime)
        return 1;
    return 0;
}

int GetDirectoryInfo(const char *dir_path, off_t *total_size, file_info_t **files, off_t *file_count)
{
    DIR *dir = NULL;
    struct dirent *entry = {0};
    struct stat statbuf = {0};
    char full_path[1024] = {0};
    *total_size = 0;
    *file_count = 0;
    dir = opendir(dir_path);
    if (dir == NULL)
    {
        LOG_Error("无法打开目录 %s: %s\n", dir_path, strerror(errno));
        return -1;
    }

    // 第一次遍历：统计文件数量
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (stat(full_path, &statbuf) == -1)
        {
            LOG_Error("无法获取文件信息 %s: %s\n", full_path, strerror(errno));
            continue;
        }

        if (S_ISREG(statbuf.st_mode))
        {
            (*file_count)++;
        }
    }

    rewinddir(dir);
    LOG_Debug("file_count:%lu", (*file_count));
    // 分配内存
    *files = (file_info_t *)calloc((*file_count) + 5, sizeof(file_info_t));
    ;
    if (*files == NULL)
    {
        LOG_Error("calloc 内存分配失败");
        closedir(dir);
        return -1;
    }

    int index = 0;
    // 第二次遍历：收集文件信息
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        memset(full_path, 0, sizeof(full_path));
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (stat(full_path, &statbuf) == -1)
        {
            continue;
        }

        if (S_ISREG(statbuf.st_mode))
        {
            // strncpy((*files)[index].path, full_path, sizeof((*files)[index].path) - 1);
            (*files)[index].path = strdup(full_path);
            if (((*files)[index].path) == NULL)
            {
                LOG_Error("strdup 内存分配失败");
                FreeFileList((*files), index);
                closedir(dir);
                return -1;
            }
           
            //(*files)[index].path[sizeof((*files)[index].path) - 1] = '\0';
            {
                time_t parsed_mtime = (time_t)(ParseTimestampFromFileName(full_path) / 1000000);
                if (parsed_mtime <= 0)
                {
                    (*files)[index].mtime = statbuf.st_mtime;
                }
                else
                {
                    (*files)[index].mtime = parsed_mtime;
                }
            }
            (*files)[index].size = statbuf.st_size;
            (*total_size) += statbuf.st_size;
            index++;
        }
    }
    LOG_Debug("index:%d", index);
    *file_count = index;
    closedir(dir);
    return 0;
}

static unsigned int RemoveFile(file_info_t *files, off_t file_count, off_t *current_size, off_t target_size)
{
    unsigned int deleted_count = 0;
    qsort(files, file_count, sizeof(file_info_t), compare_files_by_time);
    for (off_t i = 0; (i < file_count) && ((*current_size) > target_size); i++)
    {
        LOG_Debug("删除文件释放空间: %s (大小: %.2f MB, 修改时间: %s)", files[i].path, (double)files[i].size / (1024 * 1024), ctime(&files[i].mtime));
        if (unlink(files[i].path) == 0)
        {
            *current_size -= files[i].size;
            deleted_count++;
            free(files[i].path);
            files[i].path = NULL;
        }
        else
        {
            LOG_Debug("删除文件失败 %s: %s", files[i].path, strerror(errno));
        }
    }

    if (deleted_count > 0)
    {
        LOG_Debug("已删除 %d 个文件，当前目录大小: %.2f GB\n", deleted_count, (double)*current_size / (1024 * 1024 * 1024));
    }

    return deleted_count;
}

static void PrintDirectoryInfo(const char *dir_path, off_t size, off_t max)
{
    double size_gb = (double)size / (1024 * 1024 * 1024);
    double size_mb = (double)size / (1024 * 1024);
    double max_gb = (double)max / (1024 * 1024 * 1024);

    LOG_Debug("目录: %s", dir_path);
    LOG_Debug("大小: %.2f GB (%.2f MB)", size_gb, size_mb);
    LOG_Debug("限制: %.2f GB", max_gb);

    if (size > max)
    {
        LOG_Debug("状态: 超出限制 %.2f GB", (size_gb - max_gb));
    }
    else
    {
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
    ret = GetDirectoryInfo(DIRECTORY, &total_size, &files, &file_count);
    if (ret < 0)
    {
        if (files)
        {
            FreeFileList(files, file_count);
        }
        LOG_Error("GetDirectoryInfo Fail");
        return -1;
    }

    LOG_Debug("file_count:%lu", file_count);
    PrintDirectoryInfo(DIRECTORY, total_size, target_size);
    deleted_count = RemoveFile(files, file_count, &total_size, target_size);
    LOG_Debug("Remove File Count:%u", deleted_count);
    if (files)
    {
        FreeFileList(files, file_count);
    }

    LOG_Debug("Remove File Count:%u end", deleted_count);
    return 0;
}

void *CanFileHandler(void *arg)
{
    int ret = 0;
    while (1)
    {
        ret = FileProcesFunction(EMMC_32G_MAX_SIZE_BYTES);
        if (ret < 0)
        {
            LOG_Error("FileProcesFunction error");
        }
        sleep(60);
    }
}

void *TarFileHander(void *arg)
{
    int ret = 0;
    char *latest_file = NULL;
    LOG_Debug("TarFileHander start...");
    while (1)
    {
        if (WaitForCompletionTimeout(GetComp(), -1))
        {
            // 压缩文件
            latest_file = FindLatestCanFile(DIRECTORY);
            if (latest_file)
            {
                ret = TarGzCompress(latest_file);
                if (ret < 0)
                {
                    LOG_Error("Tar Gz Compress Error");
                }
                free(latest_file);
                latest_file = NULL;
            }
            else
            {
                LOG_Error("Find Latest Can File Error");
            }
        }
    }
}

/* 解析文件名时间 */
int ParseFileTime(const char *filename, FileTime_t *ft)
{
    if (sscanf(filename,
               "can_%d_%d_%d_%d_%d_%d_%d",
               &ft->year,
               &ft->month,
               &ft->day,
               &ft->hour,
               &ft->minute,
               &ft->second,
               &ft->ms) == 7)
    {
        return 0;
    }
    return -1;
}

/* 比较时间 */
int CompareFileTime(const FileTime_t *a, const FileTime_t *b)
{
    if (a->year != b->year)
        return a->year - b->year;
    if (a->month != b->month)
        return a->month - b->month;
    if (a->day != b->day)
        return a->day - b->day;
    if (a->hour != b->hour)
        return a->hour - b->hour;
    if (a->minute != b->minute)
        return a->minute - b->minute;
    if (a->second != b->second)
        return a->second - b->second;
    return a->ms - b->ms;
}

int GetLatestFile(const char *dirPath, char *latestFile, size_t len)
{
    if (!dirPath || !latestFile)
        return -1;

    DIR *dir = opendir(dirPath);
    if (!dir)
        return -1;

    struct dirent *entry;
    FileTime_t latestTime = {0};
    int found = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strncmp(entry->d_name, "can_", 4) != 0)
            continue;

        FileTime_t curTime;
        if (ParseFileTime(entry->d_name, &curTime) != 0)
            continue;

        if (!found || CompareFileTime(&curTime, &latestTime) > 0)
        {
            latestTime = curTime;

            snprintf(latestFile, len, "%s%s", dirPath, entry->d_name);

            found = 1;
        }
    }

    closedir(dir);

    return found ? 0 : -1;
}

static int extract_tgz_libarchive(const char *tgz_path, const char *dest_dir)
{
    struct archive *a = NULL;
    struct archive *ext = NULL;
    struct archive_entry *entry = NULL;
    struct stat st;
    char old_cwd[PATH_MAX];

    int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM |
                ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;
    int ret = -1, r;

    memset(&st, 0, sizeof(st));
    memset(old_cwd, 0, sizeof(old_cwd));

    a = archive_read_new();
    if (!a)
        return -1;

    archive_read_support_filter_gzip(a);
    archive_read_support_format_tar(a);

    ext = archive_write_disk_new();
    if (!ext)
        goto cleanup;

    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);

    r = archive_read_open_filename(a, tgz_path, 10240);
    if (r != ARCHIVE_OK)
    {
        LOG_Error("Cannot open archive '%s': %s", tgz_path, archive_error_string(a));
        goto cleanup;
    }

    if (stat(dest_dir, &st) == -1 && mkdir(dest_dir, 0755) != 0)
    {
        LOG_Error("Cannot create dest dir '%s': %s", dest_dir, strerror(errno));
        goto cleanup;
    }

    if (getcwd(old_cwd, sizeof(old_cwd)) == NULL || chdir(dest_dir) != 0)
    {
        LOG_Error("Cannot chdir to '%s'", dest_dir);
        goto cleanup;
    }

    for (;;)
    {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;

        if (r != ARCHIVE_OK)
        {
            LOG_Error("archive_read_next_header: %s", archive_error_string(a));
            goto restore_cwd;
        }

        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK)
        {
            LOG_Error("archive_write_header: %s", archive_error_string(ext));
            goto restore_cwd;
        }

        if (archive_entry_size(entry) > 0)
        {
            const void *buff;
            size_t size;
            la_int64_t offset;

            for (;;)
            {
                r = archive_read_data_block(a, &buff, &size, &offset);
                if (r == ARCHIVE_EOF)
                    break;

                if (r != ARCHIVE_OK)
                {
                    LOG_Error("archive_read_data_block: %s", archive_error_string(a));
                    goto restore_cwd;
                }

                r = archive_write_data_block(ext, buff, size, offset);
                if (r != ARCHIVE_OK)
                {
                    LOG_Error("archive_write_data_block: %s", archive_error_string(ext));
                    goto restore_cwd;
                }
            }
        }

        archive_write_finish_entry(ext);
    }

    ret = 0;

restore_cwd:
    chdir(old_cwd);

cleanup:
    if (a)
        archive_read_free(a);

    if (ext)
        archive_write_free(ext);

    return ret;
}

int GetLastFrameTimestamp(const char *filename, uint64_t *timestamp)
{
    char binFile[256] = {0};
    int needDelete = 0;
    int ret = -1;
    FILE *fp = NULL;
    long file_size = 0;
    size_t frame_size = sizeof(Can_frame_t);
    long remainder = 0;
    long offset = 0;
    Can_frame_t frame;

    if (!filename || !timestamp)
        return -1;

    *timestamp = 0;
    LOG_Debug("filename =%s ", filename);
    /* 取文件名 */
    const char *baseName = strrchr(filename, '/');
    baseName = (baseName) ? (baseName + 1) : filename;
    LOG_Debug("baseName =%s ", baseName);
    /* tgz 文件 */
    if (strstr(baseName, ".tgz"))
    {
        if (extract_tgz_libarchive(filename, TEMP_DIR) != 0)
        {
            LOG_Error("extract fail: %s", filename);
            goto EXIT;
        }

        snprintf(binFile, sizeof(binFile), "%s/%s", TEMP_DIR, baseName);

        char *p = strstr(binFile, ".tgz");
        if (p)
            strcpy(p, ".bin");

        needDelete = 1;
    }
    /* bin 文件 */
    else if (strstr(baseName, ".bin"))
    {
        snprintf(binFile, sizeof(binFile), "%s", filename);
    }
    else
    {
        LOG_Error("unsupported file: %s", filename);
        goto EXIT;
    }

    LOG_Debug("open file: %s", binFile);

    fp = fopen(binFile, "rb");
    if (!fp)
    {
        LOG_Error("open %s fail", binFile);
        goto EXIT;
    }

    /* 获取文件大小，判断最后一帧是否完整 */
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    remainder = file_size % frame_size;

    LOG_Debug("file_size=%ld sizeof(Can_frame_t)=%zu remainder=%ld",
              file_size, frame_size, remainder);

    /* 有余数说明末尾有不完整帧，跳过残帧读倒数第二帧 */
    offset = -(long)frame_size;
    if (remainder != 0)
    {
        offset -= remainder;
        LOG_Debug("incomplete tail detected, skip %ld bytes", remainder);
    }

    if (fseek(fp, offset, SEEK_END) != 0)
    {
        LOG_Error("fseek fail");
        goto EXIT;
    }

    memset(&frame, 0, sizeof(frame));

    if (fread(&frame, sizeof(frame), 1, fp) != 1)
    {
        LOG_Error("fread error");
        goto EXIT;
    }

    *timestamp = frame.timestamp;
    LOG_Debug("timestamp = %lu", *timestamp);
    ret = 0;

EXIT:
    if (fp)
        fclose(fp);

    if (needDelete)
    {
        if (remove(binFile) == 0)
            LOG_Debug("delete temp file success: %s", binFile);
        else
            LOG_Error("delete temp file fail: %s", binFile);
    }
    return ret;
}

uint64_t ParseTimestampFromFileName(const char *fileName)
{
    if (fileName == NULL)
    {
        return 0;
    }

    int year = 0, mon = 0, day = 0;
    int hour = 0, min = 0, sec = 0, msec = 0;

    /* 找到文件名 */
    const char *base = strrchr(fileName, '/');
    base = (base == NULL) ? fileName : (base + 1);

    /*
     * 格式：
     * can_2026_04_29_15_05_36_478.bin
     */
    int ret = sscanf(base,
                     "can_%d_%d_%d_%d_%d_%d_%d",
                     &year, &mon, &day,
                     &hour, &min, &sec, &msec);

    if (ret != 7)
    {
        return 0;
    }

    struct tm t;
    memset(&t, 0, sizeof(t));

    t.tm_year = year - 1900;
    t.tm_mon = mon - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = min;
    t.tm_sec = sec;

    time_t ts = mktime(&t);
    if (ts < 0)
    {
        return 0;
    }

    // 返回微秒，与其他时间戳单位保持一致
    return ((uint64_t)ts * 1000000ULL + (uint64_t)msec * 1000ULL);
}

int MainModemInit(uint64_t *timestamp)
{
    char filename[256] = {0};
    int ret = 0;
    ret = GetLatestFile("/mnt/canlog/", filename, sizeof(filename));
    if (ret == 0)
    {
        ret = GetLastFrameTimestamp(filename, timestamp);
        if (0 == *timestamp)
        {
            *timestamp = ParseTimestampFromFileName(filename);
        }
    }
    LOG_Debug("return ret =%d,timestamp = %lu", ret, *timestamp);
    return ret;
}

/* 压缩单个指定文件 */
int TarGzCompressSingleFile(const char *filepath)
{
    char cmd[256] = {0};
    char syscmd[512] = {0};
    int ret = 0;
    
    if (!filepath || strlen(filepath) == 0)
    {
        LOG_Error("TarGzCompressSingleFile: invalid filepath");
        return -1;
    }
    
    /* 检查文件是否存在 */
    struct stat st;
    if (stat(filepath, &st) != 0)
    {
        LOG_Error("TarGzCompressSingleFile: file not exist: %s", filepath);
        return -1;
    }
    
    /* 从完整路径中提取文件名 */
    const char *basename = strrchr(filepath, '/');
    basename = basename ? basename + 1 : filepath;
    
    /* 提取不带扩展名的文件名作为压缩输出名 */
    size_t basename_len = strlen(basename);
    if (basename_len <= 4)  /* 文件名太短，无法去掉 .bin */
    {
        LOG_Error("TarGzCompressSingleFile: invalid filename: %s", basename);
        return -1;
    }
    
    memcpy(cmd, basename, basename_len - 4);  /* 去掉 .bin */
    
    LOG_Debug("TarGzCompressSingleFile: compressing %s to %s.tgz", basename, cmd);
    
    /* 执行压缩命令 */
    snprintf(syscmd, sizeof(syscmd),
             "cd %s ; tar -zcvf %s.tgz %s & echo $! > /sys/fs/cgroup/cpu,cpuacct/limit_cpu/tasks; wait $!",
             DIRECTORY, cmd, basename);
    
    LOG_Debug("tar  %s", syscmd);         
    ret = GetSystemRetValue(syscmd);
    if (ret != 0)
    {
        LOG_Error("TarGzCompressSingleFile: compress failed: %s", syscmd);
        return -1;
    }
    
    LOG_Debug("TarGzCompressSingleFile: compress success: %s.tgz", cmd);
    
    /* 压缩成功后删除原文件 */
    memset(syscmd, 0, sizeof(syscmd));
    snprintf(syscmd, sizeof(syscmd), "cd %s ; rm -f %s", DIRECTORY, basename);
    
    ret = GetSystemRetValue(syscmd);
    if (ret != 0)
    {
        LOG_Error("TarGzCompressSingleFile: remove original file failed: %s", basename);
        return -1;
    }
    
    LOG_Debug("TarGzCompressSingleFile: original file removed: %s", basename);
    return 0;
}
