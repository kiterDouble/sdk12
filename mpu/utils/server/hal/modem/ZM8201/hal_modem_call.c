#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_call.h"

#define HAL_MODEM_CALL_INITIALED (0)
#define HAL_MODEM_CALL_UNINITIALED (-1)

typedef struct
{
    CallStatusHandleCb callCb;
    int32_t initialed;
}HalModemCallPriv_t;

static HalModemCallPriv_t gCallPriv = 
{
    .callCb = NULL,
    .initialed = HAL_MODEM_CALL_UNINITIALED,
};

static HalModemCallStatus_e gConvert[] = 
{
    HAL_MODEM_CALL_STATUS_INCOMING,
    HAL_MODEM_CALL_STATUS_DIALING,
    HAL_MODEM_CALL_STATUS_ALERTING,
    HAL_MODEM_CALL_STATUS_ACTIVE,
    HAL_MODEM_CALL_STATUS_HOLDING,
    HAL_MODEM_CALL_STATUS_DISCONNECTING,
    HAL_MODEM_CALL_STATUS_WAITING
};



int32_t HalModemCallInit(CallStatusHandleCb Cb)
{
    
    
}

int32_t HalModemCallDeInit(void)
{
    
   
}

int32_t HalModemCallStart(const uint8_t *phoneNum, int32_t *callId)
{
    
    
}

int32_t HalModemCallEnd(int32_t callId)
{
    
   
}

int32_t HalModemCallAnswer(int32_t callId)
{
   
    
}
