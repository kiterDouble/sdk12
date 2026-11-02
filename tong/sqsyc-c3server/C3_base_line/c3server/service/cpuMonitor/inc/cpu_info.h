#ifndef CPU_INFO_H
#define CPU_INFO_H

#include <stddef.h>
#include <glib.h>
#include <stdint.h>
#include <time.h>

#define CPU_HIGH_THRESHOLD      55.0
#define CPU_LOW_THRESHOLD       45.0
#define CPU_SAMPLE_INTERVAL_MS  1000
#define PROC_SAMPLE_INTERVAL_MS 5000
#define PROC_MAX_NUM            128
#define PROC_TOP_N              10
#define CPU_ABOVE95_MAX_EVENTS  20

/* ++++++++++++++++++++++++++++++++ cpu_info_usage ++++++++++++++++++++++++++++++++ */

typedef struct {
    long long user;
    long long nice;
    long long system;
    long long idle;
    long long iowait;
    long long irq;
    long long softirq;
    long long steal;
    long long guest;
    long long guest_nice;
} CpuStateTime;

int CpuState(CpuStateTime *cpuState);
double CpuCapacityUsage(const CpuStateTime *prevState, const CpuStateTime *currState);

/* ++++++++++++++++++++++++++++++++ cpu_info_proc ++++++++++++++++++++++++++++++++ */
typedef struct {
    int pid;
    unsigned long long utime;
    unsigned long long stime;
    unsigned long long prevUtime;
    unsigned long long prevStime;
    char name[64];
    int valid;
} ProcCpuInfo;

int ProcState(int pid, ProcCpuInfo *proc);
int ProcName(int pid, char *name, size_t len);
int ScanProcs(ProcCpuInfo *procList, int maxNum);
int ProcCompare(const void *a, const void *b);

/* ++++++++++++++++++++++++++++++++ cpu_info_monitor ++++++++++++++++++++++++++++++++ */

typedef struct {
    gint taskQuitFlag;
    GThread *taskThread;
    gint pmState;
    gint pmSubQuitFlag;
    GThread *pmSubThread;
} CpuMonitorTaskData_t;

int InitCpuMonitorTask(CpuMonitorTaskData_t *taskData);
void ReleaseCpuMonitorTask(CpuMonitorTaskData_t *taskData);

/* ++++++++++++++++++++++++++++++++ cpu_pm ++++++++++++++++++++++++++++++++ */
int32_t CpuPmStateMonitorInit(void* data);
int32_t CpuPmStateMonitorDeinit(void* data);

/* ++++++++++++++++++++++++++++++++ cpu_info_statistics ++++++++++++++++++++++++++++++++ */
typedef struct {
    time_t startTime;
    time_t endTime;
    int hasEnd;  /* 1=有结束时间, 0=无结束时间(窗口结束时仍在高负载) */
} Above95Event_t;

typedef struct {
    /* 分档计数 */
    unsigned int countUnder45;
    unsigned int count45to65;
    unsigned int count65to85;
    unsigned int count85to95;
    unsigned int count95to100;

    /* 极值和平均 */
    double maxUsage;
    double totalUsage;
    unsigned int sampleCount;

    /* 时间（wall-clock） */
    time_t startTime;
    time_t endTime;

    /* 系统运行时间（秒） */
    long startUpTime;
    long endUpTime;

    /* 高负载事件 */
    unsigned int above95Count;
    int above95Prev;
    int above95Active;
    Above95Event_t above95Event[CPU_ABOVE95_MAX_EVENTS];
    unsigned int above95EventCount;

    /* 内部计时（CLOCK_MONOTONIC，不包含休眠时间） */
    struct timespec monoStart;
} CpuStatWindow_t;

typedef struct {
    CpuStatWindow_t window10min;
    CpuStatWindow_t window60min;
    CpuStatWindow_t window3h;
    CpuStatWindow_t window6h;
    CpuStatWindow_t window12h;
} CpuStatisticsCtx_t;

void CpuStatisticsInit(CpuStatisticsCtx_t *ctx);
void CpuStatisticsUpdate(CpuStatisticsCtx_t *ctx, double cpuUsage, long upTime);


#endif
