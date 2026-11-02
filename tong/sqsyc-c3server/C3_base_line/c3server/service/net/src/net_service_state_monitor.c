/******************************************************************************
 * @file    net_service_state_monitor.c
 * @brief   netServer 网络状态监控模块
 * @details 本文件实现网络异常检测与自愈逻辑，运行在独立的 NetMonitor 线程中。
 *          主要功能：
 *          1. 周期性检测 modem 注册状态、APN 拨号状态、网络制式、ping 连通性；
 *          2. 当出现驻网失败、拨号失败、非 4G 驻网、ping 不通等异常时，
 *             依次触发 RESET_NET（切飞行模式）或 RESET_TBOX（请求重启 MPU）；
 *          3. 根据配置文件或默认参数控制各检查项的开关与阈值。
 * @note    线程周期为 5 秒，首次运行前等待 60 秒，给系统启动留足时间。
 ******************************************************************************/

#include <stdint.h>
#include <time.h>

#include <glib.h>

#include "sr_log.h"
#include "modem_api.h"
#include "pm_server_api.h"
#include "net_config.h"
#include "net_service.h"
#include "net_bewinner_sim_sw_task.h"
#include "net_shell_utils.h"
#include "common_utils.h"


/** 网络故障处理动作枚举 **/
typedef enum{
    NET_FAULT_HANDLE_NONE,      /**< 无异常，不处理 **/
    NET_FAULT_HANDLE_RESET_NET, /**< 触发 RESET_NET：切飞行模式 **/
    NET_FAULT_HANDLE_RESET_TBOX /**< 触发 RESET_TBOX：请求重启 MPU **/
}NetFaultHander_e;

/**
 * @brief  切入切出飞行模式
 * @param  ctx 网络服务上下文指针
 * @details 执行流程：
 *          1. 若使能北伟 SIM 卡切换，先调用 BeWinnerSimSwitch() 尝试换卡；
 *          2. 下发 airplane on，等待 3 秒；
 *          3. 下发 airplane off，完成一次飞行模式切换。
 *          该操作是 RESET_NET 级别的自愈手段，用于重新搜网、重新注册。
 */
static void AirModeSwitch(NetSrvCtx_t *ctx)
{
    /* 北伟 SIM 卡切换：在切飞行模式前先尝试切换 SIM 卡 */
    if (ctx->userCfg->supportBeWinnerSimCard){
        (void) BeWinnerSimSwitch();
        sleep_clock(2);
    }

    (void) ModemSrvSetAirplaneMode(E_MODEM_SRV_DMS_AIRPLANE_ON);
    sleep_clock(3);
    (void) ModemSrvSetAirplaneMode(E_MODEM_SRV_DMS_AIRPLANE_OFF);
}

/**
 * @brief  检查多路 APN 的拨号状态
 * @param  ctx 网络服务上下文指针
 * @return 任何一路 APN 拨号成功返回 1，全部未拨上返回 0
 * @note   本函数语义为“任一 APN 成功即认为网络可用”，用于：
 *         - LED 状态显示；
 *         - 判断是否需要执行 ping；
 *         - APN 拨号异常检测的入口条件。
 */
