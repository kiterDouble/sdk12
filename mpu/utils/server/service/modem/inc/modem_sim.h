#ifndef __MODEM_SIM_H__
#define __MODEM_SIM_H__
#include <stdint.h>
#include "modem_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name ModemSrvGetImsiProc
 * @brief 获取IMSI（国际移动用户识别码）的处理函数
 * @param reqData 请求数据的指针，通常为NULL或特定的请求命令
 * @param reqLen 请求数据的长度，对于此函数通常为0
 * @param respData 响应数据的输出缓冲区指针，用于存放返回的IMSI信息
 * @param respLen 响应数据长度的输出指针，用于指定返回的IMSI信息长度
 * @param userdata 用户数据的指针，用于传递上下文信息，例如模组服务的内部状态
 * @return 函数执行状态，RET_OK表示成功，其他值表示失败
 */
int32_t ModemSrvGetImsiProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);

/**
 * @name ModemSrvGetIccidProc
 * @brief 获取ICCID（国际移动客户识别码）的处理函数
 * @param reqData 请求数据的指针，通常为NULL或特定的请求命令
 * @param reqLen 请求数据的长度，对于此函数通常为0
 * @param respData 响应数据的输出缓冲区指针，用于存放返回的ICCID信息
 * @param respLen 响应数据长度的输出指针，用于指定返回的ICCID信息长度
 * @return 函数执行状态，RET_OK表示成功，其他值表示失败
 */
int32_t ModemSrvGetIccidProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);
/**
 * @name ModemSrvGetMsisdnProc
 * @brief 获取MSISDN（移动用户ISDN号码）
 * @param reqData 请求数据指针，用于获取MSISDN的请求信息
 * @param reqLen 请求数据长度，表示请求信息的大小
 * @param respData 响应数据指针，用于存储处理结果及MSISDN信息
 * @param respLen 响应数据长度指针，用于指示响应缓冲区的大小及实际使用情况
 * @param userdata 用户数据指针，指向ModemSrvData_t类型的服务器数据
 * @return int32_t 返回处理结果，RET_OK表示成功，RET_ERR表示失败
 */
int32_t ModemSrvGetMsisdnProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);

#ifdef __cplusplus
}
#endif
#endif