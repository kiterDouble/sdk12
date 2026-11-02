#ifndef AVNT_RPC_API_H
#define AVNT_RPC_API_H
#include "appRpcType.h"

/**
 * @brief AVNT 应用的 RPC 子命令枚举
 */
typedef enum
{
    RPC_APP_AVNT_SUBCMD_UNKNOWN_E = 0,          /**< 未知子命令（兜底） */
    RPC_APP_AVNT_SUBCMD_GET_CONNECT_STATUS_E    /**< 获取会话状态 */
} RpcAppAvntSubCmd_e;

/**
 * @brief AVNT 应用的会话状态枚举
 *
 * 用于表示当前 RPC 会话的状态。
 */
typedef enum
{
    RPC_APP_AVNT_CONN_STATE_DISCONNECTED_E   = 0,  /* 已断开连接 */
    RPC_APP_AVNT_CONN_STATE_CONNECTED_E      = 1,  /* 已建立连接 */
    RPC_APP_AVNT_CONN_STATE_AUTHENTICATED_E  = 2   /* 已登录认证成功 */
} RpcAppAvntSessionStatus_e;

/**
 * @brief 获取 TSP 会话状态（强类型版本）
 *
 * @param[out] outStatus  返回会话状态（仅当返回 RET_OK 时才写入）
 * @param[in]  timeoutMs  超时时间（毫秒）
 * @return RET_OK/RET_ERR
 */
int32_t GetAVNTSessionStatus(RpcAppAvntSessionStatus_e *outStatus, uint32_t timeoutMs);
#endif