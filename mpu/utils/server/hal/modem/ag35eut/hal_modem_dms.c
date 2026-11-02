#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "ql_dm.h"
#include "ql_type.h"
#include "sr_log.h"
#include "hal_modem_dms.h"

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
    int32_t retVal = -1;

    if (HAL_MODEM_DMS_INITIALED == gDmsPriv.initialed)
    {
        LOG_Debug("%s have initialed.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }

    retVal = ql_dm_init();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_dm_init failed.", __FUNCTION__);
        return ret;
    }

    gDmsPriv.initialed = HAL_MODEM_DMS_INITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemDmsDeinit(void)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    retVal = ql_dm_deinit();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_dm_deinit failed.", __FUNCTION__);
        return ret;
    }

    gDmsPriv.initialed = HAL_MODEM_DMS_UNINITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemDmsSetMode(HalModemDmsMode_e mode)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

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


    QL_DM_AIR_PLANE_MODE_TYPE_E modeType = QL_DM_AIR_PLANE_MODE_OFF;

    if (E_HAL_MODEM_DMS_AIRPLANE_ON == mode)
    {
        modeType = QL_DM_AIR_PLANE_MODE_ON;
    }

    retVal = ql_dm_set_air_plane_mode(modeType);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_dm_set_air_plane_mode failed.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

int32_t HalModemDmsGetMode(HalModemDmsMode_e *mode)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }
    QL_DM_AIR_PLANE_MODE_TYPE_E modeType = QL_DM_AIR_PLANE_MODE_UNKNOWN;
    retVal = ql_dm_get_air_plane_mode(&modeType);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s get mode failed.", __FUNCTION__);
        return ret;        
    }

    LOG_Debug("%s mode:%d.", __FUNCTION__, modeType);
    *mode = E_HAL_MODEM_DMS_AIRPLANE_UNKOWN;
    if (QL_DM_AIR_PLANE_MODE_ON == modeType)
    {
        *mode = E_HAL_MODEM_DMS_AIRPLANE_ON;
    }

    if (QL_DM_AIR_PLANE_MODE_OFF == modeType)
    {
        *mode = E_HAL_MODEM_DMS_AIRPLANE_OFF;
    }

    ret = RET_OK;
    return ret;
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
    int32_t retVal = -1;

    if (!imei || (len == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    ql_dm_device_serial_numbers_info_t info;

    retVal = ql_dm_get_device_serial_numbers(&info);

    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s get Serial Num failed.", __FUNCTION__);
        return ret;        
    }

    if ((1 != info.imei_valid))
    {
        LOG_Error("%s get Serial invalid.", __FUNCTION__);
        return ret; 
    }

    strcpy((char*) imei, info.imei);

    ret = RET_OK;
    return ret;
}