#include <string.h>
#include <errno.h>
#include "cpu_info.h"
#include "sr_log.h"

static void UpdateBucket(CpuStatWindow_t *window, double cpuUsage)
{
    if (cpuUsage < 45.0) {
        window->countUnder45++;
    } else if (cpuUsage < 65.0) {
        window->count45to65++;
    } else if (cpuUsage < 85.0) {
        window->count65to85++;
    } else if (cpuUsage < 95.0) {
        window->count85to95++;
    } else {
        window->count95to100++;
    }
}

static void PrintWindow(const CpuStatWindow_t *window, const char *name)
{
    double avgUsage = 0.0;
    struct tm tmStart = {0};
    struct tm tmEnd = {0};
    char startBuf[32] = {0};
    char endBuf[32] = {0};

    if (window->sampleCount > 0) {
        avgUsage = window->totalUsage / window->sampleCount;
    }

    if (localtime_r(&window->startTime, &tmStart) == NULL) {
        strncpy(startBuf, "invalid", sizeof(startBuf));
        startBuf[sizeof(startBuf) - 1] = '\0';
    } else {
        strftime(startBuf, sizeof(startBuf), "%Y-%m-%d %H:%M:%S", &tmStart);
    }

    if (localtime_r(&window->endTime, &tmEnd) == NULL) {
        strncpy(endBuf, "invalid", sizeof(endBuf));
        endBuf[sizeof(endBuf) - 1] = '\0';
    } else {
        strftime(endBuf, sizeof(endBuf), "%Y-%m-%d %H:%M:%S", &tmEnd);
    }

    LOG_Warn("=== CPU Statistics [%s] ===", name);
    LOG_Warn("  Time Range     : %s ~ %s", startBuf, endBuf);
    LOG_Warn("  UpTime Range   : %lds ~ %lds", window->startUpTime, window->endUpTime);
    LOG_Warn("  Sample Count   : %u", window->sampleCount);
    LOG_Warn("  Usage < 45%%   : %u (%.2f%%)", window->countUnder45,
             window->sampleCount > 0 ? (window->countUnder45 * 100.0 / window->sampleCount) : 0.0);
    LOG_Warn("  Usage 45~65%%  : %u (%.2f%%)", window->count45to65,
             window->sampleCount > 0 ? (window->count45to65 * 100.0 / window->sampleCount) : 0.0);
    LOG_Warn("  Usage 65~85%%  : %u (%.2f%%)", window->count65to85,
             window->sampleCount > 0 ? (window->count65to85 * 100.0 / window->sampleCount) : 0.0);
    LOG_Warn("  Usage 85~95%%  : %u (%.2f%%)", window->count85to95,
             window->sampleCount > 0 ? (window->count85to95 * 100.0 / window->sampleCount) : 0.0);
    LOG_Warn("  Usage 95~100%% : %u (%.2f%%)", window->count95to100,
             window->sampleCount > 0 ? (window->count95to100 * 100.0 / window->sampleCount) : 0.0);
    LOG_Warn("  Max Usage      : %.2f%%", window->maxUsage);
    LOG_Warn("  Avg Usage      : %.2f%%", avgUsage);
    LOG_Warn("  Above95 Count  : %u", window->above95Count);
    if (window->above95EventCount > 0) {
        LOG_Warn("  Above95 Events :");
        for (unsigned int i = 0; i < window->above95EventCount; i++) {
            struct tm tmEvtStart = {0};
            struct tm tmEvtEnd = {0};
            char evtStartBuf[32] = {0};
            char evtEndBuf[32] = {0};

            if (localtime_r(&window->above95Event[i].startTime, &tmEvtStart) == NULL) {
                strncpy(evtStartBuf, "invalid", sizeof(evtStartBuf));
                evtStartBuf[sizeof(evtStartBuf) - 1] = '\0';
            } else {
                strftime(evtStartBuf, sizeof(evtStartBuf), "%Y-%m-%d %H:%M:%S", &tmEvtStart);
            }

            if (window->above95Event[i].hasEnd) {
                if (localtime_r(&window->above95Event[i].endTime, &tmEvtEnd) == NULL) {
                    strncpy(evtEndBuf, "invalid", sizeof(evtEndBuf));
                    evtEndBuf[sizeof(evtEndBuf) - 1] = '\0';
                } else {
                    strftime(evtEndBuf, sizeof(evtEndBuf), "%Y-%m-%d %H:%M:%S", &tmEvtEnd);
                }
                LOG_Warn("    [%02u] %s ~ %s", i + 1, evtStartBuf, evtEndBuf);
            } else {
                LOG_Warn("    [%02u] %s ~ (未结束)", i + 1, evtStartBuf);
            }
        }
    }
}

