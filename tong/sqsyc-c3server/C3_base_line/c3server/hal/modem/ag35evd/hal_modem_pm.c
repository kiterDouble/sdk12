#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "ql_oe.h"
#include "ql_sleep_wakelock.h"
#include "sr_log.h"
#include "hal_modem_pm.h"
#include "ql_nw.h"
#include "ql_powerdown.h"

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

/**
 * @name HalModemPmAutosleepEnable
 * @brief 启用或禁用modem的自动休眠功能。
 * @param enable 一个布尔值，0 表示禁用，1 表示启用。其他值将被视为无效。
 * @return 返回操作的结果状态。RET_OK 表示成功，RET_ERR 表示失败。
 */
int32_t HalModemPmAutosleepEnable(int8_t enable)
{
    int32_t ret = RET_ERR;

    if ((0 != enable) && (1 != enable))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    if (0 != Ql_Autosleep_Enable(enable))
    {
        LOG_Error("%s call Ql_Autosleep_Enable fail.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

/**
 * @name HalModemForbidInd
 * @brief 设置modem禁止掩码。
 * @param ForbidIndMask 禁止指示掩码。
 * @return 函数执行状态码。返回RET_OK表示成功。
 */
int32_t HalModemForbidInd(uint32_t ForbidIndMask)
{
    return QL_NW_ForbidInd(ForbidIndMask);
}

/**
 * @name HalModemPmWakeLockCreate
 * @brief 创建Modem电源管理唤醒锁。
 * @param name 唤醒锁的名称，类型为uint8_t数组。
 * @return 若成功创建唤醒锁，则返回RET_OK（正数）；若创建失败，则返回RET_ERR（负数）。
 */
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

    gPmPriv.fd = Ql_SLP_WakeLock_Create((char *)name, strlen((char *)name));
    gPmPriv.initialed = HAL_MODEM_PM_INITIALED;
    snprintf((char *)gPmPriv.lockName, sizeof(gPmPriv.lockName), "%s", (char *)name);
    ret = RET_OK;
    return ret;
}

/**
 * @name HalModemPmWakeLockLocked
 * @brief 锁定模组电源管理的唤醒锁
 * @param name: 指向要锁定的唤醒锁名称的指针
 * @return int32_t: 锁定操作的结果状态码，RET_OK表示成功，RET_ERR表示失败。
 */
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
        LOG_Error("%s have created.", __FUNCTION__);
        return ret;
    }

    if (0 != strcmp((char *)name, (char *)gPmPriv.lockName))
    {
        LOG_Error("%s wakelock:%s not create.created:%s.", __FUNCTION__, (char *)name, (char *)gPmPriv.lockName);
        return ret;
    }
    
    if (0 != Ql_SLP_WakeLock_Lock(gPmPriv.fd))
    {
        LOG_Error("%s Ql_SLP_WakeLock_Lock fail.", __FUNCTION__);
       return ret; 
    }

    ret = RET_OK;
    return ret;    
}

/**
 * @name HalModemPmWakeLockUnlocked
 * @brief Modem电源模式解锁定。
 * @param name 指向要解锁的唤醒锁名称的指针。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
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
        LOG_Error("%s have created.", __FUNCTION__);
        return ret;
    }

    if (0 != strcmp((char *)name, (char *)gPmPriv.lockName))
    {
        LOG_Error("%s wakelock:%s not create.created:%s.", __FUNCTION__, (char *)name, (char *)gPmPriv.lockName);
        return ret;
    }
    
    if (0 != Ql_SLP_WakeLock_Unlock(gPmPriv.fd))
    {
        LOG_Error("%s Ql_SLP_WakeLock_Unlock fail.", __FUNCTION__);
       return ret; 
    }
    
    ret = RET_OK;
    return ret; 
}

/**
 * @name HalModemPmWakeLockDestroy
 * @brief 销毁Modem的电源管理唤醒锁。
 * @param name 指向要销毁的唤醒锁名称的指针。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
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

    Ql_SLP_WakeLock_Destroy(gPmPriv.fd);
    gPmPriv.fd = -1;
    gPmPriv.initialed = HAL_MODEM_PM_UNINITIALED;
    memset(gPmPriv.lockName, 0x0, sizeof(gPmPriv.lockName));
    ret = RET_OK;
    return ret;    
}

void HalModemPmDeepSleepPowerDown(int mode)
{
    if(mode < 0 || mode > 3)
        return;

    Ql_Powerdown(mode);

    return;
}

