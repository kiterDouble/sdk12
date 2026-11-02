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

/**
* @file AC784xx_Spi_Reg.h
*
* @brief Spi driver register header file.
*
*/

#ifndef AC784XX_SPI_REG_H
#define AC784XX_SPI_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*==============================================INCLUDE FILES=======================================*/
#include "Spi_Hal_Types.h"

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/
/**
* @brief SPI Module softreset. And it only reset master engine/buffer/flag logic,
*        slave buffer/flag logic, the configuration of control registers(CFG0/
*        CFG1/CFG2/CMD) will not be reset.
* @note Function ID: DES_SPI_API_300
* @param[in] Base: SPI base pointer.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SoftwareReset(SPI_Type *Base)
{
    SET_BIT32(Base->CMD, SPI_CMD_SWRST_Msk);
    CLEAR_BIT32(Base->CMD, SPI_CMD_SWRST_Msk);
}

/**
* @brief Set CS_SETUP time.
* @note Function ID: DES_SPI_API_301
* @param[in] Base: SPI base pointer.
* @param[in] Delay: The 8-bit delay value 0x00 to 0xFF (255).
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetCSSetup(SPI_Type *Base, uint8 Delay)
{
    MODIFY_REG32(Base->CFG0, SPI_CFG0_CS_SETUP_Msk, SPI_CFG0_CS_SETUP_Pos, Delay);
}

/**
* @brief Set CS_HOLD time.
* @note Function ID: DES_SPI_API_302
* @param[in] Base: SPI base pointer.
* @param[in] Delay: The 8-bit delay value 0x00 to 0xFF (255).
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetCSHold(SPI_Type *Base, uint8 Delay)
{
    MODIFY_REG32(Base->CFG0, SPI_CFG0_CS_HOLD_Msk, SPI_CFG0_CS_HOLD_Pos, Delay);
}

/**
* @brief Set CS_IDLE time.
* @note Function ID: DES_SPI_API_303
* @param[in] Base: SPI base pointer.
* @param[in] Delay: The 8-bit delay value 0x00 to 0xFF (255).
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetCSIdle(SPI_Type *Base, uint8 Delay)
{
    MODIFY_REG32(Base->CFG1, SPI_CFG1_CS_IDLE_Msk, SPI_CFG1_CS_IDLE_Pos, Delay);
}

/**
* @brief Configures the SPI PCS polarity.
* @note Function ID: DES_SPI_API_304
* @param[in] Base: SPI base pointer.
* @param[in] PcsCfg: Select which PCS to configured.
*                    - SPI_PCS_0
*                    - SPI_PCS_1
*                    - SPI_PCS_2
*                    - SPI_PCS_3
* @param[in] PcsPolarity: Set PCS as active high or low
*                         - SPI_ACTIVE_LOW
*                         - SPI_ACTIVE_HIGH
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetPcsPolarity(SPI_Type *Base, Spi_PcsType PcsCfg, Spi_PcsPolarityType PcsPolarity)
{
    MODIFY_REG32(Base->CFG2, ((uint32)1U << (SPI_CFG2_PCS0POL_Pos + (uint32)PcsCfg)),
                 (SPI_CFG2_PCS0POL_Pos + (uint32)PcsCfg), PcsPolarity);
}

/**
* @brief Set SPI CS baudrate, 1/(low_time + high_time).
* @note Function ID: DES_SPI_API_305
* @param[in] Base: Module base pointer of type SPI_Type.
* @param[in] SckLow: SPI SCK Low count, low_time = (SckLow + 1) * Bus_Period.
* @param[in] SckHigh: SPI SCK High count, high_time = (SckHigh + 1) * Bus_Period.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetBaudRate(SPI_Type *Base, uint8 SckLow, uint8 SckHigh)
{
    MODIFY_REG32(Base->CFG0, SPI_CFG0_SCK_HIGH_Msk, SPI_CFG0_SCK_HIGH_Pos, SckHigh);
    MODIFY_REG32(Base->CFG0, SPI_CFG0_SCK_LOW_Msk, SPI_CFG0_SCK_LOW_Pos, SckLow);
}

/**
* @brief Get SPI SckLow and SckHigh.
* @param[in] Base: Module base pointer of type SPI_Type.
* @param[in] SckLow: SPI SCK Low count, low_time = (SckLow + 1) * Bus_Period.
* @param[in] SckHigh: SPI SCK High count, high_time = (SckHigh + 1) * Bus_Period.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_GetSckHighLow(const SPI_Type *Base, uint8 *SckLow, uint8 *SckHigh)
{
    uint32 tmp = Base->CFG0;

    *SckLow = (uint8)((tmp & SPI_CFG0_SCK_LOW_Msk) >> SPI_CFG0_SCK_LOW_Pos);
    *SckHigh = (uint8)((tmp & SPI_CFG0_SCK_HIGH_Msk) >> SPI_CFG0_SCK_HIGH_Pos);
}

/**
* @brief Set SPI frame size(4~32 bits per frame).
* @note Function ID: DES_SPI_API_306
* @param[in] Base: SPI base pointer.
* @param[in] FrameSize: SPI frame size, support 4~32 bits.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetFrameSize(SPI_Type *Base, uint8 FrameSize)
{
    MODIFY_REG32(Base->CFG1, SPI_CFG1_FRMSIZE_Msk, SPI_CFG1_FRMSIZE_Pos, ((uint32)FrameSize - 1U));
}

/**
* @brief Enable/Disable SPI module.
* @note Function ID: DES_SPI_API_307
* @param[in] Base: SPI base pointer.
* @param[in] Cmd: Enable/disable SPI module.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetEnable(SPI_Type *Base, boolean Cmd)
{
    MODIFY_REG32(Base->CMD, SPI_CMD_SPIEN_Msk, SPI_CMD_SPIEN_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
}

/*!
 * @brief Check if SPI module is enabled.
 *
 * @param[in] base: SPI base pointer
 * @return SPI module is enabled or not
 *         -true: SPI module is enabled
 *         -false: SPI module is disabled
 */
