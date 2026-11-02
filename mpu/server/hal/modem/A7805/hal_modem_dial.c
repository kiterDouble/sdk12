#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_dial.h"
#include "Sunsea_Data.h"

#define HAL_MODEM_DIAL_INITIALED (0)
#define HAL_MODEM_DIAL_UNINITIALED (-1)
#define HAL_MODEM_DIAL_STARTED (0)
#define HAL_MODEM_DIAL_UNSTARTED (-1)

typedef struct
{
    int32_t initialed;
    int32_t isStarted;
    ModemCallbak dialCb;
    HalModemDialParam_t dialParam;
}HalModemDialPriv_t;

static HalModemDialPriv_t gDialPriv = 
{
    .initialed = HAL_MODEM_DIAL_UNINITIALED,
    .isStarted = HAL_MODEM_DIAL_UNSTARTED,
    .dialCb = NULL,
};

static void DialCallback(data_call_state_s *callState)
{
    HalModemNetInfo_t netInfo;

    if (!callState)
    {
        LOG_Error("%s args invalid.", __FUNCTION__);
        return;       
    }

    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return;
    }

    memset(&netInfo, 0x0, sizeof(HalModemNetInfo_t));
    netInfo.dialStatus = callState->state;
    strcpy((char *)netInfo.name, callState->ifname);
    strcpy((char *)netInfo.ip, inet_ntoa(callState->ip_addr.v4.iface_addr));
    strcpy((char *)netInfo.gateway, inet_ntoa(callState->ip_addr.v4.gtwy_addr));
    strcpy((char *)netInfo.priDns, inet_ntoa(callState->ip_addr.v4.dnsp_addr));
    strcpy((char *)netInfo.secDns, inet_ntoa(callState->ip_addr.v4.dnss_addr));

    if (gDialPriv.dialCb)
    {
        gDialPriv.dialCb(callState->profile_idx, &netInfo);
    }
}
int32_t HalModemDialInit(void)
{
    int32_t ret = RET_ERR;

    if (HAL_MODEM_DIAL_INITIALED == gDialPriv.initialed)
    {
        LOG_Debug("%s have initialed.", __FUNCTION__);
        ret = RET_OK;        
        return ret;
    }

    memset(&gDialPriv, 0x0, sizeof(HalModemDialPriv_t));
    gDialPriv.initialed = HAL_MODEM_DIAL_UNINITIALED;
    gDialPriv.isStarted = HAL_MODEM_DIAL_UNSTARTED;
    if (Sunsea_DataCall_Init(DialCallback) < 0)
    {
        LOG_Error("%s Sunsea_DataCall_Init failed.", __FUNCTION__);
        return ret;
    }

    gDialPriv.initialed = HAL_MODEM_DIAL_INITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemDialDeinit(void)
{
    int32_t ret = RET_ERR;

    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    Sunsea_DataCall_Destroy();
    memset(&gDialPriv, 0x0, sizeof(HalModemDialPriv_t));
    gDialPriv.initialed = HAL_MODEM_DIAL_UNINITIALED;
    gDialPriv.isStarted = HAL_MODEM_DIAL_UNSTARTED;    
    ret = RET_OK;
    return ret;
}

int32_t HalModemDialSetParam(const HalModemDialParam_t *dialParam)
{
    int32_t ret = RET_ERR;

    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    if (!dialParam || (dialParam->apnNum == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;        
    }

    memcpy(&gDialPriv.dialParam, dialParam, sizeof(HalModemDialParam_t));
    ret = RET_OK;
    return ret;
}

int32_t HalModemDialSetStatusCallback(ModemCallbak cb)
{
    int32_t ret = RET_ERR;
    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    gDialPriv.dialCb = cb;
    ret = RET_OK;
    return ret;
}

int32_t HalModemDialStart(void)
{
    int32_t ret = RET_ERR;
    int32_t len = gDialPriv.dialParam.apnNum;
    int32_t i = 0;

    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    if (len <= 0)
    {
        LOG_Error("%s have not set dialParam.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_DIAL_STARTED == gDialPriv.isStarted)
    {
        LOG_Error("%s have started.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }

    for (i = 0; i < len; i++)
    {
        data_profile_info_s profileInfo = {0};
        profileInfo.profile_idx = i + 1;

        memcpy(profileInfo.apn_name, 
            (char*)gDialPriv.dialParam.apnInfo[i].apnName, 
            strlen((char*)gDialPriv.dialParam.apnInfo[i].apnName));
        memcpy(profileInfo.username, 
            (char*)gDialPriv.dialParam.apnInfo[i].userName, 
            strlen((char*)gDialPriv.dialParam.apnInfo[i].userName));
        memcpy(profileInfo.password, 
            (char*)gDialPriv.dialParam.apnInfo[i].userPasswd, 
            strlen((char*)gDialPriv.dialParam.apnInfo[i].userPasswd));
        profileInfo.pdp_type = DATA_PROFILE_PDP_TYPE_IPV4;
        profileInfo.auth_proto = gDialPriv.dialParam.apnInfo[i].authType;
        if(Sunsea_DataCall_SetProfile(&profileInfo) < 0)
        {
            LOG_Error("%s Sunsea_DataCall_SetProfile fail,idx:%d, apn_name:%s,.", __FUNCTION__, i + 1, profileInfo.apn_name);
            return ret;
        }
    }

    int32_t isErr = 0;
    for (i = 0; i < len; i++)
    {
        data_call_params_s callParam = {0};
        data_call_error_e err = {0};

        callParam.profile_idx = i + 1;

        callParam.ip_family = DATA_CALL_TYPE_IPV4;
        callParam.reconnect = 1;
        
        if (Sunsea_DataCall_Start(&callParam, &err) < 0)
        {
            isErr = 1;
            LOG_Error("%s Sunsea_DataCall_Start fail.err:%d", __FUNCTION__, err);
            break;
        }
    }

    if (isErr)
    {
        int32_t j = 0;
        for (; j < i; j++)
        {
            uint8_t profileIdx = j + 1;
            data_call_error_e err = {0};

            Sunsea_DataCall_Stop(profileIdx, &err);
        }
        return ret;
    }

    gDialPriv.isStarted = HAL_MODEM_DIAL_STARTED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemDialStop(void)
{
    int32_t ret = RET_ERR;
    int32_t len = gDialPriv.dialParam.apnNum;
    int32_t i = 0;

    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_DIAL_STARTED != gDialPriv.isStarted)
    {
        LOG_Error("%s have not started.", __FUNCTION__);
        return ret;
    }

    if (len <= 0)
    {
        LOG_Error("%s have not set dialParam.", __FUNCTION__);
        return ret;
    }

    for (; i < len; i++)
    {
        uint8_t profileIdx = i + 1;
        data_call_error_e err = {0};

        Sunsea_DataCall_Stop(profileIdx, &err);
    }

    gDialPriv.isStarted = HAL_MODEM_DIAL_UNSTARTED;
    ret = RET_OK;
    return ret;
}