#ifndef __HAL_MODEM_NW_H__
#define __HAL_MODEM_NW_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_MODEM_NW_CELL_INFO_NUM (0x20)
typedef enum
{
    E_HAL_MODEM_NW_REG_STATUS_REGISTERED = 0x0,//注网成功
    E_HAL_MODEM_NW_REG_STATUS_UNREGISTERED//未注网
}HalModemNwRegStatus_e;

typedef struct
{
    int32_t sigType; //1:4G 0:2,3G
    int32_t rssi;
    int32_t rsrp;
    int32_t rsrq;
    int32_t sinr;
}HalModemNwLteSigInfo_t;//4G信号数据
typedef struct
{
    int32_t sigType; //1:4G 0:2,3G
    int32_t rssi;
    int32_t ecio;
    int32_t rscp;
}HalModemNwGsmSigInfo_t;//2G信号数据

typedef union
{
    HalModemNwLteSigInfo_t lteSigInfo;
    HalModemNwGsmSigInfo_t gsmSigInfo;
}HalModemNwExtSigInfo_t;


/**
 * @name HalModemNwInit
 * @brief modem网络接口初始化
 *        本函数用于初始化网络模块。如果网络模块已经初始化过，则不再重新初始化，
 *        否则将执行实际的初始化流程。初始化成功返回RET_OK，失败返回RET_ERR。
 * @param null
 * @return 返回初始化结果，RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemNwInit(void);

/**
 * @name HalModemNwDeinit
 * @brief modem网络接口反初始化。
 * @param null
 * @return 返回初始化结果，RET_OK表示成功，RET_ERR表示失败。 
 */
int32_t HalModemNwDeinit(void);

/**
 * @name HalModemNwGetRegStatus
 * @brief 获取Modem的网络注册状态。
 * @param status 指向HalModemNwRegStatus_e类型的指针，用于存储获取的网络注册状态。
 * @return 返回操作的结果状态码。RET_OK表示成功，其他值表示失败。
 */
int32_t HalModemNwGetRegStatus(HalModemNwRegStatus_e *status);


/**
 * @name HalModemNwGetCsq
 * @brief 获取当前网络信号强度。
 * @param csq 指向一个用于存储信号强度结果的 uint8_t 类型变量的指针。
 * @return n 返回操作的结果状态码。RET_OK表示成功，其他值表示失败。
 */ 
int32_t HalModemNwGetCsq(uint8_t *csq);

/**
 * @name HalModemNwGetSignalInfo
 * @brief 获取当前网络信号信息
 *        本函数用于获取当前网络信号的强度和质量信息。根据网络类型（WCDMA或LTE），填充相应的信号信息结构体。
 * @param sigInfo 指向HalModemNwExtSigInfo_t结构体的指针，该结构体用于存储获取到的信号信息。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemNwGetSignalInfo(HalModemNwExtSigInfo_t *sigInfo);

/**
 * @name HalModemNwGetOperatorInfo
 * @brief 获取当前网络运营商信息
 *        本函数用于通过调用底层接口，获取当前模块所注册的网络运营商的信息，包括移动国家代码（MCC）和移动网络代码（MNC）。
 * @param provider 一个指向字符串的指针，该字符串用于存储运营商信息（MCC和MNC的组合）。
 *                 运营商信息将以字符串形式返回，例如"46003"。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemNwGetOperatorInfo(uint8_t *provider);

int32_t HalModemNwGetRadioTechType(int *type);

#ifdef __cplusplus
}
#endif
#endif