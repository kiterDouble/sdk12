#ifndef UDS_CALLBACK_ENTYR_H
#define UDS_CALLBACK_ENTYR_H

#include <stdint.h>
#include "sr_param.h"
#include "dtcRpcApi.h"
#include "udsTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * /oemdata/config/tboxNet.ini文件更新规则：
 * 1、收到诊断读写请求时，判断配置文件是否存在，不存在则创建配置文件（从oemapp中拷贝），并更新诊断参数到配置文件中
 * 2、如果存在，则读写配置文件
 */
#define CUSTOMER_NET_CONFIG_FILE_NAME "/oemdata/config/tboxNet.ini"
#define OEPAPP_NET_CONFIG_FILE_NAME   "/oemapp/config/net.ini"

#define UDS_DATA_TYPE_UINT            (0x01u)
#define UDS_DATA_TYPE_ASCII_HEX       (0x02u)

typedef enum
{
    ConvRetOk = 0,
    ConvRetParamErr = 1,
    ConvRetFormatErr = 2,
    ConvRetOverflow = 3
} ConvRet_t;

typedef struct
{
    UDS_DID_IDS_e did;
    DeviceParamEnum paramId;
} UdsDidToParamIdMapping_t;

int AdbClose(void);

int AdbOpen(void);

void UdsCallbackEntryInit(void);

int RunCommandAndGetOutput(const char *cmd, char *outBuf, size_t outSize);

int UdsGetParamIdFromMappingTable(uint16_t did, DeviceParamEnum *paramId);

UdsNrc_e UdsReadFromParamsDb(uint16_t did, uint8_t *buf, uint16_t *len);

UdsNrc_e UdsWriteToParamsDb(uint16_t did, const uint8_t *buf, uint16_t len);

UdsNrc_e UdsReadFromCanSignal(uint16_t did, uint8_t *buf, uint16_t *io_len);

UdsNrc_e UdsReadFromFile(uint16_t did, uint8_t *buf, uint16_t *io_len);

UdsNrc_e UdsWriteToFile(uint16_t did, const uint8_t *buf, uint16_t len);

UdsNrc_e UdsReadCommFunc(uint16_t did, uint8_t *buf, uint16_t *io_len);

UdsNrc_e UdsWriteCommFunc(uint16_t did, const uint8_t *buf, uint16_t len);

UdsNrc_e Uds31ServerCommFun(uint16_t did, uint8_t subFunc, const uint8_t *in, uint16_t inLen, uint8_t *out, uint16_t *io_len, gboolean *pReturnStatus);

bool ParseUint32FromAscii(const uint8_t *val_buf, size_t val_len, uint32_t minv, uint32_t maxv, uint32_t *out);

void UintToAsciiIntoParamValue(uint8_t dst[], size_t dstSize, uint32_t v);

ConvRet_t AsciiHexBytesToU32(const uint8_t *inBytes, uint32_t inLen, uint32_t *outValue);

#ifdef __cplusplus
}
#endif
#endif // UDS_CALLBACK_ENTYR_H