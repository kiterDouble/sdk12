#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "err_code.h"
#include "sr_log.h"
#include "doip_entity.h"
#include "algorithm.h"
#include "msg_queue.h"
#include "uds_server.h"

#define SUPPORT_MAX_SEGMENT (20)
typedef struct
{
    uint32_t memaddr;
    uint32_t memsize;
    uint32_t offset;
}FlashSegmentInfo_t;
typedef struct 
{
    uint32_t segNum;
    FlashSegmentInfo_t segment[SUPPORT_MAX_SEGMENT];
    uint32_t dataLen;
    uint8_t *data;
}Flashinfo_t;

typedef uint8_t (*udsEventProcessCb)(struct UDSServer *srv, UDSServerEvent_t event, const void *arg);
typedef struct 
{
    UDSServerEvent_t event;
    udsEventProcessCb processCb;
}UdsEventProcess_t;

static MsgQueueHandle_t respQueue;
static DoipEntityHandle_t entity;
static Flashinfo_t flashinfo = 
{
    .segNum = 0,
    .dataLen = 0,
    .data = NULL
};
static uint32_t gtesterLa;
static uint16_t g16seed = 0;
//static FILE *fp = NULL;
static uint8_t flashIdx = 0;
uint8_t UdsEvProcDiagSessCtrl(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    UDSDiagSessCtrlArgs_t *ctlargs = (UDSDiagSessCtrlArgs_t *)arg;
    LOG_Debug("%s called.", __FUNCTION__);
    if (!srv || !ctlargs || (E_UDS_SRV_EVT_DiagSessCtrl != event))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;
    }

    if ((ctlargs->type == E_UDS_SESSION_TYPE_PROGRAMMING) && (srv->sessionType != E_UDS_SESSION_TYPE_EXTENDED))
    {
		LOG_Error("%s,condition not collect.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_CONDITIONS_NOT_CORRECT;
    }

    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

uint8_t UdsEvProcDiagEcuReset(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    LOG_Debug("%s called.", __FUNCTION__);
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

uint8_t UdsEvProcDiagClearDiagnostic(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    LOG_Debug("%s called.", __FUNCTION__);
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

uint8_t UdsEvProcDiagReadDataByIdent(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    LOG_Debug("%s called.", __FUNCTION__);
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

uint8_t UdsEvProcCommCtrl(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    LOG_Debug("%s called.", __FUNCTION__);
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

uint8_t UdsEvProcSecAccessRequestSeed(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    UDSSecAccessRequestSeedArgs_t *seedarg = (UDSSecAccessRequestSeedArgs_t *)arg;
    LOG_Debug("%s called.", __FUNCTION__);
    if (!srv || !seedarg || (E_UDS_SRV_EVT_SecAccessRequestSeed != event))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;
    }
    unsigned char seed[2] = {0x0,};

    if ((seedarg->level == 0x03) && (srv->sessionType != E_UDS_SESSION_TYPE_PROGRAMMING))
    {
        return E_UDS_SRV_RESP_CODE_SECURITY_ACCESS_DENIED;
    }
    srand(time(NULL));
    g16seed = rand();
    seed[0] = (g16seed >> 8) & 0xff;
    seed[1] = g16seed & 0xff;
    seedarg->copySeed(srv, seed, sizeof(seed));
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

uint8_t UdsEvProcSecAccessValidateKey(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    LOG_Debug("%s called.", __FUNCTION__);
    UDSSecAccessValidateKeyArgs_t *keyargs = (UDSSecAccessValidateKeyArgs_t *)arg;
    if (!srv || !keyargs || (E_UDS_SRV_EVT_SecAccessValidateKey != event))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;
    }

    if ((keyargs->level != 0x04) && (srv->sessionType != E_UDS_SESSION_TYPE_PROGRAMMING))
    {
        return E_UDS_SRV_RESP_CODE_SECURITY_ACCESS_DENIED;
    }

    uint16_t keyint = 0;
    keyint += (keyargs->key[0] & 0xff) << 8;
    keyint += (keyargs->key[1] & 0xff);
    
    if (keyint != calcKey(g16seed))
    {
        return E_UDS_SRV_RESP_CODE_INVALID_KEY;
    }
    flashinfo.segNum = 0;
    flashinfo.dataLen = 0;
    if (flashinfo.data)
    {
        free(flashinfo.data);
        flashinfo.data = NULL;
    }
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

uint8_t UdsEvProcWriteDataByIdent(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    LOG_Debug("%s called.", __FUNCTION__);
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

static uint8_t crc16check = 0;
static uint8_t ProcRoutineCtrlStart(struct UDSServer *srv, UDSRoutineCtrlArgs_t *ctlargs)
{
    if (!srv || !ctlargs)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;
    }

    if (0xff00 == ctlargs->id)
    {

        if (ctlargs->len != 8)
        {
            return E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT;
        }
        uint32_t memaddr = 0;
        uint32_t memsize = 0;
        uint32_t offset = 0;
        //uint32_t dataLen = 0;
        memaddr = (ctlargs->optionRecord[offset++] & 0xff) << 24;
        memaddr += (ctlargs->optionRecord[offset++] & 0xff) << 16;
        memaddr += (ctlargs->optionRecord[offset++] & 0xff) << 8;
        memaddr += ctlargs->optionRecord[offset++] & 0xff;
        memsize = (ctlargs->optionRecord[offset++] & 0xff) << 24;
        memsize += (ctlargs->optionRecord[offset++] & 0xff) << 16;
        memsize += (ctlargs->optionRecord[offset++] & 0xff) << 8;
        memsize += ctlargs->optionRecord[offset++] & 0xff;
        flashinfo.segment[flashinfo.segNum].memaddr = memaddr;
        flashinfo.segment[flashinfo.segNum].memsize = memsize;
        flashinfo.segment[flashinfo.segNum].offset = flashinfo.dataLen;
        flashinfo.segNum++;
        flashinfo.dataLen += memsize;
        return E_UDS_SRV_RESP_CODE_POSITIVE;
    }
    else if (0xff01 == ctlargs->id)
    {
        if (ctlargs->len != 10)
        {
            return E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT;
        }
        uint32_t i = 0;
        uint32_t memaddr = 0;
        uint32_t memsize = 0;
        uint32_t crc16 = 0;
        uint32_t offset = 0;

        memaddr = (ctlargs->optionRecord[offset++] & 0xff) << 24;
        memaddr += (ctlargs->optionRecord[offset++] & 0xff) << 16;
        memaddr += (ctlargs->optionRecord[offset++] & 0xff) << 8;
        memaddr += ctlargs->optionRecord[offset++] & 0xff;
        memsize = (ctlargs->optionRecord[offset++] & 0xff) << 24;
        memsize += (ctlargs->optionRecord[offset++] & 0xff) << 16;
        memsize += (ctlargs->optionRecord[offset++] & 0xff) << 8;
        memsize += ctlargs->optionRecord[offset++] & 0xff;
        crc16 = (ctlargs->optionRecord[offset++] & 0xff) << 8;
        crc16 += ctlargs->optionRecord[offset++] & 0xff;
        LOG_Debug("%s crc16:%x.", __FUNCTION__, crc16);
        for (; i < flashinfo.segNum; i++)
        {
            if ((memaddr == flashinfo.segment[i].memaddr) && (memsize == flashinfo.segment[i].memsize))
            {
                if (!flashinfo.data || ((flashinfo.segment[i].offset + flashinfo.segment[i].memsize) > flashinfo.dataLen))
                {
                    LOG_Debug("%s memdata error.", __FUNCTION__);
                    return E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;
                }

                uint16_t crc = CalcCRC16(flashinfo.data + flashinfo.segment[i].offset, flashinfo.segment[i].memsize);
                LOG_Debug("%s crc:%x.", __FUNCTION__, crc);
                crc16check = (crc16 == crc);
                uint8_t status = 0x01;
                ctlargs->copyStatusRecord(srv, &status, sizeof(status));
                return E_UDS_SRV_RESP_CODE_POSITIVE;
            }
        }
        LOG_Debug("%s memdata error.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;

    }
    else if (0xda04 == ctlargs->id)
    {
        if (ctlargs->len != 2)
        {
            return E_UDS_SRV_RESP_CODE_INCORRECT_MESG_LEN_OR_INVALID_FORMAT;
        }

        uint32_t offset = 0;
        int16_t crc16 = 0;

        crc16 = (ctlargs->optionRecord[offset++] & 0xff) << 8;
        crc16 += ctlargs->optionRecord[offset++] & 0xff;
        crc16check = (crc16 == CalcCRC16(flashinfo.data, flashinfo.dataLen));
        uint8_t status = 0x01;
        ctlargs->copyStatusRecord(srv, &status, sizeof(status));
        return E_UDS_SRV_RESP_CODE_POSITIVE;
    }
    else
    {
        return E_UDS_SRV_RESP_CODE_SUBFUN_NOT_SUPPORTED;
    }
}

static uint8_t ProcRoutineCtrlStop(struct UDSServer *srv, UDSRoutineCtrlArgs_t *ctlargs)
{
    LOG_Debug("%s called.", __FUNCTION__);

    if (!srv || !ctlargs)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;
    }
    if (0xff00 == ctlargs->id)
    {

        uint8_t status = 0x04;
        ctlargs->copyStatusRecord(srv, &status, sizeof(status));
        return E_UDS_SRV_RESP_CODE_POSITIVE;
    }
    else if (0xff01 == ctlargs->id)
    {
        uint8_t status[2] = {0x04, crc16check};
        ctlargs->copyStatusRecord(srv, status, sizeof(status));
        return E_UDS_SRV_RESP_CODE_POSITIVE;

    }
    else if (0xda04 == ctlargs->id)
    {
        uint8_t status[2] = {0x04, crc16check};
        ctlargs->copyStatusRecord(srv, status, sizeof(status));
        return E_UDS_SRV_RESP_CODE_POSITIVE;
    }
    else
    {
        return E_UDS_SRV_RESP_CODE_SUBFUN_NOT_SUPPORTED;
    }
}

static uint8_t ProcRoutineCtrlResult(struct UDSServer *srv, UDSRoutineCtrlArgs_t *ctlargs)
{
    LOG_Debug("%s called.", __FUNCTION__);

    if (!srv || !ctlargs)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;
    }
    switch(ctlargs->id)
    {
    case 0xda04:
    case 0xff00:
    case 0xff01:
    {
        uint8_t status = 0x02;
        ctlargs->copyStatusRecord(srv, &status, sizeof(status));
        return E_UDS_SRV_RESP_CODE_POSITIVE;    
    }
    default:
        return E_UDS_SRV_RESP_CODE_SUBFUN_NOT_SUPPORTED;
    }
}

uint8_t UdsEvProcRoutineCtrl(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    LOG_Debug("%s called.", __FUNCTION__);

    UDSRoutineCtrlArgs_t *ctlargs = (UDSRoutineCtrlArgs_t *)arg;
    if (!srv || !ctlargs || (E_UDS_SRV_EVT_RoutineCtrl != event))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;
    }

    if (srv->sessionType != E_UDS_SESSION_TYPE_PROGRAMMING)
    {
        return E_UDS_SRV_RESP_CODE_SECURITY_ACCESS_DENIED;
    }

    if (E_UDS_ROUTINE_CONTROL_TYPE_START == ctlargs->ctrlType)
    {
        return ProcRoutineCtrlStart(srv, ctlargs);
    }
    else if (E_UDS_ROUTINE_CONTROL_TYPE_STOP == ctlargs->ctrlType)
    {
        return ProcRoutineCtrlStop(srv, ctlargs);
    }
    else if(E_UDS_ROUTINE_CONTROL_TYPE_REQUEST_RESULTS == ctlargs->ctrlType)
    {
        return ProcRoutineCtrlResult(srv, ctlargs);
    }
    else 
    {
        return E_UDS_SRV_RESP_CODE_SUBFUN_NOT_SUPPORTED;
    }
}
static uint32_t gxferlen = 0;
uint8_t UdsEvProcRequestDownload(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    LOG_Debug("%s called.", __FUNCTION__);

    UDSRequestDownloadArgs_t *downArgs = (UDSRequestDownloadArgs_t *)arg;
    if (!srv || !downArgs || (E_UDS_SRV_EVT_RequestDownload != event))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;
    }
    if (srv->sessionType != E_UDS_SESSION_TYPE_PROGRAMMING)
    {
        return E_UDS_SRV_RESP_CODE_SECURITY_ACCESS_DENIED;
    }
    if (flashinfo.dataLen > 0)
    {
        if (!flashinfo.data)
        {
            flashinfo.data = (uint8_t*)calloc(1, flashinfo.dataLen);
            if (!flashinfo.data)
            {
                return E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;
            }
        }
    }
    else
    {
        return E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;
    }

    uint32_t i = 0;
    uint32_t offset = 0xffffffff;

    for (; i < flashinfo.segNum; i++)
    {
        if ((flashinfo.segment[i].memaddr == (uint32_t)downArgs->addr) && (flashinfo.segment[i].memsize == downArgs->size))
        {
            offset = flashinfo.segment[i].offset;
            break;
        }
    }

    if (offset == 0xffffffff)
    {
        return E_UDS_SRV_RESP_CODE_UPLOAD_DOWNLOAD_NOT_ACCEPTED;
    }

    flashIdx = i;
    //fp = fmemopen(flashinfo.data + offset, downArgs->size, "w");
    gxferlen = 0;
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

uint8_t UdsEvProcRequestUpload(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    LOG_Debug("%s called.", __FUNCTION__);

    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

uint8_t UdsEvProcTransferData(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    UDSTransferDataArgs_t *xferargs = (UDSTransferDataArgs_t *)arg;
    
    if (!srv || !xferargs || (E_UDS_SRV_EVT_TransferData != event))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;
    }

    if (srv->sessionType != E_UDS_SESSION_TYPE_PROGRAMMING)
    {
        return E_UDS_SRV_RESP_CODE_SECURITY_ACCESS_DENIED;
    }
#if 0
    if (!fp)
    {
        LOG_Error("---%s, fp is NULL.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;
    }
#endif     
    //fwrite(xferargs->data, 1, xferargs->len, fp);
    memmove(&flashinfo.data[flashinfo.segment[flashIdx].offset + gxferlen], xferargs->data, xferargs->len);
    //LOG_Debug("%s , xferargs->len:%d,last byte:%02x", __FUNCTION__, xferargs->len, xferargs->data[xferargs->len -1]);
    gxferlen += xferargs->len;
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

int32_t writeFile(const uint8_t *data, uint32_t dataLen) 
{
    FILE *file = fopen("example.bin", "wb"); // 以二进制写入模式打开文件
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

uint8_t UdsEvProcRequestTransferExit(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    LOG_Debug("%s called.", __FUNCTION__);

    UDSRequestTransferExitArgs_t *exitArgs = (UDSRequestTransferExitArgs_t*)arg;
    if (!srv || !exitArgs || (E_UDS_SRV_EVT_RequestTransferExit != event))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;
    }

    if (srv->sessionType != E_UDS_SESSION_TYPE_PROGRAMMING)
    {
        return E_UDS_SRV_RESP_CODE_SECURITY_ACCESS_DENIED;
    }
#if 0
    if (!fp)
    {
        LOG_Error("---%s, fp is NULL.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_UPLOAD_DOWNLOAD_NOT_ACCEPTED;
    }
    fclose(fp);
#endif
    uint8_t crc8 = CalcCRC8(flashinfo.data + flashinfo.segment[flashIdx].offset, flashinfo.segment[flashIdx].memsize);
    writeFile(flashinfo.data + flashinfo.segment[flashIdx].offset, flashinfo.segment[flashIdx].memsize);
    exitArgs->copyResponse(srv, &crc8, sizeof(crc8));
    LOG_Debug("%s gxferlen:%x", __FUNCTION__, gxferlen);
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

uint8_t UdsEvProcSessionTimeout(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    //LOG_Debug("%s called.", __FUNCTION__);

    if (!srv || (E_UDS_SRV_EVT_SessionTimeout != event))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return E_UDS_SRV_RESP_CODE_GENERAL_PROGRAMMING_FAILURE;
    }

    if (srv->sessionType == E_UDS_SESSION_TYPE_DEFAULT)
    {
        return E_UDS_SRV_RESP_CODE_POSITIVE;
    }
    flashinfo.segNum = 0;
    flashinfo.dataLen = 0;
    if (flashinfo.data)
    {
        free(flashinfo.data);
        flashinfo.data = NULL;
    }
    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

uint8_t UdsEvProcControlDTCSetting(struct UDSServer *srv, UDSServerEvent_t event, const void *arg)
{
    LOG_Debug("%s called.", __FUNCTION__);

    return E_UDS_SRV_RESP_CODE_POSITIVE;
}

static UdsEventProcess_t gEventProcess[] = 
{
    {E_UDS_SRV_EVT_DiagSessCtrl, UdsEvProcDiagSessCtrl},
    {E_UDS_SRV_EVT_EcuReset, UdsEvProcDiagEcuReset},
    {E_UDS_SRV_EVT_ClearDiagnostic, UdsEvProcDiagClearDiagnostic},
    {E_UDS_SRV_EVT_ReadDataByIdent, UdsEvProcDiagReadDataByIdent},
    {E_UDS_SRV_EVT_CommCtrl, UdsEvProcCommCtrl},
    {E_UDS_SRV_EVT_SecAccessRequestSeed, UdsEvProcSecAccessRequestSeed},
    {E_UDS_SRV_EVT_SecAccessValidateKey, UdsEvProcSecAccessValidateKey},
    {E_UDS_SRV_EVT_WriteDataByIdent, UdsEvProcWriteDataByIdent},
    {E_UDS_SRV_EVT_RoutineCtrl, UdsEvProcRoutineCtrl},
    {E_UDS_SRV_EVT_RequestDownload, UdsEvProcRequestDownload},
    {E_UDS_SRV_EVT_RequestUpload, UdsEvProcRequestUpload},
    {E_UDS_SRV_EVT_TransferData, UdsEvProcTransferData},
    {E_UDS_SRV_EVT_RequestTransferExit, UdsEvProcRequestTransferExit},
    {E_UDS_SRV_EVT_SessionTimeout, UdsEvProcSessionTimeout},
    {E_UDS_SRV_EVT_ControlDTCSetting, UdsEvProcControlDTCSetting}
};

void UdsRequestProcess(uint32_t testerLa, const uint8_t *udsData, uint32_t dataLen)
{
    gtesterLa = testerLa;

    if (!udsData || !dataLen)
    {
        return;
    }
    LOG_Debug("%s called. dataLen:%d", __FUNCTION__, dataLen);
    SendMsgQueue(respQueue, (void *) udsData, dataLen);
}

static ssize_t UdsSrvRecv(struct UDSTpHandle *hdl, uint8_t *buf, size_t count, uint32_t *tpAddrType)
{
    if (!hdl || !buf)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return RET_ERR;
    }
    uint32_t dataLen = 0;
    if (RET_OK != RecvMsgQueue(respQueue, buf, &dataLen, 1))
    {
        return 0;
    }
    LOG_DebugHex(buf, ((dataLen>128)? 128:dataLen), "entity recv");
    LOG_Debug("%s , dataLen:%d,last byte:%02x", __FUNCTION__, dataLen, buf[dataLen -1]);
    return dataLen;
}

static ssize_t UdsSrvSend(struct UDSTpHandle *hdl, const uint8_t *buf, size_t bufLen, uint32_t tpAddrType)
{
    if (!hdl || !buf)
    {
        LOG_Error("%s invalid param.", __FUNCTION__);
        return RET_ERR;
    }

    //LOG_Error("%s invalid param.", __FUNCTION__);
    LOG_DebugHex(buf, ((bufLen>128)? 128:bufLen), "enetity send");
    if (RET_OK != DoipEntityUdsResponse(entity, gtesterLa, buf, bufLen))
    {
        return 0;
    }
    //LOG_DebugHex(buf, count, "enetity send");
    return bufLen;
}

static uint32_t UdsSrvPoll(struct UDSTpHandle *hdl)
{
    return RET_OK;
}

static uint8_t UdsSrvProcess(UDSServer_t *srv, UDSServerEvent_t event, const void *arg)
{
    int i = 0;

    for (; i < sizeof(gEventProcess) / sizeof(UdsEventProcess_t); i++)
    {
        if ((gEventProcess[i].event == event) && gEventProcess[i].processCb)
        {
            return gEventProcess[i].processCb(srv, event, arg);
        }
    }

    return E_UDS_SRV_RESP_CODE_SERVICE_NOT_SUPPORTED;
}

int main(int argc, char *argv[])
{
    LOG_INIT("demo");
    if (RET_OK != CreateMsgQueue(&respQueue))
    {
        LOG_Error("%s CreateMsgQueue resp fail.", __FUNCTION__);
        return -1;
    }


    DoipEntityCfg_t cfg = 
    {
        .activeType = 0xe2,
        .testerNum = 1,
        .testerLas = {0x1001,},
        .entityLa = 0x1002,
        .functionLa = 0xe400,
        .eid = {0x02, 0x47, 0x57, 0x4d, 0x00, 0x97},
        .gid = {0x02, 0x47, 0x57, 0x4d, 0x00, 0x99},
        .vin = {0x0,}      
    };

    if(RET_OK != DoipEntityInit(&entity, &cfg, UdsRequestProcess))
    {
        DestroyMsgQueue(respQueue);
        return -1;
    }

    UDSServer_t server;
    UDSTpHandle_t udsTphdl = 
    {
        .send = UdsSrvSend,
        .recv = UdsSrvRecv,
        .poll = UdsSrvPoll,
        .impl = NULL
    };

    UDSServerConfig_t udscfg = 
    {
        .EventProcessCb = UdsSrvProcess,
        .tp = &udsTphdl,
        .securityLevel1 = 0x03,
        .securityLevel2 = 0x00
    };

    if (UDSServerInit(&server, &udscfg) < 0)
    {
        LOG_Error("%s UDSServerInit fail.", __FUNCTION__);
        DoipEntityDeinit(entity);
        DestroyMsgQueue(respQueue);
        return RET_ERR;
    }

    while (1)
    {
        UDSServerPoll(&server);
        SleepMillis(1);
    }
    UDSServerDeInit(&server);
    DoipEntityDeinit(entity);
    DestroyMsgQueue(respQueue);
    return 0;
}