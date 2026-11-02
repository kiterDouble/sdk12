#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sendfile.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <archive.h>
#include <archive_entry.h>
#include "crcTool.h"
#include "sr_log.h"


/* 文件信息结构体 */
typedef struct
{
    char filePath[512];
    time_t createTime;
} FileInfo_t;

/* 文件列表结构体 */
typedef struct
{
    FileInfo_t *files;
    int count;
    int capacity;
} FileList_t;

/* 批量输出缓冲区管理 */
typedef struct
{
    FILE *fp;
    char *buf;
    size_t capacity;
    size_t pos;
} BatchWriter;

#pragma pack(push, 1)
typedef struct
{
    uint64_t timestamp; // 时间戳毫秒
    uint8_t channel;    // CAN channel
    uint32_t can_id;
    uint8_t direction; // 'R' for RX, 'T' for TX
    uint8_t flags;     // bit0=FD, bit1=TX/RX, bit2=BRS, bit3=ESI
    uint8_t dlc;
    uint8_t data[8];
} Can_frame_t;
#pragma pack(pop)

/* ---------- 文件列表操作 ---------- */
static int InitFileList(FileList_t *list, int capacity)
{
    if (!list)
    {
        LOG_Error("InitFileList: list is NULL\n");
        return -1;
    }
    list->files = malloc(sizeof(FileInfo_t) * capacity);
    if (!list->files)
    {
        LOG_Error("InitFileList: malloc failed for files array\n");
        return -1;
    }
    list->count = 0;
    list->capacity = capacity;
    return 0;
}

static int AddFileToList(FileList_t *list, const char *filePath, time_t createTime)
{
    if (!list || !filePath)
    {
        LOG_Error("AddFileToList: invalid params, list=%p, filePath=%p\n", list, filePath);
        return -1;
    }
    if (list->count >= list->capacity)
    {
        int newCapacity = list->capacity * 2;
        FileInfo_t *newFiles = realloc(list->files, sizeof(FileInfo_t) * newCapacity);
        if (!newFiles)
        {
            LOG_Error("AddFileToList: realloc failed for files array\n");
            return -1;
        }
        list->files = newFiles;
        list->capacity = newCapacity;
    }
    strncpy(list->files[list->count].filePath, filePath, sizeof(list->files[list->count].filePath) - 1);
    list->files[list->count].filePath[sizeof(list->files[list->count].filePath) - 1] = '\0';
    list->files[list->count].createTime = createTime;
    list->count++;
    return 0;
}

static void FreeFileList(FileList_t *list)
{
    if (list && list->files)
    {
        free(list->files);
        list->files = NULL;
    }
    if (list)
    {
        list->count = 0;
        list->capacity = 0;
    }
}

static void ReleaseFileList(FileList_t **fileList)
{
    if (fileList && *fileList)
    {
        FreeFileList(*fileList);
        free(*fileList);
        *fileList = NULL;
    }
}

/* ---------- 时间戳工具 ---------- */
static time_t ExtractCreateTimeFromFileName(const char *fileName, const char *fileExt)
{
    if (!fileName || !fileExt)
    {
        LOG_Error("ExtractCreateTimeFromFileName: invalid params, fileName=%p, fileExt=%p\n", fileName, fileExt);
        return 0;
    }
    char basename[256] = {0}, timestr[256] = {0};
    struct tm tm = {0};

    const char *ptr = strrchr(fileName, '/');
    if (ptr)
        strncpy(basename, ptr + 1, sizeof(basename) - 1);
    else
        strncpy(basename, fileName, sizeof(basename) - 1);

    if (strncmp(basename, "can_", 4) == 0)
        strncpy(timestr, basename + 4, sizeof(timestr) - 1);
    else
        strncpy(timestr, basename, sizeof(timestr) - 1);
    timestr[sizeof(timestr) - 1] = '\0';

    size_t extLen = strlen(fileExt);
    size_t timestrLen = strlen(timestr);
    if (timestrLen > extLen && strcmp(timestr + timestrLen - extLen, fileExt) == 0)
        timestr[timestrLen - extLen] = '\0';

    int year, month, day, hour, minute, second;
    if (sscanf(timestr, "%d_%d_%d_%d_%d_%d", &year, &month, &day, &hour, &minute, &second) == 6)
    {
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;
        tm.tm_isdst = -1;
        time_t t = mktime(&tm);
        return (t == (time_t)-1) ? 0 : t;
    }
    return 0;
}

