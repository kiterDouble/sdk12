#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "ql_fota_api.h"
#include "ql_absys_api.h"
#include "sr_log.h"
#include "hal_modem_upgrade.h"

#define E_OTA_STATE_ERROR ((int32_t)E_WRITE_SEG_FAILED + 1)
static int GetSystemRetValue(const uint8_t *cmd)
{
	pid_t status;
	
	if(NULL == cmd)
	{
		return RET_ERR;
	}
	//LOG_Debug("%s exe start", cmd);
	status = system((const char *)cmd);
	//LOG_Debug("%s exe end:%d", cmd, status);
	if(-1 == status)
	{
        
		return RET_ERR;
	}
	
	if(!WIFEXITED(status))
	{
		return RET_ERR;
	}
	
	return WEXITSTATUS(status);
}

int32_t HalModemUpgradeImpl(const uint8_t *filePath)
{
    int32_t ret = RET_ERR;
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

    if (!filePath || (strlen((const char *)filePath) == 0U)|| (strlen((const char *)filePath) > 512U))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    update_info_t update_info;
    int32_t retVal;

    retVal = ql_abfota_get_update_status(&update_info);
	if (0 != retVal)
	{
        LOG_Error("%s ql_abfota_get_update_status failed,return: %d", __FUNCTION__, retVal);
		return ret;
	}

	if((SUCCEED != update_info.ota_state) && (FAILED != update_info.ota_state))
	{
		LOG_Error("%s ota state:%d, do not upgrade.", __FUNCTION__, update_info.ota_state);
		return (int32_t)E_OTA_STATE_ERROR;
	}
    
    char fullpath[512] = {0x0,};
    (void)snprintf(fullpath, sizeof(fullpath), "%s/update.zip", (const char*)filePath);
    retVal = ql_abfota_start_update(fullpath);
	if ((int32_t)E_NO_ERROR != retVal)
	{
        LOG_Error("%s ql_abfota_start_update failed,return: %d", __FUNCTION__, retVal);
		return retVal;
	}
    
    do
    {
        (void)ql_abfota_get_update_status(&update_info);
    }
    while(UPDATE == update_info.ota_state);

    LOG_Debug("%s status:%d.", __FUNCTION__, update_info.ota_state);
    if (WRITEDONE == update_info.ota_state)
    {
        ret = RET_OK;
    }
    
    return ret;
}

int32_t HalModemGetUpgradeStatus(const uint8_t *filePath)
{
    int32_t ret = RET_ERR;
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);

    if (!filePath || (strlen((const char *)filePath) == 0U)|| (strlen((const char *)filePath) > 512U))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    update_info_t update_info;
    int32_t retVal;
    
    retVal = ql_abfota_get_update_status(&update_info);
    if (retVal != 0) 
    {
        LOG_Error("run ql_abfota_start_update failed, api return: %d", retVal);
        return ret;
    }

    retVal = (int32_t)update_info.ota_state;

    if (WRITEDONE == (uint32_t)retVal)
    {
        retVal = 2;
    }

    if (BACKUP == (uint32_t)retVal)
    {
        retVal = 4;
    }
    return retVal;
}

int32_t HalModemSwitch(void)
{    
    update_info_t update_info;
    int32_t ret = RET_ERR;
    int32_t retVal;
    LOG_Debug("func:%s line:%d.", __FUNCTION__, __LINE__);
    
    LOG_Debug("%s called.", __FUNCTION__);
    retVal = ql_abfota_get_update_status(&update_info);
    if (retVal != 0) 
    {
        LOG_Error("run ql_abfota_start_update failed, api return: %d", retVal);
        return ret;
    }

    if (WRITEDONE == update_info.ota_state)
    {
        (void)ql_absys_switch();
        ret = RET_OK;
    }
    //ql_absys_switch called need reboot
#if 0 
    {
        LOG_Error("update_info.ota_state:%d,just reboot.", update_info.ota_state);
        char cmd[] = "reboot -f";
        ret = GetSystemRetValue((const uint8_t*)cmd);
    }
#else

#endif

    return ret;
}

int32_t HalModemRestore(void)
{    
    update_info_t update_info;
    int32_t retryTimes = 0;
    
    LOG_Debug("%s called.", __FUNCTION__);
    do {
        // 查询升级状态
        int32_t retVal = ql_abfota_get_update_status(&update_info);
        if (retVal != 0) {
            LOG_Error("run ql_abfota_start_update failed, api return: %d", retVal);
            return RET_ERR;
        }
        switch (update_info.ota_state) {
            case SUCCEED:
                LOG_Info("Restore succeeded");
                return RET_OK;
            case BACKUP:    // 升级操作前如果当前状态为BACKUP或WRITEDONE需要做同步操作
            case FAILED:    // 升级MPU失败条件下需要做同步操作
            case WRITEDONE: // 升级MPU成功但升级MCU失败条件下需要做同步操作
            case NEEDSYNC:  // 升级MPU成功但需要同步状态
                retVal = ql_absys_sync();
                if (retVal != 0) {
                    LOG_Error("run ql_absys_sync failed, api return: %d", retVal);
                    continue;
                }
                break;
            default:
                LOG_Error("Unexpected state: %d", update_info.ota_state);
                return RET_ERR;
        }
        g_usleep(500 * 1000);
    } while (++retryTimes < 3);
    LOG_Error("restore failed, stop retry.");
    return RET_ERR;
}