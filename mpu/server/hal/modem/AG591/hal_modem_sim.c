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

static uint8_t  g_ActiveSimSlot = 1; // 20260817 add 当前使用的sim slot

// 20260817 add
static void ModemSimSetActiveSlot(uint8_t simSlotIndex)
{
    g_ActiveSimSlot = simSlotIndex;
}

// 20260817 add 
static uint8_t HalModemSimUpdateActiveSlot()
{
    ql_sim_card_info_t info = {0};
    int32_t ret = RET_ERR;
    int32_t retVal = -1;
    uint8_t slotstatus = 0;

    //HalModemSimGetSIM1CardStatus();
    retVal = ql_ms_sim_get_card_info(QL_SIM_SLOT_2, &info);
    if (QL_ERR_OK != retVal)
    {
        // 获取不到slot2信息
        LOG_Error("sim_slot_2: %s GetCardStatus QL_SIM_SLOT_2 fail retVal:%d", __FUNCTION__,retVal);
        return ret;
    }

    // 打印当前slot信息
    LOG_Debug("sim_slot_2: %s e_card_state:%d. %d", __FUNCTION__, info.state, g_ActiveSimSlot);
   
    if ((QL_SIM_CARD_STATE_PRESENT == info.state) && (g_ActiveSimSlot != 2))     
    {
        // slot2 物理插卡， 但是配置参数是1
        retVal = ql_ms_sim_get_sim_slot_active(QL_SIM_SLOT_1, &slotstatus);
        if(retVal != QL_ERR_OK)
        {
            LOG_Error("sim_slot_2: %s Get Sim QL_SIM_SLOT_1 Slot Fail %d", __FUNCTION__,retVal);
        }

        if(slotstatus != 0)
        {
            // 关闭slot1
            retVal = ql_ms_sim_set_sim_slot_active(QL_SIM_SLOT_1, 0);
            if(retVal != QL_ERR_OK)
            {
                LOG_Error("sim_slot_2: %s Sim QL_SIM_SLOT_1 Slot Inactive Fail %d", __FUNCTION__,retVal);
                
            }
        }
        else
        {
            LOG_Debug("sim_slot_2: %s Sim QL_SIM_SLOT_1 Slot Inactive ", __FUNCTION__);
        }

        // 激活slot2
        retVal = ql_ms_sim_set_sim_slot_active(QL_SIM_SLOT_2, 1);
        if(retVal != QL_ERR_OK)
        {
            LOG_Error("sim_slot_2: %s Sim QL_SIM_SLOT_2 Slot Active Fail retVal:%d", __FUNCTION__,retVal);
            //return ret;
        }
        // 使用slot2
        ModemSimSetActiveSlot(2);
    }
    else if((QL_SIM_CARD_STATE_PRESENT != info.state) && (g_ActiveSimSlot == 1))
    {
        // slot2没有物理插卡， 使用slot1
        LOG_Debug("sim_slot_2: %s e_card_state:%d. %d", __FUNCTION__, info.state,g_ActiveSimSlot);
        retVal = ql_ms_sim_get_sim_slot_active(QL_SIM_SLOT_2, &slotstatus);
        if(retVal != QL_ERR_OK)
        {
            LOG_Error("sim_slot_2: %s Get Sim QL_SIM_SLOT_2 Slot  Fail retVal:%d", __FUNCTION__,retVal);
        }

        if(slotstatus != 0)
        {
            retVal = ql_ms_sim_set_sim_slot_active(QL_SIM_SLOT_2, 0);
            if(retVal != QL_ERR_OK)
            {
                LOG_Error("sim_slot_2: %s Sim QL_SIM_SLOT_2 Slot Inactive Fail retVal:%d", __FUNCTION__,retVal);
                
            }
        }
        else
        {
            LOG_Debug("sim_slot_2: %s Sim QL_SIM_SLOT_2 Slot Inactive ", __FUNCTION__);
        }

        retVal = ql_ms_sim_set_sim_slot_active(QL_SIM_SLOT_1, 1);
        if(retVal != QL_ERR_OK)
        {
            LOG_Error("sim_slot_2: %s Sim QL_SIM_SLOT_1 Slot Active Fail retVal:%d", __FUNCTION__,retVal);
            //return ret;
        }
        ModemSimSetActiveSlot(1);
    }
    
    LOG_Debug("sim_slot_2: %s Sim State: %x Sim Slot %d Active ", __FUNCTION__,info.state,g_ActiveSimSlot);

    return g_ActiveSimSlot;
}


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
    retVal = HalModemSimUpdateActiveSlot();
    if(retVal < 0)
    {
        LOG_Error("%s Hal Modem Sim Update Active Solt Fail retVal:%d", __FUNCTION__,retVal);
        return ret;
    }

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
    // add
    int32_t activesimslot = 0;
    QL_SIM_SLOT_E slot = QL_SIM_SLOT_1;


    if (!cardStat || !appStat)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;        
    }

    if (HAL_MODEM_SIM_INITIALED != gSimPriv.initialed)
    {
        *cardStat = E_HAL_MODEM_SIM_CARD_STATUS_ABSENT; // add
        *appStat = E_HAL_MODEM_SIM_APP_STATUS_ERROR;
        LOG_Error("%s have not init.", __FUNCTION__);
        return RET_ERR;
    }

    // add
    activesimslot = HalModemSimGetActiveSlot_2(false);
    if(activesimslot < 0)
    {
        // 沒有插卡狀態
        *cardStat = E_HAL_MODEM_SIM_CARD_STATUS_ABSENT; // add
        *appStat = E_HAL_MODEM_SIM_APP_STATUS_ERROR;
        LOG_Error("sim_slot_2: %s activesimslot:%d.", __FUNCTION__, activesimslot);
        return RET_ERR;
    }
    if(activesimslot == 1)
    {
        slot = QL_SIM_SLOT_1;
    }
    else if(activesimslot == 2)
    {
        slot = QL_SIM_SLOT_2;
    }
    else
    {
        LOG_Error("sim_slot_2: %s HalModemSimGetActiveSlot_2 activesimslot:%d", __FUNCTION__,activesimslot);
    }
    LOG_Debug("sim_slot_2: %s slot:%d.", __FUNCTION__, slot);
    retVal = ql_ms_sim_get_card_info(slot, &info);


    //retVal = ql_ms_sim_get_card_info(QL_SIM_SLOT_1, &info);
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
    else if (QL_SIM_SUBSCRIPTION_SEC == info.app_3gpp.subscription)
    {
        LOG_Debug("%s app_3gpp status:%d.", __FUNCTION__, info.app_3gpp.app_state);
        if (info.app_3gpp.app_state == QL_SIM_APP_STATE_READY)
        {
            *appStat = E_HAL_MODEM_SIM_APP_STATUS_READY;
        }
    }
    else if (QL_SIM_SUBSCRIPTION_SEC == info.app_3gpp2.subscription)
    {
        LOG_Debug("%s app_3gpp status:%d.", __FUNCTION__, info.app_3gpp2.app_state);
        if (info.app_3gpp2.app_state== QL_SIM_APP_STATE_READY)
        {
            *appStat = E_HAL_MODEM_SIM_APP_STATUS_READY;
        }
    }
    else if (QL_SIM_SUBSCRIPTION_SEC == info.app_isim.subscription)
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

