#include "message.h"


/**
 * @name DoipParserHeader
 * @brief 解析DOIP消息头
 *        本函数用于解析接收到的DOIP（Diagnostic Over IP）消息的头部信息，提取出协议类型、消息方向、负载类型和负载长度等关键信息。
 * @param data 指向接收到的原始DOIP数据包的指针。
 * @param dataLen 接收到的数据包长度。
 * @param header 指向用于存储解析后消息头信息的结构体的指针。
 * @return 若解析成功，返回RET_OK（假设为0）；若参数无效或数据包长度不足，返回RET_ERR（假设为非0值）。
 */
int32_t DoipParserHeader(const uint8_t *data, uint32_t dataLen, DoipMsgHeader_t *header)
{
    int32_t ret = RET_ERR;
    if (!data || (dataLen < sizeof(DoipPacket_t)) || !header)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    const DoipPacket_t *packet = (DoipPacket_t *)data;

    header->protocalType = packet->protocal;
    header->typeReverse = packet->reverse;
    header->payloadType = BYTES_TO_UINT16(packet->type[0],  packet->type[1]);
    header->payloadLen = BYTES_TO_UINT32(packet->length[0],packet->length[1],packet->length[2],packet->length[3]);
    ret = RET_OK;
    return ret;
}

/**
 * @name DoipGenerateHeader
 * @brief 生成DOIP消息头
 *        本函数用于根据给定的消息类型和长度生成一个DOIP消息的头部结构体。
 * @param type 消息负载类型，枚举类型DoipPayloadType_e。
 * @param length 消息负载的长度，单位为字节。
 * @param header 指向接收消息头的结构体的指针。
 * @return 函数执行状态，RET_OK表示成功，RET_ERR表示错误。
 */
int32_t DoipGenerateHeader(DoipPayloadType_e type, uint32_t length, DoipMsgHeader_t *header)
{
    int32_t ret = RET_ERR;
    if (!header)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    header->protocalType = 0x02;
    header->typeReverse = 0xfd;
    header->payloadType = type;
    header->payloadLen = length;
    ret = RET_OK;
    return ret;
}

/**
 * @name DoipGeneratePacket
 * @brief 生成DOIP包。
 *        根据提供的DOIP消息头和负载数据，构建一个DOIP包。
 * @param header 指向DOIP消息头的常量指针。消息头包含协议类型、负载类型、负载长度等信息。
 * @param payload 指向负载数据的常量字节数组指针。如果消息头指定的负载长度大于0，此参数不能为空。
 * @param data 指向用于存储生成的DOIP包的字节数组的指针。
 * @param dataLen 指向用于存储生成的DOIP包长度的整型变量的指针。
 * @return 函数执行状态码。RET_OK表示成功，RET_ERR表示错误。
 */
int32_t DoipGeneratePacket(const DoipMsgHeader_t *header, 
                        const uint8_t *payload, 
                        uint8_t *data, 
                        uint32_t *dataLen)
{
    int32_t ret = RET_ERR;
    if (!header || ((header->payloadLen > 0) && !payload) || !data || !dataLen 
        || (*dataLen < sizeof(DoipPacket_t) + header->payloadLen))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipPacket_t *packet = (DoipPacket_t *)data;
    packet->protocal = header->protocalType;
    packet->reverse = header->typeReverse;
    packet->type[0] = (header->payloadType >> 8) & 0xff;
    packet->type[1] = header->payloadType & 0xff;
    packet->length[0]= (header->payloadLen >> 24) & 0xff;
    packet->length[1] = (header->payloadLen >> 16) & 0xff;
    packet->length[2] = (header->payloadLen >> 8) & 0xff;
    packet->length[3] = header->payloadLen & 0xff;
    if (header->payloadLen > 0)
    {
        memcpy(packet->payload, payload, header->payloadLen);
    }
    *dataLen = sizeof(DoipPacket_t) + header->payloadLen;
    ret = RET_OK;
    return ret;
}