static int CheckDialStatus(NetSrvCtx_t *ctx)
{
    for (int i = 0; i < ctx->userCfg->apnNum; i++)
    {
        if (ctx->strNetInfo[i].dialStatus == 1)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief  获取网络制式异常检查当前应使用的周期
 * @param  checkParam 网络异常检查参数指针
 * @param  index      当前已触发切网次数（从 0 开始）
 * @return 当前阶段应使用的检查周期（单位：秒）
 * @details 本函数为“非 4G 切飞行模式”提供递进式周期配置：
 *          - 若 index < periodCount，使用 resetNetPeriod[index]；
 *          - 若 index >= periodCount，使用 resetNetPeriod[periodCount - 1]；
 *          - 若 periodCount == 0，返回默认兜底值 NET_TYPE_CHECK_RESET_PERIOD_SECOND；
 *          - 若 periodCount 超过数组大小，截断到数组最大值，防止越界。
 *
 *          设计意图：首次异常时快速尝试恢复，后续逐步放宽重试间隔，
 *          避免对网络/模块造成持续冲击。
 *
 *          例如 periodCount=3, resetNetPeriod={10,20,30}, resetNetMaxCount=3 表示：
 *          - 第 1 次：非 4G 持续 10s 后切飞行模式；
 *          - 第 2 次：非 4G 持续 20s 后切飞行模式；
 *          - 第 3 次：非 4G 持续 30s 后切飞行模式；
 *          - 超过 resetNetMaxCount 后不再切网。
 */
static uint32_t GetNetTypeCurrentCheckPeriod(NetAnomalyCheckParam_t* checkParam, uint32_t index)
{
    uint32_t maxPeriodCount = sizeof(checkParam->netTypeCheckParam.resetNetPeriod)/sizeof(uint32_t);
    uint32_t periodCount = checkParam->netTypeCheckParam.periodCount;
    uint32_t period = NET_TYPE_CHECK_RESET_NET_PERIOD_DEFAULT;

    do {
        /* periodCount 为 0 表示未配置，使用默认兜底周期 */
        if(periodCount == 0)
        {
            break;
        }

        /* 防止 periodCount 超过数组容量导致越界 */
        if(periodCount > maxPeriodCount)
        {
            periodCount = maxPeriodCount;
        }

        /* 当前索引在有效范围内，取对应阶段周期；否则取最后一档周期 */
        if (index < periodCount)
        {
            period = checkParam->netTypeCheckParam.resetNetPeriod[index];
        }
        else
        {
            period = checkParam->netTypeCheckParam.resetNetPeriod[periodCount - 1];
        }
    }while(0);

    return period;
}

/**
 * @brief  检查网络制式是否为 4G，非 4G 时触发 RESET_NET
 * @param  ctx        网络服务上下文指针
 * @param  checkParam 网络异常检查参数指针
 * @param  cycleDuration 监控周期（单位：秒）
 * @return NET_FAULT_HANDLE_NONE / RESET_NET
 * @details 触发条件与流程：
 *          1. 必须处于注册成功且信号有效（csq != 99）状态；
 *          2. 必须至少有一路 APN 拨号成功；
 *          3. 查询当前网络制式，若 sigType != 1（非 4G）：
 *             - 累计非 4G 持续时间 noReg4GDuration；
 *             - 当持续时间达到当前阶段阈值（由 GetNetTypeCurrentCheckPeriod 决定）
 *               时，触发 RESET_NET，并递增 noReg4GResetNetCount；
 *             - 当 noReg4GResetNetCount >= resetNetMaxCount 时停止切网，
 *               避免无限循环。
 *          4. 一旦回到 4G，清零所有计数器。
 *
 *          阶段阈值支持递进配置：例如首次 10s、第二次 20s、第三次 30s，
 *          实现“先快后慢”的重试策略。
 * @note   noReg4GDuration / noReg4GResetNetCount 为 static 变量，
 *         函数退出后保持状态，直到异常恢复或进程退出。
 */
static NetFaultHander_e CheckNetType(NetSrvCtx_t *ctx, NetAnomalyCheckParam_t* checkParam, uint32_t cycleDuration)
{
    static uint32_t noReg4GDuration = 0;       /* 非 4G 持续时间（秒） */
    static uint32_t noReg4GResetNetCount = 0;  /* 已触发切网次数 */
    ModemSrvExtSigInfo_t modemSrvExtSigInfo = {0};
    NetFaultHander_e netFaultHandler = NET_FAULT_HANDLE_NONE;

    if (E_MODEM_SRV_NW_REG_STATUS_REGISTERED == ctx->modemSrvNotify->dataRegStatus && ctx->modemSrvNotify->csq != 99)
    {
        /** 驻网成功 **/
        if (CheckDialStatus(ctx) == 1)
        {
            /** 拨号成功，查询当前网络制式 **/
            ModemSrvGetExtSignalInfo(&modemSrvExtSigInfo);

            if (modemSrvExtSigInfo.lteSiginfo.sigType != 1)  // 0：2G/3G，1：4G
            {
                noReg4GDuration += cycleDuration;
                LOG_Warn("[%s] : current network mode is not 4G: %d, noReg4GDuration:%u(s)", __FUNCTION__, modemSrvExtSigInfo.lteSiginfo.sigType, (unsigned)noReg4GDuration);
                /** 在 resetNetMaxCount 范围内才继续切网 **/
                if (noReg4GResetNetCount < checkParam->netTypeCheckParam.resetNetMaxCount)
                {
                    if (noReg4GDuration >= GetNetTypeCurrentCheckPeriod(checkParam, noReg4GResetNetCount))
                    {
                        noReg4GResetNetCount++;
                        noReg4GDuration = 0;
                    }
                    netFaultHandler = NET_FAULT_HANDLE_RESET_NET;
                    LOG_Warn("[%s] : network mode not 4G, trigger reset network", __FUNCTION__);
                }
            }
            else
            {
                noReg4GDuration = 0;
                noReg4GResetNetCount = 0;
            }
        }
    }

    return netFaultHandler;
}

/**
 * @brief  检查 APN 拨号状态，拨号失败持续时触发 RESET_NET / RESET_TBOX
 * @param  ctx        网络服务上下文指针
 * @param  checkParam 网络异常检查参数指针
 * @param  cycleDuration 监控周期（单位：秒）
 * @return NET_FAULT_HANDLE_NONE / RESET_NET / RESET_TBOX
 * @details 触发条件与流程：
 *          1. 必须处于注册成功且信号有效状态；
 *          2. 若所有 APN 均未拨号成功（CheckDialStatus == 0），累计拨号失败时间；
 *          3. 当累计时间 >= apnDialFailTriggerResetNetPeriod 时：
 *             - 触发 RESET_NET；
 *             - apnDialResetNetCount 自增；
 *             - 若 apnDialResetNetCount > apnDialFailResetNetTriggerResetTboxCount，
 *               升级为 RESET_TBOX，并重置计数器。
 *          4. 一旦任一路 APN 拨号成功，清零计数器。
 */
static NetFaultHander_e CheckApnDial(NetSrvCtx_t *ctx, NetAnomalyCheckParam_t* checkParam, uint32_t cycleDuration)
{
    static uint32_t apnDialDuration = 0;       /* 拨号失败累计时间（秒） */
    static uint32_t apnDialResetNetCount = 0;  /* 拨号失败已触发切网次数 */
    NetFaultHander_e netFaultHandler = NET_FAULT_HANDLE_NONE;

    if (E_MODEM_SRV_NW_REG_STATUS_REGISTERED == ctx->modemSrvNotify->dataRegStatus && ctx->modemSrvNotify->csq != 99)
    {
        /** 驻网成功 **/
        if (CheckDialStatus(ctx) == 0)
        {
            /** 拨号失败 **/
            LOG_Warn("[%s] : apn dialing failed, checking retry conditions.", __FUNCTION__);
        
            apnDialDuration += cycleDuration;

            /* 拨号失败持续超过阈值，触发切飞行模式 */
            if (apnDialDuration >= checkParam->apnDialCheckParam.apnDialFailTriggerResetNetPeriod) 
            {
                apnDialResetNetCount++;
                apnDialDuration = 0;
                LOG_Warn("[%s] : apn dialing failed for more than %u s, resetNetCount=%u",
                        __FUNCTION__, (unsigned)checkParam->apnDialCheckParam.apnDialFailTriggerResetNetPeriod, (unsigned)apnDialResetNetCount);

                netFaultHandler = NET_FAULT_HANDLE_RESET_NET; /* 切换飞行模式 */
                LOG_Warn("[%s] : apn dialing fault, trigger reset network (%u/%u)",
                        __FUNCTION__, (unsigned)apnDialResetNetCount, (unsigned)checkParam->apnDialCheckParam.apnDialFailResetNetTriggerResetTboxCount);
                
                /* 切网次数超过阈值，升级为重启 TBOX */
                if (apnDialResetNetCount > checkParam->apnDialCheckParam.apnDialFailResetNetTriggerResetTboxCount)
                {
                    apnDialResetNetCount = 0;
                    netFaultHandler = NET_FAULT_HANDLE_RESET_TBOX; /* 重启模块 */
                    LOG_Warn("[%s] : apn dialing fault reset net count exceed %u, trigger reset tbox",
                        __FUNCTION__, (unsigned)checkParam->apnDialCheckParam.apnDialFailResetNetTriggerResetTboxCount);
                }
            }
        }
        else
        {
            apnDialDuration = 0;
            apnDialResetNetCount = 0;
        }
    }

    return netFaultHandler;
}

/**
 * @brief  使用 ping 检查网络实际连通性
 * @param  ctx        网络服务上下文指针
 * @param  checkParam 网络异常检查参数指针
 * @param  cycleDuration 监控周期（单位：秒）
 * @return NET_FAULT_HANDLE_NONE / RESET_NET / RESET_TBOX
 * @details 触发条件与流程：
 *          1. 必须处于注册成功且信号有效状态；
 *          2. 必须至少有一路 APN 拨号成功；
 *          3. 按 pingCycleDurationSecond 周期 ping 8.8.8.8；
 *          4. ping 成功：清零失败计数；
 *          5. ping 失败：递增 pingFailCount；
 *             - 当 pingFailCount > pingFailedCountTriggerResetNet 时，
 *               触发 RESET_NET，清零 pingFailCount，pingFailResetNetCount 自增；
 *             - 当 pingFailResetNetCount > pingFailedResetNetCountTriggerResetTbox 时，
 *               升级为 RESET_TBOX，并重置计数器。
 *          6. 拨号失败期间，将 pingWaitingSecond 设置为 pingCycleDurationSecond - 20，
 *             使得拨号恢复后约 20 秒即可开始 ping，不必再等完整周期。
 */
static NetFaultHander_e CheckNetConnection(NetSrvCtx_t *ctx, NetAnomalyCheckParam_t* checkParam, uint32_t cycleDuration)
{   
    static uint32_t pingFailCount = 0;        /* 连续 ping 失败次数 */
    static uint32_t pingFailResetNetCount = 0; /* ping 失败已触发切网次数 */
    static uint32_t pingWaitingSecond = 0;     /* 距离下一次 ping 的等待时间（秒） */

    NetFaultHander_e netFaultHandler = NET_FAULT_HANDLE_NONE;
    int pingResult = -1;

    if (E_MODEM_SRV_NW_REG_STATUS_REGISTERED == ctx->modemSrvNotify->dataRegStatus && ctx->modemSrvNotify->csq != 99)
    {
        /** 驻网成功 **/
        if (CheckDialStatus(ctx) == 1)
        {
            /** 拨号成功 **/
            pingWaitingSecond += cycleDuration;
            if (pingWaitingSecond >= checkParam->netConnectionCheckParam.pingCycleDurationSecond)
            {
                pingWaitingSecond = 0;
                pingResult = DebugCmd("ping -c 1 -w 1 8.8.8.8");
                if (pingResult == 0)
                {
                    pingFailCount = 0;
                    pingFailResetNetCount = 0;
                    LOG_Warn("[%s] :  PING succeeded,net connect.", __FUNCTION__);
                }
                else
                {
                    LOG_Error("[%s] :  PING failed,net disconnect.", __FUNCTION__);
                    pingFailCount++;
                    LOG_Debug("[%s] : pingFailCount=%u", __FUNCTION__, (unsigned)pingFailCount);

                    if (pingFailCount > checkParam->netConnectionCheckParam.pingFailedCountTriggerResetNet)
                    {
                        LOG_Debug("[%s] : ping failed > %u times, reset netWork ",
                                __FUNCTION__, (unsigned)checkParam->netConnectionCheckParam.pingFailedCountTriggerResetNet);
                        netFaultHandler = NET_FAULT_HANDLE_RESET_NET;
                        pingFailCount = 0;
                        pingFailResetNetCount++;

                        if (pingFailResetNetCount > checkParam->netConnectionCheckParam.pingFailedResetNetCountTriggerResetTbox)
                        {
                            LOG_Debug("[%s] : ping failed ResetNetCount %u times, reset netWork ",
                                    __FUNCTION__, (unsigned)checkParam->netConnectionCheckParam.pingFailedResetNetCountTriggerResetTbox);
                            netFaultHandler = NET_FAULT_HANDLE_RESET_TBOX;
                            pingFailResetNetCount = 0;
                        }
                    }
                }
            }
        }
        else
        {
            /** 拨号失败 **/
            pingWaitingSecond = (checkParam->netConnectionCheckParam.pingCycleDurationSecond > 20) ?
                                (checkParam->netConnectionCheckParam.pingCycleDurationSecond - 20) : 20;
        }
    }

    return netFaultHandler;
}

/**
 * @brief  检查 modem 驻网状态
 * @param  ctx        网络服务上下文指针
 * @param  checkParam 网络异常检查参数指针
 * @param  cycleDuration 监控周期（单位：秒）
 * @return NET_FAULT_HANDLE_NONE / RESET_NET / RESET_TBOX
 * @details 触发条件与流程：
 *          1. 若已注册且信号有效（csq != 99），清零计数器；
 *          2. 若未注册或注册但 csq == 99（无信号），累计 noRegDuration；
 *          3. 当 noRegDuration > regFailTriggerResetNetPeriod 时：
 *             - 触发 RESET_NET；
 *             - noRegResetNetWorkCount 自增；
 *             - 若 noRegResetNetWorkCount > regFailResetNetTriggerResetTboxCount，
 *               升级为 RESET_TBOX，并重置计数器。
 */
static NetFaultHander_e CheckNetRegisteredState(NetSrvCtx_t *ctx,NetAnomalyCheckParam_t* checkParam, uint32_t cycleDuration)
{
    static uint32_t noRegDuration = 0;              /* 驻网失败持续时间(second) */
    static uint32_t noRegResetNetWorkCount = 0;     /* 驻网失败触发重启网络次数 */
    NetFaultHander_e netFaultHandler = NET_FAULT_HANDLE_NONE;

    do{
        if (E_MODEM_SRV_NW_REG_STATUS_REGISTERED == ctx->modemSrvNotify->dataRegStatus && ctx->modemSrvNotify->csq != 99)
        {
            noRegDuration = 0;
            noRegResetNetWorkCount = 0;
            break;
        }
        else if(E_MODEM_SRV_NW_REG_STATUS_UNREGISTERED == ctx->modemSrvNotify->dataRegStatus ||
               (E_MODEM_SRV_NW_REG_STATUS_REGISTERED   == ctx->modemSrvNotify->dataRegStatus && ctx->modemSrvNotify->csq == 99))
        {
            noRegDuration += cycleDuration;          
        }

        /* 驻网失败持续超过阈值，触发切飞行模式 */
        if (noRegDuration > checkParam->netRegCheckParam.regFailTriggerResetNetPeriod)
        {
            noRegDuration = 0;
            noRegResetNetWorkCount++;
            LOG_Warn("[%s] : net unregister/no signal for more than %u s, resetNetCount=%u",
                __FUNCTION__, (unsigned)checkParam->netRegCheckParam.regFailTriggerResetNetPeriod, (unsigned)noRegResetNetWorkCount);

            netFaultHandler = NET_FAULT_HANDLE_RESET_NET;
            LOG_Warn("[%s] : net unregister/no signal, trigger reset network (%u/%u)",
                __FUNCTION__, (unsigned)noRegResetNetWorkCount, (unsigned)checkParam->netRegCheckParam.regFailResetNetTriggerResetTboxCount);

            /* 切网次数超过阈值，升级为重启 TBOX */
            if (noRegResetNetWorkCount > checkParam->netRegCheckParam.regFailResetNetTriggerResetTboxCount)
            {
                noRegResetNetWorkCount = 0;
                netFaultHandler = NET_FAULT_HANDLE_RESET_TBOX;
                LOG_Warn("[%s] : net unregister/no signal reset net count exceed %u, trigger reset tbox",
                    __FUNCTION__, (unsigned)checkParam->netRegCheckParam.regFailResetNetTriggerResetTboxCount);
            }
        }        
    }while(0);

    return netFaultHandler;
}

/**
 * @brief  网络状态监控线程主函数
 * @param  pArg 线程参数，传入 NetSrvCtx_t * 上下文指针
 * @return 理论上不会返回
 * @details 执行流程：
 *          1. 若配置文件未提供网络异常检查参数，填充默认参数；
 *          2. 初始化 LED GPIO（若配置有效）；
 *          3. 等待系统启动 60 秒；
 *          4. 进入死循环，每 5 秒执行一次：
 *             a. 调用 NetBackoffProcess() 处理退避逻辑；
 *             b. 根据拨号状态更新 LED；
 *             c. 睡眠状态下跳过检查；
 *             d. 根据注册/信号状态更新 dialStatus；
 *             e. 按优先级依次调用：
 *                - NetRegisteredStateCheck（驻网异常）
 *                - CheckApnDial（拨号异常）
 *                - CheckNetType（网络制式异常）
 *                - CheckNetworkConnection（ping 连通性异常）
 *             f. 根据返回的 faultHandler 执行 RESET_NET 或 RESET_TBOX；
 *          5. RESET_TBOX 被 pmServer 拒绝时，降级为 RESET_NET。
 * @note   各检查函数内部使用 static 变量维护状态，因此本线程只需周期性调用即可。
 */
gpointer NetMonitor(gpointer pArg)
{
    const uint32_t MonitorCycleDurationSec = 5;
    NetSrvCtx_t *ctx = (NetSrvCtx_t *)pArg;
    NetFaultHander_e netFaultHandler = NET_FAULT_HANDLE_NONE;
    uint8_t dialStatus = 0;     /* 链路是否通畅(ping)*/ 
    uint8_t lastDialStatus = 0xFF;
    NetAnomalyCheckParam_t *checkParam = &ctx->userCfg->netAnomalyCheckParam;

    /** 若配置文件未提供任何网络异常检查 section，使用宏定义默认值 **/
    if (checkParam->netRegCheckEnable      == 0 &&
        checkParam->apnDialCheckEnable     == 0 &&
        checkParam->netTypeCheckEnable     == 0 &&
        checkParam->netConnectionCheckEnable == 0)
    {
        /** 驻网异常 **/
        checkParam->netRegCheckEnable = 1;
        checkParam->netRegCheckParam.regFailTriggerResetNetPeriod = NET_REG_FAIL_TRIGGER_RESET_NET_PERIOD_DEFAULT;
        checkParam->netRegCheckParam.regFailResetNetTriggerResetTboxCount = NET_REG_FAIL_RESET_NET_TRIGGER_RESET_TBOX_COUNT_DEFAULT;

        /** APN 拨号异常 **/
        checkParam->apnDialCheckEnable = 1;
        checkParam->apnDialCheckParam.apnDialFailTriggerResetNetPeriod = NET_APN_DIAL_FAIL_TRIGGER_RESET_NET_PERIOD_DEFAULT;
        checkParam->apnDialCheckParam.apnDialFailResetNetTriggerResetTboxCount = NET_APN_DIAL_FAIL_RESET_NET_TRIGGER_RESET_TBOX_COUNT_DEFAULT;

        /** 网络制式异常（非 4G） **/
        checkParam->netTypeCheckEnable = 1;
        checkParam->netTypeCheckParam.periodCount = 1;
        checkParam->netTypeCheckParam.resetNetPeriod[0] = NET_TYPE_CHECK_RESET_NET_PERIOD_DEFAULT;
        checkParam->netTypeCheckParam.resetNetMaxCount = NET_TYPE_CHECK_RESET_NET_MAX_COUNT_DEFAULT;

        /** 网络连接异常（ping） **/
        checkParam->netConnectionCheckEnable = 1;
        checkParam->netConnectionCheckParam.pingCycleDurationSecond = NET_CONNECTION_PING_CYCLE_DURATION_SECOND_DEFAULT;
        checkParam->netConnectionCheckParam.pingFailedCountTriggerResetNet = NET_CONNECTION_PING_FAILED_COUNT_TRIGGER_RESET_NET_DEFAULT;
        checkParam->netConnectionCheckParam.pingFailedResetNetCountTriggerResetTbox = NET_CONNECTION_PING_FAILED_RESET_NET_COUNT_TRIGGER_RESET_TBOX_DEFAULT;

        LOG_Info("[%s] : net anomaly check param use macro default values.", __FUNCTION__);
    }

    // 初始化LED状态
    if (ctx->userCfg->ledPinNum != 0)
    {
        Ql_GPIO_Init(ctx->userCfg->ledPinNum, 1); // 1 for output direction
    }
    
    //等待系统启动
    sleep_clock(60);
    
    while (1)
    {
        // net backoff process
        NetBackoffProcess();

        /* 根据拨号状态变化更新 LED 电平 */
        if ((ctx->userCfg->ledPinNum != 0) && (dialStatus != lastDialStatus))
        {
            Ql_GPIO_SetLevel(ctx->userCfg->ledPinNum, dialStatus == 1 ? 1 : 0);
            lastDialStatus = dialStatus;
            LOG_Info("[NET LED] changed to %s", dialStatus == 1 ? "ON" : "OFF");
        }

        sleep_clock(MonitorCycleDurationSec);

        /* 睡眠状态不检查 modem 状态 */
        if (PM_STATE_SLEEP == ctx->pmState->PmState)
        {
            dialStatus = 0;
            LOG_Debug("net is sleep,so not check modem status.");
            continue;
        }

        /* 根据注册/信号状态更新 dialStatus，用于 LED 显示 */
        if(E_MODEM_SRV_NW_REG_STATUS_UNREGISTERED == ctx->modemSrvNotify->dataRegStatus || (E_MODEM_SRV_NW_REG_STATUS_REGISTERED == ctx->modemSrvNotify->dataRegStatus && ctx->modemSrvNotify->csq == 99))
        {
            dialStatus = 0;
        }
        else if (E_MODEM_SRV_NW_REG_STATUS_REGISTERED == ctx->modemSrvNotify->dataRegStatus && ctx->modemSrvNotify->csq != 99)
        {
            dialStatus = CheckDialStatus(ctx);
        }

        /* 按优先级依次检查各类网络异常，任一异常触发即跳出 */
        do{
            /** 检查驻网状态是否需要异常处理 **/
            if (ctx->userCfg->netAnomalyCheckParam.netRegCheckEnable)
            {
                netFaultHandler = CheckNetRegisteredState(ctx, &ctx->userCfg->netAnomalyCheckParam, MonitorCycleDurationSec);
                if (netFaultHandler != NET_FAULT_HANDLE_NONE){
                    break;
                }
            }

            /** 检查APN拨号需要异常处理 **/
            if (ctx->userCfg->netAnomalyCheckParam.apnDialCheckEnable)
            {
                netFaultHandler = CheckApnDial(ctx, &ctx->userCfg->netAnomalyCheckParam, MonitorCycleDurationSec);
                if (netFaultHandler != NET_FAULT_HANDLE_NONE){
                    break;
                }
            }

            /** 检查驻网网络制式是否需要异常处理 **/
            if (ctx->userCfg->netAnomalyCheckParam.netTypeCheckEnable)
            {
                netFaultHandler = CheckNetType(ctx, &ctx->userCfg->netAnomalyCheckParam, MonitorCycleDurationSec);
                if (netFaultHandler != NET_FAULT_HANDLE_NONE){
                    break;
                }
            }

            /** 直接使用ping 检查网络是否通畅 **/
            if (ctx->userCfg->netAnomalyCheckParam.netConnectionCheckEnable)
            {
                netFaultHandler = CheckNetConnection(ctx, &ctx->userCfg->netAnomalyCheckParam, MonitorCycleDurationSec);
                if (netFaultHandler != NET_FAULT_HANDLE_NONE){
                    break;
                }
            }
        }while(0);

        /* 根据检查结果执行对应故障处理 */
        switch(netFaultHandler){
            case NET_FAULT_HANDLE_NONE:
                break;
            case NET_FAULT_HANDLE_RESET_NET:
                LOG_Warn("reset network");
                AirModeSwitch(ctx);
                break;
            case NET_FAULT_HANDLE_RESET_TBOX:
                /* 通过pmServer检查是否允许在本点火周期内重启MPU */
                LOG_Warn("reboot MPU");
                if (0 != PmReqReboot("NetFault",false))
                {
                    LOG_Warn("reboot MPU skipped: already rebooted in current cycle, reset netWork");
                    AirModeSwitch(ctx);                    
                }           
                break;
            default:
                break;                
        }

    }
}
