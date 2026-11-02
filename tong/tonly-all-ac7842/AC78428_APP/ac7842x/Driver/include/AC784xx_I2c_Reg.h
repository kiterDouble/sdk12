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
 * @file AC784xx_I2C_Reg.h
 * @brief This file provides Reg i2c api.
 */

#ifndef AC784XX_I2C_REG_H
#define  AC784XX_I2C_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "I2c_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/*!
* @brief I2C hardware timeout count macro.
* @note DES_I2C_MACRO_201
*/
#define I2C_HW_DEADLINE_TIMEOUT (0x80000UL)

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
* @brief Enable/disable I2C master synchronization.
* @note Function ID:DES_I2C_API_239
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[in] IsEnable: enabling state
*            - true to enable
*            - false to disable
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SetSYNC(I2C_Type *const Base, boolean IsEnable)
{
    MODIFY_REG32(Base->CTRL1, I2C_CTRL1_SYNCEN_Msk, I2C_CTRL1_SYNCEN_Pos,
                 ((TRUE == IsEnable) ? 1U : 0U));
}

/**
* @brief Enable/disable I2C master arbitration.
* @note Function ID:DES_I2C_API_240
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[in] IsEnable: enabling state
*            - true to enable
*            - false to disable
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SetARB(I2C_Type *const Base, boolean IsEnable)
{
    MODIFY_REG32(Base->CTRL1, I2C_CTRL1_ARBEN_Msk, I2C_CTRL1_ARBEN_Pos,
                 ((TRUE == IsEnable) ? 1U : 0U));
}

/**
* @brief Set i2c master/slave.
* @note Function ID:DES_I2C_API_241
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[in] Mode: i2c mode
*            - I2C_SLAVE
*            - I2C_MASTER
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SetMSTR(I2C_Type *const Base, I2c_ModeTypes Mode)
{
    MODIFY_REG32(Base->CTRL0, I2C_CTRL0_MSTR_Msk, I2C_CTRL0_MSTR_Pos, Mode);
}

/**
* @brief Set i2c module IsEnable.
* @note Function ID:DES_I2C_API_242
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[in] IsEnable: enabling state
*            - true to enable
*            - false to disable
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SetModuleEnable(I2C_Type *const Base, boolean IsEnable)
{
    MODIFY_REG32(Base->CTRL0, I2C_CTRL0_IICEN_Msk, I2C_CTRL0_IICEN_Pos,
                 ((TRUE == IsEnable) ? 1U : 0U));
}

/**
* @brief Set slave address.
* @note Function ID:DES_I2C_API_243
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[in] SlaveAddr: slave 7~10bit address value
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SetSlaveAddr(I2C_Type *const Base, uint16 SlaveAddr)
{
    MODIFY_REG32(Base->ADDR0, I2C_ADDR0_AD_Msk, I2C_ADDR0_AD_Pos, (uint32)SlaveAddr & 0x7FU);
    MODIFY_REG32(Base->ADDR1, I2C_ADDR1_AD_Msk, I2C_ADDR1_AD_Pos, (uint32)SlaveAddr >> 7U);
}

/*!
 * @brief Set slave range address.
 *
 * @param[in] base: I2C base pointer
 * @param[in] rangeAddr: slave range address value
 * @return None
 */
LOCAL_INLINE void I2C_Reg_SetSlaveRangeAddr(I2C_Type *const base, uint8 rangeAddr)
{
    MODIFY_REG32(base->ADDR1, I2C_ADDR1_RAD_Msk, I2C_ADDR1_RAD_Pos, rangeAddr);
}

/*!
 * @brief Enable/disable slave range address.
 *
 * @param[in] base: I2C base pointer
 * @param[in] enable: enabling state
 *            - true to enable
 *            - false to disable
 * @return None
 */
LOCAL_INLINE void I2C_Reg_SetRAD(I2C_Type *const base, boolean enable)
{
    MODIFY_REG32(base->ADDR1, I2C_ADDR1_RMEN_Msk, I2C_ADDR1_RMEN_Pos, enable);
}

/*!
 * @brief Enable/disable slave monitor function.
 *
 * @param[in] base: I2C base pointer
 * @param[in] enable: enabling state
 *            - true to enable
 *            - false to disable
 * @return None
 */