LOCAL_INLINE boolean SPI_Reg_IsModuleEnabled(const SPI_Type *base)
{
    return ((((base->CMD) & SPI_CMD_SPIEN_Msk) == SPI_CMD_SPIEN_Msk) ? TRUE : FALSE);
}

/**
* @brief Set SPI master or slave mode.
* @note Function ID: DES_SPI_API_308
* @param[in] Base: SPI base pointer.
* @param[in] Mode: Set SPI master mode or not.
*                  - SPI_MASTER: SPI master mode.
*                  - SPI_SLAVE: SPI slave mode.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetMasterSlaveMode(SPI_Type *Base, Spi_ModeType Mode)
{
    MODIFY_REG32(Base->CFG1, SPI_CFG1_MSTR_Msk, SPI_CFG1_MSTR_Pos, Mode);
}

/**
* @brief Returns whether the SPI module is in master mode.
* @note Function ID: DES_SPI_API_309
* @param[in] Base: SPI base pointer.
* @param[inout] None
* @param[out] None
* @return Returns the status of SPI master or slave mode.
*         - TRUE: SPI work in master mode.
*         - FALSE: SPI work in salve mode.
*/
LOCAL_INLINE boolean Spi_Reg_IsMaster(const SPI_Type *Base)
{
    return ((0U != (((Base->CFG1) & SPI_CFG1_MSTR_Msk) >> SPI_CFG1_MSTR_Pos)) ? TRUE : FALSE);
}

/*!
 * @brief Returns whether the SPI module is in Cs Continuous mode.
 *
 * @param[in] base: SPI base pointer
 * @return Returns the status of SPI Continuous or DisContinuous mode
 *         -true: SPI work in Continuous mode
 *         -false: SPI work in DisContinuous mode
 */
