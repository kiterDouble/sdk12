#ifndef PROPERTY_RPC_HANDLER_H
#define PROPERTY_RPC_HANDLER_H

#include <stdint.h>
#include "rpc_property_api.h"

// 处理函数类型定义
typedef void (*PropertyProcCb)(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata);

// 函数表结构体
typedef struct
{
    PropertySubcmd_e sub;
    PropertyProcCb procCb;
} PropertySubProc_t;

/**
 * @brief Property RPC 回调函数
 *
 * @param subcmd     子命令码
 * @param reqData    请求数据
 * @param reqLen     请求数据长度
 * @param respData   响应数据缓冲区
 * @param respLen    响应数据长度（输出）
 * @param userdata   用户数据
 */
void propertyRpcCallback(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata);

#endif // PROPERTY_RPC_HANDLER_H
