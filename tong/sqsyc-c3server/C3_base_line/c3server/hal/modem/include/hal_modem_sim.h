#ifndef __HAL_MODEM_SIM_H__
#define __HAL_MODEM_SIM_H__
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    E_HAL_MODEM_SIM_CARD_STATUS_PRESENT = 0x0,
    E_HAL_MODEM_SIM_CARD_STATUS_ABSENT,
    E_HAL_MODEM_SIM_CARD_STATUS_ERROR,
    E_HAL_MODEM_SIM_CARD_STATUS_BLOCKED,
}HalModemSimCardStatus_e;

typedef enum
{
    E_HAL_MODEM_SIM_APP_STATUS_READY = 0x0,
    E_HAL_MODEM_SIM_APP_STATUS_ERROR,
}HalModemSimAppStatus_e;

/**
 * @name HalModemSimInit
 * @brief 初始化Modem的SIM卡模块。
 * @param null
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。 
 */
int32_t HalModemSimInit(void);

/**
 * @name HalModemSimDeinit
 * @brief 反初始化Modem的SIM卡模块。
 * @param null
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。 
 */
int32_t HalModemSimDeinit(void);

/**
 * @name HalModemSimGetCardStatus
 * @brief 获取SIM卡的状态和应用状态。
 * @param cardStat 指向HalModemSimCardStatus_e类型的指针，用于接收SIM卡状态。
 * @param appStat 指向HalModemSimAppStatus_e类型的指针，用于接收SIM卡应用状态。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemSimGetCardStatus(HalModemSimCardStatus_e *cardStat, HalModemSimAppStatus_e *appStat);

/**
 * @name HalModemSimGetImsi
 * @brief 获取SIM卡的IMSI（国际移动用户识别码）。
 * @param imsi 指向存储IMSI字符串的缓冲区的指针。
 * @param len 指定缓冲区的长度。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemSimGetImsi(uint8_t *imsi, uint8_t len);

/**
 * @name HalModemSimGetIccid
 * @brief 获取SIM卡的ICCID。
 * @param iccid 指向存储ICCID的缓冲区的指针。
 * @param len 指定ICCID缓冲区的长度。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemSimGetIccid(uint8_t *iccid, uint8_t len);

/**
 * @name HalModemSimGetMsisdn
 * @brief 获取MSISDN（移动用户ISDN号码）
 * @param msisdn 指向用于存储MSISDN的缓冲区的指针
 * @param len 缓冲区的长度
 * @return 返回RET_OK表示成功，RET_ERR表示失败
 */
int32_t HalModemSimGetMsisdn(uint8_t *msisdn, uint8_t len);
#ifdef __cplusplus
}
#endif

#endif