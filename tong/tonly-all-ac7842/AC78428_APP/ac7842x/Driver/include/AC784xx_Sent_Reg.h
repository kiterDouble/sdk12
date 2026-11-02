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
 * @file AC784xx_Sent_Reg.h
 *
 * @brief This file provides sent hardware access functions.
 *
 */

#ifndef AC784XX_SENT_REG_H
#define AC784XX_SENT_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"
#include "Sent_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
#define IRQ_STATUS_MASK (SENT_CHANNEL_INTSTAT_RSI_Msk | SENT_CHANNEL_INTSTAT_RDI_Msk \
                         | SENT_CHANNEL_INTSTAT_RFI_Msk| SENT_CHANNEL_INTSTAT_TDI_Msk \
                         | SENT_CHANNEL_INTSTAT_TBI_Msk | SENT_CHANNEL_INTSTAT_FRI_Msk \
                         | SENT_CHANNEL_INTSTAT_FDI_Msk | SENT_CHANNEL_INTSTAT_NNI_Msk \
                         | SENT_CHANNEL_INTSTAT_NVI_Msk | SENT_CHANNEL_INTSTAT_CRCI_Msk \
                         | SENT_CHANNEL_INTSTAT_WSI_Msk | SENT_CHANNEL_INTSTAT_SDI_Msk \
                         | SENT_CHANNEL_INTSTAT_SCRI_Msk | SENT_CHANNEL_INTSTAT_WDI_Msk \
                         | SENT_CHANNEL_INTSTAT_ESI_Msk | SENT_CHANNEL_INTSTAT_HFLL_Msk \
                         | SENT_CHANNEL_INTSTAT_FDONE_Msk)

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
* @brief Get SENT base.
*
* @param[in] Instance : SENT hardware channel ID.
* @return SENT_Type*: the SENT base addr.
*/
LOCAL_INLINE SENT_CHANNEL_Type *Sent_Reg_GetBase(uint8 Instance)
{
    DEVICE_ASSERT(Instance < SENT_INSTANCE_MAX);
    SENT_CHANNEL_Type *const SentBase[SENT_INSTANCE_MAX] =
    {
        SENT_CHANNEL0, SENT_CHANNEL1,
#if defined (AC7843X)
        SENT_CHANNEL2, SENT_CHANNEL3
#endif
    };

    return SentBase[Instance];
}

/*!
 * @brief SENT set channel enable bit.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Enable: channel enable or disable
 * @return void
 */
LOCAL_INLINE void Sent_Reg_EnableChannel(SENT_CHANNEL_Type *Base, boolean Enable)
{
    if (TRUE == Enable)
    {
        SET_BIT32(Base->RCR, SENT_CHANNEL_RCR_CHEN_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->RCR, SENT_CHANNEL_RCR_CHEN_Msk);
    }
}

/*!
 * @brief SENT set channel pause pulse receive enable bit.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Enable: pause pulse enable or disable
 * @return void
 */
LOCAL_INLINE void Sent_Reg_EnablePausePulse(SENT_CHANNEL_Type *Base, boolean Enable)
{
    if (TRUE == Enable)
    {
        SET_BIT32(Base->RCR, SENT_CHANNEL_RCR_PAUSE_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->RCR, SENT_CHANNEL_RCR_PAUSE_Msk);
    }
}

/*!
 * @brief SENT set channel alternate CRC enable bit.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Enable: Alternate CRC enable or disable
 * @return void
 */
LOCAL_INLINE void Sent_Reg_EnableAlternateCRC(SENT_CHANNEL_Type *Base, boolean Enable)
{
    if (TRUE == Enable)
    {
        SET_BIT32(Base->RCR, SENT_CHANNEL_RCR_ACE_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->RCR, SENT_CHANNEL_RCR_ACE_Msk);
    }
}

/*!
 * @brief SENT set channel status nibble in CRC enable bit.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Enable: status nibble in CRC enable or disable
 * @return void
 */
LOCAL_INLINE void Sent_Reg_EnableStatusNibbleInCRC(SENT_CHANNEL_Type *Base, boolean Enable)
{
    if (TRUE == Enable)
    {
        SET_BIT32(Base->RCR, SENT_CHANNEL_RCR_SNI_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->RCR, SENT_CHANNEL_RCR_SNI_Msk);
    }
}

