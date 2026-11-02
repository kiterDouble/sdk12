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
 * @file I2c_Hal.c
 * @brief This file provides I2c hal function
 */
#ifdef __cplusplus
extern "C"
{
#endif

/* ===========================================  Include  ============================================ */
#include "AC784xx_I2c_Reg.h"
#include "Ckgen_Hal.h"
#include "Dma_Hal.h"
#include "I2c_Hal.h"
#include "Rcm_Hal.h"
#include "Core_Hal.h"

/* ============================================  DEFINES AND MACROS  ============================================ */
#define I2C_WAIT_BND(CH, STATUS, TIMEOUT) \
    { \
        TIMEOUT = 0U; \
        STATUS = I2c_Reg_GetStatus0(CH); \
        while ((0U == (STATUS & I2C_STATUS0_BND_Msk)) && (TIMEOUT < I2C_HW_DEADLINE_TIMEOUT)) \
        { \
            STATUS = I2c_Reg_GetStatus0(CH); \
            TIMEOUT++; \
        } \
    }

#define I2C_WAIT_STATUS(CH, STATUS, MASK) \
    { \
        STATUS = I2c_Reg_GetStatus0(CH); \
        while (0U == (STATUS & MASK)) \
        { \
            STATUS = I2c_Reg_GetStatus0(CH); \
        } \
    }

/* ============================================= TYPEDEFS ================================================ */
/*!
 * @brief Master internal context structure
 * @note DES_I2C_TYP_001
 */
typedef struct
{
    uint8 TxDmaChannel;                 /*!< Tx Channel number for DMA */
    uint8 RxDmaChannel;                 /*!< Rx Channel number for DMA */
    uint16 SlaveAddress;                /*!< Slave address */
    uint32 RxSize;                      /*!< Size of receive data buffer */
    uint32 TxSize;                      /*!< Size of transmit data buffer */
    uint32 RxPointer;                   /*!< position of receive data in user defined buffer */
    uint32 SourceClock;                 /*!< I2c master mode source clock */
    uint8 *RxBuffPtr;                   /*!< Pointer to receive data buffer */
    const uint8 *TxBuffPtr;             /*!< Pointer to transmit data buffer */
    I2c_Hal_DirType DirType;            /*!< Type of I2C transfer direction */
    I2c_Hal_TransferType TransferType;  /*!< Type of I2C transfer, DMA or interrupt */
    I2c_Hal_MasterStatusType Status;    /*!< Status of last driver operation */
    boolean SendStop;                   /*!< Specifies if STOP condition must be generated after current transfer*/
    boolean Is10bitAddr;                /*!< i2c slave addr is 10bit or not */
} I2c_Hal_MasterStateType;

/*!
 * @brief Slave internal context structure
 * @note DES_I2C_TYP_002
 */
typedef struct
{
    boolean slaveListening;             /*!< Slave mode (always listening or on demand only) */
    uint8 TxDmaChannel;                 /*!< Tx Channel number for DMA */
    uint8 RxDmaChannel;                 /*!< Rx Channel number for DMA */
    uint16 SlaveAddress;                /*!< Slave address, 7-bit or 10-bit */
    uint32 RxSize;                      /*!< Size of receive data buffer */
    uint32 TxSize;                      /*!< Size of transmit data buffer */
    uint32 RxPointer;                   /*!< position of receive data in user defined buffer */
    uint8 *RxBuffPtr;                   /*!< Pointer to receive data buffer */
    const uint8 *TxBuffPtr;             /*!< Pointer to transmit data buffer */
    I2c_Hal_TransferType TransferType;  /*!< Type of I2C transfer, DMA or interrupt */
    I2c_Hal_SlaveStatusType Status;     /*!< Status of last driver operation */
} I2c_Hal_SlaveStateType;

/* =========================================== LOCAL VARIABLES ============================================== */

#if (CONFIG_I2C0_ENABLE)
static I2c_Hal_MasterStateType I2c0MasterStatePtr;
static I2c_Hal_SlaveStateType I2c0SlaveStatePtr;
static I2c_Hal_ChannelConfigType I2c0ChannelConfigPtr;
#endif

#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
#if (CONFIG_I2C1_ENABLE)
static I2c_Hal_MasterStateType I2c1MasterStatePtr;
static I2c_Hal_SlaveStateType I2c1SlaveStatePtr;
static I2c_Hal_ChannelConfigType I2c1ChannelConfigPtr;
#endif
#endif

#if defined (AC7843X)
#if (CONFIG_I2C2_ENABLE)
static I2c_Hal_MasterStateType I2c2MasterStatePtr;
static I2c_Hal_SlaveStateType I2c2SlaveStatePtr;
static I2c_Hal_ChannelConfigType I2c2ChannelConfigPtr;
#endif
#endif

/* Pointer to runtime state structure */
static I2c_Hal_MasterStateType *const I2cMasterStatePtr[I2C_INSTANCE_MAX] =
{
#if (CONFIG_I2C0_ENABLE)
    &I2c0MasterStatePtr,
#else
    NULL_PTR,
#endif

#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
#if (CONFIG_I2C1_ENABLE)
    &I2c1MasterStatePtr,
#else
    NULL_PTR,
#endif
#endif

#if defined (AC7843X)
#if (CONFIG_I2C2_ENABLE)
    &I2c2MasterStatePtr,
#else
    NULL_PTR,
#endif
#endif
};

static I2c_Hal_SlaveStateType *const I2cSlaveStatePtr[I2C_INSTANCE_MAX] =
{
#if (CONFIG_I2C0_ENABLE)
    &I2c0SlaveStatePtr,
#else
    NULL_PTR,
#endif

#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
#if (CONFIG_I2C1_ENABLE)
    &I2c1SlaveStatePtr,
#else
    NULL_PTR,
#endif
#endif

#if defined (AC7843X)
#if (CONFIG_I2C2_ENABLE)
    &I2c2SlaveStatePtr,
#else
    NULL_PTR,
#endif
#endif
};

static I2c_Hal_ChannelConfigType *const I2cChannelConfigPtr[I2C_INSTANCE_MAX] =
{
#if (CONFIG_I2C0_ENABLE)
    &I2c0ChannelConfigPtr,
#else
    NULL_PTR,
#endif

#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
#if (CONFIG_I2C1_ENABLE)
    &I2c1ChannelConfigPtr,
#else
    NULL_PTR,
#endif
#endif

#if defined (AC7843X)
#if (CONFIG_I2C2_ENABLE)
    &I2c2ChannelConfigPtr,
#else
    NULL_PTR,
#endif
#endif
};

/**
* @brief Base pointers for SPI instances.
*/
static I2C_Type *const I2c_HalBase[I2C_INSTANCE_MAX] = I2C_BASE_PTRS;

/**
* @brief Table to save I2C IRQ enumeration.
*/
static const IRQn_Type I2c_HalIrqId[I2C_INSTANCE_MAX] = I2C_IRQS;

/**
* @brief Table to save I2C soft resets.
*/
static const Rcm_ResetIDType I2c_HalClockReset[I2C_INSTANCE_MAX] =
{
    RCM_RESET_ID_I2C0,
#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
    RCM_RESET_ID_I2C1,
#endif
#if defined (AC7843X)
    RCM_RESET_ID_I2C2,
#endif
};
static const Ckgen_BusClkIdType I2c_HalClock[I2C_INSTANCE_MAX] =
{
    CKGEN_I2C0_BUS_CLK,
#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
    CKGEN_I2C1_BUS_CLK,
#endif
#if defined (AC7843X)
    CKGEN_I2C2_BUS_CLK,
#endif
};

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/* ======================================  Functions definition  ==================================== */
ISR(I2C0_IRQHandler);
#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
ISR(I2C1_IRQHandler);
#endif
#if defined (AC7843X)
ISR(I2C2_IRQHandler);
#endif

/**
* @brief Get internal hardware ready status.
* @note Function ID:DES_I2C_API_252
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @return Hal_StatusType: The result of execution
*/
static Hal_StatusType I2c_Hal_WaitForReady
(
    const I2C_Type *Base
)
{
    Hal_StatusType ReturnType = STATUS_SUCCESS;
    volatile uint32 Timeout = 0U;
    uint32 ReadyBit;

    ReadyBit = READ_BIT32(I2c_Reg_GetStatus0(Base), I2C_STATUS0_READY_Msk);
    /*PRQA S 3415 ++ # AMCE_I2C_3415.*/
    while ((0U == ReadyBit) && (Timeout < I2C_HW_DEADLINE_TIMEOUT))
    /*PRQA S 3415 -- */
    {
        ReadyBit = READ_BIT32(I2c_Reg_GetStatus0(Base), I2C_STATUS0_READY_Msk);
        Timeout++; //PRQA S 3387 # AMCE_I2C_3387
    }
    if (I2C_HW_DEADLINE_TIMEOUT <= Timeout)
    {
        ReturnType = STATUS_ERROR;
    }
    return ReturnType;
}

/**
* @brief Send start signal for master.
* @note Function ID:DES_I2C_API_253
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @return Hal_StatusType: The result of execution
*/
static Hal_StatusType I2c_Hal_Start
(
    I2C_Type *Base
)
{
    Hal_StatusType ReturnType;
    volatile uint32 Timeout = 0U;

    /* I2c Tx IsEnable */
    I2c_Reg_TxEn(Base);
    /* send start bus */
    I2c_Reg_SendStart(Base);
    /* wait start status */
    while (Timeout < I2C_HW_DEADLINE_TIMEOUT)
    {
        if (0U != (I2c_Reg_IsStart(Base)))
        {
            break;
        }
        Timeout++; //PRQA S 3387 # AMCE_I2C_3387
    }
    if (I2C_HW_DEADLINE_TIMEOUT <= Timeout)
    {
        ReturnType = STATUS_ERROR;
    }
    else
    {
        Timeout = 0U;
        /* wait busy status */
        /*PRQA S 3415 ++ # AMCE_I2C_3415.*/
        while ((0U == (I2c_Reg_GetStatus0(Base) & I2C_STATUS0_BUSY_Msk)) && (Timeout < I2C_HW_DEADLINE_TIMEOUT))
        /*PRQA S 3415 -- */
        {
            Timeout++; //PRQA S 3387 # AMCE_I2C_3387
        }
        if (I2C_HW_DEADLINE_TIMEOUT <= Timeout)
        {
            ReturnType = STATUS_ERROR;
        }
        else
        {
            /* clear the start flag bit */
            I2c_Reg_ClearStartFlag(Base);
            ReturnType = I2c_Hal_WaitForReady(Base);
        }
    }

    return ReturnType;
}

/**
* @brief Wait one byte transmit finished.
* @note Function ID:DES_I2C_API_257
* @note Service ID: NA
* @param[in] Base: I2C Base pointer
* @return Hal_StatusType: i2c hardware status
*/
static Hal_StatusType I2c_Hal_WaitOneByteFinished
(
    I2C_Type *Base
)
{
    volatile uint32 Timeout = 0U;
    uint32 TmpReg;
    Hal_StatusType ReturnType = STATUS_SUCCESS;

    TmpReg = I2c_Reg_GetStatus0(Base);
    while (I2C_HW_DEADLINE_TIMEOUT > Timeout)
    {
        if (0U != (TmpReg & I2C_STATUS0_BND_Msk))
        {
            break;
        }
        TmpReg = I2c_Reg_GetStatus0(Base);
        Timeout++; //PRQA S 3387 # AMCE_I2C_3387
    }
    if ((TmpReg & I2C_STATUS0_RACK_Msk) != 0U)
    {
        I2c_Reg_ClearStatus0(Base, (uint32)I2C_STATUS0_RACK_Msk);
    }
    if ((TmpReg & I2C_STATUS0_BND_Msk) != 0U)
    {
        I2c_Reg_ClearStatus0(Base, (uint32)I2C_STATUS0_BND_Msk);
    }
    if (I2C_HW_DEADLINE_TIMEOUT <= Timeout)
    {
        ReturnType = STATUS_ERROR;
    }

    return ReturnType;
}

/**
* @brief Ends current transmission or reception.
* @note Function ID:DES_I2C_API_244
* @param[in] Instance: The I2C Instance number
* @param[in] MasterPtr: I2C master driver pointer
* @param[in] Dir: I2C_READ or I2C_WRITE
* @return void
*/
static void I2c_Hal_MasterEndTransferPoll
(
    uint8 Instance,
    I2c_Hal_MasterStateType *MasterPtr,
    I2c_Hal_DirType Dir
)
{
    I2C_Type *ChannelAddress;
    if ((Instance < I2C_INSTANCE_MAX) && (MasterPtr != NULL_PTR))
    {
        ChannelAddress = I2c_HalBase[Instance];
        /* check if read data*/
        if (I2C_READ == Dir)
        {
            /* send Ack and read the last byte data*/
            I2c_Reg_SendAck(ChannelAddress);
            I2c_Reg_ReceiveLastOneByte(ChannelAddress, &MasterPtr->RxBuffPtr[0]);
            /*PRQA S 2982 ++ # AMCE_I2C_2982.*/
            MasterPtr->RxPointer++;
            /*PRQA S 2982 -- #.*/
        }

        /* Disable all interrupt events */
        I2c_Reg_SetInterrupt(ChannelAddress, FALSE);

        /* Send stop single */
        if (TRUE == MasterPtr->SendStop)
        {
            I2c_Reg_SendStop(ChannelAddress);
        }
        /* Clear I2C BND flag */
        I2c_Reg_ClearStatus0(ChannelAddress, (uint32)I2C_STATUS0_BND_Msk);
        if (NULL_PTR != I2cChannelConfigPtr[Instance]->Callback)
        {
            I2cChannelConfigPtr[Instance]->Callback(Instance, (uint32)I2C_MASTER_EVENT_END_TRANSFER);
        }

        MasterPtr->RxSize = 0U;
        MasterPtr->TxSize = 0U;
        MasterPtr->RxPointer = 0U;
        MasterPtr->RxBuffPtr = NULL_PTR;
        MasterPtr->TxBuffPtr = NULL_PTR;
    }
}

/**
* @brief Ends current transmission or reception.
* @note Function ID:DES_I2C_API_206
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] MasterPtr: I2C master driver pointer
* @param[in] SendStop: If TRUE generate stop condition after the transmission, otherwise not
* @return void
*/
static void I2c_Hal_MasterEndTransfer
(
    uint8 Instance,
    I2c_Hal_MasterStateType *MasterPtr,
    boolean SendStop
)
{
    DEVICE_ASSERT(MasterPtr != NULL_PTR);
    I2C_Type *ChannelAddress;
    if ((Instance < I2C_INSTANCE_MAX) && (MasterPtr != NULL_PTR))
    {
        ChannelAddress = I2c_HalBase[Instance];
        /* Disable all interrupt events */
        I2c_Reg_SetInterrupt(ChannelAddress, FALSE);
        if (I2C_USING_DMA == MasterPtr->TransferType)
        {
            /* Stop DMA Channel */
            (void)Dma_Hal_StopCh(MasterPtr->TxDmaChannel);
            (void)Dma_Hal_StopCh(MasterPtr->RxDmaChannel);
            /* Disable I2C DMA request */
            I2c_Reg_SetDMARx(ChannelAddress, FALSE);
            I2c_Reg_SetDMATx(ChannelAddress, FALSE);
        }

        /* Send stop single */
        if (TRUE == SendStop)
        {
            I2c_Reg_SendStop(ChannelAddress);
        }

        MasterPtr->TxBuffPtr = NULL_PTR;
        MasterPtr->TxSize = 0U;
        MasterPtr->RxPointer = 0U;
        MasterPtr->RxBuffPtr = NULL_PTR;
        MasterPtr->RxSize = 0U;
    }
}

/**
* @brief Handle a transmit request for master.
* @note Function ID:DES_I2C_API_207
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] MasterPtr: I2C master driver pointer
* @return void
*/
static void I2c_Hal_MasterHandleTransmitDataRequest
(
    uint8 Instance,
    I2c_Hal_MasterStateType *MasterPtr
)
{
    DEVICE_ASSERT(MasterPtr->TxBuffPtr != NULL_PTR);
    I2C_Type *ChannelAddress;
    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        /* stop send when TxSize is 0 */
        if (0U == MasterPtr->TxSize)
        {
            /* There is no more data in buffer, the transmission is over */
            I2c_Hal_MasterEndTransfer(Instance, MasterPtr, MasterPtr->SendStop);
            /* Clear I2C BND flag */
            I2c_Reg_ClearStatus0(ChannelAddress, (uint32)I2C_STATUS0_BND_Msk);
            MasterPtr->Status = I2C_MASTER_CHANNEL_FINISHED;
            if (NULL_PTR != I2cChannelConfigPtr[Instance]->Callback)
            {
                I2cChannelConfigPtr[Instance]->Callback(Instance, (uint32)I2C_MASTER_EVENT_END_TRANSFER);
            }
        }
        else
        {
            /* Write data to send */
            I2c_Reg_WriteDataReg(ChannelAddress, MasterPtr->TxBuffPtr[0U]);
            MasterPtr->TxSize--;
            MasterPtr->TxBuffPtr++;
        }
    }
}

