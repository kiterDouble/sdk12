#include "string.h"
#include "mpuHal.h"
#include "eolTestSyncWithCpu.h"
#include "logHal.h"
#include "FreeRTOS.h"
#include "task.h"

#define MPU_SYNC_EOL_TEST_AID 0x05
#define MPU_SYNC_EOL_TEST_MID 0x10
#define MPU_SYNC_EOL_TEST_SUBCMD_REQ 0x01
#define MPU_SYNC_EOL_TEST_SUBCMD_RESP 0x02

#define PASSTHROUGH_AID 0x05
#define PASSTHROUGH_MID 0x10
#define PASSTHROUGH_SUB_MCU_TO_MPU 0x22
#define PASSTHROUGH_SUB_MPU_TO_MCU 0x23

static int16_t g_mpuHandle = -1;
static uint8_t g_dataBuffer[300] = {0};
static MpuHalDataPack_t g_dataPack;
static uint8_t g_recvDataBuffer[300] = {0};

static uint8_t g_mpuDataBuffer[512] = {0};
static MpuHalDataPack_t g_mpuDataPack;

int16_t CanPassthrough_RequestAndGetResponse(const uint8_t *pUdsRequest, uint16_t reqLength,
                                             uint8_t *pUdsResponse, uint16_t *pRespLength)
{
    int16_t ret;
    uint16_t repeatCount = 0;
    uint8_t rxSuccess = 0;
    uint16_t maxRepeatCount = 2;

    if (pUdsRequest == NULL || pUdsResponse == NULL || pRespLength == NULL)
    {
        return -1;
    }

    if (reqLength >= 3 && pUdsRequest[0] == 0x2E && pUdsRequest[1] == 0xB2 && pUdsRequest[2] == 0x89)
    {
        maxRepeatCount = 10; 
    }
    // ---------------------------------------------------------

    g_mpuDataPack.aid = PASSTHROUGH_AID;
    g_mpuDataPack.mid = PASSTHROUGH_MID;
    g_mpuDataPack.subcommand = PASSTHROUGH_SUB_MCU_TO_MPU; // 0x22
    memcpy(g_mpuDataBuffer, pUdsRequest, reqLength);
    g_mpuDataPack.pDataBuffer = g_mpuDataBuffer;
    g_mpuDataPack.dataLength = reqLength;

    g_dataPack.pDataBuffer = g_dataBuffer;
    g_dataPack.dataBufferSize = sizeof(g_dataBuffer);
    while (MpuHalReceive(g_mpuHandle, &g_dataPack, 0) == 0)
    {
    }

    MpuHalTransmit(g_mpuHandle, &g_mpuDataPack, MPU_HAL_UART_MODE);

    g_dataPack.pDataBuffer = g_dataBuffer;
    g_dataPack.dataBufferSize = sizeof(g_dataBuffer);

    do
    {
        ret = MpuHalReceive(g_mpuHandle, &g_dataPack, 100);

        if (ret == 0)
        {
            uint8_t subCommand = g_dataPack.subcommand & 0x7F;
            if (g_dataPack.aid == PASSTHROUGH_AID &&
                g_dataPack.mid == PASSTHROUGH_MID &&
                subCommand == PASSTHROUGH_SUB_MPU_TO_MCU &&
                g_mpuDataBuffer[2] == g_dataBuffer[2] &&
                g_mpuDataBuffer[1] == g_dataBuffer[1])
            {
                memcpy(pUdsResponse, g_dataPack.pDataBuffer, g_dataPack.dataLength);
                *pRespLength = g_dataPack.dataLength;
                rxSuccess = 1;
                break;
            }
            else
            {
                // DO NOT
            }
        }
        else
        {
            repeatCount++;
            if (repeatCount >= maxRepeatCount)
            {
                break;
            }

            MpuHalTransmit(g_mpuHandle, &g_mpuDataPack, MPU_HAL_UART_MODE);
            vTaskDelay(50);
        }

    } while (1);

    if (rxSuccess)
    {
        return 0; // 成功
    }
    else
    {
        return -1; // 失败
    }
}
void EolTestSyncWithCpuInit(void)
{
    MpuHalFilter_t filter;
    g_mpuHandle = MpuHalOpen();

    filter.aid = PASSTHROUGH_AID;
    filter.midMin = PASSTHROUGH_MID;
    filter.midMax = PASSTHROUGH_MID;

    MpuHalSetRxFilter(g_mpuHandle, &filter);
    MpuHalSetRxBuffer(g_mpuHandle, g_recvDataBuffer, sizeof(g_recvDataBuffer));

    return;
}
#if 0
static void PackRequestControlCpuInterface(uint8_t controlItem, uint8_t *pControlData, uint16_t ControlDataLength)
{
    uint16_t i = 0;
    uint16_t lenth = 0;

    g_mpuDataPack.aid = MPU_SYNC_EOL_TEST_AID;
    g_mpuDataPack.mid = MPU_SYNC_EOL_TEST_MID;
    g_mpuDataPack.subcommand = MPU_SYNC_EOL_TEST_SUBCMD_REQ;

    memset(g_mpuDataBuffer, 0, sizeof(g_mpuDataBuffer));

    g_mpuDataPack.dataBufferSize = sizeof(g_mpuDataBuffer);
    g_mpuDataBuffer[0] = controlItem;
    g_mpuDataBuffer[1] = ControlDataLength & 0xFF;
    for (i = 0; i < ControlDataLength; i++)
    {
        g_mpuDataBuffer[2 + i] = pControlData[i];
    }

    lenth = ControlDataLength + 2;
    g_mpuDataPack.pDataBuffer = g_mpuDataBuffer;
    g_mpuDataPack.dataLength = lenth;

    return;
}
#endif
int16_t EolTestSyncWithCpuTransmit(uint8_t item, uint8_t *pData, uint16_t length)
{
#if 0
    int16_t ret;
    uint16_t repeatCount;
    uint8_t rxSucess;
    ret = 0;
    repeatCount = 0;
    rxSucess = 0;
    // if(CheckControlItemIsValid(item)!=0)
    //{
    //     return -1;
    // }
    g_dataPack.pDataBuffer = g_dataBuffer;
    g_dataPack.dataBufferSize = sizeof(g_dataBuffer);
    do
    {
        if (repeatCount >= 1)
        {
            ret = 1;
            break;
        }
        // UartDriverHalReceiveWait(m_uartHandle,m_uartRxData,&rxLength,0);
        MpuHalReceive(g_mpuHandle, &g_dataPack, 0);
        PackRequestControlCpuInterface(item, pData, length);
        MpuHalTransmit(g_mpuHandle, &g_mpuDataPack, MPU_HAL_UART_MODE);
        while (1)
        {
            ret = MpuHalReceive(g_mpuHandle, &g_dataPack, 150);
            if (ret == 0)
            {
                uint8_t subCommand;
                uint8_t mid;
                mid = g_dataPack.mid;
                subCommand = g_dataPack.subcommand & 0x7F;
                if (MPU_SYNC_EOL_TEST_MID == mid)
                {
                    if ((MPU_SYNC_EOL_TEST_SUBCMD_RESP == subCommand) && (g_dataPack.pDataBuffer[1] == item))
                    {
                        // check return value is success
                        if (g_dataPack.pDataBuffer[0] != 0)
                        {
                            ret = 1; // operation failed
                        }
                        rxSucess = 1;
                        break;
                    }
                }
            }
            else
            {
                repeatCount++;
                break;
            }
        }
        if (rxSucess)
        {
            break;
        }
    } while (1);

    if (rxSucess)
    {
        return 0;
    }
    else
    {
        return -1;
    }
#endif
    return -1;
}
#if 0
static void PackRequestGetCpuInfo(uint8_t GetInfoItem)
{

    g_mpuDataPack.aid = MPU_SYNC_EOL_TEST_AID;
    g_mpuDataPack.mid = MPU_SYNC_EOL_TEST_MID;
    g_mpuDataPack.subcommand = MPU_SYNC_EOL_TEST_SUBCMD_REQ;

    memset(g_mpuDataBuffer, 0, sizeof(g_mpuDataBuffer));

    g_mpuDataPack.dataBufferSize = sizeof(g_mpuDataBuffer);
    g_mpuDataBuffer[0] = GetInfoItem;

    g_mpuDataPack.pDataBuffer = g_mpuDataBuffer;
    g_mpuDataPack.dataLength = 1;

    return;
}
#endif
int16_t EolTestSyncWithCpuRecv(uint8_t item, uint8_t *pData, uint16_t *pLength)
{
    return 0;
#if 0
    int16_t ret;
    uint16_t repeatCount;
    uint8_t rxSucess;
    ret = 0;
    repeatCount = 0;
    rxSucess = 0;
    // if(CheckGetItemIsValid(item)!=0)
    //{
    //     return -1;
    // }
    g_dataPack.pDataBuffer = g_dataBuffer;
    g_dataPack.dataBufferSize = sizeof(g_dataBuffer);
    do
    {
        if (repeatCount >= 3)
        {
            ret = 1;
            break;
        }
        // clear uart buffer
        while (1)
        {
            int16_t ret;
            ret = MpuHalReceive(g_mpuHandle, &g_dataPack, 0);
            if (ret != 0)
            {
                break;
            }
        }

        PackRequestGetCpuInfo(item);
        // UartDriverHalTransmit(m_uartHandle,m_uartTxData,txLength);
        MpuHalTransmit(g_mpuHandle, &g_mpuDataPack, MPU_HAL_UART_MODE);
        while (1)
        {
            ret = MpuHalReceive(g_mpuHandle, &g_dataPack, 150);
            if (ret == 0)
            {
                uint8_t subCommand;
                uint8_t mid;
                mid = g_dataPack.mid;
                subCommand = g_dataPack.subcommand & 0x7F;
                if (MPU_SYNC_EOL_TEST_MID == mid)
                {
                    if (MPU_SYNC_EOL_TEST_SUBCMD_RESP == subCommand)
                    {
                        // check return value is success
                        if ((g_dataPack.pDataBuffer[1] == item))
                        {
                            uint16_t length;
                            length = g_dataPack.pDataBuffer[2];
                            for (uint16_t i = 0; i < length; i++)
                            {
                                pData[i] = g_dataPack.pDataBuffer[3 + i];
                            }
                            *pLength = length;
                            rxSucess = 1;
                            break;
                        }
                    }
                }
            }
            else
            {
                repeatCount++;
                break;
            }
        }
        if (rxSucess)
        {
            break;
        }
    } while (1);

    if (rxSucess)
    {
        return 0;
    }
    else
    {
        return -1;
    }
#endif
}

// 0xB262_cxl
int16_t EolTestSyncWithCpuGetPkiStatus(uint8_t *pPkiStatus)
{
    uint8_t response_byte = 0;
    uint16_t response_length = 0;
    int16_t ret;

    if (pPkiStatus == NULL)
    {
        return -1;
    }
    ret = EolTestSyncWithCpuRecv(SYNC_CPU_GET_INFO_ITEM_PKI_STATUS, &response_byte, &response_length);

    if (ret == 0 && response_length >= 1)
    {
        *pPkiStatus = response_byte;
        return 0;
    }

    return -1;
}