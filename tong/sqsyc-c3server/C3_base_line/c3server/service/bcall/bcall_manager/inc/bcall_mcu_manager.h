#ifndef _BCALL_MCU_MANAGER_H_
#define _BCALL_MCU_MANAGER_H_

#include "bcall_server_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TRIGGER_SUCCESS (1)
#define TRIGGER_FAIL    (0)

/**
* @brief 发送BCALL状态同步
*
* @param bcallStatus EnubcallStatus中的状态类型
* @return int32_t 返回值，如果发送成功则返回RET_OK，否则返回RET_ERR
*/
extern int32_t SendMcuBcallStatusSync(EnuBcallStatus bcallStatus);
/**
* @brief 发送BCALL事件确认
*
* @param ack 确认状态（1：成功，0：失败）
* @return int32_t 返回值，如果发送成功则返回RET_OK，否则返回RET_ERR
*/
extern int32_t SendMcuBcallEventAck(uint8_t ack);
/**
 * @brief MCU服务器的主函数。
 *
 * 该函数初始化了服务器所需的组件，启动了一个消息接收线程，并订阅了MCU的消息队列。
 * 
 *
 * @param pArg 传递的额外参数指针（在本例中未使用）。
 * @return 函数的返回值（在本例中始终为RET_OK）。
 */
extern gpointer McuServer(gpointer pArg);


#ifdef __cplusplus
}
#endif

#endif //_BCALL_MCU_MANAGER_H_



