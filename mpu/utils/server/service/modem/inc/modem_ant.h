#ifndef __MODEM_ANT_H__
#define __MODEM_ANT_H__
#include <stdint.h>
#include "modem_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name ModemSrvAntGetMainStatus
 * @brief 获取模组外部天线状态
 *        本函数用于通过读取天线ADC值来判断天线的状态，包括短路、正常和开路三种状态。
 * @param srvData 指向Modem服务数据结构的指针，包含天线配置信息。
 * @param status 指向HalModemAntStatus_e枚举的指针，用于存储获取的天线状态。
 * @return 返回操作结果，成功返回RET_OK，失败返回RET_ERR。
 */
int32_t ModemSrvAntGetMainStatus(ModemSrvData_t *srvData, HalModemAntStatus_e *status);
/**
 * @name ModemSrvAntGetSecondaryStatus
 * @brief 获取模组外部副天线状态
 *        本函数用于通过读取天线ADC值来判断天线的状态，包括短路、正常和开路三种状态。
 * @param srvData 指向Modem服务数据结构的指针，包含天线配置信息。
 * @param status 指向HalModemAntStatus_e枚举的指针，用于存储获取的天线状态。
 * @return 返回操作结果，成功返回RET_OK，失败返回RET_ERR。
 */
int32_t ModemSrvAntGetSecondaryStatus(ModemSrvData_t *srvData, HalModemAntStatus_e *status);
/**
 * @name ModemSrvAntCheck
 * @brief 检查并调整调制解调器的天线状态
 *        本函数用于定期检查调制解调器的天线电路状态，并根据当前天线类型来决定是否需要切换天线。
 *        如果当前天线电路正常，并且正在使用内部天线，则切换到外部天线。
 *        如果当前天线电路异常，并且正在使用外部天线，则切换到内部天线。
 * @param srvData 指向ModemSrvData_t结构体的指针，包含调制解调器服务的数据。
 * @return 返回函数执行状态，RET_OK表示成功，RET_ERR表示失败。
 */
int32_t ModemSrvAntCheck(ModemSrvData_t *srvData);
#ifdef __cplusplus
}
#endif
#endif
