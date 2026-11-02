#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_nw.h"

#define HAL_MODEM_NW_INITIALED (0)
#define HAL_MODEM_NW_UNINITIALED (-1)

typedef struct
{
    int32_t initialed;
}HalModemNwPriv_t;

static HalModemNwPriv_t gNwPriv = 
{
    .initialed = HAL_MODEM_NW_UNINITIALED
};
int32_t HalModemNwInit(void)
{
    
}

int32_t HalModemNwDeinit(void)
{
   
}

int32_t HalModemNwGetRegStatus(HalModemNwRegStatus_e *status)
{
    
}

static int32_t HalModemNwRssi2Csq(int32_t rssi)
{
    
}

int32_t HalModemNwGetCsq(uint8_t *csq)
{
    
}

int32_t HalModemNwGetSignalInfo(HalModemNwExtSigInfo_t *sigInfo)
{
    
}

int32_t HalModemNwGetOperatorInfo(uint8_t *provider)
{
   
}