LOCAL_INLINE boolean SPI_Reg_IsCsContinuousMode(const SPI_Type *base)
{
    uint32 temp = (base)->CFG1;

    return ((((temp & SPI_CFG1_CSOE_Msk) == SPI_CFG1_CSOE_Msk)
             && ((temp & SPI_CFG1_CONT_CS_Msk) == SPI_CFG1_CONT_CS_Msk)) ? TRUE : FALSE);
}

/**
* @brief Gets the SPI status flag enable.
* @note Function ID: DES_SPI_API_310
* @param[in] Base: SPI base pointer.
* @param[in] StatusFlag: Select which status will be return status.
* @param[inout] None
* @param[out] None
* @return State of the status flag.
*         - TRUE: the status flag is "1".
*         - FALSE: the status flag is "0".
*/
LOCAL_INLINE boolean Spi_Reg_GetStatusFlag(const SPI_Type *Base, Spi_StatusFlagType StatusFlag)
{
    return ((0U != (((Base->STATUS) >> (uint8)StatusFlag) & 1U)) ? TRUE : FALSE);
}

/*!
 * @brief Clear SPI Status Flag.
 *
 * @param[in] base: SPI base pointer
 * @return none
 */
LOCAL_INLINE void SPI_Reg_ClearStatusFlag(SPI_Type *Base)
{
    Base->STATUS = (uint32)SPI_ALL_STATUS;
}

/**
* @brief Clear current SPI Tx underflow flag.
* @note Function ID: DES_SPI_API_311
* @param[in] Base: SPI base pointer.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_ClearTxUF(SPI_Type *Base)
{
    Base->STATUS = ((uint32)1U << (uint32)SPI_STATUS_TXUF_Pos);
}

/**
* @brief Clear current SPI Rx overflow flag.
* @note Function ID: DES_SPI_API_312
* @param[in] Base: SPI base pointer.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_ClearRxOF(SPI_Type *Base)
{
    Base->STATUS = ((uint32)1U << (uint32)SPI_STATUS_RXOF_Pos);
}

/**
* @brief Configures the SPI interrupts.
* @note Function ID: DES_SPI_API_313
* @param[in] Base: SPI base pointer.
* @param[in] InterruptSrc: The interrupt source, which will be enabled.
* @param[in] Cmd: Enable or disable the interrupt source.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetIntMode(SPI_Type *Base, Spi_InterruptType InterruptSrc, boolean Cmd)
{
    if (TRUE == Cmd)
    {
        SET_BIT32(Base->CFG1, ((uint32)1U << (uint8)InterruptSrc));
    }
    else
    {
        CLEAR_BIT32(Base->CFG1, ((uint32)1U << (uint8)InterruptSrc));
    }
}

/**
* @brief Returns if the SPI interrupt request is enabled or disabled.
* @note Function ID: DES_SPI_API_314
* @param[in] Base: SPI base pointer.
* @param[in] InterruptSrc: The interrupt source.
* @param[inout] None
* @param[out] None
* @return Return the interrupt source enable status.
*         - TRUE: the interrupt source is enabled.
*         - FALSE: the interrupt source is disabled.
*/
LOCAL_INLINE boolean Spi_Reg_GetIntMode(const SPI_Type *Base, Spi_InterruptType InterruptSrc)
{
    return ((0U != (((Base->CFG1) >> (uint8)InterruptSrc) & 1U)) ? TRUE : FALSE);
}

