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
* @file Spi_Hal.h
*
* @brief Spi driver hal api header file.
*
*/

#ifndef SPI_HAL_H
#define SPI_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Spi_Hal_Types.h"

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
* @brief This function initializes a SPI instance.
* @note Function ID: DES_SPI_API_200
* @param[in] Instance: SPI instance.
* @param[in] ConfigPtr: The data structure containing information about a device on the SPI bus.
* @return None
*/
void Spi_Hal_Init(uint8 Instance, const Spi_HalConfigType *ConfigPtr);

/**
* @brief This function de-initializes a SPI instance.
* @note Function ID: DES_SPI_API_201
* @param[in] Instance: SPI instance.
* @return void
*/
void Spi_Hal_DeInit(uint8 Instance);

/**
* @brief This function gets default configuration for SPI hal driver.
* @note Function ID: DES_SPI_API_202
* @param[out] ConfigPtr: Pointer to configuration structure which is filled with default configuration.
* @return void
*/
void Spi_Hal_GetDefaultConfig(Spi_HalConfigType *ConfigPtr);

/**
* @brief This function configures the SPI frame parameters.
* @note Function ID: DES_SPI_API_204
* @param[in] Instance: SPI instance.
* @param[in] FrmSize: Number of bits per frame, support 4~32 bits.
* @param[in] MsbFirst: Option to transmit MSB first.
* @return void
*/
void Spi_Hal_ConfigureFrame(uint8 Instance, uint8 FrmSize, boolean MsbFirst);

/**
* @brief This function transfers by polling mode, which will not return untill timeout or transfer finish.
* @note Function ID: DES_SPI_API_205
* @param[in] Instance: SPI instance.
* @param[in] TxBuffPtr: The pointer to the data buffer of the data to send.
* @param[inout] RxBuffPtr: Pointer to the buffer where the received bytes are stored.
* @param[in] ByteCount: The number of bytes to send and receive.
* @param[in] Timeout: A timeout count value for the transfer. If the transfer takes longer than
*                     this amount of time, the transfer is aborted and a STATUS_TIMEOUT error returned.
* @return Hal_StatusType
*/
Hal_StatusType Spi_Hal_TransceivePoll
(
    uint8 Instance,
    const uint8 *TxBuffPtr,
    uint8 *RxBuffPtr,
    uint16 ByteCount,
    uint32 Timeout
);

/**
* @brief SPI transmission, reception by interrupt.
* @note Function ID: DES_SPI_API_207
* @param[in] Instance: SPI instance.
* @param[in] TxBuffPtr: The pointer to the data buffer of the data to send.
* @param[inout] RxBuffPtr: Pointer to the buffer where the received bytes are stored.
* @param[in] ByteCount: The number of bytes to send and receive.
* @return Hal_StatusType
*/
Hal_StatusType Spi_Hal_TransceiveInt
(
    uint8 Instance,
    const uint8 *TxBuffPtr,
    uint8 *RxBuffPtr,
    uint16 ByteCount
);

/**
* @brief SPI transmission,reception by DMA.
* @note Function ID: DES_SPI_API_209
* @param[in] Instance: SPI instance.
* @param[in] TxBuffPtr: The pointer to the data buffer of the data to send.
* @param[inout] RxBuffPtr: Pointer to the buffer where the received bytes are stored.
* @param[in] ByteCount: The number of bytes to send and receive.
* @return Hal_StatusType
*/
Hal_StatusType Spi_Hal_TransceiveDma
(
    uint8 Instance,
    const uint8 *TxBuffPtr,
    const uint8 *RxBuffPtr,
    uint16 ByteCount
);

#ifndef SPI_SDK_NON_EXTENDED_API
/**
* @brief This function get spi receive data when in rx buffer.
* @note Function ID: DES_SPI_API_233
* @param[in] Instance: SPI instance.
* @param[in] ReceiveBuffer: To set the BaudRate value.
* @return uint16: SPI get receive data len.
*/
uint16 Spi_Hal_SlaveGetReceiveLen
(
    uint8 Instance,
    uint8 **ReceiveBuffer
);

