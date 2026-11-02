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

static void DialCallback(int callId, 
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

    (void)memset(&netInfo, 0x0, sizeof(HalModemNetInfo_t));

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
    LOG_Debug("%s preStatus:%d,status->call_status:%d,call_end_reason_type=%d,call_end_reason_code=0x%X", 
    __FUNCTION__, preStatus,status->call_status,status->call_end_reason_type,status->call_end_reason_code);    
    (void)strcpy((char *)netInfo.name, status->device);
    if (0 != status->has_addr)
    {
        (void)strcpy((char *)netInfo.ip, status->addr.addr);
        (void)strcpy((char *)netInfo.gateway, status->addr.gateway);
        (void)strcpy((char *)netInfo.priDns, status->addr.dnsp);
        (void)strcpy((char *)netInfo.secDns, status->addr.dnss);
        LOG_Debug("%s, pid:%d.", __FUNCTION__, callId);
        LOG_Debug("%s, tatus->addr.addr:%s.", __FUNCTION__, status->addr.addr);
        LOG_Debug("%s, tatus->addr.gateway:%s.", __FUNCTION__, status->addr.gateway);
        LOG_Debug("%s, tatus->addr.dnsp:%s.", __FUNCTION__, status->addr.dnsp);
        LOG_Debug("%s, tatus->addr.dnss:%s.", __FUNCTION__, status->addr.dnss);
    }

    if ((gDialPriv.dialCb) && (preDialStatus != netInfo.dialStatus))
    {
        LOG_Debug("%s dialCb called.", __FUNCTION__);
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
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);
    (void)memset(&gDialPriv, 0x0, sizeof(HalModemDialPriv_t));
    gDialPriv.initialed = HAL_MODEM_DIAL_UNINITIALED;
    gDialPriv.isStarted = HAL_MODEM_DIAL_UNSTARTED;
    retVal = ql_data_call_init();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_data_call_init failed,retVal:%d", __FUNCTION__, retVal);
        return ret;
    }
    retVal = ql_data_call_set_status_ind_cb(DialCallback);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_data_call_set_status_ind_cb failed,retVal:%d", __FUNCTION__, retVal);
        (void)ql_data_call_deinit();
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
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }


    retVal = ql_data_call_deinit();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_data_call_deinit failed,retVal:%d", __FUNCTION__, retVal);
        return ret;
    }
    (void)memset(&gDialPriv, 0x0, sizeof(HalModemDialPriv_t));
    gDialPriv.initialed = HAL_MODEM_DIAL_UNINITIALED;
    gDialPriv.isStarted = HAL_MODEM_DIAL_UNSTARTED;    
    ret = RET_OK;
    return ret;
}

int32_t HalModemDialSetParam(const HalModemDialParam_t *dialParam)
{
    int32_t ret = RET_ERR;
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

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

    (void)memcpy(&gDialPriv.dialParam, dialParam, sizeof(HalModemDialParam_t));
    ret = RET_OK;
    return ret;
}

