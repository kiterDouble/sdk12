#ifndef __FILE_INTERFACE_H__
#define __FILE_INTERFACE_H__


#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <time.h>
#include "can_file_save.h"
#define MAX_FILE_SIZE (20 * 1024 * 1024) // 20MB

#define FILE_EXTENSION ".bin"
#define RMFILE_EXTENSION ".tgz"
#define DIRECTORY   "/mnt/canlog/"

#define FILE_PREFIX "can_"
#define FILE_SUFFIX ".bin"


//#define TARGET_DIR "/mnt/canlog/"


#define EMMC_8G_MAX_SIZE (5)
#define EMMC_32G_MAX_SIZE (15)

#define EMMC_8G_MAX_SIZE_BYTES (5 * 1024LL * 1024LL * 1024LL)
#define EMMC_32G_MAX_SIZE_BYTES (9 * 1024LL * 1024LL * 1024LL)

// 文件信息结构体
typedef struct {
    char *path;
    time_t mtime;
    off_t size;
} file_info_t;




typedef struct
{
    FILE *current_file;
    char current_path[256];
    char bkfilename[256];
} FileWriterCtx_t;

int WriteDataWithRotation(FileWriterCtx_t *ctx,Can_frame_t *frames, int count);
int TarGzCompress(const char *filename);
int TarGzCompressSingleFile(const char *filepath);
void * CanFileHandler(void *arg);
void * TarFileHander(void *arg);
void * ThreadCheckTimeSync(void *arg);

int MainModemInit(uint64_t *timestamp);
#ifdef __cplusplus
}
#endif
#endif
