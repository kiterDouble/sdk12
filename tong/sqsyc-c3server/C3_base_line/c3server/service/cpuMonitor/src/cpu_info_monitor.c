
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#include "cpu_info.h"
#include "pm_server_api.h"
#include "sr_log.h"

typedef struct {
    int isMonitoringProcs;
    unsigned int procSampleTimer;
    unsigned int procSampleCount;
    ProcCpuInfo procList[PROC_MAX_NUM];
    ProcCpuInfo procListPrev[PROC_MAX_NUM];
    int procCount;
    int procCountPrev;
} ProcMonitorContext_t;

static long CpuUpTime(void)
{
    struct sysinfo sysInfo;

    if (sysinfo(&sysInfo) != 0) {
        LOG_Debug("读取系统运行时间失败");
        return -1;
    }

    return sysInfo.uptime;
}

static void CpuNanoSleepAbs(struct timespec *nextWake, unsigned int milliSec)
{
    struct timespec now;
    struct timespec rel;

    if (nextWake == NULL) {
        return;
    }

    while (1) {
        if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
            LOG_Error("[%s] clock_gettime failed: %s", __func__, strerror(errno));
            /* fallback: 使用 nanosleep 休眠请求的时长，避免 usleep 整数溢出 */
            struct timespec fallback = {
                .tv_sec = milliSec / 1000,
                .tv_nsec = (milliSec % 1000) * 1000000L,
            };
            nanosleep(&fallback, NULL);
            break;
        }
        rel.tv_sec = nextWake->tv_sec - now.tv_sec;
        rel.tv_nsec = nextWake->tv_nsec - now.tv_nsec;
        if (rel.tv_nsec < 0) {
            rel.tv_sec--;
            rel.tv_nsec += 1000000000L;
        }
        if (rel.tv_sec < 0 || (rel.tv_sec == 0 && rel.tv_nsec <= 0)) {
            break;
        }
        if (nanosleep(&rel, NULL) == 0 || errno != EINTR) {
            break;
        }
    }

    /* 推进到下一次唤醒时间点 */
    nextWake->tv_sec += milliSec / 1000;
    nextWake->tv_nsec += (milliSec % 1000) * 1000000L;
    if (nextWake->tv_nsec >= 1000000000L) {
        nextWake->tv_sec++;
        nextWake->tv_nsec -= 1000000000L;
    }
}

static void StartProcMonitoring(ProcMonitorContext_t *ctx)
{
    int i = 0;

    ctx->procCountPrev = ScanProcs(ctx->procListPrev, PROC_MAX_NUM);
    for (i = 0; i < ctx->procCountPrev; i++) {
        ctx->procListPrev[i].prevUtime = ctx->procListPrev[i].utime;
        ctx->procListPrev[i].prevStime = ctx->procListPrev[i].stime;
    }
    ctx->isMonitoringProcs = 1;
    ctx->procSampleTimer = 0;
    ctx->procSampleCount = 0;
}

static void StopProcMonitoring(ProcMonitorContext_t *ctx)
{
    ctx->isMonitoringProcs = 0;
    ctx->procSampleTimer = 0;
    ctx->procSampleCount = 0;
    memset(ctx->procList, 0, sizeof(ctx->procList));
    memset(ctx->procListPrev, 0, sizeof(ctx->procListPrev));
    ctx->procCount = 0;
    ctx->procCountPrev = 0;
}

static void SampleAndPrintProcs(ProcMonitorContext_t *ctx)
{
    int i = 0;
    int j = 0;
    unsigned long long procDiff = 0;
    long clkTck = sysconf(_SC_CLK_TCK);
    if (clkTck <= 0) {
        LOG_Warn("sysconf(_SC_CLK_TCK) failed, skip proc sampling");
        return;
    }
    double procCpuPercent = 0.0;

    ctx->procCount = ScanProcs(ctx->procList, PROC_MAX_NUM);

    for (i = 0; i < ctx->procCount; i++) {
        ctx->procList[i].prevUtime = ctx->procList[i].utime;
        ctx->procList[i].prevStime = ctx->procList[i].stime;
        for (j = 0; j < ctx->procCountPrev; j++) {
            if (ctx->procListPrev[j].pid == ctx->procList[i].pid) {
                ctx->procList[i].prevUtime = ctx->procListPrev[j].utime;
                ctx->procList[i].prevStime = ctx->procListPrev[j].stime;
                break;
            }
        }
        /* PID 复用检测：若当前 tick 小于基线，说明进程已换，标记无效 */
        if (ctx->procList[i].utime < ctx->procList[i].prevUtime ||
            ctx->procList[i].stime < ctx->procList[i].prevStime) {
            ctx->procList[i].valid = 0;
        }
    }

    qsort(ctx->procList, ctx->procCount, sizeof(ProcCpuInfo), ProcCompare);

    LOG_Warn("== TOP %d", PROC_TOP_N);
    for (i = 0; i < PROC_TOP_N && i < ctx->procCount; i++) {
        if (!ctx->procList[i].valid) break;
        procDiff = (ctx->procList[i].utime >= ctx->procList[i].prevUtime ?
                    ctx->procList[i].utime - ctx->procList[i].prevUtime : 0) +
                   (ctx->procList[i].stime >= ctx->procList[i].prevStime ?
                    ctx->procList[i].stime - ctx->procList[i].prevStime : 0);
        procCpuPercent = (double)procDiff * 100.0 / ((double)PROC_SAMPLE_INTERVAL_MS / 1000.0 * clkTck);
        LOG_Warn("  [%d] %s: %.2f%%", ctx->procList[i].pid, ctx->procList[i].name, procCpuPercent);
    }

    int validCount = 0;
    for (i = 0; i < ctx->procCount; i++) {
        if (ctx->procList[i].valid) {
            ctx->procListPrev[validCount] = ctx->procList[i];
            ctx->procListPrev[validCount].prevUtime = ctx->procListPrev[validCount].utime;
            ctx->procListPrev[validCount].prevStime = ctx->procListPrev[validCount].stime;
            validCount++;
        }
    }
    ctx->procCountPrev = validCount;
    ctx->procSampleCount++;

    ctx->procSampleTimer = 0;
}


