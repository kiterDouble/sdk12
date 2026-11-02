
#ifndef _SR_IPC_H
#define _SR_IPC_H

// 将所有可能包含C++特性的头文件移到extern "C"块外面
#include "queue_msg.h"
#include <glib.h>
#include <glib-object.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief ipc通信 接收端回调函数类型定义
     *
     * @param data 接收到的字节流指针
     * @param len 接收到的字节流长度
     * @param userdata 用户数据 通过SubscriptionQueueMsg传入
     */
    typedef void (*QueSubCallback)(const uint8_t *data, const uint16_t len, void* userdata);

    /**
     * @brief rpc通信 服务端回调函数类型定义
     *
     * @param subcmd 向服务端请求的消息类型
     * @param reqData 服务端回调函数接收到的请求，请求消息序列化后的字节流指针
     * @param reqLen 服务端回调函数接收到的请求，请求消息序列化后的字节流长度
     * @param respData 服务端回调函数处理完成后，将结果消息序列化后传递给此参数，之后由rpc库负责处理
     * @param respLen 服务端回调函数处理完成后，将结果消息序列化后的消息长度传递给此参数，之后由rpc库负责处理
     * @param userdata 用户参数
     */
    typedef void (*RpcServerCallback)(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);

    /**
     * @brief 初始化shm，在start_queue_msgrcv_thread()函数中会有调用，app仅发布消息而不订阅消息时，需要使用此函数初始化
     *
     * @return RET_OK：初始化成功
     *         RET_ERR：初始化失败
     */
    int32_t InitSysQueTree();

    /**
     * @brief 清理shm
     *
     * @return RET_ERR：失败
     *         RET_OK：成功
     */
    int32_t DestorySysQueTree();

    /**
     * @brief 订阅消息函数
     *
     * @param type 消息类型，这是一个枚举值
     * @param callbackFunc 回调函数指针 仅在StartQueueMsgRcvThread时有效；使用StartQueueMsgRcv时无效（需要手动读取消息队列）
     * @param userdata 用户数据
     *
     * @return RET_ERR：失败
     *         RET_OK：成功
     */
    int32_t SubscriptionQueueMsg(SYSQueMsgType_e type, QueSubCallback callbackFuc, void* userdata);

    /**
     * @brief 取消订阅函数
     *
     * @param type 消息类型，这是一个枚举值
     *
     * @return RET_ERR：失败
     *         RET_OK：成功
     */
    int32_t UnSubscriptionQueueMsg(SYSQueMsgType_e type);

    /**
     * @brief 发布消息函数
     *
     * @param type 消息类型，这是一个枚举值
     * @param data 指向发送的字节流的指针
     * @param len 
     * 
     * @return RET_ERR：失败
     *         RET_OK：成功
     */
    int32_t SendMsgToAllQueue(SYSQueMsgType_e type, const void *data, const int32_t len);

    /**
     * @brief 判断ipc是否初始化过
     * 
     * @return true 在当前进程中还未初始化过ipc
     * @return false 在当前进程中已经初始化过ipc
     */
    bool ipcIsFirstCall(void);

    /**
     * @brief 初始化一系列前置条件以及开启一个线程进行消息订阅接收
     *        同一个进程仅第一次初始化时生效
     *        必须在主线程中进行初始化
     *
     * @param threadName 进行消息接收的线程名
     * 
     * @return RET_ERR：失败
     *         RET_OK：成功
     */
    int32_t StartQueueMsgRcvThread(const gchar *threadName);

    /**
     * @brief 初始化一系列前置条件，不设置消息订阅接收线程，需要使用GetQueueMsgWithType主动接收订阅数据
     *        同一个进程仅第一次初始化时生效
     *        必须在主线程中进行初始化
     *        与StartQueueMsgRcvThread接口二选一
     *
     * @param threadName 进行消息接收的线程名
     * 
     * @return RET_ERR：失败
     *         RET_OK：成功
     */
    int32_t StartQueueMsgRcv(void);

    /**
     * @brief 主动获取当前进程所属的消息队列下订阅过的消息 仅限使用StartQueueMsgRcv初始化时使用此接口
     * 
     * @param type 需要获取消息队列中的数据类型
     * @param recvData 需要用户申请一个buffer由此参数传入，获取消息完毕后会传递给buffer
     * @param recvLen 接收到的数据长度，需要用户申请一个uint16_t变量并将变量指针由此参数传入，获取消息完毕后会更新此变量
     * @param timeout 超时时间 单位ms 小于等于0为无超时时间（阻塞性接收），大于0时作为超时时间。
     * 
     * @return int32_t RET_ERR：失败 其他：成功
     */
    int32_t GetQueueMsgWithType(SYSQueMsgType_e type, uint8_t *recvData, uint16_t *recvLen, int32_t timeout);

    /**
     * @brief rpc 客户端用来初始化rpc的两个队列（在main函数中进行）
     *
     * @return RET_ERR：失败
     *         RET_OK：成功
     */
    int32_t CreatRpcQueue(void);

    /**
     * @brief rpc 客户端 同步进行RPC调用函数
     *
     * @param type 服务端类型，这是一个枚举值
     * @param subcmd 向该服务端请求的消息类型
     * @param timeOut 超时时间 单位ms 小于等于0为无超时时间，大于0时作为超时时间。设置超时时间时，取值需要大于等于2。
     * @param reqData 客户端进行rpc调用时，将参数序列化后的字节流指针
     * @param reqLen 客户端进行rpc调用时，将参数序列化后的字节流长度
     * @param respData 客户端进行rpc调用时，服务端返回的结果序列化后的字节流指针，需要用户申请一个buffer由此参数传入，rpc库接收来自服务端的消息完毕后会传递给buffer
     * @param respLen 客户端进行rpc调用时，服务端返回的结果序列化后的字节流长度，需要用户申请一个uint16_t变量并将变量指针由此参数传入，rpc库接收来自服务端的消息完毕后会更新此变量
     * 
     * @return RET_ERR：失败。返回失败时，接口内不会对respData和respLen进行操作
     *         RET_OK：成功
     *         RET_ERR_NORMAL_TIMEOUT：超时。返回超时时，接口内不会对respData和respLen进行操作
     */
    int32_t RpcGetMsgSync(RPCServerType_e type, const uint16_t subcmd, const int32_t timeOut, const uint8_t *reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen);

    /**
     * @brief rpc 服务端 服务函数
     *
     * @param type 服务端类型，这是一个枚举值
     * @param callbackFuc 客户端进行rpc调用时，服务端用户提供的请求处理函数
     * @param poolSize 处理线程数量。1：单线程处理 2~4：使用线程池进行多线程处理 最大设置为4
     * @param userdata 用户数据数据，通过参数传递给callbackFuc使用
     * 
     * @return RET_ERR：失败
     *         RET_OK：成功
     */
    int32_t RpcStartMsgServer(RPCServerType_e type, RpcServerCallback callbackFuc, uint8_t poolSize, void* userdata);

#ifdef __cplusplus
}
#endif
#endif
