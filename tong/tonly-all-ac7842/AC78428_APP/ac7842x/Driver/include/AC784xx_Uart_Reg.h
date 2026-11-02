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
 * @file AC784xx_Uart_Reg.h
 *
 * @brief This file provides extern Reg lin api.
 */

#ifndef AC784XX_UART_REG_H
#define AC784XX_UART_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"
#include "Uart_Hal_Types.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
#define UART_DIV_MASK           (0xFFU)
#define UART_DIV_H_POS          (0x8U)
#define UART_DIV_FRAC_VAL       (0x20U)
#define UART_INDEX_MAX          (4UL)

#define UART_SHIFT              (16U)
#define UART_LSR0_REG_ID        (1U)
#define UART_LSR1_REG_ID        (2U)
#define UART_IER_REG_ID         (3U)
#define UART_MATCHCR_REG_ID     (4U)
#define UART_LINCR_REG_ID       (5U)
#define UART_IDLE_REG_ID        (6U)

#define UART_ALL_ERR_FLAG       (UART_LSR0_OE_Msk | UART_LSR0_PE_Msk | UART_LSR0_FE_Msk \
                                    | UART_LSR0_BI_Msk | UART_LSR0_NE_Msk)

/*===================================================ENUMS==========================================*/
/*!
 * @brief UART interrupt configuration structure.
 */
typedef enum
{
    UART_INT_RX_NOT_EMPTY = (((uint32)UART_IER_REG_ID << (uint32)UART_SHIFT) \
                             | (uint32)UART_IER_ERXNE_Pos),
    UART_INT_TX_NOT_FULL = (((uint32)UART_IER_REG_ID << (uint32)UART_SHIFT) \
                            | (uint32)UART_IER_ETXNF_Pos),
    UART_INT_TX_COMPLETE = (((uint32)UART_IER_REG_ID << (uint32)UART_SHIFT) \
                            | (uint32)UART_IER_ETC_Pos),
    UART_INT_PARITY_ERR = (((uint32)UART_IER_REG_ID << (uint32)UART_SHIFT) \
                           | (uint32)UART_IER_EPE_Pos),
    UART_INT_FRAME_ERR = (((uint32)UART_IER_REG_ID << (uint32)UART_SHIFT) \
                          | (uint32)UART_IER_EFE_Pos),
    UART_INT_NOISE_ERR = (((uint32)UART_IER_REG_ID << (uint32)UART_SHIFT) \
                          | (uint32)UART_IER_ENE_Pos),
    UART_INT_RX_OVERRUN = (((uint32)UART_IER_REG_ID << (uint32)UART_SHIFT) \
                           | (uint32)UART_IER_EOEBI_Pos),
    UART_INT_CTS_CHANGE = (((uint32)UART_IER_REG_ID << (uint32)UART_SHIFT) \
                           | (uint32)UART_IER_EDCTS_Pos),
    UART_INT_TX_FULL = (((uint32)UART_IER_REG_ID << (uint32)UART_SHIFT) \
                        | (uint32)UART_IER_ETXDF_Pos),
    UART_INT_DATA_MATCH = (((uint32)UART_MATCHCR_REG_ID << (uint32)UART_SHIFT) \
                           | (uint32)UART_MTCHCR_MTCHIE_Pos),
    UART_INT_LIN_WAKEUP = (((uint32)UART_LINCR_REG_ID << (uint32)UART_SHIFT) \
                           | (uint32)UART_LINCR_LINWAKIE_Pos),
    UART_INT_LIN_SYNC_ERR = (((uint32)UART_LINCR_REG_ID << (uint32)UART_SHIFT) \
                             | (uint32)UART_LINCR_SYNERRIE_Pos),
    UART_INT_LIN_BREAK = (((uint32)UART_LINCR_REG_ID << (uint32)UART_SHIFT) \
                          | (uint32)UART_LINCR_LBRKIE_Pos),
    UART_INT_IDLE_LINE = (((uint32)UART_IDLE_REG_ID << (uint32)UART_SHIFT) \
                          | (uint32)UART_IDLE_IDLEIE_Pos),
} Uart_InterruptConfigType;

