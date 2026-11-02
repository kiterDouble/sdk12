#ifndef __DOCAN_CAN_TP_H__
#define __DOCAN_CAN_TP_H__
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CANTP_OK (0)
#define CANTP_FAIL (-1)


typedef enum
{
    E_CANTP_TRANSMIT_OK = 0x0,
    E_CANTP_TRANSMIT_NOT_OK
}CanTpTransmitVal_e;

typedef void* CanTpHandle_t;
typedef void (*CanTpTransmitErrorCb_t)(uint32_t result);
typedef void (*CanTpReceiveCb_t)(uint32_t canId, const uint8_t *buffer, uint32_t bufLen);

/**
 * @name CanTpInit
 * @brief CAN-TP初始化。
 * @param hdl 指向CanTpHandle_t的指针，用于存储协议栈的句柄。
 * @param rxCb 接收回调函数，用于处理接收到的CAN消息。
 * @param transErrCb 传输错误回调函数，用于处理传输过程中的错误。
 * @return 成功返回RET_OK，失败返回RET_ERR。
 */
int32_t CanTpInit(CanTpHandle_t *hdl, CanTpReceiveCb_t rxCb, CanTpTransmitErrorCb_t transErrCb);

/**
 * @name CanTpTransmit
 * @brief CAN-TP发送函数(可指定帧格式标志)
 * @param hdl CanTp处理句柄，用于访问私有数据
 * @param canId CAN消息的ID
 * @param frameFlag 帧格式标志, 通过透传协议reserve字节传递给MCU:
 *                  bit0=FDF(CANFD帧), bit1=BRS(可变波特率); 0-经典CAN帧
 * @param data 指向要发送数据的指针
 * @param dataLen 要发送的数据长度
 * @return int32_t 返回传输结果，RET_OK表示成功，其他值表示失败
 */
int32_t CanTpTransmit(CanTpHandle_t hdl, uint32_t canId, uint8_t frameFlag, uint8_t *data, uint32_t dataLen);

/**
 * @name CanTpDeinit
 * @brief 反初始化CAN-TP协议栈。
 * @param hdl 协议栈实例的句柄。
 * @return int32_t 返回RET_OK表示成功，RET_ERR表示失败。
 */
int32_t CanTpDeinit(CanTpHandle_t hdl);

/**
 * @name CanTpLock
 * @brief CAN TP加锁
 * @param hdl CanTpHandle_t类型，CAN-TP的句柄，用于标识特定的CAN-TP实例。
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t CanTpLock(CanTpHandle_t hdl);

/**
 * @name CanTpUnlock
 * @brief CAN TP解锁
 * @param hdl CanTpHandle_t类型，CAN TP的处理句柄，指向CanTpPriv_t结构体的指针
 * @return 操作成功返回RET_OK，失败返回RET_ERR。 
 */
int32_t CanTpUnlock(CanTpHandle_t hdl);

/**
 * @name CanTpSetLockIgnore
 * @brief CAN TP设置锁忽略，锁定状态由上层APP控制
 * @param hdl CanTpHandle_t类型，CAN TP的处理句柄，指向CanTpPriv_t结构体的指针
 * @return 操作成功返回RET_OK，失败返回RET_ERR。 
 */
int32_t CanTpSetLockIgnore(CanTpHandle_t hdl);
#ifdef __cplusplus
}
#endif
#endif