#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_sms.h"

#define HAL_MODEM_SMS_INITIALED (0)
#define HAL_MODEM_SMS_UNINITIALED (-1)

typedef struct
{
    HalModemSmsRecvCallback smsRecvCb;
    int32_t initialed;
}HalModemSmsPriv_t;

HalModemSmsPriv_t gSmsPriv =
{
    .smsRecvCb = NULL,
    .initialed = HAL_MODEM_SMS_UNINITIALED
};



int32_t HalModemSmsInit(HalModemSmsRecvCallback recvCb)
{
}

int32_t HalModemSmsDeinit(void)
{
    
}

int32_t HalModemSmsSend(const uint8_t *phoneNum, const uint8_t *mesg, uint8_t mesgLen)
{
    
}