/**
 * ====================================================
 * @file    net_backoff.c
 * @brief   基于 PM 休眠状态与驻网状态的射频退避控制（仅 Sleep 场景）
 *
 * 设计说明简要：
 *   1) PM 状态变化时，由外部 PM 回调调用 NetBackoffOnPmStateChange()
 *      通知本模块最新 PM 状态；
 *   2) 在 NetBackoffProcess() 中周期调用退避状态机逻辑；
 *   3) 仅处理一种场景：
 *        - 休眠且无网络：退避序列 5min, 10min, 20min, 40min, ... , 24h；
 *   4) 每一轮 Sleep 场景逻辑：
 *        - 检测到休眠且无网时，只计算退避时间并设置定时唤醒，然后立即允许系统休眠；
 *        - 到达定时唤醒后，打开射频，监测驻网 1min；
 *        - 1min 内若驻网成功：退出退避机制，释放休眠锁，射频保持开启；
 *        - 1min 内若仍未驻网：
 *             * 释放休眠锁；
 *             * 关闭射频；
 *             * 设置下一次定时唤醒（当前退避时间）；
 *             * 系统可再次进入休眠，唤醒后重复上述逻辑。
 *
 * 说明：
 *   - 唤醒后的无网场景（例如 KL15 唤醒无网）不在本模块处理范围内，
 *     由上层统一控制，无需退避。
 *   - 本文件不依赖具体定时器实现，通过单调时间戳 + 周期轮询 + PM 定时唤醒
 *     实现退避节奏。
 *   - 请在网络主线程中固定周期（例如 1 秒）调用 NetBackoffProcess()。
 *   - PM 回调统一在其他模块中实现，并在回调中调用 NetBackoffOnPmStateChange()
 *     以通知本模块。
 * ====================================================
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include "hal_modem_nw.h"
#include "pm_server_api.h"
#include "sr_log.h"
#include "modem_api.h"
#include "net_service.h"

#define NET_BACKOFF_SLEEP_EVENT_NAME      "net_backoff_sleep_event"
#define NET_BACKOFF_SLEEP_EVENT_ID        CPU_INVALID_EVENT_ID

/* ========================= 射频退避状态机定义 ========================= */

/**
 * @brief 退避场景类型（当前仅保留 Sleep 场景）
 */
typedef enum
{
    NetBackoffSceneNone  = 0,  /**< 未启用退避 */
    NetBackoffSceneSleep = 1   /**< 休眠 + 无网 场景 */
} NetBackoffScene_t;

/**
 * @brief 退避内部状态
 */
typedef enum
{
    NetBackoffStateIdle        = 0,  /**< 未启用 / 已退出退避 */
    NetBackoffStateOpenMonitor = 1,  /**< 射频打开 + 1min 监测驻网 */
    NetBackoffStateRfOff       = 2   /**< 射频关闭 + 退避等待（依赖 PM 定时唤醒） */
} NetBackoffState_t;

/**
 * @brief 射频退避控制上下文
 */
typedef struct
{
    NetBackoffScene_t Scene;            /**< 当前退避场景 */
    NetBackoffState_t State;            /**< 当前内部状态 */
    uint32_t          StepIndex;        /**< 当前第几轮退避（0,1,2,...） */
    uint32_t          CurOffSec;        /**< 当前关射频退避时间（秒） */
    uint64_t          StateEnterTimeMs; /**< 当前状态进入时刻（毫秒） */
    uint64_t          NextActionTimeMs; /**< 下一次动作触发时间（毫秒） */
} NetBackoffCtrl_t;

extern NetSrvCtx_t g_srvCtx;

/* 全局退避控制实例 */
static NetBackoffCtrl_t g_NetBackoff;
/* Sleep 场景的唤醒闹钟是否已创建 */
static bool             g_SleepAlarmCreated = false;

/**
 * ============================================================
 * 函数名称: NetBackoffGetNowMs
 * 函数作用: 获取当前单调时间（毫秒）
 *
 * 参数说明:
 *   无
 *
 * 返回值:
 *   uint64_t - 当前单调毫秒时间，失败时返回 0
 * ============================================================
 */
