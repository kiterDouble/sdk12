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
* SPECIFICATION(S) : specification of DMA Driver, AUTOSAR Release 4.4.0
******************************************************************************/
/**
* @file AC784xx_Dma_Reg.h
*
* @brief  dma register interface
*
*/

#ifndef AC784XX_DMA_REG_H
#define AC784XX_DMA_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"
/*=====================================Source FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
static DMA_ChannelType *const DmaBase[DMA_CH_MAX] =
{
    DMA0_CHANNEL0, DMA0_CHANNEL1, DMA0_CHANNEL2, DMA0_CHANNEL3, DMA0_CHANNEL4, DMA0_CHANNEL5, DMA0_CHANNEL6,
    DMA0_CHANNEL7, DMA0_CHANNEL8, DMA0_CHANNEL9, DMA0_CHANNEL10, DMA0_CHANNEL11, DMA0_CHANNEL12, DMA0_CHANNEL13,
    DMA0_CHANNEL14, DMA0_CHANNEL15
};
/*============================================FUNCTION PROTOTYPES===================================*/

/**
* @brief Enable/Disable DMA channel
* @note Function ID : DES_MCL_API_160
* @param[in] ChannelId: dma channel id
* @param[in] En: Enable/Disable DMA channel
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetChannel(uint8 ChannelId, boolean En)
{
    MODIFY_REG32(DmaBase[ChannelId]->CHAN_ENABLE, DMA_CHANNEL_CHAN_ENABLE_CHAN_ENABLE_Msk, \
                 DMA_CHANNEL_CHAN_ENABLE_CHAN_ENABLE_Pos, En);
}

/**
* @brief DMA Top warm reset
* @note Function ID : DES_MCL_API_161
* @return void
*/
LOCAL_INLINE void Dma_Reg_TopWarmRst(void)
{
    MODIFY_REG32(DMA0_TOP_RST->TOP_RST, DMA_TOP_RST_WARM_RST_Msk, DMA_TOP_RST_WARM_RST_Pos, 1U);
}

/**
* @brief DMA Top Hard reset
* @note Function ID : DES_MCL_API_162
* @return void
*/
LOCAL_INLINE void Dma_Reg_TopHardRst(void)
{
    MODIFY_REG32(DMA0_TOP_RST->TOP_RST, DMA_TOP_RST_HARD_RST_Msk, DMA_TOP_RST_HARD_RST_Pos, 1U);
    MODIFY_REG32(DMA0_TOP_RST->TOP_RST, DMA_TOP_RST_HARD_RST_Msk, DMA_TOP_RST_HARD_RST_Pos, 0U);
}

/**
* @brief DMA channel warm reset.
* @note Function ID : DES_MCL_API_163
* @param[in] ChannelId: dma channel id
* @return void
*/
LOCAL_INLINE void Dma_Reg_ChannelWarmRst(uint8 ChannelId)
{
    MODIFY_REG32(DmaBase[ChannelId]->RST, DMA_CHANNEL_RST_WARM_RST_Msk, DMA_CHANNEL_RST_WARM_RST_Pos, 1U);
}

/**
* @brief DMA channel hard reset.
* @note Function ID : DES_MCL_API_164
* @param[in] ChannelId: dma channel id
* @return void
*/
LOCAL_INLINE void Dma_Reg_ChannelHardRst(uint8 ChannelId)
{
    MODIFY_REG32(DmaBase[ChannelId]->RST, DMA_CHANNEL_RST_HARD_RST_Msk, DMA_CHANNEL_RST_HARD_RST_Pos, 1U);
    MODIFY_REG32(DmaBase[ChannelId]->RST, DMA_CHANNEL_RST_HARD_RST_Msk, DMA_CHANNEL_RST_HARD_RST_Pos, 0U);
}

