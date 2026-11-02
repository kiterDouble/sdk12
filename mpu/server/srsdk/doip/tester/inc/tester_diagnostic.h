#ifndef __TESTER_DIAGNOSTIC_H__
#define __TESTER_DIAGNOSTIC_H__

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "doip_tester.h"
#include "message_proc.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef void* TesterDiagHandle_t;

/**
 * @name TesterDiagConnect
 * @brief 建立诊断连接
 * @param hdl 指向TesterDiagHandle_t的指针，用于返回诊断会话的句柄
 * @param base 指向event_base的指针，用于事件基础结构
 * @param node 指向DoipTesterNode_t的指针，包含服务器的节点信息
 * @param funs 指向DoipTesterCbs_t的指针，包含回调函数
 * @return  返回函数执行的状态码。成功时返回RET_OK，错误时返回RET_ERR。
 */
int32_t TesterDiagConnect(TesterDiagHandle_t *hdl, 
                        struct event_base *base, 
                        const DoipTesterNode_t *node, 
                        const DoipTesterCbs_t *funs);

/**
 * @name TesterDiagUdsRequest
 * @brief 测试设备发送UDS请求。
 * @param hdl 诊断句柄，用于标识当前的诊断会话。
 * @param targetLa 目标逻辑地址，指定要发送UDS请求的网络节点。
 * @param udsData UDS请求数据，包含具体的诊断请求信息。
 * @param udsDataLen UDS请求数据长度，指定udsData参数中数据的字节长度。
 * @return 返回操作结果，RET_OK表示成功，RET_ERR表示失败。
 */
int32_t TesterDiagUdsRequest(TesterDiagHandle_t hdl, uint32_t targetLa, const uint8_t *udsData, uint32_t udsDataLen);

/**
 * @name TesterDiagDisconnect
 * @brief 断开测试设备诊断连接
 *        该函数用于断开与测试设备的诊断连接，释放相关资源。
 * @param hdl 诊断句柄，指向TesterDiagPriv_t结构体的指针。
 * @return 返回操作结果，RET_OK表示成功，RET_ERR表示失败。
 */
int32_t TesterDiagDisconnect(TesterDiagHandle_t hdl);
#ifdef __cplusplus
}
#endif
#endif