static int compare_files(const void *a, const void *b)
{
    const FileInfo_t *fa = (const FileInfo_t *)a;
    const FileInfo_t *fb = (const FileInfo_t *)b;
    return (fa->createTime > fb->createTime) - (fa->createTime < fb->createTime);
}

/* ---------- 文件查找 ---------- */
static FileList_t *FindFilesByTime(uint64_t startTime, uint64_t endTime, const char *searchDir, const char *fileExt)
{
    if (!searchDir || !fileExt)
    {
        LOG_Error("FindFilesByTime: invalid params, searchDir=%p, fileExt=%p\n", searchDir, fileExt);
        return NULL;
    }

    DIR *dir = opendir(searchDir);
    if (!dir)
    {
        LOG_Error("Cannot open directory: %s, errno=%d\n", searchDir, errno);
        return NULL;
    }

    FileList_t *list = malloc(sizeof(FileList_t));
    if (!list)
    {
        LOG_Error("FindFilesByTime: malloc failed for FileList_t\n");
        closedir(dir);
        return NULL;
    }
    if (InitFileList(list, 100) != 0)
    {
        LOG_Error("FindFilesByTime: InitFileList failed\n");
        closedir(dir);
        free(list);
        return NULL;
    }

    struct dirent *entry;
    struct stat statBuf;
    char fullPath[512];
    time_t st_start = (time_t)startTime;
    time_t st_end = (time_t)endTime;
    size_t extLen = strlen(fileExt);

    while ((entry = readdir(dir)) != NULL)
    {
        /* d_type 在某些文件系统(如 NFS) 上可能返回 DT_UNKNOWN, 需 fallback 到 stat */
        if (entry->d_type != DT_REG && entry->d_type != DT_UNKNOWN)
            continue;

        size_t nameLen = strlen(entry->d_name);
        if (nameLen <= extLen || strcmp(entry->d_name + nameLen - extLen, fileExt) != 0)
            continue;

        snprintf(fullPath, sizeof(fullPath), "%s/%s", searchDir, entry->d_name);
        if (stat(fullPath, &statBuf) != 0)
            continue;

        /* stat 已获取, 用 S_ISREG 精确判断是否为普通文件 */
        if (!S_ISREG(statBuf.st_mode))
            continue;

        time_t mtime = statBuf.st_mtime;
        time_t createTime = ExtractCreateTimeFromFileName(entry->d_name, fileExt);

        if (createTime > st_end && mtime > st_end)
            continue;
        if (createTime <= st_end && mtime >= st_start)
        {
            AddFileToList(list, fullPath, createTime);
        }
    }

    closedir(dir);
    if (list->count > 1)
        qsort(list->files, list->count, sizeof(FileInfo_t), compare_files);
    return list;
}

/* ---------- 文件复制 ---------- */
static int copy_file_sendfile(const char *src_path, const char *dst_path)
{
    int src_fd = open(src_path, O_RDONLY);
    if (src_fd == -1)
    {
        LOG_Error("copy_file_sendfile: failed to open src file '%s', errno=%d\n", src_path, errno);
        return -1;
    }
    int dst_fd = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd == -1)
    {
        LOG_Error("copy_file_sendfile: failed to open dst file '%s', errno=%d\n", dst_path, errno);
        close(src_fd);
        return -1;
    }
    struct stat st;
    if (fstat(src_fd, &st) == -1)
    {
        LOG_Error("copy_file_sendfile: fstat failed for '%s', errno=%d\n", src_path, errno);
        close(src_fd);
        close(dst_fd);
        return -1;
    }
    off_t offset = 0;
    off_t remaining = st.st_size;
    while (remaining > 0)
    {
        ssize_t sent = sendfile(dst_fd, src_fd, &offset, remaining);
        if (sent <= 0)
        {
            if (sent == -1 && errno == EINTR)
                continue;
            LOG_Error("copy_file_sendfile: sendfile failed, errno=%d\n", errno);
            close(src_fd);
            close(dst_fd);
            unlink(dst_path); /* 删除可能不完整的目标文件 */
            return -1;
        }
        remaining -= sent;
    }
    close(src_fd);
    close(dst_fd);
    return 0;
}

