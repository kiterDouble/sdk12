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
 * @file AC784xx_Spm_Reg.h
 *
 * @brief This file provides extern Low level Spm register api.
 *
 */

#ifndef AC784XX_SPM_REG_H
#define AC784XX_SPM_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* endif of __cplusplus */
/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/*!
 * @brief Set the power mode.
 * @note Function ID: DES_MCU_API_409
 * @param[in] Mode: power mode register value
 * @return void
 */
LOCAL_INLINE void Spm_Reg_SetPowerMode(uint32 Mode)
{
    SET_BIT32(SPM->PWR_MGR_CFG0, SPM_PWR_MGR_CFG0_PWR_EN_Msk);
    MODIFY_REG32(SPM->PWR_MGR_CFG0, SPM_PWR_MGR_CFG0_POWER_MODE_Msk, SPM_PWR_MGR_CFG0_POWER_MODE_Pos, Mode);
    /* enable AWIC */
    SET_BIT32(SPM->PWR_MGR_CFG0, SPM_PWR_MGR_CFG0_AWIC_EN_Msk);
}

/*!
 * @brief Get the current power mode.
 * @note Function ID: DES_MCU_API_410
 * @return the current power mode register value
 */
LOCAL_INLINE uint32 Spm_Reg_GetPowerMode(void)
{
    return READ_BIT32(SPM->STATUS, SPM_STATUS_CURR_POWER_MODE_Msk);
}

/*!
 * @brief Get the power control mode.
 * @note Function ID: DES_MCU_API_411
 * @return the power control mode register value
 */
LOCAL_INLINE uint32 Spm_Reg_GetPowerControlMode(void)
{
    return EXTRACT_BIT32(SPM->PWR_MGR_CFG0, SPM_PWR_MGR_CFG0_POWER_MODE_Msk, SPM_PWR_MGR_CFG0_POWER_MODE_Pos);
}

/*!
 * @brief Enable or disable XOSC bypass mode.
 * @note Function ID: DES_MCU_API_412
 * @param[in] IsEnable: enable state
 *            - true: enable XOSC bypass
 *            - false: disable XOSC bypass
 * @return void
 */
LOCAL_INLINE void Spm_Reg_EnableXOSCBypassMode(boolean IsEnable)
{
    uint32 Enable = (TRUE == IsEnable) ? 1UL : 0UL;

    MODIFY_REG32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_XOSC_HSEBYP_Msk, SPM_PWR_MGR_CFG1_XOSC_HSEBYP_Pos, Enable);
}

#if defined (AC7840X) || defined(AC7843X)
/*!
 * @brief Enable or disable HSI.
 * @note Function ID: DES_MCU_API_413
 * @param[in] En: enable state
 *            - true: enable HSI
 *            - false: disable HSI
 * @return void
 */
LOCAL_INLINE void Spm_Reg_EnableHSI(boolean IsEnable)
{
    uint32 Enable = (TRUE == IsEnable) ? 1UL : 0UL;

    MODIFY_REG32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_HSI_EN_NORMAL_Msk, SPM_PWR_MGR_CFG1_HSI_EN_NORMAL_Pos, Enable);
}

/*!
 * @brief Get HSI Enable.
 * @note Function ID: DES_MCU_API_414
 * @return The HSI status
 *         - true : HSI enable
 *         - false: HSI disable
 */
LOCAL_INLINE boolean Spm_Reg_GetHSIEnable(void)
{
    uint32 Value;
    boolean Enable = FALSE;

    Value = EXTRACT_BIT32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_HSI_EN_NORMAL_Msk, SPM_PWR_MGR_CFG1_HSI_EN_NORMAL_Pos);
    if (Value != 0U)
    {
        Enable = TRUE;
    }

    return Enable;
}

/*!
 * @brief Get HSI In VLPS Enable.
 * @note Function ID: DES_MCU_API_415
 * @return The HSI status
 *         - true : HSI enable
 *         - false: HSI disable
 */
LOCAL_INLINE boolean Spm_Reg_GetHSIInVLPSEnable(void)
{
    uint32 Value;
    boolean Enable = FALSE;

    Value = EXTRACT_BIT32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_HSI_EN_VLPS_Msk, SPM_PWR_MGR_CFG1_HSI_EN_VLPS_Pos);
    if (Value != 0U)
    {
        Enable = TRUE;
    }

    return Enable;
}