/**
* @brief Set DMA channel Source start address
* @note Function ID : DES_MCL_API_165
* @param[in] ChannelId: dma channel id
* @param[in] StartAddr: start address
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetSrcStartAddr(uint8 ChannelId, uint32 StartAddr)
{
    WRITE_REG32(DmaBase[ChannelId]->SSTART_ADDR, StartAddr);
}

/**
* @brief Set DMA channel Source end address
* @note Function ID : DES_MCL_API_166
* @param[in] ChannelId: dma channel id
* @param[in] EndAddr: end address
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetSrcEndAddr(uint8 ChannelId, uint32 EndAddr)
{
    WRITE_REG32(DmaBase[ChannelId]->SEND_ADDR, EndAddr);
}

/**
* @brief Set DMA channel destination start address.
* @note Function ID : DES_MCL_API_167
* @param[in] ChannelId: dma channel id
* @param[in] StartAddr: start address
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetDestStartAddr(uint8 ChannelId, uint32 StartAddr)
{
    WRITE_REG32(DmaBase[ChannelId]->DSTART_ADDR, StartAddr);
}

/**
* @brief Set DMA channel destination end address
* @note Function ID : DES_MCL_API_168
* @param[in] ChannelId: dma channel id
* @param[in] EndAddr: end address
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetDestEndAddr(uint8 ChannelId, uint32 EndAddr)
{
    WRITE_REG32(DmaBase[ChannelId]->DEND_ADDR, EndAddr);
}

/**
* @brief Set DMA channel Source transfer size
* @note Function ID : DES_MCL_API_169
* @param[in] ChannelId: dma channel id
* @param[in] TransferSize: transfer size
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetSrcTransferSize(uint8 ChannelId, uint32 TransferSize)
{
    MODIFY_REG32(DmaBase[ChannelId]->CONFIG, DMA_CHANNEL_CONFIG_SSIZE_Msk, DMA_CHANNEL_CONFIG_SSIZE_Pos, TransferSize);
}

/**
* @brief Set DMA channel destination transfer size
* @note Function ID : DES_MCL_API_170
* @param[in] ChannelId: dma channel id
* @param[in] TransferSize: transfer size
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetDstTransferSize(uint8 ChannelId, uint32 TransferSize)
{
    MODIFY_REG32(DmaBase[ChannelId]->CONFIG, DMA_CHANNEL_CONFIG_DSIZE_Msk, DMA_CHANNEL_CONFIG_DSIZE_Pos, TransferSize);
}

/**
* @brief Set DMA channel Source address offset
* @note Function ID : DES_MCL_API_171
* @param[in] ChannelId: dma channel id
* @param[in] Offset: address offset
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetSrcAddrOffSet(uint8 ChannelId, uint16 Offset)
{
    MODIFY_REG32(DmaBase[ChannelId]->ADDR_OFFSET, DMA_CHANNEL_ADDR_OFFSET_SOFFSET_Msk, \
                 DMA_CHANNEL_ADDR_OFFSET_SOFFSET_Pos, Offset);
}

/**
* @brief Set DMA channel destination address offset
* @note Function ID : DES_MCL_API_172
* @param[in] ChannelId: dma channel id
* @param[in] Offset: address offset
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetDestAddrOffSet(uint8 ChannelId, uint16 Offset)
{
    MODIFY_REG32(DmaBase[ChannelId]->ADDR_OFFSET, DMA_CHANNEL_ADDR_OFFSET_DOFFSET_Msk, \
                 DMA_CHANNEL_ADDR_OFFSET_DOFFSET_Pos, Offset);
}

/**
* @brief Set DMA channel transfer length by bytes
* @note Function ID : DES_MCL_API_173
* @param[in] ChannelId: dma channel id
* @param[in] BytesLength: address offset
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetTransferLength(uint8 ChannelId, uint32 BytesLength)
{
    MODIFY_REG32(DmaBase[ChannelId]->CHAN_LENGTH, DMA_CHANNEL_CHAN_LENGTH_CHAN_LENGTH_Msk, \
                 DMA_CHANNEL_CHAN_LENGTH_CHAN_LENGTH_Pos, BytesLength);
}

/**
* @brief Get DMA channel total transfer length by bytes
* @note Function ID : DES_MCL_API_174
* @param[in] ChannelId: dma channel id
* @return DMA channel transfer length.
*/
LOCAL_INLINE uint32 Dma_Reg_GetTransferLength(uint8 ChannelId)
{
    return (READ_REG32(DmaBase[ChannelId]->CHAN_LENGTH) & DMA_CHANNEL_CHAN_LENGTH_CHAN_LENGTH_Msk);
}

