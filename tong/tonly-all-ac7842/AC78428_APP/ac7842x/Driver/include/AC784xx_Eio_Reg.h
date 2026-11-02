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
 * AutoChips Inc. (C) 2021. All rights reserved.
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
 * @file AC784xx_Eio_Reg.h
 *
 * @brief This file provides eio hardware integration functions interface.
 *
 */

#ifndef AC784XX_EIO_REG_H
#define AC784XX_EIO_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  Includes  =========================================== */
#include "Eio_Hal_Types.h"

/* ============================================  Define  ============================================ */
/* CTRL Bit Fields */
#define EIO_CTRL_FLEXEN(x)                    (((uint32)(((uint32)(x))<<EIO_CTRL_ENHANCED_EN_Pos))&EIO_CTRL_ENHANCED_EN_Msk)
#define EIO_CTRL_SWRST(x)                     (((uint32)(((uint32)(x))<<EIO_CTRL_SWRST_Pos))&EIO_CTRL_SWRST_Msk)
#define EIO_CTRL_DBGE(x)                      (((uint32)(((uint32)(x))<<EIO_CTRL_DBGE_Pos))&EIO_CTRL_DBGE_Msk)
/* PIN Bit Fields */
#define EIO_PIN_PDI(x)                        (((uint32)(((uint32)(x))<<EIO_PIN_PDI_Pos))&EIO_PIN_PDI_Msk)
/* PosSTAT Bit Fields */
#define EIO_SHIFTSTAT_SSF(x)                  (((uint32)(((uint32)(x))<<EIO_SHIFTSTAT_SSF_Pos))&EIO_SHIFTSTAT_SSF_Msk)
/* PosERR Bit Fields */
#define EIO_SHIFTERR_SEF(x)                   (((uint32)(((uint32)(x))<<EIO_SHIFTERR_SEF_Pos))&EIO_SHIFTERR_SEF_Msk)
/* TIMSTAT Bit Fields */
#define EIO_TIMSTAT_TSF(x)                    (((uint32)(((uint32)(x))<<EIO_TIMSTAT_TSF_Pos))&EIO_TIMSTAT_TSF_Msk)
/* PosSIEN Bit Fields */
#define EIO_SHIFTSIEN_SSIE(x)                 (((uint32)(((uint32)(x))<<EIO_SHIFTSIEN_SSIE_Pos))&EIO_SHIFTSIEN_SSIE_Msk)
/* PosEIEN Bit Fields */
#define EIO_SHIFTEIEN_SEIE(x)                 (((uint32)(((uint32)(x))<<EIO_SHIFTEIEN_SEIE_Pos))&EIO_SHIFTEIEN_SEIE_Msk)
/* TIMIEN Bit Fields */
#define EIO_TIMIEN_TEIE(x)                    (((uint32)(((uint32)(x))<<EIO_TIMIEN_TEIE_Pos))&EIO_TIMIEN_TEIE_Msk)
/* PosSDEN Bit Fields */
#define EIO_SHIFTSDEN_SSDE(x)                 (((uint32)(((uint32)(x))<<EIO_SHIFTSDEN_SSDE_Pos))&EIO_SHIFTSDEN_SSDE_Msk)
/* PosCTL Bit Fields */
#define EIO_SHIFTCTL_SMOD(x)                  (((uint32)(((uint32)(x))<<EIO_SHIFTCTL0_SMOD_Pos))&EIO_SHIFTCTL0_SMOD_Msk)
#define EIO_SHIFTCTL_PINPOL(x)                (((uint32)(((uint32)(x))<<EIO_SHIFTCTL0_PINPOL_Pos))&EIO_SHIFTCTL0_PINPOL_Msk)
#define EIO_SHIFTCTL_PINSEL(x)                (((uint32)(((uint32)(x))<<EIO_SHIFTCTL0_PINSEL_Pos))&EIO_SHIFTCTL0_PINSEL_Msk)
#define EIO_SHIFTCTL_PINCFG(x)                (((uint32)(((uint32)(x))<<EIO_SHIFTCTL0_PINCFG_Pos))&EIO_SHIFTCTL0_PINCFG_Msk)
#define EIO_SHIFTCTL_TIMPOL(x)                (((uint32)(((uint32)(x))<<EIO_SHIFTCTL0_TIMPOL_Pos))&EIO_SHIFTCTL0_TIMPOL_Msk)
#define EIO_SHIFTCTL_TIMSEL(x)                (((uint32)(((uint32)(x))<<EIO_SHIFTCTL0_TIMSEL_Pos))&EIO_SHIFTCTL0_TIMSEL_Msk)
/* PosCFG Bit Fields */
#define EIO_SHIFTCFG_SSTART(x)                (((uint32)(((uint32)(x))<<EIO_SHIFTCFG0_SSTART_Pos))&EIO_SHIFTCFG0_SSTART_Msk)
#define EIO_SHIFTCFG_SSTOP(x)                 (((uint32)(((uint32)(x))<<EIO_SHIFTCFG0_SSTOP_Pos))&EIO_SHIFTCFG0_SSTOP_Msk)
#define EIO_SHIFTCFG_INSRC(x)                 (((uint32)(((uint32)(x))<<EIO_SHIFTCFG0_INSRC_Pos))&EIO_SHIFTCFG0_INSRC_Msk)
/* PosBUF Bit Fields */
#define EIO_SHIFTBUF_SHIFBUF(x)               (((uint32)(((uint32)(x))<<EIO_SHIFTBUF0_SHIFTBUF_Pos))&EIO_SHIFTBUF0_SHIFTBUF_Msk)
/* PosBUFBIS Bit Fields */
#define EIO_SHIFTBUFBIS_SHIFBUFBIS(x)         (((uint32)(((uint32)(x))<<EIO_SHIFTBUFBIS0_SHIFTBUFBIS_Pos))&EIO_SHIFTBUFBIS0_SHIFTBUFBIS_Msk)
/* PosBUFBYS Bit Fields */
#define EIO_SHIFTBUFBYS_SHIFBUFBYS(x)         (((uint32)(((uint32)(x))<<EIO_SHIFTBUFBYS0_SHIFTBUFBYS_Pos))&EIO_SHIFTBUFBYS0_SHIFTBUFBYS_Msk)
/* PosBUFBBS Bit Fields */
#define EIO_SHIFTBUFBBS_SHIFBUFBBS(x)         (((uint32)(((uint32)(x))<<EIO_SHIFTBUFBBS0_SHIFTBUFBBS_Pos))&EIO_SHIFTBUFBBS0_SHIFTBUFBBS_Msk)
/* TIMCTL Bit Fields */
#define EIO_TIMCTL_TIMOD(x)                   (((uint32)(((uint32)(x))<<EIO_TIMCTL0_TIMOD_Pos))&EIO_TIMCTL0_TIMOD_Msk)
#define EIO_TIMCTL_PINPOL(x)                  (((uint32)(((uint32)(x))<<EIO_TIMCTL0_PINPOL_Pos))&EIO_TIMCTL0_PINPOL_Msk)
#define EIO_TIMCTL_PINSEL(x)                  (((uint32)(((uint32)(x))<<EIO_TIMCTL0_PINSEL_Pos))&EIO_TIMCTL0_PINSEL_Msk)
#define EIO_TIMCTL_PINCFG(x)                  (((uint32)(((uint32)(x))<<EIO_TIMCTL0_PINCFG_Pos))&EIO_TIMCTL0_PINCFG_Msk)
#define EIO_TIMCTL_TRGSRC(x)                  (((uint32)(((uint32)(x))<<EIO_TIMCTL0_TRGSRC_Pos))&EIO_TIMCTL0_TRGSRC_Msk)
#define EIO_TIMCTL_TRGPOL(x)                  (((uint32)(((uint32)(x))<<EIO_TIMCTL0_TRGPOL_Pos))&EIO_TIMCTL0_TRGPOL_Msk)
#define EIO_TIMCTL_TRGSEL(x)                  (((uint32)(((uint32)(x))<<EIO_TIMCTL0_TRGSEL_Pos))&EIO_TIMCTL0_TRGSEL_Msk)
/* TIMCFG Bit Fields */
#define EIO_TIMCFG_TSTART(x)                  (((uint32)(((uint32)(x))<<EIO_TIMCFG0_TSTART_Pos))&EIO_TIMCFG0_TSTART_Msk)
#define EIO_TIMCFG_TSTOP(x)                   (((uint32)(((uint32)(x))<<EIO_TIMCFG0_TSTOP_Pos))&EIO_TIMCFG0_TSTOP_Msk)
#define EIO_TIMCFG_TIMENA(x)                  (((uint32)(((uint32)(x))<<EIO_TIMCFG0_TIMENA_Pos))&EIO_TIMCFG0_TIMENA_Msk)
#define EIO_TIMCFG_TIMDIS(x)                  (((uint32)(((uint32)(x))<<EIO_TIMCFG0_TIMDIS_Pos))&EIO_TIMCFG0_TIMDIS_Msk)
#define EIO_TIMCFG_TIMRST(x)                  (((uint32)(((uint32)(x))<<EIO_TIMCFG0_TIMRST_Pos))&EIO_TIMCFG0_TIMRST_Msk)
#define EIO_TIMCFG_TIMDEC(x)                  (((uint32)(((uint32)(x))<<EIO_TIMCFG0_TIMDEC_Pos))&EIO_TIMCFG0_TIMDEC_Msk)
#define EIO_TIMCFG_TIMOUT(x)                  (((uint32)(((uint32)(x))<<EIO_TIMCFG0_TIMOUT_Pos))&EIO_TIMCFG0_TIMOUT_Msk)
/* TIMCMP Bit Fields */
#define EIO_TIMCMP_CMP(x)                     (((uint32)(((uint32)(x))<<EIO_TIMCMP0_CMP_Pos))&EIO_TIMCMP0_CMP_Msk)

