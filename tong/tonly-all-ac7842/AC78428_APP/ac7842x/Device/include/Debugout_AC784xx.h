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
 * @file Debugout_AC784xx.h
 *
 * @brief This file provides debug information output integration functions interfaces.
 *
 */

#ifndef DEBUGOUT_AC784XX_H
#define DEBUGOUT_AC784XX_H

#ifdef __cplusplus
extern "C" {
#endif

/* ===========================================  INCLUDE FILES  =========================================== */
#include "Platform_Types.h"

/* ============================================  DEFINES AND MACROS  ============================================ */

/* ============================================= TYPEDEFS ================================================ */

/* =========================================== LOCAL VARIABLES ============================================== */

/* ====================================  FUNCTION PROTOTYPES  ===================================== */
/*!
 * @brief Set the debug out is invalid.
 * @note  Function ID: [DES_DBG_API_001]
 * @return void
 */
void Debug_DeInit(boolean IsCloseDevice);

/*!
 * @brief Init debug out, and set the debug out is valid.
 * @note  Function ID: [DES_DBG_API_000]
 * @return void
 */
void Debug_Init(void);

/*!
 * @brief Printf log to uart.
 * @note  Function ID: [DES_DBG_API_002]
 * @return void
 */
void Debug_Printf(const char *format, ...);

#if defined(SDK_AC7840E_MINIMAL_PRINT)
/*!
 * @brief Print float value to debug UART (bare-metal, no malloc/vsnprintf).
 * @note  Safe to call from pre-OS / fault-handler context.
 * @param[in] f: value to print
 */
void uart_put_float(float32 f);

/*!
 * @brief Flush debug output buffer.
 * @return void
 */
void Debug_Flush(void);
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DEBUGOUT_AC784XX_H */

/* =============================================  EOF  ============================================== */
