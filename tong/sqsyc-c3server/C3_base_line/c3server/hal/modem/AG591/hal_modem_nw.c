#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ql_type.h"
#include "ql_nw.h"
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

/**
 * @name HalModemNwInit
 * @brief modem网络接口初始化
 *        本函数用于初始化网络模块。如果网络模块已经初始化过，则不再重新初始化，
 *        否则将执行实际的初始化流程。初始化成功返回RET_OK，失败返回RET_ERR。
 * @param null
 * @return 返回初始化结果，RET_OK表示成功，RET_ERR表示失败。
 */
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

    retVal = ql_nw_init();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_nw_init failed.", __FUNCTION__);
        return ret;
    }

    gNwPriv.initialed = HAL_MODEM_NW_INITIALED;
    ret = RET_OK;
    return ret;   
}

/**
 * @name HalModemNwDeinit
 * @brief modem网络接口反初始化。
 * @param null
 * @return 返回初始化结果，RET_OK表示成功，RET_ERR表示失败。 
 */
int32_t HalModemNwDeinit(void)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_NW_INITIALED != gNwPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    retVal = ql_nw_deinit();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_nw_deinit failed.", __FUNCTION__);
        return ret;
    }

    gNwPriv.initialed = HAL_MODEM_NW_UNINITIALED;
    ret = RET_OK;
    return ret;
}

/**
 * @name HalModemNwGetRegStatus
 * @brief 获取Modem的网络注册状态。
 * @param status 指向HalModemNwRegStatus_e类型的指针，用于存储获取的网络注册状态。
 * @return 返回操作的结果状态码。RET_OK表示成功，其他值表示失败。
 */
int32_t HalModemNwGetRegStatus(HalModemNwRegStatus_e *status)
{
    ql_nw_reg_status_info_t info;
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

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
        LOG_Error("%s QL_MCM_NW_GetRegStatus failed.", __FUNCTION__);
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

/**
 * @name HalModemNwRssi2Csq
 * @brief 将网络RSSI（接收信号强度指示）值转换为信号质量（CSQ）值。
 * @param rssi 接收信号强度指示值，单位为dBm。
 * @return 返回相应的信号质量值。信号质量范围为0到99。
 */ 
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


/**
 * @name HalModemNwGetCsq
 * @brief 获取当前网络信号强度。
 * @param csq 指向一个用于存储信号强度结果的 uint8_t 类型变量的指针。
 * @return n 返回操作的结果状态码。RET_OK表示成功，其他值表示失败。
 */ 
int32_t HalModemNwGetCsq(uint8_t *csq)
{
    ql_nw_signal_strength_info_t ptInfo;
    QL_NW_SIGNAL_STRENGTH_LEVEL_E level;
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

    retVal = ql_nw_get_signal_strength(&ptInfo, &level);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_nw_get_signal_strength failed.", __FUNCTION__);
        return ret;
    }

    if (ptInfo.has_gsm)
    {
        *csq = HalModemNwRssi2Csq(ptInfo.gsm.rssi);
    }
    else if (ptInfo.has_wcdma)
    {
        *csq = HalModemNwRssi2Csq(ptInfo.wcdma.rssi);
    }
    else if (ptInfo.has_tdscdma)
    {
        *csq = HalModemNwRssi2Csq(ptInfo.tdscdma.rssi);
    }
    else if (ptInfo.has_lte)
    {
        *csq = HalModemNwRssi2Csq(ptInfo.lte.rssi);
    }
    else if (ptInfo.has_nr5g)
    {
        *csq = HalModemNwRssi2Csq(ptInfo.nr5g.rsrq);
    }
    else if (ptInfo.has_cdma)
    {
        *csq = HalModemNwRssi2Csq(ptInfo.cdma.rssi);
    }
    else if (ptInfo.has_hdr)
    {
        *csq = HalModemNwRssi2Csq(ptInfo.hdr.rssi);
    }
    else
    {
        *csq  = 99; 
    }
    ret = RET_OK;
    return ret;
}

