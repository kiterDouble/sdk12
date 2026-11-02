/*************************************************
 Copyright (C) 2025 SiRun (AnHui). All rights reserved.
 File Name: canMsgToCpu.c
 Author:
 Created Time:
 Description: Multi-channel CAN frame collection, subscription
              filtering, and ring-buffer packing for SOC forwarding.
 Others:
 *************************************************/

/****************************** include ***************************************/
#include "canMsgToCpu.h"
#include <string.h>

/****************************** Macro Definitions ******************************/

/* Convert host-order uint32_t to big-endian byte sequence (byte 0 = MSB). */
static inline void store_be32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)((v >> 24) & 0xFFu);
    p[1] = (uint8_t)((v >> 16) & 0xFFu);
    p[2] = (uint8_t)((v >> 8) & 0xFFu);
    p[3] = (uint8_t)(v & 0xFFu);
}

/****************************** Type Definitions ******************************/
typedef struct
{
    uint32_t size;
    uint32_t elementCount;
} CanIdConfigure_t;

typedef struct
{
    int16_t buffer[RX_CAN_CONFIGURE_BUFFER_SIZE];
    int16_t index;
} StackBuffer_t;

/****************************** Global Variables ******************************/
CanMsgRxConfigure_t g_canRxToCpuConfigureBuffer[RX_CAN_CONFIGURE_CHANNEL_NUMBER][RX_CAN_CONFIGURE_BUFFER_SIZE];
static CanIdConfigure_t g_canRxToCpuConfigure[RX_CAN_CONFIGURE_CHANNEL_NUMBER];
static StackBuffer_t g_stack;

/* Dual ring buffer for CAN frame records */
static CanFrameDualBuf_t g_canFrameDualBuf;

/****************************** Function Declarations *************************/
static void StackInit(StackBuffer_t *pStack);
static void StackPush(StackBuffer_t *pStack, int16_t value);
static int16_t StackPop(StackBuffer_t *pStack);
static int16_t StackTop(StackBuffer_t *pStack);
static int16_t StackIsEmpty(StackBuffer_t *pStack);

/* ---- Stack helpers (for non-recursive quicksort) ---- */

static void StackInit(StackBuffer_t *pStack)
{
    pStack->index = 0;
}

static void StackPush(StackBuffer_t *pStack, int16_t value)
{
    if (pStack->index >= (int16_t)(sizeof(pStack->buffer) / sizeof(pStack->buffer[0]))) {
        return;
    }
    pStack->buffer[pStack->index] = value;
    pStack->index++;
}

static int16_t StackPop(StackBuffer_t *pStack)
{
    if (pStack->index <= 0) {
        return 0;
    }
    pStack->index--;
    return pStack->buffer[pStack->index];
}

static int16_t StackTop(StackBuffer_t *pStack)
{
    if (pStack->index == 0) {
        return 0;
    }
    return pStack->buffer[(pStack->index - 1)];
}

static int16_t StackIsEmpty(StackBuffer_t *pStack)
{
    if (pStack->index <= 0) {
        return 0;
    }
    return 1;
}

/* ---- Non-recursive quicksort by canId (ascending) ---- */

static int16_t Partition(CanMsgRxConfigure_t *pConfigureArray, int16_t left, int16_t right)
{
    CanMsgRxConfigure_t pivot;
    memcpy(&pivot, &pConfigureArray[left], sizeof(CanMsgRxConfigure_t));
    while (left < right) {
        while ((left < right) && (pConfigureArray[right].canId >= pivot.canId)) {
            right--;
        }
        memcpy(&pConfigureArray[left], &pConfigureArray[right], sizeof(CanMsgRxConfigure_t));
        while ((left < right) && (pConfigureArray[left].canId <= pivot.canId)) {
            left++;
        }
        memcpy(&pConfigureArray[right], &pConfigureArray[left], sizeof(CanMsgRxConfigure_t));
    }
    memcpy(&pConfigureArray[left], &pivot, sizeof(CanMsgRxConfigure_t));
    return left;
}

static void CanMsgConfigureSortLittleToBig(CanMsgRxConfigure_t *pConfigureArray, uint16_t arraySize)
{
    int16_t left = 0;
    int16_t right;
    int16_t index;
    if (arraySize == 0) {
        return;
    }
    right = arraySize - 1;
    StackInit(&g_stack);
    StackPush(&g_stack, left);
    StackPush(&g_stack, right);
    while (StackIsEmpty(&g_stack) != 0) {
        right = StackTop(&g_stack);
        StackPop(&g_stack);
        left = StackTop(&g_stack);
        StackPop(&g_stack);

        index = Partition(pConfigureArray, left, right);
        if ((index - 1) > left) {
            StackPush(&g_stack, left);
            StackPush(&g_stack, index - 1);
        }
        if ((index + 1) < right) {
            StackPush(&g_stack, index + 1);
            StackPush(&g_stack, right);
        }
    }
}