static void UpdateSingleWindow(CpuStatWindow_t *window, double cpuUsage,
                                struct timespec monoNow, long periodSec,
                                const char *name, long upTime, time_t realNowSec)
{
    /* 首次采样，记录起始时间 */
    if (window->sampleCount == 0) {
        window->startTime = realNowSec;
        window->startUpTime = upTime;
    }

    /* 更新分档 */
    UpdateBucket(window, cpuUsage);

    /* 更新累计值 */
    window->totalUsage += cpuUsage;
    window->sampleCount++;
    if (cpuUsage > window->maxUsage) {
        window->maxUsage = cpuUsage;
    }

    /* 更新高负载事件状态 */
    if (cpuUsage > 95.0) {
        if (!window->above95Prev) {
            window->above95Count++;
            if (window->above95EventCount < CPU_ABOVE95_MAX_EVENTS) {
                window->above95Event[window->above95EventCount].startTime = realNowSec;
                window->above95Event[window->above95EventCount].endTime = 0;
                window->above95Event[window->above95EventCount].hasEnd = 0;
            }
        }
        window->above95Prev = 1;
    } else {
        if (window->above95Prev) {
            if (window->above95EventCount < CPU_ABOVE95_MAX_EVENTS) {
                window->above95Event[window->above95EventCount].endTime = realNowSec;
                window->above95Event[window->above95EventCount].hasEnd = 1;
                window->above95EventCount++;
            }
        }
        window->above95Prev = 0;
    }
    window->above95Active = window->above95Prev;

    /* 更新结束时间和结束运行时间 */
    window->endTime = realNowSec;
    window->endUpTime = upTime;

    /* 检查窗口是否到期（基于 CLOCK_MONOTONIC，不包含休眠时间） */
    long long elapsed = (long long)monoNow.tv_sec - (long long)window->monoStart.tv_sec;
    if (elapsed >= periodSec) {

        PrintWindow(window, name);

        /* 重置窗口 */
        memset(window, 0, sizeof(*window));
        window->startTime = realNowSec;
        window->monoStart = monoNow;
    }
}

void CpuStatisticsInit(CpuStatisticsCtx_t *ctx)
{
    struct timespec monoNow;

    if (ctx == NULL) {
        return;
    }

    memset(ctx, 0, sizeof(*ctx));
    if (clock_gettime(CLOCK_MONOTONIC, &monoNow) != 0) {
        LOG_Error("[%s] clock_gettime failed: %s", __func__, strerror(errno));
        return;
    }
    ctx->window10min.monoStart = monoNow;
    ctx->window60min.monoStart = monoNow;
    ctx->window3h.monoStart = monoNow;
    ctx->window6h.monoStart = monoNow;
    ctx->window12h.monoStart = monoNow;
}

void CpuStatisticsUpdate(CpuStatisticsCtx_t *ctx, double cpuUsage, long upTime)
{
    struct timespec monoNow;
    struct timespec realNow;

    if (ctx == NULL) {
        return;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &monoNow) != 0 ||
        clock_gettime(CLOCK_REALTIME, &realNow) != 0) {
        LOG_Error("[%s] clock_gettime failed: %s", __func__, strerror(errno));
        return;
    }

    UpdateSingleWindow(&ctx->window10min, cpuUsage, monoNow, 600, "10min", upTime, realNow.tv_sec);
    UpdateSingleWindow(&ctx->window60min, cpuUsage, monoNow, 3600, "60min", upTime, realNow.tv_sec);
    UpdateSingleWindow(&ctx->window3h, cpuUsage, monoNow, 10800, "3h", upTime, realNow.tv_sec);
    UpdateSingleWindow(&ctx->window6h, cpuUsage, monoNow, 21600, "6h", upTime, realNow.tv_sec);
    UpdateSingleWindow(&ctx->window12h, cpuUsage, monoNow, 43200, "12h", upTime, realNow.tv_sec);
}