/**
* @brief Handle a receive request for master.
* @note Function ID:DES_I2C_API_208
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] MasterPtr: I2C master driver pointer
* @return void
*/
static void I2c_Hal_MasterHandleReceiveDataReadyEvent
(
    uint8 Instance,
    I2c_Hal_MasterStateType *MasterPtr
)
{
    I2C_Type *ChannelAddress;
    if ((Instance < I2C_INSTANCE_MAX) && (MasterPtr != NULL_PTR))
    {
        ChannelAddress = I2c_HalBase[Instance];

        /* Check Tx or Rx */
        if (I2c_Reg_IsTx(ChannelAddress) != 0U)
        {
            /* Enable RX */
            I2c_Reg_RxEn(ChannelAddress);
            if (1U == MasterPtr->RxSize)
            {
                /* send Nack */
                I2c_Reg_SendNack(ChannelAddress);
            }
            else
            {
                /* send Ack */
                I2c_Reg_SendAck(ChannelAddress);
            }
            /* Dump read data register */
            (void)I2c_Reg_ReadDataReg(ChannelAddress);
        }
        else
        {
            if ((1U == MasterPtr->RxSize) || ((MasterPtr->RxSize - 2U) < MasterPtr->RxPointer))
            {
                /* send Ack */
                I2c_Reg_SendAck(ChannelAddress);
                I2c_Reg_ReceiveLastOneByte(ChannelAddress, &MasterPtr->RxBuffPtr[0]);
                MasterPtr->RxPointer++;
                /* There is no more data in buffer, the transmission is over */
                I2c_Hal_MasterEndTransfer(Instance, MasterPtr, MasterPtr->SendStop);

                MasterPtr->Status = I2C_MASTER_CHANNEL_FINISHED;
                if (NULL_PTR != I2cChannelConfigPtr[Instance]->Callback)
                {
                    I2cChannelConfigPtr[Instance]->Callback(Instance, (uint32)I2C_MASTER_EVENT_END_TRANSFER);
                }
            }
            else
            {
                if ((MasterPtr->RxSize - 2U) > MasterPtr->RxPointer)
                {
                    /* send Ack */
                    I2c_Reg_SendAck(ChannelAddress);
                    MasterPtr->RxBuffPtr[0] = ((uint8)ChannelAddress->DATA);
                    MasterPtr->RxBuffPtr++;
                    MasterPtr->RxPointer++;
                }
                else
                {
                    /* send Nack */
                    I2c_Reg_SendNack(ChannelAddress);
                    MasterPtr->RxBuffPtr[0U] = ((uint8)ChannelAddress->DATA);
                    MasterPtr->RxBuffPtr++;
                    MasterPtr->RxPointer++;
                }
            }
        }
    }
}

/**
* @brief Start I2C transmission and send Slave read address.
* @note Function ID:DES_I2C_API_209
* @note Service ID: NA
* @param[in] ChannelAddress: I2C ChannelAddress pointer
* @param[in] MasterPtr: I2C master driver pointer
* @return Hal_StatusType: The result of execution
*/
static Hal_StatusType I2c_Hal_MasterSendReadAddress
(
    I2C_Type *ChannelAddress,
    const I2c_Hal_MasterStateType *MasterPtr
)
{
    Hal_StatusType ReturnType = STATUS_SUCCESS;
    if (STATUS_ERROR == I2c_Hal_WaitForReady(ChannelAddress))/* wait ready failed */
    {
        ReturnType = STATUS_ERROR;
    }
    else
    {
        if (STATUS_ERROR == I2c_Hal_Start(ChannelAddress))/* start failed */
        {
            ReturnType = STATUS_ERROR;
        }
        else/* start success */
        {
            if (TRUE == MasterPtr->Is10bitAddr)
            {
                I2c_Reg_SetInterrupt(ChannelAddress, FALSE);
                /*PRQA S 2985 ++ # AMCE_QAC_2985.*/
                I2c_Reg_TransmitOneByte(ChannelAddress, ((I2C_ADDEXT_PRIMARY_BYTE_FIX
                                        + (uint8)(MasterPtr->SlaveAddress >> 8U)) << 1U) | (uint8)I2C_WRITE);
                /*PRQA S 2985 -- */
                (void)I2c_Hal_WaitOneByteFinished(ChannelAddress);
                I2c_Reg_TransmitOneByte(ChannelAddress, (uint8)MasterPtr->SlaveAddress);
                /*PRQA S 4432 ++ # AMCE_I2C_4432.*/
                (void)I2c_Hal_WaitOneByteFinished(ChannelAddress);
                ReturnType = I2c_Hal_Start(ChannelAddress);
                /*PRQA S 4432 -- */
                I2c_Reg_SetInterrupt(ChannelAddress, TRUE);
                if (STATUS_SUCCESS == ReturnType)
                {
                    I2c_Reg_TxEn(ChannelAddress);
                    I2c_Reg_WriteDataReg(ChannelAddress, ((I2C_ADDEXT_PRIMARY_BYTE_FIX \
                                                           + (uint8)(MasterPtr->SlaveAddress >> 8U)) << 1U) \
                                         | (uint8)I2C_READ);
                }
            }
            else
            {
                /* I2c Tx enable */
                I2c_Reg_TxEn(ChannelAddress);
                I2c_Reg_WriteDataReg(ChannelAddress, (uint8)((MasterPtr->SlaveAddress << 1U) | (uint8)I2C_READ));
            }
        }
    }

    return ReturnType;
}

/**
* @brief Start I2C transmission and send Slave write address.
* @note Function ID:DES_I2C_API_210
* @note Service ID: NA
* @param[in] ChannelAddress: I2C ChannelAddress pointer
* @param[in] MasterPtr: I2C master driver pointer
* @return Hal_StatusType: The result of execution
*/
static Hal_StatusType I2c_Hal_MasterSendWriteAddress
(
    I2C_Type *ChannelAddress,
    const I2c_Hal_MasterStateType *MasterPtr
)
{
    Hal_StatusType ReturnType = STATUS_SUCCESS;
    Hal_StatusType TempRet;
    TempRet = I2c_Hal_WaitForReady(ChannelAddress);

    if (STATUS_ERROR == TempRet)/* wait ready failed */
    {
        ReturnType = STATUS_ERROR;
    }
    else
    {
        TempRet = I2c_Hal_Start(ChannelAddress);
        if (STATUS_ERROR == TempRet)/* start failed */
        {
            ReturnType = STATUS_ERROR;
        }
        else/* start success */
        {
            if (TRUE == MasterPtr->Is10bitAddr)
            {
                I2c_Reg_SetInterrupt(ChannelAddress, FALSE);
                if (I2C_USING_DMA == MasterPtr->TransferType)
                {
                    I2c_Reg_SetDMATx(ChannelAddress, FALSE);
                }
                /*PRQA S 2985 ++ # AMCE_QAC_2985.*/
                I2c_Reg_TransmitOneByte(ChannelAddress, ((I2C_ADDEXT_PRIMARY_BYTE_FIX
                                        + (uint8)(MasterPtr->SlaveAddress >> 8U)) << 1U) | (uint8)I2C_WRITE);
                TempRet = I2c_Hal_WaitOneByteFinished(ChannelAddress);
                /*PRQA S 2985 -- */
                I2c_Reg_SetInterrupt(ChannelAddress, TRUE);
                if (I2C_USING_DMA == MasterPtr->TransferType)
                {
                    I2c_Reg_SetDMATx(ChannelAddress, TRUE);
                }
                if (STATUS_SUCCESS == TempRet)
                {
                    I2c_Reg_TxEn(ChannelAddress);
                    I2c_Reg_WriteDataReg(ChannelAddress, (uint8)MasterPtr->SlaveAddress);
                }
            }
            else
            {
                /* I2c Tx enable */
                I2c_Reg_TxEn(ChannelAddress);
                /*PRQA S 2985 ++ # AMCE_QAC_2985.*/
                I2c_Reg_WriteDataReg(ChannelAddress, (uint8)((MasterPtr->SlaveAddress << 1U) | (uint8)I2C_WRITE));
                /*PRQA S 2985 -- */
            }
        }
    }

    return ReturnType;
}

/**
* @brief Start I2C transmission and send Slave read address on poll.
* @note Function ID:DES_I2C_API_256
* @note Service ID: NA
* @param[in] ChannelAddress: I2C ChannelAddress pointer
* @param[in] MasterPtr: I2C master driver pointer
* @return Hal_StatusType: The result of execution
*/
static Hal_StatusType I2c_Hal_MasterPollReadAddress
(
    I2C_Type *ChannelAddress,
    const I2c_Hal_MasterStateType *MasterPtr
)
{
    Hal_StatusType ReturnType = STATUS_SUCCESS;

    if (STATUS_ERROR == I2c_Hal_WaitForReady(ChannelAddress))/* wait ready failed */
    {
        ReturnType = STATUS_ERROR;
    }
    else
    {
        if (STATUS_ERROR == I2c_Hal_Start(ChannelAddress))/* start failed */
        {
            ReturnType = STATUS_ERROR;
        }
        else/* start success */
        {
            if (TRUE == MasterPtr->Is10bitAddr)
            {
                /*PRQA S 2985 ++ # AMCE_QAC_2985.*/
                I2c_Reg_TransmitOneByte(ChannelAddress, ((I2C_ADDEXT_PRIMARY_BYTE_FIX
                                        + (uint8)(MasterPtr->SlaveAddress >> 8U)) << 1U) | (uint8)I2C_WRITE);
                /*PRQA S 2985 -- */
                (void)I2c_Hal_WaitOneByteFinished(ChannelAddress);
                I2c_Reg_TransmitOneByte(ChannelAddress, (uint8)MasterPtr->SlaveAddress);
                /*PRQA S 4432 ++ # AMCE_I2C_4432.*/
                (void)I2c_Hal_WaitOneByteFinished(ChannelAddress);
                ReturnType = I2c_Hal_Start(ChannelAddress);
                /*PRQA S 4432 -- */
                if (STATUS_SUCCESS == ReturnType)
                {
                    I2c_Reg_TxEn(ChannelAddress);
                    I2c_Reg_WriteDataReg(ChannelAddress, ((I2C_ADDEXT_PRIMARY_BYTE_FIX
                                                           + (uint8)(MasterPtr->SlaveAddress >> 8U)) << 1U) \
                                         | (uint8)I2C_READ);
                }
            }
            else
            {
                /* I2c Tx enable */
                I2c_Reg_TxEn(ChannelAddress);
                I2c_Reg_WriteDataReg(ChannelAddress, (uint8)((MasterPtr->SlaveAddress << 1U) | (uint8)I2C_READ));
            }
        }
    }

    return ReturnType;
}

/**
* @brief Start I2C transmission and send Slave write address on poll.
* @note Function ID:DES_I2C_API_255
* @note Service ID: NA
* @param[in] ChannelAddress: I2C ChannelAddress pointer
* @param[in] MasterPtr: I2C master driver pointer
* @return Hal_StatusType: The result of execution
*/
static Hal_StatusType I2c_Hal_MasterPollWriteAddress
(
    I2C_Type *ChannelAddress,
    const I2c_Hal_MasterStateType *MasterPtr
)
{
    Hal_StatusType ReturnType = STATUS_SUCCESS;
    Hal_StatusType TempRet;
    TempRet = I2c_Hal_WaitForReady(ChannelAddress);

    if (STATUS_ERROR == TempRet)/* wait ready failed */
    {
        ReturnType = STATUS_ERROR;
    }
    else
    {
        TempRet = I2c_Hal_Start(ChannelAddress);
        if (STATUS_ERROR == TempRet)/* start failed */
        {
            ReturnType = STATUS_ERROR;
        }
        else/* start success */
        {
            if (TRUE == MasterPtr->Is10bitAddr)
            {
                /*PRQA S 2985 ++ # AMCE_QAC_2985.*/
                I2c_Reg_TransmitOneByte(ChannelAddress, ((I2C_ADDEXT_PRIMARY_BYTE_FIX
                                        + (uint8)(MasterPtr->SlaveAddress >> 8U)) << 1U) | (uint8)I2C_WRITE);
                /*PRQA S 2985 -- */
                TempRet = I2c_Hal_WaitOneByteFinished(ChannelAddress);
                if (STATUS_SUCCESS == TempRet)
                {
                    I2c_Reg_TxEn(ChannelAddress);
                    I2c_Reg_WriteDataReg(ChannelAddress, (uint8)MasterPtr->SlaveAddress);
                }
            }
            else
            {
                /* I2c Tx enable */
                I2c_Reg_TxEn(ChannelAddress);
                /*PRQA S 2985 ++ # AMCE_QAC_2985.*/
                I2c_Reg_WriteDataReg(ChannelAddress, (uint8)((MasterPtr->SlaveAddress << 1U) | (uint8)I2C_WRITE));
                /*PRQA S 2985 -- */
            }
        }
    }

    return ReturnType;
}

