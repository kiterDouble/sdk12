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
/*****************************************************************************/
/**
 * @file     Core_AC784xx.h
 * @brief    AC784XX Cortex-M4 Core Peripheral Access Layer Header File
 */
/*****************************************************************************/
/*
 * Copyright (c) 2009-2020 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CORE_AC784XX_H
#define CORE_AC784XX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"               /* CMSIS compiler specific defines */

/*PRQA S 0602 EOF # AMCE_SYS_0602 */
/*PRQA S 0603 EOF # AMCE_SYS_0603 */
#if   defined (__ICCARM__)
    #pragma system_include         /* treat file as system include file for MISRA check */
#elif defined (__clang__)
    #pragma clang system_header   /* treat file as system include file */
#endif

/** \brief  No-op
 */
#ifndef __NOP
LOCAL_INLINE void __NOP(void)
{
    ASMV_KEYWORD ("nop");    //PRQA S 1006 # AMCE_SYS_1006
}
#endif

/** \brief  dmb
 */
#ifndef __DMB
LOCAL_INLINE void __DMB(void)
{
#if defined (COMPILER_ARM_CC) || defined (COMPILER_IAR) || defined(COMPILER_GHS)
    ASMV_KEYWORD ("dmb");
#else
    ASMV_KEYWORD ("dmb 0xf" : : : "memory");  //PRQA S 1006 # AMCE_SYS_1006
#endif
}
#endif

/** \brief  dsb
 */
#ifndef __DSB
LOCAL_INLINE void __DSB(void)
{
#if defined (COMPILER_ARM_CC) || defined (COMPILER_IAR) || defined(COMPILER_GHS)
    ASMV_KEYWORD ("dsb");
#else
    ASMV_KEYWORD ("dsb 0xf" : : : "memory");  //PRQA S 1006 # AMCE_SYS_1006
#endif
}
#endif

/** \brief  isb
 */
#ifndef __ISB
LOCAL_INLINE void __ISB(void)
{
#if defined (COMPILER_ARM_CC) || defined (COMPILER_IAR) || defined(COMPILER_GHS)
    ASMV_KEYWORD ("isb");
#else
    ASMV_KEYWORD ("isb 0xf" : : : "memory");   //PRQA S 1006 # AMCE_SYS_1006
#endif
}
#endif

/** \brief  Enter low-power standby state
 *    WFI (Wait For Interrupt) makes the processor suspend execution (Clock is stopped) until an IRQ interrupts.
 */
#ifndef __WFI
LOCAL_INLINE void __WFI(void)
{
    ASMV_KEYWORD ("wfi");               //PRQA S 1006 # AMCE_SYS_1006
}
#endif

/* IO definitions (access restrictions to peripheral registers) */
/**
    \defgroup CMSIS_glob_defs CMSIS Global Defines

    <strong>IO Type Qualifiers</strong> are used
    \li to specify the access to peripheral variables.
    \li for automatic generation of peripheral register debug information.
*/
#ifdef __cplusplus
    #define   __I     volatile             /*!< Defines 'read only' permissions */
#else
    #define   __I     volatile const       /*!< Defines 'read only' permissions */
#endif
#define     __O     volatile             /*!< Defines 'write only' permissions */
#define     __IO    volatile             /*!< Defines 'read / write' permissions */

/* following defines should be used for structure members */
#define     __IM     volatile const      /*! Defines 'read only' structure member permissions */
#define     __OM     volatile            /*! Defines 'write only' structure member permissions */
#define     __IOM    volatile            /*! Defines 'read / write' structure member permissions */

/*@} end of group Cortex_M4 */

/*******************************************************************************
 *                 Register Abstraction
  Core Register contain:
  - Core Register
  - Core NVIC Register
  - Core SCB Register
  - Core SysTick Register
  - Core Debug Register
  - Core MPU Register
  - Core FPU Register
 ******************************************************************************/
/**
  \defgroup CMSIS_core_register Defines and Type Definitions
  \brief Type definitions and defines for Cortex-M processor based devices.
*/

/**
  \ingroup    CMSIS_core_register
  \defgroup   CMSIS_CORE  Status and Control Registers
  \brief      Core Register type definitions.
  @{
 */

/**
  \brief  Union type to access the Application Program Status Register (APSR).
 */
typedef union
{
    struct
    {
        uint32 _reserved0: 16;  /*!< bit:  0..15  Reserved */
        uint32 GE: 4;           /*!< bit: 16..19  Greater than or Equal flags */
        uint32 _reserved1: 7;   /*!< bit: 20..26  Reserved */
        uint32 Q: 1;            /*!< bit:     27  Saturation condition flag */
        uint32 V: 1;            /*!< bit:     28  Overflow condition code flag */
        uint32 C: 1;            /*!< bit:     29  Carry condition code flag */
        uint32 Z: 1;            /*!< bit:     30  Zero condition code flag */
        uint32 N: 1;            /*!< bit:     31  Negative condition code flag */
    } b;                        /*!< Structure used for bit  access */
    uint32 w;                   /*!< Type      used for word access */
} APSR_Type;                       //PRQA S 1535 # AMCE_SYS_1535

/* APSR Register Definitions */
#define APSR_N_Pos                         31U                                            /*!< APSR: N Position */
#define APSR_N_Msk                         (1UL << APSR_N_Pos)                            /*!< APSR: N Mask */

#define APSR_Z_Pos                         30U                                            /*!< APSR: Z Position */
#define APSR_Z_Msk                         (1UL << APSR_Z_Pos)                            /*!< APSR: Z Mask */

#define APSR_C_Pos                         29U                                            /*!< APSR: C Position */
#define APSR_C_Msk                         (1UL << APSR_C_Pos)                            /*!< APSR: C Mask */

#define APSR_V_Pos                         28U                                            /*!< APSR: V Position */
#define APSR_V_Msk                         (1UL << APSR_V_Pos)                            /*!< APSR: V Mask */

#define APSR_Q_Pos                         27U                                            /*!< APSR: Q Position */
#define APSR_Q_Msk                         (1UL << APSR_Q_Pos)                            /*!< APSR: Q Mask */

#define APSR_GE_Pos                        16U                                            /*!< APSR: GE Position */
#define APSR_GE_Msk                        (0xFUL << APSR_GE_Pos)                         /*!< APSR: GE Mask */

/**
  \brief  Union type to access the Interrupt Program Status Register (IPSR).
 */
typedef union
{
    struct
    {
        uint32 ISR:9;               /*!< bit:  0.. 8  Exception number */ //PRQA S 0784 # AMCE_SYS_0784
        uint32 _reserved0: 23;      /*!< bit:  9..31  Reserved */
    } b;                            /*!< Structure used for bit  access */
    uint32 w;                       /*!< Type      used for word access */
} IPSR_Type;                         //PRQA S 1535 # AMCE_SYS_1535

/* IPSR Register Definitions */
#define IPSR_ISR_Pos                        0U                                            /*!< IPSR: ISR Position */
#define IPSR_ISR_Msk                       (0x1FFUL)                  /*!< IPSR: ISR Mask */

/**
  \brief  Union type to access the Special-Purpose Program Status Registers (xPSR).
 */
typedef union
{
    struct
    {
        uint32 ISR:9;           /*!< bit:  0.. 8  Exception number */ //PRQA S 0784 # AMCE_SYS_0784
        uint32 _reserved0: 1;   /*!< bit:      9  Reserved */
        uint32 ICI_IT_1: 6;     /*!< bit: 10..15  ICI/IT part 1 */
        uint32 GE: 4;           /*!< bit: 16..19  Greater than or Equal flags */
        uint32 _reserved1: 4;   /*!< bit: 20..23  Reserved */
        uint32 T: 1;            /*!< bit:     24  Thumb bit */
        uint32 ICI_IT_2: 2;     /*!< bit: 25..26  ICI/IT part 2 */
        uint32 Q: 1;            /*!< bit:     27  Saturation condition flag */
        uint32 V: 1;            /*!< bit:     28  Overflow condition code flag */
        uint32 C: 1;            /*!< bit:     29  Carry condition code flag */
        uint32 Z: 1;            /*!< bit:     30  Zero condition code flag */
        uint32 N: 1;            /*!< bit:     31  Negative condition code flag */
    } b;                        /*!< Structure used for bit  access */
    uint32 w;                   /*!< Type      used for word access */
} xPSR_Type;                     //PRQA S 1535 # AMCE_SYS_1535

/* xPSR Register Definitions */
#define xPSR_N_Pos                         31U                                            /*!< xPSR: N Position */
#define xPSR_N_Msk                         (1UL << xPSR_N_Pos)                            /*!< xPSR: N Mask */

#define xPSR_Z_Pos                         30U                                            /*!< xPSR: Z Position */
#define xPSR_Z_Msk                         (1UL << xPSR_Z_Pos)                            /*!< xPSR: Z Mask */

#define xPSR_C_Pos                         29U                                            /*!< xPSR: C Position */
#define xPSR_C_Msk                         (1UL << xPSR_C_Pos)                            /*!< xPSR: C Mask */

#define xPSR_V_Pos                         28U                                            /*!< xPSR: V Position */
#define xPSR_V_Msk                         (1UL << xPSR_V_Pos)                            /*!< xPSR: V Mask */

#define xPSR_Q_Pos                         27U                                            /*!< xPSR: Q Position */
#define xPSR_Q_Msk                         (1UL << xPSR_Q_Pos)                            /*!< xPSR: Q Mask */

#define xPSR_ICI_IT_2_Pos                  25U                                            /*!< xPSR: ICI/IT part 2 Position */
#define xPSR_ICI_IT_2_Msk                  (3UL << xPSR_ICI_IT_2_Pos)                     /*!< xPSR: ICI/IT part 2 Mask */

#define xPSR_T_Pos                         24U                                            /*!< xPSR: T Position */
#define xPSR_T_Msk                         (1UL << xPSR_T_Pos)                            /*!< xPSR: T Mask */

#define xPSR_GE_Pos                        16U                                            /*!< xPSR: GE Position */
#define xPSR_GE_Msk                        (0xFUL << xPSR_GE_Pos)                         /*!< xPSR: GE Mask */

#define xPSR_ICI_IT_1_Pos                  10U                                            /*!< xPSR: ICI/IT part 1 Position */
#define xPSR_ICI_IT_1_Msk                  (0x3FUL << xPSR_ICI_IT_1_Pos)                  /*!< xPSR: ICI/IT part 1 Mask */

#define xPSR_ISR_Pos                        0U                                            /*!< xPSR: ISR Position */
#define xPSR_ISR_Msk                       (0x1FFUL)                  /*!< xPSR: ISR Mask */

/**
  \brief  Union type to access the Control Registers (CONTROL).
 */
typedef union
{
    struct
    {
        uint32 nPRIV: 1;        /*!< bit:      0  Execution privilege in Thread mode */
        uint32 SPSEL: 1;        /*!< bit:      1  Stack to be used */
        uint32 FPCA: 1;         /*!< bit:      2  FP extension active flag */
        uint32 _reserved0: 29;  /*!< bit:  3..31  Reserved */
    } b;                        /*!< Structure used for bit  access */
    uint32 w;                   /*!< Type      used for word access */
} CONTROL_Type;               //PRQA S 1535 # AMCE_SYS_1535

/* CONTROL Register Definitions */
#define CONTROL_FPCA_Pos                    2U                                            /*!< CONTROL: FPCA Position */
#define CONTROL_FPCA_Msk                   (1UL << CONTROL_FPCA_Pos)                      /*!< CONTROL: FPCA Mask */

#define CONTROL_SPSEL_Pos                   1U                                            /*!< CONTROL: SPSEL Position */
#define CONTROL_SPSEL_Msk                  (1UL << CONTROL_SPSEL_Pos)                     /*!< CONTROL: SPSEL Mask */

#define CONTROL_nPRIV_Pos                   0U                                            /*!< CONTROL: nPRIV Position */
#define CONTROL_nPRIV_Msk                  (1UL)                 /*!< CONTROL: nPRIV Mask */

/*@} end of group CMSIS_CORE */

/**
  \ingroup    CMSIS_core_register
  \defgroup   CMSIS_NVIC  Nested Vectored Interrupt Controller (NVIC)
  \brief      Type definitions for the NVIC Registers
  @{
 */

/**
  \brief  Structure type to access the Nested Vectored Interrupt Controller (NVIC).
 */
typedef struct
{
    __IOM uint32 ISER[8U];  /*!< Offset: 0x000 (R/W)  Interrupt Set Enable Register */
    uint32 RESERVED0[24U];
    __IOM uint32 ICER[8U];  /*!< Offset: 0x080 (R/W)  Interrupt Clear Enable Register */
    uint32 RESERVED1[24U];
    __IOM uint32 ISPR[8U];  /*!< Offset: 0x100 (R/W)  Interrupt Set Pending Register */
    uint32 RESERVED2[24U];
    __IOM uint32 ICPR[8U];  /*!< Offset: 0x180 (R/W)  Interrupt Clear Pending Register */
    uint32 RESERVED3[24U];
    __IOM uint32 IABR[8U];  /*!< Offset: 0x200 (R/W)  Interrupt Active bit Register */
    uint32 RESERVED4[56U];
    __IOM uint8 IP[240U];   /*!< Offset: 0x300 (R/W)  Interrupt Priority Register (8Bit wide) */
    uint32 RESERVED5[644U];
    __OM uint32 STIR;       /*!< Offset: 0xE00 ( /W)  Software Trigger Interrupt Register */
} NVIC_Type;

/* Software Triggered Interrupt Register Definitions */
#define NVIC_STIR_INTID_Pos                 0U                                         /*!< STIR: INTLINESNUM Position */
#define NVIC_STIR_INTID_Msk                (0x1FFUL)        /*!< STIR: INTLINESNUM Mask */

/*@} end of group CMSIS_NVIC */

/**
  \ingroup  CMSIS_core_register
  \defgroup CMSIS_SCB     System Control Block (SCB)
  \brief    Type definitions for the System Control Block Registers
  @{
 */

/**
  \brief  Structure type to access the System Control Block (SCB).
 */
typedef struct
{
    __IM uint32 CPUID;      /*!< Offset: 0x000 (R/ )  CPUID Base Register */
    __IOM uint32 ICSR;      /*!< Offset: 0x004 (R/W)  Interrupt Control and State Register */
    __IOM uint32 VTOR;      /*!< Offset: 0x008 (R/W)  Vector Table Offset Register */
    __IOM uint32 AIRCR;     /*!< Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register */
    __IOM uint32 SCR;       /*!< Offset: 0x010 (R/W)  System Control Register */
    __IOM uint32 CCR;       /*!< Offset: 0x014 (R/W)  Configuration Control Register */
    __IOM uint8 SHP[12U];   /*!< Offset: 0x018 (R/W)  System Handlers Priority Registers (4-7, 8-11, 12-15) */
    __IOM uint32 SHCSR;     /*!< Offset: 0x024 (R/W)  System Handler Control and State Register */
    __IOM uint32 CFSR;      /*!< Offset: 0x028 (R/W)  Configurable Fault Status Register */
    __IOM uint32 HFSR;      /*!< Offset: 0x02C (R/W)  HardFault Status Register */
    __IOM uint32 DFSR;      /*!< Offset: 0x030 (R/W)  Debug Fault Status Register */
    __IOM uint32 MMFAR;     /*!< Offset: 0x034 (R/W)  MemManage Fault Address Register */
    __IOM uint32 BFAR;      /*!< Offset: 0x038 (R/W)  BusFault Address Register */
    __IOM uint32 AFSR;      /*!< Offset: 0x03C (R/W)  Auxiliary Fault Status Register */
    __IM uint32 PFR[2U];    /*!< Offset: 0x040 (R/ )  Processor Feature Register */
    __IM uint32 DFR;        /*!< Offset: 0x048 (R/ )  Debug Feature Register */
    __IM uint32 ADR;        /*!< Offset: 0x04C (R/ )  Auxiliary Feature Register */
    __IM uint32 MMFR[4U];   /*!< Offset: 0x050 (R/ )  Memory Model Feature Register */
    __IM uint32 ISAR[5U];   /*!< Offset: 0x060 (R/ )  Instruction Set Attributes Register */
    uint32 RESERVED0[5U];
    __IOM uint32 CPACR;     /*!< Offset: 0x088 (R/W)  Coprocessor Access Control Register */
} SCB_Type;

/* SCB CPUID Register Definitions */
#define SCB_CPUID_IMPLEMENTER_Pos          24U                                            /*!< SCB CPUID: IMPLEMENTER Position */
#define SCB_CPUID_IMPLEMENTER_Msk          (0xFFUL << SCB_CPUID_IMPLEMENTER_Pos)          /*!< SCB CPUID: IMPLEMENTER Mask */

#define SCB_CPUID_VARIANT_Pos              20U                                            /*!< SCB CPUID: VARIANT Position */
#define SCB_CPUID_VARIANT_Msk              (0xFUL << SCB_CPUID_VARIANT_Pos)               /*!< SCB CPUID: VARIANT Mask */

#define SCB_CPUID_ARCHITECTURE_Pos         16U                                            /*!< SCB CPUID: ARCHITECTURE Position */
#define SCB_CPUID_ARCHITECTURE_Msk         (0xFUL << SCB_CPUID_ARCHITECTURE_Pos)          /*!< SCB CPUID: ARCHITECTURE Mask */

#define SCB_CPUID_PARTNO_Pos                4U                                            /*!< SCB CPUID: PARTNO Position */
#define SCB_CPUID_PARTNO_Msk               (0xFFFUL << SCB_CPUID_PARTNO_Pos)              /*!< SCB CPUID: PARTNO Mask */

#define SCB_CPUID_REVISION_Pos              0U                                            /*!< SCB CPUID: REVISION Position */
#define SCB_CPUID_REVISION_Msk             (0xFUL)          /*!< SCB CPUID: REVISION Mask */

/* SCB Interrupt Control State Register Definitions */
#define SCB_ICSR_NMIPENDSET_Pos            31U                                            /*!< SCB ICSR: NMIPENDSET Position */
#define SCB_ICSR_NMIPENDSET_Msk            (1UL << SCB_ICSR_NMIPENDSET_Pos)               /*!< SCB ICSR: NMIPENDSET Mask */

#define SCB_ICSR_PENDSVSET_Pos             28U                                            /*!< SCB ICSR: PENDSVSET Position */
#define SCB_ICSR_PENDSVSET_Msk             (1UL << SCB_ICSR_PENDSVSET_Pos)                /*!< SCB ICSR: PENDSVSET Mask */

