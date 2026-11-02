#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "sr_param.h"
#include "glib.h"

int32_t GetParam(ParamInfo_t *param)
{
    if (param == NULL)
    {
        LOG_Debug("GetParam failed: param is NULL");
        return RET_ERR;
    }
    if (param->id > E_ParamId_UserEnd)
    {
        LOG_Debug("GetParam failed:paramId out of bounds");
        return RET_ERR;
    }
    CreatRpcQueue();
    uint16_t recvlen;
    ParamInfo_t resp;
    RpcGetMsgSync(RPCSERVER_TYPE_PARAMSERVER, PARAMSERVER_MSGTYPE_SUBCMD_GETPARAM, 0,
                  (uint8_t *)param, sizeof(ParamInfo_t),
                  (uint8_t *)&resp, &recvlen);
    if (sizeof(ParamInfo_t) == recvlen && resp.id != E_ParamId_InvalidId)
    {
        *param = resp;
        return RET_OK;
    }
    else
    {
        return RET_ERR;
    }
}

int32_t UpdateParam(uint32_t id, uint8_t *value, uint16_t dataLen)
{
    if (id > E_ParamId_UserEnd)
    {
        LOG_Debug("UpdateParam failed:paramId out of bounds");
        return RET_ERR;
    }
    CreatRpcQueue();
    ParamInfo_t param;
    memset(&param,0x00,sizeof(ParamInfo_t));
    param.id = id;
    int8_t ret;
    uint16_t recvlen;
    memcpy(param.value, value, dataLen);

    RpcGetMsgSync(RPCSERVER_TYPE_PARAMSERVER, PARAMSERVER_MSGTYPE_SUBCMD_UPDATEPARAM, 0,
                  (uint8_t *)&param, sizeof(ParamInfo_t),
                  (uint8_t *)&ret, &recvlen);
    return ret;
}

int32_t AddUserParam(ParamInfo_t param)
{
    if (param.id < E_ParamId_UserStart || param.id > E_ParamId_UserEnd)
    {
        LOG_Debug("AddUserParam failed:paramId out of bounds");
        return RET_ERR;
    }
    CreatRpcQueue();
    int8_t ret;
    uint16_t recvlen;
    RpcGetMsgSync(RPCSERVER_TYPE_PARAMSERVER, PARAMSERVER_MSGTYPE_SUBCMD_ADDUSERPARAM, 0,
                  (uint8_t *)&param, sizeof(ParamInfo_t),
                  (uint8_t *)&ret, &recvlen);
    return ret;
}

int32_t DeleteUserParam(uint32_t id)
{
    if (id < E_ParamId_UserStart || id > E_ParamId_UserEnd)
    {
        LOG_Debug("DeleteUserParam failed:paramId out of bounds");
        return RET_ERR;
    }
    CreatRpcQueue();
    ParamInfo_t param;
    memset(&param,0x00,sizeof(ParamInfo_t));
    param.id = id;
    int8_t ret;
    uint16_t recvlen;

    RpcGetMsgSync(RPCSERVER_TYPE_PARAMSERVER, PARAMSERVER_MSGTYPE_SUBCMD_DELETEPARAM, 0,
                  (uint8_t *)&param, sizeof(ParamInfo_t),
                  (uint8_t *)&ret, &recvlen);
    return ret;
}
