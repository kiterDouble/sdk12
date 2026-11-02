#include "message_0002.h"


/**
 * @name DoipParserMessage0002
 * @brief 解析车辆ID请求消息（消息类型0002）
 *        该函数用于解析接收到的数据，提取车辆电子标识（EID），并将其存储在提供的请求参数结构中。
 * @param data 指向接收到的数据包的指针。
 * @param dataLen 接收到的数据包的长度。
 * @param reqPL 指向存储车辆ID请求信息的结构体的指针。
 * @return 若解析成功，返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipParserMessage0002(const uint8_t *data, uint32_t dataLen, VehicleIDReqWithEid_t *reqPL)
{
    int32_t ret = RET_ERR;
    if (!data || (dataLen < (sizeof(DoipPacket_t) + (uint32_t)E_PL_VEHICLE_INDENTIFY_EID_LEN)) || !reqPL)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header = {0};
    
    (void)DoipParserHeader(data, dataLen, &header);
    if ((uint16_t)E_PL_TYPE_VEHICLE_INDENTIFY_EID_REQ != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    const DoipPacket_t *packet = (const DoipPacket_t *)((const void*)data);
    (void)memcpy(reqPL->eid, packet->payload, E_PL_VEHICLE_INDENTIFY_EID_LEN);
    ret = RET_OK;
    return ret;
}

/**
 * @name DoipGenMessage0002
 * @brief 生成车辆ID请求消息（基于EID）。
 * @param reqPL 指向包含车辆EID（电子识别码）的请求结构体的指针。
 * @param data 指向用于存储生成消息数据的缓冲区的指针。
 * @param dataLen 指向存储缓冲区长度的变量的指针，函数执行后会更新为实际使用的长度。
 * @return 返回生成消息的操作结果，成功返回RET_OK，错误返回RET_ERR。
 */
int32_t DoipGenMessage0002(const VehicleIDReqWithEid_t *reqPL, uint8_t *data, uint32_t *dataLen)
{
    int32_t ret = RET_ERR;
    if (!reqPL ||!data || !dataLen || (*dataLen < (sizeof(DoipPacket_t) + (uint32_t)E_PL_VEHICLE_INDENTIFY_EID_LEN)))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header = {0};
    (void)DoipGenerateHeader(E_PL_TYPE_VEHICLE_INDENTIFY_EID_REQ, E_PL_VEHICLE_INDENTIFY_EID_LEN, &header);
    ret = DoipGeneratePacket(&header, (void*)&reqPL->eid, data, dataLen);
    return ret;
}

/**
 * @name DoipCheckMessage0002
 * @brief 检查Doip消息0002的合法性。
 * @param data 指向Doip消息数据的指针。
 * @param dataLen Doip消息数据的长度。
 * @return 如果消息合法，返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipCheckMessage0002(const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!data || (dataLen < (sizeof(DoipPacket_t) + (uint32_t)E_PL_VEHICLE_INDENTIFY_EID_LEN)))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header = {0};
    
    (void)DoipParserHeader(data, dataLen, &header);

    if ((uint16_t)E_PL_TYPE_VEHICLE_INDENTIFY_EID_REQ != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    if ((uint32_t)E_PL_VEHICLE_INDENTIFY_EID_LEN != header.payloadLen)
    {
        LOG_Error("%s PL len check fail,len:%d.", __FUNCTION__, header.payloadLen);
        return ret;        
    }
    ret = RET_OK;
    return ret;
}