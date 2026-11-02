#ifndef _PM_SERVER_API_H_
#define _PM_SERVER_API_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 工作模式
#define PM_WORK_NORMAL            (1)   // TBOX正常模式
#define PM_WORK_TRANSPORT         (2)   // TBOX运输模式
#define PM_WORK_FACTORY           (3)   // TBOX工厂模式

// 电源管理状态
#define PM_STATE_NORMAL           (1)   // 电源管理正常状态
#define PM_STATE_SLEEP            (2)   // 电源管理休眠状态

// 唤醒源
#define PM_WAKEUP_SOURCE_INVALID  (0)   // 无效唤醒源
#define PM_WAKEUP_SOURCE_CAN1     (1)   // CAN1 唤醒
#define PM_WAKEUP_SOURCE_CAN2     (2)   // CAN2 唤醒
#define PM_WAKEUP_SOURCE_CAN3     (3)   // CAN3 唤醒
#define PM_WAKEUP_SOURCE_CAN4     (4)   // CAN4 唤醒
#define PM_WAKEUP_SOURCE_CAN5     (5)   // CAN5 唤醒
#define PM_WAKEUP_SOURCE_CAN6     (6)   // CAN6 唤醒
#define PM_WAKEUP_SOURCE_CAN7     (7)   // CAN7 唤醒
#define PM_WAKEUP_SOURCE_CAN8     (8)   // CAN8 唤醒
#define PM_WAKEUP_SOURCE_MCU      (9)   // MCU 唤醒
#define PM_WAKEUP_SOURCE_KL15     (10)  // KL15 唤醒
#define PM_WAKEUP_SOURCE_KL30     (11)  // KL30 唤醒
#define PM_WAKEUP_SOURCE_BLE      (12)  // 蓝牙 唤醒
#define PM_WAKEUP_SOURCE_RTC      (13)  // MCU RTC 唤醒
#define PM_WAKEUP_SOURCE_G_SENSOR (14)  // G-Sensor 唤醒
#define PM_WAKEUP_SOURCE_BCALL    (15)  // B-Call按键唤醒
#define PM_WAKEUP_SOURCE_ECALL    (16)  // E-Call按键唤醒
#define PM_WAKEUP_SOURCE_TIMER    (32)  // MPU定时器唤醒
#define PM_WAKEUP_SOURCE_SMS      (33)  // 短信唤醒
#define PM_WAKEUP_SOURCE_RING     (34)  // 电话唤醒
#define PM_WAKEUP_SOURCE_DATA     (35)  // 网络数据唤醒
#define PM_WAKEUP_SOURCE_ETH      (36)  // 以太网唤醒
#define PM_WAKEUP_SOURCE_TIMER_HEARTBEAT   (37)  // 定时上报数据唤醒
#define PM_WAKEUP_SOURCE_TIMER_REBOOT      (38)  // 重启唤醒
#define PM_WAKEUP_SOURCE_TIMER_WEEK        (39)  // 7天统计时间唤醒
#define PM_WAKEUP_SOURCE_OTA               (40)  //来自网络数据的OTA数据
#define PM_WAKEUP_SOURCE_DTC               (41)  //来自网络数据的DTC唤醒
#define PM_WAKEUP_SOURCE_ECALL_INCOME      (42)  // E-Call回拨唤醒
#define PM_WAKEUP_SOURCE_TIMER_MCU         (43)  // MPU定时器唤醒(唤醒mcu)
#define PM_WAKEUP_SOURCE_END               (44)

// CPU 定时器事件 用户在此添加自定义ID
#define CPU_INVALID_EVENT_ID                            (0)   // 无效事件编号
#define CPU_SEND_HEARTBEAT_SLEEPALARM_EVENT_ID          (1)   // 定时上报数事件编号
#define CPU_REBOOT_SLEEPALARM_EVENT_ID                  (2)   // 重启事件编号
#define CPU_WEEK_SLEEPALARM_EVENT_ID                    (3)   // 7天统计时间事件编号
#define CPU_ECALL_T9_TIMER                              (4)   // ecall t9 定时器
#define CPU_ECALL_T10_TIMER                             (5)   // ecall t10 定时器
#define CPU_TIMER_END                                   (9)   //0-9为系统预留定时器不对外使用

/*10-30为app层预留定时器ID，用户可以根据自己的需求进行定义，注意定时器不能重复创建，同一个ID删除后可再次创建*/
#define CPU_APP_TIMER_START                             (10)
#define CPU_APP_TIMER_BATTERY_HEAT                      (11) // 电池加热定时唤醒
#define CPU_APP_TIMER_TCP_KEEPALIVE					    (13) // TCP KeepAlive定时唤醒
#define CPU_APP_TIMER_END                               (30)  

// 睡眠事件
#define PM_SLEEP_EVENT_UPGRADE    "upgrade"   // 升级事件
#define PM_SLEEP_EVENT_ECALL      "ecall"     // ECALL事件

// ECU升级休眠事件（按零件定义）
#define PM_SLEEP_EVENT_UPGRADE_TBOX     "upgrade_tbox_event"     // TBOX升级事件
#define PM_SLEEP_EVENT_UPGRADE_VCU      "upgrade_vcu_event"      // VCU升级事件
#define PM_SLEEP_EVENT_UPGRADE_TMS      "upgrade_tms_event"      // TMS升级事件
#define PM_SLEEP_EVENT_UPGRADE_BMS      "upgrade_bms_event"      // BMS升级事件
#define PM_SLEEP_EVENT_UPGRADE_MCU      "upgrade_mcu_event"      // MCU升级事件
#define PM_SLEEP_EVENT_UPGRADE_MPU      "upgrade_mpu_event"      // MPU升级事件
#define PM_SLEEP_EVENT_UPGRADE_OTHER    "upgrade_other_event"    // 其他零件升级事件

//周期性发布电源事件结构体，SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY 订阅信息
typedef struct
{
	uint8_t WorkMode;
	uint8_t PmState;
	uint8_t WakeupSrc;
	uint8_t SleepAlarmEventId;
} MPUPmStateChange_t;

// 电源管理对外接口
uint8_t PmGetWakeupSrc(void);
int8_t PmCreateSleepAlarm(uint8_t SleepAlarmEventId, uint32_t SleepAlarmSecond, uint16_t WakeupOption);
int8_t PmCreateSleepAlarmEveryDay(uint8_t SleepAlarmEventId, uint8_t SleepAlarmHour, uint8_t SleepAlarmMinute, uint16_t WakeupOption);
int8_t PmDestroySleepAlarm(uint8_t SleepAlarmEventId);
int8_t PmSetSleepEvent(const char *Event);
int8_t PmCleanSleepEvent(const char *Event);
int8_t PmIsSleepEventExists(const char *Event);
int8_t PmSmsSleepSet(uint8_t smsSubWakeType);

//唤醒外设接口，同步数据唤醒源接口
#define WAKE_MODE       (1)  //唤醒设备      PmWakeupSetDevices参数mode阈值
#define POWER_MODE      (2)  //上电设备      PmWakeupSetDevices参数mode阈值
int8_t PmWakeupSetDevices(const char * name,uint8_t mode);
int8_t PmSyncDataWakeSourceAndWakeupSystem(uint8_t WakeupSrc);
int8_t PmSleepSystem(void);

/* 请求系统重启，forceFlag为强制重启标志（0=非强制，一次休眠唤醒周期内只能重启一次；非0=强制） */
int32_t PmReqReboot(const char *eventName, uint8_t forceFlag);




#ifdef __cplusplus
}
#endif

#endif //_PM_SERVER_API_H_