/**
* @brief Enable/disable the SPI Transmit Data DMA request.
* @note Function ID: DES_SPI_API_315
* @param[in] Base: SPI base pointer.
* @param[in] Cmd: Enable or disable the TX DMA request.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetTxDmaCmd(SPI_Type *Base, boolean Cmd)
{
    MODIFY_REG32(Base->CFG1, SPI_CFG1_DMATXEN_Msk, SPI_CFG1_DMATXEN_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
}

/**
* @brief Enable/disable the SPI Receive Data DMA request.
* @note Function ID: DES_SPI_API_316
* @param[in] Base: SPI base pointer.
* @param[in] Cmd: Enable or disable the RX DMA request.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetRxDmaCmd(SPI_Type *Base, boolean Cmd)
{
    MODIFY_REG32(Base->CFG1, SPI_CFG1_DMARXEN_Msk, SPI_CFG1_DMARXEN_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
}

/**
* @brief Configures the SPI SOUT/SIN pin configuration mode.
* @note Function ID: DES_SPI_API_317
* @param[in] Base: SPI base pointer.
* @param[in] PinCfg: Select configuration for the SOUT/SIN pins.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetPinConfigMode(SPI_Type *Base, Spi_PinCfgType PinCfg)
{
    MODIFY_REG32(Base->CFG1, SPI_CFG1_PINCFG_Msk, SPI_CFG1_PINCFG_Pos, PinCfg);
}

/**
* @brief Set SPI CPHA.
* @note Function ID: DES_SPI_API_318
* @param[in] Base: SPI base pointer.
* @param[in] ClkPhase: SPI clock CPHA
*                      - SPI_CPHA_0
*                      - SPI_CPHA_1
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetCPHA(SPI_Type *Base, Spi_CphaType ClkPhase)
{
    MODIFY_REG32(Base->CFG1, SPI_CFG1_CPHA_Msk, SPI_CFG1_CPHA_Pos, ClkPhase);
}

/**
* @brief Set SPI CPOL.
* @note Function ID: DES_SPI_API_319
* @param[in] Base: SPI base pointer.
* @param[in] ClkPolarity: SPI cpol select.
*                         - SPI_CPOL_LOW
*                         - SPI_CPOL_HIGH
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetCPOL(SPI_Type *Base, Spi_CpolType ClkPolarity)
{
    MODIFY_REG32(Base->CFG1, SPI_CFG1_CPOL_Msk, SPI_CFG1_CPOL_Pos, ClkPolarity);
}

/**
* @brief Set SPI RX MSB first.
* @note Function ID: DES_SPI_API_320
* @param[in] Base: SPI base pointer.
* @param[in] Cmd: Set the SPI RX MSB first.
*                 - TRUE: RX MSB
*                 - FALSE: RX LSB
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetRxMSB(SPI_Type *Base, boolean Cmd)
{
    MODIFY_REG32(Base->CFG1, SPI_CFG1_RMSBF_Msk, SPI_CFG1_RMSBF_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
}

/**
* @brief Set SPI TX MSB first.
* @note Function ID: DES_SPI_API_321
* @param[in] Base: SPI base pointer.
* @param[in] Cmd: Set the SPI TX MSB first.
*                 - TRUE: TX MSB
*                 - FALSE: TX LSB
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetTxMSB(SPI_Type *Base, boolean Cmd)
{
#if defined (AC7840X)
    MODIFY_REG32(Base->CFG1, SPI_CFG1_MSBF_Msk, SPI_CFG1_MSBF_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
#else
    MODIFY_REG32(Base->CFG1, SPI_CFG1_TMSBF_Msk, SPI_CFG1_TMSBF_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
#endif
}

/**
* @brief Set SPI CS continuous or disContinuous mode.
* @note Function ID: DES_SPI_API_322
* @param[in] Base: SPI base pointer.
* @param[in] Cmd: Set the SPI CS output continuous mode.
*                 - TRUE: CS continuous mode.
*                 - FALSE: CS discontinuous mode.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetContinuousCS(SPI_Type *Base, boolean Cmd)
{
    MODIFY_REG32(Base->CFG1, SPI_CFG1_CONT_CS_Msk, SPI_CFG1_CONT_CS_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
}

/**
* @brief Set SPI CS output enable.
* @note Function ID: DES_SPI_API_323
* @param[in] Base: SPI base pointer.
* @param[in] Cmd: Set the SPI CS hardware output.
*                 - TRUE: enable the CS hardware output(CS control by hardware).
*                 - FALSE: disable the CS hardware output(CS control by software).
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetCSOE(SPI_Type *Base, boolean Cmd)
{
    MODIFY_REG32(Base->CFG1, SPI_CFG1_CSOE_Msk, SPI_CFG1_CSOE_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
}

/**
* @brief Enable/Disable SPI CS mode fault detect function.
* @note Function ID: DES_SPI_API_324
* @param[in] Base: SPI base pointer.
* @param[in] Cmd: Set the SPI CS mode fault detect function.
*                 - TRUE: enable the master mode fault detect function.
*                 - FALSE: disable the master mode fault detect function.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetModeFault(SPI_Type *Base, boolean Cmd)
{
    MODIFY_REG32(Base->CFG1, SPI_CFG1_MODFEN_Msk, SPI_CFG1_MODFEN_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
}

/**
* @brief Release SPI CS, which will be changed to inactive enable.
* @note Function ID: DES_SPI_API_325
* @param[in] Base: SPI base pointer.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_ReleaseCS(SPI_Type *Base)
{
    SET_BIT32(Base->CMD, SPI_CMD_CSRLS_Msk);
}

/**
* @brief Sets the PCS flag to the value of the PcsCfg parameter.
* @note Function ID: DES_SPI_API_326
* @param[in] Base: SPI base pointer.
* @param[in] PcsCfg: Select which CS is assert.
*                    - SPI_PCS_0
*                    - SPI_PCS_1
*                    - SPI_PCS_2
*                    - SPI_PCS_3
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetPcsCfg(SPI_Type *Base, Spi_PcsType PcsCfg)
{
    MODIFY_REG32(Base->CFG2, SPI_CFG2_PCSCFG_Msk, SPI_CFG2_PCSCFG_Pos, PcsCfg);
}

/**
* @brief Set SPI transfer data width.
* @note Function ID: DES_SPI_API_327
* @param[in] Base: SPI base pointer.
* @param[in] DataWidth: transfer data width.
*                       - SPI_DATA_WIDTH_1BIT
*                       - SPI_DATA_WIDTH_2BIT
*                       - SPI_DATA_WIDTH_4BIT
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetDataWidth(SPI_Type *Base, Spi_TransferWidthType DataWidth)
{
    MODIFY_REG32(Base->CFG2, SPI_CFG2_WIDTH_Msk, SPI_CFG2_WIDTH_Pos, DataWidth);
}

/**
* @brief Writes data into the TX data register.
* @note Function ID: DES_SPI_API_328
* @param[in] Base: SPI base pointer.
* @param[in] Data: The data word to be sent.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_WriteData(SPI_Type *Base, uint32 Data)
{
    Base->DATA = Data;
}

/**
* @brief Reads data from the data register.
* @note Function ID: DES_SPI_API_329
* @param[in] Base: SPI base pointer.
* @param[inout] None
* @param[out] None
* @return The RX data read from the data register.
*/
LOCAL_INLINE uint32 Spi_Reg_ReadData(const SPI_Type *Base)
{
    return (uint32)(Base->DATA);
}

