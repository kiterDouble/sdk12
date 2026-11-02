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
 * AutoChips Inc. (C) 2023. All rights reserved.
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

 * @file Can_Hal.h
 *
 * @brief This file provides extern Can Hal API implement.
 *
 */

#ifndef CAN_HAL_H
#define CAN_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Can_Hal_Types.h"
#include "AC784xx_Can_Reg.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
/**
 * @brief Initialize a CAN hardware unit.
 * @details Initializes the specified CAN hardware unit with the provided configuration.
 * @param[in] Instance: CAN hardware unit index.
 * @param[in] ConfigPtr: Pointer to CAN hardware configuration structure.
 * @return void
 * @pre None.
 * @post The CAN hardware unit is initialized and ready for operation.
 * @note DES_CAN_API_001
 */
void Can_Hal_Init(uint8 Instance, const Can_HalConfigType *ConfigPtr);

/**
 * @brief Deinitialize a CAN hardware unit.
 * @details Deinitializes the specified CAN hardware unit and releases resources.
 * @param[in] Instance: CAN hardware unit index.
 * @return void
 * @pre The CAN hardware unit must be initialized.
 * @post The CAN hardware unit is deinitialized.
 * @note DES_CAN_API_002
 */
void Can_Hal_Deinit(uint8 Instance);

/**
 * @brief Set the work state for a CAN hardware unit.
 * @details Sets the specified CAN hardware unit to the given work state.
 * @param[in] Instance: CAN hardware unit index.
 * @param[in] State: Work state to be set.
 * @return Hal_StatusType: STATUS_SUCCESS if successful, STATUS_ERROR otherwise.
 * @pre The CAN hardware unit must be initialized.
 * @post The CAN hardware unit is set to the specified state.
 * @note DES_CAN_API_003
 */
Hal_StatusType Can_Hal_SetControllerState(uint8 Instance, Can_HalStateType State);

/**
 * @brief Get the work state of a CAN hardware unit.
 * @details Returns the current work state of the specified CAN hardware unit.
 * @param[in] Instance: CAN hardware unit index.
 * @return Can_HalStateType: Current work state of the CAN hardware unit.
 * @pre The CAN hardware unit must be initialized.
 * @post None.
 * @note DES_CAN_API_004
 */
Can_HalStateType Can_Hal_GetControllerState(uint8 Instance);

/**
 * @brief Write and send a CAN frame.
 * @details Writes the provided CAN frame information to the hardware and triggers transmission.
 * @param[in] Instance: CAN hardware unit index.
 * @param[in] BufferId: Transmit buffer index.
 * @param[in] MessagePtr: Pointer to CAN frame information to send.
 * @return Hal_StatusType: STATUS_SUCCESS if successful, STATUS_BUSY if hardware is busy.
 * @pre The CAN hardware unit must be initialized.
 * @post The CAN frame is sent if successful.
 * @note DES_CAN_API_005
 */
#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
Hal_StatusType Can_Hal_WriteTxBuffer(uint8 Instance, uint8 BufferId, const Can_MessageInfoType *MessagePtr);
#elif defined (AC7843X)
Hal_StatusType Can_Hal_WriteTxBuffer(uint8 Instance, uint8 BufferId, Can_MessageInfoType *MessagePtr);
#endif

#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
/**
 * @brief CAN frame is set to hardware if hardware is free.
 * @details Writes the provided CAN frame information to the hardware and not triggers transmission.
 * @param[in] Instance: CAN hardware unit index.
 * @param[in] BufferId: Transmit buffer index.
 * @param[in] MessagePtr: Pointer to CAN frame information to send.
 * @return Hal_StatusType: STATUS_SUCCESS if successful, STATUS_BUSY if hardware is busy.
 * @pre The CAN hardware unit must be initialized.
 * @post The CAN frame is set to hardware if successful.
 * @note DES_CAN_API_024
 */
Hal_StatusType Can_Hal_WriteTxBufferMsgSetOnly(uint8 Instance, uint8 BufferId, const Can_MessageInfoType *MessagePtr);
#endif

/**
 * @brief Read a received CAN frame from hardware.
 * @details Reads the received CAN frame information from the specified hardware buffer.
 * @param[in] Instance: CAN hardware unit index.
 * @param[in] BufferId: Receive buffer index.
 * @param[out] MessagePtr: Pointer to store received CAN frame information.
 * @return Hal_StatusType: STATUS_SUCCESS if successful, STATUS_ERROR otherwise.
 * @pre The CAN hardware unit must be initialized.
 * @post The received frame information is stored in MessagePtr.
 * @note DES_CAN_API_006
 */
Hal_StatusType Can_Hal_ReadRxBuffer(uint8 Instance, uint8 BufferId, Can_MessageInfoType *MessagePtr);

