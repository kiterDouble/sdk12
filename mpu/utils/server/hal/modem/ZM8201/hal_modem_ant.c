#include "sr_log.h"
#include "err_code.h"
#include "hal_modem_ant.h"


#define HAL_MODEM_ANT_INITIALED (0)
#define HAL_MODEM_ANT_UNINITIALED (-1)

typedef struct
{
    int32_t initialed;
}HalModemAntPriv_t;

HalModemAntPriv_t gAntPriv =
{
    .initialed = HAL_MODEM_ANT_UNINITIALED
};


int32_t HalModemAntInit(void)
{
    
    return RET_OK;
}

int32_t HalModemAntGetAdc(int32_t *adcVal)
{
    return RET_OK;
}

int32_t HalModemSecAntGetAdc(int32_t *adcVal)
{
    
    return RET_OK;
}

int32_t HalModemAntGetCurrent(HalModemAntType_e *ant)
{
    
    return RET_OK;
}

int32_t HalModemAntChange(HalModemAntType_e ant)
{

    return RET_OK;
}

int32_t HalModemAntDeinit(void)
{
    return RET_OK;
}