/*!
* @brief SPI using interrupt receive by NolimitLen mode, which will
*        return after start transfer.The user needs to check whether the receiveBuffer
*        has valid data the Spi_Hal_GetTransceiveStatus function.
*
* @param[in] Instance: SPI module instance
* @param[in] ReceiveBuffer1: Pointer to the buffer where the received bytes are stored.
* @param[in] ReceiveBuffer2: Pointer to the buffer where the received bytes are stored.
* @param[in] MaxreceiveByteCount: The receiveBuffer max len.
* @return STATUS_SUCCESS The transfer was successful, or
*         STATUS_BUSY Cannot perform transfer because a transfer is already in progress
*/
Hal_StatusType Spi_Hal_SlaveReceiveNolimitLen(uint8 Instance,
        uint8 *ReceiveBuffer1,
        uint8 *ReceiveBuffer2,
        uint16 MaxreceiveByteCount);

/**
* @brief This function stop SPI Transceive.
* @note Function ID: DES_SPI_API_235
* @param[in] Instance: SPI instance.
* @return Hal_StatusType
*/
Hal_StatusType Spi_Hal_AbortTransceive(uint8 Instance);

/**
* @brief This function get SPI Transceive Status.
* @note Function ID: DES_SPI_API_236
* @param[in] Instance: SPI instance.
* @return Spi_TransceiveStatusType:the Transceive Status.
*/
Spi_TransceiveStatusType Spi_Hal_GetTransceiveStatus(uint8 Instance);
#endif

/**
* @brief This function set the SPI BaudRate.
* @note Function ID: DES_SPI_API_230
* @param[in] Instance: SPI instance.
* @param[in] BaudRate: To set the BaudRate value.
* @return Hal_StatusType
*/
Hal_StatusType Spi_Hal_SetBaudRate(uint8 Instance, uint32 BaudRate);

#ifndef SPI_SDK_NON_EXTENDED_API
/**
* @brief This function get the SPI BaudRate.
* @note Function ID: DES_SPI_API_240
* @param[in] Instance: SPI instance.
* @return uint32:SPI BaudRate
*/
uint32 Spi_Hal_GetBaudRate(uint8 Instance);

/**
* @brief This function reset the SPI Status.
* @note Function ID: DES_SPI_API_237
* @param[in] Instance: SPI instance.
* @return void
*/
void Spi_Hal_SoftwareReset(uint8 Instance);

/**
* @brief This function set the SPI pin mode.
* @note Function ID: DES_SPI_API_238
* @param[in] Instance: SPI instance.
* @param[in] PinCfg: SPI pin mode.
* @return void
*/
void Spi_Hal_SetPinMode(uint8 Instance, Spi_PinCfgType PinCfg);

/**
* @brief This function select the SPI cs pin.
* @note Function ID: DES_SPI_API_239
* @param[in] Instance: SPI instance.
* @param[in] Pcs: SPI CS select.
* @param[in] Polarity: CS Polarity.
* @return void
*/
void Spi_Hal_SetCsPin(uint8 Instance, Spi_PcsType Pcs, Spi_PcsPolarityType Polarity);

/**
* @brief This function set the SPI Match Data.
* @note Function ID: DES_SPI_API_241
* @param[in] Instance: SPI instance.
* @param[in] Data: Data to Match.
* @param[in] Enable: Enable or Disable Data Match.
* @return void
*/
void Spi_Hal_SetMatchData(uint8 Instance, uint32 Data, boolean Enable);

/**
* @brief This function set the SPI FrameSize.
* @note Function ID: DES_SPI_API_242
* @param[in] Instance: SPI instance.
* @param[in] Framesize: Number of bits per frame, support 4~32 bits.
* @return void
*/
void Spi_Hal_SetFrameSize(uint8 Instance, uint8 Framesize);
#endif

/**
* @brief Get the SPI base.
* @note Function ID: DES_SPI_API_231
* @param[in] Instance: SPI instance.
* @return SPI_Type: SPI Base Addr.
*/
SPI_Type *Spi_Hal_GetBase(uint8 Instance);

/**
* @brief This function checks whether the status of SPI hal driver is busy.
* @note Function ID: DES_SPI_API_211
* @param[in] Instance: SPI instance.
* @return boolean
*         - TRUE: The SPI hal driver is busy.
*         - FALSE: The SPI hal driver is not busy.
*/
boolean Spi_Hal_CheckBusy(uint8 Instance);

/**
* @brief This function initializes a SPI instance.
* @note Function ID: DES_SPI_API_232
* @param[in] Instance: SPI instance.
* @param[in] ConfigPtr: The data structure containing information about a device on the SPI bus.
* @return void
*/
void Spi_Hal_ConfigureBus(uint8 Instance, const Spi_HalConfigType *ConfigPtr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /**< SPI_HAL_H */

/*====================================================EOF===========================================*/
