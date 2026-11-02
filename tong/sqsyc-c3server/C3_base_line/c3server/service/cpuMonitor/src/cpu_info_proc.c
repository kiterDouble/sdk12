#include "cpu_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include "sr_log.h"


int ProcState(int pid, ProcCpuInfo *proc)
{
    int retVal = -1;

    if (proc == NULL) {
        return -1;
    }
    FILE *filePtr = NULL;
    char path[64];
    char lineBuf[2048];
    char *parsePtr = NULL;
    char *openParen = NULL;
    unsigned long long utime = 0;
    unsigned long long stime = 0;

    do {
        snprintf(path, sizeof(path), "/proc/%d/stat", pid);
        filePtr = fopen(path, "r");
        if (filePtr == NULL) break;

        if (fgets(lineBuf, sizeof(lineBuf), filePtr) == NULL) break;

        openParen = strchr(lineBuf, '(');
        parsePtr = strrchr(lineBuf, ')');
        if (openParen == NULL || parsePtr == NULL || parsePtr <= openParen) break;
        parsePtr++;
        /* comm 字段结束后必须是空格，确保找到的是真正的字段分隔位置 */
        if (*parsePtr != ' ') break;

        if (sscanf(parsePtr, " %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %llu %llu",
                   &utime, &stime) != 2) break;

        proc->pid = pid;
        proc->utime = utime;
        proc->stime = stime;
        retVal = 0;
    } while (0);

    if (filePtr != NULL) {
        fclose(filePtr);
    }

    return retVal;
}

int ProcName(int pid, char *name, size_t len)
{
    int retVal = -1;

    if (name == NULL || len == 0) {
        return -1;
    }
    FILE *filePtr = NULL;
    char path[64];

    do {
        snprintf(path, sizeof(path), "/proc/%d/comm", pid);
        filePtr = fopen(path, "r");
        if (filePtr == NULL) break;

        if (fgets(name, len, filePtr) == NULL) break;

        name[strcspn(name, "\n")] = '\0';
        retVal = 0;
    } while (0);

    if (filePtr != NULL) {
        fclose(filePtr);
    }

    return retVal;
}

int ScanProcs(ProcCpuInfo *procList, int maxNum)
{
    int count = 0;

    if (procList == NULL || maxNum <= 0) {
        return 0;
    }
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    int pid = 0;
    ProcCpuInfo proc = {0};

    do {
        dir = opendir("/proc");
        if (dir == NULL) break;

        while ((entry = readdir(dir)) != NULL && count < maxNum) {
            if (entry->d_name[0] < '0' || entry->d_name[0] > '9') continue;

            char *endPtr = NULL;
            long pidLong = strtol(entry->d_name, &endPtr, 10);
            if (endPtr == entry->d_name || *endPtr != '\0' || pidLong <= 0 || pidLong > INT_MAX) continue;
            pid = (int)pidLong;

            memset(&proc, 0, sizeof(proc));
            if (ProcState(pid, &proc) != 0) continue;

            /* 过滤休眠半休眠进程 */
            if ((proc.utime + proc.stime) < 2) continue;

            if (ProcName(pid, proc.name, sizeof(proc.name)) != 0) {
                snprintf(proc.name, sizeof(proc.name), "unknown");
            }

            proc.valid = 1;
            procList[count++] = proc;
        }
    } while (0);

    if (dir != NULL) {
        closedir(dir);
    }

    return count;
}

int ProcCompare(const void *a, const void *b)
{
    const ProcCpuInfo *pa = a;
    const ProcCpuInfo *pb = b;
    unsigned long long diffA = 0;
    unsigned long long diffB = 0;

    if (!pa->valid && !pb->valid) return 0;
    if (!pa->valid) return 1;
    if (!pb->valid) return -1;

    diffA = (pa->utime >= pa->prevUtime ? pa->utime - pa->prevUtime : 0) +
            (pa->stime >= pa->prevStime ? pa->stime - pa->prevStime : 0);
    diffB = (pb->utime >= pb->prevUtime ? pb->utime - pb->prevUtime : 0) +
            (pb->stime >= pb->prevStime ? pb->stime - pb->prevStime : 0);

    if (diffA < diffB) return 1;
    if (diffA > diffB) return -1;
    return 0;
}
