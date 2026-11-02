#include "message_0004.h"

/**
 * @name DoipParserMessage0004
 * @brief 解析Doip消息0004，用于处理车辆识别响应。
 * @param data 指向Doip消息数据的指针。
 * @param dataLen Doip消息数据的长度。
 * @param resPL 指向用于存储解析结果的VehicleAnnoRes_t结构体的指针。
 * @return 若解析成功返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipParserMessage0004(const uint8_t *data, uint32_t dataLen, VehicleAnnoRes_t *resPL)
{
    int32_t ret = RET_ERR;
    if (!data || (dataLen < sizeof(DoipPacket_t) + E_PL_VEHICLE_INDENTIFY_RES_MIN_LEN) || !resPL)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header;
    uint8_t offset = 0;
    
    DoipParserHeader(data, dataLen, &header);
    if (E_PL_TYPE_ANNOUNCE_OR_IDENTIFYRES_RES != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }
    DoipPacket_t *packet = (DoipPacket_t *)data;
    memcpy(resPL->vin, &packet->payload[offset], sizeof(resPL->vin)); 
    offset += sizeof(resPL->vin);
    memcpy(resPL->logicAddr, &packet->payload[offset], sizeof(resPL->logicAddr));
    offset += sizeof(resPL->logicAddr);
    memcpy(resPL->eid, &packet->payload[offset], sizeof(resPL->eid));
    offset += sizeof(resPL->eid);
    memcpy(resPL->gid, &packet->payload[offset], sizeof(resPL->gid));
    offset += sizeof(resPL->gid);
    resPL->actionReq = 0x00;
    if (E_PL_VEHICLE_INDENTIFY_RES_LEN == header.payloadLen)
    {
        resPL->actionReq = packet->payload[offset];
    }

    ret = RET_OK;
    return ret;
}

/**
 * @name DoipGenMessage0004
 * @brief 生成车辆识别消息DoipGenMessage0004
 *        本函数用于根据给定的车辆信息资源生成一个特定格式的车辆识别消息。
 * @param resPL 指向包含车辆信息资源的指针。车辆信息包括VIN码、逻辑地址、电子标识符和组标识符。
 * @param data 指向用于存储生成消息数据的缓冲区的指针。
 * @param dataLen 指向存储消息数据长度的变量的指针。在函数调用前，该变量应初始化为预期的消息长度。
 *                函数执行后，如果成功，该变量将被更新为实际生成的消息长度。
 * @return 返回生成消息的操作结果。成功时返回RET_OK，错误时返回RET_ERR。
 */
int32_t DoipGenMessage0004(const VehicleAnnoRes_t *resPL, uint8_t *data, uint32_t *dataLen)
{
    int32_t ret = RET_ERR;
    if (!resPL ||!data || !dataLen || (*dataLen < sizeof(DoipPacket_t) + E_PL_VEHICLE_INDENTIFY_RES_LEN))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header;
    uint8_t offset = 0;
    uint8_t payload[E_PL_VEHICLE_INDENTIFY_RES_LEN] = {0x0,};

    memcpy(&payload[offset], resPL->vin, sizeof(resPL->vin)); 
    offset += sizeof(resPL->vin);
    memcpy(&payload[offset], resPL->logicAddr, sizeof(resPL->logicAddr));
    offset += sizeof(resPL->logicAddr);
    memcpy(&payload[offset], resPL->eid, sizeof(resPL->eid));
    offset += sizeof(resPL->eid);
    memcpy(&payload[offset], resPL->gid, sizeof(resPL->gid));
    offset += sizeof(resPL->gid);
    payload[offset] = resPL->actionReq;
    DoipGenerateHeader(E_PL_TYPE_ANNOUNCE_OR_IDENTIFYRES_RES, E_PL_VEHICLE_INDENTIFY_RES_LEN, &header);
    ret = DoipGeneratePacket(&header, payload, data, dataLen);
    return ret;
}

/**
 * @name DoipCheckMessage0004
 * @brief 检查Doip消息是否符合0004规范
 * @param data 指向Doip数据包的指针
 * @param dataLen 数据包长度
 * @return 若检查通过返回RET_OK，否则返回RET_ERR
 */
int32_t DoipCheckMessage0004(const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!data || (dataLen < sizeof(DoipPacket_t) + E_PL_VEHICLE_INDENTIFY_RES_MIN_LEN))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header;
    
    DoipParserHeader(data, dataLen, &header);

    if (E_PL_TYPE_ANNOUNCE_OR_IDENTIFYRES_RES != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    if ((E_PL_VEHICLE_INDENTIFY_RES_MIN_LEN != header.payloadLen)
        && (E_PL_VEHICLE_INDENTIFY_RES_LEN != header.payloadLen))
    {
        LOG_Error("%s PL len check fail,len:%d.", __FUNCTION__, header.payloadLen);
        return ret;        
    }
    ret = RET_OK;
    return ret;
}