/*!
 * @brief SENT set serial message process enable bit.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Enable: serial message process enable or disable
 * @return void
 */
LOCAL_INLINE void Sent_Reg_EnableSerialMsgProcess(SENT_CHANNEL_Type *Base, boolean Enable)
{
    if (TRUE == Enable)
    {
        SET_BIT32(Base->RCR, SENT_CHANNEL_RCR_SPEN_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->RCR, SENT_CHANNEL_RCR_SPEN_Msk);
    }
}

/*!
 * @brief SENT set serial message CRC enable bit.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Enable: serial message CRC enable or disable
 * @return void
 */
LOCAL_INLINE void Sent_Reg_EnableSerialMsgCRC(SENT_CHANNEL_Type *Base, boolean Enable)
{
    if (TRUE == Enable)
    {
        SET_BIT32(Base->RCR, SENT_CHANNEL_RCR_SCRC_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->RCR, SENT_CHANNEL_RCR_SCRC_Msk);
    }
}

/*!
 * @brief SENT set fast message CRC enable bit.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Enable: fast message CRC enable or disable
 * @return void
 */
LOCAL_INLINE void Sent_Reg_EnableFastMsgCRC(SENT_CHANNEL_Type *Base, boolean Enable)
{
    if (TRUE == Enable)
    {
        SET_BIT32(Base->RCR, SENT_CHANNEL_RCR_FCRC_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->RCR, SENT_CHANNEL_RCR_FCRC_Msk);
    }
}

/*!
 * @brief SENT set drift error enable bit.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Enable: drift error enable or disable
 * @return void
 */
LOCAL_INLINE void Sent_Reg_DisableDriftError(SENT_CHANNEL_Type *Base, boolean Enable)
{
    if (TRUE == Enable)
    {
        SET_BIT32(Base->RCR, SENT_CHANNEL_RCR_DED_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->RCR, SENT_CHANNEL_RCR_DED_Msk);
    }
}

/*!
 * @brief SENT set fast CRC augument zero enable bit.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Enable: CRC augument zero enable or disable
 * @return void
 */
LOCAL_INLINE void Sent_Reg_EnableCRCAugumentZero(SENT_CHANNEL_Type *Base, boolean Enable)
{
    if (TRUE == Enable)
    {
        SET_BIT32(Base->RCR, SENT_CHANNEL_RCR_CRZ_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->RCR, SENT_CHANNEL_RCR_CRZ_Msk);
    }
}

/*!
 * @brief SENT set FDFL mode enable bit.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Enable: FDFL mode enable or disable
 * @return void
 */
LOCAL_INLINE void Sent_Reg_EnableFDFLMode(SENT_CHANNEL_Type *Base, boolean Enable)
{
    if (TRUE == Enable)
    {
        SET_BIT32(Base->RCR, SENT_CHANNEL_RCR_FDFL_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->RCR, SENT_CHANNEL_RCR_FDFL_Msk);
    }
}

/*!
 * @brief SENT get channel state
 *
 * @param[in] Base: SENT base pointer
 * @return channel status
 */
LOCAL_INLINE Sent_ChannelStatusType Sent_Reg_GetChannelStatus(const SENT_CHANNEL_Type *Base)
{
    uint32 status;

    status = (Base->RSR & SENT_CHANNEL_RSR_CSTATUS_Msk) >> SENT_CHANNEL_RSR_CSTATUS_Pos;
    /*PRQA S 4342 ++ # AMCE_SENT_4342. */
    return (Sent_ChannelStatusType)status;
    /*PRQA S 4342 -- */
}

/*!
 * @brief SENT get enhanced serial msg config type.
 *
 * @param[in] Base: SENT channel base pointer
 * @return enhanced serial msg config type
 */
LOCAL_INLINE Sent_EnhancedConfigType Sent_Reg_GetEnhancedConfigType(const SENT_CHANNEL_Type *Base)
{
    uint32 config;

    config = (Base->SDS & SENT_CHANNEL_SDS_CON_Msk) >> SENT_CHANNEL_SDS_CON_Pos;
    /*PRQA S 4342 ++ # AMCE_SENT_4342. */
    return (Sent_EnhancedConfigType)config;
    /*PRQA S 4342 -- */
}

