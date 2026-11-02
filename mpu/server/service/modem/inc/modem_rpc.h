#ifndef __MODEM_RPC_H__
#define __MODEM_RPC_H__
#include <stdint.h>
#include "modem_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @name ModemSrvRpcProcCallback
 * @brief RPC处理回调函数。此函数用于处理来自调制解调器的RPC（远程过程调用）请求，
 *        根据请求的子命令找到对应的处理函数，并执行相应的处理。
 * @param subcmd 子命令标识，用于指定需要执行的具体操作。
 * @param reqData 请求数据，包含了执行操作所需的参数。
 * @param reqLen 请求数据的长度。
 * @param respData 响应数据，用于返回处理结果。
 * @param respLen 响应数据长度的指针，用于返回响应数据的长度。
 * @param userdata 用户自定义数据，可在处理函数中使用。
 * @return 函数无返回值
 */
void ModemSrvRpcProcCallback(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);
#ifdef __cplusplus
}
#endif

#endif