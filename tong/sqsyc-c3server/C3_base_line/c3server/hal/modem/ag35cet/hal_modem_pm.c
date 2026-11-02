#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/reboot.h>
#include <unistd.h>

#include "err_code.h"
#include "ql_sleep_wakelock.h"
#include "sr_log.h"
#include "hal_modem_pm.h"
#include "ql_nw.h"

#define HAL_MODEM_PM_NAME_MAX (256)

#define HAL_MODEM_PM_INITIALED (0)
#define HAL_MODEM_PM_UNINITIALED (-1)

typedef struct
{
    int32_t fd;
    int32_t initialed;
    uint8_t lockName[HAL_MODEM_PM_NAME_MAX];
}HalModemPmPriv_t;

static HalModemPmPriv_t gPmPriv =
{
    .fd = -1,
    .initialed = HAL_MODEM_PM_UNINITIALED,
    .lockName = {0x0,}
};
static uint32_t g_oldForbidIndMask = 0xFFFFFFFF; // 上一次设置的电源模式标志

int32_t HalModemPmAutosleepEnable(int8_t enable)
{
    int32_t ret = RET_ERR;

    if ((0 != enable) && (1 != enable))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    if (0 != ql_autosleep_enable((char)enable))
    {
        LOG_Error("%s call ql_autosleep_enable fail.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

int32_t HalModemForbidInd(uint32_t ForbidIndMask)
{
    LOG_Debug("%s : set ForbidIndMask = 0x%08x, old ForbidIndMask = 0x%08x", __FUNCTION__, ForbidIndMask,
              g_oldForbidIndMask);
    if (g_oldForbidIndMask == ForbidIndMask)
    {
        return RET_OK;
    }

    g_oldForbidIndMask = ForbidIndMask;
    if(0U == ForbidIndMask)
    {
        LOG_Debug("%s : set mode = %d.",__FUNCTION__,ForbidIndMask);
        (void)ql_nw_set_power_mode(0);
    }
    else
    {
        LOG_Debug("%s : set mode = %d.",__FUNCTION__,0x16);
        (void)ql_nw_set_power_mode(0x16);
    }
    return RET_OK;
}

int32_t HalModemPmWakeLockCreate(const uint8_t *name)
{
    int32_t ret = RET_ERR;

    if (!name)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_PM_INITIALED == gPmPriv.initialed)
    {
        if (0 == strcmp((const char *)name, (char *)gPmPriv.lockName))
        {
            LOG_Error("%s have init,return ok.", __FUNCTION__);
            ret = RET_OK;
            return ret;            
        }
        else
        {
            LOG_Error("%s have init, fail.lockName:%s.", __FUNCTION__, gPmPriv.lockName);
            return ret;  
        }

    }

    gPmPriv.fd = ql_slp_wakelock_create((const char *)name, strlen((const char *)name));
    if (gPmPriv.fd < 0)
    {
        LOG_Error("%s ql_slp_wakelock_create failed.", __FUNCTION__);
        return ret;          
    }
    gPmPriv.initialed = HAL_MODEM_PM_INITIALED;
    (void)snprintf((char *)gPmPriv.lockName, sizeof(gPmPriv.lockName), "%s", (const char *)name);
    ret = RET_OK;
    return ret;
}

int32_t HalModemPmWakeLockLocked(const uint8_t *name)
{
    int32_t ret = RET_ERR;

    if (!name)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    } 

    if (HAL_MODEM_PM_INITIALED != gPmPriv.initialed)
    {
        LOG_Error("%s have not created.", __FUNCTION__);
        return ret;
    }

    if (0 != strcmp((const char *)name, (char *)gPmPriv.lockName))
    {
        LOG_Error("%s wakelock:%s not create.created:%s.", __FUNCTION__, (const char *)name, (char *)gPmPriv.lockName);
        return ret;
    }
    
    if (0 != ql_slp_wakelock_lock(gPmPriv.fd))
    {
        LOG_Error("%s ql_slp_wakelock_lock fail.", __FUNCTION__);
       return ret; 
    }

    ret = RET_OK;
    return ret;    
}

int32_t HalModemPmWakeLockUnlocked(const uint8_t *name)
{
    int32_t ret = RET_ERR;

    if (!name)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_PM_INITIALED != gPmPriv.initialed)
    {
        LOG_Error("%s have not created.", __FUNCTION__);
        return ret;
    }

    if (0 != strcmp((const char *)name, (char *)gPmPriv.lockName))
    {
        LOG_Error("%s wakelock:%s not create.created:%s.", __FUNCTION__, (const char *)name, (char *)gPmPriv.lockName);
        return ret;
    }

    if (0 != ql_slp_wakelock_unlock(gPmPriv.fd))
    {
        LOG_Error("%s ql_slp_wakelock_unlock fail.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

int32_t HalModemPmWakeLockLockedEx(const uint8_t *name)
{
    int32_t fd = -1;

    if (!name)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return -1;
    }

    fd = ql_slp_wakelock_create((const char *)name, strlen((const char *)name));
    if (fd < 0)
    {
        LOG_Error("%s ql_slp_wakelock_create failed.", __FUNCTION__);
        return -1;
    }

    if (0 != ql_slp_wakelock_lock(fd))
    {
        LOG_Error("%s ql_slp_wakelock_lock fail.", __FUNCTION__);
        (void)ql_slp_wakelock_destroy(fd);
        return -1;
    }

    LOG_Debug("%s : create wakelock [%s] fd = %d.", __FUNCTION__, name, fd);
    return fd;
}

int32_t HalModemPmWakeLockUnlockedEx(int32_t fd)
{
    int32_t ret = RET_ERR;

    if (fd < 0)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (0 != ql_slp_wakelock_unlock(fd))
    {
        LOG_Error("%s ql_slp_wakelock_unlock fail.", __FUNCTION__);
        (void)ql_slp_wakelock_destroy(fd);
        return ret;
    }

    (void)ql_slp_wakelock_destroy(fd);
    LOG_Debug("%s : destroy wakelock fd = %d.", __FUNCTION__, fd);

    ret = RET_OK;
    return ret;
}

int32_t HalModemPmWakeLockDestroy(const uint8_t *name)
{
    int32_t ret = RET_ERR;

    if (!name)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_PM_UNINITIALED == gPmPriv.initialed)
    {
        LOG_Error("%s have not init.", __FUNCTION__);
        return ret;  

    }

    if (0 != strcmp((const char *)((const void*)name), (char *)((void*)gPmPriv.lockName)))
    {
        LOG_Error("%s wakelock:%s not create.created:%s.", __FUNCTION__, (const char *)((const void*)name), (char *)((void*)gPmPriv.lockName));
        return ret;
    }

    (void)ql_slp_wakelock_destroy(gPmPriv.fd);
    gPmPriv.fd = -1;
    gPmPriv.initialed = HAL_MODEM_PM_UNINITIALED;
    (void)memset(gPmPriv.lockName, 0x0, sizeof(gPmPriv.lockName));
    ret = RET_OK;
    return ret;    
}

void HalModemPmDeepSleepPowerDown(int mode)
{
    LOG_Debug("%s : sleep into deep sleep mode , power off.",__FUNCTION__);
    //system("ql_cmd at AT+QPOWD=1");

    (void)sync();

    (void)reboot(RB_POWER_OFF);

    return;
}


