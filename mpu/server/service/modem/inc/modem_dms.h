#ifndef __MODEM_DMS_H__
#define __MODEM_DMS_H__
#include <stdint.h>
#include "modem_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @name ModemsrvGetImeiProc
 * @brief 获取IMEI号的处理函数
 * @param reqData 请求数据的指针
 * @param reqLen 请求数据的长度
 * @param respData 响应数据的指针
 * @param respLen 响应数据长度的指针
 * @param userdata 用户数据的指针
 * @return int32_t 返回操作结果，RET_OK表示成功，其他值表示失败
 */
int32_t ModemsrvGetImeiProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);

/**
 * @name ModemSrvGetAirplaneModeProc
 * @brief 获取飞行模式状态的处理函数
 * @param reqData 指向请求数据的指针。该数据由调用者提供，用于执行特定操作。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。该函数将在此缓冲区中返回操作的结果。
 * @param respLen 指向响应数据长度的指针。函数会更新此参数以指示返回的响应数据长度。
 * @param userdata 用户数据的指针。通常用于存储与调用此函数的上下文相关的数据。
 * @return 执行结果的错误码。RET_OK 表示成功，其他值表示错误。
 */
int32_t ModemSrvGetAirplaneModeProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);

/**
 * @name ModemSrvSetAirplaneModeProc
 * @brief 设置飞行模式的处理函数
 *        该函数用于处理设置飞行模式的请求，通过调用HAL来实现模式的切换，并返回操作结果。
 * @param reqData 指向请求数据的指针。请求数据中应包含要设置的飞行模式。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。响应数据中应包含操作结果。
 * @param respLen 指向响应数据长度的指针。用于返回响应数据的实际长度。
 * @param userdata 用户数据指针，通常用于传递模块内部状态或配置。
 * @return 操作结果，RET_OK表示成功，其他值表示失败。
 */
int32_t ModemSrvSetAirplaneModeProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);

/**
 * @name ModemSrvGetQoosEnableProc
 * @brief 获取Qoos启用状态的处理函数
 * @param reqData 指向请求数据的指针。该数据由调用者提供，用于执行特定操作。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。该函数将在此缓冲区中返回操作的结果。
 * @param respLen 指向响应数据长度的指针。函数会更新此参数以指示返回的响应数据长度。
 * @return 操作结果，RET_OK表示成功，其他值表示失败。
 */
int32_t ModemSrvGetQoosEnableProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);

/**
 * @name ModemSrvSetQoosEnableProc
 * @brief 设置QOOS使能处理函数
 * @param reqData 指向请求数据的指针。该数据由调用者提供，用于执行特定操作。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。该函数将在此缓冲区中返回操作的结果。
 * @param respLen 指向响应数据长度的指针。函数会更新此参数以指示返回的响应数据长度。
 * @return 操作结果，RET_OK表示成功，其他值表示失败。
 */
int32_t ModemSrvSetQoosEnableProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);

/**
 * @name ModemSrvGetQoosCfgProc
 * @brief 获取Qoos配置的处理函数
 * @param reqData 指向请求数据的指针。该数据由调用者提供，用于执行特定操作。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。该函数将在此缓冲区中返回操作的结果。
 * @param respLen 指向响应数据长度的指针。函数会更新此参数以指示返回的响应数据长度。
 * @return 操作结果，RET_OK表示成功，其他值表示失败。
 */
int32_t ModemSrvGetQoosCfgProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);

/**
 * @name ModemSrvSetQoosCfgProc
 * @brief 设置Qoos配置的处理函数
 * @param reqData 指向请求数据的指针。该数据由调用者提供，用于执行特定操作。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。该函数将在此缓冲区中返回操作的结果。
 * @param respLen 指向响应数据长度的指针。函数会更新此参数以指示返回的响应数据长度。
 * @return 操作结果，RET_OK表示成功，其他值表示失败。
 */
int32_t ModemSrvSetQoosCfgProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata);
#ifdef __cplusplus
}
#endif
#endif