#include <stdlib.h>
#include <string.h>
#include "crcTool.h"
#include "err_code.h"
#include "sr_log.h"
#include "mcu_bus.h"
#include "mcu_send.h"
#include "mcu_lock.h"

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
    McuSrvData_t *srvData = (McuSrvData_t *)userdata;
    McuSrvRpcReq_t *req = (McuSrvRpcReq_t *) reqData;
    McuSrvRpcRes_t *res = (McuSrvRpcRes_t *) respData;
    uint8_t flag = 0;

    if (!srvData || !req || (req->dataLen == 0) || !res ||!respLen)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    if(subcmd == 0)
    {
    	res->type = req->type;
    	res->result = RET_ERR;
    	if (srvData->dataBusFd < 0)
    	{
        	LOG_Error("%s dataBus not ready.", __FUNCTION__);
        	res->result = RET_ERR;
        	return;        
    	}

    	uint8_t aid = (req->type >> 16) & 0xff;
    	uint8_t mid = (req->type >> 8) & 0xff;
    	uint8_t sub = req->type & 0xff;

        if(getSendDataFlag() != 0 && aid != 0x03) 
        {
            LOG_Debug("%s MCU is upgrading send data stop", __FUNCTION__);
            return;
        }

    	//check upgrade.
    	if ((srvData->upgradeStatus != -1) && (aid != 0x03))
    	{
        	LOG_Error("%s in upgrade status, can not send.", __FUNCTION__);
        	return; 
    	}

    	data[offset++] = 0x55;
    	data[offset++] = 0xaa;
    	data[offset++] = aid;
    	data[offset++] = mid;
    	data[offset++] = sub | 0x80;
    	data[offset++] = (req->dataLen >> 8) & 0xff;
    	data[offset++] = req->dataLen & 0xff;
    	data[offset++] = 0x00;
    	memcpy(&data[offset], req->data, req->dataLen);
    	offset += req->dataLen;

    	uint16_t crc16 = Crc16(data, offset, Crc16Ccitt);
    	data[offset++] = (crc16 >> 8) & 0xff;
    	data[offset++] = crc16 & 0xff;

    	LOG_DebugHex(data,offset,"McuSrvBusSend");

    	McuSrvBusSend(srvData, data, offset);
    	res->result = RET_OK;
    	*respLen = sizeof(McuSrvRpcRes_t);
    }
    else if(subcmd == 1)
    {
        flag = req->data[0];
        setSendDataFlag(flag);
        res->result = RET_OK;
        *respLen = 1;
    }
    else if(subcmd == 2)
    {
        flag = getSendDataFlag();
        res->result = flag;
        *respLen = 1;
    }
    return; 
}
