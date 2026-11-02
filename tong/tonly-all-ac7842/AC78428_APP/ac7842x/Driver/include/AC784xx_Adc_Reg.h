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
/*!
 * @file AC784xx_Adc_Reg.h
 * @brief Adc access register inline function definition.
 */

#ifndef AC784XX_ADC_REG_H
#define AC784XX_ADC_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Adc_Hal_Types.h"
#include "Device_Register.h"

/* ============================================  DEFINES AND MACROS  ============================================ */
#define ADC_EACH_REG_SPT_NUM    (10UL)      /*!< ADC sampel timer setting number in each SPT register */

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/* =====================================  Functions definition  ===================================== */
/*!
 * @brief Set adc clock prescaler.
 * @note Function ID: DES_ADC_API_507
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - 0: ADC0
 *                  - 1: ADC1
 * @param[in] psc: prescaler Value
 *                  - ADC_CLK_DIVIDE_1
 *                  - ADC_CLK_DIVIDE_2
 *                  - ADC_CLK_DIVIDE_3
 *                  - ADC_CLK_DIVIDE_4
 *                  - ADC_CLK_DIVIDE_5
 *                  - ADC_CLK_DIVIDE_6
 *                  - ADC_CLK_DIVIDE_7
 *                  - ADC_CLK_DIVIDE_8
 *                  - ADC_CLK_DIVIDE_9
 *                  - ADC_CLK_DIVIDE_10
 *                  - ADC_CLK_DIVIDE_11
 *                  - ADC_CLK_DIVIDE_12
 *                  - ADC_CLK_DIVIDE_13
 *                  - ADC_CLK_DIVIDE_14
 *                  - ADC_CLK_DIVIDE_15
 *                  - ADC_CLK_DIVIDE_16
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetClockPrescaler(ADC_Type *const Base, const Adc_PrescaleType psc)
{
    MODIFY_REG32(Base->CTRL1, ADC_CTRL1_PSC_Msk, ADC_CTRL1_PSC_Pos, psc);
}

#ifndef AC7840E
/*!
 * @brief Set adc Resolution.
 * @note Function ID: DES_ADC_API_509
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Resolution: Resolution setting
 *                  - ADC_RESOLUTION_12BIT
 *                  - ADC_RESOLUTION_10BIT
 *                  - ADC_RESOLUTION_8BIT
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetResolution(ADC_Type *const Base, const Adc_ResolutionType Resolution)
{
    MODIFY_REG32(Base->CTRL1, ADC_CTRL1_RESL_Msk, ADC_CTRL1_RESL_Pos, Resolution);
}
#endif

/*!
 * @brief Set adc voltage reference source.
 * @note Function ID: DES_ADC_API_511
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] VoltageRef: voltage reference source
 *                  - ADC_VOLTAGEREF_VREF
 *                  - ADC_VOLTAGEREF_VDDA
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetVoltageReference(ADC_Type *const Base, const Adc_VoltageReferenceType VoltageRef)
{
    MODIFY_REG32(Base->SQL, ADC_SQL_REFSEL_Msk, ADC_SQL_REFSEL_Pos, VoltageRef);
}

/*!
 * @brief Set adc power mode.
 * @note Function ID: DES_ADC_API_513
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] mode: enable or disable ADC power
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetPowerEnableFlag(ADC_Type *const Base, const boolean mode)
{
    MODIFY_REG32(Base->CTRL1, ADC_CTRL1_ADON_Msk, ADC_CTRL1_ADON_Pos, \
                 ((TRUE == mode) ? (uint32)1U : (uint32)0U));
}

/*!
 * @brief Set scan convert mode enable flag.
 * @note Function ID: DES_ADC_API_515
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] State: enable or disable scan convert mode
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetScanEnableFlag(ADC_Type *const Base, const boolean State)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_SCAN_Msk, ADC_CTRL0_SCAN_Pos, \
                 ((TRUE == State) ? (uint32)1U : (uint32)0U));
}

/*!
 * @brief Set ADC interval mode.
 * @note Function ID: DES_ADC_API_517
 * @note Service ID: none
 * Can only be configured in mode3/5, other modes do not work.
 *
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] State: enable or disable scan convert mode
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetIntervalEnableFlag(ADC_Type *const Base, const boolean State)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_INTERVAL_Msk, ADC_CTRL0_INTERVAL_Pos, \
                 ((TRUE == State) ? (uint32)1U : (uint32)0U));
}

/*!
 * @brief Set continuous convert mode.
 * @note Function ID: DES_ADC_API_519
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] State: enable or disable continuous convert mode
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetContinuousEnableFlag(ADC_Type *const Base, const boolean State)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_CONT_Msk, ADC_CTRL0_CONT_Pos, \
                 ((TRUE == State) ? (uint32)1U : (uint32)0U));
}

/*!
 * @brief Set discontinuous mode enable flag of regular group.
 * @note Function ID: DES_ADC_API_521
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] State: enable or disable discontinuous convert mode
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetRegularDiscontinuousEnableFlag(ADC_Type *const Base, const boolean State)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_DISCEN_Msk, ADC_CTRL0_DISCEN_Pos,
                 ((TRUE == State) ? (uint32)1U : (uint32)0U));
}

/*!
 * @brief Set discontinuous mode number for regular group.
 * @note Function ID: DES_ADC_API_523
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] num: discontinuous convert number for regular group
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetRegularDiscontinuousNum(ADC_Type *const Base, const uint8 num)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_DISCNUM_Msk, ADC_CTRL0_DISCNUM_Pos, num);
}

/*!
 * @brief Set ADC automatic inject injection group convert after regular group
 *          function enable flag.
 * @note Function ID: DES_ADC_API_525
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] State: enable or disable automatic inject convert mode
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetInjectAutoEnableFlag(ADC_Type *const Base, const boolean State)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_IAUTO_Msk, ADC_CTRL0_IAUTO_Pos, \
                 ((TRUE == State) ? (uint32)1U : (uint32)0U));
}

/*!
 * @brief Set discontinuous mode for injection group.
 * @note Function ID: DES_ADC_API_527
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] State: enable or disable injecttion group discontinuous convert mode
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetInjectDiscontinuousEnableFlag(ADC_Type *const Base, const boolean State)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_IDISCEN_Msk, ADC_CTRL0_IDISCEN_Pos, \
                 ((TRUE == State) ? (uint32)1U : (uint32)0U));
}

/*!
 * @brief Set regular group trigger source.
 * @note Function ID: DES_ADC_API_529
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] source: trigger source type
 *                  - ADC_TRIGG_SRC_SW
 *                  - ADC_TRIGG_SRC_HW
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetRegularTriggerSource(ADC_Type *const Base, const Adc_TriggerSourceType source)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_EXTTRIG_Msk, ADC_CTRL0_EXTTRIG_Pos, source);
}

/*!
 * @brief Start software trigger for regular group convert.
 * @note Function ID: DES_ADC_API_531
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SoftwareStartRegularConvert(ADC_Type *const Base)
{
    SET_BIT32(Base->CTRL0, ADC_CTRL0_SWSTART_Msk);
}

/*!
 * @brief Start software trigger for inject group convert.
 * @note Function ID: DES_ADC_API_531
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SoftwareStartInjectConvert(ADC_Type *const Base)
{
    SET_BIT32(Base->CTRL0, ADC_CTRL0_ISWSTART_Msk);
}

/*!
 * @brief Set injected group trigger source.
 * @note Function ID: DES_ADC_API_532
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] source: trigger source type
 *                  - ADC_TRIGG_SRC_SW
 *                  - ADC_TRIGG_SRC_HW
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetInjectTriggerSource(ADC_Type *const Base, const Adc_TriggerSourceType source)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_IEXTTRIG_Msk, ADC_CTRL0_IEXTTRIG_Pos, source);
}

/*!
 * @brief Set injection group offset Value.
 *
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Num: injection group index, should be less than ADC_INJECT_SEQUENCE_LEGNTH
 * @param[in] Offset: offset value, which will be subtracted after injection
 *                  group index convertion complete.
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetInjectOffset(ADC_Type *const Base, uint8 Num, uint16 Offset)
{
    MODIFY_REG32(Base->IOFR[Num], ADC_IOFR_IOFR_Msk, ADC_IOFR_IOFR_Pos, Offset);
}

/*!
 * @brief Get injection group offset value.
 *
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Num: injection group index, should be less than ADC_INJECT_SEQUENCE_LEGNTH
 * @return offset value
 */
