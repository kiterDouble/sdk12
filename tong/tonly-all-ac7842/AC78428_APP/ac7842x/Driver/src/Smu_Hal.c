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
 * @file Smu_Hal.c
 *
 * @brief This file provides Hal Smu api.
 *
 */

/*==============================================INCLUDE FILES=======================================*/

#include "AC784xx_Smu_Reg.h"
#include "Smu_Hal.h"
#include "Ckgen_Hal.h"

#ifdef IRQ_CONTROL_IN_SMU_HAL
#include "Core_Hal.h"
#endif

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/
/*!
 * @brief pointer of ESM configuration.
 */
static const Smu_ConfigType *Smu_Hal_ConfigPtr;

/*============================================FUNCTION PROTOTYPES===================================*/
/*!
 * @brief Lock SMU registers write permission.
 * @note  Function ID: DES_SMU_API_001
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
void Smu_Hal_Lock(void)
{
    Smu_Reg_LatentFaultRegLock();
    Smu_Reg_SPFaultRegLock();
#if defined (AC7843X)
    Smu_Reg_HsmSPFaultRegLock();
#endif
}

/*!
 * @brief Unlock SMU registers write permission.
 * @note  Function ID: DES_SMU_API_002
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
void Smu_Hal_Unlock(void)
{
#if defined (AC7843X)
    Smu_Reg_HsmSPFaultRegUnlock();
#endif
    Smu_Reg_SPFaultRegUnlock();
    Smu_Reg_LatentFaultRegUnlock();
}

/*!
 * @brief Init SMU driver.
 * @note  Function ID: DES_SMU_API_003
 * @warning This function should be called at privileged mode.
 * @param[in] ConfigPtr: point of SMU driver configuration.
 * @return Command has been accepted or not.
 *            -STATUS_SUCCESS: command has been accepted.
 *            -STATUS_ERROR: command has not been accepted e.g. due to parameter error.
 */
Hal_StatusType Smu_Hal_Init(const Smu_ConfigType *ConfigPtr)
{
    Hal_StatusType ReturnVal = STATUS_ERROR;

    if ((NULL_PTR != ConfigPtr) && (ConfigPtr->Reset_Count <= SMU_RESET_THRESHOLD))
    {
        /* Enable SMU Clock. */
        (void) Ckgen_Hal_EnablePeriphClk(CKGEN_SMU_BUS_CLK, TRUE);
#if defined (AC7843X)
        (void) Ckgen_Hal_EnablePeriphClk(CKGEN_SMU_HSM_BUS_CLK, TRUE);
#endif
        /* Unlock SMU registers write permission. */
        Smu_Hal_Unlock();

        Smu_Hal_ConfigPtr = ConfigPtr;

        /* Configure reset count threshold, single point fault and latent fault enable. */
        Smu_Reg_SetResetCounterThreshold(ConfigPtr->Reset_Count);
        Smu_Reg_SetSPFaultResetEnable(ConfigPtr->Reset_Spfs);
        Smu_Reg_SetLatentFaultResetEnable(ConfigPtr->Reset_Lfs);

#if defined (AC7843X)
        Smu_Reg_SetHsmResetCounterThreshold(ConfigPtr->Reset_HsmCount);
        Smu_Reg_SetHsmSPFaultResetEnable(ConfigPtr->Reset_HsmSpfs);
        Smu_Reg_SetHsmSPFaultInterruptEnable(ConfigPtr->Irq_HsmSpfSrc);
#endif

#if defined (AC7840E) || defined (AC7842X) || defined (AC7843X)
        Smu_Reg_SetSPFaultInterruptEnable(ConfigPtr->Irq_SpfSrc);
        Smu_Reg_SetLatentFaultInterruptEnable(ConfigPtr->Irq_LfSrc);

#if defined (IRQ_CONTROL_IN_SMU_HAL) && (IRQ_CONTROL_IN_SMU_HAL == 1)
        if (0U != (ConfigPtr->Irq_SpfSrc
        + ConfigPtr->Irq_LfSrc
#if defined (AC7843X)
        + ConfigPtr->Irq_HsmSpfSrc
#endif
        ))
        {
            Core_Hal_EnableIrq(SMU_IRQn);
        }
#endif /* IRQ_CONTROL_IN_SMU_HAL */
#endif

#if defined (AC7840E)
        /* Configure FSP control register, FSP single point fault and latent fault enable. */
        Smu_Reg_SetSPFaultFspEnable(ConfigPtr->Fsp_SpfSrc);
        Smu_Reg_SetLatentFaultFspEnable(ConfigPtr->Fsp_LfSrc);
        Smu_Reg_SetFspControl(ConfigPtr->Fsp_Ctrl);
#endif

        ReturnVal = STATUS_SUCCESS;
    }

    return ReturnVal;
}

