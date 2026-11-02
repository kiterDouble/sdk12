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
 * @file eio_comm.c
 *
 * @brief This file provides eio comm integration functions.
 *
 */

/* PRQA S 1503 EOF # AMCE_QAC_NoUsedApi */
/* ===========================================  Includes  =========================================== */
#include "AC784xx_Eio_Reg.h"
#include "Eio_Common_Hal.h"
#include "Eio_Hal_Types.h"
#include "Ckgen_Hal.h"
#include "Rcm_Hal.h"
#include "Core_Hal.h"

/* ============================================  Define  ============================================ */

/* ===========================================  Typedef  ============================================ */
/**
 * @brief EIO device context structure.
 * @note DES ID: DES_EIO_TYP_001
 */
typedef struct
{
    boolean InitState;                                              /*!< Initialization state flag */
    uint8 ResourceAllocation;                                      /*!< Mask to keep track of resources allocated on current device */
    Eio_CommonStateType *EioStatePtr[EIO_MAX_SHIFTER_COUNT];       /*!< Array of pointers to runtime state structures. Each EIO instance can have at most
                                                                      one driver instance per shifter. */
} Eio_DeviceStateType;

/* ==========================================  Variables  =========================================== */
/**
 * @brief Table of base addresses for EIO instances.
 * @note DES ID: DES_EIO_VAR_001
 */
EIO_Type * const EioBase[EIO_INSTANCE_COUNT] = EIO_BASE_PTRS;

/**
 * @brief Table of device state structures.
 * @note DES ID: DES_EIO_VAR_002
 */
static Eio_DeviceStateType EioDeviceState[EIO_INSTANCE_COUNT];

/**
 * @brief Table of EIO IRQ numbers.
 * @note DES ID: DES_EIO_VAR_003
 */
static const IRQn_Type EioIrqId[EIO_INSTANCE_COUNT] = EIO_IRQS;

/**
 * @brief EIO bus clock sources for getting the input clock frequency.
 * @note DES ID: DES_EIO_VAR_004
 */
static const Ckgen_BusClkIdType Eio_HalBusClock[EIO_INSTANCE_COUNT] = {CKGEN_EIO_BUS_CLK};

/**
 * @brief EIO clock identifiers.
 * @note DES ID: DES_EIO_VAR_005
 */
const Ckgen_ClkIdType Eio_HalClock[EIO_INSTANCE_COUNT] = {CKGEN_EIO_CLK};

/**
 * @brief EIO reset control identifiers.
 * @note DES ID: DES_EIO_VAR_006
 */
static const Rcm_ResetIDType EioHalClockReset[EIO_INSTANCE_COUNT] = {RCM_RESET_ID_EIO};

/**
 * @brief EIO DMA request sources for each shifter.
 * @note DES ID: DES_EIO_VAR_007
 */
const Dma_RequestSourceType EioDMASrc[EIO_INSTANCE_COUNT][EIO_MAX_SHIFTER_COUNT] =
        {{DMA_REQ_EIO_SHIFTER0, DMA_REQ_EIO_SHIFTER1, DMA_REQ_EIO_SHIFTER2, DMA_REQ_EIO_SHIFTER3}};

/* ====================================  Functions declaration  ===================================== */
/**
 * @brief EIO interrupt service routine.
 *
 * @details
 * This is the interrupt handler for all EIO instances. It checks for pending
 * shifter, shifter error, and timer events, and calls the appropriate driver
 * ISR callback for each driver instance that has pending events.
 *
 * @param[in] void: None
 *
 * @return void
 *
 * @pre
 *     EIO device must be initialized.
 *     Driver instances must be properly registered.
 *
 * @post
 *     Driver ISR callbacks are invoked for pending events.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     This function is called from interrupt context.
 *     DES ID: DES_EIO_API_010
 */
ISR(EIO_IRQHandler);

/* =====================================  Functions definition  ===================================== */
/**
 * @brief Allocate a specific set of timers and shifters for a driver instance.
 *
 * @details
 * This function attempts to allocate exactly the shifters and timers specified by
 * Driver->ResourceIndex and Driver->ResourceCount. It checks whether the requested
 * resources are available and marks them as busy if so.
 *
 * @param[in] Instance: EIO peripheral instance number.
 * @param[in] Driver: Pointer to the EIO common driver context structure.
 *                    Driver->ResourceIndex must be set to the desired first resource index.
 *                    Driver->ResourceCount must be set to the required number of resources.
 *
 * @return Hal_StatusType
 *     STATUS_SUCCESS: Requested resources allocated successfully.
 *     STATUS_ERROR: One or more requested resources are already in use.
 *
 * @pre
 *     Instance must be valid and less than EIO_INSTANCE_COUNT.
 *     Driver->ResourceIndex + Driver->ResourceCount must not exceed EIO_MAX_SHIFTER_COUNT.
 *
 * @post
 *     If successful, resource allocation map is updated.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     DES ID: DES_EIO_API_001
 */
