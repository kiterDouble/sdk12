#ifndef __HAL_MODEM_DMS_H__
#define __HAL_MODEM_DMS_H__
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define HAL_REBOOT_EVENT_NAME_MAX_LEN      (64U)
#define HAL_REBOOT_MAX_ATTEMPTS            (3U)
#define HAL_REBOOT_SYNC_TIMES              (2U)
#define HAL_REBOOT_RETRY_DELAY_US          (200U * 1000U)

typedef enum
{
    E_HAL_MODEM_DMS_AIRPLANE_OFF = 0x0,//正常模式
    E_HAL_MODEM_DMS_AIRPLANE_ON,//飞行模式
    E_HAL_MODEM_DMS_AIRPLANE_UNKOWN,//其他模式或异常
}HalModemDmsMode_e;

typedef enum
{
    E_HAL_MODEM_DMS_QOOS_ENABLE_STATE_OFF = 0, // 禁用QOOS
    E_HAL_MODEM_DMS_QOOS_ENABLE_STATE_ON  = 1, // 启用QOOS
}HalModemDmsQoosEnableStatusType_e;

typedef enum
{
    HAL_REBOOT_OK = 0,
    HAL_REBOOT_ERR_PARAM = -1,
    HAL_REBOOT_ERR_SYS = -2,
    HAL_REBOOT_ERR_NO_PRIV = -3
} HalRebootRet_e;

typedef struct 
{
    int timeSec1;       // 第一轮搜网间隔时间，范围1-225秒
    int timeSec2;       // 第二轮搜网间隔时间，范围1-225秒
    int timeSec3;       // 第三轮搜网间隔时间，范围1-225秒
}HalModemDmsQoosConfig_t;

/**
 * @name HalModemDmsInit
 * @brief 初始化Modem的DMS模块。
 *        该函数负责检查DMS模块是否已经初始化，如果未初始化，则执行初始化过程。
 * @param null
 * @return 返回初始化结果。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDmsInit(void);

/**
 * @name HalModemDmsDeinit
 * @brief Modem DMS服务反初始化。
 * @param null
 * @return 返回初始化结果。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDmsDeinit(void);

/**
 * @name HalModemDmsSetMode
 * @brief modem dms设置飞行模式。
 *        该函数用于根据传入的模式参数设置调制解调器的飞行模式状态，支持开启或关闭飞行模式。
 * @param mode 指定的飞行模式状态，使用HalModemDmsMode_e枚举类型。
 *             E_HAL_MODEM_DMS_AIRPLANE_OFF 表示关闭飞行模式，
 *             E_HAL_MODEM_DMS_AIRPLANE_ON 表示开启飞行模式。
 * @return 返回初始化结果。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDmsSetMode(HalModemDmsMode_e mode);

/**
 * @name HalModemDmsGetMode
 * @brief 获取当前Modem的飞行模式状态。
 * @param mode 指向HalModemDmsMode_e类型的指针，用于存储获取到的飞行模式状态。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDmsGetMode(HalModemDmsMode_e *mode);

/**
 * @name HalModemDmsSetQoosEnableStatus
 * @brief 设置Modem的QoS启用状态
 *        本函数用于设置Modem设备的QooS启用状态。根据输入的启用状态，
 *        对Modem的QoS功能进行开启或关闭。
 * @param enable QoS启用状态类型，采用HalModemDmsQoosEnableStatusType_e枚举类型。
 *               如果为启用状态，则开启QoS功能；如果为禁用状态，则关闭QoS功能。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDmsSetQoosEnableStatus(HalModemDmsQoosEnableStatusType_e enable);

/**
 * @name HalModemDmsGetQoosEnableStatus
 * @brief 获取QoS启用状态。
 * @param enable QoS启用状态类型，采用HalModemDmsQoosEnableStatusType_e枚举类型。
 *               如果为启用状态，则开启QooS功能；如果为禁用状态，则关闭QooS功能。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDmsGetQoosEnableStatus(HalModemDmsQoosEnableStatusType_e *enable);

/**
 * @name HalModemDmsSetQoosConfig
 * @brief 设置Modem的QoS配置
 *        本函数用于设置Modem设备的QooS配置。
 * @param qoosCfg 指向包含QoS配置信息的结构体HalModemDmsQoosConfig_t的指针。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDmsSetQoosConfig(const HalModemDmsQoosConfig_t *qoosCfg);

/**
 * @name HalModemDmsGetQoosConfig
 * @brief 获取Modem的QoS配置信息
 *        本函数用于获取Modem设备的QooS配置信息。
 * @param qoosCfg 指向一个HalModemDmsQoosConfig_t类型的指针，用于接收QooS配置信息。
 * @return 函数返回一个整型值。在当前实现中，总是返回RET_OK，表示操作成功。
 */
int32_t HalModemDmsGetQoosConfig(HalModemDmsQoosConfig_t *qoosCfg);

/**
 * @name HalModemDmsGetImei
 * @brief 获取设备的IMEI号
 * @param imei 指向存储IMEI号的缓冲区的指针。
 * @param len  imei缓冲区的长度。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDmsGetImei(uint8_t *imei, uint8_t len);

int32_t HalModemReboot(const char *eventName);

/**
 * @name HalModemDmsGetCpuOccupancy
 * @brief 获取AP侧CPU占用率
 * @param cpuOccupancy 指向float的指针，用于存储CPU占用率（%）
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDmsGetCpuOccupancy(float *cpuOccupancy);

/**
 * @name HalModemDmsGetCpuTemperature
 * @brief 读取CPU温度，数据来源于 /sys/class/thermal/thermal_zone0/temp
 * @param cpuTemperature 指向float的指针，用于存储CPU温度（°C）
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 *         若读取温度文件失败，cpuTemperature将被设置为0。
 */
int32_t HalModemDmsGetCpuTemperature(float *cpuTemperature);

#ifdef __cplusplus
}
#endif
#endif
