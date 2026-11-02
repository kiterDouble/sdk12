#include <stdlib.h>
#include <string.h>
#include "err_code.h"

#include "ecall_server_api.h"
#include "sr_ipc.h"
#include "sr_log.h"

static uint8_t g_IpcInitFlag = 0;

int32_t GetEcallStatusInfo(EnuEcallStatus *ecallStatus,int32_t timeOut)
{
    EnuEcallStatus recvdata = 0;
    uint16_t recvlen = 0;

    memset(&recvdata,0, sizeof(recvdata));
    if(!g_IpcInitFlag)
    {    
        CreatRpcQueue();
        g_IpcInitFlag = 1;
    }
    RpcGetMsgSync(RPCSERVER_TYPE_ECALLSERVER, 0, timeOut, NULL, 0, (uint8_t *)&recvdata, &recvlen);
    if(recvlen == sizeof(recvdata))
    {
        memcpy(ecallStatus,&recvdata,sizeof(recvdata));
        LOG_Info("%s : RpcGetMsgSync success!",__FUNCTION__);
        return RET_OK;
    }
    LOG_Error("%s : RpcGetMsgSync failed!",__FUNCTION__);
    return RET_ERR;
}

const char *GetEcallStatusString(EnuEcallStatus status)
{
    const char *statusString[] =
    {
        "init",
        "incoming",
        "calling",
        "active",
        "end",
        "abnormal_end",
        "wait_psap",
        "send_msd",
        "push_msd",
        "update_msd",
        "msd_send_success",
        "msd_send_fault",
    };

    if(status >= ECALL_API_STATUS_INIT && status < MAX_ECALL_API_STATUS)
        return statusString[status];

    return "unknown";
}