/**
* @brief I2C DMA transfer callback function for master.
* @note Function ID:DES_I2C_API_211
* @note Service ID: NA
* @param[in] Args: Function Args
* @return void
*/

static void I2c_Hal_MasterDmaCallback
(
    /*PRQA S 3673 ++ # AMCE_I2C_3673.*/
    void *Args
    /*PRQA S 3673 -- */
)
{
    I2C_Type *ChannelAddress;
    I2c_Hal_MasterStateType *MasterPtr;
    /*PRQA S 0316 ++ # AMCE_QAC_0316.*/
    const Dma_ChannelCBInfoType *DmaInfo = (Dma_ChannelCBInfoType *)Args;
    /*PRQA S 0316 -- */
    /*PRQA S 0326 ++ # AMCE_I2C_0326.*/
    uint32 Instance = (uint32)(DmaInfo->UserArgs);
    /*PRQA S 0326 -- */

    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        MasterPtr = I2cMasterStatePtr[Instance];
        if (0U != (DmaInfo->DmaEvent & DMA_ERROR_EVENT))/* Set status error if an error occurred in DMA channel */
        {
            MasterPtr->Status = I2C_MASTER_CHANNEL_DMA_ERROR;
        }
        else if (0U != (DmaInfo->DmaEvent & DMA_FINISH_EVENT))
        {
            if (I2C_WRITE == MasterPtr->DirType)/* direction is write */
            {
                I2c_Reg_SetDMATx(ChannelAddress, FALSE);
            }
            else/* direction is read */
            {
                I2c_Hal_MasterEndTransfer((uint8)Instance, MasterPtr, MasterPtr->SendStop);
                MasterPtr->Status = I2C_MASTER_CHANNEL_FINISHED;

                if (NULL_PTR != I2cChannelConfigPtr[Instance]->Callback)
                {
                    I2cChannelConfigPtr[Instance]->Callback((uint8)Instance, (uint32)I2C_MASTER_EVENT_END_TRANSFER);
                }
            }
        }
        else
        {
            /*do nothing*/
        }
    }
}

/**
* @brief I2C DMA transfer callback function for master.
* @note Function ID:DES_I2C_API_245
* @note Service ID: NA
* @param[in] Args: Function parameter
* @return void
*/
static void I2c_Hal_SlaveDmaCallback
(
    /*PRQA S 3673 ++ # AMCE_I2C_3673.*/
    void *Args
    /*PRQA S 3673 -- */
)
{
    /*PRQA S 0316 ++ # AMCE_QAC_0316.*/
    const Dma_ChannelCBInfoType *DmaInfo = (Dma_ChannelCBInfoType *)Args;
    /*PRQA S 0316 -- */
    /*PRQA S 0326 ++ # AMCE_I2C_0326.*/
    uint32 Instance = (uint32)(DmaInfo->UserArgs);
    /*PRQA S 0326 -- */

    if (Instance < I2C_INSTANCE_MAX)
    {
        /* Set status error if an error occurred in DMA channel */
        if (0U != (DmaInfo->DmaEvent & DMA_ERROR_EVENT))
        {
            I2cSlaveStatePtr[Instance]->Status = I2C_SLAVE_CHANNEL_DMA_ERROR;
        }
        else if (0U != (DmaInfo->DmaEvent & DMA_FINISH_EVENT))
        {
            I2cSlaveStatePtr[Instance]->Status = I2C_SLAVE_CHANNEL_FINISHED;
        }
        else
        {
            /*do nothing*/
        }
    }
}
/*PRQA S 3673 -- */

/**
* @brief Start DMA transmit for master.
* @note Function ID:DES_I2C_API_213
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @return void
*/
static void I2c_Hal_MasterTransmitDma
(
    uint8 Instance
)
{
    I2C_Type *ChannelAddress;
    Hal_StatusType Status;
    const I2c_Hal_MasterStateType *MasterPtr;
    Dma_TransferConfigType TransferConfig;

    if (Instance < I2C_INSTANCE_MAX)
    {
        MasterPtr = I2cMasterStatePtr[Instance];
        ChannelAddress = I2c_HalBase[Instance];
        TransferConfig.DestStartAddr = (uint32)(&(ChannelAddress->DATA));
        TransferConfig.Length = (uint16)MasterPtr->TxSize;
        TransferConfig.SrcStartAddr = (uint32)MasterPtr->TxBuffPtr;
        TransferConfig.Callback = I2c_Hal_MasterDmaCallback;
        /*PRQA S 0326 ++ # AMCE_I2C_0326.*/
        TransferConfig.UserArgs = (void *)(uint32)Instance;
        /*PRQA S 0326 -- */
        TransferConfig.Type = DMA_TRANSFER_MEM2PERIPH;
        TransferConfig.DestUnit = DMA_TRANSFER_UNIT_1B;
        TransferConfig.SrcUnit = DMA_TRANSFER_UNIT_1B;
        TransferConfig.SrcOffset = 1U;
        TransferConfig.DestOffset = 0U;
        TransferConfig.CircularMode = FALSE;
        TransferConfig.TriggerMode = FALSE;
#if defined (AC7840E)
        TransferConfig.DaisyMode = FALSE;
#endif
        TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
        /*PRQA S 2985 ++ # AMCE_QAC_2985.*/
        TransferConfig.SrcEndAddr = TransferConfig.SrcStartAddr + (uint16)(TransferConfig.Length \
                                    / (uint8)(1U << (uint8)TransferConfig.SrcUnit) * TransferConfig.SrcOffset);
        TransferConfig.DestEndAddr = TransferConfig.DestStartAddr + (uint16)(TransferConfig.Length \
                                     / (uint8)(1U << (uint8)TransferConfig.DestUnit) * TransferConfig.DestOffset);
        /*PRQA S 2985 -- */
        (void)Dma_Hal_ConfigCh(MasterPtr->TxDmaChannel, &TransferConfig);
        Status = Dma_Hal_StartCh(MasterPtr->TxDmaChannel);
        DEVICE_ASSERT(Status != STATUS_ERROR);
        (void)Status;
        /* Set i2c interrupt */
        I2c_Reg_SetInterrupt(ChannelAddress, TRUE);
        I2c_Reg_ClearStatus0(ChannelAddress, (uint32)I2C_STATUS0_RACK_Msk);
        /* Enable NACK interrupt */
        I2c_Reg_SetNackInterrupt(ChannelAddress, TRUE);
        /* Enable I2C DMA TX */
        I2c_Reg_SetDMATx(ChannelAddress, TRUE);
        /* Master send slave address */
        (void)I2c_Hal_MasterSendWriteAddress(ChannelAddress, MasterPtr);
    }
}

/**
* @brief Handle an address valid event for Slave.
* @note Function ID:DES_I2C_API_214
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] SlavePtr: I2C Slave driver pointer
* @return void
*/
static void I2c_Hal_SlaveHandleAddressValidEvent
(
    uint8 Instance,
    I2c_Hal_SlaveStateType *SlavePtr
)
{
    I2C_Type *ChannelAddress;
    uint32 IntStatus0;

    DEVICE_ASSERT(Instance < I2C_INSTANCE_MAX);
    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        IntStatus0 = I2c_Reg_GetStatus0(ChannelAddress);
        /* Clear STATUS0 */
        I2c_Reg_ClearStatus0(ChannelAddress, (uint32)I2C_STATUS0_SAMF_Msk);

        if (NULL_PTR != I2cChannelConfigPtr[Instance]->Callback)
        {
            I2cChannelConfigPtr[Instance]->Callback(Instance, (uint32)I2C_SLAVE_EVENT_ADDRESS_MATCH);
            if (0U != (IntStatus0 & I2C_STATUS0_SRW_Msk))
            {
                I2cChannelConfigPtr[Instance]->Callback(Instance, (uint32)I2C_SLAVE_EVENT_TX_REQ);
            }
            else
            {
                I2cChannelConfigPtr[Instance]->Callback(Instance, (uint32)I2C_SLAVE_EVENT_RX_REQ);
            }
        }

        if (0U != (IntStatus0 & I2C_STATUS0_SRW_Msk))/* check SRW value */
        {
            I2c_Reg_WriteDataReg(ChannelAddress, SlavePtr->TxBuffPtr[0U]);
            if (SlavePtr->TxSize > 0U)
            {
                SlavePtr->TxBuffPtr++;
                SlavePtr->TxSize--;
            }
        }
        else
        {
            if (SlavePtr->RxSize > 0U)/* slave send ack*/
            {
                /* send Ack */
                I2c_Reg_SendAck(ChannelAddress);
            }
            else/* slave send Nack*/
            {
                /* send Nack */
                I2c_Reg_SendNack(ChannelAddress);
            }
        }
        SlavePtr->Status = I2C_SLAVE_CHANNEL_BUSY_TRANSMIT;
    }
}

/**
* @brief Handle a transmit data event for Slave.
* @note Function ID:DES_I2C_API_215
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] SlavePtr: I2C Slave driver pointer
* @return void
*/
static void I2c_Hal_SlaveHandleTransmitDataEvent
(
    uint8 Instance,
    I2c_Hal_SlaveStateType *SlavePtr
)
{
    I2C_Type *ChannelAddress;

    DEVICE_ASSERT(Instance < I2C_INSTANCE_MAX);
    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        if (0U == SlavePtr->TxSize)/* tx size not 0, do transaction */
        {
            /* Clear I2C BND flag */
            I2c_Reg_ClearStatus0(ChannelAddress, (uint32)I2C_STATUS0_BND_Msk);
            /* Out of data, call callback to alLow user to provide a new buffer */
            SlavePtr->Status = I2C_SLAVE_CHANNEL_TX_EMPTY;
            if (NULL_PTR != I2cChannelConfigPtr[Instance]->Callback)
            {
                I2cChannelConfigPtr[Instance]->Callback(Instance, (uint32)I2C_SLAVE_EVENT_TX_EMPTY);
            }
            if (TRUE == SlavePtr->slaveListening)
            {
                /* if slaveListening is enable,Write invalid 0xFF to avoid pull down sda always*/
                I2c_Reg_WriteDataReg(ChannelAddress, 0xFF);
            }
        }
        else
        {
            /* Write data to send */
            I2c_Reg_WriteDataReg(ChannelAddress, SlavePtr->TxBuffPtr[0U]);
            SlavePtr->TxBuffPtr++;
            SlavePtr->TxSize--;
        }
        if (I2c_Reg_IsTxUF(ChannelAddress) != 0U)/* check i2c underflow */
        {
            SlavePtr->Status = I2C_SLAVE_CHANNEL_TX_UNDERRUN;
            I2c_Reg_ClearStatus1(ChannelAddress, (uint32)I2C_STATUS1_TXUF_Msk);
            /* I2C tx is underflow, record tx underflow event and send dummy char */
            I2c_Reg_WriteDataReg(ChannelAddress, 0xFFU);
        }
    }
}

/**
* @brief Handle a receive data event for Slave.
* @note Function ID:DES_I2C_API_216
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] SlavePtr: I2C Slave driver pointer
* @return void
*/
static void I2c_Hal_SlaveHandleReceiveDataEvent
(
    uint8 Instance,
    I2c_Hal_SlaveStateType *SlavePtr
)
{
    I2C_Type *ChannelAddress;

    DEVICE_ASSERT(Instance < I2C_INSTANCE_MAX);
    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        if ((I2c_Reg_IsRxOF(ChannelAddress) != 0U) ||
                (0U == SlavePtr->RxSize) || (SlavePtr->RxPointer >= SlavePtr->RxSize))
        {
            SlavePtr->Status = I2C_SLAVE_CHANNEL_RX_OVERRUN;
            /* clear status1 */
            I2c_Reg_ClearStatus1(ChannelAddress, (uint32)I2C_STATUS1_RXOF_Msk);
            /* I2C rx is overfLow, record rx overfLow event and dummy read data */
            /* Dump read data register */
            (void)I2c_Reg_ReadDataReg(ChannelAddress);
            I2c_Reg_SendNack(ChannelAddress);
        }
        else
        {
            /* send Ack*/
            I2c_Reg_SendAck(ChannelAddress);
            SlavePtr->RxBuffPtr[0U] = I2c_Reg_ReadDataReg(ChannelAddress);
            SlavePtr->RxBuffPtr++;
            SlavePtr->RxPointer++;
        }
    }
}

/**
* @brief Ends current transmission or reception.
* @note Function ID:DES_I2C_API_217
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] SlavePtr: I2C Slave driver pointer
* @return void
*/
static void I2c_Hal_SlaveEndTransfer
(
    uint8 Instance,
    I2c_Hal_SlaveStateType *SlavePtr
)
{
    I2C_Type *ChannelAddress;

    DEVICE_ASSERT(Instance < I2C_INSTANCE_MAX);
    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        /* Disable all interrupt events */
        I2c_Reg_SetInterrupt(ChannelAddress, FALSE);
        if (I2C_USING_DMA == SlavePtr->TransferType)
        {
            /* Stop DMA Channel */
            (void)Dma_Hal_StopCh(SlavePtr->TxDmaChannel);
            (void)Dma_Hal_StopCh(SlavePtr->RxDmaChannel);
            /* Disable I2C DMA request */
            (void)I2c_Reg_SetDMARx(ChannelAddress, FALSE);
            (void)I2c_Reg_SetDMATx(ChannelAddress, FALSE);
        }
    }
    SlavePtr->TxBuffPtr = NULL_PTR;
    SlavePtr->TxSize = 0U;
    SlavePtr->RxBuffPtr = NULL_PTR;
    SlavePtr->RxSize = 0U;
    SlavePtr->RxPointer = 0U;
}

/**
* @brief Handle Slave end transfer operations.
* @note Function ID:DES_I2C_API_218
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] SlavePtr: I2C Slave driver pointer
* @return void
*/
static void I2c_Hal_SlaveEndTransferPoll
(
    uint8 Instance,
    I2c_Hal_SlaveStateType *SlavePtr
)
{
    I2C_Type *ChannelAddress;

    DEVICE_ASSERT(Instance < I2C_INSTANCE_MAX);
    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        /* Disable all interrupt events */
        I2c_Reg_SetInterrupt(ChannelAddress, FALSE);

        SlavePtr->TxBuffPtr = NULL_PTR;
        SlavePtr->TxSize = 0U;
        SlavePtr->RxBuffPtr = NULL_PTR;
        SlavePtr->RxSize = 0U;
        SlavePtr->RxPointer = 0U;

        SlavePtr->Status = I2C_SLAVE_CHANNEL_FINISHED;
        if (NULL_PTR != I2cChannelConfigPtr[Instance]->Callback)
        {
            I2cChannelConfigPtr[Instance]->Callback(Instance, (uint32)I2C_SLAVE_EVENT_STOP);
        }
    }
}