static uint64_t NetBackoffGetNowMs(void)
{
    struct timespec ts = {0};
    int32_t         ret;

    ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (ret != 0)
    {
        /* 兜底：失败时返回 0，状态机会自然等待 */
        return 0U;
    }

    return (((uint64_t)ts.tv_sec) * 1000ULL) + (((uint64_t)ts.tv_nsec) / 1000000ULL);
}

/**
 * ============================================================
 * 函数名称: NetIsRegistered
 * 函数作用: 查询当前是否已经驻网（仅判断注册状态）
 *
 * 参数说明:
 *   无
 *
 * 返回值:
 *   true  - 已驻网
 *   false - 未驻网
 * ============================================================
 */
static bool NetIsRegistered(void)
{
    ModemSrvNotify_t modemSrvNotify;
    bool             isRegistered = false;

    (void)memset(&modemSrvNotify, 0, sizeof(ModemSrvNotify_t));

    memcpy(&modemSrvNotify, g_srvCtx.modemSrvNotify, sizeof(ModemSrvNotify_t));

    if (modemSrvNotify.dataRegStatus == E_HAL_MODEM_NW_REG_STATUS_REGISTERED)
    {
        isRegistered = true;
    }

    return isRegistered;
}

/**
 * ============================================================
 * 函数名称: NetBackoffScheduleWake
 * 函数作用: 设置定时唤醒，用于 Sleep 场景退避的唤醒调度
 *
 * 参数说明:
 *   offSeconds - 距离唤醒的时间（秒）
 *
 * 返回值:
 *   无
 *
 * 注意事项:
 *   - 本接口只在 Sleep 场景下调用。
 *   - 若之前已经创建过同一 EVENT ID 的闹钟，会先销毁旧闹钟再创建新的，
 *     确保“每次使用前重新设置定时唤醒”。
 * ============================================================
 */
static void NetBackoffScheduleWake(uint32_t offSeconds)
{
    /* 0 秒没有意义，直接返回 */
    if (offSeconds == 0U)
    {
        return;
    }

    /* 如已创建过闹钟，先销毁旧闹钟，避免重复 */
    if (g_SleepAlarmCreated)
    {
        PmDestroySleepAlarm(NET_BACKOFF_SLEEP_EVENT_ID);
        g_SleepAlarmCreated = false;
    }

    /* 创建新的定时唤醒闹钟 */
    PmCreateSleepAlarm(NET_BACKOFF_SLEEP_EVENT_ID, (int32_t)offSeconds, 0);
    g_SleepAlarmCreated = true;

    LOG_Debug("[Backoff] Schedule wake in %u sec.\n", (unsigned int)offSeconds);
}

/**
 * ============================================================
 * 函数名称: NetBackoffCalcOffTimeSec
 * 函数作用: 计算 Sleep 场景下的关射频退避时间（秒）
 *
 * 参数说明:
 *   stepIndex - 第几轮退避（0,1,2,...）
 *
 * 返回值:
 *   uint32_t  - 关射频时间（秒），最大不超过 24 小时
 * ============================================================
 */
static uint32_t NetBackoffCalcOffTimeSec(uint32_t stepIndex)
{
    const uint32_t baseMin = 5U;        /* Sleep 场景基数：5 分钟 */
    const uint32_t maxMin  = 24U * 60U; /* 24 小时 */
    uint32_t       offMin;

    if (stepIndex >= 31U)
    {
        /* 防止左移溢出，直接视为最大值 */
        offMin = maxMin;
    }
    else
    {
        offMin = baseMin << stepIndex;
    }

    if (offMin > maxMin)
    {
        offMin = maxMin;
    }

    return offMin * 60U;
}

/**
 * ============================================================
 * 函数名称: NetBackoffReset
 * 函数作用: 重置退避控制上下文，退出退避机制
 *
 * 参数说明:
 *   无
 *
 * 返回值:
 *   无
 * ============================================================
 */
