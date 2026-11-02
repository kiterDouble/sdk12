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

/*!
 * @file Eio_I2c_Hal.c
 *
 * @brief This file provides eio i2c integration functions.
 *
 */

/* ===========================================  Includes  =========================================== */
#include "Ckgen_Hal.h"
#include "Eio_I2c_Hal.h"
#include "Rcm_Hal.h"
#include "AC784xx_Eio_Reg.h"
#include "Eio_Common_Hal.h"
#include "Dma_Hal.h"

/* ============================================  Define  ============================================ */
/*!
 * @brief Constraints used for baud rate computation
 */
#define DIVIDER_MIN_VALUE  (1U)
#define DIVIDER_MAX_VALUE  (0xFFU)

/*!
 * @brief Shifters/Timers used for I2C simulation The param[in]eter x represents the
 * resourceIndex value for the current driver instance
 */
#define I2C_TX_SHIFTER(x)     (x)
#define I2C_RX_SHIFTER(x)     (uint8)((x) + 1UL)
#define I2C_SCL_TIMER(x)      (x)
#define I2C_CONTROL_TIMER(x)  (uint8)((x) + 1UL)

#define EIO_IIC_TIMEOUT_VALUE (0x20000U)

/* ===========================================  Typedef  ============================================ */

/* ==========================================  Variables  =========================================== */
/* Table of base addresses for EIO instances */
static EIO_Type * const g_eioBase[EIO_INSTANCE_COUNT] = EIO_BASE_PTRS;

/*PRQA S 3218 ++ # this function is only accessed in one function.*/
/* EIO clock sources, for getting the input clock frequency */
static const Ckgen_ClkIdType g_eioClock[EIO_INSTANCE_COUNT] = {CKGEN_EIO_CLK};
/*PRQA S 3218 -- */

static eio_i2c_master_state_t eio_i2c_master_state;

/* ====================================  Functions declaration  ===================================== */

/* =====================================  Functions definition  ===================================== */

static void Eio_I2c_Hal_MasterComputeBaudRateDivider(uint32 baudRate, uint16 *divider, uint32 inputClock)
{
    uint32 tmpDiv;

    /* Compute divider: ((input_clock / baud_rate) / 2) - 1 - 1. The extra -1 is from the
       timer reset setting used for clock stretching. Round to nearest integer */
    tmpDiv = (((uint32)inputClock + (uint32)baudRate) / (2U * (uint32)baudRate)) - 2U;
    /* Enforce upper/lower limits */
    if (tmpDiv < DIVIDER_MIN_VALUE)
    {
        tmpDiv = DIVIDER_MIN_VALUE;
    }
    if (tmpDiv > DIVIDER_MAX_VALUE)
    {
        tmpDiv = DIVIDER_MAX_VALUE;
    }

    *divider = (uint16)tmpDiv;
}

static void Eio_I2c_Hal_MasterConfigure(const eio_i2c_master_state_t *master, uint32 inputClock, uint32 baudRate)
{
    EIO_Type *baseAddr;
    uint16 divider;
    uint8 resourceIndex;    /* Index of first used internal resource instance (shifter and timer) */

    baseAddr = g_eioBase[master->eioCommon.Instance];
    resourceIndex = master->eioCommon.ResourceIndex;

    /* Compute divider.*/
    Eio_I2c_Hal_MasterComputeBaudRateDivider(baudRate, &divider, inputClock);

    /* Configure tx shifter */
    Eio_Reg_SetShifterConfig(baseAddr,
                         I2C_TX_SHIFTER(resourceIndex),
                         EIO_SHIFTER_START_BIT_0,
                         EIO_SHIFTER_STOP_BIT_1,
                         EIO_SHIFTER_SOURCE_PIN);
    /* Shifter disabled and pin enabled causes the pin to be held low.
       So disable pin too, will be enabled at transfer time.  */
    Eio_Reg_SetShifterControl(baseAddr,
                          I2C_TX_SHIFTER(resourceIndex),
                          EIO_SHIFTER_MODE_TRANSMIT,
                          master->sdaPin,                    /* output on SDA pin */
                          EIO_PIN_POLARITY_LOW,
                          EIO_PIN_CONFIG_OPEN_DRAIN,
                          I2C_CONTROL_TIMER(resourceIndex),                     /* use control timer to drive the shifter */
                          EIO_TIMER_POLARITY_POSEDGE);

    /* Configure rx shifter */
    Eio_Reg_SetShifterConfig(baseAddr,
                         I2C_RX_SHIFTER(resourceIndex),
                         EIO_SHIFTER_START_BIT_DISABLED,
                         EIO_SHIFTER_STOP_BIT_0,
                         EIO_SHIFTER_SOURCE_PIN);
    Eio_Reg_SetShifterControl(baseAddr,
                          I2C_RX_SHIFTER(resourceIndex),
                          EIO_SHIFTER_MODE_RECEIVE,
                          master->sdaPin,                    /* input from SDA pin */
                          EIO_PIN_POLARITY_HIGH,
                          EIO_PIN_CONFIG_DISABLED,
                          I2C_CONTROL_TIMER(resourceIndex),                     /* use control timer to drive the shifter */
                          EIO_TIMER_POLARITY_NEGEDGE);

    /* Configure SCL timer */
    Eio_Reg_SetTimerCompare(baseAddr, I2C_SCL_TIMER(resourceIndex), divider);
    Eio_Reg_SetTimerConfig(baseAddr,
                       I2C_SCL_TIMER(resourceIndex),
                       EIO_TIMER_START_BIT_ENABLED,
                       EIO_TIMER_STOP_BIT_TIM_DIS,
                       EIO_TIMER_ENABLE_TRG_HIGH,         /* enable when Tx data is available */
                       EIO_TIMER_DISABLE_NEVER,
                       EIO_TIMER_RESET_PIN_OUT,           /* reset if output equals pin (for clock stretching) */
                       EIO_TIMER_DECREMENT_CLK_SHIFT_TMR, /* decrement on EIO clock */
                       EIO_TIMER_INITOUT_ZERO);
    Eio_Reg_SetTimerControl(baseAddr,
                        I2C_SCL_TIMER(resourceIndex),
                        (uint8)((uint8)(I2C_TX_SHIFTER(resourceIndex) << 2U) + 1U), /* trigger on tx shifter status flag */
                        EIO_TRIGGER_POLARITY_LOW,
                        EIO_TRIGGER_SOURCE_INTERNAL,
                        master->sclPin,                  /* output on SCL pin */
                        EIO_PIN_POLARITY_HIGH,
                        EIO_PIN_CONFIG_OPEN_DRAIN,    /* enable output */
                        EIO_TIMER_MODE_DISABLED);

    /* Configure control timer for shifters */
    Eio_Reg_SetTimerCompare(baseAddr, I2C_CONTROL_TIMER(resourceIndex), 0x000FU);
    Eio_Reg_SetTimerConfig(baseAddr,
                       I2C_CONTROL_TIMER(resourceIndex),
                       EIO_TIMER_START_BIT_ENABLED,
                       EIO_TIMER_STOP_BIT_TIM_CMP,
                       EIO_TIMER_ENABLE_TIM_ENABLE,       /* enable on SCL timer enable */
                       EIO_TIMER_DISABLE_TIM_DISABLE,     /* disable on SCL timer disable */
                       EIO_TIMER_RESET_NEVER,
                       EIO_TIMER_DECREMENT_PIN_SHIFT_PIN, /* decrement on SCL pin input */
                       EIO_TIMER_INITOUT_ONE);
    Eio_Reg_SetTimerControl(baseAddr,
                        I2C_CONTROL_TIMER(resourceIndex),
                        (uint8)((uint8)(I2C_TX_SHIFTER(resourceIndex) << 2U) + 1U), /* trigger on tx shifter status flag */
                        EIO_TRIGGER_POLARITY_LOW,
                        EIO_TRIGGER_SOURCE_INTERNAL,
                        master->sclPin,                      /* use SCL pin as input */
                        EIO_PIN_POLARITY_LOW,
                        EIO_PIN_CONFIG_DISABLED,
                        EIO_TIMER_MODE_DISABLED);
}

