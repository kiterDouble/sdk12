#include "message_0003.h"

/**
 * @name DoipParserMessage0003
 * @brief 解析DoIP消息0003，提取车辆识别码（VIN）。
 * @param data 指向DoIP数据包起始位置的指针。
 * @param dataLen DoIP数据包的长度。
 * @param reqPL 指向存储解析结果的结构体的指针。
 * @return 若成功提取VIN码则返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipParserMessage0003(const uint8_t *data, uint32_t dataLen, VehicleIDReqWithVin_t *reqPL)
{
    int32_t ret = RET_ERR;
    if (!data || (dataLen < sizeof(DoipPacket_t) + E_PL_VEHICLE_INDENTIFY_VIN_LEN) || !reqPL)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header;
    
    DoipParserHeader(data, dataLen, &header);
    if (E_PL_TYPE_VEHICLE_INDENTIFY_VIN_REQ != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    const DoipPacket_t *packet = (DoipPacket_t *)data;
    memcpy(reqPL->vin, packet->payload, E_PL_VEHICLE_INDENTIFY_VIN_LEN);
    ret = RET_OK;
    return ret;
}

/**
 * @name DoipGenMessage0003
 * @brief 生成车辆ID请求消息（基于VIN）。
 * @param reqPL 指向包含车辆VIN的请求结构体的指针。
 * @param data 指向用于存储生成消息数据的缓冲区的指针。
 * @param dataLen 指向存储缓冲区长度的变量的指针，输入时应包含预期的最大长度。
 * @return 若成功，返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipGenMessage0003(const VehicleIDReqWithVin_t *reqPL, uint8_t *data, uint32_t *dataLen)
{
    int32_t ret = RET_ERR;
    if (!reqPL ||!data || !dataLen || (*dataLen < sizeof(DoipPacket_t) + E_PL_VEHICLE_INDENTIFY_VIN_LEN))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header;
    DoipGenerateHeader(E_PL_TYPE_VEHICLE_INDENTIFY_VIN_REQ, E_PL_VEHICLE_INDENTIFY_VIN_LEN, &header);
    ret = DoipGeneratePacket(&header, (void*)&reqPL->vin, data, dataLen);
    return ret;
}

/**
 * @name DoipCheckMessage0003
 * @brief 检查Doip消息是否符合指定格式和内容。
 * @param data 指向待检查数据的指针。
 * @param dataLen 数据长度。
 * @return 如果消息检查通过，返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipCheckMessage0003(const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!data || (dataLen < sizeof(DoipPacket_t) + E_PL_VEHICLE_INDENTIFY_VIN_LEN))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header;
    
    DoipParserHeader(data, dataLen, &header);

    if (E_PL_TYPE_VEHICLE_INDENTIFY_VIN_REQ != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    if (E_PL_VEHICLE_INDENTIFY_VIN_LEN != header.payloadLen)
    {
        LOG_Error("%s PL len check fail,len:%d.", __FUNCTION__, header.payloadLen);
        return ret;        
    }
    ret = RET_OK;
    return ret;
}