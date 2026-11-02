#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ql_type.h"
#include "ql_data_call.h"
#include "ql_net_common.h"
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_dial.h"

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

void DialCallback(int callId, 
                QL_NET_DATA_CALL_STATUS_E preStatus, 
                ql_data_call_status_t *status)
{
    HalModemNetInfo_t netInfo;
    uint8_t preDialStatus = 0;

    if (!status)
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

    if ((QL_NET_DATA_CALL_STATUS_PARTIAL_V4_CONNECTED == preStatus) 
        || (QL_NET_DATA_CALL_STATUS_CONNECTED == preStatus))
    {
        preDialStatus = 1;
    }

    if ((QL_NET_DATA_CALL_STATUS_PARTIAL_V4_CONNECTED == status->call_status) 
        || (QL_NET_DATA_CALL_STATUS_CONNECTED == status->call_status))
    {
        netInfo.dialStatus = 1;
    }

    strcpy((char *)netInfo.name, status->device);
    if (status->has_addr)
    {
        strcpy((char *)netInfo.ip, status->addr.addr);
        strcpy((char *)netInfo.gateway, status->addr.gateway);
        strcpy((char *)netInfo.priDns, status->addr.dnsp);
        strcpy((char *)netInfo.secDns, status->addr.dnss);
    }

    if ((gDialPriv.dialCb) && (preDialStatus != netInfo.dialStatus))
    {
        gDialPriv.dialCb(callId, &netInfo);
    }
}

int32_t HalModemDialInit(void)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_DIAL_INITIALED == gDialPriv.initialed)
    {
        LOG_Debug("%s have initialed.", __FUNCTION__);
        ret = RET_OK;        
        return ret;
    }

    memset(&gDialPriv, 0x0, sizeof(HalModemDialPriv_t));
    gDialPriv.initialed = HAL_MODEM_DIAL_UNINITIALED;
    gDialPriv.isStarted = HAL_MODEM_DIAL_UNSTARTED;
    retVal = ql_data_call_init();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_data_call_init failed.", __FUNCTION__);
        return ret;
    }
    retVal = ql_data_call_set_status_ind_cb(DialCallback);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_data_call_set_status_ind_cb failed.", __FUNCTION__);
        ql_data_call_deinit();
        return ret;
    }
    gDialPriv.initialed = HAL_MODEM_DIAL_INITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemDialDeinit(void)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }


    retVal = ql_data_call_deinit();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_data_call_deinit failed.", __FUNCTION__);
        return ret;
    }
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
    int32_t retVal = -1;
    int32_t i = 0;
    int32_t len = gDialPriv.dialParam.apnNum;
    const int32_t defaultId = 1;

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

    int32_t isErr = 0;
    ql_data_call_apn_config_t apnCfg;
    memset(&apnCfg, 0, sizeof(apnCfg));
    strncpy(apnCfg.apn_name, "DEFAULT", sizeof(apnCfg.apn_name));
    apnCfg.ip_ver = QL_NET_IP_VER_V4;

    retVal = ql_data_call_set_apn_config(defaultId, &apnCfg);
    if(QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_data_call_set_apn_config failed, default APN1, ret=%d", __FUNCTION__, retVal);
        return ret;
    }

    ql_data_call_param_t *param = ql_data_call_param_alloc();
    if(param == NULL)
    {
        LOG_Error("%s ql_data_call_param_alloc fail.", __FUNCTION__);
        return ret;
    }

    for (; i < len; i++)
    {
        int32_t apnid = defaultId + 1 +i;//APN1 default, APN2~4 for user.
        int32_t callId = gDialPriv.dialParam.apnInfo[i].callId;
        int32_t intervalList[20] = {0,};

        memset(&apnCfg, 0, sizeof(apnCfg));
        memcpy(apnCfg.apn_name, 
            (char*)gDialPriv.dialParam.apnInfo[i].apnName, 
            strlen((char*)gDialPriv.dialParam.apnInfo[i].apnName));
        memcpy(apnCfg.username, 
            (char*)gDialPriv.dialParam.apnInfo[i].userName, 
            strlen((char*)gDialPriv.dialParam.apnInfo[i].userName));
        memcpy(apnCfg.password, 
            (char*)gDialPriv.dialParam.apnInfo[i].userPasswd, 
            strlen((char*)gDialPriv.dialParam.apnInfo[i].userPasswd));
        apnCfg.ip_ver = QL_NET_IP_VER_V4;

        retVal = ql_data_call_set_apn_config(apnid, &apnCfg);
        if(QL_ERR_OK != retVal)
        {
            LOG_Error("%s ql_data_call_set_apn_config failed, %d, ret=%d", __FUNCTION__, apnid, retVal);
            isErr = 1;
            break;
        }

        retVal = ql_data_call_create(callId, (char*)gDialPriv.dialParam.apnInfo[i].apnName, 0);
        if(QL_ERR_OK != retVal)
        {
            LOG_Error("%s ql_data_call_create fail, call_id=%d, ret=%d", __FUNCTION__, callId, retVal);
            isErr = 1;
            break;
        }

        ql_data_call_param_init(param); 
        ql_data_call_param_set_apn_id(param, apnid);
        ql_data_call_param_set_ip_version(param, QL_NET_IP_VER_V4);
        ql_data_call_param_set_reconnect_mode(param, QL_NET_DATA_CALL_RECONNECT_NORMAL);
        intervalList[0] = 20;
        ql_data_call_param_set_reconnect_interval(param, intervalList, 1);

        retVal = ql_data_call_config(callId, param);
        if(QL_ERR_OK != retVal)
        {
            LOG_Error("%s ql_data_call_config, call_id=%d, ret=%d", __FUNCTION__, callId, retVal);
            isErr = 1;
            break;
        }

        LOG_Debug("Start data call : apn1");
        retVal = ql_data_call_start(callId);
        if(QL_ERR_OK != retVal)
        {
            LOG_Error("%s ql_data_call_start, call_id=%d, ret=%d", __FUNCTION__, callId, retVal);
            isErr = 1;
            break;
        }
    }

    ql_data_call_param_free(param);
    if (isErr)
    {
        int32_t j = 0;

        for (; j < i; j++)
        {
            ql_data_call_stop(gDialPriv.dialParam.apnInfo[i].callId);
            
        }
        LOG_Error("%s isErr true, return", __FUNCTION__);
        return ret;
    }
    
    gDialPriv.isStarted = HAL_MODEM_DIAL_STARTED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemDialStop(void)
{
    int32_t ret = RET_ERR;
    int32_t i = 0;
    int32_t len = gDialPriv.dialParam.apnNum;

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
        int32_t callId = gDialPriv.dialParam.apnInfo[i].callId;
        ql_data_call_stop(callId);
    }

    gDialPriv.isStarted = HAL_MODEM_DIAL_UNSTARTED;
    ret = RET_OK;
    return ret;
}