/**
* @brief Get address of the data register.
* @note Function ID: DES_SPI_API_330
* @param[in] Base: SPI base pointer.
* @param[inout] None
* @param[out] None
* @return The address of the data register.
*/
LOCAL_INLINE uint32 Spi_Reg_GetDataAddress(const SPI_Type *Base)
{
    return (uint32)(&(Base->DATA));
}

/**
* @brief Config Master no overflow mode.
* @note Function ID: DES_SPI_API_331
* @param[in] Base: SPI base pointer.
* @param[in] Cmd: Enable or disable SPI no over flow mode.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetMasterNoOverflowMode(SPI_Type *Base, boolean Cmd)
{
    MODIFY_REG32(Base->CFG2, SPI_CFG2_MNOV_Msk, SPI_CFG2_MNOV_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
}

/**
* @brief Enable/Disable TX Only mode.
* @note Function ID: DES_SPI_API_332
* @param[in] Base: SPI base pointer.
* @param[in] Cmd: enable/disable SPI TX only mode.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetTxOnly(SPI_Type *Base, boolean Cmd)
{
    MODIFY_REG32(Base->CFG2, SPI_CFG2_TOEN_Msk, SPI_CFG2_TOEN_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
}

/**
* @brief Get TX Only mode.
* @note Function ID: DES_SPI_API_333
* @param[in] Base: SPI base pointer.
* @param[inout] None
* @param[out] None
* @return Returns SPI TX only mode is enabled/disabled.
*         - TRUE: SPI TX only mode is enabled.
*         - FALSE: SPI TX only mode is disabled.
*/
LOCAL_INLINE boolean Spi_Reg_GetTxOnly(const SPI_Type *Base)
{
    return ((0U != (((Base->CFG2) >> (uint8)SPI_CFG2_TOEN_Pos) & 1U)) ? TRUE : FALSE);
}