/* ---------- 解压 tgz (libarchive) ---------- */
static int extract_tgz_libarchive(const char *tgz_path, const char *dest_dir)
{
    struct archive *a = NULL;
    struct archive *ext = NULL;
    struct archive_entry *entry;
    int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM |
                ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;
    int ret = -1, r;

    a = archive_read_new();
    if (!a)
    {
        LOG_Error("extract_tgz_libarchive: archive_read_new failed\n");
        return -1;
    }
    archive_read_support_filter_gzip(a);
    archive_read_support_format_tar(a);

    ext = archive_write_disk_new();
    if (!ext)
    {
        LOG_Error("extract_tgz_libarchive: archive_write_disk_new failed\n");
        goto cleanup;
    }
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);

    r = archive_read_open_filename(a, tgz_path, 10240);
    if (r != ARCHIVE_OK)
    {
        LOG_Error("Cannot open archive '%s': %s", tgz_path, archive_error_string(a));
        goto cleanup;
    }

    struct stat st = {0};
    if (stat(dest_dir, &st) == -1 && mkdir(dest_dir, 0755) != 0)
    {
        LOG_Error("Cannot create dest dir '%s': %s", dest_dir, strerror(errno));
        goto cleanup;
    }

    char old_cwd[PATH_MAX];
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
        /* 每解压一个文件后让出 CPU，降低解压期间的 CPU 占用率 */
        usleep(1000);
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

static int ProcessTarFile(FileList_t *fileList, const char *tmpTarDir)
{
    if (!fileList || !tmpTarDir)
    {
        LOG_Error("ProcessTarFile: invalid params, fileList=%p, tmpTarDir=%p\n", fileList, tmpTarDir);
        return -1;
    }
    //LOG_Debug("ProcessTarFile: processing %d files to dir '%s'\n", fileList->count, tmpTarDir);

    if (mkdir(tmpTarDir, 0777) != 0 && errno != EEXIST)
    {
        LOG_Error("ProcessTarFile: cannot create directory '%s', errno=%d\n", tmpTarDir, errno);
        return -1;
    }
   // LOG_Debug("ProcessTarFile: directory '%s' ready\n", tmpTarDir);

    char dstPath[PATH_MAX];
    int successCount = 0;
    int failCount = 0;

    for (int i = 0; i < fileList->count; i++)
    {
        const char *path = fileList->files[i].filePath;
        size_t len = strlen(path);
       // LOG_Debug("ProcessTarFile: processing file %d/%d: '%s'\n", i + 1, fileList->count, path);

        if (len > 4 && strcmp(path + len - 4, ".tgz") == 0)
        {
            int ret = extract_tgz_libarchive(path, tmpTarDir);
            if (ret != 0)
            {
                LOG_Error("ProcessTarFile: failed to extract '%s', ret=%d\n", path, ret);
                failCount++;
            }
            else
            {
                //LOG_Debug("ProcessTarFile: successfully extracted '%s'\n", path);
                successCount++;
            }
        }
        else if (len > 4 && strcmp(path + len - 4, ".bin") == 0)
        {
            const char *fname = strrchr(path, '/');
            fname = fname ? fname + 1 : path;
            snprintf(dstPath, sizeof(dstPath), "%s/%s", tmpTarDir, fname);
            int ret = copy_file_sendfile(path, dstPath);
            if (ret != 0)
            {
                LOG_Error("ProcessTarFile: failed to copy '%s' to '%s', ret=%d\n", path, dstPath, ret);
                failCount++;
            }
            else
            {
                //LOG_Debug("ProcessTarFile: successfully copied '%s' to '%s'\n", path, dstPath);
                successCount++;
            }
        }
        else
        {
            LOG_Debug("ProcessTarFile: skipping file '%s' (unknown extension)\n", path);
        }
        /* 每处理完一个文件后让出 CPU，避免连续 I/O 占用过高 */
        usleep(5000);
    }

    //LOG_Debug("ProcessTarFile: completed, success=%d, fail=%d\n", successCount, failCount);
    return 0;
}

