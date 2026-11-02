#include "message_8002.h"


/**
 * @name DoipParserMessage8002
 * @brief 解析DOIP消息8002
 *        本函数用于解析接收到的DOIP（Diagnostic over IP）消息，检查消息是否为期望的诊断确认响应。
 * @param data 指向接收到的数据包的指针。
 * @param dataLen 接收到的数据包的长度。
 * @return 如果解析成功且消息类型匹配，则返回RET_OK（定义为成功返回值）；否则返回RET_ERR（定义为错误返回值）。
 */
int32_t DoipParserMessage8002(const uint8_t *data, uint32_t dataLen)
{
    
    int32_t ret = RET_ERR;
    if (!data || (dataLen < sizeof(DoipPacket_t) + (uint32_t)E_PL_DIAGNOSTIC_ACK_REQ_MIN_LEN))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header = {0};
    
    (void)DoipParserHeader(data, dataLen, &header);
    if ((uint16_t)E_PL_TYPE_DIAG_POSITIVE_ACK_RES != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

/**
 * @name DoipGenMessage8002
 * @brief 生成DOIP 8002消息，用于响应诊断消息。
 * @param sourceLa 源逻辑地址，32位无符号整数。
 * @param targetLa 目标逻辑地址，32位无符号整数。
 * @param code 代码字段，表示特定的诊断响应或错误代码。
 * @param udsData UDS（统一诊断服务）数据，指向包含要发送的数据的缓冲区。
 * @param udsLen UDS数据长度，指定udsData中的字节数量。
 * @param data 指向用于存储生成消息的缓冲区的指针。
 * @param dataLen 指向存储消息长度的变量的指针。
 * @return 函数执行状态，RET_OK表示成功，RET_ERR表示错误。
 */
int32_t DoipGenMessage8002(uint32_t sourceLa, 
                            uint32_t targetLa, 
                            uint8_t code, 
                            const uint8_t *udsData, 
                            uint32_t udsLen, 
                            uint8_t *data, 
                            uint32_t *dataLen)
{
    int32_t ret = RET_ERR;
    if (((udsLen > 0U) && !udsData) || !data || !dataLen || (*dataLen < sizeof(DoipPacket_t) + sizeof(DiagnosticMessageAck_t) + udsLen))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipPacket_t *packet = (DoipPacket_t *)((void*)data);
    DoipMsgHeader_t header = {0};
    (void)DoipGenerateHeader(E_PL_TYPE_DIAG_POSITIVE_ACK_RES, sizeof(DiagnosticMessageAck_t) + udsLen, &header);
    
    packet->protocal = header.protocalType;
    packet->reverse = header.typeReverse;
    packet->type[0] = (uint8_t)((header.payloadType >> 8U) & 0xffU);
    packet->type[1] = (uint8_t)(header.payloadType & 0xffU);
    packet->length[0]= (uint8_t)((header.payloadLen >> 24U) & 0xffU);
    packet->length[1] = (uint8_t)((header.payloadLen >> 16U) & 0xffU);
    packet->length[2] = (uint8_t)((header.payloadLen >> 8U) & 0xffU);
    packet->length[3] = (uint8_t)(header.payloadLen & 0xffU);
    DiagnosticMessageAck_t *diagAckPL = (DiagnosticMessageAck_t *)((void*)packet->payload);
    diagAckPL->sourceAddr[0] = (uint8_t)((sourceLa >> 8U) & 0xffU);
    diagAckPL->sourceAddr[1] = (uint8_t)(sourceLa & 0xffU);
    diagAckPL->targetAddr[0] = (uint8_t)((targetLa >> 8U) & 0xffU);
    diagAckPL->targetAddr[1] = (uint8_t)(targetLa & 0xffU);
    diagAckPL->code = code;
    if (udsLen > 0U)
    {
        (void)memcpy(diagAckPL->userdata, udsData, udsLen);
    }
    *dataLen = sizeof(DoipPacket_t) + sizeof(DiagnosticMessageAck_t) + udsLen;
    ret = RET_OK;
    return ret;
}

/**
 * @name DoipCheckMessage8002
 * @brief 检查DOIP（Diagnostic Over IP）消息是否符合8002协议要求。
 * @param data 指向DOIP消息数据的指针。
 * @param dataLen DOIP消息数据的长度。
 * @return 如果消息符合要求返回RET_OK（假设为0），否则返回RET_ERR（假设为非0值）。
 */
int32_t DoipCheckMessage8002(const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!data || (dataLen < (sizeof(DoipPacket_t) + (uint32_t)E_PL_DIAGNOSTIC_ACK_REQ_MIN_LEN)))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header = {0};
    
    (void)DoipParserHeader(data, dataLen, &header);

    if ((uint16_t)E_PL_TYPE_DIAG_POSITIVE_ACK_RES != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    if ((uint32_t)E_PL_DIAGNOSTIC_ACK_REQ_MIN_LEN > header.payloadLen)
    {
        LOG_Error("%s PL len check fail,len:%d.", __FUNCTION__, header.payloadLen);
        return ret;        
    }
    ret = RET_OK;
    return ret;
}