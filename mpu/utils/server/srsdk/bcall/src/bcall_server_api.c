#include <stdlib.h>
#include <string.h>
#include "err_code.h"

#include "bcall_server_api.h"
#include "sr_ipc.h"
#include "sr_log.h"

static uint8_t g_IpcInitFlag = 0;

int32_t GetBcallStatusInfo(EnuBcallStatus *bcallStatus,int32_t timeOut)
{
    EnuBcallStatus recvdata = 0;
    uint16_t recvlen = 0;

    memset(&recvdata,0, sizeof(recvdata));
    if(!g_IpcInitFlag)
    {    
        CreatRpcQueue();
        g_IpcInitFlag = 1;
    }
    RpcGetMsgSync(RPCSERVER_TYPE_BCALLSERVER, 0, timeOut, NULL, 0, (uint8_t *)&recvdata, &recvlen);
    if(recvlen == sizeof(recvdata))
    {
        memcpy(bcallStatus,&recvdata,sizeof(recvdata));
        LOG_Info("%s : RpcGetMsgSync success!",__FUNCTION__);
        return RET_OK;
    }
    LOG_Error("%s : RpcGetMsgSync failed!",__FUNCTION__);
    return RET_ERR;
}

