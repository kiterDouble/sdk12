#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include "asc_file_processor.h"
#include "sirunMdlSafety.h"

#include"sr_log.h"

#define CAN_ASC_FILE_PATH "/mnt/canlog"
#define CAN_UPLOAD_FILE_TMP_DIR "/mnt/canlog/log"

/* 时间解析：支持时间戳 或 "2026-04-10 00:00:00" */
uint64_t parse_time(const char *str)
{
    if (!str)
    {
        LOG_Error("parse_time: str is NULL\n");
        return 0;
    }

    /* 纯数字 -> 时间戳 */
    if (strspn(str, "0123456789") == strlen(str))
    {
        uint64_t result = strtoull(str, NULL, 10);
        LOG_Debug("parse_time: parsed as timestamp, value=%lu\n", result);
        return result;
    }

    /* 字符串时间 */
    struct tm tm = {0};
    if (strptime(str, "%Y-%m-%d %H:%M:%S", &tm) != NULL)
    {
        uint64_t result = (uint64_t)mktime(&tm);
        LOG_Debug("parse_time: parsed as datetime string, value=%lu\n", result);
        return result;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    LOG_INIT("asc_file");
   
    uint64_t startTime = 0;
    uint64_t endTime = 0;

    /* 默认时间（可选） */
    uint64_t defaultStart = 1775808216;
    uint64_t defaultEnd = 1776757386;
    char *tmp = "/mnt/canlog/";

    /* 降低进程优先级，避免处理大量数据时占用过多 CPU 影响其他业务 */
    nice(10);

//    int32_t ret = SirunSetCoreDumpInner((int8_t *)tmp);
//    if (ret != 0)
//    {
//        LOG_Error("main: SirunSetCoreDumpInner failed, ret=%d\n", ret);
//    }
   
    
    int32_t ret = SirunSignalCatchInit();
    if (ret != 0)
    {
        LOG_Error("main: SirunSignalCatchInit failed, ret=%d\n", ret);
    }
   

    if (argc >= 3)
    {
        startTime = parse_time(argv[1]);
        endTime = parse_time(argv[2]);
    }
    else
    {
        LOG_Debug("main: using default time\n");
        startTime = defaultStart;
        endTime = defaultEnd;
    }

    /* 校验 */
    if (startTime == 0 || endTime == 0)
    {
        LOG_Error("main: time parse failed, startTime=%lu, endTime=%lu\n", startTime, endTime);
        return -1;
    }

    if (startTime > endTime)
    {
        LOG_Error("main: startTime > endTime, startTime=%lu, endTime=%lu\n", startTime, endTime);
        return -1;
    }

    //LOG_Debug("main: calling ProcessCanAscFiles, startTime=%lu, endTime=%lu", startTime, endTime);
    ret = ProcessCanAscFiles(startTime, endTime, CAN_ASC_FILE_PATH, CAN_UPLOAD_FILE_TMP_DIR);
    if (ret != 0)
    {
        LOG_Error("main: ProcessCanAscFiles failed, ret=%d\n", ret);
        return -1;
    }
    LOG_Debug("main: ProcessCanAscFiles completed successfully\n");
    return 0;
}
