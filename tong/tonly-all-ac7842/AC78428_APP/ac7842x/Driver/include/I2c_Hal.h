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

/*!
 * @file I2c_Hal.h
 * @brief This file provides cdd I2c hal function extern
 */

#ifndef I2C_HAL_H
#define I2C_HAL_H

#ifdef __cplusplus
extern "C"
{
#endif
/*==============================================INCLUDE FILES=======================================*/
#include "I2c_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
* @brief Initializes the I2C module.
* @note Function ID:DES_I2C_API_200
* @note Service ID: NA
* @param[in] Instance : I2C hardware channel ID
* @param[in] ChannelConfigPtr : Pointer to I2c_ChannelConfigType
* @return void
*/
void I2c_Hal_Init
(
    uint8 Instance,
    const I2c_Hal_ChannelConfigType *ChannelConfigPtr
);

/**
* @brief DeInitializes the I2C module.
* @note Function ID:DES_I2C_API_201
* @note Service ID: NA
* @param[in] Instance : I2C hardware channel ID
* @return void
*/
void I2c_Hal_DeInit
(
    uint8 Instance
);

/**
* @brief Get the currently configured baud rate.
* @note Function ID: DES_I2C_API_240
* @note Service ID: NA
* @param[in] Instance : I2C hardware channel ID
* @return uint32:Get the currently baud rate
*
*/
uint32 I2c_Hal_MasterGetBaudRate
(
    uint8 Instance
);

/**
* @brief Set the currently configured baud rate.
* @note Function ID: DES_I2C_API_204
* @note Service ID: NA
* @param[in] Instance : I2C hardware channel ID
* @param[in] BaudRate: I2C channel Set baudRate
* @return Hal_StatusType: STATUS_SUCCESS  Set the currently configured baud rate. Success
*                         STATUS_ERROR Set the currently configured baud rate Failed
*/
Hal_StatusType I2c_Hal_MasterSetBaudRate
(
    uint8 Instance,
    uint32 BaudRate
);

/**
* @brief Starts an synchronous transmission on the I2C bus.
* @note Function ID:DES_I2C_API_241
* @note Service ID: NA
* @param[in] Instance : I2C hardware channel ID
* @param[inout] DataTransmitPtr: transmit data used
* @return Hal_StatusType: STATUS_SUCCESS:  Success to Send or receive
*                         STATUS_ERROR: Otherwise
*/
Hal_StatusType I2c_Hal_SyncTransceive
(
    uint8 Instance,
    const DataTransmitType *DataTransmitPtr
);

/**
* @brief Starts an asynchronous transmission on the I2C bus.
* @note Function ID:DES_I2C_API_202
* @note Service ID: NA
* @param[in] Instance : I2C hardware channel ID
* @param[inout] DataTransmitPtr: transmit data used
* @return Hal_StatusType: STATUS_SUCCESS:  Success to Send or receive
*                         STATUS_ERROR: Otherwise
*/
Hal_StatusType I2c_Hal_AsyncTransceive
(
    uint8 Instance,
    const DataTransmitType *DataTransmitPtr
);

/**
* @brief Starts an asynchronous transmission on the I2C bus.
* @note Function ID:DES_I2C_API_242
* @note Service ID: NA
* @param[in] Instance : I2C hardware channel ID
* @return Hal_StatusType: STATUS_SUCCESS:  Success to Send or receive
*                         STATUS_ERROR: Otherwise
*/
Hal_StatusType I2c_Hal_AbortTransceive
(
    uint8 Instance
);

/**
* @brief Gets the status of an I2C channel.
* @note Function ID:DES_I2C_API_203
* @note Service ID: NA
* @param[in] Instance : I2C hardware channel ID
* @return I2c_ChannelStatusType: I2c channel currenr status
*/
I2c_Hal_ChannelStatusType I2c_Hal_GetStatus
(
    uint8 Instance
);

/**
* @brief Get the I2C base.
* @note Function ID:DES_I2C_API_243
* @param[in] Instance : I2C hardware channel ID.
* @return I2C_Type*: the I2C base addr.
*/
I2C_Type *I2c_Hal_GetBase
(
    uint8 Instance
);

/*!
 * @brief Provide a buffer for transmitting data.
 *
 * @param[in] Instance: The I2C instance number
 * @param[in] TxBuff: Pointer to the data to be transferred
 * @param[in] TxSize: Length of the data to be transferred
 * @return The result of execution
 */
#ifndef I2C_SDK_NON_EXTENDED_API
Hal_StatusType I2C_Hal_SlaveSetTxBuffer
(
    uint8 Instance,
    const uint8 *TxBuff,
    uint32 TxSize
);

/*!
 * @brief Provide a buffer for receiving data.
 *
 * @param[in] Instance: The I2C instance number
 * @param[in] RxBuff: Pointer to the buffer where to store received data
 * @param[in] RxSize: Length of the data to be transferred
 * @return The result of execution
 */
Hal_StatusType I2C_Hal_SlaveSetRxBuffer
(
    uint8 Instance,
    uint8 *RxBuff,
    uint32 RxSize
);

/*!
 * @brief Get the listening receiving data.
 *
 * @param[in] Instance: The I2C instance number
 * @return The receive data len
 */
uint32 I2C_Hal_SlaveGetRxSize
(
    uint8 Instance
);
#endif

#ifdef __cplusplus
}
#endif

#endif /* CDD_I2c_Hal_H */
/*============================================EOF===================================*/
