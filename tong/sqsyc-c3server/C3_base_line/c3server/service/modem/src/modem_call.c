#include "modem_call.h"

/**
 * @name ModemSrvCallStatusHandle
 * @brief 处理Modem通话状态信息
 *        本函数用于处理来自Modem的通话状态信息，将每个通话的状态、ID和电话号码作为消息发送到所有队列。
 * 
 * @param callList 指向HalModemCallInfoList_t结构的指针，包含通话信息的列表。
 *                 该结构体中包含了通话的状态、ID和电话号码等信息。
 * 
 * @return 函数无返回值
 */
void ModemSrvCallStatusHandle(HalModemCallInfoList_t *callList)
{
    uint32_t i = 0;

    if (!callList)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    for (; i < callList->infoNum; i++)
    {
        ModemSrvCallStatus_t voice;

        (void)memset(&voice, 0x0, sizeof(ModemSrvCallStatus_t));
        voice.status = (int32_t)callList->callInfo[i].status;
        voice.callId = callList->callInfo[i].callId;
        (void)memcpy(voice.phoneNum, callList->callInfo[i].phoneNum, sizeof(voice.phoneNum));
        (void)SendMsgToAllQueue(SYS_QUE_TYPE_MODEM_CALL_NOTIFY, &voice, sizeof(ModemSrvCallStatus_t));
    }
}


/**
 * @name ModemSrvCallInit
 * @brief Modem服务通话功能初始化。
 *        该函数初始化调制解调器的通话功能，它会调用HAL的相应函数来进行底层的初始化操作。
 * @param null
 * @return 返回初始化操作的结果状态码。
 */
int32_t ModemSrvCallInit(void)
{
    return HalModemCallInit(ModemSrvCallStatusHandle);
}

/**
 * @name ModemSrvStartCallProc
 * @brief 拨打电话处理函数。
 * @param reqData 指向请求数据的指针，包含要拨打的电话号码。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针，此函数会在此处存储调用结果。
 * @param respLen 指向响应数据长度的指针，函数会更新此参数以指示返回的响应数据长度。
 * @param userdata 用户数据指针，通常用于存储模块的内部状态或配置。
 * @return 执行结果，RET_OK 表示成功，其他值表示失败。
 */
int32_t ModemSrvStartCallProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
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
    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_CALL].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }


    const uint8_t *const phoneNum = reqData;
    int32_t *callId = (int32_t *)((void*)res->data);

    res->result = HalModemCallStart(phoneNum, callId);

    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemCallStart fail.", __FUNCTION__);
        return ret;
    }

    res->dataLen  = sizeof(int32_t);
    *respLen += (uint16_t)res->dataLen;
    ret = RET_OK;
    return ret;    
}

/**
 * @name ModemSrvAcceptCallProc
 * @brief 接听电话处理函数。
 * @param reqData 指向请求数据的指针。该数据应包含要接听的通话的ID。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。该数据应包含操作结果和相关数据长度。
 * @param respLen 指向响应数据长度的指针。
 * @param userdata 用户数据的指针，通常用于传递上下文信息。
 * @return 函数执行状态码。RET_OK表示成功，其他值表示错误。
 */
int32_t ModemSrvAcceptCallProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
    ModemSrvData_t *srvData = (ModemSrvData_t *) userdata;
    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *)((void *)respData);
    int32_t ret = RET_ERR;

    if (!srvData || ((reqLen > 0U) && !reqData) || !res || !respLen)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    res->result = RET_ERR;
    res->dataLen = 0;
    *respLen = (uint16_t)sizeof(ModemSrvRpcRes_t);
    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_CALL].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }


    int32_t callId = *((const int32_t *const) ((const void *const)reqData));

    res->result = HalModemCallAnswer(callId);

    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemCallAnswer fail.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;    
}

/**
 * @name ModemSrvStopCallProc
 * @brief 挂断电话处理函数
 * @param reqData 指向请求数据的指针。该数据应包含要挂断通话的ID。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。该响应将包含操作的结果。
 * @param respLen 指向响应数据长度的指针。
 * @param userdata 用户数据的指针，通常用于传递给函数内部以供使用。
 * @return 函数执行状态码。RET_OK表示成功，其他值表示错误。
 */
int32_t ModemSrvStopCallProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
    ModemSrvData_t *srvData = (ModemSrvData_t *) userdata;
    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *)((void *)respData);
    int32_t ret = RET_ERR;

    if (!srvData || ((reqLen > 0U) && !reqData) || !res || !respLen)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    res->result = RET_ERR;
    res->dataLen = 0;
    *respLen = (uint16_t)sizeof(ModemSrvRpcRes_t);
    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_CALL].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }


    int32_t callId = *((const int32_t *const) ((const void *const) reqData));

    res->result = HalModemCallEnd(callId);

    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemCallEnd fail.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;    
}

/**
 * @name ModemSrvDeinitProc
 * @brief 反初始化Modem服务进程。
 *        该函数用于执行特定类型的调制解调器服务反初始化操作。它首先验证输入参数的有效性，
 *        然后根据请求的数据类型调用相应的反初始化回调函数，并更新服务状态。
 * @param reqData 指向请求数据的指针。请求数据应包含要执行反初始化的服务类型的标识。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据缓冲区的指针。响应数据将包含操作结果和返回的数据长度。
 * @param respLen 指向响应数据长度的指针。函数执行后，这里将更新为实际的响应数据长度。
 * @param userdata 用户数据指针，通常用于传递服务的上下文信息。
 * @return 操作结果，RET_OK 表示成功，RET_ERR 表示失败。
 */
int32_t ModemSrvDeinitProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
    ModemSrvData_t *srvData = (ModemSrvData_t *) userdata;
    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *)((void *)respData);
    int32_t ret = RET_ERR;

    if (!srvData || ((reqLen > 0U) && !reqData) || !res || !respLen)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    res->result = RET_ERR;
    res->dataLen = 0;
    *respLen = (uint16_t)sizeof(ModemSrvRpcRes_t);
    ModemSrvJobType_e type = (ModemSrvJobType_e)reqData[0];
    if (type >= E_MODEM_SRV_JOB_TYPE_MAX)
    {
        LOG_Error("%s type invalid.", __FUNCTION__);
        return ret;
    }
    
    if (srvData->jobs[type].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }
    res->result = srvData->jobs[type].deinitCb();
    
    if (res->result == RET_OK)
    {
        (void)pthread_mutex_lock(&srvData->mutex);
        srvData->jobs[type].status = E_MODEM_SRV_JOB_STATUS_UNINITIALED;
        (void)pthread_mutex_unlock(&srvData->mutex);
    }
    ret = RET_OK;
    return ret;
}