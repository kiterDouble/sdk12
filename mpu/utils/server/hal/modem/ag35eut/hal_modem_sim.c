#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "ql_type.h"
#include "ql_sim.h"
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
    retVal = ql_sim_init();

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

    retVal = ql_sim_deinit();

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
    ql_sim_card_info_t info;
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

    retVal = ql_sim_get_card_info(QL_SIM_SLOT_1, &info);
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

    if (info.app_3gpp.app_state == QL_SIM_APP_STATE_READY)
    {
        *appStat = E_HAL_MODEM_SIM_APP_STATUS_READY;
    }

    ret = RET_OK;
    return ret;
}

int32_t HalModemSimGetImsi(uint8_t *imsi, uint8_t len)
{
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

    retVal = ql_sim_get_imsi(QL_SIM_SLOT_1, QL_SIM_APP_TYPE_3GPP, (char *)imsi, len);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s QL_MCM_SIM_GetIMSI fail.", __FUNCTION__);
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
    retVal = ql_sim_get_iccid(QL_SIM_SLOT_1, (char *)iccid, len);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s GetICCID fail.", __FUNCTION__);
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

    retVal = ql_sim_get_phone_num(QL_SIM_SLOT_1, QL_SIM_APP_TYPE_3GPP, (char *)msisdn, len);

    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_sim_get_phone_num fail.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}