/* ---- Binary search in subscription table ---- */

int16_t CanMsgGetSubscribeIndex(uint8_t channel, uint32_t canId)
{
    int16_t index = -1;
    uint32_t size;
    uint32_t indexMin, indexMax, half, i;

    if (channel >= RX_CAN_CONFIGURE_CHANNEL_NUMBER) {
        return -1;
    }
    size = g_canRxToCpuConfigure[channel].size;
    if (size == 0) {
        return index;
    }

    indexMin = 0;
    indexMax = size - 1;
    for (i = 0; i < 50; i++) {
        if (indexMax - indexMin < 2) {
            if (canId == g_canRxToCpuConfigureBuffer[channel][indexMin].canId) {
                index = (int16_t)indexMin;
            } else if (canId == g_canRxToCpuConfigureBuffer[channel][indexMax].canId) {
                index = (int16_t)indexMax;
            }
            break;
        }
        half = (indexMin + indexMax) / 2;
        if (canId > g_canRxToCpuConfigureBuffer[channel][half].canId) {
            indexMin = half;
        } else {
            indexMax = half;
        }
    }
    return index;
}

uint8_t CanMsgIsCanIdSubscribed(uint8_t channel, uint32_t canId)
{
    return (CanMsgGetSubscribeIndex(channel, canId) >= 0) ? 1u : 0u;
}

/* ---- Ring buffer operations ---- */

int16_t CanFrameRingBufPush(CanFrameRingBuf_t *pBuf, const CanFrameRecord_t *pFrame)
{
    uint16_t recordLen;
    uint16_t freeSpace;
    uint16_t remain;

    recordLen = CAN_FRAME_HEADER_SIZE + pFrame->dlc;

    /* calculate free space in ring buffer */
    if (pBuf->writeIndex >= pBuf->readIndex)
        freeSpace = CAN_FRAME_RING_BUF_SIZE - (pBuf->writeIndex - pBuf->readIndex) - 1;
    else
        freeSpace = pBuf->readIndex - pBuf->writeIndex - 1;

    if (freeSpace < recordLen)
        return -1; /* buffer full, drop frame */

    /* handle wrap-around */
    remain = CAN_FRAME_RING_BUF_SIZE - pBuf->writeIndex;
    if (remain >= recordLen) {
        memcpy(&pBuf->data[pBuf->writeIndex], pFrame, recordLen);
    } else {
        memcpy(&pBuf->data[pBuf->writeIndex], pFrame, remain);
        memcpy(&pBuf->data[0], (const uint8_t *)pFrame + remain, recordLen - remain);
    }
    pBuf->writeIndex = (uint16_t)((pBuf->writeIndex + recordLen) % CAN_FRAME_RING_BUF_SIZE);
    pBuf->frameCount++;
    return 0;
}

uint16_t CanFrameRingBufDataLen(const CanFrameRingBuf_t *pBuf)
{
    uint16_t dataLen;
    if (pBuf->writeIndex >= pBuf->readIndex)
        dataLen = pBuf->writeIndex - pBuf->readIndex;
    else
        dataLen = CAN_FRAME_RING_BUF_SIZE - pBuf->readIndex + pBuf->writeIndex;
    return dataLen;
}

void CanFrameRingBufRead(const CanFrameRingBuf_t *pBuf, uint8_t *pOut, uint16_t offset, uint16_t length)
{
    uint16_t readPos;
    uint16_t remain;

    readPos = (uint16_t)((pBuf->readIndex + offset) % CAN_FRAME_RING_BUF_SIZE);
    remain = CAN_FRAME_RING_BUF_SIZE - readPos;

    if (remain >= length) {
        memcpy(pOut, &pBuf->data[readPos], length);
    } else {
        memcpy(pOut, &pBuf->data[readPos], remain);
        memcpy(pOut + remain, &pBuf->data[0], length - remain);
    }
}

void CanFrameRingBufReset(CanFrameRingBuf_t *pBuf)
{
    pBuf->writeIndex = 0;
    pBuf->readIndex = 0;
    pBuf->frameCount = 0;
}

CanFrameRingBuf_t *CanFrameDualBufSwap(void)
{
    uint8_t sendBuf = g_canFrameDualBuf.activeBuf;
    /* switch active buffer so CAN RX writes to the other one */
    g_canFrameDualBuf.activeBuf ^= 1;
    return &g_canFrameDualBuf.buf[sendBuf];
}

CanFrameDualBuf_t *CanFrameDualBufGet(void)
{
    return &g_canFrameDualBuf;
}

