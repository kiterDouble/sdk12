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
 * @file Spi_Hal.c
 *
 * @brief Spi driver hal api source file.
 *
 */
/*==============================================INCLUDE FILES=======================================*/
#include "AC784xx_Spi_Reg.h"
#include "Spi_Hal.h"
#include "Ckgen_Hal.h"
#include "Rcm_Hal.h"
#include "Dma_Hal.h"
#include "Core_Hal.h"
#include "OsIf_Time.h"

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/** @brief The timeout used to wait for TX/RX transmission to complete a frame.
*          For OsIf counter, it is recommended to use 1320(us) which is enough for 2 frames in fifo at 32bit 50k bps.
*          And in loop mode, there are some rough empirical value by counting test at 120MHz sys_clock:
*              0xFFF - 170us, 0x1FFF - 341us, 0x4FFF - 853us, 0x5FFF - 1.024ms, 0xFFFF - 2.73ms.
*/
#define SPI_TIMEOUT_FRAME   ((uint32)1320U)

/** @brief SPI DATA register access mask and pos when it converts to different fram size. */
#define SPI_1BYTE_MSK       (0xFFU)
#define SPI_2BYTES_MSK      (0xFFFFU)
#define SPI_BIT8_POS        (8U)
#define SPI_BIT16_POS       (16U)
#define SPI_BIT24_POS       (24U)

/*===================================================ENUMS==========================================*/
/*!
 * @brief Type of SPI transfer(tx or rx)
 * @note DES_SPI_TYP_001
 */
typedef enum
{
    SPI_HAL_TX = 0x00U,     /*!< Transmit only */
    SPI_HAL_RX = 0x01U,     /*!< Receive only */
    SPI_HAL_TXRX = 0x02U    /*!< Transmit and receive */
} Spi_TransceiveType;

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
/**
* @brief Base pointers for SPI instances.
* @note DES_SPI_VAR_008
*/
static SPI_Type *const Spi_HalBase[SPI_INSTANCE_MAX] = SPI_BASE_PTRS;

/**
* @brief Table to save SPI IRQ enumeration.
* @note DES_SPI_VAR_009
*/
static const IRQn_Type Spi_HalIrqId[SPI_INSTANCE_MAX] = SPI_IRQS;

/**
* @brief Table to save SPI soft resets.
* @note DES_SPI_VAR_010
*/
static const Rcm_ResetIDType Spi_HalClockReset[SPI_INSTANCE_MAX] =
{
    RCM_RESET_ID_SPI0,
    RCM_RESET_ID_SPI1,
    RCM_RESET_ID_SPI2,
#if defined (AC7842X) || defined (AC7843X)
    RCM_RESET_ID_SPI3,
#endif
#if defined (AC7843X)
    RCM_RESET_ID_SPI4,
#endif
};

/**
* @brief Table to save SPI bus clocks.
* @note DES_SPI_VAR_011
*/
static const Ckgen_BusClkIdType Spi_HalClock[SPI_INSTANCE_MAX] =
{
    CKGEN_SPI0_BUS_CLK,
    CKGEN_SPI1_BUS_CLK,
    CKGEN_SPI2_BUS_CLK,
#if defined (AC7842X) || defined (AC7843X)
    CKGEN_SPI3_BUS_CLK,
#endif
#if defined (AC7843X)
    CKGEN_SPI4_BUS_CLK,
#endif
};

/*PRQA S 3218 ++ # the function that occupy a large of stack space are allowed. */
/**
* @brief Table to save PCC clock source, for getting the input clcok frequency.
* @note DES_SPI_VAR_012
*/
static const Ckgen_ClkIdType SpiClock[SPI_INSTANCE_MAX] =
{
    CKGEN_SPI0_CLK,
    CKGEN_SPI1_CLK,
    CKGEN_SPI2_CLK,
#if defined (AC7842X) || defined (AC7843X)
    CKGEN_SPI3_CLK,
#endif
#if defined (AC7843X)
    CKGEN_SPI4_CLK,
#endif
};
/*PRQA S 3218 -- */

/**
* @brief Pointer to runtime state structure.
* @note DES_SPI_VAR_013
*/
#if (CONFIG_SPI0_ENABLE)
static Spi_StateType Spi0_HalStatePtr;
#endif

#if (CONFIG_SPI1_ENABLE)
static Spi_StateType Spi1_HalStatePtr;
#endif

#if (CONFIG_SPI2_ENABLE)
static Spi_StateType Spi2_HalStatePtr;
#endif

#if defined (AC7842X) || defined (AC7843X)
#if (CONFIG_SPI3_ENABLE)
static Spi_StateType Spi3_HalStatePtr;
#endif
#endif

#if defined (AC7843X)
#if (CONFIG_SPI4_ENABLE)
static Spi_StateType Spi4_HalStatePtr;
#endif
#endif

static Spi_StateType *const Spi_HalStatePtr[SPI_INSTANCE_MAX] =
{
#if (CONFIG_SPI0_ENABLE)
    &Spi0_HalStatePtr,
#else
    NULL_PTR,
#endif

#if (CONFIG_SPI1_ENABLE)
    &Spi1_HalStatePtr,
#else
    NULL_PTR,
#endif

#if (CONFIG_SPI2_ENABLE)
    &Spi2_HalStatePtr,
#else
    NULL_PTR,
#endif

#if defined (AC7842X) || defined (AC7843X)
#if (CONFIG_SPI3_ENABLE)
    &Spi3_HalStatePtr,
#else
    NULL_PTR,
#endif
#endif

#if defined (AC7843X)
#if (CONFIG_SPI4_ENABLE)
    &Spi4_HalStatePtr,
#else
    NULL_PTR,
#endif
#endif
};

/* PRQA S 3218 ++ # AMCE_SPI_3218.*/
/**
 * @brief table to spi config default value.
 * @note DES ID: DES_SPI_VAR_014
 */
static const Spi_HalConfigType Spi_DefaultConfig =
{
    .CsSetup = 5U,
    .CsHold = 5U,
    .CsIdle = 5U,
    .FrmSize = 8U,
    .TxDmaChannel = 0xFFU,
    .RxDmaChannel = 0xFFU,
    .MsbFirst = TRUE,
    .CsOutputEn = TRUE,
    .ContinuousCs = TRUE,
    .HreqEn = FALSE,
    .Mode = SPI_MASTER,
    .Cpol = SPI_CPOL_LOW,
    .Cpha = SPI_CPHA_0,
    .PcsCfg = SPI_PCS_0,
    .PcsPol = SPI_PCS_POLARITY_LOW,
    .HreqPol = SPI_HREQ_POLARITY_HIGH,
    .Width = SPI_DATA_WIDTH_1BIT,
    .PinCfg = SPI_SOUT_MOSI_SIN_MISO,
    .Callback = NULL_PTR,
    .BaudRate = 1000000U,
};
/* PRQA S 3218 -- # AMCE_SPI_3218.*/

/*============================================FUNCTION PROTOTYPES===================================*/
void SPI0_IRQHandler(void);
void SPI1_IRQHandler(void);
void SPI2_IRQHandler(void);
#if defined (AC7842X) || defined (AC7843X)
void SPI3_IRQHandler(void);
#endif
#if defined (AC7843X)
void SPI4_IRQHandler(void);
#endif

/**
* @brief This function waits for SPI status idle.
* @note DES_SPI_API_226
*
* @param[in] Base: Base pointers for SPI instance.
* @param[in] StatusFlag: Type of status to wait.
* @param[in] TimeOutValue: wait timeout value.
* @param[in] TimeOut: wait timeout or not.
*
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Operation successful
* @retval STATUS_TIMEOUT: Operation timed out
*
* @pre SPI module must be initialized
* @post SPI status is checked and waited for
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_226
*/
static Hal_StatusType Spi_Hal_WaitStatusTimeOut
(
    const SPI_Type *Base,
    Spi_StatusFlagType StatusFlag,
    uint32 TimeOutValue,
    boolean TimeOut
)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    uint32 Counter = 0U;

    /* Wait for module idle. */
    if (TRUE == TimeOut)
    {
        while ((FALSE == Spi_Reg_GetStatusFlag(Base, StatusFlag)) && (Counter < (TimeOutValue * 1000U)))
        {
            OsIf_UDelay(1);
            Counter++;
        }
        if (Counter >= (TimeOutValue * 1000U))
        {
            Ret = STATUS_TIMEOUT;
        }
    }
    else
    {
        while ((FALSE == Spi_Reg_GetStatusFlag(Base, StatusFlag)))
        {
            /* do nothing */
        }
    }

    return Ret;
}

/**
* @brief This function writes data to DATA register of the spcified module.
* @note DES_SPI_API_212
*
* @param[in] Base: Base pointers for SPI instance.
* @param[in] StatePtr: Pointer to runtime state structure for SPI instance.
*
* @return void
*
* @pre SPI module must be initialized
* @post Data is written to SPI DATA register
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_212
*/
static void Spi_Hal_WriteData(SPI_Type *Base, Spi_StateType *StatePtr)
{
    uint32 DataSend = 0U;
    uint16 TxCount;

    /* Save TX data to a temp value and update TX count. */
    TxCount = StatePtr->TxCount;
    if (StatePtr->FrmSize <= 8U)
    {
        /* Frame size is not greater than 8bits, data access 1 byte alignment. */
        if (NULL_PTR != StatePtr->TxBuffPtr) /* Get data from TX buffer. */
        {
            DataSend = (uint32)(StatePtr->TxBuffPtr[TxCount]);
        }
        TxCount++;
    }
    else if (StatePtr->FrmSize <= 16U)
    {
        /* Frame size is not greater than 16bits, data access 2 bytes alignment. */
        if (NULL_PTR != StatePtr->TxBuffPtr) /* Get data from TX buffer. */
        {
            DataSend = (uint32)(StatePtr->TxBuffPtr[TxCount]);
            DataSend |= ((uint32)(StatePtr->TxBuffPtr[TxCount + 1U]) << SPI_BIT8_POS);
        }
        TxCount += 2U;
    }
    else
    {
        /* Frame size is greater than 16bits, data access 4 bytes alignment. */
        if (NULL_PTR != StatePtr->TxBuffPtr) /* Get data from TX buffer. */
        {
            DataSend = (uint32)(StatePtr->TxBuffPtr[TxCount]);
            DataSend |= ((uint32)(StatePtr->TxBuffPtr[TxCount + 1U]) << SPI_BIT8_POS);
            DataSend |= ((uint32)(StatePtr->TxBuffPtr[TxCount + 2U]) << SPI_BIT16_POS);
            DataSend |= ((uint32)(StatePtr->TxBuffPtr[TxCount + 3U]) << SPI_BIT24_POS);
        }
        TxCount += 4U;
    }
    StatePtr->TxCount = TxCount;

    /* Write data into the TX data register. */
    Spi_Reg_WriteData(Base, DataSend);
}