LOCAL_INLINE void I2C_Reg_SetMNT(I2C_Type *const base, boolean enable)
{
    MODIFY_REG32(base->CTRL2, I2C_CTRL2_MNTEN_Msk, I2C_CTRL2_MNTEN_Pos, enable);
}

/*!
 * @brief Enable/disable slave general call.
 *
 * @param[in] base: I2C base pointer
 * @param[in] enable: enabling state
 *            - true to enable
 *            - false to disable
 * @return None
 */
LOCAL_INLINE void I2C_Reg_SetGCA(I2C_Type *const base, boolean enable)
{
    MODIFY_REG32(base->CTRL1, I2C_CTRL1_GCAEN_Msk, I2C_CTRL1_GCAEN_Pos, enable);
}

/*!
 * @brief Get the slave extend address status.
 *
 * @param[in] base: I2C base pointer
 * @return slave extend address enabling bit
 */
LOCAL_INLINE uint32 I2C_Reg_IsADEXT(const I2C_Type *const base)
{
    return (base->CTRL1 & I2C_CTRL1_ADEXT_Msk);
}

/*!
 * @brief Get the BND status.
 *
 * @param[in] base: I2C base pointer
 * @return BND flag
 */
LOCAL_INLINE uint32 I2C_Reg_IsBND(const I2C_Type *const base)
{
    return (base->STATUS0 & I2C_STATUS0_BND_Msk);
}

/*!
 * @brief Get the NACK status.
 *
 * @param[in] base: I2C base pointer
 * @return NACK flag
 */
LOCAL_INLINE uint32 I2C_Reg_IsNack(const I2C_Type *const base)
{
    return (base->STATUS0 & I2C_STATUS0_RACK_Msk);
}

/*!
 * @brief Get the BUS busy status.
 *
 * @param[in] base: I2C base pointer
 * @return busy flag
 */
LOCAL_INLINE uint32 I2C_Reg_IsBusy(const I2C_Type *const base)
{
    return (base->STATUS0 & I2C_STATUS0_BUSY_Msk);
}

/*!
 * @brief Get the core ready status.
 *
 * @param[in] base: I2C base pointer
 * @return ready flag
 */
LOCAL_INLINE uint32 I2C_Reg_IsReady(const I2C_Type *const base)
{
    return (base->STATUS0 & I2C_STATUS0_READY_Msk);
}

/*!
 * @brief Get the device role.
 *
 * @param[in] base: I2C base pointer
 * @return wheather is master or not
 */
LOCAL_INLINE uint32 I2C_Reg_IsMaster(const I2C_Type *const base)
{
    return (base->CTRL0 & I2C_CTRL0_MSTR_Msk);
}

/**
* @brief Enable/disable slave address extention.
* @param[in] base: I2C base pointer
* @param[in] IsEnable: enabling state
*            - true to enable
*            - false to disable
* @return void
*/
LOCAL_INLINE void I2C_Reg_SetADEXT(I2C_Type *const Base, boolean IsEnable)
{
    MODIFY_REG32(Base->CTRL1, I2C_CTRL1_ADEXT_Msk, I2C_CTRL1_ADEXT_Pos,
                 ((TRUE == IsEnable) ? 1U : 0U));
}

#if !defined(AC7840E)
/**
* @brief Enable/disable I2C wakeup.
* @note Function ID:DES_I2C_API_244
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[in] IsEnable: enabling state
*            - true to enable
*            - false to disable
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SetWakeup(I2C_Type *const Base, boolean IsEnable)
{
    MODIFY_REG32(Base->CTRL0, I2C_CTRL0_WUEN_Msk, I2C_CTRL0_WUEN_Pos,
                 ((TRUE == IsEnable) ? 1U : 0U));
}
#endif

/**
* @brief Enable/dosable I2C slave stretch.
* @note Function ID:DES_I2C_API_245
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[in] IsEnable: enabling state
*            - true to enable
*            - false to disable
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SetStretch(I2C_Type *const Base, boolean IsEnable)
{
    MODIFY_REG32(Base->CTRL1, I2C_CTRL1_STREN_Msk, I2C_CTRL1_STREN_Pos,
                 ((TRUE == IsEnable) ? 1U : 0U));
}

/**
* @brief Set i2c SCL sample step cnt.
* @note Function ID:DES_I2C_API_247
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[in] SampleCnt: sample cnt
* @param[in] StepCnt: step cnt
*            - sample_width = (SampleCnt + 1) * APB period
*            - half_pulse_width = (StepCnt + 1) * sample_width
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SetSampleStep(I2C_Type *const Base, uint32 SampleCnt, uint32 StepCnt)
{
    /* Write the best StepCnt and SampleCnt to register */
    MODIFY_REG32(Base->SAMPLE_CNT, I2C_SAMPLE_CNT_DIV_Msk, I2C_SAMPLE_CNT_DIV_Pos,
                 ((SampleCnt > StepCnt) ? SampleCnt : StepCnt));
    MODIFY_REG32(Base->STEP_CNT, I2C_STEP_CNT_DIV_Msk, I2C_STEP_CNT_DIV_Pos,
                 ((SampleCnt > StepCnt) ? StepCnt : SampleCnt));
}

