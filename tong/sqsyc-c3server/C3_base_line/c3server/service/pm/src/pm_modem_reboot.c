#ifndef _GNU_SOURCE
#define _GNU_SOURCE  /* CLOCK_BOOTTIME */
#endif

#include <pthread.h>
#include <time.h>
#include <errno.h>

#include "pm_modem.h"
#include "pm_timer.h"
#include "pm_server_api.h"
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_dms.h"
#include "rpc_property_api.h"
#include "pm_modem_reboot.h"

/**
 * @brief 尝试获取本休眠唤醒周期的非强制重启额度
 *
 * @return int32_t 0 获得额度，else 本周期已重启过，其他负值 计数写入失败
 */
static int32_t PmModemRebootCntAcquire(void)
{
    int32_t ret = 0;
    uint8_t rebootCnt = 0;

    do
    {
        rebootCnt = RpcPropertyGetUint8(PM_REBOOT_CNT_PROP_KEY, 0);
        if(rebootCnt > 0)
        {
            LOG_Warn("non-forced reboot rejected, already rebooted in this sleep-wake cycle");
            ret = -1;
            break;
        }

        /* 写入失败则不放行，防止property异常时失去防循环保护 */
        ret = RpcPropertySetUint8(PM_REBOOT_CNT_PROP_KEY, (uint8_t)(rebootCnt + 1));
        if(ret != 0)
        {
            LOG_Error("failed to persist reboot count, ret=%d", ret);
        }
    } while(0);

    return ret;
}

/**
 * @brief 清除非强制重启计数，系统进入休眠时调用，开启新的休眠唤醒周期
 */
void PmModemRebootCountClear(void)
{
    if(RpcPropertySetUint8(PM_REBOOT_CNT_PROP_KEY, 0) != 0)
    {
        LOG_Error("failed to clear reboot count");
    }
}

/**
 * @brief 检测自重启条件1：T-BOX启动时长达到配置阈值未重启（挂钟时长，含休眠）
 *
 * @return int8_t 1 满足，0 不满足，RET_ERR 检测失败
 */
static int8_t PmModemRebootCondContCheck(void)
{
    int8_t ret = 0;
    uint8_t curWakeupSrc = PM_WAKEUP_SOURCE_INVALID;
    uint64_t thresholdSec = 0;
    struct timespec bootTime = {0};
    PmUpTimeAutoRebootCfg_t upTimeCfg = {0};

    GetPmUpTimeAutoRebootConfig(&upTimeCfg);

    do
    {
        /* durationMin为0: 不开启该重启机制 */
        if(upTimeCfg.durationMin == 0U)
        {
            break;
        }

        /* 唤醒原因限制: 配置非0且与当前唤醒源不一致时不重启 */
        curWakeupSrc = PmSchedGetWakeupSrc();
        if((upTimeCfg.wakeUpReason != PM_AUTO_REBOOT_WAKE_REASON_NO_LIMIT) && (curWakeupSrc != upTimeCfg.wakeUpReason))
        {
            LOG_Warn("[%s] config wakeUpReason = %d, current wakeUpReason = %d", __func__,  upTimeCfg.wakeUpReason , curWakeupSrc);
            break;
        }

        /* CLOCK_BOOTTIME：自上次重启的挂钟时长（含休眠） */
        if(clock_gettime(CLOCK_BOOTTIME, &bootTime) != 0)
        {
            LOG_Error("[%s] failed to get boottime clock, errno=%d", errno, __func__, errno);
            ret = RET_ERR;
            break;
        }

        thresholdSec = (uint64_t)upTimeCfg.durationMin * 60U;
        if((uint64_t)bootTime.tv_sec >= thresholdSec)
        {
            LOG_Warn("[%s] reboot condition met: %llu seconds since last reboot (>= %llus)",
                     __func__, (unsigned long long)bootTime.tv_sec, (unsigned long long)thresholdSec);
            ret = 1;
        }
    } while(0);

    return ret;
}

/**
 * @brief 检测自重启条件2：T-BOX累计工作时长达到配置阈值未重启（运行时长，休眠期间停走）
 *
 * @return int8_t 1 满足，0 不满足，RET_ERR 检测失败
 */
static int8_t PmModemRebootCondAccCheck(void)
{
    int8_t ret = 0;
    uint8_t curWakeupSrc = PM_WAKEUP_SOURCE_INVALID;
    uint64_t thresholdSec = 0;
    struct timespec monoTime = {0};
    PmWorkTimeAutoRebootCfg_t workTimeCfg = {0};

    GetPmWorkTimeAutoRebootConfig(&workTimeCfg);

    do
    {
        /* durationMin为0: 不开启该重启机制 */
        if(workTimeCfg.durationMin == 0U)
        {
            break;
        }

        /* 唤醒原因限制: 配置非0且与当前唤醒源不一致时不重启 */
        curWakeupSrc = PmSchedGetWakeupSrc();
        if((workTimeCfg.wakeUpReason != PM_AUTO_REBOOT_WAKE_REASON_NO_LIMIT) && (curWakeupSrc != workTimeCfg.wakeUpReason))
        {
            LOG_Warn("[%s] config wakeUpReason = %d, current wakeUpReason = %d", __func__,  workTimeCfg.wakeUpReason , curWakeupSrc);
            break;
        }

        /* CLOCK_MONOTONIC：自上次重启的运行时长（休眠期间停走） */
        if(clock_gettime(CLOCK_MONOTONIC, &monoTime) != 0)
        {
            LOG_Error("[%s] failed to get monotonic clock, errno=%d", __func__, errno);
            ret = RET_ERR;
            break;
        }

        thresholdSec = (uint64_t)workTimeCfg.durationMin * 60U;
        if((uint64_t)monoTime.tv_sec >= thresholdSec)
        {
            LOG_Warn("[%s] reboot condition met: %llu seconds accumulated work time (>= %llus)",
                      __func__, (unsigned long long)monoTime.tv_sec, (unsigned long long)thresholdSec);
            ret = 1;
        }
    } while(0);

    return ret;
}