static inline int BatchWriter_Init(BatchWriter *bw, FILE *fp, size_t cap)
{
    if (!bw || !fp)
    {
        LOG_Error("BatchWriter_Init: invalid params, bw=%p, fp=%p\n", bw, fp);
        return -1;
    }
    bw->buf = malloc(cap);
    if (!bw->buf)
    {
        LOG_Error("BatchWriter_Init: malloc failed for buffer, size=%zu\n", cap);
        return -1;
    }
    bw->fp = fp;
    bw->capacity = cap;
    bw->pos = 0;
    return 0;
}

static inline int BatchWriter_Flush(BatchWriter *bw)
{
    if (bw->pos > 0)
    {
        size_t expected = bw->pos;
        size_t written = fwrite(bw->buf, 1, expected, bw->fp);
        bw->pos = 0;
        if (written != expected)
        {
            LOG_Error("BatchWriter_Flush: fwrite incomplete, expected %zu, wrote %zu",
                      expected, written);
            return -1;
        }
    }
    return 0;
}

static inline int BatchWriter_Write(BatchWriter *bw, const void *data, size_t len)
{
    if (len == 0)
        return 0;
    if (bw->pos + len > bw->capacity)
    {
        if (BatchWriter_Flush(bw) != 0)
        {
            LOG_Error("BatchWriter_Write: BatchWriter_Flush failed\n");
            return -1;
        }
        if (len >= bw->capacity)
        {
            size_t written = fwrite(data, 1, len, bw->fp);
            if (written != len)
            {
                LOG_Error("BatchWriter_Write: fwrite incomplete, expected %zu, wrote %zu\n",
                          len, written);
                return -1;
            }
            return 0;
        }
    }
    memcpy(bw->buf + bw->pos, data, len);
    bw->pos += len;
    return 0;
}

static inline int BatchWriter_WriteStr(BatchWriter *bw, const char *s)
{
    return BatchWriter_Write(bw, s, strlen(s));
}

static inline int BatchWriter_Putc(BatchWriter *bw, char c)
{
    if (bw->pos >= bw->capacity)
    {
        if (BatchWriter_Flush(bw) != 0)
        {
            LOG_Error("BatchWriter_Putc: BatchWriter_Flush failed\n");
            return -1;
        }
    }
    if (bw->buf) /* 防御性检查, Init 可能失败 */
        bw->buf[bw->pos++] = c;
    return 0;
}

static inline void BatchWriter_Destroy(BatchWriter *bw)
{
    if (BatchWriter_Flush(bw) != 0)
        LOG_Error("BatchWriter_Destroy: flush failed, data may be lost");
    free(bw->buf);
}