/* ---- Frame push: write directly to ring buffer (no subscription filter) ---- */

int16_t CanFramePush(uint8_t channel, const CanHalMsg_t *pCanMsg, uint32_t timestampMs, uint32_t timestampUs)
{
    CanFrameRecord_t frame;
    CanFrameRingBuf_t *pActive;
    uint8_t dlc;

    if (channel >= RX_CAN_CONFIGURE_CHANNEL_NUMBER) {
        return -1;
    }

    dlc = pCanMsg->dlc;
    if (dlc > CAN_FRAME_MAX_DATA_SIZE) {
        dlc = CAN_FRAME_MAX_DATA_SIZE;
    }

    /* build frame record */
    frame.channel = channel;
    frame.flags = (pCanMsg->canId & 0x80000000u) ? CAN_FRAME_FLAG_EXTENDED : 0u;
    frame.dlc = dlc;
    frame.reserved = 0;
    store_be32((uint8_t *)&frame.canId, pCanMsg->canId & 0x1FFFFFFFu);
    store_be32((uint8_t *)&frame.timestampMs, timestampMs);
    store_be32((uint8_t *)&frame.timestampUs, timestampUs);
    memcpy(frame.canData, pCanMsg->canData, dlc);

    pActive = &g_canFrameDualBuf.buf[g_canFrameDualBuf.activeBuf];

    if (CanFrameRingBufPush(pActive, &frame) != 0) {
        g_canFrameDualBuf.frameDropCount++;
        return -1;
    }
    return 0;
}

/* ---- Initialization & subscription table management ---- */

int16_t CanMsgTansmitToCpuInitialize(void)
{
    uint32_t i, j;
    for (i = 0; i < RX_CAN_CONFIGURE_CHANNEL_NUMBER; i++) {
        for (j = 0; j < RX_CAN_CONFIGURE_BUFFER_SIZE; j++) {
            g_canRxToCpuConfigureBuffer[i][j].msgRepeatNumber = 0x00;
        }
    }
    /* reset dual ring buffer */
    CanFrameRingBufReset(&g_canFrameDualBuf.buf[0]);
    CanFrameRingBufReset(&g_canFrameDualBuf.buf[1]);
    g_canFrameDualBuf.activeBuf = 0;
    g_canFrameDualBuf.spiBusy = 0;
    g_canFrameDualBuf.frameDropCount = 0;
    return 0;
}

int16_t GetCanMsgConfigureBufferSize(uint8_t canChannel)
{
    if (canChannel >= RX_CAN_CONFIGURE_CHANNEL_NUMBER) {
        return -1;
    }
    return RX_CAN_CONFIGURE_BUFFER_SIZE;
}

int16_t CanMsgConfigureBufferAdd(uint8_t canChannel, CanMsgRxConfigure_t *pElement)
{
    if (canChannel >= RX_CAN_CONFIGURE_CHANNEL_NUMBER) {
        return -1;
    }
    if (g_canRxToCpuConfigure[canChannel].elementCount >= RX_CAN_CONFIGURE_BUFFER_SIZE) {
        return -1;
    }
    g_canRxToCpuConfigureBuffer[canChannel][g_canRxToCpuConfigure[canChannel].elementCount].canId = pElement->canId;
    g_canRxToCpuConfigureBuffer[canChannel][g_canRxToCpuConfigure[canChannel].elementCount].msgRepeatNumber = pElement->msgRepeatNumber;
    g_canRxToCpuConfigure[canChannel].elementCount++;
    return 0;
}

void SetCanMsgConfigureBufferInvalidData(void)
{
    uint32_t i;
    for (i = 0; i < RX_CAN_CONFIGURE_CHANNEL_NUMBER; i++) {
        g_canRxToCpuConfigure[i].size = 0x00;
        g_canRxToCpuConfigure[i].elementCount = 0x00;
    }
}

void SetCanMsgConfigureBufferValidData(void)
{
    uint32_t i;
    for (i = 0; i < RX_CAN_CONFIGURE_CHANNEL_NUMBER; i++) {
        CanMsgConfigureSortLittleToBig(g_canRxToCpuConfigureBuffer[i], g_canRxToCpuConfigure[i].elementCount);
    }
    for (i = 0; i < RX_CAN_CONFIGURE_CHANNEL_NUMBER; i++) {
        g_canRxToCpuConfigure[i].size = g_canRxToCpuConfigure[i].elementCount;
    }
}

uint8_t CanMsgConfigureBufferDataIsValid(void)
{
    uint32_t i;
    for (i = 0; i < RX_CAN_CONFIGURE_CHANNEL_NUMBER; i++) {
        if (g_canRxToCpuConfigure[i].size) {
            return 1;
        }
    }
    return 0;
}