LOCAL_INLINE uint16 Adc_Reg_GetInjectOffset(const ADC_Type *const Base, const uint8 Num)
{
    return (uint16)(READ_BIT32(Base->IOFR[Num], ADC_IOFR_IOFR_Msk) >> ADC_IOFR_IOFR_Pos);
}

/*!
 * @brief Set regualr group convertion length.
 * @note Function ID: DES_ADC_API_537
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] length: regular group length
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetRegularLength(ADC_Type *const Base, const uint8 length)
{
    MODIFY_REG32(Base->SQL, ADC_SQL_RSQL_Msk, ADC_SQL_RSQL_Pos, \
                 (((uint32)length > 0U) ? ((uint32)length - 1U) : 0U));
}

/*!
 * @brief Get regualr group convertion length.
 * @note Function ID: DES_ADC_API_538
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @return regualr gropu length setting
 */
LOCAL_INLINE uint8 Adc_Reg_GetRegularLength(const ADC_Type *const Base)
{
    return (uint8)((READ_BIT32(Base->SQL, ADC_SQL_RSQL_Msk) >> ADC_SQL_RSQL_Pos) + 1U);
}

/*!
 * @brief Set injection group convertion length.
 * @note Function ID: DES_ADC_API_539
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] length: injection group length
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetInjectLength(ADC_Type *const Base, const uint8 length)
{
    MODIFY_REG32(Base->SQL, ADC_SQL_ISQL_Msk, ADC_SQL_ISQL_Pos, \
                 ((((uint32)length > 0UL) ? ((uint32)length - 1UL) : 0UL)));
}

/*!
 * @brief Get injection group convertion length.
 * @note Function ID: DES_ADC_API_540
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @return injection group length setting
 */
LOCAL_INLINE uint8 Adc_Reg_GetInjectLength(const ADC_Type *const Base)
{
    return (uint8)((READ_BIT32(Base->SQL, ADC_SQL_ISQL_Msk) >> ADC_SQL_ISQL_Pos) + 1U);
}

/*!
 * @brief Set Channel sample time.
 * @note Function ID: DES_ADC_API_541
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Channel: adc Channel
 * @param[in] Time: sample time
 *                  - ADC_SPT_CLK_5
 *                  - ADC_SPT_CLK_10
 *                  - ADC_SPT_CLK_15
 *                  - ADC_SPT_CLK_23
 *                  - ADC_SPT_CLK_35
 *                  - ADC_SPT_CLK_45
 *                  - ADC_SPT_CLK_85
 *                  - ADC_SPT_CLK_185
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetChannelSampleTime
(
    ADC_Type *const Base,
    const Adc_InputChannelType Channel,
    const Adc_SamplingTimeType Time
)
{
    uint8 index, pos;

    index = (uint8)(((uint32)Channel / ADC_EACH_REG_SPT_NUM) & 0xFFUL);
    pos = (uint8)((((uint32)Channel % ADC_EACH_REG_SPT_NUM) * ADC_SPT_Width) & 0xFFUL);
    if (index < ADC_SAMPLE_REG_NUM)
    {
        /*PRQA S 4391 ++ # AMCE_ADC_4391.*/
        MODIFY_REG32((Base->SPT[index]), ADC_SPT_Msk << pos, pos, (uint32)Time);
        /*PRQA S 4391 -- # AMCE_ADC_4391.*/
    }
}

#if defined (AC7840X) || defined (AC7843X) || defined (AC7840E)
/*!
 * @brief Set internal Supply Channel.
 * @note Function ID: DES_ADC_API_543
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Channel: adc Channel
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetSupplyChannel(ADC_Type *const Base, const Adc_InternalChannelSrcType Channel)
{
    MODIFY_REG32(Base->SQL, ADC_SQL_SUPPLY_Msk, ADC_SQL_SUPPLY_Pos, Channel);
}

/*!
 * @brief Get internal supply channel setting.
 * @note Function ID: DES_ADC_API_512
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 * @return current supply channel setting
 */
LOCAL_INLINE Adc_InternalChannelSrcType ADC_GetSupplyChannel(const ADC_Type * const Base)
{
    /*PRQA S 4394,4342 ++ # AMCE_ADC_4394, AMCE_ADC_4342.*/
    return (Adc_InternalChannelSrcType)(READ_BIT32(Base->SQL, ADC_SQL_SUPPLY_Msk) >> ADC_SQL_SUPPLY_Pos);
    /*PRQA S 4394,4342 -- # AMCE_ADC_4394, AMCE_ADC_4342.*/
}
#endif

/*!
 * @brief Set ADC DMA enable flag.
 * @note Function ID: DES_ADC_API_545
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] State: enable or disbale DMA function in ADC
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetDMAEnableFlag(ADC_Type *const Base, boolean State)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_DMAEN_Msk, ADC_CTRL0_DMAEN_Pos, State);
}

/*!
 * @brief Get ADC DMA enable flag.
 * @note Function ID: DES_ADC_API_545
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @return enable or disbale DMA function in ADC
 */
LOCAL_INLINE boolean Adc_Reg_GetDMAEnableFlag(const ADC_Type *const Base)
{
    uint32 tmp = READ_BIT32(Base->CTRL0, ADC_CTRL0_DMAEN_Msk) >> ADC_CTRL0_DMAEN_Pos;
    return ((tmp != 0U) ? 1U : 0U);
}

/*!
 * @brief Set regular group sequence.
 * @note Function ID: DES_ADC_API_547
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Seq: adc sequence
 * @param[in] Channel: adc Channel
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetRegularConversionChannel
(
    ADC_Type *const Base,
    const Adc_SequenceType Seq,
    Adc_InputChannelType Channel
)
{
    MODIFY_REG32(Base->RSQR[Seq], ADC_SQR_SEQ_Msk, ADC_SQR_SEQ_Pos, Channel);
}

/*!
 * @brief Set regular group end of convertion(EOC) interrupt enable flag.
 * @note Function ID: DES_ADC_API_549
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Seq: adc sequence
 * @param[in] State: enable or disbale EOC interrupt
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetRegularEOCInterruptEnableFlag
(
    ADC_Type *const Base,
    const Adc_SequenceType Seq,
    const boolean State
)
{
    MODIFY_REG32(Base->RSQR[Seq], ADC_SQR_IE_Msk, ADC_SQR_IE_Pos, \
                 ((TRUE == State) ? (uint32)1U : (uint32)0U));
}

/*!
 * @brief Get regular group EOC interrupt enable flag.
 * @note Function ID: DES_ADC_API_550
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Seq: adc sequence
 * @return whether EOC interrupt of particular sequence is enabled
 */
LOCAL_INLINE boolean Adc_Reg_GetRegularEOCInterruptEnableFlag(const ADC_Type *const Base, const Adc_SequenceType Seq)
{
    uint32 tmp = READ_BIT32(Base->RSQR[Seq], ADC_SQR_IE_Msk) >> ADC_SQR_IE_Pos;
    return ((tmp != 0U) ? 1U : 0U);
}

/*!
 * @brief Get regular group EOC flag.
 * @note Function ID: DES_ADC_API_551
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Seq: adc sequence
 * @return EOC flag of particular sequence
 */
LOCAL_INLINE boolean Adc_Reg_GetRegularEOCFlag(const ADC_Type *const Base, const Adc_SequenceType Seq)
{
#if defined (AC7840X)
    uint32 tmp = READ_BIT32(Base->RSQR[Seq], ADC_SQR_EOC_Msk) >> ADC_SQR_EOC_Pos;
#elif defined (AC7842X)  || defined (AC7843X) || defined (AC7840E)
    uint32 tmp = READ_BIT32(Base->REOC, BIT((uint32)Seq));
#endif
    return ((tmp != 0U) ? 1U : 0U);
}