/**
* @brief Handle Slave end transfer operations.
* @note Function ID:DES_I2C_API_218
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] SlavePtr: I2C Slave driver pointer
* @return void
*/
static void I2c_Hal_SlaveEndTransferHandler
(
    uint8 Instance,
    I2c_Hal_SlaveStateType *SlavePtr
)
{
    I2C_Type *ChannelAddress;

    DEVICE_ASSERT(Instance < I2C_INSTANCE_MAX);
    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        /* Stop DMA Channel if Slave is transferring data in DMA mode */
        if (I2C_USING_DMA == SlavePtr->TransferType)
        {
            I2c_Reg_ClearStatus0(ChannelAddress, (uint32)I2C_STATUS0_RACK_Msk);
        }
        if (FALSE == SlavePtr->slaveListening)
        {
            I2c_Hal_SlaveEndTransfer(Instance, SlavePtr);
        }
        SlavePtr->Status = I2C_SLAVE_CHANNEL_FINISHED;
        if (NULL_PTR != I2cChannelConfigPtr[Instance]->Callback)
        {
            I2cChannelConfigPtr[Instance]->Callback(Instance, (uint32)I2C_SLAVE_EVENT_STOP);
        }
    }
}

/**
* @brief Initialize the I2C master mode driver ChannelAddressd on configuration input.
* @note Function ID:DES_I2C_API_219
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] MasterChannelConfigPtr: I2C master user configuration pointer
* @return void
*/
static void I2c_Hal_MasterInit
(
    uint8 Instance,
    const I2c_Hal_MasterConfigType *MasterChannelConfigPtr
)
{
    I2C_Type *ChannelAddress;
    Hal_StatusType ReturnType;
    I2c_Hal_MasterStateType *MasterPtr;
    /* Table to save PCC clock source, for getting the input clcok frequency. */
    static const Ckgen_ClkIdType s_i2cClock[I2C_INSTANCE_MAX] =
    {
        CKGEN_I2C0_CLK,
#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
        CKGEN_I2C1_CLK,
#endif
#if defined (AC7843X)
        CKGEN_I2C2_CLK,
#endif
    };

    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        MasterPtr = I2cMasterStatePtr[Instance];

        /* Initialize driver status structure */
        MasterPtr->RxBuffPtr = NULL_PTR;
        MasterPtr->RxSize = 0U;
        MasterPtr->TxBuffPtr = NULL_PTR;
        MasterPtr->TxSize = 0U;
        MasterPtr->Status = I2C_MASTER_CHANNEL_IDLE;
        MasterPtr->SlaveAddress = MasterChannelConfigPtr->SlaveAddress;
        MasterPtr->SendStop = MasterChannelConfigPtr->SendStop;
        MasterPtr->TransferType = MasterChannelConfigPtr->TransferType;
        /* Store DMA channel number used in transfer */
        MasterPtr->TxDmaChannel = MasterChannelConfigPtr->TxDmaChannel;
        MasterPtr->RxDmaChannel = MasterChannelConfigPtr->RxDmaChannel;
        MasterPtr->RxPointer = 0U;
        MasterPtr->DirType = I2C_WRITE;
        ReturnType = Ckgen_Hal_GetFreq(s_i2cClock[Instance], &MasterPtr->SourceClock);
        DEVICE_ASSERT(ReturnType != STATUS_ERROR);
        (void)ReturnType;
        /* Enable I2c Nvic */
        Core_Hal_EnableIrq(I2c_HalIrqId[Instance]);
        /* Set Slave address */
        MasterPtr->SlaveAddress = MasterChannelConfigPtr->SlaveAddress;
        MasterPtr->Is10bitAddr = MasterChannelConfigPtr->Is10bitAddr;
        /* Set master synchronization, arbitration pltie*/
        I2c_Reg_SetSYNC(ChannelAddress, MasterChannelConfigPtr->SyncEn);
        I2c_Reg_SetARB(ChannelAddress, MasterChannelConfigPtr->Arbitration);
        I2c_Reg_SetPltie(ChannelAddress, MasterChannelConfigPtr->SclSdaLowEn,
                         MasterChannelConfigPtr->PinLow, MasterChannelConfigPtr->TimeCfgTypes);
        /* i2c master/Slave */
        I2c_Reg_SetMSTR(ChannelAddress, I2C_MASTER);
        /* Enable I2c Moudle */
        I2c_Reg_SetModuleEnable(ChannelAddress, TRUE);
        /* Set baud rate */
        ReturnType = I2c_Hal_MasterSetBaudRate(Instance, MasterChannelConfigPtr->BaudRate);
        DEVICE_ASSERT(ReturnType != STATUS_ERROR);
    }
}

/**
* @brief This is a helper function which implements absolute difference between two numbers.
* @note Function ID:DES_I2C_API_220
* @note Service ID: NA
* @param[in] a: The one of number
* @param[in] b: Another one of number
* @return uint32: Difference between two number
*/
static uint32 I2c_Hal_AbsDif
(
    uint32 a,
    uint32 b
)
{
    uint32 RetNum;

    if (a > b)
    {
        RetNum = a - b;
    }
    else
    {
        RetNum = b - a;
    }

    return RetNum;
}

/**
* @brief I2c master mode async send data.
* @note Function ID:DES_I2C_API_221
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] MasterPtr: I2C master driver pointer
* @return Hal_StatusType: The result of execution
*/
static Hal_StatusType I2c_Hal_MasterAsyncSendData
(
    uint8 Instance,
    const I2c_Hal_MasterStateType *MasterPtr
)
{
    DEVICE_ASSERT(Instance < I2C_INSTANCE_MAX);
    Hal_StatusType ReturnType = STATUS_ERROR;
    I2C_Type *ChannelAddress;

    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        if (I2C_USING_DMA == MasterPtr->TransferType)/* use dma mode */
        {
            /* Enbale i2c DMA transmit */
            I2c_Hal_MasterTransmitDma(Instance);
            ReturnType = STATUS_SUCCESS;
        }
        else
        {
            I2c_Reg_SetInterrupt(ChannelAddress, TRUE);
            /* Master send Slave address */
            ReturnType = I2c_Hal_MasterSendWriteAddress(ChannelAddress, MasterPtr);
        }
    }
    return ReturnType;
}

/**
* @brief Perform a non-blocking send transaction on the I2C bus.
* @note Function ID:DES_I2C_API_222
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] TxBuffPtr: Pointer to the data to be transferred
* @param[in] TxSize: Length of the data to be transferred
* @param[in] SendStop: If TRUE generate stop condition after the transmission, otherwise not
* @return Hal_StatusType: The result of execution
*/
static Hal_StatusType I2c_Hal_MasterSendData
(
    uint8 Instance,
    const uint8 *TxBuffPtr,
    uint32 TxSize,
    boolean SendStop
)
{
    I2c_Hal_MasterStateType *MasterPtr;
    Hal_StatusType ReturnType = STATUS_ERROR;
    DEVICE_ASSERT(Instance < I2C_INSTANCE_MAX);
    DEVICE_ASSERT(TxBuffPtr != NULL_PTR);
    DEVICE_ASSERT(TxSize != 0U);

    if (Instance < I2C_INSTANCE_MAX)
    {
        MasterPtr = I2cMasterStatePtr[Instance];
        if ((I2C_MASTER_CHANNEL_BUSY_SEND == MasterPtr->Status) || \
                (I2C_MASTER_CHANNEL_BUSY_RECEIVE == MasterPtr->Status))
        {
            ReturnType = STATUS_BUSY;
        }
        else
        {
            MasterPtr->Status = I2C_MASTER_CHANNEL_BUSY_SEND;
            /* Copy parameters to driver state structure */
            MasterPtr->TxBuffPtr = TxBuffPtr;
            MasterPtr->TxSize = TxSize;
            MasterPtr->SendStop = SendStop;
            MasterPtr->DirType = I2C_WRITE;
            ReturnType = I2c_Hal_MasterAsyncSendData(Instance, MasterPtr);
            if (STATUS_ERROR == ReturnType)
            {
                MasterPtr->Status = I2C_MASTER_CHANNEL_FINISHED;
            }
        }
    }
    return ReturnType;
}

/**
* @brief Perform a non-blocking send transaction on the I2C bus.
* @note Function ID:DES_I2C_API_251
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] TxBuffPtr: Pointer to the data to be transferred
* @param[in] TxSize: Length of the data to be transferred
* @param[in] SendStop: If TRUE generate stop condition after the transmission, otherwise not
* @return Hal_StatusType: The result of execution
*/
static Hal_StatusType I2c_Hal_MasterSendDataPoll
(
    uint8 Instance,
    const uint8 *TxBuffPtr,
    uint32 TxSize,
    boolean SendStop
)
{
    DEVICE_ASSERT(Instance < I2C_INSTANCE_MAX);
    DEVICE_ASSERT(TxBuffPtr != NULL_PTR);
    I2C_Type *ChannelAddress;
    I2c_Hal_MasterStateType *MasterPtr;
    Hal_StatusType ReturnType = STATUS_ERROR;
    uint32 i;
    volatile uint32 Timeout = 0U;
    uint32 Status0;

    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        MasterPtr = I2cMasterStatePtr[Instance];
        MasterPtr->Status = I2C_MASTER_CHANNEL_BUSY_SEND;
        /* Copy parameters to driver state structure */
        MasterPtr->TxBuffPtr = TxBuffPtr;
        MasterPtr->TxSize = TxSize;
        MasterPtr->SendStop = SendStop;
        MasterPtr->DirType = I2C_WRITE;
        /* Master send Slave address */
        ReturnType = I2c_Hal_MasterPollWriteAddress(ChannelAddress, MasterPtr);
    }

    if (STATUS_SUCCESS == ReturnType)
    {
        for (i = 0U; i < TxSize; i++)
        {
            /*PRQA S 3415 ++ # AMCE_I2C_3415.*/
            /*PRQA S 3387 ++ # AMCE_I2C_3387.*/
            I2C_WAIT_BND(ChannelAddress, (Status0), (Timeout));
            /*PRQA S 3387 -- */
            /*PRQA S 3415 -- */
            if (I2C_HW_DEADLINE_TIMEOUT <= Timeout)
            {
                break;
            }
            I2c_Hal_MasterHandleTransmitDataRequest(Instance, MasterPtr);
        }
        /*send stop bit after send TxSize data*/
        if (I2C_HW_DEADLINE_TIMEOUT <= Timeout)
        {
            MasterPtr->Status = I2C_MASTER_CHANNEL_TIMEOUT;
            ReturnType = STATUS_TIMEOUT;
        }
        else
        {
            /*PRQA S 3415 ++ # AMCE_I2C_3415.*/
            /*PRQA S 3387 ++ # AMCE_I2C_3387.*/
            I2C_WAIT_BND(ChannelAddress, (Status0), (Timeout));
            /*PRQA S 3387 -- */
            /*PRQA S 3415 -- */
            MasterPtr->Status = I2C_MASTER_CHANNEL_FINISHED;
        }
        I2c_Hal_MasterEndTransferPoll(Instance, MasterPtr, I2C_WRITE);
    }

    return ReturnType;
}

/**
* @brief Perform a non-blocking receive transaction on the I2C bus.
* @note Function ID:DES_I2C_API_251
* @note Service ID: NA
* @param[in] Instance : The I2C Instance number
* @param[in] RxBuffPtr: Pointer to the buffer where to store received data
* @param[in] RxSize: Length of the data to be transferred
* @param[in] SendStop: Specifies whether or not to generate stop condition after the reception
* @return Hal_StatusType: The result of execution
*/
static Hal_StatusType I2c_Hal_MasterReceiveDataPoll
(
    uint8 Instance,
    uint8 *RxBuffPtr,
    uint32 RxSize,
    boolean SendStop
)
{
    DEVICE_ASSERT(Instance < I2C_INSTANCE_MAX);
    DEVICE_ASSERT(RxBuffPtr != NULL_PTR);
    I2C_Type *ChannelAddress;
    I2c_Hal_MasterStateType *MasterPtr;
    Hal_StatusType ReturnType = STATUS_ERROR;
    uint32 i;
    volatile uint32 Timeout = 0U;
    uint32 Status0;

    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        MasterPtr = I2cMasterStatePtr[Instance];
        MasterPtr->Status = I2C_MASTER_CHANNEL_BUSY_RECEIVE;
        /* Copy parameters to driver state structure */
        MasterPtr->RxBuffPtr = RxBuffPtr;
        MasterPtr->RxSize = RxSize;
        MasterPtr->SendStop = SendStop;
        MasterPtr->DirType = I2C_READ;

        /* Master send Slave address */
        ReturnType = I2c_Hal_MasterPollReadAddress(ChannelAddress, MasterPtr);
    }

    if (STATUS_SUCCESS == ReturnType)
    {
        for (i = 0U; i < RxSize; i++)
        {
            /*PRQA S 3415 ++ # AMCE_I2C_3415.*/
            /*PRQA S 3387 ++ # AMCE_I2C_3387.*/
            I2C_WAIT_BND(ChannelAddress, (Status0), (Timeout));
            /*PRQA S 3387 -- */
            /*PRQA S 3415 -- */
            if (I2C_HW_DEADLINE_TIMEOUT <= Timeout)
            {
                break;
            }
            I2c_Hal_MasterHandleReceiveDataReadyEvent(Instance, MasterPtr);
        }

        if (I2C_HW_DEADLINE_TIMEOUT <= Timeout)
        {
            MasterPtr->Status = I2C_MASTER_CHANNEL_TIMEOUT;
            ReturnType = STATUS_TIMEOUT;
        }
        else
        {
            /*PRQA S 3415 ++ # AMCE_I2C_3415.*/
            /*PRQA S 3387 ++ # AMCE_I2C_3387.*/
            I2C_WAIT_BND(ChannelAddress, (Status0), (Timeout));
            /*PRQA S 3387 -- */
            /*PRQA S 3415 -- */
            MasterPtr->Status = I2C_MASTER_CHANNEL_FINISHED;
        }
        I2c_Hal_MasterEndTransferPoll(Instance, MasterPtr, I2C_READ);
    }

    return ReturnType;
}

