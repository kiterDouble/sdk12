#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ql_util_common.h"
#include "ql-sdk/ql_type.h"
#include "ql-sdk/ql_nw.h"
#include "ql-sdk/ql_ms_nw.h"
#include "ql-sdk/ql_ms_sim.h"
#include "hal_modem_sim.h"
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

    // add
    int32_t activesimslot = 0; // add
    activesimslot = HalModemSimGetActiveSlot_2(false);
    if(activesimslot < 0 )
    {
        LOG_Error("sim_slot_2: %s HalModemSimGetActiveSolt", __FUNCTION__);
        return ret;
    }

    ql_nw_reg_status_info_t t_info;
    memset(&t_info, 0, sizeof(t_info)); 
    retVal = ql_ms_nw_get_data_reg_status(activesimslot, &t_info);
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
static int32_t HalModemNwRsrp2Csq(int32_t rsrp)  //5G 信号强度映射
{
    int32_t csq;

    if (rsrp < -140)
    {
        csq = 0;
    }
    else if (rsrp <= -80)
    {
        csq = (rsrp + 140) * 30 / 60;
    }
    else if (rsrp < 0)
    {
        csq = 31;
    }
    else // 未知或不可用
    {
        csq = 99;
    }
    return csq;
}

int32_t HalModemNwGetRegStatusInfo(HalModemNwRegStatusInfo_t *regInfo)
{
    int32_t retValue = RET_ERR;
    ql_nw_reg_status_info_t regStatusInfo;

    if(NULL == regInfo)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return retValue;
    }

    if (HAL_MODEM_NW_INITIALED != gNwPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return retValue;
    }

    // add
    int32_t activesimslot = 0; // add
    activesimslot = HalModemSimGetActiveSlot_2(false);
    if(activesimslot < 0 )
    {
        LOG_Error("sim_slot_2: %s HalModemSimGetActiveSolt", __FUNCTION__);
        return retValue;
    }

    retValue = ql_ms_nw_get_data_reg_status(activesimslot, &regStatusInfo);
    if(QL_ERR_OK != retValue)
    {
       LOG_Error("%s ql_ms_nw_get_signal_strength failed.", __FUNCTION__);
        return retValue; 
    }

    memcpy(regInfo->mcc, regStatusInfo.mcc, 4);
    memcpy(regInfo->mnc, regStatusInfo.mnc, 4);
    regInfo->cid = regStatusInfo.cid;
    regInfo->lac = regStatusInfo.lac;
    regInfo->pcid5g = regStatusInfo.nr5g_pci;
    regInfo->cid5g = regStatusInfo.nr5g_cid;

    retValue = RET_OK;
    return retValue;
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

    // add
    int32_t activesimslot = 0; // add
    activesimslot = HalModemSimGetActiveSlot_2(false);
    if(activesimslot < 0 )
    {
        LOG_Error("sim_slot_2: %s HalModemSimGetActiveSolt", __FUNCTION__);
        return ret;
    }


    ql_nw_signal_strength_info_t info;
    QL_NW_SIGNAL_STRENGTH_LEVEL_E level = QL_NW_SIGNAL_STRENGTH_LEVEL_NONE;
    memset(&info, 0, sizeof(info));
    retVal = ql_ms_nw_get_signal_strength(activesimslot, &info, &level);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_nw_get_signal_strength failed.", __FUNCTION__);
        return ret;
    }


    if(info.has_nr5g)
    {
        *csq = HalModemNwRsrp2Csq(info.nr5g.rsrp);
    } 
    else if (info.has_lte)
    {
        *csq = HalModemNwRssi2Csq(info.lte.rssi);
    } 
    else if(info.has_cdma)
    {
        *csq = HalModemNwRssi2Csq(info.cdma.rssi);
    } 
    else if (info.has_gsm)
    {
        *csq = HalModemNwRssi2Csq(info.gsm.rssi);
    }
    else if(info.has_hdr)
    {
        *csq = HalModemNwRssi2Csq(info.hdr.rssi);
    }
    else if (info.has_wcdma)
    {
        *csq = HalModemNwRssi2Csq(info.wcdma.rssi);
    } 
    else if (info.has_tdscdma)
    {
        *csq = HalModemNwRssi2Csq(info.tdscdma.rssi);
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

    // add
    int32_t activesimslot = 0; // add
    activesimslot = HalModemSimGetActiveSlot_2(false);
    if(activesimslot < 0 )
    {
        LOG_Error("sim_slot_2: %s HalModemSimGetActiveSolt", __FUNCTION__);
        return ret;
    }

    ql_nw_signal_strength_info_t info;
    QL_NW_SIGNAL_STRENGTH_LEVEL_E level = QL_NW_SIGNAL_STRENGTH_LEVEL_NONE;
    memset(&info, 0, sizeof(info));
    retVal = ql_ms_nw_get_signal_strength(activesimslot, &info, &level);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_nw_get_signal_strength failed.", __FUNCTION__);
        return ret;
    }

    if(info.has_nr5g)
    {
        sigInfo->lteSigInfo.sigType = 3;
        sigInfo->lteSigInfo.rssi = info.nr5g.rsrp;
        sigInfo->lteSigInfo.rsrp = info.nr5g.rsrp;
        sigInfo->lteSigInfo.rsrq = info.nr5g.rsrq;
        sigInfo->lteSigInfo.sinr = info.nr5g.snr;
    }
    else if(info.has_lte)
    {
        sigInfo->lteSigInfo.sigType = 1;
        sigInfo->lteSigInfo.rssi = info.lte.rssi;
        sigInfo->lteSigInfo.rsrp = info.lte.rsrp;
        sigInfo->lteSigInfo.rsrq = info.lte.rsrq;
        sigInfo->lteSigInfo.sinr = info.lte.snr;
    }
    else if(info.has_cdma)
    {
        sigInfo->lteSigInfo.sigType = 2;
        sigInfo->lteSigInfo.rssi = info.cdma.rssi;
        sigInfo->lteSigInfo.ecio = info.cdma.ecio;
    } 
    else if(info.has_gsm)
    {
        sigInfo->lteSigInfo.sigType = 0;
        sigInfo->lteSigInfo.rssi = info.gsm.rssi;
    }   
    else if(info.has_hdr)
    {
        sigInfo->lteSigInfo.sigType = 0;
        sigInfo->lteSigInfo.rssi = info.hdr.rssi;
        sigInfo->lteSigInfo.ecio = info.hdr.ecio;
    }   
    else if(info.has_wcdma) //591 不支持
    {
        sigInfo->lteSigInfo.sigType = 0;
        sigInfo->lteSigInfo.rssi = info.wcdma.rssi;
        sigInfo->lteSigInfo.ecio = info.wcdma.ecio;
    } 
    else if(info.has_tdscdma) // 591 不支持
    {
        sigInfo->lteSigInfo.sigType = 0;
        sigInfo->lteSigInfo.rssi = info.tdscdma.rssi;
        sigInfo->lteSigInfo.ecio = info.tdscdma.ecio;
        sigInfo->lteSigInfo.rscp = info.tdscdma.rscp;
    } 
    else
    {
        LOG_Error("%s sigType error.", __FUNCTION__);
        return ret;
    }
    
    sigInfo->lteSigInfo.level = level; //返回信号等级.
    
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

    // add
    int32_t activesimslot = 0; // add
    activesimslot = HalModemSimGetActiveSlot_2(false);
    if(activesimslot < 0 )
    {
        LOG_Error("sim_slot_2: %s HalModemSimGetActiveSolt", __FUNCTION__);
        return ret;
    }

    ql_nw_mobile_operator_name_info_t t_info;

    memset(&t_info, 0, sizeof(t_info));
    retVal = ql_ms_nw_get_mobile_operator_name(activesimslot, &t_info);
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
