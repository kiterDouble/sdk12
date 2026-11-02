#ifndef __HAL_MODEM_DIAL_H__
#define __HAL_MODEM_DIAL_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MODEM_APN_MAX (5)
#define MODEM_TEXT_LEN (128)
#define MODEM_IPADDR_LEN (16)

typedef struct
{
    int32_t callId;//call ID
    uint8_t  apnName[MODEM_TEXT_LEN];//APN名称
    uint8_t  userName[MODEM_TEXT_LEN];//用户名称
    uint8_t  userPasswd[MODEM_TEXT_LEN];//用户密码
    //int32_t ipVersion;//ipv4或ipv6 just support ipv4
    int32_t authType;//授权类型
}HalModemApnInfo_t;

typedef struct
{
  HalModemApnInfo_t apnInfo[MODEM_APN_MAX];//apn列表
  int32_t apnNum;//apn列表长度
}HalModemDialParam_t;

typedef struct
{
    uint8_t dialStatus;//数据拨号状态 0 disconnected, 1 connected
    uint8_t name[MODEM_IPADDR_LEN];//网卡名称
    uint8_t ip[MODEM_IPADDR_LEN];//数据网卡ip地址
    uint8_t gateway[MODEM_IPADDR_LEN];//数据网卡网关
    uint8_t priDns[MODEM_IPADDR_LEN];//数据网卡首选DNS
    uint8_t secDns[MODEM_IPADDR_LEN];//数据网卡备用DNS
}HalModemNetInfo_t;

/**
 * @name ModemCallbak
 * @brief 模组APN拨号回调
 * @param callId 输入参数, apn call id
 * @param netInfo 输入参数, 模组拨号APN网络参数
 * @return 函数无返回值
 */
typedef void (*ModemCallbak)(int32_t callId, const HalModemNetInfo_t *netInfo);
/**
 * @name HalModemDialInit
 * @brief Modem拨号初始化。
 *        该函数负责在Modem上初始化拨号设置，如果已经初始化过，则直接返回成功。如果未初始化，则进行初始化流程，
 *        包括重置私有拨号状态、调用底层API进行初始化并设置状态回调。
 * @param null
 * @return 返回操作结果，RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDialInit(void);

/**
 * @name HalModemDialDeinit
 * @brief modem拨号反初始化
 * @param null
 * @return 返回操作结果，RET_OK表示成功，RET_ERR表示失败。 
 */
int32_t HalModemDialDeinit(void);

/**
 * @name HalModemDialSetParam
 * @brief 设置modem拨号参数
 *        本函数用于配置调制解调器的拨号参数，包括APN等。在调用此函数之前，必须先初始化拨号模块。
 * @param dialParam 指向HalModemDialParam_t结构体的指针，包含了拨号所需的参数。
 *                  如果此参数为NULL或者其中的apnNum为0，则视为参数无效。
 * @return 函数执行状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDialSetParam(const HalModemDialParam_t *dialParam);

/**
 * @name HalModemDialSetStatusCallback
 * @brief 设置Modem拨号状态回调函数
 *        本函数用于在Modem初始化完成后，设置一个回调函数，以便于接收Modem拨号的相关状态更新。
 * @param cb 指向回调函数的指针。该回调函数应当由用户实现，用于处理Modem拨号状态的改变。
 * @return 返回操作结果。如果操作成功，返回RET_OK；如果Modem未初始化，则返回RET_ERR。
 */
int32_t HalModemDialSetStatusCallback(ModemCallbak cb);

/**
 * @name HalModemDialStart
 * @brief 启动拨号功能，配置APN参数，并尝试建立数据呼叫。
 *        该函数首先检查拨号功能是否已初始化，是否已设置拨号参数，以及是否已经开始拨号。
 *        如果以上检查失败，则返回错误。如果检查通过，函数将为每个提供的APN配置参数，
 *        尝试创建数据呼叫并启动它。
 * @param null
 * @return 使用自定义错误代码表示成功或失败。
 */
int32_t HalModemDialStart(void);

/**
 * @name HalModemDialStartEx
 * @brief 启动指定路的拨号功能，配置APN参数，并尝试建立数据呼叫。
 *        该函数首先检查拨号功能是否已初始化，是否已设置拨号参数。
 *        如果以上检查失败，则返回错误。如果检查通过，函数将为指定的callId配置参数，
 *        尝试创建数据呼叫并启动它。
 * @param callId 指定要启动拨号的call ID，如果为-1则启动所有路
 * @param interval 指定自动拨号间隔，如果为0则默认20s
 * @return 使用自定义错误代码表示成功或失败。
 */
int32_t HalModemDialStartEx(uint8_t pid, uint8_t autoReConnect);

/**
 * @name HalModemDialStop
 * @brief 停止拨号操作
 *        该函数用于停止当前正在进行的拨号操作。它会终止所有通过HalModemDialStart启动的电话连接。
 * @param null
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDialStop(void);

/**
 * @name HalModemDialStopEx
 * @brief 停止指定路的拨号操作
 *        该函数用于停止指定callId的拨号连接，不检查isStarted状态。
 * @param pid 指定要停止拨号的call ID
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDialStopEx(uint8_t pid);
#ifdef __cplusplus
}
#endif
#endif