typedef enum
{
    UART_RX_DATA_READY = (((uint32)UART_LSR0_REG_ID << (uint32)UART_SHIFT) \
                          | (uint32)UART_LSR0_DR_Pos),
    UART_RX_OVERRUN = (((uint32)UART_LSR0_REG_ID << (uint32)UART_SHIFT) \
                       | (uint32)UART_LSR0_OE_Pos),
    UART_PARITY_ERR = (((uint32)UART_LSR0_REG_ID << (uint32)UART_SHIFT) \
                       | (uint32)UART_LSR0_PE_Pos),
    UART_FRAME_ERR = (((uint32)UART_LSR0_REG_ID << (uint32)UART_SHIFT) \
                      | (uint32)UART_LSR0_FE_Pos),
    UART_BREAK_ERR = (((uint32)UART_LSR0_REG_ID << (uint32)UART_SHIFT) \
                      | (uint32)UART_LSR0_BI_Pos),
    UART_TX_DATA_NOT_FULL = (((uint32)UART_LSR0_REG_ID << (uint32)UART_SHIFT) \
                             | (uint32)UART_LSR0_TXNF_Pos),
    UART_TX_COMPLETE = (((uint32)UART_LSR0_REG_ID << (uint32)UART_SHIFT) \
                        | (uint32)UART_LSR0_TC_Pos),
    UART_NOISE_ERR = (((uint32)UART_LSR0_REG_ID << (uint32)UART_SHIFT) \
                      | (uint32)UART_LSR0_NE_Pos),
    UART_TX_DATA_FULL = (((uint32)UART_LSR0_REG_ID << (uint32)UART_SHIFT) \
                         | (uint32)UART_LSR0_TXDF_Pos),
    UART_IDLE_LINE = (((uint32)UART_LSR1_REG_ID << (uint32)UART_SHIFT) \
                      | (uint32)UART_LSR1_IDLE_Pos),
    UART_LIN_SYNC_ERR = (((uint32)UART_LSR1_REG_ID << (uint32)UART_SHIFT) \
                         | (uint32)UART_LSR1_SYNERR_Pos),
    UART_LIN_BREAK = (((uint32)UART_LSR1_REG_ID << (uint32)UART_SHIFT) \
                      | (uint32)UART_LSR1_FBRK_Pos),
    UART_CTS_CHANGE = (((uint32)UART_LSR1_REG_ID << (uint32)UART_SHIFT) \
                       | (uint32)UART_LSR1_DCTS_Pos),
    UART_LIN_WAKEUP = (((uint32)UART_LSR1_REG_ID << (uint32)UART_SHIFT) \
                       | (uint32)UART_LSR1_LINWAK_Pos),
    UART_IDLE = (((uint32)UART_LSR1_REG_ID << (uint32)UART_SHIFT) \
                 | (uint32)UART_LSR1_UART_IDLE_Pos),
    UART_CTS = (((uint32)UART_LSR1_REG_ID << (uint32)UART_SHIFT) \
                | (uint32)UART_LSR1_CTS_Pos),
    UART_RTS = (((uint32)UART_LSR1_REG_ID << (uint32)UART_SHIFT) \
                | (uint32)UART_LSR1_RTS_Pos),
    UART_DATA_MATCH = (((uint32)UART_LSR1_REG_ID << (uint32)UART_SHIFT) \
                       | (uint32)UART_LSR1_DTMTCH_Pos),
    UART_ADDR_FILTER_MATCH = (((uint32)UART_LSR1_REG_ID << (uint32)UART_SHIFT) \
                              | (uint32)UART_LSR1_ADDMTCH_Pos)
} Uart_StatusFlagType;

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

/* =====================================  Functions definition  ===================================== */
/**
* @brief Get UART base.
*
* @param[in] Instance : UART hardware channel ID.
* @return UART_Type*: the UART base addr.
*/
LOCAL_INLINE UART_Type *Uart_Reg_GetBase(uint8 Instance)
{
    DEVICE_ASSERT(Instance < UART_INSTANCE_MAX);
    UART_Type *const UartBase[UART_INSTANCE_MAX] =
    {
        UART0, UART1, UART2,
#if !defined(AC7840E)
        UART3,
#endif
#if defined (AC7843X)
        UART4, UART5, UART6, UART7
#endif
    };

    return UartBase[Instance];
}

/**
* @brief uart interrupt disable or enable
*
* @param [in] UARTx: UART interrupt sources
* @param [in] InterruptEn: interrupt en bits
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetInterruptEn(UART_Type *Base, uint16 InterruptEn)
{
    /* set interrupt enable register */
    MODIFY_REG32(Base->IER, 0x1FFU, 0U, ((uint32)InterruptEn & 0x1FFU));
}

/**
* @brief Get UART status flags.
*
* @param [in] Base: UART Base pointer
* @param [in] statusFlag: Select which status flag
* @return The selected status flag is true or false
*/
LOCAL_INLINE boolean Uart_Reg_GetStatusFlag(const UART_Type *Base, Uart_StatusFlagType StatusFlag)
{
    uint32 Reg = (uint32)(StatusFlag) >> UART_SHIFT;
    boolean RetVal = (boolean)FALSE;

    switch (Reg)
    {
    case UART_LSR0_REG_ID:
        RetVal = (((Base->LSR0 >> (uint8)((uint16)StatusFlag & 0x000FU)) & 1U) > 0U) ? TRUE : FALSE;
        break;

    case UART_LSR1_REG_ID:
        RetVal = (((Base->LSR1 >> (uint8)((uint16)StatusFlag & 0x000FU)) & 1U) > 0U) ? TRUE : FALSE;
        break;

    default:
        /* Do nothing */
        break;
    }

    return RetVal;
}