#define SCB_ICSR_PENDSVCLR_Pos             27U                                            /*!< SCB ICSR: PENDSVCLR Position */
#define SCB_ICSR_PENDSVCLR_Msk             (1UL << SCB_ICSR_PENDSVCLR_Pos)                /*!< SCB ICSR: PENDSVCLR Mask */

#define SCB_ICSR_PENDSTSET_Pos             26U                                            /*!< SCB ICSR: PENDSTSET Position */
#define SCB_ICSR_PENDSTSET_Msk             (1UL << SCB_ICSR_PENDSTSET_Pos)                /*!< SCB ICSR: PENDSTSET Mask */

#define SCB_ICSR_PENDSTCLR_Pos             25U                                            /*!< SCB ICSR: PENDSTCLR Position */
#define SCB_ICSR_PENDSTCLR_Msk             (1UL << SCB_ICSR_PENDSTCLR_Pos)                /*!< SCB ICSR: PENDSTCLR Mask */

#define SCB_ICSR_ISRPREEMPT_Pos            23U                                            /*!< SCB ICSR: ISRPREEMPT Position */
#define SCB_ICSR_ISRPREEMPT_Msk            (1UL << SCB_ICSR_ISRPREEMPT_Pos)               /*!< SCB ICSR: ISRPREEMPT Mask */

#define SCB_ICSR_ISRPENDING_Pos            22U                                            /*!< SCB ICSR: ISRPENDING Position */
#define SCB_ICSR_ISRPENDING_Msk            (1UL << SCB_ICSR_ISRPENDING_Pos)               /*!< SCB ICSR: ISRPENDING Mask */

#define SCB_ICSR_VECTPENDING_Pos           12U                                            /*!< SCB ICSR: VECTPENDING Position */
#define SCB_ICSR_VECTPENDING_Msk           (0x1FFUL << SCB_ICSR_VECTPENDING_Pos)          /*!< SCB ICSR: VECTPENDING Mask */

#define SCB_ICSR_RETTOBASE_Pos             11U                                            /*!< SCB ICSR: RETTOBASE Position */
#define SCB_ICSR_RETTOBASE_Msk             (1UL << SCB_ICSR_RETTOBASE_Pos)                /*!< SCB ICSR: RETTOBASE Mask */

#define SCB_ICSR_VECTACTIVE_Pos             0U                                            /*!< SCB ICSR: VECTACTIVE Position */
#define SCB_ICSR_VECTACTIVE_Msk            (0x1FFUL)       /*!< SCB ICSR: VECTACTIVE Mask */

/* SCB Vector Table Offset Register Definitions */
#define SCB_VTOR_TBLOFF_Pos                 7U                                            /*!< SCB VTOR: TBLOFF Position */
#define SCB_VTOR_TBLOFF_Msk                (0x1FFFFFFUL << SCB_VTOR_TBLOFF_Pos)           /*!< SCB VTOR: TBLOFF Mask */

/* SCB Application Interrupt and Reset Control Register Definitions */
#define SCB_AIRCR_VECTKEY_Pos              16U                                            /*!< SCB AIRCR: VECTKEY Position */
#define SCB_AIRCR_VECTKEY_Msk              (0xFFFFUL << SCB_AIRCR_VECTKEY_Pos)            /*!< SCB AIRCR: VECTKEY Mask */

#define SCB_AIRCR_VECTKEYSTAT_Pos          16U                                            /*!< SCB AIRCR: VECTKEYSTAT Position */
#define SCB_AIRCR_VECTKEYSTAT_Msk          (0xFFFFUL << SCB_AIRCR_VECTKEYSTAT_Pos)        /*!< SCB AIRCR: VECTKEYSTAT Mask */

#define SCB_AIRCR_ENDIANESS_Pos            15U                                            /*!< SCB AIRCR: ENDIANESS Position */
#define SCB_AIRCR_ENDIANESS_Msk            (1UL << SCB_AIRCR_ENDIANESS_Pos)               /*!< SCB AIRCR: ENDIANESS Mask */

#define SCB_AIRCR_PRIGROUP_Pos              8U                                            /*!< SCB AIRCR: PRIGROUP Position */
#define SCB_AIRCR_PRIGROUP_Msk             (7UL << SCB_AIRCR_PRIGROUP_Pos)                /*!< SCB AIRCR: PRIGROUP Mask */

#define SCB_AIRCR_SYSRESETREQ_Pos           2U                                            /*!< SCB AIRCR: SYSRESETREQ Position */
#define SCB_AIRCR_SYSRESETREQ_Msk          (1UL << SCB_AIRCR_SYSRESETREQ_Pos)             /*!< SCB AIRCR: SYSRESETREQ Mask */

#define SCB_AIRCR_VECTCLRACTIVE_Pos         1U                                            /*!< SCB AIRCR: VECTCLRACTIVE Position */
#define SCB_AIRCR_VECTCLRACTIVE_Msk        (1UL << SCB_AIRCR_VECTCLRACTIVE_Pos)           /*!< SCB AIRCR: VECTCLRACTIVE Mask */

#define SCB_AIRCR_VECTRESET_Pos             0U                                            /*!< SCB AIRCR: VECTRESET Position */
#define SCB_AIRCR_VECTRESET_Msk            (1UL)           /*!< SCB AIRCR: VECTRESET Mask */

/* SCB System Control Register Definitions */
#define SCB_SCR_SEVONPEND_Pos               4U                                            /*!< SCB SCR: SEVONPEND Position */
#define SCB_SCR_SEVONPEND_Msk              (1UL << SCB_SCR_SEVONPEND_Pos)                 /*!< SCB SCR: SEVONPEND Mask */

#define SCB_SCR_SLEEPDEEP_Pos               2U                                            /*!< SCB SCR: SLEEPDEEP Position */
#define SCB_SCR_SLEEPDEEP_Msk              (1UL << SCB_SCR_SLEEPDEEP_Pos)                 /*!< SCB SCR: SLEEPDEEP Mask */

#define SCB_SCR_SLEEPONEXIT_Pos             1U                                            /*!< SCB SCR: SLEEPONEXIT Position */
#define SCB_SCR_SLEEPONEXIT_Msk            (1UL << SCB_SCR_SLEEPONEXIT_Pos)               /*!< SCB SCR: SLEEPONEXIT Mask */

/* SCB Configuration Control Register Definitions */
#define SCB_CCR_STKALIGN_Pos                9U                                            /*!< SCB CCR: STKALIGN Position */
#define SCB_CCR_STKALIGN_Msk               (1UL << SCB_CCR_STKALIGN_Pos)                  /*!< SCB CCR: STKALIGN Mask */

#define SCB_CCR_BFHFNMIGN_Pos               8U                                            /*!< SCB CCR: BFHFNMIGN Position */
#define SCB_CCR_BFHFNMIGN_Msk              (1UL << SCB_CCR_BFHFNMIGN_Pos)                 /*!< SCB CCR: BFHFNMIGN Mask */

#define SCB_CCR_DIV_0_TRP_Pos               4U                                            /*!< SCB CCR: DIV_0_TRP Position */
#define SCB_CCR_DIV_0_TRP_Msk              (1UL << SCB_CCR_DIV_0_TRP_Pos)                 /*!< SCB CCR: DIV_0_TRP Mask */

#define SCB_CCR_UNALIGN_TRP_Pos             3U                                            /*!< SCB CCR: UNALIGN_TRP Position */
#define SCB_CCR_UNALIGN_TRP_Msk            (1UL << SCB_CCR_UNALIGN_TRP_Pos)               /*!< SCB CCR: UNALIGN_TRP Mask */

#define SCB_CCR_USERSETMPEND_Pos            1U                                            /*!< SCB CCR: USERSETMPEND Position */
#define SCB_CCR_USERSETMPEND_Msk           (1UL << SCB_CCR_USERSETMPEND_Pos)              /*!< SCB CCR: USERSETMPEND Mask */

#define SCB_CCR_NONBASETHRDENA_Pos          0U                                            /*!< SCB CCR: NONBASETHRDENA Position */
#define SCB_CCR_NONBASETHRDENA_Msk         (1UL)        /*!< SCB CCR: NONBASETHRDENA Mask */

/* SCB System Handler Control and State Register Definitions */
#define SCB_SHCSR_USGFAULTENA_Pos          18U                                            /*!< SCB SHCSR: USGFAULTENA Position */
#define SCB_SHCSR_USGFAULTENA_Msk          (1UL << SCB_SHCSR_USGFAULTENA_Pos)             /*!< SCB SHCSR: USGFAULTENA Mask */

#define SCB_SHCSR_BUSFAULTENA_Pos          17U                                            /*!< SCB SHCSR: BUSFAULTENA Position */
#define SCB_SHCSR_BUSFAULTENA_Msk          (1UL << SCB_SHCSR_BUSFAULTENA_Pos)             /*!< SCB SHCSR: BUSFAULTENA Mask */

#define SCB_SHCSR_MEMFAULTENA_Pos          16U                                            /*!< SCB SHCSR: MEMFAULTENA Position */
#define SCB_SHCSR_MEMFAULTENA_Msk          (1UL << SCB_SHCSR_MEMFAULTENA_Pos)             /*!< SCB SHCSR: MEMFAULTENA Mask */

#define SCB_SHCSR_SVCALLPENDED_Pos         15U                                            /*!< SCB SHCSR: SVCALLPENDED Position */
#define SCB_SHCSR_SVCALLPENDED_Msk         (1UL << SCB_SHCSR_SVCALLPENDED_Pos)            /*!< SCB SHCSR: SVCALLPENDED Mask */

#define SCB_SHCSR_BUSFAULTPENDED_Pos       14U                                            /*!< SCB SHCSR: BUSFAULTPENDED Position */
#define SCB_SHCSR_BUSFAULTPENDED_Msk       (1UL << SCB_SHCSR_BUSFAULTPENDED_Pos)          /*!< SCB SHCSR: BUSFAULTPENDED Mask */

#define SCB_SHCSR_MEMFAULTPENDED_Pos       13U                                            /*!< SCB SHCSR: MEMFAULTPENDED Position */
#define SCB_SHCSR_MEMFAULTPENDED_Msk       (1UL << SCB_SHCSR_MEMFAULTPENDED_Pos)          /*!< SCB SHCSR: MEMFAULTPENDED Mask */

#define SCB_SHCSR_USGFAULTPENDED_Pos       12U                                            /*!< SCB SHCSR: USGFAULTPENDED Position */
#define SCB_SHCSR_USGFAULTPENDED_Msk       (1UL << SCB_SHCSR_USGFAULTPENDED_Pos)          /*!< SCB SHCSR: USGFAULTPENDED Mask */

#define SCB_SHCSR_SYSTICKACT_Pos           11U                                            /*!< SCB SHCSR: SYSTICKACT Position */
#define SCB_SHCSR_SYSTICKACT_Msk           (1UL << SCB_SHCSR_SYSTICKACT_Pos)              /*!< SCB SHCSR: SYSTICKACT Mask */

#define SCB_SHCSR_PENDSVACT_Pos            10U                                            /*!< SCB SHCSR: PENDSVACT Position */
#define SCB_SHCSR_PENDSVACT_Msk            (1UL << SCB_SHCSR_PENDSVACT_Pos)               /*!< SCB SHCSR: PENDSVACT Mask */

#define SCB_SHCSR_MONITORACT_Pos            8U                                            /*!< SCB SHCSR: MONITORACT Position */
#define SCB_SHCSR_MONITORACT_Msk           (1UL << SCB_SHCSR_MONITORACT_Pos)              /*!< SCB SHCSR: MONITORACT Mask */

#define SCB_SHCSR_SVCALLACT_Pos             7U                                            /*!< SCB SHCSR: SVCALLACT Position */
#define SCB_SHCSR_SVCALLACT_Msk            (1UL << SCB_SHCSR_SVCALLACT_Pos)               /*!< SCB SHCSR: SVCALLACT Mask */

#define SCB_SHCSR_USGFAULTACT_Pos           3U                                            /*!< SCB SHCSR: USGFAULTACT Position */
#define SCB_SHCSR_USGFAULTACT_Msk          (1UL << SCB_SHCSR_USGFAULTACT_Pos)             /*!< SCB SHCSR: USGFAULTACT Mask */

#define SCB_SHCSR_BUSFAULTACT_Pos           1U                                            /*!< SCB SHCSR: BUSFAULTACT Position */
#define SCB_SHCSR_BUSFAULTACT_Msk          (1UL << SCB_SHCSR_BUSFAULTACT_Pos)             /*!< SCB SHCSR: BUSFAULTACT Mask */

#define SCB_SHCSR_MEMFAULTACT_Pos           0U                                            /*!< SCB SHCSR: MEMFAULTACT Position */
#define SCB_SHCSR_MEMFAULTACT_Msk          (1UL)         /*!< SCB SHCSR: MEMFAULTACT Mask */

/* SCB Configurable Fault Status Register Definitions */
#define SCB_CFSR_USGFAULTSR_Pos            16U                                            /*!< SCB CFSR: Usage Fault Status Register Position */
#define SCB_CFSR_USGFAULTSR_Msk            (0xFFFFUL << SCB_CFSR_USGFAULTSR_Pos)          /*!< SCB CFSR: Usage Fault Status Register Mask */

#define SCB_CFSR_BUSFAULTSR_Pos             8U                                            /*!< SCB CFSR: Bus Fault Status Register Position */
#define SCB_CFSR_BUSFAULTSR_Msk            (0xFFUL << SCB_CFSR_BUSFAULTSR_Pos)            /*!< SCB CFSR: Bus Fault Status Register Mask */

#define SCB_CFSR_MEMFAULTSR_Pos             0U                                            /*!< SCB CFSR: Memory Manage Fault Status Register Position */
#define SCB_CFSR_MEMFAULTSR_Msk            (0xFFUL)        /*!< SCB CFSR: Memory Manage Fault Status Register Mask */

/* MemManage Fault Status Register (part of SCB Configurable Fault Status Register) */
#define SCB_CFSR_MMARVALID_Pos             (SCB_CFSR_MEMFAULTSR_Pos + 7U)                 /*!< SCB CFSR (MMFSR): MMARVALID Position */
#define SCB_CFSR_MMARVALID_Msk             (1UL << SCB_CFSR_MMARVALID_Pos)                /*!< SCB CFSR (MMFSR): MMARVALID Mask */

#define SCB_CFSR_MLSPERR_Pos               (SCB_CFSR_MEMFAULTSR_Pos + 5U)                 /*!< SCB CFSR (MMFSR): MLSPERR Position */
#define SCB_CFSR_MLSPERR_Msk               (1UL << SCB_CFSR_MLSPERR_Pos)                  /*!< SCB CFSR (MMFSR): MLSPERR Mask */

#define SCB_CFSR_MSTKERR_Pos               (SCB_CFSR_MEMFAULTSR_Pos + 4U)                 /*!< SCB CFSR (MMFSR): MSTKERR Position */
#define SCB_CFSR_MSTKERR_Msk               (1UL << SCB_CFSR_MSTKERR_Pos)                  /*!< SCB CFSR (MMFSR): MSTKERR Mask */

#define SCB_CFSR_MUNSTKERR_Pos             (SCB_CFSR_MEMFAULTSR_Pos + 3U)                 /*!< SCB CFSR (MMFSR): MUNSTKERR Position */
#define SCB_CFSR_MUNSTKERR_Msk             (1UL << SCB_CFSR_MUNSTKERR_Pos)                /*!< SCB CFSR (MMFSR): MUNSTKERR Mask */

#define SCB_CFSR_DACCVIOL_Pos              (SCB_CFSR_MEMFAULTSR_Pos + 1U)                 /*!< SCB CFSR (MMFSR): DACCVIOL Position */
#define SCB_CFSR_DACCVIOL_Msk              (1UL << SCB_CFSR_DACCVIOL_Pos)                 /*!< SCB CFSR (MMFSR): DACCVIOL Mask */

#define SCB_CFSR_IACCVIOL_Pos              (SCB_CFSR_MEMFAULTSR_Pos + 0U)                 /*!< SCB CFSR (MMFSR): IACCVIOL Position */
#define SCB_CFSR_IACCVIOL_Msk              (1UL)             /*!< SCB CFSR (MMFSR): IACCVIOL Mask */

/* BusFault Status Register (part of SCB Configurable Fault Status Register) */
#define SCB_CFSR_BFARVALID_Pos            (SCB_CFSR_BUSFAULTSR_Pos + 7U)                  /*!< SCB CFSR (BFSR): BFARVALID Position */
#define SCB_CFSR_BFARVALID_Msk            (1UL << SCB_CFSR_BFARVALID_Pos)                 /*!< SCB CFSR (BFSR): BFARVALID Mask */

#define SCB_CFSR_LSPERR_Pos               (SCB_CFSR_BUSFAULTSR_Pos + 5U)                  /*!< SCB CFSR (BFSR): LSPERR Position */
#define SCB_CFSR_LSPERR_Msk               (1UL << SCB_CFSR_LSPERR_Pos)                    /*!< SCB CFSR (BFSR): LSPERR Mask */

#define SCB_CFSR_STKERR_Pos               (SCB_CFSR_BUSFAULTSR_Pos + 4U)                  /*!< SCB CFSR (BFSR): STKERR Position */
#define SCB_CFSR_STKERR_Msk               (1UL << SCB_CFSR_STKERR_Pos)                    /*!< SCB CFSR (BFSR): STKERR Mask */

#define SCB_CFSR_UNSTKERR_Pos             (SCB_CFSR_BUSFAULTSR_Pos + 3U)                  /*!< SCB CFSR (BFSR): UNSTKERR Position */
#define SCB_CFSR_UNSTKERR_Msk             (1UL << SCB_CFSR_UNSTKERR_Pos)                  /*!< SCB CFSR (BFSR): UNSTKERR Mask */

#define SCB_CFSR_IMPRECISERR_Pos          (SCB_CFSR_BUSFAULTSR_Pos + 2U)                  /*!< SCB CFSR (BFSR): IMPRECISERR Position */
#define SCB_CFSR_IMPRECISERR_Msk          (1UL << SCB_CFSR_IMPRECISERR_Pos)               /*!< SCB CFSR (BFSR): IMPRECISERR Mask */

#define SCB_CFSR_PRECISERR_Pos            (SCB_CFSR_BUSFAULTSR_Pos + 1U)                  /*!< SCB CFSR (BFSR): PRECISERR Position */
#define SCB_CFSR_PRECISERR_Msk            (1UL << SCB_CFSR_PRECISERR_Pos)                 /*!< SCB CFSR (BFSR): PRECISERR Mask */