/* ===========================================  Typedef  ============================================ */

/* ==========================================  Variables  =========================================== */

/* ====================================  Functions declaration  ===================================== */

/* =====================================  Functions definition  ===================================== */

/**
 * @brief This function enables or disables EIO functionality in debug mode.
 * @note Function ID: DES_EIO_REG_001
 * @param [in] BaseAddr: EIO base address
 * @param [in] IsEnable: enabled or disabled
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetDebugMode(EIO_Type *BaseAddr, boolean IsEnable)
{
    uint32 regValue = (uint32)BaseAddr->CTRL;
    regValue &= (uint32)(~(EIO_CTRL_DBGE_Msk));
    regValue |= EIO_CTRL_DBGE(IsEnable); /*cstat !MISRAC2012-Rule-10.5*/
    BaseAddr->CTRL = (uint32)regValue;
}

/**
 * @brief Calling this function with enable parameter set to true resets all internal master logic and registers, except the EIO Control Register.
 * @note Function ID: DES_EIO_REG_002
 * @param [in] BaseAddr: EIO base address
 * @param [in] IsEnable: enabled or disabled
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetSoftwareReset(EIO_Type *BaseAddr, boolean IsEnable)
{
    uint32 regValue = (uint32)BaseAddr->CTRL;
    regValue &= (uint32)(~(EIO_CTRL_SWRST_Msk));
    regValue |= EIO_CTRL_SWRST(IsEnable); /*cstat !MISRAC2012-Rule-10.5*/
    BaseAddr->CTRL = (uint32)regValue;
}

/**
 * @brief This function enables or disables the EIO module.
 * @note Function ID: DES_EIO_REG_003
 * @param [in] BaseAddr: EIO base address
 * @param [in] IsEnable: enabled or disabled
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetEnable(EIO_Type *BaseAddr, boolean IsEnable)
{
    uint32 regValue = (uint32)BaseAddr->CTRL;
    regValue &= (uint32)(~(EIO_CTRL_ENHANCED_EN_Msk));
    regValue |= EIO_CTRL_FLEXEN(IsEnable); /*cstat !MISRAC2012-Rule-10.5*/
    BaseAddr->CTRL = (uint32)regValue;
}

/**
 * @brief This function returns the data read from all the EIO pins.
 * @note Function ID: DES_EIO_REG_004
 * @param [in] BaseAddr: EIO base address
 * @return the data read from all the EIO pins
 */
LOCAL_INLINE uint8 Eio_Reg_GetPinData(const EIO_Type *BaseAddr)
{
    uint32 tmp = BaseAddr->PIN;
    tmp = (tmp & EIO_PIN_PDI_Msk) >> EIO_PIN_PDI_Pos;
    return (uint8)tmp;
}

/**
 * @brief This function returns the value of the status flag for the specified shifter.
 * @note Function ID: DES_EIO_REG_005
 * @param [in] BaseAddr: EIO base address
 * @param [in] Shifter: value of shifter
 * @return the value of the status flag for the specified shifter
 */
LOCAL_INLINE boolean Eio_Reg_GetShifterStatus(const EIO_Type *BaseAddr, uint8 Shifter)
{
    uint32 regValue = (uint32)BaseAddr->SHIFTSTAT;
    regValue = (regValue >> Shifter) & 1U;
    return (boolean)regValue; /*cstat !MISRAC2012-Rule-10.5*/
}

/**
 * @brief This function returns the value of the status flags for all shifters. Each bit in
 *        the returned value specifies the status for one shifter, starting with
 *        shifter 0 from least significant bit.
 *        The meaning of the status flag depends on the current mode.
 *        - Transmit mode: shifter buffer is empty and ready to accept more data
 *        - Receive mode: shifter buffer is full and received data can be read from it
 *        - Match Store mode: match occurred between shifter buffer and shifter
 *        - Match Continuous mode: current match result between shifter buffer and shifter
 *
 * @param[in] baseAddr: EIO baseAddr
 * @return the value of the status flags for all shifters
 */
LOCAL_INLINE uint32 Eio_Reg_GetAllShifterStatus(const EIO_Type *BaseAddr)
{
    return BaseAddr->SHIFTSTAT;
}

/**
 * @brief This function clears the status flag for the specified shifter. This is possible in
 *        all modes except Match Continuous mode.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] shifter: value of shifter
 * @return none
 */
