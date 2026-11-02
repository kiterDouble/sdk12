/* Copyright Statement:
 *
 * This software/firmware and related documentation ("AutoChips Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to AutoChips Inc. and/or its licensors. Without
 * the prior written permission of AutoChips inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of AutoChips Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * AutoChips Inc. (C) 2024. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AUTOCHIPS SOFTWARE")
 * RECEIVED FROM AUTOCHIPS AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. AUTOCHIPS EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES AUTOCHIPS PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE AUTOCHIPS SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN AUTOCHIPS
 * SOFTWARE. AUTOCHIPS SHALL ALSO NOT BE RESPONSIBLE FOR ANY AUTOCHIPS SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AUTOCHIPS'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AUTOCHIPS SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT AUTOCHIPS'S OPTION, TO REVISE OR REPLACE THE
 * AUTOCHIPS SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO AUTOCHIPS FOR SUCH AUTOCHIPS SOFTWARE AT ISSUE.
 */

/**
* @file Spi_Hal_Types.h
*
* @brief Spi driver hal types header file.
*
*/

#ifndef SPI_HAL_TYPES_H
#define SPI_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"
#include "Platform_Types.h"

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/
/**
* @brief Type of SPI Event.
* @note DES_SPI_TYP_001
*/
enum
{
    SPI_STATUS_NONE = 0U,           /*!< SPI None status. */
    SPI_STATUS_RX_OVERFLOW_MASK,    /*!< SPI RX overflow status. */
    SPI_STATUS_TX_UNDERFLOW_MASK,   /*!< SPI TX underflow status. */
    SPI_STATUS_RX_FINISH_MASK,      /*!< SPI RX finsh status. */
    SPI_STATUS_TX_FINISH_MASK,      /*!< SPI TX finsh status. */
    SPI_STATUS_DATA_MATCH_MASK,     /*!< SPI Data match status. */
    SPI_STATUS_MEM_BUSY_MASK,       /*!< SPI Meby busy status. */
    SPI_STATUS_TIMEOUT_MASK,        /*!< SPI TX/RX timeout status. */
    SPI_STATUS_ABORT_MASK,          /*!< SPI TX/RX abort status. */
    SPI_STATUS_DMA_ERROR_MASK       /*!< SPI Dma error status. */
};

/**
* @brief Type of SPI status flags.
* @note DES_SPI_TYP_002
*/
typedef enum
{
    SPI_TX_DATA_FLAG = 0U,          /*!< TX data not full flag */
    SPI_RX_DATA_FLAG = 1U,          /*!< RX data not empty flag */
    SPI_TRANSMIT_ERROR = 2U,        /*!< Transmit Error flag (TX underrun) */
    SPI_RECEIVE_ERROR = 3U,         /*!< Receive Error flag (RX overrun) */
    SPI_MODE_FAULT_ERROR = 4U,      /*!< Master mode fault error flag */
    SPI_DATA_MATCH = 5U,            /*!< Data match flag */
    SPI_MASTER_BUSY = 7U,           /*!< SPI engine busy flag */
    SPI_MODULE_IDLE = 8U,           /*!< Module idle flag */
    SPI_ALL_STATUS = 0x0000002CU    /*!< Used for clearing all w1c status flags */
} Spi_StatusFlagType;

/**
* @brief Type of SPI interrupt source.
* @note DES_SPI_TYP_003
*/
typedef enum
{
    SPI_INT_TX_DATA = 8U,           /*!< TX data not full interrupt */
    SPI_INT_RX_DATA = 9U,           /*!< RX data not empty interrupt */
    SPI_INT_TRANSMIT_ERROR = 10U,   /*!< Transmit Error interrupt (TX underrun) */
    SPI_INT_RECEIVE_ERROR = 11U,    /*!< Receive Error interrupt (RX overrun) */
    SPI_INT_MODE_FAULT_ERROR = 13U, /*!< Master mode fault error interrupt */
    SPI_INT_DATA_MATCH = 31U        /*!< Data match interrupt */
} Spi_InterruptType;

