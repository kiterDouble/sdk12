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

 * @file AC784xx_Can_Reg.h
 *
 * @brief This file provides extern Can Reg API.
 *
 */

#ifndef AC784XX_CAN_REG_H
#define AC784XX_CAN_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"

/*============================================DEFINES AND MACROS====================================*/
#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
#define CTRL0_WRITE_PREPARE_BITS (CAN_CTRL0_TSNEXT_Msk | CAN_CTRL0_TPE_Msk | CAN_CTRL0_TSALL_Msk | CAN_CTRL0_TSONE_Msk)
#define CTRL1_WRITE_PREPARE_BITS (CAN_CTRL1_AIF_Msk | CAN_CTRL1_EIF_Msk |  CAN_CTRL1_TSIF_Msk | \
                                  CAN_CTRL1_TPIF_Msk | CAN_CTRL1_RAFIF_Msk | CAN_CTRL1_RFIF_Msk | \
                                  CAN_CTRL1_ROIF_Msk | CAN_CTRL1_RIF_Msk | CAN_CTRL1_BEIF_Msk | \
                                  CAN_CTRL1_ALIF_Msk | CAN_CTRL1_EPIF_Msk)
#define VERMEM_WRITE_PREPARE_BITS (CAN_VERMEM_MDWIF_Msk | CAN_VERMEM_MDEIF_Msk)

/**
 * @def CAN_COMMON_IRQ_EN_MASKS
 * @brief CAN common IRQ all supports enable masks.
 * @note DES_CAN_MACRO_002
 */
#define CAN_COMMON_IRQ_EN_MASKS (CAN_CTRL1_EIE_Msk | CAN_CTRL1_TSIE_Msk | CAN_CTRL1_TPIE_Msk |\
                                 CAN_CTRL1_RAFIE_Msk | CAN_CTRL1_RFIE_Msk | CAN_CTRL1_ROIE_Msk |\
                                 CAN_CTRL1_RIE_Msk | CAN_CTRL1_EPIE_Msk | CAN_CTRL1_ALIE_Msk |\
                                 CAN_CTRL1_BEIE_Msk)

/**
 * @def CAN_COMMON_IRQ_FLAG_MASKS
 * @brief CAN common IRQ all supports flag masks.
 * @note DES_CAN_MACRO_003
 */
#define CAN_COMMON_IRQ_FLAG_MASKS (CAN_CTRL1_EIF_Msk | CAN_CTRL1_BEIF_Msk | CAN_CTRL1_AIF_Msk |\
                                   CAN_CTRL1_RAFIF_Msk | CAN_CTRL1_RFIF_Msk | CAN_CTRL1_ROIF_Msk |\
                                   CAN_CTRL1_RIF_Msk | CAN_CTRL1_EPIF_Msk | CAN_CTRL1_ALIF_Msk |\
                                   CAN_CTRL1_TSIF_Msk | CAN_CTRL1_TPIF_Msk)

/**
 * @def CAN_ECC_IRQ_EN_MASKS
 * @brief CAN ECC IRQ all supports enable masks.
 * @note DES_CAN_MACRO_004
 */
#define CAN_ECC_IRQ_EN_MASKS            (CAN_VERMEM_MDWIE_Msk | CAN_VERMEM_MDEIE_Msk)

/**
 * @def CAN_ECC_IRQ_FLAG_MASKS
 * @brief CAN ECC IRQ all supports flag masks.
 * @note DES_CAN_MACRO_005
 */
#define CAN_ECC_IRQ_FLAG_MASKS          (CAN_VERMEM_MDWIF_Msk | CAN_VERMEM_MDEIF_Msk)

/**
* @brief CAN ACFCTRL Offset.
*/
#define CAN_ACFCTRL1_OFFSET             (16U)
#define CAN_ACFCTRL2_OFFSET             (48U)

/*!
 * @brief CAN secondary send buffer state type enumeration.
 */
typedef enum
{
    CAN_TSSTAT_EMPTY = 0x00U, /*!< can secondary send buffer state empty */
    CAN_TSSTAT_LESS_HALF, /*!< can secondary send buffer state <= half full */
    CAN_TSSTAT_MORE_HALF, /*!< can secondary send buffer state > half full */
    CAN_TSSTAT_FULL, /*!< can secondary send buffer state full */
} Can_TsStatusType;//PRQA S 1535 # AMCE_CAN_1535
#elif defined (AC7843X)
/**
 * @def CAN_COMMON_IRQ_EN_MASKS
 * @brief CAN common IRQ all supports enable masks.
 * @note DES_CAN_MACRO_015
 */
#define CAN_COMMON_IRQ_EN_MASKS         (0x3FFFFFFFUL)
/**
 * @def DMU_IRQ_ALL_ENABLE_MSK
 * @brief Enable all DMU interrupt mask.
 * @note DES_CAN_MACRO_016
 */
#define DMU_IRQ_ALL_ENABLE_MSK          (0x601FDF00UL)

/**
 * @brief High priority message status struct.
 * @note DES_CAN_TYP_019
 */
typedef struct
{
    uint8 FLST; /*!< Filter list */
    uint8 FIDX; /*!< Filter index */
    uint8 MSI; /*!< Message storage indicator */
    uint8 BIDX; /*!< Buffer index */
} Can_HpmsStatusType;

/**
 * @brief Message RAM configuration struct.
 * @note DES_CAN_TYP_020
 */
typedef struct
{
    uint8 txFifoNumber; /*!< Tx fifo number */
    uint8 txDbufferNumber; /*!< Dedicate tx buffer number */
    uint8 stdFilterNumber; /*!< Standard filter number */
    uint8 extFilterNumber; /*!< Extened filter number */
    uint8 rxFifo0Number; /*!< Rx fifo 0 number */
    uint8 rxFifo1Number; /*!< Rx fifo 1 number */
    uint8 rxDbufferNumber; /*!< Dedicate rx buffer number */
    uint8 txEventFifoNumber; /*!< Tx event Fifo number */
    uint8 rxDbufferDataSize; /*!< Dedicate rx buffer data size */
    uint8 rxFifo0DataSize; /*!< Rx fifo 0 data size */
    uint8 rxFifo1DataSize; /*!< Rx fifo 1 data size */
    uint8 txBufferDataSize; /*!< Tx buffer data size */
} Can_MramConfigType;

/**
 * @brief Message RAM start address struct.
 * @note DES_CAN_TYP_021
 */
typedef struct
{
    uint32 rxFifo0StartAddress;      /*!< Rx FIFO 0 start address */
    uint32 rxFifo1StartAddress;      /*!< Rx FIFO 1 start address */
    uint32 rxDbufferStartAddress;    /*!< Dedicated RX buffer start address */
    uint32 txBufferStartAddress;     /*!< TX buffer start address */
    uint32 stdFilterStartAddress;    /*!< Standard filter start address */
    uint32 extFilterStartAddress;    /*!< Extended filter start address */
    uint32 txEventfifoStartAddress;  /*!< TX event FIFO start address */
    uint32 endAddress;               /*!< End address */
} Can_MramAddressType;
#endif
/* ======================================  Functions definition  ==================================== */
#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)

#if defined (AC7842X) || defined (AC7840E)
LOCAL_INLINE uint32 Can_Reg_GetTbuc
(
    CAN_Type const * const CanModuleNx
)
{
    return EXTRACT_BIT32(CanModuleNx->TBU, CAN_TBU_TBUC_Msk, CAN_TBU_TBUC_Pos);
}
#endif

LOCAL_INLINE void Can_Reg_SetEwl
(
    CAN_Type *CanModuleNx,
    uint8 Val
)
{
    MODIFY_REG32(CanModuleNx->CTRL1, CTRL1_WRITE_PREPARE_BITS | CAN_CTRL1_EWL_Msk, CAN_CTRL1_EWL_Pos, Val);
}

LOCAL_INLINE void Can_Reg_SetTpss
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_TPSS_Msk, CAN_CTRL0_TPSS_Pos, State);
}

LOCAL_INLINE void Can_Reg_SetTsss
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_TSSS_Msk, CAN_CTRL0_TSSS_Pos, State);
}

LOCAL_INLINE void Can_Reg_SetBusOffRecDisable
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_BOREC_Msk, CAN_CTRL0_BOREC_Pos, State);
}

LOCAL_INLINE void Can_Reg_SetTpa
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_TPA_Msk, CAN_CTRL0_TPA_Pos, State);
}

LOCAL_INLINE void Can_Reg_SetTsa
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_TSA_Msk, CAN_CTRL0_TSA_Pos, State);
}

LOCAL_INLINE void Can_Reg_SetTpe
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_TPE_Msk, CAN_CTRL0_TPE_Pos, State);
}

LOCAL_INLINE void Can_Reg_SetTsall
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_TSALL_Msk, CAN_CTRL0_TSALL_Pos, State);
}

LOCAL_INLINE void Can_Reg_SetTsone
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_TSONE_Msk, CAN_CTRL0_TSONE_Pos, State);
}

LOCAL_INLINE uint32 Can_Reg_GetTsstat
(
    CAN_Type const * const CanModuleNx
)
{
    return EXTRACT_BIT32(CanModuleNx->CTRL0, CAN_CTRL0_TSSTAT_Msk, CAN_CTRL0_TSSTAT_Pos);
}