/**
* @brief Sends the UART 7/8-bit character.
*
* @param [in] Base: UART Instance
* @param [in] Data: data to send (7/8-bit)
* @return void
*/
LOCAL_INLINE void Uart_Reg_PutChar(UART_Type *Base, uint8 Data)
{
    Base->RBR = ((uint32)Data & UART_RBR_RBR_THR_Msk);
}

/**
* @brief UART send 9-bits data.
*
* @param [in] Base: UART Instance
* @param [in] Data: The data which will be send
* @return void
*/
LOCAL_INLINE void Uart_Reg_PutChar9(UART_Type *Base, uint16 Data)
{
    Base->RBR = ((uint32)Data & UART_RBR_RBR_THR_Msk);
}

/**
* @brief Gets the UART 7/8-bit character.
*
* @param [in] Base: UART Instance
* @param [in] ReadData: Data read from receiver (7/8-bit)
* @return void
*/
LOCAL_INLINE void Uart_Reg_GetChar(const UART_Type *Base, uint8 *ReadData)
{
    DEVICE_ASSERT(ReadData != NULL_PTR);

    *ReadData = (uint8)Base->RBR;
}

/**
* @brief UART get 9-bits data.
*
* @param [in] Base: UART Instance
* @param [in] ReadData: Data read from receiver (7/8-bit)
* @return void
*/
LOCAL_INLINE void Uart_Reg_GetChar9(const UART_Type *Base, uint16 *ReadData)
{
    /* get 9-bits data from uart RBR register */
    *ReadData = (uint16)((Base->RBR) & UART_RBR_RBR_THR_Msk) ;
}

/**
* @brief Send LIN break.
*
* @param [in] Base: UART Base pointer
* @return void
*/
LOCAL_INLINE void Uart_Reg_SendLINBreak(UART_Type *Base)
{
    SET_BIT32(Base->LINCR, UART_LINCR_SDBRK_Msk);
}

/**
* @brief Get LSR0 register value.
*
* @param [in] Base: UART Base pointer
* @return LSR0 register value
*/
LOCAL_INLINE uint32 Uart_Reg_GetLSR0(const UART_Type *Base)
{
    return (Base->LSR0);
}

/**
* @brief Get LSR1 register value.
*
* @param [in] Base: UART Base pointer
* @return LSR1 register value
*/
LOCAL_INLINE uint32 Uart_Reg_GetLSR1(const UART_Type *Base)
{
    return (Base->LSR1);
}

/**
* @brief Set UART interrupt mode.
*
* @param [in] Base: UART interrupt sources
* @param [in] IntSrc: LINCR reg data
* @param [in] IsEnable: TRUE or FALSE
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetIntMode(UART_Type *Base, Uart_InterruptConfigType IntSrc, boolean IsEnable)
{
    uint32 Reg;
    uint16 IntRegOffset;
    uint32 Mask;

    Reg = (uint32)(IntSrc) >> UART_SHIFT;
    IntRegOffset = (uint16)IntSrc & 0x000FU;
    Mask = (uint32)((uint32)0x1U << (IntRegOffset & 0x000FU));
    switch (Reg)
    {
    case UART_IER_REG_ID:
        MODIFY_REG32(Base->IER, Mask, (uint32)IntRegOffset, (TRUE == IsEnable) ? 1UL : 0UL);
        break;

    case UART_MATCHCR_REG_ID:
        MODIFY_REG32(Base->MTCHCR, Mask, (uint32)IntRegOffset, (TRUE == IsEnable) ? 1UL : 0UL);
        break;

    case UART_LINCR_REG_ID:
        MODIFY_REG32(Base->LINCR, Mask, (uint32)IntRegOffset, (TRUE == IsEnable) ? 1UL : 0UL);
        break;

    default :
        /* Do nothing */
        break;
    }
}

/**
* @brief IsEnable or disable the UART error interrupts.
*
* @param [in] Base: UART Base pointer
* @param [in] IsEnable: set error interrupt
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetErrorInterrupts(UART_Type *Base, boolean IsEnable)
{
    /* Configure the error interrupts */
    Uart_Reg_SetIntMode(Base, UART_INT_RX_OVERRUN, IsEnable);
    Uart_Reg_SetIntMode(Base, UART_INT_PARITY_ERR, IsEnable);
    Uart_Reg_SetIntMode(Base, UART_INT_NOISE_ERR, IsEnable);
    Uart_Reg_SetIntMode(Base, UART_INT_FRAME_ERR, IsEnable);
}

/**
* @brief Check if error interrupts are enabled.
*
* @param [in] Base: UART Base pointer
* @return TRUE means enabled, FLASE means disabled
*/
LOCAL_INLINE boolean Uart_Reg_IsErrorInterruptEnable(const UART_Type *Base)
{
    uint32 Mask = UART_IER_EPE_Msk | UART_IER_EFE_Msk | UART_IER_ENE_Msk | UART_IER_EOEBI_Msk;

    return ((Base->IER & Mask) != 0U) ? TRUE : FALSE;
}

