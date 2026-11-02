#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_dms.h"

#define HAL_MODEM_DMS_INITIALED (0)
#define HAL_MODEM_DMS_UNINITIALED (-1)
typedef struct
{
    int32_t initialed;
}HalModemDmsPriv_t;

static HalModemDmsPriv_t gDmsPriv =
{
    .initialed = HAL_MODEM_DMS_UNINITIALED
};

int32_t HalModemDmsInit(void)
{
    
}

int32_t HalModemDmsDeinit(void)
{
    
}

int32_t HalModemDmsSetMode(HalModemDmsMode_e mode)
{
    
}

int32_t HalModemDmsGetMode(HalModemDmsMode_e *mode)
{
    
}

int32_t HalModemDmsSetQoosEnableStatus(HalModemDmsQoosEnableStatusType_e enable)
{
    return RET_OK;
}

int32_t HalModemDmsGetQoosEnableStatus(HalModemDmsQoosEnableStatusType_e *enable)
{
    return RET_OK;
}

int32_t HalModemDmsSetQoosConfig(const HalModemDmsQoosConfig_t *qoosCfg)
{
    return RET_OK;
}

int32_t HalModemDmsGetQoosConfig(HalModemDmsQoosConfig_t *qoosCfg)
{
    return RET_OK;
}

int32_t HalModemDmsGetImei(uint8_t *imei, uint8_t len)
{
   
}