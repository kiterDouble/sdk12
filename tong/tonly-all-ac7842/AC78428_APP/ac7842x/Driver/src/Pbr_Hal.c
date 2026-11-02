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
 * @file Pbr_Hal.c
 *
 * @brief pbr hal source file.
 */
/*==============================================INCLUDE FILES=======================================*/
#include "Pbr_Hal.h"
/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

/* Implementation of Pbr_Hal_Init declared in Pbr_Hal.h */
void Pbr_Hal_Init(const Pbr_CfgType *Cfg) //PRQA S 1503 # This external HAL API is kept for upper-layer integration even when the current project does not call it directly.
{
    uint8 Idx; /* used for loop*/

    DEVICE_ASSERT(NULL_PTR != Cfg);
    /* PRQA S 2812 ++ # AMCE_MCL_2812*/
    DEVICE_ASSERT(NULL_PTR != Cfg->MasterCfg);
    /* PRQA S 2812 -- # AMCE_MCL_2812*/
    DEVICE_ASSERT(NULL_PTR != Cfg->PeriphCfg);

    if (NULL_PTR != Cfg->PeriphCfg)
    {
        for (Idx = 0; Idx < Cfg->PeriphCfgCnt; Idx++)
        {
            Pbr_Reg_SetPeriphAccess((uint32)Cfg->PeriphCfg[Idx].PeriphId, Cfg->PeriphCfg[Idx].Value);
        }
    }
    if (NULL_PTR != Cfg->MasterCfg)
    {
        for (Idx = 0; Idx < Cfg->MasterCfgCnt; Idx++)
        {
            Pbr_Reg_SetMasterPrivilege((uint32)Cfg->MasterCfg[Idx].MasterId, Cfg->MasterCfg[Idx].Value);
        }
    }
}
/* =============================================  EOF  ============================================== */
