#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_upgrade.h"
#include "Sunsea_OTAUpgrade.h"

static int GetSystemRetValue(const uint8_t *cmd)
{
	pid_t status;
	
	if(NULL == cmd)
	{
		return RET_ERR;
	}
	status = system((char *)cmd);
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
    UPDATE_STATE state;
    int32_t ret = RET_ERR;

    if (!filePath || (strlen((const char *)filePath) == 0) || (strlen((const char *)filePath) > 512))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    
    if (Sunsea_Query_Update_State(&state) < 0)
    {
        LOG_Error("%s ql_abfota_get_update_status failed.", __FUNCTION__);
		return ret;
    }

    if ((UPDATE_STATE_IDLE != state) && (UPDATE_STATE_FAILED != state))
    {
        LOG_Error("%s ota state:%d, do not upgrade.", __FUNCTION__, state);
        return ret;
    }

    char fullpath[512] = {0x0,};
    snprintf(fullpath, sizeof(fullpath), "%s/update.zip", (const char*)filePath);
    if (Sunsea_Upgrade_Through_SD((char *)fullpath) < 0)
    {
        LOG_Error("%s Sunsea_Upgrade_Through_SD failed.", __FUNCTION__);
		return ret;
    }

    do
    {
        state = UPDATE_STATE_UPDATING;
        Sunsea_Query_Update_State(&state);
    } while (UPDATE_STATE_UPDATING == state);
    
    if (UPDATE_STATE_UPDATED == state)
    {
        LOG_Error("%s update state success.", __FUNCTION__);
		ret = RET_OK;
    }
    else
    {
        LOG_Error("%s update state failed.", __FUNCTION__);
    }

    return ret;
}

int32_t HalModemGetUpgradeStatus(const uint8_t *filePath)
{
    int32_t ret = RET_ERR;
    if (!filePath || (strlen((char *)filePath) == 0)|| (strlen((char *)filePath) > 512))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    UPDATE_STATE state;
    
    if (Sunsea_Query_Update_State(&state) < 0) 
    {
        LOG_Error("%s Sunsea_Query_Update_State failed", __FUNCTION__);
        return ret;
    }

    int32_t retVal = 0;
    if (UPDATE_STATE_UPDATING == state)
    {
        retVal = 1;
    }

    if (UPDATE_STATE_UPDATED == state)
    {
        retVal = 4;
    }

    if (UPDATE_STATE_FAILED == state)
    {
        retVal = 3;
    }

    return retVal;
}

int32_t HalModemSwitch(void)
{
    int32_t ret;
    char cmd[] = "reboot";

    ret = GetSystemRetValue((const uint8_t*)cmd);
    return ret;
}