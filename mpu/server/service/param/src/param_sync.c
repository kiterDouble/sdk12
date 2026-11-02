#include <glib.h>
#include <glib/gprintf.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "sr_log.h"
#include "err_code.h"
#include "param_database.h"
#include "param_sync.h"

uint8_t flagParamSyncAck[E_ParamId_UserEnd + 1] = {0};
uint8_t flagGetMcuParas[E_ParamId_UserEnd + 1] = {0};
extern const char *element[][8];

void* UpdateParamFromMcu(void* pArg)
{
    //统计来自MCU的参数数量
    uint16_t mcuParaNum = 0;
    uint8_t reqData[E_ParamId_UserEnd + 2] = {0};
    for(int index = 0; index < E_ParamId_UserEnd + 1; index++)
    {
        if(PARAM_SOURCES_MCU == GetParamsSources(index))
        {
            mcuParaNum ++;
        }
    }
    while (1)
    {
        //当前已向MCU同步的参数数量
        uint16_t curSyncNum = 0;
        uint16_t reqParaLen = 1;
        memset(reqData, 0x00, sizeof(reqData));
        for (int index = 0; index < E_ParamId_UserEnd + 1; index++)
        {
            if (PARAM_SOURCES_MCU == GetParamsSources(index))
            {
                //如果当前参数已同步
                if(flagGetMcuParas[index] == 1)
                {
                    curSyncNum++;
                }
                else
                {
                    //本次传输的参数个数
                    reqData[0]++;
                    reqData[reqParaLen++] = index;
                    LOG_Debug("req param to mcu:param index:%d",index);
                }
            }
        }
        if(curSyncNum == mcuParaNum)
        {
            break;
        }
        else
        {
            // 向MCU获取参数
            McuMesg_t sendToMcu;
            sendToMcu.type = SYS_QUE_TYPE_MCU_SYNC_TBOX_READ_REQ;
            // 参数的索引值数组，最大32。如果有超过32个MCU参数的情况，那么每次只请求当前还未获取到的前32个参数
            sendToMcu.payloadLen = (reqParaLen > 33 ? 33 : reqParaLen);
            sendToMcu.payload = reqData;
            McuSrvSendMesg(&sendToMcu);
            usleep(500000);
        }
    }
    return NULL;
}

void *UpdateParamFromModem(void *pArg)
{
    char iccid[25];
    char imei[25];
    char imsi[25];

    GetParamsValue(E_ParamId_IMEI, imei);
    GetParamsValue(E_ParamId_IMSI, imsi);
    GetParamsValue(E_ParamId_ICCID, iccid);

    LOG_Debug("get imei from db :%s", imei);
    LOG_Debug("get imsi from db :%s", imsi);
    LOG_Debug("get iccid from db :%s", iccid);

    // if (strcmp(iccid, element[E_ParamId_ICCID][2]) != 0 &&
    //     strcmp(imei, element[E_ParamId_IMEI][2]) != 0 &&
    //     strcmp(imsi, element[E_ParamId_IMSI][2]) != 0)
    // {
    //     LOG_Debug("ICCID, IMSI, and IMEI already exist in the DB file, and not rpc to modem");
    //     return NULL;
    // }

    // LOG_Debug("start get imei from modem");
    // if(strcmp(imei, element[E_ParamId_IMEI][2]) == 0)
    // {
        memset(imei, 0, 25);
        while (RET_OK != ModemSrvGetImei((uint8_t *)imei))
        {
            LOG_Error("param get imei from At error");
            memset(imei, 0, 25);
            sleep(1);
        }
        LOG_Debug("get imei from at :%s", imei);
        SetParamsValue(E_ParamId_IMEI, imei, true);
    // }

    // LOG_Debug("start get iccid from modem");
    // if(strcmp(iccid, element[E_ParamId_ICCID][2]) == 0)
    // {
        memset(iccid, 0, 25);
        while (RET_OK != ModemSrvGetIccid((uint8_t *)iccid))
        {
            LOG_Error("param get iccid from At error");
            memset(iccid, 0, 25);
            sleep(3); /* sleep 3s, Prevent affecting the normal sleep mode of the modem*/
        }
        LOG_Debug("get iccid from at :%s", iccid);
        SetParamsValue(E_ParamId_ICCID, iccid, true);
    // }

    // LOG_Debug("start get imsi from modem");
    // if(strcmp(imsi, element[E_ParamId_IMSI][2]) == 0)
    // {
        memset(imsi, 0, 25);
        while (RET_OK != ModemSrvGetImsi((uint8_t *)imsi))
        {
            LOG_Error("param get imsi from At error");
            memset(imsi, 0, 25);
            sleep(3);/* sleep 3s, Prevent affecting the normal sleep mode of the modem*/
        }
        LOG_Debug("get imsi from at :%s", imsi);
        SetParamsValue(E_ParamId_IMSI, imsi, true);
    // }

    // SetParamsValue(E_ParamId_SW_Version, "185001001001_001", false);

    return NULL;
}