/**
* @brief This function reads data from DATA register of the spcified module.
* @note DES_SPI_API_213
*
* @param[in] Base: Base pointers for SPI instance.
* @param[in] StatePtr: Pointer to runtime state structure for SPI instance.
*
* @return void
*
* @pre SPI module must be initialized
* @post Data is read from SPI DATA register
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_213
*/
static void Spi_Hal_ReadData(const SPI_Type *Base, Spi_StateType *StatePtr)
{
    uint32 DataRecv;
    uint16 RxCount;

    /* Read data from the RX data register. */
    DataRecv = Spi_Reg_ReadData(Base);
    /* Save data to RX buffer and update RX count. */
    RxCount = StatePtr->RxCount;
    if (StatePtr->FrmSize <= 8U)
    {
        if (NULL_PTR != StatePtr->RxBuffPtr)
        {
            /* Frame size is not greater than 8bits, data access 1 byte alignment. */
            StatePtr->RxBuffPtr[RxCount] = (uint8)(DataRecv & (uint32)SPI_1BYTE_MSK);
        }
        RxCount++;
    }
    else if (StatePtr->FrmSize <= 16U)
    {
        if (NULL_PTR != StatePtr->RxBuffPtr)
        {
            /* Frame size is not greater than 16bits, data access 2 bytes alignment. */
            StatePtr->RxBuffPtr[RxCount] = (uint8)(DataRecv & (uint32)SPI_1BYTE_MSK);
            StatePtr->RxBuffPtr[RxCount + 1U] = (uint8)((DataRecv >> SPI_BIT8_POS) & (uint32)SPI_1BYTE_MSK);
        }
        RxCount += 2U;
    }
    else
    {
        if (NULL_PTR != StatePtr->RxBuffPtr)
        {
            /* Frame size is greater than 16bits, data access 4 bytes alignment. */
            StatePtr->RxBuffPtr[RxCount] = (uint8)(DataRecv & (uint32)SPI_1BYTE_MSK);
            StatePtr->RxBuffPtr[RxCount + 1U] = (uint8)((DataRecv >> SPI_BIT8_POS) & (uint32)SPI_1BYTE_MSK);
            StatePtr->RxBuffPtr[RxCount + 2U] = (uint8)((DataRecv >> SPI_BIT16_POS) & (uint32)SPI_1BYTE_MSK);
            /*PRQA S 2985 ++ # AMCE_QAC_2985.*/
            StatePtr->RxBuffPtr[RxCount + 3U] = (uint8)((DataRecv >> SPI_BIT24_POS) & (uint32)SPI_1BYTE_MSK);
            /*PRQA S 2985 --*/
        }
        RxCount += 4U;
    }
    StatePtr->RxCount = RxCount;
}

/**
* @brief This function releases SPI Master CS to stop continuous selection of Slave.
* @note DES_SPI_API_214
*
* @param[in] Base: Base pointers for SPI instance.
*
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Operation successful
*
* @pre SPI module must be initialized and in master mode
* @post SPI CS is released
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_214
*/
static Hal_StatusType Spi_Hal_ReleaseCS(SPI_Type *Base)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    DEVICE_ASSERT(NULL_PTR != Base);

    Spi_Reg_ReleaseCS(Base);

    return Ret;
}

/**
* @brief This function waits for SPI module idle.
* @note DES_SPI_API_215
*
* @param[in] Base: Base pointers for SPI instance.
*
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Operation successful
* @retval STATUS_TIMEOUT: Operation timed out
*
* @pre SPI module must be initialized
* @post SPI module is waited to become idle
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_215
*/
static Hal_StatusType Spi_Hal_WaitIdle(const SPI_Type *Base)
{
    DEVICE_ASSERT(NULL_PTR != Base);

    return Spi_Hal_WaitStatusTimeOut(Base, SPI_MODULE_IDLE, SPI_TIMEOUT_FRAME, TRUE);
}

/**
* @brief This function clears SPI Tx under flow and Rx over flow status.
* @note DES_SPI_API_216
*
* @param[in] Base: Base pointers for SPI instance.
*
* @return Hal_StatusType
* @retval STATUS_SUCCESS: No error occurred
* @retval STATUS_ERROR: Error occurred and cleared
*
* @pre SPI module must be initialized
* @post SPI error flags are cleared
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_216
*/
static Hal_StatusType Spi_Hal_ClearTxUFRxOF(SPI_Type *Base)
{
    Hal_StatusType Ret = STATUS_SUCCESS;
    DEVICE_ASSERT(NULL_PTR != Base);

    /* Check tx error flag . */
    if (TRUE == Spi_Reg_GetStatusFlag(Base, SPI_TRANSMIT_ERROR))
    {
        Spi_Reg_ClearTxUF(Base);
        Ret = STATUS_ERROR;
    }

    /* Check rx error flag . */
    if (TRUE == Spi_Reg_GetStatusFlag(Base, SPI_RECEIVE_ERROR))
    {
        Spi_Reg_ClearRxOF(Base);
        Ret = STATUS_ERROR;
    }

    return Ret;
}

/**
* @brief Release SPI CS and disable interrupt after complete interrupt transfer.
* @note DES_SPI_API_217
*
* @param[in] Instance: SPI instance.
*
* @return void
*
* @pre SPI module must be initialized
* @post SPI CS is released and interrupts are disabled
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_217
*/
static void Spi_Hal_CompleteIrqTransfer(uint8 Instance)
{
    SPI_Type *Base;
    const Spi_StateType *StatePtr;

    if (Instance < SPI_INSTANCE_MAX)
    {
        DEVICE_ASSERT(NULL_PTR != Spi_HalBase[Instance]);
        DEVICE_ASSERT(NULL_PTR != Spi_HalStatePtr[Instance]);

        Base = Spi_HalBase[Instance];
        StatePtr = Spi_HalStatePtr[Instance];

        if (TRUE == Spi_Reg_IsMaster(Base))
        {
            (void)Spi_Hal_ReleaseCS(Base); /* Release SPI CS to inactive. */
        }

        if (SPI_USING_DMA == StatePtr->TransferType)
        {
            if (TRUE == Spi_Reg_GetTxOnly(Base))
            {
                (void)Dma_Hal_StopCh(StatePtr->TxDmaChannel); /* Stop TX DMA channel. */
            }
            else
            {
                (void)Dma_Hal_StopCh(StatePtr->TxDmaChannel); /* Stop TX DMA channel. */
                (void)Dma_Hal_StopCh(StatePtr->RxDmaChannel); /* Stop RX DMA channel. */
            }
        }

        Spi_Reg_SetIntMode(Base, SPI_INT_TX_DATA, FALSE); /* Disable TXEIE. */
        Spi_Reg_SetIntMode(Base, SPI_INT_RX_DATA, FALSE); /* Disable RXFIE. */
        Spi_Reg_SetIntMode(Base, SPI_INT_RECEIVE_ERROR, FALSE); /* Disable RX overrun interrupt. */

        /* Calling user callback function. */
        if (NULL_PTR != StatePtr->Callback)
        {
            StatePtr->Callback(Instance, StatePtr->Status);
        }
    }
}