static void Eio_I2c_Hal_WriteData(eio_i2c_master_state_t *master)
{
    EIO_Type *baseAddr;
    uint32 data;
    uint8 resourceIndex;    /* Index of first used internal resource instance (shifter and timer) */

    baseAddr = g_eioBase[master->eioCommon.Instance];
    resourceIndex = master->eioCommon.ResourceIndex;

    /* If txRemainingBytes == 0 the transmission is over */
    if (master->txRemainingBytes != 0U)
    {
        master->txRemainingBytes--;
        if (master->txRemainingBytes == 0U)
        {
            /* Done transmitting */
            if (master->sendStop == TRUE)
            {
                /* Transmit stop condition */
                data = 0x00U;
            }
            else
            {
                /* Do not transmit stop condition */
                data = 0xFFU;
            }
        }
        else if (master->receive == TRUE)
        {
            /* Transmit 0xFF to leave the line free while receiving */
            data = 0xFFU;
        }
        else
        {
            /* Read data from user buffer */
            data =  *(master->txData);
            master->txData++;
        }

        /* Shift data before bit-swapping it to get the relevant bits in the lower part of the shifter */
        data <<= 24U;
        Eio_Reg_WriteShifterBuffer(baseAddr, I2C_TX_SHIFTER(resourceIndex), data, EIO_SHIFTER_RW_MODE_BIT_SWAP);
    }
}

static void Eio_I2c_Hal_MasterEnableTransfer(const eio_i2c_master_state_t *master)
{
    EIO_Type *baseAddr;
    uint8 resourceIndex;    /* Index of first used internal resource instance (shifter and timer) */

    if (master != NULL_PTR)
    {
        resourceIndex = master->eioCommon.ResourceIndex;
        baseAddr = g_eioBase[master->eioCommon.Instance];

        /* enable timers and shifters */
        Eio_Reg_SetShifterMode(baseAddr, I2C_TX_SHIFTER(resourceIndex), EIO_SHIFTER_MODE_TRANSMIT);
        Eio_Reg_SetShifterMode(baseAddr, I2C_RX_SHIFTER(resourceIndex), EIO_SHIFTER_MODE_RECEIVE);

        Eio_Reg_SetTimerMode(baseAddr, I2C_CONTROL_TIMER(resourceIndex), EIO_TIMER_MODE_16BIT);
        Eio_Reg_SetTimerMode(baseAddr, I2C_SCL_TIMER(resourceIndex), EIO_TIMER_MODE_8BIT_BAUD);

        /* enable Tx pin */
        Eio_Reg_SetShifterPinConfig(baseAddr, I2C_TX_SHIFTER(resourceIndex), EIO_PIN_CONFIG_OPEN_DRAIN);
    }
}

static void Eio_I2c_Hal_MasterEndTransfer(eio_i2c_master_state_t *master)
{
    EIO_Type *baseAddr;
    uint8 resourceIndex;    /* Index of first used internal resource instance (shifter and timer) */

    baseAddr = g_eioBase[master->eioCommon.Instance];
    resourceIndex = master->eioCommon.ResourceIndex;

    /* Restore Rx stop bit, in case it was changed by a receive */
    Eio_Reg_SetShifterStopBit(baseAddr, I2C_RX_SHIFTER(resourceIndex), EIO_SHIFTER_STOP_BIT_0);
    /* Restore Tx stop bit, in case it was changed by a receive */
    Eio_Reg_SetShifterStopBit(baseAddr, I2C_TX_SHIFTER(resourceIndex), EIO_SHIFTER_STOP_BIT_1);
    /* Clear Rx status in case there is a character left in the buffer */
    Eio_Reg_ClearShifterStatus(baseAddr, I2C_RX_SHIFTER(resourceIndex));

    /* Disable error interrupt for Rx shifter */
    Eio_Reg_SetShifterErrorInterrupt(baseAddr, (uint8)(1U << I2C_RX_SHIFTER(resourceIndex)), FALSE);
    /* Disable interrupt for SCL timer */
    Eio_Reg_SetTimerInterrupt(baseAddr, (uint8)(1U << I2C_SCL_TIMER(resourceIndex)), FALSE);
    /* Stop DMA channels */
    (void)Dma_Hal_StopCh(master->txDMAChannel);
    (void)Dma_Hal_StopCh(master->rxDMAChannel);
    /* Disable EIO DMA requests for both shifters */
    Eio_Reg_SetShifterDMARequest(baseAddr, (uint8)((1U << I2C_TX_SHIFTER(resourceIndex)) \
        | (1U << I2C_RX_SHIFTER(resourceIndex))), FALSE);

    master->driverIdle = TRUE;
}