/**
* @brief Set i2c interrupt.
* @note Function ID:DES_I2C_API_248
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[in] IsEnable: enabling state
*            - true to enable
*            - false to disable
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SetInterrupt(I2C_Type *const Base, boolean IsEnable)
{
    MODIFY_REG32(Base->CTRL0, I2C_CTRL0_IICIE_Msk, I2C_CTRL0_IICIE_Pos,
                 ((TRUE == IsEnable) ? 1U : 0U));
}

/**
* @brief Clear the status1 mask bit.
* @note Function ID:DES_I2C_API_249
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[in] ClearMsk: the mask bit to clear
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_ClearStatus0(I2C_Type *const Base, uint32 ClearMsk)
{
    Base->STATUS0 = ClearMsk;
}

/**
* @brief Clear the status1 mask bit.
* @note Function ID:DES_I2C_API_250
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[in] IsEnable: enabling state
*            - true to enable
*            - false to disable
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SetNackInterrupt(I2C_Type *const Base, boolean IsEnable)
{
    MODIFY_REG32(Base->CTRL2, I2C_CTRL2_NACKIE_Msk, I2C_CTRL2_NACKIE_Pos,
                 ((TRUE == IsEnable) ? 1U : 0U));
}

/**
* @brief Enable/disable dma tx.
* @note Function ID:DES_I2C_API_251
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[in] IsEnable: enabling state
*            - true to enable
*            - false to disable
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SetDMATx(I2C_Type *const Base, boolean IsEnable)
{
    MODIFY_REG32(Base->CTRL3, I2C_CTRL3_DMATXEN_Msk, I2C_CTRL3_DMATXEN_Pos,
                 ((TRUE == IsEnable) ? 1U : 0U));
}

/**
* @brief Set transfer tx direction.
* @note Function ID:DES_I2C_API_254
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_TxEn(I2C_Type *const Base)
{
    Base->CTRL0 |= I2C_CTRL0_TX_Msk;
}

/**
* @brief Write Data register.
* @note Function ID:DES_I2C_API_255
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[in] Data: the Data to write
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_WriteDataReg(I2C_Type *const Base, uint8 Data)
{
    Base->DATA = Data;
}

/**
* @brief Enable/disable dma rx.
* @note Function ID:DES_I2C_API_256
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[in] IsEnable: enabling state
*            - true to enable
*            - false to disable
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SetDMARx(I2C_Type *const Base, boolean IsEnable)
{
    MODIFY_REG32(Base->CTRL3, I2C_CTRL3_DMARXEN_Msk, I2C_CTRL3_DMARXEN_Pos,
                 ((TRUE == IsEnable) ? 1U : 0U));
}

/**
* @brief Set transfer rx direction.
* @note Function ID:DES_I2C_API_259
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_RxEn(I2C_Type *const Base)
{
    Base->CTRL0 &= ~I2C_CTRL0_TX_Msk;
}

/**
* @brief Read data register.
* @note Function ID:DES_I2C_API_260
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[out] None
* @return uint8: Read data
*/
LOCAL_INLINE uint8 I2c_Reg_ReadDataReg(const I2C_Type *Base)
{
    return (uint8)(Base->DATA);
}