/**
* @brief Bottom half of interrupt handler for SPI, which will update the state
*        stored in the state structs to transfer data.This is not a
*        public API as it is called whenever an interrupt occurs.
* @note DES_SPI_API_218
*
* @param[in] Instance: SPI instance.
*
* @return void
*
* @pre SPI module must be initialized
* @post SPI transfer state is updated
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: Callable from ISR
*     DES ID: DES_SPI_API_218
*/
static void Spi_Hal_IRQHandlerBottom(uint8 Instance)
{
    SPI_Type *Base;
    Spi_StateType *StatePtr;
    uint8 RxCount;
#if defined (AC7840X)
    uint32 timeOut = SPI_TIMEOUT_FRAME;
#endif
    boolean RxOnlyTranComplete = FALSE;

    if (Instance < SPI_INSTANCE_MAX)
    {
        Base = Spi_HalBase[Instance];
        StatePtr = Spi_HalStatePtr[Instance];
        DEVICE_ASSERT(NULL_PTR != Base);
        DEVICE_ASSERT(NULL_PTR != StatePtr);

        /* Check data match flag and int */
        /* PRQA S 3415 ++ # AMCE_SPI_3415*/
        if ((TRUE == Spi_Reg_GetStatusFlag(Base, SPI_DATA_MATCH)) &&
                (TRUE == Spi_Reg_GetIntMode(Base, SPI_INT_DATA_MATCH)))
        /* PRQA S 3415 -- */
        {
            Spi_Reg_ClearRDMF(Base);
            /* PRQA S 2812 ++ # AMCE_SPI_2812.*/
            StatePtr->Status = SPI_STATUS_DATA_MATCH_MASK;
            /* PRQA S 2812 -- */
            /* Calling user callback function. */
            if (NULL_PTR != StatePtr->Callback)
            {
                StatePtr->Callback(Instance, StatePtr->Status);
            }
        }

        /* Check read buffer only if there are remaining bytes to read. */
        /* PRQA S 3415 ++ # AMCE_SPI_3415 */
        while ((TRUE == Spi_Reg_GetStatusFlag(Base, SPI_RX_DATA_FLAG)) &&
                (TRUE == Spi_Reg_GetIntMode(Base, SPI_INT_RX_DATA))) /* RX data not empty. */
            /* PRQA S 3415 -- */
        {
            if (TRUE == Spi_Reg_GetRxOnly(Base))
            {
                if (StatePtr->FrmSize <= 8U)
                {
                    RxCount = 1U;
                }
                else if (StatePtr->FrmSize <= 16U)
                {
                    RxCount = 2U;
                }
                else
                {
                    RxCount = 4U;
                }

                /* In Rx only mode,the last data need disable rxonly to disable clock */
                if (StatePtr->RxCount == (StatePtr->XferCount - RxCount))
                {
                    RxOnlyTranComplete = TRUE;
#if defined (AC7840X)
                    Spi_Reg_SetRxOnly(Base, FALSE);
#endif
                }
#if defined (AC7840X)
                /* when baud is low,rx only need get busy before rxff,hw limit */
                while ((TRUE == Spi_Reg_GetStatusFlag(Base, SPI_MASTER_BUSY)) && (timeOut > 0U))
                {
                    timeOut--;
                }
#endif
            }

            if (StatePtr->RxCount < StatePtr->XferCount)
            {
                Spi_Hal_ReadData(Base, StatePtr);
            }

            /* Data has been read in. */
            if (StatePtr->RxCount >= StatePtr->XferCount)
            {
                StatePtr->Status = SPI_STATUS_RX_FINISH_MASK;
            }
        }

        /* Handle Transmit data. */
        /* PRQA S 3415 ++ # AMCE_SPI_3415 */
        if ((TRUE == Spi_Reg_GetStatusFlag(Base, SPI_TX_DATA_FLAG)) &&
                (TRUE == Spi_Reg_GetIntMode(Base, SPI_INT_TX_DATA))) /* TX data not full. */
        /* PRQA S 3415 -- */
        {
            if (StatePtr->TxCount < StatePtr->XferCount) /* Data is being sent, wait for next TXEF. */
            {
                Spi_Hal_WriteData(Base, StatePtr);
            }

            if (StatePtr->TxCount >= StatePtr->XferCount)
            {
                Spi_Reg_SetIntMode(Base, SPI_INT_TX_DATA, FALSE); /* Disable TXEIE. */
            }
        }

        /* Check if we're done with this transfer. */
        if ((TRUE == RxOnlyTranComplete) || (StatePtr->TxCount == StatePtr->XferCount))
        {
            if ((TRUE == Spi_Reg_GetTxOnly(Base)) || (StatePtr->RxCount == StatePtr->XferCount))
            {
                StatePtr->Status = SPI_STATUS_TX_FINISH_MASK;
                Spi_Hal_CompleteIrqTransfer(Instance);
            }
        }
    }
}

/**
* @brief Interrupt handler for SPI, which will update the state
*        stored in the state structs to transfer data.This is not a
*        public API as it is called whenever an interrupt occurs.
* @note DES_SPI_API_219
*
* @param[in] Instance: SPI instance.
*
* @return void
*
* @pre SPI module must be initialized
* @post SPI interrupt is handled
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: Callable from ISR
*     DES ID: DES_SPI_API_219
*/
static void Spi_Hal_IRQHandler(uint8 Instance)
{
    SPI_Type *Base;
    Spi_StateType *StatePtr;

    if (Instance < SPI_INSTANCE_MAX)
    {
        Base = Spi_HalBase[Instance];
        StatePtr = Spi_HalStatePtr[Instance];

        DEVICE_ASSERT(NULL_PTR != Base);
        DEVICE_ASSERT(NULL_PTR != StatePtr);

        /* PRQA S 2812 ++ # AMCE_SPI_2812.*/
        StatePtr->InProgress = TRUE; /* Update transfer status. */
        /* PRQA S 2812 -- */

        /* If an error is detected the transfer will be aborted. */
        if (TRUE == Spi_Reg_GetStatusFlag(Base, SPI_TRANSMIT_ERROR)) /* TX underrun */
        {
            if (TRUE == Spi_Reg_GetIntMode(Base, SPI_INT_TRANSMIT_ERROR))
            {
                Spi_Reg_ClearTxUF(Base);
                StatePtr->Status = SPI_STATUS_TX_UNDERFLOW_MASK;
                Spi_Hal_CompleteIrqTransfer(Instance);
            }
        }
        else if (TRUE == Spi_Reg_GetStatusFlag(Base, SPI_RECEIVE_ERROR))
        {
            if (TRUE == Spi_Reg_GetIntMode(Base, SPI_INT_RECEIVE_ERROR)) /* RX overrun */
            {
                Spi_Reg_ClearRxOF(Base);
                StatePtr->Status = SPI_STATUS_RX_OVERFLOW_MASK;
                Spi_Hal_CompleteIrqTransfer(Instance);
            }
        }
        else
        {
            Spi_Hal_IRQHandlerBottom(Instance); /* Check TXEF & RXFF when there is no error. */
        }

        StatePtr->InProgress = FALSE; /* The transfer is complete. */
    }
}

/**
* @brief DMA callback handler for SPI, which will update the state
*        stored in the state structs to transfer data.This is not a
*        public API as it is called whenever a DMA transfer end.
* @note DES_SPI_API_220
*
* @param[in] Params: Callback parameters.
*
* @return void
*
* @pre SPI module must be initialized
* @post SPI DMA transfer is handled
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: Callable from ISR
*     DES ID: DES_SPI_API_220
*/
static void Spi_Hal_DMAHandler
(
    /*PRQA S 3673 ++ # AMCE_SPI_3673.*/
    void *Params
    /*PRQA S 3673 -- */
)
{
    SPI_Type *Base;
    Spi_StateType *StatePtr;
    DEVICE_ASSERT(NULL_PTR != Params);
    /*PRQA S 0316 ++ # AMCE_QAC_0316.*/
    const Dma_ChannelCBInfoType *DmaInfo = (Dma_ChannelCBInfoType *)Params;
    /*PRQA S 0316 -- */
    /*PRQA S 0326 ++ # AMCE_SPI_0326.*/
    uint32 Instance = (uint32)(DmaInfo->UserArgs);
    /*PRQA S 0326 -- */

    if (Instance < SPI_INSTANCE_MAX)
    {
        Base = Spi_HalBase[Instance];
        StatePtr = Spi_HalStatePtr[Instance];
        DEVICE_ASSERT(NULL_PTR != Base);
        DEVICE_ASSERT(NULL_PTR != StatePtr);

        /* PRQA S 2812 ++ # AMCE_SPI_2812.*/
        StatePtr->InProgress = TRUE; /* Update transfer status. */
        /* PRQA S 2812 -- */
        if (TRUE == Spi_Reg_IsMaster(Base))
        {
            (void)Spi_Hal_ReleaseCS(Base); /* Release SPI CS to inactive. */
        }

        Spi_Reg_SetTxOnly(Base, FALSE); /* Disable Tx only mode. */
        Spi_Reg_SetRxOnly(Base, FALSE); /* Disable Rx only mode. */
        Spi_Reg_SetIntMode(Base, SPI_INT_RECEIVE_ERROR, FALSE); /* Disable RX overrun interrupt. */
        Spi_Reg_SetTxDmaCmd(Base, FALSE); /* SPI Transmit Data DMA request */
        Spi_Reg_SetRxDmaCmd(Base, FALSE); /* SPI Receive Data DMA request */

        if (DmaInfo->ChannelId == StatePtr->TxDmaChannel)
        {
            (void)Dma_Hal_StopCh(StatePtr->TxDmaChannel); /* Stop TX DMA channel. */
            if (0U != (DmaInfo->DmaEvent & DMA_FINISH_EVENT))
            {
                StatePtr->Status = SPI_STATUS_TX_FINISH_MASK;
            }
            else
            {
                StatePtr->Status = SPI_STATUS_DMA_ERROR_MASK;
            }
        }
        else
        {
            (void)Dma_Hal_StopCh(StatePtr->TxDmaChannel); /* Stop TX DMA channel. */
            (void)Dma_Hal_StopCh(StatePtr->RxDmaChannel); /* Stop RX DMA channel. */
            if (0U != (DmaInfo->DmaEvent & DMA_FINISH_EVENT))
            {
                StatePtr->Status = SPI_STATUS_RX_FINISH_MASK;
            }
            else
            {
                StatePtr->Status = SPI_STATUS_DMA_ERROR_MASK;
            }
        }

        /* Calling user callback function. */
        if (NULL_PTR != StatePtr->Callback)
        {
            StatePtr->Callback((uint8)Instance, StatePtr->Status);
        }

        StatePtr->InProgress = FALSE; /* The transfer is complete. */
    }
}