/*!
 * @brief Enable or disable HSI in VLPS.
 * @note Function ID: DES_MCU_API_416
 * @param[in] IsEnable: enable state
 *            - true: enable HSI in VLPS
 *            - false: disable HSI in VLPS
 * @return void
 */
LOCAL_INLINE void Spm_Reg_EnableHSIInVLPS(boolean IsEnable)
{
    uint32 Enable = (TRUE == IsEnable) ? 1UL : 0UL;

    MODIFY_REG32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_HSI_EN_VLPS_Msk, SPM_PWR_MGR_CFG1_HSI_EN_VLPS_Pos, Enable);
}

/*!
 * @brief Gets HSI status.
 * @note Function ID: DES_MCU_API_417
 * @return The HSI status
 *         - true : HSI valid
 *         - false: HSI invalid
 */

LOCAL_INLINE boolean Spm_Reg_GetHSIStatus(void)
{
    uint32 Value;
    boolean Status = FALSE;

    Value = EXTRACT_BIT32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_HSI_RDY_Msk, SPM_PWR_MGR_CFG1_HSI_RDY_Pos);
    if (Value != 0U)
    {
        Status = TRUE;
    }

    return Status;
}

#elif defined (AC7842X) /* endif of AC7840X AC7843X */
/*!
 * @brief Get VHSI In VLPS Enable.
 * @note Function ID: DES_MCU_API_418
 * @return The VHSI status
 *         - true : VHSI enable
 *         - false: VHSI disable
 */
LOCAL_INLINE boolean Spm_Reg_GetVHSIInVLPSEnable(void)
{
    uint32 Value;
    boolean Enable = FALSE;

    Value = EXTRACT_BIT32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_VHSI_EN_VLPS_Msk, SPM_PWR_MGR_CFG1_VHSI_EN_VLPS_Pos);
    if (Value != 0U)
    {
        Enable = TRUE;
    }

    return Enable;
}

/*!
 * @brief Enable or disable VHSI in VLPS.
 * @note Function ID: DES_MCU_API_419
 * @param[in] IsEnable: enable state
 *            - true: enable VHSI in VLPS
 *            - false: disable VHSI in VLPS
 * @return void
 */
LOCAL_INLINE void Spm_Reg_EnableVHSIInVLPS(boolean IsEnable)
{
    uint32 Enable = (TRUE == IsEnable) ? 1UL : 0UL;

    MODIFY_REG32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_VHSI_EN_VLPS_Msk, SPM_PWR_MGR_CFG1_VHSI_EN_VLPS_Pos, Enable);
}

#endif /* endif of AC7842X */

/*!
 * @brief Get VHSI Enable.
 * @note Function ID: DES_MCU_API_420
 * @return The VHSI status
 *         - true : VHSI enable
 *         - false: VHSI disable
 * @return void
 */
LOCAL_INLINE boolean Spm_Reg_GetVHSIEnable(void)
{
    uint32 Value;
    boolean Enable = FALSE;

    Value = EXTRACT_BIT32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_VHSI_EN_Msk, SPM_PWR_MGR_CFG1_VHSI_EN_Pos);
    if (Value != 0U)
    {
        Enable = TRUE;
    }

    return Enable;
}

/*!
 * @brief Enable or disable VHSI.
 * @note Function ID: DES_MCU_API_421
 * @param[in] IsEnable: enable state
 *            - true: enable VHSI
 *            - false: disable VHSI
 * @return void
 */
LOCAL_INLINE void Spm_Reg_EnableVHSI(boolean IsEnable)
{
    uint32 Enable = (TRUE == IsEnable) ? 1UL : 0UL;

    MODIFY_REG32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_VHSI_EN_Msk, SPM_PWR_MGR_CFG1_VHSI_EN_Pos, Enable);
}

/*!
 * @brief Get VHSI status.
 * @note Function ID: DES_MCU_API_422
 * @return The VHSI status
 *         - true : VHSI valid
 *         - false: VHSI invalid
 */
LOCAL_INLINE boolean Spm_Reg_GetVHSIStatus(void)
{
    uint32 Value;
    boolean Status = FALSE;

    Value = EXTRACT_BIT32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_VHSI_RDY_Msk, SPM_PWR_MGR_CFG1_VHSI_RDY_Pos);
    if (Value != 0U)
    {
        Status = TRUE;
    }

    return Status;
}

/*!
 * @brief Get XOSC Enable.
 * @note Function ID: DES_MCU_API_423
 * @return The XOSC status
 *         - true : XOSC enable
 *         - false: XOSC disable
 */
