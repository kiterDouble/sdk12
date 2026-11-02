#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "uds_tp.h"
#include "uds_service.h"
#include "ota_master_log.h"
#include "ota_master_error_code.h"

typedef struct {
    UdsTpHandle_t tpHdl;
    uint32_t funcId;
} UdsSrvPriv_t;

int UdsSrvInit(UdsSrvHandle_t *hdl, uint32_t funcId)
{
    UdsSrvPriv_t *priv;
    int32_t ret = OTA_RET_ERR;

    if (!hdl) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return ret;
    }

    priv = (UdsSrvPriv_t *)calloc(1, sizeof(UdsSrvPriv_t));

    if (!priv) {
        OTA_LOG_E("%s calloc fail.\n", __FUNCTION__);
        return ret;
    }

    if (OTA_RET_OK != UdsTpInit(&priv->tpHdl)) {
        OTA_LOG_E("%s UdsTpInit fail.\n", __FUNCTION__);
        return ret;
    }
    priv->funcId = funcId;
    *hdl = (UdsSrvHandle_t)priv;
    return OTA_RET_OK;
}

int DiagnosticSessionControlReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, bool isNeedRes,
                                UdsSessionSub_t subfun) // 0x10
{
    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return UDS_SERVICE_RESULT_ERROR_PARAMETER;
    }

    UdsTpData_t uData;
    int32_t result;

    memset(&uData, 0x0, sizeof(UdsTpData_t));
    uData.canId = canId;
    uData.resId = resId;
    uData.canChannel = channel;
    uData.tx[0] = 0x10;
    uData.tx[1] = subfun;
    uData.txLen = 0x02;

    if (!isNeedRes) {
        result = UdsTpSeviceExecNoRes(priv->tpHdl, uData);
        if (result != OTA_RET_OK) {
            return UDS_SERVICE_RESULT_ERROR_TP;
        }
        return UDS_SERVICE_RESULT_OK;
    }

    result = UdsTpSeviceExec(priv->tpHdl, uData);

    if (result != OTA_RET_OK) {
        return UDS_SERVICE_RESULT_ERROR_TP;
    }

    if (uData.rxLen < 2) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[0] == 0x7F) // negative response
    {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_NEGATIVE;
    }
    if (uData.rx[0] != (0x50)) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[1] != subfun) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    return UDS_SERVICE_RESULT_OK;
}

int EcuResetReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel,
                UdsEcuResetSub_t subfun) // 0x11
{
    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return UDS_SERVICE_RESULT_ERROR_PARAMETER;
    }

    UdsTpData_t uData;
    int32_t result;

    memset(&uData, 0x0, sizeof(UdsTpData_t));
    uData.canId = canId;
    uData.resId = resId;
    uData.canChannel = channel;
    uData.tx[0] = 0x11;
    uData.tx[1] = subfun;
    uData.txLen = 0x02;

    result = UdsTpSeviceExec(priv->tpHdl, uData);
    if (result != OTA_RET_OK) {
        return UDS_SERVICE_RESULT_ERROR_TP;
    }

    if (uData.rxLen < 2) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }

    if (uData.rx[0] == 0x7F) // negative response
    {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_NEGATIVE;
    }
    if (uData.rx[0] != 0x51) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[1] != uData.tx[1]) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }

    return UDS_SERVICE_RESULT_OK;
}

int ClearDtcReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel,
                uint32_t clearFlag) // 0x14, function id
{
    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return UDS_SERVICE_RESULT_ERROR_PARAMETER;
    }

    UdsTpData_t uData;
    int32_t result;

    memset(&uData, 0x0, sizeof(UdsTpData_t));
    uData.canId = canId;
    uData.resId = resId;
    uData.canChannel = channel;
    uData.tx[0] = 0x14;
    uData.tx[1] = (clearFlag >> 16) & 0xFF;
    uData.tx[2] = (clearFlag >> 8) & 0xFF;
    uData.tx[3] = (clearFlag >> 0) & 0xFF;
    uData.txLen = 0x04;

    if (canId == priv->funcId) {
        result = UdsTpSeviceExecNoRes(priv->tpHdl, uData);
        if (result != OTA_RET_OK) {
            return UDS_SERVICE_RESULT_ERROR_TP;
        }
        return UDS_SERVICE_RESULT_OK;
    }

    result = UdsTpSeviceExec(priv->tpHdl, uData);
    if (result != OTA_RET_OK) {
        return UDS_SERVICE_RESULT_ERROR_TP;
    }
    if (uData.rxLen < 1) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[0] == 0x7F) // negative response
    {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_NEGATIVE;
    }
    if (uData.rx[0] != (0x14 + 0x40)) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }

    return UDS_SERVICE_RESULT_OK;
}

int ReadDataByIdentifierReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint32_t did,
                            uint8_t data[],
                            uint32_t *len) // 0x22
{

    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;

    if (!priv || !data || !len) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return UDS_SERVICE_RESULT_ERROR_PARAMETER;
    }

    UdsTpData_t uData;
    int32_t result;

    memset(&uData, 0x0, sizeof(UdsTpData_t));
    uData.canId = canId;
    uData.resId = resId;
    uData.canChannel = channel;
    uData.tx[0] = 0x22;
    uData.tx[1] = did >> 8;
    uData.tx[2] = did;
    uData.txLen = 0x03;

    result = UdsTpSeviceExec(priv->tpHdl, uData);
    if (result != OTA_RET_OK) {
        return UDS_SERVICE_RESULT_ERROR_TP;
    }

    if (uData.rxLen < 3) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[0] == 0x7F) // negative response
    {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_NEGATIVE;
    }
    if (uData.rx[0] != (0x22 + 0x40)) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[1] != uData.tx[1]) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[2] != uData.tx[2]) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }

    uint32_t dataLength = uData.rxLen - 3;

    if (dataLength > 0) {
        memcpy(data, &uData.rx[3], dataLength);
    }

    *len = dataLength;
    return UDS_SERVICE_RESULT_OK;
}

int SecurityAccessReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint8_t secLevel,
                      uint8_t data[],
                      uint32_t *len) // 0x27
{
    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return UDS_SERVICE_RESULT_ERROR_PARAMETER;
    }

    UdsTpData_t uData;
    int32_t result;

    memset(&uData, 0x0, sizeof(UdsTpData_t));
    uData.canId = canId;
    uData.resId = resId;
    uData.canChannel = channel;
    uData.tx[0] = 0x27;
    uData.tx[1] = secLevel;
    uData.txLen = 0x02;
    if (*len > 0) {
        memcpy(&uData.tx[2], data, *len);
        uData.txLen += *len;
    }
    result = UdsTpSeviceExec(priv->tpHdl, uData);
    if (result != OTA_RET_OK) {
        return UDS_SERVICE_RESULT_ERROR_TP;
    }

    if (uData.rxLen < 2) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[0] == 0x7F) // negative response
    {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_NEGATIVE;
    }
    if (uData.rx[0] != (0x27 + 0x40)) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[1] != secLevel) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    uint32_t seedLength = uData.rxLen - 2;

    if (seedLength > 0) {
        memcpy(data, &uData.rx[2], seedLength);
    }

    *len = seedLength;
    return UDS_SERVICE_RESULT_OK;
}

static int CommunicationControlReqInternal(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel,
                                          uint8_t controlType, uint8_t commnicationType, uint32_t resTimeoutMs) // 0x28
{
    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return UDS_SERVICE_RESULT_ERROR_PARAMETER;
    }

    UdsTpData_t uData;
    int32_t result;

    memset(&uData, 0x0, sizeof(UdsTpData_t));
    uData.canId = canId;
    uData.resId = resId;
    uData.canChannel = channel;
    uData.tx[0] = 0x28;
    uData.tx[1] = controlType;
    uData.tx[2] = commnicationType;
    uData.txLen = 0x03;

    // 未指定超时时, 功能寻址按"只发不等响应"处理; 显式指定超时(>0)则即使功能寻址也要等ECU应答
    if ((resTimeoutMs == 0) && (canId == priv->funcId)) {
        result = UdsTpSeviceExecNoRes(priv->tpHdl, uData);
        if (result != OTA_RET_OK) {
            return UDS_SERVICE_RESULT_ERROR_TP;
        }
        return UDS_SERVICE_RESULT_OK;
    }

    // 可选的单次等响应超时(ms)
    if (resTimeoutMs > 0) {
        UdsTpSetResTimeout(priv->tpHdl, resTimeoutMs);
    }
    result = UdsTpSeviceExec(priv->tpHdl, uData);
    if (resTimeoutMs > 0) {
        UdsTpSetResTimeout(priv->tpHdl, 0); // 恢复默认超时
    }
    if (result != OTA_RET_OK) {
        return UDS_SERVICE_RESULT_ERROR_TP;
    }

    if (uData.rxLen < 2) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }

    if (uData.rx[0] == 0x7F) // negative response
    {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_NEGATIVE;
    }

    if (uData.rx[0] != (0x28 + 0x40)) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }

    if (uData.rx[1] != uData.tx[1]) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }

    return UDS_SERVICE_RESULT_OK;
}

int CommunicationControlReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint8_t controlType,
                            uint8_t commnicationType) // 0x28, function id, maybe doip not used.
{
    return CommunicationControlReqInternal(hdl, canId, resId, channel, controlType, commnicationType, 0);
}

int CommunicationControlReqTimeout(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel,
                                   uint8_t controlType, uint8_t commnicationType, uint32_t resTimeoutMs)
{
    return CommunicationControlReqInternal(hdl, canId, resId, channel, controlType, commnicationType, resTimeoutMs);
}

int WriteDataByIdentifierReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint32_t did,
                             uint8_t data[],
                             uint32_t len) // 0x2E
{
    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;

    if (!priv || !data || !len) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return UDS_SERVICE_RESULT_ERROR_PARAMETER;
    }

    UdsTpData_t uData;
    int32_t result;

    memset(&uData, 0x0, sizeof(UdsTpData_t));
    uData.canId = canId;
    uData.resId = resId;
    uData.canChannel = channel;
    uData.tx[0] = 0x2E;
    uData.tx[1] = (did >> 8) & 0xFF; // did H
    uData.tx[2] = did & 0xFF;        // did L
    uData.txLen = 0x03;
    memcpy(&uData.tx[3], data, len);
    uData.txLen += len;

    result = UdsTpSeviceExec(priv->tpHdl, uData);
    if (result != OTA_RET_OK) {
        return UDS_SERVICE_RESULT_ERROR_TP;
    }

    if (uData.rxLen < 3) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[0] == 0x7F) // negative response
    {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_NEGATIVE;
    }
    if (uData.rx[0] != (0x2E + 0x40)) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[1] != uData.tx[1]) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[2] != uData.tx[2]) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    return UDS_SERVICE_RESULT_OK;
}

int RoutineControlReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, UdsRoutineCtrlSub_t subfub,
                      uint32_t routineId, uint8_t data[],
                      uint32_t *len) // 0x31
{
    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;

    if (!priv || !data || !len) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return UDS_SERVICE_RESULT_ERROR_PARAMETER;
    }

    UdsTpData_t uData;
    int32_t result;

    memset(&uData, 0x0, sizeof(UdsTpData_t));
    uData.canId = canId;
    uData.resId = resId;
    uData.canChannel = channel;
    uData.tx[0] = 0x31;
    uData.tx[1] = subfub;
    uData.tx[2] = routineId >> 8; // did H
    uData.tx[3] = routineId;      // did L
    uData.txLen = 4;
    if (*len > 0) {
        memcpy(&uData.tx[4], data, *len);
        uData.txLen += *len;
    }

    result = UdsTpSeviceExec(priv->tpHdl, uData);
    if (result != OTA_RET_OK) {
        return UDS_SERVICE_RESULT_ERROR_TP;
    }

    if (uData.rxLen < 3) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[0] == 0x7F) // negative response
    {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_NEGATIVE;
    }

    if (uData.rxLen < 4) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }

    if (uData.rx[0] != (0x31 + 0x40)) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[1] != uData.tx[1]) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[2] != uData.tx[2]) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[3] != uData.tx[3]) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    uint32_t reslen = uData.rxLen - 4;
    if (reslen > 0) {
        memcpy(data, &uData.rx[4], reslen);
    } else {
        // 部分ECU(如VCU)不回routineStatusRecord，响应只有4字节(71 xx xx xx)，视为成功(0x00)
        data[0] = 0x00;
        reslen = 1;
    }

    *len = reslen;

    return UDS_SERVICE_RESULT_OK;
}

int RequestDownloadReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint8_t dataFormatId,
                       uint8_t addressAndLengthFormatId, const uint8_t memaddr[], const uint8_t memsize[],
                       uint32_t *pMaxNumberOfBlockLength) // 0x34

{
    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;

    if (!priv || !memaddr || !memsize || !pMaxNumberOfBlockLength) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return UDS_SERVICE_RESULT_ERROR_PARAMETER;
    }

    UdsTpData_t uData;
    int32_t result;
    uint8_t addressFormat;
    uint8_t sizeFormat;
    uint8_t i = 0;

    memset(&uData, 0x0, sizeof(UdsTpData_t));
    uData.canId = canId;
    uData.resId = resId;
    uData.canChannel = channel;
    uData.tx[0] = 0x34;
    uData.tx[1] = dataFormatId;
    uData.tx[2] = addressAndLengthFormatId;
    uData.txLen = 0x03;

    addressFormat = addressAndLengthFormatId & 0x0F;
    sizeFormat = (addressAndLengthFormatId >> 4) & 0x0F;

    for (i = 0; i < addressFormat; i++) {
        uData.tx[3 + i] = memaddr[i];
    }
    for (i = 0; i < sizeFormat; i++) {
        uData.tx[3 + addressFormat + i] = memsize[i];
    }

    uData.txLen += addressFormat + sizeFormat;
    result = UdsTpSeviceExec(priv->tpHdl, uData);
    if (result != OTA_RET_OK) {
        return UDS_SERVICE_RESULT_ERROR_TP;
    }

    if (uData.rxLen < 3) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[0] == 0x7F) // negative response
    {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_NEGATIVE;
    }
    if (uData.rx[0] != (0x34 + 0x40)) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }

    uint8_t dataFormatIdentifier = uData.rx[1];
    uint32_t expectedLength = 3;
    if (dataFormatIdentifier == 0x20) {
        expectedLength = 4;
    } else if (dataFormatIdentifier == 0x30) {
        expectedLength = 5;
    } else if (dataFormatIdentifier == 0x40) {
        expectedLength = 6;
    }

    if (uData.rxLen < expectedLength) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }

    switch (dataFormatIdentifier) {
        case 0x10:
            *pMaxNumberOfBlockLength = uData.rx[2];
            break;
        case 0x20:
            *pMaxNumberOfBlockLength = (uData.rx[2] << 8) | uData.rx[3];
            break;
        case 0x30:
            *pMaxNumberOfBlockLength = (uData.rx[2] << 16) | (uData.rx[3] << 8) | uData.rx[4];
            break;
        case 0x40:
            *pMaxNumberOfBlockLength = (uData.rx[2] << 24) | (uData.rx[3] << 16) | (uData.rx[4] << 8) | uData.rx[5];
            break;
        default:
            return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    return UDS_SERVICE_RESULT_OK;
}

int TransferDataReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint8_t blockSequenceCounter,
                    const uint8_t data[],
                    uint32_t len, uint8_t *nrc) // 0x36
{
    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;

    if (nrc) {
        *nrc = 0;
    }

    if (!priv || !data || !len) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return UDS_SERVICE_RESULT_ERROR_PARAMETER;
    }

    UdsTpData_t uData;
    int32_t result;

    memset(&uData, 0x0, sizeof(UdsTpData_t));
    uData.canId = canId;
    uData.resId = resId;
    uData.canChannel = channel;
    uData.tx[0] = 0x36;
    uData.tx[1] = blockSequenceCounter;
    uData.txLen = 0x02;
    memcpy(&uData.tx[2], data, len);
    uData.txLen += len;
    result = UdsTpSeviceExec(priv->tpHdl, uData);
    if (result != OTA_RET_OK) {
        return UDS_SERVICE_RESULT_ERROR_TP;
    }

    if (uData.rxLen < 2) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[0] == 0x7F) // negative response
    {
        if (nrc && (uData.rxLen >= 3)) {
            *nrc = uData.rx[2];
        }
        return UDS_SERVICE_RESULT_ERROR_RESPONE_NEGATIVE;
    }
    if (uData.rx[0] != (0x36 + 0x40)) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[1] != uData.tx[1]) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    return UDS_SERVICE_RESULT_OK;
}

int RequestTransferExitReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint8_t data[],
                           uint32_t *len) // 0x37
{
    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return UDS_SERVICE_RESULT_ERROR_PARAMETER;
    }

    UdsTpData_t uData;
    int32_t result;

    memset(&uData, 0x0, sizeof(UdsTpData_t));
    uData.canId = canId;
    uData.resId = resId;
    uData.canChannel = channel;
    uData.tx[0] = 0x37;
    uData.txLen = 0x01;
    if (*len > 0) {
        memcpy(&uData.tx[1], data, *len);
        uData.txLen += *len;
    }
    result = UdsTpSeviceExec(priv->tpHdl, uData);
    if (result != OTA_RET_OK) {
        return UDS_SERVICE_RESULT_ERROR_TP;
    }

    if (uData.rxLen < 1) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[0] == 0x7F) // negative response
    {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_NEGATIVE;
    }
    if (uData.rx[0] != (0x37 + 0x40)) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }

    if ((uData.rxLen - 1) > 0) {
        memcpy(data, (uData.rx + 1), (uData.rxLen - 1));
    }
    *len = (uData.rxLen - 1);
    return UDS_SERVICE_RESULT_OK;
}

int TesterPresentReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, bool suppressResponse) // 0x3E
{
    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return UDS_SERVICE_RESULT_ERROR_PARAMETER;
    }

    UdsTpData_t uData;
    int32_t result;

    memset(&uData, 0x0, sizeof(UdsTpData_t));
    uData.canId = canId;
    uData.resId = resId;
    uData.canChannel = channel;
    uData.tx[0] = 0x3e;
    // suppressResponse: true=0x80(不需要响应), false=0x00(需要响应)
    uData.tx[1] = suppressResponse ? 0x80 : 0x00;
    uData.txLen = 0x02;

    if (suppressResponse) {
        // 不需要响应，使用无响应执行
        result = UdsTpSeviceExecNoRes(priv->tpHdl, uData);
        if (result != OTA_RET_OK) {
            return UDS_SERVICE_RESULT_ERROR_TP;
        }
    } else {
        // 需要响应，使用正常执行并等待响应
        result = UdsTpSeviceExec(priv->tpHdl, uData);
        if (result != OTA_RET_OK) {
            return UDS_SERVICE_RESULT_ERROR_TP;
        }
        // 检查响应: 期望 7E xx
        if (uData.rxLen < 2 || uData.rx[0] != 0x7e) {
            OTA_LOG_E("%s response format error, rx[0]=0x%02X, rxLen=%d\n", __FUNCTION__, uData.rx[0], uData.rxLen);
            return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
        }
    }
    return UDS_SERVICE_RESULT_OK;
}

static int ControlDTCSettingReqInternal(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel,
                                        UdsCtrlDtcSettingSub_t subfub, uint32_t resTimeoutMs) // 0x85
{
    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return UDS_SERVICE_RESULT_ERROR_PARAMETER;
    }

    UdsTpData_t uData;
    int32_t result;

    memset(&uData, 0x0, sizeof(UdsTpData_t));
    uData.canId = canId;
    uData.resId = resId;
    uData.canChannel = channel;
    uData.tx[0] = 0x85;
    uData.tx[1] = subfub;
    uData.txLen = 0x02;

    // 未指定超时时, 功能寻址按"只发不等响应"处理; 显式指定超时(>0)则即使功能寻址也要等ECU应答
    if ((resTimeoutMs == 0) && (canId == priv->funcId)) {
        result = UdsTpSeviceExecNoRes(priv->tpHdl, uData);
        if (result != OTA_RET_OK) {
            return UDS_SERVICE_RESULT_ERROR_TP;
        }
        return UDS_SERVICE_RESULT_OK;
    }

    // 可选的单次等响应超时(ms)
    if (resTimeoutMs > 0) {
        UdsTpSetResTimeout(priv->tpHdl, resTimeoutMs);
    }
    result = UdsTpSeviceExec(priv->tpHdl, uData);
    if (resTimeoutMs > 0) {
        UdsTpSetResTimeout(priv->tpHdl, 0); // 恢复默认超时
    }
    if (result != OTA_RET_OK) {
        return UDS_SERVICE_RESULT_ERROR_TP;
    }
    if (uData.rxLen < 2) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[0] == 0x7F) // negative response
    {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_NEGATIVE;
    }
    if (uData.rx[0] != (0x85 + 0x40)) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[1] != subfub) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }

    return UDS_SERVICE_RESULT_OK;
}