/*!
 * @brief Clear regular group EOC flag.
 * @note Function ID: DES_ADC_API_552
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Seq: adc sequence
 * @return none
 */
LOCAL_INLINE void Adc_Reg_ClearRegularEOCFlag(ADC_Type *const Base, const Adc_SequenceType Seq)
{
#if defined (AC7840X)
    SET_BIT32(Base->RSQR[Seq], ADC_SQR_EOC_Msk);
#elif defined (AC7842X)  || defined (AC7843X) || defined (AC7840E)
    MODIFY_REG32(Base->REOC, 0xFFFFFFFFU, ((uint32)Seq), 1U);
#endif
}

/*!
 * @brief Set injection group sequence.
 * @note Function ID: DES_ADC_API_553
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Seq: adc sequence
 * @param[in] Channel: adc Channel
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetInjectConversionChannel
(
    ADC_Type *const Base,
    const Adc_SequenceType Seq,
    Adc_InputChannelType Channel
)
{
    MODIFY_REG32(Base->ISQR[(uint32)Seq - (uint32)ADC_ISEQ_0], ADC_SQR_SEQ_Msk, ADC_SQR_SEQ_Pos, Channel);
}

/*!
 * @brief Set injection group end of convertion(EOC) interrupt enable flag.
 * @note Function ID: DES_ADC_API_555
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Seq: adc sequence
 * @param[in] State: enable or disbale EOC interrupt
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetInjectEOCInterruptEnableFlag
(
    ADC_Type *const Base,
    const Adc_SequenceType Seq,
    const boolean State
)
{
    MODIFY_REG32(Base->ISQR[(uint32)Seq - (uint32)ADC_ISEQ_0], ADC_SQR_IE_Msk, ADC_SQR_IE_Pos, \
                 ((TRUE == State) ? (uint32)1U : (uint32)0U));
}

/*!
 * @brief Get injection group EOC interrupt enable flag.
 * @note Function ID: DES_ADC_API_556
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Seq: adc sequence
 * @return whether EOC interrupt of particular sequence is enabled
 */
LOCAL_INLINE boolean Adc_Reg_GetInjectEOCInterruptEnableFlag(const ADC_Type *const Base, const Adc_SequenceType Seq)
{
    uint32 tmp = READ_BIT32(Base->ISQR[(uint32)Seq - (uint32)ADC_ISEQ_0], ADC_SQR_IE_Msk) >> ADC_SQR_IE_Pos;
    return ((tmp != 0U) ? 1U : 0U);
}

/*!
 * @brief Get injection group EOC flag.
 * @note Function ID: DES_ADC_API_557
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Seq: adc sequence
 * @return EOC flag of particular sequence
 */
LOCAL_INLINE boolean Adc_Reg_GetInjectEOCFlag(const ADC_Type *const Base, const Adc_SequenceType Seq)
{
#if defined (AC7840X)
    uint32 tmp = READ_BIT32(Base->ISQR[(uint32)Seq - (uint32)ADC_ISEQ_0], ADC_SQR_EOC_Msk) >> ADC_SQR_EOC_Pos;
#elif defined (AC7842X)  || defined (AC7843X) || defined (AC7840E)
    uint32 tmp = READ_BIT32(Base->IEOC, (BIT(((uint32)Seq - (uint32)ADC_ISEQ_0))));
#endif
    return ((tmp != 0U) ? 1U : 0U);
}

/*!
 * @brief Clear injection group EOC flag.
 * @note Function ID: DES_ADC_API_558
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Seq: adc sequence
 * @return none
 */
LOCAL_INLINE void Adc_Reg_ClearInjectEOCFlag(ADC_Type *const Base, const Adc_SequenceType Seq)
{
#if defined (AC7840X)
    SET_BIT32(Base->ISQR[(uint32)Seq - (uint32)ADC_ISEQ_0], ADC_SQR_EOC_Msk);
#elif defined (AC7842X)  || defined (AC7843X) || defined (AC7840E)
    MODIFY_REG32(Base->IEOC, 0xFU, ((uint32)Seq - (uint32)ADC_ISEQ_0), 1U);
#endif
}

/*!
 * @brief Set ADC data Alignment.
 * @note Function ID: DES_ADC_API_559
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] align: data Alignment setting
 *                  - ADC_ALIGN_RIGHT
 *                  - ADC_ALIGN_LEFT
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetDataAlign(ADC_Type *const Base, const Adc_ResultAlignmentType align)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_ALIGN_Msk, ADC_CTRL0_ALIGN_Pos, align);
}

/*!
 * @brief Get ADC regular group data.
 * @note Function ID: DES_ADC_API_561
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Seq: adc seqeunce
 * @return ADC data of particular sequence
 */
LOCAL_INLINE uint16 Adc_Reg_GetRegularData(const ADC_Type *const Base, const Adc_SequenceType Seq)
{
    return (uint16)(READ_BIT32(Base->RDR[Seq], ADC_DR_DATA_Msk) >> ADC_DR_DATA_Pos);
}

/*!
 * @brief Get ADC regular group parity val.
 * @note Function ID: DES_ADC_API_561
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Seq: adc seqeunce
 * @return ADC parity val of particular sequence
 */
LOCAL_INLINE uint8 Adc_Reg_GetRegularParityVal(const ADC_Type *const Base, const Adc_SequenceType Seq)
{
    return (uint8)(READ_BIT32(Base->RDR[Seq], ADC_DR_EVEN_PARITY_Msk) >> ADC_DR_EVEN_PARITY_Pos);
}

/*!
 * @brief Get ADC injection data.
 * @note Function ID: DES_ADC_API_564
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Seq: adc seqeunce
 * @return ADC data of particular sequence
 */
LOCAL_INLINE uint16 Adc_Reg_GetInjectData(const ADC_Type *const Base, const Adc_SequenceType Seq)
{
    return (uint16)(READ_BIT32(Base->IDR[(uint32)Seq - (uint32)ADC_ISEQ_0], ADC_DR_DATA_Msk) >> ADC_DR_DATA_Pos);
}

/*!
 * @brief Get ADC injection group parity val.
 * @note Function ID: DES_ADC_API_561
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Seq: adc seqeunce
 * @return ADC parity val of particular sequence
 */
LOCAL_INLINE uint8 Adc_Reg_GetInjectParityVal(const ADC_Type *const Base, const Adc_SequenceType Seq)
{
    return (uint8)(READ_BIT32(Base->IDR[(uint32)Seq - (uint32)ADC_ISEQ_0],
                              ADC_DR_EVEN_PARITY_Msk) >> ADC_DR_EVEN_PARITY_Pos);
}

/*!
 * @brief Set hardware average enable flag.
 *
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] State: enable or disbale hardware average function
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetAverageEnableFlag(ADC_Type *const Base, boolean State)
{
    MODIFY_REG32(Base->CTRL1, ADC_CTRL1_AVGE_Msk, ADC_CTRL1_AVGE_Pos, State);
}

/*!
 * @brief Get hardware average enable flag.
 *
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @return whether hardware average function is enabled
 */
LOCAL_INLINE boolean Adc_Reg_GetAverageEnableFlag(const ADC_Type *const Base)
{
    uint32 tmp = READ_BIT32(Base->CTRL1, ADC_CTRL1_AVGE_Msk) >> ADC_CTRL1_AVGE_Pos;
    return ((tmp != 0U) ? 1U : 0U);
}

/*!
 * @brief Set hardware average mode.
 *
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] averageMode: average mode
 *                  - ADC_AVERAGE_4
 *                  - ADC_AVERAGE_8
 *                  - ADC_AVERAGE_16
 *                  - ADC_AVERAGE_32
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetAverageMode(ADC_Type *const Base, uint8 averageMode)
{
    MODIFY_REG32(Base->CTRL1, ADC_CTRL1_AVGS_Msk, ADC_CTRL1_AVGS_Pos, averageMode);
}

/*!
 * @brief Get hardware average mode.
 *
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @return  average mode
 *                  - ADC_AVERAGE_4
 *                  - ADC_AVERAGE_8
 *                  - ADC_AVERAGE_16
 *                  - ADC_AVERAGE_32
 */