/**
* @brief Get number of bytes transfered by DMA channel
* @note Function ID : DES_MCL_API_175
* @param[in] ChannelId: dma channel id
* @return DMA channel transfered bytes number
*/
LOCAL_INLINE uint32 Dma_Reg_GetTransferedBytes(uint8 ChannelId)
{
    return (READ_REG32(DmaBase[ChannelId]->DATA_TRANS_NUM) & DMA_CHANNEL_DATA_TRANS_NUM_DATA_TRANS_NUM_Msk);
}

/**
* @brief Set DMA channel priority.
* @note Function ID : DES_MCL_API_176
* @param[in] ChannelId: dma channel id
* @param[in] Priority: DMA chanenl priority
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetChannelPriority(uint8 ChannelId, uint32 Priority)
{
    MODIFY_REG32(DmaBase[ChannelId]->CONFIG, DMA_CHANNEL_CONFIG_CHAN_PRIORITY_Msk, DMA_CHANNEL_CONFIG_CHAN_PRIORITY_Pos,
                 Priority);
}

/**
* @brief Set DMA channel circular mode
* @note Function ID : DES_MCL_API_177
* @param[in] ChannelId: dma channel id
* @param[in] En: Enable/disable DMA channel circular mode
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetCircularMode(uint8 ChannelId, boolean En)
{
    MODIFY_REG32(DmaBase[ChannelId]->CONFIG, DMA_CHANNEL_CONFIG_CHAN_CIRCULAR_Msk, DMA_CHANNEL_CONFIG_CHAN_CIRCULAR_Pos,
                 En);
}

/**
* @brief Set DMA channel FIFO move fast function
* @note Function ID : DES_MCL_API_178
* @param[in] ChannelId: dma channel id
* @param[in] En: Enable or disable move data to FIFO early
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetFIFOFastFunction(uint8 ChannelId, boolean En)
{
    MODIFY_REG32(DmaBase[ChannelId]->CONFIG, DMA_CHANNEL_CONFIG_CHAN_FIFO_Msk, DMA_CHANNEL_CONFIG_CHAN_FIFO_Pos, En);
}

/**
* @brief Get the number of the data left in the DMA channel's internal FIFO
* @note Function ID : DES_MCL_API_179
* @param[in] ChannelId: dma channel id
* @return DMA Channel FIFO left data bytes number
*/
LOCAL_INLINE uint32 Dma_Reg_GetInterFIFODataLeftBytes(uint8 ChannelId)
{
    return (READ_REG32(DmaBase[ChannelId]->FIFO_LEFT_NUM) & DMA_CHANNEL_FIFO_LEFT_NUM_FIFO_LEFT_NUM_Msk);
}

/**
* @brief Flush DMA channel data
* @note Function ID : DES_MCL_API_180
* @param[in] ChannelId: dma channel id
* @return void
*/
LOCAL_INLINE void Dma_Reg_FlushChannelFIFO(uint8 ChannelId)
{
    SET_BIT32(DmaBase[ChannelId]->RST, DMA_CHANNEL_RST_FLUSH_Msk);
}

