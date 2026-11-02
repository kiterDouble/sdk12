#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/reboot.h>
#include <unistd.h>

#include "err_code.h"
#include "ql_sleep_wakelock.h"
#include "ql_lpm.h"
#include "ql_can.h"
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

static HalModemPmPriv_t gPmMcuLock =
{
    .fd = -1,
    .initialed = HAL_MODEM_PM_UNINITIALED,
    .lockName = {0x0,}
};

static int32_t  g_InitCallBackFlag = 0;

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
    if (0 != ql_autosleep_enable(enable))
    {
        LOG_Error("%s call ql_autosleep_enable fail.", __FUNCTION__);
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


static int64_t get_timestamp(void)
{
    struct timespec res;
    int ret = 0;
    ret = clock_gettime(CLOCK_BOOTTIME, &res);
    if (ret != 0) {
        return -1;
    }

    return ((res.tv_sec * 1000ll) + (res.tv_nsec / 1000000));
}

int32_t HalModemForbidInd(uint32_t ForbidIndMask)
{
    static uint32_t oldForbidIndMask = 0xFF;
    LOG_Debug("%s : set mode = %u, old mode = %u",__FUNCTION__,ForbidIndMask,oldForbidIndMask);
    if(oldForbidIndMask == ForbidIndMask)
    {
        return RET_OK;
    }
    oldForbidIndMask = ForbidIndMask;
    if(0 == ForbidIndMask)
    {
        LOG_Debug("%s : set mode = %d.",__FUNCTION__,ForbidIndMask);
        ql_can_send_resume(get_timestamp(), 0);
        ql_nw_set_power_mode(0);
    }
    else
    {
        LOG_Debug("%s : set mode = %d.",__FUNCTION__,2);
        ql_can_send_suspend(get_timestamp(), 1);
        ql_nw_set_power_mode(2);
    }
    return RET_OK;
}

static void ql_lpm_report_handler(ql_wakeup_info_t *msg)
{
    if(35 != msg->irq)
    {
      HalModemPmWakeLockLocked(gPmPriv.lockName);
    }

    LOG_Debug("====== apps wakeup info ======\tirq:%d %s\tipc msg:src node[%d],port[%d]\tnetwork msg:dev(%s) %s -> %s\t",
            msg->irq,msg->irq_name, msg->ipc_msg.src_node, msg->ipc_msg.src_port, msg->network_msg.dev_name,
            msg->network_msg.src_ip, msg->network_msg.dst_ip);

}

static void ql_lpm_handler(ql_lpm_edge_t edge_state)
{
    switch (edge_state)
    {
        //wakeup
        case E_QL_LPM_RISING:
            if (HAL_MODEM_PM_INITIALED == gPmMcuLock.initialed)
            {
                ql_slp_wakelock_lock(gPmMcuLock.fd);
            }
            LOG_Debug("wakeup_out pin output high level to notify mcu that I`m awakened.\n");
            break;
        //suspend
        case E_QL_LPM_FALLING:
            if (HAL_MODEM_PM_INITIALED == gPmMcuLock.initialed)
            {
                ql_slp_wakelock_unlock(gPmMcuLock.fd);
            }
            LOG_Debug("wakeup_out pin output high level to notify mcu that I`m awakened.\n");
            LOG_Debug("do something before enter sleep mode\n");
            LOG_Debug("module will enter sleep mode\n");
            break;
        case E_QL_LPM_CUSTOMIZED_FIRST_FALLING:
            LOG_Debug("customer personalization first irq have falling interrupt,do something by customer design\n");
            break;
        case E_QL_LPM_CUSTOMIZED_FIRST_RISING:
            LOG_Debug("customer personalization first irq have rising interrupt,do something by customer design\n");
            break;
        case E_QL_LPM_CUSTOMIZED_SEC_FALLING:
            LOG_Debug("customer personalization sec irq have falling interrupt,do something by customer design\n");
            break;
        case E_QL_LPM_CUSTOMIZED_SEC_RISING:
            LOG_Debug("customer personalization sec irq have rising interrupt,do something by customer design\n");
            break;
        default:
            break;
    }
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
    if(0 == g_InitCallBackFlag)
    {
        g_InitCallBackFlag = 1;
        ret = ql_lpm_init(ql_lpm_handler);
        LOG_Error("%s ql_lpm_init, ret: %d", __FUNCTION__, ret);
        ql_lpm_wakeup_report_init(ql_lpm_report_handler);
        // LOG_Error("%s ql_lpm_wakeup_report_init, ret: %d", __FUNCTION__, ret);
    }

    
    gPmMcuLock.fd = ql_slp_wakelock_create((char *)"MCULOCK", strlen((char *)"MCULOCK"));
    if (gPmMcuLock.fd < 0)
    {
        LOG_Error("%s ql_slp_wakelock_create MCULOCK failed.", __FUNCTION__);
        return ret;          
    }
    gPmMcuLock.initialed = HAL_MODEM_PM_INITIALED;
    snprintf((char *)gPmMcuLock.lockName, sizeof(gPmMcuLock.lockName), "%s", (char *)"MCULOCK");

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
    LOG_Debug("%s[%d]",__FUNCTION__, ql_slp_wakelock_status(gPmPriv.fd));
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
    int32_t unlockRet = 0;
    
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
    
    if (0 != (unlockRet = ql_slp_wakelock_unlock(gPmPriv.fd)))
    {
        LOG_Error("%s ql_slp_wakelock_unlock fail [%d] .", __FUNCTION__,unlockRet);
        return ret;
    }
    
    LOG_Debug("%s[%d]",__FUNCTION__, ql_slp_wakelock_status(gPmPriv.fd));
    ret = RET_OK;
    return ret; 
}

/**
 * @name HalModemPmWakeLockUnlocked
 * @brief Modem电源模式解锁定。
 * @param name 指向要解锁的唤醒锁名称的指针。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemGetPmWakeLockStatus(const uint8_t *name)
{
    int32_t ret = RET_ERR;
    int32_t unlockRet = 0;
    
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
    
    ret = ql_slp_wakelock_status(gPmPriv.fd);
    LOG_Debug("%s[%d]",__FUNCTION__, ret);

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

    ql_slp_wakelock_destroy(gPmPriv.fd);
    gPmPriv.fd = -1;
    gPmPriv.initialed = HAL_MODEM_PM_UNINITIALED;
    memset(gPmPriv.lockName, 0x0, sizeof(gPmPriv.lockName));

    ql_slp_wakelock_destroy(gPmMcuLock.fd);
    gPmMcuLock.fd = -1;
    gPmMcuLock.initialed = HAL_MODEM_PM_UNINITIALED;
    memset(gPmMcuLock.lockName, 0x0, sizeof(gPmMcuLock.lockName));

    ret = RET_OK;
    return ret;
}
void HalModemPmDeepSleepPowerDown(int mode)
{
    LOG_Debug("%s : sleep into deep sleep mode , power off.",__FUNCTION__);
    //system("ql_cmd at AT+QPOWD=1");
    
    sync();
    
    reboot(RB_POWER_OFF);
    
    return;
}