LOCAL_INLINE uint8 Adc_Reg_GetAverageMode(const ADC_Type *const Base)
{
    return (uint8)(READ_BIT32(Base->CTRL1, ADC_CTRL1_AVGS_Msk) >> ADC_CTRL1_AVGS_Pos);
}

/*!
 * @brief Adc_Reg_SwRstClkCtrl.
 * @note Function ID: DES_ADC_API_606
 * @note Service ID: none
 * @param[in] Instance: adc module instance
 * @param[in] IsEnable: enable/disable
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SwRstClkCtrl(uint32 Instance, boolean IsEnable)
{
    /*PRQA S 4391 ++ # AMCE_ADC_4391.*/
    MODIFY_REG32(CKGEN->PERI_SFT_RST2, \
                 (CKGEN_PERI_SFT_RST2_SRST_ADC0_Msk << Instance), \
                 (CKGEN_PERI_SFT_RST2_SRST_ADC0_Pos + Instance), \
                 IsEnable);
    /*PRQA S 4391 -- # AMCE_ADC_4391.*/
}

/*!
 * @brief BusClk Enable Ctrl.
 * @note Function ID: DES_ADC_API_607
 * @note Service ID: none
 * @param[in] Instance: adc module instance
 * @param[in] IsEnable: enable flag
 * @return none
 */
LOCAL_INLINE void Adc_Reg_BusClkEnCtrl(uint32 Instance, boolean IsEnable)
{
    /*PRQA S 4391 ++ # AMCE_ADC_4391.*/
    MODIFY_REG32(CKGEN->PERI_CLK_EN2, \
                 (CKGEN_PERI_CLK_EN2_ADC0_EN_Msk << Instance), \
                 (CKGEN_PERI_CLK_EN2_ADC0_EN_Pos + Instance), \
                 IsEnable);
    /*PRQA S 4391 -- # AMCE_ADC_4391.*/
}

/*!
 * @brief Get STR reg flag.
 * @note Function ID: DES_ADC_API_609
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @return STR flag State
 */
LOCAL_INLINE uint32 Adc_Reg_GetSTRFlag(const ADC_Type *const Base)
{
    uint32 tmp = READ_REG32(Base->STR);
    return tmp;
}

/*!
 * @brief Clear STR reg flag.
 * @note Function ID: DES_ADC_API_610
 * @note Service ID: none
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Mask: Register value.
 * @return analog monitor flag State
 */
LOCAL_INLINE void Adc_Reg_ClearSTRFlag(ADC_Type *const Base, uint32 Mask)
{
#if defined (AC7840X)
    CLEAR_BIT32(Base->STR, Mask);
#elif defined (AC7842X)  || defined (AC7843X) || defined (AC7840E)
    SET_BIT32(Base->STR, Mask);
#endif
}

/*!
* @brief Set AMO Interrupt.
*
* @param[in] Base: adc module
                - ADC0
* @param[in] State: enabling State
                - ENABLE
                - DISABLE
* @return none
*/
LOCAL_INLINE void Adc_Reg_SetAMOInterrupt(ADC_Type *const Base, const boolean State)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_AMOIE_Msk, ADC_CTRL0_AMOIE_Pos, State);
}

/*!
* @brief Set analog monitor single channel.
*
* @param[in] Base: adc module
                - ADC0
* @param[in] Channel: analog monitor channel
                - ADC_CH_0
                - ADC_CH_1
                ...
                - ADC_CH_TSENSOR
* @return none
*/
LOCAL_INLINE void Adc_Reg_SetAMOSingleChannel(ADC_Type *const Base, const Adc_InputChannelType Channel)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_AMOCH_Msk, ADC_CTRL0_AMOCH_Pos, Channel);
}

/*!
* @brief Set analog monitor single channel mode.
*
* @param[in] Base: adc module
                - ADC0
* @param[in] State: enabling State
                - ENABLE
                - DISABLE
* @return none
*/
LOCAL_INLINE void Adc_Reg_SetAMOSingleChannelMode(ADC_Type *const Base, const boolean State)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_AMOSGL_Msk, ADC_CTRL0_AMOSGL_Pos, State);
}

/*!
* @brief Set regular group analog monitor mode.
*
* @param[in] Base: adc module
                - ADC0
* @param[in] State: enabling State
                - ENABLE
                - DISABLE
* @return none
*/
LOCAL_INLINE void Adc_Reg_SetAMORegularMode(ADC_Type *const Base, const boolean State)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_AMOEN_Msk, ADC_CTRL0_AMOEN_Pos, State);
}

/*!
* @brief Set injected group analog monitor mode.
*
* @param[in] Base: adc module
                - ADC0
* @param[in] State: enabling State
                - ENABLE
                - DISABLE
* @return none
*/
LOCAL_INLINE void Adc_Reg_SetAMOInjectMode(ADC_Type *const Base, const boolean State)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_IAMOEN_Msk, ADC_CTRL0_IAMOEN_Pos, State);
}

/*!
* @brief Set analog monitor trigger type.
*
* @param[in] Base: adc module
                - ADC0
* @param[in] Mode: amo trigger mode
                - ADC_AMO_TRIGGER_LEVEL
                - ADC_AMO_TRIGGER_EDGE
* @return none
*/
LOCAL_INLINE void Adc_Reg_SetAMOTriggerMode(ADC_Type *const Base, Adc_AmoTriggerModeType Mode)
{
    MODIFY_REG32(Base->CTRL0, ADC_CTRL0_AMOMODE_Msk, ADC_CTRL0_AMOMODE_Pos, Mode);
}

/*!
* @brief Set analog monitor threshold.
*
* @param[in] Base: adc module
                - ADC0
* @param[in] HighValue: high threshold value
                - 0~0xfff
* @param[in] LowValue: low threshold value
                - 0~0xfff
* @return none
*/
LOCAL_INLINE void Adc_Reg_SetAMOThreshold(ADC_Type *const Base, uint16 HighValue, uint16 LowValue)
{
    MODIFY_REG32(Base->AMOHR, ADC_AMOHR_AMOHT_Msk, ADC_AMOHR_AMOHT_Pos, HighValue);
    MODIFY_REG32(Base->AMOLR, ADC_AMOLR_AMOLT_Msk, ADC_AMOLR_AMOLT_Pos, LowValue);
}

/*!
* @brief Set analog monitor offset.
*
* @param[in] Base: adc module
                - ADC0
* @param[in] HighOffset: high offset value
                - 0~0xfff
* @param[in] LowOffset: low offset value
                - 0~0xfff
* @return none
*/
LOCAL_INLINE void Adc_Reg_SetAMOOffset(ADC_Type *const Base, uint16 HighOffset, uint16 LowOffset)
{
    MODIFY_REG32(Base->AMOHR, ADC_AMOHR_AMOHO_Msk, ADC_AMOHR_AMOHO_Pos, HighOffset);
    MODIFY_REG32(Base->AMOLR, ADC_AMOLR_AMOLO_Msk, ADC_AMOLR_AMOLO_Pos, LowOffset);
}

/*!
 * @brief Get idle flag.
 *
 * @param[in] base: adc module
 *                  - ADC0
 *                  - ADC1
 * @return idle flag state.
 */
LOCAL_INLINE boolean Adc_Reg_GetIdleFlag(const ADC_Type *const Base)
{
    return ((READ_BIT32(Base->STR, ADC_STR_IDLE_Msk) != 0U) ? 1U : 0U);
}

#if defined (AC7840E)
/*!
 * @brief Get LDO ready flag.
 *
 * @param[in] base: adc module
 *                  - ADC0
 *                  - ADC1
 * @return LDO flag state.
 */
LOCAL_INLINE boolean Adc_Reg_GetLdoFlag(const ADC_Type *const Base)
{
    return ((READ_BIT32(Base->STR, ADC_STR_ADCLDORDY_Msk) != 0U) ? 1U : 0U);
}
#endif