LOCAL_INLINE uint32 Can_Reg_GetTpe
(
    CAN_Type const * const CanModuleNx
)
{
    return EXTRACT_BIT32(CanModuleNx->CTRL0, CAN_CTRL0_TPE_Msk, CAN_CTRL0_TPE_Pos);
}

LOCAL_INLINE uint32 Can_Reg_GetCtrl0
(
    CAN_Type const * const CanModuleNx
)
{
    return READ_REG32(CanModuleNx->CTRL0);
}

/**
* @brief Set RREL register value.
* @note    Function ID:  DES_CAN_API_308
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[inout] None
* @param[out] None
* @return Rrel value
*/
LOCAL_INLINE void Can_Reg_SetRrel
(
    CAN_Type *CanModuleNx
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_RREL_Msk, CAN_CTRL0_RREL_Pos, 1);
}

/**
* @brief the setting of int enable.
* @note    Function ID:  DES_CAN_API_309
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[in]  IntMask -  mask for interruptiont.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetIntEnable
(
    CAN_Type *CanModuleNx,
    uint32 IntMask
)
{
    MODIFY_REG32(CanModuleNx->CTRL1, CTRL1_WRITE_PREPARE_BITS | CAN_COMMON_IRQ_EN_MASKS, 0, IntMask);
}

/**
* @brief set  SSPoff value.
* @note    Function ID:  DES_CAN_API_312
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[in] offset -   given offset to be set.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetSspOff
(
    CAN_Type *CanModuleNx,
    uint8 Offset
)
{
    MODIFY_REG32(CanModuleNx->ERRINFO, CAN_ERRINFO_SSPOFF_Msk, CAN_ERRINFO_SSPOFF_Pos, Offset);
}

/**
* @brief set  Tdc value.
* @note    Function ID:  DES_CAN_API_313
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[in] State -   given State to be set.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetTdc
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->ERRINFO, CAN_ERRINFO_TDCEN_Msk, CAN_ERRINFO_TDCEN_Pos, State);
}

LOCAL_INLINE uint32 Can_Reg_GetTimeStamp
(
    CAN_Type const * const CanModuleNx
)
{
    return EXTRACT_BIT32(CanModuleNx->ACFCTRL0, CAN_ACFCTRL0_TIMEEN_Msk, CAN_ACFCTRL0_TIMEEN_Pos);
}

/**
* @brief set time stamp value.
* @note    Function ID:  DES_CAN_API_314
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[in] State -   given State to be set.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetTimeStamp
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->ACFCTRL0, CAN_ACFCTRL0_TIMEEN_Msk, CAN_ACFCTRL0_TIMEEN_Pos, State);
}

/**
* @brief set time position value.
* @note    Function ID:  DES_CAN_API_315
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[in] pos -   given position value to be set.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetTimePosition
(
    CAN_Type *CanModuleNx,
    uint8 pos
)
{
    MODIFY_REG32(CanModuleNx->ACFCTRL0, CAN_ACFCTRL0_TIMEPOS_Msk, CAN_ACFCTRL0_TIMEPOS_Pos, pos);
}

/**
* @brief set time count value.
* @note    Function ID:  DES_CAN_API_316
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[in] pos -   given position value to be set.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetTimeCount
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->ACFCTRL0, CAN_ACFCTRL0_TCEN_Msk, CAN_ACFCTRL0_TCEN_Pos, State);
}

/**
* @brief set time clock source value.
* @note    Function ID:  DES_CAN_API_317
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[in] src -   clock source parameter.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetTimeClockSrc
(
    CAN_Type *CanModuleNx,
    uint8 Src
)
{
    MODIFY_REG32(CanModuleNx->ACFCTRL0, CAN_ACFCTRL0_TCSS_Msk, CAN_ACFCTRL0_TCSS_Pos, Src);
}

/**
* @brief setting the  MemEcc.
* @note    Function ID:  DES_CAN_API_318
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[in] mask -  parameter for mask.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetMemEcc
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->VERMEM, VERMEM_WRITE_PREPARE_BITS | CAN_VERMEM_MEEN_Msk, CAN_VERMEM_MEEN_Pos, State);

}

/**
* @brief enable setting Ecc intteruption.
* @note    Function ID:  DES_CAN_API_319
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[in] memEccIntMask - control variable.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetEccIntEnable
(
    CAN_Type *CanModuleNx,
    uint32 MemEccIntMask
)
{
    MODIFY_REG32(CanModuleNx->VERMEM, VERMEM_WRITE_PREPARE_BITS | CAN_ECC_IRQ_EN_MASKS, 0, MemEccIntMask);
}

/**
* @brief check if Mem is Initialized Done.
* @note    Function ID:  DES_CAN_API_320
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[inout] None
* @param[out] None
* @return uint32
*/
LOCAL_INLINE uint32 Can_Reg_IsMemInitDone
(
    CAN_Type const * const CanModuleNx
)
{
    return EXTRACT_BIT32(CanModuleNx->VERMEM, CAN_VERMEM_MEID_Msk, CAN_VERMEM_MEID_Pos);
}