/*!
 * @brief De-Initialize SMU module.
 * @note  Function ID: DES_SMU_API_004
 * @warning This function should be called at privileged mode.
 * @param[in] void.
 * @return void
 */
void Smu_Hal_DeInit(void)
{
    DEVICE_ASSERT(Smu_Hal_ConfigPtr != NULL_PTR);

#if defined (AC7840E) || defined (AC7842X) || defined (AC7843X)
#if defined (IRQ_CONTROL_IN_SMU_HAL) && (IRQ_CONTROL_IN_SMU_HAL == 1)
    if (0U != (Smu_Hal_ConfigPtr->Irq_SpfSrc
        + Smu_Hal_ConfigPtr->Irq_LfSrc
#if defined (AC7843X)
        + Smu_Hal_ConfigPtr->Irq_HsmSpfSrc
#endif
        ))
    {
        Core_Hal_DisableIrq(SMU_IRQn);
        Core_Hal_ClearPendingIrq(SMU_IRQn);
    }
#endif /* IRQ_CONTROL_IN_SMU_HAL */
#endif

    /* Disable SMU Clock. */
    (void) Ckgen_Hal_EnablePeriphClk(CKGEN_SMU_BUS_CLK, FALSE);
#if defined (AC7843X)
    (void) Ckgen_Hal_EnablePeriphClk(CKGEN_SMU_HSM_BUS_CLK, FALSE);
#endif
}

/*!
 * @brief Get current SMU count, single point fault, latent fault values.
 * @note  Function ID: DES_SMU_API_005
 * @warning This function should be called at privileged mode.
 * @param[out] FaultPtr: pointer of current SMU status.
 * @return Command has been accepted or not.
 *            -STATUS_SUCCESS: command has been accepted.
 *            -STATUS_ERROR: command has not been accepted e.g. due to parameter error.
 *            -STATUS_SMU_FAULT_OCCURRED: Fault has been detected in current.
 */
Hal_StatusType Smu_Hal_GetCurrentFaults(Smu_FaultInfoType *FaultPtr)
{
    Hal_StatusType ReturnVal = STATUS_SUCCESS;

    if (NULL_PTR == FaultPtr)
    {
        ReturnVal = STATUS_ERROR;
    }
    else
    {
        FaultPtr->Count = Smu_Reg_GetResetCounter();
        FaultPtr->Spfs = Smu_Reg_GetSPFaultStatus();
        FaultPtr->Lfs = Smu_Reg_GetLatentFaultStatus();
        FaultPtr->SwSpfs = Smu_Reg_GetSoftwareSPFault();
        FaultPtr->SwLfs = Smu_Reg_GetSoftwareLatentFault();
#if defined (AC7843X)
        FaultPtr->HsmCount = Smu_Reg_GetHsmResetCounter();
        FaultPtr->HsmSpfs  = Smu_Reg_GetHsmSPFaultStatus();
        FaultPtr->HsmSwSpfs = Smu_Reg_GetHsmSoftwareSPFault();
#endif
#if defined (AC7840E)
        FaultPtr->FspState = Smu_Reg_GetFspState();
#endif
        /* Report fault detected when accumulated count is not zero. */
        if ((FaultPtr->Count > 0U)
#if defined (AC7843X)
            || (FaultPtr->HsmCount > 0U)
#endif
        )
        {
            ReturnVal = STATUS_SMU_FAULT_OCCURRED;
        }
    }

    return ReturnVal;
}