/**
* @brief Enable/Disable DMA channel transfer finish interrupt
* @note Function ID : DES_MCL_API_181
* @param[in] ChannelId: dma channel id
* @param[in] En: Enable/disable DMA channel finish interrupt
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetFinishInterrupt(uint8 ChannelId, boolean En)
{
    MODIFY_REG32(DmaBase[ChannelId]->INTEN, DMA_CHANNEL_INTEN_FINISH_INTERRUPT_ENABLE_Msk, \
                 DMA_CHANNEL_INTEN_FINISH_INTERRUPT_ENABLE_Pos, En);
}

/**
* @brief Enable/Disable DMA channel Error interrupt
* @note Function ID : DES_MCL_API_182
* @param[in] ChannelId: dma channel id
* @param[in] En: Enable/disable DMA channel Error interrupt
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetErrorInterrupt(uint8 ChannelId, boolean En)
{
    MODIFY_REG32(DmaBase[ChannelId]->INTEN, DMA_CHANNEL_INTEN_TRANS_ERROR_INTERRUPT_ENABLE_Msk, \
                 DMA_CHANNEL_INTEN_TRANS_ERROR_INTERRUPT_ENABLE_Pos, En);
}

/**
* @brief Enable or disable DMA channel trigger
* @note Function ID : DES_MCL_API_183
* @param[in] ChannelId: dma channel id
* @param[in] En: Enable or disable DMA channel trigger
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetChannelTrigger(uint8 ChannelId, boolean En)
{
    MODIFY_REG32(DmaBase[ChannelId]->DMAMUX_CFG, DMA_CHANNEL_DMAMUX_CFG_TRIG_EN_Msk, \
                 DMA_CHANNEL_DMAMUX_CFG_TRIG_EN_Pos, En);
}

/**
* @brief Configures the DMA request for the DMA channel
* @note Function ID : DES_MCL_API_184
* @param[in] ChannelId: dma channel id
* @param[in] Source: DMA request Source
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetChannelSource(uint8 ChannelId, uint32 Source)
{
    MODIFY_REG32(DmaBase[ChannelId]->DMAMUX_CFG, DMA_CHANNEL_DMAMUX_CFG_REQ_ID_Msk, DMA_CHANNEL_DMAMUX_CFG_REQ_ID_Pos,
                 Source);
}

#if defined (AC7840E)
/**
* @brief Enable or disable DMA channel trigger
* @note Function ID : DES_MCL_API_183
* @param[in] ChannelId: dma channel id
* @param[in] En: Enable or disable DMA channel trigger
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetChannelTriggerOutSel(uint8 ChannelId, uint8 SelVal)
{
    MODIFY_REG32(DmaBase[ChannelId]->DMAMUX_CFG, DMA_CHANNEL_DMAMUX_CFG_TRIG_OUT_SEL_Msk, \
                 DMA_CHANNEL_DMAMUX_CFG_TRIG_OUT_SEL_Pos, SelVal);
}

/**
* @brief Configures the DMA request for the DMA channel
* @note Function ID : DES_MCL_API_184
* @param[in] ChannelId: dma channel id
* @param[in] Source: DMA request Source
* @return void
*/
LOCAL_INLINE void Dma_Reg_EnChannelDaisy(uint8 ChannelId, boolean Enable)
{
    MODIFY_REG32(DmaBase[ChannelId]->DMAMUX_CFG, DMA_CHANNEL_DMAMUX_CFG_TRIG_DAISY_Msk, \
    DMA_CHANNEL_DMAMUX_CFG_TRIG_DAISY_Pos, Enable);
    if (TRUE == Enable)
    {
        DMA0_TOP_RST->TOP_RST |= 0x1U << 3U;
    }
}

LOCAL_INLINE uint32 Dma_Reg_GetChannelDaisy(uint8 ChannelId)
{
    return EXTRACT_BIT32(DmaBase[ChannelId]->DMAMUX_CFG, DMA_CHANNEL_DMAMUX_CFG_TRIG_DAISY_Msk, \
                         DMA_CHANNEL_DMAMUX_CFG_TRIG_DAISY_Pos);
}

#endif

/**
* @brief Set DMA debug function
* @note Function ID : DES_MCL_API_185
* @param[in] ChannelId: dma channel id
* @param[in] En: Enable or disable debug fucntion
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetChannelDebug(uint8 ChannelId, boolean En)
{
    MODIFY_REG32(DmaBase[ChannelId]->CHAN_ENABLE, DMA_CHANNEL_CHAN_ENABLE_EDBG_Msk, DMA_CHANNEL_CHAN_ENABLE_EDBG_Pos, En);
}

/**
* @brief Set DMA channel transfer pause/resume
* @note Function ID : DES_MCL_API_186
* @param[in] ChannelId: dma channel id
* @param[in] En: Enable/disable DMA pause
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetChannelPause(uint8 ChannelId, boolean En)
{
    MODIFY_REG32(DmaBase[ChannelId]->STOP, DMA_CHANNEL_STOP_STOP_Msk, DMA_CHANNEL_STOP_STOP_Pos, En);
}

/**
* @brief get DMA channel status
* @note Function ID : DES_MCL_API_187
* @param[in] ChannelId: dma channel id
* @return DMA channel status
*/
LOCAL_INLINE uint32 Dma_Reg_GetDmaStatus(uint8 ChannelId)
{
    uint32 Status;

    Status = READ_REG32(DmaBase[ChannelId]->STATUS);

    return Status;
}

