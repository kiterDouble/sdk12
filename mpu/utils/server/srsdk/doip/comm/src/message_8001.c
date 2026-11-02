#include "message_8001.h"

/**
 * @name DoipParserMessage8001
 * @brief 解析DOIP消息（8001消息）
 *        此函数用于解析接收到的DOIP（车辆诊断over以太网协议）消息，检查消息是否为期望的诊断请求类型。
 * @param data 指向接收到的数据缓冲区的指针。
 * @param dataLen 数据缓冲区的长度。
 * @return 如果解析成功且消息类型匹配，则返回RET_OK（定义为成功返回值）；否则返回RET_ERR（定义为错误返回值）。
 */
int32_t DoipParserMessage8001(const uint8_t *data, uint32_t dataLen)
{
    
    int32_t ret = RET_ERR;
    if (!data || (dataLen < sizeof(DoipPacket_t) + E_PL_DIAGNOSTIC_REQ_MIN_LEN))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header;
    
    DoipParserHeader(data, dataLen, &header);
    if (E_PL_TYPE_DIAGNOSTIC_REQ != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

/**
 * @name DoipGenMessage8001
 * @brief 生成DOIP消息(8001)，用于诊断通信。
 * @param sourceLa 源逻辑地址，16位无符号整型。
 * @param targetLa 目标逻辑地址，16位无符号整型。
 * @param udsData UDS（统一诊断服务）数据，指向数据的指针。
 * @param udsLen UDS数据长度，无符号整型。
 * @param data 指向用于存放生成消息数据的缓冲区的指针。
 * @param dataLen 指向用于指示缓冲区长度的变量的指针。
 * @return 若成功返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipGenMessage8001(uint32_t sourceLa, 
                            uint32_t targetLa, 
                            const uint8_t *udsData, 
                            uint32_t udsLen, 
                            uint8_t *data, 
                            uint32_t *dataLen)
{
    int32_t ret = RET_ERR;
    if (!udsData ||!data || !dataLen || (*dataLen < sizeof(DoipPacket_t) + sizeof(DiagnosticMessage_t) + udsLen))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipPacket_t *packet = (DoipPacket_t *)data;
    DoipMsgHeader_t header;
    DoipGenerateHeader(E_PL_TYPE_DIAGNOSTIC_REQ, sizeof(DiagnosticMessage_t) + udsLen, &header);
    
    packet->protocal = header.protocalType;
    packet->reverse = header.typeReverse;
    packet->type[0] = (header.payloadType >> 8) & 0xff;
    packet->type[1] = header.payloadType & 0xff;
    packet->length[0]= (header.payloadLen >> 24) & 0xff;
    packet->length[1] = (header.payloadLen >> 16) & 0xff;
    packet->length[2] = (header.payloadLen >> 8) & 0xff;
    packet->length[3] = header.payloadLen & 0xff;
    DiagnosticMessage_t *diagPL = (DiagnosticMessage_t *)packet->payload;
    diagPL->sourceAddr[0] = (sourceLa >> 8) & 0xff;
    diagPL->sourceAddr[1] = sourceLa & 0xff;
    diagPL->targetAddr[0] = (targetLa >> 8) & 0xff;
    diagPL->targetAddr[1] = targetLa & 0xff;
    memcpy(diagPL->userdata, udsData, udsLen);

    *dataLen = sizeof(DoipPacket_t) + sizeof(DiagnosticMessage_t) + udsLen;
    ret = RET_OK;
    return ret;
}


/**
 * @name DoipCheckMessage8001
 * @brief 检查Doip消息是否符合8001协议规范
 * @param data 指向Doip消息数据的指针
 * @param dataLen Doip消息数据的长度
 * @return 若消息符合规范则返回RET_OK，否则返回RET_ERR
 */
int32_t DoipCheckMessage8001(const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!data || (dataLen < sizeof(DoipPacket_t) + E_PL_DIAGNOSTIC_REQ_MIN_LEN))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header;
    
    DoipParserHeader(data, dataLen, &header);

    if (E_PL_TYPE_DIAGNOSTIC_REQ != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    if (E_PL_DIAGNOSTIC_REQ_MIN_LEN > header.payloadLen)
    {
        LOG_Error("%s PL len check fail,len:%d.", __FUNCTION__, header.payloadLen);
        return ret;        
    }
    ret = RET_OK;
    return ret;
}
