/*************************************************
 Copyright (C) 2025 SiRun (AnHui). All rights reserved.
 File Name: canMsgSpiTransmit.c
 Author:
 Created Time:
 Description: SPI batch transmitter for multi-channel CAN
              frame forwarding to SOC. Uses dual-ring-buffer
              swap with time-window/threshold dual trigger
              and chunked SPI DMA send.
 Others:
 *************************************************/

/****************************** include ***************************************/
#include "canMsgSpiTransmit.h"
#include "FreeRTOS.h"
#include "canMsgToCpu.h"
#include "logHal.h"
#include "mpuHal.h"
#include "task.h"

#include <string.h>

/****************************** Macro Definitions ******************************/

/****************************** Type Definitions ******************************/

/****************************** Global Variables ******************************/
static int16_t g_spiMpuHandle = -1;
static uint32_t g_spiTxTimeCount = 0U;
static uint8_t g_spiTxBuffer[SPI_TX_PAYLOAD_MAX];

/* Pending send state: when a dual-buf swap has occurred but the
   send buffer has not been fully drained yet */
static CanFrameRingBuf_t *g_pPendingSendBuf = NULL;
static uint16_t g_pendingOffset = 0U;
static uint16_t g_pendingTotal = 0U;
static uint32_t g_spiTotalSentBytes = 0U;

/****************************** Function Declarations *************************/
static void CanMsgSpiSendFromBuf(CanFrameRingBuf_t *pBuf);
static void CanMsgSpiSendNextChunk(void);

/****************************** Private Function Implementations ***************/

/* Send all data from a ring buffer in chunks via SPI.
   The logical payload begins with a 2-byte big-endian header that
   carries the number of CanFrameRecord_t entries; the records follow. */
static void CanMsgSpiSendFromBuf(CanFrameRingBuf_t *pBuf)
{
    g_pPendingSendBuf = pBuf;
    g_pendingOffset = 0U;
    g_pendingTotal = (uint16_t)(CanFrameRingBufDataLen(pBuf) + 2U);
    CanMsgSpiSendNextChunk();
}

/* Send the next chunk of pending data. Called initially and after
   each chunk's MpuHalTransmit returns. */
static void CanMsgSpiSendNextChunk(void)
{
    uint16_t chunkLen;
    uint16_t frameCount;
    MpuHalDataPack_t txData;

    if (g_pPendingSendBuf == NULL) {
        return;
    }

    if (g_pendingOffset >= g_pendingTotal) {
        /* all chunks sent, reset the buffer for reuse */
        CanFrameRingBufReset(g_pPendingSendBuf);
        g_pPendingSendBuf = NULL;
        CanFrameDualBufGet()->spiBusy = 0;
        return;
    }

    if (g_pendingOffset == 0U) {
        /* First chunk: prepend 2-byte big-endian frame count. */
        frameCount = g_pPendingSendBuf->frameCount;
        g_spiTxBuffer[0] = (uint8_t)((frameCount >> 8) & 0xFFU);
        g_spiTxBuffer[1] = (uint8_t)(frameCount & 0xFFU);

        chunkLen = g_pendingTotal - g_pendingOffset;
        if (chunkLen > SPI_TX_PAYLOAD_MAX) {
            chunkLen = SPI_TX_PAYLOAD_MAX;
        }

        /* Append the first (chunkLen - 2) record bytes after the header. */
        if (chunkLen > 2U) {
            CanFrameRingBufRead(g_pPendingSendBuf, &g_spiTxBuffer[2], 0U, (uint16_t)(chunkLen - 2U));
        }
    } else {
        chunkLen = g_pendingTotal - g_pendingOffset;
        if (chunkLen > SPI_TX_PAYLOAD_MAX) {
            chunkLen = SPI_TX_PAYLOAD_MAX;
        }
        /* Continue reading records; skip the 2-byte header at the logical front. */
        CanFrameRingBufRead(g_pPendingSendBuf, g_spiTxBuffer, (uint16_t)(g_pendingOffset - 2U), chunkLen);
    }

    txData.aid = 0x02;
    txData.mid = 0x04;
    txData.subcommand = 0x00;
    txData.dataBufferSize = chunkLen;
    txData.dataLength = chunkLen;
    txData.pDataBuffer = g_spiTxBuffer;

    if (MpuHalTransmit(g_spiMpuHandle, &txData, MPU_HAL_SPI_MODE) != MPU_HAL_STATUS_OK) {
        // TBOX_PRINT("CanMsgSpi: MpuHalTransmit failed, dropping %d bytes\r\n", chunkLen);
        /* skip this chunk and try the next after a yield */
        vTaskDelay(pdMS_TO_TICKS(1));
        g_pendingOffset += chunkLen;
        CanMsgSpiSendNextChunk();
        return;
    }

    g_pendingOffset += chunkLen;
    g_spiTotalSentBytes += chunkLen;

    /* If more chunks remain, send them after a brief yield to let
       MpuHalSpiTxTask process. The SPI DMA at 40MHz takes ~0.4ms
       per 2KB chunk, so a 1ms delay is sufficient. */
    if (g_pendingOffset < g_pendingTotal) {
        vTaskDelay(pdMS_TO_TICKS(1));
        CanMsgSpiSendNextChunk();
    } else {
        /* all chunks queued, reset buffer and release busy flag */
        CanFrameRingBufReset(g_pPendingSendBuf);
        g_pPendingSendBuf = NULL;
        CanFrameDualBufGet()->spiBusy = 0;
    }
}