static void Eio_I2c_Hal_MasterStopTransfer(eio_i2c_master_state_t *master)
{
    EIO_Type *baseAddr;
    uint8 resourceIndex;    /* Index of first used internal resource instance (shifter and timer) */

    resourceIndex = master->eioCommon.ResourceIndex;
    baseAddr = g_eioBase[master->eioCommon.Instance];

    Eio_Reg_SetTimerMode(baseAddr, I2C_SCL_TIMER(resourceIndex), EIO_TIMER_MODE_DISABLED);
    Eio_Reg_SetTimerMode(baseAddr, I2C_CONTROL_TIMER(resourceIndex), EIO_TIMER_MODE_DISABLED);

    /* clear any leftover error flags */
    Eio_Reg_ClearShifterErrorStatus(baseAddr, I2C_TX_SHIFTER(resourceIndex));
    Eio_Reg_ClearShifterErrorStatus(baseAddr, I2C_RX_SHIFTER(resourceIndex));
    /* discard any leftover rx. data */
    Eio_Reg_ClearShifterStatus(baseAddr, I2C_RX_SHIFTER(resourceIndex));
    /* Clear timer status */
    Eio_Reg_ClearTimerStatus(baseAddr, I2C_SCL_TIMER(resourceIndex));

    Eio_Reg_SetShifterStopBit(baseAddr, I2C_TX_SHIFTER(resourceIndex), EIO_SHIFTER_STOP_BIT_1);
    /* end the transfer */
    Eio_I2c_Hal_MasterEndTransfer(master);

    /* Check receive overflow */
    if ((master->rxRemainingBytes != 0U) && (master->status == STATUS_SUCCESS) && (master->receive == TRUE) && \
        (master->driverType != EIO_DRIVER_TYPE_DMA))
    {
        master->status = STATUS_I2C_RX_OVERRUN;
    }
}

static void Eio_I2c_Hal_MasterEndDmaTransfer(void *stateStruct)
{
    eio_i2c_master_state_t *master;
    uint8 resourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    EIO_Type *baseAddr;
    uint16 timerCmp;

    DEVICE_ASSERT(stateStruct != NULL_PTR);

    /*PRQA S 0316 ++ # functions allow conversion of a pointer to void.*/
    master = (eio_i2c_master_state_t *)stateStruct;
    /*PRQA S 0316 -- */
    baseAddr = g_eioBase[master->eioCommon.Instance];
    resourceIndex = master->eioCommon.ResourceIndex;

    /*PRQA S 3415 ++ # the right operand of the && and || operations has no side effect.*/
    /* Check for DMA transfer errors */
    if ((Dma_Hal_GetChStatus(master->txDMAChannel) == STATUS_ERROR) ||
            (Dma_Hal_GetChStatus(master->rxDMAChannel) == STATUS_ERROR))
    /*PRQA S 3415 -- # the right operand of the && and || operations has no side effect.*/
    {
        master->status = STATUS_ERROR;
        /* Force the transfer to stop */
        Eio_I2c_Hal_MasterStopTransfer(master);
        /* Call callback to announce the event to the user */
        if (master->callback != NULL_PTR)
        {
            master->callback(EIO_I2C_MASTER_EVENT_END_TRANSFER);
        }
    }
    else
    {
        /* Check for NACK */
        if (master->addrReceived == FALSE)
        {
            /* This was the address byte */
            master->addrReceived = TRUE;
            Eio_Reg_ClearShifterErrorStatus(baseAddr, I2C_RX_SHIFTER(resourceIndex));
        }
        else if (TRUE == Eio_Reg_GetShifterErrorStatus(baseAddr, I2C_RX_SHIFTER(resourceIndex)))
        {
            Eio_Reg_ClearShifterErrorStatus(baseAddr, I2C_RX_SHIFTER(resourceIndex));
            if (master->eventCount >= 2U)
            {
                master->status = STATUS_I2C_RECEIVED_NACK;
                /* Force the transfer to stop */
                Eio_I2c_Hal_MasterStopTransfer(master);
                /* Call callback to announce the event to the user */
                if (master->callback != NULL_PTR)
                {
                    master->callback(EIO_I2C_MASTER_EVENT_END_TRANSFER);
                }
            }
        }
        else
        {
            //do nothing
        }

        /* Check if the transfer is over */
        if (TRUE == Eio_Reg_GetTimerStatus(baseAddr, I2C_SCL_TIMER(resourceIndex)))
        {
            master->eventCount--;
            /* Clear timer status */
            Eio_Reg_ClearTimerStatus(baseAddr, I2C_SCL_TIMER(resourceIndex));
            if (master->eventCount == 2U)
            {
                /* PRQA S 2985 ++ #considered an invalid operation, it is actually meaningful*/
                /* Adjust number of ticks in high part of timer compare register  for the last reload */
                timerCmp = Eio_Reg_GetTimerCompare(baseAddr, I2C_SCL_TIMER(resourceIndex));
                timerCmp = (uint16)((uint32)timerCmp & 0x00FFU) | \
                    (uint16)(((uint32)(master->lastReload) & 0xFFU) << 8U);
                /* PRQA S 2985 -- */
                Eio_Reg_SetTimerCompare(baseAddr, I2C_SCL_TIMER(resourceIndex), timerCmp);
            }
            if (master->eventCount == 1U)
            {
                /* Timer will disable on the next countdown complete */
                Eio_Reg_SetTimerDisable(baseAddr, I2C_SCL_TIMER(resourceIndex), EIO_TIMER_DISABLE_TIM_CMP);
            }

            if (master->eventCount == 0U)
            {
                Eio_Reg_SetTimerDisable(baseAddr, I2C_SCL_TIMER(resourceIndex), EIO_TIMER_DISABLE_NEVER);
                /* Record success if there was no error */
                if (master->status == STATUS_BUSY)
                {
                    master->status = STATUS_SUCCESS;
                }
                /* End transfer */
                Eio_I2c_Hal_MasterStopTransfer(master);
                /* Call callback to announce the event to the user */
                if (master->callback != NULL_PTR)
                {
                    master->callback(EIO_I2C_MASTER_EVENT_END_TRANSFER);
                }
            }
        }
    }
}