/*!
 * @brief  Get last SMU count, single point fault, latent fault values.
 * @note  Function ID: DES_SMU_API_006
 * @warning This function should be called at privileged mode.
 * @param[out] FaultPtr: pointer of last SMU status.
 * @return Command has been accepted or not.
 *            -STATUS_SUCCESS: command has been accepted.
 *            -STATUS_ERROR: command has not been accepted e.g. due to parameter error.
 *            -STATUS_SMU_FAULT_OCCURRED: Fault has been detected at last boot.
 */
Hal_StatusType Smu_Hal_GetLastFaults(Smu_FaultInfoType *FaultPtr)
{
    Hal_StatusType ReturnVal = STATUS_SUCCESS;

    if (NULL_PTR == FaultPtr)
    {
        ReturnVal = STATUS_ERROR;
    }
    else
    {
        FaultPtr->Count = Smu_Reg_GetResetCounterShadow();
        FaultPtr->Spfs = Smu_Reg_GetSPFaultStatusShadow();
        FaultPtr->Lfs = Smu_Reg_GetLatentFaultStatusShadow();
        FaultPtr->SwSpfs = Smu_Reg_GetSoftwareSPFaultShadow();
        FaultPtr->SwLfs = Smu_Reg_GetSoftwareLatentFaultShadow();
#if defined (AC7843X)
        FaultPtr->HsmCount = Smu_Reg_GetHsmResetCounterShadow();
        FaultPtr->HsmSpfs  = Smu_Reg_GetHsmSPFaultStatusShadow();
        FaultPtr->HsmSwSpfs = Smu_Reg_GetHsmSoftwareSPFaultShadow();
#endif
#if defined (AC7840E)
        /* FSP state has no shadow register, set to 0 for last faults query. */
        FaultPtr->FspState = 0U;
#endif
        /* Report fault detected when accumulated count is not zero. */
        if ((FaultPtr->Count > 0U)
#if defined (AC7843X)
            || (FaultPtr->HsmCount > 0U)
#endif
        )
        {
            ReturnVal = STATUS_SMU_FAULT_OCCURRED;
        }

#if defined (AC7843X)
        Smu_Reg_SetHsmSPFaultStatusShadow(0xFFFFFFFFU);
        Smu_Reg_ClearHsmResetCounterShadow();
#endif
        Smu_Reg_SetSPFaultStatusShadow(0xFFFFFFFFU);
        Smu_Reg_SetLatentFaultStatusShadow(0xFFFFFFFFU);
        Smu_Reg_ClearResetCounterShadow();
    }

    return ReturnVal;
}

/*!
 * @brief Clear current SMU count, single point fault, latent fault values.
 * @note  Function ID: DES_SMU_API_007
 * @warning This function should be called at privileged mode.
 * @param[in] Reset: Reset SMU count to zero.
 * @return void
 */
void Smu_Hal_ClearCurrentFaultInfo(boolean Reset)
{
    Smu_Reg_SetSPFaultStatus(0xFFFFFFFFU);
    Smu_Reg_SetLatentFaultStatus(0xFFFFFFFFU);
    Smu_Reg_SetSoftwareSPFault(0U);
    Smu_Reg_SetSoftwareLatentFault(0U);

#if defined (AC7843X)
    Smu_Reg_SetHsmSPFaultStatus(0xFFFFFFFFU);
    Smu_Reg_SetHsmSoftwareSPFault(0U);
#endif

#if defined (AC7840E)
    /* Release FSP fault state to non-fault state. */
    Smu_Reg_SetFspRelease();
#endif

    if (Reset == TRUE)
    {
        Smu_Reg_ClearResetCounter();
#if defined (AC7843X)
        Smu_Reg_ClearHsmResetCounter();
#endif
    }
}

/*!
 * @brief Assert a software single point or latent fault to SMU.
 * @note  Function ID: DES_SMU_API_008
 * @warning This function should be called at privileged mode.
 * @param[in] FaultType: Fault type, software single point or latent fault.
 * @param[in] FaultId: Fault ID.
 * @return Command has been accepted or not.
 *            -STATUS_SUCCESS: command has been accepted.
 *            -STATUS_ERROR: command has not been accepted e.g. due to parameter error.
 */