#define SCB_CFSR_IBUSERR_Pos              (SCB_CFSR_BUSFAULTSR_Pos + 0U)                  /*!< SCB CFSR (BFSR): IBUSERR Position */
#define SCB_CFSR_IBUSERR_Msk              (1UL << SCB_CFSR_IBUSERR_Pos)                   /*!< SCB CFSR (BFSR): IBUSERR Mask */

/* UsageFault Status Register (part of SCB Configurable Fault Status Register) */
#define SCB_CFSR_DIVBYZERO_Pos            (SCB_CFSR_USGFAULTSR_Pos + 9U)                  /*!< SCB CFSR (UFSR): DIVBYZERO Position */
#define SCB_CFSR_DIVBYZERO_Msk            (1UL << SCB_CFSR_DIVBYZERO_Pos)                 /*!< SCB CFSR (UFSR): DIVBYZERO Mask */

#define SCB_CFSR_UNALIGNED_Pos            (SCB_CFSR_USGFAULTSR_Pos + 8U)                  /*!< SCB CFSR (UFSR): UNALIGNED Position */
#define SCB_CFSR_UNALIGNED_Msk            (1UL << SCB_CFSR_UNALIGNED_Pos)                 /*!< SCB CFSR (UFSR): UNALIGNED Mask */

#define SCB_CFSR_NOCP_Pos                 (SCB_CFSR_USGFAULTSR_Pos + 3U)                  /*!< SCB CFSR (UFSR): NOCP Position */
#define SCB_CFSR_NOCP_Msk                 (1UL << SCB_CFSR_NOCP_Pos)                      /*!< SCB CFSR (UFSR): NOCP Mask */

#define SCB_CFSR_INVPC_Pos                (SCB_CFSR_USGFAULTSR_Pos + 2U)                  /*!< SCB CFSR (UFSR): INVPC Position */
#define SCB_CFSR_INVPC_Msk                (1UL << SCB_CFSR_INVPC_Pos)                     /*!< SCB CFSR (UFSR): INVPC Mask */

#define SCB_CFSR_INVSTATE_Pos             (SCB_CFSR_USGFAULTSR_Pos + 1U)                  /*!< SCB CFSR (UFSR): INVSTATE Position */
#define SCB_CFSR_INVSTATE_Msk             (1UL << SCB_CFSR_INVSTATE_Pos)                  /*!< SCB CFSR (UFSR): INVSTATE Mask */

#define SCB_CFSR_UNDEFINSTR_Pos           (SCB_CFSR_USGFAULTSR_Pos + 0U)                  /*!< SCB CFSR (UFSR): UNDEFINSTR Position */
#define SCB_CFSR_UNDEFINSTR_Msk           (1UL << SCB_CFSR_UNDEFINSTR_Pos)                /*!< SCB CFSR (UFSR): UNDEFINSTR Mask */

/* SCB Hard Fault Status Register Definitions */
#define SCB_HFSR_DEBUGEVT_Pos              31U                                            /*!< SCB HFSR: DEBUGEVT Position */
#define SCB_HFSR_DEBUGEVT_Msk              (1UL << SCB_HFSR_DEBUGEVT_Pos)                 /*!< SCB HFSR: DEBUGEVT Mask */

#define SCB_HFSR_FORCED_Pos                30U                                            /*!< SCB HFSR: FORCED Position */
#define SCB_HFSR_FORCED_Msk                (1UL << SCB_HFSR_FORCED_Pos)                   /*!< SCB HFSR: FORCED Mask */

#define SCB_HFSR_VECTTBL_Pos                1U                                            /*!< SCB HFSR: VECTTBL Position */
#define SCB_HFSR_VECTTBL_Msk               (1UL << SCB_HFSR_VECTTBL_Pos)                  /*!< SCB HFSR: VECTTBL Mask */

/* SCB Debug Fault Status Register Definitions */
#define SCB_DFSR_EXTERNAL_Pos               4U                                            /*!< SCB DFSR: EXTERNAL Position */
#define SCB_DFSR_EXTERNAL_Msk              (1UL << SCB_DFSR_EXTERNAL_Pos)                 /*!< SCB DFSR: EXTERNAL Mask */

#define SCB_DFSR_VCATCH_Pos                 3U                                            /*!< SCB DFSR: VCATCH Position */
#define SCB_DFSR_VCATCH_Msk                (1UL << SCB_DFSR_VCATCH_Pos)                   /*!< SCB DFSR: VCATCH Mask */

#define SCB_DFSR_DWTTRAP_Pos                2U                                            /*!< SCB DFSR: DWTTRAP Position */
#define SCB_DFSR_DWTTRAP_Msk               (1UL << SCB_DFSR_DWTTRAP_Pos)                  /*!< SCB DFSR: DWTTRAP Mask */

#define SCB_DFSR_BKPT_Pos                   1U                                            /*!< SCB DFSR: BKPT Position */
#define SCB_DFSR_BKPT_Msk                  (1UL << SCB_DFSR_BKPT_Pos)                     /*!< SCB DFSR: BKPT Mask */

#define SCB_DFSR_HALTED_Pos                 0U                                            /*!< SCB DFSR: HALTED Position */
#define SCB_DFSR_HALTED_Msk                (1UL)                                         /*!< SCB DFSR: HALTED Mask */

/*@} end of group CMSIS_SCB */

/**
  \ingroup  CMSIS_core_register
  \defgroup CMSIS_SCnSCB System Controls not in SCB (SCnSCB)
  \brief    Type definitions for the System Control and ID Register not in the SCB
  @{
 */

/**
  \brief  Structure type to access the System Control and ID Register not in the SCB.
 */
typedef struct
{
    uint32 RESERVED0[1U];
    __IM uint32 ICTR;   /*!< Offset: 0x004 (R/ )  Interrupt Controller Type Register */
    __IOM uint32 ACTLR; /*!< Offset: 0x008 (R/W)  Auxiliary Control Register */
} SCnSCB_Type;                 //PRQA S 1535 # AMCE_SYS_1535

/* Interrupt Controller Type Register Definitions */
#define SCnSCB_ICTR_INTLINESNUM_Pos         0U                                         /*!< ICTR: INTLINESNUM Position */
#define SCnSCB_ICTR_INTLINESNUM_Msk        (0xFUL)  /*!< ICTR: INTLINESNUM Mask */

/* Auxiliary Control Register Definitions */
#define SCnSCB_ACTLR_DISOOFP_Pos            9U                                         /*!< ACTLR: DISOOFP Position */
#define SCnSCB_ACTLR_DISOOFP_Msk           (1UL << SCnSCB_ACTLR_DISOOFP_Pos)           /*!< ACTLR: DISOOFP Mask */

#define SCnSCB_ACTLR_DISFPCA_Pos            8U                                         /*!< ACTLR: DISFPCA Position */
#define SCnSCB_ACTLR_DISFPCA_Msk           (1UL << SCnSCB_ACTLR_DISFPCA_Pos)           /*!< ACTLR: DISFPCA Mask */

#define SCnSCB_ACTLR_DISFOLD_Pos            2U                                         /*!< ACTLR: DISFOLD Position */
#define SCnSCB_ACTLR_DISFOLD_Msk           (1UL << SCnSCB_ACTLR_DISFOLD_Pos)           /*!< ACTLR: DISFOLD Mask */

#define SCnSCB_ACTLR_DISDEFWBUF_Pos         1U                                         /*!< ACTLR: DISDEFWBUF Position */
#define SCnSCB_ACTLR_DISDEFWBUF_Msk        (1UL << SCnSCB_ACTLR_DISDEFWBUF_Pos)        /*!< ACTLR: DISDEFWBUF Mask */

#define SCnSCB_ACTLR_DISMCYCINT_Pos         0U                                         /*!< ACTLR: DISMCYCINT Position */
#define SCnSCB_ACTLR_DISMCYCINT_Msk        (1UL)    /*!< ACTLR: DISMCYCINT Mask */

/*@} end of group CMSIS_SCnotSCB */

/**
  \ingroup  CMSIS_core_register
  \defgroup CMSIS_SysTick     System Tick Timer (SysTick)
  \brief    Type definitions for the System Timer Registers.
  @{
 */

/**
  \brief  Structure type to access the System Timer (SysTick).
 */
typedef struct
{
    __IOM uint32 CTRL; /*!< Offset: 0x000 (R/W)  SysTick Control and Status Register */
    __IOM uint32 LOAD; /*!< Offset: 0x004 (R/W)  SysTick Reload Value Register */
    __IOM uint32 VAL;  /*!< Offset: 0x008 (R/W)  SysTick Current Value Register */
    __IM uint32 CALIB; /*!< Offset: 0x00C (R/ )  SysTick Calibration Register */
} SysTick_Type;

/* SysTick Control / Status Register Definitions */
#define SysTick_CTRL_COUNTFLAG_Pos         16U                                            /*!< SysTick CTRL: COUNTFLAG Position */
#define SysTick_CTRL_COUNTFLAG_Msk         (1UL << SysTick_CTRL_COUNTFLAG_Pos)            /*!< SysTick CTRL: COUNTFLAG Mask */

#define SysTick_CTRL_CLKSOURCE_Pos          2U                                            /*!< SysTick CTRL: CLKSOURCE Position */
#define SysTick_CTRL_CLKSOURCE_Msk         (1UL << SysTick_CTRL_CLKSOURCE_Pos)            /*!< SysTick CTRL: CLKSOURCE Mask */

#define SysTick_CTRL_TICKINT_Pos            1U                                            /*!< SysTick CTRL: TICKINT Position */
#define SysTick_CTRL_TICKINT_Msk           (1UL << SysTick_CTRL_TICKINT_Pos)              /*!< SysTick CTRL: TICKINT Mask */

#define SysTick_CTRL_ENABLE_Pos             0U                                            /*!< SysTick CTRL: ENABLE Position */
#define SysTick_CTRL_ENABLE_Msk            (1UL)           /*!< SysTick CTRL: ENABLE Mask */

/* SysTick Reload Register Definitions */
#define SysTick_LOAD_RELOAD_Pos             0U                                            /*!< SysTick LOAD: RELOAD Position */
#define SysTick_LOAD_RELOAD_Msk            (0xFFFFFFUL)    /*!< SysTick LOAD: RELOAD Mask */

/* SysTick Current Register Definitions */
#define SysTick_VAL_CURRENT_Pos             0U                                            /*!< SysTick VAL: CURRENT Position */
#define SysTick_VAL_CURRENT_Msk            (0xFFFFFFUL)    /*!< SysTick VAL: CURRENT Mask */

/* SysTick Calibration Register Definitions */
#define SysTick_CALIB_NOREF_Pos            31U                                            /*!< SysTick CALIB: NOREF Position */
#define SysTick_CALIB_NOREF_Msk            (1UL << SysTick_CALIB_NOREF_Pos)               /*!< SysTick CALIB: NOREF Mask */

#define SysTick_CALIB_SKEW_Pos             30U                                            /*!< SysTick CALIB: SKEW Position */
#define SysTick_CALIB_SKEW_Msk             (1UL << SysTick_CALIB_SKEW_Pos)                /*!< SysTick CALIB: SKEW Mask */

#define SysTick_CALIB_TENMS_Pos             0U                                            /*!< SysTick CALIB: TENMS Position */
#define SysTick_CALIB_TENMS_Msk            (0xFFFFFFUL)    /*!< SysTick CALIB: TENMS Mask */

/*@} end of group CMSIS_SysTick */

/**
  \ingroup  CMSIS_core_register
  \defgroup CMSIS_ITM     Instrumentation Trace Macrocell (ITM)
  \brief    Type definitions for the Instrumentation Trace Macrocell (ITM)
  @{
 */

/**
  \brief  Structure type to access the Instrumentation Trace Macrocell Register (ITM).
 */
typedef struct
{
    __OM union
    {
        __OM uint8 u8;          /*!< Offset: 0x000 ( /W)  ITM Stimulus Port 8-bit */
        __OM uint16 u16;        /*!< Offset: 0x000 ( /W)  ITM Stimulus Port 16-bit */
        __OM uint32 u32;        /*!< Offset: 0x000 ( /W)  ITM Stimulus Port 32-bit */
    } PORT[32U];                /*!< Offset: 0x000 ( /W)  ITM Stimulus Port Registers */
    uint32 RESERVED0[864U];
    __IOM uint32 TER;           /*!< Offset: 0xE00 (R/W)  ITM Trace Enable Register */
    uint32 RESERVED1[15U];
    __IOM uint32 TPR;           /*!< Offset: 0xE40 (R/W)  ITM Trace Privilege Register */
    uint32 RESERVED2[15U];
    __IOM uint32 TCR;           /*!< Offset: 0xE80 (R/W)  ITM Trace Control Register */
    uint32 RESERVED3[32U];
    uint32 RESERVED4[43U];
    __OM uint32 LAR;            /*!< Offset: 0xFB0 ( /W)  ITM Lock Access Register */
    __IM uint32 LSR;            /*!< Offset: 0xFB4 (R/ )  ITM Lock Status Register */
    uint32 RESERVED5[6U];
    __IM uint32 PID4;           /*!< Offset: 0xFD0 (R/ )  ITM Peripheral Identification Register #4 */
    __IM uint32 PID5;           /*!< Offset: 0xFD4 (R/ )  ITM Peripheral Identification Register #5 */
    __IM uint32 PID6;           /*!< Offset: 0xFD8 (R/ )  ITM Peripheral Identification Register #6 */
    __IM uint32 PID7;           /*!< Offset: 0xFDC (R/ )  ITM Peripheral Identification Register #7 */
    __IM uint32 PID0;           /*!< Offset: 0xFE0 (R/ )  ITM Peripheral Identification Register #0 */
    __IM uint32 PID1;           /*!< Offset: 0xFE4 (R/ )  ITM Peripheral Identification Register #1 */
    __IM uint32 PID2;           /*!< Offset: 0xFE8 (R/ )  ITM Peripheral Identification Register #2 */
    __IM uint32 PID3;           /*!< Offset: 0xFEC (R/ )  ITM Peripheral Identification Register #3 */
    __IM uint32 CID0;           /*!< Offset: 0xFF0 (R/ )  ITM Component  Identification Register #0 */
    __IM uint32 CID1;           /*!< Offset: 0xFF4 (R/ )  ITM Component  Identification Register #1 */
    __IM uint32 CID2;           /*!< Offset: 0xFF8 (R/ )  ITM Component  Identification Register #2 */
    __IM uint32 CID3;           /*!< Offset: 0xFFC (R/ )  ITM Component  Identification Register #3 */
} ITM_Type;

/* ITM Trace Privilege Register Definitions */
#define ITM_TPR_PRIVMASK_Pos                0U                                            /*!< ITM TPR: PRIVMASK Position */
#define ITM_TPR_PRIVMASK_Msk               (0xFFFFFFFFUL)     /*!< ITM TPR: PRIVMASK Mask */

/* ITM Trace Control Register Definitions */
#define ITM_TCR_BUSY_Pos                   23U                                            /*!< ITM TCR: BUSY Position */
#define ITM_TCR_BUSY_Msk                   (1UL << ITM_TCR_BUSY_Pos)                      /*!< ITM TCR: BUSY Mask */

#define ITM_TCR_TraceBusID_Pos             16U                                            /*!< ITM TCR: ATBID Position */
#define ITM_TCR_TraceBusID_Msk             (0x7FUL << ITM_TCR_TraceBusID_Pos)             /*!< ITM TCR: ATBID Mask */

#define ITM_TCR_GTSFREQ_Pos                10U                                            /*!< ITM TCR: Global timestamp frequency Position */
#define ITM_TCR_GTSFREQ_Msk                (3UL << ITM_TCR_GTSFREQ_Pos)                   /*!< ITM TCR: Global timestamp frequency Mask */

#define ITM_TCR_TSPrescale_Pos              8U                                            /*!< ITM TCR: TSPrescale Position */
#define ITM_TCR_TSPrescale_Msk             (3UL << ITM_TCR_TSPrescale_Pos)                /*!< ITM TCR: TSPrescale Mask */

#define ITM_TCR_SWOENA_Pos                  4U                                            /*!< ITM TCR: SWOENA Position */
#define ITM_TCR_SWOENA_Msk                 (1UL << ITM_TCR_SWOENA_Pos)                    /*!< ITM TCR: SWOENA Mask */

#define ITM_TCR_DWTENA_Pos                  3U                                            /*!< ITM TCR: DWTENA Position */
#define ITM_TCR_DWTENA_Msk                 (1UL << ITM_TCR_DWTENA_Pos)                    /*!< ITM TCR: DWTENA Mask */

#define ITM_TCR_SYNCENA_Pos                 2U                                            /*!< ITM TCR: SYNCENA Position */
#define ITM_TCR_SYNCENA_Msk                (1UL << ITM_TCR_SYNCENA_Pos)                   /*!< ITM TCR: SYNCENA Mask */

#define ITM_TCR_TSENA_Pos                   1U                                            /*!< ITM TCR: TSENA Position */
#define ITM_TCR_TSENA_Msk                  (1UL << ITM_TCR_TSENA_Pos)                     /*!< ITM TCR: TSENA Mask */

#define ITM_TCR_ITMENA_Pos                  0U                                            /*!< ITM TCR: ITM Enable bit Position */
#define ITM_TCR_ITMENA_Msk                 (1UL)                /*!< ITM TCR: ITM Enable bit Mask */

/* ITM Lock Status Register Definitions */
#define ITM_LSR_ByteAcc_Pos                 2U                                            /*!< ITM LSR: ByteAcc Position */
#define ITM_LSR_ByteAcc_Msk                (1UL << ITM_LSR_ByteAcc_Pos)                   /*!< ITM LSR: ByteAcc Mask */

#define ITM_LSR_Access_Pos                  1U                                            /*!< ITM LSR: Access Position */
#define ITM_LSR_Access_Msk                 (1UL << ITM_LSR_Access_Pos)                    /*!< ITM LSR: Access Mask */

#define ITM_LSR_Present_Pos                 0U                                            /*!< ITM LSR: Present Position */
#define ITM_LSR_Present_Msk                (1UL)               /*!< ITM LSR: Present Mask */

/*@}*/ /* end of group CMSIS_ITM */

/**
  \ingroup  CMSIS_core_register
  \defgroup CMSIS_DWT     Data Watchpoint and Trace (DWT)
  \brief    Type definitions for the Data Watchpoint and Trace (DWT)
  @{
 */

/**
  \brief  Structure type to access the Data Watchpoint and Trace Register (DWT).
 */
