
#include <stdint.h>
#include <unistd.h>

#include "pm_server_api.h"
#include "sr_log.h"
#include "sr_ipc.h"

#include "ctx.h"

static gint g_hsmPmStateHandleThreadQuitFlag = 0;
static GThread* g_hsmPmStateHandleThread = NULL;

static void HsmPmStateManagerCallback(const uint8_t *recvData, const uint16_t dataLen, void *userData)
{
    Ctx_t *ctx = (Ctx_t*)userData;
    MPUPmStateChange_t *pmStatus = (MPUPmStateChange_t *)recvData;    
    do
    {
        if (recvData == NULL || ctx == NULL) {
            LOG_Warn("[%s] param is NULL", __func__);
            break;
        }

        // LOG_Warn("WorkMode:%d, PmState:%d, WakeupSrc:%d, SleepAlarmEventId:%d",
        //     pmStatus->WorkMode, pmStatus->PmState, pmStatus->WakeupSrc, pmStatus->SleepAlarmEventId);

        if (dataLen != sizeof(MPUPmStateChange_t)) {
            LOG_Warn("[%s] dataLen[%d] is invalid\n", __func__, dataLen);
            break;
        }

        if (ctx->pmState != pmStatus->PmState) {
            ctx->pmState = pmStatus->PmState;
            LOG_Warn("[%s] pmState is changed to %d", __func__, ctx->pmState);
        }

    } while (0);
}

static void* HsmPmStateHandleThreadFunc(void* data)
{
    Ctx_t *ctx = (Ctx_t*)data;
    int ret = -1;
    int failCount = 0;
    uint32_t sleepMs = 0;

    while (g_atomic_int_get(&g_hsmPmStateHandleThreadQuitFlag) == 0)
    {
        sleepMs = (failCount >= HSM_INIT_FAIL_COUNT_MAX) ? 1000 : 100;

        g_mutex_lock(&ctx->hsmApiMutex);
        if (ctx->pmState == PM_STATE_NORMAL)
        {
            if (ctx->hsmWorkState != E_HSM_WORK_STATE_NORMAL)
            {
                ret = ctx->hsmApi->init(ctx);
                if (ret == 0)
                {
                    ctx->hsmWorkState = E_HSM_WORK_STATE_NORMAL;
                    failCount = 0;
                    LOG_Info("[%s] HSM API Init success, state -> Normal", __func__);
                }
                else
                {
                    failCount++;
                    LOG_Error("[%s] HSM API Init Failed, ret = %d", __func__, ret);
                    if (failCount >= HSM_INIT_FAIL_COUNT_MAX)
                    {
                        ctx->hsmWorkState = E_HSM_WORK_STATE_FAULT;
                        LOG_Error("[%s] HSM API Init Failed, state -> Fault", __func__);
                    }
                }
            }
            else
            {
                failCount = 0;
            }
        }
        else
        {
            if (ctx->hsmWorkState == E_HSM_WORK_STATE_NORMAL)
            {
                ret = ctx->hsmApi->release(ctx);
                if (ret == 0)
                {
                    ctx->hsmWorkState = E_HSM_WORK_STATE_UNKNOWN;
                    failCount = 0;
                    LOG_Info("[%s] HSM API Release success, state -> Unknown", __func__);
                }
                else
                {
                    failCount++;
                    LOG_Error("[%s] HSM API Release Failed, ret = %d", __func__, ret);
                }
            }
            else
            {
                failCount = 0;
            }
        }
        g_mutex_unlock(&ctx->hsmApiMutex);

        usleep(sleepMs * 1000);
    }
    return NULL;
}


int HsmPmStateManagerInit(void* data)
{
    /* 初始化hsmData处理线程 */
    g_hsmPmStateHandleThread = g_thread_new("HSMTaskDataHandleThread", HsmPmStateHandleThreadFunc, data);
    if (g_hsmPmStateHandleThread == NULL)
    {
        LOG_Fatal("[%s] Create HSMTaskDataHandleThread failed", __func__);
        return -1;
    }

    return SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, HsmPmStateManagerCallback, data);
}

int HsmPmStateManagerRelease(void* data)
{
    Ctx_t *ctx = (Ctx_t*)data;

    g_atomic_int_set(&g_hsmPmStateHandleThreadQuitFlag, 1);

    /* 释放HSMData处理线程 */
    if (g_hsmPmStateHandleThread != NULL){
        g_thread_join(g_hsmPmStateHandleThread);
        g_thread_unref(g_hsmPmStateHandleThread);
    }

    /* 进程退出时如果 HSM 仍处于 Normal 状态，主动调用 release 释放后端资源 */
    if (ctx != NULL && ctx->hsmWorkState == E_HSM_WORK_STATE_NORMAL) {
        g_mutex_lock(&ctx->hsmApiMutex);
        if (ctx->hsmApi != NULL && ctx->hsmApi->release != NULL) {
            (void)ctx->hsmApi->release(ctx);
        }
        ctx->hsmWorkState = E_HSM_WORK_STATE_UNKNOWN;
        g_mutex_unlock(&ctx->hsmApiMutex);
    }

    return UnSubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY);
}