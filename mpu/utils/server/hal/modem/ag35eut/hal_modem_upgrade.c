#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "ql_fota_api.h"
#include "ql_absys_api.h"
#include "sr_log.h"
#include "hal_modem_upgrade.h"

#define E_OTA_STATE_ERROR (E_WRITE_SEG_FAILED + 1)
static int GetSystemRetValue(const uint8_t *cmd)
{
	pid_t status;
	
	if(NULL == cmd)
	{
		return RET_ERR;
	}
	//LOG_Debug("%s exe start", cmd);
	status = system((char *)cmd);
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

    if (!filePath || (strlen((const char *)filePath) == 0)|| (strlen((const char *)filePath) > 512))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    update_info_t update_info;
    int32_t retVal;

    retVal = ql_abfota_get_update_status(&update_info);
	if (0 != retVal)
	{
        LOG_Error("%s ql_abfota_get_update_status failed.", __FUNCTION__);
		return ret;
	}

	if((SUCCEED != update_info.ota_state) && (FAILED != update_info.ota_state))
	{
		LOG_Error("%s ota state:%d, do not upgrade.", __FUNCTION__, update_info.ota_state);
		return E_OTA_STATE_ERROR;
	}
    
    char fullpath[512] = {0x0,};
    snprintf(fullpath, sizeof(fullpath), "%s/update.zip", (const char*)filePath);
    retVal = ql_abfota_start_update(fullpath);
	if (E_NO_ERROR != retVal)
	{
        LOG_Error("%s ql_abfota_start_update failed.", __FUNCTION__);
		return retVal;
	}
    
    do
    {
        ql_abfota_get_update_status(&update_info);
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
    if (!filePath || (strlen((char *)filePath) == 0)|| (strlen((char *)filePath) > 512))
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

    retVal = update_info.ota_state;

    if (WRITEDONE == retVal)
    {
        retVal = 2;
    }

    if (BACKUP == retVal)
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
    
    LOG_Debug("%s called.", __FUNCTION__);
    retVal = ql_abfota_get_update_status(&update_info);
    if (retVal != 0) 
    {
        LOG_Error("run ql_abfota_start_update failed, api return: %d", retVal);
        return ret;
    }

    if (WRITEDONE == update_info.ota_state)
    {
        ql_absys_switch();
        ret = RET_OK;
    }
    #if 0 //ql_absys_switch called need reboot
    else
    #endif
    {
        LOG_Error("update_info.ota_state:%d,just reboot.", update_info.ota_state);
        char cmd[] = "reboot";
        ret = GetSystemRetValue((const uint8_t*)cmd);
    }

    return ret;
}