int32_t HalModemDialSetStatusCallback(ModemCallbak cb)
{
    int32_t ret = RET_ERR;

    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);
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
    int32_t ret = 0;
    int32_t i = 0;
    int32_t pdpNum = gDialPriv.dialParam.apnNum;
    // int apnId = 0;
    int callId = 0;

    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);
    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    if (pdpNum <= 0)
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

    int32_t intervalList[20] = {0,};
    ql_data_call_apn_config_t pdnInfo = {0};
    ql_data_call_param_t *param = ql_data_call_param_alloc();
    if (param == NULL)
    {
        printf("Failed to ql_data_call_param_alloc, memory is not enough\n");
        return -1;
    }

    for (i = 0; i < pdpNum; i++)
    {
        callId = gDialPriv.dialParam.apnInfo[i].callId;
        (void)memset(&pdnInfo, 0x0, sizeof(ql_data_call_apn_config_t));
        /**
         * step 1 : 设置 PDN info
         */
        // ip type
        pdnInfo.ip_ver = QL_NET_IP_VER_V4;
        // apn
        (void)strncpy(pdnInfo.apn_name, gDialPriv.dialParam.apnInfo[i].apnName, sizeof(pdnInfo.apn_name));
        // username
        (void)strncpy(pdnInfo.username, gDialPriv.dialParam.apnInfo[i].userName, sizeof(pdnInfo.username));
        // password
        (void)strncpy(pdnInfo.password, gDialPriv.dialParam.apnInfo[i].userPasswd, sizeof(pdnInfo.password));
        // auth type
        pdnInfo.auth_pref = gDialPriv.dialParam.apnInfo[i].authType;
        if (callId == 1) // 默认承载需单独设置
        {
            // apnId = callId;
            // ret = ql_data_call_set_apn_config(apnId, &pdnInfo);
            ret = ql_data_call_set_attach_apn_config(&pdnInfo);
            if(ret != 0)
            {
                LOG_Error("%s ql_data_call_set_attach_apn_config failed, default APN1, ret=%d", __FUNCTION__, ret);
                ret = -1;
                break;
            }
        }
        else
        {
            // apnId = callId + 1;
            ret = ql_data_call_set_apn_config(callId, &pdnInfo);
            if(ret != 0)
            {
                LOG_Error("%s ql_data_call_set_apn_config failed, default APN1, ret=%d", __FUNCTION__, ret);
                ret = -1;
                break;
            }
        }

        /**
         * step 2 : create
         */
        ret = ql_data_call_create(callId, (char*)gDialPriv.dialParam.apnInfo[i].apnName, 0);
        if(QL_ERR_OK != ret)
        {
            LOG_Error("%s ql_data_call_create fail, call_id=%d, ret=%d", __FUNCTION__, callId, ret);
            ret = -1;
            break;
        }

        /**
         * step 3 : start data call
         */
        (void)ql_data_call_param_init(param); 
        (void)ql_data_call_param_set_apn_id(param, callId);
        (void)ql_data_call_param_set_ip_version(param, pdnInfo.ip_ver);
        (void)ql_data_call_param_set_apn_name(param, pdnInfo.apn_name);
        (void)ql_data_call_param_set_user_name(param, pdnInfo.username);
        (void)ql_data_call_param_set_user_password(param, pdnInfo.password);
        (void)ql_data_call_param_set_auth_pref(param, pdnInfo.auth_pref);
        (void)ql_data_call_param_set_reconnect_mode(param, QL_NET_DATA_CALL_RECONNECT_NORMAL);
        intervalList[0] = 20;
        (void)ql_data_call_param_set_reconnect_interval(param, intervalList, 1);

        ret = ql_data_call_config(callId, param);
        if(QL_ERR_OK != ret)
        {
            LOG_Error("%s ql_data_call_config, call_id=%d, ret=%d", __FUNCTION__, callId, ret);
            ret = -1;
            break;
        }

        LOG_Debug("Start data call : pid:%d", callId);
        ret = ql_data_call_start(callId);
        if(QL_ERR_OK != ret)
        {
            LOG_Error("%s ql_data_call_start, call_id=%d, ret=%d", __FUNCTION__, callId, ret);
            ret = -1;
            break;
        }
    }

    (void)ql_data_call_param_free(param);
    if (ret != 0)
    {
        for (i = 0; i < pdpNum; i++)
        {
            (void)ql_data_call_stop(gDialPriv.dialParam.apnInfo[i].callId);           
        }
        LOG_Error("%s isErr true, return", __FUNCTION__);
    }
    else
    {
        gDialPriv.isStarted = HAL_MODEM_DIAL_STARTED;
    }
    
    return ret;
}

