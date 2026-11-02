#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "ql_oe.h"
#include "ql_mcm_sim.h"
#include "sr_log.h"
#include "hal_modem_sim.h"

#define HAL_MODEM_SIM_INITIALED (0)
#define HAL_MODEM_SIM_UNINITIALED (-1)

typedef struct
{
    sim_client_handle_type simHdl;
    int32_t initialed;
}HalModemSimPriv_t;

static HalModemSimPriv_t gSimPriv = 
{
    .simHdl = 0x0,
    .initialed = HAL_MODEM_SIM_UNINITIALED
};

/**
 * @name HalModemSimInit
 * @brief 初始化Modem的SIM卡模块。
 * @param null
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。 
 */
int32_t HalModemSimInit(void)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_SIM_INITIALED == gSimPriv.initialed)
    {
        LOG_Error("%s have init.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }
    retVal = QL_MCM_SIM_Client_Init(&gSimPriv.simHdl);

    if (E_QL_SUCCESS != retVal)
    {
        LOG_Error("%s init fail.", __FUNCTION__);
        return ret;        
    }

    gSimPriv.initialed = HAL_MODEM_SIM_INITIALED;
    ret = RET_OK;
    return ret;
}

/**
 * @name HalModemSimDeinit
 * @brief 反初始化Modem的SIM卡模块。
 * @param null
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。 
 */
int32_t HalModemSimDeinit(void)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_SIM_INITIALED != gSimPriv.initialed)
    {
        LOG_Error("%s have not init.", __FUNCTION__);
        return ret;
    }

    retVal = QL_MCM_SIM_Client_Deinit(gSimPriv.simHdl);

    if (E_QL_SUCCESS != retVal)
    {
        LOG_Error("%s Deinit fail.", __FUNCTION__);
        return ret;        
    }

    gSimPriv.initialed = HAL_MODEM_SIM_UNINITIALED;
    gSimPriv.simHdl = 0x0;
    ret = RET_OK;
    return ret;
}