int ControlDTCSettingReq(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel,
                         UdsCtrlDtcSettingSub_t subfub) // 0x85, function id
{
    return ControlDTCSettingReqInternal(hdl, canId, resId, channel, subfub, 0);
}

int ControlDTCSettingReqTimeout(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel,
                                UdsCtrlDtcSettingSub_t subfub, uint32_t resTimeoutMs)
{
    return ControlDTCSettingReqInternal(hdl, canId, resId, channel, subfub, resTimeoutMs);
}

int ReadDTCInfoByStatusMask(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint8_t statusMask,
                            uint32_t *len,
                            uint8_t dtcCode[]) // 0x19 02 read DTC code
{
    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return UDS_SERVICE_RESULT_ERROR_PARAMETER;
    }

    UdsTpData_t uData;
    int32_t result;

    memset(&uData, 0x0, sizeof(UdsTpData_t));
    uData.canId = canId;
    uData.resId = resId;
    uData.canChannel = channel;
    uData.tx[0] = 0x19;
    uData.tx[1] = 0x02;
    uData.tx[2] = statusMask;
    uData.txLen = 0x03;

    result = UdsTpSeviceExec(priv->tpHdl, uData);

    if (result != OTA_RET_OK) {
        return UDS_SERVICE_RESULT_ERROR_TP;
    }
    if (uData.rxLen < 3) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[0] == 0x7F) // negative response
    {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_NEGATIVE;
    }
    if (uData.rx[0] != (0x19 + 0x40)) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[1] != 0x02) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[2] < statusMask) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (((uData.rxLen - 3) / 4) > 0) {
        memcpy(dtcCode, (uData.rx + 3), (uData.rxLen - 3));
    }
    *len = (uData.rxLen - 3);

    return UDS_SERVICE_RESULT_OK;
}

int ReadReadDataByIdentifier(UdsSrvHandle_t hdl, uint32_t canId, uint32_t resId, uint8_t channel, uint16_t did,
                             uint32_t *len,
                             uint8_t didInfo[]) // 0x22 read DataByIdentifier
{
    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return UDS_SERVICE_RESULT_ERROR_PARAMETER;
    }

    UdsTpData_t uData;
    int32_t result;

    memset(&uData, 0x0, sizeof(UdsTpData_t));
    uData.canId = canId;
    uData.resId = resId;
    uData.canChannel = channel;
    uData.tx[0] = 0x22;
    uData.tx[1] = (did >> 8) & 0xFF;
    uData.tx[2] = (did >> 0) & 0xFF;
    uData.txLen = 0x03;

    result = UdsTpSeviceExec(priv->tpHdl, uData);
    if (result != OTA_RET_OK) {
        return UDS_SERVICE_RESULT_ERROR_TP;
    }
    if (uData.rxLen < 3) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (uData.rx[0] == 0x7F) // negative response
    {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_NEGATIVE;
    }
    if (uData.rx[0] != (0x22 + 0x40)) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if (((uData.rx[1] << 8) | uData.rx[2]) != did) {
        return UDS_SERVICE_RESULT_ERROR_RESPONE_FORMAT;
    }
    if ((uData.rxLen - 3) > 0) {
        memcpy(didInfo, (uData.rx + 3), (uData.rxLen - 3));
    }
    *len = (uData.rxLen - 3);

    return UDS_SERVICE_RESULT_OK;
}

int UdsSrvDeinit(UdsSrvHandle_t hdl)
{
    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;
    int32_t ret = OTA_RET_ERR;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return ret;
    }

    UdsTpDeinit(priv->tpHdl);
    free(priv);
    return OTA_RET_OK;
}

int UdsSrvClearQueue(UdsSrvHandle_t hdl)
{
    UdsSrvPriv_t *priv = (UdsSrvPriv_t *)hdl;

    if (!priv) {
        OTA_LOG_E("%s invalid param.\n", __FUNCTION__);
        return OTA_RET_ERR;
    }

    return UdsTpClearQueue(priv->tpHdl);
}

