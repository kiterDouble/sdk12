#ifndef __HAL_MODEM_DMS_H__
#define __HAL_MODEM_DMS_H__
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    E_HAL_MODEM_DMS_AIRPLANE_OFF = 0x0,//正常模式
    E_HAL_MODEM_DMS_AIRPLANE_ON,//飞行模式
    E_HAL_MODEM_DMS_AIRPLANE_UNKOWN,//其他模式或异常
}HalModemDmsMode_e;

typedef enum
{
    E_HAL_MODEM_DMS_QOOS_ENABLE_STATE_OFF = 0,//禁用QOOS
    E_HAL_MODEM_DMS_QOOS_ENABLE_STATE_ON_NO_SERVICE = 1,//无服务下开启QOOS
    E_HAL_MODEM_DMS_QOOS_ENABLE_STATE_ON = 2,//无服务和受限服务状态下开启QOOS
}HalModemDmsQoosEnableStatusType_e;

typedef struct 
{
    uint16_t preTime;//第一次搜网时间。范围:1-600:默认值:180:单位:秒。
    uint16_t sleepTime;//搜网间隔时间。范围:1~43200;默认值:30;单位:秒。	
    uint16_t acqTime;//搜网时间。范围:1~600;默认值:20;单位;秒。
    uint8_t	cirCnt;//循环次数。范围:5-255;默认值:10:单位:次。	
    uint8_t	sleepStep;//sleep_time 的调整时间。范围:0~60:默认值:15:单位:秒。	
    uint8_t	acqStep;//acq_time 的调整时间。范围:0~60;默认值;0;单位:秒。
    uint8_t	adjCnt;//调整次数。范围:0-5:默认值:2:单位:次。
    uint8_t	idleInterval;//掉网检查时间间隔。范围:1~60:默认值:1:单位:秒。	
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
#ifdef __cplusplus
}
#endif
#endif
