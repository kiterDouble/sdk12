#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ql_util_common.h"
#include "ql-sdk/ql_type.h"
#include "ql-sdk/ql_ms_dm.h"

#include "err_code.h"
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

    retVal = ql_ms_dm_init();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s QL_MCM_DM_Client_Init failed.", __FUNCTION__);
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

    retVal = ql_ms_dm_deinit();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s QL_MCM_DM_Client_Deinit failed.", __FUNCTION__);
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

    QL_DM_AIR_PLANE_MODE_TYPE_E air_plane_mode = QL_DM_AIR_PLANE_MODE_ON;
    if (E_HAL_MODEM_DMS_AIRPLANE_OFF == mode)
    {
        air_plane_mode = QL_DM_AIR_PLANE_MODE_OFF;
    }

    retVal = ql_ms_dm_set_air_plane_mode(QL_SIM_1, air_plane_mode);

    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_dm_set_air_plane_mode failed.ret:%d", __FUNCTION__, retVal);
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

    QL_DM_AIR_PLANE_MODE_TYPE_E air_plane_mode;
    retVal = ql_ms_dm_get_air_plane_mode(QL_SIM_1, &air_plane_mode);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s get air_plane_mode failed.", __FUNCTION__);
        return ret;        
    }

    LOG_Debug("%s air_plane_mode:%d.", __FUNCTION__, air_plane_mode);
    *mode = E_HAL_MODEM_DMS_AIRPLANE_UNKOWN;
    if (QL_DM_AIR_PLANE_MODE_ON == air_plane_mode)
    {
        *mode = E_HAL_MODEM_DMS_AIRPLANE_ON;
    }

    if (QL_DM_AIR_PLANE_MODE_OFF == air_plane_mode)
    {
        *mode = E_HAL_MODEM_DMS_AIRPLANE_OFF;
    }

    ret = RET_OK;
    return ret;
}

int32_t HalModemDmsSetQoosEnableStatus(HalModemDmsQoosEnableStatusType_e enable)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    retVal = ql_ms_dm_set_qoos_enable(QL_SIM_1, enable);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_dm_set_qoos_enable failed.", __FUNCTION__);
        return ret;        
    }

    return RET_OK;
}

int32_t HalModemDmsGetQoosEnableStatus(HalModemDmsQoosEnableStatusType_e *enable)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    QL_DM_QOOS_ENABLE_TYPE_E value;
    retVal = ql_ms_dm_get_qoos_enable(QL_SIM_1, &value);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_dm_get_qoos_enable failed.", __FUNCTION__);
        return ret;        
    }

    *enable = value;
    return RET_OK;
}

int32_t HalModemDmsSetQoosConfig(const HalModemDmsQoosConfig_t *qoosCfg)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (!qoosCfg)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    ql_dm_qoos_config_t config;

    config.pre_time = qoosCfg->preTime;
    config.sleep_time = qoosCfg->sleepTime;
    config.acq_time = qoosCfg->acqTime;
    config.cir_cnt = qoosCfg->cirCnt;
    config.sleep_step = qoosCfg->sleepStep;
    config.acq_step = qoosCfg->acqStep;
    config.adj_cnt = qoosCfg->adjCnt;
    config.idle_interval = qoosCfg->idleInterval;

    retVal = ql_ms_dm_set_qoos_config(QL_SIM_1, config);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_dm_get_qoos_enable failed.", __FUNCTION__);
        return ret;        
    }
    return RET_OK;
}

int32_t HalModemDmsGetQoosConfig(HalModemDmsQoosConfig_t *qoosCfg)
{

    int32_t ret = RET_ERR;
    int32_t retVal = -1;


    if (!qoosCfg)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    ql_dm_qoos_config_t config;

    memset(&config, 0, sizeof(config));
    retVal = ql_ms_dm_get_qoos_config(QL_SIM_1, &config);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s get ql_ms_dm_get_qoos_config failed.", __FUNCTION__);
        return ret;        
    }

    qoosCfg->preTime = config.pre_time;
    qoosCfg->sleepTime = config.sleep_time; 
    qoosCfg->acqTime = config.acq_time;
    qoosCfg->cirCnt = config.cir_cnt;
    qoosCfg->sleepStep = config.sleep_step;
    qoosCfg->acqStep = config.acq_step;
    qoosCfg->adjCnt = config.adj_cnt;
    qoosCfg->idleInterval = config.idle_interval;
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

    ql_dm_device_serial_numbers_info_t t_info;
    memset(&t_info, 0, sizeof(ql_dm_device_serial_numbers_info_t));

    retVal = ql_ms_dm_get_device_serial_numbers(&t_info);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s get ql_ms_dm_get_device_serial_numbers failed.", __FUNCTION__);
        return ret;        
    }

    if(1 != t_info.imei_valid)
    {
        LOG_Error("%s get Serial invalid.", __FUNCTION__);
        return ret;
    }

    LOG_Debug("%s imei:%s.", __FUNCTION__, t_info.imei);
    strcpy((char*) imei, t_info.imei);

    ret = RET_OK;
    return ret;
}