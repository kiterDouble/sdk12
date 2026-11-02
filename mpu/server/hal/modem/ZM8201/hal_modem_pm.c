#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_pm.h"

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
   
}

int32_t HalModemForbidInd(uint32_t ForbidIndMask)
{
   
    return RET_OK;
}

int32_t HalModemPmWakeLockCreate(const uint8_t *name)
{
    
}

int32_t HalModemPmWakeLockLocked(const uint8_t *name)
{
    
}

int32_t HalModemPmWakeLockUnlocked(const uint8_t *name)
{
    
}

int32_t HalModemPmWakeLockDestroy(const uint8_t *name)
{
    
}