/**
* @brief Enable transfer stop.
* @note Function ID:DES_I2C_API_261
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SendStop(I2C_Type *const Base)
{
    Base->STARTSTOP |= I2C_STARTSTOP_STOP_Msk;
}

/**
* @brief Set start stop interrupt.
* @note Function ID:DES_I2C_API_262
* @note Service ID: NA
* @param[in] Base: I2C base pointer
* @param[in] Enable: enabling state
*          - TRUE to enable
*          - FALSE to disable
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SetSSInterrupt(I2C_Type *Base, boolean IsEnable)
{
    uint32 Tmp;

    Tmp = (Base->DGLCFG & (~(I2C_DGLCFG_STARTF_Msk | I2C_DGLCFG_STOPF_Msk)));
    Tmp &= ~I2C_DGLCFG_SSIE_Msk;
    WRITE_REG32(Base->DGLCFG, ((TRUE == IsEnable) ? (Tmp | I2C_DGLCFG_SSIE_Msk) : Tmp));
}

/**
* @brief Get status0 register.
* @note Function ID:DES_I2C_API_263
* @note Service ID: NA
* @param[in] Base: I2C base pointer
* @param[out] None
* @return uint32: status0 register
*/
LOCAL_INLINE uint32 I2c_Reg_GetStatus0(const I2C_Type *const Base)
{
    return (Base->STATUS0);
}

/**
* @brief Get the start flag.
* @note Function ID:DES_I2C_API_264
* @note Service ID: NA
* @param[in] Base: I2C base pointer
* @param[out] None
* @return uint32: the start flag
*/
LOCAL_INLINE uint32 I2c_Reg_IsStart(const I2C_Type *const Base)
{
    return (Base->DGLCFG & I2C_DGLCFG_STARTF_Msk);
}

/**
* @brief Get the start/stop int enable.
* @note Function ID:DES_I2C_API_265
* @note Service ID: NA
* @param[in] Base: I2C base pointer
* @param[out] None
* @return uint32: the start/stop int enable flag
*/
LOCAL_INLINE uint32 I2c_Reg_IsSSIntEnable(const I2C_Type *const Base)
{
    return (Base->DGLCFG & I2C_DGLCFG_SSIE_Msk);
}

/**
* @brief Get the stop flag.
* @note Function ID:DES_I2C_API_266
* @note Service ID: NA
* @param[in] Base: I2C base pointer
* @param[out] None
* @return uint32: the stop flag
*/
LOCAL_INLINE uint32 I2c_Reg_IsStop(const I2C_Type *const Base)
{
    return (Base->DGLCFG & I2C_DGLCFG_STOPF_Msk);
}

/**
* @brief Clear the start flag bit.
* @note Function ID:DES_I2C_API_267
* @note Service ID: NA
* @param[in] Base: I2C base pointer
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_ClearStartFlag(I2C_Type *const Base)
{
    uint32 Tmp = Base->DGLCFG & (~I2C_DGLCFG_STOPF_Msk);

    Tmp |= I2C_DGLCFG_STARTF_Msk;
    Base->DGLCFG = Tmp;
}

/**
* @brief Clear the stop flag bit.
* @note Function ID:DES_I2C_API_268
* @note Service ID: NA
* @param[in] Base: I2C base pointer
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_ClearStopFlag(I2C_Type *const Base)
{
    uint32 Tmp = Base->DGLCFG & (~I2C_DGLCFG_STARTF_Msk);

    Tmp |= I2C_DGLCFG_STOPF_Msk;
    Base->DGLCFG = Tmp;
}

/**
* @brief Get whether the Tx is DMA or not.
* @note Function ID:DES_I2C_API_269
* @note Service ID: NA
* @param[in] Base: I2C base pointer
* @param[out] None
* @return uint32: DMATX status
*/
LOCAL_INLINE uint32 I2c_Reg_IsDMATxEnable(const I2C_Type *const Base)
{
    return (Base->CTRL3 & I2C_CTRL3_DMATXEN_Msk);
}

/*!
 * @brief Get whether the Rx is DMA or not.
 *
 * @param[in] base: I2C base pointer
 * @return DMARX status
 */
LOCAL_INLINE uint32 I2C_Reg_IsDMARxEnable(const I2C_Type *const base)
{
    return (base->CTRL3 & I2C_CTRL3_DMARXEN_Msk);
}

/*!
 * @brief Enable I2C debug mode or not.
 *
 * @param[in] base: I2C base pointer
 * @param[in] enable: enabling state
 *            - true to enable
 *            - false to disable
 * @return None
 */
