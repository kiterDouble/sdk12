#include <stdint.h>
#include <string.h>

#include "sr_log.h"
#include "net_service.h"


void NetModemSrvInfoCallback(const uint8_t *data, const uint16_t len, void *userdata)
{
    NetSrvCtx_t* ctx = (NetSrvCtx_t*)userdata;

    if (sizeof(ModemSrvNotify_t) == len && NULL != data)
    {
        memcpy(ctx->modemSrvNotify, data, len);
    }
    else
    {
        LOG_Error("%s recv modem data leng is wrong.",__FUNCTION__);
    }

    return;
}