/**
* @brief Enable/Disable RX Only mode.
* @note Function ID: DES_SPI_API_334
* @param[in] Base: SPI base pointer.
* @param[in] Cmd: enable/disable SPI RX only mode.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetRxOnly(SPI_Type *Base, boolean Cmd)
{
    MODIFY_REG32(Base->CFG2, SPI_CFG2_ROEN_Msk, SPI_CFG2_ROEN_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
}

/**
* @brief Get RX Only mode.
* @note Function ID: DES_SPI_API_335
* @param[in] Base: SPI base pointer.
* @param[inout] None
* @param[out] None
* @return Returns SPI RX only mode is enabled/disabled.
*         - TRUE: SPI RX only mode is enabled.
*         - FALSE: SPI RX only mode is disabled.
*/
LOCAL_INLINE boolean Spi_Reg_GetRxOnly(const SPI_Type *Base)
{
    return ((0U != (((Base->CFG2) >> (uint8)SPI_CFG2_ROEN_Pos) & 1U)) ? TRUE : FALSE);
}

/**
* @brief Set SPI host request polarity.
* @note Function ID: DES_SPI_API_336
* @param[in] Base: SPI base pointer.
* @param[in] HreqPolarity: Host request polarity.
*                          - SPI_HREQ_POLARITY_LOW
*                          - SPI_HREQ_POLARITY_HIGH
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetHreqPolarity(SPI_Type *Base, Spi_HreqPolarityType HreqPolarity)
{
    MODIFY_REG32(Base->CFG2, SPI_CFG2_HRPOL_Msk, SPI_CFG2_HRPOL_Pos, HreqPolarity);
}

/**
* @brief Enable or disable SPI host request.
* @note Function ID: DES_SPI_API_337
* @param[in] Base: SPI base pointer.
* @param[in] Cmd: Enable or disable SPI host request.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetHreq(SPI_Type *Base, boolean Cmd)
{
    MODIFY_REG32(Base->CFG2, SPI_CFG2_HREN_Msk, SPI_CFG2_HREN_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
}

/**
* @brief Enable or disable SPI in debug mode.
* @note Function ID: DES_SPI_API_338
* @param[in] Base: SPI base pointer.
* @param[in] Cmd: Enable or disable SPI in debug mode.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_SetDebug(SPI_Type *Base, boolean Cmd)
{
    MODIFY_REG32(Base->CFG2, SPI_CFG2_DBGEN_Msk, SPI_CFG2_DBGEN_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
}

/*!
 * @brief Enable or disable SPI Master Mode Fault Detect in cs0.
 *
 * @param[in] base: SPI base pointer
 * @param[in] enable: Enable or disable SPI Master Mode Fault Detect
 * @return none
 */
LOCAL_INLINE void SPI_Reg_SetMasterModeFaultDetectCs0(SPI_Type *base, boolean enable)
{
    MODIFY_REG32(base->CFG2, SPI_CFG2_CS0DETEN_Msk, SPI_CFG2_CS0DETEN_Pos, enable);
}

/*!
 * @brief Enable or disable SPI Master Mode Fault Detect in cs1.
 *
 * @param[in] base: SPI base pointer
 * @param[in] enable: Enable or disable SPI Master Mode Fault Detect
 * @return none
 */
