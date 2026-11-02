#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ql_type.h"
#include "ql_dm.h"
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

/**
 * @name HalModemDmsInit
 * @brief 初始化Modem的DMS模块。
 *        该函数负责检查DMS模块是否已经初始化，如果未初始化，则执行初始化过程。
 * @param null
 * @return 返回初始化结果。RET_OK表示成功，RET_ERR表示失败。
 */
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

/**
 * @name HalModemDmsDeinit
 * @brief Modem DMS服务反初始化。
 * @param null
 * @return 返回初始化结果。RET_OK表示成功，RET_ERR表示失败。
 */
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

/**
 * @name HalModemDmsSetMode
 * @brief modem dms设置飞行模式。
 *        该函数用于根据传入的模式参数设置调制解调器的飞行模式状态，支持开启或关闭飞行模式。
 * @param mode 指定的飞行模式状态，使用HalModemDmsMode_e枚举类型。
 *             E_HAL_MODEM_DMS_AIRPLANE_OFF 表示关闭飞行模式，
 *             E_HAL_MODEM_DMS_AIRPLANE_ON 表示开启飞行模式。
 * @return 返回初始化结果。RET_OK表示成功，RET_ERR表示失败。
 */
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

/**
 * @name HalModemDmsGetMode
 * @brief 获取当前Modem的飞行模式状态。
 * @param mode 指向HalModemDmsMode_e类型的指针，用于存储获取到的飞行模式状态。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
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

/**
 * @name HalModemDmsSetQoosEnableStatus
 * @brief 设置Modem的QoS启用状态
 *        本函数用于设置Modem设备的QooS启用状态。根据输入的启用状态，
 *        对Modem的QoS功能进行开启或关闭。
 * @param enable QoS启用状态类型，采用HalModemDmsQoosEnableStatusType_e枚举类型。
 *               如果为启用状态，则开启QoS功能；如果为禁用状态，则关闭QoS功能。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDmsSetQoosEnableStatus(HalModemDmsQoosEnableStatusType_e enable)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }
    
    retVal = ql_dm_set_qoos_enable(enable);

    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s set qoos enable fail.", __FUNCTION__);
        return ret;       
    }
    ret = RET_OK;

    return ret;
    return RET_OK;
}

/**
 * @name HalModemDmsGetQoosEnableStatus
 * @brief 获取QoS启用状态。
 * @param enable QoS启用状态类型，采用HalModemDmsQoosEnableStatusType_e枚举类型。
 *               如果为启用状态，则开启QooS功能；如果为禁用状态，则关闭QooS功能。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDmsGetQoosEnableStatus(HalModemDmsQoosEnableStatusType_e *enable)
{
    QL_DM_QOOS_ENABLE_TYPE_E qlEnable;
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    retVal = ql_dm_get_qoos_enable(&qlEnable);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s get qoos enable fail.", __FUNCTION__);
        return ret;       
    }

    *enable = qlEnable;

    ret = RET_OK;

    return ret;
    return RET_OK;
}

/**
 * @name HalModemDmsSetQoosConfig
 * @brief 设置Modem的QoS配置
 *        本函数用于设置Modem设备的QooS配置。
 * @param qoosCfg 指向包含QoS配置信息的结构体HalModemDmsQoosConfig_t的指针。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDmsSetQoosConfig(const HalModemDmsQoosConfig_t *qoosCfg)
{
    ql_dm_qoos_config_t config;
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

    config.pre_time = qoosCfg->preTime;
    config.sleep_time = qoosCfg->sleepTime;
    config.acq_time = qoosCfg->acqTime;
    config.cir_cnt = qoosCfg->cirCnt;
    config.sleep_step = qoosCfg->sleepStep;
    config.acq_step = qoosCfg->acqStep;
    config.adj_cnt = qoosCfg->adjCnt;
    config.idle_interval = qoosCfg->idleInterval;

    retVal = ql_dm_set_qoos_config(config);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s set config failed.", __FUNCTION__);
        return ret;        
    }

    ret = RET_OK;
    return ret;
}

/**
 * @name HalModemDmsGetQoosConfig
 * @brief 获取Modem的QoS配置信息
 *        本函数用于获取Modem设备的QooS配置信息。
 * @param qoosCfg 指向一个HalModemDmsQoosConfig_t类型的指针，用于接收QooS配置信息。
 * @return 函数返回一个整型值。在当前实现中，总是返回RET_OK，表示操作成功。
 */
int32_t HalModemDmsGetQoosConfig(HalModemDmsQoosConfig_t *qoosCfg)
{
    ql_dm_qoos_config_t config;
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

    retVal = ql_dm_get_qoos_config(&config);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s get config failed.", __FUNCTION__);
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
    ret = RET_OK;

    return ret;
}

/**
 * @name HalModemDmsGetImei
 * @brief 获取设备的IMEI号
 * @param imei 指向存储IMEI号的缓冲区的指针。
 * @param len  imei缓冲区的长度。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
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