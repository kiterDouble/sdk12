#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "ql_nw.h"
#include "ql_type.h"
#include "hal_modem_nw.h"

#define HAL_MODEM_NW_INITIALED (0)
#define HAL_MODEM_NW_UNINITIALED (-1)

#define RETURN_VALUE(retval) ((QL_ERR_SERVICE_ABORT == retval) ? RET_ERR_MODEMH_SERVICE_ABORT : retval)
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
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

    if (HAL_MODEM_NW_INITIALED == gNwPriv.initialed)
    {
        LOG_Debug("%s have initialed.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }

    retVal = ql_nw_init();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_nw_init failed,retVal:%d", __FUNCTION__, retVal);
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
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

    if (HAL_MODEM_NW_INITIALED != gNwPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    retVal = ql_nw_deinit();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_nw_deinit failed,retVal:%d", __FUNCTION__, retVal);
        return ret;
    }

    gNwPriv.initialed = HAL_MODEM_NW_UNINITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemNwGetRegStatus(HalModemNwRegStatus_e *status)
{
    ql_nw_reg_status_info_t info;
    int32_t ret = RET_ERR;
    int32_t retVal = -1;
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

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

    retVal = ql_nw_get_data_reg_status(&info);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s QL_MCM_NW_GetRegStatus failed,retVal:%d", __FUNCTION__, retVal);
        ret = RETURN_VALUE(retVal);
        return ret;
    }

    *status = E_HAL_MODEM_NW_REG_STATUS_REGISTERED;
    LOG_Debug("%s ql_nw_get_data_reg_status status:%d.", __FUNCTION__, info.reg_state);
            
    if (QL_NW_SERVICE_FULL != info.reg_state)
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
    ql_nw_signal_strength_info_t ptInfo;
    QL_NW_SIGNAL_STRENGTH_LEVEL_E level;
    int32_t ret = RET_ERR;
    int32_t retVal = -1;
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

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

    retVal = ql_nw_get_signal_strength(&ptInfo, &level);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_nw_get_signal_strength failed,retVal:%d", __FUNCTION__, retVal);
        return ret;
    }

    if (ptInfo.has_wcdma)
    {
        *csq = HalModemNwRssi2Csq(ptInfo.wcdma.rssi);
    }
    else if (ptInfo.has_lte)
    {
        *csq = HalModemNwRssi2Csq(ptInfo.lte.rssi);
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
    ql_nw_signal_strength_info_t ptInfo;
    QL_NW_SIGNAL_STRENGTH_LEVEL_E level;
    int32_t ret = RET_ERR;
    int32_t retVal = -1;
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

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
    retVal = ql_nw_get_signal_strength(&ptInfo, &level);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_nw_get_signal_strength failed.retVal:%d", __FUNCTION__, retVal);
        return ret;
    }

    if (ptInfo.has_wcdma)
    {
        sigInfo->gsmSigInfo.sigType = 0;
        sigInfo->gsmSigInfo.rssi = ptInfo.wcdma.rssi;
        sigInfo->gsmSigInfo.ecio = ptInfo.wcdma.ecio;
    }
    else if (ptInfo.has_lte)
    {
        sigInfo->lteSigInfo.sigType = 1;
        sigInfo->lteSigInfo.rssi = ptInfo.lte.rssi;
        sigInfo->lteSigInfo.rsrp = ptInfo.lte.rsrp;
        sigInfo->lteSigInfo.rsrq = ptInfo.lte.rsrq;
        sigInfo->lteSigInfo.sinr = ptInfo.lte.snr;
    }
    else
    {
        LOG_Error("%s csq invalid.", __FUNCTION__);
        return ret;        
    }
    
    ret = RET_OK;
    return ret;
}

int32_t HalModemNwGetOperatorInfo(uint8_t *provider)
{
    ql_nw_reg_status_info_t nameInfo;
    int32_t ret = RET_ERR;
    int32_t retVal = -1;
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

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

    retVal = ql_nw_get_data_reg_status(&nameInfo);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_nw_get_data_reg_status failed.retVal:%d", __FUNCTION__, retVal);
        return ret;
    }

    sprintf((char *)provider, 
            "%s%s", 
            (char *)nameInfo.mcc, 
            (char *)nameInfo.mnc);
    ret = RET_OK;
    return ret;
}