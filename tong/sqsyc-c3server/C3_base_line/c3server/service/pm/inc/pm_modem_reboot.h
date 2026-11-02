#ifndef _PM_MODEM_REBOOT_H_
#define _PM_MODEM_REBOOT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 非强制重启计数属性key：0=本周期可重启，>0=本周期已重启次数（persist.前缀持久化，重启后仍保留） */
#define PM_REBOOT_CNT_PROP_KEY  "persist.tbox.reboot.count"

/* 自动重启配置文件路径: 优先使用 /oemdata 下的配置, 不存在则使用 /oemapp 默认配置 */
#define PM_AUTO_REBOOT_CFG_PATH         "/oemapp/config/pm_auto_reboot.ini"
#define PM_AUTO_REBOOT_CFG_PATH_PRIMARY "/oemdata/config/pm_auto_reboot.ini"

/* 缺省配置: 全部置0, 即默认不开启自动重启机制, 由 pm_auto_reboot.ini 配置开启 */
#define DEFAULT_UP_TIME_DURATION_MIN    (0U)
#define DEFAULT_WORK_TIME_DURATION_MIN  (0U)
#define DEFAULT_RTC_WAKE_UP_TRIGGER     (0U)
#define PM_AUTO_REBOOT_WAKE_REASON_NO_LIMIT   (0) /* 唤醒原因不限制取值 */

/* 启动时长自动重启配置(UpTimeAutoReboot) */
typedef struct {
    uint32_t durationMin;       /* 自动重启时长阀值(单位:分钟), 0=不开启该重启机制 */
    uint8_t  wakeUpReason;      /* 唤醒原因限制(PM_WAKEUP_SOURCE_*), 0=不做限制 */
    uint8_t  rtcWakeUpTrigger;  /* 是否开启Rtc唤醒定时器, 启动时长满足条件后唤醒 */
} PmUpTimeAutoRebootCfg_t;

/* 连续工作时长自动重启配置(WorkTimeAutoReboot) */
typedef struct {
    uint32_t durationMin;       /* 自动重启时长阀值(单位:分钟), 0=不开启该重启机制 */
    uint8_t  wakeUpReason;      /* 唤醒原因限制(PM_WAKEUP_SOURCE_*), 0=不做限制 */
} PmWorkTimeAutoRebootCfg_t;

/**
 * @brief  加载自动重启配置文件(pm_auto_reboot.ini)
 *         配置文件不存在或解析失败时保持缺省配置
 * @return int8_t  RET_ERR表示失败，RET_OK表示成功
 */
int8_t LoadPmAutoRebootConfig(void);

/**
 * @brief  获取启动时长自动重启配置
 * @param  cfg  输出配置结构体
 * @return void
 */
void GetPmUpTimeAutoRebootConfig(PmUpTimeAutoRebootCfg_t *cfg);

/**
 * @brief  获取连续工作时长自动重启配置
 * @param  cfg  输出配置结构体
 * @return void
 */
void GetPmWorkTimeAutoRebootConfig(PmWorkTimeAutoRebootCfg_t *cfg);


/**
 * @brief 请求系统重启
 *
 * @param eventName 重启事件名称，描述重启原因的字符串，可为NULL
 * @param forceFlag 强制重启标志，0=非强制（一次休眠唤醒周期内只能重启一次），非0=强制（不受限）
 * @return int32_t 重启接口调用结果，取值：0 成功（通常无返回，系统直接重启），
 *         else 本周期已重启过被拒绝，其他负值 失败
 */
int32_t PmModemReboot(const char *eventName, uint8_t forceFlag);

/**
 * @brief 清除非强制重启计数，系统进入休眠时调用，开启新的休眠唤醒周期
 */
void PmModemRebootCountClear(void);

/**
 * @brief 检测是否满足自重启条件，满足任一条件即可在下次休眠前执行重启
 *
 * @return int8_t 1 满足自重启条件，0 不满足，RET_ERR 检测失败
 */
int8_t PmModemRebootCondCheck(void);

/**
 * @brief 自重启功能初始化：按配置创建启动时长达到阈值后触发的一次性定时器（CLOCK_BOOTTIME_ALARM），
 *        到点时若系统处于休眠状态则唤醒重启，工作状态则不处理；
 *        配置未开启该机制(durationMin=0)或未开启Rtc唤醒定时器(rtcWakeUpTrigger=0)时不创建
 *
 * @return int8_t RET_OK 成功，RET_ERR 失败
 */
int8_t PmModemAutoRebootInit(void);

#ifdef __cplusplus
}
#endif

#endif /* _PM_MODEM_REBOOT_H_ */
