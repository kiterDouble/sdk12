#ifndef _CAN_MSG_TO_CPU_H
#define _CAN_MSG_TO_CPU_H

#include <stdint.h>
#include "canHal.h"

#define RX_CAN_CONFIGURE_BUFFER_SIZE            300
#define RX_CAN_CONFIGURE_CHANNEL_NUMBER         4

typedef struct
{
  uint32_t canId;
  uint8_t msgRepeatNumber;
}CanMsgRxConfigure_t;

/* ---- CAN frame record for transparent forwarding to SOC ---- */

#define CAN_FRAME_HEADER_SIZE               (16u)
#define CAN_FRAME_MAX_DATA_SIZE             (64u)
#define CAN_FRAME_MAX_RECORD_SIZE           (CAN_FRAME_HEADER_SIZE + CAN_FRAME_MAX_DATA_SIZE)

/* flags bit definitions */
#define CAN_FRAME_FLAG_EXTENDED             (0x01u)
#define CAN_FRAME_FLAG_FD                   (0x02u)
#define CAN_FRAME_FLAG_BRS                  (0x04u)

/* Ring buffer for batching CAN frame records */
#define CAN_FRAME_RING_BUF_SIZE             (4096u)

typedef struct
{
    uint8_t  channel;
    uint8_t  flags;
    uint8_t  dlc;
    uint8_t  reserved;
    uint32_t canId;
    uint32_t timestampMs;
    uint32_t timestampUs;
    uint8_t  canData[CAN_FRAME_MAX_DATA_SIZE];
} CanFrameRecord_t;

typedef struct
{
    uint8_t  data[CAN_FRAME_RING_BUF_SIZE];
    uint16_t writeIndex;
    uint16_t readIndex;
    uint16_t frameCount;
} CanFrameRingBuf_t;

/* Dual buffer: one for CAN RX write, one for SPI TX read */
typedef struct
{
    CanFrameRingBuf_t buf[2];
    volatile uint8_t  activeBuf;
    volatile uint8_t  spiBusy;
    volatile uint32_t frameDropCount;
} CanFrameDualBuf_t;

/* ---- Initialization & subscription table ---- */

int16_t CanMsgTansmitToCpuInitialize(void);

int16_t GetCanMsgConfigureBufferSize(uint8_t canChannel);
int16_t CanMsgConfigureBufferAdd(uint8_t canChannel,CanMsgRxConfigure_t *pEelement);
void SetCanMsgConfigureBufferInvalidData(void);
void SetCanMsgConfigureBufferValidData(void);
uint8_t CanMsgConfigureBufferDataIsValid(void);

/* ---- Subscription filtering ---- */

int16_t CanMsgGetSubscribeIndex(uint8_t channel, uint32_t canId);
uint8_t CanMsgIsCanIdSubscribed(uint8_t channel, uint32_t canId);

/* ---- Ring buffer operations ---- */

int16_t CanFrameRingBufPush(CanFrameRingBuf_t *pBuf, const CanFrameRecord_t *pFrame);
uint16_t CanFrameRingBufDataLen(const CanFrameRingBuf_t *pBuf);
void CanFrameRingBufRead(const CanFrameRingBuf_t *pBuf, uint8_t *pOut, uint16_t offset, uint16_t length);
void CanFrameRingBufReset(CanFrameRingBuf_t *pBuf);
CanFrameRingBuf_t* CanFrameDualBufSwap(void);

/* ---- Frame push (replaces SaveCanMsgToBuffer) ---- */

int16_t CanFramePush(uint8_t channel, const CanHalMsg_t *pCanMsg, uint32_t timestampMs, uint32_t timestampUs);

/* Global dual buffer accessor */
CanFrameDualBuf_t* CanFrameDualBufGet(void);

#endif    //_CAN_MSG_TO_CPU_H