static void *CpuInfoMonitorTaskFunc(void *arg)
{
    CpuMonitorTaskData_t *taskData = (CpuMonitorTaskData_t *)arg;
    CpuStateTime prevState = {0};
    CpuStateTime currState = {0};
    ProcMonitorContext_t procCtx = {0};
    CpuStatisticsCtx_t cpuStats = {0};
    struct timespec nextWake = {0};
    long cpuUpTime = 0;
    double cpuUsage = 0.0;

    if (taskData == NULL) {
        return NULL;
    }

    CpuStatisticsInit(&cpuStats);

    while (g_atomic_int_get(&taskData->taskQuitFlag) == 0) {
        /* 检查 PM 状态，休眠时挂起采样线程，避免周期性唤醒影响 MPU 休眠 */
        if (g_atomic_int_get(&taskData->pmState) == PM_STATE_SLEEP) {
            LOG_Warn("[%s] PM state is SLEEP, suspend cpu monitor", __func__);
            sleep(1);
            continue;
        }

        /* 按绝对时间点休眠，并推进到下一次唤醒时间 */
        if (nextWake.tv_sec == 0 && nextWake.tv_nsec == 0) {
            clock_gettime(CLOCK_MONOTONIC, &nextWake);
        } else {
            /* PM 休眠恢复或采样延迟导致 nextWake 落后超过一个周期时，
             * 重新校准到当前时间，避免追赶式突发执行 */
            struct timespec now;
            if (clock_gettime(CLOCK_MONOTONIC, &now) == 0) {
                long long gapSec = (long long)now.tv_sec - (long long)nextWake.tv_sec;
                long long gapNsec = (long long)now.tv_nsec - (long long)nextWake.tv_nsec;
                if (gapNsec < 0) {
                    gapSec--;
                    gapNsec += 1000000000L;
                }
                if (gapSec >= 2) {
                    LOG_Warn("[%s] nextWake lag behind %lld.%09lld s, recalibrate",
                             __func__, gapSec, gapNsec);
                    nextWake = now;
                }
            }
        }
        CpuNanoSleepAbs(&nextWake, CPU_SAMPLE_INTERVAL_MS);

        /* 读取系统运行时间（秒） */
        cpuUpTime = CpuUpTime();
        if (cpuUpTime < 0) {
            LOG_Warn("CpuUpTime read failed, skip this round");
            continue;
        }

        /* 读取 /proc/stat 获取当前 CPU 时间统计 */
        if (CpuState(&currState) != 0) {
            LOG_Warn("CpuState read failed, skip this round");
            continue;
        }

        /* 首次采样：prevState 尚未初始化（idle 为 0），
         * 直接将当前状态保存为基线，跳过本轮计算 */
        if (prevState.idle == 0) {
            prevState = currState;
            continue;
        }

        /* 计算本轮 CPU 使用率，并更新基线供下一轮使用 */
        cpuUsage = CpuCapacityUsage(&prevState, &currState);
        LOG_Debug("[%ld] usage: %.2f%%", cpuUpTime, cpuUsage);

        CpuStatisticsUpdate(&cpuStats, cpuUsage, cpuUpTime);

        prevState = currState;

        /* 根据 CPU 使用率阈值决定是否启动/停止进程级监控 */
        if (cpuUsage >= CPU_HIGH_THRESHOLD && !procCtx.isMonitoringProcs) {
            StartProcMonitoring(&procCtx);
            LOG_Warn("CPU high (%.2f%%), start proc monitoring", cpuUsage);
        } else if (cpuUsage <= CPU_LOW_THRESHOLD && procCtx.isMonitoringProcs && procCtx.procSampleCount >= 3) {
            StopProcMonitoring(&procCtx);
            LOG_Warn("CPU low (%.2f%%), sampled %u times, stop proc monitoring", cpuUsage, procCtx.procSampleCount);
        }

        /* 若处于进程监控状态，累加定时器，到达间隔后采样并打印 TOP N */
        if (procCtx.isMonitoringProcs) {
            procCtx.procSampleTimer += CPU_SAMPLE_INTERVAL_MS;
            if (procCtx.procSampleTimer >= PROC_SAMPLE_INTERVAL_MS) {
                SampleAndPrintProcs(&procCtx);
            }
        }
    }

    return NULL;
}


int InitCpuMonitorTask(CpuMonitorTaskData_t *taskData)
{
    if (taskData == NULL) {
        LOG_Error("input taskData is NULL");
        return -1;
    }

    g_atomic_int_set(&taskData->taskQuitFlag, 0);
    g_atomic_int_set(&taskData->pmState, PM_STATE_NORMAL);

    taskData->taskThread = g_thread_new("cpu_monitor", CpuInfoMonitorTaskFunc, (void *)taskData);
    if (taskData->taskThread == NULL) {
        LOG_Fatal("Create CpuMonitorTaskThread failed");
        return -1;
    }

    return 0;
}

void ReleaseCpuMonitorTask(CpuMonitorTaskData_t *taskData)
{
    if (taskData == NULL) {
        return;
    }

    g_atomic_int_set(&taskData->taskQuitFlag, 1);

    if (taskData->taskThread != NULL) {
        g_thread_join(taskData->taskThread);
        g_thread_unref(taskData->taskThread);
        taskData->taskThread = NULL;
    }
}