/*!
* @brief Set UART idle interrupt.
*        Note: Please enable idle function first by called function
*              UART_SetIdleFun, otherwise idle interrupt is invalid.
*
* @param [in] Base: UART base pointer
* @param [in] IsEnable: IsEnable/disable idle interrupt
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetIdleInterrupt(UART_Type *Base, boolean IsEnable)
{
    MODIFY_REG32(Base->IDLE, UART_IDLE_IDLEEN_Msk, UART_IDLE_IDLEEN_Pos, (TRUE == IsEnable) ? 1UL : 0UL);
    MODIFY_REG32(Base->IDLE, UART_IDLE_IDLEIE_Msk, UART_IDLE_IDLEIE_Pos, (TRUE == IsEnable) ? 1UL : 0UL);
}

/**
* @brief Configures UART transmit DMA request.
*
* @param [in] Base: UART Base pointer
* @param [in] IsEnable: Transmit DMA request configuration
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetTxDmaCmd(UART_Type *Base, boolean IsEnable)
{
    Base->DMA_EN = (Base->DMA_EN & ~UART_DMA_EN_TX_DMA_EN_Msk) | \
                   (((TRUE == IsEnable) ? 1UL : 0UL) << UART_DMA_EN_TX_DMA_EN_Pos);
}

/**
* @brief Configures UART receive DMA request.
*
* @param [in] Base: UART Base pointer
* @param [in] IsEnable: Receive DMA request configuration (enable: 1/disable: 0)
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetRxDmaCmd(UART_Type *Base, boolean IsEnable)
{
    Base->DMA_EN = (Base->DMA_EN & ~UART_DMA_EN_RX_DMA_EN_Msk) | \
                   (((TRUE == IsEnable) ? 1UL : 0UL) << UART_DMA_EN_RX_DMA_EN_Pos);
}

/**
* @brief Clears the error status flags.
*
* @param [in] Base: UART Base pointer
* @param [in] Mask: Error flag
* @return void
*/
LOCAL_INLINE void Uart_Reg_ClearErrorFlags(UART_Type *Base, uint32 Mask)
{
    Base->LSR0 = (UART_ALL_ERR_FLAG & Mask);
}

/**
* @brief Get UART interrupt source is enabled or disabled.
*
* @param [in] Base: UART Base pointer
* @param [in] IntSrc: UART interrupt sources
* @return Whether the interrupt source is enabled or disabled
*/
LOCAL_INLINE boolean Uart_Reg_GetIntMode(const UART_Type *Base, Uart_InterruptConfigType IntSrc)
{
    uint32 Reg = (uint32)(IntSrc) >> UART_SHIFT;
    boolean RetVal = (boolean)FALSE;

    switch (Reg)
    {
    case UART_IER_REG_ID:
        RetVal = (((Base->IER >> ((uint16)IntSrc & 0xFFFFU)) & 1U) > 0U) ? TRUE : FALSE;
        break;

    case UART_MATCHCR_REG_ID:
        RetVal = (((Base->MTCHCR >> ((uint16)IntSrc & 0xFFFFU)) & 1U) > 0U) ? TRUE : FALSE;
        break;

    case UART_LINCR_REG_ID:
        RetVal = (((Base->LINCR >> ((uint16)IntSrc & 0xFFFFU)) & 1U) > 0U) ? TRUE : FALSE;
        break;

    case UART_IDLE_REG_ID:
        RetVal = (((Base->IDLE >> ((uint16)IntSrc & 0xFFFFU)) & 1U) > 0U) ? TRUE : FALSE;
        break;
    default:
        /* Do nothing */
        break;
    }

    return RetVal;
}

/**
* @brief This function is used to set sample counter.
*
* @param [in] Base: UART base pointer
* @param [in] SampleCnt: Uart_SampleCNTConfigType
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetSampleCounter(UART_Type *Base, uint32 SampleCnt)
{
    WRITE_REG32(Base->SMP_CNT, SampleCnt & (uint32)UART_SMP_CNT_SMP_CNT_Msk);
}

/*!
 * @brief Get UART sample counter value.
 *
 * @param [in] Base: UART base pointer
 * @return UART over sample counter value
 */
LOCAL_INLINE uint32 Uart_Reg_GetSampleCounter(const UART_Type *Base)
{
    return Base->SMP_CNT & UART_SMP_CNT_SMP_CNT_Msk;
}