static void NetBackoffReset(void)
{
    /* 如果之前创建过定时唤醒闹钟，需要在退出退避时删除 */
    if (g_SleepAlarmCreated)
    {
        PmDestroySleepAlarm(NET_BACKOFF_SLEEP_EVENT_ID);
        g_SleepAlarmCreated = false;
        LOG_Debug("[Backoff] Destroy sleep alarm on reset.\n");
    }

    (void)memset(&g_NetBackoff, 0, sizeof(NetBackoffCtrl_t));
    g_NetBackoff.Scene     = NetBackoffSceneNone;
    g_NetBackoff.State     = NetBackoffStateIdle;
    g_NetBackoff.StepIndex = 0U;

    /* 退出退避前，确保清理 Sleep 事件标记 */
    PmCleanSleepEvent(NET_BACKOFF_SLEEP_EVENT_NAME);
}

/* ========================= 场景切换与状态切换 ========================= */

/**
 * ============================================================
 * 函数名称: NetBackoffStartOpenMonitorPhase
 * 函数作用: 进入“射频打开 + 驻网监测”阶段（1 分钟）
 *
 * 参数说明:
 *   无
 *
 * 返回值:
 *   无
 *
 * 注意事项:
 *   - 本阶段只负责打开射频，并在固定时间窗口内检测是否已经驻网，
 *     不在本函数中触发拨号或建立数据业务。
 *   - Sleep 场景下会先获取休眠锁，保证监测期间系统不进入深度休眠。
 * ============================================================
 */
static void NetBackoffStartOpenMonitorPhase(void)
{
    uint64_t nowMs;

    /* 仅在 Sleep 场景下获取休眠锁 */
    if (g_NetBackoff.Scene == NetBackoffSceneSleep)
    {
        PmSetSleepEvent(NET_BACKOFF_SLEEP_EVENT_NAME);
    }

    nowMs = NetBackoffGetNowMs();

    /* 打开射频 */
    // ModemSrvSetAirplaneMode(E_MODEM_SRV_DMS_AIRPLANE_OFF);
    LOG_Debug("[Backoff] Open RF, at+cfun=1\n");
    (void)system("ql_cmd at AT+CFUN=1");

    g_NetBackoff.State            = NetBackoffStateOpenMonitor;
    g_NetBackoff.StateEnterTimeMs = nowMs;
    g_NetBackoff.NextActionTimeMs = nowMs + (60ULL * 1000ULL); /* 1 分钟监测窗口 */

    LOG_Debug("[Backoff] Enter OPEN_MONITOR, scene=%d\n", (int)g_NetBackoff.Scene);
}

/**
 * ============================================================
 * 函数名称: NetBackoffStartOffPhase
 * 函数作用: 进入“射频关闭 + 退避等待”阶段
 *
 * 参数说明:
 *   无
 *
 * 返回值:
 *   无
 *
 * 注意事项:
 *   - 仅 Sleep 场景使用：
 *       1) 计算本轮退避时间；
 *       2) 释放休眠锁；
 *       3) 配置定时唤醒；
 *       4) 关闭射频。
 * ============================================================
 */
static void NetBackoffStartOffPhase(void)
{
    uint64_t nowMs;

    /* 计算当前这轮的退避时间（5min,10min,...,24h） */
    g_NetBackoff.CurOffSec = NetBackoffCalcOffTimeSec(g_NetBackoff.StepIndex);

    /* 下一轮退避时间加倍（已经封顶在 24h 内） */
    if (g_NetBackoff.StepIndex < 16U)
    {
        g_NetBackoff.StepIndex++;
    }

    /* 关闭射频 */
    // ModemSrvSetAirplaneMode(E_MODEM_SRV_DMS_AIRPLANE_ON);
    LOG_Debug("[Backoff] Close RF, at+cfun=0");
    (void)system("ql_cmd at AT+CFUN=0");

    nowMs = NetBackoffGetNowMs();

    g_NetBackoff.State            = NetBackoffStateRfOff;
    g_NetBackoff.StateEnterTimeMs = nowMs;
    g_NetBackoff.NextActionTimeMs = nowMs + (((uint64_t)g_NetBackoff.CurOffSec) * 1000ULL);

    LOG_Debug("[Backoff] Enter RF_OFF, off=%u sec\n",
              (unsigned int)g_NetBackoff.CurOffSec);

    /* Sleep 场景：释放休眠锁 + 设置定时唤醒 */
    if (g_NetBackoff.Scene == NetBackoffSceneSleep)
    {
        PmCleanSleepEvent(NET_BACKOFF_SLEEP_EVENT_NAME);
        NetBackoffScheduleWake(g_NetBackoff.CurOffSec);
    }
}