typedef struct
{
    __IOM uint32 CTRL;          /*!< Offset: 0x000 (R/W)  Control Register */
    __IOM uint32 CYCCNT;        /*!< Offset: 0x004 (R/W)  Cycle Count Register */
    __IOM uint32 CPICNT;        /*!< Offset: 0x008 (R/W)  CPI Count Register */
    __IOM uint32 EXCCNT;        /*!< Offset: 0x00C (R/W)  Exception Overhead Count Register */
    __IOM uint32 SLEEPCNT;      /*!< Offset: 0x010 (R/W)  Sleep Count Register */
    __IOM uint32 LSUCNT;        /*!< Offset: 0x014 (R/W)  LSU Count Register */
    __IOM uint32 FOLDCNT;       /*!< Offset: 0x018 (R/W)  Folded-instruction Count Register */
    __IM uint32 PCSR;           /*!< Offset: 0x01C (R/ )  Program Counter Sample Register */
    __IOM uint32 COMP0;         /*!< Offset: 0x020 (R/W)  Comparator Register 0 */
    __IOM uint32 MASK0;         /*!< Offset: 0x024 (R/W)  Mask Register 0 */
    __IOM uint32 FUNCTION0;     /*!< Offset: 0x028 (R/W)  Function Register 0 */
    uint32 RESERVED0[1U];
    __IOM uint32 COMP1;         /*!< Offset: 0x030 (R/W)  Comparator Register 1 */
    __IOM uint32 MASK1;         /*!< Offset: 0x034 (R/W)  Mask Register 1 */
    __IOM uint32 FUNCTION1;     /*!< Offset: 0x038 (R/W)  Function Register 1 */
    uint32 RESERVED1[1U];
    __IOM uint32 COMP2;         /*!< Offset: 0x040 (R/W)  Comparator Register 2 */
    __IOM uint32 MASK2;         /*!< Offset: 0x044 (R/W)  Mask Register 2 */
    __IOM uint32 FUNCTION2;     /*!< Offset: 0x048 (R/W)  Function Register 2 */
    uint32 RESERVED2[1U];
    __IOM uint32 COMP3;         /*!< Offset: 0x050 (R/W)  Comparator Register 3 */
    __IOM uint32 MASK3;         /*!< Offset: 0x054 (R/W)  Mask Register 3 */
    __IOM uint32 FUNCTION3;     /*!< Offset: 0x058 (R/W)  Function Register 3 */
} DWT_Type;

/* DWT Control Register Definitions */
#define DWT_CTRL_NUMCOMP_Pos               28U                                         /*!< DWT CTRL: NUMCOMP Position */
#define DWT_CTRL_NUMCOMP_Msk               (0xFUL << DWT_CTRL_NUMCOMP_Pos)             /*!< DWT CTRL: NUMCOMP Mask */

#define DWT_CTRL_NOTRCPKT_Pos              27U                                         /*!< DWT CTRL: NOTRCPKT Position */
#define DWT_CTRL_NOTRCPKT_Msk              (0x1UL << DWT_CTRL_NOTRCPKT_Pos)            /*!< DWT CTRL: NOTRCPKT Mask */

#define DWT_CTRL_NOEXTTRIG_Pos             26U                                         /*!< DWT CTRL: NOEXTTRIG Position */
#define DWT_CTRL_NOEXTTRIG_Msk             (0x1UL << DWT_CTRL_NOEXTTRIG_Pos)           /*!< DWT CTRL: NOEXTTRIG Mask */

#define DWT_CTRL_NOCYCCNT_Pos              25U                                         /*!< DWT CTRL: NOCYCCNT Position */
#define DWT_CTRL_NOCYCCNT_Msk              (0x1UL << DWT_CTRL_NOCYCCNT_Pos)            /*!< DWT CTRL: NOCYCCNT Mask */

#define DWT_CTRL_NOPRFCNT_Pos              24U                                         /*!< DWT CTRL: NOPRFCNT Position */
#define DWT_CTRL_NOPRFCNT_Msk              (0x1UL << DWT_CTRL_NOPRFCNT_Pos)            /*!< DWT CTRL: NOPRFCNT Mask */

#define DWT_CTRL_CYCEVTENA_Pos             22U                                         /*!< DWT CTRL: CYCEVTENA Position */
#define DWT_CTRL_CYCEVTENA_Msk             (0x1UL << DWT_CTRL_CYCEVTENA_Pos)           /*!< DWT CTRL: CYCEVTENA Mask */

#define DWT_CTRL_FOLDEVTENA_Pos            21U                                         /*!< DWT CTRL: FOLDEVTENA Position */
#define DWT_CTRL_FOLDEVTENA_Msk            (0x1UL << DWT_CTRL_FOLDEVTENA_Pos)          /*!< DWT CTRL: FOLDEVTENA Mask */

#define DWT_CTRL_LSUEVTENA_Pos             20U                                         /*!< DWT CTRL: LSUEVTENA Position */
#define DWT_CTRL_LSUEVTENA_Msk             (0x1UL << DWT_CTRL_LSUEVTENA_Pos)           /*!< DWT CTRL: LSUEVTENA Mask */

#define DWT_CTRL_SLEEPEVTENA_Pos           19U                                         /*!< DWT CTRL: SLEEPEVTENA Position */
#define DWT_CTRL_SLEEPEVTENA_Msk           (0x1UL << DWT_CTRL_SLEEPEVTENA_Pos)         /*!< DWT CTRL: SLEEPEVTENA Mask */

#define DWT_CTRL_EXCEVTENA_Pos             18U                                         /*!< DWT CTRL: EXCEVTENA Position */
#define DWT_CTRL_EXCEVTENA_Msk             (0x1UL << DWT_CTRL_EXCEVTENA_Pos)           /*!< DWT CTRL: EXCEVTENA Mask */

#define DWT_CTRL_CPIEVTENA_Pos             17U                                         /*!< DWT CTRL: CPIEVTENA Position */
#define DWT_CTRL_CPIEVTENA_Msk             (0x1UL << DWT_CTRL_CPIEVTENA_Pos)           /*!< DWT CTRL: CPIEVTENA Mask */

#define DWT_CTRL_EXCTRCENA_Pos             16U                                         /*!< DWT CTRL: EXCTRCENA Position */
#define DWT_CTRL_EXCTRCENA_Msk             (0x1UL << DWT_CTRL_EXCTRCENA_Pos)           /*!< DWT CTRL: EXCTRCENA Mask */

#define DWT_CTRL_PCSAMPLENA_Pos            12U                                         /*!< DWT CTRL: PCSAMPLENA Position */
#define DWT_CTRL_PCSAMPLENA_Msk            (0x1UL << DWT_CTRL_PCSAMPLENA_Pos)          /*!< DWT CTRL: PCSAMPLENA Mask */

#define DWT_CTRL_SYNCTAP_Pos               10U                                         /*!< DWT CTRL: SYNCTAP Position */
#define DWT_CTRL_SYNCTAP_Msk               (0x3UL << DWT_CTRL_SYNCTAP_Pos)             /*!< DWT CTRL: SYNCTAP Mask */

#define DWT_CTRL_CYCTAP_Pos                 9U                                         /*!< DWT CTRL: CYCTAP Position */
#define DWT_CTRL_CYCTAP_Msk                (0x1UL << DWT_CTRL_CYCTAP_Pos)              /*!< DWT CTRL: CYCTAP Mask */

#define DWT_CTRL_POSTINIT_Pos               5U                                         /*!< DWT CTRL: POSTINIT Position */
#define DWT_CTRL_POSTINIT_Msk              (0xFUL << DWT_CTRL_POSTINIT_Pos)            /*!< DWT CTRL: POSTINIT Mask */

#define DWT_CTRL_POSTPRESET_Pos             1U                                         /*!< DWT CTRL: POSTPRESET Position */
#define DWT_CTRL_POSTPRESET_Msk            (0xFUL << DWT_CTRL_POSTPRESET_Pos)          /*!< DWT CTRL: POSTPRESET Mask */

#define DWT_CTRL_CYCCNTENA_Pos              0U                                         /*!< DWT CTRL: CYCCNTENA Position */
#define DWT_CTRL_CYCCNTENA_Msk             (0x1UL)       /*!< DWT CTRL: CYCCNTENA Mask */

/* DWT CPI Count Register Definitions */
#define DWT_CPICNT_CPICNT_Pos               0U                                         /*!< DWT CPICNT: CPICNT Position */
#define DWT_CPICNT_CPICNT_Msk              (0xFFUL)       /*!< DWT CPICNT: CPICNT Mask */

/* DWT Exception Overhead Count Register Definitions */
#define DWT_EXCCNT_EXCCNT_Pos               0U                                         /*!< DWT EXCCNT: EXCCNT Position */
#define DWT_EXCCNT_EXCCNT_Msk              (0xFFUL)       /*!< DWT EXCCNT: EXCCNT Mask */

/* DWT Sleep Count Register Definitions */
#define DWT_SLEEPCNT_SLEEPCNT_Pos           0U                                         /*!< DWT SLEEPCNT: SLEEPCNT Position */
#define DWT_SLEEPCNT_SLEEPCNT_Msk          (0xFFUL)   /*!< DWT SLEEPCNT: SLEEPCNT Mask */

/* DWT LSU Count Register Definitions */
#define DWT_LSUCNT_LSUCNT_Pos               0U                                         /*!< DWT LSUCNT: LSUCNT Position */
#define DWT_LSUCNT_LSUCNT_Msk              (0xFFUL)       /*!< DWT LSUCNT: LSUCNT Mask */

/* DWT Folded-instruction Count Register Definitions */
#define DWT_FOLDCNT_FOLDCNT_Pos             0U                                         /*!< DWT FOLDCNT: FOLDCNT Position */
#define DWT_FOLDCNT_FOLDCNT_Msk            (0xFFUL)     /*!< DWT FOLDCNT: FOLDCNT Mask */

/* DWT Comparator Mask Register Definitions */
#define DWT_MASK_MASK_Pos                   0U                                         /*!< DWT MASK: MASK Position */
#define DWT_MASK_MASK_Msk                  (0x1FUL)           /*!< DWT MASK: MASK Mask */

/* DWT Comparator Function Register Definitions */
#define DWT_FUNCTION_MATCHED_Pos           24U                                         /*!< DWT FUNCTION: MATCHED Position */
#define DWT_FUNCTION_MATCHED_Msk           (0x1UL << DWT_FUNCTION_MATCHED_Pos)         /*!< DWT FUNCTION: MATCHED Mask */

#define DWT_FUNCTION_DATAVADDR1_Pos        16U                                         /*!< DWT FUNCTION: DATAVADDR1 Position */
#define DWT_FUNCTION_DATAVADDR1_Msk        (0xFUL << DWT_FUNCTION_DATAVADDR1_Pos)      /*!< DWT FUNCTION: DATAVADDR1 Mask */

#define DWT_FUNCTION_DATAVADDR0_Pos        12U                                         /*!< DWT FUNCTION: DATAVADDR0 Position */
#define DWT_FUNCTION_DATAVADDR0_Msk        (0xFUL << DWT_FUNCTION_DATAVADDR0_Pos)      /*!< DWT FUNCTION: DATAVADDR0 Mask */

#define DWT_FUNCTION_DATAVSIZE_Pos         10U                                         /*!< DWT FUNCTION: DATAVSIZE Position */
#define DWT_FUNCTION_DATAVSIZE_Msk         (0x3UL << DWT_FUNCTION_DATAVSIZE_Pos)       /*!< DWT FUNCTION: DATAVSIZE Mask */

#define DWT_FUNCTION_LNK1ENA_Pos            9U                                         /*!< DWT FUNCTION: LNK1ENA Position */
#define DWT_FUNCTION_LNK1ENA_Msk           (0x1UL << DWT_FUNCTION_LNK1ENA_Pos)         /*!< DWT FUNCTION: LNK1ENA Mask */

#define DWT_FUNCTION_DATAVMATCH_Pos         8U                                         /*!< DWT FUNCTION: DATAVMATCH Position */
#define DWT_FUNCTION_DATAVMATCH_Msk        (0x1UL << DWT_FUNCTION_DATAVMATCH_Pos)      /*!< DWT FUNCTION: DATAVMATCH Mask */

#define DWT_FUNCTION_CYCMATCH_Pos           7U                                         /*!< DWT FUNCTION: CYCMATCH Position */
#define DWT_FUNCTION_CYCMATCH_Msk          (0x1UL << DWT_FUNCTION_CYCMATCH_Pos)        /*!< DWT FUNCTION: CYCMATCH Mask */

#define DWT_FUNCTION_EMITRANGE_Pos          5U                                         /*!< DWT FUNCTION: EMITRANGE Position */
#define DWT_FUNCTION_EMITRANGE_Msk         (0x1UL << DWT_FUNCTION_EMITRANGE_Pos)       /*!< DWT FUNCTION: EMITRANGE Mask */

#define DWT_FUNCTION_FUNCTION_Pos           0U                                         /*!< DWT FUNCTION: FUNCTION Position */
#define DWT_FUNCTION_FUNCTION_Msk          (0xFUL)    /*!< DWT FUNCTION: FUNCTION Mask */

/*@}*/ /* end of group CMSIS_DWT */

/**
  \ingroup  CMSIS_core_register
  \defgroup CMSIS_TPI     Trace Port Interface (TPI)
  \brief    Type definitions for the Trace Port Interface (TPI)
  @{
 */

/**
  \brief  Structure type to access the Trace Port Interface Register (TPI).
 */
typedef struct
{
    __IM uint32 SSPSR;          /*!< Offset: 0x000 (R/ )  Supported Parallel Port Size Register */
    __IOM uint32 CSPSR;         /*!< Offset: 0x004 (R/W)  Current Parallel Port Size Register */
    uint32 RESERVED0[2U];
    __IOM uint32 ACPR;          /*!< Offset: 0x010 (R/W)  Asynchronous Clock Prescaler Register */
    uint32 RESERVED1[55U];
    __IOM uint32 SPPR;          /*!< Offset: 0x0F0 (R/W)  Selected Pin Protocol Register */
    uint32 RESERVED2[131U];
    __IM uint32 FFSR;           /*!< Offset: 0x300 (R/ )  Formatter and Flush Status Register */
    __IOM uint32 FFCR;          /*!< Offset: 0x304 (R/W)  Formatter and Flush Control Register */
    __IM uint32 FSCR;           /*!< Offset: 0x308 (R/ )  Formatter Synchronization Counter Register */
    uint32 RESERVED3[759U];
    __IM uint32 TRIGGER;        /*!< Offset: 0xEE8 (R/ )  TRIGGER Register */
    __IM uint32 FIFO0;          /*!< Offset: 0xEEC (R/ )  Integration ETM Data */
    __IM uint32 ITATBCTR2;      /*!< Offset: 0xEF0 (R/ )  ITATBCTR2 */
    uint32 RESERVED4[1U];
    __IM uint32 ITATBCTR0;      /*!< Offset: 0xEF8 (R/ )  ITATBCTR0 */
    __IM uint32 FIFO1;          /*!< Offset: 0xEFC (R/ )  Integration ITM Data */
    __IOM uint32 ITCTRL;        /*!< Offset: 0xF00 (R/W)  Integration Mode Control */
    uint32 RESERVED5[39U];
    __IOM uint32 CLAIMSET;      /*!< Offset: 0xFA0 (R/W)  Claim tag set */
    __IOM uint32 CLAIMCLR;      /*!< Offset: 0xFA4 (R/W)  Claim tag clear */
    uint32 RESERVED7[8U];
    __IM uint32 DEVID;          /*!< Offset: 0xFC8 (R/ )  TPIU_DEVID */
    __IM uint32 DEVTYPE;        /*!< Offset: 0xFCC (R/ )  TPIU_DEVTYPE */
} TPI_Type;                      //PRQA S 1535 # AMCE_SYS_1535

/* TPI Asynchronous Clock Prescaler Register Definitions */
#define TPI_ACPR_PRESCALER_Pos              0U                                         /*!< TPI ACPR: PRESCALER Position */
#define TPI_ACPR_PRESCALER_Msk             (0x1FFFUL)    /*!< TPI ACPR: PRESCALER Mask */

/* TPI Selected Pin Protocol Register Definitions */
#define TPI_SPPR_TXMODE_Pos                 0U                                         /*!< TPI SPPR: TXMODE Position */
#define TPI_SPPR_TXMODE_Msk                (0x3UL)          /*!< TPI SPPR: TXMODE Mask */

/* TPI Formatter and Flush Status Register Definitions */
#define TPI_FFSR_FtNonStop_Pos              3U                                         /*!< TPI FFSR: FtNonStop Position */
#define TPI_FFSR_FtNonStop_Msk             (0x1UL << TPI_FFSR_FtNonStop_Pos)           /*!< TPI FFSR: FtNonStop Mask */

#define TPI_FFSR_TCPresent_Pos              2U                                         /*!< TPI FFSR: TCPresent Position */
#define TPI_FFSR_TCPresent_Msk             (0x1UL << TPI_FFSR_TCPresent_Pos)           /*!< TPI FFSR: TCPresent Mask */

#define TPI_FFSR_FtStopped_Pos              1U                                         /*!< TPI FFSR: FtStopped Position */
#define TPI_FFSR_FtStopped_Msk             (0x1UL << TPI_FFSR_FtStopped_Pos)           /*!< TPI FFSR: FtStopped Mask */

#define TPI_FFSR_FlInProg_Pos               0U                                         /*!< TPI FFSR: FlInProg Position */
#define TPI_FFSR_FlInProg_Msk              (0x1UL)        /*!< TPI FFSR: FlInProg Mask */

/* TPI Formatter and Flush Control Register Definitions */
#define TPI_FFCR_TrigIn_Pos                 8U                                         /*!< TPI FFCR: TrigIn Position */
#define TPI_FFCR_TrigIn_Msk                (0x1UL << TPI_FFCR_TrigIn_Pos)              /*!< TPI FFCR: TrigIn Mask */

#define TPI_FFCR_EnFCont_Pos                1U                                         /*!< TPI FFCR: EnFCont Position */
#define TPI_FFCR_EnFCont_Msk               (0x1UL << TPI_FFCR_EnFCont_Pos)             /*!< TPI FFCR: EnFCont Mask */

/* TPI TRIGGER Register Definitions */
#define TPI_TRIGGER_TRIGGER_Pos             0U                                         /*!< TPI TRIGGER: TRIGGER Position */
#define TPI_TRIGGER_TRIGGER_Msk            (0x1UL)      /*!< TPI TRIGGER: TRIGGER Mask */

