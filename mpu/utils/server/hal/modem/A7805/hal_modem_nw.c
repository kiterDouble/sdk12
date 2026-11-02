#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_comm.h"
#include "hal_modem_nw.h"
#include "Sunsea_NW.h"

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
    if (HAL_MODEM_NW_INITIALED == gNwPriv.initialed)
    {
        LOG_Debug("%s have initialed.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }

    if(HalModemCommInit() != RET_OK)
    {
        LOG_Error("%s HalModemCommInit failed.", __FUNCTION__);
        return ret;
    }

    gNwPriv.initialed = HAL_MODEM_NW_INITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemNwDeinit(void)
{
    int32_t ret = RET_ERR;

    if (HAL_MODEM_NW_INITIALED != gNwPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    gNwPriv.initialed = HAL_MODEM_NW_UNINITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemNwGetRegStatus(HalModemNwRegStatus_e *status)
{
    nw_registration_state_e reg_state;
    nw_cs_attach_state_e cs_state;
    nw_ps_attach_state_e ps_state;
    int32_t ret = RET_ERR;

    if (!status)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_NW_INITIALED != gNwPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    if (Sunsea_NW_GetRegStatus(&reg_state, &cs_state, &ps_state) < 0)
    {
        LOG_Error("%s Sunsea_NW_GetRegStatus fail.", __FUNCTION__);
        return ret;        
    }

    *status = E_HAL_MODEM_NW_REG_STATUS_UNREGISTERED;
    if ((NW_REGISTERED == reg_state) || (NW_REGISTERED_ROAMING == reg_state))
    {
        *status = E_HAL_MODEM_NW_REG_STATUS_REGISTERED;
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
    signal_info_e sigInfo;
    nw_access_radio_if_e radioIf;
    int32_t ret = RET_ERR;

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
    
    if ((RET_OK != HalModemNwGetRegStatus(&status)) || (E_HAL_MODEM_NW_REG_STATUS_REGISTERED != status))
    {
        *csq  = 99;
        return RET_OK;
    }

    if (Sunsea_NW_GetSignal(&sigInfo, &radioIf) < 0)
    {
        LOG_Error("%s Sunsea_NW_GetSignal fail.", __FUNCTION__);
        return ret;
    }

    if (NW_RADIO_IF_CDMA_EVDO == radioIf)
    {
        *csq = HalModemNwRssi2Csq(sigInfo.u.cdma_sig.rssi - 110);
    }
    else if (NW_RADIO_IF_GSM == radioIf)
    {
       *csq = HalModemNwRssi2Csq(sigInfo.u.gsm_sig.rssi - 110);
    }
    else if (NW_RADIO_IF_UMTS == radioIf)
    {
        *csq = HalModemNwRssi2Csq(sigInfo.u.umts_sig.rscp - 120);
    }
    else if (NW_RADIO_IF_LTE == radioIf)
    {
        *csq = HalModemNwRssi2Csq(sigInfo.u.lte_sig.rssi - 110);
    }
    else if (NW_RADIO_IF_NR5G == radioIf)
    {
        *csq = HalModemNwRssi2Csq(sigInfo.u.nr5g_sig.rsrq - 140);
    }
    else
    {
        *csq  = 99; 
    }
    ret = RET_OK;
    return ret;
}
int32_t HalModemNwGetSignalInfo(HalModemNwExtSigInfo_t *sigInfo)
{
    signal_info_e signalInfo;
    nw_access_radio_if_e radioIf;
    int32_t ret = RET_ERR;

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

    HalModemNwRegStatus_e status;
    if ((RET_OK != HalModemNwGetRegStatus(&status)) || (E_HAL_MODEM_NW_REG_STATUS_REGISTERED != status))
    {
        LOG_Error("%s HalModemNwGetRegStatus unregistered.", __FUNCTION__);
        return ret;
    }

    if (Sunsea_NW_GetSignal(&signalInfo, &radioIf) < 0)
    {
        LOG_Error("%s Sunsea_NW_GetSignal fail.", __FUNCTION__);
        return ret;
    }

    if (NW_RADIO_IF_CDMA_EVDO == radioIf)
    {
        sigInfo->gsmSigInfo.sigType = 0;
        sigInfo->gsmSigInfo.rssi = (signalInfo.u.cdma_sig.rssi - 110);
        sigInfo->gsmSigInfo.ecio = signalInfo.u.cdma_sig.ecio;
    }
    else if (NW_RADIO_IF_GSM == radioIf)
    {
        sigInfo->gsmSigInfo.sigType = 0;
        sigInfo->gsmSigInfo.rssi = (signalInfo.u.lte_sig.rssi - 110);
    }
    else if (NW_RADIO_IF_UMTS == radioIf)
    {
        sigInfo->gsmSigInfo.sigType = 0;
        sigInfo->gsmSigInfo.rssi = (signalInfo.u.umts_sig.rscp - 120);
    }
    else if (NW_RADIO_IF_LTE == radioIf)
    {
        sigInfo->gsmSigInfo.sigType = 1;
        sigInfo->lteSigInfo.rssi = (signalInfo.u.lte_sig.rssi - 110);
        sigInfo->lteSigInfo.rsrp = (signalInfo.u.lte_sig.rsrp - 140);
        sigInfo->lteSigInfo.rsrq = (signalInfo.u.nr5g_sig.rsrq - 40) / 2;
        sigInfo->lteSigInfo.sinr = signalInfo.u.lte_sig.snr;
    }
    else if (NW_RADIO_IF_NR5G == radioIf)
    {
        sigInfo->gsmSigInfo.sigType = 1;
        sigInfo->lteSigInfo.rsrp = (signalInfo.u.nr5g_sig.rsrp - 140);
        sigInfo->lteSigInfo.rsrq = (signalInfo.u.nr5g_sig.rsrq - 40) / 2;
        sigInfo->lteSigInfo.sinr = signalInfo.u.nr5g_sig.snr;
    }
    else
    {
        return ret;
    }

    ret = RET_OK;
    return ret;
}

int32_t HalModemNwGetOperatorInfo(uint8_t *provider)
{
    int32_t ret = RET_ERR;

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

    nw_plmn_e plmn;
    if (Sunsea_NW_GetPlmn(&plmn) < 0)
    {
        LOG_Error("%s Sunsea_NW_GetPlmn fail.", __FUNCTION__);
        return ret;
    }

    sprintf((char *)provider, 
            "%s%s", 
            plmn.mcc, 
            plmn.mnc);
    ret = RET_OK; 
    return ret;
}