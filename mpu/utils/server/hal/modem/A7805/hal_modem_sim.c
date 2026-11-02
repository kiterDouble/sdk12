#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_comm.h"
#include "hal_modem_sim.h"
#include "Sunsea_UIM.h"

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

    if (HAL_MODEM_SIM_INITIALED == gSimPriv.initialed)
    {
        LOG_Error("%s have init.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }

    if (HalModemCommInit() != RET_OK)
    {
        LOG_Error("%s HalModemCommInit failed.", __FUNCTION__);
        return ret;
    }

    gSimPriv.initialed = HAL_MODEM_SIM_INITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemSimDeinit(void)
{
    int32_t ret = RET_ERR;

    if (HAL_MODEM_SIM_INITIALED != gSimPriv.initialed)
    {
        LOG_Error("%s have not init.", __FUNCTION__);
        return ret;
    }

    gSimPriv.initialed = HAL_MODEM_SIM_UNINITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemSimGetCardStatus(HalModemSimCardStatus_e *cardStat, HalModemSimAppStatus_e *appStat)
{
    int32_t ret = RET_ERR;

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

    uim_card_status_e status;
    if(Sunsea_UIM_GetUimCardStatus(UIM_ID_FIRST, &status) < 0)
    {
        return ret;
    }

    if (UIM_CARD_NOT_INSERTED == status)
    {
        *cardStat = E_HAL_MODEM_SIM_CARD_STATUS_ABSENT;
        *appStat = E_HAL_MODEM_SIM_APP_STATUS_ERROR;
    }
    else if (UIM_CARD_READY == status)
    {
        *cardStat = E_HAL_MODEM_SIM_CARD_STATUS_PRESENT;
        *appStat = E_HAL_MODEM_SIM_APP_STATUS_READY;
    }
    else if (UIM_CARD_BLOCKED == status)
    {
        *cardStat = E_HAL_MODEM_SIM_CARD_STATUS_BLOCKED;
        *appStat = E_HAL_MODEM_SIM_APP_STATUS_ERROR;
    }
    else
    {
        *cardStat = E_HAL_MODEM_SIM_CARD_STATUS_ERROR;
        *appStat = E_HAL_MODEM_SIM_APP_STATUS_ERROR;
    }

    ret = RET_OK;
    return ret;
}

int32_t HalModemSimGetImsi(uint8_t *imsi, uint8_t len)
{
    int32_t ret = RET_ERR;

    if (!imsi || !len)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;        
    }

    if (HAL_MODEM_SIM_INITIALED != gSimPriv.initialed)
    {
        LOG_Error("%s have not init.", __FUNCTION__);
        return ret;
    }
    
    int32_t imeiLen = 0;
    return Sunsea_UIM_GetIMSI(UIM_ID_FIRST, (char*)imsi, &imeiLen);
}

int32_t HalModemSimGetIccid(uint8_t *iccid, uint8_t len)
{
    int32_t ret = RET_ERR;

    if (!iccid || !len)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;        
    }

    if (HAL_MODEM_SIM_INITIALED != gSimPriv.initialed)
    {
        LOG_Error("%s have not init.", __FUNCTION__);
        return ret;
    }
    
    int32_t iccidLen = 0;
    return Sunsea_UIM_GetICCID(UIM_ID_FIRST, (char*)iccid, &iccidLen);
}

int32_t HalModemSimGetMsisdn(uint8_t *msisdn, uint8_t len)
{
    return RET_OK;
}