// 20260817 add
int32_t HalModemSimGetActiveSlot_2(bool bSimInit) 
{
    ql_sim_card_info_t info = {0};
    int32_t ret = RET_ERR;
    int32_t retVal = -1;
    uint8_t slotstatus = -1;
    int32_t activesimslot = 0;
    if(bSimInit)
    {
        // 初始化
        retVal = ql_ms_sim_init();
        if (QL_ERR_OK != retVal)
        {
            LOG_Error("sim_slot_2: %s init fail.retVal:%d", __FUNCTION__,retVal);
            return RET_ERR;        
        }

        retVal = HalModemSimUpdateActiveSlot();
        if(retVal < 0)
        {
            LOG_Error("sim_slot_2: %s Hal Modem Sim Update Active Slot Fail retVal:%d", __FUNCTION__,retVal);
            return RET_ERR;
        }
        return retVal;
    }
   

    retVal = ql_ms_sim_get_card_info(QL_SIM_SLOT_2, &info);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("sim_slot_2: %s GetCardStatus fail retVal:%d", __FUNCTION__,retVal);
        return RET_ERR;
    }

    if ((QL_SIM_CARD_STATE_PRESENT == info.state))     
    {
        // retVal = ql_ms_sim_get_sim_slot_active(QL_SIM_SLOT_2,&slotstatus);
        // if(retVal != QL_ERR_OK)
        // {
        //     LOG_Error("%s Get Sim QL_SIM_SLOT_2 Slot  Fail retVal:%d", __FUNCTION__,retVal);
        // }

        // if(slotstatus == 1)
        // {
        //     activesimslot = 2;
        // }
        activesimslot = 2;
    }
    else if((QL_SIM_CARD_STATE_PRESENT != info.state))
    {
        retVal = ql_ms_sim_get_sim_slot_active(QL_SIM_SLOT_1,&slotstatus);
        if(retVal != QL_ERR_OK)
        {
             LOG_Error("sim_slot_2: %s Get Sim QL_SIM_SLOT_1 Slot  Fail retVal:%d", __FUNCTION__,retVal);
        }

        if(slotstatus == 1)
        {
            activesimslot = 1;
        }
        else
        {
            activesimslot = 0;
            LOG_Error("sim_slot_2: %s QL_SIM_SLOT_1 ql_ms_sim_get_sim_slot_active retVal:%d", __FUNCTION__, activesimslot);
        }
    }
   
    //LOG_Debug("%s Sim State: %x Sim Slot %d Active ", __FUNCTION__,info.state,activesimslot);

    if(activesimslot != 1 && activesimslot != 2)
    {
        LOG_Error("sim_slot_2: %s Get Sim Slot Active Error", __FUNCTION__);
        return RET_ERR;
    }
    return activesimslot;
}

