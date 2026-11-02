#include <stdlib.h>
#include <string.h>
#include <stdint.h>
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

int32_t HalModemPmAutosleepEnable(int8_t enable)
{
    int32_t ret = RET_ERR;

    if ((0 != enable) && (1 != enable))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    if (0 != ql_autosleep_enable(enable))
    {
        LOG_Error("%s call ql_autosleep_enable fail.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

int32_t HalModemForbidInd(uint32_t ForbidIndMask)
{
    if(0 == ForbidIndMask)
    {
        LOG_Debug("%s : set mode = %d.",__FUNCTION__,ForbidIndMask);
        ql_nw_set_power_mode(0);
    }
    else
    {
        LOG_Debug("%s : set mode = %d.",__FUNCTION__,0x16);
        ql_nw_set_power_mode(0x16);
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
        if (0 == strcmp((char *)name, (char *)gPmPriv.lockName))
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

    gPmPriv.fd = ql_slp_wakelock_create((char *)name, strlen((char *)name));
    if (gPmPriv.fd < 0)
    {
        LOG_Error("%s ql_slp_wakelock_create failed.", __FUNCTION__);
        return ret;          
    }
    gPmPriv.initialed = HAL_MODEM_PM_INITIALED;
    snprintf((char *)gPmPriv.lockName, sizeof(gPmPriv.lockName), "%s", (char *)name);
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

    if (0 != strcmp((char *)name, (char *)gPmPriv.lockName))
    {
        LOG_Error("%s wakelock:%s not create.created:%s.", __FUNCTION__, (char *)name, (char *)gPmPriv.lockName);
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

    if (0 != strcmp((char *)name, (char *)gPmPriv.lockName))
    {
        LOG_Error("%s wakelock:%s not create.created:%s.", __FUNCTION__, (char *)name, (char *)gPmPriv.lockName);
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

    if (0 != strcmp((char *)name, (char *)gPmPriv.lockName))
    {
        LOG_Error("%s wakelock:%s not create.created:%s.", __FUNCTION__, (char *)name, (char *)gPmPriv.lockName);
        return ret;
    }

    ql_slp_wakelock_destroy(gPmPriv.fd);
    gPmPriv.fd = -1;
    gPmPriv.initialed = HAL_MODEM_PM_UNINITIALED;
    memset(gPmPriv.lockName, 0x0, sizeof(gPmPriv.lockName));
    ret = RET_OK;
    return ret;    
}