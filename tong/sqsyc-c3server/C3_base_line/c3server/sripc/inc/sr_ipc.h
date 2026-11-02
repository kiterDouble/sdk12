/**
 * @file    sr_ipc.h
 * @brief   进程间通信库对外接口（工业级重构版）
 *
 * 提供两种通信模式：
 *   1. Pub/Sub 消息队列（POSIX System V msgqueue + 共享内存订阅树）
 *   2. RPC 同步调用（每线程独立回复队列，彻底消除共享队列竞争）
 *
 * 线程安全：所有公开接口均为线程安全。
 * 进程安全：共享内存通过 PTHREAD_PROCESS_SHARED + PTHREAD_MUTEX_ROBUST 保护。
 *
 * 重构变更说明（相对于原版）：
 *   - clienttid 由 uint16_t 扩展为 int32_t，消除 tid > 65535 截断 bug
 *   - ipcIsFirstCall() 更名为 IpcIsInitialized()，语义正向化
 *   - DestorySysQueTree 拼写修正为 DestroySysQueTree
 *   - RpcGetMsgSyncEx 响应缓冲区长度参数前置，与 respData 相邻
 */

#ifndef SR_IPC_H_
#define SR_IPC_H_

/*
 * 头文件包含必须在 extern "C" 块外：
 * 若放在块内，C++ 编译时会强制系统头文件中的 C++ 声明具有 C 链接，
 * 导致 redefinition / linkage 错误。
 */
#include "queue_msg.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* =========================================================================
 * 回调函数类型
 * ========================================================================= */

/**
 * @brief Pub/Sub 订阅回调
 *
 * @note  data 指针仅在回调函数返回前有效，回调内不得持有该指针。
 *        data 为 4 字节对齐分配，可安全传入 flatcc verify_as_root。
 *        datalen == 0 时 data 为 NULL。
 *
 * @param[in] data      接收到的序列化字节流（4 字节对齐），可为 NULL
 * @param[in] len       字节流长度，为 0 时 data == NULL
 * @param[in] userdata  调用 SubscriptionQueueMsg() 时传入的用户数据
 */
typedef void (*QueSubCallback)(const uint8_t *data, uint16_t len, void *userdata);

/**
 * @brief RPC 服务端处理回调
 *
 * @note  reqData 指针仅在回调函数返回前有效，回调内不得持有该指针。
 *        reqData 为 4 字节对齐分配，可安全传入 flatcc verify_as_root。
 *        reqDataLen == 0 时 reqData 为 NULL。
 *        respData 由库预分配，最大可写入 SYSQUE_MSG_DATA_MAX_LEN 字节，
 *        写入超限字节将被静默截断。
 *
 * @param[in]  subcmd      子命令
 * @param[in]  reqData     请求序列化字节流（4 字节对齐），可为 NULL
 * @param[in]  reqDataLen  请求字节流长度，为 0 时 reqData == NULL
 * @param[out] respData    响应字节流写入地址（库预分配，容量 SYSQUE_MSG_DATA_MAX_LEN）
 * @param[out] respLen     响应字节流实际写入长度
 * @param[in]  userdata    调用 RpcStartMsgServer() 时传入的用户数据
 */
typedef void (*RpcServerCallback)(uint16_t        subcmd,
                                  const uint8_t  *reqData,
                                  uint16_t        reqDataLen,
                                  uint8_t        *respData,
                                  uint16_t       *respLen,
                                  void           *userdata);

/* =========================================================================
 * 返回值约定（与 err_code.h 兼容）
 * RET_OK / RET_ERR / RET_ERR_NORMAL_TIMEOUT 由 err_code.h 定义。
 * ========================================================================= */

/* =========================================================================
 * Pub/Sub 接口
 * ========================================================================= */

/**
 * @brief 初始化共享内存订阅树
 *
 * StartQueueMsgRcvThread() / StartQueueMsgRcv() 内部已包含此调用。
 * 仅当进程只发布消息、不订阅时才需要单独调用本接口。
 *
 * @return RET_OK   成功
 * @return RET_ERR  失败
 */
int32_t InitSysQueTree(void);

/**
 * @brief 销毁共享内存订阅树
 *
 * @warning 跨进程共享资源，仅应由最后退出的进程（或管理进程）调用。
 *          调用后本进程不得再使用任何 Pub/Sub 接口。
 *
 * @return RET_OK   成功
 * @return RET_ERR  失败
 */