LOCAL_INLINE boolean Eio_I2c_Hal_MasterBusBusy(const EIO_Type *baseAddr, const eio_i2c_master_state_t *master)
{
    boolean Ret = FALSE;
    if (master != NULL_PTR)
    {
        uint8 pinMask = (uint8)((1U << master->sdaPin) | (1U << master->sclPin));
        if ((Eio_Reg_GetPinData(baseAddr) & pinMask) == pinMask)
        {
            /* both pins are high, bus is not busy */
            Ret = FALSE;
        }
        else
        {
            /* bus is busy */
            Ret =  TRUE;
        }
    }

    return Ret;
}

static void Eio_I2c_Hal_MasterSetBytesNo(EIO_Type *baseAddr, eio_i2c_master_state_t *master)
{
    uint16 timerCmp;
    uint32 edgeNo;        /* total number of clock edges */
    uint32 counter;       /* edge counter per timer reload */
    uint32 reloads;       /* number of timer reloads */
    uint8 resourceIndex;    /* Index of first used internal resource instance (shifter and timer) */

    resourceIndex = master->eioCommon.ResourceIndex;
    /* Compute number of SCL edges, including address */
    edgeNo = (master->txRemainingBytes * 18U) + 2U;
    reloads = (uint32)((edgeNo + 255U) / 256U);
    counter = (uint32)((uint32)(edgeNo + (reloads - 1U)) / (uint32)reloads); /* round up */

    /* Set number of ticks in high part of timer compare register */
    timerCmp = Eio_Reg_GetTimerCompare(baseAddr, I2C_SCL_TIMER(resourceIndex));
    timerCmp = (uint16)((timerCmp & 0x00FFU) | (uint16)(((counter - 1U) & 0xFFU) << 8U));
    Eio_Reg_SetTimerCompare(baseAddr, I2C_SCL_TIMER(resourceIndex), timerCmp);

    /* Store reload information */
    master->eventCount = (uint16) reloads;
    master->lastReload = (uint8)(edgeNo - ((reloads - 1U) * counter) - 1U);
    /* Handle no reload case */
    if (reloads == 1U)
    {
        Eio_Reg_SetTimerDisable(baseAddr, I2C_SCL_TIMER(resourceIndex), EIO_TIMER_DISABLE_TIM_CMP);
    }
}

LOCAL_INLINE uint32 Eio_I2c_Hal_MasterComputeTxRegAddr(const eio_i2c_master_state_t *master)
{
    uint32 addr;
    const EIO_Type *baseAddr;
    uint8 shifter;

    baseAddr = g_eioBase[master->eioCommon.Instance];
    shifter = I2C_TX_SHIFTER(master->eioCommon.ResourceIndex);
    addr = (uint32)(&(baseAddr->SHIFTBUFBIS[shifter])) + (sizeof(uint32) - 1U);

    return addr;
}

LOCAL_INLINE uint32 Eio_I2c_Hal_MasterComputeRxRegAddr(const eio_i2c_master_state_t *master)
{
    uint32 addr;
    const EIO_Type *baseAddr;
    uint8 shifter;

    baseAddr = g_eioBase[master->eioCommon.Instance];
    shifter = I2C_RX_SHIFTER(master->eioCommon.ResourceIndex);
    addr = (uint32)(&(baseAddr->SHIFTBUFBIS[shifter]));

    return addr;
}

/*PRQA S 3673 ++ # unmodified pointer types are allowed in interrupt functions without const modification.*/
static void Eio_I2c_Hal_MasterDmaSendCallback(void *Args)
/*PRQA S 3673 -- # unmodified pointer types are allowed in interrupt functions without const modification.*/
{
    uint8 resourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    const EIO_Type *baseAddr;
    eio_i2c_master_state_t *master;

    (void)Args;
    master = &eio_i2c_master_state;
    baseAddr = g_eioBase[master->eioCommon.Instance];
    resourceIndex = master->eioCommon.ResourceIndex;

    /* last byte for stop condition */
    if (master->txRemainingBytes == 1U)
    {
        uint32 timeout = EIO_IIC_TIMEOUT_VALUE;
        /* wait for last byte transfer to shifter */
        while (Eio_Reg_GetShifterStatus(baseAddr, I2C_TX_SHIFTER(resourceIndex)) == FALSE)
        {
            timeout --;
            if (timeout == 0U)
            {
                master->status = STATUS_ERROR;
                /* Force the transfer to stop */
                Eio_I2c_Hal_MasterStopTransfer(master);
                break;
            }
        }

        Eio_I2c_Hal_WriteData(master);
    }
    Eio_I2c_Hal_MasterEndDmaTransfer(master);
}