/* TPI Integration ETM Data Register Definitions (FIFO0) */
#define TPI_FIFO0_ITM_ATVALID_Pos          29U                                         /*!< TPI FIFO0: ITM_ATVALID Position */
#define TPI_FIFO0_ITM_ATVALID_Msk          (0x1UL << TPI_FIFO0_ITM_ATVALID_Pos)        /*!< TPI FIFO0: ITM_ATVALID Mask */

#define TPI_FIFO0_ITM_bytecount_Pos        27U                                         /*!< TPI FIFO0: ITM_bytecount Position */
#define TPI_FIFO0_ITM_bytecount_Msk        (0x3UL << TPI_FIFO0_ITM_bytecount_Pos)      /*!< TPI FIFO0: ITM_bytecount Mask */

#define TPI_FIFO0_ETM_ATVALID_Pos          26U                                         /*!< TPI FIFO0: ETM_ATVALID Position */
#define TPI_FIFO0_ETM_ATVALID_Msk          (0x1UL << TPI_FIFO0_ETM_ATVALID_Pos)        /*!< TPI FIFO0: ETM_ATVALID Mask */

#define TPI_FIFO0_ETM_bytecount_Pos        24U                                         /*!< TPI FIFO0: ETM_bytecount Position */
#define TPI_FIFO0_ETM_bytecount_Msk        (0x3UL << TPI_FIFO0_ETM_bytecount_Pos)      /*!< TPI FIFO0: ETM_bytecount Mask */

#define TPI_FIFO0_ETM2_Pos                 16U                                         /*!< TPI FIFO0: ETM2 Position */
#define TPI_FIFO0_ETM2_Msk                 (0xFFUL << TPI_FIFO0_ETM2_Pos)              /*!< TPI FIFO0: ETM2 Mask */

#define TPI_FIFO0_ETM1_Pos                  8U                                         /*!< TPI FIFO0: ETM1 Position */
#define TPI_FIFO0_ETM1_Msk                 (0xFFUL << TPI_FIFO0_ETM1_Pos)              /*!< TPI FIFO0: ETM1 Mask */

#define TPI_FIFO0_ETM0_Pos                  0U                                         /*!< TPI FIFO0: ETM0 Position */
#define TPI_FIFO0_ETM0_Msk                 (0xFFUL)          /*!< TPI FIFO0: ETM0 Mask */

/* TPI ITATBCTR2 Register Definitions */
#define TPI_ITATBCTR2_ATREADY2_Pos          0U                                         /*!< TPI ITATBCTR2: ATREADY2 Position */
#define TPI_ITATBCTR2_ATREADY2_Msk         (0x1UL)   /*!< TPI ITATBCTR2: ATREADY2 Mask */

#define TPI_ITATBCTR2_ATREADY1_Pos          0U                                         /*!< TPI ITATBCTR2: ATREADY1 Position */
#define TPI_ITATBCTR2_ATREADY1_Msk         (0x1UL)   /*!< TPI ITATBCTR2: ATREADY1 Mask */

/* TPI Integration ITM Data Register Definitions (FIFO1) */
#define TPI_FIFO1_ITM_ATVALID_Pos          29U                                         /*!< TPI FIFO1: ITM_ATVALID Position */
#define TPI_FIFO1_ITM_ATVALID_Msk          (0x1UL << TPI_FIFO1_ITM_ATVALID_Pos)        /*!< TPI FIFO1: ITM_ATVALID Mask */

#define TPI_FIFO1_ITM_bytecount_Pos        27U                                         /*!< TPI FIFO1: ITM_bytecount Position */
#define TPI_FIFO1_ITM_bytecount_Msk        (0x3UL << TPI_FIFO1_ITM_bytecount_Pos)      /*!< TPI FIFO1: ITM_bytecount Mask */

#define TPI_FIFO1_ETM_ATVALID_Pos          26U                                         /*!< TPI FIFO1: ETM_ATVALID Position */
#define TPI_FIFO1_ETM_ATVALID_Msk          (0x1UL << TPI_FIFO1_ETM_ATVALID_Pos)        /*!< TPI FIFO1: ETM_ATVALID Mask */

#define TPI_FIFO1_ETM_bytecount_Pos        24U                                         /*!< TPI FIFO1: ETM_bytecount Position */
#define TPI_FIFO1_ETM_bytecount_Msk        (0x3UL << TPI_FIFO1_ETM_bytecount_Pos)      /*!< TPI FIFO1: ETM_bytecount Mask */

#define TPI_FIFO1_ITM2_Pos                 16U                                         /*!< TPI FIFO1: ITM2 Position */
#define TPI_FIFO1_ITM2_Msk                 (0xFFUL << TPI_FIFO1_ITM2_Pos)              /*!< TPI FIFO1: ITM2 Mask */

#define TPI_FIFO1_ITM1_Pos                  8U                                         /*!< TPI FIFO1: ITM1 Position */
#define TPI_FIFO1_ITM1_Msk                 (0xFFUL << TPI_FIFO1_ITM1_Pos)              /*!< TPI FIFO1: ITM1 Mask */

#define TPI_FIFO1_ITM0_Pos                  0U                                         /*!< TPI FIFO1: ITM0 Position */
#define TPI_FIFO1_ITM0_Msk                 (0xFFUL)          /*!< TPI FIFO1: ITM0 Mask */

/* TPI ITATBCTR0 Register Definitions */
#define TPI_ITATBCTR0_ATREADY2_Pos          0U                                         /*!< TPI ITATBCTR0: ATREADY2 Position */
#define TPI_ITATBCTR0_ATREADY2_Msk         (0x1UL)   /*!< TPI ITATBCTR0: ATREADY2 Mask */

#define TPI_ITATBCTR0_ATREADY1_Pos          0U                                         /*!< TPI ITATBCTR0: ATREADY1 Position */
#define TPI_ITATBCTR0_ATREADY1_Msk         (0x1UL)   /*!< TPI ITATBCTR0: ATREADY1 Mask */

/* TPI Integration Mode Control Register Definitions */
#define TPI_ITCTRL_Mode_Pos                 0U                                         /*!< TPI ITCTRL: Mode Position */
#define TPI_ITCTRL_Mode_Msk                (0x3UL)          /*!< TPI ITCTRL: Mode Mask */

/* TPI DEVID Register Definitions */
#define TPI_DEVID_NRZVALID_Pos             11U                                         /*!< TPI DEVID: NRZVALID Position */
#define TPI_DEVID_NRZVALID_Msk             (0x1UL << TPI_DEVID_NRZVALID_Pos)           /*!< TPI DEVID: NRZVALID Mask */

#define TPI_DEVID_MANCVALID_Pos            10U                                         /*!< TPI DEVID: MANCVALID Position */
#define TPI_DEVID_MANCVALID_Msk            (0x1UL << TPI_DEVID_MANCVALID_Pos)          /*!< TPI DEVID: MANCVALID Mask */

#define TPI_DEVID_PTINVALID_Pos             9U                                         /*!< TPI DEVID: PTINVALID Position */
#define TPI_DEVID_PTINVALID_Msk            (0x1UL << TPI_DEVID_PTINVALID_Pos)          /*!< TPI DEVID: PTINVALID Mask */

#define TPI_DEVID_MinBufSz_Pos              6U                                         /*!< TPI DEVID: MinBufSz Position */
#define TPI_DEVID_MinBufSz_Msk             (0x7UL << TPI_DEVID_MinBufSz_Pos)           /*!< TPI DEVID: MinBufSz Mask */

#define TPI_DEVID_AsynClkIn_Pos             5U                                         /*!< TPI DEVID: AsynClkIn Position */
#define TPI_DEVID_AsynClkIn_Msk            (0x1UL << TPI_DEVID_AsynClkIn_Pos)          /*!< TPI DEVID: AsynClkIn Mask */

#define TPI_DEVID_NrTraceInput_Pos          0U                                         /*!< TPI DEVID: NrTraceInput Position */
#define TPI_DEVID_NrTraceInput_Msk         (0x1FUL)  /*!< TPI DEVID: NrTraceInput Mask */

/* TPI DEVTYPE Register Definitions */
#define TPI_DEVTYPE_SubType_Pos             4U                                         /*!< TPI DEVTYPE: SubType Position */
#define TPI_DEVTYPE_SubType_Msk            (0xFUL)      /*!< TPI DEVTYPE: SubType Mask */

#define TPI_DEVTYPE_MajorType_Pos           0U                                         /*!< TPI DEVTYPE: MajorType Position */
#define TPI_DEVTYPE_MajorType_Msk          (0xFUL << TPI_DEVTYPE_MajorType_Pos)        /*!< TPI DEVTYPE: MajorType Mask */

/*@}*/ /* end of group CMSIS_TPI */

#if defined (__MPU_PRESENT) && (__MPU_PRESENT == 1U)
/**
  \ingroup  CMSIS_core_register
  \defgroup CMSIS_MPU     Memory Protection Unit (MPU)
  \brief    Type definitions for the Memory Protection Unit (MPU)
  @{
 */

/**
  \brief  Structure type to access the Memory Protection Unit (MPU).
 */
typedef struct
{
    __IM uint32 TYPE;       /*!< Offset: 0x000 (R/ )  MPU Type Register */
    __IOM uint32 CTRL;      /*!< Offset: 0x004 (R/W)  MPU Control Register */
    __IOM uint32 RNR;       /*!< Offset: 0x008 (R/W)  MPU Region RNRber Register */
    __IOM uint32 RBAR;      /*!< Offset: 0x00C (R/W)  MPU Region Base Address Register */
    __IOM uint32 RASR;      /*!< Offset: 0x010 (R/W)  MPU Region Attribute and Size Register */
    __IOM uint32 RBAR_A1;   /*!< Offset: 0x014 (R/W)  MPU Alias 1 Region Base Address Register */
    __IOM uint32 RASR_A1;   /*!< Offset: 0x018 (R/W)  MPU Alias 1 Region Attribute and Size Register */
    __IOM uint32 RBAR_A2;   /*!< Offset: 0x01C (R/W)  MPU Alias 2 Region Base Address Register */
    __IOM uint32 RASR_A2;   /*!< Offset: 0x020 (R/W)  MPU Alias 2 Region Attribute and Size Register */
    __IOM uint32 RBAR_A3;   /*!< Offset: 0x024 (R/W)  MPU Alias 3 Region Base Address Register */
    __IOM uint32 RASR_A3;   /*!< Offset: 0x028 (R/W)  MPU Alias 3 Region Attribute and Size Register */
} MPU_Type;

#define MPU_TYPE_RALIASES                  4U

/* MPU Type Register Definitions */
#define MPU_TYPE_IREGION_Pos               16U                                            /*!< MPU TYPE: IREGION Position */
#define MPU_TYPE_IREGION_Msk               (0xFFUL << MPU_TYPE_IREGION_Pos)               /*!< MPU TYPE: IREGION Mask */

#define MPU_TYPE_DREGION_Pos                8U                                            /*!< MPU TYPE: DREGION Position */
#define MPU_TYPE_DREGION_Msk               (0xFFUL << MPU_TYPE_DREGION_Pos)               /*!< MPU TYPE: DREGION Mask */

#define MPU_TYPE_SEPARATE_Pos               0U                                            /*!< MPU TYPE: SEPARATE Position */
#define MPU_TYPE_SEPARATE_Msk              (1UL)             /*!< MPU TYPE: SEPARATE Mask */

/* MPU Control Register Definitions */
#define MPU_CTRL_PRIVDEFENA_Pos             2U                                            /*!< MPU CTRL: PRIVDEFENA Position */
#define MPU_CTRL_PRIVDEFENA_Msk            (1UL << MPU_CTRL_PRIVDEFENA_Pos)               /*!< MPU CTRL: PRIVDEFENA Mask */

#define MPU_CTRL_HFNMIENA_Pos               1U                                            /*!< MPU CTRL: HFNMIENA Position */
#define MPU_CTRL_HFNMIENA_Msk              (1UL << MPU_CTRL_HFNMIENA_Pos)                 /*!< MPU CTRL: HFNMIENA Mask */

#define MPU_CTRL_ENABLE_Pos                 0U                                            /*!< MPU CTRL: ENABLE Position */
#define MPU_CTRL_ENABLE_Msk                (1UL)               /*!< MPU CTRL: ENABLE Mask */

/* MPU Region Number Register Definitions */
#define MPU_RNR_REGION_Pos                  0U                                            /*!< MPU RNR: REGION Position */
#define MPU_RNR_REGION_Msk                 (0xFFUL)             /*!< MPU RNR: REGION Mask */

/* MPU Region Base Address Register Definitions */
#define MPU_RBAR_ADDR_Pos                   5U                                            /*!< MPU RBAR: ADDR Position */
#define MPU_RBAR_ADDR_Msk                  (0x7FFFFFFUL << MPU_RBAR_ADDR_Pos)             /*!< MPU RBAR: ADDR Mask */

#define MPU_RBAR_VALID_Pos                  4U                                            /*!< MPU RBAR: VALID Position */
#define MPU_RBAR_VALID_Msk                 (1UL << MPU_RBAR_VALID_Pos)                    /*!< MPU RBAR: VALID Mask */

#define MPU_RBAR_REGION_Pos                 0U                                            /*!< MPU RBAR: REGION Position */
#define MPU_RBAR_REGION_Msk                (0xFUL)             /*!< MPU RBAR: REGION Mask */

/* MPU Region Attribute and Size Register Definitions */
#define MPU_RASR_ATTRS_Pos                 16U                                            /*!< MPU RASR: MPU Region Attribute field Position */
#define MPU_RASR_ATTRS_Msk                 (0xFFFFUL << MPU_RASR_ATTRS_Pos)               /*!< MPU RASR: MPU Region Attribute field Mask */

#define MPU_RASR_XN_Pos                    28U                                            /*!< MPU RASR: ATTRS.XN Position */
#define MPU_RASR_XN_Msk                    (1UL << MPU_RASR_XN_Pos)                       /*!< MPU RASR: ATTRS.XN Mask */

#define MPU_RASR_AP_Pos                    24U                                            /*!< MPU RASR: ATTRS.AP Position */
#define MPU_RASR_AP_Msk                    (0x7UL << MPU_RASR_AP_Pos)                     /*!< MPU RASR: ATTRS.AP Mask */

#define MPU_RASR_TEX_Pos                   19U                                            /*!< MPU RASR: ATTRS.TEX Position */
#define MPU_RASR_TEX_Msk                   (0x7UL << MPU_RASR_TEX_Pos)                    /*!< MPU RASR: ATTRS.TEX Mask */

#define MPU_RASR_S_Pos                     18U                                            /*!< MPU RASR: ATTRS.S Position */
#define MPU_RASR_S_Msk                     (1UL << MPU_RASR_S_Pos)                        /*!< MPU RASR: ATTRS.S Mask */

#define MPU_RASR_C_Pos                     17U                                            /*!< MPU RASR: ATTRS.C Position */
#define MPU_RASR_C_Msk                     (1UL << MPU_RASR_C_Pos)                        /*!< MPU RASR: ATTRS.C Mask */

#define MPU_RASR_B_Pos                     16U                                            /*!< MPU RASR: ATTRS.B Position */
#define MPU_RASR_B_Msk                     (1UL << MPU_RASR_B_Pos)                        /*!< MPU RASR: ATTRS.B Mask */

#define MPU_RASR_SRD_Pos                    8U                                            /*!< MPU RASR: Sub-Region Disable Position */
#define MPU_RASR_SRD_Msk                   (0xFFUL << MPU_RASR_SRD_Pos)                   /*!< MPU RASR: Sub-Region Disable Mask */

#define MPU_RASR_SIZE_Pos                   1U                                            /*!< MPU RASR: Region Size Field Position */
#define MPU_RASR_SIZE_Msk                  (0x1FUL << MPU_RASR_SIZE_Pos)                  /*!< MPU RASR: Region Size Field Mask */

#define MPU_RASR_ENABLE_Pos                 0U                                            /*!< MPU RASR: Region enable bit Position */
#define MPU_RASR_ENABLE_Msk                (1UL)               /*!< MPU RASR: Region enable bit Disable Mask */

/*@} end of group CMSIS_MPU */
#endif /* defined __MPU_PRESENT && __MPU_PRESENT == 1U */

/**
  \ingroup  CMSIS_core_register
  \defgroup CMSIS_FPU     Floating Point Unit (FPU)
  \brief    Type definitions for the Floating Point Unit (FPU)
  @{
 */

/**
  \brief  Structure type to access the Floating Point Unit (FPU).
 */
typedef struct
{
    uint32 RESERVED0[1U];
    __IOM uint32 FPCCR;     /*!< Offset: 0x004 (R/W)  Floating-Point Context Control Register */
    __IOM uint32 FPCAR;     /*!< Offset: 0x008 (R/W)  Floating-Point Context Address Register */
    __IOM uint32 FPDSCR;    /*!< Offset: 0x00C (R/W)  Floating-Point Default Status Control Register */
    __IM uint32 MVFR0;      /*!< Offset: 0x010 (R/ )  Media and FP Feature Register 0 */
    __IM uint32 MVFR1;      /*!< Offset: 0x014 (R/ )  Media and FP Feature Register 1 */
    __IM uint32 MVFR2;      /*!< Offset: 0x018 (R/ )  Media and FP Feature Register 2 */
} FPU_Type;

/* Floating-Point Context Control Register Definitions */
#define FPU_FPCCR_ASPEN_Pos                31U                                            /*!< FPCCR: ASPEN bit Position */
#define FPU_FPCCR_ASPEN_Msk                (1UL << FPU_FPCCR_ASPEN_Pos)                   /*!< FPCCR: ASPEN bit Mask */

#define FPU_FPCCR_LSPEN_Pos                30U                                            /*!< FPCCR: LSPEN Position */
#define FPU_FPCCR_LSPEN_Msk                (1UL << FPU_FPCCR_LSPEN_Pos)                   /*!< FPCCR: LSPEN bit Mask */

#define FPU_FPCCR_MONRDY_Pos                8U                                            /*!< FPCCR: MONRDY Position */
#define FPU_FPCCR_MONRDY_Msk               (1UL << FPU_FPCCR_MONRDY_Pos)                  /*!< FPCCR: MONRDY bit Mask */

#define FPU_FPCCR_BFRDY_Pos                 6U                                            /*!< FPCCR: BFRDY Position */
#define FPU_FPCCR_BFRDY_Msk                (1UL << FPU_FPCCR_BFRDY_Pos)                   /*!< FPCCR: BFRDY bit Mask */