/**
* @brief Start I2C transmission and send Slave write address.
* @note Function ID:DES_I2C_API_223
* @note Service ID: NA
* @param[in] ChannelAddress: I2C ChannelAddress pointer
* @param[in] MasterPtr: I2C master driver pointer
* @return Hal_StatusType: The result of execution
*/
static Hal_StatusType I2c_Hal_MasterDmaWriteAddress
(
    I2C_Type *ChannelAddress,
    const I2c_Hal_MasterStateType *MasterPtr
)
{
    Hal_StatusType ReturnType = STATUS_ERROR;
    Hal_StatusType RetTmp;

    RetTmp = I2c_Hal_WaitForReady(ChannelAddress);
    if ((STATUS_ERROR == RetTmp) && (MasterPtr == NULL_PTR))
    {
        ReturnType = STATUS_ERROR;
    }
    else
    {
        RetTmp = I2c_Hal_Start(ChannelAddress);
        if (STATUS_ERROR == RetTmp)/* start failed */
        {
            ReturnType = STATUS_ERROR;
        }
        else/* start success */
        {
            if (TRUE == MasterPtr->Is10bitAddr)
            {
                /*PRQA S 2985 ++ # AMCE_QAC_2985.*/
                I2c_Reg_TransmitOneByte(ChannelAddress, (uint8)((I2C_ADDEXT_PRIMARY_BYTE_FIX
                                        + (MasterPtr->SlaveAddress >> 8U)) << 1U) | (uint8)I2C_WRITE);
                /*PRQA S 2985 -- */
                (void)I2c_Hal_WaitOneByteFinished(ChannelAddress);
                I2c_Reg_TransmitOneByte(ChannelAddress, (uint8)MasterPtr->SlaveAddress);
                /*PRQA S 4432 ++ # AMCE_I2C_4432.*/
                (void)I2c_Hal_WaitOneByteFinished(ChannelAddress);
                (void)I2c_Hal_Start(ChannelAddress);
                I2c_Reg_TransmitOneByte(ChannelAddress, ((I2C_ADDEXT_PRIMARY_BYTE_FIX
                                        + (uint8)(MasterPtr->SlaveAddress >> 8U)) << 1U) | (uint8)I2C_READ);
                ReturnType = I2c_Hal_WaitOneByteFinished(ChannelAddress);
                /*PRQA S 4432 -- */
            }
            else
            {
                I2c_Reg_TxEn(ChannelAddress);
                I2c_Reg_WriteDataReg(ChannelAddress, (uint8)((MasterPtr->SlaveAddress << 1U) | (uint8)I2C_READ));
                ReturnType = I2c_Hal_WaitOneByteFinished(ChannelAddress);
            }
            if (STATUS_SUCCESS == ReturnType)/* transaction one byte error */
            {
                /* Rx Enable */
                I2c_Reg_RxEn(ChannelAddress);
                /* Dump read data register */
                (void)I2c_Reg_ReadDataReg(ChannelAddress);
            }
            else
            {
                I2c_Reg_SendStop(ChannelAddress);
            }
        }
    }

    return ReturnType;
}

/**
* @brief Start DMA transmit for master.
* @note Function ID:DES_I2C_API_224
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @return void
*/
static void I2c_Hal_MasterReceiveDma
(
    uint8 Instance
)
{
    I2C_Type *ChannelAddress;
    Hal_StatusType Status;
    const I2c_Hal_MasterStateType *MasterPtr;
    Dma_TransferConfigType TransferConfig;

    DEVICE_ASSERT(Instance < I2C_INSTANCE_MAX);

    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        MasterPtr = I2cMasterStatePtr[Instance];
        TransferConfig.DestStartAddr = (uint32)MasterPtr->RxBuffPtr;
        TransferConfig.Length = (uint16)MasterPtr->RxSize;
        TransferConfig.SrcStartAddr = (uint32)(&(ChannelAddress->DATA));
        TransferConfig.Callback = I2c_Hal_MasterDmaCallback;
        /*PRQA S 0326 ++ # AMCE_I2C_0326.*/
        TransferConfig.UserArgs = (void *)(uint32)Instance;
        /*PRQA S 0326 -- */
        TransferConfig.Type = DMA_TRANSFER_PERIPH2MEM;
        TransferConfig.DestUnit = DMA_TRANSFER_UNIT_1B;
        TransferConfig.SrcUnit = DMA_TRANSFER_UNIT_1B;
        TransferConfig.SrcOffset = 0U;
        TransferConfig.DestOffset = 1U;
        TransferConfig.CircularMode = FALSE;
        TransferConfig.TriggerMode = FALSE;
#if defined (AC7840E)
        TransferConfig.DaisyMode = FALSE;
#endif
        TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
        /* PRQA S 2985 ++ # AMCE_QAC_2985*/
        TransferConfig.SrcEndAddr = TransferConfig.SrcStartAddr + (uint16)(TransferConfig.Length \
                                    / (uint8)(1U << (uint8)TransferConfig.SrcUnit) * TransferConfig.SrcOffset);
        TransferConfig.DestEndAddr = TransferConfig.DestStartAddr + (uint16)(TransferConfig.Length \
                                     / (uint8)(1U << (uint8)TransferConfig.DestUnit) * TransferConfig.DestOffset);
        /* PRQA S 2985 -- */
        (void)Dma_Hal_ConfigCh(MasterPtr->RxDmaChannel, &TransferConfig);
        Status = Dma_Hal_StartCh(MasterPtr->RxDmaChannel);
        DEVICE_ASSERT(Status != STATUS_ERROR);
        (void)Status;
        /* Set i2c interrupt */
        I2c_Reg_SetInterrupt(ChannelAddress, TRUE);
        I2c_Reg_ClearStatus0(ChannelAddress, (uint32)I2C_STATUS0_RACK_Msk);
        /* Enable NACK interrupt */
        I2c_Reg_SetNackInterrupt(ChannelAddress, (boolean)TRUE);
        /* Enable I2C DMA RX */
        I2c_Reg_SetDMARx(ChannelAddress, TRUE);
        /* Master send slave address */
        (void)I2c_Hal_MasterDmaWriteAddress(ChannelAddress, MasterPtr);
    }
}

/**
* @brief I2c master mode async receive data.
* @note Function ID:DES_I2C_API_225
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] MasterPtr: master mode channel config
* @return Hal_StatusType: The result of execution
*/
static Hal_StatusType I2c_Hal_MasterAsyncReceiveData
(
    uint8 Instance,
    const I2c_Hal_MasterStateType *MasterPtr
)
{
    I2C_Type *ChannelAddress;
    Hal_StatusType ReturnType = STATUS_ERROR;

    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        if (I2C_USING_DMA == MasterPtr->TransferType)/* use dma mode */
        {
            /* Enbale i2c DMA receive */
            I2c_Hal_MasterReceiveDma(Instance);
            ReturnType = (Hal_StatusType)STATUS_SUCCESS;
        }
        else
        {
            I2c_Reg_SetInterrupt(ChannelAddress, TRUE);
            /* Master send Slave address */
            ReturnType = I2c_Hal_MasterSendReadAddress(ChannelAddress, MasterPtr);
        }
    }

    return ReturnType;
}

/**
* @brief Perform a non-blocking receive transaction on the I2C bus.
* @note Function ID:DES_I2C_API_226
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] RxBuffPtr: Pointer to the buffer where to store received data
* @param[in] RxSize: Length of the data to be transferred
* @param[in] SendStop: Specifies whether or not to generate stop condition after the reception
* @return Hal_StatusType: The result of execution
*/
static Hal_StatusType I2c_Hal_MasterReceiveData
(
    uint8 Instance,
    uint8 *RxBuffPtr,
    uint32 RxSize,
    boolean SendStop
)
{
    I2c_Hal_MasterStateType *MasterPtr;
    Hal_StatusType ReturnType = STATUS_ERROR;
    DEVICE_ASSERT(Instance < I2C_INSTANCE_MAX);
    DEVICE_ASSERT(RxBuffPtr != NULL_PTR);
    DEVICE_ASSERT(RxSize != 0U);

    if (Instance < I2C_INSTANCE_MAX)
    {
        MasterPtr = I2cMasterStatePtr[Instance];
        if ((I2C_MASTER_CHANNEL_BUSY_SEND == MasterPtr->Status) || \
                (I2C_MASTER_CHANNEL_BUSY_RECEIVE == MasterPtr->Status))
        {
            ReturnType = STATUS_BUSY;
        }
        else
        {
            MasterPtr->Status = I2C_MASTER_CHANNEL_BUSY_RECEIVE;
            /* Copy parameters to driver state structure */
            MasterPtr->RxBuffPtr = RxBuffPtr;
            MasterPtr->RxSize = RxSize;
            MasterPtr->SendStop = SendStop;
            MasterPtr->DirType = I2C_READ;
            ReturnType = I2c_Hal_MasterAsyncReceiveData(Instance, MasterPtr);
            if (STATUS_ERROR == ReturnType)
            {
                MasterPtr->Status = I2C_MASTER_CHANNEL_FINISHED;
            }
        }
    }
    return ReturnType;
}

/**
* @brief Initialize the I2C Slave mode driver.
* @note Function ID:DES_I2C_API_228
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] SlaveConfigPtr: Pointer to the I2C Slave user configuration structure
* @return void
*/
static void I2c_Hal_SlaveInit
(
    uint8 Instance,
    const I2c_Hal_SlaveConfigType *SlaveConfigPtr
)
{
    I2C_Type *ChannelAddress;
    I2c_Hal_SlaveStateType *SlavePtr;

    DEVICE_ASSERT(Instance < I2C_INSTANCE_MAX);
    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        SlavePtr = I2cSlaveStatePtr[Instance];
        /* Initialize driver status structure */
        SlavePtr->RxBuffPtr = NULL_PTR;
        SlavePtr->TxSize = 0U;
        SlavePtr->RxPointer = 0U;
        SlavePtr->TxBuffPtr = NULL_PTR;
        SlavePtr->RxSize = 0U;
        SlavePtr->Status = I2C_SLAVE_CHANNEL_IDLE;
        SlavePtr->TransferType = SlaveConfigPtr->TransferType;
        /* Store DMA channel number used in transfer */
        SlavePtr->TxDmaChannel = SlaveConfigPtr->TxDmaChannel;
        SlavePtr->RxDmaChannel = SlaveConfigPtr->RxDmaChannel;
        SlavePtr->slaveListening = SlaveConfigPtr->slaveListening;
        /* Enable I2c Nvic */
        Core_Hal_EnableIrq(I2c_HalIrqId[Instance]);
        /* Set Slave address */
        I2c_Reg_SetSlaveAddr(ChannelAddress, SlaveConfigPtr->SlaveAddress);
        if (TRUE == SlaveConfigPtr->Is10bitAddr)
        {
            /* Set 10bit address */
            I2C_Reg_SetADEXT(ChannelAddress, TRUE);
        }
        /* Slave wakeup setting */
#if !defined(AC7840E)
        if (TRUE == SlaveConfigPtr->WakeupEn)
        {
            I2c_Reg_SetWakeup(ChannelAddress, TRUE);
        }
#endif
        /* Slave stretch */
        I2c_Reg_SetStretch(ChannelAddress, SlaveConfigPtr->StretchEn);
        /* i2c master/slave */
        I2c_Reg_SetMSTR(ChannelAddress, I2C_SLAVE);
        if (TRUE == SlaveConfigPtr->slaveListening)
        {
            I2c_Reg_SetInterrupt(ChannelAddress, TRUE);
            /* i2c start stop int */
            I2c_Reg_SetSSInterrupt(ChannelAddress, TRUE);
            /* i2c module enable */
            I2c_Reg_SetModuleEnable(ChannelAddress, TRUE);
        }
    }
}

/**
* @brief Start DMA receive for slave.
* @note Function ID:DES_I2C_API_229
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @return void
*/
static void I2c_Hal_SlaveTransmitDma
(
    uint8 Instance
)
{
    I2C_Type *ChannelAddress;
    const I2c_Hal_SlaveStateType *SlavePtr;
    Dma_TransferConfigType TransferConfig;

    DEVICE_ASSERT(Instance < I2C_INSTANCE_MAX);
    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];

        SlavePtr = I2cSlaveStatePtr[Instance];
        I2c_Reg_ClearStatus0(ChannelAddress, (uint32)I2C_STATUS0_RACK_Msk);
        /* Enable NACK interrupt */
        I2c_Reg_SetNackInterrupt(ChannelAddress, TRUE);
        /* Enable I2C DMA TX */
        I2c_Reg_SetDMATx(ChannelAddress, TRUE);
        TransferConfig.DestStartAddr = (uint32)(&(ChannelAddress->DATA));
        TransferConfig.Length = (uint16)SlavePtr->TxSize;
        TransferConfig.SrcStartAddr = (uint32)SlavePtr->TxBuffPtr;
        TransferConfig.Callback = I2c_Hal_SlaveDmaCallback;
        /*PRQA S 0326 ++ # AMCE_I2C_0326.*/
        TransferConfig.UserArgs = (void *)(uint32)Instance;
        /*PRQA S 0326 -- */
        TransferConfig.Type = DMA_TRANSFER_MEM2PERIPH;
        TransferConfig.DestUnit = DMA_TRANSFER_UNIT_1B;
        TransferConfig.SrcUnit = DMA_TRANSFER_UNIT_1B;
        TransferConfig.SrcOffset = 1U;
        TransferConfig.DestOffset = 0U;
        TransferConfig.CircularMode = FALSE;
        TransferConfig.TriggerMode = FALSE;
#if defined (AC7840E)
        TransferConfig.DaisyMode = FALSE;
#endif
        TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
        /* PRQA S 2985 ++ # AMCE_QAC_2985 */
        TransferConfig.SrcEndAddr = TransferConfig.SrcStartAddr + (uint16)(TransferConfig.Length \
                                    / (uint8)(1U << (uint8)TransferConfig.SrcUnit) * TransferConfig.SrcOffset);
        TransferConfig.DestEndAddr = TransferConfig.DestStartAddr + (uint16)(TransferConfig.Length \
                                     / (uint8)(1U << (uint8)TransferConfig.DestUnit) * TransferConfig.DestOffset);
        /* PRQA S 2985 -- */
        (void)Dma_Hal_ConfigCh(SlavePtr->TxDmaChannel, &TransferConfig);
        (void)Dma_Hal_StartCh(SlavePtr->TxDmaChannel);
    }
}