/*!
 * @brief Set Calibration function enable flag.
 *
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] State: enable or disbale calibration function
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetCalibrationEnableFlag(ADC_Type *const Base, boolean State)
{
    MODIFY_REG32(Base->CTRL1, ADC_CTRL1_CALEN_Msk, ADC_CTRL1_CALEN_Pos, State);
}

/*!
 * @brief Set GE OE calibration voltage input signal enable flag.
 *
 * @param[in] Instance: adc instance number
 * @param[in] State: enable or disable calibration voltage input
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetGEOEVINEnableFlag(uint8 Instance, boolean State)
{
    if (0U == Instance)
    {
#if defined (AC7840X)
        MODIFY_REG32(ANA->AUXADC_CFG1, \
                     ANA_AUXADC_CFG1_RG_ADC0_GEOECAL_VIN_EN_Msk, ANA_AUXADC_CFG1_RG_ADC0_GEOECAL_VIN_EN_Pos, State);
#elif defined (AC7842X)  || defined (AC7843X)
        MODIFY_REG32(ANA->AUXADC_CFG0, \
                     ANA_AUXADC_CFG0_RG_ADC0_GEOECAL_VIN_EN_Msk, ANA_AUXADC_CFG0_RG_ADC0_GEOECAL_VIN_EN_Pos, State);
#else
        (void)State;
#endif
    }
    else
    {
#if defined (AC7840X)
        MODIFY_REG32(ANA->AUXADC_CFG5, \
                     ANA_AUXADC_CFG5_RG_ADC1_GEOECAL_VIN_EN_Msk, ANA_AUXADC_CFG5_RG_ADC1_GEOECAL_VIN_EN_Pos, State);
#elif defined (AC7842X)
        MODIFY_REG32(ANA->AUXADC_CFG2, \
                     ANA_AUXADC_CFG2_RG_ADC1_GEOECAL_VIN_EN_Msk, ANA_AUXADC_CFG2_RG_ADC1_GEOECAL_VIN_EN_Pos, State);
#elif defined (AC7843X)
        MODIFY_REG32(ANA->AUXADC_CFG1, \
                     ANA_AUXADC_CFG1_RG_ADC1_GEOECAL_VIN_EN_Msk, ANA_AUXADC_CFG1_RG_ADC1_GEOECAL_VIN_EN_Pos, State);
#else
        (void)State;
#endif
    }
}

/*!
 * @brief Get GE OE calibration voltage input signal enable flag.
 *
 * @param[in] Instance: adc instance number
 * @return whether calibration voltage input is enabled
 */
LOCAL_INLINE boolean Adc_Reg_GetGEOEVINEnableFlag(uint8 Instance)
{
    uint32 tmp = 0U;

    if (0U == Instance)
    {
#if defined (AC7840X)
        tmp = READ_BIT32(ANA->AUXADC_CFG1, ANA_AUXADC_CFG1_RG_ADC0_GEOECAL_VIN_EN_Msk) \
              >> ANA_AUXADC_CFG1_RG_ADC0_GEOECAL_VIN_EN_Pos;
#elif defined (AC7842X)  || defined (AC7843X)
        tmp = READ_BIT32(ANA->AUXADC_CFG0, ANA_AUXADC_CFG0_RG_ADC0_GEOECAL_VIN_EN_Msk) \
              >> ANA_AUXADC_CFG0_RG_ADC0_GEOECAL_VIN_EN_Pos;
#endif
    }
    else
    {
#if defined (AC7840X)
        tmp = READ_BIT32(ANA->AUXADC_CFG5, ANA_AUXADC_CFG5_RG_ADC1_GEOECAL_VIN_EN_Msk) \
              >> ANA_AUXADC_CFG5_RG_ADC1_GEOECAL_VIN_EN_Pos;
#elif defined (AC7842X)
        tmp = READ_BIT32(ANA->AUXADC_CFG2, ANA_AUXADC_CFG2_RG_ADC1_GEOECAL_VIN_EN_Msk) \
              >> ANA_AUXADC_CFG2_RG_ADC1_GEOECAL_VIN_EN_Pos;
#elif defined (AC7843X)
        tmp = READ_BIT32(ANA->AUXADC_CFG1, ANA_AUXADC_CFG1_RG_ADC1_GEOECAL_VIN_EN_Msk) \
              >> ANA_AUXADC_CFG1_RG_ADC1_GEOECAL_VIN_EN_Pos;
#endif
    }
    return ((tmp != 0U) ? 1U : 0U);
}

/*!
 * @brief Set GE OE calibration input signal.
 *
 * @param[in] Instance: adc instance number
 * @param[in] Vin: calibration input signal
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetGEOEVIN(uint8 Instance, uint8 Vin)
{
    if (0U == Instance)
    {
#if defined (AC7840X)
        MODIFY_REG32(ANA->AUXADC_CFG1, \
                     ANA_AUXADC_CFG1_RG_ADC0_GEOECAL_VIN_SEL_Msk, ANA_AUXADC_CFG1_RG_ADC0_GEOECAL_VIN_SEL_Pos, Vin);
#elif defined (AC7842X)  || defined (AC7843X)
        MODIFY_REG32(ANA->AUXADC_CFG0, \
                     ANA_AUXADC_CFG0_RG_ADC0_GEOECAL_VIN_SEL_Msk, ANA_AUXADC_CFG0_RG_ADC0_GEOECAL_VIN_SEL_Pos, Vin);
#else
        (void)Vin;
#endif
    }
    else
    {
#if defined (AC7840X)
        MODIFY_REG32(ANA->AUXADC_CFG5, \
                     ANA_AUXADC_CFG5_RG_ADC1_GEOECAL_VIN_SEL_Msk, ANA_AUXADC_CFG5_RG_ADC1_GEOECAL_VIN_SEL_Pos, Vin);
#elif defined (AC7842X)
        MODIFY_REG32(ANA->AUXADC_CFG2, \
                     ANA_AUXADC_CFG2_RG_ADC1_GEOECAL_VIN_SEL_Msk, ANA_AUXADC_CFG2_RG_ADC1_GEOECAL_VIN_SEL_Pos, Vin);
#elif defined (AC7843X)
        MODIFY_REG32(ANA->AUXADC_CFG1, \
                     ANA_AUXADC_CFG1_RG_ADC1_GEOECAL_VIN_SEL_Msk, ANA_AUXADC_CFG1_RG_ADC1_GEOECAL_VIN_SEL_Pos, Vin);
#else
        (void)Vin;
#endif
    }
}

/*!
 * @brief Get GE OE calibration input signal.
 *
 * @param[in] Instance: adc instance number
 * @return calibration input signal
 */
LOCAL_INLINE uint8 Adc_Reg_GetGEOEVIN(uint8 Instance)
{
    uint32 tmp = 0U;

    if (0U == Instance)
    {
#if defined (AC7840X)
        tmp = READ_BIT32(ANA->AUXADC_CFG1, ANA_AUXADC_CFG1_RG_ADC0_GEOECAL_VIN_SEL_Msk) \
              >> ANA_AUXADC_CFG1_RG_ADC0_GEOECAL_VIN_SEL_Pos;
#elif defined (AC7842X)  || defined (AC7843X)
        tmp = READ_BIT32(ANA->AUXADC_CFG0, ANA_AUXADC_CFG0_RG_ADC0_GEOECAL_VIN_SEL_Msk) \
              >> ANA_AUXADC_CFG0_RG_ADC0_GEOECAL_VIN_SEL_Pos;
#endif
    }
    else
    {
#if defined (AC7840X)
        tmp = READ_BIT32(ANA->AUXADC_CFG5, ANA_AUXADC_CFG5_RG_ADC1_GEOECAL_VIN_SEL_Msk) \
              >> ANA_AUXADC_CFG5_RG_ADC1_GEOECAL_VIN_SEL_Pos;
#elif defined (AC7842X)
        tmp = READ_BIT32(ANA->AUXADC_CFG2, ANA_AUXADC_CFG2_RG_ADC1_GEOECAL_VIN_SEL_Msk) \
              >> ANA_AUXADC_CFG2_RG_ADC1_GEOECAL_VIN_SEL_Pos;
#elif defined (AC7843X)
        tmp = READ_BIT32(ANA->AUXADC_CFG1, ANA_AUXADC_CFG1_RG_ADC1_GEOECAL_VIN_SEL_Msk) \
              >> ANA_AUXADC_CFG1_RG_ADC1_GEOECAL_VIN_SEL_Pos;
#endif
    }
    return ((tmp != 0U) ? 1U : 0U);
}