int32_t HalModemSimGetImsi(uint8_t *imsi, uint8_t len)
{
    ql_sim_card_info_t info = {0};
    QL_SIM_APP_TYPE_E type;
    int32_t ret = RET_ERR;
    int32_t retVal = -1;
    // add
    int32_t activesimslot = 0;
    QL_SIM_SLOT_E slot = QL_SIM_SLOT_1;

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

    // add
    activesimslot = HalModemSimGetActiveSlot_2(false);
    if(activesimslot < 0 )
    {
        LOG_Error("%s HalModemSimGetActiveSlot_2 fail",__FUNCTION__);
        return ret;
    }
    if(activesimslot == 1)
    {
        slot = QL_SIM_SLOT_1;
    }else{
        slot = QL_SIM_SLOT_2;
    }
    retVal = ql_ms_sim_get_card_info(slot, &info);
    //

    //retVal = ql_ms_sim_get_card_info(QL_SIM_SLOT_1, &info);
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
    else if (QL_SIM_SUBSCRIPTION_SEC == info.app_3gpp.subscription)
    {
        type = QL_SIM_APP_TYPE_3GPP;
    }
    else if (QL_SIM_SUBSCRIPTION_SEC == info.app_3gpp2.subscription)
    {
        type = QL_SIM_APP_TYPE_3GPP2;
    }

    else if (QL_SIM_SUBSCRIPTION_SEC == info.app_isim.subscription)
    {
        type = QL_SIM_APP_TYPE_ISIM;
    }
    else 
    {
        type = QL_SIM_APP_TYPE_UNKNOWN;
        LOG_Error("%s APP type unknow.", __FUNCTION__);
        return ret;
    }


    ret = ql_ms_sim_get_imsi(slot, type, (char*)imsi, len);
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
    // add
    int32_t activesimslot = 0;
    QL_SIM_SLOT_E slot = QL_SIM_SLOT_1;


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

    // add
    activesimslot = HalModemSimGetActiveSlot_2(false);
    if(activesimslot < 0 )
    {
        LOG_Error("sim_slot_2: %s HalModemSimGetActiveSlot_2 fail",__FUNCTION__);
        return ret;
    }

    if(activesimslot == 1)
    {
        slot = QL_SIM_SLOT_1;
    }else{
        slot = QL_SIM_SLOT_2;
    }

    retVal = ql_ms_sim_get_iccid(slot, (char*)iccid, len);
    //retVal = ql_ms_sim_get_iccid(QL_SIM_SLOT_1, (char*)iccid, len);
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
    // add
    int32_t activesimslot = 0;
    QL_SIM_SLOT_E slot = QL_SIM_SLOT_1;

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

    // add
    activesimslot = HalModemSimGetActiveSlot_2(false);
    if(activesimslot < 0 )
    {
        LOG_Error("sim_slot_2: %s HalModemSimGetActiveSlot_2 fail",__FUNCTION__);
        return ret;
    }

    if(activesimslot == 1)
    {
        slot = QL_SIM_SLOT_1;
    }else{
        slot = QL_SIM_SLOT_2;
    }
    retVal = ql_ms_sim_get_phone_num(slot, QL_SIM_APP_TYPE_3GPP, (char *)msisdn, len);


    //retVal = ql_ms_sim_get_phone_num(QL_SIM_SLOT_1, QL_SIM_APP_TYPE_3GPP, (char *)msisdn, len);

    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_sim_get_phone_num fail.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