/**
* @brief Start DMA receive for slave.
* @note Function ID:DES_I2C_API_231
* @note Service ID: NA
* @param[in] Instance : The I2C Instance number
* @return void
*/
static void I2c_Hal_SlaveReceiveDma
(
    uint8 Instance
)
{
    I2C_Type *ChannelAddress;
    const I2c_Hal_SlaveStateType *SlavePtr;
    Dma_TransferConfigType TransferConfig;

    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        SlavePtr = I2cSlaveStatePtr[Instance];
        /* Enable I2C DMA RX */
        I2c_Reg_SetDMARx(ChannelAddress, TRUE);
        TransferConfig.DestStartAddr = (uint32)SlavePtr->RxBuffPtr;
        TransferConfig.Length = (uint16)SlavePtr->RxSize;
        TransferConfig.SrcStartAddr = (uint32)(&(ChannelAddress->DATA));
        TransferConfig.Callback = I2c_Hal_SlaveDmaCallback;
        /*PRQA S 0326 ++ # AMCE_I2C_0326.*/
        TransferConfig.UserArgs = (void *)(uint32)Instance;
        /*PRQA S 0326 -- */
        TransferConfig.Type = DMA_TRANSFER_PERIPH2MEM;
        TransferConfig.DestUnit = DMA_TRANSFER_UNIT_1B;
        TransferConfig.SrcUnit = DMA_TRANSFER_UNIT_1B;
        TransferConfig.SrcOffset = 0U;
        TransferConfig.DestOffset = 1U;
        TransferConfig.CircularMode = FALSE;
        TransferConfig.TriggerMode = FALSE;
#if defined (AC7840E)
        TransferConfig.DaisyMode = FALSE;
#endif
        TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
        /* PRQA S 2985 ++ # AMCE_QAC_2985 */
        TransferConfig.SrcEndAddr = TransferConfig.SrcStartAddr + (uint16)(TransferConfig.Length \
                                    / (uint8)(1U << (uint8)TransferConfig.SrcUnit) * TransferConfig.SrcOffset);
        TransferConfig.DestEndAddr = TransferConfig.DestStartAddr + (uint16)(TransferConfig.Length \
                                     / (uint8)(1U << (uint8)TransferConfig.DestUnit) * TransferConfig.DestOffset);
        /* PRQA S 2985 -- */
        (void)Dma_Hal_ConfigCh(SlavePtr->RxDmaChannel, &TransferConfig);
        (void)Dma_Hal_StartCh(SlavePtr->RxDmaChannel);
    }
}

/**
* @brief Start DMA for slave.
* @note Function ID:DES_I2C_API_234
* @note Service ID: NA
* @param[in] Instance : I2C channel to be HwUnit
* @return void
*/
static void I2c_Hal_SlaveStartDmaTransfer(uint8 Instance)
{
    const I2c_Hal_SlaveStateType *SlavePtr;
    if (Instance < I2C_INSTANCE_MAX)
    {
        SlavePtr = I2cSlaveStatePtr[Instance];
        if (SlavePtr->TxSize > 0U)/* TxSize not 0 means write */
        {
            I2c_Hal_SlaveTransmitDma(Instance);
        }
        else/* TxSize not 0 means read */
        {
            I2c_Hal_SlaveReceiveDma(Instance);
        }
    }
}
/**
* @brief Perform a non-blocking send transaction on the I2C bus.
* @note Function ID:DES_I2C_API_230
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] TxBuffPtr: Pointer to the data to be transferred
* @param[in] TxSize: Length of the data to be transferred
* @return Hal_StatusType:The result of execution
*/
static Hal_StatusType I2c_Hal_SlaveSendData
(
    uint8 Instance,
    const uint8 *TxBuffPtr,
    uint32 TxSize
)
{
    DEVICE_ASSERT(TxBuffPtr != NULL_PTR);
    I2C_Type *ChannelAddress;
    I2c_Hal_SlaveStateType *SlavePtr;
    Hal_StatusType ReturnType = STATUS_ERROR;

    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        SlavePtr = I2cSlaveStatePtr[Instance];
        /* If the slave is in listening mode the user should not use this function or blocking counterpart */
        DEVICE_ASSERT(SlavePtr->slaveListening == FALSE);
        /* Check if Slave is busy */
        if ((I2C_SLAVE_CHANNEL_BUSY_SEND == SlavePtr->Status) || (I2C_SLAVE_CHANNEL_BUSY_RECEIVE == SlavePtr->Status))
        {
            ReturnType = STATUS_BUSY;
        }
        else
        {
            SlavePtr->Status = I2C_SLAVE_CHANNEL_BUSY_SEND;
            SlavePtr->TxBuffPtr = TxBuffPtr;
            SlavePtr->TxSize = TxSize;
            /* i2c start stop int */
            I2c_Reg_ClearStopFlag(ChannelAddress);
            I2c_Reg_SetSSInterrupt(ChannelAddress, TRUE);
            I2c_Reg_SetInterrupt(ChannelAddress, TRUE);
            /* i2c module ENABLE */
            I2c_Reg_SetModuleEnable(ChannelAddress, TRUE);
            if (I2C_USING_DMA == SlavePtr->TransferType)
            {
                I2c_Hal_SlaveStartDmaTransfer(Instance);
            }
            ReturnType = STATUS_SUCCESS;
        }
    }

    return ReturnType;
}

/**
* @brief Perform a non-blocking receive transaction on the I2C bus.
* @note Function ID:DES_I2C_API_232
* @note Service ID: NA
* @param[in] Instance : The I2C Instance number
* @param[in] RxBuffPtr: Pointer to the buffer where to store received data
* @param[in] RxSize: Length of the data to be transferred
* @return Hal_StatusType: The result of execution
*/
static Hal_StatusType I2c_Hal_SlaveReceiveData
(
    uint8 Instance,
    uint8 *RxBuffPtr,
    uint32 RxSize
)
{
    DEVICE_ASSERT(RxBuffPtr != NULL_PTR);
    I2C_Type *ChannelAddress;
    I2c_Hal_SlaveStateType *SlavePtr;
    Hal_StatusType ReturnType = STATUS_ERROR;

    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        SlavePtr = I2cSlaveStatePtr[Instance];
        /* If the slave is in listening mode the user should not use this function or blocking counterpart */
        DEVICE_ASSERT(SlavePtr->slaveListening == FALSE);
        /* Check if Slave is busy */
        if ((I2C_SLAVE_CHANNEL_BUSY_SEND == SlavePtr->Status) || (I2C_SLAVE_CHANNEL_BUSY_RECEIVE == SlavePtr->Status))
        {
            ReturnType = STATUS_BUSY;
        }
        else
        {
            SlavePtr->Status = I2C_SLAVE_CHANNEL_BUSY_RECEIVE;
            SlavePtr->RxBuffPtr = RxBuffPtr;
            SlavePtr->RxSize = RxSize;
            /* i2c start stop int */
            I2c_Reg_ClearStopFlag(ChannelAddress);
            I2c_Reg_SetSSInterrupt(ChannelAddress, TRUE);
            I2c_Reg_SetInterrupt(ChannelAddress, TRUE);
            /* i2c module enable */
            I2c_Reg_SetModuleEnable(ChannelAddress, TRUE);
            if (I2C_USING_DMA == SlavePtr->TransferType)
            {
                I2c_Hal_SlaveStartDmaTransfer(Instance);
            }
            ReturnType = STATUS_SUCCESS;
        }
    }

    return ReturnType;
}

/**
* @brief Perform poll send transaction on the I2C bus.
* @note Function ID:DES_I2C_API_252
* @note Service ID: NA
* @param[in] Instance: The I2C Instance number
* @param[in] BuffPtr: Pointer to the data to be transferred
* @param[in] Size: Length of the data to be transferred
* @param[in] DirType: I2C_WRITE or I2C_READ
* @return Hal_StatusType:The result of execution
*/
static Hal_StatusType I2c_Hal_SlaveTransferPoll
(
    uint8 Instance,
    uint8 *BuffPtr,
    uint32 Size,
    I2c_Hal_DirType DirType
)
{
    DEVICE_ASSERT(BuffPtr != NULL_PTR);

    I2C_Type *ChannelAddress;
    I2c_Hal_SlaveStateType *SlavePtr;
    Hal_StatusType ReturnType = STATUS_ERROR;
    volatile uint32 Timeout = 0U;
    uint32 Status0;
    uint32 i;

    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        SlavePtr = I2cSlaveStatePtr[Instance];
        /* Check if Slave is busy */
        if ((I2C_SLAVE_CHANNEL_BUSY_SEND == SlavePtr->Status) || (I2C_SLAVE_CHANNEL_BUSY_RECEIVE == SlavePtr->Status))
        {
            ReturnType = STATUS_BUSY;
        }
        else
        {
            SlavePtr->TxBuffPtr = BuffPtr;
            SlavePtr->TxSize = Size;
            SlavePtr->RxBuffPtr = BuffPtr;
            SlavePtr->RxSize = Size;
            /* i2c start stop int */
            I2c_Reg_SetSSInterrupt(ChannelAddress, TRUE);
            /* i2c module ENABLE */
            I2c_Reg_SetModuleEnable(ChannelAddress, TRUE);
            /*PRQA S 3387 ++ # AMCE_I2C_3387.*/
            I2C_WAIT_STATUS(ChannelAddress, (Status0), (I2C_STATUS0_SAMF_Msk));
            /*PRQA S 3387 -- */
            /* Address match && Clear STATUS0*/
            I2c_Reg_ClearStatus0(ChannelAddress, (uint32)I2C_STATUS0_SAMF_Msk);
            I2c_Reg_ClearStartFlag(ChannelAddress);

            if (NULL_PTR != I2cChannelConfigPtr[Instance]->Callback)
            {
                I2cChannelConfigPtr[Instance]->Callback(Instance, (uint32)I2C_SLAVE_EVENT_ADDRESS_MATCH);
                if (DirType == I2C_WRITE)
                {
                    I2cChannelConfigPtr[Instance]->Callback(Instance, (uint32)I2C_SLAVE_EVENT_TX_REQ);
                }
                else
                {
                    I2cChannelConfigPtr[Instance]->Callback(Instance, (uint32)I2C_SLAVE_EVENT_RX_REQ);
                }
            }

            if (DirType == I2C_WRITE)/* direction is write */
            {
                I2c_Reg_WriteDataReg(ChannelAddress, SlavePtr->TxBuffPtr[0U]);
                SlavePtr->TxBuffPtr++;
                SlavePtr->TxSize--;
            }
            else
            {
                if (SlavePtr->RxSize > 0U)/* slave send ack*/
                {
                    /* send Ack */
                    I2c_Reg_SendAck(ChannelAddress);
                }
                else/* slave send Nack*/
                {
                    /* send Nack */
                    I2c_Reg_SendNack(ChannelAddress);
                }
            }

            for (i = 0U; i < Size; i++)
            {
                /*PRQA S 3387 ++ # AMCE_I2C_3387.*/
                I2C_WAIT_STATUS(ChannelAddress, (Status0), (I2C_STATUS0_BND_Msk));
                /*PRQA S 3387 -- */
                /* Check slave transfer direction */
                if (DirType == I2C_WRITE)
                {
                    I2c_Hal_SlaveHandleTransmitDataEvent(Instance, SlavePtr);
                }
                else
                {
                    I2c_Hal_SlaveHandleReceiveDataEvent(Instance, SlavePtr);
                }
            }

            /* wait stop andr clear it */
            /*PRQA S 3415 ++ # AMCE_I2C_3415.*/
            while ((0U == I2c_Reg_IsStop(ChannelAddress)) && (Timeout < I2C_HW_DEADLINE_TIMEOUT))
            /*PRQA S 3415 -- */
            {
                Timeout++; //PRQA S 3387 # AMCE_I2C_3387
            }

            if (I2C_HW_DEADLINE_TIMEOUT <= Timeout)
            {
                ReturnType = STATUS_TIMEOUT;
            }
            else
            {
                ReturnType = STATUS_SUCCESS;
            }

            I2c_Reg_ClearStopFlag(ChannelAddress);
            I2c_Hal_SlaveEndTransferPoll(Instance, SlavePtr);
        }
    }

    return ReturnType;
}