LOCAL_INLINE boolean Spm_Reg_GetXOSCEnable(void)
{
    uint32 Value;
    boolean Enable = FALSE;

    Value = EXTRACT_BIT32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_XOSC_HSEEN_Msk, SPM_PWR_MGR_CFG1_XOSC_HSEEN_Pos);
    if (Value != 0U)
    {
        Enable = TRUE;
    }

    return Enable;
}

/*!
 * @brief Enable or disable XOSC.
 * @note Function ID: DES_MCU_API_424
 * @param[in] IsEnable: enable state
 *            - true: enable XOSC
 *            - false: disable XOSC
 * @return void
 */
LOCAL_INLINE void Spm_Reg_EnableXOSC(boolean IsEnable)
{
    uint32 Enable = (TRUE == IsEnable) ? 1UL : 0UL;

    MODIFY_REG32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_XOSC_HSEEN_Msk, SPM_PWR_MGR_CFG1_XOSC_HSEEN_Pos, Enable);
}

/*!
 * @brief Get XOSC status.
 * @note Function ID: DES_MCU_API_425
 * @return The XOSC status
 *         - true : XOSC valid
 *         - false: XOSC invalid
 */
LOCAL_INLINE boolean Spm_Reg_GetXOSCStatus(void)
{
    uint32 Value;
    boolean Status = FALSE;

    Value = EXTRACT_BIT32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_XOSC_RDY_Msk, SPM_PWR_MGR_CFG1_XOSC_RDY_Pos);
    if (Value != 0U)
    {
        Status = TRUE;
    }

    return Status;
}

/*!
 * @brief Get SPLL Enable.
 * @note Function ID: DES_MCU_API_426
 * @return The SPLL status
 *         - true : SPLL enable
 *         - false: SPLL disable
 */
LOCAL_INLINE boolean Spm_Reg_GetSPLLEnable(void)
{
    uint32 Value;
    boolean Enable = FALSE;

    Value = EXTRACT_BIT32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_SPLL_EN_Msk, SPM_PWR_MGR_CFG1_SPLL_EN_Pos);
    if (Value != 0U)
    {
        Enable = TRUE;
    }

    return Enable;
}

/*!
 * @brief Enable or disable SPLL.
 * @note Function ID: DES_MCU_API_427
 * @param[in] IsEnable: enable state
 *            - true: enable SPLL
 *            - false: disable SPLL
 * @return void
 */
LOCAL_INLINE void Spm_Reg_EnableSPLL(boolean IsEnable)
{
    uint32 Enable = (TRUE == IsEnable) ? 1UL : 0UL;

    MODIFY_REG32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_SPLL_EN_Msk, SPM_PWR_MGR_CFG1_SPLL_EN_Pos, Enable);
}

/*!
 * @brief Get SPLL status.
 * @note Function ID: DES_MCU_API_428
 * @return The SPLL status
 *         - true : SPLL valid
 *         - false: SPLL invalid
 */
LOCAL_INLINE boolean Spm_Reg_GetSPLLStatus(void)
{
    uint32 Value;
    boolean Status = FALSE;

    Value = EXTRACT_BIT32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_SPLL_RDY_Msk, SPM_PWR_MGR_CFG1_SPLL_RDY_Pos);
    if (Value != 0U)
    {
        Status = TRUE;
    }

    return Status;
}

/*!
 * @brief Get sleep ack status.
 * @note Function ID: DES_MCU_API_429
 * @return sleep ack status value
 *         - BIT0:SLEEP_ACK_I2C0
 *         - BIT2:SLEEP_ACK_SPI0
 *         - BIT3:SLEEP_ACK_SPI1
 *         - BIT4:SLEEP_ACK_SPI2
 *         - BIT5:SLEEP_ACK_CAN0
 *         - BIT6:SLEEP_ACK_CAN1
 *         - BIT7:SLEEP_ACK_CAN2
 *         - BIT8:SLEEP_ACK_CAN3
 *         - BIT9: SLEEP_ACK_CAN4
 *         - BIT10: SLEEP_ACK_CAN5
 *         - BIT11:SLEEP_ACK_UART0
 *         - BIT12:SLEEP_ACK_UART1
 *         - BIT13:SLEEP_ACK_UART2
 *         - BIT14:SLEEP_ACK_UART3
 *         - BIT15: SLEEP_ACK_UART4
 *         - BIT16: SLEEP_ACK_UART5
 *         - BIT17:SLEEP_ACK_DMA0
 *         - BIT18:SLEEP_ACK_EIO
 *         - BIT19:SLEEP_ACK_FLASH
 *         - BIT20: SLEEP_ACK_I2C2
 *         - BIT21: SLEEP_ACK_SPI3
 *         - BIT22: SLEEP_ACK_SPI4
 *         - BIT23: SLEEP_ACK_UART6
 *         - BIT24: SLEEP_ACK_UART7
 *         - BIT25: SLEEP_ACK_SENT
 *         - BIT30: SLEEP_ACK_CACHE
 */
