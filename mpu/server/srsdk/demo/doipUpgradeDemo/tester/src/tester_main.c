#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "err_code.h"
#include "sr_log.h"
#include "uds_client.h"
#include "doip_tp.h"
#include "cintelhex.h"
#include "algorithm.h"
#include "flash_info.h"

static uint32_t isActivated = 0;
static uint16_t seed = 0x0;
static Flashinfo_t *flashinfo;
static uint32_t segIdx = 0;
DoipTesterNode_t node = 
{
        .ipAddr = {0x0,},
        .activeType = 0xe2,
        .functionalLa = 0xe400,
        .sourceLa = 0x1001,
        .targetLa = 0x1002,
        .maxPL = 0x40000
};

static void DoipConnNotify(uint32_t targetLa, int32_t status)
{
    if (status == E_DOIP_TESTER_CONNECT_SUCCESS)
    {
        isActivated = 1;
    }
    else
    {
        isActivated = 0;
    }
}

static void DoipOnlineNotify(uint32_t targetLa)
{
    LOG_Debug("%s Onlined.targertLa:%x",__FUNCTION__, targetLa);
}

static ssize_t UpgradeRecv(struct UDSTpHandle *hdl, uint8_t *buf, size_t count, uint32_t *tpAddrType)
{
    uint32_t dataLen = 0;
    if ((isActivated == 0) || (RET_OK != DoipTpGetResponse(buf, &dataLen)))
    {
        return -1;
    }
    LOG_DebugHex(buf, dataLen, "tester recv");
    *tpAddrType = E_UDS_TP_ADDR_TYPE_PHYSICAL;
    return dataLen;
}

static ssize_t UpgradeSend(struct UDSTpHandle *hdl, const uint8_t *buf, size_t bufLen, uint32_t tpAddrType)
{
    uint32_t targetLa = node.targetLa;

    if (E_UDS_TP_ADDR_TYPE_FUNCTION == tpAddrType)
    {
        targetLa = node.functionalLa;
    }
    LOG_DebugHex(buf, ((bufLen>128)? 128:bufLen), "tester send");
    if ((isActivated == 0) || (RET_OK != DoipTpTransmit(targetLa, buf, bufLen)))
    {
        return -1;
    }

    return bufLen;
}

static uint32_t UpgradePoll(struct UDSTpHandle *hdl)
{
    return RET_OK;
}

