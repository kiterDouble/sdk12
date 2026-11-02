
#include "message_proc.h"


/**
 * @name DoipCheckHeader
 * @brief 检查DoIP消息头是否合法。
 * @param data 指向DoIP消息数据的指针。
 * @param dataLen 消息数据的长度。
 * @param err 错误码的输出参数，用于返回错误代码。
 * @return RET_OK表示成功，RET_ERR表示失败。
 */
int32_t DoipCheckHeader(const uint8_t *data, uint32_t dataLen, uint32_t *err)
{
    int32_t ret = RET_OK;
    int32_t i = 0;
    static const CheckLen_t checks[] = 
    {
        {E_PL_TYPE_HEADER_NEGTIVE_ACK, DoipCheckMessage0000}, 
        {E_PL_TYPE_VEHICLE_INDENTIFY_REQ, DoipCheckMessage0001}, 
        {E_PL_TYPE_VEHICLE_INDENTIFY_EID_REQ, DoipCheckMessage0002}, 
        {E_PL_TYPE_VEHICLE_INDENTIFY_VIN_REQ, DoipCheckMessage0003}, 
        {E_PL_TYPE_ANNOUNCE_OR_IDENTIFYRES_RES, DoipCheckMessage0004}, 
        {E_PL_TYPE_ROUTING_ACTIVATION_REQ, DoipCheckMessage0005}, 
        {E_PL_TYPE_ROUTING_ACTIVE_RES, DoipCheckMessage0006}, 
        {E_PL_TYPE_ALIVE_CHECK_REQ, DoipCheckMessage0007}, 
        {E_PL_TYPE_ALIVE_CHECK_RES, DoipCheckMessage0008}, 
        {E_PL_TYPE_ENTITY_STATUS_REQ, DoipCheckMessage4001}, 
        {E_PL_TYPE_ENTITY_STATUS_RES, DoipCheckMessage4002}, 
        {E_PL_TYPE_POWER_MODE_REQ, DoipCheckMessage4003}, 
        {E_PL_TYPE_POWER_MODE_RES, DoipCheckMessage4004}, 
        {E_PL_TYPE_DIAGNOSTIC_REQ, DoipCheckMessage8001}, 
        {E_PL_TYPE_DIAG_POSITIVE_ACK_RES, DoipCheckMessage8002}, 
        {E_PL_TYPE_DIAG_NEGATIVE_ACK_RES, DoipCheckMessage8003}        
    };
    DoipMsgHeader_t header;

    DoipParserHeader(data, dataLen, &header);

    if ((header.protocalType != 0x02) || (header.typeReverse != 0xfd))
    {
        LOG_Error("%s err:%d.", __FUNCTION__, *err);
        *err = E_GDHNAE_INCORRECT_PATTERN_FORMAT;
        ret = RET_ERR;
        return ret;
    }

    for (i = 0; i < sizeof(checks) / sizeof(CheckLen_t); i++)
    {
        if (header.payloadType == checks[i].type)
        {
            if (RET_ERR == checks[i].checkLenCb(data, dataLen))
            {
                LOG_Error("%s type:%x, len:%d, err:%d.", __FUNCTION__, header.payloadType, header.payloadLen, *err);
                *err = E_GDHNAE_INVALID_PAYLOAD_LENGTH;
                ret = RET_ERR;
                return ret;
            }
            break;
        }
    }

    if (i == sizeof(checks) / sizeof(CheckLen_t))
    {
        LOG_Error("%s type:%d, err:%d.", __FUNCTION__, header.payloadType, *err);
        *err = E_GDHNAE_UNKNOWN_PAYLOAD_TYPE;
        ret = RET_ERR;
    }
    
    return ret;
}