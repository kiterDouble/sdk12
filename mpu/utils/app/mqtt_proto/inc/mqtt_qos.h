#ifndef __MQTT_QOS_H__
#define __MQTT_QOS_H__

/**
 * @brief QoS功能初始化
 * 
 * @return int 
 */
int QoS_Init(void);

/**
 * @brief QoS功能反初始化
 * 
 */
void QoS_DeInit(void);

/**
 * @brief 打开QoS功能
 * 
 * @return int 
 */
int QoS_Enable(void);

/**
 * @brief 关闭QoS功能
 * 
 * @return int 
 */
int QoS_Disable(void);

/**
 * @brief 从MCU同步QoS开关状态，如果已打开，需要使能QoS
 * 
 * @return int 
 */
int QoS_CheckSwitch(void);

/**
 * @brief Ping检测线程：周期性检测延迟，写入共享变量
 */
void *QoS_PingDetectThread(void *arg);

/**
 * @brief 带宽调整线程：根据Ping结果动态调整带宽
 */
void *QoS_BwAdjustThread(void *arg);
/**
 * @brief 延迟QoS调整线程（示例：循环检测+调整）
 * @param arg 线程参数（无）
 * @return 线程返回值
 */
void *QoS_LatencyAdjust(void *arg);

#endif  /* __MQTT_QOS_H__ */