/**
 * @brief 检测是否满足自重启条件，满足任一条件即可在下次休眠前执行重启
 *
 * @return int8_t 1 满足自重启条件，0 不满足，RET_ERR 检测失败
 */
int8_t PmModemRebootCondCheck(void)
{
    if((PmModemRebootCondContCheck() == 1) || (PmModemRebootCondAccCheck() == 1))
    {
        LOG_Warn("meet reboot condition, tbox will reboot after going to sleep ...");
        return 1;
    }

    return 0;
}

/**
 * @brief 自重启功能初始化：按配置创建启动时长达到阈值后触发的一次性定时器（CLOCK_BOOTTIME_ALARM），
 *        到点时若系统处于休眠状态则唤醒重启，工作状态则不处理；
 *        配置未开启该机制(durationMin=0)或未开启Rtc唤醒定时器(rtcWakeUpTrigger=0)时不创建
 *
 * @return int8_t RET_OK 成功，RET_ERR 失败
 */
int8_t PmModemAutoRebootInit(void)
{
    struct timespec bootTime = {0};
    PmUpTimeAutoRebootCfg_t cfg = {0};
    uint64_t thresholdSec = 0;
    int64_t remainSec = 0;
    int8_t ret = RET_OK;

    GetPmUpTimeAutoRebootConfig(&cfg);

    do
    {
        /* durationMin为0: 不开启该重启机制，无需创建唤醒定时器 */
        if(cfg.durationMin == 0U)
        {
            LOG_Info("auto reboot alarm skip: uptime auto reboot disabled (durationMin=0)");
            break;
        }

        /* rtcWakeUpTrigger为0: 启动时长满足条件后不主动唤醒 */
        if(cfg.rtcWakeUpTrigger == 0U)
        {
            LOG_Info("auto reboot alarm skip: rtcWakeUpTrigger disabled");
            break;
        }

        if(clock_gettime(CLOCK_BOOTTIME, &bootTime) != 0)
        {
            LOG_Error("failed to get boottime clock, errno=%d", errno);
            ret = RET_ERR;
            break;
        }

        thresholdSec = (uint64_t)cfg.durationMin * 60U;

        /* 阈值锚定开机时刻，pmServer重启后按剩余时间折算，不会重新计时 */
        remainSec = (int64_t)thresholdSec - (int64_t)bootTime.tv_sec;
        if(remainSec <= 0)
        {
            /* 开机已超阈值，当前为工作状态，按需求本次不处理 */
            LOG_Warn("auto reboot timer skip: %lld seconds since boot (>= %llus)", (long long)bootTime.tv_sec,
                     (unsigned long long)thresholdSec);
            break;
        }

        ret = CreatePmSleepAlarmByBootTime(CPU_REBOOT_SLEEPALARM_EVENT_ID, (uint32_t)remainSec, CPU_REBOOT_SLEEPALARM_EVENT_ID);
        if(ret != RET_OK)
        {
            LOG_Error("create auto reboot alarm failed, ret=%d", ret);
            break;
        }

        LOG_Info("auto reboot alarm created, will trigger in %lld seconds", (long long)remainSec);
    } while(0);

    return ret;
}

/**
 * @brief 请求系统重启
 *
 * @param eventName 重启事件名称，描述重启原因的字符串，可为NULL
 * @param forceFlag 强制重启标志，0=非强制（一次休眠唤醒周期内只能重启一次），非0=强制（不受限）
 * @return int32_t 重启接口调用结果，取值：0 成功（通常无返回，系统直接重启），
 *         else 本周期已重启过被拒绝，其他负值 失败
 */
int32_t PmModemReboot(const char *eventName, uint8_t forceFlag)
{
    int32_t ret = 0;

    LOG_Warn("%s eventName: %s, forceFlag: %d", __FUNCTION__, (eventName != NULL) ? eventName : "NULL", forceFlag);

    do{
        if(forceFlag == 0)
        {
            if ((RET_OK == PmSleepEventIsExist(PM_SLEEP_EVENT_UPGRADE)) || (RET_OK == PmSleepEventIsExist(PM_SLEEP_EVENT_ECALL)))
            {
                LOG_Warn("%s cpu is updating or ecalling", __FUNCTION__);
                ret = -1;
                break;
            }

            ret = PmModemRebootCntAcquire();
            if(ret != 0)
            {
                break;
            }
        }

        ret = HalModemReboot(eventName);

    }while(0);

    LOG_Warn("%s ret: %d", __FUNCTION__, ret);

    return ret;
}
