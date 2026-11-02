#ifndef ___HAL_MODEM_CALL_H__
#define ___HAL_MODEM_CALL_H__
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define VOICE_NUMBER_MAX (81)
#define VOICE_CALL_INFO_MAX (8)
typedef enum
{
    HAL_MODEM_CALL_STATUS_ACTIVE = 0x0,
    HAL_MODEM_CALL_STATUS_HOLDING = 0x1,
    HAL_MODEM_CALL_STATUS_DIALING= 0x2,
    HAL_MODEM_CALL_STATUS_ALERTING= 0x3,
    HAL_MODEM_CALL_STATUS_INCOMING= 0x4,
    HAL_MODEM_CALL_STATUS_WAITING= 0x5,
    HAL_MODEM_CALL_STATUS_OFFERING= 0x6,
    HAL_MODEM_CALL_STATUS_DISCONNECTING= 0x7,
    HAL_MODEM_CALL_STATUS_DISCONNECTED
}HalModemCallStatus_e;

typedef struct
{
    HalModemCallStatus_e status;
    int32_t callId;
    uint8_t phoneNum[VOICE_NUMBER_MAX];
}HalModemCallInfo_t;

typedef struct
{
    uint32_t infoNum;
    HalModemCallInfo_t callInfo[VOICE_CALL_INFO_MAX];
}HalModemCallInfoList_t;
/**
 * @name CallStatusHandleCb
 * @brief 语音回调
 * @param 
 *      HalModemCallInfoList_t:输入参数, 通话列表
 * @return None
 */
typedef void (*CallStatusHandleCb)(HalModemCallInfoList_t *callList);

/**
 * @name HalModemCallInit
 * @brief modem通话功能初始化。
 * @param Cb 通话状态回调函数的指针。
 * @return 成功返回RET_OK，失败返回RET_ERR。
 */
int32_t HalModemCallInit(CallStatusHandleCb Cb);

/**
 * @name HalModemCallDeInit
 * @brief modem通话模块反初始化。
 * @param null
 * @return 成功返回RET_OK，失败返回RET_ERR。
 */
int32_t HalModemCallDeInit(void);

/**
 * @name HalModemCallStart
 * @brief 拨打电话
 * @param phoneNum 指向要拨打的电话号码的指针。
 * @param callId 指向接收呼叫ID的指针，呼叫启动成功时会填充呼叫ID。
 * @return 成功返回RET_OK，失败返回RET_ERR。
 */
int32_t HalModemCallStart(const uint8_t *phoneNum, int32_t *callId);

/**
 * @name HalModemCallEnd
 * @brief 挂断电话。
 * @param callId 通话的ID，用于标识要挂断的通话。
 * @return 成功返回RET_OK，失败返回RET_ERR。
 */
int32_t HalModemCallEnd(int32_t callId);

/**
 * @name HalModemCallAnswer
 * @brief 接听电话。
 * @param callId 电话呼叫的ID。
 * @return 返回操作结果，RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemCallAnswer(int32_t callId);
#ifdef __cplusplus
}
#endif

#endif