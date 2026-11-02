#ifndef _CAN_MSG_SPI_TRANSMIT_H
#define _CAN_MSG_SPI_TRANSMIT_H

#include <stdint.h>
#include "mpuHal.h"

/* SPI TX trigger parameters */
#define SPI_TX_TIME_WINDOW_MS        (20u)
#define SPI_TX_FRAME_THRESHOLD       (8u)

/* Max payload per SPI DMA packet (2048 - 6B SPI header - margin) */
#define SPI_TX_PAYLOAD_MAX           (2030u)

int16_t CanMsgSpiTransmitInitialize(int16_t mpuHandle);
void CanMsgSpiTransmitCycleProcess(uint32_t cycleTimeMs);
void CanMsgSpiTransmitFlushAll(void);

uint32_t CanMsgSpiTransmitGetSentBytes(void);

#endif    //_CAN_MSG_SPI_TRANSMIT_H
