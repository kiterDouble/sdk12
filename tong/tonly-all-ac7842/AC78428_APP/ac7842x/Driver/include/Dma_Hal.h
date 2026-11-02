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
/******************************************************************************
* SPECIFICATION(S) : specification of DIO Driver, AUTOSAR Release 4.4.0
******************************************************************************/
/**
* @file Dma_Hal.h
*
* @brief dma hal interface for SDK
*
*/

#ifndef DMA_HAL_H
#define DMA_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "AC784xx_Dma_Reg.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/**
 * @brief Dma channel interrupt callback type.
 */
typedef struct
{
    uint8 ChannelId; /*!< Dma channel id */
    uint32 DmaEvent; /*!< DMA finish or error event, use DMA_FINISH_EVENT or DMA_ERROR_EVENT  to parse event*/
    void *UserArgs; /*!< User-defined callback parameters */
} Dma_ChannelCBInfoType;

/*PRQA S 3630 ++ # AMCE_QAC_3630*/
/**
 * @brief Dma channel address type.
 */
typedef struct
{
    uint16 SrcOffset; /*!< Source address offset */
    uint16 DestOffset; /*!< Destination address offset */
#if defined (AC7840E)
    uint32 Length; /*!< Transfer length */
#else
    uint16 Length; /*!< Transfer length */
#endif
    uint32 SrcStartAddr; /*!< Transfer source start address */
    uint32 DestStartAddr; /*!< Transfer destination start address */
    uint32 SrcEndAddr; /*!< Transfer source end address*/
    uint32 DestEndAddr; /*!< Transfer destination end address */
} Dma_ChannelAddrType;

/**
 * @brief The user configuration structure for the an DMA channel.
 */
typedef struct
{
    boolean TriggerMode; /*!< Enable/Disable the trigge mode  */
    boolean CircularMode; /*!< Enable/Disable the circular mode */
#if defined (AC7840E)
    boolean DaisyMode;
    Dma_DaisyTriggerType DaisyTrigger; /**< Daisy Trigger select*/
#endif
    uint8 IrqSrc; /*!< Enable/Disable irq, half finish irq only valid for 7842x and 7843x */
    uint16 SrcOffset; /*!< Source address offset */
    uint16 DestOffset; /*!< Destination address offset*/
#if defined (AC7840E)
    uint32 Length; /*!< Transfer length */
#else
    uint16 Length; /*!< Transfer length */
#endif
    Dma_TransferUnitType SrcUnit; /*!< DMA channel source transfer unit size */
    Dma_TransferUnitType DestUnit; /*!< DMA channel destination transfer unit size */
    Dma_TransferType Type; /*!< DMA channel transfer type */
    uint32 SrcStartAddr; /*!< Transfer source start address */
    uint32 DestStartAddr; /*!< Transfer destination start address */
    uint32 SrcEndAddr; /*!< Transfer source end address */
    uint32 DestEndAddr; /*!< Transfer destination end address */
    Hal_CallbackType Callback; /*!< iUser-defined callback function */
    void *UserArgs; /*!< User-defined callback parameters */
} Dma_TransferConfigType;

/**
 * @brief The user configuration structure for the an DMA driver channel.
 */
typedef struct
{
    uint8 HwChannelId; /**< Dma channel ID*/
    uint8 VirtualChannelId; /**< Dma channel ID*/
    Dma_ChannelPriorityType Priority; /**< DMA channel priority  */
    Dma_RequestSourceType ReqSrc; /**< DMA channel transfer type */
} Dma_ChannelConfigType;

/**
 * @brief The user configuration structure for the an DMA driver channel.
 */
typedef struct
{
    uint8 ChannelCnt; /**< Dma channel counter*/
    const Dma_ChannelConfigType *ChannelCfg; /**< DMA channel cfg */
} Dma_ConfigType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/
/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
 * @brief Initializes the DMA module.
 * @note  Function ID : DES_MCL_API_101
 * @param[in] ConfigPtr: The pointer to the DMA module Configed structure list.
 * @return void
 */
void Dma_Hal_Init(const Dma_ConfigType *ConfigPtr);

/**
* @brief De-initializes the DMA module.
* @note  Function ID : DES_MCL_API_102
* @return void
*/
void Dma_Hal_Deinit(void);

/**
 * @brief Configures data transfer with DMA
 * @note  Function ID : DES_MCL_API_103
 * @param[in] ChannelId: Dma channel id
 * @param[in] ConfigPtr: DMA transfer configration info of Channel
 * @return STATUS_UNSUPPORTED means fail or STATUS_SUCCESS means success.
 */
Hal_StatusType Dma_Hal_ConfigCh(uint8 ChannelId, const Dma_TransferConfigType *ConfigPtr);

/**
 * @brief Get DMA channel Status
 * @note  Function ID : DES_MCL_API_104
 * @param[in] ChannelId: Dma channel id
 * @return enum Dma_ChannelStatusType.
 *               STATUS_IDLE
 *               STATUS_ABORT
 *               STATUS_BUSY
 *               STATUS_CONFIG
 */
Hal_StatusType Dma_Hal_GetChStatus(uint8 ChannelId);

/**
 * @brief Starts the DMA channel.
 * @note  Function ID : DES_MCL_API_105
 * @param[in] ChannelId: dma channel id
 * @return Hal_StatusType: start success or not
 */
