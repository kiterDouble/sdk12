#include <string.h>
#include "err_code.h"
#include "sr_log.h"
#include "mcu_bus.h"

/**
 * @name McuSrvBusInit
 * @brief 初始化MCU服务总线
 *        该函数用于初始化MCU服务总线，包括打开总线连接等初始化操作。
 * @param srvData 指向McuSrvData_t结构体的指针，包含总线配置信息和操作句柄等。
 * @return int32_t 返回初始化结果，RET_OK表示成功，RET_ERR表示失败。
 */
int32_t McuSrvBusInit(McuSrvData_t *srvData)
{
    int32_t ret = RET_ERR;

    if ((NULL == srvData) || (NULL == srvData->BusOpen))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    srvData->dataBusFd = srvData->BusOpen((void*)&srvData->cfg);
    if (srvData->dataBusFd == -1)
    {
        LOG_Error("%s BusOpen %s fail.", __FUNCTION__, srvData->cfg.device);
        return ret;
    }
    srvData->recvLen = 0U;
    LOG_Debug("%s BusOpen %s success.", __FUNCTION__, srvData->cfg.device);
    ret = RET_OK;
    return ret;
}

/**
 * @name McuSrvBusRecv
 * @brief 从MCU服务总线接收数据
 *        本函数用于通过服务总线从MCU接收数据。调用此函数前，必须确保服务总线已初始化并打开。
 * @param srvData 指向McuSrvData_t结构体的指针，包含服务相关的数据和函数指针。
 * @param data 指向接收缓冲区的指针，用于存储从服务总线接收到的数据。
 * @param dataLen 指定接收缓冲区的长度，单位为字节。
 * @return 返回接收数据长度，错误时返回-1。
 */
int32_t McuSrvBusRecv(McuSrvData_t *srvData, uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if ((NULL == srvData) || (NULL == data) || (dataLen == 0U) || (NULL == srvData->busRead))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    // LOG_Debug("McuSrvBusRecv");
    return srvData->busRead(srvData->dataBusFd, data, dataLen);
}

/**
 * @name McuSrvBusSend
 * @brief 通过服务总线发送数据 
 * @param srvData 指向McuSrvData_t结构体的指针，包含服务数据和操作函数
 * @param data 指向要发送数据的指针
 * @param dataLen 要发送数据的长度
 * @return int32_t 返回发送数据长度，失败返回-1
 */
int32_t McuSrvBusSend(McuSrvData_t *srvData, const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if ((NULL == srvData) || (NULL == data) || (dataLen == 0U) || (NULL == srvData->BusWrite))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    // LOG_Debug("McuSrvBusSend");
    return srvData->BusWrite(srvData->dataBusFd, data, dataLen);
}

/**
 * @name McuSrvBusDeinit
 * @brief 反初始化MCU服务总线
 *        该函数用于执行总线的反初始化操作，关闭数据总线连接。
 * @param srvData 指向McuSrvData_t结构体的指针，包含总线操作的上下文信息。
 * @return int32_t 返回操作的状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t McuSrvBusDeinit(McuSrvData_t *srvData)
{
    int32_t ret = RET_ERR;

    if (!srvData || (NULL == srvData->BusClose))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    srvData->BusClose(srvData->dataBusFd);
    srvData->dataBusFd = -1;
    srvData->recvLen = 0U;
    ret = RET_OK;
    return ret;
}