static int MergeAndFilterAscFiles(FileList_t *fileList, const char *outputFile, uint64_t startTime, uint64_t endTime)
{
    if (!fileList || !outputFile)
    {
        LOG_Error("MergeAndFilterAscFiles: invalid params, fileList=%p, outputFile=%p\n", fileList, outputFile);
        return -1;
    }
    if (fileList->count == 0)
    {
        LOG_Debug("MergeAndFilterAscFiles: no files to process\n");
        return 0;
    }

    uint64_t start_ms = startTime * 1000 * 1000ULL;
    uint64_t end_ms = endTime * 1000 * 1000ULL;
    double maxTime = (double)(end_ms - start_ms) / 1000.0 / 1000.0;
    //LOG_Debug("MergeAndFilterAscFiles: start_ms=%lu, end_ms=%lu, maxTime=%.6f\n", start_ms, end_ms, maxTime);

    FILE *outFp = fopen(outputFile, "w");
    if (!outFp)
    {
        LOG_Error("MergeAndFilterAscFiles: failed to open output file '%s', errno=%d\n", outputFile, errno);
        return -1;
    }

    /* ---------- 写 ASC 头 ---------- */
    struct tm tm;
    time_t startTs = (time_t)(startTime);
    localtime_r(&startTs, &tm);

    char time_str[128];
    strftime(time_str, sizeof(time_str), "%a %b %d %I:%M:%S", &tm);

    fprintf(outFp,
            "date %s.000 %s %d\n"
            "base hex timestamps absolute\n"
            "internal events logged\n"
            "// version 18.3.0",
            time_str,
            (tm.tm_hour >= 12) ? "pm" : "am",
            tm.tm_year + 1900);

    BatchWriter bw;
    if (BatchWriter_Init(&bw, outFp, 64 * 1024) != 0)
    {
        LOG_Error("MergeAndFilterAscFiles: BatchWriter_Init failed\n");
        fclose(outFp);
        return -1;
    }

    /* ---------- 遍历所有 bin 文件 ---------- */
    int totalFrames = 0;
    int skippedFrames = 0;

    /* 批量读取缓冲区：每次读取 4096 帧（约 96KB），大幅减少 fread 系统调用次数 */
    #define BULK_READ_COUNT 4096
    /* 每处理 YIELD_EVERY_FRAMES 帧后让出 CPU 一次，降低 CPU 占用率 */
    #define YIELD_EVERY_FRAMES 10000
    #define YIELD_USLEEP_US    2000  /* 每次让出 2ms */
    Can_frame_t *bulkBuf = malloc(sizeof(Can_frame_t) * BULK_READ_COUNT);
    if (!bulkBuf)
    {
        LOG_Error("MergeAndFilterAscFiles: malloc failed for bulk read buffer\n");
        BatchWriter_Destroy(&bw);
        fclose(outFp);
        return -1;
    }

    /* 帧计数器，用于周期性让出 CPU */
    int yieldCounter = 0;

    for (int i = 0; i < fileList->count; i++)
    {
        const char *path = fileList->files[i].filePath;
        FILE *fp = fopen(path, "rb");
        if (!fp)
        {
            LOG_Error("MergeAndFilterAscFiles: failed to open file '%s', errno=%d\n", path, errno);
            continue;
        }

        /* 设置 256KB 大缓冲区，减少 I/O 系统调用 */
        setvbuf(fp, NULL, _IOFBF, 256 * 1024);

        int fileFrames = 0;
        int earlyBreak = 0;
        size_t readCount;

        while ((readCount = fread(bulkBuf, sizeof(Can_frame_t), BULK_READ_COUNT, fp)) > 0)
        {
            for (size_t j = 0; j < readCount; j++)
            {
                Can_frame_t *pFrame = &bulkBuf[j];

                if (pFrame->timestamp < start_ms)
                {
                    skippedFrames++;
                    continue;
                }

                if (pFrame->timestamp > end_ms)
                {
                    skippedFrames++;
                    earlyBreak = 1;
                    break;
                }

                double ts = (double)((pFrame->timestamp - start_ms) / 1000000.0);

                if (ts < 0.0 || ts > maxTime)
                {
                    skippedFrames++;
                    continue;
                }

                char line[256];
                long len = snprintf(line, sizeof(line),
                                    "\n%.6f\t\t%d\t%X\t%s\t%c\t%u\t%02X %02X %02X %02X %02X %02X %02X %02X",
                                    ts,
                                    pFrame->channel,
                                    pFrame->can_id,
                                    pFrame->direction == 0 ? "RX" : "TX",
                                    'd',
                                    pFrame->dlc,
                                    pFrame->data[0], pFrame->data[1], pFrame->data[2], pFrame->data[3],
                                    pFrame->data[4], pFrame->data[5], pFrame->data[6], pFrame->data[7]);

                int ret = BatchWriter_Write(&bw, line, len);
                if (ret != 0)
                {
                    LOG_Error("MergeAndFilterAscFiles: BatchWriter_Write failed\n");
                }
                fileFrames++;
                totalFrames++;
                yieldCounter++;

                /* 周期性让出 CPU，降低 CPU 占用率 */
                if (yieldCounter >= YIELD_EVERY_FRAMES)
                {
                    yieldCounter = 0;
                    usleep(YIELD_USLEEP_US);
                }
            }

            if (earlyBreak)
                break;

        }

        fclose(fp);
    }

    free(bulkBuf);

    BatchWriter_Destroy(&bw);
    fclose(outFp);

    //LOG_Debug("MergeAndFilterAscFiles: completed, totalFrames=%d, skippedFrames=%d\n", totalFrames, skippedFrames);

    return 0;
}

