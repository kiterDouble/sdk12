#include "message_8003.h"

/**
 * @name DoipParserMessage8003
 * @brief 解析DOIP消息（8003服务）
 *        此函数用于解析接收到的DOIP（车辆诊断over以太网协议）消息，检查其是否为期望的诊断否定响应（NACK）。
 * @param data 指向接收到的数据缓冲区的指针。
 * @param dataLen 数据缓冲区的长度。
 * @return 如果解析成功且消息类型匹配，则返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipParserMessage8003(const uint8_t *data, uint32_t dataLen)
{
    
    int32_t ret = RET_ERR;
    if (!data || (dataLen < sizeof(DoipPacket_t) + E_PL_DIAGNOSTIC_NACK_REQ_MIN_LEN))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header;
    
    DoipParserHeader(data, dataLen, &header);
    if (E_PL_TYPE_DIAG_NEGATIVE_ACK_RES != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

/**
 * @name DoipGenMessage8003
 * @brief 生成DOIP协议的消息，用于响应UDS请求并返回否定应答。
 * @param sourceLa 源逻辑地址，16位无符号整数。
 * @param targetLa 目标逻辑地址，16位无符号整数。
 * @param code 否定应答代码，8位无符号整数。
 * @param udsData UDS数据部分，指向一个无符号字符数组的指针。
 * @param udsLen UDS数据长度，无符号整数。
 * @param data 指向用于存储生成消息的缓冲区的指针。
 * @param dataLen 指向存储消息长度的变量的指针。
 * @return 函数执行状态，RET_OK表示成功，RET_ERR表示错误。
 */
int32_t DoipGenMessage8003(uint32_t sourceLa, 
                            uint32_t targetLa, 
                            uint8_t code, 
                            const uint8_t *udsData, 
                            uint32_t udsLen, 
                            uint8_t *data, 
                            uint32_t *dataLen)
{
    int32_t ret = RET_ERR;
    if (((udsLen > 0) && !udsData) || !data || !dataLen || (*dataLen < sizeof(DoipPacket_t) + sizeof(DiagnosticMessageNack_t) + udsLen))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipPacket_t *packet = (DoipPacket_t *)data;
    DoipMsgHeader_t header;
    DoipGenerateHeader(E_PL_TYPE_DIAG_NEGATIVE_ACK_RES, sizeof(DiagnosticMessageNack_t) + udsLen, &header);
    
    packet->protocal = header.protocalType;
    packet->reverse = header.typeReverse;
    packet->type[0] = (header.payloadType >> 8) & 0xff;
    packet->type[1] = header.payloadType & 0xff;
    packet->length[0]= (header.payloadLen >> 24) & 0xff;
    packet->length[1] = (header.payloadLen >> 16) & 0xff;
    packet->length[2] = (header.payloadLen >> 8) & 0xff;
    packet->length[3] = header.payloadLen & 0xff;
    DiagnosticMessageNack_t *diagNackPL = (DiagnosticMessageNack_t *)packet->payload;
    diagNackPL->sourceAddr[0] = (sourceLa >> 8) & 0xff;
    diagNackPL->sourceAddr[1] = sourceLa & 0xff;
    diagNackPL->targetAddr[0] = (targetLa >> 8) & 0xff;
    diagNackPL->targetAddr[1] = targetLa & 0xff;
    diagNackPL->code = code;
    if (udsLen > 0)
    {
        memcpy(diagNackPL->userdata, udsData, udsLen);
    }
    *dataLen = sizeof(DoipPacket_t) + sizeof(DiagnosticMessageNack_t) + udsLen;
    ret = RET_OK;
    return ret;
}


/**
 * @name DoipCheckMessage8003
 * @brief 检查Doip消息是否符合8003错误码的格式和内容。
 * @param data 指向Doip消息数据的指针。
 * @param dataLen Doip消息数据的长度。
 * @return 如果消息符合要求返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipCheckMessage8003(const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!data || (dataLen < sizeof(DoipPacket_t) + E_PL_DIAGNOSTIC_NACK_REQ_MIN_LEN))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header;
    
    DoipParserHeader(data, dataLen, &header);

    if (E_PL_TYPE_DIAG_NEGATIVE_ACK_RES != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    if (E_PL_DIAGNOSTIC_NACK_REQ_MIN_LEN > header.payloadLen)
    {
        LOG_Error("%s PL len check fail,len:%d.", __FUNCTION__, header.payloadLen);
        return ret;        
    }
    ret = RET_OK;
    return ret;
}