/*!
 * @brief SENT set output invert enable bit.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Enable: output invert enable or disable
 * @return void
 */
LOCAL_INLINE void Sent_Reg_EnableOutputInvert(SENT_CHANNEL_Type *Base, boolean Enable)
{
    if (TRUE == Enable)
    {
        SET_BIT32(Base->IOCR, SENT_CHANNEL_IOCR_OIE_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->IOCR, SENT_CHANNEL_IOCR_OIE_Msk);
    }
}

/*!
 * @brief SENT set input invert enable bit.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Enable: input invert enable or disable
 * @return void
 */
LOCAL_INLINE void Sent_Reg_EnableInputInvert(SENT_CHANNEL_Type *Base, boolean Enable)
{
    if (TRUE == Enable)
    {
        SET_BIT32(Base->IOCR, SENT_CHANNEL_IOCR_IIE_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->IOCR, SENT_CHANNEL_IOCR_IIE_Msk);
    }
}

/*!
 * @brief SENT get irq status
 *
 * @param[in] Base: SENT channel base pointer
 * @return irq status
 */
LOCAL_INLINE uint32 Sent_Reg_GetIrqStatus(const SENT_CHANNEL_Type *Base)
{
    return (Base->INTSTAT) & IRQ_STATUS_MASK;
}

/*!
 * @brief SENT set irq status
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Irq: irq status
 * @return void
 */
LOCAL_INLINE void Sent_Reg_SetIrqStatus(SENT_CHANNEL_Type *Base, uint32 Irq)
{
    WRITE_REG32(Base->INTSET, Irq & IRQ_STATUS_MASK);
}

/*!
 * @brief SENT clear irq status
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Irq: irq status
 * @return void
 */
LOCAL_INLINE void Sent_Reg_ClearIrqStatus(SENT_CHANNEL_Type *Base, uint32 Irq)
{
    WRITE_REG32(Base->INTCLR, Irq & IRQ_STATUS_MASK);
}

/*!
 * @brief SENT set irq enable bit
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Irq: Irq enable bit
 * @return void
 */
LOCAL_INLINE void Sent_Reg_EnableIrq(SENT_CHANNEL_Type *Base, uint32 Irq)
{
    WRITE_REG32(Base->INTEN, Irq & IRQ_STATUS_MASK);
}

/*!
 * @brief SENT set fifo flush status.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Enable: fifo flush enable or disable
 * @return void
 */
LOCAL_INLINE void Sent_Reg_EnableFlushFifo(SENT_CHANNEL_Type *Base, boolean Enable)
{
    if (TRUE == Enable)
    {
        SET_BIT32(Base->FIFO, SENT_CHANNEL_FIFO_FLUSH_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->FIFO, SENT_CHANNEL_FIFO_FLUSH_Msk);
    }
}

/*!
 * @brief SENT set moulde clock divider
 *
 * @param[in] Base: SENT module base pointer
 * @param[in] Step: divider value
 */
LOCAL_INLINE void Sent_Reg_SetModuleClkDivider(SENT_CTRL_Type *Base, uint16 Step)
{
    MODIFY_REG32(Base->FDR,
                 SENT_CTRL_FDR_STEP_Msk,
                 SENT_CTRL_FDR_STEP_Pos,
                 Step);
}

/*!
 * @brief SENT get moulde clock divider
 *
 * @param[in] Base: SENT module base pointer
 * @return module clock divider vaule
 */
LOCAL_INLINE uint16 Sent_Reg_GetModuleClkDivider(const SENT_CTRL_Type *Base)
{
    return (uint16)((Base->FDR & SENT_CTRL_FDR_STEP_Msk) >> SENT_CTRL_FDR_STEP_Pos);
}

/*!
 * @brief SENT get module timestamp vaule
 *
 * @param[in] Base: SENT module base pointer
 * @return module timestamp vaule
 */
LOCAL_INLINE uint32 Sent_Reg_GetCurrentTimeStamp(const SENT_CTRL_Type *Base)
{
    return ((Base->CTS & SENT_CTRL_CTS_CTS_Msk) >> SENT_CTRL_CTS_CTS_Pos);
}

/*!
 * @brief SENT set moulde timestamp divider
 *
 * @param[in] Base: SENT module base pointer
 * @param[in] Step: divider value
 * @return void
 */
