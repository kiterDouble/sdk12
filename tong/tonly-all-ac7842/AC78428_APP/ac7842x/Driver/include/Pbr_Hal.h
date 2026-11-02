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
 * @file Pbr_Hal.h
 *
 * @brief pbr hal define.
 */

#ifndef PBR_HAL_H
#define PBR_HAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ===========================================  INCLUDE FILES  =========================================== */
#include "AC784xx_Pbr_Reg.h"
/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */
/*PRQA S 3630 ++ # AMCE_QAC_3630*/
/*!
 * @brief pbr master configure structure
 */
typedef struct
{
    uint8 Value;/* register value */
    Pbr_MasterIdType MasterId;
} Pbr_MasterCfgType;

/*!
 * @brief pbr peripheral configure structure
 */
typedef struct
{
    uint8 Value;/* register value */
    Pbr_PeriphIdType PeriphId;
} Pbr_PeriphCfgType;

/*!
 * @brief pbr configure structure
 */
typedef struct
{
    uint8 MasterCfgCnt;
    uint8 PeriphCfgCnt;
    const Pbr_MasterCfgType *MasterCfg;
    const Pbr_PeriphCfgType *PeriphCfg;
} Pbr_CfgType;
/*PRQA S 3630 -- # AMCE_QAC_3630*/
/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/**
 * @brief Initialize pbr master and peripheral configuration
 * @note Function ID: DES_MCL_API_301
 * @param[in] Cfg: the pointer to the Pbr_CfgType structure
 * @return void
 */
void Pbr_Hal_Init(const Pbr_CfgType *Cfg);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* PBR_HAL_H */
/* =============================================  EOF  ============================================== */
