#include "message_4004.h"


/**
 * @name DoipParserMessage4004
 * @brief 解析4004消息以获取电源模式信息
 * @param data 指向DOIP消息数据的指针
 * @param dataLen DOIP消息数据的长度
 * @param resPL 指向用于存储解析结果的PowerModeInfoRes_t结构体的指针
 * @return 若成功解析消息，则返回RET_OK；否则返回RET_ERR。
 */
int32_t DoipParserMessage4004(const uint8_t *data, uint32_t dataLen, PowerModeInfoRes_t *resPL)
{
    
    int32_t ret = RET_ERR;
    if (!data || (dataLen < (sizeof(DoipPacket_t) + (uint32_t)E_PL_POWER_MODE_RES_LEN)) || !resPL)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header = {0};
    
    (void)DoipParserHeader(data, dataLen, &header);
    if ((uint16_t)E_PL_TYPE_POWER_MODE_RES != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    const DoipPacket_t *packet = (const DoipPacket_t *)((const void*)data);
    resPL->powerMode = packet->payload[0];
    ret = RET_OK;
    return ret;
}

/**
 * @name DoipGenMessage4004
 * @brief 生成针对4004报文的DOIP消息
 *        本函数用于根据给定的电源模式信息响应，生成一个包括DOIP头和电源模式信息的完整DOIP消息。
 * @param resPL 指向PowerModeInfoRes_t结构的指针，包含电源模式的响应信息。
 * @param data 指向用于存储生成消息的缓冲区的指针。
 * @param dataLen 指向存储缓冲区长度的变量的指针，函数执行后，此处将更新为实际使用的缓冲区长度。
 * @return 返回生成消息的操作结果，成功时返回RET_OK，错误时返回其他错误码。
 */
int32_t DoipGenMessage4004(const PowerModeInfoRes_t *resPL, uint8_t *data, uint32_t *dataLen)
{
    int32_t ret = RET_ERR;
    if (!resPL ||!data || !dataLen || (*dataLen < (sizeof(DoipPacket_t) + (uint32_t)E_PL_POWER_MODE_RES_LEN)))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header = {0};
    (void)DoipGenerateHeader(E_PL_TYPE_POWER_MODE_RES, E_PL_POWER_MODE_RES_LEN, &header);
    ret = DoipGeneratePacket(&header, &resPL->powerMode, data, dataLen);
    return ret;
}

/**
 * @name DoipCheckMessage4004
 * @brief 检查4004消息是否有效
 *        本函数用于校验基于DOIP协议的4004消息的内容是否符合预期的格式和长度。
 *        主要通过解析消息头来执行校验。
 * @param data 指向待检查DOIP消息数据的指针。
 * @param dataLen 待检查DOIP消息数据的长度。
 * @return 返回校验结果，RET_OK表示校验通过，RET_ERR表示校验失败。
 */
int32_t DoipCheckMessage4004(const uint8_t *data, uint32_t dataLen)
{
    int32_t ret = RET_ERR;

    if (!data || (dataLen < (sizeof(DoipPacket_t) + (uint32_t)E_PL_POWER_MODE_RES_LEN)))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    DoipMsgHeader_t header = {0};
    
    (void)DoipParserHeader(data, dataLen, &header);

    if ((uint16_t)E_PL_TYPE_POWER_MODE_RES != header.payloadType)
    {
        LOG_Error("%s PL type not mask,Type:%d.", __FUNCTION__, header.payloadType);
        return ret;
    }

    if ((uint32_t)E_PL_POWER_MODE_RES_LEN != header.payloadLen)
    {
        LOG_Error("%s PL len check fail,len:%d.", __FUNCTION__, header.payloadLen);
        return ret;        
    }
    ret = RET_OK;
    return ret;
}