#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
/**
 * @brief Get the transmit status of a CAN buffer.
 * @details Returns the status of the specified transmit buffer.
 * @param[in] Instance: CAN hardware unit index.
 * @param[in] BufferId: Transmit buffer index.
 * @return Hal_StatusType: STATUS_SUCCESS if sent, STATUS_ERROR otherwise.
 * @pre The CAN hardware unit must be initialized.
 * @post None.
 * @note DES_CAN_API_007
 */
Hal_StatusType Can_Hal_GetTxStatus(uint8 Instance, uint8 BufferId);

#elif defined (AC7843X)
#ifndef CAN_SDK_NON_EXTENDED_API
/**
 * @brief Get the transmit status of a CAN buffer.
 * @details Returns the status of the specified transmit buffer.
 * @param[in] Instance: CAN hardware unit index.
 * @param[in] HwBufIdx: retrun by Can_Hal_WriteTxBuffer args MessagePtr->HwBufIdx.
 * @return Hal_StatusType: STATUS_SUCCESS if sent, STATUS_ERROR otherwise.
 * @pre The CAN hardware unit must be initialized.
 * @post None.
 * @note DES_CAN_API_007
 */
Hal_StatusType Can_Hal_GetTxStatus(uint8 Instance, uint8 HwBufIdx);
#endif
#endif

#ifndef CAN_SDK_NON_EXTENDED_API
/**
 * @brief Get the receive status of a CAN buffer.
 * @details Returns the status of the specified receive buffer.
 * @param[in] Instance: CAN hardware unit index.
 * @param[in] BufferId: Receive buffer index.
 * @return Hal_StatusType: STATUS_SUCCESS if received, STATUS_ERROR otherwise.
 * @pre The CAN hardware unit must be initialized.
 * @post None.
 * @note DES_CAN_API_008
 */
Hal_StatusType Can_Hal_GetRxStatus(uint8 Instance, uint8 BufferId);
#endif

/**
 * @brief Get the number of CAN transmit errors.
 * @details Returns the number of frames that encountered transmit errors.
 * @param[in] Instance: CAN hardware unit index.
 * @return uint8: Number of transmit error frames.
 * @pre The CAN hardware unit must be initialized.
 * @post None.
 * @note DES_CAN_API_009
 */
uint8 Can_Hal_GetTxErrorCount(uint8 Instance);

/**
 * @brief Get the number of CAN receive errors.
 * @details Returns the number of frames that encountered receive errors.
 * @param[in] Instance: CAN hardware unit index.
 * @return uint8: Number of receive error frames.
 * @pre The CAN hardware unit must be initialized.
 * @post None.
 * @note DES_CAN_API_010
 */
uint8 Can_Hal_GetRxErrorCount(uint8 Instance);

/**
 * @brief Get the CAN error state.
 * @details Returns the current error state of the CAN hardware unit.
 * @param[in] Instance: CAN hardware unit index.
 * @return Can_DevErrorStateType: Current CAN error state.
 * @pre The CAN hardware unit must be initialized.
 * @post None.
 * @note DES_CAN_API_011
 */
Can_DevErrorStateType Can_Hal_GetErrorState(uint8 Instance);

/**
 * @brief Get CAN error mask information.
 * @details Returns the error mask information for the CAN hardware unit.
 * @param[in] Instance: CAN hardware unit index.
 * @return uint32: CAN error mask information.
 * @pre The CAN hardware unit must be initialized.
 * @post None.
 * @note DES_CAN_API_012
 */
uint32 Can_Hal_GetErrorsInfo(uint8 Instance);

#ifndef CAN_SDK_NON_EXTENDED_API
/**
 * @brief Set CAN controller extended mode.
 * @details Configures the CAN controller to the specified extended mode.
 * @param[in] Instance: CAN hardware unit index.
 * @param[in] Mode: Extended mode to configure.
 * @return void
 * @pre The CAN hardware unit must be initialized.
 * @post The CAN controller is set to the specified mode.
 * @note DES_CAN_API_013
 */
void Can_Hal_ConfigExtendMode(uint8 Instance, Can_ExtendModeType Mode);

/**
 * @brief Configure CAN timestamp function.
 * @details Configures the timestamp functionality for the CAN hardware unit.
 * @param[in] Instance: CAN hardware unit index.
 * @param[in] tsConfigPtr: Pointer to timestamp configuration structure.
 * @return void
 * @pre The CAN hardware unit must be initialized.
 * @post Timestamp function is configured.
 * @note DES_CAN_API_014
 */
void Can_Hal_ConfigTimeStamp(uint8 Instance, const Can_TimeStampType *tsConfigPtr);

#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
/**
 * @brief Get the CAN transmit buffer complete timestamp value.
 * @details Returns the timestamp value when the transmit buffer completes sending.
 * @param[in] Instance: CAN hardware unit index.
 * @param[in] BufferId: Transmit buffer index.
 * @return uint32: Transmit complete timestamp value.
 * @pre The CAN hardware unit must be initialized.
 * @post None.
 * @note DES_CAN_API_015
 */
