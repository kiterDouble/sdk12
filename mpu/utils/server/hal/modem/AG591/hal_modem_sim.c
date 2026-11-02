#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ql_util_common.h"
#include "ql-sdk/ql_type.h"
#include "ql-sdk/ql_ms_sim.h"
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_sim.h"

#define HAL_MODEM_SIM_INITIALED (0)
#define HAL_MODEM_SIM_UNINITIALED (-1)

typedef struct
{
    int32_t initialed;
}HalModemSimPriv_t;

static HalModemSimPriv_t gSimPriv = 
{
    .initialed = HAL_MODEM_SIM_UNINITIALED
};
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
    retVal = ql_ms_sim_init();

    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s init fail.", __FUNCTION__);
        return ret;        
    }

    gSimPriv.initialed = HAL_MODEM_SIM_INITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemSimDeinit(void)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_SIM_INITIALED != gSimPriv.initialed)
    {
        LOG_Error("%s have not init.", __FUNCTION__);
        return ret;
    }

    retVal = ql_ms_sim_deinit();

    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s Deinit fail.", __FUNCTION__);
        return ret;        
    }

    gSimPriv.initialed = HAL_MODEM_SIM_UNINITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemSimGetCardStatus(HalModemSimCardStatus_e *cardStat, HalModemSimAppStatus_e *appStat)
{
    ql_sim_card_info_t info = {0};
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

    retVal = ql_ms_sim_get_card_info(QL_SIM_SLOT_1, &info);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s GetCardStatus fail.", __FUNCTION__);
        return ret;
    }
    *cardStat = E_HAL_MODEM_SIM_CARD_STATUS_ERROR;
    LOG_Debug("%s e_card_state:%d.", __FUNCTION__, info.state);
    if (QL_SIM_CARD_STATE_ABSENT == info.state)
    {
        *cardStat = E_HAL_MODEM_SIM_CARD_STATUS_ABSENT;
    }
    if (QL_SIM_CARD_STATE_PRESENT == info.state)     
    {
        *cardStat = E_HAL_MODEM_SIM_CARD_STATUS_PRESENT;
    }
    
    *appStat = E_HAL_MODEM_SIM_APP_STATUS_ERROR;

    if (QL_SIM_SUBSCRIPTION_PRI == info.app_3gpp.subscription)
    {
        LOG_Debug("%s app_3gpp status:%d.", __FUNCTION__, info.app_3gpp.app_state);
        if (info.app_3gpp.app_state == QL_SIM_APP_STATE_READY)
        {
            *appStat = E_HAL_MODEM_SIM_APP_STATUS_READY;
        }
    }
    else if (QL_SIM_SUBSCRIPTION_PRI == info.app_3gpp2.subscription)
    {
        LOG_Debug("%s app_3gpp status:%d.", __FUNCTION__, info.app_3gpp2.app_state);
        if (info.app_3gpp2.app_state== QL_SIM_APP_STATE_READY)
        {
            *appStat = E_HAL_MODEM_SIM_APP_STATUS_READY;
        }
    }
    else if (QL_SIM_SUBSCRIPTION_PRI == info.app_isim.subscription)
    {
        LOG_Debug("%s app_3gpp2 status:%d.", __FUNCTION__, info.app_isim.app_state);
        if (info.app_isim.app_state == QL_SIM_APP_STATE_READY)
        {
            *appStat = E_HAL_MODEM_SIM_APP_STATUS_READY;
        }
    }
    else
    {
        LOG_Debug("%s type unknow.", __FUNCTION__);
    }

    ret = RET_OK;
    return ret;
}

/**
 * @brief 获取当前激活的 (U)SIM 卡槽
 * @return int 激活的卡槽号 (QL_SIM_SLOT_1 或 QL_SIM_SLOT_2)，错误时返回 E_HAL_MODEM_SIM_SLOT_INVALID
 */
HalModemSimSlot_e HalModemSimGetActiveSlot(void) 
{
    int ret;
    uint8_t active_slot = 0;
    QL_SIM_SLOT_E slot = QL_SIM_SLOT_1; // 默认查询卡槽 1，可根据需要调整

    // 查询激活的卡槽
    ret = ql_ms_sim_get_sim_slot_active(slot, &active_slot);
    if (ret == QL_ERR_OK)
    {
        if (active_slot == 1)
        {
            LOG_Debug("Active SIM slot: %d\n", slot);
            return E_HAL_MODEM_SIM_SLOT_1; // 返回激活的卡槽号
        } 
        else
        {
            // 如果卡槽 1 未激活，尝试卡槽 2
            slot = QL_SIM_SLOT_2;
            ret = ql_ms_sim_get_sim_slot_active(slot, &active_slot);
            if (ret == QL_ERR_OK && active_slot == 1)
            {
                LOG_Debug("Active SIM slot: %d\n", slot);
                return E_HAL_MODEM_SIM_SLOT_2; // 返回激活的卡槽号
            }
        }
    }

    // 错误处理
    LOG_Debug("Failed to get active SIM slot: %d\n", ret);
    return E_HAL_MODEM_SIM_SLOT_INVALID; // 未找到激活的卡槽或发生错误
}
int32_t HalModemSimGetImsi(uint8_t *imsi, uint8_t len)
{
    ql_sim_card_info_t info = {0};
    QL_SIM_APP_TYPE_E type;
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

    retVal = ql_ms_sim_get_card_info(QL_SIM_SLOT_1, &info);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_sim_get_card_info fail.", __FUNCTION__);
        return ret;
    }

    if (QL_SIM_SUBSCRIPTION_PRI == info.app_3gpp.subscription)
    {
        type = QL_SIM_APP_TYPE_3GPP;
    }
    else if (QL_SIM_SUBSCRIPTION_PRI == info.app_3gpp2.subscription)
    {
        type = QL_SIM_APP_TYPE_3GPP2;
    }

    else if (QL_SIM_SUBSCRIPTION_PRI == info.app_isim.subscription)
    {
        type = QL_SIM_APP_TYPE_ISIM;
    }
    else 
    {
        type = QL_SIM_APP_TYPE_UNKNOWN;
        LOG_Error("%s APP type unknow.", __FUNCTION__);
        return ret;
    }


    ret = ql_ms_sim_get_imsi(QL_SIM_SLOT_1, type, (char*)imsi, len);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_sim_get_imsi fail.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

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

    retVal = ql_ms_sim_get_iccid(QL_SIM_SLOT_1, (char*)iccid, len);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_sim_get_iccid fail.", __FUNCTION__);
        return ret;
    }
        
    ret = RET_OK;
    return ret;
}
int32_t HalModemSimGetMsisdn(uint8_t *msisdn, uint8_t len)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (!msisdn || (len == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;        
    }

    if (HAL_MODEM_SIM_INITIALED != gSimPriv.initialed)
    {
        LOG_Error("%s have not init.", __FUNCTION__);
        return ret;
    }

    retVal = ql_ms_sim_get_phone_num(QL_SIM_SLOT_1, QL_SIM_APP_TYPE_3GPP, (char *)msisdn, len);

    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_sim_get_phone_num fail.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