#define FPU_FPCCR_MMRDY_Pos                 5U                                            /*!< FPCCR: MMRDY Position */
#define FPU_FPCCR_MMRDY_Msk                (1UL << FPU_FPCCR_MMRDY_Pos)                   /*!< FPCCR: MMRDY bit Mask */

#define FPU_FPCCR_HFRDY_Pos                 4U                                            /*!< FPCCR: HFRDY Position */
#define FPU_FPCCR_HFRDY_Msk                (1UL << FPU_FPCCR_HFRDY_Pos)                   /*!< FPCCR: HFRDY bit Mask */

#define FPU_FPCCR_THREAD_Pos                3U                                            /*!< FPCCR: processor mode bit Position */
#define FPU_FPCCR_THREAD_Msk               (1UL << FPU_FPCCR_THREAD_Pos)                  /*!< FPCCR: processor mode active bit Mask */

#define FPU_FPCCR_USER_Pos                  1U                                            /*!< FPCCR: privilege level bit Position */
#define FPU_FPCCR_USER_Msk                 (1UL << FPU_FPCCR_USER_Pos)                    /*!< FPCCR: privilege level bit Mask */

#define FPU_FPCCR_LSPACT_Pos                0U                                            /*!< FPCCR: Lazy state preservation active bit Position */
#define FPU_FPCCR_LSPACT_Msk               (1UL)              /*!< FPCCR: Lazy state preservation active bit Mask */

/* Floating-Point Context Address Register Definitions */
#define FPU_FPCAR_ADDRESS_Pos               3U                                            /*!< FPCAR: ADDRESS bit Position */
#define FPU_FPCAR_ADDRESS_Msk              (0x1FFFFFFFUL << FPU_FPCAR_ADDRESS_Pos)        /*!< FPCAR: ADDRESS bit Mask */

/* Floating-Point Default Status Control Register Definitions */
#define FPU_FPDSCR_AHP_Pos                 26U                                            /*!< FPDSCR: AHP bit Position */
#define FPU_FPDSCR_AHP_Msk                 (1UL << FPU_FPDSCR_AHP_Pos)                    /*!< FPDSCR: AHP bit Mask */

#define FPU_FPDSCR_DN_Pos                  25U                                            /*!< FPDSCR: DN bit Position */
#define FPU_FPDSCR_DN_Msk                  (1UL << FPU_FPDSCR_DN_Pos)                     /*!< FPDSCR: DN bit Mask */

#define FPU_FPDSCR_FZ_Pos                  24U                                            /*!< FPDSCR: FZ bit Position */
#define FPU_FPDSCR_FZ_Msk                  (1UL << FPU_FPDSCR_FZ_Pos)                     /*!< FPDSCR: FZ bit Mask */

#define FPU_FPDSCR_RMode_Pos               22U                                            /*!< FPDSCR: RMode bit Position */
#define FPU_FPDSCR_RMode_Msk               (3UL << FPU_FPDSCR_RMode_Pos)                  /*!< FPDSCR: RMode bit Mask */

/* Media and FP Feature Register 0 Definitions */
#define FPU_MVFR0_FP_rounding_modes_Pos    28U                                            /*!< MVFR0: FP rounding modes bits Position */
#define FPU_MVFR0_FP_rounding_modes_Msk    (0xFUL << FPU_MVFR0_FP_rounding_modes_Pos)     /*!< MVFR0: FP rounding modes bits Mask */

#define FPU_MVFR0_Short_vectors_Pos        24U                                            /*!< MVFR0: Short vectors bits Position */
#define FPU_MVFR0_Short_vectors_Msk        (0xFUL << FPU_MVFR0_Short_vectors_Pos)         /*!< MVFR0: Short vectors bits Mask */

#define FPU_MVFR0_Square_root_Pos          20U                                            /*!< MVFR0: Square root bits Position */
#define FPU_MVFR0_Square_root_Msk          (0xFUL << FPU_MVFR0_Square_root_Pos)           /*!< MVFR0: Square root bits Mask */

#define FPU_MVFR0_Divide_Pos               16U                                            /*!< MVFR0: Divide bits Position */
#define FPU_MVFR0_Divide_Msk               (0xFUL << FPU_MVFR0_Divide_Pos)                /*!< MVFR0: Divide bits Mask */

#define FPU_MVFR0_FP_excep_trapping_Pos    12U                                            /*!< MVFR0: FP exception trapping bits Position */
#define FPU_MVFR0_FP_excep_trapping_Msk    (0xFUL << FPU_MVFR0_FP_excep_trapping_Pos)     /*!< MVFR0: FP exception trapping bits Mask */

#define FPU_MVFR0_Double_precision_Pos      8U                                            /*!< MVFR0: Double-precision bits Position */
#define FPU_MVFR0_Double_precision_Msk     (0xFUL << FPU_MVFR0_Double_precision_Pos)      /*!< MVFR0: Double-precision bits Mask */

#define FPU_MVFR0_Single_precision_Pos      4U                                            /*!< MVFR0: Single-precision bits Position */
#define FPU_MVFR0_Single_precision_Msk     (0xFUL << FPU_MVFR0_Single_precision_Pos)      /*!< MVFR0: Single-precision bits Mask */

#define FPU_MVFR0_A_SIMD_registers_Pos      0U                                            /*!< MVFR0: A_SIMD registers bits Position */
#define FPU_MVFR0_A_SIMD_registers_Msk     (0xFUL)  /*!< MVFR0: A_SIMD registers bits Mask */

/* Media and FP Feature Register 1 Definitions */
#define FPU_MVFR1_FP_fused_MAC_Pos         28U                                            /*!< MVFR1: FP fused MAC bits Position */
#define FPU_MVFR1_FP_fused_MAC_Msk         (0xFUL << FPU_MVFR1_FP_fused_MAC_Pos)          /*!< MVFR1: FP fused MAC bits Mask */

#define FPU_MVFR1_FP_HPFP_Pos              24U                                            /*!< MVFR1: FP HPFP bits Position */
#define FPU_MVFR1_FP_HPFP_Msk              (0xFUL << FPU_MVFR1_FP_HPFP_Pos)               /*!< MVFR1: FP HPFP bits Mask */

#define FPU_MVFR1_D_NaN_mode_Pos            4U                                            /*!< MVFR1: D_NaN mode bits Position */
#define FPU_MVFR1_D_NaN_mode_Msk           (0xFUL << FPU_MVFR1_D_NaN_mode_Pos)            /*!< MVFR1: D_NaN mode bits Mask */

#define FPU_MVFR1_FtZ_mode_Pos              0U                                            /*!< MVFR1: FtZ mode bits Position */
#define FPU_MVFR1_FtZ_mode_Msk             (0xFUL)          /*!< MVFR1: FtZ mode bits Mask */

/* Media and FP Feature Register 2 Definitions */

#define FPU_MVFR2_VFP_Misc_Pos              4U                                            /*!< MVFR2: VFP Misc bits Position */
#define FPU_MVFR2_VFP_Misc_Msk             (0xFUL << FPU_MVFR2_VFP_Misc_Pos)              /*!< MVFR2: VFP Misc bits Mask */

/*@} end of group CMSIS_FPU */

/**
  \ingroup  CMSIS_core_register
  \defgroup CMSIS_CoreDebug       Core Debug Registers (CoreDebug)
  \brief    Type definitions for the Core Debug Registers
  @{
 */

/**
  \brief  Structure type to access the Core Debug Register (CoreDebug).
 */
typedef struct
{
    __IOM uint32 DHCSR; /*!< Offset: 0x000 (R/W)  Debug Halting Control and Status Register */
    __OM uint32 DCRSR; /*!< Offset: 0x004 ( /W)  Debug Core Register Selector Register */
    __IOM uint32 DCRDR; /*!< Offset: 0x008 (R/W)  Debug Core Register Data Register */
    __IOM uint32 DEMCR; /*!< Offset: 0x00C (R/W)  Debug Exception and Monitor Control Register */
} CoreDebug_Type;

/* Debug Halting Control and Status Register Definitions */
#define CoreDebug_DHCSR_DBGKEY_Pos         16U                                            /*!< CoreDebug DHCSR: DBGKEY Position */
#define CoreDebug_DHCSR_DBGKEY_Msk         (0xFFFFUL << CoreDebug_DHCSR_DBGKEY_Pos)       /*!< CoreDebug DHCSR: DBGKEY Mask */

#define CoreDebug_DHCSR_S_RESET_ST_Pos     25U                                            /*!< CoreDebug DHCSR: S_RESET_ST Position */
#define CoreDebug_DHCSR_S_RESET_ST_Msk     (1UL << CoreDebug_DHCSR_S_RESET_ST_Pos)        /*!< CoreDebug DHCSR: S_RESET_ST Mask */

#define CoreDebug_DHCSR_S_RETIRE_ST_Pos    24U                                            /*!< CoreDebug DHCSR: S_RETIRE_ST Position */
#define CoreDebug_DHCSR_S_RETIRE_ST_Msk    (1UL << CoreDebug_DHCSR_S_RETIRE_ST_Pos)       /*!< CoreDebug DHCSR: S_RETIRE_ST Mask */

#define CoreDebug_DHCSR_S_LOCKUP_Pos       19U                                            /*!< CoreDebug DHCSR: S_LOCKUP Position */
#define CoreDebug_DHCSR_S_LOCKUP_Msk       (1UL << CoreDebug_DHCSR_S_LOCKUP_Pos)          /*!< CoreDebug DHCSR: S_LOCKUP Mask */

#define CoreDebug_DHCSR_S_SLEEP_Pos        18U                                            /*!< CoreDebug DHCSR: S_SLEEP Position */
#define CoreDebug_DHCSR_S_SLEEP_Msk        (1UL << CoreDebug_DHCSR_S_SLEEP_Pos)           /*!< CoreDebug DHCSR: S_SLEEP Mask */

#define CoreDebug_DHCSR_S_HALT_Pos         17U                                            /*!< CoreDebug DHCSR: S_HALT Position */
#define CoreDebug_DHCSR_S_HALT_Msk         (1UL << CoreDebug_DHCSR_S_HALT_Pos)            /*!< CoreDebug DHCSR: S_HALT Mask */

#define CoreDebug_DHCSR_S_REGRDY_Pos       16U                                            /*!< CoreDebug DHCSR: S_REGRDY Position */
#define CoreDebug_DHCSR_S_REGRDY_Msk       (1UL << CoreDebug_DHCSR_S_REGRDY_Pos)          /*!< CoreDebug DHCSR: S_REGRDY Mask */

#define CoreDebug_DHCSR_C_SNAPSTALL_Pos     5U                                            /*!< CoreDebug DHCSR: C_SNAPSTALL Position */
#define CoreDebug_DHCSR_C_SNAPSTALL_Msk    (1UL << CoreDebug_DHCSR_C_SNAPSTALL_Pos)       /*!< CoreDebug DHCSR: C_SNAPSTALL Mask */

#define CoreDebug_DHCSR_C_MASKINTS_Pos      3U                                            /*!< CoreDebug DHCSR: C_MASKINTS Position */
#define CoreDebug_DHCSR_C_MASKINTS_Msk     (1UL << CoreDebug_DHCSR_C_MASKINTS_Pos)        /*!< CoreDebug DHCSR: C_MASKINTS Mask */

#define CoreDebug_DHCSR_C_STEP_Pos          2U                                            /*!< CoreDebug DHCSR: C_STEP Position */
#define CoreDebug_DHCSR_C_STEP_Msk         (1UL << CoreDebug_DHCSR_C_STEP_Pos)            /*!< CoreDebug DHCSR: C_STEP Mask */

#define CoreDebug_DHCSR_C_HALT_Pos          1U                                            /*!< CoreDebug DHCSR: C_HALT Position */
#define CoreDebug_DHCSR_C_HALT_Msk         (1UL << CoreDebug_DHCSR_C_HALT_Pos)            /*!< CoreDebug DHCSR: C_HALT Mask */

#define CoreDebug_DHCSR_C_DEBUGEN_Pos       0U                                            /*!< CoreDebug DHCSR: C_DEBUGEN Position */
#define CoreDebug_DHCSR_C_DEBUGEN_Msk      (1UL)     /*!< CoreDebug DHCSR: C_DEBUGEN Mask */

/* Debug Core Register Selector Register Definitions */
#define CoreDebug_DCRSR_REGWnR_Pos         16U                                            /*!< CoreDebug DCRSR: REGWnR Position */
#define CoreDebug_DCRSR_REGWnR_Msk         (1UL << CoreDebug_DCRSR_REGWnR_Pos)            /*!< CoreDebug DCRSR: REGWnR Mask */

#define CoreDebug_DCRSR_REGSEL_Pos          0U                                            /*!< CoreDebug DCRSR: REGSEL Position */
#define CoreDebug_DCRSR_REGSEL_Msk         (0x1FUL)     /*!< CoreDebug DCRSR: REGSEL Mask */

/* Debug Exception and Monitor Control Register Definitions */
#define CoreDebug_DEMCR_TRCENA_Pos         24U                                            /*!< CoreDebug DEMCR: TRCENA Position */
#define CoreDebug_DEMCR_TRCENA_Msk         (1UL << CoreDebug_DEMCR_TRCENA_Pos)            /*!< CoreDebug DEMCR: TRCENA Mask */

#define CoreDebug_DEMCR_MON_REQ_Pos        19U                                            /*!< CoreDebug DEMCR: MON_REQ Position */
#define CoreDebug_DEMCR_MON_REQ_Msk        (1UL << CoreDebug_DEMCR_MON_REQ_Pos)           /*!< CoreDebug DEMCR: MON_REQ Mask */

#define CoreDebug_DEMCR_MON_STEP_Pos       18U                                            /*!< CoreDebug DEMCR: MON_STEP Position */
#define CoreDebug_DEMCR_MON_STEP_Msk       (1UL << CoreDebug_DEMCR_MON_STEP_Pos)          /*!< CoreDebug DEMCR: MON_STEP Mask */

#define CoreDebug_DEMCR_MON_PEND_Pos       17U                                            /*!< CoreDebug DEMCR: MON_PEND Position */
#define CoreDebug_DEMCR_MON_PEND_Msk       (1UL << CoreDebug_DEMCR_MON_PEND_Pos)          /*!< CoreDebug DEMCR: MON_PEND Mask */

#define CoreDebug_DEMCR_MON_EN_Pos         16U                                            /*!< CoreDebug DEMCR: MON_EN Position */
#define CoreDebug_DEMCR_MON_EN_Msk         (1UL << CoreDebug_DEMCR_MON_EN_Pos)            /*!< CoreDebug DEMCR: MON_EN Mask */

#define CoreDebug_DEMCR_VC_HARDERR_Pos     10U                                            /*!< CoreDebug DEMCR: VC_HARDERR Position */
#define CoreDebug_DEMCR_VC_HARDERR_Msk     (1UL << CoreDebug_DEMCR_VC_HARDERR_Pos)        /*!< CoreDebug DEMCR: VC_HARDERR Mask */

#define CoreDebug_DEMCR_VC_INTERR_Pos       9U                                            /*!< CoreDebug DEMCR: VC_INTERR Position */
#define CoreDebug_DEMCR_VC_INTERR_Msk      (1UL << CoreDebug_DEMCR_VC_INTERR_Pos)         /*!< CoreDebug DEMCR: VC_INTERR Mask */

#define CoreDebug_DEMCR_VC_BUSERR_Pos       8U                                            /*!< CoreDebug DEMCR: VC_BUSERR Position */
#define CoreDebug_DEMCR_VC_BUSERR_Msk      (1UL << CoreDebug_DEMCR_VC_BUSERR_Pos)         /*!< CoreDebug DEMCR: VC_BUSERR Mask */

#define CoreDebug_DEMCR_VC_STATERR_Pos      7U                                            /*!< CoreDebug DEMCR: VC_STATERR Position */
#define CoreDebug_DEMCR_VC_STATERR_Msk     (1UL << CoreDebug_DEMCR_VC_STATERR_Pos)        /*!< CoreDebug DEMCR: VC_STATERR Mask */

#define CoreDebug_DEMCR_VC_CHKERR_Pos       6U                                            /*!< CoreDebug DEMCR: VC_CHKERR Position */
#define CoreDebug_DEMCR_VC_CHKERR_Msk      (1UL << CoreDebug_DEMCR_VC_CHKERR_Pos)         /*!< CoreDebug DEMCR: VC_CHKERR Mask */

#define CoreDebug_DEMCR_VC_NOCPERR_Pos      5U                                            /*!< CoreDebug DEMCR: VC_NOCPERR Position */
#define CoreDebug_DEMCR_VC_NOCPERR_Msk     (1UL << CoreDebug_DEMCR_VC_NOCPERR_Pos)        /*!< CoreDebug DEMCR: VC_NOCPERR Mask */

#define CoreDebug_DEMCR_VC_MMERR_Pos        4U                                            /*!< CoreDebug DEMCR: VC_MMERR Position */
#define CoreDebug_DEMCR_VC_MMERR_Msk       (1UL << CoreDebug_DEMCR_VC_MMERR_Pos)          /*!< CoreDebug DEMCR: VC_MMERR Mask */

#define CoreDebug_DEMCR_VC_CORERESET_Pos    0U                                            /*!< CoreDebug DEMCR: VC_CORERESET Position */
#define CoreDebug_DEMCR_VC_CORERESET_Msk   (1UL)  /*!< CoreDebug DEMCR: VC_CORERESET Mask */

/*@} end of group CMSIS_CoreDebug */

/**
  \ingroup    CMSIS_core_register
  \defgroup   CMSIS_core_base     Core Definitions
  \brief      Definitions for base addresses, unions, and structures.
  @{
 */

/* Memory mapping of Core Hardware */
#define SCS_BASE            (0xE000E000U)                            /*!< System Control Space Base Address */
#define ITM_BASE            (0xE0000000U)                            /*!< ITM Base Address */
#define DWT_BASE            (0xE0001000U)                            /*!< DWT Base Address */
#define TPI_BASE            (0xE0040000U)                            /*!< TPI Base Address */
#define CoreDebug_BASE      (0xE000EDF0U)                            /*!< Core Debug Base Address */
#define SysTick_BASE        (SCS_BASE +  0x0010U)                    /*!< SysTick Base Address */
#define NVIC_BASE           (SCS_BASE +  0x0100U)                    /*!< NVIC Base Address */
#define SCB_BASE            (SCS_BASE +  0x0D00U)                    /*!< System Control Block Base Address */