/**
 * ============================================================
 * 函数名称: NetBackoffEnterSleepScene
 * 函数作用: 进入 Sleep 场景退避
 *
 * 参数说明:
 *   无
 *
 * 返回值:
 *   无
 *
 * 设计要点:
 *   - 仅在“第一次进入 Sleep 场景且无网”时生效：
 *       * 设置 Scene=Sleep；
 *       * 直接进入 RF_OFF 退避阶段（计算退避时间+设置定时唤醒）；
 *       * 不立即做 1 分钟网络检测。
 *   - 后续再次进入 Sleep（同一轮退避内）仅记录 PM 状态变化，
 *     不会重复设置闹钟或变更 StepIndex。
 * ============================================================
 */
static void NetBackoffEnterSleepScene(void)
{
    /* 若已经在 Sleep 场景中，则不重复初始化，避免重复设置退避 */
    if (g_NetBackoff.Scene == NetBackoffSceneSleep)
    {
        return;
    }

    g_NetBackoff.Scene = NetBackoffSceneSleep;

    /*
     * 首次进入 Sleep 场景时：
     *   - 不立即做 1 分钟监测；
     *   - 先根据当前 StepIndex 计算退避时间；
     *   - 设置定时唤醒并允许系统进入休眠。
     * 等到定时唤醒后，在 PM 回调中调用 NetBackoffStartOpenMonitorPhase()
     * 进行 1 分钟网络检测。
     */
    NetBackoffStartOffPhase();
}

/* ========================= PM 状态处理 ========================= */

/**
 * ============================================================
 * 函数名称: NetBackoffHandlePmStateChange
 * 函数作用: 处理 PM 状态变化，决定是否进入或退出 Sleep 退避场景
 *
 * 参数说明:
 *   pmState - 最新的 PM 状态指针
 *
 * 返回值:
 *   无
 *
 * 注意事项:
 *   - 仅针对 PM_STATE_SLEEP 进行退避处理；
 *   - 对于非 Sleep 状态，根据唤醒源决定是否退出退避机制或进入 1 分钟监测。
 * ============================================================
 */
static void NetBackoffHandlePmStateChange(MPUPmStateChange_t *pmState)
{
    static uint8_t s_LastPmState = 0U;
    uint8_t        curPmState;

    if (pmState == NULL)
    {
        return;
    }

    curPmState = pmState->PmState;

    if (curPmState == s_LastPmState)
    {
        return;
    }

    LOG_Debug("[Backoff] PM change: PmState=%u, WakeupSrc=%u\n",
              (unsigned int)pmState->PmState,
              (unsigned int)pmState->WakeupSrc);

    /* 1. 进入休眠：若当前无网络，则启动 Sleep 场景退避（只设置定时唤醒，不立即检测） */
    if (curPmState == PM_STATE_SLEEP)
    {
        if (!NetIsRegistered())
        {
            NetBackoffEnterSleepScene();
        }
        else
        {
            /* 已驻网则不需要退避，保持现状 */
        }

        s_LastPmState = curPmState;
        return;
    }

    /*
     * 2. 非 Sleep 状态：
     *    - 若上一状态是 SLEEP 且唤醒源为 KL15：视为点火唤醒，直接退出退避并清零 StepIndex；
     *    - 若上一状态是 SLEEP 且唤醒源非 KL15：视为定时唤醒或其它非点火唤醒，
     *      若当前处于 Sleep 场景，则进入 1 分钟网络监测阶段。
     */
    if (s_LastPmState == PM_STATE_SLEEP)
    {
        if (pmState->WakeupSrc == PM_WAKEUP_SOURCE_KL15)
        {
            LOG_Debug("[Backoff] Leave sleep by KL15, reset backoff.\n");
            NetBackoffReset();
        }
        else
        {
            LOG_Debug("[Backoff] Leave sleep by non-KL15, start monitor.\n");

            if (g_NetBackoff.Scene == NetBackoffSceneSleep)
            {
                /* 定时唤醒后进入 1 分钟网络检测 */
                NetBackoffStartOpenMonitorPhase();
            }
        }
    }

    s_LastPmState = curPmState;
}

