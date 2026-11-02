#include "message_0000.h"

/**
 * @name DoipParserMessage0000
 * @brief 解析Doip消息0000，提取通用头部否定应答信息。
 * @param data 指向Doip数据包起始位置的指针。
 * @param dataLen 数据包长度。
 * @param ghNack 指向GeneralHeaderNack_t结构体的指针，用于存储解析结果。
 * @return 若成功返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipParserMessage0000(const uint8_t *data, uint32_t dataLen, GeneralHeaderNack_t *ghNack)
{
    
    int32_t ret = RET_ERR;
    if (!data || (dataLen < sizeof(DoipPacket_t) + E_PL_HEADER_NEGTIVE_ACK_LEN) || !ghNack)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header;
    
    DoipParserHeader(data, dataLen, &header);
    if (E_PL_TYPE_HEADER_NEGTIVE_ACK != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    DoipPacket_t *packet = (DoipPacket_t *)data;
    ghNack->nack = packet->payload[0];
    ret = RET_OK;
    return ret;
}

/**
 * @name DoipGenMessage0000
 * @brief 生成DOIP协议中的消息0000，主要用于发送否定应答。
 * @param ghNack 指向GeneralHeaderNack_t结构的指针，包含否定应答的详细信息。
 * @param data 指向用于存储生成消息的数据缓冲区的指针。
 * @param dataLen 指向存储数据缓冲区长度的变量的指针，入参和出参，用于输入时指定数据缓冲区的初始长度，
 *                输出时返回实际使用的数据长度。
 * @return 返回生成消息的操作结果，成功时返回RET_OK，错误时返回其他错误码。
 */
int32_t DoipGenMessage0000(const GeneralHeaderNack_t *ghNack, uint8_t *data, uint32_t *dataLen)
{
    int32_t ret = RET_ERR;
    if (!ghNack ||!data || !dataLen || (*dataLen < sizeof(DoipPacket_t) + E_PL_HEADER_NEGTIVE_ACK_LEN))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header;
    DoipGenerateHeader(E_PL_TYPE_HEADER_NEGTIVE_ACK, E_PL_HEADER_NEGTIVE_ACK_LEN, &header);
    ret = DoipGeneratePacket(&header, &ghNack->nack, data, dataLen);
    return ret;
}

/**
 * @name DoipCheckMessage0000
 * @brief 检查Doip消息是否为特定的否定应答类型。
 * @param data 指向Doip消息数据的指针。
 * @param dataLen Doip消息数据的长度。
 * @return 如果消息是预期的否定应答类型则返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipCheckMessage0000(const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!data || (dataLen < sizeof(DoipPacket_t) + E_PL_HEADER_NEGTIVE_ACK_LEN))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header;
    
    DoipParserHeader(data, dataLen, &header);

    if (E_PL_TYPE_HEADER_NEGTIVE_ACK != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    if (E_PL_HEADER_NEGTIVE_ACK_LEN != header.payloadLen)
    {
        LOG_Error("%s PL len check fail,len:%d.", __FUNCTION__, header.payloadLen);
        return ret;        
    }
    ret = RET_OK;
    return ret;
}