LOCAL_INLINE void Eio_Reg_ClearShifterStatus(EIO_Type *BaseAddr, uint8 shifter)
{
    BaseAddr->SHIFTSTAT = 1UL << shifter;
}

/**
 * @brief This function returns the value of the error status flag for the specified shifter.
 *        The meaning of the error status flag depends on the current mode.
 *        - Transmit mode: shifter buffer was not written before it was transferred in the shifter (buffer overrun)
 *        - Receive mode: shifter buffer was not read before new data was transferred from the shifter (buffer underrun)
 *        - Match Store mode: match event occurred before the previous match data was read from shifter buffer (buffer overrun)
 *        - Match Continuous mode: match occurred between shifter buffer and shifter
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] shifter: value of shifter
 * @return the value of the error status flag for the specified shifter
 */
LOCAL_INLINE boolean Eio_Reg_GetShifterErrorStatus(const EIO_Type *BaseAddr, uint8 shifter)
{
    uint32 regValue = (uint32)BaseAddr->SHIFTERR;
    regValue = (regValue >> shifter) & 1U;
    return (boolean)regValue; /*cstat !MISRAC2012-Rule-10.5*/
}

/**
 * @brief This function returns the value of the error status flags for all shifters. Each bit in
 *        the returned value specifies the error status for one shifter, starting with
 *        shifter 0 from least significant bit.
 *        The meaning of the error status flag depends on the current mode.
 *        - Transmit mode: shifter buffer was not written before it was transferred in the shifter (buffer overrun)
 *        - Receive mode: shifter buffer was not read before new data was transferred from the shifter (buffer underrun)
 *        - Match Store mode: match event occurred before the previous match data was read from shifter buffer (buffer overrun)
 *        - Match Continuous mode: match occurred between shifter buffer and shifter
 *
 * @param[in] baseAddr: EIO baseAddr
 * @return the value of the error status flags for all shifters
 */
LOCAL_INLINE uint32 Eio_Reg_GetAllShifterErrorStatus(const EIO_Type *BaseAddr)
{
    return BaseAddr->SHIFTERR;
}

/**
 * @brief This function clears the error status flag for the specified shifter.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] shifter: value of shifter
 * @return none
 */
LOCAL_INLINE void Eio_Reg_ClearShifterErrorStatus(EIO_Type *BaseAddr, uint8 shifter)
{
    BaseAddr->SHIFTERR = 1UL << shifter;
}

/**
 * @brief This function returns the value of the status flag for the specified timer.
 *        The meaning of the status flag depends on the current mode.
 *        - 8-bit counter mode: the timer status flag is set when the upper 8-bit counter equals zero
 *        and decrements. This also causes the counter to reload with the value in the compare register.
 *        - 8-bit PWM mode: the upper 8-bit counter equals zero and decrements. This also causes the
 *        counter to reload with the value in the compare register.
 *        - 16-bit counter mode: the 16-bit counter equals zero and decrements. This also causes the
 *        counter to reload with the value in the compare register.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] timer: value of timer
 * @return the value of the status flag for the specified timer
 */
LOCAL_INLINE boolean Eio_Reg_GetTimerStatus(const EIO_Type *BaseAddr, uint8 timer)
{
    uint32 regValue = (uint32)BaseAddr->TIMSTAT;
    regValue = (regValue >> timer) & 1U;
    return (boolean)regValue; /*cstat !MISRAC2012-Rule-10.5*/
}

/**
 * @brief This function returns the value of the status flags for all timers. Each bit in
 *        the returned value specifies the status for one timer, starting with
 *        timer 0 from least significant bit.
 *        The meaning of the status flag depends on the current mode.
 *        - 8-bit counter mode: the timer status flag is set when the upper 8-bit counter equals zero
 *        and decrements. This also causes the counter to reload with the value in the compare register.
 *        - 8-bit PWM mode: the upper 8-bit counter equals zero and decrements. This also causes the
 *        counter to reload with the value in the compare register.
 *        - 16-bit counter mode: the 16-bit counter equals zero and decrements. This also causes the
 *        counter to reload with the value in the compare register.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @return the value of the status flags for all timers
 */
LOCAL_INLINE uint32 Eio_Reg_GetAllTimerStatus(const EIO_Type *BaseAddr)
{
    return BaseAddr->TIMSTAT;
}

/**
 * @brief This function clears the status flag for the specified timer.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] timer: value of timer
 * @return none
 */
LOCAL_INLINE void Eio_Reg_ClearTimerStatus(EIO_Type *BaseAddr, uint8 timer)
{
    BaseAddr->TIMSTAT = 1UL << timer;
}

/**
 * @brief Returns the state of the interrupt for all shifters. Each bit in
 *        the returned value specifies the interrupt state for one shifter, starting with
 *        shifter 0 from least significant bit.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @return the state of the interrupt for all shifters
 */
LOCAL_INLINE uint32 Eio_Reg_GetAllShifterInterrupt(const EIO_Type *BaseAddr)
{
    return BaseAddr->SHIFTSIEN;
}

/**
 * @brief Enable or disable specified shifter interrupts. The interrupt mask must contain a bit of 1 for each shifter who's interrupt must be enabled or disabled.
 * @note Function ID: DES_EIO_REG_015
 * @param [in] BaseAddr: EIO base address
 * @param [in] InterruptMask: value of interrupt mask
 * @param [in] IsEnable: enabled or disabled
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetShifterInterrupt(EIO_Type *BaseAddr, uint8 InterruptMask, boolean IsEnable)
{
    uint32 tmp;

    tmp = BaseAddr->SHIFTSIEN;

    if (IsEnable == TRUE)
    {
        tmp |= InterruptMask;
    }
    else
    {
        tmp &= ~(uint32)InterruptMask;
    }

    BaseAddr->SHIFTSIEN = tmp;
}

/**
 * @brief Returns the state of the error interrupt for all shifters. Each bit in
 *        the returned value specifies the interrupt state for one shifter, starting with
 *        shifter 0 from least significant bit.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @return the state of the error interrupt for all shifters
 */
LOCAL_INLINE uint32 Eio_Reg_GetAllShifterErrorInterrupt(const EIO_Type *BaseAddr)
{
    return BaseAddr->SHIFTEIEN;
}

/**
 * @brief Enable or disable specified shifter error interrupts. The interrupt mask must contain a
 *        bit of 1 for each shifter who's error interrupt must be enabled or disabled.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] interruptMask: value of interruptMask
 * @param[in] enable:enabled or disabled
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetShifterErrorInterrupt(EIO_Type *BaseAddr, uint8 interruptMask, boolean IsEnable)
{
    uint32 tmp;

    tmp = BaseAddr->SHIFTEIEN;

    if (IsEnable == TRUE)
    {
        tmp |= interruptMask;
    }
    else
    {
        tmp &= ~(uint32)interruptMask;
    }
    BaseAddr->SHIFTEIEN = tmp;
}

/**
 * @brief Returns the state of the interrupt for all timers. Each bit in
 *        the returned value specifies the interrupt state for one timer, starting with
 *        timer 0 from least significant bit.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @return the state of the interrupt for all timers
 */
