/*************************************************
 Copyright (C) 2025 SiRun (WuXi). All rights reserved.
 File Name: canMsgDynamicConfigure.c
 Author:
 Created Time:
 Description: Dynamic CAN ID subscription sync between MCU and SOC.
              SOC sends per-channel CAN ID subscription tables to MCU
              via the MPU (SPI/UART) protocol. MCU requests in a
              round-robin across 4 CAN channels.
 Others:
 *************************************************/

/****************************** include ***************************************/
#include "canMsgDynamicConfigure.h"
#include "canMsgToCpu.h"
#include "mpuHal.h"

#include <string.h>

/****************************** Macro Definitions ******************************/
#define CPU_RESPONSE_TIME_OUT_TIME (2000U)
#define SUBSCRIBE_INDEX_NUM_PER_BATCH (10U)
#define SUBSCRIBE_CHANNEL_NUM (4U)

/****************************** Type Definitions ******************************/
typedef enum {
    E_SYNC_STATE_INIT = 0,
    E_SYNC_STATE_GET_CPU_REQUEST,
    E_SYNC_STATE_GET_CPU_WAIT_RESPONSE,
    E_SYNC_STATE_WAIT_CPU_UPDATE,
} CanMsgConfigureSyncState_e;

/****************************** Global Variables ******************************/
static int16_t g_canMsgConfigureSyncMpuHandle = -1;
static CanMsgConfigureSyncState_e g_canConfigureSyncState = E_SYNC_STATE_INIT;
static uint16_t g_canMsgIndex = 0U;
static uint32_t g_timeOutCount = 0U;
static const uint16_t g_indexNumberMaxOneTime = SUBSCRIBE_INDEX_NUM_PER_BATCH;
static uint8_t g_syncChannel = 0U; /* current channel being synced */

/****************************** Function Declarations *************************/
static int16_t PackGetConfigureFromCpuRequest(uint8_t channel, uint16_t startIndex,
                                              uint16_t indexNum, uint8_t *pDataOut,
                                              uint16_t *pLengthOut);
static int16_t ParseProtocolDataToCanConfigureBuffer(uint8_t *pProtocalData,
                                                     uint16_t protocalLength,
                                                     uint8_t *pOutChannel);

/****************************** Private Function Implementations ***************/

/* Pack a subscription request for one channel.
 * Format: [1B channel][2B startIndex (big-endian)][2B indexNum (big-endian)] */
static int16_t PackGetConfigureFromCpuRequest(uint8_t channel, uint16_t startIndex,
                                              uint16_t indexNum, uint8_t *pDataOut,
                                              uint16_t *pLengthOut)
{
    pDataOut[0] = channel;
    pDataOut[1] = (uint8_t)((startIndex >> 8) & 0xFF);
    pDataOut[2] = (uint8_t)(startIndex & 0xFF);
    pDataOut[3] = (uint8_t)((indexNum >> 8) & 0xFF);
    pDataOut[4] = (uint8_t)(indexNum & 0xFF);
    *pLengthOut = 5;
    return 0;
}

/* Parse SOC reply containing subscription entries for one channel.
 * Format: [1B channel][1B reserved][2B elementNum (BE)][N x 5B entries]
 * Each entry: [4B canId (BE)][1B msgRepeatNumber] */
static int16_t ParseProtocolDataToCanConfigureBuffer(uint8_t *pProtocalData,
                                                     uint16_t protocalLength,
                                                     uint8_t *pOutChannel)
{
    int16_t rxElementNum = -1;
    CanMsgRxConfigure_t msgConfigureData;
    uint8_t canChannel;
    int16_t i;

    (void)protocalLength; /* length already validated by caller */

    canChannel = pProtocalData[0];
    *pOutChannel = canChannel;

    rxElementNum = (int16_t)((pProtocalData[2] << 8) + pProtocalData[3]);

    for (i = 0; i < rxElementNum; i++) {
        msgConfigureData.canId = ((uint32_t)pProtocalData[5 + i * 5] << 24) +
                                 ((uint32_t)pProtocalData[6 + i * 5] << 16) +
                                 ((uint32_t)pProtocalData[7 + i * 5] << 8) +
                                 ((uint32_t)pProtocalData[8 + i * 5]);
        msgConfigureData.msgRepeatNumber = pProtocalData[4 + i * 5];
        CanMsgConfigureBufferAdd(canChannel, &msgConfigureData);
    }

    return rxElementNum;
}

