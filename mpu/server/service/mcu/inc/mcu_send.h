#ifndef __MCU_SEND_H__
#define __MCU_SEND_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name McuSrvSendMesgCallback
 * @brief 回调函数，用于向MCU发送消息。
 * @param subcmd 子命令码。
 * @param reqData 请求数据的指针。
 * @param reqLen 请求数据的长度。
 * @param respData 响应数据的指针，此函数会填充响应结果。
 * @param respLen 响应数据长度的指针，函数执行完毕后会更新此长度。
 * @param userdata 用户自定义数据，通常用于传递上下文信息。
 */
void McuSrvSendMesgCallback(const uint16_t subcmd, 
                            const uint8_t *const reqData, 
                            const uint16_t reqLen, 
                            uint8_t *respData, 
                            uint16_t *respLen, 
                            void* userdata);
#ifdef __cplusplus
}
#endif
#endif