#include "message_0001.h"

/**
 * @name DoipParserMessage0001
 * @brief 解析Doip消息0001
 *        本函数用于解析车辆识别请求的消息。首先会校验输入的数据是否有效，然后解析消息头，
 *        并检查消息类型是否符合预期。只有当所有检查都通过时，才返回成功。
 * @param data 指向待解析数据的指针。
 * @param dataLen 数据长度。
 * @return 若解析成功返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipParserMessage0001(const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;
    if (!data || (dataLen < sizeof(DoipPacket_t)))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header = {0};
    
    (void)DoipParserHeader(data, dataLen, &header);
    if ((uint16_t)E_PL_TYPE_VEHICLE_INDENTIFY_REQ != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

/**
 * @name DoipGenMessage0001
 * @brief 生成Doip消息0001
 *        本函数用于生成一条车辆识别请求的消息。首先会校验输入参数的合法性，然后构造消息的头部，
 *        并调用DoipGeneratePacket函数完成消息体的生成。 
 * @param data 指向用于存储生成消息数据的缓冲区的指针。
 * @param dataLen 指向存储缓冲区长度的变量的指针。函数执行后，此处将更新为实际使用的缓冲区长度。
 * @return 返回生成消息的操作结果，成功时返回RET_OK，错误时返回RET_ERR。
 */
int32_t DoipGenMessage0001(uint8_t *data, uint32_t *dataLen)
{
    int32_t ret = RET_ERR;
    if (!data || !dataLen || (*dataLen < sizeof(DoipPacket_t)))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header = {0};
    (void)DoipGenerateHeader(E_PL_TYPE_VEHICLE_INDENTIFY_REQ, E_PL_VEHICLE_INDENTIFY_LEN, &header);
    ret = DoipGeneratePacket(&header, NULL, data, dataLen);
    return ret;
}

/**
 * @name DoipCheckMessage0001
 * @brief 检查Doip消息0001的合法性。
 * @param data 指向Doip消息数据的指针。
 * @param dataLen Doip消息数据的长度。
 * @return 如果消息合法，返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipCheckMessage0001(const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!data || (dataLen < (sizeof(DoipPacket_t) + (uint32_t)E_PL_VEHICLE_INDENTIFY_LEN)))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header = {0};
    
    (void)DoipParserHeader(data, dataLen, &header);

    if ((uint16_t)E_PL_TYPE_VEHICLE_INDENTIFY_REQ != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    if ((uint32_t)E_PL_VEHICLE_INDENTIFY_LEN != header.payloadLen)
    {
        LOG_Error("%s PL len check fail,len:%d.", __FUNCTION__, header.payloadLen);
        return ret;        
    }
    ret = RET_OK;
    return ret;
}