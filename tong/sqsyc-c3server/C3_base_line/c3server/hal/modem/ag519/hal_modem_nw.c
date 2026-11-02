#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ql_util_common.h"
#include "ql-sdk/ql_type.h"
#include "ql-sdk/ql_ms_nw.h"
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_nw.h"

#define HAL_MODEM_NW_INITIALED (0)
#define HAL_MODEM_NW_UNINITIALED (-1)

typedef struct
{
    int32_t initialed;
}HalModemNwPriv_t;

static HalModemNwPriv_t gNwPriv = 
{
    .initialed = HAL_MODEM_NW_UNINITIALED
};
int32_t HalModemNwInit(void)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_NW_INITIALED == gNwPriv.initialed)
    {
        LOG_Debug("%s have initialed.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }

    retVal = ql_ms_nw_init();

    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_nw_init failed.", __FUNCTION__);
        return ret;
    }

    gNwPriv.initialed = HAL_MODEM_NW_INITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemNwDeinit(void)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_NW_INITIALED != gNwPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    retVal = ql_ms_nw_deinit();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_nw_deinit failed.", __FUNCTION__);
        return ret;
    }

    gNwPriv.initialed = HAL_MODEM_NW_UNINITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemNwGetRegStatus(HalModemNwRegStatus_e *status)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_NW_INITIALED != gNwPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    ql_nw_reg_status_info_t t_info;
    memset(&t_info, 0, sizeof(t_info)); 
    retVal = ql_ms_nw_get_data_reg_status(QL_SIM_1, &t_info);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_nw_get_data_reg_status failed.", __FUNCTION__);
        return ret;
    }

    *status = E_HAL_MODEM_NW_REG_STATUS_REGISTERED;

    if (QL_NW_SERVICE_FULL != t_info.reg_state)
    {
        *status = E_HAL_MODEM_NW_REG_STATUS_UNREGISTERED;
    }
    ret = RET_OK;
    return ret;
}

static int32_t HalModemNwRssi2Csq(int32_t rssi)
{
    int32_t csq;

    if (rssi < -113)
    {
        csq = 0;
    }
    else if( rssi < -51)
    {
        csq = (rssi + 113)/2;
    }
    else if (rssi < 0)
    {
        csq = 31;
    }
    else
    {
        csq = 99;
    }
    return csq;
}

int32_t HalModemNwGetCsq(uint8_t *csq)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (!csq)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_NW_INITIALED != gNwPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    HalModemNwRegStatus_e status;
    retVal = HalModemNwGetRegStatus(&status);
    
    if ((RET_OK != retVal) || (E_HAL_MODEM_NW_REG_STATUS_REGISTERED != status))
    {
        *csq  = 99;
        return RET_OK;
    }

    ql_nw_signal_strength_info_t info;
    QL_NW_SIGNAL_STRENGTH_LEVEL_E level = QL_NW_SIGNAL_STRENGTH_LEVEL_NONE;
    memset(&info, 0, sizeof(info));
    retVal = ql_ms_nw_get_signal_strength(QL_SIM_1, &info, &level);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_nw_get_signal_strength failed.", __FUNCTION__);
        return ret;
    }

    if (info.has_gsm)
    {
        *csq = HalModemNwRssi2Csq(info.gsm.rssi);
    } 
    else if (info.has_wcdma)
    {
        *csq = HalModemNwRssi2Csq(info.wcdma.rssi);
    } 
    else if (info.has_tdscdma)
    {
        *csq = HalModemNwRssi2Csq(info.tdscdma.rssi);
    }
    else if (info.has_lte)
    {
        *csq = HalModemNwRssi2Csq(info.lte.rssi);
    } 
    
    if(info.has_nr5g)
    {
        *csq = HalModemNwRssi2Csq(info.nr5g.rsrp);
    }
    else if(info.has_cdma)
    {
        *csq = HalModemNwRssi2Csq(info.cdma.rssi);
    } 
    else if(info.has_hdr)
    {
        *csq = HalModemNwRssi2Csq(info.hdr.rssi);
    }
    else
    {
        LOG_Error("%s sigType error.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

int32_t HalModemNwGetSignalInfo(HalModemNwExtSigInfo_t *sigInfo)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (!sigInfo)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_NW_INITIALED != gNwPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    memset(sigInfo, 0x0, sizeof(HalModemNwExtSigInfo_t));
    QL_NW_SIGNAL_STRENGTH_LEVEL_E level = QL_NW_SIGNAL_STRENGTH_LEVEL_NONE;
    ql_nw_signal_strength_info_t info;
    memset(&info, 0, sizeof(info));
    retVal = ql_ms_nw_get_signal_strength(QL_SIM_1, &info, &level);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_nw_get_signal_strength failed.", __FUNCTION__);
        return ret;
    }

    if(info.has_gsm)
    {
        sigInfo->gsmSigInfo.sigType = 0;
        sigInfo->gsmSigInfo.rssi = info.gsm.rssi;
    } 
    else if(info.has_wcdma)
    {
        sigInfo->gsmSigInfo.sigType = 0;
        sigInfo->gsmSigInfo.rssi = info.wcdma.rssi;
        sigInfo->gsmSigInfo.ecio = info.wcdma.ecio;
    } 
    else if(info.has_tdscdma)
    {
        sigInfo->gsmSigInfo.sigType = 0;
        sigInfo->gsmSigInfo.rssi = info.tdscdma.rssi;
        sigInfo->gsmSigInfo.ecio = info.tdscdma.ecio;
        sigInfo->gsmSigInfo.rscp = info.tdscdma.rscp;
    } 
    else if(info.has_lte)
    {
        sigInfo->lteSigInfo.sigType = 1;
        sigInfo->lteSigInfo.rssi = info.lte.rssi;
        sigInfo->lteSigInfo.rsrp = info.lte.rsrp;
        sigInfo->lteSigInfo.rsrq = info.lte.rsrq;
        sigInfo->lteSigInfo.sinr = info.lte.snr;
    } 
    else if(info.has_nr5g)
    {
        sigInfo->gsmSigInfo.sigType = 1;
        sigInfo->gsmSigInfo.rssi = info.nr5g.rsrp;
        sigInfo->lteSigInfo.rsrp = info.nr5g.rsrp;
        sigInfo->lteSigInfo.rsrq = info.nr5g.rsrq;
        sigInfo->lteSigInfo.sinr = info.nr5g.snr;
    }
    else if(info.has_cdma)
    {
        sigInfo->gsmSigInfo.sigType = 0;
        sigInfo->gsmSigInfo.rssi = info.cdma.rssi;
        sigInfo->gsmSigInfo.ecio = info.cdma.ecio;
    } 
    else if(info.has_hdr)
    {
        sigInfo->gsmSigInfo.sigType = 0;
        sigInfo->gsmSigInfo.rssi = info.hdr.rssi;
        sigInfo->gsmSigInfo.ecio = info.hdr.ecio;
    }
    else
    {
        LOG_Error("%s sigType error.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

int32_t HalModemNwGetOperatorInfo(uint8_t *provider)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (!provider)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_NW_INITIALED != gNwPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    ql_nw_mobile_operator_name_info_t t_info;

    memset(&t_info, 0, sizeof(t_info));
    retVal = ql_ms_nw_get_mobile_operator_name(QL_SIM_1, &t_info);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_nw_get_mobile_operator_name failed.", __FUNCTION__);
        return ret;
    }

    sprintf((char *)provider, 
            "%s%s", 
            (char *)t_info.mcc, 
            (char *)t_info.mnc);
    ret = RET_OK;
    return ret;
}