/**
* @brief Initializes the I2C module.
* @note Function ID:[DES_I2C_API_200]
* @note Service ID: NA
* @param[in] Instance : I2C hardware channel ID
* @param[in] ChannelConfigPtr : Pointer to I2c_ChannelConfigType
* @return Hal_StatusType: STATUS_SUCCESS  Initializes Success
                          STATUS_ERROR Initializes Failed
*/
void I2c_Hal_Init
(
    uint8 Instance,
    const I2c_Hal_ChannelConfigType *ChannelConfigPtr
)
{
    DEVICE_ASSERT(ChannelConfigPtr != NULL_PTR);
    if ((Instance < I2C_INSTANCE_MAX) && (NULL_PTR != ChannelConfigPtr))
    {
        /* Clear global variable */
        I2cChannelConfigPtr[Instance]->I2cMode = ChannelConfigPtr->I2cMode;
        I2cChannelConfigPtr[Instance]->Callback = ChannelConfigPtr->Callback;

        /* enable i2c channel clock */
        (void)Ckgen_Hal_EnablePeriphClk(I2c_HalClock[Instance], TRUE);
        Rcm_Hal_SetResetState(I2c_HalClockReset[Instance], RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState(I2c_HalClockReset[Instance], RCM_RESET_STATE_DEASSERT);
        if (I2C_MASTER == ChannelConfigPtr->I2cMode)
        {
            I2c_Hal_MasterInit(Instance, ChannelConfigPtr->MasterConfigPtr);
        }
        else
        {
            I2c_Hal_SlaveInit(Instance, ChannelConfigPtr->SlaveConfigPtr);
        }
    }
}

/**
* @brief DeInitializes the I2C module.
* @note Function ID:[DES_I2C_API_201]
* @note Service ID: NA
* @param[in] Instance : I2C hardware channel ID
* @return void
*/
void I2c_Hal_DeInit
(
    uint8 Instance
)
{
    if (Instance < I2C_INSTANCE_MAX)
    {
        /* Disable I2C module interrupt */
        Core_Hal_DisableIrq(I2c_HalIrqId[Instance]);
        if (I2C_MASTER == I2cChannelConfigPtr[Instance]->I2cMode)
        {
            /* clear materStatePtr */
            I2cMasterStatePtr[Instance]->TxSize = 0U;
            I2cMasterStatePtr[Instance]->RxSize = 0U;
            I2cMasterStatePtr[Instance]->RxPointer = 0U;
            I2cMasterStatePtr[Instance]->TxBuffPtr = NULL_PTR;
            I2cMasterStatePtr[Instance]->RxBuffPtr = NULL_PTR;
            I2cMasterStatePtr[Instance]->Status = I2C_MASTER_CHANNEL_IDLE;
            I2cMasterStatePtr[Instance]->SourceClock = 0U;
            I2cMasterStatePtr[Instance]->Is10bitAddr = FALSE;
        }
        else
        {
            /* clear slaveStatePtr */
            I2cSlaveStatePtr[Instance]->TxSize = 0U;
            I2cSlaveStatePtr[Instance]->RxSize = 0U;
            I2cSlaveStatePtr[Instance]->RxPointer = 0U;
            I2cSlaveStatePtr[Instance]->TxBuffPtr = NULL_PTR;
            I2cSlaveStatePtr[Instance]->RxBuffPtr = NULL_PTR;
            I2cSlaveStatePtr[Instance]->Status = I2C_SLAVE_CHANNEL_IDLE;
        }
        /* Clear ChannleConfigPtr */
        I2cChannelConfigPtr[Instance]->Callback = NULL_PTR;
        I2cChannelConfigPtr[Instance]->MasterConfigPtr = NULL_PTR;
        I2cChannelConfigPtr[Instance]->SlaveConfigPtr = NULL_PTR;
        /* Set I2C module at reset state and disable SPI bus clock. */
        Rcm_Hal_SetResetState(I2c_HalClockReset[Instance], RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState(I2c_HalClockReset[Instance], RCM_RESET_STATE_DEASSERT);
        /* Disable i2c channel clock */
        (void)Ckgen_Hal_EnablePeriphClk(I2c_HalClock[Instance], FALSE);
        Core_Hal_ClearPendingIrq(I2c_HalIrqId[Instance]);
    }
}

/**
* @brief Get the currently configured baud rate.
* @note Function ID: DES_I2C_API_240
* @note Service ID: NA
* @param[in] Instance : I2C hardware channel ID
* @return uint32:Get the currently baud rate
*
*/
uint32 I2c_Hal_MasterGetBaudRate
(
    uint8 Instance
)
{
    const I2C_Type *ChannelAddress;
    const I2c_Hal_MasterStateType *MasterPtr;
    uint8 sampleCnt;
    uint8 stepCnt;
    uint32 BaudRate = 0U;

    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        MasterPtr = I2cMasterStatePtr[Instance];
        stepCnt = I2C_Reg_GetStepCnt(ChannelAddress);
        sampleCnt = I2C_Reg_GetSampleCnt(ChannelAddress);
        /* PRQA S 1891 ++ #essentially unsigned type is being implicitly*/
        BaudRate = MasterPtr->SourceClock / ((stepCnt + 1U) * (sampleCnt + 1U) * 2U);
        /* PRQA S 1891 -- */
    }

    return BaudRate;
}

/**
* @brief Set the currently configured baud rate.
* @note Function ID: DES_I2C_API_204
* @note Service ID: NA
* @param[in] Instance : I2C hardware channel ID
* @param[in] BaudRate: I2C channel Set baudRate
* @return Hal_StatusType: STATUS_SUCCESS  Set the currently configured baud rate. Success
*                         STATUS_ERROR Set the currently configured baud rate Failed
*/
Hal_StatusType I2c_Hal_MasterSetBaudRate
(
    uint8 Instance,
    uint32 BaudRate
)
{
    I2C_Type *ChannelAddress;
    const I2c_Hal_MasterStateType *MasterPtr;
    Hal_StatusType ReturnType = STATUS_SUCCESS;
    uint8 SampleCnt;
    uint8 StepCnt;
    uint32 InputClock;
    uint32 BestBaudRate = 0xFFFFFFFFU;
    uint32 BestStep = 0U;
    uint32 BestSample = 0U;
    uint32 Freq1;
    uint32 Freq2;
    uint32 LowNum, HighNum;
    uint32 TempBestBaud;
    uint32 TempBestSample;

    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        MasterPtr = I2cMasterStatePtr[Instance];
        InputClock = MasterPtr->SourceClock;
        if (BaudRate != 0U)
        {
            for (StepCnt = 3U; StepCnt < 255U; StepCnt++)
            {
                LowNum = 1U;
                HighNum = 255U;
                /* Implement golden section search algorithm */
                do
                {
                    SampleCnt = (uint8)((LowNum + HighNum) / 2U);
                    Freq1 = InputClock / ((StepCnt + 1UL) * (SampleCnt + (uint32)1U) * ((uint32)2U));

                    if (I2c_Hal_AbsDif(BaudRate, BestBaudRate) > I2c_Hal_AbsDif(BaudRate, Freq1))
                    {
                        BestBaudRate = Freq1;
                    }
                    if (Freq1 < BaudRate)
                    {
                        HighNum = SampleCnt;
                    }
                    else
                    {
                        LowNum = SampleCnt;
                    }
                }
                while ((HighNum - LowNum) > 1U);

                /* Evaluate last 2 scaler values */
                Freq1 = InputClock / ((StepCnt + (uint32)1U) * (LowNum + (uint32)1U) * (uint32)2U);
                Freq2 = InputClock / ((StepCnt + (uint32)1U) * (HighNum + (uint32)1U) * (uint32)2U);
                uint32 Freq1Rte = I2c_Hal_AbsDif(BaudRate, Freq1);
                uint32 Freq2Rte = I2c_Hal_AbsDif(BaudRate, Freq2);
                /* select best baudRate */
                if (Freq1Rte > Freq2Rte)
                {
                    TempBestBaud = Freq2;
                    TempBestSample = HighNum;
                }
                else
                {
                    TempBestBaud = Freq1;
                    TempBestSample = LowNum;
                }
                uint32 BestBaudRte = I2c_Hal_AbsDif(BaudRate, BestBaudRate);
                uint32 BestTempRte = I2c_Hal_AbsDif(BaudRate, TempBestBaud);
                if (BestBaudRte >= BestTempRte)
                {
                    BestBaudRate = TempBestBaud;
                    BestSample = TempBestSample;
                    BestStep = StepCnt;
                }

                /* If currently BaudRate is equal to target BaudRate stop the search */
                if (BestBaudRate == BaudRate)
                {
                    break;
                }
            }
            /* Write the best StepCnt and SampleCnt to register */
            I2c_Reg_SetSampleStep(ChannelAddress, BestStep, BestSample);
        }
        else
        {
            ReturnType = STATUS_ERROR;
        }
    }

    return ReturnType;
}

/**
* @brief Starts an synchronous transmission on the I2C bus.
* @note Function ID:DES_I2C_API_241
* @note Service ID: NA
* @param[in] Instance : I2C hardware channel ID
* @param[inout] DataTransmitPtr: transmit data used
* @return Hal_StatusType: STATUS_SUCCESS:  Success to Send or receive
*                         STATUS_ERROR: Otherwise
*/
Hal_StatusType I2c_Hal_SyncTransceive
(
    uint8 Instance,
    const DataTransmitType *DataTransmitPtr
)
{
    Hal_StatusType ReturnType = STATUS_ERROR;
    I2C_Type *ChannelAddress;

    DEVICE_ASSERT(DataTransmitPtr != NULL_PTR);
    if ((Instance < I2C_INSTANCE_MAX) && (NULL_PTR != DataTransmitPtr))
    {
        ChannelAddress = I2c_HalBase[Instance];
        if (I2C_MASTER == I2cChannelConfigPtr[Instance]->I2cMode)/* work in master mode */
        {
            /* change SlaveAddress to be configurate */
            I2cMasterStatePtr[Instance]->Is10bitAddr = DataTransmitPtr->Is10bitAddr;
            I2cMasterStatePtr[Instance]->SlaveAddress = DataTransmitPtr->SlaveAddress;
            /* clear the Pltie flag bit*/
            I2c_Reg_ClearPltieFlag(ChannelAddress);
            if (DataTransmitPtr->DirType == I2C_WRITE)/* direction is write */
            {
                ReturnType = I2c_Hal_MasterSendDataPoll(Instance, DataTransmitPtr->DataBufferPtr,
                                                        DataTransmitPtr->DataLength, DataTransmitPtr->SendStop);
            }
            else/* direction is read */
            {
                ReturnType = I2c_Hal_MasterReceiveDataPoll(Instance, DataTransmitPtr->DataBufferPtr, \
                             DataTransmitPtr->DataLength, DataTransmitPtr->SendStop);
            }
        }
        else/* work in slave mode */
        {
            ReturnType = I2c_Hal_SlaveTransferPoll(Instance, DataTransmitPtr->DataBufferPtr, \
                                                   DataTransmitPtr->DataLength, DataTransmitPtr->DirType);
        }
    }

    return ReturnType;
}

/**
* @brief Starts an asynchronous transmission on the I2C bus.
* @note Function ID:[DES_I2C_API_202]
* @note Service ID: NA
* @param[in] Instance : I2C hardware channel ID
* @param[inout] DataTransmitPtr: transmit data used
* @return Hal_StatusType: STATUS_SUCCESS:  Success to Send or receive
*                         STATUS_ERROR: Otherwise
*/
Hal_StatusType I2c_Hal_AsyncTransceive
(
    uint8 Instance,
    const DataTransmitType *DataTransmitPtr
)
{
    Hal_StatusType ReturnType = STATUS_ERROR;
    I2C_Type *ChannelAddress;

    if ((Instance < I2C_INSTANCE_MAX) && (NULL_PTR != DataTransmitPtr))
    {
        ChannelAddress = I2c_HalBase[Instance];
        if (I2C_MASTER == I2cChannelConfigPtr[Instance]->I2cMode)/* work in master mode */
        {
            /* change SlaveAddress to be configurate */
            I2cMasterStatePtr[Instance]->Is10bitAddr = DataTransmitPtr->Is10bitAddr;
            I2cMasterStatePtr[Instance]->SlaveAddress = DataTransmitPtr->SlaveAddress;
            /* clear the Pltie flag bit*/
            I2c_Reg_ClearPltieFlag(ChannelAddress);
            if (DataTransmitPtr->DirType == I2C_WRITE)/* direction is write */
            {
                ReturnType = I2c_Hal_MasterSendData(Instance, DataTransmitPtr->DataBufferPtr, \
                                                    DataTransmitPtr->DataLength, DataTransmitPtr->SendStop);
            }
            else/* direction is read */
            {
                ReturnType = I2c_Hal_MasterReceiveData(Instance, DataTransmitPtr->DataBufferPtr, \
                                                       DataTransmitPtr->DataLength, DataTransmitPtr->SendStop);
            }
        }
        else/* work in slave mode */
        {
            /* change SlaveAddress to be configurate */
            if (I2C_WRITE == DataTransmitPtr->DirType)/* direction is write */
            {
                ReturnType = I2c_Hal_SlaveSendData(Instance, DataTransmitPtr->DataBufferPtr, \
                                                   DataTransmitPtr->DataLength);
            }
            else/* direction is read */
            {
                ReturnType = I2c_Hal_SlaveReceiveData(Instance, DataTransmitPtr->DataBufferPtr, \
                                                      DataTransmitPtr->DataLength);
            }
        }
    }

    return ReturnType;
}

/**
* @brief Stop an asynchronous transmission on the I2C bus.
* @note Function ID:DES_I2C_API_242
* @note Service ID: NA
* @param[in] Instance : I2C hardware channel ID
* @return Hal_StatusType: STATUS_SUCCESS:  Success to Send or receive
*                         STATUS_ERROR: Otherwise
*/
Hal_StatusType I2c_Hal_AbortTransceive
(
    uint8 Instance
)
{
    Hal_StatusType Ret = STATUS_ERROR;
    if (Instance < I2C_INSTANCE_MAX)
    {
        /* Clear I2C BND flag */
        I2c_Reg_ClearStatus0(I2c_HalBase[Instance], (uint32)I2C_STATUS0_BND_Msk);
        if (I2C_MASTER == I2cChannelConfigPtr[Instance]->I2cMode)/* work in master mode */
        {
            I2cMasterStatePtr[Instance]->Status = I2C_MASTER_CHANNEL_ABORTED;
            I2c_Hal_MasterEndTransfer(Instance, I2cMasterStatePtr[Instance], TRUE);
        }
        else/* work in slave mode */
        {
            if (FALSE == I2cSlaveStatePtr[Instance]->slaveListening)
            {
                /* End transfer: force stop generation */
                I2cSlaveStatePtr[Instance]->Status = I2C_SLAVE_CHANNEL_ABORTED;
                I2c_Hal_SlaveEndTransfer(Instance, I2cSlaveStatePtr[Instance]);
            }
        }
        Ret = STATUS_SUCCESS;
    }
    return Ret;
}

/**
* @brief Gets the status of an I2C channel.
* @note Function ID:DES_I2C_API_203
* @note Service ID: NA
* @param[in] Instance : I2C hardware channel ID
* @return I2c_ChannelStatusType: I2c channel currenr status
*/
I2c_Hal_ChannelStatusType I2c_Hal_GetStatus
(
    uint8 Instance
)
{
    const I2c_Hal_MasterStateType *MasterPtr;
    const I2c_Hal_SlaveStateType *SlavePtr;
    I2c_Hal_ChannelStatusType ChannelStatus = I2C_CHANNEL_ERROR_PRESENT;

    if (Instance < I2C_INSTANCE_MAX)
    {
        MasterPtr = I2cMasterStatePtr[Instance];
        SlavePtr = I2cSlaveStatePtr[Instance];
        ChannelStatus = I2C_CHANNEL_IDLE;

        if (I2C_MASTER == I2cChannelConfigPtr[Instance]->I2cMode)/* work in master mode */
        {
            switch (MasterPtr->Status)
            {
            case (I2C_MASTER_CHANNEL_IDLE):
                ChannelStatus = I2C_CHANNEL_IDLE;
                break;

            case (I2C_MASTER_CHANNEL_BUSY_SEND):
            case (I2C_MASTER_CHANNEL_BUSY_RECEIVE):
                ChannelStatus = I2C_CHANNEL_BUSY_TRANSMIT;
                break;

            case (I2C_MASTER_CHANNEL_FINISHED):
                ChannelStatus = I2C_CHANNEL_FINISHED;
                break;

            case (I2C_MASTER_CHANNEL_ARBITRATION_LOST):
            case (I2C_MASTER_CHANNEL_TIMEOUT):
            case (I2C_MASTER_CHANNEL_RECEIVE_NACK):
                ChannelStatus = I2C_CHANNEL_ERROR_PRESENT;
                break;

            case (I2C_MASTER_CHANNEL_ABORTED):
                ChannelStatus = I2C_CHANNEL_ABORTED_SUCCESS;
                break;

            default :
                /* do nothing */
                break;
            }
        }
        else/* work in slave mode */
        {
            switch (SlavePtr->Status)
            {
            case (I2C_SLAVE_CHANNEL_IDLE):
                ChannelStatus = I2C_CHANNEL_IDLE;
                break;

            case (I2C_SLAVE_CHANNEL_BUSY_SEND):
            case (I2C_SLAVE_CHANNEL_BUSY_RECEIVE):
            case (I2C_SLAVE_CHANNEL_BUSY_TRANSMIT):
                ChannelStatus = I2C_CHANNEL_BUSY_TRANSMIT;
                break;

            case (I2C_SLAVE_CHANNEL_FINISHED):
                ChannelStatus = I2C_CHANNEL_FINISHED;
                break;

            case (I2C_SLAVE_CHANNEL_DMA_ERROR):
            case (I2C_SLAVE_CHANNEL_TX_EMPTY):
            case (I2C_SLAVE_CHANNEL_TX_UNDERRUN):
            case (I2C_SLAVE_CHANNEL_RX_OVERRUN):
                ChannelStatus = I2C_CHANNEL_ERROR_PRESENT;
                break;

            case (I2C_SLAVE_CHANNEL_ABORTED):
                ChannelStatus = I2C_CHANNEL_ABORTED_SUCCESS;
                break;

            default :
                /* do nothing */
                break;
            }
        }
    }
    return ChannelStatus;
}