/**
* @brief This function is used to set baud rate divisor.
*
* @param [in] BaseAddress: UART base pointer
* @param [in] BaudRateDivisor: the baudRate what will to set
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetBaudRateDivisor(UART_Type *Base, float32 BaudRateDivisor)
{
    uint32 IntDivisor;
    uint32 FracDivisor;

    IntDivisor = (uint32)BaudRateDivisor;
    /*PRQA S 4395 ++ # AMCE_UART_4395.*/
    FracDivisor = (uint32)((BaudRateDivisor - (float32)IntDivisor) * (float32)UART_DIV_FRAC_VAL);
    /*PRQA S 4395 -- */

    WRITE_REG32(Base->DIV_L, IntDivisor & UART_DIV_MASK);
    WRITE_REG32(Base->DIV_H, ((IntDivisor >> UART_DIV_H_POS) & UART_DIV_MASK));
    WRITE_REG32(Base->DIV_FRAC, FracDivisor);
}

/*!
 * @brief Gets the UART baud rate divisor.
 *
 * @param [in] Base: UART base pointer
 * @return The baud rate divisor
 */
LOCAL_INLINE float32 Uart_Reg_GetBaudRateDivisor(const UART_Type *Base)
{
    uint16 IntDivisor = 0U;
    float32 Divisor = 0.0F;

    IntDivisor = (uint16)((Base->DIV_H & UART_DIV_H_DIV_H_Msk) << 8U);
    IntDivisor |= (uint16)(Base->DIV_L & UART_DIV_L_DIV_L_Msk);
    Divisor = (float32)IntDivisor + ((float32)Base->DIV_FRAC / 32.0F);

    return Divisor;
}

/**
* @brief UART set bit count per char.
*
* @param [in] Base: UART base pointer
* @param [in] BitCountPerChar: 7, 8 or 9 bits per char
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetBitCountPerChar
(
    UART_Type *Base,
    Uart_PerCharConfigType BitCountPerChar
)
{
    MODIFY_REG32(Base->LCR0, UART_LCR0_WLS1_WLS0_Msk, UART_LCR0_WLS1_WLS0_Pos, BitCountPerChar);
}

/**
* @brief UART set parity mode.
*
* @param [in] Base: UART base pointer
* @param [in] Parity: parity disable, odd or even
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetParityMode(UART_Type *Base, Uart_ParityModeConfigType Parity)
{
    uint32 RegVal;

    RegVal = READ_MEM32(&(Base->LCR0));
    WRITE_REG32(Base->LCR0, RegVal & (~UART_LCR0_SP_Msk));

    switch (Parity)
    {
    case UART_PARITY_ODD: /* UART_PARITY_ODD */
        RegVal |= UART_LCR0_PEN_Msk; /* parity ENABLE */
        RegVal &= ~UART_LCR0_EPS_Msk; /* set EPS to odd */
        break;

    case UART_PARITY_EVEN: /* UART_PARITY_EVEN */
        RegVal |= UART_LCR0_PEN_Msk; /* parity ENABLE */
        RegVal |= UART_LCR0_EPS_Msk; /* set EPS to even*/
        break;

    case UART_PARITY_DISABLED: /* UART_PARITY_DISABLED */
        RegVal &= ~UART_LCR0_PEN_Msk; /* parity DISABLE */
        RegVal &= ~UART_LCR0_EPS_Msk; /* set EPS to odd*/
        break;

    default:
        RegVal &= ~UART_LCR0_PEN_Msk; /* parity DISABLE */
        RegVal &= ~UART_LCR0_EPS_Msk; /* set EPS to odd*/
        break;
    }

    WRITE_REG32(Base->LCR0, RegVal);
}