LOCAL_INLINE void I2C_Reg_SetDebug(I2C_Type *const base, boolean enable)
{
    MODIFY_REG32(base->CTRL0, I2C_CTRL0_DBGEN_Msk, I2C_CTRL0_DBGEN_Pos, enable);
}

/*!
 * @brief Enable I2C software reset of master and slave or not.
 *
 * @param[in] base: I2C base pointer
 * @param[in] enable: enabling state
 *            - true to enable
 *            - false to disable
 * @return None
 */
LOCAL_INLINE void I2C_Reg_SetSoftwareReset(I2C_Type *const base, boolean enable)
{
    MODIFY_REG32(base->CTRL0, I2C_CTRL0_SRST_Msk, I2C_CTRL0_SRST_Pos, enable);
}

/*!
 * @brief Enable SMBus Alert or not.
 *
 * @param[in] base: I2C base pointer
 * @param[in] enable: enabling state
 *            - true to enable
 *            - false to disable
 * @return None
 */
LOCAL_INLINE void I2C_Reg_SetSMBusAlert(I2C_Type *const base, boolean enable)
{
    MODIFY_REG32(base->CTRL1, I2C_CTRL1_SAEN_Msk, I2C_CTRL1_SAEN_Pos, enable);
}

/*!
 * @brief Set BND_DMA interrupt.
 *
 * @param[in] base: I2C base pointer
 * @param[in] enable: enabling state
 *            - true to enable
 *            - false to disable
 * @return None
 */
LOCAL_INLINE void I2C_Reg_SetBNDDMAInterrupt(I2C_Type *const base, boolean enable)
{
    MODIFY_REG32(base->CTRL2, I2C_CTRL2_BND_DMA_IE_Msk, I2C_CTRL2_BND_DMA_IE_Pos, enable);
}

/*!
 * @brief Get BND_DMA interrupt status.
 *
 * @param[in] base: I2C base pointer
 * @param[in] enable: enabling state
 *            - true to enable
 *            - false to disable
 * @return BND_DMA interrupt enable status
 */
LOCAL_INLINE uint32 I2C_Reg_IsBNDDMAInterrupt(const I2C_Type *const base)
{
    return (base->CTRL2 & I2C_CTRL2_BND_DMA_IE_Msk);
}

/*!
 * @brief Enable/disable slave tx buff empty interrupt.
 *
 * @param[in] base: I2C base pointer
 * @param[in] enable: enabling state
 *            - true to enable
 *            - false to disable
 * @return None
 */
LOCAL_INLINE void I2C_Reg_SetTxEInterrupt(I2C_Type *const base, boolean enable)
{
    MODIFY_REG32(base->CTRL2, I2C_CTRL2_TXEIE_Msk, I2C_CTRL2_TXEIE_Pos, enable);
}

/*!
 * @brief Enable/disable slave rx buff full interrupt.
 *
 * @param[in] base: I2C base pointer
 * @param[in] enable: enabling state
 *            - true to enable
 *            - false to disable
 * @return None
 */
LOCAL_INLINE void I2C_Reg_SetRxFInterrupt(I2C_Type *const base, boolean enable)
{
    MODIFY_REG32(base->CTRL2, I2C_CTRL2_RXFIE_Msk, I2C_CTRL2_RXFIE_Pos, enable);
}

/*!
 * @brief Enable/disable slave tx under flow interrupt.
 *
 * @param[in] base: I2C base pointer
 * @param[in] enable: enabling state
 *            - true to enable
 *            - false to disable
 * @return None
 */
LOCAL_INLINE void I2C_Reg_SetTxUFInterrupt(I2C_Type *const base, boolean enable)
{
    MODIFY_REG32(base->CTRL2, I2C_CTRL2_TXUFIE_Msk, I2C_CTRL2_TXUFIE_Pos, enable);
}

/*!
 * @brief Enable/disable slave rx over flow interrupt.
 *
 * @param[in] base: I2C base pointer
 * @param[in] enable: enabling state
 *            - true to enable
 *            - false to disable
 * @return None
 */
LOCAL_INLINE void I2C_Reg_SetRxOFInterrupt(I2C_Type *const base, boolean enable)
{
    MODIFY_REG32(base->CTRL2, I2C_CTRL2_RXOFIE_Msk, I2C_CTRL2_RXOFIE_Pos, enable);
}