void HandleMcuReadParamReq(const uint8_t *data, const uint16_t len, void *userdata)
{
    LOG_DebugHex(data,len,"HandleMcuReadParamReq");

    int32_t ret;

    char paramValue[100];
    int32_t paramValueInt = 0;
    uint8_t type = 0;

    uint32_t paramNum = data[0];

    uint8_t paramDataSendToMCU[1024];
    int32_t paramDataLenSum = 0;
    paramDataSendToMCU[paramDataLenSum++] = paramNum;
    LOG_Debug("[param sync]  reqest %d params ", paramNum);
    for (int32_t i = 0; i < paramNum; i++)
    {
        /*get param value*/
        memset(paramValue, 0, sizeof(paramValue));
        DeviceParamEnum paramIndex = (DeviceParamEnum)(data[i + 1]);
        if (paramIndex > E_ParamId_UserEnd)
        {
            return;
        }
        LOG_Debug("[param sync] index %d ", paramIndex);

        if (GetParamsValueType(paramIndex, &type) == RET_ERR)
        {
            LOG_Debug("[param sync] the param [%d] can not be set", paramIndex);
            return;
        }

        if (type == PARAM_TYPE_INT)
        {
            ret = GetParamsValue(paramIndex, &paramValueInt);
            if (ret != RET_OK)
            {
                LOG_Error("[param sync ] get param errorn ");
            }
            else
            {
                LOG_Debug("[param sync] get param value %d ", paramValueInt);
            }
            /*add param index(1 byte) into buffer*/
            paramDataSendToMCU[paramDataLenSum++] = data[i + 1];

            /*add param len(1 byte) into buffer*/
            paramDataSendToMCU[paramDataLenSum++] = 4;

            /*add param value into buffer*/
            paramDataSendToMCU[paramDataLenSum++] = paramValueInt >> 24;
            paramDataSendToMCU[paramDataLenSum++] = paramValueInt >> 16;
            paramDataSendToMCU[paramDataLenSum++] = paramValueInt >> 8;
            paramDataSendToMCU[paramDataLenSum++] = paramValueInt;
        }
        else
        {
            ret = GetParamsValue(paramIndex, paramValue);
            if (ret != RET_OK)
            {
                LOG_Error("[param sync] get param errorn ");
            }
            else
            {
                LOG_Debug("[param sync] get param value %s ", paramValue);
            }
            /*add param index(1 byte) into buffer*/
            paramDataSendToMCU[paramDataLenSum++] = data[i + 1];
            /*add param len(1 byte) into buffer*/
            uint8_t paramLen = strlen(paramValue);
            paramDataSendToMCU[paramDataLenSum++] = paramLen;
            /*add param value into buffer*/
            memcpy(&paramDataSendToMCU[paramDataLenSum], paramValue, paramLen);
            paramDataLenSum += paramLen;
            LOG_Debug("[param sync] paramlen %d  paramsumlen %d ", paramLen, paramDataLenSum);

            // check iccid all is 0
            if (paramIndex == E_ParamId_ICCID)
            {
                if (strcmp(element[E_ParamId_ICCID][2], paramValue) == 0)
                {
                    LOG_Error("param sync iccid all is 0,not response to MCU");
                    return;
                }
            }

            // check imsi all is 0
            if (paramIndex == E_ParamId_IMSI)
            {
                if (strcmp(element[E_ParamId_IMSI][2], paramValue) == 0)
                {
                    LOG_Error("param sync imsi all is 0,not response to MCU");
                    return;
                }
            }

            // check imei is 0
            if (paramIndex == E_ParamId_IMEI)
            {
                if (strcmp(element[E_ParamId_IMEI][2], paramValue) == 0)
                {
                    LOG_Error("param sync imei all is 0,not response to MCU");
                    return;
                }
            }
        }
    }

    /*res : send params to MCU a01m03s02*/
    McuMesg_t sendToMcu;
    sendToMcu.type = SYS_QUE_TYPE_MCU_SYNC_TBOX_READ_RES;
    sendToMcu.payloadLen = paramDataLenSum;
    sendToMcu.payload = paramDataSendToMCU;
    ret = McuSrvSendMesg(&sendToMcu);
    // ret = SendMcuMsg(MSG_QUEUE_SYNC_PARAM, paramDataLenSum, PARAM_SYNC_RES, paramDataSendToMCU);

    if (ret != RET_OK)
    {
        LOG_Debug("[param sync] sent data to MCU erron ");
        return;
    }
    else
    {
        LOG_Debug("[param sync]send to mcu type %d len %x ", SYS_QUE_TYPE_MCU_SYNC_TBOX_READ_RES, paramDataLenSum);
    }
}