#if defined (AC7840X) || defined (AC7842X)
/**
* @brief set LPF enable control bit.
* @note    Function ID:  DES_CAN_API_321
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[in] State - control variable.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetLpfen
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->WAKEUP, CAN_WAKEUP_LPFEN_Msk, CAN_WAKEUP_LPFEN_Pos, State);
}

/**
* @brief enable can wake up.
* @note    Function ID:  DES_CAN_API_322
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[in] State - control variable.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Regl_SetWuen
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->WAKEUP, CAN_WAKEUP_WUEN_Msk, CAN_WAKEUP_WUEN_Pos, State);
}
#elif defined (AC7840E)
/**
* @brief set LPF0 enable control bit.
* @note    Function ID:  DES_CAN_API_321
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[in] State - control variable.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetLpfen0
(
    SPM_Type *Base,
    boolean Enable
)
{
    MODIFY_REG32(Base->PWR_MGR_CFG0, SPM_PWR_MGR_CFG0_CAN0_LPFEN_Msk, SPM_PWR_MGR_CFG0_CAN0_LPFEN_Pos, Enable);
}

/**
* @brief set LPF1 enable control bit.
* @note    Function ID:  DES_CAN_API_321
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[in] State - control variable.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetLpfen1
(
    SPM_Type *Base,
    boolean Enable
)
{
    MODIFY_REG32(Base->PWR_MGR_CFG0, SPM_PWR_MGR_CFG0_CAN1_LPFEN_Msk, SPM_PWR_MGR_CFG0_CAN1_LPFEN_Pos, Enable);
}

/**
* @brief enable can0 wake up.
* @note    Function ID:  DES_CAN_API_322
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[in] State - control variable.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetWuen0
(
    SPM_Type *Base,
    boolean Enable
)
{
    MODIFY_REG32(Base->PWR_MGR_CFG0, SPM_PWR_MGR_CFG0_CAN0_WUEN_Msk, SPM_PWR_MGR_CFG0_CAN0_WUEN_Pos, Enable);
}

/**
* @brief enable can1 wake up.
* @note    Function ID:  DES_CAN_API_322
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[in] State - control variable.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetWuen1
(
    SPM_Type *Base,
    boolean Enable
)
{
    MODIFY_REG32(Base->PWR_MGR_CFG0, SPM_PWR_MGR_CFG0_CAN1_WUEN_Msk, SPM_PWR_MGR_CFG0_CAN1_WUEN_Pos, Enable);
}
#endif

LOCAL_INLINE void Can_Reg_SetFilterIndex
(
    CAN_Type *CanModuleNx,
    uint8 Index
)
{
    MODIFY_REG32(CanModuleNx->ACFCTRL0, CAN_ACFCTRL0_ACFADR_Msk, CAN_ACFCTRL0_ACFADR_Pos, Index);
}

LOCAL_INLINE void Can_Reg_SetFilterCode
(
    CAN_Type *CanModuleNx,
    uint32 Code
)
{
    CLEAR_BIT32(CanModuleNx->ACFCTRL0, CAN_ACFCTRL0_SELMASK_Msk);
    WRITE_REG32(CanModuleNx->ACF, (Code & CAN_ACF_ACODE_Msk));
}

LOCAL_INLINE void Can_Reg_SetFilterMask
(
    CAN_Type *CanModuleNx,
    uint32 Mask
)
{
    SET_BIT32(CanModuleNx->ACFCTRL0, CAN_ACFCTRL0_SELMASK_Msk);
    WRITE_REG32(CanModuleNx->ACF, (Mask & (CAN_ACF_ACODE_Msk | CAN_ACF_AIDE_Msk | CAN_ACF_AIDEE_Msk)));
}

LOCAL_INLINE void Can_Reg_SetFilter0AcfEn
(
    CAN_Type *CanModuleNx,
    uint32 Index,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->ACFCTRL0,
        BIT_SHIFT(Index) << CAN_ACFCTRL0_ACFEN_Pos, CAN_ACFCTRL0_ACFEN_Pos, (uint32)State << Index);
}

LOCAL_INLINE void Can_Reg_SetFilter1AcfEn
(
    CAN_Type *CanModuleNx,
    uint32 Index,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->ACFCTRL1,
        BIT_SHIFT(Index - CAN_ACFCTRL1_OFFSET), CAN_ACFCTRL1_ACFEN_Pos, (uint32)State << (Index - CAN_ACFCTRL1_OFFSET));
}

LOCAL_INLINE void Can_Reg_SetFilter2AcfEn
(
    CAN_Type *CanModuleNx,
    uint32 Index,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->ACFCTRL2,
        BIT_SHIFT(Index - CAN_ACFCTRL2_OFFSET), CAN_ACFCTRL2_ACFEN_Pos, (uint32)State << (Index - CAN_ACFCTRL2_OFFSET));
}

/**
* @brief can bus off setting.
* @note Function ID:[DES_CAN_API_325]
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE uint32 Can_Reg_IsBusOff
(
    CAN_Type const * const CanModuleNx
)
{
    return READ_BIT32(CanModuleNx->CTRL0, CAN_CTRL0_BUSOFF_Msk);
}

/**
* @brief can bus off clear.
* @note Function ID:[DES_CAN_API_326]
* @param[in] CanModuleNx - given CanModuleNx value.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_ClearBusOff
(
    CAN_Type *CanModuleNx
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_BUSOFF_Msk, CAN_CTRL0_BUSOFF_Pos, 0U);
}

/**
* @brief CAN Set LBMI register.
* @note Function ID:[DES_CAN_API_327]
* @param[in] CanModuleNx -  Given CanModuleNx value.
* @param[in] State - State value to be set.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetLbmi
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_LBMI_Msk, CAN_CTRL0_LBMI_Pos, State);
}

/**
* @brief set can Lbme value.
* @note Function ID:[DES_CAN_API_328]
* @param[in] CanModuleNx -  Given CanModuleNx value.
* @param[in] State - State value to be set.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetLbme
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_LBME_Msk, CAN_CTRL0_LBME_Pos, State);
}

/**
* @brief set can reset.
* @note Function ID:[DES_CAN_API_329]
* @param[in] CanModuleNx -  Given CanModuleNx value.
* @param[in] State - State value to be set.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetReset
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_RESET_Msk, CAN_CTRL0_RESET_Pos, State);
}
/**
* @brief get can reset information.
* @note Function ID:[DES_CAN_API_330]
* @param[in] CanModuleNx -  Given CanModuleNx value.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE uint32 Can_Reg_GetReset
(
    CAN_Type const * const CanModuleNx
)
{
    return EXTRACT_BIT32(CanModuleNx->CTRL0, CAN_CTRL0_RESET_Msk, CAN_CTRL0_RESET_Pos);
}

/**
* @brief get can standby status.
* @note Function ID:[DES_CAN_API_334]
* @param[in] CanModuleNx -  Given CanModuleNx value.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE uint32 Can_Reg_GetStandby
(
    CAN_Type const * const CanModuleNx
)
{
    return EXTRACT_BIT32(CanModuleNx->CTRL0, CAN_CTRL0_STBY_Msk, CAN_CTRL0_STBY_Pos);
}

/**
* @brief set can Lom.
* @note Function ID:[DES_CAN_API_335]
* @param[in] CanModuleNx -  Given CanModuleNx value.
* @param[in] State - State value to be set
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetLom
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_LOM_Msk, CAN_CTRL0_LOM_Pos, State);
}

/**
* @brief set Fd Iso.
* @note Function ID:[DES_CAN_API_336]
* @param[in] CanModuleNx -  Given CanModuleNx value.
* @param[in] State - State to be set
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetFdIso
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_FDISO_Msk, CAN_CTRL0_FDISO_Pos, State);
}

/**
* @brief get receive State.
* @note Function ID:[DES_CAN_API_337]
* @param[in] CanModuleNx -  Given CanModuleNx value.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE uint32 Can_Reg_GetRStat
(
    CAN_Type const * const CanModuleNx
)
{
    return EXTRACT_BIT32(CanModuleNx->CTRL0, CAN_CTRL0_RSTAT_Msk, CAN_CTRL0_RSTAT_Pos);
}

/**
* @brief set dma recv.
* @note Function ID:[DES_CAN_API_338]
* @param[in] CanModuleNx -  given CanModuleNx value.
* @param[in] State -  State value to be set.
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetDmaRecv
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_DREN_Msk, CAN_CTRL0_DREN_Pos, State);
}

/**
* @brief set Rom.
* @note Function ID:[DES_CAN_API_338]
* @param[in] CanModuleNx -  given CanModuleNx value.
* @param[in] DisRxOverWrite -  Disable overwrite old message
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetRom
(
    CAN_Type *CanModuleNx,
    uint8 DisRxOverWrite
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_ROM_Msk, CAN_CTRL0_ROM_Pos, DisRxOverWrite);
}

/**
* @brief set Sack.
* @note Function ID:[DES_CAN_API_339]
* @param[in] CanModuleNx -  given CanModuleNx value.
* @param[in] State - State value to be set.
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetSack
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_SACK_Msk, CAN_CTRL0_SACK_Pos, State);
}

/**
* @brief Get Epass value.
* @note Function ID:[DES_CAN_API_340]
* @param[in] CanModuleNx -  given CanModuleNx value.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE uint32 Can_Reg_GetEpass
(
    CAN_Type const * const CanModuleNx
)
{
    return EXTRACT_BIT32(CanModuleNx->CTRL1, CAN_CTRL1_EPASS_Msk, CAN_CTRL1_EPASS_Pos);
}

/**
* @brief get  koer value.
* @note Function ID:[DES_CAN_API_341]
* @param[in] CanModuleNx -  given CanModuleNx value.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE uint32 Can_Reg_GetKoer
(
    CAN_Type const * const CanModuleNx
)
{
    return EXTRACT_BIT32(CanModuleNx->ERRINFO, CAN_ERRINFO_KOER_Msk, CAN_ERRINFO_KOER_Pos);
}

/**
* @brief get recount value.
* @note Function ID:[DES_CAN_API_342]
* @param[in] CanModuleNx -  given CanModuleNx value.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE uint32 Can_Reg_GetReCnt
(
    CAN_Type const * const CanModuleNx
)
{
    return EXTRACT_BIT32(CanModuleNx->ERRINFO, CAN_ERRINFO_RECNT_Msk, CAN_ERRINFO_RECNT_Pos);
}

/**
* @brief get time count value.
* @note Function ID:[DES_CAN_API_343]
* @param[in] CanModuleNx -  given CanModuleNx value.
* @param[inout] None
* @param[out] None
* @return void
*/
LOCAL_INLINE uint32 Can_Reg_GetTeCnt
(
    CAN_Type const * const CanModuleNx
)
{
    return EXTRACT_BIT32(CanModuleNx->ERRINFO, CAN_ERRINFO_TECNT_Msk, CAN_ERRINFO_TECNT_Pos);
}

LOCAL_INLINE void Can_Reg_SetSBitrate
(
    CAN_Type *CanModuleNx,
    uint16 Presc,
    uint8 Sjw,
    uint8 Seg1,
    uint8 Seg2
)
{
    WRITE_REG32(CanModuleNx->SBITRATE, ((uint32)Presc << CAN_SBITRATE_S_PRESC_Pos) | \
                     ((uint32)Sjw << CAN_SBITRATE_S_SJW_Pos) | \
                     ((uint32)Seg2 << CAN_SBITRATE_S_SEG_2_Pos) | (uint32)Seg1);
}

LOCAL_INLINE void Can_Reg_SetFBitrate
(
    CAN_Type *CanModuleNx,
    uint16 Presc,
    uint8 Sjw,
    uint8 Seg1,
    uint8 Seg2
)
{
    WRITE_REG32(CanModuleNx->FBITRATE, ((uint32)Presc << CAN_SBITRATE_S_PRESC_Pos) | \
                     ((uint32)Sjw << CAN_SBITRATE_S_SJW_Pos) | \
                     ((uint32)Seg2 << CAN_SBITRATE_S_SEG_2_Pos) | (uint32)Seg1);
}

LOCAL_INLINE void Can_Reg_SetTbsel
(
    CAN_Type *CanModuleNx,
    uint8 BufferId
)
{
    /* Select transmit buffer */
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_TBSEL_Msk, CAN_CTRL0_TBSEL_Pos, BufferId);
}

LOCAL_INLINE void Can_Reg_SetIdEsi
(
    CAN_Type *CanModuleNx,
    uint32 IdEsi
)
{
    WRITE_REG32(CanModuleNx->TBUF.ID_ESI, IdEsi);
}

LOCAL_INLINE uint32 Can_Reg_GetIdEsi
(
    CAN_Type const * const CanModuleNx
)
{
    return READ_REG32(CanModuleNx->RBUF.ID_ESI);
}

LOCAL_INLINE void Can_Reg_SetCtrlAddrVal
(
    CAN_Type *CanModuleNx,
    uint32 Val
)
{
    WRITE_REG32(CanModuleNx->TBUF.CTRL, Val);
}

LOCAL_INLINE uint32 Can_Reg_GetCtrlAddrVal
(
    CAN_Type const * const CanModuleNx
)
{
    return READ_REG32(CanModuleNx->RBUF.CTRL);
}

LOCAL_INLINE void Can_Reg_SetTbufData
(
    CAN_Type *CanModuleNx,
    uint8 Offset,
    uint32 DataVal
)
{
    WRITE_REG32(CanModuleNx->TBUF.DATA[Offset], DataVal);
}