Hal_StatusType Smu_Hal_AssertSwFault(Smu_FaultType FaultType, uint8 FaultId)
{
    Hal_StatusType ReturnVal = STATUS_SUCCESS;
    uint32 Fault = BIT_SHIFT(FaultId);

    if ((SMU_SWSPF == FaultType) && (FaultId < (uint8)SMU_SWSPF_MAX))
    {
        /* Reserve previous software single point fault. */
        Fault |= Smu_Reg_GetSoftwareSPFault();

        /* First clean, and then set fault. */
        Smu_Reg_SetSoftwareSPFault(0U);
        Smu_Reg_SetSoftwareSPFault(Fault);
    }
    else if ((SMU_SWLF == FaultType) && (FaultId < (uint8)SMU_SWLF_MAX))
    {
        /* Reserve previous latent fault. */
        Fault |= Smu_Reg_GetSoftwareLatentFault();

        /* First clean, and then set fault. */
        Smu_Reg_SetSoftwareLatentFault(0U);
        Smu_Reg_SetSoftwareLatentFault(Fault);
    }
#if defined (AC7843X)
    else if ((SMU_HSM_SWSPF == FaultType) && (FaultId < (uint8)SMU_HSM_SWSPF_MAX))
    {
        /* Reserve previous HSM single point fault. */
        Fault |= Smu_Reg_GetHsmSoftwareSPFault();

        /* First clean, and then set fault. */
        Smu_Reg_SetHsmSoftwareSPFault(0U);
        Smu_Reg_SetHsmSoftwareSPFault(Fault);
    }
#endif
    else
    {
        ReturnVal = STATUS_ERROR;
    }

    return ReturnVal;
}

/*!
 * @brief Deassert a software single point or latent fault to SMU.
 * @note  Function ID: DES_SMU_API_009
 * @warning This function should be called at privileged mode.
 * @param[in] FaultType: Fault type, software single point or latent fault.
 * @param[in] FaultId: Fault ID.
 * @return Command has been accepted or not.
 *            -STATUS_SUCCESS: command has been accepted.
 *            -STATUS_ERROR: command has not been accepted e.g. due to parameter error.
 */
Hal_StatusType Smu_Hal_DeassertSwFault(Smu_FaultType FaultType, uint8 FaultId)
{
    Hal_StatusType ReturnVal = STATUS_SUCCESS;
    uint32 Fault = ~(BIT_SHIFT(FaultId));

    if ((SMU_SWSPF == FaultType) && (FaultId < (uint8)SMU_SWSPF_MAX))
    {
        /* Reserve previous software single point fault. */
        Fault &= Smu_Reg_GetSoftwareSPFault();

        Smu_Reg_SetSoftwareSPFault(Fault);

        /* If no available software single point fault, clear software single point fault status. */
        if (0U == Fault)
        {
            Smu_Reg_SetSPFaultStatus(BIT_SHIFT((uint8)SMU_HWSPF_MAX));
        }
    }
    else if ((SMU_SWLF == FaultType) && (FaultId < (uint8)SMU_SWLF_MAX))
    {
        /* Reserve previous latent fault. */
        Fault &= Smu_Reg_GetSoftwareLatentFault();

        Smu_Reg_SetSoftwareLatentFault(Fault);

        /* If no available software latent fault, clear software latent fault status. */
        if (0U == Fault)
        {
            Smu_Reg_SetLatentFaultStatus(BIT_SHIFT((uint8)SMU_HWLF_MAX));
        }
    }
#if defined (AC7843X)
    else if((SMU_HSM_SWSPF == FaultType) && (FaultId < (uint8)SMU_HSM_SWSPF_MAX))
    {
        /* Reserve previous software single point fault. */
        Fault &= Smu_Reg_GetHsmSoftwareSPFault();

        Smu_Reg_SetHsmSoftwareSPFault(Fault);

        /* If no available software single point fault, clear software single point fault status. */
        if (0U == Fault)
        {
            Smu_Reg_SetHsmSPFaultStatus(BIT_SHIFT((uint8)SMU_HSM_HWSPF_MAX));
        }
    }
#endif
    else
    {
        ReturnVal = STATUS_ERROR;
    }

    return ReturnVal;
}

