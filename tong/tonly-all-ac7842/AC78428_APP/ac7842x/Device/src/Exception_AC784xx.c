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

/**
 * @file Exception_AC784xx.c
 *
 * @brief This file provides system clock config integration functions.
 *
 */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Device_Types.h"
#ifdef EEP_MPU_PROTECTION
#include "Mpu_Hal.h"
#endif
/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/* ======================================  Functions define  ======================================== */
WEAK void HardFault_Custom_Callback(void) //PRQA S 1505,3408 # AMCE_SYS_1505,AMCE_SYS_3408
{

}
/**
 * @brief Override default Hardfault handler for print backtrace.
 * @return void
 */
/*PRQA S 3006 ++ # AMCE_SYS_3006*/
void HardFault_Handler(void) //PRQA S 1503,3408 # AMCE_SYS_1503,AMCE_SYS_3408
{
#ifdef EEP_MPU_PROTECTION
    Hal_StatusType Status;
    Mpu_SlaveType SlaveId;
    Mpu_ErrorInfoType ErrorInfo;
#endif
#if defined (AC7843X)
    /*open hsm debug path*/
    (*(volatile uint32 *)0xE0084018U) &= ~(1U << 1U);
#endif
    HardFault_Custom_Callback();    //PRQA S 2987 # AMCE_SYS_2987
#ifdef EEP_MPU_PROTECTION
    Status = Mpu_Hal_GetErrorSlaveId(&SlaveId);
    if (STATUS_ERROR == Status)
    {
        (void)Mpu_Hal_GetSlaveErrorInfo(SlaveId, &ErrorInfo);
        Mpu_Hal_ClearSlaveError(SlaveId);
    }
    else
#endif
    {
        for (;;)
        {
            ASMV_KEYWORD("nop"); /* No operation*/ //PRQA S 1006 # AMCE_SYS_1006*/
        }
    }
}
/*PRQA S 3006 -- */
/*cstat +MISRAC2012-Rule-11.4*/
/* =============================================  EOF  ============================================== */