LOCAL_INLINE uint32 Can_Reg_GetTbufData
(
    CAN_Type const * const CanModuleNx,
    uint8 Offset
)
{
    return READ_REG32(CanModuleNx->RBUF.DATA[Offset]);
}

LOCAL_INLINE void Can_Reg_SetTsnext
(
    CAN_Type *CanModuleNx
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS, CAN_CTRL0_TSNEXT_Pos, 1U);
}

LOCAL_INLINE uint32 Can_Reg_GetRts(CAN_Type const * const CanModuleNx)
{
    return CanModuleNx->RBUF.TS[0];
}

LOCAL_INLINE uint32 Can_Reg_GetTts(CAN_Type const * const CanModuleNx)
{
    return READ_REG32(CanModuleNx->TBUF.TS[0]);
}

/**
* @brief Set CAN_CTRL0 STBY register value.
* @param[in] device - Given device No.
* @param[out] None
* @return void
*/
LOCAL_INLINE void Can_Reg_SetStandby
(
    CAN_Type *CanModuleNx,
    uint8 State
)
{
    MODIFY_REG32(CanModuleNx->CTRL0, CTRL0_WRITE_PREPARE_BITS | CAN_CTRL0_STBY_Msk, CAN_CTRL0_STBY_Pos, State);
}

LOCAL_INLINE uint32 Can_Reg_GetCommonIrqFlagMasks
(
    CAN_Type const * const CanModuleNx
)
{
    return READ_BIT32(CanModuleNx->CTRL1, CAN_COMMON_IRQ_FLAG_MASKS);
}

LOCAL_INLINE void Can_Reg_ClearCommonIrqFlagMasks
(
    CAN_Type *CanModuleNx,
    uint32 Mask
)
{
    MODIFY_REG32(CanModuleNx->CTRL1, CTRL1_WRITE_PREPARE_BITS, 0, Mask);
}

LOCAL_INLINE uint32 Can_Reg_GetEccIrqFlagMasks
(
    CAN_Type const * const CanModuleNx
)
{
    return READ_BIT32(CanModuleNx->VERMEM, CAN_ECC_IRQ_FLAG_MASKS);
}

LOCAL_INLINE void Can_Reg_ClearEccIrqFlagMasks
(
    CAN_Type *CanModuleNx,
    uint32 Mask
)
{
     MODIFY_REG32(CanModuleNx->VERMEM, VERMEM_WRITE_PREPARE_BITS, 0, Mask);
}

/*!
 * @brief Adc_Reg_SwRstClkCtrl.
 * @note Service ID: none
 * @param[in] Instance: adc module instance
 *              IsEnable: enable/disable
 * @return none
 */
LOCAL_INLINE void Can_Reg_SwRstClkCtrl(uint32 Instance, uint8 IsEnable)
{
    MODIFY_REG32(CKGEN->PERI_SFT_RST2, \
                 ((uint32)CKGEN_PERI_SFT_RST1_SRST_CAN0_Msk << Instance), \
                 ((uint32)CKGEN_PERI_SFT_RST1_SRST_CAN0_Pos + Instance), \
                 IsEnable);
}

/*!
 * @brief BusClk Enable Ctrl.
 * @note Service ID: none
 * @param[in] Instance: adc module instance
 *              IsEnable: enable/disable
 * @return none
 */
LOCAL_INLINE void Can_Reg_BusClkEnCtrl(uint32 Instance, uint8 IsEnable)
{
    MODIFY_REG32(CKGEN->PERI_CLK_EN1, \
                 ((uint32)CKGEN_PERI_CLK_EN1_CAN0_EN_Msk << Instance), \
                 ((uint32)CKGEN_PERI_CLK_EN1_CAN0_EN_Pos + Instance), \
                 IsEnable);
}

/*!
 * @brief BusClk Enable Ctrl.
 * @note Service ID: none
 * @param[in] Instance: adc module instance
 *              IsEnable: enable/disable
 * @return none
 */
LOCAL_INLINE void Can_RegTsClkDivCtrl(uint32 Instance, uint8 Div)
{
    MODIFY_REG32(CKGEN->PERI_CLK_DIV, \
                 ((uint32)CKGEN_PERI_CLK_DIV_CAN0_TS_DIV_Msk << Instance), \
                 ((uint32)CKGEN_PERI_CLK_DIV_CAN0_TS_DIV_Pos + Instance), \
                 Div);
}
#elif defined (AC7843X)
/*!
 * @brief Set global filter.
 *
 * @param[in] base: CAN module
 * @param[in] ANFS: Accept non-matching standard frames
 * @param[in] ANFE: Accept non-matching extended frames
 * @param[in] RRFS: Reject standard remote frames
 * @param[in] RRFE: Reject extended remote frames
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetGlobalFilter(CAN_Type *base, uint8 ANFS, uint8 ANFE, uint8 RRFS, uint8 RRFE)
{
    WRITE_REG32(base->GFC, ((((uint32)ANFS & 0x3UL) << CAN_GFC_ANFS_Pos) | \
                   (((uint32)ANFE & 0x3UL) << CAN_GFC_ANFE_Pos) | \
                   (((uint32)RRFS & 0x1UL) << CAN_GFC_RRFS_Pos) | \
                   (((uint32)RRFE & 0x1UL) << CAN_GFC_RRFE_Pos)));
}

/*!
 * @brief Set message RAM element number and data size.
 *
 * @param[in] base: CAN module
 * @param[in] config: Message RAM configuration
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetMRAMSize(CAN_Type *base, Can_MramConfigType const *config)
{
    MODIFY_REG32(base->RXF0C, CAN_RXF0C_F0S_Msk, CAN_RXF0C_F0S_Pos, config->rxFifo0Number);
    MODIFY_REG32(base->RXF1C, CAN_RXF1C_F1S_Msk, CAN_RXF1C_F1S_Pos, config->rxFifo1Number);
    MODIFY_REG32(base->TXBC, CAN_TXBC_TFQS_Msk, CAN_TXBC_TFQS_Pos, config->txFifoNumber);
    MODIFY_REG32(base->TXBC, CAN_TXBC_NDTB_Msk, CAN_TXBC_NDTB_Pos, config->txDbufferNumber);
    MODIFY_REG32(base->TXEFC, CAN_TXEFC_EFS_Msk, CAN_TXEFC_EFS_Pos, config->txEventFifoNumber);
    MODIFY_REG32(base->SIDFC, CAN_SIDFC_LSS_Msk, CAN_SIDFC_LSS_Pos, config->stdFilterNumber);
    MODIFY_REG32(base->XIDFC, CAN_XIDFC_LSE_Msk, CAN_XIDFC_LSE_Pos, config->extFilterNumber);
    MODIFY_REG32(base->RXESC, CAN_RXESC_RBDS_Msk, CAN_RXESC_RBDS_Pos, config->rxDbufferDataSize);
    MODIFY_REG32(base->RXESC, CAN_RXESC_F0DS_Msk, CAN_RXESC_F0DS_Pos, config->rxFifo0DataSize);
    MODIFY_REG32(base->RXESC, CAN_RXESC_F1DS_Msk, CAN_RXESC_F1DS_Pos, config->rxFifo1DataSize);
    MODIFY_REG32(base->TXESC, CAN_TXESC_TBDS_Msk, CAN_TXESC_TBDS_Pos, config->txBufferDataSize);
}

/*!
 * @brief Set message RAM element start address.
 *
 * @param[in] base: CAN module
 * @param[in] config: Message RAM configuration
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetMRAMAdress(CAN_Type *base,  Can_MramAddressType const * const config)
{
    MODIFY_REG32(base->RXF0C, CAN_RXF0C_F0SA_Msk, CAN_RXF0C_F0SA_Pos, config->rxFifo0StartAddress);
    MODIFY_REG32(base->RXF1C, CAN_RXF1C_F1SA_Msk, CAN_RXF1C_F1SA_Pos, config->rxFifo1StartAddress);
    MODIFY_REG32(base->RXBC, CAN_RXBC_RBSA_Msk, CAN_RXBC_RBSA_Pos, config->rxDbufferStartAddress);
    MODIFY_REG32(base->TXBC, CAN_TXBC_TBSA_Msk, CAN_TXBC_TBSA_Pos, config->txBufferStartAddress);
    MODIFY_REG32(base->TXEFC, CAN_TXEFC_EFSA_Msk, CAN_TXEFC_EFSA_Pos, config->txEventfifoStartAddress);
    MODIFY_REG32(base->SIDFC, CAN_SIDFC_FLSSA_Msk, CAN_SIDFC_FLSSA_Pos, config->stdFilterStartAddress);
    MODIFY_REG32(base->XIDFC, CAN_XIDFC_FLESA_Msk, CAN_XIDFC_FLESA_Pos, config->extFilterStartAddress);
}

/*!
 * @brief Set timestamp.
 *
 * @param[in] base: CAN module
 * @param[in] config: timestamp configuration
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetTimeStamp(CAN_Type *base, uint8 tss, uint8 prescaler)
{
    MODIFY_REG32(base->TSCC, CAN_TSCC_TSS_Msk, CAN_TSCC_TSS_Pos, tss);
    MODIFY_REG32(base->TSCC, CAN_TSCC_TCP_Msk, CAN_TSCC_TCP_Pos, prescaler);
}

/*!
 * @brief Set CAN normal bitrate.
 *
 * @param[in] base: CAN module
 * @param[in] bitrate: pointer to bitrate configuration
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetRXF0A(CAN_Type *base, uint32 Val)
{
    WRITE_REG32(base->RXF0A, Val);
}

/*!
 * @brief Set CAN normal bitrate.
 *
 * @param[in] base: CAN module
 * @param[in] bitrate: pointer to bitrate configuration
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetRXF1A(CAN_Type *base, uint32 Val)
{
    WRITE_REG32(base->RXF1A, Val);
}

/*!
 * @brief Set CAN normal bitrate.
 *
 * @param[in] base: CAN module
 * @param[in] bitrate: pointer to bitrate configuration
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetTXEFA(CAN_Type *base, uint32 Val)
{
    WRITE_REG32(base->TXEFA, Val);
}

/*!
 * @brief Set CAN normal bitrate.
 *
 * @param[in] base: CAN module
 * @param[in] bitrate: pointer to bitrate configuration
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetNBTP(CAN_Type *base, uint32 Val)
{
    WRITE_REG32(base->NBTP, Val);
}

/*!
 * @brief Set CAN FD data bitrate.
 *
 * @param[in] base: CAN module
 * @param[in] bitrate: pointer to bitrate configuration
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetDBTP(CAN_Type *base, uint32 Val)
{
    WRITE_REG32(base->DBTP, Val);
}

/*!
 * @brief Set low power wake up enable.
 *
 * @param[in] base: CAN module
 * @param[in] mask: CAN channel mask
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetWakeup(CAN_CTRL_Type *base, uint8 mask)
{
    MODIFY_REG32(base->LPCR, CAN_CTRL_LPCR_LPWAKEEN_Msk, CAN_CTRL_LPCR_LPWAKEEN_Pos, mask);
}

/*!
 * @brief Set low power wake up enable.
 *
 * @param[in] base: CAN module
 * @param[in] mask: CAN channel mask
 * @return none
 */
