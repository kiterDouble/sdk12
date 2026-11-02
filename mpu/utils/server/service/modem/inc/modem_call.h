#ifndef __MODEM_CALL_H__
#define __MODEM_CALL_H__
#include <stdint.h>
#include "modem_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name ModemSrvCallStatusHandle
 * @brief 处理Modem通话状态信息
 *        本函数用于处理来自Modem的通话状态信息，将每个通话的状态、ID和电话号码作为消息发送到所有队列。
 * 
 * @param callList 指向HalModemCallInfoList_t结构的指针，包含通话信息的列表。
 *                 该结构体中包含了通话的状态、ID和电话号码等信息。
 * 
 * @return 函数无返回值
 */
void ModemSrvCallStatusHandle(HalModemCallInfoList_t *callList);

/**
 * @name ModemSrvCallInit
 * @brief Modem服务通话功能初始化。
 *        该函数初始化调制解调器的通话功能，它会调用HAL的相应函数来进行底层的初始化操作。
 * @param null
 * @return 返回初始化操作的结果状态码。
 */
int32_t ModemSrvCallInit(void);

/**
 * @name ModemSrvStartCallProc
 * @brief 拨打电话处理函数。
 * @param reqData 指向请求数据的指针，包含要拨打的电话号码。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针，此函数会在此处存储调用结果。
 * @param respLen 指向响应数据长度的指针，函数会更新此参数以指示返回的响应数据长度。
 * @param userdata 用户数据指针，通常用于存储模块的内部状态或配置。
 * @return 执行结果，RET_OK 表示成功，其他值表示失败。
 */
int32_t ModemSrvStartCallProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);

/**
 * @name ModemSrvAcceptCallProc
 * @brief 接听电话处理函数。
 * @param reqData 指向请求数据的指针。该数据应包含要接听的通话的ID。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。该数据应包含操作结果和相关数据长度。
 * @param respLen 指向响应数据长度的指针。
 * @param userdata 用户数据的指针，通常用于传递上下文信息。
 * @return 函数执行状态码。RET_OK表示成功，其他值表示错误。
 */
int32_t ModemSrvAcceptCallProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);

/**
 * @name ModemSrvStopCallProc
 * @brief 挂断电话处理函数
 * @param reqData 指向请求数据的指针。该数据应包含要挂断通话的ID。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。该响应将包含操作的结果。
 * @param respLen 指向响应数据长度的指针。
 * @param userdata 用户数据的指针，通常用于传递给函数内部以供使用。
 * @return 函数执行状态码。RET_OK表示成功，其他值表示错误。
 */
int32_t ModemSrvStopCallProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);

/**
 * @name ModemSrvDeinitProc
 * @brief 反初始化Modem服务进程。
 *        该函数用于执行特定类型的调制解调器服务反初始化操作。它首先验证输入参数的有效性，
 *        然后根据请求的数据类型调用相应的反初始化回调函数，并更新服务状态。
 * @param reqData 指向请求数据的指针。请求数据应包含要执行反初始化的服务类型的标识。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据缓冲区的指针。响应数据将包含操作结果和返回的数据长度。
 * @param respLen 指向响应数据长度的指针。函数执行后，这里将更新为实际的响应数据长度。
 * @param userdata 用户数据指针，通常用于传递服务的上下文信息。
 * @return 操作结果，RET_OK 表示成功，RET_ERR 表示失败。
 */
int32_t ModemSrvDeinitProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);
#ifdef __cplusplus
}
#endif
#endif