LOCAL_INLINE void SPI_Reg_SetMasterModeFaultDetectCs1(SPI_Type *base, boolean enable)
{
    MODIFY_REG32(base->CFG2, SPI_CFG2_CS1DETEN_Msk, SPI_CFG2_CS1DETEN_Pos, enable);
}

/*!
 * @brief Enable or disable SPI Master Mode Fault Detect in cs2.
 *
 * @param[in] base: SPI base pointer
 * @param[in] enable: Enable or disable SPI Master Mode Fault Detect
 * @return none
 */
LOCAL_INLINE void SPI_Reg_SetMasterModeFaultDetectCs2(SPI_Type *base, boolean enable)
{
    MODIFY_REG32(base->CFG2, SPI_CFG2_CS2DETEN_Msk, SPI_CFG2_CS2DETEN_Pos, enable);
}

/*!
 * @brief Enable or disable SPI Master Mode Fault Detect in cs3.
 *
 * @param[in] base: SPI base pointer
 * @param[in] enable: Enable or disable SPI Master Mode Fault Detect
 * @return none
 */
LOCAL_INLINE void SPI_Reg_SetMasterModeFaultDetectCs3(SPI_Type *base, boolean enable)
{
    MODIFY_REG32(base->CFG2, SPI_CFG2_CS3DETEN_Msk, SPI_CFG2_CS3DETEN_Pos, enable);
}

/*!
 * @brief Set SPI match data.
 *
 * @param[in] base: SPI base pointer
 * @return none
 */
LOCAL_INLINE void Spi_Reg_SetMatchData(SPI_Type *Base, uint32 Data)
{
    WRITE_REG32(Base->DMV, Data);
}

/*!
 * @brief Set SPI match data.
 *
 * @param[in] base: SPI base pointer
 * @return none
 */
LOCAL_INLINE void Spi_Reg_SetDMIE(SPI_Type *Base, boolean Cmd)
{
    MODIFY_REG32(Base->CFG1, SPI_CFG1_DMIE_Msk, SPI_CFG1_DMIE_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
}

/**
* @brief Clear current SPI RDMF flag.
* @param[in] Base: SPI base pointer.
* @param[inout] None
* @param[out] None
* @return None
*/
LOCAL_INLINE void Spi_Reg_ClearRDMF(SPI_Type *Base)
{
    Base->STATUS = ((uint32)1U << (uint32)SPI_STATUS_RDMF_Pos);
}

/*!
 * @brief Toggle SPI RX Only.
 *
 * @param[in] base: SPI base pointer
 * @return none
 */
LOCAL_INLINE void Spi_Reg_SetROTRIG(SPI_Type *Base, boolean Cmd)
{
    MODIFY_REG32(Base->CMD, SPI_CMD_ROTRIG_Msk, SPI_CMD_ROTRIG_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
}

#if defined (AC7842X)  || defined (AC7843X) || defined (AC7840E)
/*!
 * @brief Set SPI RX Only COUNT.
 *
 * @param[in] base: SPI base pointer
 * @param[in] count: How many data to read
 * @return none
 */
LOCAL_INLINE void Spi_Reg_SetROCNT(SPI_Type *Base, uint16 Count)
{
    MODIFY_REG32(Base->CMD, SPI_CMD_ROCNT_Msk, SPI_CMD_ROCNT_Pos, Count);
}

/*!
 * @brief Set SPI DATAME Enable.
 *
 * @param[in] base: SPI base pointer
 * @param[in] Cmd: Enable or disable SPI in DATAME mode.
 * @return none
 */
LOCAL_INLINE void Spi_Reg_SetDATAME(SPI_Type *Base, boolean Cmd)
{
    MODIFY_REG32(Base->CFG2, SPI_CFG2_DATAME_Msk, SPI_CFG2_DATAME_Pos,
                 ((TRUE == Cmd) ? 1U : 0U));
}
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /**< AC7840X_SPI_REG_H */

/*====================================================EOF===========================================*/