/**
* @brief Get the I2C base.
* @note Function ID:DES_I2C_API_243
* @param[in] Instance : I2C hardware channel ID.
* @return I2C_Type*: the I2C base addr.
*/
I2C_Type *I2c_Hal_GetBase
(
    uint8 Instance
)
{
    I2C_Type *Ret = NULL_PTR;
    if (Instance < I2C_INSTANCE_MAX)
    {
        Ret = I2c_HalBase[Instance];
    }

    return Ret;
}

/**
* @brief Handle master end dma transmit
* @note Function ID:DES_I2C_API_279
* @note Service ID: NA
* @param[in] Instance : I2C channel to be HwUnit
* @param[in] MasterPtr : I2c Master driver pointer
* @return void
*/
static void I2c_Hal_DmaEndTransmit
(
    uint8 Instance,
    I2c_Hal_MasterStateType *MasterPtr
)
{
    I2C_Type *ChannelAddress;
    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        if (I2C_USING_DMA == MasterPtr->TransferType)/* using dma */
        {
            if (I2C_WRITE == MasterPtr->DirType)/* direction is write */
            {
                if (0U != I2c_Reg_IsDMATxEnable(ChannelAddress))/* check dma enable */
                {
                    I2c_Reg_SetDMATx(ChannelAddress, FALSE);
                    /* Stop DMA Channel */
                    (void)Dma_Hal_StopCh(MasterPtr->TxDmaChannel);
                }
            }
            else/* direction is read */
            {
                I2c_Reg_SetDMARx(ChannelAddress, FALSE);
            }
        }
        if (0U == MasterPtr->TxSize)
        {
            MasterPtr->Status = I2C_MASTER_CHANNEL_FINISHED;
        }
        else
        {
            MasterPtr->Status = I2C_MASTER_CHANNEL_RECEIVE_NACK;
        }
        /* Receive NACK */
        I2c_Reg_ClearStatus0(ChannelAddress, (uint32)I2C_STATUS0_RACK_Msk);
        /* Clear I2C BND flag */
        I2c_Reg_ClearStatus0(ChannelAddress, (uint32)I2C_STATUS0_BND_Msk);
        I2c_Hal_MasterEndTransfer(Instance, MasterPtr, MasterPtr->SendStop);
    }
}

/**
* @brief Handle master operation when I2C interrupt occurs.
* @note Function ID:DES_I2C_API_233
* @note Service ID: NA
* @param[in] Instance : I2C channel to be HwUnit
* @return void
*/
static void I2c_Hal_MasterIRQHandler
(
    uint8 Instance
)
{
    I2C_Type *ChannelAddress;
    I2c_Hal_MasterStateType *MasterPtr;
    uint32 Intr_Status0;

    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        MasterPtr = I2cMasterStatePtr[Instance];
        Intr_Status0 = I2c_Reg_GetStatus0(ChannelAddress);
        uint32 SSIntNum = I2c_Reg_IsSSIntEnable(ChannelAddress);
        uint32 StatNum = I2c_Reg_IsStart(ChannelAddress);
        uint32 StopNum = I2c_Reg_IsStop(ChannelAddress);
        uint32 Pltie = I2c_Reg_IsPltie(ChannelAddress);
        /* check which event caused the interrupt */
        if ((0U != StatNum) && (0U != SSIntNum))/* receive start */
        {
            /* clear the start flag bit*/
            I2c_Reg_ClearStartFlag(ChannelAddress);
        }
        else if ((0U != StopNum) && (0U != SSIntNum))/* receive stop */
        {
            /* clear the stop flag bit*/
            I2c_Reg_ClearStopFlag(ChannelAddress);
        }
        else if (0U != Pltie)/* receive Pltie */
        {
            /* clear the Pltie flag bit*/
            I2c_Reg_ClearPltieFlag(ChannelAddress);
            if (NULL_PTR != I2cChannelConfigPtr[Instance]->Callback)
            {
                I2cChannelConfigPtr[Instance]->Callback(Instance, (uint32)I2C_MASTER_EVENT_PLTIE);
            }
        }
        else
        {
            /* Do nothing */
        }

        if (0U != (Intr_Status0 & I2C_STATUS0_RACK_Msk))/* receive RACK */
        {
            I2c_Hal_DmaEndTransmit(Instance, MasterPtr);
        }
        else
        {
            if (0U != (Intr_Status0 & I2C_STATUS0_BND_Msk))/* receive BND */
            {
                if (I2C_USING_INTERRUPTS == MasterPtr->TransferType)/* normal interrupt mode */
                {
                    if (I2C_WRITE == MasterPtr->DirType)/* direction is write */
                    {
                        I2c_Hal_MasterHandleTransmitDataRequest(Instance, MasterPtr);
                    }
                    else/* direction is read */
                    {
                        I2c_Hal_MasterHandleReceiveDataReadyEvent(Instance, MasterPtr);
                    }
                }
                else/* dma interrupt mode */
                {
                    /* Clear I2C BND flag */
                    I2c_Reg_ClearStatus0(ChannelAddress, (uint32)I2C_STATUS0_BND_Msk);
                    /* End I2C master transfer */
                    I2c_Hal_MasterEndTransfer(Instance, MasterPtr, MasterPtr->SendStop);

                    MasterPtr->Status = I2C_MASTER_CHANNEL_FINISHED;
                    if (NULL_PTR != I2cChannelConfigPtr[Instance]->Callback)
                    {
                        I2cChannelConfigPtr[Instance]->Callback(Instance, (uint32)I2C_MASTER_EVENT_END_TRANSFER);
                    }
                }
            }
        }

        if (0U != (Intr_Status0 & I2C_STATUS0_ARBLOST_Msk))/* receive arbitration */
        {
            /* clear Arbitration lost */
            I2c_Reg_ClearStatus0(ChannelAddress, (uint32)I2C_STATUS0_ARBLOST_Msk);
            /* i2c master/ */
            I2c_Reg_SetMSTR(ChannelAddress, I2C_MASTER);

            MasterPtr->Status = I2C_MASTER_CHANNEL_ARBITRATION_LOST;
            if (NULL_PTR != I2cChannelConfigPtr[Instance]->Callback)
            {
                I2cChannelConfigPtr[Instance]->Callback(Instance, (uint32)I2C_MASTER_EVENT_END_TRANSFER);
            }
        }
    }
}

/**
* @brief Handle slave operation when I2C interrupt occurs.
* @note Function ID:DES_I2C_API_235
* @note Service ID: NA
* @param[in] Instance : I2C channel to be HwUnit
* @return void
*/
static void I2c_Hal_SlaveIRQHandler
(
    uint8 Instance
)
{
    I2C_Type *ChannelAddress;
    I2c_Hal_SlaveStateType *SlavePtr;
    uint32 Intr_Status0;

    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        SlavePtr = I2cSlaveStatePtr[Instance];
        Intr_Status0 = I2c_Reg_GetStatus0(ChannelAddress);
        uint32 IsStartNum = I2c_Reg_IsStart(ChannelAddress);
        uint32 SSIntNum = I2c_Reg_IsSSIntEnable(ChannelAddress);
        uint32 IsStopNum = I2c_Reg_IsStop(ChannelAddress);
        /* check which event caused the interrupt */
        if ((0U != IsStartNum) && (SSIntNum != 0U))/* receive start */
        {
            /* clear the start flag bit*/
            I2c_Reg_ClearStartFlag(ChannelAddress);
            if ((TRUE == SlavePtr->slaveListening) && (I2C_USING_DMA == SlavePtr->TransferType))
            {
                I2c_Reg_ClearStatus0(ChannelAddress, (uint32)I2C_STATUS0_RACK_Msk);
                I2c_Hal_SlaveStartDmaTransfer(Instance);
            }
        }
        else if ((0U != IsStopNum) && (SSIntNum != 0U))/* receive stop */
        {
            /* clear the stop flag bit*/
            I2c_Reg_ClearStopFlag(ChannelAddress);
            if ((SlavePtr->Status == I2C_SLAVE_CHANNEL_BUSY_SEND) ||
                    (SlavePtr->Status == I2C_SLAVE_CHANNEL_BUSY_RECEIVE))
            {
                /* Report success if no error was recorded */
                SlavePtr->Status = I2C_SLAVE_CHANNEL_FINISHED;
            }
            I2c_Hal_SlaveEndTransferHandler(Instance, SlavePtr);
        }
        else
        {
            /* do nothing */
        }
        if (I2C_USING_INTERRUPTS == SlavePtr->TransferType)/* normal interrupt mode */
        {
            /* Check which event caused the interrupt */
            if (0U != (Intr_Status0 & I2C_STATUS0_SAMF_Msk))
            {
                /* Address match */
                I2c_Hal_SlaveHandleAddressValidEvent(Instance, SlavePtr);
            }
            if (0U != (Intr_Status0 & I2C_STATUS0_BND_Msk))
            {
                /* Check slave transfer direction */
                if (0U != (Intr_Status0 & I2C_STATUS0_SRW_Msk))/* check SRW value not 0 is write */
                {
                    I2c_Hal_SlaveHandleTransmitDataEvent(Instance, SlavePtr);
                }
                else
                {
                    I2c_Hal_SlaveHandleReceiveDataEvent(Instance, SlavePtr);
                }
            }
        }
        else/* dma interrupt mode */
        {
            if (0U != (Intr_Status0 & I2C_STATUS0_SAMF_Msk))/* SAMF interrupt */
            {
                I2c_Reg_ClearStatus0(ChannelAddress, (uint32)I2C_STATUS0_SAMF_Msk);
            }
            if (0U != (Intr_Status0 & I2C_STATUS0_RACK_Msk))/* RACk interrupt */
            {
                /* DISABLE NACK interrupt */
                I2c_Reg_SetNackInterrupt(ChannelAddress, FALSE);
            }
            if (0U != (Intr_Status0 & I2C_STATUS0_BND_Msk))/* BND interrupt */
            {
                I2c_Reg_ClearStatus0(ChannelAddress, (uint32)I2C_STATUS0_BND_Msk);
            }
        }
    }
}

/*!
 * @brief Provide a buffer for transmitting data.
 *
 * @param[in] Instance: The I2C instance number
 * @param[in] TxBuff: Pointer to the data to be transferred
 * @param[in] TxSize: Length of the data to be transferred
 * @return The result of execution
 */
#ifndef I2C_SDK_NON_EXTENDED_API
Hal_StatusType I2C_Hal_SlaveSetTxBuffer(uint8 Instance, const uint8 *TxBuff, uint32 TxSize)
{
    DEVICE_ASSERT(Instance < I2C_INSTANCE_MAX);
    DEVICE_ASSERT(TxBuff != NULL_PTR);
    DEVICE_ASSERT(TxSize > 0U);
    I2c_Hal_SlaveStateType *SlavePtr;
    Hal_StatusType Ret = STATUS_ERROR;

    if (Instance < I2C_INSTANCE_MAX)
    {
        SlavePtr = I2cSlaveStatePtr[Instance];
        SlavePtr->TxBuffPtr = TxBuff;
        SlavePtr->TxSize = TxSize;
        Ret = STATUS_SUCCESS;
    }

    return Ret;
}

/*!
 * @brief Provide a buffer for receiving data.
 *
 * @param[in] Instance: The I2C instance number
 * @param[in] RxBuff: Pointer to the buffer where to store received data
 * @param[in] RxSize: Length of the data to be transferred
 * @return The result of execution
 */
Hal_StatusType I2C_Hal_SlaveSetRxBuffer(uint8 Instance, uint8 *RxBuff, uint32 RxSize)
{
    DEVICE_ASSERT(RxBuff != NULL_PTR);
    DEVICE_ASSERT(RxSize > 0U);
    I2c_Hal_SlaveStateType *SlavePtr;
    Hal_StatusType Ret = STATUS_ERROR;

    if (Instance < I2C_INSTANCE_MAX)
    {
        SlavePtr = I2cSlaveStatePtr[Instance];
        SlavePtr->RxBuffPtr = RxBuff;
        SlavePtr->RxSize = RxSize;
        SlavePtr->RxPointer = 0U;
        Ret = STATUS_SUCCESS;
    }
    return Ret;
}

/*!
 * @brief Get the listening receiving data.
 *
 * @param[in] Instance: The I2C instance number
 * @return The receive data len
 */
uint32 I2C_Hal_SlaveGetRxSize(uint8 Instance)
{
    uint32 Len = 0U;
    const I2c_Hal_SlaveStateType *SlavePtr;
    if (Instance < I2C_INSTANCE_MAX)
    {
        SlavePtr = I2cSlaveStatePtr[Instance];
        Len = SlavePtr->RxPointer;
    }

    return Len;
}
#endif

/**
* @brief Handle operation when I2C interrupt occurs.
* @note Function ID:DES_I2C_API_205
* @note Service ID: NA
* @param[in] Instance : I2C channel to be HwUnit
* @return void
*/
static void I2c_Hal_IRQHandler
(
    uint8 Instance
)
{
    DEVICE_ASSERT(Instance < I2C_INSTANCE_MAX);
    const I2C_Type *ChannelAddress;
    uint32 Data;

    if (Instance < I2C_INSTANCE_MAX)
    {
        ChannelAddress = I2c_HalBase[Instance];
        Data = I2c_Reg_GetStatus0(ChannelAddress);
        Data &= I2C_STATUS0_ARBLOST_Msk;
        if ((I2C_MASTER == I2cChannelConfigPtr[Instance]->I2cMode) || (Data != 0U))/* work in master mode */
        {
            I2c_Hal_MasterIRQHandler(Instance);
        }
        else/* work in slave mode */
        {
            I2c_Hal_SlaveIRQHandler(Instance);
        }
    }
}

ISR(I2C0_IRQHandler)//PRQA S 1503,3408 # AMCE_QAC_NoUsedApi,AMCE_QAC_3408
{
    I2c_Hal_IRQHandler(0U);
}

#if defined (AC7842X) || defined (AC7843X) || defined (AC7840E)
ISR(I2C1_IRQHandler)//PRQA S 1503,3408 # AMCE_QAC_NoUsedApi,AMCE_QAC_3408
{
    I2c_Hal_IRQHandler(1U);
}
#endif

#if defined (AC7843X)
ISR(I2C2_IRQHandler)//PRQA S 1503,3408 # AMCE_QAC_NoUsedApi,AMCE_QAC_3408
{
    I2c_Hal_IRQHandler(2U);
}
#endif

#ifdef __cplusplus
}
#endif
/* =============================================  EOF  ============================================== */