int32_t HalModemDialStartEx(uint8_t pid, uint8_t autoReConnect)
{
    int32_t ret = 0;
    int32_t i = 0;
    int32_t pdpNum = gDialPriv.dialParam.apnNum;
    int index = 0;

    LOG_Debug("func:%s line:%d, pid=%d", __FUNCTION__, __LINE__, pid);
    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return -1;
    }

    // 查找指定的 callId
    int32_t found = 0;
    for (i = 0; i < pdpNum; i++)
    {
        if (gDialPriv.dialParam.apnInfo[i].callId == pid)
        {
            found = 1;
            index = i;
            LOG_Info("%s dial specific callId=%d, index=%d", __FUNCTION__, pid, i);
            break;
        }
    }

    if (!found)
    {
        LOG_Error("%s callId=%d not found in dialParam", __FUNCTION__, pid);
        return -1;
    }

    int32_t intervalList[20] = {0,};
    ql_data_call_apn_config_t pdnInfo = {0};
    ql_data_call_param_t *param = ql_data_call_param_alloc();
    if (param == NULL)
    {
        LOG_Error("Failed to ql_data_call_param_alloc, memory is not enough");
        return -1;
    }

    (void)memset(&pdnInfo, 0x0, sizeof(ql_data_call_apn_config_t));
    /**
     * step 1 : 设置 PDN info
     */
    // ip type
    pdnInfo.ip_ver = QL_NET_IP_VER_V4;
    // apn
    (void)strncpy(pdnInfo.apn_name, gDialPriv.dialParam.apnInfo[index].apnName, sizeof(pdnInfo.apn_name));
    // username
    (void)strncpy(pdnInfo.username, gDialPriv.dialParam.apnInfo[index].userName, sizeof(pdnInfo.username));
    // password
    (void)strncpy(pdnInfo.password, gDialPriv.dialParam.apnInfo[index].userPasswd, sizeof(pdnInfo.password));
    // auth type
    pdnInfo.auth_pref = gDialPriv.dialParam.apnInfo[index].authType;
    if (pid == 1) // 默认承载需单独设置
    {
        ret = ql_data_call_set_attach_apn_config(&pdnInfo);
        if(ret != 0)
        {
            LOG_Error("%s ql_data_call_set_attach_apn_config failed, callId=%d, ret=%d", __FUNCTION__, pid, ret);
            ql_data_call_param_free(param);
            return -1;
        }
    }
    else
    {
        ret = ql_data_call_set_apn_config(pid, &pdnInfo);
        if(ret != 0)
        {
            LOG_Error("%s ql_data_call_set_apn_config failed, callId=%d, ret=%d", __FUNCTION__, pid, ret);
            ql_data_call_param_free(param);
            return -1;
        }
    }

    /**
     * step 2 : create
     */
    ret = ql_data_call_create(pid, (char*)gDialPriv.dialParam.apnInfo[index].apnName, 0);
    if(QL_ERR_OK != ret)
    {
        LOG_Error("%s ql_data_call_create fail, call_id=%d, ret=%d", __FUNCTION__, pid, ret);
        ql_data_call_param_free(param);
        return -1;
    }

    /**
     * step 3 : start data call
     */
    (void)ql_data_call_param_init(param);
    (void)ql_data_call_param_set_apn_id(param, pid);
    (void)ql_data_call_param_set_ip_version(param, pdnInfo.ip_ver);
    if(autoReConnect)
    {
        (void)ql_data_call_param_set_reconnect_mode(param, QL_NET_DATA_CALL_RECONNECT_NORMAL);
        intervalList[0] = 20;
        (void)ql_data_call_param_set_reconnect_interval(param, intervalList, 1);
    }
    else
    {
        (void)ql_data_call_param_set_reconnect_mode(param, QL_NET_DATA_CALL_RECONNECT_DISABLE);
    }

    ret = ql_data_call_config(pid, param);
    if(QL_ERR_OK != ret)
    {
        LOG_Error("%s ql_data_call_config, call_id=%d, ret=%d", __FUNCTION__, pid, ret);
        ql_data_call_param_free(param);
        return -1;
    }

    LOG_Info("Start data call : callId=%d", pid);
    ret = ql_data_call_start(pid);
    if(QL_ERR_OK != ret)
    {
        LOG_Error("%s ql_data_call_start, call_id=%d, ret=%d", __FUNCTION__, pid, ret);
        ql_data_call_param_free(param);
        return -1;
    }

    (void)ql_data_call_param_free(param);

    return ret;
}

int32_t HalModemDialStop(void)
{
    int32_t ret = RET_ERR;
    int32_t i = 0;
    int32_t len = gDialPriv.dialParam.apnNum;

    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);
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
        (void)ql_data_call_stop(callId);
    }

    gDialPriv.isStarted = HAL_MODEM_DIAL_UNSTARTED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemDialStopEx(uint8_t pid)
{
    int32_t ret = RET_ERR;

    LOG_Debug("func:%s line:%d, pid=%d", __FUNCTION__, __LINE__, pid);

    // 只检查是否初始化，不检查isStarted状态
    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    // 停止指定的callId
    LOG_Info("%s stopping callId=%d", __FUNCTION__, pid);
    ret = ql_data_call_stop(pid);
    LOG_Debug("%s ql_data_call_stop, callId=%d, ret=%d", __FUNCTION__, pid, ret);
    // 删除数据拨号实例
    ret = ql_data_call_delete(pid);
    LOG_Debug("%s ql_data_call_delete, callId=%d, ret=%d", __FUNCTION__, pid, ret);

    ret = RET_OK;
    return ret;
}