/**
* @brief SPI transmission,reception by DMA.
* @note DES_SPI_API_227
*
* @param[in] Instance: SPI instance.
* @param[in] TxBuffPtr: The pointer to the data buffer of the data to send.
* @param[inout] RxBuffPtr: Pointer to the buffer where the received bytes are stored.
* @param[in] ByteCount: The number of bytes to send and receive.
* @param[in] TransType: Transfer Type,TX or RX.
*
* @return void
*
* @pre SPI module must be initialized
* @post SPI DMA transfer is configured and started
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_227
*/
static void Spi_Hal_SetDMAChannel
(
    uint8 Instance,
    const uint8 *TxBuffPtr,
    const uint8 *RxBuffPtr,
    uint16 ByteCount,
    Spi_TransceiveType TransType
)
{
    SPI_Type *Base;
    const Spi_StateType *StatePtr;
    Dma_TransferConfigType DmaTransferConfig;
    Dma_TransferUnitType DmaTransferUint;

    if (Instance < SPI_INSTANCE_MAX)
    {
        DEVICE_ASSERT(NULL_PTR != Spi_HalBase[Instance]);
        DEVICE_ASSERT(NULL_PTR != Spi_HalStatePtr[Instance]);

        Base = Spi_HalBase[Instance];
        StatePtr = Spi_HalStatePtr[Instance];

        if (StatePtr->FrmSize <= 8U)
        {
            /* Frame size is not greater than 8bits, data access 1 byte alignment. */
            DmaTransferUint = DMA_TRANSFER_UNIT_1B;
        }
        else if (StatePtr->FrmSize <= 16U)
        {
            /* Frame size is not greater than 16bits, data access 2 bytes alignment. */
            DmaTransferUint = DMA_TRANSFER_UNIT_2B;
        }
        else
        {
            /* Frame size is greater than 16bits, data access 4 bytes alignment. */
            DmaTransferUint = DMA_TRANSFER_UNIT_4B;
        }

        DmaTransferConfig.SrcUnit = DmaTransferUint;
        DmaTransferConfig.DestUnit = DmaTransferUint;
        DmaTransferConfig.Length = ByteCount;
        DmaTransferConfig.CircularMode = FALSE;
        DmaTransferConfig.TriggerMode = FALSE;
#if defined (AC7840E)
        DmaTransferConfig.DaisyMode = FALSE;
#endif
        DmaTransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
        DmaTransferConfig.Callback = Spi_Hal_DMAHandler;
        /*PRQA S 0326 ++ # AMCE_SPI_0326.*/
        DmaTransferConfig.UserArgs = (void *)(uint32)Instance;
        /*PRQA S 0326 -- */
        if (SPI_HAL_TX == TransType)
        {
            DmaTransferConfig.Type = DMA_TRANSFER_MEM2PERIPH;
            DmaTransferConfig.DestStartAddr = Spi_Reg_GetDataAddress(Base);
            DmaTransferConfig.SrcStartAddr = (uint32)TxBuffPtr;
            DmaTransferConfig.SrcOffset = (uint16)(1UL << ((uint16)DmaTransferUint));
            DmaTransferConfig.DestOffset = 0U;
            /* PRQA S 2985 ++ # AMCE_QAC_2985 */
            DmaTransferConfig.SrcEndAddr = DmaTransferConfig.SrcStartAddr + (uint16)(ByteCount / \
                                           (uint8)(1U << (uint8)DmaTransferConfig.SrcUnit) * \
                                           DmaTransferConfig.SrcOffset);
            DmaTransferConfig.DestEndAddr = DmaTransferConfig.DestStartAddr + (uint16)(ByteCount / \
                                            (uint8)(1U << (uint8)DmaTransferConfig.DestUnit) * \
                                            DmaTransferConfig.DestOffset);
            /* PRQA S 2985 -- */
            (void)Dma_Hal_ConfigCh(StatePtr->TxDmaChannel, &DmaTransferConfig);
            (void)Dma_Hal_StartCh(StatePtr->TxDmaChannel); /* Start TX DMA channel. */
            Spi_Reg_SetTxDmaCmd(Base, TRUE); /* SPI Transmit Data DMA request */
        }
        else
        {
            /*rx dma:rx only or txrx config rx dma*/
            DmaTransferConfig.Length = ByteCount;
            DmaTransferConfig.Type = DMA_TRANSFER_PERIPH2MEM;
            DmaTransferConfig.DestStartAddr = (uint32)RxBuffPtr;
            DmaTransferConfig.SrcStartAddr = Spi_Reg_GetDataAddress(Base);
            DmaTransferConfig.SrcOffset = 0U;
            DmaTransferConfig.DestOffset = (uint8)(1U << ((uint16)DmaTransferUint));
            /* PRQA S 2985 ++ # AMCE_QAC_2985 */
            DmaTransferConfig.SrcEndAddr = DmaTransferConfig.SrcStartAddr + (uint16)(ByteCount / \
                                           (uint8)(1U << (uint8)DmaTransferConfig.SrcUnit) * \
                                           DmaTransferConfig.SrcOffset);
            DmaTransferConfig.DestEndAddr = DmaTransferConfig.DestStartAddr + (uint16)(ByteCount / \
                                            (uint8)(1U << (uint8)DmaTransferConfig.DestUnit) * \
                                            DmaTransferConfig.DestOffset);
            /* PRQA S 2985 -- */
            (void)Dma_Hal_ConfigCh(StatePtr->RxDmaChannel, &DmaTransferConfig);

            /*tx dma:when rx only,tx dma send invalid data use rxbuffer because of dma must use usram*/
            if (SPI_HAL_RX == TransType)
            {
                DmaTransferConfig.SrcStartAddr = (uint32)RxBuffPtr;
                DmaTransferConfig.SrcOffset = 0U;
            }
            else
            {
                DmaTransferConfig.SrcStartAddr = (uint32)TxBuffPtr;
                DmaTransferConfig.SrcOffset = (uint8)(1U << ((uint16)DmaTransferUint));
            }
            DmaTransferConfig.Type = DMA_TRANSFER_MEM2PERIPH;
            DmaTransferConfig.DestStartAddr = Spi_Reg_GetDataAddress(Base);
            DmaTransferConfig.DestOffset = 0U;
            DmaTransferConfig.Callback = NULL_PTR;
            /* PRQA S 2985 ++ # AMCE_QAC_2985 */
            DmaTransferConfig.SrcEndAddr = DmaTransferConfig.SrcStartAddr + (uint16)(ByteCount / \
                                           (uint8)(1U << (uint8)DmaTransferConfig.SrcUnit) * \
                                           DmaTransferConfig.SrcOffset);
            DmaTransferConfig.DestEndAddr = DmaTransferConfig.DestStartAddr + (uint16)(ByteCount / \
                                            (uint8)(1U << (uint8)DmaTransferConfig.DestUnit) * \
                                            DmaTransferConfig.DestOffset);
            /* PRQA S 2985 -- */
            (void)Dma_Hal_ConfigCh(StatePtr->TxDmaChannel, &DmaTransferConfig);

            (void)Dma_Hal_StartCh(StatePtr->RxDmaChannel); /* Start RX DMA channel. */
            (void)Dma_Hal_StartCh(StatePtr->TxDmaChannel); /* Start TX DMA channel. */
            Spi_Reg_SetRxDmaCmd(Base, TRUE); /* SPI Receive Data DMA request */
            Spi_Reg_SetTxDmaCmd(Base, TRUE); /* SPI Transmit Data DMA request */
        }
    }
}

/**
* @brief This function transfers by polling mode, which will not return untill timeout or transfer finish.
* @note DES_SPI_API_228
*
* @param[in] Instance: SPI instance.
* @param[in] ByteCount: The number of bytes to send and receive.
* @param[in] Timeout: A timeout count value for the transfer. If the transfer takes longer than
*                     this amount of time, the transfer is aborted and a STATUS_TIMEOUT error returned.
* @param[in] TransType: Transfer Type,TX or RX.
*
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Operation successful
* @retval STATUS_TIMEOUT: Operation timed out
* @retval STATUS_ERROR: Operation failed
*
* @pre SPI module must be initialized
* @post SPI transfer is completed or timed out
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_228
*/
static Hal_StatusType Spi_Hal_PollData
(
    uint8 Instance,
    uint16 ByteCount,
    uint32 Timeout,
    Spi_TransceiveType TransType
)
{
    SPI_Type *Base;
    Spi_StateType *StatePtr;
#if defined (AC7840X)
    uint32 Ticks;
    uint32 Counter;
    uint32 Elapse;
    uint8 RxCount;
#endif
    Hal_StatusType Ret = STATUS_ERROR;

    if (Instance < SPI_INSTANCE_MAX)
    {
        Base = Spi_HalBase[Instance];
        StatePtr = Spi_HalStatePtr[Instance];
#if defined (AC7840X)
        Ticks = OsIf_MicrosToTicks(Timeout);
#endif

        if (SPI_HAL_TX == TransType)
        {
            while (StatePtr->TxCount < ByteCount)
            {
                Ret = Spi_Hal_WaitStatusTimeOut(Base, SPI_TX_DATA_FLAG, Timeout, TRUE);

                if (STATUS_SUCCESS == Ret)
                {
                    Spi_Hal_WriteData(Base, StatePtr);
                }
                else
                {
                    Ret = STATUS_TIMEOUT;
                    StatePtr->Status = SPI_STATUS_TIMEOUT_MASK;
                    break;
                }
                StatePtr->Status = SPI_STATUS_TX_FINISH_MASK;
            }
        }
        else if (SPI_HAL_RX == TransType)
        {
            while (StatePtr->RxCount < ByteCount)
            {
#if defined (AC7840X)
                /* Check whether data was received. */
                Counter = OsIf_GetCounter();
                Elapse = 0U;
                /* when baud is low,rx only need get busy before rxff,hw limit*/
                if (TRUE == Spi_Reg_IsMaster(Base))
                {
                    while ((TRUE == Spi_Reg_GetStatusFlag(Base, SPI_MASTER_BUSY)) && (Elapse < Ticks))
                    {
                        Elapse = OsIf_GetElapsed(&Counter);
                    }
                }
#endif
                Ret = Spi_Hal_WaitStatusTimeOut(Base, SPI_RX_DATA_FLAG, Timeout, TRUE);
#if defined (AC7840X)
                if (StatePtr->FrmSize <= 8U)
                {
                    RxCount = 1U;
                }
                else if (StatePtr->FrmSize <= 16U)
                {
                    RxCount = 2U;
                }
                else
                {
                    RxCount = 4U;
                }

                /* In Rx only mode,the last data need disable rxonly to disable clock */
                if (StatePtr->RxCount == (StatePtr->XferCount - RxCount))
                {
                    Spi_Reg_SetRxOnly(Base, FALSE);
                }
#endif
                if (STATUS_SUCCESS == Ret)
                {
                    Spi_Hal_ReadData(Base, StatePtr);
                }
                else
                {
                    Ret = STATUS_TIMEOUT;
                    StatePtr->Status = SPI_STATUS_TIMEOUT_MASK;
                    break;
                }
                StatePtr->Status = SPI_STATUS_RX_FINISH_MASK;
            }
        }
        else
        {
            while (StatePtr->RxCount < ByteCount)
            {
                Ret = Spi_Hal_WaitStatusTimeOut(Base, SPI_TX_DATA_FLAG, Timeout, TRUE);

                if (STATUS_SUCCESS == Ret)
                {
                    Spi_Hal_WriteData(Base, StatePtr);
                }
                else
                {
                    StatePtr->Status = SPI_STATUS_TIMEOUT_MASK;
                }

                Ret = Spi_Hal_WaitStatusTimeOut(Base, SPI_RX_DATA_FLAG, Timeout, TRUE);

                if (STATUS_SUCCESS == Ret)
                {
                    Spi_Hal_ReadData(Base, StatePtr);
                }
                else
                {
                    Ret = STATUS_TIMEOUT;
                    StatePtr->Status = SPI_STATUS_TIMEOUT_MASK;
                    break;
                }
                StatePtr->Status = SPI_STATUS_TX_FINISH_MASK;
            }
        }
    }

    return Ret;
}

