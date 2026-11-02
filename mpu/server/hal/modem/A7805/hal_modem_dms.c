#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_dms.h"
#include "hal_modem_comm.h"
#include "Sunsea_DM.h"

#define HAL_MODEM_DMS_INITIALED (0)
#define HAL_MODEM_DMS_UNINITIALED (-1)
typedef struct
{
    int32_t initialed;
}HalModemDmsPriv_t;

static HalModemDmsPriv_t gDmsPriv =
{
    .initialed = HAL_MODEM_DMS_UNINITIALED
};

int32_t HalModemDmsInit(void)
{
    int32_t ret = RET_ERR;

    if (HAL_MODEM_DMS_INITIALED == gDmsPriv.initialed)
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
    gDmsPriv.initialed = HAL_MODEM_DMS_INITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemDmsDeinit(void)
{
    int32_t ret = RET_ERR;

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }
    gDmsPriv.initialed = HAL_MODEM_DMS_UNINITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemDmsSetMode(HalModemDmsMode_e mode)
{
    int32_t ret = RET_ERR;

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }
    if ((mode != E_HAL_MODEM_DMS_AIRPLANE_OFF) && (mode != E_HAL_MODEM_DMS_AIRPLANE_ON))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    phone_mode_e modeType = PHONE_MODE_FULL;

    if (E_HAL_MODEM_DMS_AIRPLANE_ON == mode)
    {
        modeType = PHONE_MODE_LPM;
    }
    return Sunsea_DM_SetPhoneMode(modeType);
}

int32_t HalModemDmsGetMode(HalModemDmsMode_e *mode)
{
    int32_t ret = RET_ERR;

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }
    if (NULL == mode)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    phone_mode_e phone_mode;
    if(Sunsea_DM_GetPhoneMode(&phone_mode) < 0)
    {
        LOG_Error("%s Sunsea_DM_GetPhoneMode fail.", __FUNCTION__);
        return ret;
    }
    
    *mode = E_HAL_MODEM_DMS_AIRPLANE_UNKOWN;
    if (PHONE_MODE_LPM == phone_mode)
    {
        *mode = E_HAL_MODEM_DMS_AIRPLANE_ON;
    }

    if (PHONE_MODE_FULL == phone_mode)
    {
        *mode = E_HAL_MODEM_DMS_AIRPLANE_OFF;
    }

    return RET_OK;
}

int32_t HalModemDmsSetQoosEnableStatus(HalModemDmsQoosEnableStatusType_e enable)
{
    return RET_OK;
}

int32_t HalModemDmsGetQoosEnableStatus(HalModemDmsQoosEnableStatusType_e *enable)
{
    return RET_OK;
}

int32_t HalModemDmsSetQoosConfig(const HalModemDmsQoosConfig_t *qoosCfg)
{
    return RET_OK;
}

int32_t HalModemDmsGetQoosConfig(HalModemDmsQoosConfig_t *qoosCfg)
{
    return RET_OK;
}

int32_t HalModemDmsGetImei(uint8_t *imei, uint8_t len)
{
    int32_t ret = RET_ERR;

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }
    if (NULL == imei)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    int32_t imeiLen = 0;
    return Sunsea_DM_GetIMEI((char*) imei, &imeiLen);
}