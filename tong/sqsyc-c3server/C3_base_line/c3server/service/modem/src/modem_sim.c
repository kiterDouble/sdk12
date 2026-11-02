#include "modem_sim.h"

/**
 * @name ModemSrvGetImsiProc
 * @brief 获取IMSI（国际移动用户识别码）的处理函数
 * @param reqData 请求数据的指针，通常为NULL或特定的请求命令
 * @param reqLen 请求数据的长度，对于此函数通常为0
 * @param respData 响应数据的输出缓冲区指针，用于存放返回的IMSI信息
 * @param respLen 响应数据长度的输出指针，用于指定返回的IMSI信息长度
 * @param userdata 用户数据的指针，用于传递上下文信息，例如模组服务的内部状态
 * @return 函数执行状态，RET_OK表示成功，其他值表示失败
 */
int32_t ModemSrvGetImsiProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
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

    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_SIM].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }

    uint8_t imsi[64] = {0x0,};
    uint8_t len = (uint8_t)sizeof(imsi);

    res->result = HalModemSimGetImsi(imsi, len);

    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemSimGetImsi fail.", __FUNCTION__);
        return ret;
    }

    res->dataLen  = strlen((void*) imsi);
    (void)strcpy((void*)res->data, (void*)imsi);
    res->data[res->dataLen] = 0x0;
    *respLen += (uint16_t)res->dataLen;
    ret = RET_OK;
    return ret;
}

/**
 * @name ModemSrvGetIccidProc
 * @brief 获取ICCID（国际移动客户识别码）的处理函数
 * @param reqData 请求数据的指针，通常为NULL或特定的请求命令
 * @param reqLen 请求数据的长度，对于此函数通常为0
 * @param respData 响应数据的输出缓冲区指针，用于存放返回的ICCID信息
 * @param respLen 响应数据长度的输出指针，用于指定返回的ICCID信息长度
 * @return 函数执行状态，RET_OK表示成功，其他值表示失败
 */
int32_t ModemSrvGetIccidProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
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
    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_SIM].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }

    uint8_t iccid[64]={0x0,};
    uint8_t len = (uint8_t)sizeof(iccid);

    res->result = HalModemSimGetIccid(iccid, len);

    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemSimGetIccid fail.", __FUNCTION__);
        return ret;
    }

    res->dataLen  = strlen((void*) iccid);
    (void)strcpy((void*)res->data, (void*)iccid);
    *respLen += (uint16_t)res->dataLen;
    ret = RET_OK;
    return ret;    
}

/**
 * @name ModemSrvGetMsisdnProc
 * @brief 获取MSISDN（移动用户ISDN号码）
 * @param reqData 请求数据指针，用于获取MSISDN的请求信息
 * @param reqLen 请求数据长度，表示请求信息的大小
 * @param respData 响应数据指针，用于存储处理结果及MSISDN信息
 * @param respLen 响应数据长度指针，用于指示响应缓冲区的大小及实际使用情况
 * @param userdata 用户数据指针，指向ModemSrvData_t类型的服务器数据
 * @return int32_t 返回处理结果，RET_OK表示成功，RET_ERR表示失败
 */
int32_t ModemSrvGetMsisdnProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
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
    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_SIM].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }

    uint8_t msisdn[86]={0x0,};
    uint8_t len = (uint8_t)sizeof(msisdn);

    res->result = HalModemSimGetMsisdn(msisdn, len);

    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemSimGetIccid fail.", __FUNCTION__);
        return ret;
    }

    res->dataLen  = strlen((void*) msisdn);
    (void)strcpy((void*)res->data, (void*)msisdn);
    res->data[res->dataLen] = 0x0;
    *respLen += (uint16_t)res->dataLen;
    ret = RET_OK;
    return ret;    
}