static Hal_StatusType Eio_Hal_AllocateFixed(uint8 Instance, const Eio_CommonStateType *Driver)
{
    uint8 Mask;
    Hal_StatusType Status = STATUS_ERROR;

    /*PRQA S 2812 ++ # AMCE_EIO_2812.*/
    Mask = (uint8)((1U << Driver->ResourceCount) - 1U) << Driver->ResourceIndex;
    /*PRQA S 2812 -- */

    /* Check if the requested resources are available */
    if ((EioDeviceState[Instance].ResourceAllocation & Mask) == 0U)
    {
        /* Mark resources as busy */
        EioDeviceState[Instance].ResourceAllocation |= Mask;
        Status = STATUS_SUCCESS;
    }

    return Status;
}

/**
 * @brief De-allocate timers and shifters for a driver instance.
 *
 * @details
 * This function releases the EIO resources (shifters and timers) that were previously
 * allocated to a driver instance. It clears the corresponding bits in the resource
 * allocation map to mark them as available.
 *
 * @param[in] Instance: EIO peripheral instance number.
 * @param[in] Driver: Pointer to the EIO common driver context structure.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than EIO_INSTANCE_COUNT.
 *     Driver must have been previously allocated resources.
 *
 * @post
 *     Resource allocation map is updated to free the resources.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     DES ID: DES_EIO_API_002
 */
static void Eio_Hal_Deallocate(uint8 Instance, const Eio_CommonStateType *Driver)
{
    uint8 Mask;

    /*PRQA S 2812 ++ # AMCE_EIO_2812.*/
    Mask = (uint8)((1U << Driver->ResourceCount) - 1U);
    /*PRQA S 2812 -- */
    Mask <<= Driver->ResourceIndex;
    EioDeviceState[Instance].ResourceAllocation &= (uint8)~Mask;
}

/**
 * @brief Initializes the resources for the current driver.
 *
 * @details
 * This function initializes all shifters and timers allocated to the driver.
 * It disables all modes, interrupts and DMA requests, and clears any pending
 * status flags for each resource.
 *
 * @param[in] Instance: EIO peripheral instance number.
 * @param[in] Driver: Pointer to the EIO common driver context structure.
 *
 * @return void
 *
 * @pre
 *     Instance must be valid and less than EIO_INSTANCE_COUNT.
 *     Resources must have been allocated to the driver.
 *
 * @post
 *     All allocated shifters and timers are in disabled and clean state.
 *
 * @note
 *     Reentrancy: Non-reentrant.
 *     DES ID: DES_EIO_API_003
 */
static void Eio_Hal_ResourcesInit(uint8 Instance, const Eio_CommonStateType *Driver)
{
    uint8 Resource;
    uint8 ResourceCount;
    uint8 ResourceIndex;
    EIO_Type *BaseAddr = EioBase[Instance];

    ResourceCount = Driver->ResourceCount;
    ResourceIndex = Driver->ResourceIndex;
    for (Resource = ResourceIndex; Resource < (ResourceIndex + ResourceCount); Resource++)
    {
        /* Ensure all shifters/timers are disabled */
        Eio_Reg_SetShifterMode(BaseAddr, Resource, EIO_SHIFTER_MODE_DISABLED);
        Eio_Reg_SetTimerMode(BaseAddr, Resource, EIO_TIMER_MODE_DISABLED);
        /* Ensure all interrupts and DMA requests are disabled */
        Eio_Reg_SetShifterInterrupt(BaseAddr, (uint8)(1U << Resource), FALSE);
        Eio_Reg_SetShifterErrorInterrupt(BaseAddr, (uint8)(1U << Resource), FALSE);
        Eio_Reg_SetTimerInterrupt(BaseAddr, (uint8)(1U << Resource), FALSE);
        Eio_Reg_SetShifterDMARequest(BaseAddr, (uint8)(1U << Resource), FALSE);
        /* Clear any leftover flags */
        Eio_Reg_ClearShifterStatus(BaseAddr, Resource);
        Eio_Reg_ClearShifterErrorStatus(BaseAddr, Resource);
        Eio_Reg_ClearTimerStatus(BaseAddr, Resource);
    }
}

