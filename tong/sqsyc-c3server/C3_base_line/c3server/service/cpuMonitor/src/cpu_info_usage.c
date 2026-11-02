#include "cpu_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "sr_log.h"


int CpuState(CpuStateTime *cpuState)
{
    int retVal = -1;

    if (cpuState == NULL) {
        return -1;
    }
    FILE *filePtr = NULL;
    char lineBuf[512];
    char *parsePtr = NULL;
    char *endPtr = NULL;
    long long fieldVals[10] = {0};
    int fieldIdx = 0;
    long long fieldVal = 0;

    do {
        filePtr = fopen("/proc/stat", "r");
        if (filePtr == NULL) {
            LOG_Debug("failed to open /proc/stat");
            break;
        }

        if (fgets(lineBuf, sizeof(lineBuf), filePtr) == NULL) {
            LOG_Debug("failed to read /proc/stat");
            break;
        }

        parsePtr = lineBuf;
        while (*parsePtr == ' ' || *parsePtr == '\t') parsePtr++;
        if (strncmp(parsePtr, "cpu", 3) != 0 ||
            (parsePtr[3] != ' ' && parsePtr[3] != '\t' && parsePtr[3] != '\0')) {
            LOG_Debug("invalid /proc/stat format");
            break;
        }
        parsePtr += 3;

        fieldIdx = 0;
        while (fieldIdx < 10) {
            errno = 0;
            fieldVal = strtoll(parsePtr, &endPtr, 10);
            if (parsePtr == endPtr) break;
            if (errno == ERANGE) {
                LOG_Warn("[%s] /proc/stat field overflow", __func__);
                break;
            }
            fieldVals[fieldIdx++] = fieldVal;
            parsePtr = endPtr;
        }

        if (fieldIdx < 7) {
            LOG_Debug("读取CPU时间字段不足");
            break;
        }

        cpuState->user       = fieldVals[0];
        cpuState->nice       = fieldVals[1];
        cpuState->system     = fieldVals[2];
        cpuState->idle       = fieldVals[3];
        cpuState->iowait     = fieldVals[4];
        cpuState->irq        = fieldVals[5];
        cpuState->softirq    = fieldVals[6];
        cpuState->steal      = (fieldIdx > 7) ? fieldVals[7] : 0;
        cpuState->guest      = (fieldIdx > 8) ? fieldVals[8] : 0;
        cpuState->guest_nice = (fieldIdx > 9) ? fieldVals[9] : 0;

        retVal = 0;
    } while (0);

    if (filePtr != NULL) {
        fclose(filePtr);
    }

    return retVal;
}


double CpuCapacityUsage(const CpuStateTime *prevState, const CpuStateTime *currState)
{
    if (prevState == NULL || currState == NULL) {
        return 0.0;
    }

    long long prevIdle = 0;
    long long currIdle = 0;
    long long prevNonIdle = 0;
    long long currNonIdle = 0;
    long long prevTotal = 0;
    long long currTotal = 0;
    long long totalDiff = 0;
    long long idleDiff = 0;
    double cpuUsage = 0.0;

    prevIdle = prevState->idle + prevState->iowait;
    currIdle = currState->idle + currState->iowait;

    prevNonIdle = prevState->user + prevState->nice + prevState->system +
                  prevState->irq + prevState->softirq + prevState->steal;
    currNonIdle = currState->user + currState->nice + currState->system +
                  currState->irq + currState->softirq + currState->steal;

    prevTotal = prevIdle + prevNonIdle;
    currTotal = currIdle + currNonIdle;

    totalDiff = currTotal - prevTotal;
    idleDiff = currIdle - prevIdle;

    if (totalDiff <= 0) {
        cpuUsage = 0.0;
    } else {
        cpuUsage = (double)(totalDiff - idleDiff) / totalDiff * 100.0;
        if (cpuUsage < 0.0) {
            cpuUsage = 0.0;
        } else if (cpuUsage > 100.0) {
            cpuUsage = 100.0;
        }
    }

    return cpuUsage;
}
