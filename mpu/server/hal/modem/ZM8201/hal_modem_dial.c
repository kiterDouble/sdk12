#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_dial.h"

#define HAL_MODEM_DIAL_INITIALED (0)
#define HAL_MODEM_DIAL_UNINITIALED (-1)
#define HAL_MODEM_DIAL_STARTED (0)
#define HAL_MODEM_DIAL_UNSTARTED (-1)

typedef struct
{
    int32_t initialed;
    int32_t isStarted;
    ModemCallbak dialCb;
    HalModemDialParam_t dialParam;
}HalModemDialPriv_t;


static HalModemDialPriv_t gDialPriv = 
{
    .initialed = HAL_MODEM_DIAL_UNINITIALED,
    .isStarted = HAL_MODEM_DIAL_UNSTARTED,
    .dialCb = NULL,
};


int32_t HalModemDialInit(void)
{
    
}

int32_t HalModemDialDeinit(void)
{
    
}

int32_t HalModemDialSetParam(const HalModemDialParam_t *dialParam)
{
    
}

int32_t HalModemDialSetStatusCallback(ModemCallbak cb)
{
    
}

int32_t HalModemDialStart(void)
{
    
}

int32_t HalModemDialStop(void)
{

}