static void ConfigureFromCpuSyncCycleProcess(MpuHalDataPack_t *pRxPack, uint32_t cycleTime_ms)
{
    uint8_t txData[64];
    uint16_t txLength = 0U;
    uint8_t subCommand = 0U;
    MpuHalDataPack_t txPack;

    if (E_SYNC_STATE_INIT == g_canConfigureSyncState) {
        g_canConfigureSyncState = E_SYNC_STATE_GET_CPU_REQUEST;
    } else if (E_SYNC_STATE_GET_CPU_REQUEST == g_canConfigureSyncState) {
        /* request subscription table for current channel */
        PackGetConfigureFromCpuRequest(g_syncChannel, g_canMsgIndex,
                                       g_indexNumberMaxOneTime, txData, &txLength);
        txPack.aid = 0x02;
        txPack.mid = 0x01;
        txPack.subcommand = 0x01;
        txPack.dataBufferSize = sizeof(txData);
        txPack.dataLength = txLength;
        txPack.pDataBuffer = txData;
        MpuHalTransmit(g_canMsgConfigureSyncMpuHandle, &txPack, MPU_HAL_SPI_MODE);
        g_timeOutCount = 0;
        g_canConfigureSyncState = E_SYNC_STATE_GET_CPU_WAIT_RESPONSE;
    } else if (E_SYNC_STATE_GET_CPU_WAIT_RESPONSE == g_canConfigureSyncState) {
        if (pRxPack != NULL) {
            subCommand = pRxPack->subcommand & 0x7F;
            if (subCommand == 0x02) {
                int16_t rxElementNum;
                uint8_t replyChannel = 0xFF;

                rxElementNum = ParseProtocolDataToCanConfigureBuffer(pRxPack->pDataBuffer,
                                                                     pRxPack->dataLength,
                                                                     &replyChannel);

                if ((rxElementNum >= 0) && (replyChannel == g_syncChannel)) {
                    if (rxElementNum < (int16_t)g_indexNumberMaxOneTime) {
                        /* this channel is fully synced, move to next channel */
                        g_canMsgIndex = 0x00;
                        SetCanMsgConfigureBufferValidData();
                        g_syncChannel = (uint8_t)((g_syncChannel + 1) % SUBSCRIBE_CHANNEL_NUM);
                        g_canConfigureSyncState = E_SYNC_STATE_GET_CPU_REQUEST;
                    } else {
                        /* more entries for this channel */
                        g_canMsgIndex += g_indexNumberMaxOneTime;
                        g_canConfigureSyncState = E_SYNC_STATE_GET_CPU_REQUEST;
                    }
                }
            } else {
                g_timeOutCount += cycleTime_ms;
            }
        } else {
            g_timeOutCount += cycleTime_ms;
            if (g_timeOutCount >= CPU_RESPONSE_TIME_OUT_TIME) {
                g_canConfigureSyncState = E_SYNC_STATE_GET_CPU_REQUEST;
            }
        }
    }
}

/****************************** Public Function Implementations ***************/

void CanMsgToCpuConfigureSyncCycleProcess(MpuHalDataPack_t *pRxData, uint32_t cycleTime_ms)
{
    ConfigureFromCpuSyncCycleProcess(pRxData, cycleTime_ms);
}

int16_t CanMsgDynamicConfigureInitialize(int16_t mpuHandle)
{
    g_canMsgConfigureSyncMpuHandle = mpuHandle;
    g_syncChannel = 0;
    g_canMsgIndex = 0;
    g_canConfigureSyncState = E_SYNC_STATE_INIT;
    return 0;
}