/**
 * @name HalModemSimGetCardStatus
 * @brief 获取SIM卡的状态和应用状态。
 * @param cardStat 指向HalModemSimCardStatus_e类型的指针，用于接收SIM卡状态。
 * @param appStat 指向HalModemSimAppStatus_e类型的指针，用于接收SIM卡应用状态。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemSimGetCardStatus(HalModemSimCardStatus_e *cardStat, HalModemSimAppStatus_e *appStat)
{
    QL_MCM_SIM_CARD_STATUS_INFO_T info;
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (!cardStat || !appStat)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;        
    }

    if (HAL_MODEM_SIM_INITIALED != gSimPriv.initialed)
    {
        LOG_Error("%s have not init.", __FUNCTION__);
        return ret;
    }
    retVal = QL_MCM_SIM_GetCardStatus(gSimPriv.simHdl, E_QL_MCM_SIM_SLOT_ID_1, &info);
    if (E_QL_SUCCESS != retVal)
    {
        LOG_Error("%s GetCardStatus fail.", __FUNCTION__);
        return ret;
    }
    *cardStat = E_HAL_MODEM_SIM_CARD_STATUS_ERROR;
    LOG_Debug("%s e_card_state:%d.", __FUNCTION__, info.e_card_state);
    if (E_QL_MCM_SIM_CARD_STATE_ABSENT == info.e_card_state)
    {
        *cardStat = E_HAL_MODEM_SIM_CARD_STATUS_ABSENT;
    }
    if (E_QL_MCM_SIM_CARD_STATE_PRESENT == info.e_card_state)     
    {
        //E_QL_MCM_SIM_APP_STATE_READY == info.)
        *cardStat = E_HAL_MODEM_SIM_CARD_STATUS_PRESENT;
    }
    
    *appStat = E_HAL_MODEM_SIM_APP_STATUS_ERROR;
    if (E_QL_MCM_SIM_PROV_STATE_PRI == info.card_app_info.app_isim.subscription)
    {
        LOG_Debug("%s app_isim status:%d.", __FUNCTION__, info.card_app_info.app_isim.app_state);
        if (info.card_app_info.app_isim.app_state == E_QL_MCM_SIM_APP_STATE_READY)
        {
            *appStat = E_HAL_MODEM_SIM_APP_STATUS_READY;
        }
    }

    if (E_QL_MCM_SIM_PROV_STATE_PRI == info.card_app_info.app_3gpp.subscription)
    {
        LOG_Debug("%s app_3gpp status:%d.", __FUNCTION__, info.card_app_info.app_3gpp.app_state);
        if (info.card_app_info.app_3gpp.app_state == E_QL_MCM_SIM_APP_STATE_READY)
        {
            *appStat = E_HAL_MODEM_SIM_APP_STATUS_READY;
        }
    }

    if (E_QL_MCM_SIM_PROV_STATE_PRI == info.card_app_info.app_3gpp2.subscription)
    {
        LOG_Debug("%s app_3gpp2 status:%d.", __FUNCTION__, info.card_app_info.app_3gpp2.app_state);
        if (info.card_app_info.app_3gpp2.app_state == E_QL_MCM_SIM_APP_STATE_READY)
        {
            *appStat = E_HAL_MODEM_SIM_APP_STATUS_READY;
        }
    }

    ret = RET_OK;
    return ret;
}

/**
 * @name HalModemSimGetImsi
 * @brief 获取SIM卡的IMSI（国际移动用户识别码）。
 * @param imsi 指向存储IMSI字符串的缓冲区的指针。
 * @param len 指定缓冲区的长度。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemSimGetImsi(uint8_t *imsi, uint8_t len)
{
    QL_MCM_SIM_CARD_STATUS_INFO_T info;
    QL_SIM_APP_ID_INFO_T appInfo;
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (!imsi || (len == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;        
    }

    if (HAL_MODEM_SIM_INITIALED != gSimPriv.initialed)
    {
        LOG_Error("%s have not init.", __FUNCTION__);
        return ret;
    }
    retVal = QL_MCM_SIM_GetCardStatus(gSimPriv.simHdl, E_QL_MCM_SIM_SLOT_ID_1, &info);
    if (E_QL_SUCCESS != retVal)
    {
        LOG_Error("%s GetCardStatus fail.", __FUNCTION__);
        return ret;
    }

    appInfo.e_slot_id = E_QL_MCM_SIM_SLOT_ID_1;
    if (E_QL_MCM_SIM_PROV_STATE_PRI == info.card_app_info.app_isim.subscription)
    {
        appInfo.e_app = E_QL_MCM_SIM_APP_TYPE_ISIM;
    }
    else if (E_QL_MCM_SIM_PROV_STATE_PRI == info.card_app_info.app_3gpp.subscription)
    {
        appInfo.e_app = E_QL_MCM_SIM_APP_TYPE_3GPP;
    }
    else if (E_QL_MCM_SIM_PROV_STATE_PRI == info.card_app_info.app_3gpp2.subscription)
    {
        appInfo.e_app = E_QL_MCM_SIM_APP_TYPE_3GPP2;
    }
    else
    {
        appInfo.e_app = E_QL_MCM_SIM_APP_TYPE_UNKNOWN;
    }

    retVal = QL_MCM_SIM_GetIMSI(gSimPriv.simHdl, &appInfo, (char *)imsi, len);
    if (E_QL_SUCCESS != retVal)
    {
        LOG_Error("%s QL_MCM_SIM_GetIMSI fail.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

/**
 * @name HalModemSimGetIccid
 * @brief 获取SIM卡的ICCID。
 * @param iccid 指向存储ICCID的缓冲区的指针。
 * @param len 指定ICCID缓冲区的长度。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemSimGetIccid(uint8_t *iccid, uint8_t len)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;


    if (!iccid || (len == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;        
    }

    if (HAL_MODEM_SIM_INITIALED != gSimPriv.initialed)
    {
        LOG_Error("%s have not init.", __FUNCTION__);
        return ret;
    }
    retVal = QL_MCM_SIM_GetICCID(gSimPriv.simHdl, E_QL_MCM_SIM_SLOT_ID_1, (char *)iccid, len);
    if (E_QL_SUCCESS != retVal)
    {
        LOG_Error("%s GetICCID fail.", __FUNCTION__);
        return ret;
    }
        
    ret = RET_OK;
    return ret;
}

int32_t HalModemSimGetMsisdn(uint8_t *msisdn, uint8_t len)
{
    return RET_OK;
}