/*!
 * @brief Inject a hardware fault to SMU.
 * @note  Function ID: DES_SMU_API_010
 * @warning This function should be called at privileged mode.
 * @param[in] FaultType: Fault type, Hardware single point or latent fault.
 * @param[in] FaultId: Fault ID.
 * @return Command has been accepted or not.
 *            -STATUS_SUCCESS: command has been accepted.
 *            -STATUS_ERROR: command has not been accepted e.g. due to parameter error.
 */
Hal_StatusType Smu_Hal_InjectFault(Smu_FaultType FaultType, uint8 FaultId)
{
    Hal_StatusType ReturnVal = STATUS_SUCCESS;
    uint32 Fault = BIT_SHIFT(FaultId);

    if ((SMU_HWSPF == FaultType) && (FaultId < (uint8)SMU_HWSPF_MAX))
    {
        Fault |= SMU_PATHCHK0_EN_Msk;
        Smu_Reg_SetPathCheck0(Fault);
    }
    else if ((SMU_HWLF == FaultType) && (FaultId < (uint8)SMU_HWLF_MAX))
    {
        Fault |= SMU_PATHCHK1_EN_Msk;
        Smu_Reg_SetPathCheck1(Fault);
    }
#if defined (AC7843X)
    else if ((SMU_HSM_HWSPF == FaultType) && (FaultId < (uint8)SMU_HSM_HWSPF_MAX))
    {
        Fault |= SMU_HSM_PATHCHK0_HSM_EN_Msk;
        Smu_Reg_SetHsmPathCheck0(Fault);
    }
#endif
    else
    {
        ReturnVal = STATUS_ERROR;
    }

    return ReturnVal;
}

/*!
 * @brief Clear a hardware fault to SMU.
 * @note  Function ID: DES_SMU_API_011
 * @warning This function should be called at privileged mode.
 * @param[in] FaultType: Fault type, Hardware single point or latent fault.
 * @param[in] FaultId: Fault ID.
 * @return Command has been accepted or not.
 *            -STATUS_SUCCESS: command has been accepted.
 *            -STATUS_ERROR: command has not been accepted e.g. due to parameter error.
 */
Hal_StatusType Smu_Hal_ClearFault(Smu_FaultType FaultType, uint8 FaultId)
{
    Hal_StatusType ReturnVal = STATUS_SUCCESS;
    uint32 Fault = BIT_SHIFT(FaultId);

    if ((SMU_HWSPF == FaultType) && (FaultId < (uint8)SMU_HWSPF_MAX))
    {
        Smu_Reg_SetSPFaultStatus(Fault);
    }
    else if ((SMU_HWLF == FaultType) && (FaultId < (uint8)SMU_HWLF_MAX))
    {
        Smu_Reg_SetLatentFaultStatus(Fault);
    }
#if defined (AC7843X)
    else if ((SMU_HSM_HWSPF == FaultType) && (FaultId < (uint8)SMU_HSM_HWSPF_MAX))
    {
        Smu_Reg_SetHsmSPFaultStatus(Fault);
    }
#endif
    else
    {
        ReturnVal = STATUS_ERROR;
    }

    return ReturnVal;
}

#if defined (IRQ_CONTROL_IN_SMU_HAL) && (IRQ_CONTROL_IN_SMU_HAL == 1)
#if defined (AC7840E) || defined (AC7842X) || defined (AC7843X)
/*!
 * @brief SMU IRQ handler.
 * @note  Function ID: DES_SMU_API_012
 * @warning This function should be called at privileged mode.
 * @param[in] void
 * @return void
 */
ISR(SMU_IRQHandler)  //PRQA S 1503,3408 # it is handler.
{
    Hal_StatusType Ret;
    Smu_FaultInfoType Fault;

    /* Read more error information. */
    Ret = Smu_Hal_GetCurrentFaults(&Fault);

    /* Clear SMU status. */
    Smu_Hal_ClearCurrentFaultInfo(FALSE);
    if (Ret != STATUS_SUCCESS)
    {
        /* User to clear fault. */

        if ((Smu_Hal_ConfigPtr != NULL_PTR) && (Smu_Hal_ConfigPtr->Irq_Callback != NULL_PTR))
        {
            Smu_Hal_ConfigPtr->Irq_Callback((void *)&Fault);
        }
    }
}
#endif
#endif