void HandleMcuSyncParam(const uint8_t *data, const uint16_t len, void *userdata)
{
    LOG_DebugHex(data,len,"HandleMcuSyncParam");

    int32_t ret;

    DeviceParamEnum paramIndex = E_ParamId_ICCID;
    
    char paramValue[100] = {0};
    int32_t paramValueInt = 0;
    uint8_t type = 0;

    uint32_t syncAckParamNum = 0;
    uint8_t paramDataSendToMCU[1024];

    int32_t paramDataLenSum = 0;
    paramIndex = (DeviceParamEnum)(data[paramDataLenSum++]);
    /*is check param valid*/
    if (paramIndex > E_ParamId_UserEnd || paramIndex == E_ParamId_ICCID ||
        paramIndex == E_ParamId_IMSI || paramIndex == E_ParamId_IMEI)
    {
        LOG_Debug("[param sync] the param [%d] can not be set ", paramIndex);
        return;
    }

    uint32_t paramLen = data[paramDataLenSum++];

    memset(paramValue, 0, sizeof(paramValue));
    memcpy(paramValue, &data[paramDataLenSum], paramLen);

    if (GetParamsValueType(paramIndex, &type) == RET_ERR)
    {
        LOG_Debug("[param sync] the param [%d] can not be set ", paramIndex);
        return;
    }
    if (type == PARAM_TYPE_INT)
    {

        paramValueInt = (paramValue[0] << 24) +
                        (paramValue[1] << 16) +
                        (paramValue[2] << 8) + paramValue[3];

        ret = SetParamsValue(paramIndex, &paramValueInt, false);
    }
    else if (type == PARAM_TYPE_STR)
    {
        ret = SetParamsValue(paramIndex, paramValue, false);
    }
    else
    {
        LOG_Debug("[param sync] the param [%d] can not be set ", paramIndex);
        return;
    }

    if (ret != RET_OK)
    {
        LOG_Debug("[param sync] erron : %x ", ret);
    }

    paramDataSendToMCU[syncAckParamNum++] = (ret == RET_OK ? 0 : 1);

    memset(paramValue, 0, sizeof(paramValue));
    if (type == PARAM_TYPE_INT)
    {
        ret = GetParamsValue(paramIndex, &paramValueInt);
        if (ret != RET_OK)
        {
            LOG_Error("[param sync ] get param errorn ");
        }
        else
        {
            LOG_Debug("[param sync] get param value %d ", paramValueInt);
        }
        /*add param index(1 byte) into buffer*/
        paramDataSendToMCU[syncAckParamNum++] = paramIndex;

        /*add param len(1 byte) into buffer*/
        paramDataSendToMCU[syncAckParamNum++] = sizeof(uint32_t);

        /*add param value into buffer*/
        paramDataSendToMCU[syncAckParamNum++] = paramValueInt >> 24;
        paramDataSendToMCU[syncAckParamNum++] = paramValueInt >> 16;
        paramDataSendToMCU[syncAckParamNum++] = paramValueInt >> 8;
        paramDataSendToMCU[syncAckParamNum++] = paramValueInt;
    }
    else
    {
        ret = GetParamsValue(paramIndex, paramValue);
        if (ret != RET_OK)
        {
            LOG_Error("[param sync ] get param errorn ");
        }
        else
        {
            LOG_Debug("[param sync] get param value %s ", paramValue);
        }
        /*add param index(1 byte) into buffer*/
        paramDataSendToMCU[syncAckParamNum++] = paramIndex;

        /*add param len(1 byte) into buffer*/
        paramLen = strlen(paramValue);
        paramDataSendToMCU[syncAckParamNum++] = paramLen;

        /*add param value into buffer*/
        memcpy(&paramDataSendToMCU[syncAckParamNum], paramValue, paramLen);
        syncAckParamNum += paramLen;
        LOG_Debug("[param sync] paramlen %d  paramsumlen %d ", paramLen, syncAckParamNum);
    }

    McuMesg_t sendToMcu;
    sendToMcu.type = SYS_QUE_TYPE_MCU_SYNC_TBOX_MCU_WRITE_RES;
    sendToMcu.payloadLen = syncAckParamNum;
    sendToMcu.payload = paramDataSendToMCU;
    ret = McuSrvSendMesg(&sendToMcu);

    if (ret != RET_OK)
    {
        LOG_Debug("[param sync] sent data to MCU erron ");
    }
}

