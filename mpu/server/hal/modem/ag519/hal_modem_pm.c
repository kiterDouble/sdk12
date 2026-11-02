#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_pm.h"

#define HAL_MODEM_PM_NAME_MAX (256)

#define HAL_MODEM_PM_INITIALED (0)
#define HAL_MODEM_PM_UNINITIALED (-1)

int32_t HalModemPmAutosleepEnable(int8_t enable)
{
    int32_t ret = RET_ERR;
    ret = RET_OK;
    return ret;
}

int32_t HalModemForbidInd(uint32_t ForbidIndMask)
{
    return RET_OK;
}

int32_t HalModemPmWakeLockCreate(const uint8_t *name)
{
    int32_t ret = RET_ERR;
    ret = RET_OK;
    return ret;
}

int32_t HalModemPmWakeLockLocked(const uint8_t *name)
{
    int32_t ret = RET_ERR;
    ret = RET_OK;
    return ret;    
}

int32_t HalModemPmWakeLockUnlocked(const uint8_t *name)
{
    int32_t ret = RET_ERR;
    ret = RET_OK;
    return ret; 
}

int32_t HalModemPmWakeLockDestroy(const uint8_t *name)
{
    int32_t ret = RET_ERR;
    ret = RET_OK;
    return ret;    
}