/**
* @brief Configures the number of stop bits.
*
* @param [in] Base: UART base pointer
* @param [in] StopBitCount: Number of stop bits
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetStopBitCount(UART_Type *Base, Uart_StopBitConfigType StopBitCount)
{
    MODIFY_REG32(Base->LCR0, UART_LCR0_STB_Msk, UART_LCR0_STB_Pos, StopBitCount);
}

/**
* @brief IsEnable or disable UART fifo.
*
* @param [in] Base: UART base pointer
* @param [in] IsEnable: enable or disable UART fifo
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetFIFO(UART_Type *Base, boolean IsEnable)
{
    MODIFY_REG32(Base->FCR, UART_FCR_FIFOE_Msk, UART_FCR_FIFOE_Pos, (TRUE == IsEnable) ? 1UL : 0UL);
}

/**
* @brief This function is used to set transmitter cmd.
*
* @param [in] Base: UART base pointer
* @param [in] IsEnable: enable or disable set transmit cmd
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetTransmitterCmd(UART_Type *Base, boolean IsEnable)
{
    MODIFY_REG32(Base->LCR1, UART_LCR1_TXEN_Msk, UART_LCR1_TXEN_Pos, (TRUE == IsEnable) ? 1UL : 0UL);
}

/**
* @brief This function is used to set receiver cmd.
*
* @param [in] Base: UART base pointer
* @param [in] IsEnable: enable or disable set receive cmd
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetReceiverCmd(UART_Type *Base, boolean IsEnable)
{
    MODIFY_REG32(Base->LCR1, UART_LCR1_RXEN_Msk, UART_LCR1_RXEN_Pos, (TRUE == IsEnable) ? 1UL : 0UL);
}

/*!
 * @brief Set LIN break threshold length for slave detect.
 *
 * @param [in] Base: UART base pointer
 * @param [in] Length:LIN break threshold length
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetLinBreakThreshold(UART_Type *Base, uint32 Length)
{
    MODIFY_REG32(Base->LINCR, UART_LINCR_LBRKDL_Msk, UART_LINCR_LBRKDL_Pos, Length);
}

/**
* @brief This function is used to set LIN break lenght.
*
* @param [in] Base: UART base pointer
* @param [in] Length: BRKLGH length
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetLinBreakLength(UART_Type *Base, uint32 Length)
{
    MODIFY_REG32(Base->BRKLGH, UART_BRKLGH_BRKLGH_Msk, UART_BRKLGH_BRKLGH_Pos, Length);
}

/**
* @brief This function is used to set LIN ctrl.
*
* @param [in] Base: UART base pointer
* @param [in] LinCtrl: LINCR reg data
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetLinCtrl(UART_Type *Base, uint32 LinCtrl)
{
    MODIFY_REG32(Base->LINCR, 0xFFU, 0U, LinCtrl);
}

/**
* @brief This function is used to set LIN sleep.
*
* @param [in] Base: UART interrupt sources
* @param [in] IsEnable: TREU or FALSE
* @return void
*/
LOCAL_INLINE void Uart_Reg_SetLinSleep(UART_Type *Base, boolean IsEnable)
{
    MODIFY_REG32(Base->LINCR, UART_LINCR_LINSLP_Msk, UART_LINCR_LINSLP_Pos, (TRUE == IsEnable) ? 1UL : 0UL);
}

/**
* @brief This function is used to read LIN sleep status.
*
* @param [in] Base: UART interrupt sources
* @return void
*/
LOCAL_INLINE uint32 Uart_Reg_ReadLinSleep(const UART_Type *Base)
{
    uint32 LinCRData = READ_BIT32(Base->LINCR, UART_LINCR_LINSLP_Msk);

    return LinCRData;
}

/**
* @brief Clear UART status flags.
*
* @param [in] Base: UART interrupt sources
* @param [in] StatusFlag: Select which status flag to be cleared
* @return void
*/
LOCAL_INLINE void Uart_Reg_ClearStatusFlag(UART_Type *Base, Uart_StatusFlagType StatusFlag)
{
    uint32 Reg = ((uint32)StatusFlag) >> UART_SHIFT;

    switch (StatusFlag)
    {
    /* These flags will be cleared by hardware */
    case UART_RX_DATA_READY:
    case UART_TX_DATA_NOT_FULL:
    case UART_TX_COMPLETE:
    case UART_TX_DATA_FULL:
    case UART_IDLE:
    case UART_CTS:
    case UART_RTS:
        break;

    default: /* Write "1" to cleared */
        if (UART_LSR0_REG_ID == Reg)
        {
            Base->LSR0 = (uint32)1U << ((uint16)StatusFlag & 0x000FU);
        }
        else /* UART_LSR1_REG_ID */
        {
            Base->LSR1 = (uint32)1U << ((uint16)StatusFlag & 0x000FU);
        }
        break;
    }
}

/*!
 * @brief Set UART data match function.
 *
 * @param [in] Base: UART base pointer
 * @param [in] Data: UART match data
 * @param [in] IsEnable: Enable or disable data match function
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetDataMatch(UART_Type *Base, uint16 Data, boolean IsEnable)
{
    if (TRUE == IsEnable)
    {
        WRITE_REG32(Base->DATA, (uint32)Data & UART_DATA_DATA_Msk);
        SET_BIT32(Base->MTCHCR, UART_MTCHCR_DTMTCH_EN_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->MTCHCR, UART_MTCHCR_DTMTCH_EN_Msk);
    }
}

/*!
 * @brief Set UART address filter function
 *
 * @param [in] BaseAddress: UART base pointer
 * @param [in] Addr: UART match address
 * @param [in] IsEnable: Enable or disable address filter function
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetAddrFilter(UART_Type *Base, uint16 Addr, boolean IsEnable)
{
    if (TRUE == IsEnable)
    {
        WRITE_REG32(Base->ADDR, (uint32)Addr & UART_ADDR_ADDR_Msk);
        SET_BIT32(Base->MTCHCR, UART_MTCHCR_ADDMTCH_EN_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->MTCHCR, UART_MTCHCR_ADDMTCH_EN_Msk);
    }
}

/*!
 * @brief Set UART hardware flow control function.
 *
 * @param [in] BaseAddress: UART base pointer
 * @param [in] RtsCts: Set UART hardware flow control function
 *                   -UART_RTSCTS_NONE: NO RTS or CTS
 *                   -UART_RTS_ONLY: RTS only
 *                   -UART_CTS_ONLY: CTS only
 *                   -UART_RTSCTS_ALL: Enable RTS and CTS
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetCtsRts(UART_Type *Base, Uart_RtsCtsType RtsCts)
{
    uint32 RegVal = Base->EFR;

    switch (RtsCts)
    {
    case UART_RTSCTS_NONE: /* NO RTS or CTS */
        RegVal &= ~UART_EFR_AUTO_RTS_Msk;
        RegVal &= ~UART_EFR_AUTO_CTS_Msk;
        break;

    case UART_RTS_ONLY: /* RTS only */
        RegVal |= UART_EFR_AUTO_RTS_Msk;
        RegVal &= ~UART_EFR_AUTO_CTS_Msk;
        break;

    case UART_CTS_ONLY: /* CTS only */
        RegVal &= ~UART_EFR_AUTO_RTS_Msk;
        RegVal |= UART_EFR_AUTO_CTS_Msk;
        break;

    case UART_RTSCTS_ALL: /* Enable RTS and CTS */
        RegVal |= UART_EFR_AUTO_RTS_Msk;
        RegVal |= UART_EFR_AUTO_CTS_Msk;
        break;

    default:
        /* Do nothing */
        break;
    }

    WRITE_REG32(Base->EFR, RegVal);
}