#define SCnSCB              ((SCnSCB_Type *)SCS_BASE)            /*!< System control Register not in SCB */
#define SCB                 ((SCB_Type *)SCB_BASE)               /*!< SCB configuration struct */
#define SysTick             ((SysTick_Type *)SysTick_BASE)       /*!< SysTick configuration struct */
#define NVIC                ((NVIC_Type *)NVIC_BASE)             /*!< NVIC configuration struct */
#define ITM                 ((ITM_Type *)ITM_BASE)               /*!< ITM configuration struct */
#define DWT                 ((DWT_Type *)DWT_BASE)               /*!< DWT configuration struct */
#define TPI                 ((TPI_Type *)TPI_BASE)               /*!< TPI configuration struct */
#define CoreDebug           ((CoreDebug_Type *)CoreDebug_BASE)   /*!< Core Debug configuration struct */

#if defined (__MPU_PRESENT) && (__MPU_PRESENT == 1U)
    #define MPU_BASE          (SCS_BASE +  0x0D90U)                    /*!< Memory Protection Unit */
    #define MPU               ((MPU_Type *)MPU_BASE)   /*!< Memory Protection Unit */
#endif

#define FPU_BASE            (SCS_BASE +  0x0F30U)                    /*!< Floating Point Unit */
#define FPU                 ((FPU_Type *)FPU_BASE)                  /*!< Floating Point Unit */

/*******************************************************************************
 *                Hardware Abstraction Layer
  Core Function Interface contains:
  - Core NVIC Functions
  - Core SysTick Functions
  - Core Debug Functions
  - Core Register Access Functions
 ******************************************************************************/
/**
  \defgroup CMSIS_Core_FunctionInterface Functions and Instructions Reference
*/

/* ###########################  Core Function Access  ########################### */
/**
  \ingroup  CMSIS_Core_FunctionInterface
  \defgroup CMSIS_Core_RegAccFunctions CMSIS Core Register Access Functions
  \brief    Functions that manage system registers.
  @{
 */
#if ! defined (COMPILER_ARM_CC)
/**
  * @brief   Enable IRQ Interrupts
  * @details Enables IRQ interrupts by clearing the I-bit in the CPSR. Can only be executed in Privileged modes.
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE void __enable_irq(void)
{
#if defined (COMPILER_IAR) || defined(COMPILER_GHS)
    ASMV_KEYWORD("cpsie i"); //PRQA S 1006 # AMCE_SYS_1006
#else
    ASMV_KEYWORD("cpsie i" : : : "memory"); //PRQA S 1006 # AMCE_SYS_1006
#endif
}
/*PRQA S 3006,3008 -- */

/**
  * @brief   Disable IRQ Interrupts
  * @details Disables IRQ interrupts by setting the I-bit in the CPSR. Can only be executed in Privileged modes.
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE void __disable_irq(void)
{
#if defined (COMPILER_IAR) || defined(COMPILER_GHS)
    ASMV_KEYWORD("cpsid i"); //PRQA S 1006 # AMCE_SYS_1006
#else
    ASMV_KEYWORD("cpsid i" : : : "memory"); //PRQA S 1006 # AMCE_SYS_1006
#endif
}
#endif
/*PRQA S 3006,3008 -- */

/**
 * @brief   Get Control Register
 * details Returns the content of the Control Register.
 * return               Control Register value
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE uint32 __get_CONTROL(void)
{
#if defined (COMPILER_ARM_CC)
    register uint32 __regControl __asm("control"); //PRQA S 1006 # AMCE_SYS_1006
    return (__regControl);
#else
    uint32 result;

    ASMV_KEYWORD("MRS %0, control" : "=r"(result)); //PRQA S 1006 # AMCE_SYS_1006
    return (result);
#endif
}
/*PRQA S 3006,3008 -- */

/**
  \brief   Set Control Register
  \details Writes the given value to the Control Register.
  \param [in]    control  Control Register value to set
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE void __set_CONTROL(uint32 control)
{
#if defined (COMPILER_ARM_CC)
    register uint32 __regControl __asm("control"); //PRQA S 1006 # AMCE_SYS_1006
    __regControl = control;
#else
    (void) control; /* avoid misra warning. */
    ASMV_KEYWORD("MSR control, %0" : : "r"(control) : "memory"); //PRQA S 1006 # AMCE_SYS_1006
#endif
}
/*PRQA S 3006,3008 -- */

/**
  \brief   Get IPSR Register
  \details Returns the content of the IPSR Register.
  \return               IPSR Register value
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE uint32 __get_IPSR(void)
{
#if defined (COMPILER_ARM_CC)
    register uint32 __regIPSR __asm("ipsr"); //PRQA S 1006 # AMCE_SYS_1006
    return (__regIPSR);
#else
    uint32 result;

    ASMV_KEYWORD("MRS %0, ipsr" : "=r"(result)); //PRQA S 1006 # AMCE_SYS_1006
    return (result);
#endif
}
/*PRQA S 3006,3008 -- */

/**
  \brief   Get APSR Register
  \details Returns the content of the APSR Register.
  \return               APSR Register value
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE uint32 __get_APSR(void)
{
#if defined (COMPILER_ARM_CC)
    register uint32 __regAPSR __asm("apsr"); //PRQA S 1006 # AMCE_SYS_1006
    return (__regAPSR);
#else
    uint32 result;

    ASMV_KEYWORD("MRS %0, apsr" : "=r"(result)); //PRQA S 1006 # AMCE_SYS_1006
    return (result);
#endif
}
/*PRQA S 3006,3008 -- */

/**
  \brief   Get xPSR Register
  \details Returns the content of the xPSR Register.
  \return               xPSR Register value
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE uint32 __get_xPSR(void)
{
#if defined (COMPILER_ARM_CC)
    register uint32 __regXPSR __asm("xpsr"); //PRQA S 1006 # AMCE_SYS_1006
    return (__regXPSR);
#else
    uint32 result;

    ASMV_KEYWORD("MRS %0, xpsr" : "=r"(result)); //PRQA S 1006 # AMCE_SYS_1006
    return (result);
#endif
}
/*PRQA S 3006,3008 -- */

/**
  \brief   Get Process Stack Pointer
  \details Returns the current value of the Process Stack Pointer (PSP).
  \return               PSP Register value
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE uint32 __get_PSP(void)
{
#if defined (COMPILER_ARM_CC)
    register uint32 __regProcessStackPointer __asm("psp"); //PRQA S 1006 # AMCE_SYS_1006
    return (__regProcessStackPointer);
#else
    register uint32 result;

    ASMV_KEYWORD("MRS %0, psp" : "=r"(result)); //PRQA S 1006 # AMCE_SYS_1006
    return (result);
#endif
}
/*PRQA S 3006,3008 -- */

/**
  \brief   Set Process Stack Pointer
  \details Assigns the given value to the Process Stack Pointer (PSP).
  \param [in]    topOfProcStack  Process Stack Pointer value to set
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE void __set_PSP(uint32 topOfProcStack)
{
#if defined (COMPILER_ARM_CC)
    register uint32 __regProcessStackPointer __asm("psp"); //PRQA S 1006 # AMCE_SYS_1006
    __regProcessStackPointer = topOfProcStack;
#else
    (void) topOfProcStack; /* avoid misra warning. */
    ASMV_KEYWORD("MSR psp, %0" : : "r"(topOfProcStack) :); //PRQA S 1006 # AMCE_SYS_1006
#endif
}
/*PRQA S 3006,3008 -- */

/**
  \brief   Get Main Stack Pointer
  \details Returns the current value of the Main Stack Pointer (MSP).
  \return               MSP Register value
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE uint32 __get_MSP(void)
{
#if defined (COMPILER_ARM_CC)
    register uint32 __regMainStackPointer __asm("msp"); //PRQA S 1006 # AMCE_SYS_1006
    return (__regMainStackPointer);
#else
    register uint32 result;

    ASMV_KEYWORD("MRS %0, msp" : "=r"(result)); //PRQA S 1006 # AMCE_SYS_1006
    return (result);
#endif
}
/*PRQA S 3006,3008 -- */

/**
  \brief   Set Main Stack Pointer
  \details Assigns the given value to the Main Stack Pointer (MSP).
  \param [in]    topOfMainStack  Main Stack Pointer value to set
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE void __set_MSP(uint32 topOfMainStack)
{
#if defined (COMPILER_ARM_CC)
    register uint32 __regMainStackPointer __asm("msp"); //PRQA S 1006 # AMCE_SYS_1006
    __regMainStackPointer = topOfMainStack;
#else
    (void) topOfMainStack; /* avoid misra warning. */
    ASMV_KEYWORD("MSR msp, %0" : : "r"(topOfMainStack) :); //PRQA S 1006 # AMCE_SYS_1006
#endif
}
/*PRQA S 3006,3008 -- */

/**
  \brief   Get Priority Mask
  \details Returns the current state of the priority mask bit from the Priority Mask Register.
  \return               Priority Mask value
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE uint32 __get_PRIMASK(void)
{
#if defined (COMPILER_ARM_CC)
    register uint32 __regPriMask __asm("primask"); //PRQA S 1006 # AMCE_SYS_1006
    return (__regPriMask);
#else
    uint32 result;

    ASMV_KEYWORD("MRS %0, primask" : "=r"(result) :: "memory"); //PRQA S 1006 # AMCE_SYS_1006
    return (result);
#endif
}
/*PRQA S 3006,3008 -- */

/**
  \brief   Set Priority Mask
  \details Assigns the given value to the Priority Mask Register.
  \param [in]    priMask  Priority Mask
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE void __set_PRIMASK(uint32 priMask)
{
#if defined (COMPILER_ARM_CC)
    register uint32 __regPriMask __asm("primask"); //PRQA S 1006 # AMCE_SYS_1006
    __regPriMask = (priMask);
#else
    (void) priMask; /* avoid misra warning. */
    ASMV_KEYWORD("MSR primask, %0" : : "r"(priMask) : "memory"); //PRQA S 1006 # AMCE_SYS_1006
#endif
}
/*PRQA S 3008 -- */

/**
  \brief   Enable FIQ
  \details Enables FIQ interrupts by clearing the F-bit in the CPSR.
           Can only be executed in Privileged modes.
 */
/*PRQA S 3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE void __enable_fault_irq(void)
{
#if defined (COMPILER_ARM_CC) || defined (COMPILER_IAR)
    ASMV_KEYWORD("cpsie f"); //PRQA S 1006 # AMCE_SYS_1006
#else
    ASMV_KEYWORD("cpsie f" : : : "memory"); //PRQA S 1006 # AMCE_SYS_1006
#endif
}
/*PRQA S 3008 -- */

/**
  \brief   Disable FIQ
  \details Disables FIQ interrupts by setting the F-bit in the CPSR.
           Can only be executed in Privileged modes.
 */
/*PRQA S 3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE void __disable_fault_irq(void)
{
#if defined (COMPILER_ARM_CC) || defined (COMPILER_IAR) || defined(COMPILER_GHS)
    ASMV_KEYWORD("cpsid f"); //PRQA S 1006 # AMCE_SYS_1006
#else
    ASMV_KEYWORD("cpsid f" : : : "memory"); //PRQA S 1006 # AMCE_SYS_1006
#endif
}
/*PRQA S 3006,3008 -- */

/**
  \brief   Get Base Priority
  \details Returns the current value of the Base Priority register.
  \return               Base Priority register value
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE uint32 __get_BASEPRI(void)
{
#if defined (COMPILER_ARM_CC)
    register uint32 __regBasePri __asm("basepri"); //PRQA S 1006 # AMCE_SYS_1006
    return (__regBasePri);
#else
    uint32 result;

    ASMV_KEYWORD("MRS %0, basepri" : "=r"(result)); //PRQA S 1006 # AMCE_SYS_1006
    return (result);
#endif
}
/*PRQA S 3006,3008 -- */

/**
  \brief   Set Base Priority
  \details Assigns the given value to the Base Priority register.
  \param [in]    basePri  Base Priority value to set
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE void __set_BASEPRI(uint32 basePri)
{
#if defined (COMPILER_ARM_CC)
    register uint32 __regBasePri __asm("basepri"); //PRQA S 1006 # AMCE_SYS_1006
    __regBasePri = (basePri & 0xFFU);
#else
    (void) basePri; /* avoid misra warning. */
    ASMV_KEYWORD("MSR basepri, %0" : : "r"(basePri) : "memory"); //PRQA S 1006 # AMCE_SYS_1006
#endif
}
/*PRQA S 3006,3008 -- */

/**
  \brief   Get Fault Mask
  \details Returns the current value of the Fault Mask register.
  \return               Fault Mask register value
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE uint32 __get_FAULTMASK(void)
{
#if defined (COMPILER_ARM_CC)
    register uint32 __regFaultMask __asm("faultmask"); //PRQA S 1006 # AMCE_SYS_1006
    return (__regFaultMask);
#else
    uint32 result;

    ASMV_KEYWORD("MRS %0, faultmask" : "=r"(result)); //PRQA S 1006 # AMCE_SYS_1006
    return (result);
#endif
}
/*PRQA S 3006,3008 -- */

/**
  \brief   Set Fault Mask
  \details Assigns the given value to the Fault Mask register.
  \param [in]    faultMask  Fault Mask value to set
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE void __set_FAULTMASK(uint32 faultMask)
{
#if defined (COMPILER_ARM_CC)
    register uint32 __regFaultMask __asm("faultmask"); //PRQA S 1006 # AMCE_SYS_1006
    __regFaultMask = (faultMask & (uint32)1U);
#else
    (void) faultMask; /* avoid misra warning. */
    ASMV_KEYWORD("MSR faultmask, %0" : : "r"(faultMask) : "memory"); //PRQA S 1006 # AMCE_SYS_1006
#endif
}
/*PRQA S 3006,3008 -- */

/**
  \brief   Get FPSCR
  \details Returns the current value of the Floating Point Status/Control register.
  \return               Floating Point Status/Control register value
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE uint32 __get_FPSCR(void)
{
#if ((defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
     (defined (__FPU_USED) && (__FPU_USED == 1U)))
#if defined (COMPILER_ARM_CC)
    register uint32 __regfpscr __asm("fpscr"); //PRQA S 1006 # AMCE_SYS_1006
    return (__regfpscr);
#else
    register uint32 result;
    ASMV_KEYWORD("VMRS %0, fpscr" : "=r"(result)); //PRQA S 1006 # AMCE_SYS_1006
    return (result);
#endif
#else
    return (0U);
#endif
}
/*PRQA S 3006,3008 -- */

/**
  \brief   Set FPSCR
  \details Assigns the given value to the Floating Point Status/Control register.
  \param [in]    fpscr  Floating Point Status/Control value to set
 */
/*PRQA S 3006,3008 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE void __set_FPSCR(uint32 fpscr)
{
#if ((defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
     (defined (__FPU_USED) && (__FPU_USED == 1U)))
#if defined (COMPILER_ARM_CC)
    register uint32 __regfpscr __asm("fpscr");
    __regfpscr = (fpscr);
#else
    (void) fpscr; /* avoid misra warning. */
    ASMV_KEYWORD("VMSR fpscr, %0" : : "r"(fpscr) : "memory"); //PRQA S 1006 # AMCE_SYS_1006
#endif
#else
    (void)fpscr;
#endif
}
/*PRQA S 3006,3008 -- */

/*@} end of CMSIS_Core_RegAccFunctions */

/* ##########################   NVIC functions  #################################### */
/**
  \ingroup  CMSIS_Core_FunctionInterface
  \defgroup CMSIS_Core_NVICFunctions NVIC Functions
  \brief    Functions that manage interrupts and exceptions via the NVIC.
  @{
 */

#ifdef CMSIS_NVIC_VIRTUAL
    #ifndef CMSIS_NVIC_VIRTUAL_HEADER_FILE
        #define CMSIS_NVIC_VIRTUAL_HEADER_FILE "cmsis_nvic_virtual.h"
    #endif
#include CMSIS_NVIC_VIRTUAL_HEADER_FILE
#else
    #define NVIC_SetPriorityGrouping    __NVIC_SetPriorityGrouping
    #define NVIC_GetPriorityGrouping    __NVIC_GetPriorityGrouping
    #define NVIC_EnableIRQ              __NVIC_EnableIRQ
    #define NVIC_GetEnableIRQ           __NVIC_GetEnableIRQ
    #define NVIC_DisableIRQ             __NVIC_DisableIRQ
    #define NVIC_GetPendingIRQ          __NVIC_GetPendingIRQ
    #define NVIC_SetPendingIRQ          __NVIC_SetPendingIRQ
    #define NVIC_ClearPendingIRQ        __NVIC_ClearPendingIRQ
    #define NVIC_GetActive              __NVIC_GetActive
    #define NVIC_SetPriority            __NVIC_SetPriority
    #define NVIC_GetPriority            __NVIC_GetPriority
    #define NVIC_SystemReset            __NVIC_SystemReset
#endif /* CMSIS_NVIC_VIRTUAL */

#ifdef CMSIS_VECTAB_VIRTUAL
    #ifndef CMSIS_VECTAB_VIRTUAL_HEADER_FILE
        #define CMSIS_VECTAB_VIRTUAL_HEADER_FILE "cmsis_vectab_virtual.h"
    #endif
#include CMSIS_VECTAB_VIRTUAL_HEADER_FILE
#else
     #define NVIC_SetVector              __NVIC_SetVector
     #define NVIC_GetVector              __NVIC_GetVector
#endif  /* (CMSIS_VECTAB_VIRTUAL) */

#define NVIC_USER_IRQ_OFFSET          16

/* The following EXC_RETURN values are saved the LR on exception entry */
#define EXC_RETURN_HANDLER         (0xFFFFFFF1U)     /* return to Handler mode, uses MSP after return                               */
#define EXC_RETURN_THREAD_MSP      (0xFFFFFFF9U)     /* return to Thread mode, uses MSP after return                                */
#define EXC_RETURN_THREAD_PSP      (0xFFFFFFFDU)     /* return to Thread mode, uses PSP after return                                */
#define EXC_RETURN_HANDLER_FPU     (0xFFFFFFE1U)     /* return to Handler mode, uses MSP after return, restore floating-point state */
#define EXC_RETURN_THREAD_MSP_FPU  (0xFFFFFFE9U)     /* return to Thread mode, uses MSP after return, restore floating-point state  */
#define EXC_RETURN_THREAD_PSP_FPU  (0xFFFFFFEDU)     /* return to Thread mode, uses PSP after return, restore floating-point state  */

/**
  \brief   Set Priority Grouping
  \details Sets the priority grouping field using the required unlock sequence.
           The parameter PriorityGroup is assigned to the field SCB->AIRCR [10:8] PRIGROUP field.
           Only values from 0..7 are used.
           In case of a conflict between priority grouping and available
           priority bits (__NVIC_PRIO_BITS), the smallest possible priority group is set.
  \param [in]      PriorityGroup  Priority grouping field.
 */