LOCAL_INLINE uint32 Spm_Reg_GetModuleSleepACKStatus(void)
{
    return READ_REG32(SPM->PERIPH_SLEEP_ACK_STATUS);
}

/*!
 * @brief Select reset or interrupt after ack time out.
 * @note Function ID: DES_MCU_API_430
 * @param[in] IsReset: action after ack timeout.
 *            - 0: interrupt
 *            - 1: cpu reset
 * @return void
 */
LOCAL_INLINE void Spm_Reg_SelectACKTimeoutAction(uint32 IsReset)
{
#if defined(AC7840E)
    MODIFY_REG32(SPM->PWR_MGR_CFG0, SPM_PWR_MGR_CFG0_SLP_TIMOUT_SEL_Msk, SPM_PWR_MGR_CFG0_SLP_TIMOUT_SEL_Pos,
                 IsReset);
#elif defined (AC7840X) || defined (AC7842X) || defined (AC7843X) /* AC7840E */
    MODIFY_REG32(SPM->PWR_MGR_CFG0, SPM_PWR_MGR_CFG0_ACK_TIMEOUT_ACTION_Msk, SPM_PWR_MGR_CFG0_ACK_TIMEOUT_ACTION_Pos,
                 IsReset);
#endif
}

/*!
 * @brief Get ack timeout flag.
 * @note Function ID: DES_MCU_API_431
 * @return ack timeout flag.
 */
LOCAL_INLINE uint32 Spm_Reg_GetACKTimeoutFlag(void)
{
#if defined(AC7840E)
    return EXTRACT_BIT32(SPM->STATUS, SPM_STATUS_SLP_ACK_TOF_Msk, SPM_STATUS_SLP_ACK_TOF_Pos);
#elif defined (AC7840X) || defined (AC7842X) || defined (AC7843X) /* AC7840E */
    return EXTRACT_BIT32(SPM->STATUS, SPM_STATUS_ACK_TIMEOUT_FLAG_Msk, SPM_STATUS_ACK_TIMEOUT_FLAG_Pos);
#endif
}

/*!
 * @brief Clear ack timeout flag.
 * @note Function ID: DES_MCU_API_432
 * @return void
 */
LOCAL_INLINE void Spm_Reg_ClearACKTimeOutFlag(void)
{
#if defined(AC7840E)
    SET_BIT32(SPM->STATUS, SPM_STATUS_SLP_ACK_TOF_Msk);
#elif defined (AC7840X) || defined (AC7842X) || defined (AC7843X) /* AC7840E */
    SET_BIT32(SPM->STATUS, SPM_STATUS_ACK_TIMEOUT_FLAG_Msk);
#endif
}

#if defined (AC7843X) || defined (AC7842X) || defined (AC7840X)
/*!
 * @brief Get standby wakeup Eanble.
 * @note Function ID: DES_MCU_API_433
 * @return uint32: standby wakeup Eanble
 */
LOCAL_INLINE uint32 Spm_Reg_GetStandbyWakeupEn(void)
{
    return READ_REG32(SPM->STB_WP_EN);
}

/*!
 * @brief Set standby wakeup status.
 * @note Function ID: DES_MCU_API_434
 * @param[in] En: enable state
 * @return void
 */
LOCAL_INLINE void Spm_Reg_SetStandbyWakeupEn(uint32 En)
{
    WRITE_REG32(SPM->STB_WP_EN, En);
}
#elif defined (AC7840E)

/*!
 * @brief Set sram to lower power mode when mcu enter lower power mode.
 *
 * @param[in] none
 * @return none
 */
LOCAL_INLINE void Spm_Reg_SetSramLpEn(void)
{
    MODIFY_REG32(SPM->SRAM_POW_CTRL0, SPM_SRAM_POW_CTRL0_SRAML_LP_EN_Msk, SPM_SRAM_POW_CTRL0_SRAML_LP_EN_Pos, 0x2U);
    MODIFY_REG32(SPM->SRAM_POW_CTRL1, SPM_SRAM_POW_CTRL1_SRAMU_LP_EN_Msk, SPM_SRAM_POW_CTRL1_SRAMU_LP_EN_Pos, 0x2U);
}