/*PRQA S 3673 ++ # unmodified pointer types are allowed in interrupt functions without const modification.*/
static void Eio_I2c_Hal_MasterDmaReceiveCallback(void *Args)
/*PRQA S 3673 -- # unmodified pointer types are allowed in interrupt functions without const modification.*/
{
    uint8 resourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    EIO_Type *baseAddr;
    eio_i2c_master_state_t *master;

    (void)Args;
    master = &eio_i2c_master_state;
    baseAddr = g_eioBase[master->eioCommon.Instance];
    resourceIndex = master->eioCommon.ResourceIndex;

    if ((master->rxRemainingBytes == 1U) && (master->receive == TRUE))
    {
        /* Send NACK for last byte */
        Eio_Reg_SetShifterStopBit(baseAddr, I2C_TX_SHIFTER(resourceIndex), EIO_SHIFTER_STOP_BIT_1);
        /* Also instruct rx shifter to expect NACK */
        Eio_Reg_SetShifterStopBit(baseAddr, I2C_RX_SHIFTER(resourceIndex), EIO_SHIFTER_STOP_BIT_1);
        /* wait for last byte transfer to shifter */
        uint32 timeout = EIO_IIC_TIMEOUT_VALUE;
        while (Eio_Reg_GetShifterStatus(baseAddr, I2C_RX_SHIFTER(resourceIndex)) == FALSE)
        {
            timeout --;
            if (timeout == 0U)
            {
                master->status = STATUS_ERROR;
                /* Force the transfer to stop */
                Eio_I2c_Hal_MasterStopTransfer(master);
                break;
            }
        }

        master->rxRemainingBytes --;
        /* Read data from rx shifter */
        master->rxData[(master->dmaReceiveBytes) - 1U] = (uint8)Eio_Reg_ReadShifterBuffer(baseAddr, \
            I2C_RX_SHIFTER(resourceIndex), EIO_SHIFTER_RW_MODE_BIT_SWAP);
    }

    Eio_I2c_Hal_MasterEndDmaTransfer(master);
}

static Hal_StatusType Eio_I2c_Hal_MasterStartDmaTransfer(eio_i2c_master_state_t *master)
{
    uint8 resourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    EIO_Type *baseAddr;
    Hal_StatusType retCode;
    baseAddr = g_eioBase[master->eioCommon.Instance];
    resourceIndex = master->eioCommon.ResourceIndex;
    Dma_TransferConfigType TransferConfig;

    if (master->receive == TRUE)
    {
        /* if receiving, send 0xFFFFFFFF to keep the line clear */
        master->dummyDmaIdle = 0xFFFFFFFFU;

        /*cstat !MISRAC2012-Rule-11.4 Pointer conversion is required for reading and writing register operation.*/
        TransferConfig.DestStartAddr = Eio_I2c_Hal_MasterComputeTxRegAddr(master);
        TransferConfig.Length = (uint16)(master->txRemainingBytes - 1U);
        /*cstat !MISRAC2012-Rule-11.4 Pointer conversion is required for reading and writing register operation.*/
        TransferConfig.SrcStartAddr = (uint32)(&(master->dummyDmaIdle));
        /*cstat -MISRAC2012-Rule-11.1 The conversion is needed by callback function type convert.*/
        TransferConfig.Callback = Eio_I2c_Hal_MasterDmaSendCallback;
        TransferConfig.UserArgs = NULL_PTR;
        TransferConfig.Type = DMA_TRANSFER_PERIPH2PERIPH;
        TransferConfig.DestUnit = DMA_TRANSFER_UNIT_1B;
        TransferConfig.SrcUnit = DMA_TRANSFER_UNIT_1B;
        TransferConfig.SrcOffset = 0U;
        TransferConfig.DestOffset = 0U;
        TransferConfig.CircularMode = FALSE;
#if defined (AC7840E)
        TransferConfig.DaisyMode = FALSE;
#endif
        TransferConfig.TriggerMode = FALSE;
        TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
        /* PRQA S 2985 ++ #considered an invalid operation, it is actually meaningful*/
        TransferConfig.SrcEndAddr = TransferConfig.SrcStartAddr + (uint16)(TransferConfig.Length \
                                    / (uint8)(1U << (uint8)TransferConfig.SrcUnit) * TransferConfig.SrcOffset);
        TransferConfig.DestEndAddr = TransferConfig.DestStartAddr + (uint16)(TransferConfig.Length \
                                     / (uint8)(1U << (uint8)TransferConfig.DestUnit) * TransferConfig.DestOffset);
        /* PRQA S 2985 -- */
        retCode = Dma_Hal_ConfigCh(master->txDMAChannel, &TransferConfig);

        if (master->rxRemainingBytes == 1UL)
        {
            /* Send NACK for last byte */
            Eio_Reg_SetShifterStopBit(baseAddr, I2C_TX_SHIFTER(resourceIndex), EIO_SHIFTER_STOP_BIT_1);
            /* Also instruct rx shifter to expect NACK */
            Eio_Reg_SetShifterStopBit(baseAddr, I2C_RX_SHIFTER(resourceIndex), EIO_SHIFTER_STOP_BIT_1);
        }
        else
        {
            /* Send NACK for not last byte */
            Eio_Reg_SetShifterStopBit(baseAddr, I2C_TX_SHIFTER(resourceIndex), EIO_SHIFTER_STOP_BIT_0);
        }
    }
    else
    {
        /*cstat !MISRAC2012-Rule-11.4 Pointer conversion is required for reading and writing register operation.*/
        TransferConfig.DestStartAddr = Eio_I2c_Hal_MasterComputeTxRegAddr(master);
        TransferConfig.Length = (uint16)(master->txRemainingBytes - 1U);
        /*cstat !MISRAC2012-Rule-11.4 Pointer conversion is required for reading and writing register operation.*/
        TransferConfig.SrcStartAddr = (uint32)(master->txData);
        /*cstat -MISRAC2012-Rule-11.1 The conversion is needed by callback function type convert.*/
        TransferConfig.Callback = Eio_I2c_Hal_MasterDmaSendCallback;
        TransferConfig.UserArgs = NULL_PTR;
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
        /* PRQA S 2985 ++ #considered an invalid operation, it is actually meaningful*/
        TransferConfig.SrcEndAddr = TransferConfig.SrcStartAddr + (uint16)(TransferConfig.Length \
                                    / (uint8)(1U << (uint8)TransferConfig.SrcUnit) * TransferConfig.SrcOffset);
        TransferConfig.DestEndAddr = TransferConfig.DestStartAddr + (uint16)(TransferConfig.Length \
                                     / (uint8)(1U << (uint8)TransferConfig.DestUnit) * TransferConfig.DestOffset);
        /* PRQA S 2985 -- */
        retCode = Dma_Hal_ConfigCh(master->txDMAChannel, &TransferConfig);
    }

    if (retCode == STATUS_SUCCESS)
    {
        /* last byte for stop condition */
        master->txRemainingBytes = 1U;

        if (master->receive == TRUE)
        {
            master->dmaReceiveBytes = master->rxRemainingBytes;

            /*cstat !MISRAC2012-Rule-11.4 Pointer conversion is required for reading and writing register operation.*/
            TransferConfig.DestStartAddr = (uint32)(master->rxData);
            /*cstat !MISRAC2012-Rule-11.4 Pointer conversion is required for reading and writing register operation.*/
            TransferConfig.SrcStartAddr = Eio_I2c_Hal_MasterComputeRxRegAddr(master);
            /*cstat -MISRAC2012-Rule-11.1 The conversion is needed by callback function type convert.*/
            TransferConfig.Callback = Eio_I2c_Hal_MasterDmaReceiveCallback;
            TransferConfig.UserArgs = NULL_PTR;
            TransferConfig.Type = DMA_TRANSFER_PERIPH2MEM;
            TransferConfig.DestUnit = DMA_TRANSFER_UNIT_1B;
            TransferConfig.SrcUnit = DMA_TRANSFER_UNIT_1B;
            TransferConfig.CircularMode = FALSE;
            TransferConfig.TriggerMode = FALSE;
#if defined (AC7840E)
            TransferConfig.DaisyMode = FALSE;
#endif
            TransferConfig.SrcOffset = 0U;
            TransferConfig.DestOffset = 1U;
            TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;

            if (master->rxRemainingBytes == 1UL)
            {
                TransferConfig.Length = (uint16)master->rxRemainingBytes;
                master->rxRemainingBytes = 0U;
            }
            else
            {
                TransferConfig.Length = (uint16)(master->rxRemainingBytes - 1U);
                /* Send NACK for last byte */
                master->rxRemainingBytes = 1U;
            }
            /* PRQA S 2985 ++ #considered an invalid operation, it is actually meaningful*/
            TransferConfig.SrcEndAddr = TransferConfig.SrcStartAddr + (uint16)(TransferConfig.Length \
                                        / (uint8)(1U << (uint8)TransferConfig.SrcUnit) * TransferConfig.SrcOffset);
            TransferConfig.DestEndAddr = TransferConfig.DestStartAddr + (uint16)(TransferConfig.Length \
                                         / (uint8)(1U << (uint8)TransferConfig.DestUnit) * TransferConfig.DestOffset);
            /* PRQA S 2985 -- */
            retCode = Dma_Hal_ConfigCh(master->rxDMAChannel, &TransferConfig);
        }
        else
        {
            /*cstat !MISRAC2012-Rule-11.4 Pointer conversion is required for reading and writing register operation.*/
            TransferConfig.DestStartAddr = (uint32)(&(master->dummyDmaReceive));
            /*cstat !MISRAC2012-Rule-11.4 Pointer conversion is required for reading and writing register operation.*/
            TransferConfig.SrcStartAddr = Eio_I2c_Hal_MasterComputeRxRegAddr(master);
            /*cstat -MISRAC2012-Rule-11.1 The conversion is needed by callback function type convert.*/
            TransferConfig.Callback = Eio_I2c_Hal_MasterDmaReceiveCallback;
            TransferConfig.UserArgs = NULL_PTR;
            TransferConfig.Type = DMA_TRANSFER_PERIPH2PERIPH;
            TransferConfig.DestUnit = DMA_TRANSFER_UNIT_1B;
            TransferConfig.SrcUnit = DMA_TRANSFER_UNIT_1B;
            TransferConfig.CircularMode = FALSE;
#if defined (AC7840E)
            TransferConfig.DaisyMode = FALSE;
#endif
            TransferConfig.TriggerMode = FALSE;
            TransferConfig.SrcOffset = 0U;
            TransferConfig.DestOffset = 0U;
            TransferConfig.Length = (uint16)master->rxRemainingBytes;
            /* PRQA S 2985 ++ #considered an invalid operation, it is actually meaningful*/
            TransferConfig.SrcEndAddr = TransferConfig.SrcStartAddr + (uint16)(TransferConfig.Length \
                                        / (uint8)(1U << (uint8)TransferConfig.SrcUnit) * TransferConfig.SrcOffset);
            TransferConfig.DestEndAddr = TransferConfig.DestStartAddr + (uint16)(TransferConfig.Length \
                                         / (uint8)(1U << (uint8)TransferConfig.DestUnit) * TransferConfig.DestOffset);
            /* PRQA S 2985 -- */
            retCode = Dma_Hal_ConfigCh(master->rxDMAChannel, &TransferConfig);
            master->rxRemainingBytes = 0U;
        }
    }

    return retCode;
}

