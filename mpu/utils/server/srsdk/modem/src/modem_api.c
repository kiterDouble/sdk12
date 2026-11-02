#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "err_code.h"
#include "sr_log.h"
#include "modem_api.h"

int32_t ModemSrvGetImsi(uint8_t *imsi)
{
    int32_t ret = RET_ERR;

    if (!imsi)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_GET_IMSI, 0, NULL, 0, (void*)buffer, &respLen);

    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;

    if (RET_OK == ret)
    {
        memcpy((void*)imsi, (void*)res->data, res->dataLen);
    }

    return ret;
}

int32_t ModemSrvGetIccid(uint8_t *iccid)
{
    int32_t ret = RET_ERR;

    if (!iccid)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_GET_ICCID, 0, NULL, 0, (void*)buffer, &respLen);

    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;

    if (RET_OK == ret)
    {
        memcpy((void*)iccid, (void*)res->data, res->dataLen);
    }

    return ret;
}

int32_t ModemSrvGetImei(uint8_t *imei)
{
    int32_t ret = RET_ERR;

    if (!imei)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_GET_IMEI, 0, NULL, 0, (void*)buffer, &respLen);

    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;

    if (RET_OK == ret)
    {
        memcpy((void*)imei, (void*)res->data, res->dataLen);
    }

    return ret;
}

int32_t ModemSrvGetAirplaneMode(uint8_t *mode)
{
    int32_t ret = RET_ERR;

    if (!mode)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_GET_AIRPLANE_MODE, 0, NULL, 0, (void*)buffer, &respLen);

    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;

    if (RET_OK == ret)
    {
        *mode = res->data[0];
    }

    return ret;
}

int32_t ModemSrvSetAirplaneMode(uint8_t mode)
{
    int32_t ret = RET_ERR;
    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_SET_AIRPLANE_MODE, 0, &mode, sizeof(mode), (void*)buffer, &respLen);

    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;
    return ret;
}

int32_t ModemSrvGetQoosEnable(uint8_t *enable)
{
    int32_t ret = RET_ERR;

    if (!enable)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_GET_QOOS_ENABLE, 0, NULL, 0, (void*)buffer, &respLen);

    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;

    if (RET_OK == ret)
    {
        *enable = res->data[0];
    }

    return ret;
}
int32_t ModemSrvSetQoosEnable(uint8_t enable)
{
    int32_t ret = RET_ERR;
    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_SET_QOOS_ENABLE, 0, &enable, sizeof(enable), (void*)buffer, &respLen);

    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;
    return ret;
}

int32_t ModemSrvGetQoosCfg(ModemSrvQoosCfg_t *qoosCfg)
{
    int32_t ret = RET_ERR;

    if (!qoosCfg)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_GET_QOOS_CFG, 0, NULL, 0, (void*)buffer, &respLen);

    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;

    if ((RET_OK == ret) && (sizeof(ModemSrvQoosCfg_t) == res->dataLen))
    {
        memcpy(qoosCfg, res->data, res->dataLen);
    }
    return ret;
}

int32_t ModemSrvSetQoosCfg(ModemSrvQoosCfg_t *qoosCfg)
{
    int32_t ret = RET_ERR;

    if (!qoosCfg)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_SET_QOOS_CFG, 0, (void*)qoosCfg, sizeof(ModemSrvQoosCfg_t), (void*)buffer, &respLen);

    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;
    return ret;
}
int32_t ModemSrvGetExtSignalInfo(ModemSrvExtSigInfo_t *sigInfo)
{
    int32_t ret = RET_ERR;

    if (!sigInfo)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_GET_EXT_SIGNAL_INFO, 0, (void*)sigInfo, sizeof(ModemSrvExtSigInfo_t), (void*)buffer, &respLen);

    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;

    if ((RET_OK == ret) && (sizeof(ModemSrvExtSigInfo_t) == res->dataLen))
    {
        memcpy(sigInfo, res->data, res->dataLen);
    }
    return ret;
}

int32_t ModemSrvGetOperator(uint8_t*provider)
{
    int32_t ret = RET_ERR;

    if (!provider)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_GET_OPERATOR, 0, NULL, 0, (void*)buffer, &respLen);

    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;

    if (RET_OK == ret)
    {
        memcpy((void*)provider, (void*)res->data, res->dataLen);
    }

    return ret;
}

int32_t ModemSrvSendSmsMesg(uint8_t* phoneNum, uint8_t* textContent, uint32_t textLen)
{
    int32_t ret = RET_ERR;

    if (!phoneNum || !textContent || (textLen == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    ModemSrvSms_t sms;
    strcpy((void*)sms.sourceAddr, (void*)phoneNum);
    strcpy((void*)sms.msgContent, (void*)textContent);
    sms.msgLen = textLen;

    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_SEND_SMS_MESG, 0, (void*)&sms, sizeof(ModemSrvSms_t), (void*)buffer, &respLen);
    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;
    return ret;
}

int32_t ModemSrvStartCall(uint8_t* phoneNum, int32_t *callId)
{
    int32_t ret = RET_ERR;

    if (!phoneNum || !callId)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_START_CALL, 0, (void*)phoneNum, strlen((void *)phoneNum), (void*)buffer, &respLen);
    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;
    if (RET_OK == ret)
    {
        const int32_t *id = (int32_t*)res->data;
        *callId = *id;        
    }

    return ret;
}

int32_t ModemSrvAcceptCall(int32_t callId)
{
    int32_t ret = RET_ERR;

    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_ACCEPT_CALL, 0, (void*)&callId, sizeof(int32_t), (void*)buffer, &respLen);
    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;
    return ret;
}

int32_t ModemSrvStopCall(int32_t callId)
{
    int32_t ret = RET_ERR;

    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_END_CALL, 0, (void*)&callId, sizeof(int32_t), (void*)buffer, &respLen);
    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;
    return ret;
}

int32_t ModemSrvDeinitJob(uint8_t idx)
{
    int32_t ret = RET_ERR;

    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_DEINIT_JOB, 0, (void*)&idx, sizeof(uint8_t), (void*)buffer, &respLen);
    const ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;
    return ret;
}

int32_t ModemSrvGetPhoneNum(uint8_t *phoneNum)
{
    int32_t ret = RET_ERR;

    if (!phoneNum)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    uint8_t buffer[1024] = {0x0,};
    uint16_t respLen = sizeof(buffer);

    RpcGetMsgSync(RPCSERVER_TYPE_MODEMSERVER, E_MODEM_SRV_RPC_SUB_CMD_GET_MSISDN, 0, NULL, 0, (void*)buffer, &respLen);

    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *) buffer;
    ret = res->result;

    if (RET_OK == ret)
    {
        memcpy((void*)phoneNum, (void*)res->data, res->dataLen);
    }

    return ret;
}