/**
* @brief SPI master or slave configuration.
* @note DES_SPI_TYP_004
*/
typedef enum
{
    SPI_SLAVE = 0U,     /*!< SPI peripheral operates in slave mode. */
    SPI_MASTER          /*!< SPI peripheral operates in master mode. */
} Spi_ModeType;

/**
* @brief SPI pin (SOUT and SIN) configuration.
* @note DES_SPI_TYP_005
*/
typedef enum
{
    SPI_SOUT_MOSI_SIN_MISO = 0U,    /*!< SPI SOUT pin set as MOSI, SIN pin set as MISO */
    SPI_SOUT_MISO_SIN_MOSI          /*!< SPI SOUT pin set as MISO, SIN pin set as MOSI */
} Spi_PinCfgType;

/**
* @brief SPI transfer data width configuration.
* @note DES_SPI_TYP_006
*/
typedef enum
{
    SPI_DATA_WIDTH_1BIT = 0U,   /*!< SPI data line 1 bit, data out on SOUT, in on SIN (standby SPI) */
    SPI_DATA_WIDTH_2BIT,        /*!< SPI data line 2 bits, data out/in on SOUT, SIN */
    SPI_DATA_WIDTH_4BIT         /*!< SPI data line 4 bits, out on SDO/SDI/PCS[3:2] and in on SDO/SDI/PCS[3:2] */
} Spi_TransferWidthType;

/**
* @brief SPI hreq polarity type selection.
* @note DES_SPI_TYP_007
*/
typedef enum
{
    SPI_HREQ_POLARITY_HIGH = 0U,    /*!< SPI host request active polarity is high */
    SPI_HREQ_POLARITY_LOW = 1U      /*!< SPI host request active polarity is low */
} Spi_HreqPolarityType;

/**
* @brief SPI Clock (SCK) polarity configuration.
* @note DES_SPI_TYP_008
*/
typedef enum
{
    SPI_CPOL_LOW = 0U,      /*!< SCK is low when idle */
    SPI_CPOL_HIGH           /*!< SCK is high when idle */
} Spi_CpolType;

/**
* @brief SPI clock phase configuration.
* @note DES_SPI_TYP_009
*/
typedef enum
{
    SPI_CPHA_0 = 0U,    /*!< Data captured on SCK 1st edge, changed on 2nd. */
    SPI_CPHA_1          /*!< Data changed on SCK 1st edge, captured on 2nd. */
} Spi_CphaType;

/**
* @brief SPI PCS polarity configuration.
* @note DES_SPI_TYP_010
*/
typedef enum
{
    SPI_PCS_POLARITY_LOW = 0U,  /*!< PCS active low (idle is high). */
    SPI_PCS_POLARITY_HIGH       /*!< PCS active high (idle is low). */
} Spi_PcsPolarityType;

/**
* @brief SPI Peripheral Chip Select(PCS) configuration.
* @note DES_SPI_TYP_011
*/
typedef enum
{
    SPI_PCS_0 = 0U,     /*!< PCS0 */
    SPI_PCS_1,          /*!< PCS1 */
    SPI_PCS_2,          /*!< PCS2 */
    SPI_PCS_3,          /*!< PCS3 */
    SPI_PCS_GPIO        /*!< GPIO AS CS */
} Spi_PcsType;

/**
* @brief Type of SPI transfer(based on interrupts or DMA or Poll).
* @note DES_SPI_TYP_012
*/
typedef enum
{
    SPI_USING_DMA = 0x00U,          /*!< The driver will use DMA to perform SPI transfer*/
    SPI_USING_POLL = 0x01U,         /*!< The driver will use POLL to perform SPI transfer*/
    SPI_USING_INTERRUPT = 0x02U     /*!< The driver will use interrupt to perform SPI transfer*/
} Spi_TransferType;