/* ---------- 辅助收集函数 ---------- */
static void CollectFilesFromDir(FileList_t *dest, const char *dir,
                                uint64_t start, uint64_t end, const char *ext)
{
    if (!dest || !dir || !ext)
    {
        LOG_Error("CollectFilesFromDir: invalid params, dest=%p, dir=%p, ext=%p\n", dest, dir, ext);
        return;
    }
    FileList_t *found = FindFilesByTime(start, end, dir, ext);
    if (!found)
        return;
    for (int i = 0; i < found->count; i++)
        AddFileToList(dest, found->files[i].filePath, found->files[i].createTime);
    ReleaseFileList(&found);
}

int ProcessCanAscFiles(uint64_t startTime, uint64_t endTime, const char *canAscPath, const char *uploadTmpDir)
{
    if (!canAscPath || !uploadTmpDir)
    {
        LOG_Error("ProcessCanAscFiles: invalid params, canAscPath=%p, uploadTmpDir=%p\n", canAscPath, uploadTmpDir);
        return -1;
    }

    FileList_t *tgzList = FindFilesByTime(startTime, endTime, canAscPath, ".tgz");
    if (tgzList)
    {
        LOG_Debug("ProcessCanAscFiles: found %d .tgz files\n", tgzList->count);
        int ret = ProcessTarFile(tgzList, uploadTmpDir);
        if (ret != 0)
        {
            LOG_Error("ProcessCanAscFiles: ProcessTarFile failed, ret=%d\n", ret);
        }
        ReleaseFileList(&tgzList);
    }
    else
    {
        LOG_Debug("ProcessCanAscFiles: no .tgz files found\n");
    }

    FileList_t *allAsc = malloc(sizeof(FileList_t));
    if (!allAsc)
    {
        LOG_Error("ProcessCanAscFiles: malloc failed for allAsc\n");
        return -1;
    }

    int ret = InitFileList(allAsc, 200);
    if (ret != 0)
    {
        LOG_Error("ProcessCanAscFiles: InitFileList failed for allAsc, ret=%d\n", ret);
        free(allAsc);
        return -1;
    }

    CollectFilesFromDir(allAsc, canAscPath, startTime, endTime, ".bin");
    //LOG_Debug("ProcessCanAscFiles: collected %d .bin files from canAscPath\n", allAsc->count);

    int prevCount = allAsc->count;
    CollectFilesFromDir(allAsc, uploadTmpDir, startTime, endTime, ".bin");
    //LOG_Debug("ProcessCanAscFiles: collected %d .bin files from uploadTmpDir\n", allAsc->count - prevCount);

    //LOG_Debug("ProcessCanAscFiles: total %d .bin files collected\n", allAsc->count);

    if (allAsc->count == 0)
    {
        LOG_Error("ProcessCanAscFiles: no .bin files found\n");
        ReleaseFileList(&allAsc);
        return -1;
    }

    if (allAsc->count > 1)
    {
        qsort(allAsc->files, allAsc->count, sizeof(FileInfo_t), compare_files);
        //LOG_Debug("ProcessCanAscFiles: files sorted by time\n");
    }

    char tmpAscPath[512];
    snprintf(tmpAscPath, sizeof(tmpAscPath), "%s/tmp.asc", uploadTmpDir);

    ret = MergeAndFilterAscFiles(allAsc, tmpAscPath, startTime, endTime);
    if (ret != 0)
    {
        LOG_Error("ProcessCanAscFiles: MergeAndFilterAscFiles failed, ret=%d\n", ret);
    }
    else
    {
        //LOG_Debug("ProcessCanAscFiles: MergeAndFilterAscFiles completed successfully\n");
    }

    ReleaseFileList(&allAsc);
    LOG_Debug("ProcessCanAscFiles: completed, ret=%d\n", ret);

    return ret;
}
