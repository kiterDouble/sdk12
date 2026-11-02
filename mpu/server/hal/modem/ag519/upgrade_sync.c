#include <stdint.h>
#include "ql_fota_api.h"
#include "ql_absys.h"
#include "err_code.h"
#include "sr_log.h"


static int32_t CheckSync(void)
{
    sysstatus_t status;
    update_info_t update_info;
    int32_t retVal;

    retVal = ql_absys_getstatus(&status);
    if (retVal < 0) 
    {
        LOG_Error("failed to get absys status!!!");
        return RET_ERR;
    }
    
    retVal = ql_abfota_get_update_status(&update_info);
    if (retVal < 0)
    {
        LOG_Error("%s ql_abfota_get_update_status fail.", __FUNCTION__);
        return RET_ERR;
    }

	if (status.is_damaged)
	{
        LOG_Debug("%s damaged.", __FUNCTION__);
		retVal = ql_absys_sync();
        if (retVal < 0) 
        {
            LOG_Error("%s ql_absys_sync failed.", __FUNCTION__);
            return RET_ERR;
        }
        return RET_OK;
	}

    if ((NEEDSYNC == update_info.ota_state) || (UPDATE == update_info.ota_state))
    {
        LOG_Debug("%s need sync.", __FUNCTION__);
        retVal = ql_absys_sync();
        if (retVal < 0) 
        {
            LOG_Error("%s ql_absys_sync failed.", __FUNCTION__);
            return RET_ERR;
        }
    }
    
    return RET_OK;
}

int32_t main(int32_t argc, char *argv[])
{
    LOG_INIT("demo");
    LOG_Debug("AG35CET CHECK SYNC.");
    return CheckSync();
}