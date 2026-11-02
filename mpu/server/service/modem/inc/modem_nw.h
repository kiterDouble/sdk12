#ifndef __MODEM_NW_H__
#define __MODEM_NW_H__
#include <stdint.h>
#include "modem_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name ModemSrvGetExtSignalInfoProc
 * @brief 获取详细信号处理函数
 *        该函数用于从Hal Modem函数获取详细的信号信息，并将这些信息封装在响应数据中返回给调用者。
 * @param reqData 指向请求数据的指针。请求数据应包含请求的类型和参数。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。函数将在这里返回获取的信号信息。
 * @param respLen 指向响应数据长度的指针。函数会更新这个值以表示返回的信号信息的实际长度。
 * @param userdata 用户数据的指针。通常用于传递与调制解调器服务相关的上下文信息。
 * @return 函数执行状态码。RET_OK表示成功，其他值表示错误。
 */
int32_t ModemSrvGetExtSignalInfoProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);

/**
 * @name ModemSrvGetOperatorProc
 * @brief 获取运营商信息处理函数
 *        该函数用于从Hal Modem函数获取运营商信息，并将这些信息封装在响应数据中返回给调用者。
 * @param reqData 指向请求数据的指针。请求数据应包含请求的类型和参数。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。函数将在这里返回获取的运营商信息。
 * @param respLen 指向响应数据长度的指针。函数会更新这个值以表示返回的运营商信息的实际长度。
 * @param userdata 用户数据的指针。通常用于传递与调制解调器服务相关的上下文信息。
 * @return 函数执行状态码。RET_OK表示成功，其他值表示错误。
 */ 
int32_t ModemSrvGetOperatorProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);
#ifdef __cplusplus
}
#endif
#endif