int32_t DestroySysQueTree(void);

/**
 * @brief 查询 IPC 是否已在当前进程中完成初始化
 *
 * @return true   已初始化
 * @return false  尚未初始化
 */
bool IpcIsInitialized(void);

/**
 * @brief 初始化并启动消息接收线程（回调驱动模式）
 *
 * 同一进程中仅第一次调用生效，必须在主线程中调用。
 * 与 StartQueueMsgRcv() 二选一。
 *
 * @param[in] threadName  接收线程名称（最长 15 字节，超出静默截断）
 *
 * @return RET_OK   成功
 * @return RET_ERR  失败（已初始化或内部错误）
 */
int32_t StartQueueMsgRcvThread(const char *threadName);

/**
 * @brief 初始化消息接收（主动轮询模式，不启动接收线程）
 *
 * 同一进程中仅第一次调用生效，必须在主线程中调用。
 * 与 StartQueueMsgRcvThread() 二选一，配合 GetQueueMsgWithType() 使用。
 *
 * @return RET_OK   成功
 * @return RET_ERR  失败（已初始化或内部错误）
 */
int32_t StartQueueMsgRcv(void);

/**
 * @brief 订阅指定类型的消息
 *
 * 使用 StartQueueMsgRcvThread() 时，callbackFuc 在接收线程中被调用；
 * 使用 StartQueueMsgRcv() 时，callbackFuc 参数被忽略（轮询模式通过
 * GetQueueMsgWithType() 主动读取）。
 *
 * @param[in] type         消息类型枚举值
 * @param[in] callbackFuc  回调函数，轮询模式下传 NULL 即可
 * @param[in] userdata     透传给回调的用户数据
 *
 * @return RET_OK   成功
 * @return RET_ERR  未初始化，或内存分配失败，或共享内存锁失败
 */
int32_t SubscriptionQueueMsg(SYSQueMsgType_e type,
                             QueSubCallback  callbackFuc,
                             void           *userdata);

/**
 * @brief 取消订阅指定类型的消息
 *
 * @param[in] type  消息类型枚举值
 *
 * @return RET_OK   成功（含本就未订阅的情况）
 * @return RET_ERR  未初始化，或共享内存锁失败
 */
int32_t UnSubscriptionQueueMsg(SYSQueMsgType_e type);

/**
 * @brief 向所有订阅了指定类型消息的队列广播消息
 *
 * @param[in] type  消息类型枚举值
 * @param[in] data  序列化字节流（不可为 NULL，len > 0 时）
 * @param[in] len   字节流长度，必须满足 0 < len < SYSQUE_MSG_DATA_MAX_LEN
 *
 * @return RET_OK   成功（至少发送到一个订阅者）
 * @return RET_ERR  无订阅者、参数非法或内部错误
 */
int32_t SendMsgToAllQueue(SYSQueMsgType_e type,
                          const void     *data,
                          int32_t         len);

/**
 * @brief 主动接收当前进程队列中指定类型的消息（轮询模式专用）
 *
 * 仅限使用 StartQueueMsgRcv() 初始化后使用，与回调模式互斥。
 *
 * @param[in]  type        消息类型枚举值
 * @param[out] recvData    调用方提供的接收缓冲区（不可为 NULL）
 * @param[in]  recvBufLen  缓冲区容量（字节），应 >= SYSQUE_MSG_DATA_MAX_LEN
 * @param[out] recvLen     实际接收到的数据长度（不可为 NULL）
 * @param[in]  timeout     超时（ms），<=0 为阻塞，>0 为超时轮询
 *
 * @return RET_OK   成功
 * @return RET_ERR  失败或超时（recvData/recvLen 不被写入）
 */
int32_t GetQueueMsgWithType(SYSQueMsgType_e  type,
                            uint8_t         *recvData,
                            uint16_t         recvBufLen,
                            uint16_t        *recvLen,
                            int32_t          timeout);

/* =========================================================================
 * RPC 接口
 * ========================================================================= */

/**
 * @brief 创建 RPC 专用全局请求队列（客户端进程在 main() 中调用一次）
 *
 * 幂等操作：队列已存在时直接返回 RET_OK。
 *
 * @return RET_OK   成功
 * @return RET_ERR  失败
 */