/*!
 * @brief Set gain error and offset error value0 for external channel.
 *
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Gain: gain error in LSB. Can only be in [-4096, 4095]
 * @param[in] Offset: offset error in LSB. Can only be in [-1024, 1023]
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetGainOffset0Value(ADC_Type *const Base, sint16 Gain, sint16 Offset)
{
    sint16 GainValue = Gain, OffsetValue = Offset;
    MODIFY_REG32(Base->CALI0, ADC_CALI0_GE0_Msk, ADC_CALI0_GE0_Pos, GainValue);
    MODIFY_REG32(Base->CALI1, ADC_CALI1_OE0_Msk, ADC_CALI1_OE0_Pos, OffsetValue);
}

/*!
 * @brief Get gain error and offset error value0 for external channel.
 *
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[out] Gain: gain error in LSB.
 * @param[out] Offset: offset error in LSB.
 * @return none
 */
LOCAL_INLINE void Adc_Reg_GetGainOffset0Value(const ADC_Type *const Base, sint16 *const Gain, sint16 *const Offset)
{
    /*PRQA S 4394,4533 ++ # AMCE_ADC_4394,AMCE_ADC_4533.*/
    *Gain = (sint16)((uint16)(READ_BIT32(Base->CALI0, ADC_CALI0_GE0_Msk) >> ADC_CALI0_GE0_Pos) << 4U) >> 4U;
    *Offset = (sint16)((uint16)(READ_BIT32(Base->CALI1, ADC_CALI1_OE0_Msk) >> ADC_CALI1_OE0_Pos) << 5U) >> 5U;
    /*PRQA S 4394,4533 -- # AMCE_ADC_4394,AMCE_ADC_4533.*/
}

/*!
 * @brief Set gain error and offset error value0 for internal channel.
 *
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Gain: gain error in LSB. Can only be in [-4096, 4095]
 * @param[in] Offset: offset error in LSB. Can only be in [-1024, 1023]
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetGainOffset1Value(ADC_Type *const Base, sint16 Gain, sint16 Offset)
{
    sint16 GainValue = Gain, OffsetValue = Offset;
    MODIFY_REG32(Base->CALI0, ADC_CALI0_GE1_Msk, ADC_CALI0_GE1_Pos, GainValue);
    MODIFY_REG32(Base->CALI1, ADC_CALI1_OE1_Msk, ADC_CALI1_OE1_Pos, OffsetValue);
}

/*!
 * @brief Get gain error and offset error value0 for internal channel.
 *
 * @param[in] Base: adc module
 *                  - ADC0
 *                  - ADC1
 * @param[in] Gain: gain error in LSB.
 * @param[in] Offset: offset error in LSB.
 * @return none
 */
LOCAL_INLINE void Adc_Reg_GetGainOffset1Value(const ADC_Type *const Base, sint16 *const Gain, sint16 *const Offset)
{
    /*PRQA S 4394,4533 ++ # AMCE_ADC_4394,AMCE_ADC_4533.*/
    *Gain = (sint16)((uint16)(READ_BIT32(Base->CALI0, ADC_CALI0_GE1_Msk) >> ADC_CALI0_GE1_Pos) << 4U) >> 4U;
    *Offset = (sint16)((uint16)(READ_BIT32(Base->CALI1, ADC_CALI1_OE1_Msk) >> ADC_CALI1_OE1_Pos) << 5U) >> 5U;
    /*PRQA S 4394,4533 -- # AMCE_ADC_4394,AMCE_ADC_4533.*/
}

#if defined (AC7840X)
/*!
 * @brief Set adc0 interleave State.
 *
 * @param[in] State: interleave State
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetAdc0Interleave(boolean State)
{
    MODIFY_MEM32(&ANA->AUXADC_CFG0, ANA_AUXADC_CFG0_RG_ADC0_LDO_EN_Msk, ANA_AUXADC_CFG0_RG_ADC0_LDO_EN_Pos, State);
    MODIFY_MEM32(&ANA->AUXADC_CFG0, ANA_AUXADC_CFG0_RG_ADC0_VREFGEN_EN_Msk, ANA_AUXADC_CFG0_RG_ADC0_VREFGEN_EN_Pos, State);
    MODIFY_MEM32(&ANA->AUXADC_CFG0, ANA_AUXADC_CFG0_RG_ADC0_RGMODE_EN_Msk, ANA_AUXADC_CFG0_RG_ADC0_RGMODE_EN_Pos, State);
    MODIFY_MEM32(&ANA->AUXADC_CFG0, ANA_AUXADC_CFG0_RG_ADC0_EN_Msk, ANA_AUXADC_CFG0_RG_ADC0_EN_Pos, State);
}

/*!
 * @brief Set adc1 interleave State.
 *
 * @param[in] State: interleave State
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetAdc1Interleave(boolean State)
{
    MODIFY_MEM32(&ANA->AUXADC_CFG4, ANA_AUXADC_CFG4_RG_ADC1_LDO_EN_Msk, ANA_AUXADC_CFG4_RG_ADC1_LDO_EN_Pos, State);
    MODIFY_MEM32(&ANA->AUXADC_CFG4, ANA_AUXADC_CFG4_RG_ADC1_VREFGEN_EN_Msk, ANA_AUXADC_CFG4_RG_ADC1_VREFGEN_EN_Pos, State);
    MODIFY_MEM32(&ANA->AUXADC_CFG4, ANA_AUXADC_CFG4_RG_ADC1_RGMODE_EN_Msk, ANA_AUXADC_CFG4_RG_ADC1_RGMODE_EN_Pos, State);
    MODIFY_MEM32(&ANA->AUXADC_CFG4, ANA_AUXADC_CFG4_RG_ADC1_EN_Msk, ANA_AUXADC_CFG4_RG_ADC1_EN_Pos, State);
}

/*!
 * @brief Set adc0 interleave channel.
 *
 * @param[in] Channel: interleave channel select
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetAdc0InterleaveChannel(uint8 Channel)
{
    MODIFY_MEM32(&ANA->AUXADC_CFG0, ANA_AUXADC_CFG0_RG_ADC0_CHANNEL_SEL_Msk, ANA_AUXADC_CFG0_RG_ADC0_CHANNEL_SEL_Pos,
                 Channel);
}

/*!
 * @brief Set adc1 interleave channel
 *
 * @param[in] Channel: interleave channel select
 * @return none
 */
LOCAL_INLINE void Adc_Reg_SetAdc1InterleaveChannel(uint8 Channel)
{
    MODIFY_MEM32(&ANA->AUXADC_CFG4, ANA_AUXADC_CFG4_RG_ADC1_CHANNEL_SEL_Msk, ANA_AUXADC_CFG4_RG_ADC1_CHANNEL_SEL_Pos,
                 Channel);
}
#elif defined (AC7842X)  || defined (AC7843X) || defined (AC7840E)
#if defined (AC7842X)
/*!
 * @brief Config flash LDO output enable.
 * @param[in] State
 * @return none
 */
static inline void Adc_Reg_SetFlashLDOOutputEnable(const boolean State)
{
    MODIFY_REG32(ANA->SLDO_CFG0, ANA_SLDO_CFG0_FLHLDO_OUTPUT_Msk, ANA_SLDO_CFG0_FLHLDO_OUTPUT_Pos, (uint8)State);
}

/*!
 * @brief Get flash LDO enable.
 *
 * @param[in] none
 * @return flash LDO output enable Value
 */
static inline uint8 Adc_Reg_GetFlashLDOOutputEnableValue(void)
{
    return (uint8)(READ_BIT32(ANA->SLDO_CFG0, ANA_SLDO_CFG0_FLHLDO_OUTPUT_Msk) >> ANA_SLDO_CFG0_FLHLDO_OUTPUT_Pos);
}

/*!
 * @brief Config digital LDO output enable.
 * @param[in] State
 * @return none
 */
