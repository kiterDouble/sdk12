#include "modem_dms.h"

/**
 * @name ModemsrvGetImeiProc
 * @brief 获取IMEI号的处理函数
 * @param reqData 请求数据的指针
 * @param reqLen 请求数据的长度
 * @param respData 响应数据的指针
 * @param respLen 响应数据长度的指针
 * @param userdata 用户数据的指针
 * @return int32_t 返回操作结果，RET_OK表示成功，其他值表示失败
 */
int32_t ModemsrvGetImeiProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
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
    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_DMS].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }

    uint8_t imei[64]={0x0,};;
    uint8_t len = (uint8_t)sizeof(imei);

    res->result = HalModemDmsGetImei(imei, len);

    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemDmsGetImei fail.", __FUNCTION__);
        return ret;
    }

    res->dataLen  = strlen((void*) imei);
    (void)strcpy((void*) res->data, (void*) imei);
    res->data[res->dataLen] = 0x0U;
    *respLen += (uint16_t)res->dataLen;
    ret = RET_OK;
    return ret;
}

/**
 * @name ModemSrvGetAirplaneModeProc
 * @brief 获取飞行模式状态的处理函数
 * @param reqData 指向请求数据的指针。该数据由调用者提供，用于执行特定操作。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。该函数将在此缓冲区中返回操作的结果。
 * @param respLen 指向响应数据长度的指针。函数会更新此参数以指示返回的响应数据长度。
 * @param userdata 用户数据的指针。通常用于存储与调用此函数的上下文相关的数据。
 * @return 执行结果的错误码。RET_OK 表示成功，其他值表示错误。
 */
int32_t ModemSrvGetAirplaneModeProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
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
    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_DMS].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }

    HalModemDmsMode_e mode;

    res->result = HalModemDmsGetMode(&mode);

    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemDmsGetMode fail.", __FUNCTION__);
        return ret;
    }

    res->dataLen  = 1;
    res->data[0] = (uint8_t)mode;
    *respLen += (uint16_t)res->dataLen;
    ret = RET_OK;
    return ret;
}

/**
 * @name ModemSrvSetAirplaneModeProc
 * @brief 设置飞行模式的处理函数
 *        该函数用于处理设置飞行模式的请求，通过调用HAL来实现模式的切换，并返回操作结果。
 * @param reqData 指向请求数据的指针。请求数据中应包含要设置的飞行模式。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。响应数据中应包含操作结果。
 * @param respLen 指向响应数据长度的指针。用于返回响应数据的实际长度。
 * @param userdata 用户数据指针，通常用于传递模块内部状态或配置。
 * @return 操作结果，RET_OK表示成功，其他值表示失败。
 */
int32_t ModemSrvSetAirplaneModeProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
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
    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_DMS].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }

    HalModemDmsMode_e mode = (HalModemDmsMode_e)reqData[0];

    res->result = HalModemDmsSetMode(mode);

    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemDmsSetMode fail.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

/**
 * @name ModemSrvGetQoosEnableProc
 * @brief 获取Qoos启用状态的处理函数
 * @param reqData 指向请求数据的指针。该数据由调用者提供，用于执行特定操作。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。该函数将在此缓冲区中返回操作的结果。
 * @param respLen 指向响应数据长度的指针。函数会更新此参数以指示返回的响应数据长度。
 * @return 操作结果，RET_OK表示成功，其他值表示失败。
 */
int32_t ModemSrvGetQoosEnableProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
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
    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_DMS].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }

    HalModemDmsQoosEnableStatusType_e enable;

    res->result = HalModemDmsGetQoosEnableStatus(&enable);

    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemDmsGetQoosEnableStatus fail.", __FUNCTION__);
        return ret;
    }

    res->dataLen  = 1;
    res->data[0] = (uint8_t)enable;
    *respLen += (uint16_t)res->dataLen;
    ret = RET_OK;
    return ret;
}

/**
 * @name ModemSrvSetQoosEnableProc
 * @brief 设置QOOS使能处理函数
 * @param reqData 指向请求数据的指针。该数据由调用者提供，用于执行特定操作。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。该函数将在此缓冲区中返回操作的结果。
 * @param respLen 指向响应数据长度的指针。函数会更新此参数以指示返回的响应数据长度。
 * @return 操作结果，RET_OK表示成功，其他值表示失败。
 */
int32_t ModemSrvSetQoosEnableProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
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
    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_DMS].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }

    HalModemDmsQoosEnableStatusType_e enable = (HalModemDmsQoosEnableStatusType_e)reqData[0];

    res->result = HalModemDmsSetQoosEnableStatus(enable);

    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemDmsSetQoosEnableStatus fail.", __FUNCTION__);        
        return ret;
    }

    ret = RET_OK;
    return ret;
}

/**
 * @name ModemSrvGetQoosCfgProc
 * @brief 获取Qoos配置的处理函数
 * @param reqData 指向请求数据的指针。该数据由调用者提供，用于执行特定操作。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。该函数将在此缓冲区中返回操作的结果。
 * @param respLen 指向响应数据长度的指针。函数会更新此参数以指示返回的响应数据长度。
 * @return 操作结果，RET_OK表示成功，其他值表示失败。
 */
int32_t ModemSrvGetQoosCfgProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
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
    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_DMS].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }

    HalModemDmsQoosConfig_t cfg = {0};

    res->result = HalModemDmsGetQoosConfig(&cfg);

    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemDmsGetQoosConfig fail.", __FUNCTION__);
        return ret;
    }

    ModemSrvQoosCfg_t *resCfg = (ModemSrvQoosCfg_t *)((void*)res->data);
    resCfg->timeSec1 = cfg.timeSec1;
    resCfg->timeSec2 = cfg.timeSec2;
    resCfg->timeSec3 = cfg.timeSec3;
    res->dataLen = sizeof(ModemSrvQoosCfg_t);
    *respLen += (uint16_t)res->dataLen;
    ret = RET_OK;
    return ret;
}

/**
 * @name ModemSrvSetQoosCfgProc
 * @brief 设置Qoos配置的处理函数
 * @param reqData 指向请求数据的指针。该数据由调用者提供，用于执行特定操作。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针。该函数将在此缓冲区中返回操作的结果。
 * @param respLen 指向响应数据长度的指针。函数会更新此参数以指示返回的响应数据长度。
 * @return 操作结果，RET_OK表示成功，其他值表示失败。
 */
int32_t ModemSrvSetQoosCfgProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
    ModemSrvData_t *srvData = (ModemSrvData_t *) userdata;
    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *)((void*)respData);
    int32_t ret = RET_ERR;

    if (!srvData || ((reqLen > 0) && !reqData) || !res || !respLen)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    res->result = RET_ERR;
    res->dataLen = 0;
    *respLen = (uint16_t)sizeof(ModemSrvRpcRes_t);
    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_DMS].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }
    const ModemSrvQoosCfg_t *const resCfg = (const ModemSrvQoosCfg_t *const)((const void *const)reqData);
    HalModemDmsQoosConfig_t cfg = {0};
    
    cfg.timeSec1 = resCfg->timeSec1;
    cfg.timeSec2 = resCfg->timeSec2;
    cfg.timeSec3 = resCfg->timeSec3;
    res->result = HalModemDmsSetQoosConfig(&cfg);
    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemDmsSetQoosConfig fail.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

/**
 * @name ModemSrvGetCpuInfoProc
 * @brief 获取CPU温度和占用率的处理函数
 * @param reqData 指向请求数据的指针
 * @param reqLen 请求数据的长度
 * @param respData 指向响应数据的指针
 * @param respLen 指向响应数据长度的指针
 * @param userdata 用户数据的指针
 * @return 操作结果，RET_OK表示成功，其他值表示失败
 */
int32_t ModemSrvGetCpuInfoProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
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
    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_DMS].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }

    ModemSrvCpuInfo_t *cpuInfo = (ModemSrvCpuInfo_t *)((void*)res->data);
    cpuInfo->cpuTemperature = 0.0f;
    cpuInfo->cpuOccupancy = 0.0f;

    res->result = HalModemDmsGetCpuTemperature(&cpuInfo->cpuTemperature);
    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemDmsGetCpuTemperature fail.", __FUNCTION__);
        return ret;
    }

    res->result = HalModemDmsGetCpuOccupancy(&cpuInfo->cpuOccupancy);
    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemDmsGetCpuOccupancy fail.", __FUNCTION__);
        return ret;
    }

    res->dataLen = sizeof(ModemSrvCpuInfo_t);
    *respLen += (uint16_t)res->dataLen;
    ret = RET_OK;
    return ret;
}