/* ========================= 状态机主循环 ========================= */

/**
 * ============================================================
 * 函数名称: NetBackoffProcessFsm
 * 函数作用: 内部状态机处理函数，根据当前时间驱动状态转换
 *
 * 参数说明:
 *   无
 *
 * 返回值:
 *   无
 * ============================================================
 */
static void NetBackoffProcessFsm(void)
{
    uint64_t nowMs;

    if ((g_NetBackoff.Scene == NetBackoffSceneNone) ||
        (g_NetBackoff.State == NetBackoffStateIdle))
    {
        return;
    }

    /* 若已经驻网，则立即退出退避机制，保持射频开启，并清除 StepIndex */
    if (NetIsRegistered())
    {
        LOG_Debug("[Backoff] Network registered, stop backoff.\n");
        NetBackoffReset();
        return;
    }

    nowMs = NetBackoffGetNowMs();

    switch (g_NetBackoff.State)
    {
        case NetBackoffStateOpenMonitor:
            /* 超过 1 分钟监测窗口仍未驻网，则关射频进入退避并设置下一次定时唤醒 */
            if (nowMs >= g_NetBackoff.NextActionTimeMs)
            {
                NetBackoffStartOffPhase();
            }
            break;

        case NetBackoffStateRfOff:
            /*
             * 退避阶段仅由 PM 定时唤醒驱动：
             *   - 定时唤醒后，在 PM 回调中调用 NetBackoffStartOpenMonitorPhase()
             *     进入 1 分钟网络检测；
             *   - 此处不再根据 NextActionTimeMs 自动切换状态，
             *     避免与 PM 定时唤醒重复导致“多跑一轮监测”。
             */
            break;

        default:
            /* 防御性分支 */
            break;
    }
}

/* ========================= 对外接口实现 ========================= */

/**
 * ============================================================
 * 函数名称: NetBackoffInit
 * 函数作用: 射频退避模块初始化
 *
 * 参数说明:
 *   无
 *
 * 返回值:
 *   无
 * ============================================================
 */
void NetBackoffInit(void)
{
    NetBackoffReset();
}

/**
 * ============================================================
 * 函数名称: NetBackoffOnPmStateChange
 * 函数作用: 外部 PM 回调在状态变化时调用，通知退避模块
 *
 * 参数说明:
 *   pmState - [IN] 最新的 PM 状态
 *
 * 返回值:
 *   无
 *
 * 注意事项:
 *   - 该接口应在统一的 PM 回调（例如 NetPmChangeCallback）
 *     中调用，用于告知本模块最新的 PM 状态。
 * ============================================================
 */
void NetBackoffOnPmStateChange(MPUPmStateChange_t pmState)
{
    MPUPmStateChange_t pmStateCopy;

    (void)memset(&pmStateCopy, 0, sizeof(MPUPmStateChange_t));
    memcpy(&pmStateCopy, &pmState, sizeof(MPUPmStateChange_t));
    NetBackoffHandlePmStateChange(&pmStateCopy);
}

/**
 * ============================================================
 * 函数名称: NetBackoffProcess
 * 函数作用: 射频退避模块周期处理函数
 *
 * 参数说明:
 *   无
 *
 * 返回值:
 *   无
 *
 * 注意事项:
 *   - 建议在网络主线程中以固定周期调用，例如每 1 秒调用一次。
 *   - 函数内部仅负责根据当前时间驱动退避状态机运行；
 *   - PM 状态变化需由外部 PM 回调通过 NetBackoffOnPmStateChange()
 *     通知本模块。
 * ============================================================
 */
void NetBackoffProcess(void)
{
    NetBackoffProcessFsm();
}

/**
 * ============================================================
 * 函数名称: NetBackoffForceStop
 * 函数作用: 外部强制停止退避机制（可选接口）
 *
 * 参数说明:
 *   无
 *
 * 返回值:
 *   无
 *
 * 注意事项:
 *   - 如果在某些场景下你希望立即退出退避，例如用户手工操作
 *     需要强制联网，可以调用本接口。
 * ============================================================
 */
void NetBackoffForceStop(void)
{
    NetBackoffReset();
}
