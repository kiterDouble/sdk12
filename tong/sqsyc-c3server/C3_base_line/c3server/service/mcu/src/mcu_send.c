#include <stdlib.h>
#include <string.h>
#include "crcTool.h"
#include "err_code.h"
#include "sr_log.h"
#include "mcu_bus.h"
#include "mcu_send.h"
#include "mcu_can_filter.h"

#define MCU_FRAME_HEADER_LEN (8U)
#define MCU_FRAME_CRC_LEN (2U)

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
                            void* userdata)
{
    uint8_t data[5000] = {0x0,};
    uint32_t offset = 0;
    McuSrv_t *srvData = (McuSrv_t *)((void*)userdata);
    const McuSrvRpcReq_t *const req = (const McuSrvRpcReq_t *const)((const void *const)reqData);
    McuSrvRpcRes_t *res = (McuSrvRpcRes_t *)((void*)respData);

    if (!srvData || !req || !res || !respLen)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    *respLen = (uint16_t)sizeof(McuSrvRpcRes_t);
    res->type = 0;
    res->result = RET_ERR;

    /* reqLen 必须在访问 req->dataLen 之前校验，否则短包越界读 */
    if ((uint32_t)reqLen < (uint32_t)sizeof(McuSrvRpcReq_t))
    {
        LOG_Error("%s invalid reqLen=%u.", __FUNCTION__, (unsigned int)reqLen);
        return;
    }

    res->type = req->type;

    if ((req->dataLen == 0U) ||
        (req->dataLen > ((uint32_t)reqLen - (uint32_t)sizeof(McuSrvRpcReq_t))) ||
        (req->dataLen > ((uint32_t)sizeof(data) - MCU_FRAME_HEADER_LEN - MCU_FRAME_CRC_LEN)))
    {
        LOG_Error("%s invalid dataLen=%u, reqLen=%u.", __FUNCTION__, (unsigned int)req->dataLen, (unsigned int)reqLen);
        return;
    }

    /* CAN ID过滤表应答：不再下发MCU，解析后全局保存在McuServer */
    if (req->type == SYS_QUE_TYPE_MCU_VEHICLE_CAN_ID_RES)
    {
        (void)McuCanFilterSaveFromRes(req->data, req->dataLen);
        res->result = RET_OK;
        return;
    }

    uint8_t aid = (uint8_t)(((uint32_t)req->type >> 16U) & 0xffU);
    uint8_t mid = (uint8_t)(((uint32_t)req->type >> 8U) & 0xffU);
    uint8_t sub = (uint8_t)((uint32_t)req->type & 0xffU);

    if ((aid & 0x80) == 0x80 && srvData->spiData.dataBusFd == -1)
    {
        LOG_Error("%s %s not ready.", __FUNCTION__, srvData->spiData.cfg.device);
        res->result = RET_ERR;
        return;        
    }
    else if (srvData->uartData.dataBusFd == -1)
    {
        LOG_Error("%s %s not ready.", __FUNCTION__, srvData->uartData.cfg.device);
        res->result = RET_ERR;
        return;  
    }
    
    //check upgrade.
    if (((srvData->uartData.upgradeStatus != -1) || (srvData->spiData.upgradeStatus != -1)) && (aid != 0x03U))
    {
        LOG_Error("%s in upgrade status, can not send.", __FUNCTION__);
        return; 
    }

    data[offset++] = 0x55U;
    data[offset++] = 0xaaU;
    data[offset++] = aid & 0x7fU;
    data[offset++] = mid;
    data[offset++] = sub | 0x80U;
    data[offset++] = (uint8_t)((req->dataLen >> 8U) & 0xffU);
    data[offset++] = (uint8_t)(req->dataLen & 0xffU);
    data[offset++] = 0x00U;
    (void)memcpy(&data[offset], req->data, req->dataLen);
    offset += req->dataLen;

    uint16_t crc16 = Crc16(data, offset, Crc16Ccitt);
    data[offset++] = (uint8_t)((crc16 >> 8U) & 0xffU);
    data[offset++] = (uint8_t)(crc16 & 0xffU);

    LOG_DebugHex(data,offset,"McuSrvBusSend");

    if ((aid & 0x80) == 0x80) // spi
    {
        LOG_Debug("McuSrvBusSend spi");
        // aid = aid & 0x7f;
        (void)McuSrvBusSend(&srvData->spiData, data, offset);
    }
    else // uart
    {
        // LOG_Debug("McuSrvBusSend uart");
        (void)McuSrvBusSend(&srvData->uartData, data, offset);
    }
   
    res->result = RET_OK;
    *respLen = (uint16_t)sizeof(McuSrvRpcRes_t);
    return; 
}