#include <stdint.h>
#include <unistd.h>

#include <glib.h>

#include "pm_server_api.h"
#include "sr_ipc.h"
#include "sr_log.h"

#include "cpu_info.h"
#include "cpu_pm.h"

static void CpuPmStateCallback(const uint8_t *recvData,
                               const uint16_t dataLen,
                               void *userData)
{
    CpuMonitorTaskData_t *taskData = (CpuMonitorTaskData_t *)userData;
    MPUPmStateChange_t *pmStatus = (MPUPmStateChange_t *)recvData;

    do {
        if (recvData == NULL || taskData == NULL) {
            LOG_Warn("CpuPmStateCallback param is NULL");
            break;
        }

        if (dataLen != sizeof(MPUPmStateChange_t)) {
            LOG_Warn("CpuPmStateCallback dataLen[%d] is invalid", dataLen);
            break;
        }

        if (g_atomic_int_get(&taskData->pmState) != pmStatus->PmState) {
            g_atomic_int_set(&taskData->pmState, pmStatus->PmState);
            LOG_Warn("cpuMonitor pmState changed to %d", pmStatus->PmState);
        }
    } while (0);
}

static void *CpuPmStateSubscribeRetryThread(void *arg)
{
    CpuMonitorTaskData_t *taskData = (CpuMonitorTaskData_t *)arg;
    int32_t ret = -1;

    if (taskData == NULL) {
        return NULL;
    }

    while (g_atomic_int_get(&taskData->pmSubQuitFlag) == 0) {
        ret = SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, CpuPmStateCallback, taskData);
        if (ret == 0) {
            LOG_Info("[%s] subscribe pm power state notify success", __func__);
            break;
        }
        LOG_Warn("[%s] subscribe pm power state notify failed, ret=%d, retry after 2s", __func__, ret);
        sleep(2);
    }

    return NULL;
}

int32_t CpuPmStateMonitorInit(void* data)
{
    CpuMonitorTaskData_t *taskData = (CpuMonitorTaskData_t *)data;

    if (taskData == NULL) {
        LOG_Error("[%s] input data is NULL", __func__);
        return -1;
    }

    g_atomic_int_set(&taskData->pmSubQuitFlag, 0);

    taskData->pmSubThread = g_thread_new("cpu_pm_sub", CpuPmStateSubscribeRetryThread, taskData);
    if (taskData->pmSubThread == NULL) {
        LOG_Error("[%s] g_thread_new failed", __func__);
        return -1;
    }

    return 0;
}

int32_t CpuPmStateMonitorDeinit(void* data)
{
    CpuMonitorTaskData_t *taskData = (CpuMonitorTaskData_t *)data;

    if (taskData != NULL) {
        g_atomic_int_set(&taskData->pmSubQuitFlag, 1);
        if (taskData->pmSubThread != NULL) {
            g_thread_join(taskData->pmSubThread);
            g_thread_unref(taskData->pmSubThread);
            taskData->pmSubThread = NULL;
        }
    }

    return UnSubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY);
}