LOCAL_INLINE uint32 Can_Reg_GetWakeup(CAN_CTRL_Type const * const base)
{
    return EXTRACT_BIT32(base->LPCR, CAN_CTRL_LPCR_LPWAKEEN_Msk, CAN_CTRL_LPCR_LPWAKEEN_Pos);
}

/*!
 * @brief Set low pass filter enable for low power mode.
 *
 * @param[in] base: CAN module
 * @param[in] mask: CAN channel mask
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetLowPassFilter(CAN_CTRL_Type *base, uint8 mask)
{
    MODIFY_REG32(base->LPCR, CAN_CTRL_LPCR_LPFILTEN_Msk, CAN_CTRL_LPCR_LPFILTEN_Pos, mask);
}

/*!
 * @brief Set low pass filter enable for low power mode.
 *
 * @param[in] base: CAN module
 * @param[in] mask: CAN channel mask
 * @return none
 */
LOCAL_INLINE uint32 Can_Reg_GetLowPassFilter(CAN_CTRL_Type const * const base)
{
    return EXTRACT_BIT32(base->LPCR, CAN_CTRL_LPCR_LPFILTEN_Msk, CAN_CTRL_LPCR_LPFILTEN_Pos);
}

/*!
 * @brief Set memory ECC protect enable.
 *
 * @param[in] base: CAN module
 * @param[in] mask: CAN channel mask
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetECC(CAN_CTRL_Type *base, uint8 mask)
{
    MODIFY_REG32(base->ECCCR, CAN_CTRL_ECCCR_ECCEN_Msk, CAN_CTRL_ECCCR_ECCEN_Pos, mask);
}

/*!
 * @brief Set memory ECC protect enable.
 *
 * @param[in] base: CAN module
 * @param[in] mask: CAN channel mask
 * @return none
 */
LOCAL_INLINE uint32 Can_Reg_GetECC(CAN_CTRL_Type const * const base)
{
    return EXTRACT_BIT32(base->ECCCR, CAN_CTRL_ECCCR_ECCEN_Msk, CAN_CTRL_ECCCR_ECCEN_Pos);
}

/*!
 * @brief Set DMU enable.
 *
 * @param[in] base: CAN module
 * @param[in] mask: CAN channel mask
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetDMU(CAN_CTRL_Type *base, uint8 mask)
{
    MODIFY_REG32(base->DMUCR, CAN_CTRL_DMUCR_DMUEN_Msk, CAN_CTRL_DMUCR_DMUEN_Pos, mask);
}

/*!
 * @brief Get DMU enable.
 *
 * @param[in] base: CAN module
 * @return dma enable value
 */
LOCAL_INLINE uint32 Can_Reg_GetDMU(CAN_CTRL_Type const * const base)
{
    return EXTRACT_BIT32(base->DMUCR, CAN_CTRL_DMUCR_DMUEN_Msk, CAN_CTRL_DMUCR_DMUEN_Pos);
}

/*!
 * @brief Set extern timestamp enable.
 *
 * @param[in] base: CAN module
 * @param[in] mask: CAN channel mask
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetExtTimestampEn(CAN_CTRL_Type *base, uint8 mask)
{
    MODIFY_REG32(base->ETSC, CAN_CTRL_ETSC_ETSE_Msk, CAN_CTRL_ETSC_ETSE_Pos, mask);
}

/*!
 * @brief Set extern timestamp enable.
 *
 * @param[in] base: CAN module
 * @param[in] mask: CAN channel mask
 * @return none
 */
LOCAL_INLINE uint8 Can_Reg_GetExtTimestampEn(CAN_CTRL_Type const * const base)
{
    return (uint8)EXTRACT_BIT32(base->ETSC, CAN_CTRL_ETSC_ETSE_Msk, CAN_CTRL_ETSC_ETSE_Pos);
}

/*!
 * @brief Set extern timestamp prescaler 0.
 *
 * @param[in] base: CAN module
 * @param[in] mask: CAN channel mask
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetExtTimestamp0(CAN_CTRL_Type *base, uint8 value)
{
    MODIFY_REG32(base->ETCP0, CAN_CTRL_ETCP0_TCP_Msk, CAN_CTRL_ETCP0_TCP_Pos, value);
}

/*!
 * @brief Set extern timestamp prescaler 1.
 *
 * @param[in] base: CAN module
 * @param[in] mask: CAN channel mask
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetExtTimestamp1(CAN_CTRL_Type *base, uint8 value)
{
    MODIFY_REG32(base->ETCP1, CAN_CTRL_ETCP1_TCP_Msk, CAN_CTRL_ETCP1_TCP_Pos, value);
}

/*!
 * @brief Set software initialization state.
 *
 * @param[in] base: CAN module
 * @param[in] enable: initialization state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetINIT(CAN_Type *base, uint8 enable)
{
    MODIFY_REG32(base->CCCR, CAN_CCCR_INIT_Msk, CAN_CCCR_INIT_Pos, enable);
}

/*!
 * @brief Get software initialization state.
 *
 * @param[in] base: CAN module
 * @return initialization state
 */
