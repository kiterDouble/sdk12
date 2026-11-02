#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "sr_gnss.h"
#include "gnss_ipc_msg.h"
#include "glib.h"

void GetGnssLocation(GnssLocation_t *data)
{
    CreatRpcQueue();
    if (data != NULL)
    {
        GnssLocation_t recvbuf = {0};
        uint16_t recvlen = 0;
        RpcGetMsgSync(RPCSERVER_TYPE_GNSSSERVER, GNSSSERVER_MSGTYPE_SUBCMD_LOCATIONINFO, 0,
                      NULL, 0,
                      (uint8_t *)&recvbuf, &recvlen);
        *data = recvbuf;
    }
}
