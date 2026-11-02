#ifndef __TESTER_DISCOVERY_H__
#define __TESTER_DISCOVERY_H__

#include <event2/event.h>
#include "doip_tester.h"
#include "message_proc.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef void* TesterDiscoveryHandle_t;

/**
 * @name TesterDisCoveryInit
 * @brief 测试设备发现初始化。
 * @param hdl 指向TesterDiscoveryHandle_t的指针，用于储存内部私有数据。
 * @param base 指向事件基础结构的指针，用于事件驱动处理。
 * @param notifyCb 当在线测试器被发现时的回调函数。
 * @return 成功返回RET_OK，否则返回RET_ERR。
 */
int32_t TesterDisCoveryInit(TesterDiscoveryHandle_t *hdl, 
                        struct event_base *base, 
                        DoipTesterOnlineNotifyCb notifyCb);

/**
 * @name TesterDisCoveryDeinit
 * @brief 测试设备发现反初始化
 * @param hdl 测试设备发现句柄
 * @return 成功返回RET_OK，否则返回RET_ERR。 
 */
int32_t TesterDisCoveryDeinit(TesterDiscoveryHandle_t hdl);
#ifdef __cplusplus
}
#endif
#endif