int32_t CreatRpcQueue(void);

/**
 * @brief 启动 RPC 服务端（异步线程，持续监听请求）
 *
 * 内部调用 CreatRpcQueue()，外部无需单独调用。
 * 每个 type 应只启动一次；重复调用会创建多个监听线程，导致消息被重复处理。
 *
 * @param[in] type        服务端类型枚举值（唯一标识该服务）
 * @param[in] callbackFuc 请求处理回调（不可为 NULL）
 * @param[in] poolSize    工作线程数：1=单线程顺序处理，2~4=线程池并发处理，
 *                        传入 0 强制为 1，传入 >4 强制截断为 4
 * @param[in] userdata    透传给回调的用户数据
 *
 * @return RET_OK   成功（服务线程已启动）
 * @return RET_ERR  callbackFuc 为 NULL，或内部资源分配失败
 */
int32_t RpcStartMsgServer(RPCServerType_e   type,
                          RpcServerCallback callbackFuc,
                          uint8_t           poolSize,
                          void             *userdata);

/**
 * @brief RPC 客户端同步调用（兼容接口，响应缓冲区上限为 SYSQUE_MSG_DATA_MAX_LEN）
 *
 * 等价于以 respBufLen = SYSQUE_MSG_DATA_MAX_LEN 调用 RpcGetMsgSyncEx()。
 *
 * @param[in]  type     服务端类型枚举值
 * @param[in]  subcmd   子命令
 * @param[in]  timeOut  超时（ms），<=0 使用默认 5 秒；>0 时最小有效分辨率约 2ms
 * @param[in]  reqData  请求字节流（reqLen==0 时可为 NULL）
 * @param[in]  reqLen   请求字节流长度，不得超过 SYSQUE_MSG_DATA_MAX_LEN
 * @param[out] respData 调用方提供的响应缓冲区（不可为 NULL）
 * @param[out] respLen  实际响应长度（不可为 NULL）
 *
 * @return RET_OK                 成功
 * @return RET_ERR                Server 离线或调用失败（respData/respLen 内容未定义）
 * @return RET_ERR_NORMAL_TIMEOUT 超时（respData/respLen 内容未定义）
 */
int32_t RpcGetMsgSync(RPCServerType_e  type,
                      uint16_t         subcmd,
                      int32_t          timeOut,
                      const uint8_t   *reqData,
                      uint16_t         reqLen,
                      uint8_t         *respData,
                      uint16_t        *respLen);

/**
 * @brief RPC 客户端同步调用（安全扩展接口，支持自定义响应缓冲区长度）
 *
 * 每个调用线程拥有独立的回复队列（以线程 tid 为 key），
 * 彻底消除多线程并发 RPC 时的回复队列竞争。
 * 调用前自动清理线程队列中残留的超时回复，防止脏数据污染。
 *
 * @param[in]  type        服务端类型枚举值
 * @param[in]  subcmd      子命令
 * @param[in]  timeOut     超时（ms），<=0 使用默认 5 秒；>0 时最小有效分辨率约 2ms
 * @param[in]  reqData     请求字节流（reqLen==0 时可为 NULL）
 * @param[in]  reqLen      请求字节流长度，不得超过 SYSQUE_MSG_DATA_MAX_LEN
 * @param[out] respData    调用方提供的响应缓冲区（respBufLen==0 时可为 NULL）
 * @param[in]  respBufLen  响应缓冲区容量（字节）
 * @param[out] respLen     实际响应长度（不可为 NULL）；实际写入字节数 <= respBufLen
 *
 * @return RET_OK                 成功
 * @return RET_ERR                Server 离线或调用失败（respData/respLen 内容未定义）
 * @return RET_ERR_NORMAL_TIMEOUT 超时（respData/respLen 内容未定义）
 */
int32_t RpcGetMsgSyncEx(RPCServerType_e  type,
                        uint16_t         subcmd,
                        int32_t          timeOut,
                        const uint8_t   *reqData,
                        uint16_t         reqLen,
                        uint8_t         *respData,
                        uint16_t         respBufLen,
                        uint16_t        *respLen);

#ifdef __cplusplus
}
#endif

#endif /* SR_IPC_H_ */