LOCAL_INLINE uint32 Eio_Reg_GetAllTimerInterrupt(const EIO_Type *BaseAddr)
{
    return BaseAddr->TIMIEN;
}

/**
 * @brief Enable or disable specified timer interrupts. The interrupt mask must contain a
 *        bit of 1 for each timer who's interrupt must be enabled or disabled.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] interruptMask: value of interruptMask
 * @param[in] enable:enabled or disabled
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetTimerInterrupt(EIO_Type *BaseAddr, uint8 interruptMask, boolean IsEnable)
{
    uint32 tmp;

    tmp = BaseAddr->TIMIEN;

    if (IsEnable == TRUE)
    {
        tmp |= interruptMask;
    }
    else
    {
        tmp &= ~(uint32)interruptMask;
    }
    BaseAddr->TIMIEN = tmp;
}

/**
 * @brief Enable or disable specified shifter DMA requests. The request mask must contain a
 *        bit of 1 for each shifter who's DMA requests must be enabled or disabled.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] requestMask: value of requestMask
 * @param[in] enable:enabled or disabled
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetShifterDMARequest(EIO_Type *BaseAddr, uint8 requestMask, boolean IsEnable)
{
    uint32 tmp;

    tmp = BaseAddr->SHIFTSDEN;

    if (IsEnable == TRUE)
    {
        tmp |= requestMask;
    }
    else
    {
        tmp &= ~(uint32)requestMask;
    }
    BaseAddr->SHIFTSDEN = tmp;
}

/**
 * @brief This function assigns a timer to control the specified shifter, and also configures its polarity.
 * @note Function ID: DES_EIO_REG_025
 * @param [in] BaseAddr: EIO base address
 * @param [in] Shifter: Number of shifter
 * @param [in] Timer: Number of timer
 * @param [in] Polarity: Timer polarity configuration
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetShifterTimer(EIO_Type *BaseAddr,
                                         uint8 Shifter,
                                         uint8 Timer,
                                         Eio_TimerPolarityType Polarity)
{
    uint32 tmp = BaseAddr->SHIFTCTL[Shifter];
    tmp &= ~(EIO_SHIFTCTL0_TIMSEL_Msk);
    tmp |= EIO_SHIFTCTL_TIMSEL(Timer);
    tmp &= ~(EIO_SHIFTCTL0_TIMPOL_Msk);
    tmp |= EIO_SHIFTCTL_TIMPOL(Polarity);
    BaseAddr->SHIFTCTL[Shifter] = tmp;
}

/**
 * @brief This function assigns the specified pin to the specified shifter, and also sets
 *        its polarity and configuration.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] shifter: Number of shifter
 * @param[in] pin: Number of pin
 * @param[in] pinPolarity
 *            - 0: EIO_PIN_POLARITY_HIGH
 *            - 1: EIO_PIN_POLARITY_LOW
 * @param[in] Config
 *            - 0: EIO_PIN_CONFIG_DISABLED
 *            - 1: EIO_PIN_CONFIG_OPEN_DRAIN
 *            - 2: EIO_PIN_CONFIG_BIDIR_OUTPUT
 *            - 3: EIO_PIN_CONFIG_OUTPUT
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetShifterPin(EIO_Type *BaseAddr,
                                       uint8 Shifter,
                                       uint8 Pin,
                                       Eio_PinPolarityType Polarity,
                                       Eio_PinConfigType Config)
{
    uint32 tmp = BaseAddr->SHIFTCTL[Shifter];
    tmp &= ~(EIO_SHIFTCTL0_PINSEL_Msk);
    tmp |= EIO_SHIFTCTL_PINSEL(Pin);
    tmp &= ~(EIO_SHIFTCTL0_PINPOL_Msk);
    tmp |= EIO_SHIFTCTL_PINPOL(Polarity);
    tmp &= ~(EIO_SHIFTCTL0_PINCFG_Msk);
    tmp |= EIO_SHIFTCTL_PINCFG(Config);
    BaseAddr->SHIFTCTL[Shifter] = tmp;
}

/**
 * @brief This function configures the pin assigned to the specified specified shifter.
 *        It does not change the other pin-related settings.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] shifter: Number of shifter
 * @param[in] config
 *            - 0: EIO_PIN_CONFIG_DISABLED
 *            - 1: EIO_PIN_CONFIG_OPEN_DRAIN
 *            - 2: EIO_PIN_CONFIG_BIDIR_OUTPUT
 *            - 3: EIO_PIN_CONFIG_OUTPUT
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetShifterPinConfig(EIO_Type *BaseAddr,
                                             uint8 Shifter,
                                             Eio_PinConfigType Config)
{
    uint32 tmp = BaseAddr->SHIFTCTL[Shifter];
    tmp &= ~(EIO_SHIFTCTL0_PINCFG_Msk);
    tmp |= EIO_SHIFTCTL_PINCFG(Config);
    BaseAddr->SHIFTCTL[Shifter] = tmp;
}

/**
 * @brief This function configures the mode for the specified shifter.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] shifter: Number of shifter
 * @param[in] mode
 *            - 0: EIO_SHIFTER_MODE_DISABLED
 *            - 1: EIO_SHIFTER_MODE_RECEIVE
 *            - 2: EIO_SHIFTER_MODE_TRANSMIT
 *            - 4: EIO_SHIFTER_MODE_MATCH_STORE
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetShifterMode(EIO_Type *BaseAddr,
                                        uint8 Shifter,
                                        Eio_ShifterModeType Mode)
{
    uint32 tmp = BaseAddr->SHIFTCTL[Shifter];
    tmp &= ~(EIO_SHIFTCTL0_SMOD_Msk);
    tmp |= EIO_SHIFTCTL_SMOD(Mode);
    BaseAddr->SHIFTCTL[Shifter] = tmp;
}

/*!
 * @brief This gey the mode for the specified shifter.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] shifter: Number of shifter
 * @param[in] mode
 *            - 0: EIO_SHIFTER_MODE_DISABLED
 *            - 1: EIO_SHIFTER_MODE_RECEIVE
 *            - 2: EIO_SHIFTER_MODE_TRANSMIT
 *            - 4: EIO_SHIFTER_MODE_MATCH_STORE
 * @return none
 */
LOCAL_INLINE Eio_ShifterModeType Eio_Reg_GetShifterMode(const EIO_Type *BaseAddr, uint8 Shifter)
{
    uint32 tmp = BaseAddr->SHIFTCTL[Shifter];
    /*PRQA S 4342, 4394 ++ # AMCE_EIO_4342,AMCE_EIO_4394.*/
    return (Eio_ShifterModeType)EIO_SHIFTCTL_SMOD(tmp);
    /*PRQA S 4342, 4394 -- */
}