/**
 * @name HalModemNwGetSignalInfo
 * @brief 获取当前网络信号信息
 *        本函数用于获取当前网络信号的强度和质量信息。根据网络类型（WCDMA或LTE），填充相应的信号信息结构体。
 * @param sigInfo 指向HalModemNwExtSigInfo_t结构体的指针，该结构体用于存储获取到的信号信息。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemNwGetSignalInfo(HalModemNwExtSigInfo_t *sigInfo)
{
    ql_nw_signal_strength_info_t ptInfo;
    QL_NW_SIGNAL_STRENGTH_LEVEL_E level;
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
    retVal = ql_nw_get_signal_strength(&ptInfo, &level);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_nw_get_signal_strength failed.retVal:%d", __FUNCTION__, retVal);
        return ret;
    }

    if (ptInfo.has_gsm)
    {
        sigInfo->gsmSigInfo.sigType = 0;
        sigInfo->gsmSigInfo.rssi = ptInfo.gsm.rssi;
    }
    else if (ptInfo.has_wcdma)
    {
        sigInfo->gsmSigInfo.sigType = 0;
        sigInfo->gsmSigInfo.rssi = ptInfo.wcdma.rssi;
        sigInfo->gsmSigInfo.ecio = ptInfo.wcdma.ecio;
    }
    else if (ptInfo.has_tdscdma)
    {
        sigInfo->gsmSigInfo.sigType = 0;
        sigInfo->gsmSigInfo.rssi = ptInfo.tdscdma.rssi;
        sigInfo->gsmSigInfo.rscp = ptInfo.tdscdma.rscp;
        sigInfo->gsmSigInfo.ecio = ptInfo.tdscdma.ecio;
    }
    else if (ptInfo.has_lte)
    {
        sigInfo->lteSigInfo.sigType = 1;
        sigInfo->lteSigInfo.rssi = ptInfo.lte.rssi;
        sigInfo->lteSigInfo.rsrp = ptInfo.lte.rsrp;
        sigInfo->lteSigInfo.rsrq = ptInfo.lte.rsrq;
        sigInfo->lteSigInfo.sinr = ptInfo.lte.snr;
    }
    else if (ptInfo.has_nr5g)
    {
        sigInfo->lteSigInfo.sigType = 1;
        sigInfo->lteSigInfo.rsrp = ptInfo.nr5g.rsrp;
        sigInfo->lteSigInfo.rsrq = ptInfo.nr5g.rsrq;
        sigInfo->lteSigInfo.sinr = ptInfo.nr5g.snr;
    }
    else if (ptInfo.has_cdma)
    {
        sigInfo->gsmSigInfo.sigType = 0;
        sigInfo->gsmSigInfo.rssi = ptInfo.cdma.rssi;
        sigInfo->gsmSigInfo.ecio = ptInfo.cdma.ecio;
    }
    else if (ptInfo.has_hdr)
    {
        sigInfo->gsmSigInfo.sigType = 0;
        sigInfo->gsmSigInfo.rssi = ptInfo.hdr.rssi;
        sigInfo->gsmSigInfo.ecio = ptInfo.hdr.ecio;
    }
    else
    {
        LOG_Error("%s csq invalid.", __FUNCTION__);
        return ret;        
    }
    
    ret = RET_OK;
    return ret;
}

/**
 * @name HalModemNwGetOperatorInfo
 * @brief 获取当前网络运营商信息
 *        本函数用于通过调用底层接口，获取当前模块所注册的网络运营商的信息，包括移动国家代码（MCC）和移动网络代码（MNC）。
 * @param provider 一个指向字符串的指针，该字符串用于存储运营商信息（MCC和MNC的组合）。
 *                 运营商信息将以字符串形式返回，例如"46003"。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemNwGetOperatorInfo(uint8_t *provider)
{
    ql_nw_reg_status_info_t nameInfo;
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