LOCAL_INLINE void __NVIC_SetPriorityGrouping(uint32 PriorityGroup)
{
    uint32 reg_value;
    uint32 PriorityGroupTmp = (PriorityGroup & (uint32)0x07U); /* only values 0..7 are used          */

    reg_value = SCB->AIRCR; /* read old register configuration    */
    reg_value &= ~((uint32)SCB_AIRCR_VECTKEY_Msk | (uint32)SCB_AIRCR_PRIGROUP_Msk); /* clear bits to change               */
    reg_value = (reg_value |
                 ((uint32)0x5FAU << SCB_AIRCR_VECTKEY_Pos) |
                 (PriorityGroupTmp << SCB_AIRCR_PRIGROUP_Pos)); /* Insert write key and priority group */
    SCB->AIRCR = reg_value;
}

/**
  \brief   Get Priority Grouping
  \details Reads the priority grouping field from the NVIC Interrupt Controller.
  \return                Priority grouping field (SCB->AIRCR [10:8] PRIGROUP field).
 */
LOCAL_INLINE uint32 __NVIC_GetPriorityGrouping(void)
{
    return ((((uint32)SCB->AIRCR & (uint32)SCB_AIRCR_PRIGROUP_Msk) >> SCB_AIRCR_PRIGROUP_Pos));
}

/**
  \brief   Enable Interrupt
  \details Enables a device specific interrupt in the NVIC interrupt controller.
  \param [in]      IRQn  Device specific interrupt number.
  \note    IRQn must not be negative.
 */
LOCAL_INLINE void __NVIC_EnableIRQ(IRQn_Type IRQn)
{
    if ((sint32)(IRQn) >= 0)
    {
        NVIC->ISER[(((uint32)IRQn) >> 5U)] = (uint32)(1UL << (((uint32)IRQn) & 0x1FU));
    }
}

/**
  \brief   Get Interrupt Enable status
  \details Returns a device specific interrupt enable status from the NVIC interrupt controller.
  \param [in]      IRQn  Device specific interrupt number.
  \return             0  Interrupt is not enabled.
  \return             1  Interrupt is enabled.
  \note    IRQn must not be negative.
 */
LOCAL_INLINE uint32 __NVIC_GetEnableIRQ(IRQn_Type IRQn)
{
    if ((sint32)(IRQn) >= 0)
    {
        return (((NVIC->ISER[(((uint32)IRQn) >> 5U)] & (1UL << (((uint32)IRQn) & 0x1FU))) != 0U) ? 1U : 0U);
    }
    else
    {
        return (0U);
    }
}

/**
  \brief   Disable Interrupt
  \details Disables a device specific interrupt in the NVIC interrupt controller.
  \param [in]      IRQn  Device specific interrupt number.
  \note    IRQn must not be negative.
 */
/*PRQA S 3006 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE void __NVIC_DisableIRQ(IRQn_Type IRQn)
{
    if ((sint32)(IRQn) >= 0)
    {
        NVIC->ICER[(((uint32)IRQn) >> 5U)] = (uint32)(1UL << (((uint32)IRQn) & 0x1FU));
        __DSB(); //PRQA S 1006 # AMCE_SYS_1006
        __ISB(); //PRQA S 1006 # AMCE_SYS_1006
    }
}
/*PRQA S 3006 -- */

/**
  \brief   Get Pending Interrupt
  \details Reads the NVIC pending register and returns the pending bit for the specified device specific interrupt.
  \param [in]      IRQn  Device specific interrupt number.
  \return             0  Interrupt status is not pending.
  \return             1  Interrupt status is pending.
  \note    IRQn must not be negative.
 */
LOCAL_INLINE uint32 __NVIC_GetPendingIRQ(IRQn_Type IRQn)
{
    if ((sint32)(IRQn) >= 0)
    {
        return (((NVIC->ISPR[(((uint32)IRQn) >> 5U)] & (1UL << (((uint32)IRQn) & 0x1FU))) != 0U) ? 1U : 0U);
    }
    else
    {
        return (0U);
    }
}

/**
  \brief   Set Pending Interrupt
  \details Sets the pending bit of a device specific interrupt in the NVIC pending register.
  \param [in]      IRQn  Device specific interrupt number.
  \note    IRQn must not be negative.
 */
LOCAL_INLINE void __NVIC_SetPendingIRQ(IRQn_Type IRQn)
{
    if ((sint32)(IRQn) >= 0)
    {
        NVIC->ISPR[(((uint32)IRQn) >> 5U)] = (uint32)(1UL << (((uint32)IRQn) & 0x1FU));
    }
}

/**
  \brief   Clear Pending Interrupt
  \details Clears the pending bit of a device specific interrupt in the NVIC pending register.
  \param [in]      IRQn  Device specific interrupt number.
  \note    IRQn must not be negative.
 */
LOCAL_INLINE void __NVIC_ClearPendingIRQ(IRQn_Type IRQn)
{
    if ((sint32)(IRQn) >= 0)
    {
        NVIC->ICPR[(((uint32)IRQn) >> 5U)] = (uint32)(1UL << (((uint32)IRQn) & 0x1FU));
    }
}

/**
  \brief   Get Active Interrupt
  \details Reads the active register in the NVIC and returns the active bit for the device specific interrupt.
  \param [in]      IRQn  Device specific interrupt number.
  \return             0  Interrupt status is not active.
  \return             1  Interrupt status is active.
  \note    IRQn must not be negative.
 */
LOCAL_INLINE uint32 __NVIC_GetActive(IRQn_Type IRQn)
{
    if ((sint32)(IRQn) >= 0)
    {
        return (((NVIC->IABR[(((uint32)IRQn) >> 5U)] & (1UL << (((uint32)IRQn) & 0x1FU))) != 0U) ? 1U : 0U);
    }
    else
    {
        return (0U);
    }
}

/**
  \brief   Set Interrupt Priority
  \details Sets the priority of a device specific interrupt or a processor exception.
           The interrupt number can be positive to specify a device specific interrupt,
           or negative to specify a processor exception.
  \param [in]      IRQn  Interrupt number.
  \param [in]  priority  Priority to set.
  \note    The priority cannot be set for every processor exception.
 */
LOCAL_INLINE void __NVIC_SetPriority(IRQn_Type IRQn, uint32 priority)
{
    if ((sint32)(IRQn) >= 0)
    {
        NVIC->IP[((uint32)IRQn)] = (uint8)((priority << (8U - __NVIC_PRIO_BITS)) & (uint32)0xFFU);
    }
    else
    {
        SCB->SHP[(((uint32)IRQn) & 0xFU) - 4U] = (uint8)((priority << (8U - __NVIC_PRIO_BITS)) & (uint32)0xFFU);
    }
}

/**
  \brief   Get Interrupt Priority
  \details Reads the priority of a device specific interrupt or a processor exception.
           The interrupt number can be positive to specify a device specific interrupt,
           or negative to specify a processor exception.
  \param [in]   IRQn  Interrupt number.
  \return             Interrupt Priority.
                      Value is aligned automatically to the implemented priority bits of the microcontroller.
 */
LOCAL_INLINE uint32 __NVIC_GetPriority(IRQn_Type IRQn)
{

    if ((sint32)(IRQn) >= 0)
    {
        return (((uint32)NVIC->IP[((uint32)IRQn)] >> (8U - __NVIC_PRIO_BITS)));
    }
    else
    {
        return (((uint32)SCB->SHP[(((uint32)IRQn) & 0xFU) - 4U] >> (8U - __NVIC_PRIO_BITS)));
    }
}

/**
  \brief   Encode Priority
  \details Encodes the priority for an interrupt with the given priority group,
           preemptive priority value, and subpriority value.
           In case of a conflict between priority grouping and available
           priority bits (__NVIC_PRIO_BITS), the smallest possible priority group is set.
  \param [in]     PriorityGroup  Used priority group.
  \param [in]   PreemptPriority  Preemptive priority value (starting from 0).
  \param [in]       SubPriority  Subpriority value (starting from 0).
  \return                        Encoded priority. Value can be used in the function \ref NVIC_SetPriority().
 */
LOCAL_INLINE uint32 NVIC_EncodePriority(uint32 PriorityGroup, uint32 PreemptPriority, uint32 SubPriority)
{
    uint32 PriorityGroupTmp = (PriorityGroup & (uint32)0x07U); /* only values 0..7 are used          */
    uint32 PreemptPriorityBits;
    uint32 SubPriorityBits;

    PreemptPriorityBits = ((7U - PriorityGroupTmp) > (uint32)(__NVIC_PRIO_BITS)) \
                          ? (uint32)(__NVIC_PRIO_BITS) : (uint32)(7U - PriorityGroupTmp);
    SubPriorityBits = ((PriorityGroupTmp + (uint32)(__NVIC_PRIO_BITS)) < (uint32)7U) \
                      ? (uint32)0U : (uint32)((PriorityGroupTmp - 7U) + (uint32)(__NVIC_PRIO_BITS));

    return (((PreemptPriority & (uint32)((1UL << (PreemptPriorityBits)) - 1U)) << SubPriorityBits) |
               ((SubPriority & (uint32)((1UL << (SubPriorityBits)) - 1U))));
}

/**
  \brief   Decode Priority
  \details Decodes an interrupt priority value with a given priority group to
           preemptive priority value and subpriority value.
           In case of a conflict between priority grouping and available
           priority bits (__NVIC_PRIO_BITS) the smallest possible priority group is set.
  \param [in]         Priority   Priority value, which can be retrieved with the function \ref NVIC_GetPriority().
  \param [in]     PriorityGroup  Used priority group.
  \param [out] pPreemptPriority  Preemptive priority value (starting from 0).
  \param [out]     pSubPriority  Subpriority value (starting from 0).
 */
LOCAL_INLINE void NVIC_DecodePriority
(
    uint32 Priority,
    uint32 PriorityGroup,
    uint32 *const pPreemptPriority,
    uint32 *const pSubPriority
)
{
    uint32 PriorityGroupTmp = (PriorityGroup & (uint32)0x07U); /* only values 0..7 are used          */
    uint32 PreemptPriorityBits;
    uint32 SubPriorityBits;

    PreemptPriorityBits = ((7U - PriorityGroupTmp) > (uint32)(__NVIC_PRIO_BITS)) \
                          ? (uint32)(__NVIC_PRIO_BITS) : (uint32)(7U - PriorityGroupTmp);
    SubPriorityBits = ((PriorityGroupTmp + (uint32)(__NVIC_PRIO_BITS)) < (uint32)7U) \
                      ? (uint32)0U : (uint32)((PriorityGroupTmp - 7U) + (uint32)(__NVIC_PRIO_BITS));

    *pPreemptPriority = (Priority >> SubPriorityBits) & (uint32)((1UL << (PreemptPriorityBits)) - 1U);
    *pSubPriority = (Priority) & (uint32)((1UL << (SubPriorityBits)) - 1U);
}

/**
  \brief   Set Interrupt Vector
  \details Sets an interrupt vector in SRAM based interrupt vector table.
           The interrupt number can be positive to specify a device specific interrupt,
           or negative to specify a processor exception.
           VTOR must been relocated to SRAM before.
  \param [in]   IRQn      Interrupt number
  \param [in]   vector    Address of interrupt handler function
 */
LOCAL_INLINE void __NVIC_SetVector(IRQn_Type IRQn, uint32 vector)
{
    uint32 *vectors = (uint32 *)SCB->VTOR;
    vectors[(sint32)IRQn + NVIC_USER_IRQ_OFFSET] = vector;
    /* ARM Application Note 321 states that the M4 does not require the architectural barrier */
}

/**
  \brief   Get Interrupt Vector
  \details Reads an interrupt vector from interrupt vector table.
           The interrupt number can be positive to specify a device specific interrupt,
           or negative to specify a processor exception.
  \param [in]   IRQn      Interrupt number.
  \return                 Address of interrupt handler function
 */
LOCAL_INLINE uint32 __NVIC_GetVector(IRQn_Type IRQn)
{
    uint32 const *vectors = (uint32 *)SCB->VTOR;
    return vectors[(sint32)IRQn + NVIC_USER_IRQ_OFFSET];
}

/**
  \brief   System Reset
  \details Initiates a system reset request to reset the MCU.
 */
/*PRQA S 3006 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE void __NVIC_SystemReset(void)
{
    /* Ensure all outstanding memory accesses included buffered write are completed before reset */
    __DSB(); //PRQA S 1006 # AMCE_SYS_1006

    SCB->AIRCR = (uint32)((0x5FAUL << SCB_AIRCR_VECTKEY_Pos) |
                          (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
                          SCB_AIRCR_SYSRESETREQ_Msk); /* Keep priority group unchanged */
    __DSB(); /* Ensure completion of memory access */ //PRQA S 1006 # AMCE_SYS_1006

    for (;;) /* wait until reset */
    {
        __NOP(); //PRQA S 1006 # AMCE_SYS_1006
    }
}
/*PRQA S 3006 -- */

/*@} end of CMSIS_Core_NVICFunctions */

/* ##########################  MPU functions  #################################### */

#if defined (__MPU_PRESENT) && (__MPU_PRESENT == 1U)

#include "Mpu_Hal.h"

#endif

/* ##########################  FPU functions  #################################### */
/**
  \ingroup  CMSIS_Core_FunctionInterface
  \defgroup CMSIS_Core_FpuFunctions FPU Functions
  \brief    Function that provides FPU type.
  @{
 */

/**
  \brief   get FPU type
  \details returns the FPU type
  \returns
   - \b  0: No FPU
   - \b  1: Single precision FPU
   - \b  2: Double + Single precision FPU
 */
LOCAL_INLINE uint32 SCB_GetFPUType(void)
{
    uint32 mvfr0;

    mvfr0 = FPU->MVFR0;
    if ((mvfr0 & (FPU_MVFR0_Single_precision_Msk | FPU_MVFR0_Double_precision_Msk)) == 0x020U)
    {
        return 1U; /* Single precision FPU */
    }
    else
    {
        return 0U; /* No FPU */
    }
}

LOCAL_INLINE void FPU_Enable(void)
{
    SCB->CPACR |= 0x00F00000U;
}

LOCAL_INLINE void FPU_Disable(void)
{
    SCB->CPACR &= 0xFF0FFFFFU;
}

/*@} end of CMSIS_Core_FpuFunctions */

/* ##################################    SysTick function  ############################################ */
/**
  \ingroup  CMSIS_Core_FunctionInterface
  \defgroup CMSIS_Core_SysTickFunctions SysTick Functions
  \brief    Functions that configure the System.
  @{
 */

#if defined (__Vendor_SysTickConfig) && (__Vendor_SysTickConfig == 0U)

/**
  \brief   System Tick Configuration
  \details Initializes the System Timer and its interrupt, and starts the System Tick Timer.
           Counter is in free running mode to generate periodic interrupts.
  \param [in]  ticks  Number of ticks between two interrupts.
  \return          0  Function succeeded.
  \return          1  Function failed.
  \note    When the variable <b>__Vendor_SysTickConfig</b> is set to 1, then the
           function <b>SysTick_Config</b> is not included. In this case, the file <b><i>device</i>.h</b>
           must contain a vendor-specific implementation of this function.
 */
LOCAL_INLINE uint32 SysTick_Config(uint32 ticks)
{
    if ((ticks - 1U) > SysTick_LOAD_RELOAD_Msk)
    {
        return (1U); /* Reload value impossible */
    }

    SysTick->LOAD = (uint32)(ticks - 1U); /* set reload register */
    NVIC_SetPriority(SysTick_IRQn, (1U << __NVIC_PRIO_BITS) - 1U); /* set Priority for Systick Interrupt */
    SysTick->VAL = 0U; /* Load the SysTick Counter Value */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk; /* Enable SysTick IRQ and SysTick Timer */
    return (0U); /* Function successful */
}

#endif

/*@} end of CMSIS_Core_SysTickFunctions */
/* ##################################### Debug In/Output function ########################################### */
/**
  \ingroup  CMSIS_Core_FunctionInterface
  \defgroup CMSIS_core_DebugFunctions ITM Functions
  \brief    Functions that access the ITM debug interface.
  @{
 */

extern volatile sint32 ITM_RxBuffer; /*!< External variable to receive characters. */
#define                 ITM_RXBUFFER_EMPTY  ((sint32)0x5AA55AA5U) /*!< Value identifying \ref ITM_RxBuffer is ready for next character. */

/**
  \brief   ITM Send Character
  \details Transmits a character via the ITM channel 0, and
           \li Just returns when no debugger is connected that has booked the output.
           \li Is blocking when a debugger is connected, but the previous character sent has not been transmitted.
  \param [in]     ch  Character to transmit.
  \returns            Character to transmit.
 */
/*PRQA S 3006 ++ # AMCE_SYS_3006,AMCE_SYS_3008*/
LOCAL_INLINE uint32 ITM_SendChar(uint32 ch)
{
    if (((ITM->TCR & ITM_TCR_ITMENA_Msk) != 0U) && /* ITM enabled */
            ((ITM->TER & 1U) != 0U)) /* ITM Port #0 enabled */
    {
        while (0U == ITM->PORT[0U].u32)
        {
            __NOP(); //PRQA S 1006 # AMCE_SYS_1006
        }
        ITM->PORT[0U].u8 = (uint8)ch;
    }
    return (ch);
}
/*PRQA S 3006 -- */

/**
  \brief   ITM Receive Character
  \details Inputs a character via the external variable \ref ITM_RxBuffer.
  \return             Received character.
  \return         -1  No character pending.
 */
LOCAL_INLINE sint32 ITM_ReceiveChar(void)
{
    sint32 ch = -1; /* no character available */

    if (ITM_RxBuffer != ITM_RXBUFFER_EMPTY)
    {
        ch = ITM_RxBuffer;
        ITM_RxBuffer = ITM_RXBUFFER_EMPTY; /* ready for next character */
    }

    return (ch);
}

/**
  \brief   ITM Check Character
  \details Checks whether a character is pending for reading in the variable \ref ITM_RxBuffer.
  \return          0  No character available.
  \return          1  Character available.
 */
LOCAL_INLINE sint32 ITM_CheckChar(void)
{

    if (ITM_RxBuffer == ITM_RXBUFFER_EMPTY)
    {
        return (0); /* no character available */
    }
    else
    {
        return (1); /*    character available */
    }
}
/*@} end of CMSIS_core_DebugFunctions */

#ifdef __cplusplus
}
#endif

#endif /* CORE_AC784XX_H */