UDSSeqState_t EnterExtMode(struct UDSClient *client)//0x10 0x03
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    client->optionsCopy |= E_UDS_CLT_FUNCTIONAL;
    UDSSendDiagSessCtrl(client, E_UDS_SESSION_TYPE_EXTENDED);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t StopDTCSetting(struct UDSClient *client)//0x 85 0x02
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    client->optionsCopy |= E_UDS_CLT_FUNCTIONAL;
    UDSCtrlDTCSetting(client, E_UDS_DTC_SETTING_OFF, NULL, 0);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t DisableCommu(struct UDSClient *client)//0x28 03 03
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    client->optionsCopy |= E_UDS_CLT_FUNCTIONAL;
    UDSSendCommCtrl(client, E_UDS_COMM_CTL_TYPE_DISABLE_RX_AND_ENABLE_TX, E_UDS_COMM_TYPE_NETWORK_MANAGEMENT_COMM_MESSAGES);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t EnterProgrammingMode(struct UDSClient *client)//0x10 0x02
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    UDSSendDiagSessCtrl(client, E_UDS_SESSION_TYPE_PROGRAMMING);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t GetSecurityAccessSeed(struct UDSClient *client)//0x27 0x03
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    UDSSendSecurityAccess(client, 0x03, NULL, 0);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t UnpackSecurityAccessSeed(struct UDSClient *client)
{



    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    SecurityAccessResponse_t resp;
    UDSUnpackSecurityAccessResponse(client, &resp);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if (resp.securitySeedLength != sizeof(seed))
    {
        LOG_Error("%s seed len invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    seed = (resp.securitySeed[0] << 8) + resp.securitySeed[1];    
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t VerifySecurityAccessKey(struct UDSClient *client)//0x27 0x04
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    uint16_t key = calcKey(seed);
    uint8_t data[] = {((key >> 8) & 0xff), (key & 0xff)};
    UDSSendSecurityAccess(client, 0x04, data, sizeof(data));
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;    
}

UDSSeqState_t EreaseFlash(struct UDSClient *client)//0x31 0x01 0xff 0x00
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    
    uint8_t data[32] = {0x0,};
    uint32_t dataLen = 0;
    data[0] = (flashinfo->segment[segIdx].memaddr >> 24) & 0xff;
    data[1] = (flashinfo->segment[segIdx].memaddr >> 16) & 0xff;
    data[2] = (flashinfo->segment[segIdx].memaddr >> 8) & 0xff;
    data[3] = flashinfo->segment[segIdx].memaddr & 0xff;
    data[4] = (flashinfo->segment[segIdx].memsize >> 24) & 0xff;
    data[5] = (flashinfo->segment[segIdx].memsize >> 16) & 0xff;
    data[6] = (flashinfo->segment[segIdx].memsize >> 8) & 0xff;
    data[7] = flashinfo->segment[segIdx].memsize & 0xff;
    segIdx++;
    dataLen = 8;
    UDSSendRoutineCtrl(client, E_UDS_ROUTINE_CONTROL_TYPE_START, 0xff00, data, dataLen);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t EreaseFlashResult(struct UDSClient *client)//0x31 0x03 0xff 0x00
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    
    UDSSendRoutineCtrl(client, E_UDS_ROUTINE_CONTROL_TYPE_REQUEST_RESULTS, 0xff00, NULL, 0);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t UnpackEreaseFlashResult(struct UDSClient *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    
    RoutineControlResponse_t resp;
    UDSUnpackRoutineControlResponse(client, &resp);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if (1 != resp.routineStatusRecordLength)
    {
        LOG_Error("%s len err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if((0x03 != resp.routineStatusRecord[0]) && (0x02 != resp.routineStatusRecord[0]))
    {
        LOG_Error("%s status err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;        
    }

    if (0x03 == resp.routineStatusRecord[0])
    {
        client->cbIdx -=2;
    }

    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t EreaseFlashEnd(struct UDSClient *client)//0x31 0x02 0xff 0x00
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    
    UDSSendRoutineCtrl(client, E_UDS_ROUTINE_CONTROL_TYPE_STOP, 0xff00, NULL, 0);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t UnpackEreaseFlashEnd(struct UDSClient *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    
    RoutineControlResponse_t resp;
    UDSUnpackRoutineControlResponse(client, &resp);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if (1 != resp.routineStatusRecordLength)
    {
        LOG_Error("%s len err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if(0x04 != resp.routineStatusRecord[0])
    {
        LOG_Error("%s status err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;        
    }

    if (segIdx < flashinfo->segNum)
    {
        int32_t i = 0;
        
        for (; client->cbList[i]; i++)
        {
            if (client->cbList[i] == EreaseFlash)
            {
                client->cbIdx = i;
                break;
            }
        }
    }
    else
    {
        segIdx = 0;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t CheckProgrammingDependency(struct UDSClient *client)//0x31 0x01 0xff 0x01
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    uint8_t data[32];
    uint32_t dataLen = 0;
    uint16_t crc16;

    crc16 = CalcCRC16(flashinfo->segment[segIdx].memdata, flashinfo->segment[segIdx].memsize);
    data[0] = (flashinfo->segment[segIdx].memaddr >> 24) & 0xff;
    data[1] = (flashinfo->segment[segIdx].memaddr >> 16) & 0xff;
    data[2] = (flashinfo->segment[segIdx].memaddr >> 8) & 0xff;
    data[3] = flashinfo->segment[segIdx].memaddr & 0xff;
    data[4] = (flashinfo->segment[segIdx].memsize >> 24) & 0xff;
    data[5] = (flashinfo->segment[segIdx].memsize >> 16) & 0xff;
    data[6] = (flashinfo->segment[segIdx].memsize >> 8) & 0xff;
    data[7] = flashinfo->segment[segIdx].memsize & 0xff;
    data[8] = (crc16 >> 8) & 0xff;
    data[9] = crc16 & 0xff;
    dataLen = 10;
    segIdx++;
    UDSSendRoutineCtrl(client, E_UDS_ROUTINE_CONTROL_TYPE_START, 0xff01, data, dataLen);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t UnpackCheckProgrammingDependency(struct UDSClient *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    
    RoutineControlResponse_t resp;
    UDSUnpackRoutineControlResponse(client, &resp);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if (1 != resp.routineStatusRecordLength)
    {
        LOG_Error("%s len err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if(0x01 != resp.routineStatusRecord[0])
    {
        LOG_Error("%s status err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t CheckProgrammingDependencyResult(struct UDSClient *client)//0x31 0x03 0xff 0x01
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    
    UDSSendRoutineCtrl(client, E_UDS_ROUTINE_CONTROL_TYPE_REQUEST_RESULTS, 0xff01, NULL, 0);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t UnpackCheckProgrammingDependencyResult(struct UDSClient *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    
    RoutineControlResponse_t resp;
    UDSUnpackRoutineControlResponse(client, &resp);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if (1 != resp.routineStatusRecordLength)
    {
        LOG_Error("%s len err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if(0x02 != resp.routineStatusRecord[0])
    {
        LOG_Error("%s status err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;        
    }

    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}


UDSSeqState_t CheckProgrammingDependencyEnd(struct UDSClient *client)//0x31 0x02 0xff 0x01
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    
    UDSSendRoutineCtrl(client, E_UDS_ROUTINE_CONTROL_TYPE_STOP, 0xff01, NULL, 0);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t UnpackCheckProgrammingDependencyEnd(struct UDSClient *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    
    RoutineControlResponse_t resp;
    UDSUnpackRoutineControlResponse(client, &resp);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if (2 != resp.routineStatusRecordLength)
    {
        LOG_Error("%s len err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if ((0x04 != resp.routineStatusRecord[0]) || (0x01 != resp.routineStatusRecord[1]))
    {
        LOG_Error("%s status err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;        
    }

    if (segIdx < flashinfo->segNum)
    {
        client->cbIdx = 0;
    }
    else
    {
        segIdx = 0;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t CheckProgrammingValidation(struct UDSClient *client)//0x31 0x01 0xda 0x04
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    uint8_t data[32];
    uint32_t dataLen = 0;
    uint16_t crc16;

    crc16 = CalcCRC16(flashinfo->data, flashinfo->dataLen);
    data[0] = (crc16 >> 8) & 0xff;
    data[1] = crc16 & 0xff;
    dataLen = 2;
    UDSSendRoutineCtrl(client, E_UDS_ROUTINE_CONTROL_TYPE_START, 0xda04, data, dataLen);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t CheckProgrammingValidationResult(struct UDSClient *client)//0x31 0x01 0xda 0x04
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    
    UDSSendRoutineCtrl(client, E_UDS_ROUTINE_CONTROL_TYPE_REQUEST_RESULTS, 0xda04, NULL, 0);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t UnpackCheckProgrammingValidationResult(struct UDSClient *client)//0x31 0x01 0xda 0x04
{

    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    RoutineControlResponse_t resp;
    UDSUnpackRoutineControlResponse(client, &resp);
    if (1 != resp.routineStatusRecordLength)
    {
        LOG_Error("%s len err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if (0x02 != resp.routineStatusRecord[0])
    {
        LOG_Error("%s status err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;        
    }

    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t CheckProgrammingValidationEnd(struct UDSClient *client)//0x31 0x01 0xda 0x04
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    
    UDSSendRoutineCtrl(client, E_UDS_ROUTINE_CONTROL_TYPE_STOP, 0xda04, NULL, 0);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t UnpackCheckProgrammingValidationEnd(struct UDSClient *client)//0x31 0x01 0xda 0x04
{

    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    RoutineControlResponse_t resp;
    UDSUnpackRoutineControlResponse(client, &resp);
    if (2 != resp.routineStatusRecordLength)
    {
        LOG_Error("%s len err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    if ((0x04 != resp.routineStatusRecord[0]) || (0x01 != resp.routineStatusRecord[1]))
    {
        LOG_Error("%s status err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;        
    }

    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t EnableCommunication(struct UDSClient *client)//0x28 0x00 0x03
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    client->optionsCopy |= E_UDS_CLT_FUNCTIONAL;
    UDSSendCommCtrl(client, E_UDS_COMM_CTL_TYPE_ENABLE_RX_AND_TX, E_UDS_COMM_TYPE_NETWORK_MANAGEMENT_AND_NORMAL_COMM_MESSAGES);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t StartDTCSetting(struct UDSClient *client)//0x 85 0x01
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    client->optionsCopy |= E_UDS_CLT_FUNCTIONAL;
    UDSCtrlDTCSetting(client, E_UDS_DTC_SETTING_ON, NULL, 0);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t ClearDtc(struct UDSClient *client)//0x 14 0xff 0xff 0xff
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }

    UDSSendClearDtcInfo(client, 0xffffff);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

UDSSeqState_t EnterDefaultSession(struct UDSClient *client)//0x 10 0x01
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    client->optionsCopy |= E_UDS_CLT_FUNCTIONAL;
    UDSSendDiagSessCtrl(client, E_UDS_SESSION_TYPE_DEFAULT);
    if (client->err)
    {
        LOG_Error("%s client err.", __FUNCTION__);
        return E_UDS_SEQ_STATE_DONE;
    }
    LOG_Debug("%s, line:%d", __FUNCTION__, __LINE__);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

static int32_t UpgradePreProgramming(struct UDSClient *client)
{

    static const UDSClientCallback callbacks[] =
    {
        EnterExtMode,
        UDSClientAwaitIdle,
        StopDTCSetting,
        UDSClientAwaitIdle,
        DisableCommu,
        UDSClientAwaitIdle,
        NULL
    };

    client->cbList = callbacks;
    client->cbIdx = 0;

    int32_t running = UDS_CLIENT_RUNNING;
    do
    {
        running = UDSClientPoll(client);
        SleepMillis(10);
    }
    while (running);

    if (NULL != client->cbList[client->cbIdx])
    {
        LOG_Error("%s callbacks[client->cbIdx] not NULL.", __FUNCTION__);
        return RET_ERR;
    }
    return RET_OK;
}


static int32_t UpgradeEreaseFlash(struct UDSClient *client)
{
    static const UDSClientCallback callbacks[] =
    {
        EnterProgrammingMode,
        UDSClientAwaitIdle,
        GetSecurityAccessSeed,
        UDSClientAwaitIdle,
        UnpackSecurityAccessSeed,
        VerifySecurityAccessKey,
        UDSClientAwaitIdle,
        EreaseFlash,
        UDSClientAwaitIdle,
        EreaseFlashResult,
        UDSClientAwaitIdle,
        UnpackEreaseFlashResult,
        EreaseFlashEnd,
        UDSClientAwaitIdle,
        UnpackEreaseFlashEnd,
        NULL
    };

    client->cbList = callbacks;
    client->cbIdx = 0;

    int32_t running = UDS_CLIENT_RUNNING;
    do
    {
        running = UDSClientPoll(client);
        SleepMillis(10);
    }
    while (running);
    if (NULL != client->cbList[client->cbIdx])
    {
        LOG_Error("%s callbacks[client->cbIdx] not NULL.", __FUNCTION__);
        return RET_ERR;
    }
    return RET_OK;
}

static int32_t UpgradeProgramingCheck(struct UDSClient *client)
{
    static const UDSClientCallback callbacks[] =
    {
        CheckProgrammingDependency,
        UDSClientAwaitIdle,
        UnpackCheckProgrammingDependency,
        CheckProgrammingDependencyResult,
        UDSClientAwaitIdle,
        UnpackCheckProgrammingDependencyResult,
        CheckProgrammingDependencyEnd,
        UDSClientAwaitIdle,
        UnpackCheckProgrammingDependencyEnd,
        CheckProgrammingValidation,
        UDSClientAwaitIdle,
        CheckProgrammingValidationResult,
        UDSClientAwaitIdle,
        UnpackCheckProgrammingValidationResult,
        CheckProgrammingValidationEnd,
        UDSClientAwaitIdle,
        UnpackCheckProgrammingValidationEnd,
        NULL
    };

    segIdx = 0;
    client->cbList = callbacks;
    client->cbIdx = 0;

    int32_t running = UDS_CLIENT_RUNNING;
    do
    {
        running = UDSClientPoll(client);
        SleepMillis(10);
    }
    while (running);
    if (NULL != client->cbList[client->cbIdx])
    {
        LOG_Error("%s callbacks[client->cbIdx] not NULL.", __FUNCTION__);
        return RET_ERR;
    }
    return RET_OK;
}

typedef struct {
    uint8_t dataFormatIdentifier;
    uint8_t addressAndLengthFormatIdentifier;
    size_t memoryAddress;
    size_t memorySize;
    FILE *fd;
    uint8_t blockSequenceCounter;
    uint16_t blockLength;
} TesterDownloadSequence_t;

static UDSSeqState_t RequestDownload(UDSClient_t *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    const TesterDownloadSequence_t *plSeq = (TesterDownloadSequence_t *)client->cbData;
    UDSSendRequestDownload(client, plSeq->dataFormatIdentifier,
                           plSeq->addressAndLengthFormatIdentifier, plSeq->memoryAddress,
                           plSeq->memorySize);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

static UDSSeqState_t CheckRequestDownloadResponse(UDSClient_t *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    TesterDownloadSequence_t *plSeq = (TesterDownloadSequence_t *)client->cbData;
    RequestDownloadResponse_t resp = {0};
    UDSErr_t err = UDSUnpackRequestDownloadResponse(client, &resp);
    if (err)
    {
        client->err = err;
        return E_UDS_SEQ_STATE_DONE;
    }
    plSeq->blockLength = resp.maxNumberOfBlockLength;
    if (0 == resp.maxNumberOfBlockLength)
    {
        client->err = E_UDS_ERR;
        return E_UDS_SEQ_STATE_DONE;
    }
    return E_UDS_SEQ_STATE_GOTONEXT;
}

static UDSSeqState_t PrepareToTransfer(UDSClient_t *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    TesterDownloadSequence_t *plSeq = (TesterDownloadSequence_t *)client->cbData;
    plSeq->blockSequenceCounter = 1;
    return E_UDS_SEQ_STATE_GOTONEXT;
}

static UDSSeqState_t TransferData(UDSClient_t *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }
    TesterDownloadSequence_t *plSeq = (TesterDownloadSequence_t *)client->cbData;
    if (E_UDS_CLT_REQUEST_STATE_IDLE == client->state)
    {
        if (ferror(plSeq->fd))
        {
            fclose(plSeq->fd);
            client->err = E_UDS_ERR_FILE_IO; // 读取文件故障
            return E_UDS_SEQ_STATE_DONE;
        } 
        else if (feof(plSeq->fd))
        { // 传完了
            return E_UDS_SEQ_STATE_GOTONEXT;
        }
        else
        {
            UDSSendTransferDataStream(client, plSeq->blockSequenceCounter++, plSeq->blockLength,
                                      plSeq->fd);
        }
    }
    return E_UDS_SEQ_STATE_RUNNING;
}

static UDSSeqState_t RequestTransferExit(UDSClient_t *client)
{
    if (!client)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }

    UDSSendRequestTransferExit(client, NULL, 0);
    return E_UDS_SEQ_STATE_GOTONEXT;
}

static UDSErr_t TesterConfigDownload(UDSClient_t *client, uint8_t dataFormatIdentifier, 
                        uint8_t addressAndLengthFormatIdentifier, size_t memoryAddress, size_t memorySize, FILE *fd)
{
    if (!client || !fd)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_ERR;
    }

    static const UDSClientCallback callbacks[] =
    {
        RequestDownload, UDSClientAwaitIdle,  CheckRequestDownloadResponse, PrepareToTransfer,
        TransferData,    RequestTransferExit, UDSClientAwaitIdle, NULL
    };
    static TesterDownloadSequence_t seq = {0};
    memset(&seq, 0, sizeof(seq));
    seq.blockSequenceCounter = 1;
    seq.dataFormatIdentifier = dataFormatIdentifier;
    seq.addressAndLengthFormatIdentifier = addressAndLengthFormatIdentifier;
    seq.memoryAddress = memoryAddress;
    seq.memorySize = memorySize;
    seq.fd = fd;
    client->cbList = callbacks;
    client->cbIdx = 0;
    client->cbData = &seq;
    return E_UDS_OK;
}

int32_t writeFile(const uint8_t *data, uint32_t dataLen) 
{
    FILE *file = fopen("example2.bin", "wb"); // 以二进制写入模式打开文件
    if (file == NULL)
    {
        LOG_Debug("%s fopen fail.",__FUNCTION__);
        return -1;
    }

    // 将数据写入文件fwrite(data, 1, dataLen, file);
    fwrite(data, 1, dataLen, file);
    fclose(file);
    return 0;
}

static int32_t UpgradeProgramming(struct UDSClient *client)
{ 


    if (RET_OK != UpgradeEreaseFlash(client))
    {
        LOG_Error("%s UpgradeEreaseFlash fail.", __FUNCTION__);
        return RET_ERR;
    }

    uint32_t i = 0;
    for (; i < flashinfo->segNum; i++)
    {
        writeFile(flashinfo->segment[i].memdata, flashinfo->segment[i].memsize);
        FILE *fp = fmemopen(flashinfo->segment[i].memdata, flashinfo->segment[i].memsize, "r");

        if (!fp)
        {
            return RET_ERR;
        }
#if 0
        UDSConfigDownload(client, 0x00, 0x44, flashinfo->segment[i].memaddr, flashinfo->segment[i].memsize, fp);
#else
        TesterConfigDownload(client, 0x00, 0x44, flashinfo->segment[i].memaddr, flashinfo->segment[i].memsize, fp);
#endif
        int32_t running = UDS_CLIENT_RUNNING;
        do
        {
            running = UDSClientPoll(client);
            SleepMillis(10);
        }
        while (running);
        fclose(fp);
        if (NULL != client->cbList[client->cbIdx])
        {
            LOG_Error("%s download fail.", __FUNCTION__);
            return RET_ERR;
        }
    }
    if (RET_OK != UpgradeProgramingCheck(client))
    {
        LOG_Error("%s UpgradeProgramingCheck fail.", __FUNCTION__);
        return RET_ERR;
    }
    return RET_OK;
}

static int32_t UpgradePostProgramming(struct UDSClient *client)
{
    static const UDSClientCallback callbacks[] =
    {
        EnableCommunication,
        UDSClientAwaitIdle,
        StartDTCSetting,
        UDSClientAwaitIdle,
        ClearDtc,
        UDSClientAwaitIdle,
        EnterDefaultSession,
        UDSClientAwaitIdle,
        NULL
    };

    client->cbList = callbacks;
    client->cbIdx = 0;

    int32_t running = UDS_CLIENT_RUNNING;
    do
    {
        running = UDSClientPoll(client);
        SleepMillis(10);
    }
    while (running);

    if (NULL != client->cbList[client->cbIdx])
    {
        return RET_ERR;
    }
    return RET_OK;
}

static int32_t WaitActived(void)
{
    int32_t count = 0;

    while(count++ < 20)
    {
        if (isActivated)
        {
            return RET_OK;
        }
        SleepMillis(100);
    }
    return RET_ERR;
}

static int32_t DoipUpgrade(void)
{
    int32_t ret = RET_ERR;
    if (RET_OK != DoipTpConnect(&node))
    {
        LOG_Error("%s DoipTpConnect fail.", __FUNCTION__);
        return ret;
    }

    if (RET_OK != WaitActived())
    {
        LOG_Error("%s WaitActived fail.", __FUNCTION__);
        DoipTpDisconnect();
        return ret;        
    }

    UDSClient_t client;
    UDSTpHandle_t udsTphdl = 
    {
        .send = UpgradeSend,
        .recv = UpgradeRecv,
        .poll = UpgradePoll,
        .impl = NULL
    };
    UDSClientConfig_t cfg = 
    {
        .tp = &udsTphdl
    };

    int32_t retVal = RET_ERR;
    if (UDSClientInit(&client, &cfg) < 0)
    {
        LOG_Error("%s UDSClientInit fail.", __FUNCTION__);
        DoipTpDisconnect();
        return ret;
    }

    if (E_UDS_OK != UpgradePreProgramming(&client))
    {
        UDSClientDeInit(&client);
        DoipTpDisconnect();
        return ret;
    }

    retVal = UpgradeProgramming(&client);

    UpgradePostProgramming(&client);
    UDSClientDeInit(&client);
    DoipTpDisconnect();
    return retVal;
}

int main(int argc, char *argv[])
{
    LOG_INIT("demo");
    if (argc != 2)
    {
        LOG_Error("%s args invalid.", __FUNCTION__);
        return -1;

    }
    if (RET_OK != GetFlashInfoFromHex(&flashinfo, argv[1]))
    {
        LOG_Error("%s GetFlashInfoFromHex failed.", __FUNCTION__);
        return -1;        
    }

    if (RET_OK != DoipTpInit(DoipConnNotify, DoipOnlineNotify))
    {
        LOG_Error("%s DoipTpInit fail.", __FUNCTION__);
        FlashInfoFree(flashinfo);
        return -1;
    }
    int32_t retVal;
    retVal = DoipUpgrade();
    if (RET_OK != retVal)
    {
        LOG_Error("%s DoipUpgrade fail.", __FUNCTION__);
    }
    else
    {
        LOG_Debug("%s DoipUpgrade success.", __FUNCTION__);
    }
    DoipTpDeinit();
    FlashInfoFree(flashinfo);
    return retVal;
}