/*!
 * @brief Enable or disable RS485 function.
 *
 * @param [in] BaseAddress: UART base pointer
 * @param [in] Enable: Enable or disable RS485 function
 *                    -true: enable RS485 function
 *                    -false: disable RS485 function
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetRS485(UART_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->RS485CR, UART_RS485CR_RS485EN_Msk, UART_RS485CR_RS485EN_Pos, \
                 (TRUE == Enable) ? 1UL : 0UL);
}

/*!
 * @brief Set RS485 RTS polarity.
 *
 * @param [in] BaseAddress: UART base pointer
 * @param [in] Enable: Enable/disable/ UART RS485 RTS invert polarity
 *                    -true: enable RS485 INVPOL
 *                    -false: disable RS485 INVPOL
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetRS485Invpol(UART_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->RS485CR, UART_RS485CR_INVPOL_Msk, UART_RS485CR_INVPOL_Pos, (TRUE == Enable) ? 1UL : 0UL);
}

/*!
 * @brief Set RS485 delay function.
 *
 * @param [in] BaseAddress: UART base pointer
 * @param [in] Enable: enable or disable RS485 delay function
 *                    -true: enable RS485 delay
 *                    -false: disable RS485 delay
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetRS485Delay(UART_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->RS485CR, UART_RS485CR_DLYEN_Msk, UART_RS485CR_DLYEN_Pos, (TRUE == Enable) ? 1UL : 0UL);
}

/*!
 * @brief Set RS485 delay time.
 *
 * @param [in] BaseAddress: UART base pointer
 * @param [in] Counter: RS485 delay time[range 0~255]
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetRS485DelayCnt(UART_Type *Base, uint8 Counter)
{
    WRITE_REG32(Base->CNTR, Counter);
}

/*!
 * @brief Set RS484 Guart Time enable.
 *
 * @param[in] Base: UART base pointer
 * @param[in] enable: Enable/disable/ UART RS485  guard time
 * @return none
 */
LOCAL_INLINE void Uart_Reg_SetRS485GuardEnable(UART_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->GUARD, UART_GUARD_GUARD_EN_Msk, UART_GUARD_GUARD_EN_Pos, (TRUE == Enable) ? 1UL : 0UL);
}

/*!
 * @brief Set RS484 Guart Time.
 *
 * @param[in] base: UART base pointer
 * @param[in] guardTime: UART set guard time 0 to 15
 * @return none
 */
LOCAL_INLINE void Uart_Reg_SetRS485GuardTime(UART_Type *Base, uint8 GuardTime)
{
    MODIFY_REG32(Base->GUARD, UART_GUARD_GUARD_CNT_Msk, UART_GUARD_GUARD_CNT_Pos, GuardTime);
}