/****************************** Public Function Implementations ***************/

int16_t CanMsgSpiTransmitInitialize(int16_t mpuHandle)
{
    g_spiMpuHandle = mpuHandle;
    g_spiTxTimeCount = 0U;
    g_pPendingSendBuf = NULL;
    g_pendingOffset = 0U;
    g_pendingTotal = 0U;
    return 0;
}

void CanMsgSpiTransmitCycleProcess(uint32_t cycleTimeMs)
{
    CanFrameDualBuf_t *pDual = CanFrameDualBufGet();
    CanFrameRingBuf_t *pActive = &pDual->buf[pDual->activeBuf];

    g_spiTxTimeCount += cycleTimeMs;

    /* trigger conditions */
    uint8_t frameThresholdMet = (pActive->frameCount >= SPI_TX_FRAME_THRESHOLD) ? 1u : 0u;
    uint8_t timeWindowMet = (g_spiTxTimeCount >= SPI_TX_TIME_WINDOW_MS) ? 1u : 0u;

    if (!frameThresholdMet && !timeWindowMet) {
        return;
    }

    /* if a previous send is still in progress, skip this cycle */
    if (pDual->spiBusy) {
        return;
    }

    /* if still draining a pending send, continue it */
    if (g_pPendingSendBuf != NULL) {
        CanMsgSpiSendNextChunk();
        return;
    }

    /* check if there is data to send */
    if (CanFrameRingBufDataLen(pActive) == 0U) {
        g_spiTxTimeCount = 0U;
        return;
    }

    /* swap buffers: CAN RX will write to the other buffer */
    pDual->spiBusy = 1;
    CanFrameRingBuf_t *pSendBuf = CanFrameDualBufSwap();
    g_spiTxTimeCount = 0U;

    if (CanFrameRingBufDataLen(pSendBuf) > 0U) {
        CanMsgSpiSendFromBuf(pSendBuf);
    } else {
        CanFrameRingBufReset(pSendBuf);
        pDual->spiBusy = 0;
    }
}

void CanMsgSpiTransmitFlushAll(void)
{
    CanFrameDualBuf_t *pDual = CanFrameDualBufGet();

    if (pDual->spiBusy) {
        return;
    }

    CanFrameRingBuf_t *pActive = &pDual->buf[pDual->activeBuf];
    if (CanFrameRingBufDataLen(pActive) == 0U) {
        return;
    }

    pDual->spiBusy = 1;
    CanFrameRingBuf_t *pSendBuf = CanFrameDualBufSwap();
    CanMsgSpiSendFromBuf(pSendBuf);
}

uint32_t CanMsgSpiTransmitGetSentBytes(void)
{
    return g_spiTotalSentBytes;
}
