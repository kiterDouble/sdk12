/**
 * @file param_rpc_handle.h
 * @author jiajiangze (jiangze.jia@sirun.net)
 * @brief sdk接口server端实现
 * @version 0.1
 * @date 2023-12-27
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __PARAM_RPC_HANDLE_H__
#define __PARAM_RPC_HANDLE_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <glib.h>
#include "stdint.h"
#include "sr_param.h"
#include "mcu_api.h"

/**
 * @brief paramServer RPC 回调Server
 * 
 * @param subcmd 接口ID
 * @param reqData 请求数据
 * @param reqLen 请求数据长度
 * @param respData 回复数据
 * @param respLen 回复数据长度
 * @param userdata 用户指针
 */
void paramServerCallBack(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);
#ifdef __cplusplus
}
#endif
#endif