/*!
 * @brief Enable/disable UART IrDA function.
 *
 * @param [in] BaseAddress: UART base pointer
 * @param [in] Enable: Enable/disable UART IrDA function
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetIrDA(UART_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->LCR1, UART_LCR1_IREN_Msk, UART_LCR1_IREN_Pos, (TRUE == Enable) ? 1UL : 0UL);
}

/*!
 * @brief Set IrDA receiver minimum pulse width.
 *
 * @param [in] BaseAddress: UART base pointer
 * @param [in] Pulse: IrDA receiver minimum pulse width(range: 0~65535)
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetIrDARxWidth(UART_Type *Base, uint16 Pulse)
{
    WRITE_REG32(Base->PMIN0, (uint32)Pulse & UART_PMIN0_PMIN0_Msk);
    WRITE_REG32(Base->PMIN1, ((uint32)Pulse >> 8U) & UART_PMIN1_PMIN1_Msk);
}

/*!
 * @brief Set IrDA transmit minimum pulse width.
 *
 * @param [in] BaseAddress: UART base pointer
 * @param [in] Pulse: IrDA receiver minimum pulse width(range: 0~7)
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetIrDATxWidth(UART_Type *Base, uint8 Pulse)
{
    WRITE_REG32(Base->PWDTH, (uint32)Pulse & UART_PWDTH_PWDTH_Msk);
}

/*!
 * @brief UART inverse polarity for tx output.
 *
 * @param [in] Base: UART base pointer
 * @param [in] Enable: Enable or disable tx inverse function
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetInvTx(UART_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->LCR1, UART_LCR1_INVTX_Msk, UART_LCR1_INVTX_Pos, (TRUE == Enable) ? 1UL : 0UL);
}

/*!
 * @brief UART inverse polarity for the rx input.
 *
 * @param [in] Base: UART base pointer
 * @param [in] Enable: enable or disable rx inverse function
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetInvRx(UART_Type *Base, boolean Enable)
{
    MODIFY_REG32(Base->LCR1, UART_LCR1_INVRX_Msk, UART_LCR1_INVRX_Pos, (TRUE == Enable) ? 1UL : 0UL);
}

/*!
 * @brief Set UART single-wire mode.
 *
 * @param [in] Base: UART base pointer
 * @param [in] Enable: Enable or disable single-wire mode
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetSingleWire(UART_Type *Base, boolean Enable)
{
    if (TRUE == Enable)
    {
        SET_BIT32(Base->LCR1, UART_LCR1_RSRC_Msk);
        SET_BIT32(Base->LCR1, UART_LCR1_LOOP_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->LCR1, UART_LCR1_RSRC_Msk);
        CLEAR_BIT32(Base->LCR1, UART_LCR1_LOOP_Msk);
    }
}

/*!
 * @brief UART TX pin direction config in single-wire mode.
 *
 * @param [in] Base: UART base pointer
 * @param [in] Direction: Set tx pin direction in single-wire mode
 *                       - UART_TX_PIN_DIR_INPUT
 *                       - UART_TX_PIN_DIR_OUTPUT
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetTxDir(UART_Type *base, Uart_TxPinDirType Direction)
{
    MODIFY_REG32(base->LCR1, UART_LCR1_TXDIR_Msk, UART_LCR1_TXDIR_Pos, Direction);
}

/*!
 * @brief Set UART loop mode.
 *
 * @param [in] base: UART base pointer
 * @param [in] enable: Enable or disable loop mode
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetLoop(UART_Type *Base, boolean Enable)
{
    CLEAR_BIT32(Base->LCR1, UART_LCR1_RSRC_Msk);

    if (TRUE == Enable)
    {
        SET_BIT32(Base->LCR1, UART_LCR1_LOOP_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->LCR1, UART_LCR1_LOOP_Msk);
    }
}

/*!
 * @brief Get the value of Interrupt Enable Register (IER)
 *
 * @param [in] Base: UART base pointer
 * @return Returns the value of IER register
 */
LOCAL_INLINE uint32 Uart_Reg_GetIER(const UART_Type *Base)
{
    return Base->IER;
}

/*!
 * @brief Get the value of Match Control Register (MTCHCR)
 *
 * @param [in] Base: UART base pointer
 * @return Returns the value of MTCHCR register
 */
LOCAL_INLINE uint32 Uart_Reg_GetMTCHCR(const UART_Type *Base)
{
    return Base->MTCHCR;
}

/*!
 * @brief Get the value of Idle Register (IDLE)
 *
 * @param [in] Base: UART base pointer
 * @return Returns the value of IDLE register
 */
LOCAL_INLINE uint32 Uart_Reg_GetIDLE(const UART_Type *Base)
{
    return Base->IDLE;
}

/*!
 * @brief Set UART tx force data0.
 *
 * @param[in] Base: UART base pointer
 * @param[in] Enable: Enable or disable sout 0
 * @return void
 */
LOCAL_INLINE void Uart_Reg_SetSb(UART_Type *Base, boolean Enable)
{
    if (Enable == TRUE)
    {
        SET_BIT32(Base->LCR0, UART_LCR0_SB_Msk);
    }
    else
    {
        CLEAR_BIT32(Base->LCR0, UART_LCR0_SB_Msk);
    }
}

#if defined (AC7840E)
/*!
 * @brief Configures UART idle line check count.
 *
 * @param [in] Base: UART base pointer
 * @param [in] Value: idle line check count
 */
LOCAL_INLINE void Uart_Reg_SetIdleLineCount(UART_Type *Base, uint8 Value)
{
    MODIFY_REG32(Base->IDLELGH, UART_IDLELGH_IDLELGH_Msk, UART_IDLELGH_IDLELGH_Pos, Value);
}
#endif

#ifdef __cplusplus
}
#endif
#endif
/*============================================EOF==========================================*/