static inline void Adc_Reg_SetDigitalLDOOutputEnable(const boolean State)
{
    MODIFY_REG32(ANA->SLDO_CFG0, ANA_SLDO_CFG0_DIGLDO_OUTPUT_Msk, ANA_SLDO_CFG0_DIGLDO_OUTPUT_Pos, (uint8)State);
}

/*!
 * @brief Get digital LDO enable.
 *
 * @param[in] none
 * @return digital LDO output enable Value
 */
static inline uint8 Adc_Reg_GetDigitalLDOOutputEnableValue(void)
{
    return (uint8)(READ_BIT32(ANA->SLDO_CFG0, ANA_SLDO_CFG0_DIGLDO_OUTPUT_Msk) >> ANA_SLDO_CFG0_DIGLDO_OUTPUT_Pos);
}

/*!
 * @brief Config digital LDO enable.
 * @param[in] State
 * @return none
 */
static inline void Adc_Reg_SetDigitalLDOEnable(const boolean State)
{
    MODIFY_REG32(ANA->SLDO_CFG0, ANA_SLDO_CFG0_DIGLDO_EN_Msk, ANA_SLDO_CFG0_DIGLDO_EN_Pos, (uint8)State);
}

/*!
 * @brief Get digital LDO enable.
 *
 * @param[in] none
 * @return digital LDO enable Value
 */
static inline uint8 Adc_Reg_GetDigitalLDOEnableValue(void)
{
    return (uint8)(READ_BIT32(ANA->SLDO_CFG0, ANA_SLDO_CFG0_DIGLDO_EN_Msk) >> ANA_SLDO_CFG0_DIGLDO_EN_Pos);
}

/*!
 * @brief Config AMUX buf bypass enable.
 * @param[in] State
 * @return none
 */
static inline void Adc_Reg_SetAMUXBufBypassEnable(const boolean State)
{
    MODIFY_REG32(ANA->AUX_CFG0, ANA_AUX_CFG0_AMUX_BUF_BYPASS_EN_Msk, ANA_AUX_CFG0_AMUX_BUF_BYPASS_EN_Pos, (uint8)State);
}

/*!
 * @brief Get AMUX buf bypass enable value.
 *
 * @param[in] none
 * @return AMUX buf enable Value
 */
static inline uint8 Adc_Reg_GetAMUXBufBypassEnableValue(void)
{
    return (uint8)(READ_BIT32(ANA->AUX_CFG0, ANA_AUX_CFG0_AMUX_BUF_BYPASS_EN_Msk) >> ANA_AUX_CFG0_AMUX_BUF_BYPASS_EN_Pos);
}
#elif defined (AC7840E)
/*!
 * @brief Config AMUX buf bypass enable.
 * @param[in] State
 * @return none
 */
static inline void Adc_Reg_SetAMUXBufBypassEnable(const boolean State)
{
    MODIFY_REG32(ANA->AMUX_CFG0, ANA_AMUX_CFG0_AMUX_BUF_BYPASS_EN_Msk, ANA_AMUX_CFG0_AMUX_BUF_BYPASS_EN_Pos, (uint8)State);
}

/*!
 * @brief Get AMUX buf bypass enable value.
 *
 * @param[in] none
 * @return AMUX buf enable Value
 */
static inline uint8 Adc_Reg_GetAMUXBufBypassEnableValue(void)
{
    return (uint8)(READ_BIT32(ANA->AMUX_CFG0, ANA_AMUX_CFG0_AMUX_BUF_BYPASS_EN_Msk) >> ANA_AMUX_CFG0_AMUX_BUF_BYPASS_EN_Pos);
}
#endif

#ifndef AC7840E
/*!
 * @brief Config BG0 Output.
 *
 * @param[in] Value
 * @return none
 */
static inline void Adc_Reg_SetBG0OutputValue(const uint8 Value)
{
    MODIFY_REG32(ANA->SLDO_CFG0, ANA_SLDO_CFG0_BG0_MPX_SEL_Msk, ANA_SLDO_CFG0_BG0_MPX_SEL_Pos, Value);
}

/*!
 * @brief Get BG0 Output value.
 *
 * @param[in] none
 * @return BG0 Output Value
 */
static inline Ana_BgOutputType Adc_Reg_GetBG0OutputValue(void)
{
    /*PRQA S 4394,4342 ++ # AMCE_ADC_4394, AMCE_ADC_4342.*/
    return (Ana_BgOutputType)(READ_BIT32(ANA->SLDO_CFG0, ANA_SLDO_CFG0_BG0_MPX_SEL_Msk) >> ANA_SLDO_CFG0_BG0_MPX_SEL_Pos);
    /*PRQA S 4394,4342 -- # AMCE_ADC_4394, AMCE_ADC_4342.*/
}

/*!
 * @brief Config TSENSOR1 Output.
 * @param[in] Value
 * @return none
 */
static inline void Adc_Reg_SetTsensor1OutputValue(const uint8 Value)
{
    MODIFY_REG32(ANA->TSENSOR_CFG0, ANA_TSENSOR_CFG0_TSENSOR1_MPX_SEL_Msk, ANA_TSENSOR_CFG0_TSENSOR1_MPX_SEL_Pos, Value);
}

/*!
 * @brief Get TSENSOR1 Output value.
 * @param[in] none
 * @return Tsensor1 Output Value
 */
static inline Ana_TensorOutputType Adc_Reg_GetTsensor1OutputValue(void)
{
    /*PRQA S 4394,4342 ++ # AMCE_ADC_4394, AMCE_ADC_4342.*/
    return (Ana_TensorOutputType)(READ_BIT32(ANA->TSENSOR_CFG0, ANA_TSENSOR_CFG0_TSENSOR1_MPX_SEL_Msk) >> ANA_TSENSOR_CFG0_TSENSOR1_MPX_SEL_Pos);
    /*PRQA S 4394,4342 -- # AMCE_ADC_4394, AMCE_ADC_4342.*/
}
#endif

/*!
 * @brief Config TSENSOR0 Output.
 * @param[in] Value
 * @return none
 */
static inline void Adc_Reg_SetTsensor0OutputValue(const uint8 Value)
{
    MODIFY_REG32(ANA->TSENSOR_CFG0, ANA_TSENSOR_CFG0_TSENSOR0_MPX_SEL_Msk, ANA_TSENSOR_CFG0_TSENSOR0_MPX_SEL_Pos, Value);
}

/*!
 * @brief Get TSENSOR0 Output value.
 *
 * @param[in] none
 * @return Tsensor0 Output Value
 */
static inline Ana_TensorOutputType Adc_Reg_GetTsensor0OutputValue(void)
{
    /*PRQA S 4394,4342 ++ # AMCE_ADC_4394, AMCE_ADC_4342.*/
    return (Ana_TensorOutputType)(READ_BIT32(ANA->TSENSOR_CFG0, ANA_TSENSOR_CFG0_TSENSOR0_MPX_SEL_Msk) >> ANA_TSENSOR_CFG0_TSENSOR0_MPX_SEL_Pos);
    /*PRQA S 4394,4342 -- # AMCE_ADC_4394, AMCE_ADC_4342.*/
}

#if defined (AC7842X)
/*!
 * @brief Set internal amux channel.
 *
 * @param[in] Channel: channel
 * @return none
 */
static inline void Adc_Reg_SetAmuxInternalChannel(const Adc_InternalChannelSrcType Channel)
{
    MODIFY_REG32(ANA->AUX_CFG0, ANA_AUX_CFG0_AMUX_CHANNEL_SEL_Msk, ANA_AUX_CFG0_AMUX_CHANNEL_SEL_Pos, Channel);
}

/*!
 * @brief Get internal amux channel setting.
 *
 * @param[in] none
 * @return current internal amux channel setting
 */
static inline Adc_InternalChannelSrcType Adc_Reg_GetAmuxInternalChannel(void)
{
    uint32 value = (READ_BIT32(ANA->AUX_CFG0, ANA_AUX_CFG0_AMUX_CHANNEL_SEL_Msk) >> ANA_AUX_CFG0_AMUX_CHANNEL_SEL_Pos);
    return (Adc_InternalChannelSrcType)value;//PRQA S 4342 # AMCE_ADC_4342
}
#elif defined (AC7843X)
/*!
 * @brief Set internal amux channel.
 *
 * @param[in] Channel: channel
 * @return none
 */