#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
/**
* @brief This function get rx count.
* @note DES_SPI_API_229
*
* @param[in] Instance: SPI instance.
* @param[in] ByteCount: The number of bytes to send and receive.
*
* @return uint16: The rx count.
*
* @pre SPI module must be initialized
* @post RX count is calculated
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_229
*/
static uint16 Spi_Hal_GetRXCount(uint8 Instance, uint16 ByteCount)
{
    uint16 Count = 0U;
    Spi_StateType const *StatePtr;

    if (Instance < SPI_INSTANCE_MAX)
    {
        StatePtr = Spi_HalStatePtr[Instance];
        if (StatePtr->FrmSize <= 8U)
        {
            /* Frame size is not greater than 8bits, data access 1 byte alignment. */
            Count = ByteCount;
        }
        else if (StatePtr->FrmSize <= 16U)
        {
            /* PRQA S 1860 ++ # AMCE_SPI_1860 */
            DEVICE_ASSERT((uint16)(ByteCount % 2U) == 0U);
            /* PRQA S 1860 -- */
            /* Frame size is not greater than 16bits, data access 2 bytes alignment. */
            Count = ByteCount / 2U;
        }
        else
        {
            /* PRQA S 1860 ++ # AMCE_SPI_1860*/
            DEVICE_ASSERT((uint16)(ByteCount % 4U) == 0U);
            /* PRQA S 1860 -- */
            /* Frame size is greater than 16bits, data access 4 bytes alignment. */
            Count = ByteCount / 4U;
        }
    }

    return Count;
}
#endif

/**
* @brief This function initializes a SPI instance.
* @note DES_SPI_API_232
*
* @param[in] Instance: SPI instance.
* @param[in] ConfigPtr: The data structure containing information about a device on the SPI bus.
*
* @return void
*
* @pre None
* @post SPI bus is configured
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_232
*/
void Spi_Hal_ConfigureBus(uint8 Instance, const Spi_HalConfigType *ConfigPtr)
{
    SPI_Type *Base;
    Spi_StateType *StatePtr;

    if ((Instance < SPI_INSTANCE_MAX) && (NULL_PTR != ConfigPtr))
    {
        DEVICE_ASSERT((ConfigPtr->FrmSize >= 4U) && (ConfigPtr->FrmSize <= 32U));

        Base = Spi_HalBase[Instance];
        StatePtr = Spi_HalStatePtr[Instance];

        /* Configure internal state structure for SPI. */
        StatePtr->TxBuffPtr = NULL_PTR;
        StatePtr->RxBuffPtr = NULL_PTR;
        StatePtr->TxCount = 0U;
        StatePtr->RxCount = 0U;
        StatePtr->XferCount = 0U;
        StatePtr->Status = SPI_STATUS_NONE;
        StatePtr->InProgress = FALSE;
        StatePtr->Pcs = ConfigPtr->PcsCfg;
        StatePtr->FrmSize = ConfigPtr->FrmSize;
        StatePtr->MsbFirst = ConfigPtr->MsbFirst;
        StatePtr->Callback = ConfigPtr->Callback;
        StatePtr->TxDmaChannel = ConfigPtr->TxDmaChannel;
        StatePtr->RxDmaChannel = ConfigPtr->RxDmaChannel;

        Spi_Reg_SetMasterSlaveMode(Base, ConfigPtr->Mode); /* Select master or slave mode */
        Spi_Reg_SetCSSetup(Base, ConfigPtr->CsSetup); /* Set CS_SETUP time */
        Spi_Reg_SetCSHold(Base, ConfigPtr->CsHold); /* Set CS_HOLD time */
        Spi_Reg_SetCSIdle(Base, ConfigPtr->CsIdle); /* Set CS_IDLE time */
        Spi_Reg_SetCPOL(Base, ConfigPtr->Cpol); /* Set CLK polarity */
        Spi_Reg_SetCPHA(Base, ConfigPtr->Cpha); /* Set CLK phase */
        Spi_Reg_SetFrameSize(Base, ConfigPtr->FrmSize); /* Set bits per frame */
        Spi_Reg_SetTxMSB(Base, ConfigPtr->MsbFirst); /* Set TX MSB or LSB mode */
        Spi_Reg_SetRxMSB(Base, ConfigPtr->MsbFirst); /* Set RX MSB or LSB mode */
        Spi_Reg_SetDataWidth(Base, ConfigPtr->Width); /* Data line width 1 bit, standard SPI */
        Spi_Reg_SetCSOE(Base, ConfigPtr->CsOutputEn); /* Set CS output enabled or not */
        Spi_Reg_SetContinuousCS(Base, ConfigPtr->ContinuousCs); /* Set CS continuous or not */
        Spi_Reg_SetPcsCfg(Base, ConfigPtr->PcsCfg); /* Select which CS to use */
        Spi_Reg_SetPcsPolarity(Base, ConfigPtr->PcsCfg, ConfigPtr->PcsPol); /* Set the desired PCS polarity */
        Spi_Reg_SetPinConfigMode(Base, ConfigPtr->PinCfg); /* Set Pin (SOUT, SIN) configuration */
        Spi_Reg_SetHreq(Base, ConfigPtr->HreqEn); /* Host request */
        Spi_Reg_SetHreqPolarity(Base, ConfigPtr->HreqPol); /* Host request polarity */
        (void)Spi_Hal_SetBaudRate(Instance, ConfigPtr->BaudRate);

        /* Reset SPI status, but confifuration still valid. */
        Spi_Reg_SoftwareReset(Base);
    }
}

/**
* @brief This function configures the SPI frame parameters.
* @note DES_SPI_API_204
*
* @param[in] Instance: SPI instance.
* @param[in] FrmSize: Number of bits per frame, support 4~32 bits.
* @param[in] MsbFirst: Option to transmit MSB first.
*
* @return void
*
* @pre SPI module must be initialized
* @post SPI frame parameters are configured
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_204
*/
void Spi_Hal_ConfigureFrame(uint8 Instance, uint8 FrmSize, boolean MsbFirst)
{
    SPI_Type *Base;
    Spi_StateType *StatePtr;

    if (Instance < SPI_INSTANCE_MAX)
    {
        DEVICE_ASSERT(NULL_PTR != Spi_HalStatePtr[Instance]);
        DEVICE_ASSERT((FrmSize >= 4U) && (FrmSize <= 32U));

        Base = Spi_HalBase[Instance];
        StatePtr = Spi_HalStatePtr[Instance];
        StatePtr->FrmSize = FrmSize;
        StatePtr->MsbFirst = MsbFirst;
        Spi_Reg_SetFrameSize(Base, FrmSize); /* Set bits per frame */
        Spi_Reg_SetTxMSB(Base, MsbFirst); /* Set TX MSB or LSB mode */
        Spi_Reg_SetRxMSB(Base, MsbFirst); /* Set RX MSB or LSB mode */
    }
}