/**
* @brief clear DMA channel status
* @note Function ID : DES_MCL_API_188
* @param[in] ChannelId: dma channel id
* @return void
*/
LOCAL_INLINE void Dma_Reg_ClearDmaStatus(uint8 ChannelId)
{
    CLEAR_BIT32(DmaBase[ChannelId]->STATUS, 0xFFFFFFFFU);
}

/**
* @brief Get DMA channel circular mode
* @note Function ID : DES_MCL_API_188
* @param[in] ChannelId: dma channel id
* @return DMA channel circular mode
*/
LOCAL_INLINE uint32 Dma_Reg_GetCircularMode(uint8 ChannelId)
{
    return EXTRACT_BIT32(DmaBase[ChannelId]->CONFIG, DMA_CHANNEL_CONFIG_CHAN_CIRCULAR_Msk, \
                         DMA_CHANNEL_CONFIG_CHAN_CIRCULAR_Pos);
}

/**
* @brief Get DMA channel Source transfer size
* @note Function ID : DES_MCL_API_189
* @param[in] ChannelId: dma channel id
* @return DMA channel Source transfer size.
*/
LOCAL_INLINE uint32 Dma_Reg_GetSrcTransferSize(uint8 ChannelId)
{
    return EXTRACT_BIT32(DmaBase[ChannelId]->CONFIG, DMA_CHANNEL_CONFIG_SSIZE_Msk, DMA_CHANNEL_CONFIG_SSIZE_Pos);
}

/**
* @brief Get DMA channel destination transfer size
* @note Function ID : DES_MCL_API_190
* @param[in] ChannelId: dma channel id
* @return DMA channel destination transfer size
*/
LOCAL_INLINE uint32 Dma_Reg_GetDstTransferSize(uint8 ChannelId)
{
    return EXTRACT_BIT32(DmaBase[ChannelId]->CONFIG, DMA_CHANNEL_CONFIG_DSIZE_Msk, DMA_CHANNEL_CONFIG_DSIZE_Pos);
}

/**
* @brief Get the DMA request source
* @note Function ID : DES_MCL_API_191
* @param[in] ChannelId: dma channel id
* @return DMA request Source
*/
LOCAL_INLINE uint32 Dma_Reg_GetChannelSource(uint8 ChannelId)
{
    return EXTRACT_BIT32(DmaBase[ChannelId]->DMAMUX_CFG, DMA_CHANNEL_DMAMUX_CFG_REQ_ID_Msk, \
                         DMA_CHANNEL_DMAMUX_CFG_REQ_ID_Pos);
}

#if defined (AC7842X)  || defined (AC7843X)
/**
* @brief Enable/Disable DMA channel transfer half finish interrupt
* @note Function ID : DES_MCL_API_181
* @param[in] ChannelId: dma channel id
* @param[in] En: Enable/disable DMA channel finish interrupt
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetHalfFinishInterrupt(uint8 ChannelId, boolean En)
{
    MODIFY_REG32(DmaBase[ChannelId]->INTEN, DMA_CHANNEL_INTEN_HALF_FINISH_INTERRUPT_ENABLE_Msk, \
                 DMA_CHANNEL_INTEN_HALF_FINISH_INTERRUPT_ENABLE_Pos, En);
}
#endif /* AC7842X AC7843X */

/**
* @brief Enable/Disable DMA channel transfer half finish interrupt
* @note Function ID : DES_MCL_API_182
* @param[in] ChannelId: dma channel id
* @param[in] Irq:interrupt
* @return void
*/
LOCAL_INLINE void Dma_Reg_SetInterrupt(uint8 ChannelId, uint32 Irq)
{
    WRITE_REG32(DmaBase[ChannelId]->INTEN, Irq);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AC784XX_DMA_REG_H */
