//
// Created by Jaxsen on 2025/5/7.
//
#include "stdlib.h"
#include "srTspCallbackRegister.h"
#include "srTspFWLog.h"
#include "srTspFWMqttClient.h"

static StBaseInfoCallbacks_t g_baseInfoCallbacks = {0};

static int CheckCallbacksValid(const StBaseInfoCallbacks_t *cbs)
{
    if (!cbs->getSn)
    {
        SR_TSP_LOG_E("CheckCallbacksValid: getSn callback is NULL");
        return RET_ERR;
    }
    if (!cbs->getSim)
    {
        SR_TSP_LOG_E("CheckCallbacksValid: getSim callback is NULL");
        return RET_ERR;
    }
    if (!cbs->getImei)
    {
        SR_TSP_LOG_E("CheckCallbacksValid: getImei callback is NULL");
        return RET_ERR;
    }
    if (!cbs->getIccid)
    {
        SR_TSP_LOG_E("CheckCallbacksValid: getIccid callback is NULL");
        return RET_ERR;
    }
    if (!cbs->getProjectName)
    {
        SR_TSP_LOG_E("CheckCallbacksValid: getProjectName callback is NULL");
        return RET_ERR;
    }
    if (!cbs->getVin)
    {
        SR_TSP_LOG_E("CheckCallbacksValid: getVin callback is NULL");
        return RET_ERR;
    }
    // if (!cbs->getUploadToken) {
    //     SR_TSP_LOG_E("CheckCallbacksValid: getUploadToken callback is NULL");
    //     return RET_ERR;
    // }
    if (!cbs->getHwVersion)
    {
        SR_TSP_LOG_E("CheckCallbacksValid: getHwVersion callback is NULL");
        return RET_ERR;
    }
    if (!cbs->getSwVersionMain)
    {
        SR_TSP_LOG_E("CheckCallbacksValid: getSwVersionMain callback is NULL");
        return RET_ERR;
    }
    if (!cbs->getSwVersionSecondary)
    {
        SR_TSP_LOG_E("CheckCallbacksValid: getSwVersionSecondary callback is NULL");
        return RET_ERR;
    }
    if (!cbs->getSwVersionConfiguration)
    {
        SR_TSP_LOG_E("CheckCallbacksValid: getSwVersionConfiguration callback is NULL");
        return RET_ERR;
    }
    return RET_OK;
}

int TspBaseInfoCheckCallbacksValid(void)
{
    return CheckCallbacksValid(&g_baseInfoCallbacks);
}

int TspBaseInfoGetSn(char *buffer, int len)
{
    return g_baseInfoCallbacks.getSn(buffer, len);
}

int TspBaseInfoRegisterCallbacks(const StBaseInfoCallbacks_t *cbs)
{
    if (cbs == NULL)
    {
        SR_TSP_LOG_E("TspBaseInfoRegisterCallbacks: input is NULL");
        return RET_ERR;
    }
    if (CheckCallbacksValid(cbs) != RET_OK)
    {
        SR_TSP_LOG_E("TspBaseInfoRegisterCallbacks: input callbacks invalid");
        return RET_ERR;
    }
    g_baseInfoCallbacks = *cbs;
    return RET_OK;
}

int TspBaseInfoGetCallbacks(StBaseInfoCallbacks_t *cbsOut)
{
    if (cbsOut == NULL)
    {
        SR_TSP_LOG_E("TspBaseInfoGetCallbacks: output pointer is NULL");
        return RET_ERR;
    }
    *cbsOut = g_baseInfoCallbacks;
    return RET_OK;
}