/**
* @brief Get the transfer direction.
* @note Function ID:DES_I2C_API_270
* @note Service ID: NA
* @param[in] Base: I2C base pointer
* @param[out] None
* @return uint32: transfer direction flag
*/
LOCAL_INLINE uint32 I2c_Reg_IsTx(const I2C_Type *const Base)
{
    return (Base->CTRL0 & I2C_CTRL0_TX_Msk);
}

/**
* @brief Set transfer nack.
* @note Function ID:DES_I2C_API_271
* @note Service ID: NA
* @param[in] Base: I2C base pointer
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SendNack(I2C_Type *const Base)
{
    Base->CTRL0 |= I2C_CTRL0_TACK_Msk;
}

/**
* @brief Set transfer ack.
* @note Function ID:DES_I2C_API_272
* @note Service ID: NA
* @param[in] Base: I2C base pointer
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SendAck(I2C_Type *const Base)
{
    Base->CTRL0 &= ~I2C_CTRL0_TACK_Msk;
}

/**
* @brief Read one byte without send next clock for master.
* @note Function ID:DES_I2C_API_273
* @note Service ID: NA
* @param[in] Base: The I2C base pointer
* @param[in] Data: last one byte data to be read
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_ReceiveLastOneByte
(
    I2C_Type *Base,
    uint8 *Data
)
{
    /* change direction to avoid send out next byte clock */
    /* Enable TX */
    MODIFY_REG32(Base->CTRL0, I2C_CTRL0_TX_Msk, I2C_CTRL0_TX_Pos, I2C_ENABLE_TX);
    *Data = ((uint8)Base->DATA);
    /* when direction is Tx, read data won't clear BND flag instead of write clear */
    /* Clear I2C BND flag */
    MODIFY_REG32(Base->STATUS0, I2C_STATUS0_BND_Msk, I2C_STATUS0_BND_Pos, 1U);
}

/**
* @brief Get slave tx under flow bit.
* @note Function ID:DES_I2C_API_274
* @note Service ID: NA
* @param[in] Base: The I2C base pointer
* @param[out] None
* @return uint32: tx under flow bit
*/
LOCAL_INLINE uint32 I2c_Reg_IsTxUF(const I2C_Type *const Base)
{
    return (Base->STATUS1 & I2C_STATUS1_TXUF_Msk);
}

/**
* @brief Clear the status2 mask bit.
* @note Function ID:DES_I2C_API_275
* @note Service ID: NA
* @param[in] Base: The I2C base pointer
* @param[in] ClearMsk: the mask bit to clear
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_ClearStatus1(I2C_Type *const Base, uint32 ClearMsk)
{
    Base->STATUS1 = ClearMsk;
}

/**
* @brief Get slave rx over flow bit.
* @note Function ID:DES_I2C_API_276
* @note Service ID: NA
* @param[in] Base: The I2C base pointer
* @param[out] None
* @return uint32: rx over flow bit
*/
LOCAL_INLINE uint32 I2c_Reg_IsRxOF(const I2C_Type *const Base)
{
    return (Base->STATUS1 & I2C_STATUS1_RXOF_Msk);
}

/**
* @brief Enable transfer start.
* @note Function ID:DES_I2C_API_277
* @note Service ID: NA
* @param[in] Base: The I2C base pointer
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SendStart(I2C_Type *const Base)
{
    Base->STARTSTOP |= I2C_STARTSTOP_START_Msk;
}

/*!
 * @brief Get i2c SCL step cnt.
 *
 * @param[in] base: I2C base pointer
 * @return Step cnt
 */
LOCAL_INLINE uint8 I2C_Reg_GetStepCnt(const I2C_Type *const base)
{
    uint32 tmp = base->STEP_CNT;

    tmp = (tmp & I2C_STEP_CNT_DIV_Msk) >> I2C_STEP_CNT_DIV_Pos;

    return (uint8)tmp;
}

/*!
 * @brief Get i2c SCL sample cnt.
 *
 * @param[in] base: I2C base pointer
 * @return Sample cnt
 */
LOCAL_INLINE uint8 I2C_Reg_GetSampleCnt(const I2C_Type *const base)
{
    uint32 tmp = base->SAMPLE_CNT;

    tmp = (tmp & I2C_SAMPLE_CNT_DIV_Msk) >> I2C_SAMPLE_CNT_DIV_Pos;

    return (uint8)tmp;
}

