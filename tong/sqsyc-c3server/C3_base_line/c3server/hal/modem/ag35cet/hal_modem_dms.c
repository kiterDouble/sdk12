#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "err_code.h"
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include "ql_dm.h"
#include "ql_type.h"
#include "sr_log.h"
#include "hal_modem_dms.h"

#define HAL_MODEM_DMS_CPU_TEMP_PATH "/sys/class/thermal/thermal_zone0/temp"

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

    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);
    if (HAL_MODEM_DMS_INITIALED == gDmsPriv.initialed)
    {
        LOG_Debug("%s have initialed.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }

    retVal = ql_dm_init();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_dm_init failed,retVal:%d", __FUNCTION__, retVal);
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
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    retVal = ql_dm_deinit();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_dm_deinit failed,retVal:%d", __FUNCTION__, retVal);
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
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

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
        LOG_Error("%s ql_dm_set_air_plane_mode failed,retVal:%d", __FUNCTION__, retVal);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

int32_t HalModemDmsGetMode(HalModemDmsMode_e *mode)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }
    QL_DM_AIR_PLANE_MODE_TYPE_E modeType = QL_DM_AIR_PLANE_MODE_UNKNOWN;
    retVal = ql_dm_get_air_plane_mode(&modeType);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s get mode failed,retVal:%d", __FUNCTION__, retVal);
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
    int ret = 0;

    LOG_Debug("func:%s enable:%d", __FUNCTION__, enable);
    ret = ql_dm_set_qoos_enable(enable);
    if(ret != 0)
    {
        LOG_Debug("ql_dm_set_qoos_enable failed, ret = %d", ret);
        return RET_ERR;
    }
    LOG_Debug("ql_dm_set_qoos_enable ret = %d", ret);

    return RET_OK;
}

int32_t HalModemDmsGetQoosEnableStatus(HalModemDmsQoosEnableStatusType_e *enable)
{
    int ret = 0;
    int enableTemp = 0;
    ret = ql_dm_get_qoos_enable(&enableTemp);
    if(ret != 0)
    {
        LOG_Debug("ql_dm_get_qoos_enable failed, ret = %d", ret);
        return RET_ERR;
    }
    LOG_Debug("ql_dm_get_qoos_enable, enable is %d", enableTemp);
    if (enableTemp == 1)
    {
        *enable = E_HAL_MODEM_DMS_QOOS_ENABLE_STATE_ON;
    }
    else
    {
        *enable = E_HAL_MODEM_DMS_QOOS_ENABLE_STATE_OFF;
    }

    return RET_OK;
}

int32_t HalModemDmsSetQoosConfig(const HalModemDmsQoosConfig_t *qoosCfg)
{
    int ret = 0;
    int phase1 = 0,phase2 = 0,phase3 = 0;

    phase1 = qoosCfg->timeSec1;
    phase2 = qoosCfg->timeSec2;
    phase3 = qoosCfg->timeSec3;
    LOG_Debug("func:%s timeSec1:%d timeSec2:%d timeSec3:%d", __FUNCTION__, phase1, phase2, phase3);
    ret = ql_dm_set_qoos_config(phase1, phase2, phase3);
    if(ret != 0)
    {
        LOG_Debug("ql_dm_set_qoos_config failed, ret = %d", ret);
        return RET_ERR;
    }
    LOG_Debug("ql_dm_set_qoos_config ret = %d", ret);
    return RET_OK;
}

int32_t HalModemDmsGetQoosConfig(HalModemDmsQoosConfig_t *qoosCfg)
{
    int ret = 0;
    int phase1 = 0,phase2 = 0,phase3 = 0;
    ret = ql_dm_get_qoos_config(&phase1, &phase2, &phase3);
    if (ret != 0)
    {
        LOG_Debug("ql_dm_get_qoos_config failed, ret = %d", ret);
        return RET_ERR;
    }

    LOG_Debug("%s qoos param: %d, %d, %d\n", __FUNCTION__, phase1, phase2, phase3);
    qoosCfg->timeSec1 = phase1;
    qoosCfg->timeSec2 = phase2;
    qoosCfg->timeSec3 = phase3;
    return RET_OK;
}

int32_t HalModemDmsGetImei(uint8_t *imei, uint8_t len)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

    if (!imei || (len == 0U))
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
        LOG_Error("%s get Serial Num failed,retVal:%d", __FUNCTION__, retVal);
        return ret;        
    }

    if ((1 != info.imei_valid))
    {
        LOG_Error("%s get Serial invalid.", __FUNCTION__);
        return ret; 
    }

    (void)strcpy((char*) imei, info.imei);

    ret = RET_OK;
    return ret;
}