static void Eio_I2c_Hal_MasterSendAddress(EIO_Type *baseAddr, const eio_i2c_master_state_t *master)
{
    uint8 addrByte;
    uint8 resourceIndex;    /* Index of first used internal resource instance (shifter and timer) */

    resourceIndex = master->eioCommon.ResourceIndex;

    /*PRQA S 4559 ++ # is being used as the first operand of this conditional.*/
    /* Address byte: slave 7-bit address + D = 0(transmit) or 1 (receive) */
    addrByte = (uint8)((uint8)(master->slaveAddress << 1U) + (uint8)(master->receive ? 1U : 0U));
    /*PRQA S 4559 -- */
    Eio_Reg_WriteShifterBuffer(baseAddr, I2C_TX_SHIFTER(resourceIndex), \
        (uint32)addrByte << 24U, EIO_SHIFTER_RW_MODE_BIT_SWAP);
}

static Hal_StatusType Eio_I2c_Hal_MasterInit(uint8 instance,
                                const eio_i2c_master_user_config_t *userConfigPtr,
                                eio_i2c_master_state_t *master)
{
    uint32 inputClock;
    Hal_StatusType clkErr;
    Hal_StatusType retCode = STATUS_ERROR;

    DEVICE_ASSERT(master != NULL_PTR);
    DEVICE_ASSERT(instance < EIO_INSTANCE_COUNT);

    /*PRQA S 2812,2842 ++ # the upper layer call guarantees that a null pointer will never appear.*/
    /* Get the protocol clock frequency */
    clkErr = Ckgen_Hal_GetFreq(g_eioClock[instance], &inputClock);
    DEVICE_ASSERT(clkErr == STATUS_SUCCESS);
    DEVICE_ASSERT(inputClock > 0U);
    /*PRQA S 2812,2842 -- */

    if (master != NULL_PTR)
    {
        /* Instruct the resource allocator that we need two shifters/timers */
        master->eioCommon.ResourceIndex = userConfigPtr->resourceIndex;
        master->eioCommon.ResourceCount = 2U;
        /* Common EIO driver initialization */
        (void)Eio_Hal_InitDriver(instance, &(master->eioCommon));

        /* Initialize driver-specific context structure */
        master->driverType = userConfigPtr->driverType;
        master->slaveAddress = userConfigPtr->slaveAddress;
        master->sdaPin = userConfigPtr->sdaPin;
        master->sclPin = userConfigPtr->sclPin;
        master->callback = userConfigPtr->callback;
        master->blocking = FALSE;
        master->driverIdle = TRUE;
        master->status = STATUS_IDLE;
        /* Store DMA channel numbers */
        master->txDMAChannel = userConfigPtr->txDMAChannel;
        master->rxDMAChannel = userConfigPtr->rxDMAChannel;

        /* Configure device for I2C mode */
        Eio_I2c_Hal_MasterConfigure(master, inputClock, userConfigPtr->baudRate);

        /* For DMA transfers we use timer interrupts to signal transfer end */
        master->eioCommon.Isr = Eio_I2c_Hal_MasterEndDmaTransfer;

        (void)clkErr;

        retCode =  STATUS_SUCCESS;
    }

    return retCode;
}