Hal_StatusType Dma_Hal_StartCh(uint8 ChannelId);

/**
 * @brief Stops the DMA channel.
 * @note  Function ID : DES_MCL_API_106
 * @param[in] ChannelId: dma channel id
 * @return Hal_StatusType: stop success or not
 */
Hal_StatusType Dma_Hal_StopCh(uint8 ChannelId);

/**
 * @brief Get the transferred bytes by DMA.
 * @note  Function ID : DES_MCL_API_107
 * @param[in] ChannelId: dma channel id
 * @return The transferred bytes to be transferred by DMA
 */
uint32 Dma_Hal_GetTransBytes(uint8 ChannelId);

/**
 * @brief Update Channel source/destination address and offset
 * @note  Function ID : DES_MCL_API_108
 * @param[in] ChannelId: dma channel id
 * @param[in] ChannelAddr: the pointer to Dma_ChannelAddrType structure
 * @return void
 */
void Dma_Hal_UpdateChAddr(uint8 ChannelId, const Dma_ChannelAddrType *ChannelAddr);

/**
 * @brief disable/enable the channel interrupt.
 * @note  Function ID : DES_MCL_API_109
 * @param[in] ChannelId: dma channel id
 * @param[in] IrqSrc: Enable/Disable irq, half finish irq only valid for 7842x and 7843x
 * @return void
 */
void Dma_Hal_EnableChIrq(uint8 ChannelId, uint8 IrqSrc);

/**
 * @brief Dma channel reset.
 * @note  Function ID : DES_MCL_API_110
 * @param[in] ChannelId: dma channel id
 * @param[in] Reset: reset type
 * @return void
 */
void Dma_Hal_ChReset(uint8 ChannelId, Dma_ChannelResetType Reset);

/**
 * @brief Get dma channel id by module dma request source id
 * @note  Function ID : DES_MCL_API_112
 * @param[in] ReqSrc: dma request source id
 * @return uint8: dma channel id
 */
uint8 Dma_Hal_GetChIdByReqSrc(Dma_RequestSourceType ReqSrc);

/**
 * @brief DMA0_Channel0 Interrupt Handler Function
 * @note  Function ID : DES_MCL_API_113
 * @return void
 */
ISR(DMA0_Channel0_IRQHandler);

/**
 * @brief DMA0_Channel1 Interrupt Handler Function
 * @note  Function ID : DES_MCL_API_114
 * @return void
 */
ISR(DMA0_Channel1_IRQHandler);

/**
 * @brief DMA0_Channel2 Interrupt Handler Function
 * @note  Function ID : DES_MCL_API_115
 * @return void
 */
ISR(DMA0_Channel2_IRQHandler);

/**
 * @brief DMA0_Channel3 Interrupt Handler Function
 * @note  Function ID : DES_MCL_API_116
 * @return void
 */
ISR(DMA0_Channel3_IRQHandler);

/**
 * @brief DMA0_Channel4 Interrupt Handler Function
 * @note  Function ID : DES_MCL_API_117
 * @return void
 */
ISR(DMA0_Channel4_IRQHandler);

/**
 * @brief DMA0_Channel5 Interrupt Handler Function
 * @note  Function ID : DES_MCL_API_118
 * @return void
 */
ISR(DMA0_Channel5_IRQHandler);

/**
 * @brief DMA0_Channel6 Interrupt Handler Function
 * @note  Function ID : DES_MCL_API_119
 * @return void
 */
ISR(DMA0_Channel6_IRQHandler);

/**
 * @brief DMA0_Channel7 Interrupt Handler Function
 * @note  Function ID : DES_MCL_API_120
 * @return void
 */
ISR(DMA0_Channel7_IRQHandler);

/**
 * @brief DMA0_Channel8 Interrupt Handler Function
 * @note  Function ID : DES_MCL_API_121
 * @return void
 */
ISR(DMA0_Channel8_IRQHandler);

/**
 * @brief DMA0_Channel9 Interrupt Handler Function
 * @note  Function ID : DES_MCL_API_122
 * @return void
 */
ISR(DMA0_Channel9_IRQHandler);

/**
 * @brief DMA0_Channel10 Interrupt Handler Function
 * @note  Function ID : DES_MCL_API_123
 * @return void
 */
ISR(DMA0_Channel10_IRQHandler);

/**
 * @brief DMA0_Channel11 Interrupt Handler Function
 * @note  Function ID : DES_MCL_API_124
 * @return void
 */
ISR(DMA0_Channel11_IRQHandler);

/**
 * @brief DMA0_Channel12 Interrupt Handler Function
 * @note  Function ID : DES_MCL_API_125
 * @return void
 */
ISR(DMA0_Channel12_IRQHandler);

/**
 * @brief DMA0_Channel13 Interrupt Handler Function
 * @note  Function ID : DES_MCL_API_126
 * @return void
 */
ISR(DMA0_Channel13_IRQHandler);

/**
 * @brief DMA0_Channel14 Interrupt Handler Function
 * @note  Function ID : DES_MCL_API_127
 * @return void
 */
ISR(DMA0_Channel14_IRQHandler);

/**
 * @brief DMA0_Channel15 Interrupt Handler Function
 * @note  Function ID : DES_MCL_API_128
 * @return void
 */
ISR(DMA0_Channel15_IRQHandler);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DMA_HAL_H */
