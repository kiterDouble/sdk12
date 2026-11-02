#include <stdint.h>
#include <unistd.h>

#include <glib.h>

#include "pm_server_api.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "err_code.h"

#include "log_pack.h"
#include "log_pm.h"

static void PmWorkStateDeinit(LogSrCtx_t* ctx)
{
    if (ctx == NULL) {
        return;
    }

    if (!ctx->pmWorkState.initialized) {
        return;
    }

    g_mutex_lock(&ctx->pmWorkState.lock);

    if (ctx->pmWorkState.bootUpTimer != NULL) {
        g_timer_destroy(ctx->pmWorkState.bootUpTimer);
        ctx->pmWorkState.bootUpTimer = NULL;
    }

    if (ctx->pmWorkState.wakeUpTimer != NULL) {
        g_timer_destroy(ctx->pmWorkState.wakeUpTimer);
        ctx->pmWorkState.wakeUpTimer = NULL;
    }

    g_mutex_unlock(&ctx->pmWorkState.lock);

    g_mutex_clear(&ctx->pmWorkState.lock);
    ctx->pmWorkState.initialized = FALSE;
}

static int32_t PmWorkStateInit(LogSrCtx_t* ctx)
{
    if (ctx == NULL) {
        return RET_ERR;
    }

    if (ctx->pmWorkState.initialized) {
        LOG_Warn("[%s] already initialized", __func__);
        return RET_OK;
    }

    g_mutex_init(&ctx->pmWorkState.lock);
    ctx->pmWorkState.state = PM_STATE_NORMAL;
    ctx->pmWorkState.bootUpTimer = NULL;
    ctx->pmWorkState.wakeUpTimer = NULL;
    ctx->pmWorkState.needReconcile = 0;
    ctx->pmWorkState.initialized = TRUE;

    g_mutex_lock(&ctx->pmWorkState.lock);

    ctx->pmWorkState.bootUpTimer = g_timer_new();
    if (ctx->pmWorkState.bootUpTimer != NULL) {
        g_timer_start(ctx->pmWorkState.bootUpTimer);
    }

    ctx->pmWorkState.wakeUpTimer = g_timer_new();
    if (ctx->pmWorkState.wakeUpTimer != NULL) {
        g_timer_start(ctx->pmWorkState.wakeUpTimer);
    }

    g_mutex_unlock(&ctx->pmWorkState.lock);

    if (ctx->pmWorkState.bootUpTimer == NULL || ctx->pmWorkState.wakeUpTimer == NULL) {
        LOG_Error("[%s] timer create failed", __func__);
        PmWorkStateDeinit(ctx);
        return RET_ERR;
    }

    return RET_OK;
}


int32_t PmStateGet(LogSrCtx_t* ctx)
{
    if (ctx == NULL) {
        return PM_STATE_NORMAL;
    }

    return g_atomic_int_get(&ctx->pmWorkState.state);
}

void PmStateSet(LogSrCtx_t* ctx, int32_t state)
{
    if (ctx == NULL) {
        return;
    }

    g_atomic_int_set(&ctx->pmWorkState.state, state);
}

void PmWakeUpTimerRestart(LogSrCtx_t* ctx)
{
    if (ctx == NULL) {
        return;
    }

    g_mutex_lock(&ctx->pmWorkState.lock);
    if (ctx->pmWorkState.wakeUpTimer != NULL) {
        g_timer_start(ctx->pmWorkState.wakeUpTimer);
    }
    g_mutex_unlock(&ctx->pmWorkState.lock);
}

gdouble PmBootUpTimerElapsed(LogSrCtx_t* ctx)
{
    gdouble elapsed = 999.0;

    if (ctx == NULL) {
        return elapsed;
    }

    g_mutex_lock(&ctx->pmWorkState.lock);
    if (ctx->pmWorkState.bootUpTimer != NULL) {
        elapsed = g_timer_elapsed(ctx->pmWorkState.bootUpTimer, NULL);
    }
    g_mutex_unlock(&ctx->pmWorkState.lock);

    return elapsed;
}

gdouble PmWakeUpTimerElapsed(LogSrCtx_t* ctx)
{
    gdouble elapsed = 999.0;

    if (ctx == NULL) {
        return elapsed;
    }

    g_mutex_lock(&ctx->pmWorkState.lock);
    if (ctx->pmWorkState.wakeUpTimer != NULL) {
        elapsed = g_timer_elapsed(ctx->pmWorkState.wakeUpTimer, NULL);
    }
    g_mutex_unlock(&ctx->pmWorkState.lock);

    return elapsed;
}

gint PmNeedReconcileGet(LogSrCtx_t* ctx)
{
    if (ctx == NULL) {
        return 0;
    }

    return g_atomic_int_get(&ctx->pmWorkState.needReconcile);
}

void PmNeedReconcileSet(LogSrCtx_t* ctx, gint value)
{
    if (ctx == NULL) {
        return;
    }

    g_atomic_int_set(&ctx->pmWorkState.needReconcile, value);
}

static void LogPmStateCallback(const uint8_t *recvData,
                               const uint16_t dataLen,
                               void *userData)
{
    LogSrCtx_t *ctx = (LogSrCtx_t*)userData;
    MPUPmStateChange_t *pmStatus = (MPUPmStateChange_t*)recvData;

    do {
        if (recvData == NULL || ctx == NULL) {
            LOG_Warn("LogPmStateCallback param is NULL");
            break;
        }

        if (dataLen != sizeof(MPUPmStateChange_t)) {
            LOG_Warn("LogPmStateCallback dataLen[%d] is invalid", dataLen);
            break;
        }

        if (PmStateGet(ctx) != pmStatus->PmState) {
            PmStateSet(ctx, pmStatus->PmState);
            LOG_Warn("logServer pmState changed to %d", PmStateGet(ctx));
        } else {
            break;
        }

        /** 唤醒后刷新 wakeUpTimer，并标记需要 reconcile */
        if (PmStateGet(ctx) == PM_STATE_NORMAL) {
            PmWakeUpTimerRestart(ctx);
            PmNeedReconcileSet(ctx, 1);
            LOG_Warn("logServer wakeUpTimer refreshed, need reconcile");
        }
    } while (0);
}

int32_t LogPmStateMonitorInit(void* data)
{
    LogSrCtx_t *ctx = (LogSrCtx_t*)data;
    int32_t ret = RET_ERR;
    int32_t retry = 0;

    if (ctx == NULL) {
        LOG_Error("[%s] invalid param", __func__);
        return RET_ERR;
    }

    if (PmWorkStateInit(ctx) != RET_OK) {
        LOG_Error("[%s] PmWorkStateInit failed", __func__);
        return RET_ERR;
    }

    for (retry = 0; retry < 10; retry++) {
        ret = SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, LogPmStateCallback, data);
        if (ret == RET_OK) {
            LOG_Info("[%s] subscribe pm power state notify success, retry=%d", __func__, retry);
            break;
        }
        LOG_Warn("[%s] subscribe pm power state notify failed, retry=%d, ret=%d", __func__, retry, ret);
        sleep(1);
    }

    if (ret != RET_OK) {
        PmWorkStateDeinit(ctx);
    }

    return ret;
}

int32_t LogPmStateMonitorDeinit(void* data)
{
    LogSrCtx_t *ctx = (LogSrCtx_t*)data;
    int32_t ret = RET_ERR;

    ret = UnSubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY);

    if (ctx != NULL) {
        PmWorkStateDeinit(ctx);
    }

    return ret;
}