/**
 * @brief This function configures the control settings for the specified shifter: mode settings,
 *        pin settings and timer settings.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] shifter: Number of shifter
 * @param[in] mode
 *            - 0: EIO_SHIFTER_MODE_DISABLED
 *            - 1: EIO_SHIFTER_MODE_RECEIVE
 *            - 2: EIO_SHIFTER_MODE_TRANSMIT
 *            - 4: EIO_SHIFTER_MODE_MATCH_STORE
 * @param[in] pin: Number of pin
 * @param[in] pinPolarity
 *            - 0: EIO_PIN_POLARITY_HIGH
 *            - 1: EIO_PIN_POLARITY_LOW
 * @param[in] pinConfig
 *            - 0: EIO_PIN_CONFIG_DISABLED
 *            - 1: EIO_PIN_CONFIG_OPEN_DRAIN
 *            - 2: EIO_PIN_CONFIG_BIDIR_OUTPUT
 *            - 3: EIO_PIN_CONFIG_OUTPUT
 * @param[in] timer: Number of timer
 * @param[in] timerPolarity
 *            - 0: EIO_TIMER_POLARITY_POSEDGE
 *            - 1: EIO_TIMER_POLARITY_NEGEDGE
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetShifterControl(EIO_Type *BaseAddr,
                                           uint8 Shifter,
                                           Eio_ShifterModeType Mode,
                                           uint8 Pin,
                                           Eio_PinPolarityType PinPolarity,
                                           Eio_PinConfigType PinConfig,
                                           uint8 Timer,
                                           Eio_TimerPolarityType TimerPolarity)
{
    BaseAddr->SHIFTCTL[Shifter] = EIO_SHIFTCTL_SMOD(Mode)
                                | EIO_SHIFTCTL_PINPOL(PinPolarity)
                                | EIO_SHIFTCTL_PINSEL(Pin)
                                | EIO_SHIFTCTL_PINCFG(PinConfig)
                                | EIO_SHIFTCTL_TIMPOL(TimerPolarity)
                                | EIO_SHIFTCTL_TIMSEL(Timer);
}

/**
 * @brief This function configures the input source of the specified shifter.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] shifter: Number of shifter
 * @param[in] source
 *            - 0: EIO_SHIFTER_SOURCE_PIN
 *            - 1: EIO_SHIFTER_SOURCE_SHIFTER
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetShifterInputSource(EIO_Type *BaseAddr,
                                               uint8 Shifter,
                                               Eio_ShifterSourceType Source)
{
    uint32 tmp;

    tmp = BaseAddr->SHIFTCFG[Shifter];
    tmp &= ~(EIO_SHIFTCFG0_INSRC_Msk);
    tmp |= EIO_SHIFTCFG_INSRC(Source);

    BaseAddr->SHIFTCFG[Shifter] = tmp;
}

/**
 * @brief This function configures the sending or receiving of
 *        a stop bit in Transmit, Receive or Match Store modes.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] shifter: Number of shifter
 * @param[in] stop
 *            - 0: EIO_SHIFTER_STOP_BIT_DISABLED
 *            - 2: EIO_SHIFTER_STOP_BIT_0
 *            - 3: EIO_SHIFTER_STOP_BIT_1
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetShifterStopBit(EIO_Type *BaseAddr, uint8 Shifter, Eio_ShifterStopType Stop)
{
    uint32 tmp;

    tmp = BaseAddr->SHIFTCFG[Shifter];
    tmp &= ~(EIO_SHIFTCFG0_SSTOP_Msk);
    tmp |= EIO_SHIFTCFG_SSTOP(Stop);

    BaseAddr->SHIFTCFG[Shifter] = tmp;
}

/**
 * @brief This function configures the sending or receiving of
 *        a start bit in Transmit, Receive or Match Store modes.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] shifter: Number of shifter
 * @param[in] start
 *            - 0: EIO_SHIFTER_START_BIT_DISABLED
 *            - 1: EIO_SHIFTER_START_BIT_DISABLED_SH
 *            - 2: EIO_SHIFTER_START_BIT_0
 *            - 3: EIO_SHIFTER_START_BIT_1
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetShifterStartBit(EIO_Type *BaseAddr,
                                            uint8 Shifter,
                                            Eio_ShifterStartType Start)
{
    uint32 tmp;

    tmp = BaseAddr->SHIFTCFG[Shifter];
    tmp &= ~(EIO_SHIFTCFG0_SSTART_Msk);
    tmp |= EIO_SHIFTCFG_SSTART(Start);

    BaseAddr->SHIFTCFG[Shifter] = tmp;
}

/**
 * @brief This function sets the following configurations for the specified shifter:
 *        start bit, stop bit, input source
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] shifter: Number of shifter
 * @param[in] start
 *            - 0: EIO_SHIFTER_START_BIT_DISABLED
 *            - 1: EIO_SHIFTER_START_BIT_DISABLED_SH
 *            - 2: EIO_SHIFTER_START_BIT_0
 *            - 3: EIO_SHIFTER_START_BIT_1
 * @param[in] stop
 *            - 0: EIO_SHIFTER_STOP_BIT_DISABLED
 *            - 2: EIO_SHIFTER_STOP_BIT_0
 *            - 3: EIO_SHIFTER_STOP_BIT_1
 * @param[in] source
 *            - 0: EIO_SHIFTER_SOURCE_PIN
 *            - 1: EIO_SHIFTER_SOURCE_SHIFTER
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetShifterConfig(EIO_Type *BaseAddr,
                                          uint8 Shifter,
                                          Eio_ShifterStartType Start,
                                          Eio_ShifterStopType Stop,
                                          Eio_ShifterSourceType Source)
{
    BaseAddr->SHIFTCFG[Shifter] = EIO_SHIFTCFG_SSTART(Start)
                                | EIO_SHIFTCFG_SSTOP(Stop)
                                | EIO_SHIFTCFG_INSRC(Source);
}

/**
 * @brief This function reads data from the specified shifter buffer. The data can be
 *        read in any of the four ways allowed by the hardware - see description of type
 *        eio_shifter_buffer_mode_t for details.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] shifter: Number of shifter
 * @param[in] mode
 *            - 0: EIO_SHIFTER_RW_MODE_NORMAL
 *            - 1: EIO_SHIFTER_RW_MODE_BIT_SWAP
 * @return Read data value
 */
LOCAL_INLINE uint32 Eio_Reg_ReadShifterBuffer(const EIO_Type *BaseAddr,
                                               uint8 Shifter,
                                               Eio_ShifterBufferModeType Mode)
{
    uint32 data;

    switch (Mode)
    {
        case EIO_SHIFTER_RW_MODE_NORMAL:
            data = BaseAddr->SHIFTBUF[Shifter];
            break;

        case EIO_SHIFTER_RW_MODE_BIT_SWAP:
            data = BaseAddr->SHIFTBUFBIS[Shifter];
            break;

        default:
            /* bad read mode */
            data = 0U;
        break;
    }
    return data;
}

/**
 * @brief This function writes data in the specified shifter buffer. The data can be
 *        written in any of the four ways allowed by the hardware - see description of type
 *        eio_shifter_buffer_mode_t for details.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] shifter: Number of shifter
 * @param[in] value: Write data value
 * @param[in] mode
 *            - 0: EIO_SHIFTER_RW_MODE_NORMAL
 *            - 1: EIO_SHIFTER_RW_MODE_BIT_SWAP
 * @return none
 */