/*!
 * @brief Get standby wakeup Eanble.
 * @note Function ID: DES_MCU_API_433
 * @return uint32: standby wakeup Eanble
 */
LOCAL_INLINE uint32 Spm_Reg_GetStandbyWakeupEn0(void)
{
    return READ_REG32(SPM->STB_WP_EN0);
}

/*!
 * @brief Set standby wakeup status.
 * @note Function ID: DES_MCU_API_434
 * @param[in] En: enable state
 * @return void
 */
LOCAL_INLINE void Spm_Reg_SetStandbyWakeupEn0(uint32 En)
{
    WRITE_REG32(SPM->STB_WP_EN0, En);
}
/*!
 * @brief Get standby wakeup Eanble.
 * @note Function ID: DES_MCU_API_433
 * @return uint32: standby wakeup Eanble
 */
LOCAL_INLINE uint32 Spm_Reg_GetStandbyWakeupEn1(void)
{
    return READ_REG32(SPM->STB_WP_EN1);
}

/*!
 * @brief Set standby wakeup status.
 * @note Function ID: DES_MCU_API_434
 * @param[in] En: enable state
 * @return void
 */
LOCAL_INLINE void Spm_Reg_SetStandbyWakeupEn1(uint32 En)
{
    WRITE_REG32(SPM->STB_WP_EN1, En);
}
#endif

/*!
 * @brief Get standby wakeup status.
 * @note Function ID: DES_MCU_API_435
 * @return uint32: standby wakeup status
 *         - BIT0:SPM_WAKEUP_STATUS_PA12
 *         - BIT1:SPM_WAKEUP_STATUS_PB0
 *         - BIT2:SPM_WAKEUP_STATUS_PB1
 *         - BIT3:SPM_WAKEUP_STATUS_PB12
 *         - BIT4:SPM_WAKEUP_STATUS_PD3
 *         - BIT5:SPM_WAKEUP_STATUS_PC2
 *         - BIT6:SPM_WAKEUP_STATUS_PC3
 *         - BIT7:SPM_WAKEUP_STATUS_PC6
 *         - BIT8:SPM_WAKEUP_STATUS_PC7
 *         - BIT9:SPM_WAKEUP_STATUS_PC16
 *         - BIT10:SPM_WAKEUP_STATUS_PC17
 *         - BIT11:SPM_WAKEUP_STATUS_PD6
 *         - BIT12:SPM_WAKEUP_STATUS_PD7
 *         - BIT13:SPM_WAKEUP_STATUS_PE4
 *         - BIT14:SPM_WAKEUP_STATUS_PE5
 *         - BIT15:SPM_WAKEUP_STATUS_RTC
 *         - BIT16:SPM_WAKEUP_STATUS_FlAG
 */
LOCAL_INLINE uint32 Spm_Reg_GetStandbyWakeupStatus(void)
{
    return READ_REG32(SPM->STB_WP_STATUS);
}

/*!
 * @brief Clear standby wakeup status.
 * @note Function ID: DES_MCU_API_436
 * @return void
 */
LOCAL_INLINE void Spm_Reg_ClearStandbyWakeupStatus(void)
{
    SET_BIT32(SPM->STB_WP_STATUS, 0x1ffffU);
}

/*!
 * @brief Select lvd threshold.
 * @note Function ID: DES_MCU_API_437
 * @param[in] Threshold: Voltage threshold value for LVD
 *            - 0 : SPM_LOW
 *            - 1 : SPM_HIGH
 * @return void
 */
LOCAL_INLINE void Spm_Reg_SelectLVDThreshold(uint32 Threshold)
{
    MODIFY_REG32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_LVD_THRESHOLD_Msk, SPM_PWR_MGR_CFG1_LVD_THRESHOLD_Pos, Threshold);
}

/*!
 * @brief Select lvd threshold.
 * @note Function ID: DES_MCU_API_438
 * @return Threshold: Voltage threshold value for LVD
 *            - 0 : SPM_LOW
 *            - 1 : SPM_HIGH
 */
LOCAL_INLINE uint32 Spm_Reg_GetLVDThreshold(void)
{
    return EXTRACT_BIT32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_LVD_THRESHOLD_Msk, SPM_PWR_MGR_CFG1_LVD_THRESHOLD_Pos);
}

