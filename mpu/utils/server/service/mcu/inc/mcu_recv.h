#ifndef __MCU_RECV_H__
#define __MCU_RECV_H__
#include <stdint.h>
#include "mcu_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @name McuSrvRecvMesgProcess
 * @brief 处理从MCU接收的消息
 * @param srvData 指向McuSrvData_t结构体的指针，包含服务相关数据
 * @return int32_t 返回操作结果，RET_OK表示成功，RET_ERR表示失败
 */
int32_t McuSrvRecvMesgProcess(McuSrvData_t *srvData);
#ifdef __cplusplus
}
#endif
#endif