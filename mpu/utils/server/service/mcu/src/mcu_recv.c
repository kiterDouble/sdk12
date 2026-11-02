#include "crcTool.h"
#include "err_code.h"
#include "sr_log.h"
#include "mcu_bus.h"
#include "mcu_recv.h"
#include "mcu_lock.h"

/**
 * @name McuSrvRecvN
 * @brief 从MCU服务数据中读取指定数量的数据。
 * @param srvData 指向McuSrvData_t结构体的指针，包含服务所需的数据。
 * @param data 接收读取数据的缓冲区。
 * @param len 要读取的数据长度。
 * @return 返回实际读取的字节数，如果发生错误则返回-1。
 */
static int32_t McuSrvRecvN(McuSrvData_t *srvData, uint8_t *data, uint32_t len)
{
    int32_t offset = 0;

    if ((NULL == srvData) || (NULL == data) || (len == 0U))
    {
        LOG_Error("%s param invalid.\n", __FUNCTION__);  // 统一日志宏
        return -1;
    }

    do
    {
        int32_t readn = 0;
        readn = McuSrvBusRecv(srvData, &data[offset], (len - offset));
        if (readn < 0)
        {
            LOG_Error("%s Uart Read Error\n", __FUNCTION__);  // 统一日志宏
            return -1;
        }
        else if (readn == 0)  // 增加返回值为零的处理
        {
            LOG_Error("%s Uart Read Zero\n", __FUNCTION__);  // 新增日志信息
            return -1;
        }
        offset += readn;
    } while (offset < len);

    return offset;
}

/**
 * @name McuSrvRecvData
 * @brief 接收MCU服务数据
 * @param srvData 服务数据结构体指针
 * @param data 接收的数据缓冲区指针
 * @param dataLen 数据长度指针，输入参数为期望接收的数据长度，输出参数为实际接收的数据长度
 * @return int32_t 返回操作结果，RET_OK表示成功，RET_ERR表示失败
 */
static int32_t McuSrvRecvData(McuSrvData_t *srvData, uint8_t *data, uint32_t *dataLen)
{
    uint32_t offset = 0;
    int32_t ret = RET_ERR;
    int32_t readn = -1;
    uint16_t crc16Val = 0;

    if (!srvData || !data || (*dataLen == 0))
    {
        LOG_Error("%s param invalid.\n", __FUNCTION__);
        return ret;
    }
    do{
        readn = McuSrvBusRecv(srvData, &data[offset], 1);
        if ((readn <=0))
        {
            LOG_Error("%s read header fail, 1byte not 0x55.", __FUNCTION__);
            LOG_Debug("%s deinit bus.", __FUNCTION__);
            McuSrvBusDeinit(srvData);
            srvData->dataBusFd = -1;
            return ret;        
        }
    }while(data[offset] != 0x55);

    offset++;

    readn = McuSrvBusRecv(srvData, &data[offset], 1);
    if ((readn <=0) || (data[offset] != 0xAA))
    {
        LOG_Error("%s read header fail, 2 byte not 0xAA.", __FUNCTION__);
        if (readn <= 0)
        {
            LOG_Debug("%s deinit bus.", __FUNCTION__);
            McuSrvBusDeinit(srvData);
            srvData->dataBusFd = -1;
        }
        LOG_DebugHex(data, offset, "McuSrvRecvData head");
        return ret;
    }
    offset++;
    
    readn = McuSrvRecvN(srvData, &data[offset], 6);
    if (readn <= 0)
    {
        LOG_Error("%s McuSrvBusRecv 6byte header error.", __FUNCTION__);
        LOG_Debug("%s deinit bus.", __FUNCTION__);
        McuSrvBusDeinit(srvData);
        srvData->dataBusFd = -1;
        return ret;
    }
    offset += 6;

    uint16_t srvLen = ((data[5] &0xff) << 8) + (data[6] &0xff);
    #define MAX_SRV_LEN 4096
    if (srvLen > MAX_SRV_LEN)
    {
        LOG_Error("%s srvLen[%d] error.", __FUNCTION__, srvLen);
        return ret;
    }
    // else
    // {
    //     LOG_Debug("srvLen[%d] ok.", srvLen);
    // }
    readn = McuSrvRecvN(srvData, &data[offset], srvLen);
    //LOG_DebugHex(data,srvLen+10,"chen_tmpMcuSrvBusRecv");
    if (readn <= 0)
    {
        LOG_Error("%s McuSrvBusRecv serviceData error.", __FUNCTION__);
        LOG_Debug("%s deinit bus.", __FUNCTION__);
        McuSrvBusDeinit(srvData);
        srvData->dataBusFd = -1;
        return ret;
    }
    offset += srvLen;
    readn = McuSrvRecvN(srvData, &data[offset], 2);
    if (readn <= 0)
    {
        LOG_Error("%s McuSrvBusRecv serviceData error.", __FUNCTION__);
        LOG_Debug("%s deinit bus.", __FUNCTION__);
        McuSrvBusDeinit(srvData);
        srvData->dataBusFd = -1;
        return ret;
    }

    crc16Val = ((data[offset] & 0xff ) << 8) + (data[offset + 1] & 0xff);
    offset +=2;

    if (crc16Val != Crc16((void*)data, srvLen + 8, Crc16Ccitt))
    {
        LOG_Error("%s crc check fail.\n", __FUNCTION__);
        LOG_DebugHex(data, offset, "McuSrvRecvData CRC");
        return ret;
    }

    LOG_DebugHex(data,offset,"McuSrvBusRecv");
    *dataLen = offset;
    ret = RET_OK;
    return ret;
}

/**
 * @name McuSrvRecvMesgProcess
 * @brief 处理从MCU接收的消息
 * @param srvData 指向McuSrvData_t结构体的指针，包含服务相关数据
 * @return int32_t 返回操作结果，RET_OK表示成功，RET_ERR表示失败
 */
int32_t McuSrvRecvMesgProcess(McuSrvData_t *srvData)
{
    int32_t ret = RET_ERR;
    uint8_t data[5000] = {0x0,};
    uint32_t dataLen = sizeof(data);

    if (!srvData)
    {
        LOG_Error("%s param invalid.\n", __FUNCTION__);
        return ret;
    }

    if (srvData->dataBusFd < 0)
    {
        LOG_Error("%s data bus have not inited.\n", __FUNCTION__);
        return ret;
    }

    if (RET_OK != McuSrvRecvData(srvData, data, &dataLen))
    {
        LOG_Error("%s McuSrvRecvData failed.", __FUNCTION__);
        return ret;
    }

    uint8_t aid = data[2];
    uint8_t mid = data[3];
    uint8_t sub = data[4] & 0x7f;
    

    if ((aid == 0) && (mid == 0))
    {
        LOG_Debug("mcu_debug:%s", &data[8]);
    }
    else
    {
        uint16_t srvLen = ((data[5] &0xff) << 8) + (data[6] &0xff);
        SYSQueMsgType_e type = (SYSQueMsgType_e)(((aid & 0xff) << 16) + ((mid & 0xff) << 8) + (sub & 0xff));
        SendMsgToAllQueue(type, &data[8], srvLen);
    }

    ret = RET_OK;
    return ret;
}