/**
* @brief This function initializes a SPI instance.
* @note DES_SPI_API_200
*
* @param[in] Instance: SPI instance.
* @param[in] ConfigPtr: The data structure containing information about a device on the SPI bus.
*
* @return None
*
* @pre None
* @post SPI module is initialized
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_200
*/
void Spi_Hal_Init(uint8 Instance, const Spi_HalConfigType *ConfigPtr)
{
    SPI_Type *Base;
    Hal_StatusType Ret;

    if (Instance < SPI_INSTANCE_MAX)
    {
        DEVICE_ASSERT(NULL_PTR != ConfigPtr);
        Base = Spi_HalBase[Instance];
        /* Enable SPI bus clock and release module from reset state. */
        Ret = Ckgen_Hal_EnablePeriphClk(Spi_HalClock[Instance], TRUE);
        DEVICE_ASSERT(STATUS_ERROR != Ret);
        (void)Ret;
        Rcm_Hal_SetResetState(Spi_HalClockReset[Instance], RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState(Spi_HalClockReset[Instance], RCM_RESET_STATE_DEASSERT);
        /* Configure bus for this device.*/
        Spi_Hal_ConfigureBus(Instance, ConfigPtr);
        Spi_Reg_SetModeFault(Base, FALSE); /* CS mode fault detect */
        Spi_Reg_SetIntMode(Base, SPI_INT_MODE_FAULT_ERROR, FALSE); /* CS mode fault error interrupt */
        Spi_Reg_SetIntMode(Base, SPI_INT_TRANSMIT_ERROR, TRUE); /* Transmit Error interrupt (TX underrun) */
        Spi_Reg_SetIntMode(Base, SPI_INT_RECEIVE_ERROR, FALSE); /* Receive Error interrupt (RX overrun) */
        Spi_Reg_SetIntMode(Base, SPI_INT_DATA_MATCH, FALSE); /* Data match interrupt */
        Spi_Reg_SetRxDmaCmd(Base, FALSE); /* SPI Receive Data DMA request */
        Spi_Reg_SetTxDmaCmd(Base, FALSE); /* SPI Transmit Data DMA request */
        Spi_Reg_SetDebug(Base, FALSE); /* Debug mode */
        Spi_Reg_SetRxOnly(Base, FALSE); /* RX Only mode */
        Spi_Reg_SetTxOnly(Base, FALSE); /* TX Only mode */
        Spi_Reg_SetMasterNoOverflowMode(Base, TRUE); /* Master no overflow mode */
        /* Enable SPI interrupt. */
        Core_Hal_EnableIrq(Spi_HalIrqId[Instance]);
        /* Enable SPI module. */
        Spi_Reg_SetEnable(Base, TRUE);
    }
}

/**
* @brief This function de-initializes a SPI instance.
* @note DES_SPI_API_201
*
* @param[in] Instance: SPI instance.
*
* @return void
*
* @pre SPI module must be initialized
* @post SPI module is de-initialized
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_201
*/
void Spi_Hal_DeInit(uint8 Instance)
{
    if (Instance < SPI_INSTANCE_MAX)
    {
        /* Disable the SPI interrupt. */
        Core_Hal_DisableIrq(Spi_HalIrqId[Instance]);

        /* Set SPI module at reset state and disable SPI bus clock. */
        Rcm_Hal_SetResetState(Spi_HalClockReset[Instance], RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState(Spi_HalClockReset[Instance], RCM_RESET_STATE_DEASSERT);
        (void)Ckgen_Hal_EnablePeriphClk(Spi_HalClock[Instance], FALSE);
        Core_Hal_ClearPendingIrq(Spi_HalIrqId[Instance]);

        /* Clear the state pointer. */
        Spi_HalStatePtr[Instance]->Status = SPI_STATUS_NONE;
    }
}

/**
* @brief This function gets default configuration for SPI hal driver.
* @note DES_SPI_API_202
*
* @param[out] ConfigPtr: Pointer to configuration structure which is filled with default configuration.
*
* @return void
*
* @pre None
* @post Configuration structure is filled with default values
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_202
*/
void Spi_Hal_GetDefaultConfig(Spi_HalConfigType *ConfigPtr)
{
    if (NULL_PTR != ConfigPtr)
    {
        *ConfigPtr = Spi_DefaultConfig;
    }
}

/**
* @brief This function transfers by polling mode, which will not return untill timeout or transfer finish.
* @note DES_SPI_API_205
*
* @param[in] Instance: SPI instance.
* @param[in] TxBuffPtr: The pointer to the data buffer of the data to send.
* @param[inout] RxBuffPtr: Pointer to the buffer where the received bytes are stored.
* @param[in] ByteCount: The number of bytes to send and receive.
* @param[in] Timeout: A timeout count value for the transfer. If the transfer takes longer than
*                     this amount of time, the transfer is aborted and a STATUS_TIMEOUT error returned.
*
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Operation successful
* @retval STATUS_TIMEOUT: Operation timed out
* @retval STATUS_BUSY: Module is busy
* @retval STATUS_ERROR: Operation failed
*
* @pre SPI module must be initialized
* @post SPI transfer is completed
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_205
*/
Hal_StatusType Spi_Hal_TransceivePoll
(
    uint8 Instance,
    const uint8 *TxBuffPtr,
    uint8 *RxBuffPtr,
    uint16 ByteCount,
    uint32 Timeout
)
{
    SPI_Type *Base;
    Spi_StateType *StatePtr;
    Hal_StatusType Ret = STATUS_ERROR;
#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
    uint16 RxRoCnt;
#endif

    if (Instance < SPI_INSTANCE_MAX)
    {
        Base = Spi_HalBase[Instance];
        StatePtr = Spi_HalStatePtr[Instance];

        if ((uint16)SPI_STATUS_MEM_BUSY_MASK != StatePtr->Status)
        {
            /* Set the runtime state for transmission. */
            StatePtr->TxBuffPtr = TxBuffPtr;
            StatePtr->RxBuffPtr = RxBuffPtr;
            StatePtr->TxCount = 0U;
            StatePtr->RxCount = 0U;
            StatePtr->XferCount = ByteCount;
            StatePtr->InProgress = TRUE; /* Update transfer status. */
            StatePtr->TransferType = SPI_USING_POLL;
            /* Set the runtime state for transmission. */
            StatePtr->Status = SPI_STATUS_MEM_BUSY_MASK;
            Spi_Reg_SetTxOnly(Base, FALSE); /* Disable Tx only mode. */
            Spi_Reg_SetRxOnly(Base, FALSE); /* Disable Rx only mode. */

            /* TX Only. */
            if ((TxBuffPtr != NULL_PTR) && (RxBuffPtr == NULL_PTR))
            {
                Spi_Reg_SetTxOnly(Base, TRUE); /* Enable Tx only mode. */
                Ret = Spi_Hal_PollData(Instance, ByteCount, Timeout, SPI_HAL_TX);
            }
            /* RX Only. */
            else if ((TxBuffPtr == NULL_PTR) && (RxBuffPtr != NULL_PTR))
            {
                Spi_Reg_SetRxOnly(Base, TRUE); /* Enable Rx only mode. */
#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
                RxRoCnt = Spi_Hal_GetRXCount(Instance, ByteCount);
                if (RxRoCnt > 1U)
                {
                    /* SetROTRIG will toggle 1 clock, so need to -1U here. */
                    Spi_Reg_SetROCNT(Base, RxRoCnt - 1U);
                }
#endif
                Spi_Reg_SetROTRIG(Base, TRUE);
                Ret = Spi_Hal_PollData(Instance, ByteCount, Timeout, SPI_HAL_RX);
            }
            /* TX And RX. */
            /* PRQA S 2995 ++ # AMCE_SPI_2995*/
            else if ((TxBuffPtr != NULL_PTR) && (RxBuffPtr != NULL_PTR))
            /* PRQA S 2995 -- may be a bug of qac*/
            {
                Ret = Spi_Hal_PollData(Instance, ByteCount, Timeout, SPI_HAL_TXRX);
            }
            else
            {
                Ret = STATUS_ERROR;
            }

            if (TRUE == Spi_Reg_IsMaster(Base))
            {
                (void)Spi_Hal_ReleaseCS(Base); /* Release SPI CS to inactive. */
            }
            (void)Spi_Hal_WaitIdle(Base); /* Wait for module idle. */
            (void)Spi_Hal_ClearTxUFRxOF(Base); /* Check error flag. */
            Spi_Reg_SetTxOnly(Base, FALSE);
            Spi_Reg_SetRxOnly(Base, FALSE);
            StatePtr->Status = SPI_STATUS_RX_FINISH_MASK;
            StatePtr->InProgress = FALSE; /* The transfer is complete. */
        }
        else
        {
            Ret = STATUS_BUSY;
        }
    }

    return Ret;
}

/**
* @brief SPI transmission, reception by interrupt.
* @note DES_SPI_API_207
*
* @param[in] Instance: SPI instance.
* @param[in] TxBuffPtr: The pointer to the data buffer of the data to send.
* @param[inout] RxBuffPtr: Pointer to the buffer where the received bytes are stored.
* @param[in] ByteCount: The number of bytes to send and receive.
*
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Operation successful
* @retval STATUS_BUSY: Module is busy
* @retval STATUS_ERROR: Operation failed
*
* @pre SPI module must be initialized
* @post SPI interrupt transfer is started
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_207
*/
Hal_StatusType Spi_Hal_TransceiveInt
(
    uint8 Instance,
    const uint8 *TxBuffPtr,
    uint8 *RxBuffPtr,
    uint16 ByteCount
)
{
    Hal_StatusType Ret = STATUS_ERROR;
    SPI_Type *Base;
    Spi_StateType *StatePtr;
#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
    uint16 RxRoCnt;
#endif

    if (Instance < SPI_INSTANCE_MAX)
    {
        DEVICE_ASSERT(ByteCount != 0U);

        Base = Spi_HalBase[Instance];
        StatePtr = Spi_HalStatePtr[Instance];

        if ((uint16)SPI_STATUS_MEM_BUSY_MASK != StatePtr->Status)
        {
            /* Set the runtime state for transmission. */
            StatePtr->TxBuffPtr = TxBuffPtr;
            StatePtr->RxBuffPtr = RxBuffPtr;
            StatePtr->TxCount = 0U;
            StatePtr->RxCount = 0U;
            StatePtr->XferCount = ByteCount;
            StatePtr->Status = SPI_STATUS_MEM_BUSY_MASK;
            StatePtr->TransferType = SPI_USING_INTERRUPT;
            Spi_Reg_SetTxOnly(Base, FALSE); /* Disable Tx only mode. */
            Spi_Reg_SetRxOnly(Base, FALSE); /* Disable Rx only mode. */
            Ret = STATUS_SUCCESS;

            /* TX Only. */
            if ((TxBuffPtr != NULL_PTR) && (RxBuffPtr == NULL_PTR))
            {
                Spi_Reg_SetTxOnly(Base, TRUE); /* Enable Tx only mode. */
                Spi_Reg_SetIntMode(Base, SPI_INT_TX_DATA, TRUE); /* Enable TXEIE. */
            }
            /* RX Only. */
            else if ((TxBuffPtr == NULL_PTR) && (RxBuffPtr != NULL_PTR))
            {
                Spi_Reg_SetRxOnly(Base, TRUE); /* Enable Rx only mode. */
                Spi_Reg_SetIntMode(Base, SPI_INT_RECEIVE_ERROR, TRUE); /* Enable RX overrun interrupt. */
                Spi_Reg_SetIntMode(Base, SPI_INT_RX_DATA, TRUE); /* Enable RXFIE. */
#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
                RxRoCnt = Spi_Hal_GetRXCount(Instance, ByteCount);
                if (RxRoCnt > 1U)
                {
                    /* SetROTRIG will toggle 1 clock, so need to -1U here. */
                    Spi_Reg_SetROCNT(Base, RxRoCnt - 1U);
                }
#endif
                Spi_Reg_SetROTRIG(Base, TRUE);
            }
            /* TX And RX. */
            /* PRQA S 2995 ++ # AMCE_SPI_2995*/
            else if ((TxBuffPtr != NULL_PTR) && (RxBuffPtr != NULL_PTR))
            /* PRQA S 2995 -- may be a bug of qac*/
            {
                Spi_Reg_SetIntMode(Base, SPI_INT_RECEIVE_ERROR, TRUE); /* Enable RX overrun interrupt. */
                Spi_Reg_SetIntMode(Base, SPI_INT_RX_DATA, TRUE); /* Enable RXFIE. */
                Spi_Reg_SetIntMode(Base, SPI_INT_TX_DATA, TRUE); /* Enable TXEIE. */
            }
            else
            {
                Ret = STATUS_ERROR;
            }
        }
        else
        {
            Ret = STATUS_BUSY;
        }
    }

    return Ret;
}

/**
* @brief SPI transmission,reception by DMA.
* @note DES_SPI_API_209
*
* @param[in] Instance: SPI instance.
* @param[in] TxBuffPtr: The pointer to the data buffer of the data to send.
* @param[inout] RxBuffPtr: Pointer to the buffer where the received bytes are stored.
* @param[in] ByteCount: The number of bytes to send and receive.
*
* @return Hal_StatusType
* @retval STATUS_SUCCESS: Operation successful
* @retval STATUS_BUSY: Module is busy
* @retval STATUS_ERROR: Operation failed
*
* @pre SPI module must be initialized
* @post SPI DMA transfer is started
*
* @note
*     Reentrancy: Non-reentrant
*     ISR Category: None
*     DES ID: DES_SPI_API_209
*/
Hal_StatusType Spi_Hal_TransceiveDma
(
    uint8 Instance,
    const uint8 *TxBuffPtr,
    const uint8 *RxBuffPtr,
    uint16 ByteCount
)
{
    SPI_Type *Base;
    Spi_StateType *StatePtr;
    Hal_StatusType Ret = STATUS_ERROR;

    if (Instance < SPI_INSTANCE_MAX)
    {
        DEVICE_ASSERT(ByteCount != 0U);

        Base = Spi_HalBase[Instance];
        StatePtr = Spi_HalStatePtr[Instance];

        if ((uint16)SPI_STATUS_MEM_BUSY_MASK != StatePtr->Status)
        {
            Ret = STATUS_SUCCESS;
            /* Set the runtime state for transmission. */
            StatePtr->Status = SPI_STATUS_MEM_BUSY_MASK;
            StatePtr->TransferType = SPI_USING_DMA;
            Spi_Reg_SetTxOnly(Base, FALSE); /* Disable Tx only mode. */
            Spi_Reg_SetRxOnly(Base, FALSE); /* Disable Rx only mode. */

            /* TX Only. */
            if ((TxBuffPtr != NULL_PTR) && (RxBuffPtr == NULL_PTR))
            {
                Spi_Reg_SetTxOnly(Base, TRUE);
                Spi_Hal_SetDMAChannel(Instance, TxBuffPtr, RxBuffPtr, ByteCount, SPI_HAL_TX);
            }
            /* RX Only. */
            else if ((TxBuffPtr == NULL_PTR) && (RxBuffPtr != NULL_PTR))
            {
                Spi_Reg_SetIntMode(Base, SPI_INT_RECEIVE_ERROR, TRUE); /* Enable RX overrun interrupt. */
                Spi_Hal_SetDMAChannel(Instance, TxBuffPtr, RxBuffPtr, ByteCount, SPI_HAL_RX);
            }
            /* TX And RX. */
            /* PRQA S 2995 ++ # AMCE_SPI_2995 */
            else if ((TxBuffPtr != NULL_PTR) && (RxBuffPtr != NULL_PTR))
            /* PRQA S 2995 -- may be a bug of qac*/
            {
                Spi_Reg_SetIntMode(Base, SPI_INT_RECEIVE_ERROR, TRUE); /* Enable RX overrun interrupt. */
                Spi_Hal_SetDMAChannel(Instance, TxBuffPtr, RxBuffPtr, ByteCount, SPI_HAL_TXRX);
            }
            else
            {
                Ret = STATUS_ERROR;
            }
        }
        else
        {
            Ret = STATUS_BUSY;
        }
    }

    return Ret;
}

#ifndef SPI_SDK_NON_EXTENDED_API
/**
* @brief This function get spi receive data when in rx buffer.
* @note Function ID: DES_SPI_API_233
* @param[in] Instance: SPI instance.
* @param[in] ReceiveBuffer: To set the BaudRate value.
* @return uint16: SPI get receive data len.
*/
uint16 Spi_Hal_SlaveGetReceiveLen(uint8 Instance, uint8 **ReceiveBuffer)
{
    uint16 Len = 0U;
    SPI_Type const *Base;
    Spi_StateType *StatePtr;

    if (Instance < SPI_INSTANCE_MAX)
    {
        Base = Spi_HalBase[Instance];
        StatePtr = Spi_HalStatePtr[Instance];
        if (TRUE == Spi_Reg_GetStatusFlag(Base, SPI_MODULE_IDLE))
        {
            Len = StatePtr->RxCount;
            if (Len != 0U)
            {
                *ReceiveBuffer = StatePtr->RxBuffPtr;
                StatePtr->RxBuffPtr = StatePtr->RxBuffPtr1;
                StatePtr->RxBuffPtr1 = *ReceiveBuffer;
                StatePtr->RxCount = 0U;
            }
        }
    }

    return Len;
}

/*!
* @brief SPI using interrupt receive by NolimitLen mode, which will
*        return after start transfer.The user needs to check whether the receiveBuffer
*        has valid data the Spi_Hal_GetTransceiveStatus function.
*
* @param[in] Instance: SPI module instance
* @param[in] ReceiveBuffer1: Pointer to the buffer where the received bytes are stored.
* @param[in] ReceiveBuffer2: Pointer to the buffer where the received bytes are stored.
* @param[in] MaxreceiveByteCount: The receiveBuffer max len.
* @return STATUS_SUCCESS The transfer was successful, or
*         STATUS_BUSY Cannot perform transfer because a transfer is already in progress
*/
Hal_StatusType Spi_Hal_SlaveReceiveNolimitLen(uint8 Instance,
        uint8 *ReceiveBuffer1,
        uint8 *ReceiveBuffer2,
        uint16 MaxreceiveByteCount)
{
    DEVICE_ASSERT(NULL_PTR != ReceiveBuffer1);
    DEVICE_ASSERT(NULL_PTR != ReceiveBuffer2);
    SPI_Type *Base;
    Spi_StateType *StatePtr;
    Hal_StatusType Ret = STATUS_ERROR;

    if (Instance < SPI_INSTANCE_MAX)
    {
        Base = Spi_HalBase[Instance];
        StatePtr = Spi_HalStatePtr[Instance];

        /* Clear all interrupts sources */
        SPI_Reg_ClearStatusFlag(Base);

        /* Fill out the other members of the run-time spiState structure. */
        StatePtr->RxBuffPtr = ReceiveBuffer1;
        StatePtr->RxBuffPtr1 = ReceiveBuffer2;
        StatePtr->RxCount = 0U;
        StatePtr->XferCount = MaxreceiveByteCount;

        /* Enable Rx only mode. */
        Spi_Reg_SetRxOnly(Base, TRUE);
        /* Enable RX overrun interrupt. */
        Spi_Reg_SetIntMode(Base, SPI_INT_RECEIVE_ERROR, TRUE);
        /* Enable RXFIE. */
        Spi_Reg_SetIntMode(Base, SPI_INT_RX_DATA, TRUE);
        Ret = STATUS_SUCCESS;
    }

    return Ret;
}

/**
* @brief This function stop SPI Transceive.
* @note Function ID: DES_SPI_API_235
* @param[in] Instance: SPI instance.
* @return Hal_StatusType
*/
Hal_StatusType Spi_Hal_AbortTransceive(uint8 Instance)
{
    SPI_Type *Base;
    Spi_StateType *StatePtr;
    Hal_StatusType Ret = STATUS_ERROR;

    if (Instance < SPI_INSTANCE_MAX)
    {
        Base = Spi_HalBase[Instance];
        StatePtr = Spi_HalStatePtr[Instance];

        Spi_Reg_SetIntMode(Base, SPI_INT_TX_DATA, FALSE); /* Disable TXEIE. */
        Spi_Reg_SetIntMode(Base, SPI_INT_RX_DATA, FALSE); /* Disable RXFIE. */
        Spi_Reg_SetIntMode(Base, SPI_INT_RECEIVE_ERROR, FALSE); /* Disable RX overrun interrupt. */
        if (SPI_USING_DMA == StatePtr->TransferType)
        {
            Spi_Reg_SetTxDmaCmd(Base, FALSE); /* SPI Transmit Data DMA request */
            Spi_Reg_SetRxDmaCmd(Base, FALSE); /* SPI Receive Data DMA request */
            (void)Dma_Hal_StopCh(StatePtr->TxDmaChannel); /* Stop TX DMA channel. */
            (void)Dma_Hal_StopCh(StatePtr->RxDmaChannel); /* Stop RX DMA channel. */
        }

        if (TRUE == Spi_Reg_IsMaster(Base))
        {
            (void)Spi_Hal_ReleaseCS(Base); /* Release SPI CS to inactive. */
        }
        StatePtr->InProgress = FALSE; /* The transfer is complete. */
        StatePtr->Status = SPI_STATUS_ABORT_MASK;
        Ret = STATUS_SUCCESS;
    }

    return Ret;
}

/**
* @brief This function get SPI Transceive Status.
* @note Function ID: DES_SPI_API_236
* @param[in] Instance: SPI instance.
* @return Spi_TransceiveStatusType:the Transceive Status.
*/
Spi_TransceiveStatusType Spi_Hal_GetTransceiveStatus(uint8 Instance)
{
    const Spi_StateType *StatePtr;
    Spi_TransceiveStatusType Status = SPI_TRANSCEIVE_NOT_STARTED;
    if (Instance < SPI_INSTANCE_MAX)
    {
        /* Reset SPI status, but confifuration still valid. */
        StatePtr = Spi_HalStatePtr[Instance];
        switch (StatePtr->Status)
        {
        case (SPI_STATUS_MEM_BUSY_MASK):
            Status = SPI_TRANSCEIVE_BUSY;
            break;

        case (SPI_STATUS_TIMEOUT_MASK):
            Status = SPI_TRANSCEIVE_TIMEOUT;
            break;

        case (SPI_STATUS_RX_OVERFLOW_MASK):
        case (SPI_STATUS_TX_UNDERFLOW_MASK):
        case (SPI_STATUS_DMA_ERROR_MASK):
            Status = SPI_TRANSCEIVE_ERROR;
            break;

        case (SPI_STATUS_RX_FINISH_MASK):
        case (SPI_STATUS_TX_FINISH_MASK):
        case (SPI_STATUS_DATA_MATCH_MASK):
            Status = SPI_TRANSCEIVE_SUCCESS;
            break;

        case (SPI_STATUS_ABORT_MASK):
            Status = SPI_TRANSCEIVE_SUCCESS;
            break;

        default :
            /* do nothing */
            break;
        }
    }

    return Status;
}

/**
* @brief This function reset the SPI Status.
* @note Function ID: DES_SPI_API_237
* @param[in] Instance: SPI instance.
* @return void
*/
void Spi_Hal_SoftwareReset(uint8 Instance)
{
    SPI_Type *Base;
    if (Instance < SPI_INSTANCE_MAX)
    {
        Base = Spi_HalBase[Instance];
        /* Reset SPI status, but confifuration still valid. */
        Spi_Reg_SoftwareReset(Base);
    }
}

/**
* @brief This function set the SPI pin mode.
* @note Function ID: DES_SPI_API_238
* @param[in] Instance: SPI instance.
* @param[in] PinCfg: SPI pin mode.
* @return void
*/
void Spi_Hal_SetPinMode(uint8 Instance, Spi_PinCfgType PinCfg)
{
    SPI_Type *Base;
    if (Instance < SPI_INSTANCE_MAX)
    {
        Base = Spi_HalBase[Instance];
        /* Set Pin (SOUT, SIN) configuration */
        Spi_Reg_SetPinConfigMode(Base, PinCfg);
    }
}

/**
* @brief This function select the SPI cs pin.
* @note Function ID: DES_SPI_API_239
* @param[in] Instance: SPI instance.
* @param[in] Pcs: SPI CS select.
* @param[in] Polarity: CS Polarity.
* @return void
*/
void Spi_Hal_SetCsPin(uint8 Instance, Spi_PcsType Pcs, Spi_PcsPolarityType Polarity)
{
    SPI_Type *Base;
    Spi_StateType *StatePtr;
    if (Instance < SPI_INSTANCE_MAX)
    {
        Base = Spi_HalBase[Instance];
        StatePtr = Spi_HalStatePtr[Instance];

        if (Pcs == SPI_PCS_GPIO)
        {
            Spi_Reg_SetCSOE(Base, FALSE);
        }
        else
        {
            Spi_Reg_SetPcsPolarity(Base, Pcs, Polarity); /* Set the desired PCS polarity */
            Spi_Reg_SetPcsCfg(Base, Pcs); /* Select which CS to use */
            Spi_Reg_SetCSOE(Base, TRUE);
        }
        StatePtr->Pcs = Pcs;
    }
}
#endif

/*PRQA S 1505 ++ # The function is only reference in the translation unit where it is defined. */
/**
* @brief This function set the SPI BaudRate.
* @note Function ID: DES_SPI_API_230
* @param[in] Instance: SPI instance.
* @param[in] BaudRate: To set the BaudRate value.
* @return Hal_StatusType
*/
Hal_StatusType Spi_Hal_SetBaudRate(uint8 Instance, uint32 BaudRate)
{
    SPI_Type *Base;
    uint8 divisor = 0U;
    /* calculated baud rate */
    uint32 sourceClockInHz = 0U;
    Hal_StatusType ReturnType = STATUS_ERROR;

    if (Instance < SPI_INSTANCE_MAX)
    {
        Base = Spi_HalBase[Instance];
        (void)Ckgen_Hal_GetFreq(SpiClock[Instance], &sourceClockInHz);
        /* bitPerSec must be smaller than sourceClockInHz/2 */
        if (BaudRate > (sourceClockInHz >> 1U))
        {
            ReturnType = STATUS_ERROR;
        }
        else
        {
            /* Get SCK_HIGH and SCK_LOW */
            divisor = (uint8)((sourceClockInHz / BaudRate) - 2U);
            /* PRQA S 1891 ++ # AMCE_SPI_1891 */
            /* PRQA S 2985 ++ # AMCE_QAC_2985.*/
            divisor = (uint8)((divisor >> 1U) & SPI_CFG0_SCK_HIGH_Msk);
            /* PRQA S 2985 -- */
            /* PRQA S 1891 -- */

            /* Set SCK_HIGH and SCK_LOW */
            Spi_Reg_SetBaudRate(Base, divisor, divisor); /* Configure the baud rate */

            ReturnType = STATUS_SUCCESS;
        }
    }

    return ReturnType;
}
/*PRQA S 1505 -- */

#ifndef SPI_SDK_NON_EXTENDED_API
/**
* @brief This function set the SPI BaudRate.
* @note Function ID: DES_SPI_API_240
* @param[in] Instance: SPI instance.
* @return uint32:SPI BaudRate
*/
uint32 Spi_Hal_GetBaudRate(uint8 Instance)
{
    SPI_Type *Base;
    uint32 Ret = 0U;
    /* calculated baud rate */
    uint32 SourceClockInHz = 0U;
    uint8 Low = 0U;
    uint8 High = 0U;

    if (Instance < SPI_INSTANCE_MAX)
    {
        Base = Spi_HalBase[Instance];
        (void)Ckgen_Hal_GetFreq(SpiClock[Instance], &SourceClockInHz);
        /* Get SCK_HIGH and SCK_LOW */
        Spi_Reg_GetSckHighLow(Base, &Low, &High);

        /* Actual calculated baud rate */
        /* PRQA S 1891 ++ # AMCE_SPI_1891 */
        Ret = SourceClockInHz / (Low + High + 2U);
        /* PRQA S 1891 -- */
    }

    return Ret;
}

/**
* @brief This function set the SPI Match Data.
* @note Function ID: DES_SPI_API_241
* @param[in] Instance: SPI instance.
* @param[in] Data: Data to Match.
* @param[in] Enable: Enable or Disable Data Match.
* @return void
*/
void Spi_Hal_SetMatchData(uint8 Instance, uint32 Data, boolean Enable)
{
    SPI_Type *Base;
    if (Instance < SPI_INSTANCE_MAX)
    {
        Base = Spi_HalBase[Instance];
        Spi_Reg_ClearRDMF(Base);
        Spi_Reg_SetMatchData(Base, Data);
        Spi_Reg_SetDMIE(Base, Enable);
#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
        Spi_Reg_SetDATAME(Base, Enable);
#endif
    }
}
#endif

/**
* @brief Get the SPI base.
* @note Function ID: DES_SPI_API_231
* @param[in] Instance: SPI instance.
* @return SPI_Type: SPI Base Addr.
*/
SPI_Type *Spi_Hal_GetBase(uint8 Instance)
{
    SPI_Type *Ret = NULL_PTR;
    if (Instance < SPI_INSTANCE_MAX)
    {
        Ret = Spi_HalBase[Instance];
    }

    return Ret;
}

#ifndef SPI_SDK_NON_EXTENDED_API
/**
* @brief This function set the SPI FrameSize.
* @note Function ID: DES_SPI_API_242
* @param[in] Instance: SPI instance.
* @param[in] Framesize: Number of bits per frame, support 4~32 bits.
* @return void
*/
void Spi_Hal_SetFrameSize(uint8 Instance, uint8 Framesize)
{
    SPI_Type *Base;
    Spi_StateType *StatePtr;
    DEVICE_ASSERT((Framesize >= 4U) && (Framesize <= 32U));

    if (Instance < SPI_INSTANCE_MAX)
    {
        Base = Spi_HalBase[Instance];
        StatePtr = Spi_HalStatePtr[Instance];

        StatePtr->FrmSize = Framesize;
        /* Set bits per frame */
        Spi_Reg_SetFrameSize(Base, Framesize);
    }
}
#endif

/**
* @brief This function checks whether the status of SPI hal driver is busy.
* @note Function ID: DES_SPI_API_211
* @param[in] Instance: SPI instance.
* @return boolean
*         - TRUE: The SPI hal driver is busy.
*         - FALSE: The SPI hal driver is not busy.
*/
boolean Spi_Hal_CheckBusy(uint8 Instance)
{
    boolean Ret = TRUE;
    const Spi_StateType *StatePtr;

    if (Instance < SPI_INSTANCE_MAX)
    {
        StatePtr = Spi_HalStatePtr[Instance];
        Ret = StatePtr->InProgress;
    }

    return Ret;
}

/**
* @brief This function is the implementation of SPI0 handler named in startup code.
*        It passes the instance to the shared SPI IRQ handler.
* @note Function ID: DES_SPI_API_221
* @return void
*/
void SPI0_IRQHandler(void)
{
    Spi_Hal_IRQHandler(0U);
}

/**
* @brief This function is the implementation of SPI1 handler named in startup code.
*        It passes the instance to the shared SPI IRQ handler.
* @note Function ID: DES_SPI_API_222
* @return void
*/
void SPI1_IRQHandler(void)
{
    Spi_Hal_IRQHandler(1U);
}

/**
* @brief This function is the implementation of SPI2 handler named in startup code.
*        It passes the instance to the shared SPI IRQ handler.
* @note Function ID: DES_SPI_API_223
* @return void
*/
void SPI2_IRQHandler(void)
{
    Spi_Hal_IRQHandler(2U);
}

#if defined (AC7842X) || defined (AC7843X)
/**
* @brief This function is the implementation of SPI3 handler named in startup code.
*        It passes the instance to the shared SPI IRQ handler.
* @note Function ID: DES_SPI_API_224
* @return void
*/
void SPI3_IRQHandler(void)
{
    Spi_Hal_IRQHandler(3U);
}
#endif

#if defined (AC7843X)
/**
* @brief This function is the implementation of SPI4 handler named in startup code.
*        It passes the instance to the shared SPI IRQ handler.
* @note Function ID: DES_SPI_API_225
* @return void
*/
void SPI4_IRQHandler(void)
{
    Spi_Hal_IRQHandler(4U);
}
#endif

/*====================================================EOF===========================================*/