LOCAL_INLINE void Sent_Reg_SetModuleTimeStampDivider(SENT_CTRL_Type *Base, uint32 Step)
{
    MODIFY_REG32(Base->TDIV,
                 SENT_CTRL_TDIV_TDIV_Msk,
                 SENT_CTRL_TDIV_TDIV_Pos,
                 Step);
}

/*!
 * @brief SENT set channel pre divider
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Step: divider value
 * @return void
 */
LOCAL_INLINE void Sent_Reg_SetChannelPreDivider(SENT_CHANNEL_Type *Base, uint16 Step)
{
    MODIFY_REG32(Base->CPDR,
                 SENT_CHANNEL_CPDR_PDIV_Msk,
                 SENT_CHANNEL_CPDR_PDIV_Pos,
                 Step);
}

/*!
 * @brief SENT set channel fractional divider divider
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Step: divider value
 * @return void
 */
LOCAL_INLINE void Sent_Reg_SetChannelFractionalDivider(SENT_CHANNEL_Type *Base, uint16 Step)
{
    MODIFY_REG32(Base->CFDR,
                 SENT_CHANNEL_CFDR_DIV_Msk,
                 SENT_CHANNEL_CFDR_DIV_Pos,
                 Step);
}

/*!
 * @brief SENT set channel frame check mode
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Mode: frame check mode
 * @return void
 */
LOCAL_INLINE void Sent_Reg_SetFrameCheckMode(SENT_CHANNEL_Type *Base, Sent_FrameCheckModeType Mode)
{
    MODIFY_REG32(Base->RCR,
                 SENT_CHANNEL_RCR_FRAME_CHECK_Msk,
                 SENT_CHANNEL_RCR_FRAME_CHECK_Pos,
                 Mode);
}

/*!
 * @brief SENT set fast channel frame length
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Length: frame length
 * @return void
 */
LOCAL_INLINE void Sent_Reg_SetFrameLength(SENT_CHANNEL_Type *Base, uint8 Length)
{
    MODIFY_REG32(Base->RCR,
                 SENT_CHANNEL_RCR_FRAME_LENGTH_Msk,
                 SENT_CHANNEL_RCR_FRAME_LENGTH_Pos,
                 Length);
}

/*!
 * @brief SENT set serial message type
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Type: serial message type
 * @return void
 */
LOCAL_INLINE void Sent_Reg_SetSerialMsgType(SENT_CHANNEL_Type *Base, Sent_SerialMsgType Type)
{
    MODIFY_REG32(Base->RCR,
                 SENT_CHANNEL_RCR_SMOD_Msk,
                 SENT_CHANNEL_RCR_SMOD_Pos,
                 Type);
}

/*!
 * @brief SENT get frame status nibbble
 *
 * @param[in] Base: SENT channel base pointer
 * @return frame status nibbble
 */
LOCAL_INLINE uint8 Sent_Reg_GetFrameStatusNibble(const SENT_CHANNEL_Type *Base)
{
    return (uint8)((Base->RSR & SENT_CHANNEL_RSR_FSTATUS_Msk) >> SENT_CHANNEL_RSR_FSTATUS_Pos);
}

/*!
 * @brief SENT get frame CRC nibbble
 *
 * @param[in] Base: SENT channel base pointer
 * @return frame CRC nibbble
 */
LOCAL_INLINE uint8 Sent_Reg_GetFrameCRCNibble(const SENT_CHANNEL_Type *Base)
{
    return (uint8)((Base->RSR & SENT_CHANNEL_RSR_CRC_Msk) >> SENT_CHANNEL_RSR_CRC_Pos);
}

/*!
 * @brief SENT get frame length counter
 *
 * @param[in] Base: SENT channel base pointer
 * @return frame length counter, including pause nibble
 */
LOCAL_INLINE uint32 Sent_Reg_GetFrameLengthCounter(const SENT_CHANNEL_Type *Base)
{
    return ((Base->RSR & SENT_CHANNEL_RSR_FRLEN_Msk) >> SENT_CHANNEL_RSR_FRLEN_Pos);
}

/*!
 * @brief SENT get serial message CRC value
 *
 * @param[in] Base: SENT channel base pointer
 * @return serial message CRC value
 */