/*!
 * @brief Select lvr threshold.
 * @note Function ID: DES_MCU_API_439
 * @param[in] Threshold: Voltage threshold value for LVR
 *            -0 : SPM_LOW
 *            -1 : SPM_HIGH
 * @return void
 */
LOCAL_INLINE void Spm_Reg_SelectLVRThreshold(uint32 Threshold)
{
    MODIFY_REG32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_LVR_THRESHOLD_Msk, SPM_PWR_MGR_CFG1_LVR_THRESHOLD_Pos, Threshold);
}

/*!
 * @brief Select lvd threshold.
 * @note Function ID: DES_MCU_API_440
 * @return Threshold: Voltage threshold value for LVD
 *            - 0 : SPM_LOW
 *            - 1 : SPM_HIGH
 */
LOCAL_INLINE uint32 Spm_Reg_GetLVRThreshold(void)
{
    return EXTRACT_BIT32(SPM->PWR_MGR_CFG1, SPM_PWR_MGR_CFG1_LVR_THRESHOLD_Msk, SPM_PWR_MGR_CFG1_LVR_THRESHOLD_Pos);
}

#if defined (AC7842X) || defined (AC7843X)
/*!
 * @brief Select cpu continue run or reset after standby wakeup.
 * @note Function ID: DES_MCU_API_441
 * @param[in] IsReset: action after ack timeout.
 *            - 0: cpu continue run
 *            - 1: cpu reset
 * @return void
 */
LOCAL_INLINE void Spm_Reg_SelectStandbyAction(uint32 IsReset)
{
    MODIFY_REG32(SPM->PWR_MGR_CFG0, SPM_PWR_MGR_CFG0_STB_WU_CPU_Msk, SPM_PWR_MGR_CFG0_STB_WU_CPU_Pos, IsReset);
}
#endif /* endif of AC7842X AC7843X */

#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
/*!
 * @brief Get sleep abort flag.
 * @note Function ID: DES_MCU_API_442
 * @return AbortFlag:
 *            - 0 : invalid flag
 *            - 1 : Sleep process was exited due to an interrupt occured
 */
LOCAL_INLINE uint32 Spm_Reg_GetSleepAbortFlag(void)
{
    return EXTRACT_BIT32(SPM->STATUS, SPM_STATUS_SLP_ABORT_Msk, SPM_STATUS_SLP_ABORT_Pos);
}
#endif

#if defined (AC7842X) || defined (AC7843X)
/*!
 * @brief Clear standby wakeup flag.
 * @note Function ID: DES_MCU_API_443
 * @return void
 */
LOCAL_INLINE void Spm_Reg_ClearStandbyWakeupFlag(void)
{
    SET_BIT32(SPM->STATUS, SPM_STATUS_STB_WU_FLG_Msk);
}

/*!
 * @brief Get standby action after standby wakeup.
 * @note Function ID: DES_MCU_API_444
 * @return Action:
 *            - 0 : cpu reset
 *            - 1 : cpu continue run
 */
LOCAL_INLINE uint32 Spm_Reg_GetStandbyAction(void)
{
    return EXTRACT_BIT32(SPM->PWR_MGR_CFG0, SPM_PWR_MGR_CFG0_STB_WU_CPU_Msk, SPM_PWR_MGR_CFG0_STB_WU_CPU_Pos);
}

/*!
 * @brief Enable/disable standby mode external reset.
 * @note Function ID: DES_MCU_API_445
 * @param[in] Mask: enable/disable standby mode external reset.
 *            - 0: disable
 *            - 1: enable
 * @return void
 */
LOCAL_INLINE void Spm_Reg_SetStandbyResetMask(uint32 Mask)
{
    MODIFY_REG32(SPM->PWR_MGR_CFG0, SPM_PWR_MGR_CFG0_RESET_MASK_Msk, SPM_PWR_MGR_CFG0_RESET_MASK_Pos, Mask);
}
#endif /* endif of AC7842X AC7843X */

#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
/*!
 * @brief Enable/disable io suspend for low power mode
 * @note Function ID: DES_MCU_API_446
 * @param[in] Enable: enable/disable .
 *            - 0: disable
 *            - 1: enable
 * @return void
 */
static inline void Spm_Reg_SetIoSuspend(uint32 Enable)
{
    MODIFY_REG32(SPM->PWR_MGR_CFG0, SPM_PWR_MGR_CFG0_IOSUS_EN_Msk, SPM_PWR_MGR_CFG0_IOSUS_EN_Pos, Enable);
}
#endif /* endif of AC7842X AC7843X AC7840E */

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* AC784XX_SPM_REG_H */