static void Eio_I2c_Hal_MasterDeinit(const eio_i2c_master_state_t *master)
{
    DEVICE_ASSERT(master != NULL_PTR);

    if (master != NULL_PTR)
    {
        /* Check if driver is busy */
        if (TRUE == master->driverIdle)
        {
            Eio_Hal_DeinitDriver(&(master->eioCommon));
        }
    }
}

static Hal_StatusType Eio_I2c_Hal_MasterStartTransfer(eio_i2c_master_state_t *master,
        uint32 size,
        boolean sendStop,
        boolean receive)
{
    EIO_Type *baseAddr;
    uint8 resourceIndex;    /* Index of first used internal resource instance (shifter and timer) */
    uint32 timeout = EIO_IIC_TIMEOUT_VALUE;
    Hal_StatusType Ret = STATUS_ERROR;

    DEVICE_ASSERT(master != NULL_PTR);

    if (master != NULL_PTR)
    {
        baseAddr = g_eioBase[master->eioCommon.Instance];
        resourceIndex = master->eioCommon.ResourceIndex;

        /* Check if bus is busy */
        if (TRUE == Eio_I2c_Hal_MasterBusBusy(baseAddr, master))
        {
            Ret = STATUS_I2C_BUS_BUSY;
        }
        else
        {
            /* Tx - one extra byte for stop condition */
            master->txRemainingBytes = size + 1U;
            master->rxRemainingBytes = size;
            master->status = STATUS_BUSY;
            master->driverIdle = FALSE;
            master->sendStop = sendStop;
            master->receive = receive;
            master->addrReceived = FALSE;
            /* Configure device for requested number of bytes, keeping the existing baud rate */
            Eio_I2c_Hal_MasterSetBytesNo(baseAddr, master);
            /* Enable timers and shifters */
            Eio_I2c_Hal_MasterEnableTransfer(master);

            /* Enable error interrupt for Rx shifter - for NACK detection */
            Eio_Reg_SetShifterErrorInterrupt(baseAddr, (uint8)(1U << I2C_RX_SHIFTER(resourceIndex)), TRUE);
            /* Enable interrupt for SCL timer - for end of transfer detection */
            Eio_Reg_SetTimerInterrupt(baseAddr, (uint8)(1U << I2C_SCL_TIMER(resourceIndex)), TRUE);

            if (STATUS_SUCCESS != Eio_I2c_Hal_MasterStartDmaTransfer(master))
            {
                Ret = STATUS_ERROR;
            }
            else
            {
                timeout = EIO_IIC_TIMEOUT_VALUE;
                while (Eio_Reg_GetShifterStatus(baseAddr, I2C_TX_SHIFTER(resourceIndex)) == FALSE)
                {
                    timeout --;
                    if (timeout == 0U)
                    {
                        master->status = STATUS_ERROR;
                        /* Force the transfer to stop */
                        Eio_I2c_Hal_MasterStopTransfer(master);
                        break;
                    }
                }
                /* Enable EIO DMA requests for tx shifters */
                Eio_Reg_SetShifterDMARequest(baseAddr, (uint8)(1U << I2C_TX_SHIFTER(resourceIndex)), TRUE);
                /* start tx dma channel */
                (void)Dma_Hal_StartCh(master->txDMAChannel);
                /* Send address to start transfer */
                Eio_I2c_Hal_MasterSendAddress(baseAddr, master);
                /* discard any leftover rx. data by send slave address*/

                timeout = EIO_IIC_TIMEOUT_VALUE;
                while (Eio_Reg_GetShifterStatus(baseAddr, I2C_RX_SHIFTER(resourceIndex)) == FALSE)
                {
                    timeout --;
                    if (timeout == 0U)
                    {
                        master->status = STATUS_ERROR;
                        /* Force the transfer to stop */
                        Eio_I2c_Hal_MasterStopTransfer(master);
                        break;
                    }
                }

                (void)Eio_Reg_ReadShifterBuffer(baseAddr, I2C_RX_SHIFTER(resourceIndex), EIO_SHIFTER_RW_MODE_BIT_SWAP);
                /* Enable EIO DMA requests for rx shifters */
                Eio_Reg_SetShifterDMARequest(baseAddr, (uint8)(1U << I2C_RX_SHIFTER(resourceIndex)), TRUE);
                /* start rx dma channel */
                (void)Dma_Hal_StartCh(master->rxDMAChannel);
                Ret = STATUS_SUCCESS;
            }
        }
    }

    return Ret;
}