LOCAL_INLINE uint8 Sent_Reg_GetSerialMsgCRC(const SENT_CHANNEL_Type *Base)
{
    return (uint8)((Base->SDS & SENT_CHANNEL_SDS_SCRC_Msk) >> SENT_CHANNEL_SDS_SCRC_Pos);
}

/*!
 * @brief SENT get serial message ID
 *
 * @param[in] Base: SENT channel base pointer
 * @return serial message ID
 */
LOCAL_INLINE uint8 Sent_Reg_GetSerialMsgID(const SENT_CHANNEL_Type *Base)
{
    return (uint8)((Base->SDS & SENT_CHANNEL_SDS_MID_Msk) >> SENT_CHANNEL_SDS_MID_Pos);
}

/*!
 * @brief SENT get serial message data
 *
 * @param[in] Base: SENT channel base pointer
 * @return serial message message data
 */
LOCAL_INLINE uint16 Sent_Reg_GetSerialMsgData(const SENT_CHANNEL_Type *Base)
{
    return (uint16)((Base->SDS & SENT_CHANNEL_SDS_SD_Msk) >> SENT_CHANNEL_SDS_SD_Pos);
}

/*!
 * @brief SENT set input data source.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Input: sent input data source 0~1
 * @return void
 */
LOCAL_INLINE void Sent_Reg_SelectInputDataSource(SENT_CHANNEL_Type *Base, Sent_InputDataSelectType Input)
{
    MODIFY_REG32(Base->IOCR,
                 SENT_CHANNEL_IOCR_INSEL_Msk,
                 SENT_CHANNEL_IOCR_INSEL_Pos,
                 Input);
}

/*!
 * @brief SENT set glitch filter depth
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Depth: glitch filter depth, received data is valid if it crosses the
                     filter depth
 * @return void
 */
LOCAL_INLINE void Sent_Reg_SetGlitchFilterDepth(SENT_CHANNEL_Type *Base, uint8 Depth)
{
    MODIFY_REG32(Base->IOCR,
                 SENT_CHANNEL_IOCR_DEPTH_Msk,
                 SENT_CHANNEL_IOCR_DEPTH_Pos,
                 Depth);
}

/*!
 * @brief SENT set external trigger source.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Trigger: external trigger source 0~3
 * @return void
 */
LOCAL_INLINE void Sent_Reg_SelectExternalTriggerSource(SENT_CHANNEL_Type *Base, Sent_ExternalTriggerType Trigger)
{
    MODIFY_REG32(Base->IOCR,
                 SENT_CHANNEL_IOCR_ETS_Msk,
                 SENT_CHANNEL_IOCR_ETS_Pos,
                 Trigger);
}

/*!
 * @brief SENT get SPC transfer state
 *
 * @param[in] Base: SENT channel base pointer
 * @return whether SPC pulse is being sent
 */
LOCAL_INLINE uint8 Sent_Reg_GetSpcTransferState(const SENT_CHANNEL_Type *Base)
{
    return (uint8)((Base->SCR & SENT_CHANNEL_SCR_TRQ_Msk) >> SENT_CHANNEL_SCR_TRQ_Pos);
}

/*!
 * @brief SENT set SPC pulse timebase type.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Timebase: timebase type
 * @return void
 */
LOCAL_INLINE void Sent_Reg_SetSPCTimebase(SENT_CHANNEL_Type *Base, Sent_SpcTimeBaseType Timebase)
{
    MODIFY_REG32(Base->SCR,
                 SENT_CHANNEL_SCR_BASE_Msk,
                 SENT_CHANNEL_SCR_BASE_Pos,
                 Timebase);
}

/*!
 * @brief SENT set SPC pulse length.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Length: pulse length, in tick times
 * @return none
 */
LOCAL_INLINE void Sent_Reg_SetSPCPulseLength(SENT_CHANNEL_Type *Base, uint8 Length)
{
    MODIFY_REG32(Base->SCR,
                 SENT_CHANNEL_SCR_PLEN_Msk,
                 SENT_CHANNEL_SCR_PLEN_Pos,
                 Length);
}

/*!
 * @brief SENT set SPC pulse delay
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Delay: sent SPC gap between messages, in clock cycle unit time
 * @return void
 */