LOCAL_INLINE void Eio_Reg_WriteShifterBuffer(EIO_Type *BaseAddr,
                                            uint8 Shifter,
                                            uint32 Value,
                                            Eio_ShifterBufferModeType Mode)
{
    switch (Mode)
    {
        case EIO_SHIFTER_RW_MODE_NORMAL:
            BaseAddr->SHIFTBUF[Shifter] = Value;
            break;

        case EIO_SHIFTER_RW_MODE_BIT_SWAP:
            BaseAddr->SHIFTBUFBIS[Shifter] = Value;
            break;

        default:
            /* bad write mode */
            break;
    }
}

/**
 * @brief This function configures the trigger for the specified timer, and also its
 *        source (internal or external) and polarity settings.
 *        For internal triggers, the selection is as follows:
 *        - 4*N - Pin 2*N input
 *        - 4*N+1 - Shifter N status flag
 *        - 4*N+2 - Pin 2*N+1 input
 *        - 4*N+3 - Timer N trigger output
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] timer: Number of timer
 * @param[in] trigger: Number of trigger
 * @param[in] triggerPolarity
 *            - 0: EIO_TRIGGER_POLARITY_HIGH
 *            - 1: EIO_TRIGGER_POLARITY_LOW
 * @param[in] triggerSource
 *            - 0: EIO_TRIGGER_SOURCE_EXTERNAL
 *            - 1: EIO_TRIGGER_SOURCE_INTERNAL
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetTimerTrigger(EIO_Type *BaseAddr,
                                         uint8 Timer,
                                         uint8 Trigger,
                                         Eio_TriggerPolarityType Polarity,
                                         Eio_TriggerSourceType Source)
{
    uint32 tmp = BaseAddr->TIMCTL[Timer];

    tmp &= ~(EIO_TIMCTL0_TRGSEL_Msk);
    tmp |= EIO_TIMCTL_TRGSEL(Trigger);
    tmp &= ~(EIO_TIMCTL0_TRGPOL_Msk);
    tmp |= EIO_TIMCTL_TRGPOL(Polarity);
    tmp &= ~(EIO_TIMCTL0_TRGSRC_Msk);
    tmp |= EIO_TIMCTL_TRGSRC(Source);

    BaseAddr->TIMCTL[Timer] = tmp;
}

/**
 * @brief This function assigns the specified pin to the specified timer, and also sets
 *        its polarity and configuration.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] timer: Number of timer
 * @param[in] pin: Number of pin
 * @param[in] pinPolarity:
 *            - 0: EIO_PIN_POLARITY_HIGH
 *            - 1: EIO_PIN_POLARITY_LOW
 * @param[in] pinConfig:
 *            - 0: EIO_PIN_CONFIG_DISABLED
 *            - 1: EIO_PIN_CONFIG_OPEN_DRAIN
 *            - 2: EIO_PIN_CONFIG_BIDIR_OUTPUT
 *            - 3: EIO_PIN_CONFIG_OUTPUT
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetTimerPin(EIO_Type *BaseAddr,
                                     uint8 Timer,
                                     uint8 Pin,
                                     Eio_PinPolarityType Polarity,
                                     Eio_PinConfigType Config)
{
    uint32 tmp = BaseAddr->TIMCTL[Timer];

    tmp &= ~(EIO_TIMCTL0_PINSEL_Msk);
    tmp |= EIO_TIMCTL_PINSEL(Pin);
    tmp &= ~(EIO_TIMCTL0_PINPOL_Msk);
    tmp |= EIO_TIMCTL_PINPOL(Polarity);
    tmp &= ~(EIO_TIMCTL0_PINCFG_Msk);
    tmp |= EIO_TIMCTL_PINCFG(Config);

    BaseAddr->TIMCTL[Timer] = tmp;
}

/**
 * @brief This function configures the mode for the specified timer.
 *        In 8-bit counter mode, the lower 8-bits of the counter and compare register are used to
 *        configure the baud rate of the timer shift clock and the upper 8-bits are used to configure
 *        the shifter bit count.
 *        In 8-bit PWM mode, the lower 8-bits of the counter and compare
 *        register are used to configure the high period of the timer shift clock and the upper
 *        8-bits are used to configure the low period of the timer shift clock. The shifter bit count
 *        is configured using another timer or external signal.
 *        In 16-bit counter mode, the full 16-bits of the counter and compare register are used to
 *        configure either the baud rate of the shift clock or the shifter bit count.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] timer: Number of timer
 * @param[in] mode
 *            - 0: EIO_TIMER_MODE_DISABLED
 *            - 1: EIO_TIMER_MODE_8BIT_BAUD
 *            - 2: EIO_TIMER_MODE_8BIT_PWM
 *            - 3: EIO_TIMER_MODE_16BIT
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetTimerMode(EIO_Type *BaseAddr,
                                      uint8 Timer,
                                      Eio_TimerModeType Mode)
{
    uint32 tmp = BaseAddr->TIMCTL[Timer];

    tmp &= ~(EIO_TIMCTL0_TIMOD_Msk);
    tmp |= EIO_TIMCTL_TIMOD(Mode);

    BaseAddr->TIMCTL[Timer] = tmp;
}

/**
 * @brief This function configures the control settings for the specified timer: mode settings,
 *        pin settings and trigger settings.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] timer: Number of timer
 * @param[in] trigger: Number of trigger
 * @param[in] triggerPolarity
 *            - 0: EIO_TRIGGER_POLARITY_HIGH
 *            - 1: EIO_TRIGGER_POLARITY_LOW
 * @param[in] triggerSource
 *            - 0: EIO_TRIGGER_SOURCE_EXTERNAL
 *            - 1: EIO_TRIGGER_SOURCE_INTERNAL
 * @param[in] pin: Number of pin
 * @param[in] pinPolarity
 *            - 0: EIO_PIN_POLARITY_HIGH
 *            - 1: EIO_PIN_POLARITY_LOW
 * @param[in] pinConfig
 *            - 0: EIO_PIN_CONFIG_DISABLED
 *            - 1: EIO_PIN_CONFIG_OPEN_DRAIN
 *            - 2: EIO_PIN_CONFIG_BIDIR_OUTPUT
 *            - 3: EIO_PIN_CONFIG_OUTPUT
 * @param[in] mode
 *            - 0: EIO_TIMER_MODE_DISABLED
 *            - 1: EIO_TIMER_MODE_8BIT_BAUD
 *            - 2: EIO_TIMER_MODE_8BIT_PWM
 *            - 3: EIO_TIMER_MODE_16BIT
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetTimerControl(EIO_Type *BaseAddr,
                                         uint8 Timer,
                                         uint8 Trigger,
                                         Eio_TriggerPolarityType TriggerPolarity,
                                         Eio_TriggerSourceType TriggerSource,
                                         uint8 Pin,
                                         Eio_PinPolarityType PinPolarity,
                                         Eio_PinConfigType PinConfig,
                                         Eio_TimerModeType Mode)
{
    BaseAddr->TIMCTL[Timer] = EIO_TIMCTL_TIMOD(Mode)
                            | EIO_TIMCTL_PINSEL(Pin)
                            | EIO_TIMCTL_PINPOL(PinPolarity)
                            | EIO_TIMCTL_PINCFG(PinConfig)
                            | EIO_TIMCTL_TRGSRC(TriggerSource)
                            | EIO_TIMCTL_TRGPOL(TriggerPolarity)
                            | EIO_TIMCTL_TRGSEL(Trigger);
}

/**
 * @brief This function configures the initial output of the specified timer and whether it is
 *        affected by the Timer reset.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] timer: Number of timer
 * @param[in] output
 *            - 0: EIO_TIMER_INITOUT_ONE
 *            - 1: EIO_TIMER_INITOUT_ZERO
 *            - 2: EIO_TIMER_INITOUT_ONE_RESET
 *            - 3: EIO_TIMER_INITOUT_ZERO_RESET
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetTimerInitialOutput(EIO_Type *BaseAddr,
                                               uint8 Timer,
                                               Eio_TimerOutputType Output)
{
    uint32 tmp = BaseAddr->TIMCFG[Timer];

    tmp &= ~(EIO_TIMCFG0_TIMOUT_Msk);
    tmp |= EIO_TIMCFG_TIMOUT(Output);

    BaseAddr->TIMCFG[Timer] = tmp;
}

/**
 * @brief This function configures the decrement condition for the specified timer and the source
 *        of the shift clock.
 *        See description of type eio_timer_decrement_t for a list of options.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] timer: Number of timer
 * @param[in] decrement
 *            - 0: EIO_TIMER_DECREMENT_CLK_SHIFT_TMR
 *            - 1: EIO_TIMER_DECREMENT_TRG_SHIFT_TMR
 *            - 2: EIO_TIMER_DECREMENT_PIN_SHIFT_PIN
 *            - 3: EIO_TIMER_DECREMENT_TRG_SHIFT_TRG
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetTimerDecrement(EIO_Type *BaseAddr,
                                           uint8 Timer,
                                           Eio_TimerDecrementType Decrement)
{
    uint32 tmp = BaseAddr->TIMCFG[Timer];

    tmp &= ~(EIO_TIMCFG0_TIMDEC_Msk);
    tmp |= EIO_TIMCFG_TIMDEC(Decrement);

    BaseAddr->TIMCFG[Timer] = tmp;
}

/**
 * @brief This function configures the conditions that cause the timer counter (and optionally
 *        output) to be reset. In 8-bit counter mode, the timer reset will only reset the
 *        lower 8-bits that configure the baud rate. In all other modes, the timer reset
 *        will reset the full 16-bits of the counter.
 *        See description of type eio_timer_decrement_t for a list of options.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] timer: Number of timer
 * @param[in] reset
 *            - 0: EIO_TIMER_RESET_NEVER
 *            - 2: EIO_TIMER_RESET_PIN_OUT
 *            - 3: EIO_TIMER_RESET_TRG_OUT
 *            - 4: EIO_TIMER_RESET_PIN_RISING
 *            - 6: EIO_TIMER_RESET_TRG_RISING
 *            - 7: EIO_TIMER_RESET_TRG_BOTH
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetTimerReset(EIO_Type *BaseAddr,
                                       uint8 Timer,
                                       Eio_TimerResetType Reset)
{
    uint32 tmp = BaseAddr->TIMCFG[Timer];

    tmp &= ~(EIO_TIMCFG0_TIMRST_Msk);
    tmp |= EIO_TIMCFG_TIMRST(Reset);

    BaseAddr->TIMCFG[Timer] = tmp;
}

/**
 * @brief This function configures the condition that cause the specified timer to be disabled.
 *        See description of type eio_timer_disable_t for a list of options.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] timer: Number of timer
 * @param[in] disable
 *            - 0: EIO_TIMER_DISABLE_NEVER
 *            - 1: EIO_TIMER_DISABLE_TIM_DISABLE
 *            - 2: EIO_TIMER_DISABLE_TIM_CMP
 *            - 3: EIO_TIMER_DISABLE_TIM_CMP_TRG_LOW
 *            - 4: EIO_TIMER_DISABLE_PIN
 *            - 5: EIO_TIMER_DISABLE_PIN_TRG_HIGH
 *            - 6: EIO_TIMER_DISABLE_TRG
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetTimerDisable(EIO_Type *BaseAddr,
                                         uint8 Timer,
                                         Eio_TimerDisableType Isdisable)
{
    uint32 tmp = BaseAddr->TIMCFG[Timer];

    tmp &= ~(EIO_TIMCFG0_TIMDIS_Msk);
    tmp |= EIO_TIMCFG_TIMDIS(Isdisable);

    BaseAddr->TIMCFG[Timer] = tmp;
}

/**
 * @brief This function configures the condition that cause the specified timer to be enabled
 *        and start decrementing.
 *        See description of type eio_timer_disable_t for a list of options.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] timer: Number of timer
 * @param[in] enable
 *            - 0: EIO_TIMER_ENABLE_ALWAYS
 *            - 1: EIO_TIMER_ENABLE_TIM_ENABLE
 *            - 2: EIO_TIMER_ENABLE_TRG_HIGH
 *            - 3: EIO_TIMER_ENABLE_TRG_PIN_HIGH
 *            - 4: EIO_TIMER_ENABLE_PIN_POSEDGE
 *            - 5: EIO_TIMER_ENABLE_PIN_POSEDGE_TRG_HIGH
 *            - 6: EIO_TIMER_ENABLE_TRG_POSEDGE
 *            - 7: EIO_TIMER_ENABLE_TRG_EDGE
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetTimerEnable(EIO_Type *BaseAddr,
                                        uint8 Timer,
                                        Eio_TimerEnableType IsEnable)
{
    uint32 tmp = BaseAddr->TIMCFG[Timer];

    tmp &= ~(EIO_TIMCFG0_TIMENA_Msk);
    tmp |= EIO_TIMCFG_TIMENA(IsEnable);

    BaseAddr->TIMCFG[Timer] = tmp;
}

/**
 * @brief This function configures stop bit insertion for the specified timer.
 *        The stop bit can be added on a timer compare (between each word) or on a timer disable.
 *        When stop bit is enabled, configured shifters will output the contents of the stop bit
 *        when the timer is disabled. When stop bit is enabled on timer disable, the timer remains
 *        disabled until the next rising edge of the shift clock. If configured for both timer
 *        compare and timer disable, only one stop bit is inserted on timer disable.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] timer: Number of timer
 * @param[in] stop
 *            - 0: EIO_TIMER_STOP_BIT_DISABLED
 *            - 1: EIO_TIMER_STOP_BIT_TIM_CMP
 *            - 2: EIO_TIMER_STOP_BIT_TIM_DIS
 *            - 3: EIO_TIMER_STOP_BIT_TIM_CMP_DIS
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetTimerStop(EIO_Type *BaseAddr,
                                      uint8 Timer,
                                      Eio_TimerStopType Stop)
{
    uint32 tmp = BaseAddr->TIMCFG[Timer];

    tmp &= ~(EIO_TIMCFG0_TSTOP_Msk);
    tmp |= EIO_TIMCFG_TSTOP(Stop);

    BaseAddr->TIMCFG[Timer] = tmp;
}

/**
 * @brief This function configures start bit insertion for the specified timer.
 *        When start bit is enabled, configured shifters will output the contents of the start
 *        bit when the timer is enabled and the timer counter will reload from the compare
 *        register on the first rising edge of the shift clock.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] timer: Number of timer
 * @param[in] start
 *            - 0: EIO_TIMER_START_BIT_DISABLED
 *            - 1: EIO_TIMER_START_BIT_ENABLED
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetTimerStart(EIO_Type *BaseAddr,
                                       uint8 Timer,
                                       Eio_TimerStartType Start)
{
    uint32 regValue = (uint32)BaseAddr->TIMCFG[Timer];

    regValue &= (uint32)(~(EIO_TIMCFG0_TSTART_Msk));
    regValue |= EIO_TIMCFG_TSTART(Start);

    BaseAddr->TIMCFG[Timer] = (uint32)regValue;
}

/**
 * @brief This function sets the following configurations for the specified timer:
 *        start bit, stop bit, enable condition, disable condition, reset condition,
 *        decrement condition, initial output
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] timer: Number of timer
 * @param[in] start
 *            - 0: EIO_TIMER_START_BIT_DISABLED
 *            - 1: EIO_TIMER_START_BIT_ENABLED
 * @param[in] stop:
 *            - 0: EIO_TIMER_STOP_BIT_DISABLED
 *            - 1: EIO_TIMER_STOP_BIT_TIM_CMP
 *            - 2: EIO_TIMER_STOP_BIT_TIM_DIS
 *            - 3: EIO_TIMER_STOP_BIT_TIM_CMP_DIS
 * @param[in] enable
 *            - 0: EIO_TIMER_ENABLE_ALWAYS
 *            - 1: EIO_TIMER_ENABLE_TIM_ENABLE
 *            - 2: EIO_TIMER_ENABLE_TRG_HIGH
 *            - 3: EIO_TIMER_ENABLE_TRG_PIN_HIGH
 *            - 4: EIO_TIMER_ENABLE_PIN_POSEDGE
 *            - 5: EIO_TIMER_ENABLE_PIN_POSEDGE_TRG_HIGH
 *            - 6: EIO_TIMER_ENABLE_TRG_POSEDGE
 *            - 7: EIO_TIMER_ENABLE_TRG_EDGE
 * @param[in] disable
 *            - 0: EIO_TIMER_DISABLE_NEVER
 *            - 1: EIO_TIMER_DISABLE_TIM_DISABLE
 *            - 2: EIO_TIMER_DISABLE_TIM_CMP
 *            - 3: EIO_TIMER_DISABLE_TIM_CMP_TRG_LOW
 *            - 4: EIO_TIMER_DISABLE_PIN
 *            - 5: EIO_TIMER_DISABLE_PIN_TRG_HIGH
 *            - 6: EIO_TIMER_DISABLE_TRG
 * @param[in] reset
 *            - 0: EIO_TIMER_RESET_NEVER
 *            - 2: EIO_TIMER_RESET_PIN_OUT
 *            - 3: EIO_TIMER_RESET_TRG_OUT
 *            - 4: EIO_TIMER_RESET_PIN_RISING
 *            - 6: EIO_TIMER_RESET_TRG_RISING
 *            - 7: EIO_TIMER_RESET_TRG_BOTH
 * @param[in] decrement
 *            - 0: EIO_TIMER_DECREMENT_CLK_SHIFT_TMR
 *            - 1: EIO_TIMER_DECREMENT_TRG_SHIFT_TMR
 *            - 2: EIO_TIMER_DECREMENT_PIN_SHIFT_PIN
 *            - 3: EIO_TIMER_DECREMENT_TRG_SHIFT_TRG
 * @param[in] output
 *            - 0: EIO_TIMER_INITOUT_ONE
 *            - 1: EIO_TIMER_INITOUT_ZERO
 *            - 2: EIO_TIMER_INITOUT_ONE_RESET
 *            - 3: EIO_TIMER_INITOUT_ZERO_RESET
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetTimerConfig(EIO_Type *BaseAddr,
                                        uint8 Timer,
                                        Eio_TimerStartType Start,
                                        Eio_TimerStopType Stop,
                                        Eio_TimerEnableType IsEnable,
                                        Eio_TimerDisableType Isdisable,
                                        Eio_TimerResetType Reset,
                                        Eio_TimerDecrementType Decrement,
                                        Eio_TimerOutputType Output)
{

    BaseAddr->TIMCFG[Timer] = EIO_TIMCFG_TSTART(Start)
                            | EIO_TIMCFG_TSTOP(Stop)
                            | EIO_TIMCFG_TIMENA(IsEnable)
                            | EIO_TIMCFG_TIMDIS(Isdisable)
                            | EIO_TIMCFG_TIMRST(Reset)
                            | EIO_TIMCFG_TIMDEC(Decrement)
                            | EIO_TIMCFG_TIMOUT(Output);
}

/**
 * @brief This function returns the compare value currently set for the specified timer.
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] timer: Number of timer
 * @return Timer compare value
 */
