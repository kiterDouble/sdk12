#ifndef HSM_HANDLER_H
#define HSM_HANDLER_H

#include <stdint.h>
#include <openssl/rsa.h>
#include "rpc_hsm_api.h"


// 处理函数类型定义
typedef void (*HsmProcCb)(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata);

// 函数表结构体
typedef struct
{
    HsmSubcmd_e sub;
    HsmProcCb procCb;
} HsmSubProc_t;

/**
 * @brief HSM RPC 回调函数
 *
 * @param subcmd     子命令码
 * @param reqData    请求数据
 * @param reqLen     请求数据长度
 * @param respData   响应数据缓冲区
 * @param respLen    响应数据长度（输出）
 * @param userdata   用户数据
 */
void hsmRpcCallback(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void *userdata);


#endif // HSM_HANDLER_H