static inline void Adc_Reg_SetAmuxInternalChannel(const Ana_AmuxChannelType Channel)
{
    MODIFY_REG32(ANA->AUX_CFG0, ANA_AUX_CFG0_AMUX_CHANNEL_SEL_Msk, ANA_AUX_CFG0_AMUX_CHANNEL_SEL_Pos, Channel);
}

/*!
 * @brief Get internal amux channel setting.
 *
 * @param[in] none
 * @return current internal amux channel setting
 */
static inline Ana_AmuxChannelType Adc_Reg_GetAmuxInternalChannel(void)
{
    uint32 value = (READ_BIT32(ANA->AUX_CFG0, ANA_AUX_CFG0_AMUX_CHANNEL_SEL_Msk) >> ANA_AUX_CFG0_AMUX_CHANNEL_SEL_Pos);
    return (Ana_AmuxChannelType)value;/*PRQA S 4342 # AMCE_ADC_4342.*/
}
#elif defined (AC7840E)
/*!
 * @brief Set internal amux channel.
 *
 * @param[in] Channel: channel
 * @return none
 */
static inline void Adc_Reg_SetAmuxInternalChannel(const Adc_InternalChannelSrcType Channel)
{
    MODIFY_REG32(ANA->AMUX_CFG0, ANA_AMUX_CFG0_AMUX_CHANNEL_SEL_Msk, ANA_AMUX_CFG0_AMUX_CHANNEL_SEL_Pos, Channel);
}

/*!
 * @brief Get internal amux channel setting.
 *
 * @param[in] none
 * @return current internal amux channel setting
 */
static inline Ana_AmuxChannelType Adc_Reg_GetAmuxInternalChannel(void)
{
    uint32 value = (READ_BIT32(ANA->AMUX_CFG0, ANA_AMUX_CFG0_AMUX_CHANNEL_SEL_Msk) >> ANA_AMUX_CFG0_AMUX_CHANNEL_SEL_Pos);
    return (Ana_AmuxChannelType)value;//PRQA S 4342 # AMCE_ADC_4342
}
#endif

/*!
 * @brief Config AMUX Output.
 * @param[in] State
 * @return none
 */
static inline void Adc_Reg_SetAMUXOutputEnable(const boolean State)
{
#if defined (AC7840E)
    MODIFY_REG32(ANA->AMUX_CFG0, ANA_AMUX_CFG0_AMUX_OUT_EN_Msk, ANA_AMUX_CFG0_AMUX_OUT_EN_Pos, (uint8)State);
#else
    MODIFY_REG32(ANA->AUX_CFG0, ANA_AUX_CFG0_AMUX_OUT_EN_Msk, ANA_AUX_CFG0_AMUX_OUT_EN_Pos, (uint8)State);
#endif
}

/*!
 * @brief Get AMUX Output value.
 *
 * @param[in] none
 * @return AMUX Output Value
 */
static inline uint8 Adc_Reg_GetAMUXOutputEnableValue(void)
{
#if defined (AC7840E)
    return (uint8)(READ_BIT32(ANA->AMUX_CFG0, ANA_AMUX_CFG0_AMUX_OUT_EN_Msk) >> ANA_AMUX_CFG0_AMUX_OUT_EN_Pos);
#else
    return (uint8)(READ_BIT32(ANA->AUX_CFG0, ANA_AUX_CFG0_AMUX_OUT_EN_Msk) >> ANA_AUX_CFG0_AMUX_OUT_EN_Pos);
#endif
}

/*!
 * @brief Config AMUX buf enable.
 * @param[in] State
 * @return none
 */
static inline void Adc_Reg_SetAMUXBufEnable(const boolean State)
{
#if defined (AC7840E)
    MODIFY_REG32(ANA->AMUX_CFG0, ANA_AMUX_CFG0_AMUX_BUF_EN_Msk, ANA_AMUX_CFG0_AMUX_BUF_EN_Pos, (uint8)State);
#else
    MODIFY_REG32(ANA->AUX_CFG0, ANA_AUX_CFG0_AMUX_BUF_EN_Msk, ANA_AUX_CFG0_AMUX_BUF_EN_Pos, (uint8)State);
#endif
}

/*!
 * @brief Get AMUX buf enable value.
 *
 * @param[in] none
 * @return AMUX buf enable Value
 */
static inline uint8 Adc_Reg_GetAMUXBufEnableValue(void)
{
#if defined (AC7840E)
    return (uint8)(READ_BIT32(ANA->AMUX_CFG0, ANA_AMUX_CFG0_AMUX_BUF_EN_Msk) >> ANA_AMUX_CFG0_AMUX_BUF_EN_Pos);
#else
    return (uint8)(READ_BIT32(ANA->AUX_CFG0, ANA_AUX_CFG0_AMUX_BUF_EN_Msk) >> ANA_AUX_CFG0_AMUX_BUF_EN_Pos);
#endif
}

#if defined (AC7840E)
/*!
 * @brief Config BG Output.
 *
 * @param[in] Value
 * @return none
 */
static inline void Adc_Reg_SetBGOutputValue(const uint8 Value)
{
    MODIFY_REG32(ANA->BG_CFG0, ANA_BG_CFG0_BG_MPX_SEL_Msk, ANA_BG_CFG0_BG_MPX_SEL_Pos, Value);
}

/*!
 * @brief Get BG Output value.
 *
 * @param[in] none
 * @return BG Output Value
 */
static inline uint8 Adc_Reg_GetBGOutputValue(void)
{
    return (uint8)(READ_BIT32(ANA->BG_CFG0, ANA_BG_CFG0_BG_MPX_SEL_Msk) >> ANA_BG_CFG0_BG_MPX_SEL_Pos);
}
#else
/*!
 * @brief Config BG1 Output.
 *
 * @param[in] Value
 * @return none
 */
static inline void Adc_Reg_SetBG1OutputValue(const uint8 Value)
{
    MODIFY_REG32(ANA->BG1_CFG0, ANA_BG1_CFG0_BG1_MPX_SEL_Msk, ANA_BG1_CFG0_BG1_MPX_SEL_Pos, Value);
}

/*!
 * @brief Get BG1 Output value.
 *
 * @param[in] none
 * @return BG1 Output Value
 */
static inline Ana_BgOutputType Adc_Reg_GetBG1OutputValue(void)
{
    /*PRQA S 4394,4342 ++ # AMCE_ADC_4394, AMCE_ADC_4342.*/
    return (Ana_BgOutputType)(READ_BIT32(ANA->BG1_CFG0, ANA_BG1_CFG0_BG1_MPX_SEL_Msk) >> ANA_BG1_CFG0_BG1_MPX_SEL_Pos);
    /*PRQA S 4394,4342 -- # AMCE_ADC_4394, AMCE_ADC_4342.*/
}

/*!
 * @brief Config AMUX XOSC enable.
 *
 * @param[in] Value
 * @return none
 */
static inline void Adc_Reg_SetAMUXXoscOutputEnable(const boolean State)
{
    MODIFY_REG32(ANA->XOSC_CFG0, ANA_XOSC_CFG0_XOSC_LDO_OUTPUT_Msk, ANA_XOSC_CFG0_XOSC_LDO_OUTPUT_Pos, (uint8)State);
}

/*!
 * @brief Get AMUX XOSC output value.
 *
 * @param[in] none
 * @return AMUX XOSC output Value
 */
static inline uint8 Adc_Reg_GetAMUXXoscOutputEnableValue(void)
{
    return (uint8)(READ_BIT32(ANA->XOSC_CFG0, ANA_XOSC_CFG0_XOSC_LDO_OUTPUT_Msk) >> ANA_XOSC_CFG0_XOSC_LDO_OUTPUT_Pos);
}
#endif
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* AC784XX_ADC_REG_H */
/* =============================================  EOF  ============================================== */