/* Implementation of Eio_Hal_InitDevice declared in Eio_Common_Hal.h */
void Eio_Hal_InitDevice(uint8 Instance)
{
    DEVICE_ASSERT(Instance < EIO_INSTANCE_COUNT);

    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    EIO_Type *BaseAddr = EioBase[Instance];
    uint8 Count;
    /* Check if the device state structure is already initialized */

    if (FALSE == EioDeviceState[Instance].InitState)
    {
        (void)Ckgen_Hal_EnablePeriphClk(Eio_HalBusClock[Instance], TRUE);
        Rcm_Hal_SetResetState(EioHalClockReset[Instance], RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState(EioHalClockReset[Instance], RCM_RESET_STATE_DEASSERT);

        /* Reset EIO module */
        Eio_Reg_Init(BaseAddr);
        /* Enable EIO interrupt in the interrupt manager */
        Core_Hal_EnableIrq(EioIrqId[Instance]);
        /* Enable module */
        Eio_Reg_SetEnable(BaseAddr, TRUE);
        /* Initialize device structure */
        EioDeviceState[Instance].ResourceAllocation = 0U;
        /* Clear state structures array */
        for (Count = 0U; Count < EIO_MAX_SHIFTER_COUNT; Count++)
        {
            EioDeviceState[Instance].EioStatePtr[Count] = NULL_PTR;
        }

        EioDeviceState[Instance].InitState = TRUE;
    }
    /*PRQA S 2842 -- */
}

/* Implementation of Eio_Hal_DeinitDevice declared in Eio_Common_Hal.h */
void Eio_Hal_DeinitDevice(uint8 Instance)
{
    DEVICE_ASSERT(Instance < EIO_INSTANCE_COUNT);

    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    EIO_Type *BaseAddr = EioBase[Instance];

    if (EioDeviceState[Instance].ResourceAllocation == 0U)
    {
        /* Reset EIO module */
        Eio_Reg_Init(BaseAddr);

        Core_Hal_DisableIrq(EioIrqId[Instance]);

        /* Disable EIO module */
        Rcm_Hal_SetResetState(EioHalClockReset[Instance], RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState(EioHalClockReset[Instance], RCM_RESET_STATE_DEASSERT);
        (void)Ckgen_Hal_EnablePeriphClk(Eio_HalBusClock[Instance], FALSE);

        /* Disable EIO interrupt in the interrupt manager */
        Core_Hal_ClearPendingIrq(EioIrqId[Instance]);

        /* Free resources */
        EioDeviceState[Instance].InitState = FALSE;
    }
    /*PRQA S 2842 -- */
}

/* Implementation of Eio_Hal_Reset declared in Eio_Common_Hal.h */
void Eio_Hal_Reset(uint8 Instance)
{
    DEVICE_ASSERT(Instance < EIO_INSTANCE_COUNT);

    /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
    EIO_Type *BaseAddr = EioBase[Instance];
    /*PRQA S 2842 -- */

    /* Reset EIO module */
    Eio_Reg_Init(BaseAddr);
}

/* Implementation of Eio_Hal_InitDriver declared in Eio_Common_Hal.h */
Hal_StatusType Eio_Hal_InitDriver(uint8 Instance, Eio_CommonStateType *Driver)
{
    DEVICE_ASSERT(Instance < EIO_INSTANCE_COUNT);
    DEVICE_ASSERT(Driver != NULL_PTR);

    uint16 Count;
    Hal_StatusType RetCode;
    Eio_CommonStateType **EioStatePtr;

    /*PRQA S 2812 ++ # AMCE_EIO_2812.*/
    /* Allocate the specifically requested shifters and timers */
    DEVICE_ASSERT(((uint8)(Driver->ResourceIndex + Driver->ResourceCount)) <= (uint8)EIO_MAX_SHIFTER_COUNT);
    /*PRQA S 2812 -- */
    RetCode = Eio_Hal_AllocateFixed(Instance, Driver);

    if (RetCode != STATUS_SUCCESS)
    {
        /* no available / requested resources found */
        RetCode = STATUS_ERROR;
    }
    else
    {
        /*PRQA S 2842 ++ # AMCE_EIO_2842.*/
        /* get driver list for this device instance */
        EioStatePtr = EioDeviceState[Instance].EioStatePtr;
        /*PRQA S 2842 -- */
        /* find an empty state structure slot for the driver */
        for (Count = 0U; Count < EIO_MAX_SHIFTER_COUNT; Count++)
        {
            if (EioStatePtr[Count] == NULL_PTR)
            {
                /* found it, place the new driver struct here */
                EioStatePtr[Count] = Driver;
                break;
            }
        }
        /* no need to check if there was room since allocation was successful */
        DEVICE_ASSERT(Count < EIO_MAX_SHIFTER_COUNT);

        /* initialize the allocated resources */
        Eio_Hal_ResourcesInit(Instance, Driver);

        /*PRQA S 2812 ++ # AMCE_EIO_2812.*/
        /* init the rest of the common state structure */
        Driver->Isr = NULL_PTR;
        Driver->Instance = Instance;
        /*PRQA S 2812 -- */
        RetCode = STATUS_SUCCESS;
    }

    return RetCode;
}

/* Implementation of Eio_Hal_DeinitDriver declared in Eio_Common_Hal.h */
void Eio_Hal_DeinitDriver(const Eio_CommonStateType *Driver)
{
    DEVICE_ASSERT(Driver != NULL_PTR);

    uint16 Count;
    uint8 Instance;
    Eio_CommonStateType **EioStatePtr;

    /*PRQA S 2812 ++ # AMCE_EIO_2812.*/
    Instance = Driver->Instance;
    /*PRQA S 2812 -- */
    /* get driver list for this device instance */
    EioStatePtr = EioDeviceState[Instance].EioStatePtr;
    /* find the driver in the list */
    for (Count = 0U; Count < EIO_MAX_SHIFTER_COUNT; Count++)
    {
        if (EioStatePtr[Count] == Driver)
        {
            /* found it - remove it */
            EioStatePtr[Count] = NULL_PTR;
            break;
        }
    }
    DEVICE_ASSERT(Count < EIO_MAX_SHIFTER_COUNT);

    /* de-allocate timers and shifters for the driver */
    Eio_Hal_Deallocate(Instance, Driver);
}

ISR(EIO_IRQHandler)
{
    uint16 Count;
    uint8 Instance;
    uint32 ResourceMask;
    uint32 ShifterEvents;
    uint32 ShifterErrorEvents;
    uint32 TimerEvents;
    uint32 EnabledInterrupts;
    EIO_Type const *BaseAddr;
    Eio_CommonStateType * const *DriverList;
    Eio_CommonStateType *DriverState;

    Instance = 0U;
    BaseAddr = EioBase[Instance];
    /* get masks of EIO events */
    /* read enabled interrupts in a separate instruction to avoid MISRA violation */
    EnabledInterrupts = Eio_Reg_GetAllShifterInterrupt(BaseAddr);
    ShifterEvents = Eio_Reg_GetAllShifterStatus(BaseAddr) & EnabledInterrupts;
    EnabledInterrupts = Eio_Reg_GetAllShifterErrorInterrupt(BaseAddr);
    ShifterErrorEvents = Eio_Reg_GetAllShifterErrorStatus(BaseAddr) & EnabledInterrupts;
    EnabledInterrupts = Eio_Reg_GetAllTimerInterrupt(BaseAddr);
    TimerEvents = Eio_Reg_GetAllTimerStatus(BaseAddr) & EnabledInterrupts;
    /* get driver list for this device instance */
    DriverList = EioDeviceState[Instance].EioStatePtr;
    /* check which driver instances need to be serviced */
    for (Count = 0U; Count < EIO_MAX_SHIFTER_COUNT; Count++)
    {
        DriverState = DriverList[Count];
        /* check if driver is initialized and uses interrupts */
        if ((DriverState != NULL_PTR) && (DriverState->Isr != NULL_PTR))
        {
            /* compute mask of shifters/timers used by this driver */
            ResourceMask = ((1UL << DriverState->ResourceCount) - 1U) << DriverState->ResourceIndex;
            /* check if this instance has any pending events */
            if (((ShifterEvents & ResourceMask) != 0U) ||
                    ((ShifterErrorEvents & ResourceMask) != 0U) ||
                    ((TimerEvents & ResourceMask) != 0U))
            {
                /* there is an event for the current instance - call the isr */
                (DriverState->Isr)(DriverState);
            }
        }
    }
}
/* =============================================  EOF  ============================================== */
