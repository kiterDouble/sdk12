#include "modem_nw.h"

/**
 * @name ModemSrvGetExtSignalInfoProc
 * @brief 获取详细信号处理函数
 *        该函数用于从Hal Modem函数获取详细的信号信息，并将这些信息封装在响应数据中返回给调用者。
 * @param reqData 指向请求数据的指针。请求数据应包含请求的类型和参数。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。函数将在这里返回获取的信号信息。
 * @param respLen 指向响应数据长度的指针。函数会更新这个值以表示返回的信号信息的实际长度。
 * @param userdata 用户数据的指针。通常用于传递与调制解调器服务相关的上下文信息。
 * @return 函数执行状态码。RET_OK表示成功，其他值表示错误。
 */
int32_t ModemSrvGetExtSignalInfoProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
    ModemSrvData_t *srvData = (ModemSrvData_t *) userdata;
    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *)((void*)respData);
    int32_t ret = RET_ERR;

    if (!srvData || ((reqLen > 0U) && !reqData) || !res || !respLen)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    res->result = RET_ERR;
    res->dataLen = 0U;
    *respLen = (uint16_t)sizeof(ModemSrvRpcRes_t);
    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_NW].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }

    HalModemNwExtSigInfo_t sigInfo;;

    res->result = HalModemNwGetSignalInfo(&sigInfo);

    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemNwGetSignalInfo fail.", __FUNCTION__);
        return ret;
    }

    ModemSrvExtSigInfo_t *resSigInfo = (ModemSrvExtSigInfo_t *)((void*)res->data);
    memcpy(resSigInfo, &sigInfo, sizeof(ModemSrvExtSigInfo_t));
    // resSigInfo->lteSiginfo.sigType = (uint8_t)sigInfo.lteSigInfo.sigType;
    // resSigInfo->lteSiginfo.rssi = (uint8_t)sigInfo.lteSigInfo.rssi;
    // resSigInfo->lteSiginfo.rsrp = (uint8_t)sigInfo.lteSigInfo.rsrp;
    // resSigInfo->lteSiginfo.rsrq = (uint8_t)sigInfo.lteSigInfo.rsrq;
    // resSigInfo->lteSiginfo.sinr = (uint8_t)sigInfo.lteSigInfo.sinr;
    // resSigInfo->lteSiginfo.rsrq = (uint8_t)sigInfo.lteSigInfo.ecio;
    // resSigInfo->lteSiginfo.sinr = (uint8_t)sigInfo.lteSigInfo.rscp;
    // resSigInfo->lteSiginfo.level = (uint8_t)sigInfo.lteSigInfo.level;
    
    res->dataLen = sizeof(ModemSrvExtSigInfo_t);
    *respLen += (uint16_t)res->dataLen;
    ret = RET_OK;
    return ret;
}

/**
 * @name ModemSrvGetOperatorProc
 * @brief 获取运营商信息处理函数
 *        该函数用于从Hal Modem函数获取运营商信息，并将这些信息封装在响应数据中返回给调用者。
 * @param reqData 指向请求数据的指针。请求数据应包含请求的类型和参数。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。函数将在这里返回获取的运营商信息。
 * @param respLen 指向响应数据长度的指针。函数会更新这个值以表示返回的运营商信息的实际长度。
 * @param userdata 用户数据的指针。通常用于传递与调制解调器服务相关的上下文信息。
 * @return 函数执行状态码。RET_OK表示成功，其他值表示错误。
 */ 
int32_t ModemSrvGetOperatorProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
    ModemSrvData_t *srvData = (ModemSrvData_t *) userdata;
    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *)((void*)respData);
    int32_t ret = RET_ERR;

    if (!srvData || ((reqLen > 0U) && !reqData) || !res || !respLen)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    res->result = RET_ERR;
    res->dataLen = 0;
    *respLen = (uint16_t)sizeof(ModemSrvRpcRes_t);

    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_NW].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }

    uint8_t operator[64] = {0x0,};;

    res->result = HalModemNwGetOperatorInfo(operator);

    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemNwGetOperatorInfo fail.", __FUNCTION__);
        return ret;
    }

    res->dataLen  = strlen((void*) operator);
    (void)strcpy((void*)res->data, (void*)operator);
    res->data[res->dataLen] = 0x0U;
    *respLen += (uint16_t)res->dataLen;
    ret = RET_OK;
    return ret;    
}