LOCAL_INLINE uint16 Eio_Reg_GetTimerCompare(const EIO_Type *BaseAddr, uint8 Timer)
{
    return (uint16)(BaseAddr->TIMCMP[Timer]);
}

/**
 * @brief This function configures the compare value for the specified timer.
 *        The timer compare value is loaded into the timer counter when the timer is first enabled,
 *        when the timer is reset and when the timer decrements down to zero.
 *        In dual 8-bit counters baud/bit mode, the lower 8-bits configure the baud rate divider
 *        and the upper 8-bits configure the number of bits in each word.
 *        In dual 8-bit counters PWM mode, the lower 8-bits configure the high period of the output
 *        and the upper 8-bits configure the low period.
 *        In 16-bit counter mode, the compare value can be used to generate the baud rate divider
 *        (if shift clock source is timer output) or the number of bits in each word (when the shift
 *        clock source is a pin or trigger input).
 *
 * @param[in] baseAddr: EIO baseAddr
 * @param[in] timer: Number of timer
 * @param[in] value: Timer compare value
 * @return none
 */
LOCAL_INLINE void Eio_Reg_SetTimerCompare(EIO_Type *BaseAddr,
                                         uint8 Timer,
                                         uint16 Value)
{
    BaseAddr->TIMCMP[Timer] = (uint32)Value;
}

/**
 * @brief This function initializes all the registers of the EIO module to their reset value.
 * @note Function ID: DES_EIO_REG_099
 * @param [in] BaseAddr: EIO base address
 * @return none
 */
LOCAL_INLINE void Eio_Reg_Init(EIO_Type *BaseAddr)
{
    /* Use software reset bit to reset the module */
    Eio_Reg_SetSoftwareReset(BaseAddr, TRUE); /*cstat !MISRAC2012-Rule-10.3*/
    /* Control register is not affected by software reset */
    BaseAddr->CTRL = 0x0U;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AC784XX_EIO_REG_H */

/* =============================================  EOF  ============================================== */