/*PRQA S 1535 ++ # typedef is declared but not used within this project. */
/*!
* @brief Type of SPI transfer status type.
* @note DES_SPI_TYP_013
*/
typedef enum
{
    SPI_TRANSCEIVE_BUSY = 0x00U,        /*!< The transfer is on going*/
    SPI_TRANSCEIVE_ERROR = 0x01U,       /*!< The transfer is error*/
    SPI_TRANSCEIVE_SUCCESS = 0x02U,     /*!< The transfer is success*/
    SPI_TRANSCEIVE_TIMEOUT = 0x03U,     /*!< The transfer is timeout*/
    SPI_TRANSCEIVE_NOT_STARTED = 0x04U  /*!< The transfer is not started*/
} Spi_TransceiveStatusType;
/*PRQA S 1535 -- */
/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*!
 * @brief Callback for SPI.
 * @note DES_SPI_TYP_014
 */
typedef void (*Spi_CallbackType)(uint8 Instance, uint16 Status);

/*PRQA S 3630 ++ # AMCE_QAC_3630 */
/*!
* @brief User configuration structure for the hardware SPI driver.
* @note DES_SPI_TYP_015
*/
typedef struct
{
    uint8 CsSetup;                  /*!< CS_SETUP, Timing between chip select and clock */
    uint8 CsHold;                   /*!< CS_HOLD, Timing between clock and chip select */
    uint8 CsIdle;                   /*!< CS_IDLE, Timing between chip select and chip select */
    uint8 FrmSize;                  /*!< Number of bits per frame, support 4~32 bits */
    uint8 TxDmaChannel;             /*!< DMA tx channel. It will be ignored if DMA mode isn't used. */
    uint8 RxDmaChannel;             /*!< DMA rx channel. It will be ignored if DMA mode isn't used. */
    boolean MsbFirst;               /*!< Option to transmit MSB first */
    boolean CsOutputEn;             /*!< The chip select output is enabled or not */
    boolean ContinuousCs;           /*!< Keeps PCS asserted until transfer complete */
    boolean HreqEn;                 /*!< Set Hreq is enabled or not*/
    Spi_ModeType Mode;              /*!< Selects master or slave mode */
    Spi_CpolType Cpol;              /*!< Selects clock polarity */
    Spi_CphaType Cpha;              /*!< Selects which phase of clock to capture data */
    Spi_PcsType PcsCfg;             /*!< Selects which PCS to use */
    Spi_PcsPolarityType PcsPol;     /*!< PCS polarity */
    Spi_HreqPolarityType HreqPol;   /*!< Hreq polarity */
    Spi_TransferWidthType Width;    /*!< Selects WIDTH */
    Spi_PinCfgType PinCfg;          /*!< SPI pin (SOUT and SIN) configuration */
    Spi_CallbackType Callback;      /*!< Calling the callback to transfer complete */
    uint32 BaudRate;                /*!< Set the SPI Transfer baudrate */
} Spi_HalConfigType;
/*PRQA S 3630 -- */

/*!
* @brief Runtime state structure for the device on the SPI bus.
* @note DES_SPI_TYP_016
*/
typedef struct
{
    uint8 FrmSize;                  /*!< Number of bits per frame, support 4~32 bits */
    uint8 TxDmaChannel;             /*!< DMA tx channel. It will be ignored if DMA mode isn't used. */
    uint8 RxDmaChannel;             /*!< DMA rx channel. It will be ignored if DMA mode isn't used. */
    boolean InProgress;             /*!< True if transfer is active */
    boolean MsbFirst;               /*!< Option to transmit MSB first */
    uint16 TxCount;                 /*!< Transmit bytes counter */
    uint16 RxCount;                 /*!< Receive bytes counter */
    uint16 XferCount;               /*!< Number of bytes to transfer */
    uint16 Status;                  /*!< The current transfer status */
    const uint8 *TxBuffPtr;         /*!< Transmit buffer base pointer */
    uint8 *RxBuffPtr;               /*!< Receive buffer base pointer */
    uint8 *RxBuffPtr1;              /*!< Receive buffer base pointer */
    Spi_PcsType Pcs;                /*!< Spi cs select */
    Spi_CallbackType Callback;      /*!< Calling the callback to transfer complete */
    Spi_TransferType TransferType;  /*!< Type of SPI transfer, DMA or interrupt */
} Spi_StateType;

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /**< SPI_HAL_TYPES_H */

/*====================================================EOF===========================================*/