static Hal_StatusType Eio_I2c_Hal_MasterSendData(eio_i2c_master_state_t *master,
                                    const uint8 *txBuff,
                                    uint32 txSize,
                                    boolean sendStop)
{
    DEVICE_ASSERT(master != NULL_PTR);
    DEVICE_ASSERT(txBuff != NULL_PTR);
    DEVICE_ASSERT(txSize > 0U);
    Hal_StatusType Ret = STATUS_ERROR;

    if (master != NULL_PTR)
    {
        /* Check if driver is busy */
        if (FALSE == master->driverIdle)
        {
            Ret =  STATUS_BUSY;
        }
        else
        {
            /* Initialize transfer data */
            master->txData = txBuff;
            /* Start the transfer */
            Ret =  Eio_I2c_Hal_MasterStartTransfer(master, txSize, sendStop, FALSE);
        }
    }

    return Ret;
}

static Hal_StatusType Eio_I2c_Hal_MasterReceiveData(eio_i2c_master_state_t *master,
                                       uint8 *rxBuff,
                                       uint32 rxSize,
                                       boolean sendStop)
{
    DEVICE_ASSERT(master != NULL_PTR);
    DEVICE_ASSERT(rxBuff != NULL_PTR);
    DEVICE_ASSERT(rxSize > 0U);
    Hal_StatusType Ret = STATUS_ERROR;

    if (master != NULL_PTR)
    {
        /* Check if driver is busy */
        if (FALSE == master->driverIdle)
        {
            Ret = STATUS_BUSY;
        }
        else
        {
            /* Initialize transfer data */
            master->rxData = rxBuff;
            /* Start the transfer */
            Ret =  Eio_I2c_Hal_MasterStartTransfer(master, rxSize, sendStop, TRUE);
        }
    }

    return Ret;
}

static Hal_StatusType Eio_I2c_Hal_MasterGetStatus(const eio_i2c_master_state_t *master)
{
    DEVICE_ASSERT(master != NULL_PTR);
    Hal_StatusType Ret = STATUS_ERROR;

    if (master != NULL_PTR)
    {
        if (FALSE == master->driverIdle)
        {
            Ret =  STATUS_BUSY;
        }
        else
        {
            Ret =  master->status;
        }
    }

    return Ret;
}

Std_ReturnType Eio_I2c_Hal_InitChannel
(
    uint8 HwChannel,
    const I2c_Hal_ChannelConfigType* ChannelConfigPtr
)
{
    Std_ReturnType ReturnType = E_OK;
    eio_i2c_master_user_config_t config;

    config.sclPin = ChannelConfigPtr->MasterConfigPtr->SclPin;
    config.sdaPin = ChannelConfigPtr->MasterConfigPtr->SdaPin;
    config.resourceIndex = ChannelConfigPtr->MasterConfigPtr->ResourceIndex;
    config.baudRate = ChannelConfigPtr->MasterConfigPtr->BaudRate;
    config.slaveAddress = ChannelConfigPtr->MasterConfigPtr->SlaveAddress;
    config.rxDMAChannel = ChannelConfigPtr->MasterConfigPtr->RxDmaChannel;
    config.txDMAChannel = ChannelConfigPtr->MasterConfigPtr->TxDmaChannel;
    /*PRQA S 0313 ++ # Casting to different function pointer type. */
    config.callback = (eio_i2c_master_callback_t)ChannelConfigPtr->Callback;
    /*PRQA S 0313 -- */
    config.driverType = EIO_DRIVER_TYPE_DMA;
    eio_i2c_master_state.eioCommon.ResourceIndex = 0;
    Eio_Hal_InitDevice(HwChannel);
    (void)Eio_I2c_Hal_MasterInit(HwChannel, &config, &eio_i2c_master_state);

    return ReturnType;
}

void Eio_I2c_Hal_DeInitChannel
(
    uint8 HwChannel
)
{
    Eio_I2c_Hal_MasterDeinit(&eio_i2c_master_state);
    Eio_Hal_DeinitDevice(HwChannel);
}

Std_ReturnType Eio_I2c_Hal_AsyncTransmit
(
    uint8 HwChannel,
    const DataTransmitType* DataTransmitPtr
)
{
    Std_ReturnType ReturnType;
    (void)HwChannel;

    if (I2C_WRITE == DataTransmitPtr->DirType)
    {
        ReturnType = (Std_ReturnType)Eio_I2c_Hal_MasterSendData(&eio_i2c_master_state, DataTransmitPtr->DataBufferPtr,
            DataTransmitPtr->DataLength, DataTransmitPtr->SendStop);
    }
    else
    {
        ReturnType = (Std_ReturnType)Eio_I2c_Hal_MasterReceiveData(&eio_i2c_master_state, \
            DataTransmitPtr->DataBufferPtr, DataTransmitPtr->DataLength, DataTransmitPtr->SendStop);
    }

    return ReturnType;
}

I2c_Hal_ChannelStatusType Eio_I2c_Hal_GetStatus
(
    uint8 HwChannel
)
{
    Hal_StatusType Status;
    I2c_Hal_ChannelStatusType ChannelStatus = I2C_CHANNEL_IDLE;
    Status = Eio_I2c_Hal_MasterGetStatus(&eio_i2c_master_state);
    (void)HwChannel;

    switch (Status)
    {
        case(STATUS_SUCCESS):
            ChannelStatus = I2C_CHANNEL_FINISHED;
            break;

        case(STATUS_BUSY):
            ChannelStatus = I2C_CHANNEL_BUSY_TRANSMIT;
            break;

        case(STATUS_IDLE):
            ChannelStatus = I2C_CHANNEL_IDLE;
            break;

        case(STATUS_ERROR):
        case(STATUS_I2C_RECEIVED_NACK):
        case(STATUS_I2C_TX_UNDERRUN):
        case(STATUS_I2C_RX_OVERRUN):
            ChannelStatus = I2C_CHANNEL_ERROR_PRESENT;
            break;

        default :
            /* do nothing */
            break;
    }

    return ChannelStatus;
}

/* =============================================  EOF  ============================================== */