/**
 * @brief Reboot the system in a safer way: sync + retry + errno + exec fallback
 * @param eventName Reboot reason/event name (can be NULL)
 * @return 0 on success (normally not returned), negative value on failure
 */
int32_t HalModemReboot(const char *eventName)
{
    char    safeName[HAL_REBOOT_EVENT_NAME_MAX_LEN] = { 0 };
    int32_t attempt = 0;

    if (eventName != NULL)
    {
        /* Prevent log injection / too long strings */
        (void)snprintf(safeName, sizeof(safeName), "%.*s",
                       (int32_t)(sizeof(safeName) - 1U), eventName);
    }
    else
    {
        (void)snprintf(safeName, sizeof(safeName), "%s", "NULL");
    }

    LOG_Warn("Reboot requested: eventName=%s", safeName);

    /* Best-effort flush filesystem buffers */
    for (attempt = 0; attempt < (int32_t)HAL_REBOOT_SYNC_TIMES; attempt++)
    {
        sync();
    }

    /* Prefer syscall reboot first */
    for (attempt = 1; attempt <= (int32_t)HAL_REBOOT_MAX_ATTEMPTS; attempt++)
    {
        errno = 0;

        /* RB_AUTOBOOT: restart the system */
        if (reboot(RB_AUTOBOOT) == 0)
        {
            /* Normally never reaches here */
            return (int32_t)HAL_REBOOT_OK;
        }

        if (errno == EPERM)
        {
            LOG_Error("Reboot syscall failed (no privilege), errno=%d(%s)",
                      errno, strerror(errno));
            return (int32_t)HAL_REBOOT_ERR_NO_PRIV;
        }

        LOG_Error("Reboot syscall failed, attempt=%d/%d, errno=%d(%s)",
                  attempt, (int32_t)HAL_REBOOT_MAX_ATTEMPTS,
                  errno, strerror(errno));

        (void)usleep((useconds_t)HAL_REBOOT_RETRY_DELAY_US);
    }

    /*
     * Fallback: exec common reboot binaries (no shell, safer than system()).
     * Note: exec only returns if failed.
     */
    {
        char *const argv1[] = { (char *)"reboot", (char *)"-f", NULL };
        (void)execv("/sbin/reboot", argv1);

        char *const argv2[] = { (char *)"reboot", NULL };
        (void)execv("/bin/reboot", argv2);

        char *const argv3[] = { (char *)"busybox", (char *)"reboot", (char *)"-f", NULL };
        (void)execv("/bin/busybox", argv3);
    }

    LOG_Error("Reboot fallback exec failed, errno=%d(%s)", errno, strerror(errno));
    return (int32_t)HAL_REBOOT_ERR_SYS;
}

int32_t HalModemDmsGetCpuOccupancy(float *cpuOccupancy)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

    if (NULL == cpuOccupancy)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    retVal = ql_dm_get_cpu_occupancy(cpuOccupancy);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_dm_get_cpu_occupancy failed, retVal:%d", __FUNCTION__, retVal);
        *cpuOccupancy = 0.0f;
        return ret;
    }

    *cpuOccupancy = roundf(*cpuOccupancy * 100.0f) / 100.0f;

    ret = RET_OK;
    return ret;
}

int32_t HalModemDmsGetCpuTemperature(float *cpuTemperature)
{
    int32_t ret = RET_ERR;
    FILE *fp = NULL;
    char buf[32] = {0};
    long tempMilli = 0;
    float tempC = 0.0f;

    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

    if (NULL == cpuTemperature)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_DMS_INITIALED != gDmsPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    fp = fopen(HAL_MODEM_DMS_CPU_TEMP_PATH, "r");
    if (NULL == fp)
    {
        LOG_Error("%s open %s failed, errno:%d(%s)", __FUNCTION__,
                  HAL_MODEM_DMS_CPU_TEMP_PATH, errno, strerror(errno));
        *cpuTemperature = 0.0f;
        return ret;
    }

    if (NULL == fgets(buf, sizeof(buf), fp))
    {
        LOG_Error("%s read %s failed", __FUNCTION__, HAL_MODEM_DMS_CPU_TEMP_PATH);
        *cpuTemperature = 0.0f;
        (void)fclose(fp);
        return ret;
    }

    (void)fclose(fp);

    tempMilli = strtol(buf, NULL, 10);
    tempC = (float)tempMilli / 1000.0f;
    *cpuTemperature = roundf(tempC * 100.0f) / 100.0f;

    ret = RET_OK;
    return ret;
}