void HandleMcuRespParam(const uint8_t *data, const uint16_t len, void *userdata)
{
    LOG_DebugHex(data,len,"HandleMcuRespParam");

    uint32_t datalen = 0;
    uint8_t number_ = data[datalen++];
    for (int i = 0; i < number_; i++)
    {
        if(datalen >= len)
        {
            LOG_Error("[param] datalen is longer than the received length!");
            break;
        }
        uint8_t index_ = data[datalen++];
        uint8_t len_ = data[datalen++];

        uint8_t type;
        GetParamsValueType(index_, &type);
        int32_t ret;
        if ((type == PARAM_TYPE_INT) && (len_ == sizeof(uint32_t)))
        {
            int32_t value = 0;
            value |= ((int32_t)(data[datalen++])) << 24;
            value |= ((int32_t)(data[datalen++])) << 16;
            value |= ((int32_t)(data[datalen++])) << 8;
            value |= ((int32_t)(data[datalen++]));
            ret = SetParamsValue((DeviceParamEnum)index_, (void *)&value, false);
        }
        else
        {
            uint8_t value[256] = {0};
            memcpy(value, &(data[datalen]), len_);
            datalen += len_;
            ret = SetParamsValue((DeviceParamEnum)index_, (void *)value, false);
        }
        if (ret != RET_OK)
        {
            LOG_Error("[param] set param fail!");
        }
        else
        {
            flagGetMcuParas[index_] = 1;
            LOG_Debug("[param] sync param from mcu success, param index: %d!", index_);
        }
    }
}

void HandleMcuRespSyncParam(const uint8_t *data, const uint16_t len, void *userdata)
{
    LOG_DebugHex(data,len,"HandleMcuRespSyncParam");

    uint16_t index = 0;
    uint8_t status = data[index++];
    uint8_t paraIndex = data[index++];
    if(0 == status)
    {
        flagParamSyncAck[paraIndex] = 0; /*应答清零*/
        LOG_Debug("[param sync]##### flagParamSyncAck[%d] = [0]", paraIndex);
    }
    else
    {
        LOG_Debug("[param sync]##### Param sync to MCU failed. index:[%d]", paraIndex);
    }
}

void SyncParamToMcu(DeviceParamEnum paramName, void* paramValue)
{
    uint8_t type;
    GetParamsValueType(paramName, &type);
    McuMesg_t sendToMcu;
    sendToMcu.type = SYS_QUE_TYPE_MCU_SYNC_TBOX_MCU_WRITE_REQ;

    uint32_t syncAckParamNum = 0;
    uint8_t paramDataSendToMCU[300];

    if (type == PARAM_TYPE_INT)
    {
        int32_t paramValueInt = *(int32_t *)paramValue;
        paramDataSendToMCU[syncAckParamNum++] = paramName;
        paramDataSendToMCU[syncAckParamNum++] = sizeof(uint32_t);
        paramDataSendToMCU[syncAckParamNum++] = paramValueInt >> 24;
        paramDataSendToMCU[syncAckParamNum++] = paramValueInt >> 16;
        paramDataSendToMCU[syncAckParamNum++] = paramValueInt >> 8;
        paramDataSendToMCU[syncAckParamNum++] = paramValueInt;
    }
    else
    {
        paramDataSendToMCU[syncAckParamNum++] = paramName;
        uint16_t paramLen = strlen(paramValue);
        paramDataSendToMCU[syncAckParamNum++] = paramLen;
        memcpy(&paramDataSendToMCU[syncAckParamNum], paramValue, paramLen);
        syncAckParamNum += paramLen;
    }
    sendToMcu.payloadLen = syncAckParamNum;
    sendToMcu.payload = paramDataSendToMCU;
    McuSrvSendMesg(&sendToMcu);
    flagParamSyncAck[(uint32_t)paramName] = 1;
}

bool CheckParamSyncAck(void *user_data)
{
    for (uint32_t i = 0; i < E_ParamId_UserEnd + 1; i++)
    {
        /*无应答重发*/
        if (flagParamSyncAck[i] == 1)
        {
            LOG_Debug("sync mcu timer:Sync param to MCU. id:%d",i);
            uint8_t type;
            GetParamsValueType(i, &type);

            if (type == PARAM_TYPE_INT)
            {
                int32_t paramValueInt = 0;
                GetParamsValue(i, &paramValueInt);
                SyncParamToMcu(i, &paramValueInt);
            }
            else
            {
                uint8_t paramValue[256] = {0};
                GetParamsValue(i, paramValue);
                SyncParamToMcu(i, paramValue);
            }
        }
    }
    return TRUE;
}