LOCAL_INLINE void Sent_Reg_SetSPCPulseDelay(SENT_CHANNEL_Type *Base, uint8 Delay)
{
    MODIFY_REG32(Base->SCR,
                 SENT_CHANNEL_SCR_DEL_Msk,
                 SENT_CHANNEL_SCR_DEL_Pos,
                 Delay);
}

/*!
 * @brief SENT set SPC pulse trigger mode.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Mode: trigger mode
 * @return void
 */
LOCAL_INLINE void Sent_Reg_SetSPCTriggerMode(SENT_CHANNEL_Type *Base, Sent_SpcTriggerModeType Mode)
{
    MODIFY_REG32(Base->SCR,
                 SENT_CHANNEL_SCR_TRIG_Msk,
                 SENT_CHANNEL_SCR_TRIG_Pos,
                 Mode);
}

/*!
 * @brief SENT set spc parameters.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Param: SPC control params
 * @return void
 */
LOCAL_INLINE void Sent_Reg_SetSPCParams(SENT_CHANNEL_Type *Base, uint32 Param)
{
    WRITE_REG32(Base->SCR, Param);
}

/*!
 * @brief SENT set data nibble view.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] View: data nibble view value, data nibble order can be adjusted
 * @return void
 */
LOCAL_INLINE void Sent_Reg_SetDataNibbleView(SENT_CHANNEL_Type *Base, uint32 View)
{
    WRITE_REG32(Base->VIEW, View);
}

/*!
 * @brief SENT get fifo empty status.
 *
 * @param[in] Base: SENT channel base pointer
 * @return fifo empty status
 */
LOCAL_INLINE uint8 Sent_Reg_GetFifoEmptyStatus(const SENT_CHANNEL_Type *Base)
{
    return (uint8)((Base->FIFO & SENT_CHANNEL_FIFO_EMPTY_Msk) >> SENT_CHANNEL_FIFO_EMPTY_Pos);
}

/*!
 * @brief SENT get fifo half full status.
 *
 * @param[in] Base: SENT channel base pointer
 * @return fifo half full status
 */
LOCAL_INLINE uint8 Sent_Reg_GetFifoHalfFullStatus(const SENT_CHANNEL_Type *Base)
{
    return (uint8)((Base->FIFO & SENT_CHANNEL_FIFO_HALF_FULL_Msk) >> SENT_CHANNEL_FIFO_HALF_FULL_Pos);
}

/*!
 * @brief SENT get fifo full status.
 *
 * @param[in] Base: SENT channel base pointer
 * @return fifo full status
 */
LOCAL_INLINE uint8 Sent_Reg_GetFifoFullStatus(const SENT_CHANNEL_Type *Base)
{
    return (uint8)((Base->FIFO & SENT_CHANNEL_FIFO_FULL_Msk) >> SENT_CHANNEL_FIFO_FULL_Pos);
}

/*!
 * @brief SENT get fifo available bytes.
 *
 * @param[in] Base: SENT channel base pointer
 * @return fifo available bytes
 */
LOCAL_INLINE uint8 Sent_Reg_GetFifoAvailable(const SENT_CHANNEL_Type *Base)
{
    return (uint8)((Base->FIFO & SENT_CHANNEL_FIFO_FIFO_AVAIL_Msk) >> SENT_CHANNEL_FIFO_FIFO_AVAIL_Pos);
}

/*!
 * @brief SENT set watchdog limit.
 *
 * @param[in] Base: SENT channel base pointer
 * @param[in] Limit: watchdog limit value
 * @return void
 */
LOCAL_INLINE void Sent_Reg_SetWatchdogLimit(SENT_CHANNEL_Type *Base, uint16 Limit)
{
    MODIFY_REG32(Base->WDT,
                 SENT_CHANNEL_WDT_WDL_Msk,
                 SENT_CHANNEL_WDT_WDL_Pos,
                 Limit);
}

/*!
 * @brief SENT get frame data nibbles.
 *
 * @param[in] Base: SENT channel base pointer
 * @return frame data nibbles
 */
LOCAL_INLINE uint32 Sent_Reg_GetFrameDataNibbles(const SENT_CHANNEL_Type *Base)
{
    return ((Base->RDR & SENT_CHANNEL_RDR_RDR_Msk) >> SENT_CHANNEL_RDR_RDR_Pos);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /**< AC7840X_SENT_REG_H */

/* =============================================  EOF  ============================================== */
