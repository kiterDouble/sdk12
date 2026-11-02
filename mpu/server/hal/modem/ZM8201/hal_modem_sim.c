#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_sim.h"

#define HAL_MODEM_SIM_INITIALED (0)
#define HAL_MODEM_SIM_UNINITIALED (-1)

typedef struct
{
    int32_t initialed;
}HalModemSimPriv_t;

static HalModemSimPriv_t gSimPriv = 
{
    .initialed = HAL_MODEM_SIM_UNINITIALED
};

int32_t HalModemSimInit(void)
{
   
}

int32_t HalModemSimDeinit(void)
{
   
}

int32_t HalModemSimGetCardStatus(HalModemSimCardStatus_e *cardStat, HalModemSimAppStatus_e *appStat)
{
   
}

int32_t HalModemSimGetImsi(uint8_t *imsi, uint8_t len)
{
    
}

int32_t HalModemSimGetIccid(uint8_t *iccid, uint8_t len)
{
   
}

int32_t HalModemSimGetMsisdn(uint8_t *msisdn, uint8_t len)
{
   
}
