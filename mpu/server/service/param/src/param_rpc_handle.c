#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "err_code.h"
#include "sr_log.h"
#include "param_database.h"
#include "param_sync.h"

void paramServerCallBack(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
    // LOG_Debug("RPC Server recv:subcmd:%d, data:", subcmd);
    // LOG_Debug_Hex(reqData,reqLen);
    int8_t ret = RET_OK;
    ParamInfo_t msgreq;
    memset(&msgreq, 0x00, sizeof(ParamInfo_t));
    memcpy(&msgreq, reqData, reqLen);
    // LOG_Debug("RPC Server recv:id:%d", msgreq.id);
    // LOG_Debug("RPC Server recv:name:%s", msgreq.name);
    // LOG_Debug("RPC Server recv:type:%d", msgreq.type);
    // LOG_Debug("RPC Server recv:id:%d", msgreq.id);
    // LOG_Debug("RPC Server recv:min:%d", msgreq.min);
    // LOG_Debug("RPC Server recv:max:%d", msgreq.max);
    // LOG_Debug("RPC Server recv:paramSources:%d", msgreq.paramSources);
    // LOG_Debug("RPC Server recv:rpcWriteEnable:%d", msgreq.rpcWriteEnable);
    // LOG_Debug("RPC Server recv:value:");
    // LOG_Debug_Hex(msgreq.value,sizeof(msgreq.value));
    if(PARAMSERVER_MSGTYPE_SUBCMD_GETPARAM == subcmd)
    {
        GetParams(&msgreq);
        *respLen = sizeof(ParamInfo_t);
        memcpy(respData, (uint8_t *)&msgreq, sizeof(ParamInfo_t));
        return;
    }
    else if(PARAMSERVER_MSGTYPE_SUBCMD_UPDATEPARAM == subcmd)
    {
        uint8_t type;
        ret = GetParamsValueType(msgreq.id, &type);
        if (ret == RET_OK)
        {
            LOG_Debug("Set param idx:%d,value:%s", msgreq.id, msgreq.value);
            if (type == PARAM_TYPE_INT)
            {
                int32_t value = atoi((char *)msgreq.value);
                ret = SetParamsValue((DeviceParamEnum)msgreq.id, (void*)&value, true);
            }
            else
            {
                ret = SetParamsValue((DeviceParamEnum)msgreq.id, (void*)msgreq.value, true);
            }
            if (ret != RET_OK)
            {
                LOG_Error("[param] set param fail!");
            }
        }
    }
    else if(PARAMSERVER_MSGTYPE_SUBCMD_ADDUSERPARAM == subcmd)
    {
        ret = SetUserParams(msgreq);
    }
    else if(PARAMSERVER_MSGTYPE_SUBCMD_DELETEPARAM == subcmd)
    {
        uint32_t id = msgreq.id;
        ret = DeleteUserParamWithId(id);
    }
    else
    {
        LOG_Error("[param] error RPC callback!");
    }
    *respLen = sizeof(uint8_t);
    memcpy(respData, (uint8_t *)&ret, sizeof(uint8_t));
}