/**
* @brief Set Pltie enable.
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_SetPltie(I2C_Type *const Base, boolean IsEnable, uint16 Value, I2c_Hal_TimeCfgTypes Types)
{
    MODIFY_REG32(Base->CTRL3, I2C_CTRL3_PINLOW_Msk, I2C_CTRL3_PINLOW_Pos,
                 ((TRUE == IsEnable) ? Value : 0U));
    MODIFY_REG32(Base->CTRL2, I2C_CTRL2_PLTIE_Msk, I2C_CTRL2_PLTIE_Pos,
                 ((TRUE == IsEnable) ? 1U : 0U));
    MODIFY_REG32(Base->CTRL3, I2C_CTRL3_TIMECFG_Msk, I2C_CTRL3_TIMECFG_Pos, Types);
}

/**
* @brief Get the Pltie flag.
* @note Service ID: NA
* @param[in] Base: I2C base pointer
* @param[out] None
* @return uint32: the Pltie flag
*/
LOCAL_INLINE uint32 I2c_Reg_IsPltie(const I2C_Type *const Base)
{
    return (Base->STATUS1 & I2C_STATUS1_PLTF_Msk);
}

/**
* @brief Clear the Pltie flag bit.
* @note Service ID: NA
* @param[in] Base: I2C base pointer
* @param[out] None
* @return void
*/
LOCAL_INLINE void I2c_Reg_ClearPltieFlag(I2C_Type *const Base)
{
    SET_BIT32(Base->STATUS1, I2C_STATUS1_PLTF_Msk);
}

/**
* @brief Write one byte with polling BND flag,
*        make sure to disable interrupt when use the interface.
* @note Function ID:DES_I2C_API_258
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @param[in] Data: the value to DATA_Reg
* @param[out] None
* @return Hal_StatusType: i2c hardware status
*/
LOCAL_INLINE void I2c_Reg_TransmitOneByte
(
    I2C_Type *Base,
    uint8 Data
)
{
    /* I2c Tx enable */
    MODIFY_REG32(Base->CTRL0, I2C_CTRL0_TX_Msk, I2C_CTRL0_TX_Pos, 1U);
    MODIFY_REG32(Base->DATA, I2C_DATA_DATA_Msk, I2C_DATA_DATA_Pos, Data);
}

/*!
 * @brief Set deglitch DGL cnt.
 *
 * @param[in] base: I2C base pointer
 * @param[in] DGLCnt: deglitch cnt
 * @return None
 */
LOCAL_INLINE void I2C_Reg_SetDGLCnt(I2C_Type *const base, uint8 DGLCnt)
{
    uint32 tmp = base->DGLCFG & (~(I2C_DGLCFG_STARTF_Msk | I2C_DGLCFG_STOPF_Msk));

    tmp &= ~I2C_DGLCFG_DGL_CNT_Msk;
    tmp |= ((uint32)DGLCnt << I2C_DGLCFG_DGL_CNT_Pos) & I2C_DGLCFG_DGL_CNT_Msk;
    base->DGLCFG = tmp;
}

/*!
 * @brief Enable/disable deglitch DGL function.
 *
 * @param[in] base: I2C base pointer
 * @param[in] enable: enabling state
 *            - true to enable
 *            - false to disable
 * @return None
 */
LOCAL_INLINE void I2C_Reg_SetDGL(I2C_Type *const base, boolean enable)
{
    uint32 tmp = base->DGLCFG & (~(I2C_DGLCFG_STARTF_Msk | I2C_DGLCFG_STOPF_Msk));

    tmp &= ~I2C_DGLCFG_DGLEN_Msk;
    if (TRUE == enable)
    {
        tmp |= I2C_DGLCFG_DGLEN_Msk;
    }
    base->DGLCFG = tmp;
}

/*!
 * @brief Get SMBus Alert Response flag.
 *
 * @param[in] base: I2C base pointer
 * @return the SMBus Alert Response flag
 */
LOCAL_INLINE uint32 I2C_Reg_IsSMBusAlertResponse(const I2C_Type *const base)
{
    return (base->STATUS1 & I2C_STATUS1_SARF_Msk);
}

#ifdef __cplusplus
}
#endif

#endif /* AC784XX_I2C_REG.h */
/* =============================================  EOF  ============================================== */
