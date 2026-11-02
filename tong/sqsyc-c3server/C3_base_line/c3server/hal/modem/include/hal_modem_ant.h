#ifndef __HAL_MODEM_ANT_H__
#define __HAL_MODEM_ANT_H__
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum
{   
    E_HAL_MODEM_ANT_STATUS_CIRCUIT_NORMAL = 0x00,//天线正常
    E_HAL_MODEM_ANT_STATUS_CIRCUIT_SHORT,//天线短路
    E_HAL_MODEM_ANT_STATUS_CIRCUIT_OPEN//天线开路
}HalModemAntStatus_e;

typedef enum
{
    E_HAL_MODEM_ANT_INTERNAL = 0x0,//内置天线
    E_HAL_MODEM_ANT_EXTERNEL//外部天线
}HalModemAntType_e;

/**
 * @name HalModemAntInit
 * @brief 模组天线金接口初始化
 *        本函数用于初始化模组的天线部分。在初始化过程中，会首先检查天线是否已经初始化过，
 *        如果已经初始化，则直接返回成功；如果未初始化，则尝试初始化GPIO引脚来控制天线电源，
 *        并标记为已初始化。
 * @param null
 * @return 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
 */
int32_t HalModemAntInit(void);

/**
 * @name HalModemAntDeinit
 * @brief Modem天线反初始化
 *        该函数用于反初始化Modem的天线部分。它首先检查天线是否已经初始化，如果未初始化则直接返回错误。
 * @param null
 * @return 返回操作结果，如果成功则返回RET_OK，如果失败则返回RET_ERR。
 */
int32_t HalModemAntDeinit(void);

/**
 * @name HalModemAntGetAdc
 * @brief 获取模天线（ADC）的值
 *        本函数用于通过ADC0获取当前的模拟输入值。
 * @param adcVal 指向一个整数的指针，用于接收ADC转换后的数字值。
 * @return 如果函数执行成功，返回RET_OK；如果参数无效，返回RET_ERR。
 */
int32_t HalModemAntGetAdc(int32_t *adcVal);
/**
 * @name HalModemSecAntGetAdc
 * @brief 获取模天线（ADC）的值
 *        本函数用于通过ADC2获取当前的模拟输入值。
 * @param adcVal 指向一个整数的指针，用于接收ADC转换后的数字值。
 * @return 如果函数执行成功，返回RET_OK；如果参数无效，返回RET_ERR。
 */
int32_t HalModemSecAntGetAdc(int32_t *adcVal);
/**
 * @name HalModemAntGetCurrent
 * @brief 获取当前Modem当前使用的天线
 *        本函数用于查询Modem当前所使用的天线是内部天线还是外部天线。
 * @param ant 指向HalModemAntType_e类型的指针，函数将通过这个指针返回当前天线的使用状态。
 *            可能的取值为E_HAL_MODEM_ANT_INTERNAL（内部天线）或E_HAL_MODEM_ANT_EXTERNEL（外部天线）。
 * @return 返回函数执行状态。RET_OK表示成功，RET_ERR表示错误。
 */
int32_t HalModemAntGetCurrent(HalModemAntType_e *ant);

/**
 * @name HalModemAntChange
 * @brief 天线切换
 *        该函数用于根据输入参数选择内部或外部天线，初始化后才能调用此函数。
 * @param ant 天线类型枚举，指定使用内部还是外部天线
 * @return int32_t 返回操作结果，成功返回RET_OK，失败返回RET_ERR
 */
int32_t HalModemAntChange(HalModemAntType_e ant);
#ifdef __cplusplus
}
#endif
#endif