LOCAL_INLINE boolean Can_Reg_GetInitState(CAN_Type const * const base)
{
    return (READ_BIT32(base->CCCR, CAN_CCCR_INIT_Msk) != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Set configuration change state.
 *
 * @param[in] base: CAN module
 * @param[in] enable: configuration change state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetCCE(CAN_Type *base, boolean enable)
{
    MODIFY_REG32(base->CCCR, CAN_CCCR_CCE_Msk, CAN_CCCR_CCE_Pos, enable);
}

/*!
 * @brief Get configuration change state.
 *
 * @param[in] base: CAN module
 * @return configuration change state
 */
LOCAL_INLINE boolean Can_Reg_GetCCEState(CAN_Type const * const base)
{
    return (READ_BIT32(base->CCCR, CAN_CCCR_CCE_Msk) != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Set transmit pause state.
 *
 * @param[in] base: CAN module
 * @param[in] enable: transmit pause state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetTXP(CAN_Type *base, uint8 enable)
{
    MODIFY_REG32(base->CCCR, CAN_CCCR_TXP_Msk, CAN_CCCR_TXP_Pos, enable);
}

/*!
 * @brief Set edge filtering during bus integration.
 *
 * @param[in] base: CAN module
 * @param[in] enable: edge filtering state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetEFBI(CAN_Type *base, uint8 enable)
{
    MODIFY_REG32(base->CCCR, CAN_CCCR_EFBI_Msk, CAN_CCCR_EFBI_Pos, enable);
}

/*!
 * @brief Set protocol exception handling disable.
 *
 * @param[in] base: CAN module
 * @param[in] enable: protocol exception handling disable state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetPXHD(CAN_Type *base, uint8 enable)
{
    MODIFY_REG32(base->CCCR, CAN_CCCR_PXHD_Msk, CAN_CCCR_PXHD_Pos, enable);
}

/*!
 * @brief Set disable automatic retransmission.
 *
 * @param[in] base: CAN module
 * @param[in] enable: automatic retransmission state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetDAR(CAN_Type *base, uint8 enable)
{
    MODIFY_REG32(base->CCCR, CAN_CCCR_DAR_Msk, CAN_CCCR_DAR_Pos, enable);
}

/*!
 * @brief Set bus monitoring mode.
 *
 * @param[in] base: CAN module
 * @param[in] enable: monitoring state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetBusMonitor(CAN_Type *base, uint8 enable)
{
    MODIFY_REG32(base->CCCR, CAN_CCCR_MON_Msk, CAN_CCCR_MON_Pos, enable);
}

/*!
 * @brief Set power down (clock stop request).
 *
 * @param[in] base: CAN module
 * @param[in] enable: power down state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetPowerDown(CAN_Type *base, uint8 enable)
{
    MODIFY_REG32(base->CCCR, CAN_CCCR_CSR_Msk, CAN_CCCR_CSR_Pos, enable);
}

/*!
 * @brief Set restricted operation mode.
 *
 * @param[in] base: CAN module
 * @param[in] enable: restricted operation state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetASM(CAN_Type *base, uint8 enable)
{
    MODIFY_REG32(base->CCCR, CAN_CCCR_ASM_Msk, CAN_CCCR_ASM_Pos, enable);
}

/*!
 * @brief Get clock stop acknowledge.
 *
 * @param[in] base: CAN module
 * @return clock stop state
 */
LOCAL_INLINE boolean Can_Reg_GetClockStop(CAN_Type const * const base)
{
    return (READ_BIT32(base->CCCR, CAN_CCCR_CSA_Msk) != 0U) ? TRUE : FALSE ;
}

/*!
 * @brief Set no ISO mode.
 *
 * @param[in] base: CAN module
 * @param[in] enable: no ISO mode
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetNISO(CAN_Type *base, uint8 enable)
{
    MODIFY_REG32(base->CCCR, CAN_CCCR_NISO_Msk, CAN_CCCR_NISO_Pos, enable);
}

/*!
 * @brief Set bit rate switch enable.
 *
 * @param[in] base: CAN module
 * @param[in] enable: bit rate switch state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetBRSE(CAN_Type *base, uint8 enable)
{
    MODIFY_REG32(base->CCCR, CAN_CCCR_BRSE_Msk, CAN_CCCR_BRSE_Pos, enable);
}

/*!
 * @brief Set FD operation enable.
 *
 * @param[in] base: CAN module
 * @param[in] enable: FD operation state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetFDOE(CAN_Type *base, uint8 enable)
{
    MODIFY_REG32(base->CCCR, CAN_CCCR_FDOE_Msk, CAN_CCCR_FDOE_Pos, enable);
}

/*!
 * @brief Set test mode enable.
 *
 * @param[in] base: CAN module
 * @param[in] enable: test mode state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetTest(CAN_Type *base, uint8 enable)
{
    MODIFY_REG32(base->CCCR, CAN_CCCR_TEST_Msk, CAN_CCCR_TEST_Pos, enable);
}

/*!
 * @brief Set loop back mode enable.
 *
 * @param[in] base: CAN module
 * @param[in] enable: loop back mode state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetLoopBack(CAN_Type *base, uint8 enable)
{
    MODIFY_REG32(base->TEST, CAN_TEST_LBCK_Msk, CAN_TEST_LBCK_Pos, enable);
}

/*!
 * @brief Set control of transmit pin.
 *
 * @param[in] base: CAN module
 * @param[in] mode: transmit pin mode
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetTxPin(CAN_Type *base, uint8 mode)
{
    MODIFY_REG32(base->TEST, CAN_TEST_TX_Msk, CAN_TEST_TX_Pos, mode);
}

/*!
 * @brief Get receive pin.
 *
 * @param[in] base: CAN module
 * @return receive pin value
 */
LOCAL_INLINE boolean Can_Reg_GetRxPin(CAN_Type const * const base)
{
    return (READ_BIT32(base->TEST, CAN_TEST_RX_Msk) != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Get started valid.
 *
 * @param[in] base: CAN module
 * @return valid state
 */
LOCAL_INLINE boolean Can_Reg_GetSVAL(CAN_Type const * const base)
{
    return (READ_BIT32(base->TEST, CAN_TEST_SVAL_Msk) != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Get prepared valid.
 *
 * @param[in] base: CAN module
 * @return valid state
 */
LOCAL_INLINE boolean Can_Reg_GetPVAL(CAN_Type const * const base)
{
    return (READ_BIT32(base->TEST, CAN_TEST_PVAL_Msk) != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Get tx buffer number started.
 *
 * @param[in] base: CAN module
 * @return tx buffer number
 */
LOCAL_INLINE uint32 Can_Reg_GetTXBNS(CAN_Type const * const base)
{
    return EXTRACT_BIT32(base->TEST, CAN_TEST_TXBNS_Msk, CAN_TEST_TXBNS_Pos);
}

/*!
 * @brief Get tx buffer number prepared.
 *
 * @param[in] base: CAN module
 * @return tx buffer number
 */
LOCAL_INLINE uint32 Can_Reg_GetTXBNP(CAN_Type const * const base)
{
    return EXTRACT_BIT32(base->TEST, CAN_TEST_TXBNP_Msk, CAN_TEST_TXBNP_Pos);
}

/*!
 * @brief Set timeout counter enable.
 *
 * @param[in] base: CAN module
 * @param[in] enable: enable state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetTimeoutEn(CAN_Type *base, uint8 enable)
{
    MODIFY_REG32(base->TOCC, CAN_TOCC_ETOC_Msk, CAN_TOCC_ETOC_Pos, enable);
}

/*!
 * @brief Get core release.
 *
 * @param[in] base: CAN module
 * @return core release revision
 */
LOCAL_INLINE uint32 Can_Reg_GetVersion(CAN_Type const * const base)
{
    return READ_REG32(base->CREL);
}

/*!
 * @brief Get endianness test value.
 *
 * @param[in] base: CAN module
 * @return endianness test value
 */
LOCAL_INLINE uint32 Can_Reg_GetEndian(CAN_Type const * const base)
{
    return READ_REG32(base->ENDN);
}

/*!
 * @brief Set core release.
 *
 * @param[in] base: CAN module
 * @param[in] enable: test mode state
 * @return core release revision
 */
LOCAL_INLINE uint32 Can_Reg_GetDMUVersion(CAN_Type const * const base)
{
    return READ_REG32(base->DMUCR);
}

/*!
 * @brief Set SSP offset.
 *
 * @param[in] base: CAN module
 * @param[in] offset: SSP offset
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetTDCOffset(CAN_Type *base, uint8 offset)
{
    MODIFY_REG32(base->TDCR, CAN_TDCR_TDCO_Msk, CAN_TDCR_TDCO_Pos, offset);
}

/*!
 * @brief Set TDC enable.
 *
 * @param[in] base: CAN module
 * @param[in] enable: TDC enable state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetTDC(CAN_Type *base, uint8 enable)
{
    MODIFY_REG32(base->DBTP, CAN_DBTP_TDC_Msk, CAN_DBTP_TDC_Pos, enable);
}

/*!
 * @brief Set TDC filter.
 *
 * @param[in] base: CAN module
 * @param[in] enable: TDC filter enable state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetTDCFilter(CAN_Type *base, uint8 length)
{
    MODIFY_REG32(base->TDCR, CAN_TDCR_TDCF_Msk, CAN_TDCR_TDCF_Pos, length);
}

/*!
 * @brief Get TDC value.
 *
 * @param[in] base: CAN module
 * @return TDC value
 */
LOCAL_INLINE uint32 Can_Reg_GetTDCValue(CAN_Type const * const base)
{
    return EXTRACT_BIT32(base->PSR, CAN_PSR_TDCV_Msk, CAN_PSR_TDCV_Pos);
}

/*!
 * @brief Set extended ID AND mask.
 *
 * @param[in] base: CAN module
 * @param[in] mask: AND mask
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetXIDAM(CAN_Type *base, uint32 mask)
{
    MODIFY_REG32(base->XIDAM, CAN_XIDAM_EIDM_Msk, CAN_XIDAM_EIDM_Pos, mask);
}

/*!
 * @brief Set event fifo watermark.
 *
 * @param[in] base: CAN module
 * @param[in] watermark
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetEventFifoWatermark(CAN_Type *base, uint8 watermark)
{
    MODIFY_REG32(base->TXEFC, CAN_TXEFC_EFWM_Msk, CAN_TXEFC_EFWM_Pos, watermark);
}

/*!
 * @brief Set rx FIFO 0 operation mode.
 *
 * @param[in] base: CAN module
 * @param[in] mode: operation mode
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetRxFifo0Mode(CAN_Type *base, uint8 mode)
{
    MODIFY_REG32(base->RXF0C, CAN_RXF0C_F0OM_Msk, CAN_RXF0C_F0OM_Pos, mode);
}

/*!
 * @brief Set rx FIFO 1 operation mode.
 *
 * @param[in] base: CAN module
 * @param[in] mode: operation mode
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetRxFifo1Mode(CAN_Type *base, uint8 mode)
{
    MODIFY_REG32(base->RXF1C, CAN_RXF1C_F1OM_Msk, CAN_RXF1C_F1OM_Pos, mode);
}

/*!
 * @brief Set rx FIFO 0 watermark.
 *
 * @param[in] base: CAN module
 * @param[in] watermark
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetRxFifo0Watermark(CAN_Type *base, uint8 watermark)
{
    MODIFY_REG32(base->RXF0C, CAN_RXF0C_F0WM_Msk, CAN_RXF0C_F0WM_Pos, watermark);
}

/*!
 * @brief Set rx FIFO 1 watermark.
 *
 * @param[in] base: CAN module
 * @param[in] watermark
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetRxFifo1Watermark(CAN_Type *base, uint8 watermark)
{
    MODIFY_REG32(base->RXF1C, CAN_RXF0C_F0WM_Msk, CAN_RXF0C_F0WM_Pos, watermark);
}

/*!
 * @brief Start transmit frame in the TBUF.
 *
 * @param[in] base: CAN module
 * @param[in] index: buffer index
 * @return none
 */
LOCAL_INLINE void Can_Reg_StartTransmit(CAN_Type *base, uint32 Mask)
{
    SET_BIT32(base->TXBAR, Mask);
}

/*!
 * @brief Abort transmitting.
 *
 * @param[in] base: CAN module
 * @param[in] index: buffer index
 * @return none
 */
LOCAL_INLINE void Can_Reg_AbortTransmit(CAN_Type *base, uint32 mask)
{
    WRITE_REG32(base->TXBCR, mask);
}

/*!
 * @brief Check transmit is pending.
 *
 * @param[in] base: CAN module
 * @param[in] index
 * @return transmit pending state
 */
LOCAL_INLINE boolean Can_Reg_IsTransmitPending(CAN_Type const * const base, uint8 index)
{
    return (READ_BIT32(base->TXBRP, BIT_SHIFT(index)) != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Check all channel transmit is pending.
 *
 * @param[in] base: CAN module
 * @param[in] enable: test mode state
 * @return pending state
 */
LOCAL_INLINE uint32 Can_Reg_AllTransmitPending(CAN_Type const * const base)
{
    return READ_REG32(base->TXBRP);
}

/*!
 * @brief Get transmission occurred.
 *
 * @param[in] base: CAN module
 * @return transmission occurred state
 */
LOCAL_INLINE uint32 Can_Reg_TransmitOccurred(CAN_Type const * const base)
{
    return READ_REG32(base->TXBTO);
}

/*!
 * @brief Check cancellation finished
 *
 * @param[in] base: CAN module
 * @return cancellation finished state
 */
LOCAL_INLINE uint32 Can_Reg_Abortstatus(CAN_Type const * const base)
{
    return READ_REG32(base->TXBCF);
}

/*!
 * @brief Get CAN communication state.
 *
 * @param[in] base: CAN module
 * @return CAN communication state
 */
LOCAL_INLINE uint32 Can_Reg_GetCANActivity(CAN_Type const * const base)
{
    return EXTRACT_BIT32(base->PSR, CAN_PSR_ACT_Msk, CAN_PSR_ACT_Pos);
}

/*!
 * @brief Get transmit rrror counter.
 *
 * @param[in] base: CAN module
 * @return transmit error counter
 */
LOCAL_INLINE uint32 Can_Reg_GetTEC(CAN_Type const * const base)
{
    return EXTRACT_BIT32(base->ECR, CAN_ECR_TEC_Msk, CAN_ECR_TEC_Pos);
}

/*!
 * @brief Get receive error counter.
 *
 * @param[in] base: CAN module
 * @return receive error counter
 */
LOCAL_INLINE uint32 Can_Reg_GetREC(CAN_Type const * const base)
{
    return EXTRACT_BIT32(base->ECR, CAN_ECR_REC_Msk, CAN_ECR_REC_Pos);
}

/*!
 * @brief Get last error code(LEC and DLEC).
 *
 * @param[in] base: CAN module
 * @return error code
 */
LOCAL_INLINE uint32 Can_Reg_GetErrorCode(CAN_Type const * const base)
{
    return READ_BIT32(base->PSR, (CAN_PSR_LEC_Msk | CAN_PSR_DLEC_Msk));
}

/*!
 * @brief Get error passive flag.
 *
 * @param[in] base: CAN module
 * @return Error passive state
 */
LOCAL_INLINE boolean Can_Reg_GetErrorPassiveFlag(CAN_Type const * const base)
{
    return (READ_BIT32(base->PSR, CAN_PSR_EP_Msk) != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Get error warning limit flag.
 *
 * @param[in] base: CAN module
 * @return Error warning limit state
 */
LOCAL_INLINE boolean Can_Reg_GetErrorWarningFlag(CAN_Type const * const base)
{
    return (READ_BIT32(base->PSR, CAN_PSR_EW_Msk) != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Check bus off state.
 *
 * @param[in] base: CAN module
 * @return Bus off state
 */
LOCAL_INLINE boolean Can_Reg_IsBusOff(CAN_Type const * const base)
{
    return (READ_BIT32(base->PSR, CAN_PSR_BO_Msk) != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Get high priority message status.
 *
 * @param[in] base: CAN module
 * @param[in] status: pointer to message status
 * @return none
 */
LOCAL_INLINE void Can_Reg_GetHPMS(CAN_Type const * const base, Can_HpmsStatusType *status)
{
    status->FLST = (uint8)EXTRACT_BIT32(base->HPMS, CAN_HPMS_FLST_Msk, CAN_HPMS_FLST_Pos);
    status->FIDX = (uint8)EXTRACT_BIT32(base->HPMS, CAN_HPMS_FIDX_Msk, CAN_HPMS_FIDX_Pos);
    status->MSI = (uint8)EXTRACT_BIT32(base->HPMS, CAN_HPMS_MSI_Msk, CAN_HPMS_MSI_Pos);
    status->BIDX = (uint8)EXTRACT_BIT32(base->HPMS, CAN_HPMS_BIDX_Msk, CAN_HPMS_BIDX_Pos);
}

/*!
 * @brief Get high priority message status.
 *
 * @param[in] base: CAN module
 * @param[in] status: pointer to message status
 * @return none
 */
LOCAL_INLINE uint32 Can_Reg_GetEP(CAN_Type const * const base)
{
    return EXTRACT_BIT32(base->IR, CAN_IR_EP_Msk, CAN_IR_EP_Pos);
}

/*!
 * @brief Get dedicate rx buffer new data 1 state.
 *
 * @param[in] base: CAN module
 * @return new data state
 */
LOCAL_INLINE uint32 Can_Reg_GetNewData1(CAN_Type const * const base)
{
    return READ_REG32(base->NDAT1);
}

/*!
 * @brief Get dedicate rx buffer new data 1 state.
 *
 * @param[in] base: CAN module
 * @return new data state
 */
LOCAL_INLINE void Can_Reg_SetNewData1(CAN_Type *base, uint32 val)
{
    WRITE_REG32(base->NDAT1, val);
}

/*!
 * @brief Get dedicate rx buffer new data 2 state.
 *
 * @param[in] base: CAN module
 * @return new data state
 */
LOCAL_INLINE uint32 Can_Reg_GetNewData2(CAN_Type const * const base)
{
    return READ_REG32(base->NDAT2);
}

/*!
 * @brief Get dedicate rx buffer new data 2 state.
 *
 * @param[in] base: CAN module
 * @return new data state
 */
LOCAL_INLINE void Can_Reg_SetNewData2(CAN_Type *base, uint32 val)
{
    WRITE_REG32(base->NDAT2, val);
}

/*!
 * @brief Get rx fifo 0 fill level.
 *
 * @param[in] base: CAN module
 * @return fill level
 */
LOCAL_INLINE uint32 Can_Reg_GetRxFifo0Fill(CAN_Type const * const base)
{
    return EXTRACT_BIT32(base->RXF0S, CAN_RXF0S_F0FL_Msk, CAN_RXF0S_F0FL_Pos);
}

/*!
 * @brief Get rx fifo 1 fill level.
 *
 * @param[in] base: CAN module
 * @return fill level
 */
LOCAL_INLINE uint32 Can_Reg_GetRxFifo1Fill(CAN_Type const * const base)
{
    return EXTRACT_BIT32(base->RXF1S, CAN_RXF1S_F1FL_Msk, CAN_RXF1S_F1FL_Pos);
}

/*!
 * @brief Get rx fifo 0 get index.
 *
 * @param[in] base: CAN module
 * @return get index
 */
LOCAL_INLINE uint8 Can_Reg_GetRxFifo0GetIndex(CAN_Type const * const base)
{
    return (uint8)EXTRACT_BIT32(base->RXF0S, CAN_RXF0S_F0GI_Msk, CAN_RXF0S_F0GI_Pos);
}

/*!
 * @brief Get rx fifo 1 get index.
 *
 * @param[in] base: CAN module
 * @return get index
 */
LOCAL_INLINE uint8 Can_Reg_GetRxFifo1GetIndex(CAN_Type const * const base)
{
    return (uint8)EXTRACT_BIT32(base->RXF1S, CAN_RXF1S_F1GI_Msk, CAN_RXF1S_F1GI_Pos);
}

/*!
 * @brief Get rx fifo 0 put index.
 *
 * @param[in] base: CAN module
 * @return put index
 */
LOCAL_INLINE uint8 Can_Reg_GetRxFifo0PutIndex(CAN_Type const * const base)
{
    return (uint8)EXTRACT_BIT32(base->RXF0S, CAN_RXF0S_F0PI_Msk, CAN_RXF0S_F0PI_Pos);
}

/*!
 * @brief Get rx fifo 1 put index.
 *
 * @param[in] base: CAN module
 * @return put index
 */
LOCAL_INLINE uint8 Can_Reg_GetRxFifo1PutIndex(CAN_Type const * const base)
{
    return (uint8)EXTRACT_BIT32(base->RXF1S, CAN_RXF1S_F1PI_Msk, CAN_RXF1S_F1PI_Pos);
}

/*!
 * @brief Get rx fifo 0 full state.
 *
 * @param[in] base: CAN module
 * @return full state
 */
LOCAL_INLINE boolean Can_Reg_GetRxFifo0Full(CAN_Type const * const base)
{
    return (READ_BIT32(base->RXF0S, CAN_RXF0S_F0F_Msk) != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Get rx fifo 1 full state.
 *
 * @param[in] base: CAN module
 * @return full state
 */
LOCAL_INLINE boolean Can_Reg_GetRxFifo1Full(CAN_Type const * const base)
{
    return (READ_BIT32(base->RXF1S, CAN_RXF1S_F1F_Msk) != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Get rx fifo 0 message lost.
 *
 * @param[in] base: CAN module
 * @return lost state
 */
LOCAL_INLINE boolean Can_Reg_GetRxFifo0Lost(CAN_Type const * const base)
{
    return (READ_BIT32(base->RXF0S, CAN_RXF0S_RF0L_Msk) != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Get rx fifo 1 message lost.
 *
 * @param[in] base: CAN module
 * @return lost state
 */
LOCAL_INLINE boolean Can_Reg_GetRxFifo1Lost(CAN_Type const * const base)
{
    return (READ_BIT32(base->RXF1S, CAN_RXF1S_RF1L_Msk) != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Get tx fifo/queue full state.
 *
 * @param[in] base: CAN module
 * @return full state
 */
LOCAL_INLINE boolean Can_Reg_GetTxFQFull(CAN_Type const * const base)
{
    return (READ_BIT32(base->TXFQS, CAN_TXFQS_TFQF_Msk) != 0U) ? TRUE : FALSE;
}

/*!
 * @brief Get tx fifo free level.
 *
 * @param[in] base: CAN module
 * @return free level
 */
LOCAL_INLINE uint32 Can_Reg_GetTxFifoFreeLevel(CAN_Type const * const base)
{
    return EXTRACT_BIT32(base->TXFQS, CAN_TXFQS_TFFL_Msk, CAN_TXFQS_TFFL_Pos);
}

/*!
 * @brief Get tx fifo/queue put index.
 *
 * @param[in] base: CAN module
 * @return put index
 */
LOCAL_INLINE uint8 Can_Reg_GetTxFQPutIndex(CAN_Type const * const base)
{
    return (uint8)EXTRACT_BIT32(base->TXFQS, CAN_TXFQS_TFQPI_Msk, CAN_TXFQS_TFQPI_Pos);
}

/*!
 * @brief Get tx fifo get index.
 *
 * @param[in] base: CAN module
 * @return get index
 */
LOCAL_INLINE uint8 Can_Reg_GetTxFifoGetIndex(CAN_Type const * const base)
{
    return (uint8)EXTRACT_BIT32(base->TXFQS, CAN_TXFQS_TFGI_Msk, CAN_TXFQS_TFGI_Pos);
}

/*!
 * @brief Get tx event fifo element lost.
 *
 * @param[in] base: CAN module
 * @return lost state
 */
LOCAL_INLINE uint32 Can_Reg_GetTxEventFifoLost(CAN_Type const * const base)
{
    return EXTRACT_BIT32(base->TXEFS, CAN_TXEFS_TEFL_Msk, CAN_TXEFS_TEFL_Pos);
}

/*!
 * @brief Get tx event fifo full.
 *
 * @param[in] base: CAN module
 * @return full state
 */
LOCAL_INLINE uint32 Can_Reg_GetTxEventFifoFull(CAN_Type const * const base)
{
    return EXTRACT_BIT32(base->TXEFS, CAN_TXEFS_EFF_Msk, CAN_TXEFS_EFF_Pos);
}

/*!
 * @brief Get tx event fifo put index.
 *
 * @param[in] base: CAN module
 * @return put index
 */
LOCAL_INLINE uint32 Can_Reg_GetTxEventFifoPutIndex(CAN_Type const * const base)
{
    return EXTRACT_BIT32(base->TXEFS, CAN_TXEFS_EFPI_Msk, CAN_TXEFS_EFPI_Pos);
}

/*!
 * @brief Get tx event fifo get index.
 *
 * @param[in] base: CAN module
 * @return get index
 */
LOCAL_INLINE uint32 Can_Reg_GetTxEventFifoGetIndex(CAN_Type const * const base)
{
    return EXTRACT_BIT32(base->TXEFS, CAN_TXEFS_EFGI_Msk, CAN_TXEFS_EFGI_Pos);
}

/*!
 * @brief Get tx event fifo fill level.
 *
 * @param[in] base: CAN module
 * @return fill level
 */
LOCAL_INLINE uint32 Can_Reg_GetTxEventFifoFill(CAN_Type const * const base)
{
    return EXTRACT_BIT32(base->TXEFS, CAN_TXEFS_EFFL_Msk, CAN_TXEFS_EFFL_Pos);
}

/*!
 * @brief Clear CAN interrupt flag.
 *
 * @param[in] base: CAN module
 * @param[in] flag: interrupt flag
 * @return none
 */
LOCAL_INLINE void Can_Reg_ClearIR(CAN_Type *base, uint32 flag)
{
    WRITE_REG32(base->IR, flag);
}

/*!
 * @brief Get CAN interrupt flag.
 *
 * @param[in] base: CAN module
 * @return CAN interrupt flag
 */
LOCAL_INLINE uint32 Can_Reg_GetIR(CAN_Type const * const base)
{
    return READ_REG32(base->IR);
}

/*!
 * @brief Set interrupt enable mask.
 *
 * @param[in] base: CAN module
 * @param[in] mask: interrupt enable mask
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetIE(CAN_Type *base, uint32 mask)
{
    MODIFY_REG32(base->IE, CAN_COMMON_IRQ_EN_MASKS, 0U, mask);
}

/*!
 * @brief Get interrupt enable mask.
 *
 * @param[in] base: CAN module
 * @return enable mask
 */
LOCAL_INLINE uint32 Can_Reg_GetIE(CAN_Type const * const base)
{
    return READ_BIT32(base->IE, CAN_COMMON_IRQ_EN_MASKS);
}

/*!
 * @brief Set tx complete interrupt enable mask.
 *
 * @param[in] base: CAN module
 * @param[in] mask: interrupt enable mask
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetTxIE(CAN_Type *base, uint32 mask)
{
    MODIFY_REG32(base->TXBTIE, CAN_TXBTIE_TIE_Msk, CAN_TXBTIE_TIE_Pos, mask);
}

/*!
 * @brief Set tx cancel finish interrupt enable mask.
 *
 * @param[in] base: CAN module
 * @param[in] mask: interrupt enable mask
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetTxCancelIE(CAN_Type *base, uint32 mask)
{
    MODIFY_REG32(base->TXBCIE, CAN_TXBCIE_CFIE_Msk, CAN_TXBCIE_CFIE_Pos, mask);
}

/*!
 * @brief Get timeout counter.
 *
 * @param[in] base: CAN module
 * @return timeout counter
 */
LOCAL_INLINE uint32 Can_Reg_GetTimeoutCounter(CAN_Type const * const base)
{
    return READ_BIT32(base->TOCV, CAN_TOCV_TOC_Msk);
}

/*!
 * @brief Clear DMU interrupt flag.
 *
 * @param[in] base: CAN module
 * @param[in] flag: DMU interrupt flag
 * @return none
 */
LOCAL_INLINE void Can_Reg_ClearDMUIR(CAN_Type *base, uint32 flag)
{
    WRITE_REG32(base->DMUIR, flag);
}

/*!
 * @brief Get DMU interrupt flag.
 *
 * @param[in] base: CAN module
 * @return interrupt flag
 */
LOCAL_INLINE uint32 Can_Reg_GetDMUIR(CAN_Type const * const base)
{
    return READ_REG32(base->DMUIR);
}

/*!
 * @brief Set DMU interrupt enable mask.
 *
 * @param[in] base: CAN module
 * @param[in] mask: interrupt enable mask
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetDMUIE(CAN_Type *base, uint32 mask)
{
    MODIFY_REG32(base->DMUIE, DMU_IRQ_ALL_ENABLE_MSK, 0U, mask);
}

/*!
 * @brief Get DMU interrupt enable mask.
 *
 * @param[in] base: CAN module
 * @return enable mask
 */
LOCAL_INLINE uint32 Can_Reg_GetDMUIE(CAN_Type const * const base)
{
    return READ_BIT32(base->DMUIE, DMU_IRQ_ALL_ENABLE_MSK);
}

/*!
 * @brief Set DMU detect element handler state.
 *
 * @param[in] base: CAN module
 * @param[in] state: test mode state
 * @return none
 */
LOCAL_INLINE void Can_Reg_SetDMUHandlerState(CAN_Type *base, uint8 state)
{
    MODIFY_REG32(base->DMUI, CAN_DMUI_DEHS_Msk, CAN_DMUI_DEHS_Pos, state);
}

/*!
 * @brief Get DMU element handler state.
 *
 * @param[in] base: CAN module
 * @return element handler state
 */
LOCAL_INLINE uint32 Can_Reg_GetDMUHandlerState(CAN_Type const * const base)
{
    return EXTRACT_BIT32(base->DMUI, CAN_DMUI_EHS_Msk, CAN_DMUI_EHS_Pos);
}
#endif
#ifdef __cplusplus
}
#endif
#endif
