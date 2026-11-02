#ifndef __ENTITY_DIAGNOSTIC_H__
#define __ENTITY_DIAGNOSTIC_H__
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "doip_entity.h"
#include "message_proc.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef void* EntityDiagHandle_t;

/**
 * @name EntityDiagInit
 * @brief 实体诊断初始化。
 * @param hdl 指向实体诊断句柄的指针，初始化完成后将包含模块的私有数据。
 * @param cfg 指向DOIP实体配置的指针，提供必要的配置信息。
 * @param reqCb 请求回调函数，用于处理来自UDS请求的回调。
 * @param base 指向事件基础结构的指针，用于事件驱动的网络通信。
 * @param establisted 指向建立标志的指针，初始化为NULL，成功建立连接后将被设置为非NULL。
 * @return 成功返回RET_OK，否则返回RET_ERR。
 */
int32_t EntityDiagInit(EntityDiagHandle_t *hdl, 
                    const DoipEntityCfg_t *cfg, 
                    UdsRequestCallBack reqCb, 
                    struct event_base *base, 
                    uint8_t *establisted);

/**
 * @name EntityDiagnUdsResponse
 * @brief 实体诊断UDS响应
 * @param hdl 实体诊断句柄
 * @param testerLa 测试者地址，用于标识发出诊断请求的设备
 * @param udsData UDS诊断数据，包含诊断请求的具体内容
 * @param dataLen UDS数据长度，指定udsData参数中数据的字节长度
 * @return 返回执行结果，RET_OK表示成功，RET_ERR表示失败
 */
int32_t EntityDiagnUdsResponse(EntityDiagHandle_t hdl, 
                            uint32_t testerLa, 
                            const uint8_t *udsData, 
                            uint32_t dataLen);

/**
 * @name EntityDiagDeinit
 * @brief 实体诊断反初始化函数。
 * @param hdl 实体诊断句柄
 * @return 返回执行结果，RET_OK表示成功，RET_ERR表示失败 
 */
int32_t EntityDiagDeinit(EntityDiagHandle_t hdl);

#ifdef __cplusplus
}
#endif
#endif