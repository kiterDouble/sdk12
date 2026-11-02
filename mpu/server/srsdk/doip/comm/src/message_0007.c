
#include "message_0007.h"


/**
 * @name 
 * @brief 解析Doip消息0007
 *        本函数用于解析接收到的Doip消息，首先验证消息数据的有效性，然后解析消息头，
 *        并检查消息类型是否为期望的 alive 检查请求类型。
 * @param data 指向接收到的消息数据的指针。
 * @param dataLen 消息数据的长度。
 * @return 如果解析成功且消息类型匹配，则返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipParserMessage0007(const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;
    if (!data || (dataLen < sizeof(DoipPacket_t)))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header;
    
    DoipParserHeader(data, dataLen, &header);
    if (E_PL_TYPE_ALIVE_CHECK_REQ != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

/**
 * @name DoipGenMessage0007
 * @brief 生成DOIP消息0007
 *        该函数用于生成一条DOIP（车辆诊断over以太网协议）的消息，消息类型为存活检查请求。
 * @param data 指向用于存储生成消息的缓冲区的指针。
 * @param dataLen 指向存储缓冲区长度的变量的指针。函数执行后，此处将更新为实际使用的缓冲区长度。
 * @return 成功返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipGenMessage0007(uint8_t *data, uint32_t *dataLen)
{
    int32_t ret = RET_ERR;
    if (!data || !dataLen || (*dataLen < sizeof(DoipPacket_t)))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header;
    DoipGenerateHeader(E_PL_TYPE_ALIVE_CHECK_REQ, E_PL_ALIVE_CHECK_REQ_LEN, &header);
    ret = DoipGeneratePacket(&header, NULL, data, dataLen);
    return ret;
}

/**
 * @name DoipCheckMessage0007
 * @brief 检查DoIP消息是否为指定的 alive 检查请求。
 * @param data 指向DoIP数据包的指针。
 * @param dataLen 数据包长度。
 * @return 如果消息是有效的alive检查请求则返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipCheckMessage0007(const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!data || (dataLen < sizeof(DoipPacket_t) + E_PL_VEHICLE_INDENTIFY_LEN))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header;
    
    DoipParserHeader(data, dataLen, &header);

    if (E_PL_TYPE_ALIVE_CHECK_REQ != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    if (E_PL_ALIVE_CHECK_REQ_LEN != header.payloadLen)
    {
        LOG_Error("%s PL len check fail,len:%d.", __FUNCTION__, header.payloadLen);
        return ret;        
    }
    ret = RET_OK;
    return ret;
}