uint32 Can_Hal_GetTxTimeStamp(uint8 Instance, uint8 BufferId);
#endif
#endif

/**
 * @brief Get the base address of the CAN hardware unit.
 * @details Returns the pointer to the base address of the specified CAN hardware unit.
 * @param[in] Instance: CAN hardware unit index.
 * @return CAN_Type*: Pointer to CAN hardware base address.
 * @pre None.
 * @post None.
 * @note DES_CAN_API_016
 */
CAN_Type *Can_Hal_GetBase(uint8 Instance);

/**
 * @brief Abort CAN transmit operation.
 * @details Aborts the ongoing transmit operation for the specified buffer.
 * @param[in] Instance: CAN hardware unit index.
 * @param[in] BufferId: Transmit buffer index.
 * @return Hal_StatusType: STATUS_SUCCESS if abort successful, STATUS_ERROR if transmit already completed.
 * @pre The CAN hardware unit must be initialized.
 * @post The transmit operation is aborted if possible.
 * @note DES_CAN_API_017
 */
Hal_StatusType Can_Hal_AbortTransmit(uint8 Instance, uint8 BufferId);

#ifndef CAN_SDK_NON_EXTENDED_API
#if defined (AC7843X)
/**
 * @brief Read an element from the TX event FIFO.
 * @details Reads a TX event FIFO element from the CAN hardware unit.
 * @param[in] Instance: CAN hardware unit index.
 * @param[out] Msg: Pointer to store TX event FIFO element.
 * @return Hal_StatusType: STATUS_SUCCESS if read successful, STATUS_ERROR otherwise.
 * @pre The CAN hardware unit must be initialized.
 * @post The TX event FIFO element is stored in Msg if successful.
 * @note DES_CAN_API_018
 */
Hal_StatusType Can_Hal_ReadTxEvent(uint8 Instance, Can_MsgEventType *Msg);
#endif
#endif

/**
 * @brief Configure and send CAN frame information to hardware.
 * @details Configures the specified buffer with frame information and triggers transmission.
 * @param[in] Instance: CAN hardware unit index.
 * @param[in] BufferIndex: Buffer index to configure.
 * @param[in] InfoPtr: Pointer to frame information to be transmitted (must not be NULL).
 * @return void
 * @pre The CAN hardware unit must be initialized.
 * @post The frame information is configured and sent.
 * @note DES_CAN_API_019
 */
void Can_Hal_SetMsgInfo(uint8 Instance, uint8 BufferIndex, const Can_MessageInfoType *InfoPtr);

#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
/**
 * @brief Start CAN transmission action.
 * @details Starts the transmission action for the specified buffer.
 * @param[in] Instance: CAN hardware unit index.
 * @param[in] BufferId: Transmit buffer index.
 * @return void
 * @pre The CAN hardware unit must be initialized.
 * @post Transmission is started for the specified buffer.
 * @note DES_CAN_API_020
 */
void Can_Hal_StartTransmit(uint8 Instance, uint8 BufferId);
#endif

#ifndef CAN_SDK_NON_EXTENDED_API
/**
 * @brief Get CAN message information from receive buffer.
 * @details Extracts CAN message information from the specified receive buffer.
 * @param[out] InfoPtr: Pointer to store message information.
 * @param[in] Buf: Pointer to receive buffer.
 * @return void
 * @pre The receive buffer must contain valid CAN message data.
 * @post Message information is stored in InfoPtr.
 * @note DES_CAN_API_021
 */
void Can_Hal_GetMsgInfo(Can_MessageInfoType *InfoPtr, const uint32 *Buf);

/**
 * @brief Start next CAN DMA transfer.
 * @details Starts the next DMA transfer for the specified CAN hardware unit and FIFO.
 * @param[in] Instance: CAN hardware unit index.
 * @param[in] FifoId: RX FIFO index.
 * @param[in] DmaDstAddr: DMA destination address.
 * @return void
 * @pre The CAN hardware unit and DMA must be initialized.
 * @post The next DMA transfer is started.
 * @note DES_CAN_API_022
 */
void Can_Hal_StartNextDma(uint8 Instance, uint8 FifoId, uint32 DmaDstAddr);

#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
/**
 * @brief Set the amount for transmit secondary buffer.
 * @details Sets the number of secondary buffers to be used for transmission.
 * @param[in] Instance: CAN hardware unit index.
 * @param[in] amount: Transmit secondary buffer amount.
 * @return void
 * @pre The CAN hardware unit must be initialized.
 * @post The secondary buffer amount is set for transmission.
 * @note DES_CAN_API_023
 */
void Can_Hal_SetTxSecAmount(uint8 Instance, Can_TxSecAmountType amount);
#endif
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
