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
 * AutoChips Inc. (C) 2026. All rights reserved.
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

/*PRQA S 0791 EOF # allow similar macro definitions.*/

/** @addtogroup AutoChips
  * @{
  */

/** @addtogroup AC7840x
  * @{
  */

#ifndef AC7840E_H
#define AC7840E_H

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup Configuration_of_CMSIS
  * @{
  */
/*PRQA S 1535 EOF # AMCE_SYS_1535 */
/* =========================================================================================================================== */
/* ================                                Interrupt Number Definition                                ================ */
/* =========================================================================================================================== */

typedef enum {
/* =======================================  ARM Cortex-M4 Specific Interrupt Numbers  ======================================== */
  NonMaskableInt_IRQn       = -14,              /*!< -14 Non Maskable Interrupt                                                */
  HardFault_IRQn            = -13,              /*!< -13 Cortex-M4 Hard Fault Interrupt                                        */
  MemoryManagement_IRQn     = -12,              /*!< -12 Cortex-M4 Memory Management Interrupt                                 */
  BusFault_IRQn             = -11,              /*!< -11 Cortex-M4 Bus Fault Interrupt                                         */
  UsageFault_IRQn           = -10,              /*!< -10 Cortex-M4 Usage Fault Interrupt                                       */
  SVCall_IRQn               = -5,               /*!< -5  Cortex-M4 SV Call Interrupt                                           */
  DebugMonitor_IRQn         = -4,               /*!< -4  Cortex-M4 Debug Monitor Interrupt                                     */
  PendSV_IRQn               = -2,               /*!< -2  Cortex-M4 Pend SV Interrupt                                           */
  SysTick_IRQn              = -1,               /*!< -1  Cortex-M4 System Tick Interrupt                                       */
/* ===========================================  AC7840e Specific Interrupt Numbers  ========================================== */
  DMA0_CHANNEL0_IRQn        =   0,              /*!< 0  DMA0 channel 0 interrupt                                               */
  DMA0_CHANNEL1_IRQn        =   1,              /*!< 1  DMA0 channel 1 interrupt                                               */
  DMA0_CHANNEL2_IRQn        =   2,              /*!< 2  DMA0 channel 2 interrupt                                               */
  DMA0_CHANNEL3_IRQn        =   3,              /*!< 3  DMA0 channel 3 interrupt                                               */
  DMA0_CHANNEL4_IRQn        =   4,              /*!< 4  DMA0 channel 4 interrupt                                               */
  DMA0_CHANNEL5_IRQn        =   5,              /*!< 5  DMA0 channel 5 interrupt                                               */
  DMA0_CHANNEL6_IRQn        =   6,              /*!< 6  DMA0 channel 6 interrupt                                               */
  DMA0_CHANNEL7_IRQn        =   7,              /*!< 7  DMA0 channel 7 interrupt                                               */
  DMA0_CHANNEL8_IRQn        =   8,              /*!< 8  DMA0 channel 8 interrupt                                               */
  DMA0_CHANNEL9_IRQn        =   9,              /*!< 9  DMA0 channel 9 interrupt                                               */
  DMA0_CHANNEL10_IRQn       =  10,              /*!< 10 DMA0 channel 10 interrupt                                              */
  DMA0_CHANNEL11_IRQn       =  11,              /*!< 11 DMA0 channel 11 interrupt                                              */
  DMA0_CHANNEL12_IRQn       =  12,              /*!< 12 DMA0 channel 12 interrupt                                              */
  DMA0_CHANNEL13_IRQn       =  13,              /*!< 13 DMA0 channel 13 interrupt                                              */
  DMA0_CHANNEL14_IRQn       =  14,              /*!< 14 DMA0 channel 14 interrupt                                              */
  DMA0_CHANNEL15_IRQn       =  15,              /*!< 15 DMA0 channel 15 interrupt                                              */
  PORTA_IRQn                =  16,              /*!< 16 PORTA interrupt                                                        */
  PORTB_IRQn                =  17,              /*!< 17 PORTB interrupt                                                        */
  PORTC_IRQn                =  18,              /*!< 18 PORTC interrupt                                                        */
  PORTD_IRQn                =  19,              /*!< 19 PORTD interrupt                                                        */
  PORTE_IRQn                =  20,              /*!< 20 PORTE interrupt                                                        */
  UART0_IRQn                =  21,              /*!< 21 UART0 interrupt                                                        */
  UART1_IRQn                =  22,              /*!< 22 UART1 interrupt                                                        */
  UART2_IRQn                =  23,              /*!< 23 UART2 interrupt                                                        */
  SPI0_IRQn                 =  27,              /*!< 27 SPI0 interrupt                                                         */
  SPI1_IRQn                 =  28,              /*!< 28 SPI1 interrupt                                                         */
  SPI2_IRQn                 =  29,              /*!< 29 SPI2 interrupt                                                         */
  I2C0_IRQn                 =  31,              /*!< 31 I2C0 interrupt                                                         */
  I2C1_IRQn                 =  32,              /*!< 32 I2C1 interrupt                                                         */
  EIO_IRQn                  =  33,              /*!< 33 EIO interrupt                                                          */
  CAN0_IRQn                 =  34,              /*!< 34 CAN0 interrupt                                                         */
  CAN0_WAKEUP_IRQn          =  35,              /*!< 35 CAN0 wakeup interrupt                                                  */
  CAN1_IRQn                 =  36,              /*!< 36 CAN1 interrupt                                                         */
  CAN1_WAKEUP_IRQn          =  37,              /*!< 37 CAN1 wakeup interrupt                                                  */
  ADC0_IRQn                 =  48,              /*!< 48 ADC0 interrupt                                                         */
  ACMP0_IRQn                =  50,              /*!< 50 ACMP0 interrupt                                                        */
  WDG_IRQn                  =  51,              /*!< 51 WDG interrupt                                                          */
  MCM_IRQn                  =  53,              /*!< 53 MCM interrupt                                                          */
  LVD_IRQn                  =  54,              /*!< 54 LVD interrupt                                                          */
  SPM_IRQn                  =  55,              /*!< 55 SPM interrupt                                                          */
  RCM_IRQn                  =  56,              /*!< 56 RCM interrupt                                                          */
  PWM0_OVERFLOW_IRQn        =  57,              /*!< 57 PWM0 Overflow interrupt                                                */
  PWM0_CHANNEL_IRQn         =  58,              /*!< 58 PWM0 Channel interrupt                                                 */
  PWM0_FAULT_IRQn           =  59,              /*!< 59 PWM0 Fault interrupt                                                   */
  PWM1_OVERFLOW_IRQn        =  60,              /*!< 60 PWM1 Overflow interrupt                                                */
  PWM1_CHANNEL_IRQn         =  61,              /*!< 61 PWM1 Channel interrupt                                                 */
  PWM1_FAULT_IRQn           =  62,              /*!< 62 PWM1 Fault interrupt                                                   */
  PWM2_OVERFLOW_IRQn        =  63,              /*!< 63 PWM2 Overflow interrupt                                                */
  PWM2_CHANNEL_IRQn         =  64,              /*!< 64 PWM2 Channel interrupt                                                 */
  PWM2_FAULT_IRQn           =  65,              /*!< 65 PWM2 Fault interrupt                                                   */
  PWM3_OVERFLOW_IRQn        =  66,              /*!< 66 PWM3 Overflow interrupt                                                */
  PWM3_CHANNEL_IRQn         =  67,              /*!< 67 PWM3 Channel interrupt                                                 */
  PWM3_FAULT_IRQn           =  68,              /*!< 68 PWM3 Fault interrupt                                                   */
  RTC_IRQn                  =  81,              /*!< 81 RTC interrupt                                                          */
  TIMER_CHANNEL0_IRQn       =  83,              /*!< 83 TIMER channel 0 interrupt                                              */
  TIMER_CHANNEL1_IRQn       =  84,              /*!< 84 TIMER channel 1 interrupt                                              */
  TIMER_CHANNEL2_IRQn       =  85,              /*!< 85 TIMER channel 2 interrupt                                              */
  TIMER_CHANNEL3_IRQn       =  86,              /*!< 86 TIMER channel 3 interrupt                                              */
  HSM_IRQn                  =  87,              /*!< 87 HSM interrupt                                                          */
  FLASH_ECC_IRQn            =  88,              /*!< 88 FLASH ECC 2bit error interrupt                                         */
  FLASH_IRQn                =  89,              /*!< 89 FLASH command complete interrupt                                       */
  FLASH_COLLISION_IRQn      =  90,              /*!< 90 FLASH collision interrupt                                              */
  ECC_1BIT_ERROR_IRQn       =  91,              /*!< 91 ECC 1bit error interrupt                                               */
  ECC_2BIT_ERROR_IRQn       =  92,              /*!< 92 ECC 2bit error interrupt                                               */
  SENT_CHANNEL0_IRQn        =  93,              /*!< 93 SENT channel 0 interrupt                                               */
  SENT_CHANNEL1_IRQn        =  94,              /*!< 94 SENT channel 1 interrupt                                               */
  SMU_IRQn                  =  95,               /*!< 95 SMU interrupt                                                          */
  MAX_IRQn                  =  96               /*!< 96 Max interrupt                                                          */
} IRQn_Type;

/* =========================================================================================================================== */
/* ================                           Processor and Core Peripheral Section                           ================ */
/* =========================================================================================================================== */

/* ---------------------------  Start of section using anonymous unions and disabling  warnings  ----------------------------- */
#if defined (__CC_ARM)
  #pragma push
  #pragma anon_unions
#elif defined (__ICCARM__)
  #pragma language=extended
#elif (defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wc11-extensions"
  #pragma clang diagnostic ignored "-Wreserved-id-macro"
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning 586
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#elif defined (__ghs__)
  /* anonymous unions are enabled by default */
#else
  #error Not supported compiler type
#endif

/* ===========================  Configuration of the ARM Cortex-M4 Processor and Core Peripherals  =========================== */
/*PRQA S 0602 ++ # use identifiers reserved for use by the standard library or compiler.*/
/*PRQA S 0603 ++ # use macros reserved for use by the standard library or compiler.*/
#define __CM4_REV                 0x0001U       /*!< CM4 Core Revision                                                         */
#define __NVIC_PRIO_BITS               4U       /*!< Number of Bits used for Priority Levels                                   */
#define __Vendor_SysTickConfig         0U       /*!< Set to 1 if different SysTick Config is used                              */
#define __VTOR_PRESENT                 1U       /*!< Set to 1 if CPU supports Vector Table Offset Register                     */
#define __MPU_PRESENT                  0U       /*!< MPU present                                                               */
#define __FPU_PRESENT                  1U       /*!< FPU present                                                               */
#ifndef ARM_CMSIS_CORE_USED
#define __FPU_USED                     1U       /*!< FPU used                                                                  */
#endif
/*PRQA S 0602 -- # use identifiers reserved for use by the standard library or compiler.*/
/*PRQA S 0603 -- # use macros reserved for use by the standard library or compiler.*/
/** @} */ /* End of group Configuration_of_CMSIS */

#ifdef ARM_CMSIS_CORE_USED
#include "core_cm4.h"                           /*!< CMSIS ARM Cortex-M4 processor and core peripherals                        */
#else
#include "Core_AC784xx.h"                       /*!< non-CMSIS ARM Cortex-M4 processor and core peripherals                    */
#endif

#include "System_AC784xx.h"                     /*!< AC7840x System                                                            */

#ifndef __I                                    /*!< Fallback for older CMSIS versions                                          */
  #define __I    volatile const
#endif
#ifndef __O                                    /*!< Fallback for older CMSIS versions                                          */
  #define __O    volatile
#endif
#ifndef __IO                                   /*!< Fallback for older CMSIS versions                                          */
  #define __IO   volatile
#endif

#ifndef __IM                                    /*!< Fallback for older CMSIS versions                                         */
  #define __IM   volatile const
#endif
#ifndef __OM                                    /*!< Fallback for older CMSIS versions                                         */
  #define __OM   volatile
#endif
#ifndef __IOM                                   /*!< Fallback for older CMSIS versions                                         */
  #define __IOM  volatile
#endif

/* =========================================================================================================================== */
/* ================                            Device Specific Peripheral Section                             ================ */
/* =========================================================================================================================== */

/* ----------------------------  End of section using anonymous unions and disabling  warnings  ------------------------------ */
#if defined (__CC_ARM)
  #pragma pop
#elif defined (__ICCARM__)
  /* anonymous unions are enabled by default */
#elif (defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
  #pragma clang diagnostic pop
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  /* anonymous unions are enabled by default */
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#elif defined (__ghs__)
  /* anonymous unions are enabled by default */
#else
  #error Not supported compiler type
#endif

/** @addtogroup Device_Peripheral_peripherals
  * @{
  */

/* =========================================================================================================================== */
/* ================                                            MCM                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief Core Platform Miscellaneous Control Module (MCM)
  */

typedef struct {                                /*!< (@ 0xE008000C) MCM Structure                                              */
  __IOM uint32  MCPCR;                        /*!< (@ 0x00000000) MCM Core Platform Control Register                         */
  __IOM uint32  MISCR;                        /*!< (@ 0x00000004) MCM Interrupt Status and Control Register                  */
  __IM  uint32  RESERVED[251];
  __IOM uint32  MLMDR0;                       /*!< (@ 0x000003F4) MCM Local MCM Local Memory Descriptor Register0            */
  __IM  uint32  MBIST;                        /*!< (@ 0x000003F8) MCM Local Memory BIST Register                             */
  __IOM uint32  MLMDR1;                       /*!< (@ 0x000003FC) MCM Local MCM Local Memory Descriptor Register1            */
  __IOM uint32  LMPECR;                       /*!< (@ 0x00000400) MCM Local Memory Cache ECC Enable Control Register         */
  __IOM uint32  LMPEIR;                       /*!< (@ 0x00000404) MCM Local Memory Cache ECC Error Status Register           */
  __IM  uint32  LMFAR;                        /*!< (@ 0x00000408) MCM Local Memory Cache ECC Error Address Register          */
  __IOM uint32  MCCR;                         /*!< (@ 0x0000040C) MCM Cache Control Register                                 */
  __IM  uint32  RESERVED1[25];
  __IOM uint32  MNCR;                         /*!< (@ 0x00000474) MCM NMI Control Register                                   */
} MCM_Type;                                     /*!< Size = 1144 (0x478)                                                       */

/* =========================================================================================================================== */
/* ================                                            MPU                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief Memory protection unit (MPU)
  */
/* MPU - Size of Registers Arrays */
#define MPU_EAR_EDR_COUNT             (3U)
#define MPU_RGD_COUNT                 (6U)

typedef struct {                                /*!< (@ 0x40087000) MPU Structure                                              */
  __IOM uint32  CESR;                         /*!< (@ 0x00000000) Control/Error Status Register                              */
  __IM  uint32  EAR[MPU_EAR_EDR_COUNT];       /*!< (@ 0x00000004) Error Address Register, slave port n                       */
  __IOM uint32  EDR[MPU_EAR_EDR_COUNT];       /*!< (@ 0x00000010) Error Detail Register, slave port n                        */
  __IM  uint32  RESERVED;
  __IOM uint32  WORD0_RGD[MPU_RGD_COUNT];     /*!< (@ 0x00000020) Region Descriptor n, Word 0                                */
  __IM  uint32  RESERVED1[2];
  __IOM uint32  WORD1_RGD[MPU_RGD_COUNT];     /*!< (@ 0x00000040) Region Descriptor n, Word 1                                */
  __IM  uint32  RESERVED2[2];
  __IOM uint32  WORD2_RGD[MPU_RGD_COUNT];     /*!< (@ 0x00000060) Region Descriptor n, Word 2                                */
  __IM  uint32  RESERVED3[2];
  __IOM uint32  WORD3_RGD[MPU_RGD_COUNT];     /*!< (@ 0x00000080) Region Descriptor n, Word 3                                */
  __IM  uint32  RESERVED4[218];
  __IOM uint32  RGDAAC[MPU_RGD_COUNT];       /*!< (@ 0x00000400) Region Descriptor n Alternate Access Control               */
} MPU_Type;                                     /*!< Size = 1048 (0x418)                                                       */

/* =========================================================================================================================== */
/* ================                                           CKGEN                                           ================ */
/* =========================================================================================================================== */

/**
  * @brief Clock Generator (CKGEN)
  */

typedef struct {                                /*!< (@ 0x40000000) CKGEN Structure                                            */
  __IOM uint32  CTRL;                         /*!< (@ 0x00000000) CKGEN Control Register                                     */
  __IOM uint32  LP_CLK_MUX;                   /*!< (@ 0x00000004) LSI Clock Mux                                              */
  __IOM uint32  PERI_CLK_EN0;                 /*!< (@ 0x00000008) Periph Clock Enable Control 0                              */
  __IOM uint32  PERI_CLK_EN1;                 /*!< (@ 0x0000000C) Periph Clock Enable Control 1                              */
  __IOM uint32  PERI_CLK_EN2;                 /*!< (@ 0x00000010) Periph Clock Enable Control 2                              */
  __IOM uint32  RCM_CTRL;                     /*!< (@ 0x00000014) MCU Reset Control                                          */
  __IOM uint32  RCM_EN;                       /*!< (@ 0x00000018) MCU Reset Enable Control                                   */
  __IOM uint32  RCM_STATUS;                   /*!< (@ 0x0000001C) MCU Reset Status                                           */
  __IOM uint32  PERI_SFT_RST0;                /*!< (@ 0x00000020) Periph Software Reset Control 0                            */
  __IOM uint32  PERI_SFT_RST1;                /*!< (@ 0x00000024) Periph Software Reset Control 1                            */
  __IOM uint32  PERI_SFT_RST2;                /*!< (@ 0x00000028) Periph Software Reset Control 2                            */
  __IOM uint32  CLK_DIV1;                     /*!< (@ 0x0000002C) Clock Divider 1                                            */
  __IOM uint32  CLK_DIV2;                     /*!< (@ 0x00000030) Clock Divider 2                                            */
  __IOM uint32  PERI_CLK_MUX0;                /*!< (@ 0x00000034) Peripheral Clock MUX0 Register                             */
  __IOM uint32  PERI_CLK_MUX1;                /*!< (@ 0x00000038) Peripheral Clock MUX1 Register                             */
  __IOM uint32  PERI_CLK_MUX2;                /*!< (@ 0x0000003C) Peripheral Clock MUX2 Register                             */
  __IOM uint32  PERI_CLK_MUX3;                /*!< (@ 0x00000040) Peripheral Clock MUX3 Register                             */
  __IM  uint32  RESERVED;
  __IOM uint32  CLK_OUT_CFG;                  /*!< (@ 0x00000048) Clock Out Configure                                        */
  __IOM uint32  PERI_CLK_DIV;                 /*!< (@ 0x0000004C) Peripheral Clock Divider                                   */
  __IOM uint32  PERI_CLK_DIV1;                /*!< (@ 0x00000050) Peripheral Clock Divider 1                                 */
} CKGEN_Type;                                   /*!< Size = 84 (0x54)                                                          */

/* =========================================================================================================================== */
/* ================                                            PBR                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief Peripheral Bridge (PBR)
  */

typedef struct {                                /*!< (@ 0x4008A000) PBR Structure                                              */
  __IOM uint32  MPR_CORE;                     /*!< (@ 0x00000000) Master Core Privilege Configure Register                   */
  __IOM uint32  MPR_DEBUG;                    /*!< (@ 0x00000004) Master Debugger Privilege Configure Register               */
  __IOM uint32  MPR_DMA;                      /*!< (@ 0x00000008) Master DMA Privilege Configure Register                    */
  __IM  uint32  RESERVED;
  __IOM uint32  PACRA;                        /*!< (@ 0x00000010) Peripheral Access Control Register A                       */
  __IOM uint32  PACRB;                        /*!< (@ 0x00000014) Peripheral Access Control Register B                       */
  __IM  uint32  RESERVED1[2];
  __IOM uint32  PACRC;                        /*!< (@ 0x00000020) Peripheral Access Control Register C                       */
  __IOM uint32  PACRD;                        /*!< (@ 0x00000024) Peripheral Access Control Register D                       */
  __IOM uint32  PACRE;                        /*!< (@ 0x00000028) Peripheral Access Control Register E                       */
  __IM  uint32  RESERVED2;
  __IOM uint32  PACRF;                        /*!< (@ 0x00000030) Peripheral Access Control Register F                       */
  __IOM uint32  PACRG;                        /*!< (@ 0x00000034) Peripheral Access Control Register G                       */
} PBR_Type;                                     /*!< Size = 56 (0x38)                                                          */

/* =========================================================================================================================== */
/* ================                                            SPM                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief System Power Manage (SPM)
  */

typedef struct {                                /*!< (@ 0x40008000) SPM Structure                                              */
  __IOM uint32  PWR_MGR_CFG0;                 /*!< (@ 0x00000000) Power Manage Config Register 0                             */
  __IOM uint32  PWR_MGR_CFG1;                 /*!< (@ 0x00000004) Power Manage Config Register 1                             */
  __IM  uint32  RESERVED;
  __IM  uint32  PERIPH_SLEEP_ACK_STATUS;      /*!< (@ 0x0000000C) Periph Sleep Ack Status Register                           */
  __IOM uint32  PERIPH_SLEEP_ACK_EN;          /*!< (@ 0x00000010) Periph Sleep Ack Enable Register                           */
  __IOM uint32  STATUS;                       /*!< (@ 0x00000014) Status Register                                            */
  __IOM uint32  SRAM_POW_CTRL0;               /*!< (@ 0x00000018) SRAM power control Register 0                              */
  __IOM uint32  SRAM_POW_CTRL1;               /*!< (@ 0x0000001C) SRAM power control Register 1                              */
  __IM  uint32  RESERVED2;
  __IOM uint32  STB_WP_EN0;                   /*!< (@ 0x00000024) Standby Wakeup Enable Register 0                           */
  __IOM uint32  STB_WP_EN1;                   /*!< (@ 0x00000028) Standby Wakeup Enable Register 1                           */
  __IOM uint32  STB_WP_STATUS;                /*!< (@ 0x0000002C) Standby Wakeup Status Register                             */
} SPM_Type;                                     /*!< Size = 48 (0x30)                                                          */

/* =========================================================================================================================== */
/* ================                                           GPIO                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief General Purpose Input/Output (GPIO)
  */

typedef struct {                                /*!< (@ 0x40085000) GPIO Structure                                             */
  __IOM uint32  PODR;                         /*!< (@ 0x00000000) PORT Output Data Register                                  */
  __IOM uint32  PSOR;                         /*!< (@ 0x00000004) PORT Set Output Register                                   */
  __IOM uint32  PROR;                         /*!< (@ 0x00000008) PORT Reset Output Register                                 */
  __IOM uint32  PIOR;                         /*!< (@ 0x0000000C) PORT Invert Output Register                                */
  __IOM uint32  PIDR;                         /*!< (@ 0x00000010) PORT Input Data Register                                   */
  __IOM uint32  POER;                         /*!< (@ 0x00000014) PORT Output Enable Register                                */
  __IOM uint32  PIER;                         /*!< (@ 0x00000018) PORT Input Enable Register                                 */
  __IOM uint32  OTYPER;                       /*!< (@ 0x0000001C) PORT Output Type Register                                  */
} GPIO_Type;                                    /*!< Size = 32 (0x20)                                                          */

/* =========================================================================================================================== */
/* ================                                           PORT                                            ================ */
/* =========================================================================================================================== */
/**
  * @brief Pin Control and Interrupts (PORT)
  */

#define PORT_PCR_COUNT                (32U)    /*!< PORT - size of registers array                                            */

typedef struct {                                /*!< (@ 0x40085200) PORT Structure                                             */
  __IOM uint32  PCR[PORT_PCR_COUNT];          /*!< (@ 0x00000000~0x0000007C) Pin Control Register n                          */
  __IM  uint32  RESERVED[8];
  __IOM uint32  ISFR;                         /*!< (@ 0x000000A0) Interrupt Status Flag Register                             */
  __IOM uint32  DFER;                         /*!< (@ 0x000000A4) Digital Filter Enable Register                             */
  __IOM uint32  DFCR;                         /*!< (@ 0x000000A8) Digital Filter Clock Register                              */
  __IOM uint32  DFWR;                         /*!< (@ 0x000000AC) Digital Filter Width Register                              */
} PORT_Type;                                    /*!< Size = 176 (0xb0)                                                         */

/* =========================================================================================================================== */
/* ================                                           CAN                                             ================ */
/* =========================================================================================================================== */

/**
  * @brief Controller Area Network (CAN)
  */

#define CAN_BUF_LENGTH           (64U / 4U)       /*!< CAN buffer length                                                         */

/**
  * @brief CAN receive buffer register define structure
  */
typedef struct
{
    __IO uint32  ID_ESI;                      /*!< [0:28]: Identifier (ID), [30]: Transmit time-stamp enable (TTSEN)         */
                                                /*!< [31]: Error state indicator (ESI)                                         */
    __IO uint32  CTRL;                        /*!< [0:3]: Data length code (DLC), [4]: Bit rate switch (BRS)                 */
                                                /*!< [5]: FD format indicator (FDF), [6]: Remote transmission request (RTR)    */
                                                /*!< [7]: Identifier extension (IDE), [12]: For lookback mode (TX)             */
                                                /*!< [13:15]: Kind of error (KOER)                                             */
    __IO uint32  DATA[CAN_BUF_LENGTH];        /*!< Buffer data                                                               */
    __IM uint32  TS[2];                       /*!< Receive or transmission time stamp                                        */
} CAN_BufType;

typedef struct {                                /*!< (@ 0x40007000) CAN structure                                              */
        CAN_BufType  RBUF;                      /*!< (@ 0x00000000) Receive buffer                                             */
        CAN_BufType  TBUF;                      /*!< (@ 0x00000050) Transmit buffer                                            */
  __IOM uint32  CTRL0;                        /*!< (@ 0x000000A0) Config state and transmit/receive control register 0       */
  __IOM uint32  CTRL1;                        /*!< (@ 0x000000A4) Interrupt enable and flag control register 1               */
  __IOM uint32  SBITRATE;                     /*!< (@ 0x000000A8) CAN normal bitrate configuration register                  */
  __IOM uint32  FBITRATE;                     /*!< (@ 0x000000AC) CAN FD data bitrate configuration register                 */
  __IOM uint32  ERRINFO;                      /*!< (@ 0x000000B0) Error type and error counter register                      */
  __IOM uint32  ACFCTRL0;                     /*!< (@ 0x000000B4) Acceptance filter control register 0                       */
  __IOM uint32  ACFCTRL1;                     /*!< (@ 0x000000B8) Acceptance filter control register 1                       */
  __IOM uint32  ACFCTRL2;                     /*!< (@ 0x000000BC) Acceptance filter control register 2                       */
  __IOM uint32  ACF;                          /*!< (@ 0x000000C0) Acceptance code register                                   */
  __IOM uint32  VERMEM;                       /*!< (@ 0x000000C4) Version and memory protection register                     */
  __IOM uint32  MEMES;                        /*!< (@ 0x000000C8) Memory error simulation register                           */
  __IM  uint32  RESERVED1[4];
  __IOM uint32  TBU;                          /*!< (@ 0x000000DC) Tx buffer used register                                    */
  __IOM uint32  RBU;                          /*!< (@ 0x000000E0) Rx buffer used register                                    */
} CAN_Type;                                     /*!< Size = 228 (0xe4)                                                         */

/* =========================================================================================================================== */
/* ================                                           UART                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief Universal Asynchronous Receiver/Transmitter (UART)
  */

typedef struct {                                /*!< (@ 0x40018000) UART Structure                                             */
  __IOM uint32  RBR;                          /*!< (@ 0x00000000) RX/TX Data Register                                        */
  __IOM uint32  DIV_L;                        /*!< (@ 0x00000004) Divisor low 8 bits register                                */
  __IOM uint32  DIV_H;                        /*!< (@ 0x00000008) Divisor high 8 bits register                               */
  __IOM uint32  LCR0;                         /*!< (@ 0x0000000C) UART control register 0                                    */
  __IOM uint32  LCR1;                         /*!< (@ 0x00000010) UART control register 1                                    */
  __IOM uint32  FCR;                          /*!< (@ 0x00000014) FIFO Control Register                                      */
  __IOM uint32  EFR;                          /*!< (@ 0x00000018) hardware flow control register                             */
  __IOM uint32  IER;                          /*!< (@ 0x0000001C) Interrupt Enable register                                  */
  __IOM uint32  LSR0;                         /*!< (@ 0x00000020) Line Status Register 0                                     */
  __IOM uint32  LSR1;                         /*!< (@ 0x00000024) Line Status Register 1                                     */
  __IOM uint32  SMP_CNT;                      /*!< (@ 0x00000028) UART sample counter register                               */
  __IOM uint32  ADDR;                         /*!< (@ 0x0000002C) UART match address register                                */
  __IOM uint32  DATA;                         /*!< (@ 0x00000030) UART match data register                                   */
  __IOM uint32  GUARD;                        /*!< (@ 0x00000034) UART guard time register                                   */
  __IM  uint32  RESERVED;
  __IOM uint32  SLEEP_EN;                     /*!< (@ 0x0000003C) UART sleep enable register                                 */
  __IOM uint32  DMA_EN;                       /*!< (@ 0x00000040) UART DMA enable register                                   */
  __IOM uint32  DIV_FRAC;                     /*!< (@ 0x00000044) Uart Fractional Divider Address                            */
  __IOM uint32  MTCHCR;                       /*!< (@ 0x00000048) Uart match fucntion enable                                 */
  __IOM uint32  RS485CR;                      /*!< (@ 0x0000004C) Uart RS485 control register                                */
  __IM  uint32  RESERVED1;
  __IOM uint32  CNTR;                         /*!< (@ 0x00000054) Uart Counter time delay in RS485 mode                      */
  __IOM uint32  IDLE;                         /*!< (@ 0x00000058) Uart IDLE register                                         */
  __IOM uint32  LINCR;                        /*!< (@ 0x0000005C) LIN Control register                                       */
  __IOM uint32  BRKLGH;                       /*!< (@ 0x00000060) LIN Break Length Select Register                           */
  __IOM uint32  PMIN0;                        /*!< (@ 0x00000064) IrDA mode pulse minimum pulse                              */
  __IOM uint32  PMIN1;                        /*!< (@ 0x00000068) IrDA mode pulse minimum pulse                              */
  __IOM uint32  PWDTH;                        /*!< (@ 0x0000006C) IrDA mode pulse width for transmit                         */
  __IM  uint32  RESERVED2;
  __IOM uint32  IDLELGH;                      /*!< (@ 0x00000074) Detection length of UART IDLE function                     */
} UART_Type;                                    /*!< Size = 116 (0x74)                                                         */

/* =========================================================================================================================== */
/* ================                                           I2C                                             ================ */
/* =========================================================================================================================== */

/**
  * @brief Inter-Integrated Circuit (I2C)
  */

typedef struct {                                /*!< (@ 0x4000E000) I2C Structure                                              */
  __IOM uint32  ADDR0;                        /*!< (@ 0x00000000) Address Register 0                                         */
  __IOM uint32  ADDR1;                        /*!< (@ 0x00000004) Address register 1                                         */
  __IOM uint32  SAMPLE_CNT;                   /*!< (@ 0x00000008) SAMPLE CNT Register                                        */
  __IOM uint32  STEP_CNT;                     /*!< (@ 0x0000000C) STEP CNT Register                                          */
  __IOM uint32  CTRL0;                        /*!< (@ 0x00000010) Control Register 0                                         */
  __IOM uint32  CTRL1;                        /*!< (@ 0x00000014) Control Register 1                                         */
  __IOM uint32  CTRL2;                        /*!< (@ 0x00000018) Control Register 2                                         */
  __IOM uint32  CTRL3;                        /*!< (@ 0x0000001C) Control Register 3                                         */
  __IOM uint32  STATUS0;                      /*!< (@ 0x00000020) Status Register 0                                          */
  __IOM uint32  STATUS1;                      /*!< (@ 0x00000024) Status Register 1                                          */
  __IOM uint32  DGLCFG;                       /*!< (@ 0x00000028) Deglitch Configuration Register                            */
  __IOM uint32  DATA;                         /*!< (@ 0x0000002C) Data Register                                              */
  __IOM uint32  STARTSTOP;                    /*!< (@ 0x00000030) START_STOP Register                                        */
} I2C_Type;                                     /*!< Size = 52 (0x34)                                                          */

/* =========================================================================================================================== */
/* ================                                           SPI                                             ================ */
/* =========================================================================================================================== */

/**
  * @brief Serial Peripheral Interface (SPI)
  */

typedef struct {                                /*!< (@ 0x4000C000) SPI Structure                                              */
  __IOM uint32  CFG0;                         /*!< (@ 0x00000000) SPI Configuration Register 0                               */
  __IOM uint32  CFG1;                         /*!< (@ 0x00000004) SPI Configuration Register 1                               */
  __IOM uint32  CMD;                          /*!< (@ 0x00000008) SPI Command Register                                       */
  __IOM uint32  STATUS;                       /*!< (@ 0x0000000C) SPI Status Register                                        */
  __IOM uint32  DATA;                         /*!< (@ 0x00000010) SPI Data Register                                          */
  __IOM uint32  CFG2;                         /*!< (@ 0x00000014) SPI configuration register 2                               */
  __IM  uint32  RESERVED[2];
  __IOM uint32  DMV;                          /*!< (@ 0x00000020) SPI data match register                                    */
} SPI_Type;                                     /*!< Size = 36 (0x24)                                                          */

/* =========================================================================================================================== */
/* ================                                            ANA                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief Analog Control Registers (ANA)
  */

typedef struct {                                /*!< (@ 0x40008800) ANA Structure                                              */
  __IM  uint32  RESERVED[4];
  __IOM uint32  TSENSOR_CFG0;                 /*!< (@ 0x00000010) TSENSOR Configuration Register0                            */
  __IM  uint32  RESERVED1[3];
  __IOM uint32  AMUX_CFG0;                    /*!< (@ 0x00000020) AMUX Configuration Register0                                */
  __IM  uint32  RESERVED2[3];
  __IOM uint32  LVR_CFG0;                     /*!< (@ 0x00000030) LVR Configuration Register0                                */
  __IM  uint32  RESERVED3[3];
  __IOM uint32  BG_CFG0;                      /*!< (@ 0x00000040) BG Configuration Register0                                */
  __IM  uint32  RESERVED4[31];
  __IOM uint32  SPLL_CFG0;                    /*!< (@ 0x000000C0) System PLL Config 0                                        */
  __IOM uint32  SPLL_CFG1;                    /*!< (@ 0x000000C4) System PLL Config 1                                        */
} ANA_Type;                                     /*!< Size = 200 (0xc8)                                                         */

/* =========================================================================================================================== */
/* ================                                           ADC                                             ================ */
/* =========================================================================================================================== */

/**
  * @brief Analog to Digital Converter (ADC)
  */

#define ADC_REGULAR_SEQ_NUM         (32U)
#define ADC_INJECT_SEQ_NUM          (4U)
#define ADC_SAMPLE_REG_NUM          (4U)

typedef struct {                                /*!< (@ 0x40003000) ADC Structure                                              */
  __IOM uint32  STR;                          /*!< (@ 0x00000000) ADC status Register                                        */
  __IOM uint32  CTRL0;                        /*!< (@ 0x00000004) ADC Control Register 0                                     */
  __IOM uint32  CTRL1;                        /*!< (@ 0x00000008) ADC Control Register 1                                     */
  __IOM uint32  IOFR[ADC_INJECT_SEQ_NUM];     /*!< (@ 0x0000000C) ADC Injection Group Offset Register                        */
  __IOM uint32  AMOHR;                        /*!< (@ 0x0000001C) AMO High threshold and offset register                     */
  __IOM uint32  AMOLR;                        /*!< (@ 0x00000020) AMO Low threshold and offset register                      */
  __IOM uint32  SPT[ADC_SAMPLE_REG_NUM];      /*!< (@ 0x00000024) ADC Sample time setting register(n)                        */
  __IOM uint32  RSQR[ADC_REGULAR_SEQ_NUM];    /*!< (@ 0x00000034) ADC regular group sequence configure register(n)           */
  __IOM uint32  ISQR[ADC_INJECT_SEQ_NUM];     /*!< (@ 0x000000B4) ADC injected group sequence configure register(n)          */
  __IOM uint32  SQL;                          /*!< (@ 0x000000C4) Sequence length of group configure register                */
  __IOM uint32  CALI0;                        /*!< (@ 0x000000C8) ADC Calibration Register 0                                 */
  __IOM uint32  CALI1;                        /*!< (@ 0x000000CC) ADC Calibration Register 1                                 */
  __IM  uint32  RESERVED[6];
  __IM  uint32  RDR[ADC_REGULAR_SEQ_NUM];     /*!< (@ 0x000000E8) ADC Regular Group data Register(n)                         */
  __IM  uint32  IDR[ADC_INJECT_SEQ_NUM];      /*!< (@ 0x00000168) ADC Injected Group data Register(n)                        */
  __IM  uint32  RESERVED1[2];
  __IOM uint32  REOC;                         /*!< (@ 0x00000180) ADC Regular Group EOC Flag                                 */
  __IM  uint32  RESERVED2;
  __IOM uint32  IEOC;                         /*!< (@ 0x00000188) ADC Injection Group EOC Flag                               */
} ADC_Type;                                     /*!< Size = 396 (0x18c)                                                         */

/* =========================================================================================================================== */
/* ================                                           ACMP                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief Analog comparator (ACMP)
  */

typedef struct {                                /*!< (@ 0x40005000) ACMP Structure                                             */
  __IOM uint32  CR0;                          /*!< (@ 0x00000000) ACMP Configuration Register 0                              */
  __IOM uint32  CR1;                          /*!< (@ 0x00000004) ACMP Configuration Register 1                              */
  __IOM uint32  CR2;                          /*!< (@ 0x00000008) ACMP configuration register 2                              */
  __IOM uint32  CR3;                          /*!< (@ 0x0000000C) ACMP configuration register 3                              */
  __IOM uint32  CR4;                          /*!< (@ 0x00000010) ACMP configuration register 4                              */
  __IM  uint32  DR;                           /*!< (@ 0x00000014) ACMP data output register 0                                */
  __IOM uint32  SR;                           /*!< (@ 0x00000018) ACMP status register 0                                     */
  __IOM uint32  FD;                           /*!< (@ 0x0000001C) ACMP polling frequency divider register                    */
  __IOM uint32  OPA;                          /*!< (@ 0x00000020) ACMP hall output A register                                */
  __IOM uint32  OPB;                          /*!< (@ 0x00000024) ACMP hall output B register                                */
  __IOM uint32  OPC;                          /*!< (@ 0x00000028) ACMP hall output C register                                */
  __IM  uint32  RESERVED[7];
  __IOM uint32  CLK;                          /*!< (@ 0x00000048) ACMP clock register                                        */
} ACMP_Type;                                    /*!< Size = 76 (0x4c)                                                          */

/* =========================================================================================================================== */
/* ================                                           ABIST                                           ================ */
/* =========================================================================================================================== */

/**
  * @brief Analog BIST (ABIST)
  */

typedef struct {                                /*!< (@ 0x40005800) ABIST Structure                                            */
  __IOM uint32  CR;                           /*!< (@ 0x00000000) ABIST Control Register                                     */
  __IOM uint32  SR;                           /*!< (@ 0x00000004) ABIST Status Register                                      */
  __IOM uint32  ER;                           /*!< (@ 0x00000008) LVD or OVD event Register                                  */
} ABIST_Type;                                   /*!< Size = 12 (0xc)                                                           */

/* =========================================================================================================================== */
/* ================                                           PWM                                             ================ */
/* =========================================================================================================================== */
/**
  * @brief Pulse Width Modulation (PWM)
  */

#define PWM_CHANNEL_MAX                 (8U)   /*!< Number of channel of the PWM module                                       */

typedef struct {                                /*!< (@ 0x40080000) PWM Structure                                              */
  __IOM uint32  INIT;                         /*!< (@ 0x00000000) PWM Initialize, Include Clock and Prescale Setting         */
  __IOM uint32  CNT;                          /*!< (@ 0x00000004) PWM Counter Value                                          */
  __IOM uint32  MCVR;                         /*!< (@ 0x00000008) PWM Counter Max Count Value Register                       */
  struct {
  __IOM uint32  CHnSCR;                       /*!< (@ 0x0000000C + 0x8 * n) Channel (n) Status And Control Register          */
  __IOM uint32  CHnV;                         /*!< (@ 0x00000010 + 0x8 * n) Channel (n) Value                                */
  } CHANNELS[PWM_CHANNEL_MAX];
  __IOM uint32  CNTIN;                        /*!< (@ 0x0000004C) Counter Initial Value                                      */
  __IOM uint32  STR;                          /*!< (@ 0x00000050) Status Register                                            */
  __IOM uint32  FUNCSEL;                      /*!< (@ 0x00000054) PWM Features(Functions) Mode Selection Register            */
  __IOM uint32  SYNC;                         /*!< (@ 0x00000058) Synchronization                                            */
  __IOM uint32  OUTINIT;                      /*!< (@ 0x0000005C) Initial State For Channels Output                          */
  __IOM uint32  OMCR;                         /*!< (@ 0x00000060) Output Mask Control Register                               */
  __IOM uint32  MODESEL;                      /*!< (@ 0x00000064) PWM Function Mode Selection                                */
  __IOM uint32  DTSET0;                       /*!< (@ 0x00000068) Deadtime Insertion Control 0                               */
  __IOM uint32  DTSET1;                       /*!< (@ 0x0000006C) Deadtime Insertion Control 1                               */
  __IOM uint32  EXTTRIG;                      /*!< (@ 0x00000070) PWM External Trigger                                       */
  __IOM uint32  CHOPOLCR;                     /*!< (@ 0x00000074) Channel Output Polarity Register                           */
  __IOM uint32  FDSR;                         /*!< (@ 0x00000078) Fault Detect Status Register                               */
  __IOM uint32  CAPFILTER;                    /*!< (@ 0x0000007C) Input Capture Filter Control                               */
  __IOM uint32  FFAFER;                       /*!< (@ 0x00000080) Fault Filter and Fault Enable Register                     */
  __IOM uint32  QDI;                          /*!< (@ 0x00000084) Quadrature Decoder Interface Configuration Register        */
  __IOM uint32  CONF;                         /*!< (@ 0x00000088) Configuration                                              */
  __IOM uint32  FLTPOL;                       /*!< (@ 0x0000008C) PWM Fault Input Polarity                                   */
  __IOM uint32  SYNCONF;                      /*!< (@ 0x00000090) Synchronization Configuration                              */
  __IOM uint32  INVCR;                        /*!< (@ 0x00000094) PWM Inverse Control Register                               */
  __IOM uint32  CHOSWCR;                      /*!< (@ 0x00000098) PWM CHannel Software Output Control Register               */
  __IOM uint32  DITHER0;                      /*!< (@ 0x0000009C) Dither function register 0                                 */
  __IOM uint32  DITHER1;                      /*!< (@ 0x000000A0) Dither function register 1                                 */
  __IOM uint32  DITHER2;                      /*!< (@ 0x000000A4) Dither function register 2                                 */
  __IOM uint32  DMACTRL;                      /*!< (@ 0x000000A8) DMA Control Register                                       */
} PWM_Type;                                     /*!< Size = 172 (0xac)                                                         */

/* =========================================================================================================================== */
/* ================                                         SENT_CTRL                                         ================ */
/* =========================================================================================================================== */

/**
  * @brief SENT Module Control (SENT_CTRL)
  */

typedef struct {                                /*!< (@ 0x40010000) SENT_CTRL Structure                                        */
  __IOM uint32  FDR;                          /*!< (@ 0x00000000) Fractional Divider Regitser                                */
  __IM  uint32  CTS;                          /*!< (@ 0x00000004) Current Time Stamp                                         */
  __IOM uint32  TDIV;                         /*!< (@ 0x00000008) Timestamp Pre Divider Register                             */
} SENT_CTRL_Type;                               /*!< Size = 12 (0xc)                                                           */

/* =========================================================================================================================== */
/* ================                                       SENT_CHANNEL                                        ================ */
/* =========================================================================================================================== */

/**
  * @brief SENT Channel (SENT_CHANNEL)
  */

typedef struct {                                /*!< (@ 0x40010100) SENT_CHANNEL Structure                                     */
  __IOM uint32  CPDR;                         /*!< (@ 0x00000000) Channel Pre Divider Regitser                               */
  __IOM uint32  CFDR;                         /*!< (@ 0x00000004) Channel Fractional Divider Regitser                        */
  __IOM uint32  RCR;                          /*!< (@ 0x00000008) Receive Data Control Register                              */
  __IOM uint32  RSR;                          /*!< (@ 0x0000000C) Receive Data Status Register                               */
  __IOM uint32  SDS;                          /*!< (@ 0x00000010) Serial Data and Status Register                            */
  __IOM uint32  IOCR;                         /*!< (@ 0x00000014) Input and Output Control Register                          */
  __IOM uint32  SCR;                          /*!< (@ 0x00000018) SPC Control Register                                       */
  __IOM uint32  VIEW;                         /*!< (@ 0x0000001C) Receive Data View Register                                 */
  __IOM uint32  INTSTAT;                      /*!< (@ 0x00000020) Interrupt Status Register                                  */
  __IOM uint32  INTSET;                       /*!< (@ 0x00000024) Interrupt Set Register                                     */
  __IOM uint32  INTCLR;                       /*!< (@ 0x00000028) Interrupt Clear Register                                   */
  __IOM uint32  INTEN;                        /*!< (@ 0x0000002C) Interrupt Enable Register                                  */
  __IOM uint32  FIFO;                         /*!< (@ 0x00000030) FIFO Control Register                                      */
  __IOM uint32  WDT;                          /*!< (@ 0x00000034) Watch Dog Timer Register                                   */
  __IOM uint32  RDR;                          /*!< (@ 0x00000038) Receive Data Register                                      */
} SENT_CHANNEL_Type;                            /*!< Size = 60 (0x3c)                                                          */

/* =========================================================================================================================== */
/* ================                                        TIMER_CTRL                                         ================ */
/* =========================================================================================================================== */

/**
  * @brief Timer Control (TIMER_CTRL)
  */

typedef struct {                                /*!< (@ 0x40011000) TIMER_CTRL Structure                                       */
  __IOM uint32  CR;                             /*!< (@ 0x00000000) Timer Control Regitser                                     */
  __IOM uint32  SR;                             /*!< (@ 0x00000004) Timer Status Regitser                                      */
  __IOM uint32  IER;                            /*!< (@ 0x00000008) TIMER Interrupt Enable Register                            */
  __IOM uint32  ENR;                            /*!< (@ 0x0000000C) TIMER Enable Register                                      */
} TIMER_CTRL_Type;                              /*!< Size = 16 (0x10)                                                          */

/* =========================================================================================================================== */
/* ================                                      TIMER_CHANNEL                                        ================ */
/* =========================================================================================================================== */

/**
  * @brief Timer channel  (TIMER_CHANNEL)
  */

typedef struct {                                /*!< (@ 0x40011010) TIMER_CHANNEL Structure                                    */
  __IOM uint32  TVAL;                         /*!< (@ 0x00000000) Timer Load Value Register                                  */
  __IOM uint32  CVAL;                         /*!< (@ 0x00000004) Timer Current Count Value Register                         */
  __IOM uint32  CTRL;                         /*!< (@ 0x00000008) Timer channel Control Register                             */
  __IOM uint32  TMVAL;                        /*!< (@ 0x0000000C) Timer Match Value Register                                 */
} TIMER_CHANNEL_Type;                           /*!< Size = 16 (0x10)                                                          */

/* =========================================================================================================================== */
/* ================                                            CTU                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief Connect Unit Module (CTU)
  */
#define TRGMUX_MODULE_NUM        (23U)
typedef struct {                                /*!< (@ 0x40006000) CTU Structure                                              */
  union {
  struct {
  __IOM uint32  TRGMUX_DMA;                   /*!< (@ 0x00000000) TRGMUX Config Register1                                    */
  __IOM uint32  TRGMUX_EXTOUT0;               /*!< (@ 0x00000004) TRGMUX Config Register2                                    */
  __IOM uint32  TRGMUX_EXTOUT1;               /*!< (@ 0x00000008) TRGMUX Config Register3                                    */
  __IOM uint32  TRGMUX_ADC0_REG;              /*!< (@ 0x0000000C) TRGMUX Config Register4                                    */
  __IOM uint32  TRGMUX_ADC0_INJ;              /*!< (@ 0x00000010) TRGMUX Config Register5                                    */
  __IM  uint32  RESERVED[2];
  __IOM uint32  TRGMUX_ACMP;                  /*!< (@ 0x0000001C) TRGMUX Config Register8                                    */
  __IOM uint32  TRGMUX_PWM0;                  /*!< (@ 0x00000020) TRGMUX Config Register9                                    */
  __IOM uint32  TRGMUX_PWM1;                  /*!< (@ 0x00000024) TRGMUX Config Register10                                   */
  __IOM uint32  TRGMUX_PWM2;                  /*!< (@ 0x00000028) TRGMUX Config Register11                                   */
  __IOM uint32  TRGMUX_PWM3;                  /*!< (@ 0x0000002C) TRGMUX Config Register12                                   */
  __IM  uint32  RESERVED1[4];
  __IOM uint32  TRGMUX_TIMER;                 /*!< (@ 0x00000040) TRGMUX Config Register15                                   */
  __IM  uint32  RESERVED2;
  __IOM uint32  TRGMUX_UART0;                 /*!< (@ 0x00000048) TRGMUX Config Register17                                   */
  __IOM uint32  TRGMUX_UART1;                 /*!< (@ 0x0000004C) TRGMUX Config Register18                                   */
  __IM  uint32  RESERVED3[2];
  __IOM uint32  TRGMUX_EIO;                   /*!< (@ 0x00000058) TRGMUX Config Register21                                   */
  } TRGMUX_MODULE;
  __IOM uint32  TRGMUXn[TRGMUX_MODULE_NUM];
  } TRGMUX;
  __IOM uint32  CTU_CFG;                      /*!< (@ 0x0000005C) CTU Config Register                                        */
  __IOM uint32  CTU_SW;                       /*!< (@ 0x00000060) CTU Software Trigger Register                              */
  __IOM uint32  PWM_MODULATION;               /*!< (@ 0x00000064) CTU PWM Modulation Register                                */
} CTU_Type;                                     /*!< Size = 104 (0x68)                                                         */

/* =========================================================================================================================== */
/* ================                                       DMA_TOP_RST                                         ================ */
/* =========================================================================================================================== */
/**
  * @brief DMA All Channnels Share The Registers (DMA_TOP_RST)
  */

typedef struct {                                /*!< (@ 0x40012000) DMA_TOP_RST Structure                                      */
  __IOM uint32  TOP_RST;                      /*!< (@ 0x00000000) TOP_RST Register                                           */
} DMA_TopRstType;                               /*!< Size = 4 (0x4)                                                            */

/* =========================================================================================================================== */
/* ================                                       DMA_Channel                                         ================ */
/* =========================================================================================================================== */

/**
  * @brief DMA channel (DMA_Channel)
  */

typedef struct {                                /*!< (@ 0x40012040) DMA_Channel Structure                                      */
  __IOM uint32  STATUS;                       /*!< (@ 0x00000000) Status Register                                            */
  __IOM uint32  INTEN;                        /*!< (@ 0x00000004) Interrupt Enable Register                                  */
  __IOM uint32  RST;                          /*!< (@ 0x00000008) Reset Register                                             */
  __IOM uint32  STOP;                         /*!< (@ 0x0000000C) Stop Register                                              */
  __IOM uint32  CONFIG;                       /*!< (@ 0x00000010) DMA Config Register                                        */
  __IOM uint32  CHAN_LENGTH;                  /*!< (@ 0x00000014) Channel Length Register                                    */
  __IOM uint32  SSTART_ADDR;                  /*!< (@ 0x00000018) Source Start Address Register                              */
  __IOM uint32  SEND_ADDR;                    /*!< (@ 0x0000001C) Source End Address Register                                */
  __IOM uint32  DSTART_ADDR;                  /*!< (@ 0x00000020) Destination Start Address Register                         */
  __IOM uint32  CHAN_ENABLE;                  /*!< (@ 0x00000024) Channel Enable Register                                    */
  __IOM uint32  DATA_TRANS_NUM;               /*!< (@ 0x00000028) Data Transfer Number Register                              */
  __IOM uint32  FIFO_LEFT_NUM;                /*!< (@ 0x0000002C) Internal FIFO Data Left Number Register                    */
  __IOM uint32  DEND_ADDR;                    /*!< (@ 0x00000030) Destination End Address Register                           */
  __IOM uint32  ADDR_OFFSET;                  /*!< (@ 0x00000034) Address Offset Register                                    */
  __IOM uint32  DMAMUX_CFG;                   /*!< (@ 0x00000038) DMAMUX Configuration Register                              */
} DMA_ChannelType;                              /*!< Size = 60 (0x3c)                                                          */

/* =========================================================================================================================== */
/* ================                                            WDG                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief Watchdog (WDG)
  */

typedef struct {                                /*!< (@ 0x4000B000) WDG Structure                                              */
  __IOM uint32  CS0;                            /*!< (@ 0x00000000) Watchdog Control and Status Register 0                     */
  __IOM uint32  CS1;                            /*!< (@ 0x00000004) Watchdog Control and Status Register 1                     */
  __IOM uint32  CNT;                            /*!< (@ 0x00000008) Watchdog Counter Value                                     */
  __IOM uint32  TOVAL;                          /*!< (@ 0x0000000C) Watchdog Timeout Value Register                            */
  __IOM uint32  WIN;                            /*!< (@ 0x00000010) Watchdog Window Register                                   */
} WDG_Type;                                     /*!< Size = 20 (0x14)                                                          */

/* =========================================================================================================================== */
/* ================                                            RTC                                            ================ */
/* =========================================================================================================================== */
/**
  * @brief Real-time counter (RTC)
  */

typedef struct {                                /*!< (@ 0x40009800) RTC Structure                                              */
  __IOM uint32  CTRL;                           /*!< (@ 0x00000000) RTC Control Register                                       */
  __IOM uint32  TAR;                            /*!< (@ 0x00000004) RTC Time Alarm Register                                    */
  __IOM uint32  TC;                             /*!< (@ 0x00000008) RTC Time Counter                                           */
  __IOM uint32  PSR;                            /*!< (@ 0x0000000C) RTC Prescaler Register                                     */
  __IM  uint32  PSC;                            /*!< (@ 0x00000010) RTC Prescaler Counter Register                             */
  __IOM uint32  SR;                             /*!< (@ 0x00000014) RTC Status Register                                        */
  __IOM uint32  LR;                             /*!< (@ 0x00000018) RTC Lock Register                                          */
} RTC_Type;                                     /*!< Size = 28 (0x1c)                                                          */

/* =========================================================================================================================== */
/* ================                                            CRC                                            ================ */
/* =========================================================================================================================== */
/**
  * @brief CRC Cyclic redundancy check (CRC)
  */

typedef struct {                                /*!< (@ 0x40084000) CRC Structure                                              */
  union {
      __IOM uint32 DATA32;                      /*!< (@ 0x00000000) DATA Register                                              */
      struct {
        __IOM uint16 L;                         /*!< (@ 0x00000000) DATA L Register                                            */
        __IOM uint16 H;                         /*!< (@ 0x00000002) DATA H Register                                            */
      } DATA16;
      struct {
        __IOM uint8 LL;                         /*!< (@ 0x00000000) DATA LL Register                                           */
        __IOM uint8 LU;                         /*!< (@ 0x00000001) DATA LU Register                                           */
        __IOM uint8 HL;                         /*!< (@ 0x00000002) DATA HL Register                                           */
        __IOM uint8 HU;                         /*!< (@ 0x00000003) DATA HU Register                                           */
      } DATA8;
  } DATAn;
  __IOM uint32  POLY;                           /*!< (@ 0x00000004) Poly Register                                              */
  __IOM uint32  CTRL;                           /*!< (@ 0x00000008) Control Register                                           */
} CRC_Type;                                     /*!< Size = 12 (0xc)                                                           */

/* =========================================================================================================================== */
/* ================                                         EIM_CTRL                                          ================ */
/* =========================================================================================================================== */
/**
  * @brief GLB_EN (EIM_CTRL)
  */

typedef struct {                                /*!< (@ 0x40088000) EIM_CTRL Structure                                         */
  __IOM  uint32  EIM_GLB_ENABLE;                /*!< (@ 0x00000000) EIM global enable                                          */
  __IOM  uint32  EIM_CHEN;                      /*!< (@ 0x00000004) EIM channel enable control                                 */
} EIM_CTRL_Type;                                /*!< Size = 8 (0x8)                                                            */

/* =========================================================================================================================== */
/* ================                                       EIM_CHANNEL                                         ================ */
/* =========================================================================================================================== */

/**
  * @brief SRAM Error Injection Bit Control (EIM_CHANNEL)
  */

typedef struct {                                /*!< (@ 0x40088010) EIM_CHANNEL Structure                                      */
  __IOM uint32  EIM_INJ_DATA;                   /*!< (@ 0x00000000) SRAML data injection control                               */
  __IOM uint32  EIM_INJ_ADDR;                   /*!< (@ 0x00000004) SRAML address injection control                            */
  __IOM uint32  EIM_INJ_ECC;                    /*!< (@ 0x00000008) SRAML ECC code injection control                           */
} EIM_CHANNEL_Type;                             /*!< Size = 12 (0xc)                                                           */

/* =========================================================================================================================== */
/* ================                                         ECC_SRAM                                          ================ */
/* =========================================================================================================================== */

/**
  * @brief SRAM Error ctrl (ECC_SRAM)
  */

typedef struct {                                /*!< (@ 0x40088100) ECC_SRAM Structure                                         */
  __IM  uint32  CH0_STATUS0;                    /*!< (@ 0x00000000) SRAM ECC CH0 status and addr0 register                     */
  __IM  uint32  CH0_STATUS1;                    /*!< (@ 0x00000004) ECC CH0 2bit error addr                                    */
  __IM  uint32  CH1_STATUS0;                    /*!< (@ 0x00000008) SRAM ECC CH1 status and addr0 register                     */
  __IM  uint32  CH1_STATUS1;                    /*!< (@ 0x0000000C) ECC CH1 2bit error addr                                    */
  __IOM uint32  ECC_ERR_CTRL;                   /*!< (@ 0x00000010) ECC Error status control registers                         */
} ECC_SRAM_Type;                                /*!< Size = 20 (0x14)                                                          */

/* =========================================================================================================================== */
/* ================                                          FLASH                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief Embedded Flash (FLASH)
  */

typedef struct {                                /*!< (@ 0x40002000) FLASH Structure                                            */
  __IOM uint32  STAT;                         /*!< (@ 0x00000000) Flash status register                                      */
  __IM  uint32  SEC;                          /*!< (@ 0x00000004) MCU secure register                                        */
  __IM  uint32  RESERVED;
  __IOM uint32  KEYUNLK;                      /*!< (@ 0x0000000C) Flash unlock register                                      */
  __IOM uint32  CNFG;                         /*!< (@ 0x00000010) Flash configure register                                   */
  __IOM uint32  CMD;                          /*!< (@ 0x00000014) Flash command ID register                                  */
  __IOM uint32  ADDR;                         /*!< (@ 0x00000018) Flash address register                                     */
  __IOM uint32  DATA0;                        /*!< (@ 0x0000001C) Flash data register 0                                      */
  __IOM uint32  DATA1;                        /*!< (@ 0x00000020) Flash data register 1                                      */
  __IOM uint32  LEN;                          /*!< (@ 0x00000024) Flash length register                                      */
  __IOM uint32  CMDCFG;                       /*!< (@ 0x00000028) Flash command configure register                           */
  __IM  uint32  RESERVED1;
  __IOM uint32  CST;                          /*!< (@ 0x00000030) Flash command start register                               */
  __IOM uint32  PPROT;                        /*!< (@ 0x00000034) P-Flash write protection register 0                        */
  __IOM uint32  PPROT1;                       /*!< (@ 0x00000038) P-Flash write protection register 1                        */
  __IOM uint32  DPROT;                        /*!< (@ 0x0000003C) D-Flash write protection register                          */
  __IOM uint32  DFADR;                        /*!< (@ 0x00000040) Flash ECC 2-bit error address register                     */
  __IM  uint32  RESERVED2[12];
  __IM  uint32  SWAPST;                       /*!< (@ 0x00000074) Flash swap status register                                 */
  __IM  uint32  LKSTS;                        /*!< (@ 0x00000078) Chip lock status register                                  */
  __IM  uint32  RESERVED3;
  __IM  uint32  CFGDEC;                       /*!< (@ 0x00000080) OTP configuration decode register                          */
  __IM  uint32  RESERVED4[8];
  __IOM uint32  SECCTL;                       /*!< (@ 0x000000A4) Chip security control register                             */
  __IOM uint32  CFGSEL;                       /*!< (@ 0x000000A8) Chip OTP configuration selection register                  */
  __IM  uint32  CFGVIEW;                      /*!< (@ 0x000000AC) Chip OTP configuration view register                       */
} FLASH_Type;                                 /*!< Size = 176 (0xB0)                                                         */

/* =========================================================================================================================== */
/* ================                                            EIO                                            ================ */
/* =========================================================================================================================== */
/**
  * @brief Enhanced Input and Output (EIO)
  */

typedef struct {                                /*!< (@ 0x4000A000) EIO Structure                                              */
  __IOM uint32  CTRL;                         /*!< (@ 0x00000000) EIO Control Register                                       */
  __IM  uint32  PIN;                          /*!< (@ 0x00000004) Pin State Register                                         */
  __IOM uint32  SHIFTSTAT;                    /*!< (@ 0x00000008) Shifter Status Register                                    */
  __IOM uint32  SHIFTERR;                     /*!< (@ 0x0000000C) Shifter Error Register                                     */
  __IOM uint32  TIMSTAT;                      /*!< (@ 0x00000010) Timer Status Register                                      */
  __IOM uint32  SHIFTSIEN;                    /*!< (@ 0x00000014) Shifter Status Interrupt Enable                            */
  __IOM uint32  SHIFTEIEN;                    /*!< (@ 0x00000018) Shifter Error Interrupt Enable                             */
  __IOM uint32  TIMIEN;                       /*!< (@ 0x0000001C) Timer Interrupt Enable Register                            */
  __IOM uint32  SHIFTSDEN;                    /*!< (@ 0x00000020) Shifter Status DMA Enable                                  */
  __IM  uint32  RESERVED[3];
  __IOM uint32  SHIFTCTL[4];                  /*!< (@ 0x00000030) Shifter Control N Register                                 */
  __IOM uint32  SHIFTCFG[4];                  /*!< (@ 0x00000040) Shifter Configuration N Register                           */
  __IOM uint32  SHIFTBUF[4];                  /*!< (@ 0x00000050) Shifter Buffer N Register                                  */
  __IOM uint32  SHIFTBUFBIS[4];               /*!< (@ 0x00000060) Shifter Buffer N Bit Swapped Register                      */
  __IOM uint32  SHIFTBUFBYS[4];               /*!< (@ 0x00000070) Shifter Buffer N Byte Swapped Register                     */
  __IOM uint32  SHIFTBUFBBS[4];               /*!< (@ 0x00000080) Shifter Buffer N Bit Byte Swapped Register                 */
  __IOM uint32  TIMCTL[4];                    /*!< (@ 0x00000090) Timer Control N Register                                   */
  __IOM uint32  TIMCFG[4];                    /*!< (@ 0x000000A0) Timer Configuration N Register                             */
  __IOM uint32  TIMCMP[4];                    /*!< (@ 0x000000B0) Timer Compare N Register                                   */
} EIO_Type;                                     /*!< Size = 192 (0xc0)                                                         */

/* =========================================================================================================================== */
/* ================                                      AC784X_SysTick                                       ================ */
/* =========================================================================================================================== */

/**
  * @brief The AC784X_SysTick Memory Map/Register Definition can be found here. (AC784X_SysTick)
  */

typedef struct {                                /*!< (@ 0xE000E010) AC784X_SysTick Structure                                   */
  __IOM uint32  CSR;                          /*!< (@ 0x00000000) SysTick Control and Status Register, offset: 0x0           */
  __IOM uint32  RVR;                          /*!< (@ 0x00000004) SysTick Reload Value Register, offset: 0x4                 */
  __IOM uint32  CVR;                          /*!< (@ 0x00000008) SysTick Current Value Register, offset: 0x8                */
  __IOM uint32  CALIB;                        /*!< (@ 0x0000000C) SysTick Calibration Value Register, offset: 0xC            */
} AC784X_SysTickType;                           /*!< Size = 16 (0x10)                                                          */

/* =========================================================================================================================== */
/* ================                                            SMU                                            ================ */
/* =========================================================================================================================== */
/**
  * @brief Safety Management Uint (SMU)
  */

typedef struct {                                /*!< (@ 0x40089000) SMU Structure                                              */
  __IOM uint32  SFES;                         /*!< (@ 0x00000000) Single Point Fault Error Status Register                   */
  __IOM uint32  SFESS;                        /*!< (@ 0x00000004) Single Point Fault Error Status Shadow Register            */
  __IOM uint32  LFES;                         /*!< (@ 0x00000008) Latent Fault Error Status Register                         */
  __IOM uint32  LFESS;                        /*!< (@ 0x0000000C) Latent Fault Error Status Shadow Register                  */
  __IOM uint32  LKSEQ0;                       /*!< (@ 0x00000010) Lock Sequence for Single Point Fault Path                  */
  __IOM uint32  LKSEQ1;                       /*!< (@ 0x00000014) Lock Sequence for Latent Fault Path                        */
  __IOM uint32  SWSFE;                        /*!< (@ 0x00000018) Software Single Point Fault Enable Register                */
  __IOM uint32  SWSFES;                       /*!< (@ 0x0000001C) Software Single Point Fault Enable Shadow Register         */
  __IOM uint32  SWLFE;                        /*!< (@ 0x00000020) Software Latent Fault Enable Register                      */
  __IOM uint32  SWLFES;                       /*!< (@ 0x00000024) Software Latent Fault Enable Shadow Register               */
  __IOM uint32  SFINTEN;                      /*!< (@ 0x00000028) Single Point Fault Interrupt Enable Register               */
  __IOM uint32  LFINTEN;                      /*!< (@ 0x0000002C) Latent Fault Interrupt Enable Register                     */
  __IOM uint32  SFRSTEN;                      /*!< (@ 0x00000030) Single Point Fault Reset Enable Register                   */
  __IOM uint32  LFRSTEN;                      /*!< (@ 0x00000034) Latent Fault Reset Request Enable Register                 */
  __IOM uint32  SRSTCNTVAL;                   /*!< (@ 0x00000038) System Reset Request Counter Threshold Register            */
  __IOM uint32  SRSTCNT;                      /*!< (@ 0x0000003C) System Reset Request Counter Register                      */
  __IOM uint32  SRSTCNTS;                     /*!< (@ 0x00000040) System Reset Request Counter Shadow Register               */
  __IOM uint32  PATHCHK0;                     /*!< (@ 0x00000044) Path Check for Single Point Fault Path Register            */
  __IOM uint32  PATHCHK1;                     /*!< (@ 0x00000048) Path Check for Latent Fault Error Path Register            */
  __IOM uint32  FSPCR;                        /*!< (@ 0x0000004C) FSP Control Register                                       */
  __IOM uint32  FSPSFCR;                      /*!< (@ 0x00000050) FSP Single Point Fault Control Register                    */
  __IOM uint32  FSPLFCR;                      /*!< (@ 0x00000054) FSP Latent Fault Control Register                          */
} SMU_Type;                                     /*!< Size = 88 (0x58)                                                          */

/* =========================================================================================================================== */
/* ================                                         CMU                                               ================ */
/* =========================================================================================================================== */
/**
  * @brief Clock Monitor Uint (CMU)
  */

typedef struct {                                /*!< (@ 0x40000800) CMU Structure                                              */
  __IOM uint32  CR;                           /*!< (@ 0x00000000) Configure Register                                         */
  __IOM uint32  RCCR;                         /*!< (@ 0x00000004) Reference Count Configure Register                         */
  __IOM uint32  HTCR;                         /*!< (@ 0x00000008) High Threshold Configure Register                          */
  __IOM uint32  LTCR;                         /*!< (@ 0x0000000C) Low Threshold Configure Register                           */
  __IM  uint32  RESERVED[2];
  __IOM uint32  SR;                           /*!< (@ 0x00000018) Status Register                                            */
  __IOM uint32  REFCNT;                       /*!< (@ 0x0000001C) Reference Count Register                                   */
  __IOM uint32  MONCNT;                       /*!< (@ 0x00000020) Monitor Count Register                                     */
} CMU_Type;                                     /*!< Size = 36 (0x24)                                                          */

/* =========================================================================================================================== */
/* ================                                            HSM                                            ================ */
/* =========================================================================================================================== */

/**
  * @brief Hardware secure module (HSM)
  */

typedef struct {                                /*!< (@ 0x4008B000) HSM Structure                                              */
  __IM  uint32  UID0;                         /*!< (@ 0x00000000) HSM UID0 register                                          */
  __IM  uint32  UID1;                         /*!< (@ 0x00000004) HSM UID1 register                                          */
  __IM  uint32  UID2;                         /*!< (@ 0x00000008) HSM UID2 register                                          */
  __IM  uint32  UID3;                         /*!< (@ 0x0000000C) HSM UID3 register                                          */
  __IOM uint32  CMD;                          /*!< (@ 0x00000010) HSM command register                                       */
  __IOM uint32  CST;                          /*!< (@ 0x00000014) HSM trigger register                                       */
  __IOM uint32  SYS_CTRL;                     /*!< (@ 0x00000018) HSM system control register                                */
  __IOM uint32  SYS_STAT;                     /*!< (@ 0x0000001C) HSM system status register                                 */
  __IM  uint32  VRY_STAT;                     /*!< (@ 0x00000020) HSM verify status register                                 */
  __IOM uint32  VRY_STAT_CLR;                 /*!< (@ 0x00000024) HSM verify status clear register                           */
  __IOM uint32  AUTH_CTRL;                    /*!< (@ 0x00000028) HSM authentication control register                        */
  __IM  uint32  AUTH_STAT;                    /*!< (@ 0x0000002C) HSM authentication status                                  */
  __IOM uint32  AUTH_STAT_CLR;                /*!< (@ 0x00000030) HSM authentication status clear                            */
  __IOM uint32  CIPIN;                        /*!< (@ 0x00000034) HSM input data                                             */
  __IOM uint32  CIPOUT;                       /*!< (@ 0x00000038) HSM output data                                            */
  __IOM uint32  IV0;                          /*!< (@ 0x0000003C) IV 0                                                       */
  __IOM uint32  IV1;                          /*!< (@ 0x00000040) IV 1                                                       */
  __IOM uint32  IV2;                          /*!< (@ 0x00000044) IV 2                                                       */
  __IOM uint32  IV3;                          /*!< (@ 0x00000048) IV 3                                                       */
  __IOM uint32  RAM_KEY0;                     /*!< (@ 0x0000004C) Ram key0                                                   */
  __IOM uint32  RAM_KEY1;                     /*!< (@ 0x00000050) Ram key1                                                   */
  __IOM uint32  RAM_KEY2;                     /*!< (@ 0x00000054) Ram key2                                                   */
  __IOM uint32  RAM_KEY3;                     /*!< (@ 0x00000058) Ram key3                                                   */
  __IOM uint32  RAM_KEY4;                     /*!< (@ 0x0000005C) Ram key4                                                   */
  __IOM uint32  RAM_KEY5;                     /*!< (@ 0x00000060) Ram 5                                                      */
  __IOM uint32  RAM_KEY6;                     /*!< (@ 0x00000064) Ram key6                                                   */
  __IOM uint32  RAM_KEY7;                     /*!< (@ 0x00000068) Ram key7                                                   */
  __IOM uint32  CHA0;                         /*!< (@ 0x0000006C) Auth challenge0                                            */
  __IOM uint32  CHA1;                         /*!< (@ 0x00000070) Auth challenge1                                            */
  __IOM uint32  CHA2;                         /*!< (@ 0x00000074) Auth challenge2                                            */
  __IOM uint32  CHA3;                         /*!< (@ 0x00000078) Auth challenge3                                            */
  __IOM uint32  RSP0;                         /*!< (@ 0x0000007C) Auth response0                                             */
  __IOM uint32  RSP1;                         /*!< (@ 0x00000080) Auth response1                                             */
  __IOM uint32  RSP2;                         /*!< (@ 0x00000084) Auth response2                                             */
  __IOM uint32  RSP3;                         /*!< (@ 0x00000088) Auth response3                                             */
  __IOM uint32  MAC0;                         /*!< (@ 0x0000008C) Mac0                                                       */
  __IOM uint32  MAC1;                         /*!< (@ 0x00000090) Mac1                                                       */
  __IOM uint32  MAC2;                         /*!< (@ 0x00000094) Mac2                                                       */
  __IOM uint32  MAC3;                         /*!< (@ 0x00000098) Mac3                                                       */
  __IOM uint32  KAC0;                         /*!< (@ 0x0000009C) Key auth code 0                                            */
  __IOM uint32  KAC1;                         /*!< (@ 0x000000A0) Key auth code 1                                            */
  __IOM uint32  KAC2;                         /*!< (@ 0x000000A4) Key auth code 2                                            */
  __IOM uint32  KAC3;                         /*!< (@ 0x000000A8) Key auth code 3                                            */
  __IOM uint32  KAC4;                         /*!< (@ 0x000000AC) Key auth code 4                                            */
  __IOM uint32  KAC5;                         /*!< (@ 0x000000B0) Key auth code 5                                            */
  __IOM uint32  KAC6;                         /*!< (@ 0x000000B4) Key auth code 6                                            */
  __IOM uint32  KAC7;                         /*!< (@ 0x000000B8) Key auth code 7                                            */
  __IOM uint32  STAT;                         /*!< (@ 0x000000BC) HSM status register                                        */
  __IOM uint32  STAT_CLR;                     /*!< (@ 0x000000C0) HSM error status clear                                     */
  __IM  uint32  CHIP_STAT;                    /*!< (@ 0x000000C4) CHIP status                                                */
  __IOM uint32  FLUSH;                        /*!< (@ 0x000000C8) Flush hsm fifo                                             */
} HSM_Type;                                     /*!< Size = 204 (0xcc)                                                         */

/** @} */ /* End of group Device_Peripheral_peripherals */

/* =========================================================================================================================== */
/* ================                          Device Specific Peripheral Address Map                           ================ */
/* =========================================================================================================================== */
#define PFLASH_BASE                 (0x00000000U)                  /*!< Base address of P-FLASH Main */
#define PFLASH_END                  (0x0008FFFFU)                  /*!< End address of P-FLASH Main */

#define DFLASH_BASE                 (0x01000000U)                  /*!< Base address of D-FLASH Main */
#define DFLASH_END                  (0x0101FFFFU)                  /*!< End address of D-FLASH Main */
#define DFLASH_INFO_BASE            (0x01022000U)                  /*!< Base address of D-FLASH Main */
#define DFLASH_INFO_END             (0x01023FFFU)                  /*!< End address of D-FLASH Main */

#define SRAM_L_BASE                 (0x1FFF8000U)                  /*!< Base address of SRAM_L */
#define SRAM_L_END                  (0x1FFFFFFFU)                  /*!< End address of SRAM_L*/

#define SRAM_U_BASE                 (0x20000000U)                  /*!< Base address of SRAM_U */
#define SRAM_U_END                  (0x20007FFFU)                  /*!< End address of SRAM_U  */

#define PERIPH_BASE                 (0x40000000U)                  /*!< Base address of Peripheral */
#define PERIPH_END                  (0x400FFFFFU)                  /*!< End address of Peripheral */

/*!< D-FLASH Infos */
#define WP_INFO_BASE                (DFLASH_INFO_BASE + 0x00001C00U)

#define UUID_BASE                   (DFLASH_INFO_BASE + 0x00001800U)
#define TYPEID_BASE                 (DFLASH_INFO_BASE + 0x00001820U)

#define CKGEN_BASE                  (PERIPH_BASE + 0x0000000U)
#define PBR_BASE                    (PERIPH_BASE + 0x008A000U)
#define SPM_BASE                    (PERIPH_BASE + 0x0008000U)
#define GPIOA_BASE                  (PERIPH_BASE + 0x0085000U)
#define GPIOB_BASE                  (PERIPH_BASE + 0x0085040U)
#define GPIOC_BASE                  (PERIPH_BASE + 0x0085080U)
#define GPIOD_BASE                  (PERIPH_BASE + 0x00850C0U)
#define GPIOE_BASE                  (PERIPH_BASE + 0x0085100U)
#define PORTA_BASE                  (PERIPH_BASE + 0x0085200U)
#define PORTB_BASE                  (PERIPH_BASE + 0x0085300U)
#define PORTC_BASE                  (PERIPH_BASE + 0x0085400U)
#define PORTD_BASE                  (PERIPH_BASE + 0x0085500U)
#define PORTE_BASE                  (PERIPH_BASE + 0x0085600U)
#define CAN0_BASE                   (PERIPH_BASE + 0x0007000U)
#define CAN1_BASE                   (PERIPH_BASE + 0x0007200U)
#define UART0_BASE                  (PERIPH_BASE + 0x0018000U)
#define UART1_BASE                  (PERIPH_BASE + 0x0018200U)
#define UART2_BASE                  (PERIPH_BASE + 0x0018400U)
#define I2C0_BASE                   (PERIPH_BASE + 0x000E000U)
#define I2C1_BASE                   (PERIPH_BASE + 0x000E800U)
#define SPI0_BASE                   (PERIPH_BASE + 0x000C000U)
#define SPI1_BASE                   (PERIPH_BASE + 0x000C800U)
#define SPI2_BASE                   (PERIPH_BASE + 0x000D000U)
#define ANA_BASE                    (PERIPH_BASE + 0x0008800U)
#define ADC0_BASE                   (PERIPH_BASE + 0x0003000U)
#define ACMP0_BASE                  (PERIPH_BASE + 0x0005000U)
#define ABIST_BASE                  (PERIPH_BASE + 0x0005800U)
#define PWM0_BASE                   (PERIPH_BASE + 0x0080000U)
#define PWM1_BASE                   (PERIPH_BASE + 0x0080800U)
#define PWM2_BASE                   (PERIPH_BASE + 0x0081000U)
#define PWM3_BASE                   (PERIPH_BASE + 0x0081800U)
#define SENT_CTRL_BASE              (PERIPH_BASE + 0x0010000U)
#define SENT_CHANNEL0_BASE          (PERIPH_BASE + 0x0010100U)
#define SENT_CHANNEL1_BASE          (PERIPH_BASE + 0x0010200U)
#define TIMER_CTRL_BASE             (PERIPH_BASE + 0x0011000U)
#define TIMER_CHANNEL0_BASE         (PERIPH_BASE + 0x0011010U)
#define TIMER_CHANNEL1_BASE         (PERIPH_BASE + 0x0011020U)
#define TIMER_CHANNEL2_BASE         (PERIPH_BASE + 0x0011030U)
#define TIMER_CHANNEL3_BASE         (PERIPH_BASE + 0x0011040U)
#define CTU_BASE                    (PERIPH_BASE + 0x0006000U)
#define DMA0_TOP_RST_BASE           (PERIPH_BASE + 0x0012000U)
#define DMA0_CHANNEL0_BASE          (PERIPH_BASE + 0x0012040U)
#define DMA0_CHANNEL1_BASE          (PERIPH_BASE + 0x0012080U)
#define DMA0_CHANNEL2_BASE          (PERIPH_BASE + 0x00120C0U)
#define DMA0_CHANNEL3_BASE          (PERIPH_BASE + 0x0012100U)
#define DMA0_CHANNEL4_BASE          (PERIPH_BASE + 0x0012140U)
#define DMA0_CHANNEL5_BASE          (PERIPH_BASE + 0x0012180U)
#define DMA0_CHANNEL6_BASE          (PERIPH_BASE + 0x00121C0U)
#define DMA0_CHANNEL7_BASE          (PERIPH_BASE + 0x0012200U)
#define DMA0_CHANNEL8_BASE          (PERIPH_BASE + 0x0012240U)
#define DMA0_CHANNEL9_BASE          (PERIPH_BASE + 0x0012280U)
#define DMA0_CHANNEL10_BASE         (PERIPH_BASE + 0x00122C0U)
#define DMA0_CHANNEL11_BASE         (PERIPH_BASE + 0x0012300U)
#define DMA0_CHANNEL12_BASE         (PERIPH_BASE + 0x0012340U)
#define DMA0_CHANNEL13_BASE         (PERIPH_BASE + 0x0012380U)
#define DMA0_CHANNEL14_BASE         (PERIPH_BASE + 0x00123C0U)
#define DMA0_CHANNEL15_BASE         (PERIPH_BASE + 0x0012400U)
#define WDG_BASE                    (PERIPH_BASE + 0x000B000U)
#define RTC_BASE                    (PERIPH_BASE + 0x0009800U)
#define CRC_BASE                    (PERIPH_BASE + 0x0084000U)
#define EIM_CTRL_BASE               (PERIPH_BASE + 0x0088000U)
#define EIM_CHANNEL0_BASE           (PERIPH_BASE + 0x0088010U)
#define EIM_CHANNEL1_BASE           (PERIPH_BASE + 0x0088080U)
#define ECC_SRAM_BASE               (PERIPH_BASE + 0x0088100U)
#define FLASH_BASE                  (PERIPH_BASE + 0x0002000U)
#define EIO_BASE                    (PERIPH_BASE + 0x000A000U)
#define AC784X_SYSTICK_BASE         (0xE000E010U)
#define MPU_BASE                    (PERIPH_BASE + 0x0087000U)
#define SMU_BASE                    (PERIPH_BASE + 0x0089000U)
#define CMU_VHSI_BASE               (PERIPH_BASE + 0x0000800U)
#define CMU_HSE_BASE                (PERIPH_BASE + 0x0000A00U)
#define CMU_PLL_BASE                (PERIPH_BASE + 0x0000C00U)
#define CMU_PLL1_BASE               (PERIPH_BASE + 0x0000D00U)
#define HSM_BASE                    (PERIPH_BASE + 0x008B000U)
#define MCM_BASE                    (0xE008000CU)
/** @} */ /* End of group Device_Peripheral_peripheralAddr */

/* Flash block sizes for different IC variants of the same model */
#define PFLASH_BLOCK_SIZE           (0x00090000U)                  /*!< P-Flash size in byte (Variant Z) */
#define DFLASH_BLOCK_SIZE           (0x00020000U)                  /*!< D-Flash size in byte (Variant Z) */
#define PFLASH_BLOCK_SIZE2          (0x00050000U)                  /*!< P-Flash size in byte (Variant Y) */
#define DFLASH_BLOCK_SIZE2          (0x00010000U)                  /*!< D-Flash size in byte (Variant Y) */
#define DFLASH_SWAP_BLOCK_SIZE      (0x00000800U)                  /*!< SWAP config page size in byte */
/* =========================================================================================================================== */
/* ================                                  Peripheral declaration                                   ================ */
/* =========================================================================================================================== */

/** @addtogroup Device_Peripheral_declaration
  * @{
  */

#define MCM                         ((MCM_Type*)               MCM_BASE)
#define MPU                         ((MPU_Type*)               MPU_BASE)
#define CKGEN                       ((CKGEN_Type*)             CKGEN_BASE)
#define PBR                         ((PBR_Type*)               PBR_BASE)
#define SPM                         ((SPM_Type*)               SPM_BASE)
#define GPIOA                       ((GPIO_Type*)              GPIOA_BASE)
#define GPIOB                       ((GPIO_Type*)              GPIOB_BASE)
#define GPIOC                       ((GPIO_Type*)              GPIOC_BASE)
#define GPIOD                       ((GPIO_Type*)              GPIOD_BASE)
#define GPIOE                       ((GPIO_Type*)              GPIOE_BASE)
#define PORTA                       ((PORT_Type*)              PORTA_BASE)
#define PORTB                       ((PORT_Type*)              PORTB_BASE)
#define PORTC                       ((PORT_Type*)              PORTC_BASE)
#define PORTD                       ((PORT_Type*)              PORTD_BASE)
#define PORTE                       ((PORT_Type*)              PORTE_BASE)
#define CAN0                        ((CAN_Type*)               CAN0_BASE)
#define CAN1                        ((CAN_Type*)               CAN1_BASE)
#define UART0                       ((UART_Type*)              UART0_BASE)
#define UART1                       ((UART_Type*)              UART1_BASE)
#define UART2                       ((UART_Type*)              UART2_BASE)
#define I2C0                        ((I2C_Type*)               I2C0_BASE)
#define I2C1                        ((I2C_Type*)               I2C1_BASE)
#define SPI0                        ((SPI_Type*)               SPI0_BASE)
#define SPI1                        ((SPI_Type*)               SPI1_BASE)
#define SPI2                        ((SPI_Type*)               SPI2_BASE)
#define ANA                         ((ANA_Type*)               ANA_BASE)
#define ADC0                        ((ADC_Type*)               ADC0_BASE)
#define ACMP0                       ((ACMP_Type*)              ACMP0_BASE)
#define ABIST                       ((ABIST_Type*)             ABIST_BASE)
#define PWM0                        ((PWM_Type*)               PWM0_BASE)
#define PWM1                        ((PWM_Type*)               PWM1_BASE)
#define PWM2                        ((PWM_Type*)               PWM2_BASE)
#define PWM3                        ((PWM_Type*)               PWM3_BASE)
#define SENT_CTRL                   ((SENT_CTRL_Type*)         SENT_CTRL_BASE)
#define SENT_CHANNEL0               ((SENT_CHANNEL_Type*)      SENT_CHANNEL0_BASE)
#define SENT_CHANNEL1               ((SENT_CHANNEL_Type*)      SENT_CHANNEL1_BASE)
#define TIMER_CTRL                  ((TIMER_CTRL_Type*)        TIMER_CTRL_BASE)
#define TIMER_CHANNEL0              ((TIMER_CHANNEL_Type*)     TIMER_CHANNEL0_BASE)
#define TIMER_CHANNEL1              ((TIMER_CHANNEL_Type*)     TIMER_CHANNEL1_BASE)
#define TIMER_CHANNEL2              ((TIMER_CHANNEL_Type*)     TIMER_CHANNEL2_BASE)
#define TIMER_CHANNEL3              ((TIMER_CHANNEL_Type*)     TIMER_CHANNEL3_BASE)
#define CTU                         ((CTU_Type*)               CTU_BASE)
#define DMA0_TOP_RST                ((DMA_TopRstType*)         DMA0_TOP_RST_BASE)
#define DMA0_CHANNEL0               ((DMA_ChannelType*)        DMA0_CHANNEL0_BASE)
#define DMA0_CHANNEL1               ((DMA_ChannelType*)        DMA0_CHANNEL1_BASE)
#define DMA0_CHANNEL2               ((DMA_ChannelType*)        DMA0_CHANNEL2_BASE)
#define DMA0_CHANNEL3               ((DMA_ChannelType*)        DMA0_CHANNEL3_BASE)
#define DMA0_CHANNEL4               ((DMA_ChannelType*)        DMA0_CHANNEL4_BASE)
#define DMA0_CHANNEL5               ((DMA_ChannelType*)        DMA0_CHANNEL5_BASE)
#define DMA0_CHANNEL6               ((DMA_ChannelType*)        DMA0_CHANNEL6_BASE)
#define DMA0_CHANNEL7               ((DMA_ChannelType*)        DMA0_CHANNEL7_BASE)
#define DMA0_CHANNEL8               ((DMA_ChannelType*)        DMA0_CHANNEL8_BASE)
#define DMA0_CHANNEL9               ((DMA_ChannelType*)        DMA0_CHANNEL9_BASE)
#define DMA0_CHANNEL10              ((DMA_ChannelType*)        DMA0_CHANNEL10_BASE)
#define DMA0_CHANNEL11              ((DMA_ChannelType*)        DMA0_CHANNEL11_BASE)
#define DMA0_CHANNEL12              ((DMA_ChannelType*)        DMA0_CHANNEL12_BASE)
#define DMA0_CHANNEL13              ((DMA_ChannelType*)        DMA0_CHANNEL13_BASE)
#define DMA0_CHANNEL14              ((DMA_ChannelType*)        DMA0_CHANNEL14_BASE)
#define DMA0_CHANNEL15              ((DMA_ChannelType*)        DMA0_CHANNEL15_BASE)
#define WDG                         ((WDG_Type*)               WDG_BASE)
#define RTC                         ((RTC_Type*)               RTC_BASE)
#define CRC                         ((CRC_Type*)               CRC_BASE)
#define EIM_CTRL                    ((EIM_CTRL_Type*)          EIM_CTRL_BASE)
#define EIM_CHANNEL0                ((EIM_CHANNEL_Type*)       EIM_CHANNEL0_BASE)
#define EIM_CHANNEL1                ((EIM_CHANNEL_Type*)       EIM_CHANNEL1_BASE)
#define ECC_SRAM                    ((ECC_SRAM_Type*)          ECC_SRAM_BASE)
#define FLASH                       ((FLASH_Type*)             FLASH_BASE)
#define EIO                         ((EIO_Type*)               EIO_BASE)
#define AC784X_SYSTICK              ((AC784X_SysTickType*)     AC784X_SYSTICK_BASE)
#define SMU                         ((SMU_Type*)               SMU_BASE)
#define CMU_VHSI                    ((CMU_Type*)               CMU_VHSI_BASE)
#define CMU_HSE                     ((CMU_Type*)               CMU_HSE_BASE)
#define CMU_PLL                     ((CMU_Type*)               CMU_PLL_BASE)
#define CMU_PLL1                    ((CMU_Type*)               CMU_PLL1_BASE)
#define HSM                         ((HSM_Type *)              HSM_BASE)

/** @} */ /* End of group Device_Peripheral_declaration */

/* =========================================================================================================================== */
/* ================                                Pos/Mask Peripheral Section                                ================ */
/* =========================================================================================================================== */

/** @addtogroup PosMask_peripherals
  * @{
  */
/* =========================================================================================================================== */
/* ================                                            MCM                                            ================ */
/* =========================================================================================================================== */

/* =========================================================  MCPCR  ========================================================= */
#define MCM_MCPCR_SAPU_Pos                (24U)                    /*!< SAPU (Bit 24)                                         */
#define MCM_MCPCR_SAPU_Msk                (0x3000000U)             /*!< SAPU (Bitfield-Mask: 0x03)                            */
#define MCM_MCPCR_SWPU_Pos                (26U)                    /*!< SWPU (Bit 26)                                         */
#define MCM_MCPCR_SWPU_Msk                (0x4000000U)             /*!< SWPU (Bitfield-Mask: 0x01)                            */
#define MCM_MCPCR_SAPL_Pos                (28U)                    /*!< SAPL (Bit 28)                                         */
#define MCM_MCPCR_SAPL_Msk                (0x30000000U)            /*!< SAPL (Bitfield-Mask: 0x03)                            */
#define MCM_MCPCR_SWPL_Pos                (30U)                    /*!< SWPL (Bit 30)                                         */
#define MCM_MCPCR_SWPL_Msk                (0x40000000U)            /*!< SWPL (Bitfield-Mask: 0x01)                            */
/* =========================================================  MISCR  ========================================================= */
#define MCM_MISCR_FIOC_Pos                (8U)                     /*!< FIOC (Bit 8)                                          */
#define MCM_MISCR_FIOC_Msk                (0x100U)                 /*!< FIOC (Bitfield-Mask: 0x01)                            */
#define MCM_MISCR_FDZC_Pos                (9U)                     /*!< FDZC (Bit 9)                                          */
#define MCM_MISCR_FDZC_Msk                (0x200U)                 /*!< FDZC (Bitfield-Mask: 0x01)                            */
#define MCM_MISCR_FOFC_Pos                (10U)                    /*!< FOFC (Bit 10)                                         */
#define MCM_MISCR_FOFC_Msk                (0x400U)                 /*!< FOFC (Bitfield-Mask: 0x01)                            */
#define MCM_MISCR_FUFC_Pos                (11U)                    /*!< FUFC (Bit 11)                                         */
#define MCM_MISCR_FUFC_Msk                (0x800U)                 /*!< FUFC (Bitfield-Mask: 0x01)                            */
#define MCM_MISCR_FIXC_Pos                (12U)                    /*!< FIXC (Bit 12)                                         */
#define MCM_MISCR_FIXC_Msk                (0x1000U)                /*!< FIXC (Bitfield-Mask: 0x01)                            */
#define MCM_MISCR_FIDC_Pos                (15U)                    /*!< FIDC (Bit 15)                                         */
#define MCM_MISCR_FIDC_Msk                (0x8000U)                /*!< FIDC (Bitfield-Mask: 0x01)                            */
#define MCM_MISCR_FIOCEN_Pos              (24U)                    /*!< FIOCEN (Bit 24)                                       */
#define MCM_MISCR_FIOCEN_Msk              (0x1000000U)             /*!< FIOCEN (Bitfield-Mask: 0x01)                          */
#define MCM_MISCR_FDZCEN_Pos              (25U)                    /*!< FDZCEN (Bit 25)                                       */
#define MCM_MISCR_FDZCEN_Msk              (0x2000000U)             /*!< FDZCEN (Bitfield-Mask: 0x01)                          */
#define MCM_MISCR_FOFCEN_Pos              (26U)                    /*!< FOFCEN (Bit 26)                                       */
#define MCM_MISCR_FOFCEN_Msk              (0x4000000U)             /*!< FOFCEN (Bitfield-Mask: 0x01)                          */
#define MCM_MISCR_FUFCEN_Pos              (27U)                    /*!< FUFCEN (Bit 27)                                       */
#define MCM_MISCR_FUFCEN_Msk              (0x8000000U)             /*!< FUFCEN (Bitfield-Mask: 0x01)                          */
#define MCM_MISCR_FIXCEN_Pos              (28U)                    /*!< FIXCEN (Bit 28)                                       */
#define MCM_MISCR_FIXCEN_Msk              (0x10000000U)            /*!< FIXCEN (Bitfield-Mask: 0x01)                          */
#define MCM_MISCR_FIDCEN_Pos              (31U)                    /*!< FIDCEN (Bit 31)                                       */
#define MCM_MISCR_FIDCEN_Msk              (0x80000000U)            /*!< FIDCEN (Bitfield-Mask: 0x01)                          */
/* ========================================================  MLMDR0  ========================================================= */
#define MCM_MLMDR0_LREEN_Pos              (1U)                     /*!< LREEN (Bit 1)                                         */
#define MCM_MLMDR0_LREEN_Msk              (0x2U)                   /*!< LREEN (Bitfield-Mask: 0x01)                           */
#define MCM_MLMDR0_UREEN_Pos              (3U)                     /*!< UREEN (Bit 3)                                         */
#define MCM_MLMDR0_UREEN_Msk              (0x8U)                   /*!< UREEN (Bitfield-Mask: 0x01)                           */
#define MCM_MLMDR0_LOCK_Pos               (16U)                    /*!< LOCK (Bit 16)                                         */
#define MCM_MLMDR0_LOCK_Msk               (0x10000U)               /*!< LOCK (Bitfield-Mask: 0x01)                            */
#define MCM_MLMDR0_LMSZ_Pos               (24U)                    /*!< LMSZ (Bit 24)                                         */
#define MCM_MLMDR0_LMSZ_Msk               (0xf000000U)             /*!< LMSZ (Bitfield-Mask: 0x0f)                            */
/* =========================================================  MBIST  ========================================================= */
#define MCM_MBIST_MBIST_DONE_Pos          (0U)                     /*!< MBIST_DONE (Bit 0)                                    */
#define MCM_MBIST_MBIST_DONE_Msk          (0x1U)                   /*!< MBIST_DONE (Bitfield-Mask: 0x01)                      */
#define MCM_MBIST_MBIST_FAIL_Pos          (1U)                     /*!< MBIST_FAIL (Bit 1)                                    */
#define MCM_MBIST_MBIST_FAIL_Msk          (0x2U)                   /*!< MBIST_FAIL (Bitfield-Mask: 0x01)                      */
/* ========================================================  MLMDR1  ========================================================= */
#define MCM_MLMDR1_CPDE_Pos               (5U)                     /*!< CPDE (Bit 5)                                          */
#define MCM_MLMDR1_CPDE_Msk               (0x20U)                  /*!< CPDE (Bitfield-Mask: 0x01)                            */
#define MCM_MLMDR1_CPFE_Pos               (7U)                     /*!< CPFE (Bit 7)                                          */
#define MCM_MLMDR1_CPFE_Msk               (0x80U)                  /*!< CPFE (Bitfield-Mask: 0x01)                            */
#define MCM_MLMDR1_LOCK_Pos               (16U)                    /*!< LOCK (Bit 16)                                         */
#define MCM_MLMDR1_LOCK_Msk               (0x10000U)               /*!< LOCK (Bitfield-Mask: 0x01)                            */
#define MCM_MLMDR1_CASZ_Pos               (24U)                    /*!< CASZ (Bit 24)                                         */
#define MCM_MLMDR1_CASZ_Msk               (0xf000000U)             /*!< CASZ (Bitfield-Mask: 0x0f)                            */
/* ========================================================  LMPECR  ========================================================= */
#define MCM_LMPECR_CPEIEN_Pos             (20U)                    /*!< CPEIEN (Bit 20)                                       */
#define MCM_LMPECR_CPEIEN_Msk             (0x100000U)              /*!< CPEIEN (Bitfield-Mask: 0x01)                          */
/* ========================================================  LMPEIR  ========================================================= */
/* =========================================================  LMFAR  ========================================================= */
/* =========================================================  MCCR  ========================================================== */
#define MCM_MCCR_CADIS_Pos                (0U)                     /*!< CADIS (Bit 0)                                         */
#define MCM_MCCR_CADIS_Msk                (0x1U)                   /*!< CADIS (Bitfield-Mask: 0x01)                           */
#define MCM_MCCR_CACL_Pos                 (1U)                     /*!< CACL (Bit 1)                                          */
#define MCM_MCCR_CACL_Msk                 (0x2U)                   /*!< CACL (Bitfield-Mask: 0x01)                            */
#define MCM_MCCR_PREEN_Pos                (8U)                     /*!< PREEN (Bit 8)                                         */
#define MCM_MCCR_PREEN_Msk                (0x100U)                 /*!< PREEN (Bitfield-Mask: 0x01)                           */
/* =========================================================  MNCR  ========================================================== */
#define MCM_MNCR_NMI_EN_Pos               (0U)                     /*!< NMI_EN (Bit 0)                                        */
#define MCM_MNCR_NMI_EN_Msk               (0x1U)                   /*!< NMI_EN (Bitfield-Mask: 0x01)                          */

/* =========================================================================================================================== */
/* ================                                            MPU                                            ================ */
/* =========================================================================================================================== */

/* =========================================================  CESR  ========================================================== */
#define MPU_CESR_VLD_Pos                  (0U)                     /*!< VLD (Bit 0)                                           */
#define MPU_CESR_VLD_Msk                  (0x1U)                   /*!< VLD (Bitfield-Mask: 0x01)                             */
#define MPU_CESR_SPERR0_Pos               (29U)                    /*!< SPERR0 (Bit 29)                                       */
#define MPU_CESR_SPERR0_Msk               (0x20000000U)            /*!< SPERR0 (Bitfield-Mask: 0x01)                          */
#define MPU_CESR_SPERR1_Pos               (30U)                    /*!< SPERR1 (Bit 30)                                       */
#define MPU_CESR_SPERR1_Msk               (0x40000000U)            /*!< SPERR1 (Bitfield-Mask: 0x01)                          */
#define MPU_CESR_SPERR2_Pos               (31U)                    /*!< SPERR2 (Bit 31)                                       */
#define MPU_CESR_SPERR2_Msk               (0x80000000U)            /*!< SPERR2 (Bitfield-Mask: 0x01)                          */
/* =========================================================  EAR0  ========================================================== */
#define MPU_EAR0_EADDR_Pos                (0U)                     /*!< EADDR (Bit 0)                                         */
#define MPU_EAR0_EADDR_Msk                (0xffffffffU)            /*!< EADDR (Bitfield-Mask: 0xffffffff)                     */
/* =========================================================  EAR1  ========================================================== */
#define MPU_EAR1_EADDR_Pos                (0U)                     /*!< EADDR (Bit 0)                                         */
#define MPU_EAR1_EADDR_Msk                (0xffffffffU)            /*!< EADDR (Bitfield-Mask: 0xffffffff)                     */
/* =========================================================  EAR2  ========================================================== */
#define MPU_EAR2_EADDR_Pos                (0U)                     /*!< EADDR (Bit 0)                                         */
#define MPU_EAR2_EADDR_Msk                (0xffffffffU)            /*!< EADDR (Bitfield-Mask: 0xffffffff)                     */
/* =========================================================  EDR0  ========================================================== */
#define MPU_EDR0_ERW_Pos                  (0U)                     /*!< ERW (Bit 0)                                           */
#define MPU_EDR0_ERW_Msk                  (0x1U)                   /*!< ERW (Bitfield-Mask: 0x01)                             */
#define MPU_EDR0_EATTR_Pos                (1U)                     /*!< EATTR (Bit 1)                                         */
#define MPU_EDR0_EATTR_Msk                (0x1eU)                  /*!< EATTR (Bitfield-Mask: 0x0f)                           */
#define MPU_EDR0_EMN_Pos                  (5U)                     /*!< EMN (Bit 5)                                           */
#define MPU_EDR0_EMN_Msk                  (0xe0U)                  /*!< EMN (Bitfield-Mask: 0x07)                             */
#define MPU_EDR0_EACD_Pos                 (16U)                    /*!< EACD (Bit 16)                                         */
#define MPU_EDR0_EACD_Msk                 (0x3f0000U)              /*!< EACD (Bitfield-Mask: 0x3f)                            */
#define MPU_EDR0_EFLG_Pos                 (31U)                    /*!< EFLG (Bit 31)                                         */
#define MPU_EDR0_EFLG_Msk                 (0x80000000U)            /*!< EFLG (Bitfield-Mask: 0x01)                            */
/* =========================================================  EDR1  ========================================================== */
#define MPU_EDR1_ERW_Pos                  (0U)                     /*!< ERW (Bit 0)                                           */
#define MPU_EDR1_ERW_Msk                  (0x1U)                   /*!< ERW (Bitfield-Mask: 0x01)                             */
#define MPU_EDR1_EATTR_Pos                (1U)                     /*!< EATTR (Bit 1)                                         */
#define MPU_EDR1_EATTR_Msk                (0x1eU)                  /*!< EATTR (Bitfield-Mask: 0x0f)                           */
#define MPU_EDR1_EMN_Pos                  (5U)                     /*!< EMN (Bit 5)                                           */
#define MPU_EDR1_EMN_Msk                  (0xe0U)                  /*!< EMN (Bitfield-Mask: 0x07)                             */
#define MPU_EDR1_EACD_Pos                 (16U)                    /*!< EACD (Bit 16)                                         */
#define MPU_EDR1_EACD_Msk                 (0x3f0000U)              /*!< EACD (Bitfield-Mask: 0x3f)                            */
#define MPU_EDR1_EFLG_Pos                 (31U)                    /*!< EFLG (Bit 31)                                         */
#define MPU_EDR1_EFLG_Msk                 (0x80000000U)            /*!< EFLG (Bitfield-Mask: 0x01)                            */
/* =========================================================  EDR2  ========================================================== */
#define MPU_EDR2_ERW_Pos                  (0U)                     /*!< ERW (Bit 0)                                           */
#define MPU_EDR2_ERW_Msk                  (0x1U)                   /*!< ERW (Bitfield-Mask: 0x01)                             */
#define MPU_EDR2_EATTR_Pos                (1U)                     /*!< EATTR (Bit 1)                                         */
#define MPU_EDR2_EATTR_Msk                (0x1eU)                  /*!< EATTR (Bitfield-Mask: 0x0f)                           */
#define MPU_EDR2_EMN_Pos                  (5U)                     /*!< EMN (Bit 5)                                           */
#define MPU_EDR2_EMN_Msk                  (0xe0U)                  /*!< EMN (Bitfield-Mask: 0x07)                             */
#define MPU_EDR2_EACD_Pos                 (16U)                    /*!< EACD (Bit 16)                                         */
#define MPU_EDR2_EACD_Msk                 (0x3f0000U)              /*!< EACD (Bitfield-Mask: 0x3f)                            */
#define MPU_EDR2_EFLG_Pos                 (31U)                    /*!< EFLG (Bit 31)                                         */
#define MPU_EDR2_EFLG_Msk                 (0x80000000U)            /*!< EFLG (Bitfield-Mask: 0x01)                            */
/* ======================================================  RGD0_WORD0  ======================================================= */
#define MPU_RGD0_WORD0_SRTADDR_Pos        (5U)                     /*!< SRTADDR (Bit 5)                                       */
#define MPU_RGD0_WORD0_SRTADDR_Msk        (0xffffffe0U)            /*!< SRTADDR (Bitfield-Mask: 0x7ffffff)                    */
/* ======================================================  RGD1_WORD0  ======================================================= */
#define MPU_RGD1_WORD0_SRTADDR_Pos        (5U)                     /*!< SRTADDR (Bit 5)                                       */
#define MPU_RGD1_WORD0_SRTADDR_Msk        (0xffffffe0U)            /*!< SRTADDR (Bitfield-Mask: 0x7ffffff)                    */
/* ======================================================  RGD2_WORD0  ======================================================= */
#define MPU_RGD2_WORD0_SRTADDR_Pos        (5U)                     /*!< SRTADDR (Bit 5)                                       */
#define MPU_RGD2_WORD0_SRTADDR_Msk        (0xffffffe0U)            /*!< SRTADDR (Bitfield-Mask: 0x7ffffff)                    */
/* ======================================================  RGD3_WORD0  ======================================================= */
#define MPU_RGD3_WORD0_SRTADDR_Pos        (5U)                     /*!< SRTADDR (Bit 5)                                       */
#define MPU_RGD3_WORD0_SRTADDR_Msk        (0xffffffe0U)            /*!< SRTADDR (Bitfield-Mask: 0x7ffffff)                    */
/* ======================================================  RGD4_WORD0  ======================================================= */
#define MPU_RGD4_WORD0_SRTADDR_Pos        (5U)                     /*!< SRTADDR (Bit 5)                                       */
#define MPU_RGD4_WORD0_SRTADDR_Msk        (0xffffffe0U)            /*!< SRTADDR (Bitfield-Mask: 0x7ffffff)                    */
/* ======================================================  RGD5_WORD0  ======================================================= */
#define MPU_RGD5_WORD0_SRTADDR_Pos        (5U)                     /*!< SRTADDR (Bit 5)                                       */
#define MPU_RGD5_WORD0_SRTADDR_Msk        (0xffffffe0U)            /*!< SRTADDR (Bitfield-Mask: 0x7ffffff)                    */
/* ======================================================  RGD0_WORD1  ======================================================= */
#define MPU_RGD0_WORD1_ENDADDR_Pos        (5U)                     /*!< ENDADDR (Bit 5)                                       */
#define MPU_RGD0_WORD1_ENDADDR_Msk        (0xffffffe0U)            /*!< ENDADDR (Bitfield-Mask: 0x7ffffff)                    */
/* ======================================================  RGD1_WORD1  ======================================================= */
#define MPU_RGD1_WORD1_ENDADDR_Pos        (5U)                     /*!< ENDADDR (Bit 5)                                       */
#define MPU_RGD1_WORD1_ENDADDR_Msk        (0xffffffe0U)            /*!< ENDADDR (Bitfield-Mask: 0x7ffffff)                    */
/* ======================================================  RGD2_WORD1  ======================================================= */
#define MPU_RGD2_WORD1_ENDADDR_Pos        (5U)                     /*!< ENDADDR (Bit 5)                                       */
#define MPU_RGD2_WORD1_ENDADDR_Msk        (0xffffffe0U)            /*!< ENDADDR (Bitfield-Mask: 0x7ffffff)                    */
/* ======================================================  RGD3_WORD1  ======================================================= */
#define MPU_RGD3_WORD1_ENDADDR_Pos        (5U)                     /*!< ENDADDR (Bit 5)                                       */
#define MPU_RGD3_WORD1_ENDADDR_Msk        (0xffffffe0U)            /*!< ENDADDR (Bitfield-Mask: 0x7ffffff)                    */
/* ======================================================  RGD4_WORD1  ======================================================= */
#define MPU_RGD4_WORD1_ENDADDR_Pos        (5U)                     /*!< ENDADDR (Bit 5)                                       */
#define MPU_RGD4_WORD1_ENDADDR_Msk        (0xffffffe0U)            /*!< ENDADDR (Bitfield-Mask: 0x7ffffff)                    */
/* ======================================================  RGD5_WORD1  ======================================================= */
#define MPU_RGD5_WORD1_ENDADDR_Pos        (5U)                     /*!< ENDADDR (Bit 5)                                       */
#define MPU_RGD5_WORD1_ENDADDR_Msk        (0xffffffe0U)            /*!< ENDADDR (Bitfield-Mask: 0x7ffffff)                    */
/* ======================================================  RGD0_WORD2  ======================================================= */
#define MPU_RGD0_WORD2_M0UX_Pos           (0U)                     /*!< M0UX (Bit 0)                                          */
#define MPU_RGD0_WORD2_M0UX_Msk           (0x1U)                   /*!< M0UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_WORD2_M0UR_Pos           (1U)                     /*!< M0UR (Bit 1)                                          */
#define MPU_RGD0_WORD2_M0UR_Msk           (0x2U)                   /*!< M0UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_WORD2_M0UW_Pos           (2U)                     /*!< M0UW (Bit 2)                                          */
#define MPU_RGD0_WORD2_M0UW_Msk           (0x4U)                   /*!< M0UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_WORD2_M0SX_Pos           (3U)                     /*!< M0SX (Bit 3)                                          */
#define MPU_RGD0_WORD2_M0SX_Msk           (0x8U)                   /*!< M0SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_WORD2_M0SR_Pos           (4U)                     /*!< M0SR (Bit 4)                                          */
#define MPU_RGD0_WORD2_M0SR_Msk           (0x10U)                  /*!< M0SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_WORD2_M0SW_Pos           (5U)                     /*!< M0SW (Bit 5)                                          */
#define MPU_RGD0_WORD2_M0SW_Msk           (0x20U)                  /*!< M0SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_WORD2_M1UX_Pos           (6U)                     /*!< M1UX (Bit 6)                                          */
#define MPU_RGD0_WORD2_M1UX_Msk           (0x40U)                  /*!< M1UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_WORD2_M1UR_Pos           (7U)                     /*!< M1UR (Bit 7)                                          */
#define MPU_RGD0_WORD2_M1UR_Msk           (0x80U)                  /*!< M1UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_WORD2_M1UW_Pos           (8U)                     /*!< M1UW (Bit 8)                                          */
#define MPU_RGD0_WORD2_M1UW_Msk           (0x100U)                 /*!< M1UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_WORD2_M1SX_Pos           (9U)                     /*!< M1SX (Bit 9)                                          */
#define MPU_RGD0_WORD2_M1SX_Msk           (0x200U)                 /*!< M1SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_WORD2_M1SR_Pos           (10U)                    /*!< M1SR (Bit 10)                                         */
#define MPU_RGD0_WORD2_M1SR_Msk           (0x400U)                 /*!< M1SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_WORD2_M1SW_Pos           (11U)                    /*!< M1SW (Bit 11)                                         */
#define MPU_RGD0_WORD2_M1SW_Msk           (0x800U)                 /*!< M1SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_WORD2_M2SX_Pos           (15U)                    /*!< M2SX (Bit 15)                                         */
#define MPU_RGD0_WORD2_M2SX_Msk           (0x8000U)                /*!< M2SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_WORD2_M2SR_Pos           (16U)                    /*!< M2SR (Bit 16)                                         */
#define MPU_RGD0_WORD2_M2SR_Msk           (0x10000U)               /*!< M2SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_WORD2_M2SW_Pos           (17U)                    /*!< M2SW (Bit 17)                                         */
#define MPU_RGD0_WORD2_M2SW_Msk           (0x20000U)               /*!< M2SW (Bitfield-Mask: 0x01)                            */
/* ======================================================  RGD1_WORD2  ======================================================= */
#define MPU_RGD1_WORD2_M0UX_Pos           (0U)                     /*!< M0UX (Bit 0)                                          */
#define MPU_RGD1_WORD2_M0UX_Msk           (0x1U)                   /*!< M0UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_WORD2_M0UR_Pos           (1U)                     /*!< M0UR (Bit 1)                                          */
#define MPU_RGD1_WORD2_M0UR_Msk           (0x2U)                   /*!< M0UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_WORD2_M0UW_Pos           (2U)                     /*!< M0UW (Bit 2)                                          */
#define MPU_RGD1_WORD2_M0UW_Msk           (0x4U)                   /*!< M0UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_WORD2_M0SX_Pos           (3U)                     /*!< M0SX (Bit 3)                                          */
#define MPU_RGD1_WORD2_M0SX_Msk           (0x8U)                   /*!< M0SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_WORD2_M0SR_Pos           (4U)                     /*!< M0SR (Bit 4)                                          */
#define MPU_RGD1_WORD2_M0SR_Msk           (0x10U)                  /*!< M0SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_WORD2_M0SW_Pos           (5U)                     /*!< M0SW (Bit 5)                                          */
#define MPU_RGD1_WORD2_M0SW_Msk           (0x20U)                  /*!< M0SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_WORD2_M1UX_Pos           (6U)                     /*!< M1UX (Bit 6)                                          */
#define MPU_RGD1_WORD2_M1UX_Msk           (0x40U)                  /*!< M1UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_WORD2_M1UR_Pos           (7U)                     /*!< M1UR (Bit 7)                                          */
#define MPU_RGD1_WORD2_M1UR_Msk           (0x80U)                  /*!< M1UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_WORD2_M1UW_Pos           (8U)                     /*!< M1UW (Bit 8)                                          */
#define MPU_RGD1_WORD2_M1UW_Msk           (0x100U)                 /*!< M1UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_WORD2_M1SX_Pos           (9U)                     /*!< M1SX (Bit 9)                                          */
#define MPU_RGD1_WORD2_M1SX_Msk           (0x200U)                 /*!< M1SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_WORD2_M1SR_Pos           (10U)                    /*!< M1SR (Bit 10)                                         */
#define MPU_RGD1_WORD2_M1SR_Msk           (0x400U)                 /*!< M1SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_WORD2_M1SW_Pos           (11U)                    /*!< M1SW (Bit 11)                                         */
#define MPU_RGD1_WORD2_M1SW_Msk           (0x800U)                 /*!< M1SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_WORD2_M2SX_Pos           (15U)                    /*!< M2SX (Bit 15)                                         */
#define MPU_RGD1_WORD2_M2SX_Msk           (0x8000U)                /*!< M2SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_WORD2_M2SR_Pos           (16U)                    /*!< M2SR (Bit 16)                                         */
#define MPU_RGD1_WORD2_M2SR_Msk           (0x10000U)               /*!< M2SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_WORD2_M2SW_Pos           (17U)                    /*!< M2SW (Bit 17)                                         */
#define MPU_RGD1_WORD2_M2SW_Msk           (0x20000U)               /*!< M2SW (Bitfield-Mask: 0x01)                            */
/* ======================================================  RGD2_WORD2  ======================================================= */
#define MPU_RGD2_WORD2_M0UX_Pos           (0U)                     /*!< M0UX (Bit 0)                                          */
#define MPU_RGD2_WORD2_M0UX_Msk           (0x1U)                   /*!< M0UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_WORD2_M0UR_Pos           (1U)                     /*!< M0UR (Bit 1)                                          */
#define MPU_RGD2_WORD2_M0UR_Msk           (0x2U)                   /*!< M0UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_WORD2_M0UW_Pos           (2U)                     /*!< M0UW (Bit 2)                                          */
#define MPU_RGD2_WORD2_M0UW_Msk           (0x4U)                   /*!< M0UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_WORD2_M0SX_Pos           (3U)                     /*!< M0SX (Bit 3)                                          */
#define MPU_RGD2_WORD2_M0SX_Msk           (0x8U)                   /*!< M0SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_WORD2_M0SR_Pos           (4U)                     /*!< M0SR (Bit 4)                                          */
#define MPU_RGD2_WORD2_M0SR_Msk           (0x10U)                  /*!< M0SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_WORD2_M0SW_Pos           (5U)                     /*!< M0SW (Bit 5)                                          */
#define MPU_RGD2_WORD2_M0SW_Msk           (0x20U)                  /*!< M0SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_WORD2_M1UX_Pos           (6U)                     /*!< M1UX (Bit 6)                                          */
#define MPU_RGD2_WORD2_M1UX_Msk           (0x40U)                  /*!< M1UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_WORD2_M1UR_Pos           (7U)                     /*!< M1UR (Bit 7)                                          */
#define MPU_RGD2_WORD2_M1UR_Msk           (0x80U)                  /*!< M1UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_WORD2_M1UW_Pos           (8U)                     /*!< M1UW (Bit 8)                                          */
#define MPU_RGD2_WORD2_M1UW_Msk           (0x100U)                 /*!< M1UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_WORD2_M1SX_Pos           (9U)                     /*!< M1SX (Bit 9)                                          */
#define MPU_RGD2_WORD2_M1SX_Msk           (0x200U)                 /*!< M1SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_WORD2_M1SR_Pos           (10U)                    /*!< M1SR (Bit 10)                                         */
#define MPU_RGD2_WORD2_M1SR_Msk           (0x400U)                 /*!< M1SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_WORD2_M1SW_Pos           (11U)                    /*!< M1SW (Bit 11)                                         */
#define MPU_RGD2_WORD2_M1SW_Msk           (0x800U)                 /*!< M1SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_WORD2_M2SX_Pos           (15U)                    /*!< M2SX (Bit 15)                                         */
#define MPU_RGD2_WORD2_M2SX_Msk           (0x8000U)                /*!< M2SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_WORD2_M2SR_Pos           (16U)                    /*!< M2SR (Bit 16)                                         */
#define MPU_RGD2_WORD2_M2SR_Msk           (0x10000U)               /*!< M2SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_WORD2_M2SW_Pos           (17U)                    /*!< M2SW (Bit 17)                                         */
#define MPU_RGD2_WORD2_M2SW_Msk           (0x20000U)               /*!< M2SW (Bitfield-Mask: 0x01)                            */
/* ======================================================  RGD3_WORD2  ======================================================= */
#define MPU_RGD3_WORD2_M0UX_Pos           (0U)                     /*!< M0UX (Bit 0)                                          */
#define MPU_RGD3_WORD2_M0UX_Msk           (0x1U)                   /*!< M0UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_WORD2_M0UR_Pos           (1U)                     /*!< M0UR (Bit 1)                                          */
#define MPU_RGD3_WORD2_M0UR_Msk           (0x2U)                   /*!< M0UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_WORD2_M0UW_Pos           (2U)                     /*!< M0UW (Bit 2)                                          */
#define MPU_RGD3_WORD2_M0UW_Msk           (0x4U)                   /*!< M0UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_WORD2_M0SX_Pos           (3U)                     /*!< M0SX (Bit 3)                                          */
#define MPU_RGD3_WORD2_M0SX_Msk           (0x8U)                   /*!< M0SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_WORD2_M0SR_Pos           (4U)                     /*!< M0SR (Bit 4)                                          */
#define MPU_RGD3_WORD2_M0SR_Msk           (0x10U)                  /*!< M0SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_WORD2_M0SW_Pos           (5U)                     /*!< M0SW (Bit 5)                                          */
#define MPU_RGD3_WORD2_M0SW_Msk           (0x20U)                  /*!< M0SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_WORD2_M1UX_Pos           (6U)                     /*!< M1UX (Bit 6)                                          */
#define MPU_RGD3_WORD2_M1UX_Msk           (0x40U)                  /*!< M1UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_WORD2_M1UR_Pos           (7U)                     /*!< M1UR (Bit 7)                                          */
#define MPU_RGD3_WORD2_M1UR_Msk           (0x80U)                  /*!< M1UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_WORD2_M1UW_Pos           (8U)                     /*!< M1UW (Bit 8)                                          */
#define MPU_RGD3_WORD2_M1UW_Msk           (0x100U)                 /*!< M1UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_WORD2_M1SX_Pos           (9U)                     /*!< M1SX (Bit 9)                                          */
#define MPU_RGD3_WORD2_M1SX_Msk           (0x200U)                 /*!< M1SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_WORD2_M1SR_Pos           (10U)                    /*!< M1SR (Bit 10)                                         */
#define MPU_RGD3_WORD2_M1SR_Msk           (0x400U)                 /*!< M1SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_WORD2_M1SW_Pos           (11U)                    /*!< M1SW (Bit 11)                                         */
#define MPU_RGD3_WORD2_M1SW_Msk           (0x800U)                 /*!< M1SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_WORD2_M2SX_Pos           (15U)                    /*!< M2SX (Bit 15)                                         */
#define MPU_RGD3_WORD2_M2SX_Msk           (0x8000U)                /*!< M2SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_WORD2_M2SR_Pos           (16U)                    /*!< M2SR (Bit 16)                                         */
#define MPU_RGD3_WORD2_M2SR_Msk           (0x10000U)               /*!< M2SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_WORD2_M2SW_Pos           (17U)                    /*!< M2SW (Bit 17)                                         */
#define MPU_RGD3_WORD2_M2SW_Msk           (0x20000U)               /*!< M2SW (Bitfield-Mask: 0x01)                            */
/* ======================================================  RGD4_WORD2  ======================================================= */
#define MPU_RGD4_WORD2_M0UX_Pos           (0U)                     /*!< M0UX (Bit 0)                                          */
#define MPU_RGD4_WORD2_M0UX_Msk           (0x1U)                   /*!< M0UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_WORD2_M0UR_Pos           (1U)                     /*!< M0UR (Bit 1)                                          */
#define MPU_RGD4_WORD2_M0UR_Msk           (0x2U)                   /*!< M0UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_WORD2_M0UW_Pos           (2U)                     /*!< M0UW (Bit 2)                                          */
#define MPU_RGD4_WORD2_M0UW_Msk           (0x4U)                   /*!< M0UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_WORD2_M0SX_Pos           (3U)                     /*!< M0SX (Bit 3)                                          */
#define MPU_RGD4_WORD2_M0SX_Msk           (0x8U)                   /*!< M0SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_WORD2_M0SR_Pos           (4U)                     /*!< M0SR (Bit 4)                                          */
#define MPU_RGD4_WORD2_M0SR_Msk           (0x10U)                  /*!< M0SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_WORD2_M0SW_Pos           (5U)                     /*!< M0SW (Bit 5)                                          */
#define MPU_RGD4_WORD2_M0SW_Msk           (0x20U)                  /*!< M0SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_WORD2_M1UX_Pos           (6U)                     /*!< M1UX (Bit 6)                                          */
#define MPU_RGD4_WORD2_M1UX_Msk           (0x40U)                  /*!< M1UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_WORD2_M1UR_Pos           (7U)                     /*!< M1UR (Bit 7)                                          */
#define MPU_RGD4_WORD2_M1UR_Msk           (0x80U)                  /*!< M1UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_WORD2_M1UW_Pos           (8U)                     /*!< M1UW (Bit 8)                                          */
#define MPU_RGD4_WORD2_M1UW_Msk           (0x100U)                 /*!< M1UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_WORD2_M1SX_Pos           (9U)                     /*!< M1SX (Bit 9)                                          */
#define MPU_RGD4_WORD2_M1SX_Msk           (0x200U)                 /*!< M1SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_WORD2_M1SR_Pos           (10U)                    /*!< M1SR (Bit 10)                                         */
#define MPU_RGD4_WORD2_M1SR_Msk           (0x400U)                 /*!< M1SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_WORD2_M1SW_Pos           (11U)                    /*!< M1SW (Bit 11)                                         */
#define MPU_RGD4_WORD2_M1SW_Msk           (0x800U)                 /*!< M1SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_WORD2_M2SX_Pos           (15U)                    /*!< M2SX (Bit 15)                                         */
#define MPU_RGD4_WORD2_M2SX_Msk           (0x8000U)                /*!< M2SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_WORD2_M2SR_Pos           (16U)                    /*!< M2SR (Bit 16)                                         */
#define MPU_RGD4_WORD2_M2SR_Msk           (0x10000U)               /*!< M2SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_WORD2_M2SW_Pos           (17U)                    /*!< M2SW (Bit 17)                                         */
#define MPU_RGD4_WORD2_M2SW_Msk           (0x20000U)               /*!< M2SW (Bitfield-Mask: 0x01)                            */
/* ======================================================  RGD5_WORD2  ======================================================= */
#define MPU_RGD5_WORD2_M0UX_Pos           (0U)                     /*!< M0UX (Bit 0)                                          */
#define MPU_RGD5_WORD2_M0UX_Msk           (0x1U)                   /*!< M0UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_WORD2_M0UR_Pos           (1U)                     /*!< M0UR (Bit 1)                                          */
#define MPU_RGD5_WORD2_M0UR_Msk           (0x2U)                   /*!< M0UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_WORD2_M0UW_Pos           (2U)                     /*!< M0UW (Bit 2)                                          */
#define MPU_RGD5_WORD2_M0UW_Msk           (0x4U)                   /*!< M0UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_WORD2_M0SX_Pos           (3U)                     /*!< M0SX (Bit 3)                                          */
#define MPU_RGD5_WORD2_M0SX_Msk           (0x8U)                   /*!< M0SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_WORD2_M0SR_Pos           (4U)                     /*!< M0SR (Bit 4)                                          */
#define MPU_RGD5_WORD2_M0SR_Msk           (0x10U)                  /*!< M0SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_WORD2_M0SW_Pos           (5U)                     /*!< M0SW (Bit 5)                                          */
#define MPU_RGD5_WORD2_M0SW_Msk           (0x20U)                  /*!< M0SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_WORD2_M1UX_Pos           (6U)                     /*!< M1UX (Bit 6)                                          */
#define MPU_RGD5_WORD2_M1UX_Msk           (0x40U)                  /*!< M1UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_WORD2_M1UR_Pos           (7U)                     /*!< M1UR (Bit 7)                                          */
#define MPU_RGD5_WORD2_M1UR_Msk           (0x80U)                  /*!< M1UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_WORD2_M1UW_Pos           (8U)                     /*!< M1UW (Bit 8)                                          */
#define MPU_RGD5_WORD2_M1UW_Msk           (0x100U)                 /*!< M1UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_WORD2_M1SX_Pos           (9U)                     /*!< M1SX (Bit 9)                                          */
#define MPU_RGD5_WORD2_M1SX_Msk           (0x200U)                 /*!< M1SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_WORD2_M1SR_Pos           (10U)                    /*!< M1SR (Bit 10)                                         */
#define MPU_RGD5_WORD2_M1SR_Msk           (0x400U)                 /*!< M1SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_WORD2_M1SW_Pos           (11U)                    /*!< M1SW (Bit 11)                                         */
#define MPU_RGD5_WORD2_M1SW_Msk           (0x800U)                 /*!< M1SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_WORD2_M2SX_Pos           (15U)                    /*!< M2SX (Bit 15)                                         */
#define MPU_RGD5_WORD2_M2SX_Msk           (0x8000U)                /*!< M2SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_WORD2_M2SR_Pos           (16U)                    /*!< M2SR (Bit 16)                                         */
#define MPU_RGD5_WORD2_M2SR_Msk           (0x10000U)               /*!< M2SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_WORD2_M2SW_Pos           (17U)                    /*!< M2SW (Bit 17)                                         */
#define MPU_RGD5_WORD2_M2SW_Msk           (0x20000U)               /*!< M2SW (Bitfield-Mask: 0x01)                            */
/* ======================================================  RGD0_WORD3  ======================================================= */
#define MPU_RGD0_WORD3_VLD_Pos            (0U)                     /*!< VLD (Bit 0)                                           */
#define MPU_RGD0_WORD3_VLD_Msk            (0x1U)                   /*!< VLD (Bitfield-Mask: 0x01)                             */
/* ======================================================  RGD1_WORD3  ======================================================= */
#define MPU_RGD1_WORD3_VLD_Pos            (0U)                     /*!< VLD (Bit 0)                                           */
#define MPU_RGD1_WORD3_VLD_Msk            (0x1U)                   /*!< VLD (Bitfield-Mask: 0x01)                             */
/* ======================================================  RGD2_WORD3  ======================================================= */
#define MPU_RGD2_WORD3_VLD_Pos            (0U)                     /*!< VLD (Bit 0)                                           */
#define MPU_RGD2_WORD3_VLD_Msk            (0x1U)                   /*!< VLD (Bitfield-Mask: 0x01)                             */
/* ======================================================  RGD3_WORD3  ======================================================= */
#define MPU_RGD3_WORD3_VLD_Pos            (0U)                     /*!< VLD (Bit 0)                                           */
#define MPU_RGD3_WORD3_VLD_Msk            (0x1U)                   /*!< VLD (Bitfield-Mask: 0x01)                             */
/* ======================================================  RGD4_WORD3  ======================================================= */
#define MPU_RGD4_WORD3_VLD_Pos            (0U)                     /*!< VLD (Bit 0)                                           */
#define MPU_RGD4_WORD3_VLD_Msk            (0x1U)                   /*!< VLD (Bitfield-Mask: 0x01)                             */
/* ======================================================  RGD5_WORD3  ======================================================= */
#define MPU_RGD5_WORD3_VLD_Pos            (0U)                     /*!< VLD (Bit 0)                                           */
#define MPU_RGD5_WORD3_VLD_Msk            (0x1U)                   /*!< VLD (Bitfield-Mask: 0x01)                             */
/* =======================================================  RGD0_AAC  ======================================================== */
#define MPU_RGD0_AAC_M0UX_Pos             (0U)                     /*!< M0UX (Bit 0)                                          */
#define MPU_RGD0_AAC_M0UX_Msk             (0x1U)                   /*!< M0UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_AAC_M0UR_Pos             (1U)                     /*!< M0UR (Bit 1)                                          */
#define MPU_RGD0_AAC_M0UR_Msk             (0x2U)                   /*!< M0UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_AAC_M0UW_Pos             (2U)                     /*!< M0UW (Bit 2)                                          */
#define MPU_RGD0_AAC_M0UW_Msk             (0x4U)                   /*!< M0UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_AAC_M0SX_Pos             (3U)                     /*!< M0SX (Bit 3)                                          */
#define MPU_RGD0_AAC_M0SX_Msk             (0x8U)                   /*!< M0SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_AAC_M0SR_Pos             (4U)                     /*!< M0SR (Bit 4)                                          */
#define MPU_RGD0_AAC_M0SR_Msk             (0x10U)                  /*!< M0SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_AAC_M0SW_Pos             (5U)                     /*!< M0SW (Bit 5)                                          */
#define MPU_RGD0_AAC_M0SW_Msk             (0x20U)                  /*!< M0SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_AAC_M1UX_Pos             (6U)                     /*!< M1UX (Bit 6)                                          */
#define MPU_RGD0_AAC_M1UX_Msk             (0x40U)                  /*!< M1UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_AAC_M1UR_Pos             (7U)                     /*!< M1UR (Bit 7)                                          */
#define MPU_RGD0_AAC_M1UR_Msk             (0x80U)                  /*!< M1UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_AAC_M1UW_Pos             (8U)                     /*!< M1UW (Bit 8)                                          */
#define MPU_RGD0_AAC_M1UW_Msk             (0x100U)                 /*!< M1UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_AAC_M1SX_Pos             (9U)                     /*!< M1SX (Bit 9)                                          */
#define MPU_RGD0_AAC_M1SX_Msk             (0x200U)                 /*!< M1SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_AAC_M1SR_Pos             (10U)                    /*!< M1SR (Bit 10)                                         */
#define MPU_RGD0_AAC_M1SR_Msk             (0x400U)                 /*!< M1SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_AAC_M1SW_Pos             (11U)                    /*!< M1SW (Bit 11)                                         */
#define MPU_RGD0_AAC_M1SW_Msk             (0x800U)                 /*!< M1SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_AAC_M2SX_Pos             (15U)                    /*!< M2SX (Bit 15)                                         */
#define MPU_RGD0_AAC_M2SX_Msk             (0x8000U)                /*!< M2SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_AAC_M2SR_Pos             (16U)                    /*!< M2SR (Bit 16)                                         */
#define MPU_RGD0_AAC_M2SR_Msk             (0x10000U)               /*!< M2SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD0_AAC_M2SW_Pos             (17U)                    /*!< M2SW (Bit 17)                                         */
#define MPU_RGD0_AAC_M2SW_Msk             (0x20000U)               /*!< M2SW (Bitfield-Mask: 0x01)                            */
/* =======================================================  RGD1_AAC  ======================================================== */
#define MPU_RGD1_AAC_M0UX_Pos             (0U)                     /*!< M0UX (Bit 0)                                          */
#define MPU_RGD1_AAC_M0UX_Msk             (0x1U)                   /*!< M0UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_AAC_M0UR_Pos             (1U)                     /*!< M0UR (Bit 1)                                          */
#define MPU_RGD1_AAC_M0UR_Msk             (0x2U)                   /*!< M0UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_AAC_M0UW_Pos             (2U)                     /*!< M0UW (Bit 2)                                          */
#define MPU_RGD1_AAC_M0UW_Msk             (0x4U)                   /*!< M0UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_AAC_M0SX_Pos             (3U)                     /*!< M0SX (Bit 3)                                          */
#define MPU_RGD1_AAC_M0SX_Msk             (0x8U)                   /*!< M0SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_AAC_M0SR_Pos             (4U)                     /*!< M0SR (Bit 4)                                          */
#define MPU_RGD1_AAC_M0SR_Msk             (0x10U)                  /*!< M0SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_AAC_M0SW_Pos             (5U)                     /*!< M0SW (Bit 5)                                          */
#define MPU_RGD1_AAC_M0SW_Msk             (0x20U)                  /*!< M0SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_AAC_M1UX_Pos             (6U)                     /*!< M1UX (Bit 6)                                          */
#define MPU_RGD1_AAC_M1UX_Msk             (0x40U)                  /*!< M1UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_AAC_M1UR_Pos             (7U)                     /*!< M1UR (Bit 7)                                          */
#define MPU_RGD1_AAC_M1UR_Msk             (0x80U)                  /*!< M1UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_AAC_M1UW_Pos             (8U)                     /*!< M1UW (Bit 8)                                          */
#define MPU_RGD1_AAC_M1UW_Msk             (0x100U)                 /*!< M1UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_AAC_M1SX_Pos             (9U)                     /*!< M1SX (Bit 9)                                          */
#define MPU_RGD1_AAC_M1SX_Msk             (0x200U)                 /*!< M1SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_AAC_M1SR_Pos             (10U)                    /*!< M1SR (Bit 10)                                         */
#define MPU_RGD1_AAC_M1SR_Msk             (0x400U)                 /*!< M1SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_AAC_M1SW_Pos             (11U)                    /*!< M1SW (Bit 11)                                         */
#define MPU_RGD1_AAC_M1SW_Msk             (0x800U)                 /*!< M1SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_AAC_M2SX_Pos             (15U)                    /*!< M2SX (Bit 15)                                         */
#define MPU_RGD1_AAC_M2SX_Msk             (0x8000U)                /*!< M2SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_AAC_M2SR_Pos             (16U)                    /*!< M2SR (Bit 16)                                         */
#define MPU_RGD1_AAC_M2SR_Msk             (0x10000U)               /*!< M2SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD1_AAC_M2SW_Pos             (17U)                    /*!< M2SW (Bit 17)                                         */
#define MPU_RGD1_AAC_M2SW_Msk             (0x20000U)               /*!< M2SW (Bitfield-Mask: 0x01)                            */
/* =======================================================  RGD2_AAC  ======================================================== */
#define MPU_RGD2_AAC_M0UX_Pos             (0U)                     /*!< M0UX (Bit 0)                                          */
#define MPU_RGD2_AAC_M0UX_Msk             (0x1U)                   /*!< M0UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_AAC_M0UR_Pos             (1U)                     /*!< M0UR (Bit 1)                                          */
#define MPU_RGD2_AAC_M0UR_Msk             (0x2U)                   /*!< M0UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_AAC_M0UW_Pos             (2U)                     /*!< M0UW (Bit 2)                                          */
#define MPU_RGD2_AAC_M0UW_Msk             (0x4U)                   /*!< M0UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_AAC_M0SX_Pos             (3U)                     /*!< M0SX (Bit 3)                                          */
#define MPU_RGD2_AAC_M0SX_Msk             (0x8U)                   /*!< M0SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_AAC_M0SR_Pos             (4U)                     /*!< M0SR (Bit 4)                                          */
#define MPU_RGD2_AAC_M0SR_Msk             (0x10U)                  /*!< M0SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_AAC_M0SW_Pos             (5U)                     /*!< M0SW (Bit 5)                                          */
#define MPU_RGD2_AAC_M0SW_Msk             (0x20U)                  /*!< M0SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_AAC_M1UX_Pos             (6U)                     /*!< M1UX (Bit 6)                                          */
#define MPU_RGD2_AAC_M1UX_Msk             (0x40U)                  /*!< M1UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_AAC_M1UR_Pos             (7U)                     /*!< M1UR (Bit 7)                                          */
#define MPU_RGD2_AAC_M1UR_Msk             (0x80U)                  /*!< M1UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_AAC_M1UW_Pos             (8U)                     /*!< M1UW (Bit 8)                                          */
#define MPU_RGD2_AAC_M1UW_Msk             (0x100U)                 /*!< M1UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_AAC_M1SX_Pos             (9U)                     /*!< M1SX (Bit 9)                                          */
#define MPU_RGD2_AAC_M1SX_Msk             (0x200U)                 /*!< M1SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_AAC_M1SR_Pos             (10U)                    /*!< M1SR (Bit 10)                                         */
#define MPU_RGD2_AAC_M1SR_Msk             (0x400U)                 /*!< M1SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_AAC_M1SW_Pos             (11U)                    /*!< M1SW (Bit 11)                                         */
#define MPU_RGD2_AAC_M1SW_Msk             (0x800U)                 /*!< M1SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_AAC_M2SX_Pos             (15U)                    /*!< M2SX (Bit 15)                                         */
#define MPU_RGD2_AAC_M2SX_Msk             (0x8000U)                /*!< M2SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_AAC_M2SR_Pos             (16U)                    /*!< M2SR (Bit 16)                                         */
#define MPU_RGD2_AAC_M2SR_Msk             (0x10000U)               /*!< M2SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD2_AAC_M2SW_Pos             (17U)                    /*!< M2SW (Bit 17)                                         */
#define MPU_RGD2_AAC_M2SW_Msk             (0x20000U)               /*!< M2SW (Bitfield-Mask: 0x01)                            */
/* =======================================================  RGD3_AAC  ======================================================== */
#define MPU_RGD3_AAC_M0UX_Pos             (0U)                     /*!< M0UX (Bit 0)                                          */
#define MPU_RGD3_AAC_M0UX_Msk             (0x1U)                   /*!< M0UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_AAC_M0UR_Pos             (1U)                     /*!< M0UR (Bit 1)                                          */
#define MPU_RGD3_AAC_M0UR_Msk             (0x2U)                   /*!< M0UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_AAC_M0UW_Pos             (2U)                     /*!< M0UW (Bit 2)                                          */
#define MPU_RGD3_AAC_M0UW_Msk             (0x4U)                   /*!< M0UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_AAC_M0SX_Pos             (3U)                     /*!< M0SX (Bit 3)                                          */
#define MPU_RGD3_AAC_M0SX_Msk             (0x8U)                   /*!< M0SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_AAC_M0SR_Pos             (4U)                     /*!< M0SR (Bit 4)                                          */
#define MPU_RGD3_AAC_M0SR_Msk             (0x10U)                  /*!< M0SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_AAC_M0SW_Pos             (5U)                     /*!< M0SW (Bit 5)                                          */
#define MPU_RGD3_AAC_M0SW_Msk             (0x20U)                  /*!< M0SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_AAC_M1UX_Pos             (6U)                     /*!< M1UX (Bit 6)                                          */
#define MPU_RGD3_AAC_M1UX_Msk             (0x40U)                  /*!< M1UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_AAC_M1UR_Pos             (7U)                     /*!< M1UR (Bit 7)                                          */
#define MPU_RGD3_AAC_M1UR_Msk             (0x80U)                  /*!< M1UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_AAC_M1UW_Pos             (8U)                     /*!< M1UW (Bit 8)                                          */
#define MPU_RGD3_AAC_M1UW_Msk             (0x100U)                 /*!< M1UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_AAC_M1SX_Pos             (9U)                     /*!< M1SX (Bit 9)                                          */
#define MPU_RGD3_AAC_M1SX_Msk             (0x200U)                 /*!< M1SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_AAC_M1SR_Pos             (10U)                    /*!< M1SR (Bit 10)                                         */
#define MPU_RGD3_AAC_M1SR_Msk             (0x400U)                 /*!< M1SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_AAC_M1SW_Pos             (11U)                    /*!< M1SW (Bit 11)                                         */
#define MPU_RGD3_AAC_M1SW_Msk             (0x800U)                 /*!< M1SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_AAC_M2SX_Pos             (15U)                    /*!< M2SX (Bit 15)                                         */
#define MPU_RGD3_AAC_M2SX_Msk             (0x8000U)                /*!< M2SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_AAC_M2SR_Pos             (16U)                    /*!< M2SR (Bit 16)                                         */
#define MPU_RGD3_AAC_M2SR_Msk             (0x10000U)               /*!< M2SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD3_AAC_M2SW_Pos             (17U)                    /*!< M2SW (Bit 17)                                         */
#define MPU_RGD3_AAC_M2SW_Msk             (0x20000U)               /*!< M2SW (Bitfield-Mask: 0x01)                            */
/* =======================================================  RGD4_AAC  ======================================================== */
#define MPU_RGD4_AAC_M0UX_Pos             (0U)                     /*!< M0UX (Bit 0)                                          */
#define MPU_RGD4_AAC_M0UX_Msk             (0x1U)                   /*!< M0UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_AAC_M0UR_Pos             (1U)                     /*!< M0UR (Bit 1)                                          */
#define MPU_RGD4_AAC_M0UR_Msk             (0x2U)                   /*!< M0UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_AAC_M0UW_Pos             (2U)                     /*!< M0UW (Bit 2)                                          */
#define MPU_RGD4_AAC_M0UW_Msk             (0x4U)                   /*!< M0UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_AAC_M0SX_Pos             (3U)                     /*!< M0SX (Bit 3)                                          */
#define MPU_RGD4_AAC_M0SX_Msk             (0x8U)                   /*!< M0SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_AAC_M0SR_Pos             (4U)                     /*!< M0SR (Bit 4)                                          */
#define MPU_RGD4_AAC_M0SR_Msk             (0x10U)                  /*!< M0SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_AAC_M0SW_Pos             (5U)                     /*!< M0SW (Bit 5)                                          */
#define MPU_RGD4_AAC_M0SW_Msk             (0x20U)                  /*!< M0SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_AAC_M1UX_Pos             (6U)                     /*!< M1UX (Bit 6)                                          */
#define MPU_RGD4_AAC_M1UX_Msk             (0x40U)                  /*!< M1UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_AAC_M1UR_Pos             (7U)                     /*!< M1UR (Bit 7)                                          */
#define MPU_RGD4_AAC_M1UR_Msk             (0x80U)                  /*!< M1UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_AAC_M1UW_Pos             (8U)                     /*!< M1UW (Bit 8)                                          */
#define MPU_RGD4_AAC_M1UW_Msk             (0x100U)                 /*!< M1UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_AAC_M1SX_Pos             (9U)                     /*!< M1SX (Bit 9)                                          */
#define MPU_RGD4_AAC_M1SX_Msk             (0x200U)                 /*!< M1SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_AAC_M1SR_Pos             (10U)                    /*!< M1SR (Bit 10)                                         */
#define MPU_RGD4_AAC_M1SR_Msk             (0x400U)                 /*!< M1SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_AAC_M1SW_Pos             (11U)                    /*!< M1SW (Bit 11)                                         */
#define MPU_RGD4_AAC_M1SW_Msk             (0x800U)                 /*!< M1SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_AAC_M2SX_Pos             (15U)                    /*!< M2SX (Bit 15)                                         */
#define MPU_RGD4_AAC_M2SX_Msk             (0x8000U)                /*!< M2SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_AAC_M2SR_Pos             (16U)                    /*!< M2SR (Bit 16)                                         */
#define MPU_RGD4_AAC_M2SR_Msk             (0x10000U)               /*!< M2SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD4_AAC_M2SW_Pos             (17U)                    /*!< M2SW (Bit 17)                                         */
#define MPU_RGD4_AAC_M2SW_Msk             (0x20000U)               /*!< M2SW (Bitfield-Mask: 0x01)                            */
/* =======================================================  RGD5_AAC  ======================================================== */
#define MPU_RGD5_AAC_M0UX_Pos             (0U)                     /*!< M0UX (Bit 0)                                          */
#define MPU_RGD5_AAC_M0UX_Msk             (0x1U)                   /*!< M0UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_AAC_M0UR_Pos             (1U)                     /*!< M0UR (Bit 1)                                          */
#define MPU_RGD5_AAC_M0UR_Msk             (0x2U)                   /*!< M0UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_AAC_M0UW_Pos             (2U)                     /*!< M0UW (Bit 2)                                          */
#define MPU_RGD5_AAC_M0UW_Msk             (0x4U)                   /*!< M0UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_AAC_M0SX_Pos             (3U)                     /*!< M0SX (Bit 3)                                          */
#define MPU_RGD5_AAC_M0SX_Msk             (0x8U)                   /*!< M0SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_AAC_M0SR_Pos             (4U)                     /*!< M0SR (Bit 4)                                          */
#define MPU_RGD5_AAC_M0SR_Msk             (0x10U)                  /*!< M0SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_AAC_M0SW_Pos             (5U)                     /*!< M0SW (Bit 5)                                          */
#define MPU_RGD5_AAC_M0SW_Msk             (0x20U)                  /*!< M0SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_AAC_M1UX_Pos             (6U)                     /*!< M1UX (Bit 6)                                          */
#define MPU_RGD5_AAC_M1UX_Msk             (0x40U)                  /*!< M1UX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_AAC_M1UR_Pos             (7U)                     /*!< M1UR (Bit 7)                                          */
#define MPU_RGD5_AAC_M1UR_Msk             (0x80U)                  /*!< M1UR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_AAC_M1UW_Pos             (8U)                     /*!< M1UW (Bit 8)                                          */
#define MPU_RGD5_AAC_M1UW_Msk             (0x100U)                 /*!< M1UW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_AAC_M1SX_Pos             (9U)                     /*!< M1SX (Bit 9)                                          */
#define MPU_RGD5_AAC_M1SX_Msk             (0x200U)                 /*!< M1SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_AAC_M1SR_Pos             (10U)                    /*!< M1SR (Bit 10)                                         */
#define MPU_RGD5_AAC_M1SR_Msk             (0x400U)                 /*!< M1SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_AAC_M1SW_Pos             (11U)                    /*!< M1SW (Bit 11)                                         */
#define MPU_RGD5_AAC_M1SW_Msk             (0x800U)                 /*!< M1SW (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_AAC_M2SX_Pos             (15U)                    /*!< M2SX (Bit 15)                                         */
#define MPU_RGD5_AAC_M2SX_Msk             (0x8000U)                /*!< M2SX (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_AAC_M2SR_Pos             (16U)                    /*!< M2SR (Bit 16)                                         */
#define MPU_RGD5_AAC_M2SR_Msk             (0x10000U)               /*!< M2SR (Bitfield-Mask: 0x01)                            */
#define MPU_RGD5_AAC_M2SW_Pos             (17U)                    /*!< M2SW (Bit 17)                                         */
#define MPU_RGD5_AAC_M2SW_Msk             (0x20000U)               /*!< M2SW (Bitfield-Mask: 0x01)                            */

/* =========================================================================================================================== */
/* ================                                           CKGEN                                           ================ */
/* =========================================================================================================================== */

/* =========================================================  CTRL  ========================================================== */
#define CKGEN_CTRL_BUS_CLK_DIV_RUN_Pos    (4U)                     /*!< BUS_CLK_DIV_RUN (Bit 4)                               */
#define CKGEN_CTRL_BUS_CLK_DIV_RUN_Msk    (0xf0U)                  /*!< BUS_CLK_DIV_RUN (Bitfield-Mask: 0x0f)                 */
#define CKGEN_CTRL_SYSCLK_DIV_Pos         (16U)                    /*!< SYSCLK_DIV (Bit 16)                                   */
#define CKGEN_CTRL_SYSCLK_DIV_Msk         (0xf0000U)               /*!< SYSCLK_DIV (Bitfield-Mask: 0x0f)                      */
#define CKGEN_CTRL_PLL_REF_SEL_Pos        (20U)                    /*!< PLL_REF_SEL (Bit 20)                                  */
#define CKGEN_CTRL_PLL_REF_SEL_Msk        (0x100000U)              /*!< PLL_REF_SEL (Bitfield-Mask: 0x01)                     */
#define CKGEN_CTRL_XOSC_MON_EN_Pos        (21U)                    /*!< XOSC_MON_EN (Bit 21)                                  */
#define CKGEN_CTRL_XOSC_MON_EN_Msk        (0x200000U)              /*!< XOSC_MON_EN (Bitfield-Mask: 0x01)                     */
#define CKGEN_CTRL_SYSCLK_SRC_SEL_RUN_Pos (22U)                    /*!< SYSCLK_SRC_SEL_RUN (Bit 22)                           */
#define CKGEN_CTRL_SYSCLK_SRC_SEL_RUN_Msk (0x3c00000U)             /*!< SYSCLK_SRC_SEL_RUN (Bitfield-Mask: 0x0f)              */
#define CKGEN_CTRL_LOCK_Pos               (31U)                    /*!< LOCK (Bit 31)                                         */
#define CKGEN_CTRL_LOCK_Msk               (0x80000000U)            /*!< LOCK (Bitfield-Mask: 0x01)                            */
/* ======================================================  LP_CLK_MUX  ======================================================= */
#define CKGEN_LP_CLK_MUX_LSI_CLK_MUX_Pos  (0U)                     /*!< LSI_CLK_MUX (Bit 0)                                   */
#define CKGEN_LP_CLK_MUX_LSI_CLK_MUX_Msk  (0x3U)                   /*!< LSI_CLK_MUX (Bitfield-Mask: 0x03)                     */
#define CKGEN_LP_CLK_MUX_CLKFREQ_Pos      (8U)                     /*!< CLKFREQ (Bit 8)                                       */
#define CKGEN_LP_CLK_MUX_CLKFREQ_Msk      (0xff00U)                /*!< CLKFREQ (Bitfield-Mask: 0xff)                         */
#define CKGEN_LP_CLK_MUX_LATENCY_Pos      (16U)                    /*!< LATENCY (Bit 16)                                      */
#define CKGEN_LP_CLK_MUX_LATENCY_Msk      (0x70000U)               /*!< LATENCY (Bitfield-Mask: 0x07)                         */
#define CKGEN_LP_CLK_MUX_LATENCY1_Pos     (20U)                    /*!< LATENCY1 (Bit 20)                                     */
#define CKGEN_LP_CLK_MUX_LATENCY1_Msk     (0xf00000U)              /*!< LATENCY1 (Bitfield-Mask: 0x0f)                        */
/* =====================================================  PERI_CLK_EN0  ====================================================== */
#define CKGEN_PERI_CLK_EN0_UART0_EN_Pos   (0U)                     /*!< UART0_EN (Bit 0)                                      */
#define CKGEN_PERI_CLK_EN0_UART0_EN_Msk   (0x1U)                   /*!< UART0_EN (Bitfield-Mask: 0x01)                        */
#define CKGEN_PERI_CLK_EN0_UART1_EN_Pos   (1U)                     /*!< UART1_EN (Bit 1)                                      */
#define CKGEN_PERI_CLK_EN0_UART1_EN_Msk   (0x2U)                   /*!< UART1_EN (Bitfield-Mask: 0x01)                        */
#define CKGEN_PERI_CLK_EN0_UART2_EN_Pos   (2U)                     /*!< UART2_EN (Bit 2)                                      */
#define CKGEN_PERI_CLK_EN0_UART2_EN_Msk   (0x4U)                   /*!< UART2_EN (Bitfield-Mask: 0x01)                        */
#define CKGEN_PERI_CLK_EN0_SPI0_EN_Pos    (6U)                     /*!< SPI0_EN (Bit 6)                                       */
#define CKGEN_PERI_CLK_EN0_SPI0_EN_Msk    (0x40U)                  /*!< SPI0_EN (Bitfield-Mask: 0x01)                         */
#define CKGEN_PERI_CLK_EN0_SPI1_EN_Pos    (7U)                     /*!< SPI1_EN (Bit 7)                                       */
#define CKGEN_PERI_CLK_EN0_SPI1_EN_Msk    (0x80U)                  /*!< SPI1_EN (Bitfield-Mask: 0x01)                         */
#define CKGEN_PERI_CLK_EN0_SPI2_EN_Pos    (8U)                     /*!< SPI2_EN (Bit 8)                                       */
#define CKGEN_PERI_CLK_EN0_SPI2_EN_Msk    (0x100U)                 /*!< SPI2_EN (Bitfield-Mask: 0x01)                         */
#define CKGEN_PERI_CLK_EN0_I2C0_EN_Pos    (9U)                     /*!< I2C0_EN (Bit 9)                                       */
#define CKGEN_PERI_CLK_EN0_I2C0_EN_Msk    (0x200U)                 /*!< I2C0_EN (Bitfield-Mask: 0x01)                         */
#define CKGEN_PERI_CLK_EN0_I2C1_EN_Pos    (10U)                    /*!< I2C1_EN (Bit 10)                                      */
#define CKGEN_PERI_CLK_EN0_I2C1_EN_Msk    (0x400U)                 /*!< I2C1_EN (Bitfield-Mask: 0x01)                         */
#define CKGEN_PERI_CLK_EN0_PWM0_EN_Pos    (15U)                    /*!< PWM0_EN (Bit 15)                                      */
#define CKGEN_PERI_CLK_EN0_PWM0_EN_Msk    (0x8000U)                /*!< PWM0_EN (Bitfield-Mask: 0x01)                         */
#define CKGEN_PERI_CLK_EN0_PWM1_EN_Pos    (16U)                    /*!< PWM1_EN (Bit 16)                                      */
#define CKGEN_PERI_CLK_EN0_PWM1_EN_Msk    (0x10000U)               /*!< PWM1_EN (Bitfield-Mask: 0x01)                         */
#define CKGEN_PERI_CLK_EN0_PWM2_EN_Pos    (17U)                    /*!< PWM2_EN (Bit 17)                                      */
#define CKGEN_PERI_CLK_EN0_PWM2_EN_Msk    (0x20000U)               /*!< PWM2_EN (Bitfield-Mask: 0x01)                         */
#define CKGEN_PERI_CLK_EN0_PWM3_EN_Pos    (18U)                    /*!< PWM3_EN (Bit 18)                                      */
#define CKGEN_PERI_CLK_EN0_PWM3_EN_Msk    (0x40000U)               /*!< PWM3_EN (Bitfield-Mask: 0x01)                         */
/* =====================================================  PERI_CLK_EN1  ====================================================== */
#define CKGEN_PERI_CLK_EN1_RTC_EN_Pos     (1U)                     /*!< RTC_EN (Bit 1)                                        */
#define CKGEN_PERI_CLK_EN1_RTC_EN_Msk     (0x2U)                   /*!< RTC_EN (Bitfield-Mask: 0x01)                          */
#define CKGEN_PERI_CLK_EN1_DMA_EN_Pos     (2U)                     /*!< DMA_EN (Bit 2)                                        */
#define CKGEN_PERI_CLK_EN1_DMA_EN_Msk     (0x4U)                   /*!< DMA_EN (Bitfield-Mask: 0x01)                          */
#define CKGEN_PERI_CLK_EN1_GPIO_EN_Pos    (5U)                     /*!< GPIO_EN (Bit 5)                                       */
#define CKGEN_PERI_CLK_EN1_GPIO_EN_Msk    (0x20U)                  /*!< GPIO_EN (Bitfield-Mask: 0x01)                         */
#define CKGEN_PERI_CLK_EN1_WDG_EN_Pos     (6U)                     /*!< WDG_EN (Bit 6)                                        */
#define CKGEN_PERI_CLK_EN1_WDG_EN_Msk     (0x40U)                  /*!< WDG_EN (Bitfield-Mask: 0x01)                          */
#define CKGEN_PERI_CLK_EN1_CRC_EN_Pos     (8U)                     /*!< CRC_EN (Bit 8)                                        */
#define CKGEN_PERI_CLK_EN1_CRC_EN_Msk     (0x100U)                 /*!< CRC_EN (Bitfield-Mask: 0x01)                          */
#define CKGEN_PERI_CLK_EN1_CAN0_EN_Pos    (9U)                     /*!< CAN0_EN (Bit 9)                                       */
#define CKGEN_PERI_CLK_EN1_CAN0_EN_Msk    (0x200U)                 /*!< CAN0_EN (Bitfield-Mask: 0x01)                         */
#define CKGEN_PERI_CLK_EN1_CAN1_EN_Pos    (10U)                    /*!< CAN1_EN (Bit 10)                                      */
#define CKGEN_PERI_CLK_EN1_CAN1_EN_Msk    (0x400U)                 /*!< CAN1_EN (Bitfield-Mask: 0x01)                         */
/* =====================================================  PERI_CLK_EN2  ====================================================== */
#define CKGEN_PERI_CLK_EN2_CTU_EN_Pos     (1U)                     /*!< CTU_EN (Bit 1)                                        */
#define CKGEN_PERI_CLK_EN2_CTU_EN_Msk     (0x2U)                   /*!< CTU_EN (Bitfield-Mask: 0x01)                          */
#define CKGEN_PERI_CLK_EN2_ACMP_EN_Pos    (8U)                     /*!< ACMP_EN (Bit 8)                                       */
#define CKGEN_PERI_CLK_EN2_ACMP_EN_Msk    (0x100U)                 /*!< ACMP_EN (Bitfield-Mask: 0x01)                         */
#define CKGEN_PERI_CLK_EN2_ADC0_EN_Pos    (11U)                    /*!< ADC0_EN (Bit 11)                                      */
#define CKGEN_PERI_CLK_EN2_ADC0_EN_Msk    (0x800U)                 /*!< ADC0_EN (Bitfield-Mask: 0x01)                         */
#define CKGEN_PERI_CLK_EN2_TIMER_EN_Pos   (13U)                    /*!< TIMER_EN (Bit 13)                                     */
#define CKGEN_PERI_CLK_EN2_TIMER_EN_Msk   (0x2000U)                /*!< TIMER_EN (Bitfield-Mask: 0x01)                        */
#define CKGEN_PERI_CLK_EN2_EIO_EN_Pos     (14U)                    /*!< EIO_EN (Bit 14)                                       */
#define CKGEN_PERI_CLK_EN2_EIO_EN_Msk     (0x4000U)                /*!< EIO_EN (Bitfield-Mask: 0x01)                          */
#define CKGEN_PERI_CLK_EN2_SENT_EN_Pos    (15U)                    /*!< SENT_EN (Bit 15)                                      */
#define CKGEN_PERI_CLK_EN2_SENT_EN_Msk    (0x8000U)                /*!< SENT_EN (Bitfield-Mask: 0x01)                         */
#define CKGEN_PERI_CLK_EN2_SMU_EN_Pos     (19U)                    /*!< SMU_EN (Bit 19)                                       */
#define CKGEN_PERI_CLK_EN2_SMU_EN_Msk     (0x80000U)               /*!< SMU_EN (Bitfield-Mask: 0x01)                          */
/* =======================================================  RCM_CTRL  ======================================================== */
#define CKGEN_RCM_CTRL_EXT_RST_FILTER_EN_Pos (0U)                  /*!< EXT_RST_FILTER_EN (Bit 0)                             */
#define CKGEN_RCM_CTRL_EXT_RST_FILTER_EN_Msk (0x1U)                /*!< EXT_RST_FILTER_EN (Bitfield-Mask: 0x01)               */
#define CKGEN_RCM_CTRL_EXT_RST_FILTER_VAL_Pos (1U)                 /*!< EXT_RST_FILTER_VAL (Bit 1)                            */
#define CKGEN_RCM_CTRL_EXT_RST_FILTER_VAL_Msk (0xfeU)              /*!< EXT_RST_FILTER_VAL (Bitfield-Mask: 0x7f)              */
#define CKGEN_RCM_CTRL_RST_DLY_TIME_Pos   (8U)                     /*!< RST_DLY_TIME (Bit 8)                                  */
#define CKGEN_RCM_CTRL_RST_DLY_TIME_Msk   (0x300U)                 /*!< RST_DLY_TIME (Bitfield-Mask: 0x03)                    */
#define CKGEN_RCM_CTRL_SW_RST_INT_EN_Pos  (10U)                    /*!< SW_RST_INT_EN (Bit 10)                                */
#define CKGEN_RCM_CTRL_SW_RST_INT_EN_Msk  (0x400U)                 /*!< SW_RST_INT_EN (Bitfield-Mask: 0x01)                   */
#define CKGEN_RCM_CTRL_LOCKUP_RST_INT_EN_Pos (11U)                 /*!< LOCKUP_RST_INT_EN (Bit 11)                            */
#define CKGEN_RCM_CTRL_LOCKUP_RST_INT_EN_Msk (0x800U)              /*!< LOCKUP_RST_INT_EN (Bitfield-Mask: 0x01)               */
#define CKGEN_RCM_CTRL_ACK_ERR_RST_INT_EN_Pos (12U)                /*!< ACK_ERR_RST_INT_EN (Bit 12)                           */
#define CKGEN_RCM_CTRL_ACK_ERR_RST_INT_EN_Msk (0x1000U)            /*!< ACK_ERR_RST_INT_EN (Bitfield-Mask: 0x01)              */
#define CKGEN_RCM_CTRL_WDG_RST_INT_EN_Pos (13U)                    /*!< WDG_RST_INT_EN (Bit 13)                               */
#define CKGEN_RCM_CTRL_WDG_RST_INT_EN_Msk (0x2000U)                /*!< WDG_RST_INT_EN (Bitfield-Mask: 0x01)                  */
#define CKGEN_RCM_CTRL_XOSC_LOSS_RST_INT_EN_Pos (15U)              /*!< XOSC_LOSS_RST_INT_EN (Bit 15)                         */
#define CKGEN_RCM_CTRL_XOSC_LOSS_RST_INT_EN_Msk (0x8000U)          /*!< XOSC_LOSS_RST_INT_EN (Bitfield-Mask: 0x01)            */
#define CKGEN_RCM_CTRL_PLL_UNLOCK_RST_INT_EN_Pos (16U)             /*!< PLL_UNLOCK_RST_INT_EN (Bit 16)                        */
#define CKGEN_RCM_CTRL_PLL_UNLOCK_RST_INT_EN_Msk (0x10000U)        /*!< PLL_UNLOCK_RST_INT_EN (Bitfield-Mask: 0x01)           */
#define CKGEN_RCM_CTRL_VHSI_LOSS_RST_INT_EN_Pos (17U)              /*!< VHSI_LOSS_RST_INT_EN (Bit 17)                         */
#define CKGEN_RCM_CTRL_VHSI_LOSS_RST_INT_EN_Msk (0x20000U)         /*!< VHSI_LOSS_RST_INT_EN (Bitfield-Mask: 0x01)            */
#define CKGEN_RCM_CTRL_SMU_ERR_RST_INT_EN_Pos (19U)                /*!< SMU_ERR_RST_INT_EN (Bit 19)                           */
#define CKGEN_RCM_CTRL_SMU_ERR_RST_INT_EN_Msk (0x80000U)           /*!< SMU_ERR_RST_INT_EN (Bitfield-Mask: 0x01)              */
#define CKGEN_RCM_CTRL_ECC2_ERR_RST_INT_EN_Pos (20U)               /*!< ECC2_ERR_RST_INT_EN (Bit 20)                          */
#define CKGEN_RCM_CTRL_ECC2_ERR_RST_INT_EN_Msk (0x100000U)         /*!< ECC2_ERR_RST_INT_EN (Bitfield-Mask: 0x01)             */
#define CKGEN_RCM_CTRL_RST_CLK_SEL_Pos    (22U)                    /*!< RST_CLK_SEL (Bit 22)                                  */
#define CKGEN_RCM_CTRL_RST_CLK_SEL_Msk    (0x400000U)              /*!< RST_CLK_SEL (Bitfield-Mask: 0x01)                     */
#define CKGEN_RCM_CTRL_RST_GLB_INT_EN_Pos (23U)                    /*!< RST_GLB_INT_EN (Bit 23)                               */
#define CKGEN_RCM_CTRL_RST_GLB_INT_EN_Msk (0x800000U)              /*!< RST_GLB_INT_EN (Bitfield-Mask: 0x01)                  */
#define CKGEN_RCM_CTRL_RST_CLK_AUTO_SEL_Pos (31U)                  /*!< RST_CLK_AUTO_SEL (Bit 31)                             */
#define CKGEN_RCM_CTRL_RST_CLK_AUTO_SEL_Msk (0x80000000U)          /*!< RST_CLK_AUTO_SEL (Bitfield-Mask: 0x01)                */
/* ========================================================  RCM_EN  ========================================================= */
#define CKGEN_RCM_EN_SW_RST_EN_Pos        (0U)                     /*!< SW_RST_EN (Bit 0)                                     */
#define CKGEN_RCM_EN_SW_RST_EN_Msk        (0x1U)                   /*!< SW_RST_EN (Bitfield-Mask: 0x01)                       */
#define CKGEN_RCM_EN_LOCKUP_RST_EN_Pos    (1U)                     /*!< LOCKUP_RST_EN (Bit 1)                                 */
#define CKGEN_RCM_EN_LOCKUP_RST_EN_Msk    (0x2U)                   /*!< LOCKUP_RST_EN (Bitfield-Mask: 0x01)                   */
#define CKGEN_RCM_EN_ACK_ERR_RST_EN_Pos   (2U)                     /*!< ACK_ERR_RST_EN (Bit 2)                                */
#define CKGEN_RCM_EN_ACK_ERR_RST_EN_Msk   (0x4U)                   /*!< ACK_ERR_RST_EN (Bitfield-Mask: 0x01)                  */
#define CKGEN_RCM_EN_WDG_RST_EN_Pos       (3U)                     /*!< WDG_RST_EN (Bit 3)                                    */
#define CKGEN_RCM_EN_WDG_RST_EN_Msk       (0x8U)                   /*!< WDG_RST_EN (Bitfield-Mask: 0x01)                      */
#define CKGEN_RCM_EN_XOSC_LOSS_RST_EN_Pos (5U)                     /*!< XOSC_LOSS_RST_EN (Bit 5)                              */
#define CKGEN_RCM_EN_XOSC_LOSS_RST_EN_Msk (0x20U)                  /*!< XOSC_LOSS_RST_EN (Bitfield-Mask: 0x01)                */
#define CKGEN_RCM_EN_PLL_UNLOCK_RST_EN_Pos (6U)                    /*!< PLL_UNLOCK_RST_EN (Bit 6)                             */
#define CKGEN_RCM_EN_PLL_UNLOCK_RST_EN_Msk (0x40U)                 /*!< PLL_UNLOCK_RST_EN (Bitfield-Mask: 0x01)               */
#define CKGEN_RCM_EN_VHSI_LOSS_RST_EN_Pos (7U)                     /*!< VHSI_LOSS_RST_EN (Bit 7)                              */
#define CKGEN_RCM_EN_VHSI_LOSS_RST_EN_Msk (0x80U)                  /*!< VHSI_LOSS_RST_EN (Bitfield-Mask: 0x01)                */
#define CKGEN_RCM_EN_EXT_RST_EN_Pos       (8U)                     /*!< EXT_RST_EN (Bit 8)                                    */
#define CKGEN_RCM_EN_EXT_RST_EN_Msk       (0x100U)                 /*!< EXT_RST_EN (Bitfield-Mask: 0x01)                      */
#define CKGEN_RCM_EN_SMU_ERR_RST_EN_Pos   (9U)                     /*!< SMU_ERR_RST_EN (Bit 9)                                */
#define CKGEN_RCM_EN_SMU_ERR_RST_EN_Msk   (0x200U)                 /*!< SMU_ERR_RST_EN (Bitfield-Mask: 0x01)                  */
#define CKGEN_RCM_EN_ECC2_ERR_RST_EN_Pos  (10U)                    /*!< ECC2_ERR_RST_EN (Bit 10)                              */
#define CKGEN_RCM_EN_ECC2_ERR_RST_EN_Msk  (0x400U)                 /*!< ECC2_ERR_RST_EN (Bitfield-Mask: 0x01)                 */
#define CKGEN_RCM_EN_NOT_INIT_SRAM_Pos    (16U)                    /*!< NOT_INIT_SRAM (Bit 16)                                */
#define CKGEN_RCM_EN_NOT_INIT_SRAM_Msk    (0x10000U)               /*!< NOT_INIT_SRAM (Bitfield-Mask: 0x01)                   */
/* ======================================================  RCM_STATUS  ======================================================= */
#define CKGEN_RCM_STATUS_SW_RST_INT_FLAG_Pos (0U)                  /*!< SW_RST_INT_FLAG (Bit 0)                               */
#define CKGEN_RCM_STATUS_SW_RST_INT_FLAG_Msk (0x1U)                /*!< SW_RST_INT_FLAG (Bitfield-Mask: 0x01)                 */
#define CKGEN_RCM_STATUS_LOCKUP_RST_INT_FLAG_Pos (1U)              /*!< LOCKUP_RST_INT_FLAG (Bit 1)                           */
#define CKGEN_RCM_STATUS_LOCKUP_RST_INT_FLAG_Msk (0x2U)            /*!< LOCKUP_RST_INT_FLAG (Bitfield-Mask: 0x01)             */
#define CKGEN_RCM_STATUS_ACK_ERR_RST_INT_FLAG_Pos (2U)             /*!< ACK_ERR_RST_INT_FLAG (Bit 2)                          */
#define CKGEN_RCM_STATUS_ACK_ERR_RST_INT_FLAG_Msk (0x4U)           /*!< ACK_ERR_RST_INT_FLAG (Bitfield-Mask: 0x01)            */
#define CKGEN_RCM_STATUS_WDG_RST_INT_FLAG_Pos (3U)                 /*!< WDG_RST_INT_FLAG (Bit 3)                              */
#define CKGEN_RCM_STATUS_WDG_RST_INT_FLAG_Msk (0x8U)               /*!< WDG_RST_INT_FLAG (Bitfield-Mask: 0x01)                */
#define CKGEN_RCM_STATUS_XOSC_LOSS_RST_INT_FLAG_Pos (5U)           /*!< XOSC_LOSS_RST_INT_FLAG (Bit 5)                        */
#define CKGEN_RCM_STATUS_XOSC_LOSS_RST_INT_FLAG_Msk (0x20U)        /*!< XOSC_LOSS_RST_INT_FLAG (Bitfield-Mask: 0x01)          */
#define CKGEN_RCM_STATUS_PLL_UNLOCK_RST_INT_FLAG_Pos (6U)          /*!< PLL_UNLOCK_RST_INT_FLAG (Bit 6)                       */
#define CKGEN_RCM_STATUS_PLL_UNLOCK_RST_INT_FLAG_Msk (0x40U)       /*!< PLL_UNLOCK_RST_INT_FLAG (Bitfield-Mask: 0x01)         */
#define CKGEN_RCM_STATUS_VHSI_LOSS_RST_INT_FLAG_Pos (7U)           /*!< VHSI_LOSS_RST_INT_FLAG (Bit 7)                        */
#define CKGEN_RCM_STATUS_VHSI_LOSS_RST_INT_FLAG_Msk (0x80U)        /*!< VHSI_LOSS_RST_INT_FLAG (Bitfield-Mask: 0x01)          */
#define CKGEN_RCM_STATUS_SMU_ERR_RST_INT_FLAG_Pos (9U)             /*!< SMU_ERR_RST_INT_FLAG (Bit 9)                          */
#define CKGEN_RCM_STATUS_SMU_ERR_RST_INT_FLAG_Msk (0x200U)         /*!< SMU_ERR_RST_INT_FLAG (Bitfield-Mask: 0x01)            */
#define CKGEN_RCM_STATUS_ECC2_ERR_RST_INT_FLAG_Pos (10U)           /*!< ECC2_ERR_RST_INT_FLAG (Bit 10)                        */
#define CKGEN_RCM_STATUS_ECC2_ERR_RST_INT_FLAG_Msk (0x400U)        /*!< ECC2_ERR_RST_INT_FLAG (Bitfield-Mask: 0x01)           */
#define CKGEN_RCM_STATUS_RST_INT_STATUS_CLR_Pos (15U)              /*!< RST_INT_STATUS_CLR (Bit 15)                           */
#define CKGEN_RCM_STATUS_RST_INT_STATUS_CLR_Msk (0x8000U)          /*!< RST_INT_STATUS_CLR (Bitfield-Mask: 0x01)              */
#define CKGEN_RCM_STATUS_POR_RST_FLAG_Pos (16U)                    /*!< POR_RST_FLAG (Bit 16)                                 */
#define CKGEN_RCM_STATUS_POR_RST_FLAG_Msk (0x10000U)               /*!< POR_RST_FLAG (Bitfield-Mask: 0x01)                    */
#define CKGEN_RCM_STATUS_LVR_RST_FLAG_Pos (17U)                    /*!< LVR_RST_FLAG (Bit 17)                                 */
#define CKGEN_RCM_STATUS_LVR_RST_FLAG_Msk (0x20000U)               /*!< LVR_RST_FLAG (Bitfield-Mask: 0x01)                    */
#define CKGEN_RCM_STATUS_SW_RST_FLAG_Pos  (18U)                    /*!< SW_RST_FLAG (Bit 18)                                  */
#define CKGEN_RCM_STATUS_SW_RST_FLAG_Msk  (0x40000U)               /*!< SW_RST_FLAG (Bitfield-Mask: 0x01)                     */
#define CKGEN_RCM_STATUS_LOCKUP_RST_FLAG_Pos (19U)                 /*!< LOCKUP_RST_FLAG (Bit 19)                              */
#define CKGEN_RCM_STATUS_LOCKUP_RST_FLAG_Msk (0x80000U)            /*!< LOCKUP_RST_FLAG (Bitfield-Mask: 0x01)                 */
#define CKGEN_RCM_STATUS_ACK_ERR_RST_FLAG_Pos (20U)                /*!< ACK_ERR_RST_FLAG (Bit 20)                             */
#define CKGEN_RCM_STATUS_ACK_ERR_RST_FLAG_Msk (0x100000U)          /*!< ACK_ERR_RST_FLAG (Bitfield-Mask: 0x01)                */
#define CKGEN_RCM_STATUS_WDG_RST_FLAG_Pos (21U)                    /*!< WDG_RST_FLAG (Bit 21)                                 */
#define CKGEN_RCM_STATUS_WDG_RST_FLAG_Msk (0x200000U)              /*!< WDG_RST_FLAG (Bitfield-Mask: 0x01)                    */
#define CKGEN_RCM_STATUS_STB_WU_RST_FLAG_Pos (22U)                 /*!< STB_WU_RST_FLAG (Bit 22)                              */
#define CKGEN_RCM_STATUS_STB_WU_RST_FLAG_Msk (0x400000U)           /*!< STB_WU_RST_FLAG (Bitfield-Mask: 0x01)                 */
#define CKGEN_RCM_STATUS_XOSC_LOSS_RST_FLAG_Pos (23U)              /*!< XOSC_LOSS_RST_FLAG (Bit 23)                           */
#define CKGEN_RCM_STATUS_XOSC_LOSS_RST_FLAG_Msk (0x800000U)        /*!< XOSC_LOSS_RST_FLAG (Bitfield-Mask: 0x01)              */
#define CKGEN_RCM_STATUS_PLL_UNLOCK_RST_FLAG_Pos (24U)             /*!< PLL_UNLOCK_RST_FLAG (Bit 24)                          */
#define CKGEN_RCM_STATUS_PLL_UNLOCK_RST_FLAG_Msk (0x1000000U)      /*!< PLL_UNLOCK_RST_FLAG (Bitfield-Mask: 0x01)             */
#define CKGEN_RCM_STATUS_VHSI_LOSS_RST_FLAG_Pos (25U)              /*!< VHSI_LOSS_RST_FLAG (Bit 25)                           */
#define CKGEN_RCM_STATUS_VHSI_LOSS_RST_FLAG_Msk (0x2000000U)       /*!< VHSI_LOSS_RST_FLAG (Bitfield-Mask: 0x01)              */
#define CKGEN_RCM_STATUS_EXT_RST_FLAG_Pos (26U)                    /*!< EXT_RST_FLAG (Bit 26)                                 */
#define CKGEN_RCM_STATUS_EXT_RST_FLAG_Msk (0x4000000U)             /*!< EXT_RST_FLAG (Bitfield-Mask: 0x01)                    */
#define CKGEN_RCM_STATUS_SMU_ERR_RST_FLAG_Pos (27U)                /*!< SMU_ERR_RST_FLAG (Bit 27)                             */
#define CKGEN_RCM_STATUS_SMU_ERR_RST_FLAG_Msk (0x8000000U)         /*!< SMU_ERR_RST_FLAG (Bitfield-Mask: 0x01)                */
#define CKGEN_RCM_STATUS_ECC2_ERR_RST_FLAG_Pos (28U)               /*!< ECC2_ERR_RST_FLAG (Bit 28)                            */
#define CKGEN_RCM_STATUS_ECC2_ERR_RST_FLAG_Msk (0x10000000U)       /*!< ECC2_ERR_RST_FLAG (Bitfield-Mask: 0x01)               */
#define CKGEN_RCM_STATUS_DIG_LVR_RST_FLAG_Pos (29U)                /*!< DIG_LVR_RST_FLAG (Bit 29)                             */
#define CKGEN_RCM_STATUS_DIG_LVR_RST_FLAG_Msk (0x20000000U)        /*!< DIG_LVR_RST_FLAG (Bitfield-Mask: 0x01)                */
#define CKGEN_RCM_STATUS_RST_STATUS_CLR_Pos (31U)                  /*!< RST_STATUS_CLR (Bit 31)                               */
#define CKGEN_RCM_STATUS_RST_STATUS_CLR_Msk (0x80000000U)          /*!< RST_STATUS_CLR (Bitfield-Mask: 0x01)                  */
/* =====================================================  PERI_SFT_RST0  ===================================================== */
#define CKGEN_PERI_SFT_RST0_SRST_UART0_Pos (0U)                    /*!< SRST_UART0 (Bit 0)                                    */
#define CKGEN_PERI_SFT_RST0_SRST_UART0_Msk (0x1U)                  /*!< SRST_UART0 (Bitfield-Mask: 0x01)                      */
#define CKGEN_PERI_SFT_RST0_SRST_UART1_Pos (1U)                    /*!< SRST_UART1 (Bit 1)                                    */
#define CKGEN_PERI_SFT_RST0_SRST_UART1_Msk (0x2U)                  /*!< SRST_UART1 (Bitfield-Mask: 0x01)                      */
#define CKGEN_PERI_SFT_RST0_SRST_UART2_Pos (2U)                    /*!< SRST_UART2 (Bit 2)                                    */
#define CKGEN_PERI_SFT_RST0_SRST_UART2_Msk (0x4U)                  /*!< SRST_UART2 (Bitfield-Mask: 0x01)                      */
#define CKGEN_PERI_SFT_RST0_SRST_SPI0_Pos (6U)                     /*!< SRST_SPI0 (Bit 6)                                     */
#define CKGEN_PERI_SFT_RST0_SRST_SPI0_Msk (0x40U)                  /*!< SRST_SPI0 (Bitfield-Mask: 0x01)                       */
#define CKGEN_PERI_SFT_RST0_SRST_SPI1_Pos (7U)                     /*!< SRST_SPI1 (Bit 7)                                     */
#define CKGEN_PERI_SFT_RST0_SRST_SPI1_Msk (0x80U)                  /*!< SRST_SPI1 (Bitfield-Mask: 0x01)                       */
#define CKGEN_PERI_SFT_RST0_SRST_SPI2_Pos (8U)                     /*!< SRST_SPI2 (Bit 8)                                     */
#define CKGEN_PERI_SFT_RST0_SRST_SPI2_Msk (0x100U)                 /*!< SRST_SPI2 (Bitfield-Mask: 0x01)                       */
#define CKGEN_PERI_SFT_RST0_SRST_I2C0_Pos (9U)                     /*!< SRST_I2C0 (Bit 9)                                     */
#define CKGEN_PERI_SFT_RST0_SRST_I2C0_Msk (0x200U)                 /*!< SRST_I2C0 (Bitfield-Mask: 0x01)                       */
#define CKGEN_PERI_SFT_RST0_SRST_I2C1_Pos (10U)                    /*!< SRST_I2C1 (Bit 10)                                    */
#define CKGEN_PERI_SFT_RST0_SRST_I2C1_Msk (0x400U)                 /*!< SRST_I2C1 (Bitfield-Mask: 0x01)                       */
#define CKGEN_PERI_SFT_RST0_SRST_PWM0_Pos (15U)                    /*!< SRST_PWM0 (Bit 15)                                    */
#define CKGEN_PERI_SFT_RST0_SRST_PWM0_Msk (0x8000U)                /*!< SRST_PWM0 (Bitfield-Mask: 0x01)                       */
#define CKGEN_PERI_SFT_RST0_SRST_PWM1_Pos (16U)                    /*!< SRST_PWM1 (Bit 16)                                    */
#define CKGEN_PERI_SFT_RST0_SRST_PWM1_Msk (0x10000U)               /*!< SRST_PWM1 (Bitfield-Mask: 0x01)                       */
#define CKGEN_PERI_SFT_RST0_SRST_PWM2_Pos (17U)                    /*!< SRST_PWM2 (Bit 17)                                    */
#define CKGEN_PERI_SFT_RST0_SRST_PWM2_Msk (0x20000U)               /*!< SRST_PWM2 (Bitfield-Mask: 0x01)                       */
#define CKGEN_PERI_SFT_RST0_SRST_PWM3_Pos (18U)                    /*!< SRST_PWM3 (Bit 18)                                    */
#define CKGEN_PERI_SFT_RST0_SRST_PWM3_Msk (0x40000U)               /*!< SRST_PWM3 (Bitfield-Mask: 0x01)                       */
/* =====================================================  PERI_SFT_RST1  ===================================================== */
#define CKGEN_PERI_SFT_RST1_SRST_DMA_Pos  (2U)                     /*!< SRST_DMA (Bit 2)                                      */
#define CKGEN_PERI_SFT_RST1_SRST_DMA_Msk  (0x4U)                   /*!< SRST_DMA (Bitfield-Mask: 0x01)                        */
#define CKGEN_PERI_SFT_RST1_SRST_GPIO_Pos (5U)                     /*!< SRST_GPIO (Bit 5)                                     */
#define CKGEN_PERI_SFT_RST1_SRST_GPIO_Msk (0x20U)                  /*!< SRST_GPIO (Bitfield-Mask: 0x01)                       */
#define CKGEN_PERI_SFT_RST1_SRST_WDG_Pos  (6U)                     /*!< SRST_WDG (Bit 6)                                      */
#define CKGEN_PERI_SFT_RST1_SRST_WDG_Msk  (0x40U)                  /*!< SRST_WDG (Bitfield-Mask: 0x01)                        */
#define CKGEN_PERI_SFT_RST1_SRST_CRC_Pos  (8U)                     /*!< SRST_CRC (Bit 8)                                      */
#define CKGEN_PERI_SFT_RST1_SRST_CRC_Msk  (0x100U)                 /*!< SRST_CRC (Bitfield-Mask: 0x01)                        */
#define CKGEN_PERI_SFT_RST1_SRST_CAN0_Pos (9U)                     /*!< SRST_CAN0 (Bit 9)                                     */
#define CKGEN_PERI_SFT_RST1_SRST_CAN0_Msk (0x200U)                 /*!< SRST_CAN0 (Bitfield-Mask: 0x01)                       */
#define CKGEN_PERI_SFT_RST1_SRST_CAN1_Pos (10U)                    /*!< SRST_CAN1 (Bit 10)                                    */
#define CKGEN_PERI_SFT_RST1_SRST_CAN1_Msk (0x400U)                 /*!< SRST_CAN1 (Bitfield-Mask: 0x01)                       */
/* =====================================================  PERI_SFT_RST2  ===================================================== */
#define CKGEN_PERI_SFT_RST2_SRST_CTU_Pos  (1U)                     /*!< SRST_CTU (Bit 1)                                      */
#define CKGEN_PERI_SFT_RST2_SRST_CTU_Msk  (0x2U)                   /*!< SRST_CTU (Bitfield-Mask: 0x01)                        */
#define CKGEN_PERI_SFT_RST2_SRST_ACMP_Pos (8U)                     /*!< SRST_ACMP (Bit 8)                                     */
#define CKGEN_PERI_SFT_RST2_SRST_ACMP_Msk (0x100U)                 /*!< SRST_ACMP (Bitfield-Mask: 0x01)                       */
#define CKGEN_PERI_SFT_RST2_SRST_ADC0_Pos (11U)                    /*!< SRST_ADC0 (Bit 11)                                    */
#define CKGEN_PERI_SFT_RST2_SRST_ADC0_Msk (0x800U)                 /*!< SRST_ADC0 (Bitfield-Mask: 0x01)                       */
#define CKGEN_PERI_SFT_RST2_SRST_TIMER_Pos (13U)                   /*!< SRST_TIMER (Bit 13)                                   */
#define CKGEN_PERI_SFT_RST2_SRST_TIMER_Msk (0x2000U)               /*!< SRST_TIMER (Bitfield-Mask: 0x01)                      */
#define CKGEN_PERI_SFT_RST2_SRST_EIO_Pos  (14U)                    /*!< SRST_EIO (Bit 14)                                     */
#define CKGEN_PERI_SFT_RST2_SRST_EIO_Msk  (0x4000U)                /*!< SRST_EIO (Bitfield-Mask: 0x01)                        */
#define CKGEN_PERI_SFT_RST2_SRST_SENT_Pos (15U)                    /*!< SRST_SENT (Bit 15)                                    */
#define CKGEN_PERI_SFT_RST2_SRST_SENT_Msk (0x8000U)                /*!< SRST_SENT (Bitfield-Mask: 0x01)                       */
/* =======================================================  CLK_DIV1  ======================================================== */
#define CKGEN_CLK_DIV1_SPLL_DIV1_Pos      (0U)                     /*!< SPLL_DIV1 (Bit 0)                                     */
#define CKGEN_CLK_DIV1_SPLL_DIV1_Msk      (0x3fU)                  /*!< SPLL_DIV1 (Bitfield-Mask: 0x3f)                       */
#define CKGEN_CLK_DIV1_VHSI_DIV1_Pos      (6U)                     /*!< VHSI_DIV1 (Bit 6)                                     */
#define CKGEN_CLK_DIV1_VHSI_DIV1_Msk      (0xfc0U)                 /*!< VHSI_DIV1 (Bitfield-Mask: 0x3f)                       */
#define CKGEN_CLK_DIV1_VHSI_8M_DIV1_Pos   (12U)                    /*!< VHSI_8M_DIV1 (Bit 12)                                 */
#define CKGEN_CLK_DIV1_VHSI_8M_DIV1_Msk   (0x3f000U)               /*!< VHSI_8M_DIV1 (Bitfield-Mask: 0x3f)                    */
#define CKGEN_CLK_DIV1_HSE_DIV1_Pos       (18U)                    /*!< HSE_DIV1 (Bit 18)                                     */
#define CKGEN_CLK_DIV1_HSE_DIV1_Msk       (0xfc0000U)              /*!< HSE_DIV1 (Bitfield-Mask: 0x3f)                        */
/* =======================================================  CLK_DIV2  ======================================================== */
#define CKGEN_CLK_DIV2_SPLL_DIV2_Pos      (0U)                     /*!< SPLL_DIV2 (Bit 0)                                     */
#define CKGEN_CLK_DIV2_SPLL_DIV2_Msk      (0x3fU)                  /*!< SPLL_DIV2 (Bitfield-Mask: 0x3f)                       */
#define CKGEN_CLK_DIV2_VHSI_DIV2_Pos      (6U)                     /*!< VHSI_DIV2 (Bit 6)                                     */
#define CKGEN_CLK_DIV2_VHSI_DIV2_Msk      (0xfc0U)                 /*!< VHSI_DIV2 (Bitfield-Mask: 0x3f)                       */
#define CKGEN_CLK_DIV2_VHSI_8M_DIV2_Pos   (12U)                    /*!< VHSI_8M_DIV2 (Bit 12)                                 */
#define CKGEN_CLK_DIV2_VHSI_8M_DIV2_Msk   (0x3f000U)               /*!< VHSI_8M_DIV2 (Bitfield-Mask: 0x3f)                    */
#define CKGEN_CLK_DIV2_HSE_DIV2_Pos       (18U)                    /*!< HSE_DIV2 (Bit 18)                                     */
#define CKGEN_CLK_DIV2_HSE_DIV2_Msk       (0xfc0000U)              /*!< HSE_DIV2 (Bitfield-Mask: 0x3f)                        */
/* =====================================================  PERI_CLK_MUX0  ===================================================== */
#define CKGEN_PERI_CLK_MUX0_I2C0_MUX_Pos  (0U)                     /*!< I2C0_MUX (Bit 0)                                      */
#define CKGEN_PERI_CLK_MUX0_I2C0_MUX_Msk  (0x7U)                   /*!< I2C0_MUX (Bitfield-Mask: 0x07)                        */
#define CKGEN_PERI_CLK_MUX0_I2C1_MUX_Pos  (4U)                     /*!< I2C1_MUX (Bit 4)                                      */
#define CKGEN_PERI_CLK_MUX0_I2C1_MUX_Msk  (0x70U)                  /*!< I2C1_MUX (Bitfield-Mask: 0x07)                        */
#define CKGEN_PERI_CLK_MUX0_TIMER_MUX_Pos (8U)                     /*!< TIMER_MUX (Bit 8)                                     */
#define CKGEN_PERI_CLK_MUX0_TIMER_MUX_Msk (0x700U)                 /*!< TIMER_MUX (Bitfield-Mask: 0x07)                       */
#define CKGEN_PERI_CLK_MUX0_SPI0_MUX_Pos  (12U)                    /*!< SPI0_MUX (Bit 12)                                     */
#define CKGEN_PERI_CLK_MUX0_SPI0_MUX_Msk  (0x7000U)                /*!< SPI0_MUX (Bitfield-Mask: 0x07)                        */
#define CKGEN_PERI_CLK_MUX0_SPI1_MUX_Pos  (16U)                    /*!< SPI1_MUX (Bit 16)                                     */
#define CKGEN_PERI_CLK_MUX0_SPI1_MUX_Msk  (0x70000U)               /*!< SPI1_MUX (Bitfield-Mask: 0x07)                        */
#define CKGEN_PERI_CLK_MUX0_SPI2_MUX_Pos  (20U)                    /*!< SPI2_MUX (Bit 20)                                     */
#define CKGEN_PERI_CLK_MUX0_SPI2_MUX_Msk  (0x700000U)              /*!< SPI2_MUX (Bitfield-Mask: 0x07)                        */
#define CKGEN_PERI_CLK_MUX0_ADC0_MUX_Pos  (24U)                    /*!< ADC0_MUX (Bit 24)                                     */
#define CKGEN_PERI_CLK_MUX0_ADC0_MUX_Msk  (0x7000000U)             /*!< ADC0_MUX (Bitfield-Mask: 0x07)                        */
/* =====================================================  PERI_CLK_MUX1  ===================================================== */
#define CKGEN_PERI_CLK_MUX1_CAN0_MUX_Pos  (0U)                     /*!< CAN0_MUX (Bit 0)                                      */
#define CKGEN_PERI_CLK_MUX1_CAN0_MUX_Msk  (0x7U)                   /*!< CAN0_MUX (Bitfield-Mask: 0x07)                        */
#define CKGEN_PERI_CLK_MUX1_CAN1_MUX_Pos  (4U)                     /*!< CAN1_MUX (Bit 4)                                      */
#define CKGEN_PERI_CLK_MUX1_CAN1_MUX_Msk  (0x70U)                  /*!< CAN1_MUX (Bitfield-Mask: 0x07)                        */
#define CKGEN_PERI_CLK_MUX1_EIO_MUX_Pos   (16U)                    /*!< EIO_MUX (Bit 16)                                      */
#define CKGEN_PERI_CLK_MUX1_EIO_MUX_Msk   (0x70000U)               /*!< EIO_MUX (Bitfield-Mask: 0x07)                         */
/* =====================================================  PERI_CLK_MUX2  ===================================================== */
#define CKGEN_PERI_CLK_MUX2_UART0_MUX_Pos (0U)                     /*!< UART0_MUX (Bit 0)                                     */
#define CKGEN_PERI_CLK_MUX2_UART0_MUX_Msk (0x7U)                   /*!< UART0_MUX (Bitfield-Mask: 0x07)                       */
#define CKGEN_PERI_CLK_MUX2_UART1_MUX_Pos (4U)                     /*!< UART1_MUX (Bit 4)                                     */
#define CKGEN_PERI_CLK_MUX2_UART1_MUX_Msk (0x70U)                  /*!< UART1_MUX (Bitfield-Mask: 0x07)                       */
#define CKGEN_PERI_CLK_MUX2_UART2_MUX_Pos (8U)                     /*!< UART2_MUX (Bit 8)                                     */
#define CKGEN_PERI_CLK_MUX2_UART2_MUX_Msk (0x700U)                 /*!< UART2_MUX (Bitfield-Mask: 0x07)                       */
/* =====================================================  PERI_CLK_MUX3  ===================================================== */
#define CKGEN_PERI_CLK_MUX3_PWM0_EXT_Pos  (0U)                     /*!< PWM0_EXT (Bit 0)                                      */
#define CKGEN_PERI_CLK_MUX3_PWM0_EXT_Msk  (0x3U)                   /*!< PWM0_EXT (Bitfield-Mask: 0x03)                        */
#define CKGEN_PERI_CLK_MUX3_PWM0_INT_Pos  (2U)                     /*!< PWM0_INT (Bit 2)                                      */
#define CKGEN_PERI_CLK_MUX3_PWM0_INT_Msk  (0xcU)                   /*!< PWM0_INT (Bitfield-Mask: 0x03)                        */
#define CKGEN_PERI_CLK_MUX3_PWM0_EXT_IN_Pos (4U)                   /*!< PWM0_EXT_IN (Bit 4)                                   */
#define CKGEN_PERI_CLK_MUX3_PWM0_EXT_IN_Msk (0x10U)                /*!< PWM0_EXT_IN (Bitfield-Mask: 0x01)                     */
#define CKGEN_PERI_CLK_MUX3_PWM1_EXT_Pos  (5U)                     /*!< PWM1_EXT (Bit 5)                                      */
#define CKGEN_PERI_CLK_MUX3_PWM1_EXT_Msk  (0x60U)                  /*!< PWM1_EXT (Bitfield-Mask: 0x03)                        */
#define CKGEN_PERI_CLK_MUX3_PWM1_INT_Pos  (7U)                     /*!< PWM1_INT (Bit 7)                                      */
#define CKGEN_PERI_CLK_MUX3_PWM1_INT_Msk  (0x180U)                 /*!< PWM1_INT (Bitfield-Mask: 0x03)                        */
#define CKGEN_PERI_CLK_MUX3_PWM1_EXT_IN_Pos (9U)                   /*!< PWM1_EXT_IN (Bit 9)                                   */
#define CKGEN_PERI_CLK_MUX3_PWM1_EXT_IN_Msk (0x200U)               /*!< PWM1_EXT_IN (Bitfield-Mask: 0x01)                     */
#define CKGEN_PERI_CLK_MUX3_PWM2_EXT_Pos  (10U)                    /*!< PWM2_EXT (Bit 10)                                     */
#define CKGEN_PERI_CLK_MUX3_PWM2_EXT_Msk  (0xc00U)                 /*!< PWM2_EXT (Bitfield-Mask: 0x03)                        */
#define CKGEN_PERI_CLK_MUX3_PWM2_INT_Pos  (12U)                    /*!< PWM2_INT (Bit 12)                                     */
#define CKGEN_PERI_CLK_MUX3_PWM2_INT_Msk  (0x3000U)                /*!< PWM2_INT (Bitfield-Mask: 0x03)                        */
#define CKGEN_PERI_CLK_MUX3_PWM2_EXT_IN_Pos (14U)                  /*!< PWM2_EXT_IN (Bit 14)                                  */
#define CKGEN_PERI_CLK_MUX3_PWM2_EXT_IN_Msk (0x4000U)              /*!< PWM2_EXT_IN (Bitfield-Mask: 0x01)                     */
#define CKGEN_PERI_CLK_MUX3_PWM3_EXT_Pos  (15U)                    /*!< PWM3_EXT (Bit 15)                                     */
#define CKGEN_PERI_CLK_MUX3_PWM3_EXT_Msk  (0x18000U)               /*!< PWM3_EXT (Bitfield-Mask: 0x03)                        */
#define CKGEN_PERI_CLK_MUX3_PWM3_INT_Pos  (17U)                    /*!< PWM3_INT (Bit 17)                                     */
#define CKGEN_PERI_CLK_MUX3_PWM3_INT_Msk  (0x60000U)               /*!< PWM3_INT (Bitfield-Mask: 0x03)                        */
#define CKGEN_PERI_CLK_MUX3_PWM3_EXT_IN_Pos (19U)                  /*!< PWM3_EXT_IN (Bit 19)                                  */
#define CKGEN_PERI_CLK_MUX3_PWM3_EXT_IN_Msk (0x80000U)             /*!< PWM3_EXT_IN (Bitfield-Mask: 0x01)                     */
/* ======================================================  CLK_OUT_CFG  ====================================================== */
#define CKGEN_CLK_OUT_CFG_MUX1_Pos        (0U)                     /*!< MUX1 (Bit 0)                                          */
#define CKGEN_CLK_OUT_CFG_MUX1_Msk        (0x7U)                   /*!< MUX1 (Bitfield-Mask: 0x07)                            */
#define CKGEN_CLK_OUT_CFG_MUX2_Pos        (4U)                     /*!< MUX2 (Bit 4)                                          */
#define CKGEN_CLK_OUT_CFG_MUX2_Msk        (0xf0U)                  /*!< MUX2 (Bitfield-Mask: 0x0f)                            */
#define CKGEN_CLK_OUT_CFG_DIV_Pos         (8U)                     /*!< DIV (Bit 8)                                           */
#define CKGEN_CLK_OUT_CFG_DIV_Msk         (0x700U)                 /*!< DIV (Bitfield-Mask: 0x07)                             */
#define CKGEN_CLK_OUT_CFG_ENABLE_Pos      (11U)                    /*!< ENABLE (Bit 11)                                       */
#define CKGEN_CLK_OUT_CFG_ENABLE_Msk      (0x800U)                 /*!< ENABLE (Bitfield-Mask: 0x01)                          */
/* =====================================================  PERI_CLK_DIV  ====================================================== */
#define CKGEN_PERI_CLK_DIV_CAN0_DIV_Pos   (0U)                     /*!< CAN0_DIV (Bit 0)                                      */
#define CKGEN_PERI_CLK_DIV_CAN0_DIV_Msk   (0x3U)                   /*!< CAN0_DIV (Bitfield-Mask: 0x03)                        */
#define CKGEN_PERI_CLK_DIV_CAN1_DIV_Pos   (2U)                     /*!< CAN1_DIV (Bit 2)                                      */
#define CKGEN_PERI_CLK_DIV_CAN1_DIV_Msk   (0xcU)                   /*!< CAN1_DIV (Bitfield-Mask: 0x03)                        */
#define CKGEN_PERI_CLK_DIV_CAN0_TS_DIV_Pos (6U)                    /*!< CAN0_TS_DIV (Bit 6)                                   */
#define CKGEN_PERI_CLK_DIV_CAN0_TS_DIV_Msk (0x1c0U)                /*!< CAN0_TS_DIV (Bitfield-Mask: 0x07)                     */
#define CKGEN_PERI_CLK_DIV_CAN1_TS_DIV_Pos (9U)                    /*!< CAN1_TS_DIV (Bit 9)                                   */
#define CKGEN_PERI_CLK_DIV_CAN1_TS_DIV_Msk (0xe00U)                /*!< CAN1_TS_DIV (Bitfield-Mask: 0x07)                     */
#define CKGEN_PERI_CLK_DIV_TPIU_DIV_Pos   (28U)                    /*!< TPIU_DIV (Bit 28)                                     */
#define CKGEN_PERI_CLK_DIV_TPIU_DIV_Msk   (0xf0000000U)            /*!< TPIU_DIV (Bitfield-Mask: 0x0f)                        */
/* =====================================================  PERI_CLK_DIV1  ===================================================== */
#define CKGEN_PERI_CLK_DIV1_EIO_DIV_Pos   (0U)                     /*!< EIO_DIV (Bit 0)                                       */
#define CKGEN_PERI_CLK_DIV1_EIO_DIV_Msk   (0xffU)                  /*!< EIO_DIV (Bitfield-Mask: 0xff)                         */

/* =========================================================================================================================== */
/* ================                                            PBR                                            ================ */
/* =========================================================================================================================== */

/* =======================================================  MPR_CORE  ======================================================== */
#define PBR_MPR_CORE_MPL_Pos              (0U)                     /*!< MPL (Bit 0)                                           */
#define PBR_MPR_CORE_MPL_Msk              (0x1U)                   /*!< MPL (Bitfield-Mask: 0x01)                             */
#define PBR_MPR_CORE_MTW_Pos              (1U)                     /*!< MTW (Bit 1)                                           */
#define PBR_MPR_CORE_MTW_Msk              (0x2U)                   /*!< MTW (Bitfield-Mask: 0x01)                             */
#define PBR_MPR_CORE_MTR_Pos              (2U)                     /*!< MTR (Bit 2)                                           */
#define PBR_MPR_CORE_MTR_Msk              (0x4U)                   /*!< MTR (Bitfield-Mask: 0x01)                             */
/* =======================================================  MPR_DEBUG  ======================================================= */
#define PBR_MPR_DEBUG_MPL_Pos             (0U)                     /*!< MPL (Bit 0)                                           */
#define PBR_MPR_DEBUG_MPL_Msk             (0x1U)                   /*!< MPL (Bitfield-Mask: 0x01)                             */
#define PBR_MPR_DEBUG_MTW_Pos             (1U)                     /*!< MTW (Bit 1)                                           */
#define PBR_MPR_DEBUG_MTW_Msk             (0x2U)                   /*!< MTW (Bitfield-Mask: 0x01)                             */
#define PBR_MPR_DEBUG_MTR_Pos             (2U)                     /*!< MTR (Bit 2)                                           */
#define PBR_MPR_DEBUG_MTR_Msk             (0x4U)                   /*!< MTR (Bitfield-Mask: 0x01)                             */
/* ========================================================  MPR_DMA  ======================================================== */
#define PBR_MPR_DMA_MPL_Pos               (0U)                     /*!< MPL (Bit 0)                                           */
#define PBR_MPR_DMA_MPL_Msk               (0x1U)                   /*!< MPL (Bitfield-Mask: 0x01)                             */
#define PBR_MPR_DMA_MTW_Pos               (1U)                     /*!< MTW (Bit 1)                                           */
#define PBR_MPR_DMA_MTW_Msk               (0x2U)                   /*!< MTW (Bitfield-Mask: 0x01)                             */
#define PBR_MPR_DMA_MTR_Pos               (2U)                     /*!< MTR (Bit 2)                                           */
#define PBR_MPR_DMA_MTR_Msk               (0x4U)                   /*!< MTR (Bitfield-Mask: 0x01)                             */
/* =========================================================  PACRA  ========================================================= */
#define PBR_PACRA_PWM0_TP_Pos             (0U)                     /*!< PWM0_TP (Bit 0)                                       */
#define PBR_PACRA_PWM0_TP_Msk             (0x1U)                   /*!< PWM0_TP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRA_PWM0_WP_Pos             (1U)                     /*!< PWM0_WP (Bit 1)                                       */
#define PBR_PACRA_PWM0_WP_Msk             (0x2U)                   /*!< PWM0_WP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRA_PWM0_SP_Pos             (2U)                     /*!< PWM0_SP (Bit 2)                                       */
#define PBR_PACRA_PWM0_SP_Msk             (0x4U)                   /*!< PWM0_SP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRA_PWM1_TP_Pos             (4U)                     /*!< PWM1_TP (Bit 4)                                       */
#define PBR_PACRA_PWM1_TP_Msk             (0x10U)                  /*!< PWM1_TP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRA_PWM1_WP_Pos             (5U)                     /*!< PWM1_WP (Bit 5)                                       */
#define PBR_PACRA_PWM1_WP_Msk             (0x20U)                  /*!< PWM1_WP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRA_PWM1_SP_Pos             (6U)                     /*!< PWM1_SP (Bit 6)                                       */
#define PBR_PACRA_PWM1_SP_Msk             (0x40U)                  /*!< PWM1_SP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRA_PWM2_TP_Pos             (8U)                     /*!< PWM2_TP (Bit 8)                                       */
#define PBR_PACRA_PWM2_TP_Msk             (0x100U)                 /*!< PWM2_TP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRA_PWM2_WP_Pos             (9U)                     /*!< PWM2_WP (Bit 9)                                       */
#define PBR_PACRA_PWM2_WP_Msk             (0x200U)                 /*!< PWM2_WP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRA_PWM2_SP_Pos             (10U)                    /*!< PWM2_SP (Bit 10)                                      */
#define PBR_PACRA_PWM2_SP_Msk             (0x400U)                 /*!< PWM2_SP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRA_PWM3_TP_Pos             (12U)                    /*!< PWM3_TP (Bit 12)                                      */
#define PBR_PACRA_PWM3_TP_Msk             (0x1000U)                /*!< PWM3_TP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRA_PWM3_WP_Pos             (13U)                    /*!< PWM3_WP (Bit 13)                                      */
#define PBR_PACRA_PWM3_WP_Msk             (0x2000U)                /*!< PWM3_WP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRA_PWM3_SP_Pos             (14U)                    /*!< PWM3_SP (Bit 14)                                      */
#define PBR_PACRA_PWM3_SP_Msk             (0x4000U)                /*!< PWM3_SP (Bitfield-Mask: 0x01)                         */
/* =========================================================  PACRB  ========================================================= */
#define PBR_PACRB_CRC_TP_Pos              (0U)                     /*!< CRC_TP (Bit 0)                                        */
#define PBR_PACRB_CRC_TP_Msk              (0x1U)                   /*!< CRC_TP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRB_CRC_WP_Pos              (1U)                     /*!< CRC_WP (Bit 1)                                        */
#define PBR_PACRB_CRC_WP_Msk              (0x2U)                   /*!< CRC_WP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRB_CRC_SP_Pos              (2U)                     /*!< CRC_SP (Bit 2)                                        */
#define PBR_PACRB_CRC_SP_Msk              (0x4U)                   /*!< CRC_SP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRB_GPIO_TP_Pos             (4U)                     /*!< GPIO_TP (Bit 4)                                       */
#define PBR_PACRB_GPIO_TP_Msk             (0x10U)                  /*!< GPIO_TP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRB_GPIO_WP_Pos             (5U)                     /*!< GPIO_WP (Bit 5)                                       */
#define PBR_PACRB_GPIO_WP_Msk             (0x20U)                  /*!< GPIO_WP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRB_GPIO_SP_Pos             (6U)                     /*!< GPIO_SP (Bit 6)                                       */
#define PBR_PACRB_GPIO_SP_Msk             (0x40U)                  /*!< GPIO_SP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRB_MPU_TP_Pos              (16U)                    /*!< MPU_TP (Bit 16)                                       */
#define PBR_PACRB_MPU_TP_Msk              (0x10000U)               /*!< MPU_TP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRB_MPU_WP_Pos              (17U)                    /*!< MPU_WP (Bit 17)                                       */
#define PBR_PACRB_MPU_WP_Msk              (0x20000U)               /*!< MPU_WP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRB_MPU_SP_Pos              (18U)                    /*!< MPU_SP (Bit 18)                                       */
#define PBR_PACRB_MPU_SP_Msk              (0x40000U)               /*!< MPU_SP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRB_EIM_TP_Pos              (20U)                    /*!< EIM_TP (Bit 20)                                       */
#define PBR_PACRB_EIM_TP_Msk              (0x100000U)              /*!< EIM_TP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRB_EIM_WP_Pos              (21U)                    /*!< EIM_WP (Bit 21)                                       */
#define PBR_PACRB_EIM_WP_Msk              (0x200000U)              /*!< EIM_WP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRB_EIM_SP_Pos              (22U)                    /*!< EIM_SP (Bit 22)                                       */
#define PBR_PACRB_EIM_SP_Msk              (0x400000U)              /*!< EIM_SP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRB_SMU_TP_Pos              (24U)                    /*!< SMU_TP (Bit 24)                                       */
#define PBR_PACRB_SMU_TP_Msk              (0x1000000U)             /*!< SMU_TP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRB_SMU_WP_Pos              (25U)                    /*!< SMU_WP (Bit 25)                                       */
#define PBR_PACRB_SMU_WP_Msk              (0x2000000U)             /*!< SMU_WP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRB_SMU_SP_Pos              (26U)                    /*!< SMU_SP (Bit 26)                                       */
#define PBR_PACRB_SMU_SP_Msk              (0x4000000U)             /*!< SMU_SP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRB_HSM_TP_Pos              (28U)                    /*!< HSM_TP (Bit 28)                                       */
#define PBR_PACRB_HSM_TP_Msk              (0x10000000U)            /*!< HSM_TP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRB_HSM_WP_Pos              (29U)                    /*!< HSM_WP (Bit 29)                                       */
#define PBR_PACRB_HSM_WP_Msk              (0x20000000U)            /*!< HSM_WP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRB_HSM_SP_Pos              (30U)                    /*!< HSM_SP (Bit 30)                                       */
#define PBR_PACRB_HSM_SP_Msk              (0x40000000U)            /*!< HSM_SP (Bitfield-Mask: 0x01)                          */
/* =========================================================  PACRC  ========================================================= */
#define PBR_PACRC_CLK_TP_Pos              (0U)                     /*!< CLK_TP (Bit 0)                                        */
#define PBR_PACRC_CLK_TP_Msk              (0x1U)                   /*!< CLK_TP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRC_CLK_WP_Pos              (1U)                     /*!< CLK_WP (Bit 1)                                        */
#define PBR_PACRC_CLK_WP_Msk              (0x2U)                   /*!< CLK_WP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRC_CLK_SP_Pos              (2U)                     /*!< CLK_SP (Bit 2)                                        */
#define PBR_PACRC_CLK_SP_Msk              (0x4U)                   /*!< CLK_SP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRC_FLASH_TP_Pos            (8U)                     /*!< FLASH_TP (Bit 8)                                      */
#define PBR_PACRC_FLASH_TP_Msk            (0x100U)                 /*!< FLASH_TP (Bitfield-Mask: 0x01)                        */
#define PBR_PACRC_FLASH_WP_Pos            (9U)                     /*!< FLASH_WP (Bit 9)                                      */
#define PBR_PACRC_FLASH_WP_Msk            (0x200U)                 /*!< FLASH_WP (Bitfield-Mask: 0x01)                        */
#define PBR_PACRC_FLASH_SP_Pos            (10U)                    /*!< FLASH_SP (Bit 10)                                     */
#define PBR_PACRC_FLASH_SP_Msk            (0x400U)                 /*!< FLASH_SP (Bitfield-Mask: 0x01)                        */
#define PBR_PACRC_ADC0_TP_Pos             (12U)                    /*!< ADC0_TP (Bit 12)                                      */
#define PBR_PACRC_ADC0_TP_Msk             (0x1000U)                /*!< ADC0_TP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRC_ADC0_WP_Pos             (13U)                    /*!< ADC0_WP (Bit 13)                                      */
#define PBR_PACRC_ADC0_WP_Msk             (0x2000U)                /*!< ADC0_WP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRC_ADC0_SP_Pos             (14U)                    /*!< ADC0_SP (Bit 14)                                      */
#define PBR_PACRC_ADC0_SP_Msk             (0x4000U)                /*!< ADC0_SP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRC_ACMP_TP_Pos             (20U)                    /*!< ACMP_TP (Bit 20)                                      */
#define PBR_PACRC_ACMP_TP_Msk             (0x100000U)              /*!< ACMP_TP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRC_ACMP_WP_Pos             (21U)                    /*!< ACMP_WP (Bit 21)                                      */
#define PBR_PACRC_ACMP_WP_Msk             (0x200000U)              /*!< ACMP_WP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRC_ACMP_SP_Pos             (22U)                    /*!< ACMP_SP (Bit 22)                                      */
#define PBR_PACRC_ACMP_SP_Msk             (0x400000U)              /*!< ACMP_SP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRC_CTU_TP_Pos              (24U)                    /*!< CTU_TP (Bit 24)                                       */
#define PBR_PACRC_CTU_TP_Msk              (0x1000000U)             /*!< CTU_TP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRC_CTU_WP_Pos              (25U)                    /*!< CTU_WP (Bit 25)                                       */
#define PBR_PACRC_CTU_WP_Msk              (0x2000000U)             /*!< CTU_WP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRC_CTU_SP_Pos              (26U)                    /*!< CTU_SP (Bit 26)                                       */
#define PBR_PACRC_CTU_SP_Msk              (0x4000000U)             /*!< CTU_SP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRC_CAN0_TP_Pos             (28U)                    /*!< CAN0_TP (Bit 28)                                      */
#define PBR_PACRC_CAN0_TP_Msk             (0x10000000U)            /*!< CAN0_TP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRC_CAN0_WP_Pos             (29U)                    /*!< CAN0_WP (Bit 29)                                      */
#define PBR_PACRC_CAN0_WP_Msk             (0x20000000U)            /*!< CAN0_WP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRC_CAN0_SP_Pos             (30U)                    /*!< CAN0_SP (Bit 30)                                      */
#define PBR_PACRC_CAN0_SP_Msk             (0x40000000U)            /*!< CAN0_SP (Bitfield-Mask: 0x01)                         */
/* =========================================================  PACRD  ========================================================= */
#define PBR_PACRD_CAN1_TP_Pos             (0U)                     /*!< CAN1_TP (Bit 0)                                       */
#define PBR_PACRD_CAN1_TP_Msk             (0x1U)                   /*!< CAN1_TP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRD_CAN1_WP_Pos             (1U)                     /*!< CAN1_WP (Bit 1)                                       */
#define PBR_PACRD_CAN1_WP_Msk             (0x2U)                   /*!< CAN1_WP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRD_CAN1_SP_Pos             (2U)                     /*!< CAN1_SP (Bit 2)                                       */
#define PBR_PACRD_CAN1_SP_Msk             (0x4U)                   /*!< CAN1_SP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRD_SPM_TP_Pos              (20U)                    /*!< SPM_TP (Bit 20)                                       */
#define PBR_PACRD_SPM_TP_Msk              (0x100000U)              /*!< SPM_TP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRD_SPM_WP_Pos              (21U)                    /*!< SPM_WP (Bit 21)                                       */
#define PBR_PACRD_SPM_WP_Msk              (0x200000U)              /*!< SPM_WP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRD_SPM_SP_Pos              (22U)                    /*!< SPM_SP (Bit 22)                                       */
#define PBR_PACRD_SPM_SP_Msk              (0x400000U)              /*!< SPM_SP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRD_RTC_TP_Pos              (24U)                    /*!< RTC_TP (Bit 24)                                       */
#define PBR_PACRD_RTC_TP_Msk              (0x1000000U)             /*!< RTC_TP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRD_RTC_WP_Pos              (25U)                    /*!< RTC_WP (Bit 25)                                       */
#define PBR_PACRD_RTC_WP_Msk              (0x2000000U)             /*!< RTC_WP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRD_RTC_SP_Pos              (26U)                    /*!< RTC_SP (Bit 26)                                       */
#define PBR_PACRD_RTC_SP_Msk              (0x4000000U)             /*!< RTC_SP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRD_EIO_TP_Pos              (28U)                    /*!< EIO_TP (Bit 28)                                       */
#define PBR_PACRD_EIO_TP_Msk              (0x10000000U)            /*!< EIO_TP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRD_EIO_WP_Pos              (29U)                    /*!< EIO_WP (Bit 29)                                       */
#define PBR_PACRD_EIO_WP_Msk              (0x20000000U)            /*!< EIO_WP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRD_EIO_SP_Pos              (30U)                    /*!< EIO_SP (Bit 30)                                       */
#define PBR_PACRD_EIO_SP_Msk              (0x40000000U)            /*!< EIO_SP (Bitfield-Mask: 0x01)                          */
/* =========================================================  PACRE  ========================================================= */
#define PBR_PACRE_WDG_TP_Pos              (0U)                     /*!< WDG_TP (Bit 0)                                        */
#define PBR_PACRE_WDG_TP_Msk              (0x1U)                   /*!< WDG_TP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRE_WDG_WP_Pos              (1U)                     /*!< WDG_WP (Bit 1)                                        */
#define PBR_PACRE_WDG_WP_Msk              (0x2U)                   /*!< WDG_WP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRE_WDG_SP_Pos              (2U)                     /*!< WDG_SP (Bit 2)                                        */
#define PBR_PACRE_WDG_SP_Msk              (0x4U)                   /*!< WDG_SP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRE_SPI0_TP_Pos             (8U)                     /*!< SPI0_TP (Bit 8)                                       */
#define PBR_PACRE_SPI0_TP_Msk             (0x100U)                 /*!< SPI0_TP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRE_SPI0_WP_Pos             (9U)                     /*!< SPI0_WP (Bit 9)                                       */
#define PBR_PACRE_SPI0_WP_Msk             (0x200U)                 /*!< SPI0_WP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRE_SPI0_SP_Pos             (10U)                    /*!< SPI0_SP (Bit 10)                                      */
#define PBR_PACRE_SPI0_SP_Msk             (0x400U)                 /*!< SPI0_SP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRE_SPI1_TP_Pos             (12U)                    /*!< SPI1_TP (Bit 12)                                      */
#define PBR_PACRE_SPI1_TP_Msk             (0x1000U)                /*!< SPI1_TP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRE_SPI1_WP_Pos             (13U)                    /*!< SPI1_WP (Bit 13)                                      */
#define PBR_PACRE_SPI1_WP_Msk             (0x2000U)                /*!< SPI1_WP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRE_SPI1_SP_Pos             (14U)                    /*!< SPI1_SP (Bit 14)                                      */
#define PBR_PACRE_SPI1_SP_Msk             (0x4000U)                /*!< SPI1_SP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRE_SPI2_TP_Pos             (16U)                    /*!< SPI2_TP (Bit 16)                                      */
#define PBR_PACRE_SPI2_TP_Msk             (0x10000U)               /*!< SPI2_TP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRE_SPI2_WP_Pos             (17U)                    /*!< SPI2_WP (Bit 17)                                      */
#define PBR_PACRE_SPI2_WP_Msk             (0x20000U)               /*!< SPI2_WP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRE_SPI2_SP_Pos             (18U)                    /*!< SPI2_SP (Bit 18)                                      */
#define PBR_PACRE_SPI2_SP_Msk             (0x40000U)               /*!< SPI2_SP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRE_I2C0_TP_Pos             (24U)                    /*!< I2C0_TP (Bit 24)                                      */
#define PBR_PACRE_I2C0_TP_Msk             (0x1000000U)             /*!< I2C0_TP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRE_I2C0_WP_Pos             (25U)                    /*!< I2C0_WP (Bit 25)                                      */
#define PBR_PACRE_I2C0_WP_Msk             (0x2000000U)             /*!< I2C0_WP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRE_I2C0_SP_Pos             (26U)                    /*!< I2C0_SP (Bit 26)                                      */
#define PBR_PACRE_I2C0_SP_Msk             (0x4000000U)             /*!< I2C0_SP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRE_I2C1_TP_Pos             (28U)                    /*!< I2C1_TP (Bit 28)                                      */
#define PBR_PACRE_I2C1_TP_Msk             (0x10000000U)            /*!< I2C1_TP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRE_I2C1_WP_Pos             (29U)                    /*!< I2C1_WP (Bit 29)                                      */
#define PBR_PACRE_I2C1_WP_Msk             (0x20000000U)            /*!< I2C1_WP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRE_I2C1_SP_Pos             (30U)                    /*!< I2C1_SP (Bit 30)                                      */
#define PBR_PACRE_I2C1_SP_Msk             (0x40000000U)            /*!< I2C1_SP (Bitfield-Mask: 0x01)                         */
/* =========================================================  PACRF  ========================================================= */
#define PBR_PACRF_TIMER_TP_Pos            (0U)                     /*!< TIMER_TP (Bit 0)                                      */
#define PBR_PACRF_TIMER_TP_Msk            (0x1U)                   /*!< TIMER_TP (Bitfield-Mask: 0x01)                        */
#define PBR_PACRF_TIMER_WP_Pos            (1U)                     /*!< TIMER_WP (Bit 1)                                      */
#define PBR_PACRF_TIMER_WP_Msk            (0x2U)                   /*!< TIMER_WP (Bitfield-Mask: 0x01)                        */
#define PBR_PACRF_TIMER_SP_Pos            (2U)                     /*!< TIMER_SP (Bit 2)                                      */
#define PBR_PACRF_TIMER_SP_Msk            (0x4U)                   /*!< TIMER_SP (Bitfield-Mask: 0x01)                        */
#define PBR_PACRF_DMA_TP_Pos              (4U)                     /*!< DMA_TP (Bit 4)                                        */
#define PBR_PACRF_DMA_TP_Msk              (0x10U)                  /*!< DMA_TP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRF_DMA_WP_Pos              (5U)                     /*!< DMA_WP (Bit 5)                                        */
#define PBR_PACRF_DMA_WP_Msk              (0x20U)                  /*!< DMA_WP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRF_DMA_SP_Pos              (6U)                     /*!< DMA_SP (Bit 6)                                        */
#define PBR_PACRF_DMA_SP_Msk              (0x40U)                  /*!< DMA_SP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRF_UART0_TP_Pos            (8U)                     /*!< UART0_TP (Bit 8)                                      */
#define PBR_PACRF_UART0_TP_Msk            (0x100U)                 /*!< UART0_TP (Bitfield-Mask: 0x01)                        */
#define PBR_PACRF_UART0_WP_Pos            (9U)                     /*!< UART0_WP (Bit 9)                                      */
#define PBR_PACRF_UART0_WP_Msk            (0x200U)                 /*!< UART0_WP (Bitfield-Mask: 0x01)                        */
#define PBR_PACRF_UART0_SP_Pos            (10U)                    /*!< UART0_SP (Bit 10)                                     */
#define PBR_PACRF_UART0_SP_Msk            (0x400U)                 /*!< UART0_SP (Bitfield-Mask: 0x01)                        */
#define PBR_PACRF_UART1_TP_Pos            (12U)                    /*!< UART1_TP (Bit 12)                                     */
#define PBR_PACRF_UART1_TP_Msk            (0x1000U)                /*!< UART1_TP (Bitfield-Mask: 0x01)                        */
#define PBR_PACRF_UART1_WP_Pos            (13U)                    /*!< UART1_WP (Bit 13)                                     */
#define PBR_PACRF_UART1_WP_Msk            (0x2000U)                /*!< UART1_WP (Bitfield-Mask: 0x01)                        */
#define PBR_PACRF_UART1_SP_Pos            (14U)                    /*!< UART1_SP (Bit 14)                                     */
#define PBR_PACRF_UART1_SP_Msk            (0x4000U)                /*!< UART1_SP (Bitfield-Mask: 0x01)                        */
#define PBR_PACRF_UART2_TP_Pos            (16U)                    /*!< UART2_TP (Bit 16)                                     */
#define PBR_PACRF_UART2_TP_Msk            (0x10000U)               /*!< UART2_TP (Bitfield-Mask: 0x01)                        */
#define PBR_PACRF_UART2_WP_Pos            (17U)                    /*!< UART2_WP (Bit 17)                                     */
#define PBR_PACRF_UART2_WP_Msk            (0x20000U)               /*!< UART2_WP (Bitfield-Mask: 0x01)                        */
#define PBR_PACRF_UART2_SP_Pos            (18U)                    /*!< UART2_SP (Bit 18)                                     */
#define PBR_PACRF_UART2_SP_Msk            (0x40000U)               /*!< UART2_SP (Bitfield-Mask: 0x01)                        */
/* =========================================================  PACRG  ========================================================= */
#define PBR_PACRG_CMU_TP_Pos              (4U)                     /*!< CMU_TP (Bit 4)                                        */
#define PBR_PACRG_CMU_TP_Msk              (0x10U)                  /*!< CMU_TP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRG_CMU_WP_Pos              (5U)                     /*!< CMU_WP (Bit 5)                                        */
#define PBR_PACRG_CMU_WP_Msk              (0x20U)                  /*!< CMU_WP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRG_CMU_SP_Pos              (6U)                     /*!< CMU_SP (Bit 6)                                        */
#define PBR_PACRG_CMU_SP_Msk              (0x40U)                  /*!< CMU_SP (Bitfield-Mask: 0x01)                          */
#define PBR_PACRG_SENT_TP_Pos             (28U)                    /*!< SENT_TP (Bit 28)                                      */
#define PBR_PACRG_SENT_TP_Msk             (0x10000000U)            /*!< SENT_TP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRG_SENT_WP_Pos             (29U)                    /*!< SENT_WP (Bit 29)                                      */
#define PBR_PACRG_SENT_WP_Msk             (0x20000000U)            /*!< SENT_WP (Bitfield-Mask: 0x01)                         */
#define PBR_PACRG_SENT_SP_Pos             (30U)                    /*!< SENT_SP (Bit 30)                                      */
#define PBR_PACRG_SENT_SP_Msk             (0x40000000U)            /*!< SENT_SP (Bitfield-Mask: 0x01)                         */

/* =========================================================================================================================== */
/* ================                                            SPM                                            ================ */
/* =========================================================================================================================== */

/* =====================================================  PWR_MGR_CFG0  ====================================================== */
#define SPM_PWR_MGR_CFG0_PWR_EN_Pos       (0U)                     /*!< PWR_EN (Bit 0)                                        */
#define SPM_PWR_MGR_CFG0_PWR_EN_Msk       (0x1U)                   /*!< PWR_EN (Bitfield-Mask: 0x01)                          */
#define SPM_PWR_MGR_CFG0_AWIC_EN_Pos      (1U)                     /*!< AWIC_EN (Bit 1)                                       */
#define SPM_PWR_MGR_CFG0_AWIC_EN_Msk      (0x2U)                   /*!< AWIC_EN (Bitfield-Mask: 0x01)                         */
#define SPM_PWR_MGR_CFG0_SLP_TIMOUT_SEL_Pos (4U)                   /*!< SLP_TIMOUT_SEL (Bit 4)                                */
#define SPM_PWR_MGR_CFG0_SLP_TIMOUT_SEL_Msk (0x10U)                /*!< SLP_TIMOUT_SEL (Bitfield-Mask: 0x01)                  */
#define SPM_PWR_MGR_CFG0_IOSUS_EN_Pos     (7U)                     /*!< IOSUS_EN (Bit 7)                                      */
#define SPM_PWR_MGR_CFG0_IOSUS_EN_Msk     (0x80U)                  /*!< IOSUS_EN (Bitfield-Mask: 0x01)                        */
#define SPM_PWR_MGR_CFG0_POWER_MODE_Pos   (8U)                     /*!< POWER_MODE (Bit 8)                                    */
#define SPM_PWR_MGR_CFG0_POWER_MODE_Msk   (0x700U)                 /*!< POWER_MODE (Bitfield-Mask: 0x07)                      */
#define SPM_PWR_MGR_CFG0_CAN0_LPFEN_Pos   (20U)                    /*!< CAN0_LPFEN (Bit 20)                                   */
#define SPM_PWR_MGR_CFG0_CAN0_LPFEN_Msk   (0x100000U)              /*!< CAN0_LPFEN (Bitfield-Mask: 0x01)                      */
#define SPM_PWR_MGR_CFG0_CAN1_LPFEN_Pos   (21U)                    /*!< CAN1_LPFEN (Bit 21)                                   */
#define SPM_PWR_MGR_CFG0_CAN1_LPFEN_Msk   (0x200000U)              /*!< CAN1_LPFEN (Bitfield-Mask: 0x01)                      */
#define SPM_PWR_MGR_CFG0_CAN0_WUEN_Pos    (24U)                    /*!< CAN0_WUEN (Bit 24)                                    */
#define SPM_PWR_MGR_CFG0_CAN0_WUEN_Msk    (0x1000000U)             /*!< CAN0_WUEN (Bitfield-Mask: 0x01)                       */
#define SPM_PWR_MGR_CFG0_CAN1_WUEN_Pos    (25U)                    /*!< CAN1_WUEN (Bit 25)                                    */
#define SPM_PWR_MGR_CFG0_CAN1_WUEN_Msk    (0x2000000U)             /*!< CAN1_WUEN (Bitfield-Mask: 0x01)                       */
/* =====================================================  PWR_MGR_CFG1  ====================================================== */
#define SPM_PWR_MGR_CFG1_LVR_THRESHOLD_Pos (0U)                    /*!< LVR_THRESHOLD (Bit 0)                                 */
#define SPM_PWR_MGR_CFG1_LVR_THRESHOLD_Msk (0x1U)                  /*!< LVR_THRESHOLD (Bitfield-Mask: 0x01)                   */
#define SPM_PWR_MGR_CFG1_LVD_THRESHOLD_Pos (1U)                    /*!< LVD_THRESHOLD (Bit 1)                                 */
#define SPM_PWR_MGR_CFG1_LVD_THRESHOLD_Msk (0x2U)                  /*!< LVD_THRESHOLD (Bitfield-Mask: 0x01)                   */
#define SPM_PWR_MGR_CFG1_VHSI_EN_Pos      (10U)                    /*!< VHSI_EN (Bit 10)                                      */
#define SPM_PWR_MGR_CFG1_VHSI_EN_Msk      (0x400U)                 /*!< VHSI_EN (Bitfield-Mask: 0x01)                         */
#define SPM_PWR_MGR_CFG1_SPLL_RDY_Pos     (23U)                    /*!< SPLL_RDY (Bit 23)                                     */
#define SPM_PWR_MGR_CFG1_SPLL_RDY_Msk     (0x800000U)              /*!< SPLL_RDY (Bitfield-Mask: 0x01)                        */
#define SPM_PWR_MGR_CFG1_VHSI_RDY_Pos     (25U)                    /*!< VHSI_RDY (Bit 25)                                     */
#define SPM_PWR_MGR_CFG1_VHSI_RDY_Msk     (0x2000000U)             /*!< VHSI_RDY (Bitfield-Mask: 0x01)                        */
#define SPM_PWR_MGR_CFG1_XOSC_RDY_Pos     (26U)                    /*!< XOSC_RDY (Bit 26)                                     */
#define SPM_PWR_MGR_CFG1_XOSC_RDY_Msk     (0x4000000U)             /*!< XOSC_RDY (Bitfield-Mask: 0x01)                        */
#define SPM_PWR_MGR_CFG1_SPLL_EN_Pos      (27U)                    /*!< SPLL_EN (Bit 27)                                      */
#define SPM_PWR_MGR_CFG1_SPLL_EN_Msk      (0x8000000U)             /*!< SPLL_EN (Bitfield-Mask: 0x01)                         */
#define SPM_PWR_MGR_CFG1_XOSC_HSEBYP_Pos  (28U)                    /*!< XOSC_HSEBYP (Bit 28)                                  */
#define SPM_PWR_MGR_CFG1_XOSC_HSEBYP_Msk  (0x10000000U)            /*!< XOSC_HSEBYP (Bitfield-Mask: 0x01)                     */
#define SPM_PWR_MGR_CFG1_XOSC_HSEEN_Pos   (29U)                    /*!< XOSC_HSEEN (Bit 29)                                   */
#define SPM_PWR_MGR_CFG1_XOSC_HSEEN_Msk   (0x20000000U)            /*!< XOSC_HSEEN (Bitfield-Mask: 0x01)                      */
/* ================================================  PERIPH_SLEEP_ACK_STATUS  ================================================ */
#define SPM_PERIPH_SLEEP_ACK_STATUS_I2C0_Pos (0U)                  /*!< I2C0 (Bit 0)                                          */
#define SPM_PERIPH_SLEEP_ACK_STATUS_I2C0_Msk (0x1U)                /*!< I2C0 (Bitfield-Mask: 0x01)                            */
#define SPM_PERIPH_SLEEP_ACK_STATUS_I2C1_Pos (1U)                  /*!< I2C1 (Bit 1)                                          */
#define SPM_PERIPH_SLEEP_ACK_STATUS_I2C1_Msk (0x2U)                /*!< I2C1 (Bitfield-Mask: 0x01)                            */
#define SPM_PERIPH_SLEEP_ACK_STATUS_SPI0_Pos (2U)                  /*!< SPI0 (Bit 2)                                          */
#define SPM_PERIPH_SLEEP_ACK_STATUS_SPI0_Msk (0x4U)                /*!< SPI0 (Bitfield-Mask: 0x01)                            */
#define SPM_PERIPH_SLEEP_ACK_STATUS_SPI1_Pos (3U)                  /*!< SPI1 (Bit 3)                                          */
#define SPM_PERIPH_SLEEP_ACK_STATUS_SPI1_Msk (0x8U)                /*!< SPI1 (Bitfield-Mask: 0x01)                            */
#define SPM_PERIPH_SLEEP_ACK_STATUS_SPI2_Pos (4U)                  /*!< SPI2 (Bit 4)                                          */
#define SPM_PERIPH_SLEEP_ACK_STATUS_SPI2_Msk (0x10U)               /*!< SPI2 (Bitfield-Mask: 0x01)                            */
#define SPM_PERIPH_SLEEP_ACK_STATUS_CAN0_Pos (5U)                  /*!< CAN0 (Bit 5)                                          */
#define SPM_PERIPH_SLEEP_ACK_STATUS_CAN0_Msk (0x20U)               /*!< CAN0 (Bitfield-Mask: 0x01)                            */
#define SPM_PERIPH_SLEEP_ACK_STATUS_CAN1_Pos (6U)                  /*!< CAN1 (Bit 6)                                          */
#define SPM_PERIPH_SLEEP_ACK_STATUS_CAN1_Msk (0x40U)               /*!< CAN1 (Bitfield-Mask: 0x01)                            */
#define SPM_PERIPH_SLEEP_ACK_STATUS_UART0_Pos (11U)                /*!< UART0 (Bit 11)                                        */
#define SPM_PERIPH_SLEEP_ACK_STATUS_UART0_Msk (0x800U)             /*!< UART0 (Bitfield-Mask: 0x01)                           */
#define SPM_PERIPH_SLEEP_ACK_STATUS_UART1_Pos (12U)                /*!< UART1 (Bit 12)                                        */
#define SPM_PERIPH_SLEEP_ACK_STATUS_UART1_Msk (0x1000U)            /*!< UART1 (Bitfield-Mask: 0x01)                           */
#define SPM_PERIPH_SLEEP_ACK_STATUS_UART2_Pos (13U)                /*!< UART2 (Bit 13)                                        */
#define SPM_PERIPH_SLEEP_ACK_STATUS_UART2_Msk (0x2000U)            /*!< UART2 (Bitfield-Mask: 0x01)                           */
#define SPM_PERIPH_SLEEP_ACK_STATUS_DMA0_Pos (17U)                 /*!< DMA0 (Bit 17)                                         */
#define SPM_PERIPH_SLEEP_ACK_STATUS_DMA0_Msk (0x20000U)            /*!< DMA0 (Bitfield-Mask: 0x01)                            */
#define SPM_PERIPH_SLEEP_ACK_STATUS_EIO_Pos (18U)                  /*!< EIO (Bit 18)                                          */
#define SPM_PERIPH_SLEEP_ACK_STATUS_EIO_Msk (0x40000U)             /*!< EIO (Bitfield-Mask: 0x01)                             */
#define SPM_PERIPH_SLEEP_ACK_STATUS_FLASH_Pos (19U)                /*!< FLASH (Bit 19)                                        */
#define SPM_PERIPH_SLEEP_ACK_STATUS_FLASH_Msk (0x80000U)           /*!< FLASH (Bitfield-Mask: 0x01)                           */
#define SPM_PERIPH_SLEEP_ACK_STATUS_SENT_Pos (25U)                 /*!< SENT (Bit 25)                                         */
#define SPM_PERIPH_SLEEP_ACK_STATUS_SENT_Msk (0x2000000U)          /*!< SENT (Bitfield-Mask: 0x01)                            */
/* ==================================================  PERIPH_SLEEP_ACK_EN  ================================================== */
#define SPM_PERIPH_SLEEP_ACK_EN_I2C0_Pos  (0U)                     /*!< I2C0 (Bit 0)                                          */
#define SPM_PERIPH_SLEEP_ACK_EN_I2C0_Msk  (0x1U)                   /*!< I2C0 (Bitfield-Mask: 0x01)                            */
#define SPM_PERIPH_SLEEP_ACK_EN_I2C1_Pos  (1U)                     /*!< I2C1 (Bit 1)                                          */
#define SPM_PERIPH_SLEEP_ACK_EN_I2C1_Msk  (0x2U)                   /*!< I2C1 (Bitfield-Mask: 0x01)                            */
#define SPM_PERIPH_SLEEP_ACK_EN_SPI0_Pos  (2U)                     /*!< SPI0 (Bit 2)                                          */
#define SPM_PERIPH_SLEEP_ACK_EN_SPI0_Msk  (0x4U)                   /*!< SPI0 (Bitfield-Mask: 0x01)                            */
#define SPM_PERIPH_SLEEP_ACK_EN_SPI1_Pos  (3U)                     /*!< SPI1 (Bit 3)                                          */
#define SPM_PERIPH_SLEEP_ACK_EN_SPI1_Msk  (0x8U)                   /*!< SPI1 (Bitfield-Mask: 0x01)                            */
#define SPM_PERIPH_SLEEP_ACK_EN_SPI2_Pos  (4U)                     /*!< SPI2 (Bit 4)                                          */
#define SPM_PERIPH_SLEEP_ACK_EN_SPI2_Msk  (0x10U)                  /*!< SPI2 (Bitfield-Mask: 0x01)                            */
#define SPM_PERIPH_SLEEP_ACK_EN_CAN0_Pos  (5U)                     /*!< CAN0 (Bit 5)                                          */
#define SPM_PERIPH_SLEEP_ACK_EN_CAN0_Msk  (0x20U)                  /*!< CAN0 (Bitfield-Mask: 0x01)                            */
#define SPM_PERIPH_SLEEP_ACK_EN_CAN1_Pos  (6U)                     /*!< CAN1 (Bit 6)                                          */
#define SPM_PERIPH_SLEEP_ACK_EN_CAN1_Msk  (0x40U)                  /*!< CAN1 (Bitfield-Mask: 0x01)                            */
#define SPM_PERIPH_SLEEP_ACK_EN_UART0_Pos (11U)                    /*!< UART0 (Bit 11)                                        */
#define SPM_PERIPH_SLEEP_ACK_EN_UART0_Msk (0x800U)                 /*!< UART0 (Bitfield-Mask: 0x01)                           */
#define SPM_PERIPH_SLEEP_ACK_EN_UART1_Pos (12U)                    /*!< UART1 (Bit 12)                                        */
#define SPM_PERIPH_SLEEP_ACK_EN_UART1_Msk (0x1000U)                /*!< UART1 (Bitfield-Mask: 0x01)                           */
#define SPM_PERIPH_SLEEP_ACK_EN_UART2_Pos (13U)                    /*!< UART2 (Bit 13)                                        */
#define SPM_PERIPH_SLEEP_ACK_EN_UART2_Msk (0x2000U)                /*!< UART2 (Bitfield-Mask: 0x01)                           */
#define SPM_PERIPH_SLEEP_ACK_EN_DMA0_Pos  (17U)                    /*!< DMA0 (Bit 17)                                         */
#define SPM_PERIPH_SLEEP_ACK_EN_DMA0_Msk  (0x20000U)               /*!< DMA0 (Bitfield-Mask: 0x01)                            */
#define SPM_PERIPH_SLEEP_ACK_EN_EIO_Pos   (18U)                    /*!< EIO (Bit 18)                                          */
#define SPM_PERIPH_SLEEP_ACK_EN_EIO_Msk   (0x40000U)               /*!< EIO (Bitfield-Mask: 0x01)                             */
#define SPM_PERIPH_SLEEP_ACK_EN_FLASH_Pos (19U)                    /*!< FLASH (Bit 19)                                        */
#define SPM_PERIPH_SLEEP_ACK_EN_FLASH_Msk (0x80000U)               /*!< FLASH (Bitfield-Mask: 0x01)                           */
#define SPM_PERIPH_SLEEP_ACK_EN_SENT_Pos  (25U)                    /*!< SENT (Bit 25)                                         */
#define SPM_PERIPH_SLEEP_ACK_EN_SENT_Msk  (0x2000000U)             /*!< SENT (Bitfield-Mask: 0x01)                            */
/* ========================================================  STATUS  ========================================================= */
#define SPM_STATUS_CURR_POWER_MODE_Pos    (0U)                     /*!< CURR_POWER_MODE (Bit 0)                               */
#define SPM_STATUS_CURR_POWER_MODE_Msk    (0x7U)                   /*!< CURR_POWER_MODE (Bitfield-Mask: 0x07)                 */
#define SPM_STATUS_SLP_ACK_TOF_Pos        (4U)                     /*!< SLP_ACK_TOF (Bit 4)                                   */
#define SPM_STATUS_SLP_ACK_TOF_Msk        (0x10U)                  /*!< SLP_ACK_TOF (Bitfield-Mask: 0x01)                     */
#define SPM_STATUS_SLP_ABORT_Pos          (6U)                     /*!< SLP_ABORT (Bit 6)                                     */
#define SPM_STATUS_SLP_ABORT_Msk          (0x40U)                  /*!< SLP_ABORT (Bitfield-Mask: 0x01)                       */
/* ====================================================  SRAM_POW_CTRL0  ===================================================== */
#define SPM_SRAM_POW_CTRL0_SRAML_LP_EN_Pos (0UL)                    /*!< SRAML_LP_EN (Bit 0)                                   */
#define SPM_SRAM_POW_CTRL0_SRAML_LP_EN_Msk (0x7UL)                  /*!< SRAML_LP_EN (Bitfield-Mask: 0x07)                     */
/* ====================================================  SRAM_POW_CTRL1  ===================================================== */
#define SPM_SRAM_POW_CTRL1_SRAMU_LP_EN_Pos (0UL)                    /*!< SRAMU_LP_EN (Bit 0)                                   */
#define SPM_SRAM_POW_CTRL1_SRAMU_LP_EN_Msk (0x7UL)                  /*!< SRAMU_LP_EN (Bitfield-Mask: 0x07)                     */
/* ======================================================  STB_WP_EN0  ======================================================= */
#define SPM_STB_WP_EN0_PA12_Pos           (0U)                     /*!< PA12 (Bit 0)                                          */
#define SPM_STB_WP_EN0_PA12_Msk           (0x7U)                   /*!< PA12 (Bitfield-Mask: 0x07)                            */
#define SPM_STB_WP_EN0_PB0_Pos            (3U)                     /*!< PB0 (Bit 3)                                           */
#define SPM_STB_WP_EN0_PB0_Msk            (0x38U)                  /*!< PB0 (Bitfield-Mask: 0x07)                             */
#define SPM_STB_WP_EN0_PB1_Pos            (6U)                     /*!< PB1 (Bit 6)                                           */
#define SPM_STB_WP_EN0_PB1_Msk            (0x1c0U)                 /*!< PB1 (Bitfield-Mask: 0x07)                             */
#define SPM_STB_WP_EN0_PB12_Pos           (9U)                     /*!< PB12 (Bit 9)                                          */
#define SPM_STB_WP_EN0_PB12_Msk           (0xe00U)                 /*!< PB12 (Bitfield-Mask: 0x07)                            */
#define SPM_STB_WP_EN0_PD3_Pos            (12U)                    /*!< PD3 (Bit 12)                                          */
#define SPM_STB_WP_EN0_PD3_Msk            (0x7000U)                /*!< PD3 (Bitfield-Mask: 0x07)                             */
#define SPM_STB_WP_EN0_PC2_Pos            (15U)                    /*!< PC2 (Bit 15)                                          */
#define SPM_STB_WP_EN0_PC2_Msk            (0x38000U)               /*!< PC2 (Bitfield-Mask: 0x07)                             */
#define SPM_STB_WP_EN0_PC3_Pos            (18U)                    /*!< PC3 (Bit 18)                                          */
#define SPM_STB_WP_EN0_PC3_Msk            (0x1c0000U)              /*!< PC3 (Bitfield-Mask: 0x07)                             */
#define SPM_STB_WP_EN0_PC6_Pos            (21U)                    /*!< PC6 (Bit 21)                                          */
#define SPM_STB_WP_EN0_PC6_Msk            (0xe00000U)              /*!< PC6 (Bitfield-Mask: 0x07)                             */
#define SPM_STB_WP_EN0_PC7_Pos            (24U)                    /*!< PC7 (Bit 24)                                          */
#define SPM_STB_WP_EN0_PC7_Msk            (0x7000000U)             /*!< PC7 (Bitfield-Mask: 0x07)                             */
#define SPM_STB_WP_EN0_PC16_Pos           (27U)                    /*!< PC16 (Bit 27)                                         */
#define SPM_STB_WP_EN0_PC16_Msk           (0x38000000U)            /*!< PC16 (Bitfield-Mask: 0x07)                            */
/* ======================================================  STB_WP_EN1  ======================================================= */
#define SPM_STB_WP_EN1_PC17_Pos           (0U)                     /*!< PC17 (Bit 0)                                          */
#define SPM_STB_WP_EN1_PC17_Msk           (0x7U)                   /*!< PC17 (Bitfield-Mask: 0x07)                            */
#define SPM_STB_WP_EN1_PD6_Pos            (3U)                     /*!< PD6 (Bit 3)                                           */
#define SPM_STB_WP_EN1_PD6_Msk            (0x38U)                  /*!< PD6 (Bitfield-Mask: 0x07)                             */
#define SPM_STB_WP_EN1_PD7_Pos            (6U)                     /*!< PD7 (Bit 6)                                           */
#define SPM_STB_WP_EN1_PD7_Msk            (0x1c0U)                 /*!< PD7 (Bitfield-Mask: 0x07)                             */
#define SPM_STB_WP_EN1_PE4_Pos            (9U)                     /*!< PE4 (Bit 9)                                           */
#define SPM_STB_WP_EN1_PE4_Msk            (0xe00U)                 /*!< PE4 (Bitfield-Mask: 0x07)                             */
#define SPM_STB_WP_EN1_PE5_Pos            (12U)                    /*!< PE5 (Bit 12)                                          */
#define SPM_STB_WP_EN1_PE5_Msk            (0x7000U)                /*!< PE5 (Bitfield-Mask: 0x07)                             */
#define SPM_STB_WP_EN1_RTC_Pos            (15U)                    /*!< RTC (Bit 15)                                          */
#define SPM_STB_WP_EN1_RTC_Msk            (0x38000U)               /*!< RTC (Bitfield-Mask: 0x07)                             */
#define SPM_STB_WP_EN1_EXTRST_Pos         (18U)                    /*!< EXTRST (Bit 18)                                       */
#define SPM_STB_WP_EN1_EXTRST_Msk         (0x1c0000U)              /*!< EXTRST (Bitfield-Mask: 0x07)                          */
/* =====================================================  STB_WP_STATUS  ===================================================== */
#define SPM_STB_WP_STATUS_PA12_Pos        (0U)                     /*!< PA12 (Bit 0)                                          */
#define SPM_STB_WP_STATUS_PA12_Msk        (0x1U)                   /*!< PA12 (Bitfield-Mask: 0x01)                            */
#define SPM_STB_WP_STATUS_PB0_Pos         (1U)                     /*!< PB0 (Bit 1)                                           */
#define SPM_STB_WP_STATUS_PB0_Msk         (0x2U)                   /*!< PB0 (Bitfield-Mask: 0x01)                             */
#define SPM_STB_WP_STATUS_PB1_Pos         (2U)                     /*!< PB1 (Bit 2)                                           */
#define SPM_STB_WP_STATUS_PB1_Msk         (0x4U)                   /*!< PB1 (Bitfield-Mask: 0x01)                             */
#define SPM_STB_WP_STATUS_PB12_Pos        (3U)                     /*!< PB12 (Bit 3)                                          */
#define SPM_STB_WP_STATUS_PB12_Msk        (0x8U)                   /*!< PB12 (Bitfield-Mask: 0x01)                            */
#define SPM_STB_WP_STATUS_PD3_Pos         (4U)                     /*!< PD3 (Bit 4)                                           */
#define SPM_STB_WP_STATUS_PD3_Msk         (0x10U)                  /*!< PD3 (Bitfield-Mask: 0x01)                             */
#define SPM_STB_WP_STATUS_PC2_Pos         (5U)                     /*!< PC2 (Bit 5)                                           */
#define SPM_STB_WP_STATUS_PC2_Msk         (0x20U)                  /*!< PC2 (Bitfield-Mask: 0x01)                             */
#define SPM_STB_WP_STATUS_PC3_Pos         (6U)                     /*!< PC3 (Bit 6)                                           */
#define SPM_STB_WP_STATUS_PC3_Msk         (0x40U)                  /*!< PC3 (Bitfield-Mask: 0x01)                             */
#define SPM_STB_WP_STATUS_PC6_Pos         (7U)                     /*!< PC6 (Bit 7)                                           */
#define SPM_STB_WP_STATUS_PC6_Msk         (0x80U)                  /*!< PC6 (Bitfield-Mask: 0x01)                             */
#define SPM_STB_WP_STATUS_PC7_Pos         (8U)                     /*!< PC7 (Bit 8)                                           */
#define SPM_STB_WP_STATUS_PC7_Msk         (0x100U)                 /*!< PC7 (Bitfield-Mask: 0x01)                             */
#define SPM_STB_WP_STATUS_PC16_Pos        (9U)                     /*!< PC16 (Bit 9)                                          */
#define SPM_STB_WP_STATUS_PC16_Msk        (0x200U)                 /*!< PC16 (Bitfield-Mask: 0x01)                            */
#define SPM_STB_WP_STATUS_PC17_Pos        (10U)                    /*!< PC17 (Bit 10)                                         */
#define SPM_STB_WP_STATUS_PC17_Msk        (0x400U)                 /*!< PC17 (Bitfield-Mask: 0x01)                            */
#define SPM_STB_WP_STATUS_PD6_Pos         (11U)                    /*!< PD6 (Bit 11)                                          */
#define SPM_STB_WP_STATUS_PD6_Msk         (0x800U)                 /*!< PD6 (Bitfield-Mask: 0x01)                             */
#define SPM_STB_WP_STATUS_PD7_Pos         (12U)                    /*!< PD7 (Bit 12)                                          */
#define SPM_STB_WP_STATUS_PD7_Msk         (0x1000U)                /*!< PD7 (Bitfield-Mask: 0x01)                             */
#define SPM_STB_WP_STATUS_PE4_Pos         (13U)                    /*!< PE4 (Bit 13)                                          */
#define SPM_STB_WP_STATUS_PE4_Msk         (0x2000U)                /*!< PE4 (Bitfield-Mask: 0x01)                             */
#define SPM_STB_WP_STATUS_PE5_Pos         (14U)                    /*!< PE5 (Bit 14)                                          */
#define SPM_STB_WP_STATUS_PE5_Msk         (0x4000U)                /*!< PE5 (Bitfield-Mask: 0x01)                             */
#define SPM_STB_WP_STATUS_RTC_Pos         (15U)                    /*!< RTC (Bit 15)                                          */
#define SPM_STB_WP_STATUS_RTC_Msk         (0x8000U)                /*!< RTC (Bitfield-Mask: 0x01)                             */
#define SPM_STB_WP_STATUS_EXTRST_Pos      (16U)                    /*!< EXTRST (Bit 16)                                       */
#define SPM_STB_WP_STATUS_EXTRST_Msk      (0x10000U)               /*!< EXTRST (Bitfield-Mask: 0x01)                          */
#define SPM_STB_WP_STATUS_STB_WP_FLG_Pos  (31U)                    /*!< STB_WP_FLG (Bit 31)                                   */
#define SPM_STB_WP_STATUS_STB_WP_FLG_Msk  (0x80000000U)            /*!< STB_WP_FLG (Bitfield-Mask: 0x01)                      */

/* =========================================================================================================================== */
/* ================                                           GPIO                                            ================ */
/* =========================================================================================================================== */

/* =========================================================  PODR  ========================================================== */
#define GPIO_PODR_PODR0_Pos               (0U)                     /*!< PODR0 (Bit 0)                                         */
#define GPIO_PODR_PODR0_Msk               (0x1U)                   /*!< PODR0 (Bitfield-Mask: 0x01)                           */
#define GPIO_PODR_PODR1_Pos               (1U)                     /*!< PODR1 (Bit 1)                                         */
#define GPIO_PODR_PODR1_Msk               (0x2U)                   /*!< PODR1 (Bitfield-Mask: 0x01)                           */
#define GPIO_PODR_PODR2_Pos               (2U)                     /*!< PODR2 (Bit 2)                                         */
#define GPIO_PODR_PODR2_Msk               (0x4U)                   /*!< PODR2 (Bitfield-Mask: 0x01)                           */
#define GPIO_PODR_PODR3_Pos               (3U)                     /*!< PODR3 (Bit 3)                                         */
#define GPIO_PODR_PODR3_Msk               (0x8U)                   /*!< PODR3 (Bitfield-Mask: 0x01)                           */
#define GPIO_PODR_PODR4_Pos               (4U)                     /*!< PODR4 (Bit 4)                                         */
#define GPIO_PODR_PODR4_Msk               (0x10U)                  /*!< PODR4 (Bitfield-Mask: 0x01)                           */
#define GPIO_PODR_PODR5_Pos               (5U)                     /*!< PODR5 (Bit 5)                                         */
#define GPIO_PODR_PODR5_Msk               (0x20U)                  /*!< PODR5 (Bitfield-Mask: 0x01)                           */
#define GPIO_PODR_PODR6_Pos               (6U)                     /*!< PODR6 (Bit 6)                                         */
#define GPIO_PODR_PODR6_Msk               (0x40U)                  /*!< PODR6 (Bitfield-Mask: 0x01)                           */
#define GPIO_PODR_PODR7_Pos               (7U)                     /*!< PODR7 (Bit 7)                                         */
#define GPIO_PODR_PODR7_Msk               (0x80U)                  /*!< PODR7 (Bitfield-Mask: 0x01)                           */
#define GPIO_PODR_PODR8_Pos               (8U)                     /*!< PODR8 (Bit 8)                                         */
#define GPIO_PODR_PODR8_Msk               (0x100U)                 /*!< PODR8 (Bitfield-Mask: 0x01)                           */
#define GPIO_PODR_PODR9_Pos               (9U)                     /*!< PODR9 (Bit 9)                                         */
#define GPIO_PODR_PODR9_Msk               (0x200U)                 /*!< PODR9 (Bitfield-Mask: 0x01)                           */
#define GPIO_PODR_PODR10_Pos              (10U)                    /*!< PODR10 (Bit 10)                                       */
#define GPIO_PODR_PODR10_Msk              (0x400U)                 /*!< PODR10 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR11_Pos              (11U)                    /*!< PODR11 (Bit 11)                                       */
#define GPIO_PODR_PODR11_Msk              (0x800U)                 /*!< PODR11 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR12_Pos              (12U)                    /*!< PODR12 (Bit 12)                                       */
#define GPIO_PODR_PODR12_Msk              (0x1000U)                /*!< PODR12 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR13_Pos              (13U)                    /*!< PODR13 (Bit 13)                                       */
#define GPIO_PODR_PODR13_Msk              (0x2000U)                /*!< PODR13 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR14_Pos              (14U)                    /*!< PODR14 (Bit 14)                                       */
#define GPIO_PODR_PODR14_Msk              (0x4000U)                /*!< PODR14 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR15_Pos              (15U)                    /*!< PODR15 (Bit 15)                                       */
#define GPIO_PODR_PODR15_Msk              (0x8000U)                /*!< PODR15 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR16_Pos              (16U)                    /*!< PODR16 (Bit 16)                                       */
#define GPIO_PODR_PODR16_Msk              (0x10000U)               /*!< PODR16 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR17_Pos              (17U)                    /*!< PODR17 (Bit 17)                                       */
#define GPIO_PODR_PODR17_Msk              (0x20000U)               /*!< PODR17 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR18_Pos              (18U)                    /*!< PODR18 (Bit 18)                                       */
#define GPIO_PODR_PODR18_Msk              (0x40000U)               /*!< PODR18 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR19_Pos              (19U)                    /*!< PODR19 (Bit 19)                                       */
#define GPIO_PODR_PODR19_Msk              (0x80000U)               /*!< PODR19 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR20_Pos              (20U)                    /*!< PODR20 (Bit 20)                                       */
#define GPIO_PODR_PODR20_Msk              (0x100000U)              /*!< PODR20 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR21_Pos              (21U)                    /*!< PODR21 (Bit 21)                                       */
#define GPIO_PODR_PODR21_Msk              (0x200000U)              /*!< PODR21 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR22_Pos              (22U)                    /*!< PODR22 (Bit 22)                                       */
#define GPIO_PODR_PODR22_Msk              (0x400000U)              /*!< PODR22 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR23_Pos              (23U)                    /*!< PODR23 (Bit 23)                                       */
#define GPIO_PODR_PODR23_Msk              (0x800000U)              /*!< PODR23 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR24_Pos              (24U)                    /*!< PODR24 (Bit 24)                                       */
#define GPIO_PODR_PODR24_Msk              (0x1000000U)             /*!< PODR24 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR25_Pos              (25U)                    /*!< PODR25 (Bit 25)                                       */
#define GPIO_PODR_PODR25_Msk              (0x2000000U)             /*!< PODR25 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR26_Pos              (26U)                    /*!< PODR26 (Bit 26)                                       */
#define GPIO_PODR_PODR26_Msk              (0x4000000U)             /*!< PODR26 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR27_Pos              (27U)                    /*!< PODR27 (Bit 27)                                       */
#define GPIO_PODR_PODR27_Msk              (0x8000000U)             /*!< PODR27 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR28_Pos              (28U)                    /*!< PODR28 (Bit 28)                                       */
#define GPIO_PODR_PODR28_Msk              (0x10000000U)            /*!< PODR28 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR29_Pos              (29U)                    /*!< PODR29 (Bit 29)                                       */
#define GPIO_PODR_PODR29_Msk              (0x20000000U)            /*!< PODR29 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR30_Pos              (30U)                    /*!< PODR30 (Bit 30)                                       */
#define GPIO_PODR_PODR30_Msk              (0x40000000U)            /*!< PODR30 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_PODR31_Pos              (31U)                    /*!< PODR31 (Bit 31)                                       */
#define GPIO_PODR_PODR31_Msk              (0x80000000U)            /*!< PODR31 (Bitfield-Mask: 0x01)                          */
#define GPIO_PODR_Pos(x)                  ((uint32)x)             /*!< PODRx  (Bit x)                                        */
#define GPIO_PODR_Msk(x)                  (0x01UL<<GPIO_PODR_Pos(x))  /*!< PODRx  (Bitfield-Mask: 0x01)                        */
/* =========================================================  PSOR  ========================================================== */
#define GPIO_PSOR_PSOR0_Pos               (0U)                     /*!< PSOR0 (Bit 0)                                         */
#define GPIO_PSOR_PSOR0_Msk               (0x1U)                   /*!< PSOR0 (Bitfield-Mask: 0x01)                           */
#define GPIO_PSOR_PSOR1_Pos               (1U)                     /*!< PSOR1 (Bit 1)                                         */
#define GPIO_PSOR_PSOR1_Msk               (0x2U)                   /*!< PSOR1 (Bitfield-Mask: 0x01)                           */
#define GPIO_PSOR_PSOR2_Pos               (2U)                     /*!< PSOR2 (Bit 2)                                         */
#define GPIO_PSOR_PSOR2_Msk               (0x4U)                   /*!< PSOR2 (Bitfield-Mask: 0x01)                           */
#define GPIO_PSOR_PSOR3_Pos               (3U)                     /*!< PSOR3 (Bit 3)                                         */
#define GPIO_PSOR_PSOR3_Msk               (0x8U)                   /*!< PSOR3 (Bitfield-Mask: 0x01)                           */
#define GPIO_PSOR_PSOR4_Pos               (4U)                     /*!< PSOR4 (Bit 4)                                         */
#define GPIO_PSOR_PSOR4_Msk               (0x10U)                  /*!< PSOR4 (Bitfield-Mask: 0x01)                           */
#define GPIO_PSOR_PSOR5_Pos               (5U)                     /*!< PSOR5 (Bit 5)                                         */
#define GPIO_PSOR_PSOR5_Msk               (0x20U)                  /*!< PSOR5 (Bitfield-Mask: 0x01)                           */
#define GPIO_PSOR_PSOR6_Pos               (6U)                     /*!< PSOR6 (Bit 6)                                         */
#define GPIO_PSOR_PSOR6_Msk               (0x40U)                  /*!< PSOR6 (Bitfield-Mask: 0x01)                           */
#define GPIO_PSOR_PSOR7_Pos               (7U)                     /*!< PSOR7 (Bit 7)                                         */
#define GPIO_PSOR_PSOR7_Msk               (0x80U)                  /*!< PSOR7 (Bitfield-Mask: 0x01)                           */
#define GPIO_PSOR_PSOR8_Pos               (8U)                     /*!< PSOR8 (Bit 8)                                         */
#define GPIO_PSOR_PSOR8_Msk               (0x100U)                 /*!< PSOR8 (Bitfield-Mask: 0x01)                           */
#define GPIO_PSOR_PSOR9_Pos               (9U)                     /*!< PSOR9 (Bit 9)                                         */
#define GPIO_PSOR_PSOR9_Msk               (0x200U)                 /*!< PSOR9 (Bitfield-Mask: 0x01)                           */
#define GPIO_PSOR_PSOR10_Pos              (10U)                    /*!< PSOR10 (Bit 10)                                       */
#define GPIO_PSOR_PSOR10_Msk              (0x400U)                 /*!< PSOR10 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR11_Pos              (11U)                    /*!< PSOR11 (Bit 11)                                       */
#define GPIO_PSOR_PSOR11_Msk              (0x800U)                 /*!< PSOR11 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR12_Pos              (12U)                    /*!< PSOR12 (Bit 12)                                       */
#define GPIO_PSOR_PSOR12_Msk              (0x1000U)                /*!< PSOR12 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR13_Pos              (13U)                    /*!< PSOR13 (Bit 13)                                       */
#define GPIO_PSOR_PSOR13_Msk              (0x2000U)                /*!< PSOR13 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR14_Pos              (14U)                    /*!< PSOR14 (Bit 14)                                       */
#define GPIO_PSOR_PSOR14_Msk              (0x4000U)                /*!< PSOR14 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR15_Pos              (15U)                    /*!< PSOR15 (Bit 15)                                       */
#define GPIO_PSOR_PSOR15_Msk              (0x8000U)                /*!< PSOR15 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR16_Pos              (16U)                    /*!< PSOR16 (Bit 16)                                       */
#define GPIO_PSOR_PSOR16_Msk              (0x10000U)               /*!< PSOR16 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR17_Pos              (17U)                    /*!< PSOR17 (Bit 17)                                       */
#define GPIO_PSOR_PSOR17_Msk              (0x20000U)               /*!< PSOR17 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR18_Pos              (18U)                    /*!< PSOR18 (Bit 18)                                       */
#define GPIO_PSOR_PSOR18_Msk              (0x40000U)               /*!< PSOR18 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR19_Pos              (19U)                    /*!< PSOR19 (Bit 19)                                       */
#define GPIO_PSOR_PSOR19_Msk              (0x80000U)               /*!< PSOR19 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR20_Pos              (20U)                    /*!< PSOR20 (Bit 20)                                       */
#define GPIO_PSOR_PSOR20_Msk              (0x100000U)              /*!< PSOR20 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR21_Pos              (21U)                    /*!< PSOR21 (Bit 21)                                       */
#define GPIO_PSOR_PSOR21_Msk              (0x200000U)              /*!< PSOR21 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR22_Pos              (22U)                    /*!< PSOR22 (Bit 22)                                       */
#define GPIO_PSOR_PSOR22_Msk              (0x400000U)              /*!< PSOR22 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR23_Pos              (23U)                    /*!< PSOR23 (Bit 23)                                       */
#define GPIO_PSOR_PSOR23_Msk              (0x800000U)              /*!< PSOR23 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR24_Pos              (24U)                    /*!< PSOR24 (Bit 24)                                       */
#define GPIO_PSOR_PSOR24_Msk              (0x1000000U)             /*!< PSOR24 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR25_Pos              (25U)                    /*!< PSOR25 (Bit 25)                                       */
#define GPIO_PSOR_PSOR25_Msk              (0x2000000U)             /*!< PSOR25 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR26_Pos              (26U)                    /*!< PSOR26 (Bit 26)                                       */
#define GPIO_PSOR_PSOR26_Msk              (0x4000000U)             /*!< PSOR26 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR27_Pos              (27U)                    /*!< PSOR27 (Bit 27)                                       */
#define GPIO_PSOR_PSOR27_Msk              (0x8000000U)             /*!< PSOR27 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR28_Pos              (28U)                    /*!< PSOR28 (Bit 28)                                       */
#define GPIO_PSOR_PSOR28_Msk              (0x10000000U)            /*!< PSOR28 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR29_Pos              (29U)                    /*!< PSOR29 (Bit 29)                                       */
#define GPIO_PSOR_PSOR29_Msk              (0x20000000U)            /*!< PSOR29 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PSOR30_Pos              (30U)                    /*!< PSOR30 (Bit 30)                                       */
#define GPIO_PSOR_PSOR30_Msk              (0x40000000U)            /*!< PSOR30 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_PODR31_Pos              (31U)                    /*!< PODR31 (Bit 31)                                       */
#define GPIO_PSOR_PODR31_Msk              (0x80000000U)            /*!< PODR31 (Bitfield-Mask: 0x01)                          */
#define GPIO_PSOR_Pos(x)                  ((uint32)x)             /*!< PSORx  (Bit x)                                        */
#define GPIO_PSOR_Msk(x)                  (0x01UL<<GPIO_PSOR_Pos(x))  /*!< PSORx  (Bitfield-Mask: 0x01)                        */
/* =========================================================  PROR  ========================================================== */
#define GPIO_PROR_PROR0_Pos               (0U)                     /*!< PROR0 (Bit 0)                                         */
#define GPIO_PROR_PROR0_Msk               (0x1U)                   /*!< PROR0 (Bitfield-Mask: 0x01)                           */
#define GPIO_PROR_PROR1_Pos               (1U)                     /*!< PROR1 (Bit 1)                                         */
#define GPIO_PROR_PROR1_Msk               (0x2U)                   /*!< PROR1 (Bitfield-Mask: 0x01)                           */
#define GPIO_PROR_PROR2_Pos               (2U)                     /*!< PROR2 (Bit 2)                                         */
#define GPIO_PROR_PROR2_Msk               (0x4U)                   /*!< PROR2 (Bitfield-Mask: 0x01)                           */
#define GPIO_PROR_PROR3_Pos               (3U)                     /*!< PROR3 (Bit 3)                                         */
#define GPIO_PROR_PROR3_Msk               (0x8U)                   /*!< PROR3 (Bitfield-Mask: 0x01)                           */
#define GPIO_PROR_PROR4_Pos               (4U)                     /*!< PROR4 (Bit 4)                                         */
#define GPIO_PROR_PROR4_Msk               (0x10U)                  /*!< PROR4 (Bitfield-Mask: 0x01)                           */
#define GPIO_PROR_PROR5_Pos               (5U)                     /*!< PROR5 (Bit 5)                                         */
#define GPIO_PROR_PROR5_Msk               (0x20U)                  /*!< PROR5 (Bitfield-Mask: 0x01)                           */
#define GPIO_PROR_PROR6_Pos               (6U)                     /*!< PROR6 (Bit 6)                                         */
#define GPIO_PROR_PROR6_Msk               (0x40U)                  /*!< PROR6 (Bitfield-Mask: 0x01)                           */
#define GPIO_PROR_PROR7_Pos               (7U)                     /*!< PROR7 (Bit 7)                                         */
#define GPIO_PROR_PROR7_Msk               (0x80U)                  /*!< PROR7 (Bitfield-Mask: 0x01)                           */
#define GPIO_PROR_PROR8_Pos               (8U)                     /*!< PROR8 (Bit 8)                                         */
#define GPIO_PROR_PROR8_Msk               (0x100U)                 /*!< PROR8 (Bitfield-Mask: 0x01)                           */
#define GPIO_PROR_PROR9_Pos               (9U)                     /*!< PROR9 (Bit 9)                                         */
#define GPIO_PROR_PROR9_Msk               (0x200U)                 /*!< PROR9 (Bitfield-Mask: 0x01)                           */
#define GPIO_PROR_PROR10_Pos              (10U)                    /*!< PROR10 (Bit 10)                                       */
#define GPIO_PROR_PROR10_Msk              (0x400U)                 /*!< PROR10 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR11_Pos              (11U)                    /*!< PROR11 (Bit 11)                                       */
#define GPIO_PROR_PROR11_Msk              (0x800U)                 /*!< PROR11 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR12_Pos              (12U)                    /*!< PROR12 (Bit 12)                                       */
#define GPIO_PROR_PROR12_Msk              (0x1000U)                /*!< PROR12 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR13_Pos              (13U)                    /*!< PROR13 (Bit 13)                                       */
#define GPIO_PROR_PROR13_Msk              (0x2000U)                /*!< PROR13 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR14_Pos              (14U)                    /*!< PROR14 (Bit 14)                                       */
#define GPIO_PROR_PROR14_Msk              (0x4000U)                /*!< PROR14 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR15_Pos              (15U)                    /*!< PROR15 (Bit 15)                                       */
#define GPIO_PROR_PROR15_Msk              (0x8000U)                /*!< PROR15 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR16_Pos              (16U)                    /*!< PROR16 (Bit 16)                                       */
#define GPIO_PROR_PROR16_Msk              (0x10000U)               /*!< PROR16 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR17_Pos              (17U)                    /*!< PROR17 (Bit 17)                                       */
#define GPIO_PROR_PROR17_Msk              (0x20000U)               /*!< PROR17 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR18_Pos              (18U)                    /*!< PROR18 (Bit 18)                                       */
#define GPIO_PROR_PROR18_Msk              (0x40000U)               /*!< PROR18 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR19_Pos              (19U)                    /*!< PROR19 (Bit 19)                                       */
#define GPIO_PROR_PROR19_Msk              (0x80000U)               /*!< PROR19 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR20_Pos              (20U)                    /*!< PROR20 (Bit 20)                                       */
#define GPIO_PROR_PROR20_Msk              (0x100000U)              /*!< PROR20 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR21_Pos              (21U)                    /*!< PROR21 (Bit 21)                                       */
#define GPIO_PROR_PROR21_Msk              (0x200000U)              /*!< PROR21 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR22_Pos              (22U)                    /*!< PROR22 (Bit 22)                                       */
#define GPIO_PROR_PROR22_Msk              (0x400000U)              /*!< PROR22 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR23_Pos              (23U)                    /*!< PROR23 (Bit 23)                                       */
#define GPIO_PROR_PROR23_Msk              (0x800000U)              /*!< PROR23 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR24_Pos              (24U)                    /*!< PROR24 (Bit 24)                                       */
#define GPIO_PROR_PROR24_Msk              (0x1000000U)             /*!< PROR24 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR25_Pos              (25U)                    /*!< PROR25 (Bit 25)                                       */
#define GPIO_PROR_PROR25_Msk              (0x2000000U)             /*!< PROR25 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR26_Pos              (26U)                    /*!< PROR26 (Bit 26)                                       */
#define GPIO_PROR_PROR26_Msk              (0x4000000U)             /*!< PROR26 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR27_Pos              (27U)                    /*!< PROR27 (Bit 27)                                       */
#define GPIO_PROR_PROR27_Msk              (0x8000000U)             /*!< PROR27 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR28_Pos              (28U)                    /*!< PROR28 (Bit 28)                                       */
#define GPIO_PROR_PROR28_Msk              (0x10000000U)            /*!< PROR28 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR29_Pos              (29U)                    /*!< PROR29 (Bit 29)                                       */
#define GPIO_PROR_PROR29_Msk              (0x20000000U)            /*!< PROR29 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PROR30_Pos              (30U)                    /*!< PROR30 (Bit 30)                                       */
#define GPIO_PROR_PROR30_Msk              (0x40000000U)            /*!< PROR30 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_PODR31_Pos              (31U)                    /*!< PODR31 (Bit 31)                                       */
#define GPIO_PROR_PODR31_Msk              (0x80000000U)            /*!< PODR31 (Bitfield-Mask: 0x01)                          */
#define GPIO_PROR_Pos(x)                  ((uint32)x)             /*!< PRORx  (Bit x)                                        */
#define GPIO_PROR_Msk(x)                  (0x01UL<<GPIO_PROR_Pos(x))  /*!< PRORx  (Bitfield-Mask: 0x01)                        */
/* =========================================================  PIOR  ========================================================== */
#define GPIO_PIOR_PIOR0_Pos               (0U)                     /*!< PIOR0 (Bit 0)                                         */
#define GPIO_PIOR_PIOR0_Msk               (0x1U)                   /*!< PIOR0 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIOR_PIOR1_Pos               (1U)                     /*!< PIOR1 (Bit 1)                                         */
#define GPIO_PIOR_PIOR1_Msk               (0x2U)                   /*!< PIOR1 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIOR_PIOR2_Pos               (2U)                     /*!< PIOR2 (Bit 2)                                         */
#define GPIO_PIOR_PIOR2_Msk               (0x4U)                   /*!< PIOR2 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIOR_PIOR3_Pos               (3U)                     /*!< PIOR3 (Bit 3)                                         */
#define GPIO_PIOR_PIOR3_Msk               (0x8U)                   /*!< PIOR3 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIOR_PIOR4_Pos               (4U)                     /*!< PIOR4 (Bit 4)                                         */
#define GPIO_PIOR_PIOR4_Msk               (0x10U)                  /*!< PIOR4 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIOR_PIOR5_Pos               (5U)                     /*!< PIOR5 (Bit 5)                                         */
#define GPIO_PIOR_PIOR5_Msk               (0x20U)                  /*!< PIOR5 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIOR_PIOR6_Pos               (6U)                     /*!< PIOR6 (Bit 6)                                         */
#define GPIO_PIOR_PIOR6_Msk               (0x40U)                  /*!< PIOR6 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIOR_PIOR7_Pos               (7U)                     /*!< PIOR7 (Bit 7)                                         */
#define GPIO_PIOR_PIOR7_Msk               (0x80U)                  /*!< PIOR7 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIOR_PIOR8_Pos               (8U)                     /*!< PIOR8 (Bit 8)                                         */
#define GPIO_PIOR_PIOR8_Msk               (0x100U)                 /*!< PIOR8 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIOR_PIOR9_Pos               (9U)                     /*!< PIOR9 (Bit 9)                                         */
#define GPIO_PIOR_PIOR9_Msk               (0x200U)                 /*!< PIOR9 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIOR_PIOR10_Pos              (10U)                    /*!< PIOR10 (Bit 10)                                       */
#define GPIO_PIOR_PIOR10_Msk              (0x400U)                 /*!< PIOR10 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR11_Pos              (11U)                    /*!< PIOR11 (Bit 11)                                       */
#define GPIO_PIOR_PIOR11_Msk              (0x800U)                 /*!< PIOR11 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR12_Pos              (12U)                    /*!< PIOR12 (Bit 12)                                       */
#define GPIO_PIOR_PIOR12_Msk              (0x1000U)                /*!< PIOR12 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR13_Pos              (13U)                    /*!< PIOR13 (Bit 13)                                       */
#define GPIO_PIOR_PIOR13_Msk              (0x2000U)                /*!< PIOR13 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR14_Pos              (14U)                    /*!< PIOR14 (Bit 14)                                       */
#define GPIO_PIOR_PIOR14_Msk              (0x4000U)                /*!< PIOR14 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR15_Pos              (15U)                    /*!< PIOR15 (Bit 15)                                       */
#define GPIO_PIOR_PIOR15_Msk              (0x8000U)                /*!< PIOR15 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR16_Pos              (16U)                    /*!< PIOR16 (Bit 16)                                       */
#define GPIO_PIOR_PIOR16_Msk              (0x10000U)               /*!< PIOR16 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR17_Pos              (17U)                    /*!< PIOR17 (Bit 17)                                       */
#define GPIO_PIOR_PIOR17_Msk              (0x20000U)               /*!< PIOR17 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR18_Pos              (18U)                    /*!< PIOR18 (Bit 18)                                       */
#define GPIO_PIOR_PIOR18_Msk              (0x40000U)               /*!< PIOR18 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR19_Pos              (19U)                    /*!< PIOR19 (Bit 19)                                       */
#define GPIO_PIOR_PIOR19_Msk              (0x80000U)               /*!< PIOR19 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR20_Pos              (20U)                    /*!< PIOR20 (Bit 20)                                       */
#define GPIO_PIOR_PIOR20_Msk              (0x100000U)              /*!< PIOR20 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR21_Pos              (21U)                    /*!< PIOR21 (Bit 21)                                       */
#define GPIO_PIOR_PIOR21_Msk              (0x200000U)              /*!< PIOR21 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR22_Pos              (22U)                    /*!< PIOR22 (Bit 22)                                       */
#define GPIO_PIOR_PIOR22_Msk              (0x400000U)              /*!< PIOR22 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR23_Pos              (23U)                    /*!< PIOR23 (Bit 23)                                       */
#define GPIO_PIOR_PIOR23_Msk              (0x800000U)              /*!< PIOR23 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR24_Pos              (24U)                    /*!< PIOR24 (Bit 24)                                       */
#define GPIO_PIOR_PIOR24_Msk              (0x1000000U)             /*!< PIOR24 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR25_Pos              (25U)                    /*!< PIOR25 (Bit 25)                                       */
#define GPIO_PIOR_PIOR25_Msk              (0x2000000U)             /*!< PIOR25 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR26_Pos              (26U)                    /*!< PIOR26 (Bit 26)                                       */
#define GPIO_PIOR_PIOR26_Msk              (0x4000000U)             /*!< PIOR26 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR27_Pos              (27U)                    /*!< PIOR27 (Bit 27)                                       */
#define GPIO_PIOR_PIOR27_Msk              (0x8000000U)             /*!< PIOR27 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR28_Pos              (28U)                    /*!< PIOR28 (Bit 28)                                       */
#define GPIO_PIOR_PIOR28_Msk              (0x10000000U)            /*!< PIOR28 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR29_Pos              (29U)                    /*!< PIOR29 (Bit 29)                                       */
#define GPIO_PIOR_PIOR29_Msk              (0x20000000U)            /*!< PIOR29 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR30_Pos              (30U)                    /*!< PIOR30 (Bit 30)                                       */
#define GPIO_PIOR_PIOR30_Msk              (0x40000000U)            /*!< PIOR30 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_PIOR31_Pos              (31U)                    /*!< PIOR31 (Bit 31)                                       */
#define GPIO_PIOR_PIOR31_Msk              (0x80000000U)            /*!< PIOR31 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIOR_Pos(x)                  ((uint32)x)             /*!< PRORx  (Bit x)                                        */
#define GPIO_PIOR_Msk(x)                  (0x01UL<<GPIO_PIOR_Pos(x))  /*!< PRORx  (Bitfield-Mask: 0x01)                        */
/* =========================================================  PIDR  ========================================================== */
#define GPIO_PIDR_PIDR0_Pos               (0U)                     /*!< PIDR0 (Bit 0)                                         */
#define GPIO_PIDR_PIDR0_Msk               (0x1U)                   /*!< PIDR0 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIDR_PIDR1_Pos               (1U)                     /*!< PIDR1 (Bit 1)                                         */
#define GPIO_PIDR_PIDR1_Msk               (0x2U)                   /*!< PIDR1 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIDR_PIDR2_Pos               (2U)                     /*!< PIDR2 (Bit 2)                                         */
#define GPIO_PIDR_PIDR2_Msk               (0x4U)                   /*!< PIDR2 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIDR_PIDR3_Pos               (3U)                     /*!< PIDR3 (Bit 3)                                         */
#define GPIO_PIDR_PIDR3_Msk               (0x8U)                   /*!< PIDR3 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIDR_PIDR4_Pos               (4U)                     /*!< PIDR4 (Bit 4)                                         */
#define GPIO_PIDR_PIDR4_Msk               (0x10U)                  /*!< PIDR4 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIDR_PIDR5_Pos               (5U)                     /*!< PIDR5 (Bit 5)                                         */
#define GPIO_PIDR_PIDR5_Msk               (0x20U)                  /*!< PIDR5 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIDR_PIDR6_Pos               (6U)                     /*!< PIDR6 (Bit 6)                                         */
#define GPIO_PIDR_PIDR6_Msk               (0x40U)                  /*!< PIDR6 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIDR_PIDR7_Pos               (7U)                     /*!< PIDR7 (Bit 7)                                         */
#define GPIO_PIDR_PIDR7_Msk               (0x80U)                  /*!< PIDR7 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIDR_PIDR8_Pos               (8U)                     /*!< PIDR8 (Bit 8)                                         */
#define GPIO_PIDR_PIDR8_Msk               (0x100U)                 /*!< PIDR8 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIDR_PIDR9_Pos               (9U)                     /*!< PIDR9 (Bit 9)                                         */
#define GPIO_PIDR_PIDR9_Msk               (0x200U)                 /*!< PIDR9 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIDR_PIDR10_Pos              (10U)                    /*!< PIDR10 (Bit 10)                                       */
#define GPIO_PIDR_PIDR10_Msk              (0x400U)                 /*!< PIDR10 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR11_Pos              (11U)                    /*!< PIDR11 (Bit 11)                                       */
#define GPIO_PIDR_PIDR11_Msk              (0x800U)                 /*!< PIDR11 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR12_Pos              (12U)                    /*!< PIDR12 (Bit 12)                                       */
#define GPIO_PIDR_PIDR12_Msk              (0x1000U)                /*!< PIDR12 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR13_Pos              (13U)                    /*!< PIDR13 (Bit 13)                                       */
#define GPIO_PIDR_PIDR13_Msk              (0x2000U)                /*!< PIDR13 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR14_Pos              (14U)                    /*!< PIDR14 (Bit 14)                                       */
#define GPIO_PIDR_PIDR14_Msk              (0x4000U)                /*!< PIDR14 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR15_Pos              (15U)                    /*!< PIDR15 (Bit 15)                                       */
#define GPIO_PIDR_PIDR15_Msk              (0x8000U)                /*!< PIDR15 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR16_Pos              (16U)                    /*!< PIDR16 (Bit 16)                                       */
#define GPIO_PIDR_PIDR16_Msk              (0x10000U)               /*!< PIDR16 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR17_Pos              (17U)                    /*!< PIDR17 (Bit 17)                                       */
#define GPIO_PIDR_PIDR17_Msk              (0x20000U)               /*!< PIDR17 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR18_Pos              (18U)                    /*!< PIDR18 (Bit 18)                                       */
#define GPIO_PIDR_PIDR18_Msk              (0x40000U)               /*!< PIDR18 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR19_Pos              (19U)                    /*!< PIDR19 (Bit 19)                                       */
#define GPIO_PIDR_PIDR19_Msk              (0x80000U)               /*!< PIDR19 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR20_Pos              (20U)                    /*!< PIDR20 (Bit 20)                                       */
#define GPIO_PIDR_PIDR20_Msk              (0x100000U)              /*!< PIDR20 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR21_Pos              (21U)                    /*!< PIDR21 (Bit 21)                                       */
#define GPIO_PIDR_PIDR21_Msk              (0x200000U)              /*!< PIDR21 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR22_Pos              (22U)                    /*!< PIDR22 (Bit 22)                                       */
#define GPIO_PIDR_PIDR22_Msk              (0x400000U)              /*!< PIDR22 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR23_Pos              (23U)                    /*!< PIDR23 (Bit 23)                                       */
#define GPIO_PIDR_PIDR23_Msk              (0x800000U)              /*!< PIDR23 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR24_Pos              (24U)                    /*!< PIDR24 (Bit 24)                                       */
#define GPIO_PIDR_PIDR24_Msk              (0x1000000U)             /*!< PIDR24 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR25_Pos              (25U)                    /*!< PIDR25 (Bit 25)                                       */
#define GPIO_PIDR_PIDR25_Msk              (0x2000000U)             /*!< PIDR25 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR26_Pos              (26U)                    /*!< PIDR26 (Bit 26)                                       */
#define GPIO_PIDR_PIDR26_Msk              (0x4000000U)             /*!< PIDR26 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR27_Pos              (27U)                    /*!< PIDR27 (Bit 27)                                       */
#define GPIO_PIDR_PIDR27_Msk              (0x8000000U)             /*!< PIDR27 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR28_Pos              (28U)                    /*!< PIDR28 (Bit 28)                                       */
#define GPIO_PIDR_PIDR28_Msk              (0x10000000U)            /*!< PIDR28 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR29_Pos              (29U)                    /*!< PIDR29 (Bit 29)                                       */
#define GPIO_PIDR_PIDR29_Msk              (0x20000000U)            /*!< PIDR29 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR30_Pos              (30U)                    /*!< PIDR30 (Bit 30)                                       */
#define GPIO_PIDR_PIDR30_Msk              (0x40000000U)            /*!< PIDR30 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_PIDR31_Pos              (31U)                    /*!< PIDR31 (Bit 31)                                       */
#define GPIO_PIDR_PIDR31_Msk              (0x80000000U)            /*!< PIDR31 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIDR_Pos(x)                  ((uint32)x)             /*!< PIDRx  (Bit x)                                        */
#define GPIO_PIDR_Msk(x)                  (0x01UL<<GPIO_PIDR_Pos(x))  /*!< PIDRx  (Bitfield-Mask: 0x01)                        */
/* =========================================================  POER  ========================================================== */
#define GPIO_POER_POER0_Pos               (0U)                     /*!< POER0 (Bit 0)                                         */
#define GPIO_POER_POER0_Msk               (0x1U)                   /*!< POER0 (Bitfield-Mask: 0x01)                           */
#define GPIO_POER_POER1_Pos               (1U)                     /*!< POER1 (Bit 1)                                         */
#define GPIO_POER_POER1_Msk               (0x2U)                   /*!< POER1 (Bitfield-Mask: 0x01)                           */
#define GPIO_POER_POER2_Pos               (2U)                     /*!< POER2 (Bit 2)                                         */
#define GPIO_POER_POER2_Msk               (0x4U)                   /*!< POER2 (Bitfield-Mask: 0x01)                           */
#define GPIO_POER_POER3_Pos               (3U)                     /*!< POER3 (Bit 3)                                         */
#define GPIO_POER_POER3_Msk               (0x8U)                   /*!< POER3 (Bitfield-Mask: 0x01)                           */
#define GPIO_POER_POER4_Pos               (4U)                     /*!< POER4 (Bit 4)                                         */
#define GPIO_POER_POER4_Msk               (0x10U)                  /*!< POER4 (Bitfield-Mask: 0x01)                           */
#define GPIO_POER_POER5_Pos               (5U)                     /*!< POER5 (Bit 5)                                         */
#define GPIO_POER_POER5_Msk               (0x20U)                  /*!< POER5 (Bitfield-Mask: 0x01)                           */
#define GPIO_POER_POER6_Pos               (6U)                     /*!< POER6 (Bit 6)                                         */
#define GPIO_POER_POER6_Msk               (0x40U)                  /*!< POER6 (Bitfield-Mask: 0x01)                           */
#define GPIO_POER_POER7_Pos               (7U)                     /*!< POER7 (Bit 7)                                         */
#define GPIO_POER_POER7_Msk               (0x80U)                  /*!< POER7 (Bitfield-Mask: 0x01)                           */
#define GPIO_POER_POER8_Pos               (8U)                     /*!< POER8 (Bit 8)                                         */
#define GPIO_POER_POER8_Msk               (0x100U)                 /*!< POER8 (Bitfield-Mask: 0x01)                           */
#define GPIO_POER_POER9_Pos               (9U)                     /*!< POER9 (Bit 9)                                         */
#define GPIO_POER_POER9_Msk               (0x200U)                 /*!< POER9 (Bitfield-Mask: 0x01)                           */
#define GPIO_POER_POER10_Pos              (10U)                    /*!< POER10 (Bit 10)                                       */
#define GPIO_POER_POER10_Msk              (0x400U)                 /*!< POER10 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER11_Pos              (11U)                    /*!< POER11 (Bit 11)                                       */
#define GPIO_POER_POER11_Msk              (0x800U)                 /*!< POER11 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER12_Pos              (12U)                    /*!< POER12 (Bit 12)                                       */
#define GPIO_POER_POER12_Msk              (0x1000U)                /*!< POER12 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER13_Pos              (13U)                    /*!< POER13 (Bit 13)                                       */
#define GPIO_POER_POER13_Msk              (0x2000U)                /*!< POER13 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER14_Pos              (14U)                    /*!< POER14 (Bit 14)                                       */
#define GPIO_POER_POER14_Msk              (0x4000U)                /*!< POER14 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER15_Pos              (15U)                    /*!< POER15 (Bit 15)                                       */
#define GPIO_POER_POER15_Msk              (0x8000U)                /*!< POER15 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER16_Pos              (16U)                    /*!< POER16 (Bit 16)                                       */
#define GPIO_POER_POER16_Msk              (0x10000U)               /*!< POER16 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER17_Pos              (17U)                    /*!< POER17 (Bit 17)                                       */
#define GPIO_POER_POER17_Msk              (0x20000U)               /*!< POER17 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER18_Pos              (18U)                    /*!< POER18 (Bit 18)                                       */
#define GPIO_POER_POER18_Msk              (0x40000U)               /*!< POER18 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER19_Pos              (19U)                    /*!< POER19 (Bit 19)                                       */
#define GPIO_POER_POER19_Msk              (0x80000U)               /*!< POER19 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER20_Pos              (20U)                    /*!< POER20 (Bit 20)                                       */
#define GPIO_POER_POER20_Msk              (0x100000U)              /*!< POER20 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER21_Pos              (21U)                    /*!< POER21 (Bit 21)                                       */
#define GPIO_POER_POER21_Msk              (0x200000U)              /*!< POER21 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER22_Pos              (22U)                    /*!< POER22 (Bit 22)                                       */
#define GPIO_POER_POER22_Msk              (0x400000U)              /*!< POER22 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER23_Pos              (23U)                    /*!< POER23 (Bit 23)                                       */
#define GPIO_POER_POER23_Msk              (0x800000U)              /*!< POER23 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER24_Pos              (24U)                    /*!< POER24 (Bit 24)                                       */
#define GPIO_POER_POER24_Msk              (0x1000000U)             /*!< POER24 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER25_Pos              (25U)                    /*!< POER25 (Bit 25)                                       */
#define GPIO_POER_POER25_Msk              (0x2000000U)             /*!< POER25 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER26_Pos              (26U)                    /*!< POER26 (Bit 26)                                       */
#define GPIO_POER_POER26_Msk              (0x4000000U)             /*!< POER26 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER27_Pos              (27U)                    /*!< POER27 (Bit 27)                                       */
#define GPIO_POER_POER27_Msk              (0x8000000U)             /*!< POER27 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER28_Pos              (28U)                    /*!< POER28 (Bit 28)                                       */
#define GPIO_POER_POER28_Msk              (0x10000000U)            /*!< POER28 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER29_Pos              (29U)                    /*!< POER29 (Bit 29)                                       */
#define GPIO_POER_POER29_Msk              (0x20000000U)            /*!< POER29 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER30_Pos              (30U)                    /*!< POER30 (Bit 30)                                       */
#define GPIO_POER_POER30_Msk              (0x40000000U)            /*!< POER30 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_POER31_Pos              (31U)                    /*!< POER31 (Bit 31)                                       */
#define GPIO_POER_POER31_Msk              (0x80000000U)            /*!< POER31 (Bitfield-Mask: 0x01)                          */
#define GPIO_POER_Pos(x)                  ((uint32)x)             /*!< POERx  (Bit x)                                        */
#define GPIO_POER_Msk(x)                  (0x01UL<<GPIO_POER_Pos(x))  /*!< POERx  (Bitfield-Mask: 0x01)                        */
/* =========================================================  PIER  ========================================================== */
#define GPIO_PIER_PIER0_Pos               (0U)                     /*!< PIER0 (Bit 0)                                         */
#define GPIO_PIER_PIER0_Msk               (0x1U)                   /*!< PIER0 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIER_PIER1_Pos               (1U)                     /*!< PIER1 (Bit 1)                                         */
#define GPIO_PIER_PIER1_Msk               (0x2U)                   /*!< PIER1 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIER_PIER2_Pos               (2U)                     /*!< PIER2 (Bit 2)                                         */
#define GPIO_PIER_PIER2_Msk               (0x4U)                   /*!< PIER2 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIER_PIER3_Pos               (3U)                     /*!< PIER3 (Bit 3)                                         */
#define GPIO_PIER_PIER3_Msk               (0x8U)                   /*!< PIER3 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIER_PIER4_Pos               (4U)                     /*!< PIER4 (Bit 4)                                         */
#define GPIO_PIER_PIER4_Msk               (0x10U)                  /*!< PIER4 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIER_PIER5_Pos               (5U)                     /*!< PIER5 (Bit 5)                                         */
#define GPIO_PIER_PIER5_Msk               (0x20U)                  /*!< PIER5 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIER_PIER6_Pos               (6U)                     /*!< PIER6 (Bit 6)                                         */
#define GPIO_PIER_PIER6_Msk               (0x40U)                  /*!< PIER6 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIER_PIER7_Pos               (7U)                     /*!< PIER7 (Bit 7)                                         */
#define GPIO_PIER_PIER7_Msk               (0x80U)                  /*!< PIER7 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIER_PIER8_Pos               (8U)                     /*!< PIER8 (Bit 8)                                         */
#define GPIO_PIER_PIER8_Msk               (0x100U)                 /*!< PIER8 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIER_PIER9_Pos               (9U)                     /*!< PIER9 (Bit 9)                                         */
#define GPIO_PIER_PIER9_Msk               (0x200U)                 /*!< PIER9 (Bitfield-Mask: 0x01)                           */
#define GPIO_PIER_PIER10_Pos              (10U)                    /*!< PIER10 (Bit 10)                                       */
#define GPIO_PIER_PIER10_Msk              (0x400U)                 /*!< PIER10 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER11_Pos              (11U)                    /*!< PIER11 (Bit 11)                                       */
#define GPIO_PIER_PIER11_Msk              (0x800U)                 /*!< PIER11 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER12_Pos              (12U)                    /*!< PIER12 (Bit 12)                                       */
#define GPIO_PIER_PIER12_Msk              (0x1000U)                /*!< PIER12 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER13_Pos              (13U)                    /*!< PIER13 (Bit 13)                                       */
#define GPIO_PIER_PIER13_Msk              (0x2000U)                /*!< PIER13 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER14_Pos              (14U)                    /*!< PIER14 (Bit 14)                                       */
#define GPIO_PIER_PIER14_Msk              (0x4000U)                /*!< PIER14 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER15_Pos              (15U)                    /*!< PIER15 (Bit 15)                                       */
#define GPIO_PIER_PIER15_Msk              (0x8000U)                /*!< PIER15 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER16_Pos              (16U)                    /*!< PIER16 (Bit 16)                                       */
#define GPIO_PIER_PIER16_Msk              (0x10000U)               /*!< PIER16 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER17_Pos              (17U)                    /*!< PIER17 (Bit 17)                                       */
#define GPIO_PIER_PIER17_Msk              (0x20000U)               /*!< PIER17 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER18_Pos              (18U)                    /*!< PIER18 (Bit 18)                                       */
#define GPIO_PIER_PIER18_Msk              (0x40000U)               /*!< PIER18 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER19_Pos              (19U)                    /*!< PIER19 (Bit 19)                                       */
#define GPIO_PIER_PIER19_Msk              (0x80000U)               /*!< PIER19 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER20_Pos              (20U)                    /*!< PIER20 (Bit 20)                                       */
#define GPIO_PIER_PIER20_Msk              (0x100000U)              /*!< PIER20 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER21_Pos              (21U)                    /*!< PIER21 (Bit 21)                                       */
#define GPIO_PIER_PIER21_Msk              (0x200000U)              /*!< PIER21 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER22_Pos              (22U)                    /*!< PIER22 (Bit 22)                                       */
#define GPIO_PIER_PIER22_Msk              (0x400000U)              /*!< PIER22 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER23_Pos              (23U)                    /*!< PIER23 (Bit 23)                                       */
#define GPIO_PIER_PIER23_Msk              (0x800000U)              /*!< PIER23 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER24_Pos              (24U)                    /*!< PIER24 (Bit 24)                                       */
#define GPIO_PIER_PIER24_Msk              (0x1000000U)             /*!< PIER24 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER25_Pos              (25U)                    /*!< PIER25 (Bit 25)                                       */
#define GPIO_PIER_PIER25_Msk              (0x2000000U)             /*!< PIER25 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER26_Pos              (26U)                    /*!< PIER26 (Bit 26)                                       */
#define GPIO_PIER_PIER26_Msk              (0x4000000U)             /*!< PIER26 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER27_Pos              (27U)                    /*!< PIER27 (Bit 27)                                       */
#define GPIO_PIER_PIER27_Msk              (0x8000000U)             /*!< PIER27 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER28_Pos              (28U)                    /*!< PIER28 (Bit 28)                                       */
#define GPIO_PIER_PIER28_Msk              (0x10000000U)            /*!< PIER28 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER29_Pos              (29U)                    /*!< PIER29 (Bit 29)                                       */
#define GPIO_PIER_PIER29_Msk              (0x20000000U)            /*!< PIER29 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER30_Pos              (30U)                    /*!< PIER30 (Bit 30)                                       */
#define GPIO_PIER_PIER30_Msk              (0x40000000U)            /*!< PIER30 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_PIER31_Pos              (31U)                    /*!< PIER31 (Bit 31)                                       */
#define GPIO_PIER_PIER31_Msk              (0x80000000U)            /*!< PIER31 (Bitfield-Mask: 0x01)                          */
#define GPIO_PIER_Pos(x)                  ((uint32)x)             /*!< PIERx  (Bit x)                                        */
#define GPIO_PIER_Msk(x)                  (0x01UL<<GPIO_PIER_Pos(x))  /*!< PIERx  (Bitfield-Mask: 0x01)                        */
/* ========================================================  OTYPER  ========================================================= */
#define GPIO_OTYPER_OTYP0_Pos             (0U)                     /*!< OTYP0 (Bit 0)                                         */
#define GPIO_OTYPER_OTYP0_Msk             (0x1U)                   /*!< OTYP0 (Bitfield-Mask: 0x01)                           */
#define GPIO_OTYPER_OTYP1_Pos             (1U)                     /*!< OTYP1 (Bit 1)                                         */
#define GPIO_OTYPER_OTYP1_Msk             (0x2U)                   /*!< OTYP1 (Bitfield-Mask: 0x01)                           */
#define GPIO_OTYPER_OTYP2_Pos             (2U)                     /*!< OTYP2 (Bit 2)                                         */
#define GPIO_OTYPER_OTYP2_Msk             (0x4U)                   /*!< OTYP2 (Bitfield-Mask: 0x01)                           */
#define GPIO_OTYPER_OTYP3_Pos             (3U)                     /*!< OTYP3 (Bit 3)                                         */
#define GPIO_OTYPER_OTYP3_Msk             (0x8U)                   /*!< OTYP3 (Bitfield-Mask: 0x01)                           */
#define GPIO_OTYPER_OTYP4_Pos             (4U)                     /*!< OTYP4 (Bit 4)                                         */
#define GPIO_OTYPER_OTYP4_Msk             (0x10U)                  /*!< OTYP4 (Bitfield-Mask: 0x01)                           */
#define GPIO_OTYPER_OTYP5_Pos             (5U)                     /*!< OTYP5 (Bit 5)                                         */
#define GPIO_OTYPER_OTYP5_Msk             (0x20U)                  /*!< OTYP5 (Bitfield-Mask: 0x01)                           */
#define GPIO_OTYPER_OTYP6_Pos             (6U)                     /*!< OTYP6 (Bit 6)                                         */
#define GPIO_OTYPER_OTYP6_Msk             (0x40U)                  /*!< OTYP6 (Bitfield-Mask: 0x01)                           */
#define GPIO_OTYPER_OTYP7_Pos             (7U)                     /*!< OTYP7 (Bit 7)                                         */
#define GPIO_OTYPER_OTYP7_Msk             (0x80U)                  /*!< OTYP7 (Bitfield-Mask: 0x01)                           */
#define GPIO_OTYPER_OTYP8_Pos             (8U)                     /*!< OTYP8 (Bit 8)                                         */
#define GPIO_OTYPER_OTYP8_Msk             (0x100U)                 /*!< OTYP8 (Bitfield-Mask: 0x01)                           */
#define GPIO_OTYPER_OTYP9_Pos             (9U)                     /*!< OTYP9 (Bit 9)                                         */
#define GPIO_OTYPER_OTYP9_Msk             (0x200U)                 /*!< OTYP9 (Bitfield-Mask: 0x01)                           */
#define GPIO_OTYPER_OTYP10_Pos            (10U)                    /*!< OTYP10 (Bit 10)                                       */
#define GPIO_OTYPER_OTYP10_Msk            (0x400U)                 /*!< OTYP10 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP11_Pos            (11U)                    /*!< OTYP11 (Bit 11)                                       */
#define GPIO_OTYPER_OTYP11_Msk            (0x800U)                 /*!< OTYP11 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP12_Pos            (12U)                    /*!< OTYP12 (Bit 12)                                       */
#define GPIO_OTYPER_OTYP12_Msk            (0x1000U)                /*!< OTYP12 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP13_Pos            (13U)                    /*!< OTYP13 (Bit 13)                                       */
#define GPIO_OTYPER_OTYP13_Msk            (0x2000U)                /*!< OTYP13 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP14_Pos            (14U)                    /*!< OTYP14 (Bit 14)                                       */
#define GPIO_OTYPER_OTYP14_Msk            (0x4000U)                /*!< OTYP14 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP15_Pos            (15U)                    /*!< OTYP15 (Bit 15)                                       */
#define GPIO_OTYPER_OTYP15_Msk            (0x8000U)                /*!< OTYP15 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP16_Pos            (16U)                    /*!< OTYP16 (Bit 16)                                       */
#define GPIO_OTYPER_OTYP16_Msk            (0x10000U)               /*!< OTYP16 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP17_Pos            (17U)                    /*!< OTYP17 (Bit 17)                                       */
#define GPIO_OTYPER_OTYP17_Msk            (0x20000U)               /*!< OTYP17 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP18_Pos            (18U)                    /*!< OTYP18 (Bit 18)                                       */
#define GPIO_OTYPER_OTYP18_Msk            (0x40000U)               /*!< OTYP18 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP19_Pos            (19U)                    /*!< OTYP19 (Bit 19)                                       */
#define GPIO_OTYPER_OTYP19_Msk            (0x80000U)               /*!< OTYP19 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP20_Pos            (20U)                    /*!< OTYP20 (Bit 20)                                       */
#define GPIO_OTYPER_OTYP20_Msk            (0x100000U)              /*!< OTYP20 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP21_Pos            (21U)                    /*!< OTYP21 (Bit 21)                                       */
#define GPIO_OTYPER_OTYP21_Msk            (0x200000U)              /*!< OTYP21 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP22_Pos            (22U)                    /*!< OTYP22 (Bit 22)                                       */
#define GPIO_OTYPER_OTYP22_Msk            (0x400000U)              /*!< OTYP22 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP23_Pos            (23U)                    /*!< OTYP23 (Bit 23)                                       */
#define GPIO_OTYPER_OTYP23_Msk            (0x800000U)              /*!< OTYP23 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP24_Pos            (24U)                    /*!< OTYP24 (Bit 24)                                       */
#define GPIO_OTYPER_OTYP24_Msk            (0x1000000U)             /*!< OTYP24 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP25_Pos            (25U)                    /*!< OTYP25 (Bit 25)                                       */
#define GPIO_OTYPER_OTYP25_Msk            (0x2000000U)             /*!< OTYP25 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP26_Pos            (26U)                    /*!< OTYP26 (Bit 26)                                       */
#define GPIO_OTYPER_OTYP26_Msk            (0x4000000U)             /*!< OTYP26 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP27_Pos            (27U)                    /*!< OTYP27 (Bit 27)                                       */
#define GPIO_OTYPER_OTYP27_Msk            (0x8000000U)             /*!< OTYP27 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP28_Pos            (28U)                    /*!< OTYP28 (Bit 28)                                       */
#define GPIO_OTYPER_OTYP28_Msk            (0x10000000U)            /*!< OTYP28 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP29_Pos            (29U)                    /*!< OTYP29 (Bit 29)                                       */
#define GPIO_OTYPER_OTYP29_Msk            (0x20000000U)            /*!< OTYP29 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP30_Pos            (30U)                    /*!< OTYP30 (Bit 30)                                       */
#define GPIO_OTYPER_OTYP30_Msk            (0x40000000U)            /*!< OTYP30 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_OTYP31_Pos            (31U)                    /*!< OTYP31 (Bit 31)                                       */
#define GPIO_OTYPER_OTYP31_Msk            (0x80000000U)            /*!< OTYP31 (Bitfield-Mask: 0x01)                          */
#define GPIO_OTYPER_Pos(x)                ((uint32)x)             /*!< OTYPx  (Bit x)                                        */
#define GPIO_OTYPER_Msk(x)                (0x01UL<<GPIO_OTYPER_Pos(x))  /*!< OTYPx  (Bitfield-Mask: 0x01)                      */

/* =========================================================================================================================== */
/* ================                                           PORT                                            ================ */
/* =========================================================================================================================== */

/* =========================================================  PCR  =========================================================== */
#define PORT_PCR_PU_Pos                   (0U)                     /*!< PU (Bit 0)                                            */
#define PORT_PCR_PU_Msk                   (0x1U)                   /*!< PU (Bitfield-Mask: 0x01)                              */
#define PORT_PCR_PD_Pos                   (1U)                     /*!< PD (Bit 1)                                            */
#define PORT_PCR_PD_Msk                   (0x2U)                   /*!< PD (Bitfield-Mask: 0x01)                              */
#define PORT_PCR_MUX_Pos                  (8U)                     /*!< MUX (Bit 8)                                           */
#define PORT_PCR_MUX_Msk                  (0x700U)                 /*!< MUX (Bitfield-Mask: 0x07)                             */
#define PORT_PCR_LK_Pos                   (15U)                    /*!< LK (Bit 15)                                           */
#define PORT_PCR_LK_Msk                   (0x8000U)                /*!< LK (Bitfield-Mask: 0x01)                              */
#define PORT_PCR_IRQC_Pos                 (16U)                    /*!< IRQC (Bit 16)                                         */
#define PORT_PCR_IRQC_Msk                 (0xf0000U)               /*!< IRQC (Bitfield-Mask: 0x0f)                            */
/* =========================================================  ISFR  ========================================================== */
#define PORT_ISFR_ISF0_Pos                (0U)                     /*!< ISF0 (Bit 0)                                          */
#define PORT_ISFR_ISF0_Msk                (0x1U)                   /*!< ISF0 (Bitfield-Mask: 0x01)                            */
#define PORT_ISFR_ISF1_Pos                (1U)                     /*!< ISF1 (Bit 1)                                          */
#define PORT_ISFR_ISF1_Msk                (0x2U)                   /*!< ISF1 (Bitfield-Mask: 0x01)                            */
#define PORT_ISFR_ISF2_Pos                (2U)                     /*!< ISF2 (Bit 2)                                          */
#define PORT_ISFR_ISF2_Msk                (0x4U)                   /*!< ISF2 (Bitfield-Mask: 0x01)                            */
#define PORT_ISFR_ISF3_Pos                (3U)                     /*!< ISF3 (Bit 3)                                          */
#define PORT_ISFR_ISF3_Msk                (0x8U)                   /*!< ISF3 (Bitfield-Mask: 0x01)                            */
#define PORT_ISFR_ISF4_Pos                (4U)                     /*!< ISF4 (Bit 4)                                          */
#define PORT_ISFR_ISF4_Msk                (0x10U)                  /*!< ISF4 (Bitfield-Mask: 0x01)                            */
#define PORT_ISFR_ISF5_Pos                (5U)                     /*!< ISF5 (Bit 5)                                          */
#define PORT_ISFR_ISF5_Msk                (0x20U)                  /*!< ISF5 (Bitfield-Mask: 0x01)                            */
#define PORT_ISFR_ISF6_Pos                (6U)                     /*!< ISF6 (Bit 6)                                          */
#define PORT_ISFR_ISF6_Msk                (0x40U)                  /*!< ISF6 (Bitfield-Mask: 0x01)                            */
#define PORT_ISFR_ISF7_Pos                (7U)                     /*!< ISF7 (Bit 7)                                          */
#define PORT_ISFR_ISF7_Msk                (0x80U)                  /*!< ISF7 (Bitfield-Mask: 0x01)                            */
#define PORT_ISFR_ISF8_Pos                (8U)                     /*!< ISF8 (Bit 8)                                          */
#define PORT_ISFR_ISF8_Msk                (0x100U)                 /*!< ISF8 (Bitfield-Mask: 0x01)                            */
#define PORT_ISFR_ISF9_Pos                (9U)                     /*!< ISF9 (Bit 9)                                          */
#define PORT_ISFR_ISF9_Msk                (0x200U)                 /*!< ISF9 (Bitfield-Mask: 0x01)                            */
#define PORT_ISFR_ISF10_Pos               (10U)                    /*!< ISF10 (Bit 10)                                        */
#define PORT_ISFR_ISF10_Msk               (0x400U)                 /*!< ISF10 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF11_Pos               (11U)                    /*!< ISF11 (Bit 11)                                        */
#define PORT_ISFR_ISF11_Msk               (0x800U)                 /*!< ISF11 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF12_Pos               (12U)                    /*!< ISF12 (Bit 12)                                        */
#define PORT_ISFR_ISF12_Msk               (0x1000U)                /*!< ISF12 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF13_Pos               (13U)                    /*!< ISF13 (Bit 13)                                        */
#define PORT_ISFR_ISF13_Msk               (0x2000U)                /*!< ISF13 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF14_Pos               (14U)                    /*!< ISF14 (Bit 14)                                        */
#define PORT_ISFR_ISF14_Msk               (0x4000U)                /*!< ISF14 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF15_Pos               (15U)                    /*!< ISF15 (Bit 15)                                        */
#define PORT_ISFR_ISF15_Msk               (0x8000U)                /*!< ISF15 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF16_Pos               (16U)                    /*!< ISF16 (Bit 16)                                        */
#define PORT_ISFR_ISF16_Msk               (0x10000U)               /*!< ISF16 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF17_Pos               (17U)                    /*!< ISF17 (Bit 17)                                        */
#define PORT_ISFR_ISF17_Msk               (0x20000U)               /*!< ISF17 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF18_Pos               (18U)                    /*!< ISF18 (Bit 18)                                        */
#define PORT_ISFR_ISF18_Msk               (0x40000U)               /*!< ISF18 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF19_Pos               (19U)                    /*!< ISF19 (Bit 19)                                        */
#define PORT_ISFR_ISF19_Msk               (0x80000U)               /*!< ISF19 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF20_Pos               (20U)                    /*!< ISF20 (Bit 20)                                        */
#define PORT_ISFR_ISF20_Msk               (0x100000U)              /*!< ISF20 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF21_Pos               (21U)                    /*!< ISF21 (Bit 21)                                        */
#define PORT_ISFR_ISF21_Msk               (0x200000U)              /*!< ISF21 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF22_Pos               (22U)                    /*!< ISF22 (Bit 22)                                        */
#define PORT_ISFR_ISF22_Msk               (0x400000U)              /*!< ISF22 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF23_Pos               (23U)                    /*!< ISF23 (Bit 23)                                        */
#define PORT_ISFR_ISF23_Msk               (0x800000U)              /*!< ISF23 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF24_Pos               (24U)                    /*!< ISF24 (Bit 24)                                        */
#define PORT_ISFR_ISF24_Msk               (0x1000000U)             /*!< ISF24 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF25_Pos               (25U)                    /*!< ISF25 (Bit 25)                                        */
#define PORT_ISFR_ISF25_Msk               (0x2000000U)             /*!< ISF25 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF26_Pos               (26U)                    /*!< ISF26 (Bit 26)                                        */
#define PORT_ISFR_ISF26_Msk               (0x4000000U)             /*!< ISF26 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF27_Pos               (27U)                    /*!< ISF27 (Bit 27)                                        */
#define PORT_ISFR_ISF27_Msk               (0x8000000U)             /*!< ISF27 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF28_Pos               (28U)                    /*!< ISF28 (Bit 28)                                        */
#define PORT_ISFR_ISF28_Msk               (0x10000000U)            /*!< ISF28 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF29_Pos               (29U)                    /*!< ISF29 (Bit 29)                                        */
#define PORT_ISFR_ISF29_Msk               (0x20000000U)            /*!< ISF29 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF30_Pos               (30U)                    /*!< ISF30 (Bit 30)                                        */
#define PORT_ISFR_ISF30_Msk               (0x40000000U)            /*!< ISF30 (Bitfield-Mask: 0x01)                           */
#define PORT_ISFR_ISF31_Pos               (31U)                    /*!< ISF31 (Bit 31)                                        */
#define PORT_ISFR_ISF31_Msk               (0x80000000U)            /*!< ISF31 (Bitfield-Mask: 0x01)                           */
#define GPIO_ISFR_Pos(x)                  ((uint32)x)             /*!< PIERx  (Bit x)                                        */
#define GPIO_ISFR_Msk(x)                  (0x01UL<<GPIO_ISFR_Pos(x))  /*!< PIERx  (Bitfield-Mask: 0x01)                        */
/* =========================================================  DFER  ========================================================== */
#define PORT_DFER_DFE0_Pos                (0U)                     /*!< DFE0 (Bit 0)                                          */
#define PORT_DFER_DFE0_Msk                (0x1U)                   /*!< DFE0 (Bitfield-Mask: 0x01)                            */
#define PORT_DFER_DFE1_Pos                (1U)                     /*!< DFE1 (Bit 1)                                          */
#define PORT_DFER_DFE1_Msk                (0x2U)                   /*!< DFE1 (Bitfield-Mask: 0x01)                            */
#define PORT_DFER_DFE2_Pos                (2U)                     /*!< DFE2 (Bit 2)                                          */
#define PORT_DFER_DFE2_Msk                (0x4U)                   /*!< DFE2 (Bitfield-Mask: 0x01)                            */
#define PORT_DFER_DFE3_Pos                (3U)                     /*!< DFE3 (Bit 3)                                          */
#define PORT_DFER_DFE3_Msk                (0x8U)                   /*!< DFE3 (Bitfield-Mask: 0x01)                            */
#define PORT_DFER_DFE4_Pos                (4U)                     /*!< DFE4 (Bit 4)                                          */
#define PORT_DFER_DFE4_Msk                (0x10U)                  /*!< DFE4 (Bitfield-Mask: 0x01)                            */
#define PORT_DFER_DFE5_Pos                (5U)                     /*!< DFE5 (Bit 5)                                          */
#define PORT_DFER_DFE5_Msk                (0x20U)                  /*!< DFE5 (Bitfield-Mask: 0x01)                            */
#define PORT_DFER_DFE6_Pos                (6U)                     /*!< DFE6 (Bit 6)                                          */
#define PORT_DFER_DFE6_Msk                (0x40U)                  /*!< DFE6 (Bitfield-Mask: 0x01)                            */
#define PORT_DFER_DFE7_Pos                (7U)                     /*!< DFE7 (Bit 7)                                          */
#define PORT_DFER_DFE7_Msk                (0x80U)                  /*!< DFE7 (Bitfield-Mask: 0x01)                            */
#define PORT_DFER_Pos(x)                  ((uint32)x)             /*!< DFERx  (Bit x)                                        */
#define PORT_DFER_Msk(x)                  (0x01UL<<PORT_DFER_Pos(x))  /*!< DFERx  (Bitfield-Mask: 0x01)                        */
/* =========================================================  DFCR  ========================================================== */
#define PORT_DFCR_CS_Pos                  (0U)                     /*!< CS (Bit 0)                                            */
#define PORT_DFCR_CS_Msk                  (0x1U)                   /*!< CS (Bitfield-Mask: 0x01)                              */
/* =========================================================  DFWR  ========================================================== */
#define PORT_DFWR_FILT_Pos                (0U)                     /*!< FILT (Bit 0)                                          */
#define PORT_DFWR_FILT_Msk                (0x1fU)                  /*!< FILT (Bitfield-Mask: 0x1f)                            */

/* =========================================================================================================================== */
/* ================                                           CAN                                             ================ */
/* =========================================================================================================================== */

/* =======================================================  TBUF/RBUF  ======================================================= */
#define CAN_INFO_ID_Pos                   (0U)                     /*!< ID (Bit 0)                                            */
#define CAN_INFO_ID_Msk                   (0x1FFFFFFFU)            /*!< ID (Bitfield-Mask: 0x1FFFFFFF)                        */
#define CAN_INFO_TTSEN_Pos                (30U)                    /*!< TTSEN (Bit 30)                                        */
#define CAN_INFO_TTSEN_Msk                (0x40000000U)            /*!< TTSEN (Bitfield-Mask: 0x01)                           */
#define CAN_INFO_ESI_Pos                  (31U)                    /*!< ESI (Bit 31)                                          */
#define CAN_INFO_ESI_Msk                  (0x80000000U)            /*!< ESI (Bitfield-Mask: 0x01)                             */
#define CAN_INFO_DLC_Pos                  (0U)                     /*!< DLC (Bit 0)                                           */
#define CAN_INFO_DLC_Msk                  (0x0FU)                  /*!< DLC (Bitfield-Mask: 0x0F)                             */
#define CAN_INFO_BRS_Pos                  (4U)                     /*!< BRS (Bit 4)                                           */
#define CAN_INFO_BRS_Msk                  (0x10U)                  /*!< BRS (Bitfield-Mask: 0x01)                             */
#define CAN_INFO_FDF_Pos                  (5U)                     /*!< FDF (Bit 5)                                           */
#define CAN_INFO_FDF_Msk                  (0x20U)                  /*!< FDF (Bitfield-Mask: 0x01)                             */
#define CAN_INFO_RTR_Pos                  (6U)                     /*!< RTR (Bit 6)                                           */
#define CAN_INFO_RTR_Msk                  (0x40U)                  /*!< RTR (Bitfield-Mask: 0x01)                             */
#define CAN_INFO_IDE_Pos                  (7U)                     /*!< IDE (Bit 7)                                           */
#define CAN_INFO_IDE_Msk                  (0x80U)                  /*!< IDE (Bitfield-Mask: 0x01)                             */
#define CAN_INFO_TX_Pos                   (12U)                    /*!< TX (Bit 12)                                           */
#define CAN_INFO_TX_Msk                   (0x1000U)                /*!< TX (Bitfield-Mask: 0x01)                              */
#define CAN_INFO_KOER_Pos                 (13U)                    /*!< KOER (Bit 13)                                         */
#define CAN_INFO_KOER_Msk                 (0xE000U)                /*!< KOER (Bitfield-Mask: 0x07)                            */
/* =========================================================  CTRL0  ========================================================= */
#define CAN_CTRL0_BUSOFF_Pos              (0U)                     /*!< BUSOFF (Bit 0)                                        */
#define CAN_CTRL0_BUSOFF_Msk              (0x1U)                   /*!< BUSOFF (Bitfield-Mask: 0x01)                          */
#define CAN_CTRL0_TACTIVE_Pos             (1U)                     /*!< TACTIVE (Bit 1)                                       */
#define CAN_CTRL0_TACTIVE_Msk             (0x2U)                   /*!< TACTIVE (Bitfield-Mask: 0x01)                         */
#define CAN_CTRL0_RACTIVE_Pos             (2U)                     /*!< RACTIVE (Bit 2)                                       */
#define CAN_CTRL0_RACTIVE_Msk             (0x4U)                   /*!< RACTIVE (Bitfield-Mask: 0x01)                         */
#define CAN_CTRL0_TSSS_Pos                (3U)                     /*!< TSSS (Bit 3)                                          */
#define CAN_CTRL0_TSSS_Msk                (0x8U)                   /*!< TSSS (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL0_TPSS_Pos                (4U)                     /*!< TPSS (Bit 4)                                          */
#define CAN_CTRL0_TPSS_Msk                (0x10U)                  /*!< TPSS (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL0_LBMI_Pos                (5U)                     /*!< LBMI (Bit 5)                                          */
#define CAN_CTRL0_LBMI_Msk                (0x20U)                  /*!< LBMI (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL0_LBME_Pos                (6U)                     /*!< LBME (Bit 6)                                          */
#define CAN_CTRL0_LBME_Msk                (0x40U)                  /*!< LBME (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL0_RESET_Pos               (7U)                     /*!< RESET (Bit 7)                                         */
#define CAN_CTRL0_RESET_Msk               (0x80U)                  /*!< RESET (Bitfield-Mask: 0x01)                           */
#define CAN_CTRL0_TSA_Pos                 (8U)                     /*!< TSA (Bit 8)                                           */
#define CAN_CTRL0_TSA_Msk                 (0x100U)                 /*!< TSA (Bitfield-Mask: 0x01)                             */
#define CAN_CTRL0_TSALL_Pos               (9U)                     /*!< TSALL (Bit 9)                                         */
#define CAN_CTRL0_TSALL_Msk               (0x200U)                 /*!< TSALL (Bitfield-Mask: 0x01)                           */
#define CAN_CTRL0_TSONE_Pos               (10U)                    /*!< TSONE (Bit 10)                                        */
#define CAN_CTRL0_TSONE_Msk               (0x400U)                 /*!< TSONE (Bitfield-Mask: 0x01)                           */
#define CAN_CTRL0_TPA_Pos                 (11U)                    /*!< TPA (Bit 11)                                          */
#define CAN_CTRL0_TPA_Msk                 (0x800U)                 /*!< TPA (Bitfield-Mask: 0x01)                             */
#define CAN_CTRL0_TPE_Pos                 (12U)                    /*!< TPE (Bit 12)                                          */
#define CAN_CTRL0_TPE_Msk                 (0x1000U)                /*!< TPE (Bitfield-Mask: 0x01)                             */
#define CAN_CTRL0_STBY_Pos                (13U)                    /*!< STBY (Bit 13)                                         */
#define CAN_CTRL0_STBY_Msk                (0x2000U)                /*!< STBY (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL0_LOM_Pos                 (14U)                    /*!< LOM (Bit 14)                                          */
#define CAN_CTRL0_LOM_Msk                 (0x4000U)                /*!< LOM (Bitfield-Mask: 0x01)                             */
#define CAN_CTRL0_TBSEL_Pos               (15U)                    /*!< TBSEL (Bit 15)                                        */
#define CAN_CTRL0_TBSEL_Msk               (0x8000U)                /*!< TBSEL (Bitfield-Mask: 0x01)                           */
#define CAN_CTRL0_TSSTAT_Pos              (16U)                    /*!< TSSTAT (Bit 16)                                       */
#define CAN_CTRL0_TSSTAT_Msk              (0x30000U)               /*!< TSSTAT (Bitfield-Mask: 0x03)                          */
#define CAN_CTRL0_IDLE_Pos                (18U)                    /*!< IDLE (Bit 18)                                         */
#define CAN_CTRL0_IDLE_Msk                (0x40000U)               /*!< IDLE (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL0_BOREC_Pos               (19U)                    /*!< BOREC (Bit 19)                                        */
#define CAN_CTRL0_BOREC_Msk               (0x80000U)               /*!< BOREC (Bitfield-Mask: 0x01)                           */
#define CAN_CTRL0_TSMODE_Pos              (21U)                    /*!< TSMODE (Bit 21)                                       */
#define CAN_CTRL0_TSMODE_Msk              (0x200000U)              /*!< TSMODE (Bitfield-Mask: 0x01)                          */
#define CAN_CTRL0_TSNEXT_Pos              (22U)                    /*!< TSNEXT (Bit 22)                                       */
#define CAN_CTRL0_TSNEXT_Msk              (0x400000U)              /*!< TSNEXT (Bitfield-Mask: 0x01)                          */
#define CAN_CTRL0_FDISO_Pos               (23U)                    /*!< FDISO (Bit 23)                                        */
#define CAN_CTRL0_FDISO_Msk               (0x800000U)              /*!< FDISO (Bitfield-Mask: 0x01)                           */
#define CAN_CTRL0_RSTAT_Pos               (24U)                    /*!< RSTAT (Bit 24)                                        */
#define CAN_CTRL0_RSTAT_Msk               (0x3000000U)             /*!< RSTAT (Bitfield-Mask: 0x03)                           */
#define CAN_CTRL0_DREN_Pos                (26U)                    /*!< DREN (Bit 26)                                         */
#define CAN_CTRL0_DREN_Msk                (0x4000000U)             /*!< DREN (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL0_RBALL_Pos               (27U)                    /*!< RBALL (Bit 27)                                        */
#define CAN_CTRL0_RBALL_Msk               (0x8000000U)             /*!< RBALL (Bitfield-Mask: 0x01)                           */
#define CAN_CTRL0_RREL_Pos                (28U)                    /*!< RREL (Bit 28)                                         */
#define CAN_CTRL0_RREL_Msk                (0x10000000U)            /*!< RREL (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL0_ROV_Pos                 (29U)                    /*!< ROV (Bit 29)                                          */
#define CAN_CTRL0_ROV_Msk                 (0x20000000U)            /*!< ROV (Bitfield-Mask: 0x01)                             */
#define CAN_CTRL0_ROM_Pos                 (30U)                    /*!< ROM (Bit 30)                                          */
#define CAN_CTRL0_ROM_Msk                 (0x40000000U)            /*!< ROM (Bitfield-Mask: 0x01)                             */
#define CAN_CTRL0_SACK_Pos                (31U)                    /*!< SACK (Bit 31)                                         */
#define CAN_CTRL0_SACK_Msk                (0x80000000U)            /*!< SACK (Bitfield-Mask: 0x01)                            */
/* =========================================================  CTRL1  ========================================================= */
#define CAN_CTRL1_TSFF_Pos                (0U)                     /*!< TSFF (Bit 0)                                          */
#define CAN_CTRL1_TSFF_Msk                (0x1U)                   /*!< TSFF (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL1_EIE_Pos                 (1U)                     /*!< EIE (Bit 1)                                           */
#define CAN_CTRL1_EIE_Msk                 (0x2U)                   /*!< EIE (Bitfield-Mask: 0x01)                             */
#define CAN_CTRL1_TSIE_Pos                (2U)                     /*!< TSIE (Bit 2)                                          */
#define CAN_CTRL1_TSIE_Msk                (0x4U)                   /*!< TSIE (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL1_TPIE_Pos                (3U)                     /*!< TPIE (Bit 3)                                          */
#define CAN_CTRL1_TPIE_Msk                (0x8U)                   /*!< TPIE (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL1_RAFIE_Pos               (4U)                     /*!< RAFIE (Bit 4)                                         */
#define CAN_CTRL1_RAFIE_Msk               (0x10U)                  /*!< RAFIE (Bitfield-Mask: 0x01)                           */
#define CAN_CTRL1_RFIE_Pos                (5U)                     /*!< RFIE (Bit 5)                                          */
#define CAN_CTRL1_RFIE_Msk                (0x20U)                  /*!< RFIE (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL1_ROIE_Pos                (6U)                     /*!< ROIE (Bit 6)                                          */
#define CAN_CTRL1_ROIE_Msk                (0x40U)                  /*!< ROIE (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL1_RIE_Pos                 (7U)                     /*!< RIE (Bit 7)                                           */
#define CAN_CTRL1_RIE_Msk                 (0x80U)                  /*!< RIE (Bitfield-Mask: 0x01)                             */
#define CAN_CTRL1_AIF_Pos                 (8U)                     /*!< AIF (Bit 8)                                           */
#define CAN_CTRL1_AIF_Msk                 (0x100U)                 /*!< AIF (Bitfield-Mask: 0x01)                             */
#define CAN_CTRL1_EIF_Pos                 (9U)                     /*!< EIF (Bit 9)                                           */
#define CAN_CTRL1_EIF_Msk                 (0x200U)                 /*!< EIF (Bitfield-Mask: 0x01)                             */
#define CAN_CTRL1_TSIF_Pos                (10U)                    /*!< TSIF (Bit 10)                                         */
#define CAN_CTRL1_TSIF_Msk                (0x400U)                 /*!< TSIF (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL1_TPIF_Pos                (11U)                    /*!< TPIF (Bit 11)                                         */
#define CAN_CTRL1_TPIF_Msk                (0x800U)                 /*!< TPIF (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL1_RAFIF_Pos               (12U)                    /*!< RAFIF (Bit 12)                                        */
#define CAN_CTRL1_RAFIF_Msk               (0x1000U)                /*!< RAFIF (Bitfield-Mask: 0x01)                           */
#define CAN_CTRL1_RFIF_Pos                (13U)                    /*!< RFIF (Bit 13)                                         */
#define CAN_CTRL1_RFIF_Msk                (0x2000U)                /*!< RFIF (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL1_ROIF_Pos                (14U)                    /*!< ROIF (Bit 14)                                         */
#define CAN_CTRL1_ROIF_Msk                (0x4000U)                /*!< ROIF (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL1_RIF_Pos                 (15U)                    /*!< RIF (Bit 15)                                          */
#define CAN_CTRL1_RIF_Msk                 (0x8000U)                /*!< RIF (Bitfield-Mask: 0x01)                             */
#define CAN_CTRL1_BEIF_Pos                (16U)                    /*!< BEIF (Bit 16)                                         */
#define CAN_CTRL1_BEIF_Msk                (0x10000U)               /*!< BEIF (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL1_BEIE_Pos                (17U)                    /*!< BEIE (Bit 17)                                         */
#define CAN_CTRL1_BEIE_Msk                (0x20000U)               /*!< BEIE (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL1_ALIF_Pos                (18U)                    /*!< ALIF (Bit 18)                                         */
#define CAN_CTRL1_ALIF_Msk                (0x40000U)               /*!< ALIF (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL1_ALIE_Pos                (19U)                    /*!< ALIE (Bit 19)                                         */
#define CAN_CTRL1_ALIE_Msk                (0x80000U)               /*!< ALIE (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL1_EPIF_Pos                (20U)                    /*!< EPIF (Bit 20)                                         */
#define CAN_CTRL1_EPIF_Msk                (0x100000U)              /*!< EPIF (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL1_EPIE_Pos                (21U)                    /*!< EPIE (Bit 21)                                         */
#define CAN_CTRL1_EPIE_Msk                (0x200000U)              /*!< EPIE (Bitfield-Mask: 0x01)                            */
#define CAN_CTRL1_EPASS_Pos               (22U)                    /*!< EPASS (Bit 22)                                        */
#define CAN_CTRL1_EPASS_Msk               (0x400000U)              /*!< EPASS (Bitfield-Mask: 0x01)                           */
#define CAN_CTRL1_EWARN_Pos               (23U)                    /*!< EWARN (Bit 23)                                        */
#define CAN_CTRL1_EWARN_Msk               (0x800000U)              /*!< EWARN (Bitfield-Mask: 0x01)                           */
#define CAN_CTRL1_EWL_Pos                 (24U)                    /*!< EWL (Bit 24)                                          */
#define CAN_CTRL1_EWL_Msk                 (0xf000000U)             /*!< EWL (Bitfield-Mask: 0x0f)                             */
#define CAN_CTRL1_AFWL_Pos                (28U)                    /*!< AFWL (Bit 28)                                         */
#define CAN_CTRL1_AFWL_Msk                (0xf0000000U)            /*!< AFWL (Bitfield-Mask: 0x0f)                            */
/* =======================================================  SBITRATE  ======================================================== */
#define CAN_SBITRATE_S_SEG_1_Pos          (0U)                     /*!< S_SEG_1 (Bit 0)                                       */
#define CAN_SBITRATE_S_SEG_1_Msk          (0xffU)                  /*!< S_SEG_1 (Bitfield-Mask: 0xff)                         */
#define CAN_SBITRATE_S_SEG_2_Pos          (8U)                     /*!< S_SEG_2 (Bit 8)                                       */
#define CAN_SBITRATE_S_SEG_2_Msk          (0x7f00U)                /*!< S_SEG_2 (Bitfield-Mask: 0x7f)                         */
#define CAN_SBITRATE_S_SJW_Pos            (16U)                    /*!< S_SJW (Bit 16)                                        */
#define CAN_SBITRATE_S_SJW_Msk            (0x7f0000U)              /*!< S_SJW (Bitfield-Mask: 0x7f)                           */
#define CAN_SBITRATE_S_PRESC_Pos          (24U)                    /*!< S_PRESC (Bit 24)                                      */
#define CAN_SBITRATE_S_PRESC_Msk          (0xff000000U)            /*!< S_PRESC (Bitfield-Mask: 0xff)                         */
/* =======================================================  FBITRATE  ======================================================== */
#define CAN_FBITRATE_F_SEG_1_Pos          (0U)                     /*!< F_SEG_1 (Bit 0)                                       */
#define CAN_FBITRATE_F_SEG_1_Msk          (0x1fU)                  /*!< F_SEG_1 (Bitfield-Mask: 0x1f)                         */
#define CAN_FBITRATE_F_SEG_2_Pos          (8U)                     /*!< F_SEG_2 (Bit 8)                                       */
#define CAN_FBITRATE_F_SEG_2_Msk          (0xf00U)                 /*!< F_SEG_2 (Bitfield-Mask: 0xf)                          */
#define CAN_FBITRATE_F_SJW_Pos            (16U)                    /*!< F_SJW (Bit 16)                                        */
#define CAN_FBITRATE_F_SJW_Msk            (0xf0000U)               /*!< F_SJW (Bitfield-Mask: 0xf)                            */
#define CAN_FBITRATE_F_PRESC_Pos          (24U)                    /*!< F_PRESC (Bit 24)                                      */
#define CAN_FBITRATE_F_PRESC_Msk          (0xff000000U)            /*!< F_PRESC (Bitfield-Mask: 0xff)                         */
/* ========================================================  ERRINFO  ======================================================== */
#define CAN_ERRINFO_ALC_Pos               (0U)                     /*!< ALC (Bit 0)                                           */
#define CAN_ERRINFO_ALC_Msk               (0x1fU)                  /*!< ALC (Bitfield-Mask: 0x1f)                             */
#define CAN_ERRINFO_KOER_Pos              (5U)                     /*!< KOER (Bit 5)                                          */
#define CAN_ERRINFO_KOER_Msk              (0xe0U)                  /*!< KOER (Bitfield-Mask: 0x07)                            */
#define CAN_ERRINFO_SSPOFF_Pos            (8U)                     /*!< SSPOFF (Bit 8)                                        */
#define CAN_ERRINFO_SSPOFF_Msk            (0x7f00U)                /*!< SSPOFF (Bitfield-Mask: 0x7f)                          */
#define CAN_ERRINFO_TDCEN_Pos             (15U)                    /*!< TDCEN (Bit 15)                                        */
#define CAN_ERRINFO_TDCEN_Msk             (0x8000U)                /*!< TDCEN (Bitfield-Mask: 0x01)                           */
#define CAN_ERRINFO_RECNT_Pos             (16U)                    /*!< RECNT (Bit 16)                                        */
#define CAN_ERRINFO_RECNT_Msk             (0xff0000U)              /*!< RECNT (Bitfield-Mask: 0xff)                           */
#define CAN_ERRINFO_TECNT_Pos             (24U)                    /*!< TECNT (Bit 24)                                        */
#define CAN_ERRINFO_TECNT_Msk             (0xff000000U)            /*!< TECNT (Bitfield-Mask: 0xff)                           */
/* =======================================================  ACFCTRL0  ======================================================== */
#define CAN_ACFCTRL0_ACFADR_Pos           (0U)                     /*!< ACFADR (Bit 0)                                        */
#define CAN_ACFCTRL0_ACFADR_Msk           (0x3fU)                  /*!< ACFADR (Bitfield-Mask: 0x3f)                          */
#define CAN_ACFCTRL0_SELMASK_Pos          (6U)                     /*!< SELMASK (Bit 6)                                       */
#define CAN_ACFCTRL0_SELMASK_Msk          (0x40U)                  /*!< SELMASK (Bitfield-Mask: 0x01)                         */
#define CAN_ACFCTRL0_TIMEEN_Pos           (8U)                     /*!< TIMEEN (Bit 8)                                        */
#define CAN_ACFCTRL0_TIMEEN_Msk           (0x100U)                 /*!< TIMEEN (Bitfield-Mask: 0x01)                          */
#define CAN_ACFCTRL0_TIMEPOS_Pos          (9U)                     /*!< TIMEPOS (Bit 9)                                       */
#define CAN_ACFCTRL0_TIMEPOS_Msk          (0x200U)                 /*!< TIMEPOS (Bitfield-Mask: 0x01)                         */
#define CAN_ACFCTRL0_TCEN_Pos             (10U)                    /*!< TCEN (Bit 10)                                         */
#define CAN_ACFCTRL0_TCEN_Msk             (0x400U)                 /*!< TCEN (Bitfield-Mask: 0x01)                            */
#define CAN_ACFCTRL0_TCSS_Pos             (11U)                    /*!< TCSS (Bit 11)                                         */
#define CAN_ACFCTRL0_TCSS_Msk             (0x800U)                 /*!< TCSS (Bitfield-Mask: 0x01)                            */
#define CAN_ACFCTRL0_ACFEN_Pos            (16U)                    /*!< ACFEN (Bit 16)                                        */
#define CAN_ACFCTRL0_ACFEN_Msk            (0xffff0000U)            /*!< ACFEN (Bitfield-Mask: 0xffff)                         */
/* =======================================================  ACFCTRL1  ======================================================== */
#define CAN_ACFCTRL1_ACFEN_Pos            (0U)                     /*!< ACFEN (Bit 0)                                         */
#define CAN_ACFCTRL1_ACFEN_Msk            (0xffffffffU)            /*!< ACFEN (Bitfield-Mask: 0xffffffff)                     */
/* =======================================================  ACFCTRL2  ======================================================== */
#define CAN_ACFCTRL2_ACFEN_Pos            (0U)                     /*!< ACFEN (Bit 0)                                         */
#define CAN_ACFCTRL2_ACFEN_Msk            (0xfffU)                 /*!< ACFEN (Bitfield-Mask: 0xfff)                          */
/* ==========================================================  ACF  ========================================================== */
#define CAN_ACF_ACODE_Pos                 (0U)                     /*!< ACODE (Bit 0)                                         */
#define CAN_ACF_ACODE_Msk                 (0x1fffffffU)            /*!< ACODE (Bitfield-Mask: 0x1fffffff)                     */
#define CAN_ACF_AIDE_Pos                  (29U)                    /*!< AIDE (Bit 29)                                         */
#define CAN_ACF_AIDE_Msk                  (0x20000000U)            /*!< AIDE (Bitfield-Mask: 0x01)                            */
#define CAN_ACF_AIDEE_Pos                 (30U)                    /*!< AIDEE (Bit 30)                                        */
#define CAN_ACF_AIDEE_Msk                 (0x40000000U)            /*!< AIDEE (Bitfield-Mask: 0x01)                           */
/* ========================================================  VERMEM  ========================================================= */
#define CAN_VERMEM_VERSION_Pos            (0U)                     /*!< VERSION (Bit 0)                                       */
#define CAN_VERMEM_VERSION_Msk            (0xffffU)                /*!< VERSION (Bitfield-Mask: 0xffff)                       */
#define CAN_VERMEM_MEEN_Pos               (16U)                    /*!< MEEN (Bit 16)                                         */
#define CAN_VERMEM_MEEN_Msk               (0x10000U)               /*!< MEEN (Bitfield-Mask: 0x01)                            */
#define CAN_VERMEM_MDWIE_Pos              (17U)                    /*!< MDWIE (Bit 17)                                        */
#define CAN_VERMEM_MDWIE_Msk              (0x20000U)               /*!< MDWIE (Bitfield-Mask: 0x01)                           */
#define CAN_VERMEM_MDWIF_Pos              (18U)                    /*!< MDWIF (Bit 18)                                        */
#define CAN_VERMEM_MDWIF_Msk              (0x40000U)               /*!< MDWIF (Bitfield-Mask: 0x01)                           */
#define CAN_VERMEM_MDEIE_Pos              (19U)                    /*!< MDEIE (Bit 19)                                        */
#define CAN_VERMEM_MDEIE_Msk              (0x80000U)               /*!< MDEIE (Bitfield-Mask: 0x01)                           */
#define CAN_VERMEM_MDEIF_Pos              (20U)                    /*!< MDEIF (Bit 20)                                        */
#define CAN_VERMEM_MDEIF_Msk              (0x100000U)              /*!< MDEIF (Bitfield-Mask: 0x01)                           */
#define CAN_VERMEM_MEID_Pos               (23U)                    /*!< MEID (Bit 23)                                         */
#define CAN_VERMEM_MEID_Msk               (0x800000U)              /*!< MEID (Bitfield-Mask: 0x01)                            */
#define CAN_VERMEM_ACFA_Pos               (24U)                    /*!< ACFA (Bit 24)                                         */
#define CAN_VERMEM_ACFA_Msk               (0x1000000U)             /*!< ACFA (Bitfield-Mask: 0x01)                            */
#define CAN_VERMEM_TXS_Pos                (25U)                    /*!< TXS (Bit 25)                                          */
#define CAN_VERMEM_TXS_Msk                (0x2000000U)             /*!< TXS (Bitfield-Mask: 0x01)                             */
#define CAN_VERMEM_HELOC_Pos              (27U)                    /*!< HELOC (Bit 27)                                        */
#define CAN_VERMEM_HELOC_Msk              (0x18000000U)            /*!< HELOC (Bitfield-Mask: 0x03)                           */
/* =========================================================  MEMES  ========================================================= */
#define CAN_MEMES_MEBP1_Pos               (0U)                     /*!< MEBP1 (Bit 0)                                         */
#define CAN_MEMES_MEBP1_Msk               (0x3fU)                  /*!< MEBP1 (Bitfield-Mask: 0x3f)                           */
#define CAN_MEMES_ME1EE_Pos               (6U)                     /*!< ME1EE (Bit 6)                                         */
#define CAN_MEMES_ME1EE_Msk               (0x40U)                  /*!< ME1EE (Bitfield-Mask: 0x01)                           */
#define CAN_MEMES_MEBP2_Pos               (8U)                     /*!< MEBP2 (Bit 8)                                         */
#define CAN_MEMES_MEBP2_Msk               (0x3f00U)                /*!< MEBP2 (Bitfield-Mask: 0x3f)                           */
#define CAN_MEMES_ME2EE_Pos               (14U)                    /*!< ME2EE (Bit 14)                                        */
#define CAN_MEMES_ME2EE_Msk               (0x4000U)                /*!< ME2EE (Bitfield-Mask: 0x01)                           */
#define CAN_MEMES_MEEEC_Pos               (16U)                    /*!< MEEEC (Bit 16)                                        */
#define CAN_MEMES_MEEEC_Msk               (0xf0000U)               /*!< MEEEC (Bitfield-Mask: 0x0f)                           */
#define CAN_MEMES_MENEC_Pos               (20U)                    /*!< MENEC (Bit 20)                                        */
#define CAN_MEMES_MENEC_Msk               (0xf00000U)              /*!< MENEC (Bitfield-Mask: 0x0f)                           */
#define CAN_MEMES_MEL_Pos                 (24U)                    /*!< MEL (Bit 24)                                          */
#define CAN_MEMES_MEL_Msk                 (0x3000000U)             /*!< MEL (Bitfield-Mask: 0x03)                             */
#define CAN_MEMES_MES_Pos                 (26U)                    /*!< MES (Bit 26)                                          */
#define CAN_MEMES_MES_Msk                 (0x4000000U)             /*!< MES (Bitfield-Mask: 0x01)                             */
/* ==========================================================  TBU  ========================================================== */
#define CAN_TBU_TBUC_Pos                  (0U)                     /*!< TBUC (Bit 0)                                          */
#define CAN_TBU_TBUC_Msk                  (0x1fU)                  /*!< TBUC (Bitfield-Mask: 0x1f)                            */
/* ==========================================================  RBU  ========================================================== */
#define CAN_RBU_RBUC_Pos                  (0U)                     /*!< RBUC (Bit 0)                                          */
#define CAN_RBU_RBUC_Msk                  (0x1fU)                  /*!< RBUC (Bitfield-Mask: 0x1f)                            */

/* =========================================================================================================================== */
/* ================                                           UART                                            ================ */
/* =========================================================================================================================== */

/* ==========================================================  RBR  ========================================================== */
#define UART_RBR_RBR_THR_Pos              (0U)                     /*!< RBR_THR (Bit 0)                                       */
#define UART_RBR_RBR_THR_Msk              (0x1ffU)                 /*!< RBR_THR (Bitfield-Mask: 0x1ff)                        */
/* =========================================================  DIV_L  ========================================================= */
#define UART_DIV_L_DIV_L_Pos              (0U)                     /*!< DIV_L (Bit 0)                                         */
#define UART_DIV_L_DIV_L_Msk              (0xffU)                  /*!< DIV_L (Bitfield-Mask: 0xff)                           */
/* =========================================================  DIV_H  ========================================================= */
#define UART_DIV_H_DIV_H_Pos              (0U)                     /*!< DIV_H (Bit 0)                                         */
#define UART_DIV_H_DIV_H_Msk              (0xffU)                  /*!< DIV_H (Bitfield-Mask: 0xff)                           */
/* =========================================================  LCR0  ========================================================== */
#define UART_LCR0_WLS1_WLS0_Pos           (0U)                     /*!< WLS1_WLS0 (Bit 0)                                     */
#define UART_LCR0_WLS1_WLS0_Msk           (0x3U)                   /*!< WLS1_WLS0 (Bitfield-Mask: 0x03)                       */
#define UART_LCR0_STB_Pos                 (2U)                     /*!< STB (Bit 2)                                           */
#define UART_LCR0_STB_Msk                 (0x4U)                   /*!< STB (Bitfield-Mask: 0x01)                             */
#define UART_LCR0_PEN_Pos                 (3U)                     /*!< PEN (Bit 3)                                           */
#define UART_LCR0_PEN_Msk                 (0x8U)                   /*!< PEN (Bitfield-Mask: 0x01)                             */
#define UART_LCR0_EPS_Pos                 (4U)                     /*!< EPS (Bit 4)                                           */
#define UART_LCR0_EPS_Msk                 (0x10U)                  /*!< EPS (Bitfield-Mask: 0x01)                             */
#define UART_LCR0_SP_Pos                  (5U)                     /*!< SP (Bit 5)                                            */
#define UART_LCR0_SP_Msk                  (0x20U)                  /*!< SP (Bitfield-Mask: 0x01)                              */
#define UART_LCR0_SB_Pos                  (6U)                     /*!< SB (Bit 6)                                            */
#define UART_LCR0_SB_Msk                  (0x40U)                  /*!< SB (Bitfield-Mask: 0x01)                              */
/* =========================================================  LCR1  ========================================================== */
#define UART_LCR1_RXEN_Pos                (0U)                     /*!< RXEN (Bit 0)                                          */
#define UART_LCR1_RXEN_Msk                (0x1U)                   /*!< RXEN (Bitfield-Mask: 0x01)                            */
#define UART_LCR1_TXEN_Pos                (1U)                     /*!< TXEN (Bit 1)                                          */
#define UART_LCR1_TXEN_Msk                (0x2U)                   /*!< TXEN (Bitfield-Mask: 0x01)                            */
#define UART_LCR1_RSRC_Pos                (2U)                     /*!< RSRC (Bit 2)                                          */
#define UART_LCR1_RSRC_Msk                (0x4U)                   /*!< RSRC (Bitfield-Mask: 0x01)                            */
#define UART_LCR1_TXDIR_Pos               (3U)                     /*!< TXDIR (Bit 3)                                         */
#define UART_LCR1_TXDIR_Msk               (0x8U)                   /*!< TXDIR (Bitfield-Mask: 0x01)                           */
#define UART_LCR1_LOOP_Pos                (4U)                     /*!< LOOP (Bit 4)                                          */
#define UART_LCR1_LOOP_Msk                (0x10U)                  /*!< LOOP (Bitfield-Mask: 0x01)                            */
#define UART_LCR1_IREN_Pos                (5U)                     /*!< IREN (Bit 5)                                          */
#define UART_LCR1_IREN_Msk                (0x20U)                  /*!< IREN (Bitfield-Mask: 0x01)                            */
#define UART_LCR1_INVRX_Pos               (6U)                     /*!< INVRX (Bit 6)                                         */
#define UART_LCR1_INVRX_Msk               (0x40U)                  /*!< INVRX (Bitfield-Mask: 0x01)                           */
#define UART_LCR1_INVTX_Pos               (7U)                     /*!< INVTX (Bit 7)                                         */
#define UART_LCR1_INVTX_Msk               (0x80U)                  /*!< INVTX (Bitfield-Mask: 0x01)                           */
/* ==========================================================  FCR  ========================================================== */
#define UART_FCR_FIFOE_Pos                (0U)                     /*!< FIFOE (Bit 0)                                         */
#define UART_FCR_FIFOE_Msk                (0x1U)                   /*!< FIFOE (Bitfield-Mask: 0x01)                           */
/* ==========================================================  EFR  ========================================================== */
#define UART_EFR_AUTO_RTS_Pos             (6U)                     /*!< AUTO_RTS (Bit 6)                                      */
#define UART_EFR_AUTO_RTS_Msk             (0x40U)                  /*!< AUTO_RTS (Bitfield-Mask: 0x01)                        */
#define UART_EFR_AUTO_CTS_Pos             (7U)                     /*!< AUTO_CTS (Bit 7)                                      */
#define UART_EFR_AUTO_CTS_Msk             (0x80U)                  /*!< AUTO_CTS (Bitfield-Mask: 0x01)                        */
/* ==========================================================  IER  ========================================================== */
#define UART_IER_ERXNE_Pos                (0U)                     /*!< ERXNE (Bit 0)                                         */
#define UART_IER_ERXNE_Msk                (0x1U)                   /*!< ERXNE (Bitfield-Mask: 0x01)                           */
#define UART_IER_ETXNF_Pos                (1U)                     /*!< ETXNF (Bit 1)                                         */
#define UART_IER_ETXNF_Msk                (0x2U)                   /*!< ETXNF (Bitfield-Mask: 0x01)                           */
#define UART_IER_ETC_Pos                  (2U)                     /*!< ETC (Bit 2)                                           */
#define UART_IER_ETC_Msk                  (0x4U)                   /*!< ETC (Bitfield-Mask: 0x01)                             */
#define UART_IER_EPE_Pos                  (3U)                     /*!< EPE (Bit 3)                                           */
#define UART_IER_EPE_Msk                  (0x8U)                   /*!< EPE (Bitfield-Mask: 0x01)                             */
#define UART_IER_EFE_Pos                  (4U)                     /*!< EFE (Bit 4)                                           */
#define UART_IER_EFE_Msk                  (0x10U)                  /*!< EFE (Bitfield-Mask: 0x01)                             */
#define UART_IER_ENE_Pos                  (5U)                     /*!< ENE (Bit 5)                                           */
#define UART_IER_ENE_Msk                  (0x20U)                  /*!< ENE (Bitfield-Mask: 0x01)                             */
#define UART_IER_EOEBI_Pos                (6U)                     /*!< EOEBI (Bit 6)                                         */
#define UART_IER_EOEBI_Msk                (0x40U)                  /*!< EOEBI (Bitfield-Mask: 0x01)                           */
#define UART_IER_EDCTS_Pos                (7U)                     /*!< EDCTS (Bit 7)                                         */
#define UART_IER_EDCTS_Msk                (0x80U)                  /*!< EDCTS (Bitfield-Mask: 0x01)                           */
#define UART_IER_ETXDF_Pos                (8U)                     /*!< ETXDF (Bit 8)                                         */
#define UART_IER_ETXDF_Msk                (0x100U)                 /*!< ETXDF (Bitfield-Mask: 0x01)                           */
/* =========================================================  LSR0  ========================================================== */
#define UART_LSR0_DR_Pos                  (0U)                     /*!< DR (Bit 0)                                            */
#define UART_LSR0_DR_Msk                  (0x1U)                   /*!< DR (Bitfield-Mask: 0x01)                              */
#define UART_LSR0_OE_Pos                  (1U)                     /*!< OE (Bit 1)                                            */
#define UART_LSR0_OE_Msk                  (0x2U)                   /*!< OE (Bitfield-Mask: 0x01)                              */
#define UART_LSR0_PE_Pos                  (2U)                     /*!< PE (Bit 2)                                            */
#define UART_LSR0_PE_Msk                  (0x4U)                   /*!< PE (Bitfield-Mask: 0x01)                              */
#define UART_LSR0_FE_Pos                  (3U)                     /*!< FE (Bit 3)                                            */
#define UART_LSR0_FE_Msk                  (0x8U)                   /*!< FE (Bitfield-Mask: 0x01)                              */
#define UART_LSR0_BI_Pos                  (4U)                     /*!< BI (Bit 4)                                            */
#define UART_LSR0_BI_Msk                  (0x10U)                  /*!< BI (Bitfield-Mask: 0x01)                              */
#define UART_LSR0_TXNF_Pos                (5U)                     /*!< TXNF (Bit 5)                                          */
#define UART_LSR0_TXNF_Msk                (0x20U)                  /*!< TXNF (Bitfield-Mask: 0x01)                            */
#define UART_LSR0_TC_Pos                  (6U)                     /*!< TC (Bit 6)                                            */
#define UART_LSR0_TC_Msk                  (0x40U)                  /*!< TC (Bitfield-Mask: 0x01)                              */
#define UART_LSR0_NE_Pos                  (7U)                     /*!< NE (Bit 7)                                            */
#define UART_LSR0_NE_Msk                  (0x80U)                  /*!< NE (Bitfield-Mask: 0x01)                              */
#define UART_LSR0_TXDF_Pos                (8U)                     /*!< TXDF (Bit 8)                                          */
#define UART_LSR0_TXDF_Msk                (0x100U)                 /*!< TXDF (Bitfield-Mask: 0x01)                            */
/* =========================================================  LSR1  ========================================================== */
#define UART_LSR1_IDLE_Pos                (0U)                     /*!< IDLE (Bit 0)                                          */
#define UART_LSR1_IDLE_Msk                (0x1U)                   /*!< IDLE (Bitfield-Mask: 0x01)                            */
#define UART_LSR1_SYNERR_Pos              (1U)                     /*!< SYNERR (Bit 1)                                        */
#define UART_LSR1_SYNERR_Msk              (0x2U)                   /*!< SYNERR (Bitfield-Mask: 0x01)                          */
#define UART_LSR1_FBRK_Pos                (2U)                     /*!< FBRK (Bit 2)                                          */
#define UART_LSR1_FBRK_Msk                (0x4U)                   /*!< FBRK (Bitfield-Mask: 0x01)                            */
#define UART_LSR1_DCTS_Pos                (3U)                     /*!< DCTS (Bit 3)                                          */
#define UART_LSR1_DCTS_Msk                (0x8U)                   /*!< DCTS (Bitfield-Mask: 0x01)                            */
#define UART_LSR1_LINWAK_Pos              (4U)                     /*!< LINWAK (Bit 4)                                        */
#define UART_LSR1_LINWAK_Msk              (0x10U)                  /*!< LINWAK (Bitfield-Mask: 0x01)                          */
#define UART_LSR1_UART_IDLE_Pos           (5U)                     /*!< UART_IDLE (Bit 5)                                     */
#define UART_LSR1_UART_IDLE_Msk           (0x20U)                  /*!< UART_IDLE (Bitfield-Mask: 0x01)                       */
#define UART_LSR1_CTS_Pos                 (6U)                     /*!< CTS (Bit 6)                                           */
#define UART_LSR1_CTS_Msk                 (0x40U)                  /*!< CTS (Bitfield-Mask: 0x01)                             */
#define UART_LSR1_RTS_Pos                 (7U)                     /*!< RTS (Bit 7)                                           */
#define UART_LSR1_RTS_Msk                 (0x80U)                  /*!< RTS (Bitfield-Mask: 0x01)                             */
#define UART_LSR1_DTMTCH_Pos              (8U)                     /*!< DTMTCH (Bit 8)                                        */
#define UART_LSR1_DTMTCH_Msk              (0x100U)                 /*!< DTMTCH (Bitfield-Mask: 0x01)                          */
#define UART_LSR1_ADDMTCH_Pos             (9U)                     /*!< ADDMTCH (Bit 9)                                       */
#define UART_LSR1_ADDMTCH_Msk             (0x200U)                 /*!< ADDMTCH (Bitfield-Mask: 0x01)                         */
/* ========================================================  SMP_CNT  ======================================================== */
#define UART_SMP_CNT_SMP_CNT_Pos          (0U)                     /*!< SMP_CNT (Bit 0)                                       */
#define UART_SMP_CNT_SMP_CNT_Msk          (0x3U)                   /*!< SMP_CNT (Bitfield-Mask: 0x03)                         */
/* =========================================================  ADDR  ========================================================== */
#define UART_ADDR_ADDR_Pos                (0U)                     /*!< ADDR (Bit 0)                                          */
#define UART_ADDR_ADDR_Msk                (0x1ffU)                 /*!< ADDR (Bitfield-Mask: 0x1ff)                           */
/* =========================================================  DATA  ========================================================== */
#define UART_DATA_DATA_Pos                (0U)                     /*!< DATA (Bit 0)                                          */
#define UART_DATA_DATA_Msk                (0x1ffU)                 /*!< DATA (Bitfield-Mask: 0x1ff)                           */
/* =========================================================  GUARD  ========================================================= */
#define UART_GUARD_GUARD_CNT_Pos          (0U)                     /*!< GUARD_CNT (Bit 0)                                     */
#define UART_GUARD_GUARD_CNT_Msk          (0xfU)                   /*!< GUARD_CNT (Bitfield-Mask: 0x0f)                       */
#define UART_GUARD_GUARD_EN_Pos           (4U)                     /*!< GUARD_EN (Bit 4)                                      */
#define UART_GUARD_GUARD_EN_Msk           (0x10U)                  /*!< GUARD_EN (Bitfield-Mask: 0x01)                        */
/* =======================================================  SLEEP_EN  ======================================================== */
#define UART_SLEEP_EN_SLEEP_EN_Pos        (0U)                     /*!< SLEEP_EN (Bit 0)                                      */
#define UART_SLEEP_EN_SLEEP_EN_Msk        (0x1U)                   /*!< SLEEP_EN (Bitfield-Mask: 0x01)                        */
/* ========================================================  DMA_EN  ========================================================= */
#define UART_DMA_EN_RX_DMA_EN_Pos         (0U)                     /*!< RX_DMA_EN (Bit 0)                                     */
#define UART_DMA_EN_RX_DMA_EN_Msk         (0x1U)                   /*!< RX_DMA_EN (Bitfield-Mask: 0x01)                       */
#define UART_DMA_EN_TX_DMA_EN_Pos         (1U)                     /*!< TX_DMA_EN (Bit 1)                                     */
#define UART_DMA_EN_TX_DMA_EN_Msk         (0x2U)                   /*!< TX_DMA_EN (Bitfield-Mask: 0x01)                       */
/* =======================================================  DIV_FRAC  ======================================================== */
#define UART_DIV_FRAC_DIV_FRAC_Pos        (0U)                     /*!< DIV_FRAC (Bit 0)                                      */
#define UART_DIV_FRAC_DIV_FRAC_Msk        (0x1fU)                  /*!< DIV_FRAC (Bitfield-Mask: 0x1f)                        */
/* ========================================================  MTCHCR  ========================================================= */
#define UART_MTCHCR_DTMTCH_EN_Pos         (0U)                     /*!< DTMTCH_EN (Bit 0)                                     */
#define UART_MTCHCR_DTMTCH_EN_Msk         (0x1U)                   /*!< DTMTCH_EN (Bitfield-Mask: 0x01)                       */
#define UART_MTCHCR_ADDMTCH_EN_Pos        (1U)                     /*!< ADDMTCH_EN (Bit 1)                                    */
#define UART_MTCHCR_ADDMTCH_EN_Msk        (0x2U)                   /*!< ADDMTCH_EN (Bitfield-Mask: 0x01)                      */
#define UART_MTCHCR_MTCHIE_Pos            (2U)                     /*!< MTCHIE (Bit 2)                                        */
#define UART_MTCHCR_MTCHIE_Msk            (0x4U)                   /*!< MTCHIE (Bitfield-Mask: 0x01)                          */
/* ========================================================  RS485CR  ======================================================== */
#define UART_RS485CR_DLYEN_Pos            (4U)                     /*!< DLYEN (Bit 4)                                         */
#define UART_RS485CR_DLYEN_Msk            (0x10U)                  /*!< DLYEN (Bitfield-Mask: 0x01)                           */
#define UART_RS485CR_INVPOL_Pos           (5U)                     /*!< INVPOL (Bit 5)                                        */
#define UART_RS485CR_INVPOL_Msk           (0x20U)                  /*!< INVPOL (Bitfield-Mask: 0x01)                          */
#define UART_RS485CR_RS485EN_Pos          (7U)                     /*!< RS485EN (Bit 7)                                       */
#define UART_RS485CR_RS485EN_Msk          (0x80U)                  /*!< RS485EN (Bitfield-Mask: 0x01)                         */
/* =========================================================  CNTR  ========================================================== */
#define UART_CNTR_CNTR_Pos                (0U)                     /*!< CNTR (Bit 0)                                          */
#define UART_CNTR_CNTR_Msk                (0xffU)                  /*!< CNTR (Bitfield-Mask: 0xff)                            */
/* =========================================================  IDLE  ========================================================== */
#define UART_IDLE_IDLEIE_Pos              (4U)                     /*!< IDLEIE (Bit 4)                                        */
#define UART_IDLE_IDLEIE_Msk              (0x10U)                  /*!< IDLEIE (Bitfield-Mask: 0x01)                          */
#define UART_IDLE_IDLEEN_Pos              (7U)                     /*!< IDLEEN (Bit 7)                                        */
#define UART_IDLE_IDLEEN_Msk              (0x80U)                  /*!< IDLEEN (Bitfield-Mask: 0x01)                          */
/* =========================================================  LINCR  ========================================================= */
#define UART_LINCR_LINSLP_Pos             (0U)                     /*!< LINSLP Bit 0)                                         */
#define UART_LINCR_LINSLP_Msk             (0x1U)                   /*!< LINSLP (Bitfield-Mask: 0x01)                          */
#define UART_LINCR_LINWAKIE_Pos           (1U)                     /*!< LINWAKIE (Bit 1)                                      */
#define UART_LINCR_LINWAKIE_Msk           (0x2U)                   /*!< LINWAKIE (Bitfield-Mask: 0x01)                        */
#define UART_LINCR_SYNERRIE_Pos           (2U)                     /*!< SYNERRIE (Bit 2)                                      */
#define UART_LINCR_SYNERRIE_Msk           (0x4U)                   /*!< SYNERRIE (Bitfield-Mask: 0x01)                        */
#define UART_LINCR_LABAUDEN_Pos           (3U)                     /*!< LABAUDEN (Bit 3)                                      */
#define UART_LINCR_LABAUDEN_Msk           (0x8U)                   /*!< LABAUDEN (Bitfield-Mask: 0x01)                        */
#define UART_LINCR_SDBRK_Pos              (4U)                     /*!< SDBRK (Bit 4)                                         */
#define UART_LINCR_SDBRK_Msk              (0x10U)                  /*!< SDBRK (Bitfield-Mask: 0x01)                           */
#define UART_LINCR_LBRKDL_Pos             (5U)                     /*!< LBRKDL (Bit 5)                                        */
#define UART_LINCR_LBRKDL_Msk             (0x20U)                  /*!< LBRKDL (Bitfield-Mask: 0x01)                          */
#define UART_LINCR_LBRKIE_Pos             (6U)                     /*!< LBRKIE (Bit 6)                                        */
#define UART_LINCR_LBRKIE_Msk             (0x40U)                  /*!< LBRKIE (Bitfield-Mask: 0x01)                          */
#define UART_LINCR_LINEN_Pos              (7U)                     /*!< LINEN (Bit 7)                                         */
#define UART_LINCR_LINEN_Msk              (0x80U)                  /*!< LINEN (Bitfield-Mask: 0x01)                           */
/* ========================================================  BRKLGH  ========================================================= */
#define UART_BRKLGH_BRKLGH_Pos            (0U)                     /*!< BRKLGH (Bit 0)                                        */
#define UART_BRKLGH_BRKLGH_Msk            (0xfU)                   /*!< BRKLGH (Bitfield-Mask: 0x0f)                          */
/* =========================================================  PMIN0  ========================================================= */
#define UART_PMIN0_PMIN0_Pos              (0U)                     /*!< PMIN0 (Bit 0)                                         */
#define UART_PMIN0_PMIN0_Msk              (0xffU)                  /*!< PMIN0 (Bitfield-Mask: 0xff)                           */
/* =========================================================  PMIN1  ========================================================= */
#define UART_PMIN1_PMIN1_Pos              (0U)                     /*!< PMIN1 (Bit 0)                                         */
#define UART_PMIN1_PMIN1_Msk              (0xffU)                  /*!< PMIN1 (Bitfield-Mask: 0xff)                           */
/* =========================================================  PWDTH  ========================================================= */
#define UART_PWDTH_PWDTH_Pos              (0U)                     /*!< PWDTH (Bit 0)                                         */
#define UART_PWDTH_PWDTH_Msk              (0x7U)                   /*!< PWDTH (Bitfield-Mask: 0x07)                           */
/* =========================================================  IDLELGH  ======================================================= */
#define UART_IDLELGH_IDLELGH_Pos          (0U)                     /*!< PWDTH (Bit 0)                                         */
#define UART_IDLELGH_IDLELGH_Msk          (0xffU)                  /*!< PWDTH (Bitfield-Mask: 0xff)                           */

/* =========================================================================================================================== */
/* ================                                           I2C                                             ================ */
/* =========================================================================================================================== */

/* =========================================================  ADDR0  ========================================================= */
#define I2C_ADDR0_AD_Pos                  (1U)                     /*!< AD (Bit 1)                                            */
#define I2C_ADDR0_AD_Msk                  (0xfeU)                  /*!< AD (Bitfield-Mask: 0x7f)                              */
/* =========================================================  ADDR1  ========================================================= */
#define I2C_ADDR1_AD_Pos                  (0U)                     /*!< AD (Bit 0)                                            */
#define I2C_ADDR1_AD_Msk                  (0x7U)                   /*!< AD (Bitfield-Mask: 0x07)                              */
#define I2C_ADDR1_RAD_Pos                 (4U)                     /*!< RAD (Bit 4)                                           */
#define I2C_ADDR1_RAD_Msk                 (0x7f0U)                 /*!< RAD (Bitfield-Mask: 0x7f)                             */
#define I2C_ADDR1_RMEN_Pos                (12U)                    /*!< RMEN (Bit 12)                                         */
#define I2C_ADDR1_RMEN_Msk                (0x1000U)                /*!< RMEN (Bitfield-Mask: 0x01)                            */
/* ======================================================  SAMPLE_CNT  ======================================================= */
#define I2C_SAMPLE_CNT_DIV_Pos            (0U)                     /*!< DIV (Bit 0)                                           */
#define I2C_SAMPLE_CNT_DIV_Msk            (0xffU)                  /*!< DIV (Bitfield-Mask: 0xff)                             */
/* =======================================================  STEP_CNT  ======================================================== */
#define I2C_STEP_CNT_DIV_Pos              (0U)                     /*!< DIV (Bit 0)                                           */
#define I2C_STEP_CNT_DIV_Msk              (0xffU)                  /*!< DIV (Bitfield-Mask: 0xff)                             */
/* =========================================================  CTRL0  ========================================================= */
#define I2C_CTRL0_SRST_Pos                (0U)                     /*!< SRST (Bit 0)                                          */
#define I2C_CTRL0_SRST_Msk                (0x1U)                   /*!< SRST (Bitfield-Mask: 0x01)                            */
#define I2C_CTRL0_DBGEN_Pos               (1U)                     /*!< DBGEN (Bit 1)                                         */
#define I2C_CTRL0_DBGEN_Msk               (0x2U)                   /*!< DBGEN (Bitfield-Mask: 0x01)                           */
#define I2C_CTRL0_TACK_Pos                (3U)                     /*!< TACK (Bit 3)                                          */
#define I2C_CTRL0_TACK_Msk                (0x8U)                   /*!< TACK (Bitfield-Mask: 0x01)                            */
#define I2C_CTRL0_TX_Pos                  (4U)                     /*!< TX (Bit 4)                                            */
#define I2C_CTRL0_TX_Msk                  (0x10U)                  /*!< TX (Bitfield-Mask: 0x01)                              */
#define I2C_CTRL0_MSTR_Pos                (5U)                     /*!< MSTR (Bit 5)                                          */
#define I2C_CTRL0_MSTR_Msk                (0x20U)                  /*!< MSTR (Bitfield-Mask: 0x01)                            */
#define I2C_CTRL0_IICIE_Pos               (6U)                     /*!< IICIE (Bit 6)                                         */
#define I2C_CTRL0_IICIE_Msk               (0x40U)                  /*!< IICIE (Bitfield-Mask: 0x01)                           */
#define I2C_CTRL0_IICEN_Pos               (7U)                     /*!< IICEN (Bit 7)                                         */
#define I2C_CTRL0_IICEN_Msk               (0x80U)                  /*!< IICEN (Bitfield-Mask: 0x01)                           */
/* =========================================================  CTRL1  ========================================================= */
#define I2C_CTRL1_STREN_Pos               (0U)                     /*!< STREN (Bit 0)                                         */
#define I2C_CTRL1_STREN_Msk               (0x1U)                   /*!< STREN (Bitfield-Mask: 0x01)                           */
#define I2C_CTRL1_SAEN_Pos                (1U)                     /*!< SAEN (Bit 1)                                          */
#define I2C_CTRL1_SAEN_Msk                (0x2U)                   /*!< SAEN (Bitfield-Mask: 0x01)                            */
#define I2C_CTRL1_ARBEN_Pos               (3U)                     /*!< ARBEN (Bit 3)                                         */
#define I2C_CTRL1_ARBEN_Msk               (0x8U)                   /*!< ARBEN (Bitfield-Mask: 0x01)                           */
#define I2C_CTRL1_SYNCEN_Pos              (4U)                     /*!< SYNCEN (Bit 4)                                        */
#define I2C_CTRL1_SYNCEN_Msk              (0x10U)                  /*!< SYNCEN (Bitfield-Mask: 0x01)                          */
#define I2C_CTRL1_ADEXT_Pos               (6U)                     /*!< ADEXT (Bit 6)                                         */
#define I2C_CTRL1_ADEXT_Msk               (0x40U)                  /*!< ADEXT (Bitfield-Mask: 0x01)                           */
#define I2C_CTRL1_GCAEN_Pos               (7U)                     /*!< GCAEN (Bit 7)                                         */
#define I2C_CTRL1_GCAEN_Msk               (0x80U)                  /*!< GCAEN (Bitfield-Mask: 0x01)                           */
/* =========================================================  CTRL2  ========================================================= */
#define I2C_CTRL2_MNTEN_Pos               (0U)                     /*!< MNTEN (Bit 0)                                         */
#define I2C_CTRL2_MNTEN_Msk               (0x1U)                   /*!< MNTEN (Bitfield-Mask: 0x01)                           */
#define I2C_CTRL2_NACKIE_Pos              (1U)                     /*!< NACKIE (Bit 1)                                        */
#define I2C_CTRL2_NACKIE_Msk              (0x2U)                   /*!< NACKIE (Bitfield-Mask: 0x01)                          */
#define I2C_CTRL2_PLTIE_Pos               (2U)                     /*!< PLTIE (Bit 2)                                         */
#define I2C_CTRL2_PLTIE_Msk               (0x4U)                   /*!< PLTIE (Bitfield-Mask: 0x01)                           */
#define I2C_CTRL2_BND_DMA_IE_Pos          (3U)                     /*!< BND_DMA_IE (Bit 3)                                    */
#define I2C_CTRL2_BND_DMA_IE_Msk          (0x8U)                   /*!< BND_DMA_IE (Bitfield-Mask: 0x01)                      */
#define I2C_CTRL2_TXEIE_Pos               (4U)                     /*!< TXEIE (Bit 4)                                         */
#define I2C_CTRL2_TXEIE_Msk               (0x10U)                  /*!< TXEIE (Bitfield-Mask: 0x01)                           */
#define I2C_CTRL2_RXFIE_Pos               (5U)                     /*!< RXFIE (Bit 5)                                         */
#define I2C_CTRL2_RXFIE_Msk               (0x20U)                  /*!< RXFIE (Bitfield-Mask: 0x01)                           */
#define I2C_CTRL2_TXUFIE_Pos              (6U)                     /*!< TXUFIE (Bit 6)                                        */
#define I2C_CTRL2_TXUFIE_Msk              (0x40U)                  /*!< TXUFIE (Bitfield-Mask: 0x01)                          */
#define I2C_CTRL2_RXOFIE_Pos              (7U)                     /*!< RXOFIE (Bit 7)                                        */
#define I2C_CTRL2_RXOFIE_Msk              (0x80U)                  /*!< RXOFIE (Bitfield-Mask: 0x01)                          */
/* =========================================================  CTRL3  ========================================================= */
#define I2C_CTRL3_DMATXEN_Pos             (0U)                     /*!< DMATXEN (Bit 0)                                       */
#define I2C_CTRL3_DMATXEN_Msk             (0x1U)                   /*!< DMATXEN (Bitfield-Mask: 0x01)                         */
#define I2C_CTRL3_DMARXEN_Pos             (1U)                     /*!< DMARXEN (Bit 1)                                       */
#define I2C_CTRL3_DMARXEN_Msk             (0x2U)                   /*!< DMARXEN (Bitfield-Mask: 0x01)                         */
#define I2C_CTRL3_TIMECFG_Pos             (2U)                     /*!< TIMECFG (Bit 2)                                       */
#define I2C_CTRL3_TIMECFG_Msk             (0x4U)                   /*!< TIMECFG (Bitfield-Mask: 0x01)                         */
#define I2C_CTRL3_PINLOW_Pos              (3U)                     /*!< PINLOW (Bit 3)                                        */
#define I2C_CTRL3_PINLOW_Msk              (0xfff8U)                /*!< PINLOW (Bitfield-Mask: 0x1fff)                        */
/* ========================================================  STATUS0  ======================================================== */
#define I2C_STATUS0_RACK_Pos              (0U)                     /*!< RACK (Bit 0)                                          */
#define I2C_STATUS0_RACK_Msk              (0x1U)                   /*!< RACK (Bitfield-Mask: 0x01)                            */
#define I2C_STATUS0_A10MF_Pos             (1U)                     /*!< A10MF (Bit 1)                                         */
#define I2C_STATUS0_A10MF_Msk             (0x2U)                   /*!< A10MF (Bitfield-Mask: 0x01)                           */
#define I2C_STATUS0_SRW_Pos               (2U)                     /*!< SRW (Bit 2)                                           */
#define I2C_STATUS0_SRW_Msk               (0x4U)                   /*!< SRW (Bitfield-Mask: 0x01)                             */
#define I2C_STATUS0_READY_Pos             (3U)                     /*!< READY (Bit 3)                                         */
#define I2C_STATUS0_READY_Msk             (0x8U)                   /*!< READY (Bitfield-Mask: 0x01)                           */
#define I2C_STATUS0_ARBLOST_Pos           (4U)                     /*!< ARBLOST (Bit 4)                                       */
#define I2C_STATUS0_ARBLOST_Msk           (0x10U)                  /*!< ARBLOST (Bitfield-Mask: 0x01)                         */
#define I2C_STATUS0_BUSY_Pos              (5U)                     /*!< BUSY (Bit 5)                                          */
#define I2C_STATUS0_BUSY_Msk              (0x20U)                  /*!< BUSY (Bitfield-Mask: 0x01)                            */
#define I2C_STATUS0_SAMF_Pos              (6U)                     /*!< SAMF (Bit 6)                                          */
#define I2C_STATUS0_SAMF_Msk              (0x40U)                  /*!< SAMF (Bitfield-Mask: 0x01)                            */
#define I2C_STATUS0_BND_Pos               (7U)                     /*!< BND (Bit 7)                                           */
#define I2C_STATUS0_BND_Msk               (0x80U)                  /*!< BND (Bitfield-Mask: 0x01)                             */
/* ========================================================  STATUS1  ======================================================== */
#define I2C_STATUS1_TXEF_Pos              (0U)                     /*!< TXEF (Bit 0)                                          */
#define I2C_STATUS1_TXEF_Msk              (0x1U)                   /*!< TXEF (Bitfield-Mask: 0x01)                            */
#define I2C_STATUS1_RXFF_Pos              (1U)                     /*!< RXFF (Bit 1)                                          */
#define I2C_STATUS1_RXFF_Msk              (0x2U)                   /*!< RXFF (Bitfield-Mask: 0x01)                            */
#define I2C_STATUS1_TXUF_Pos              (2U)                     /*!< TXUF (Bit 2)                                          */
#define I2C_STATUS1_TXUF_Msk              (0x4U)                   /*!< TXUF (Bitfield-Mask: 0x01)                            */
#define I2C_STATUS1_RXOF_Pos              (3U)                     /*!< RXOF (Bit 3)                                          */
#define I2C_STATUS1_RXOF_Msk              (0x8U)                   /*!< RXOF (Bitfield-Mask: 0x01)                            */
#define I2C_STATUS1_GCMF_Pos              (4U)                     /*!< GCMF (Bit 4)                                          */
#define I2C_STATUS1_GCMF_Msk              (0x10U)                  /*!< GCMF (Bitfield-Mask: 0x01)                            */
#define I2C_STATUS1_PLTF_Pos              (5U)                     /*!< PLTF (Bit 5)                                          */
#define I2C_STATUS1_PLTF_Msk              (0x20U)                  /*!< PLTF (Bitfield-Mask: 0x01)                            */
#define I2C_STATUS1_SARF_Pos              (6U)                     /*!< SARF (Bit 6)                                          */
#define I2C_STATUS1_SARF_Msk              (0x40U)                  /*!< SARF (Bitfield-Mask: 0x01)                            */
/* ========================================================  DGLCFG  ========================================================= */
#define I2C_DGLCFG_DGL_CNT_Pos            (0U)                     /*!< DGL_CNT (Bit 0)                                       */
#define I2C_DGLCFG_DGL_CNT_Msk            (0xfU)                   /*!< DGL_CNT (Bitfield-Mask: 0x0f)                         */
#define I2C_DGLCFG_STARTF_Pos             (4U)                     /*!< STARTF (Bit 4)                                        */
#define I2C_DGLCFG_STARTF_Msk             (0x10U)                  /*!< STARTF (Bitfield-Mask: 0x01)                          */
#define I2C_DGLCFG_SSIE_Pos               (5U)                     /*!< SSIE (Bit 5)                                          */
#define I2C_DGLCFG_SSIE_Msk               (0x20U)                  /*!< SSIE (Bitfield-Mask: 0x01)                            */
#define I2C_DGLCFG_STOPF_Pos              (6U)                     /*!< STOPF (Bit 6)                                         */
#define I2C_DGLCFG_STOPF_Msk              (0x40U)                  /*!< STOPF (Bitfield-Mask: 0x01)                           */
#define I2C_DGLCFG_DGLEN_Pos              (7U)                     /*!< DGLEN (Bit 7)                                         */
#define I2C_DGLCFG_DGLEN_Msk              (0x80U)                  /*!< DGLEN (Bitfield-Mask: 0x01)                           */
/* =========================================================  DATA  ========================================================== */
#define I2C_DATA_DATA_Pos                 (0U)                     /*!< DATA (Bit 0)                                          */
#define I2C_DATA_DATA_Msk                 (0xffU)                  /*!< DATA (Bitfield-Mask: 0xff)                            */
#define I2C_DATA_MAK_Pos                  (8U)                     /*!< MAK (Bit 8)                                           */
#define I2C_DATA_MAK_Msk                  (0x100U)                 /*!< MAK (Bitfield-Mask: 0x01)                             */
/* =======================================================  STARTSTOP  ======================================================= */
#define I2C_STARTSTOP_START_Pos           (0U)                     /*!< START (Bit 0)                                         */
#define I2C_STARTSTOP_START_Msk           (0x1U)                   /*!< START (Bitfield-Mask: 0x01)                           */
#define I2C_STARTSTOP_STOP_Pos            (1U)                     /*!< STOP (Bit 1)                                          */
#define I2C_STARTSTOP_STOP_Msk            (0x2U)                   /*!< STOP (Bitfield-Mask: 0x01)                            */

/* =========================================================================================================================== */
/* ================                                           SPI                                             ================ */
/* =========================================================================================================================== */

/* =========================================================  CFG0  ========================================================== */
#define SPI_CFG0_SCK_HIGH_Pos             (0U)                     /*!< SCK_HIGH (Bit 0)                                      */
#define SPI_CFG0_SCK_HIGH_Msk             (0xffU)                  /*!< SCK_HIGH (Bitfield-Mask: 0xff)                        */
#define SPI_CFG0_SCK_LOW_Pos              (8U)                     /*!< SCK_LOW (Bit 8)                                       */
#define SPI_CFG0_SCK_LOW_Msk              (0xff00U)                /*!< SCK_LOW (Bitfield-Mask: 0xff)                         */
#define SPI_CFG0_CS_HOLD_Pos              (16U)                    /*!< CS_HOLD (Bit 16)                                      */
#define SPI_CFG0_CS_HOLD_Msk              (0xff0000U)              /*!< CS_HOLD (Bitfield-Mask: 0xff)                         */
#define SPI_CFG0_CS_SETUP_Pos             (24U)                    /*!< CS_SETUP (Bit 24)                                     */
#define SPI_CFG0_CS_SETUP_Msk             (0xff000000U)            /*!< CS_SETUP (Bitfield-Mask: 0xff)                        */
/* =========================================================  CFG1  ========================================================== */
#define SPI_CFG1_CS_IDLE_Pos              (0U)                     /*!< CS_IDLE (Bit 0)                                       */
#define SPI_CFG1_CS_IDLE_Msk              (0xffU)                  /*!< CS_IDLE (Bitfield-Mask: 0xff)                         */
#define SPI_CFG1_TXNFIE_Pos               (8U)                     /*!< TXNFIE (Bit 8)                                        */
#define SPI_CFG1_TXNFIE_Msk               (0x100U)                 /*!< TXNFIE (Bitfield-Mask: 0x01)                          */
#define SPI_CFG1_RXNEIE_Pos               (9U)                     /*!< RXNEIE (Bit 9)                                        */
#define SPI_CFG1_RXNEIE_Msk               (0x200U)                 /*!< RXNEIE (Bitfield-Mask: 0x01)                          */
#define SPI_CFG1_TXUIE_Pos                (10U)                    /*!< TXUIE (Bit 10)                                        */
#define SPI_CFG1_TXUIE_Msk                (0x400U)                 /*!< TXUIE (Bitfield-Mask: 0x01)                           */
#define SPI_CFG1_RXOIE_Pos                (11U)                    /*!< RXOIE (Bit 11)                                        */
#define SPI_CFG1_RXOIE_Msk                (0x800U)                 /*!< RXOIE (Bitfield-Mask: 0x01)                           */
#define SPI_CFG1_MSTR_Pos                 (12U)                    /*!< MSTR (Bit 12)                                         */
#define SPI_CFG1_MSTR_Msk                 (0x1000U)                /*!< MSTR (Bitfield-Mask: 0x01)                            */
#define SPI_CFG1_MODFIE_Pos               (13U)                    /*!< MODFIE (Bit 13)                                       */
#define SPI_CFG1_MODFIE_Msk               (0x2000U)                /*!< MODFIE (Bitfield-Mask: 0x01)                          */
#define SPI_CFG1_DMATXEN_Pos              (14U)                    /*!< DMATXEN (Bit 14)                                      */
#define SPI_CFG1_DMATXEN_Msk              (0x4000U)                /*!< DMATXEN (Bitfield-Mask: 0x01)                         */
#define SPI_CFG1_DMARXEN_Pos              (15U)                    /*!< DMARXEN (Bit 15)                                      */
#define SPI_CFG1_DMARXEN_Msk              (0x8000U)                /*!< DMARXEN (Bitfield-Mask: 0x01)                         */
#define SPI_CFG1_CPOL_Pos                 (16U)                    /*!< CPOL (Bit 16)                                         */
#define SPI_CFG1_CPOL_Msk                 (0x10000U)               /*!< CPOL (Bitfield-Mask: 0x01)                            */
#define SPI_CFG1_CPHA_Pos                 (17U)                    /*!< CPHA (Bit 17)                                         */
#define SPI_CFG1_CPHA_Msk                 (0x20000U)               /*!< CPHA (Bitfield-Mask: 0x01)                            */
#define SPI_CFG1_TMSBF_Pos                (18U)                    /*!< TMSBF (Bit 18)                                        */
#define SPI_CFG1_TMSBF_Msk                (0x40000U)               /*!< TMSBF (Bitfield-Mask: 0x01)                           */
#define SPI_CFG1_RMSBF_Pos                (19U)                    /*!< RMSBF (Bit 19)                                        */
#define SPI_CFG1_RMSBF_Msk                (0x80000U)               /*!< RMSBF (Bitfield-Mask: 0x01)                           */
#define SPI_CFG1_FRMSIZE_Pos              (20U)                    /*!< FRMSIZE (Bit 20)                                      */
#define SPI_CFG1_FRMSIZE_Msk              (0x1f00000U)             /*!< FRMSIZE (Bitfield-Mask: 0x1f)                         */
#define SPI_CFG1_CSOE_Pos                 (25U)                    /*!< CSOE (Bit 25)                                         */
#define SPI_CFG1_CSOE_Msk                 (0x2000000U)             /*!< CSOE (Bitfield-Mask: 0x01)                            */
#define SPI_CFG1_MODFEN_Pos               (26U)                    /*!< MODFEN (Bit 26)                                       */
#define SPI_CFG1_MODFEN_Msk               (0x4000000U)             /*!< MODFEN (Bitfield-Mask: 0x01)                          */
#define SPI_CFG1_PINCFG_Pos               (27U)                    /*!< PINCFG (Bit 27)                                       */
#define SPI_CFG1_PINCFG_Msk               (0x8000000U)             /*!< PINCFG (Bitfield-Mask: 0x01)                          */
#define SPI_CFG1_CONT_CS_Pos              (28U)                    /*!< CONT_CS (Bit 28)                                      */
#define SPI_CFG1_CONT_CS_Msk              (0x10000000U)            /*!< CONT_CS (Bitfield-Mask: 0x01)                         */
#define SPI_CFG1_TFEIE_Pos                (29U)                    /*!< TFEIE (Bit 29)                                        */
#define SPI_CFG1_TFEIE_Msk                (0x20000000U)            /*!< TFEIE (Bitfield-Mask: 0x01)                           */
#define SPI_CFG1_RFEIE_Pos                (30U)                    /*!< RFEIE (Bit 30)                                        */
#define SPI_CFG1_RFEIE_Msk                (0x40000000U)            /*!< RFEIE (Bitfield-Mask: 0x01)                           */
#define SPI_CFG1_DMIE_Pos                 (31U)                    /*!< DMIE (Bit 31)                                         */
#define SPI_CFG1_DMIE_Msk                 (0x80000000U)            /*!< DMIE (Bitfield-Mask: 0x01)                            */
/* ==========================================================  CMD  ========================================================== */
#define SPI_CMD_SPIEN_Pos                 (0U)                     /*!< SPIEN (Bit 0)                                         */
#define SPI_CMD_SPIEN_Msk                 (0x1U)                   /*!< SPIEN (Bitfield-Mask: 0x01)                           */
#define SPI_CMD_TXCLR_Pos                 (1U)                     /*!< TXCLR (Bit 1)                                         */
#define SPI_CMD_TXCLR_Msk                 (0x2U)                   /*!< TXCLR (Bitfield-Mask: 0x01)                           */
#define SPI_CMD_RXCLR_Pos                 (2U)                     /*!< RXCLR (Bit 2)                                         */
#define SPI_CMD_RXCLR_Msk                 (0x4U)                   /*!< RXCLR (Bitfield-Mask: 0x01)                           */
#define SPI_CMD_SWRST_Pos                 (4U)                     /*!< SWRST (Bit 4)                                         */
#define SPI_CMD_SWRST_Msk                 (0x10U)                  /*!< SWRST (Bitfield-Mask: 0x01)                           */
#define SPI_CMD_CSRLS_Pos                 (5U)                     /*!< CSRLS (Bit 5)                                         */
#define SPI_CMD_CSRLS_Msk                 (0x20U)                  /*!< CSRLS (Bitfield-Mask: 0x01)                           */
#define SPI_CMD_ROTRIG_Pos                (6U)                     /*!< ROTRIG (Bit 6)                                        */
#define SPI_CMD_ROTRIG_Msk                (0x40U)                  /*!< ROTRIG (Bitfield-Mask: 0x01)                          */
#define SPI_CMD_TDOIE_Pos                 (7U)                     /*!< ROTRIG (Bit 8)                                        */
#define SPI_CMD_TDOIE_Msk                 (0x80U)                  /*!< ROTRIG (Bitfield-Mask: 0x01)                          */
#define SPI_CMD_ROCNT_Pos                 (16U)                    /*!< ROCNT (Bit 16)                                        */
#define SPI_CMD_ROCNT_Msk                 (0xffff0000U)            /*!< ROCNT (Bitfield-Mask: 0xffff)                         */
/* ========================================================  STATUS  ========================================================= */
#define SPI_STATUS_TXNFF_Pos              (0U)                     /*!< TXNFF (Bit 0)                                         */
#define SPI_STATUS_TXNFF_Msk              (0x1U)                   /*!< TXNFF (Bitfield-Mask: 0x01)                           */
#define SPI_STATUS_RXNEF_Pos              (1U)                     /*!< RXNEF (Bit 1)                                         */
#define SPI_STATUS_RXNEF_Msk              (0x2U)                   /*!< RXNEF (Bitfield-Mask: 0x01)                           */
#define SPI_STATUS_TXUF_Pos               (2U)                     /*!< TXUF (Bit 2)                                          */
#define SPI_STATUS_TXUF_Msk               (0x4U)                   /*!< TXUF (Bitfield-Mask: 0x01)                            */
#define SPI_STATUS_RXOF_Pos               (3U)                     /*!< RXOF (Bit 3)                                          */
#define SPI_STATUS_RXOF_Msk               (0x8U)                   /*!< RXOF (Bitfield-Mask: 0x01)                            */
#define SPI_STATUS_MODEF_Pos              (4U)                     /*!< MODEF (Bit 4)                                         */
#define SPI_STATUS_MODEF_Msk              (0x10U)                  /*!< MODEF (Bitfield-Mask: 0x01)                           */
#define SPI_STATUS_RDMF_Pos               (5U)                     /*!< RDMF (Bit 5)                                          */
#define SPI_STATUS_RDMF_Msk               (0x20U)                  /*!< RDMF (Bitfield-Mask: 0x01)                            */
#define SPI_STATUS_MEBY_Pos               (7U)                     /*!< MEBY (Bit 7)                                          */
#define SPI_STATUS_MEBY_Msk               (0x80U)                  /*!< MEBY (Bitfield-Mask: 0x01)                            */
#define SPI_STATUS_IDLEF_Pos              (8U)                     /*!< IDLEF (Bit 8)                                         */
#define SPI_STATUS_IDLEF_Msk              (0x100U)                 /*!< IDLEF (Bitfield-Mask: 0x01)                           */
#define SPI_STATUS_TFEMP_Pos              (9U)                     /*!< TFEMP (Bit 9)                                         */
#define SPI_STATUS_TFEMP_Msk              (0x200U)                 /*!< TFEMP (Bitfield-Mask: 0x01)                           */
#define SPI_STATUS_RFEMP_Pos              (10U)                    /*!< RFEMP (Bit 10)                                        */
#define SPI_STATUS_RFEMP_Msk              (0x400U)                 /*!< RFEMP (Bitfield-Mask: 0x01)                           */
#define SPI_STATUS_TXFIFOLEVEL_Pos        (12U)                    /*!< TXFIFOLEVEL (Bit 12)                                  */
#define SPI_STATUS_TXFIFOLEVEL_Msk        (0xf000U)                /*!< TXFIFOLEVEL (Bitfield-Mask: 0x0f)                     */
#define SPI_STATUS_TFFUL_Pos              (17U)                    /*!< TFFUL (Bit 17)                                        */
#define SPI_STATUS_TFFUL_Msk              (0x20000U)               /*!< TFFUL (Bitfield-Mask: 0x01)                           */
#define SPI_STATUS_RFFUL_Pos              (18U)                    /*!< RFFUL (Bit 18)                                        */
#define SPI_STATUS_RFFUL_Msk              (0x40000U)               /*!< RFFUL (Bitfield-Mask: 0x01)                           */
#define SPI_STATUS_RXFIFOLEVEL_Pos        (20U)                    /*!< RXFIFOLEVEL (Bit 20)                                  */
#define SPI_STATUS_RXFIFOLEVEL_Msk        (0xf00000U)              /*!< RXFIFOLEVEL (Bitfield-Mask: 0x0f)                     */
#define SPI_STATUS_TDOF_Pos               (24U)                    /*!< TDOF (Bit 24)                                        */
#define SPI_STATUS_TDOF_Msk               (0x1000000U)             /*!< TDOF (Bitfield-Mask: 0x01)                           */
/* =========================================================  DATA  ========================================================== */
#define SPI_DATA_DATA_Pos                 (0U)                     /*!< DATA (Bit 0)                                          */
#define SPI_DATA_DATA_Msk                 (0xffffffffU)            /*!< DATA (Bitfield-Mask: 0xffffffff)                      */
/* =========================================================  CFG2  ========================================================== */
#define SPI_CFG2_MNOV_Pos                 (1U)                     /*!< MNOV (Bit 1)                                          */
#define SPI_CFG2_MNOV_Msk                 (0x2U)                   /*!< MNOV (Bitfield-Mask: 0x01)                            */
#define SPI_CFG2_TOEN_Pos                 (2U)                     /*!< TOEN (Bit 2)                                          */
#define SPI_CFG2_TOEN_Msk                 (0x4U)                   /*!< TOEN (Bitfield-Mask: 0x01)                            */
#define SPI_CFG2_ROEN_Pos                 (3U)                     /*!< ROEN (Bit 3)                                          */
#define SPI_CFG2_ROEN_Msk                 (0x8U)                   /*!< ROEN (Bitfield-Mask: 0x01)                            */
#define SPI_CFG2_WIDTH_Pos                (4U)                     /*!< WIDTH (Bit 4)                                         */
#define SPI_CFG2_WIDTH_Msk                (0x30U)                  /*!< WIDTH (Bitfield-Mask: 0x03)                           */
#define SPI_CFG2_HRPOL_Pos                (6U)                     /*!< HRPOL (Bit 6)                                         */
#define SPI_CFG2_HRPOL_Msk                (0x40U)                  /*!< HRPOL (Bitfield-Mask: 0x01)                           */
#define SPI_CFG2_HREN_Pos                 (7U)                     /*!< HREN (Bit 7)                                          */
#define SPI_CFG2_HREN_Msk                 (0x80U)                  /*!< HREN (Bitfield-Mask: 0x01)                            */
#define SPI_CFG2_PCSCFG_Pos               (8U)                     /*!< PCSCFG (Bit 8)                                        */
#define SPI_CFG2_PCSCFG_Msk               (0x300U)                 /*!< PCSCFG (Bitfield-Mask: 0x03)                          */
#define SPI_CFG2_PCS0POL_Pos              (10U)                    /*!< PCS0POL (Bit 10)                                      */
#define SPI_CFG2_PCS0POL_Msk              (0x400U)                 /*!< PCS0POL (Bitfield-Mask: 0x01)                         */
#define SPI_CFG2_PCS1POL_Pos              (11U)                    /*!< PCS1POL (Bit 11)                                      */
#define SPI_CFG2_PCS1POL_Msk              (0x800U)                 /*!< PCS1POL (Bitfield-Mask: 0x01)                         */
#define SPI_CFG2_PCS2POL_Pos              (12U)                    /*!< PCS2POL (Bit 12)                                      */
#define SPI_CFG2_PCS2POL_Msk              (0x1000U)                /*!< PCS2POL (Bitfield-Mask: 0x01)                         */
#define SPI_CFG2_PCS3POL_Pos              (13U)                    /*!< PCS3POL (Bit 13)                                      */
#define SPI_CFG2_PCS3POL_Msk              (0x2000U)                /*!< PCS3POL (Bitfield-Mask: 0x01)                         */
#define SPI_CFG2_DATAME_Pos               (14U)                    /*!< DATAME (Bit 14)                                       */
#define SPI_CFG2_DATAME_Msk               (0x4000U)                /*!< DATAME (Bitfield-Mask: 0x01)                          */
#define SPI_CFG2_DBGEN_Pos                (16U)                    /*!< DBGEN (Bit 16)                                        */
#define SPI_CFG2_DBGEN_Msk                (0x10000U)               /*!< DBGEN (Bitfield-Mask: 0x01)                           */
#define SPI_CFG2_CS0DETEN_Pos             (20U)                    /*!< CS0DETEN (Bit 20)                                     */
#define SPI_CFG2_CS0DETEN_Msk             (0x100000U)              /*!< CS0DETEN (Bitfield-Mask: 0x01)                        */
#define SPI_CFG2_CS1DETEN_Pos             (21U)                    /*!< CS1DETEN (Bit 21)                                     */
#define SPI_CFG2_CS1DETEN_Msk             (0x200000U)              /*!< CS1DETEN (Bitfield-Mask: 0x01)                        */
#define SPI_CFG2_CS2DETEN_Pos             (22U)                    /*!< CS2DETEN (Bit 22)                                     */
#define SPI_CFG2_CS2DETEN_Msk             (0x400000U)              /*!< CS2DETEN (Bitfield-Mask: 0x01)                        */
#define SPI_CFG2_CS3DETEN_Pos             (23U)                    /*!< CS3DETEN (Bit 23)                                     */
#define SPI_CFG2_CS3DETEN_Msk             (0x800000U)              /*!< CS3DETEN (Bitfield-Mask: 0x01)                        */
#define SPI_CFG2_CS_CHECK_Pos             (24U)                    /*!< CS_CHECK (Bit 24)                                     */
#define SPI_CFG2_CS_CHECK_Msk             (0xff000000U)            /*!< CS_CHECK (Bitfield-Mask: 0xff)                        */
/* ==========================================================  DMV  ========================================================== */
#define SPI_DMV_DMV_Pos                   (0U)                     /*!< DMV (Bit 0)                                           */
#define SPI_DMV_DMV_Msk                   (0xffffffffU)            /*!< DMV (Bitfield-Mask: 0xffffffff)                       */

/* =========================================================================================================================== */
/* ================                                            ANA                                            ================ */
/* =========================================================================================================================== */

/* =====================================================  TSENSOR_CFG0  ====================================================== */
#define ANA_TSENSOR_CFG0_TSENSOR0_MPX_SEL_Pos (30U)                /*!< TSENSOR0_MPX_SEL (Bit 30)                             */
#define ANA_TSENSOR_CFG0_TSENSOR0_MPX_SEL_Msk (0xC0000000U)        /*!< TSENSOR0_MPX_SEL (Bitfield-Mask: 0x03)                */
/* =======================================================  AUX_CFG0  ======================================================== */
#define ANA_AMUX_CFG0_AMUX_CHANNEL_SEL_Pos (23U)                   /*!< AMUX_CHANNEL_SEL (Bit 23)                             */
#define ANA_AMUX_CFG0_AMUX_CHANNEL_SEL_Msk (0x7800000U)            /*!< AMUX_CHANNEL_SEL (Bitfield-Mask: 0x0f)                */
#define ANA_AMUX_CFG0_AMUX_OUT_EN_Pos     (29U)                    /*!< AMUX_OUT_EN (Bit 29)                                  */
#define ANA_AMUX_CFG0_AMUX_OUT_EN_Msk     (0x20000000U)            /*!< AMUX_OUT_EN (Bitfield-Mask: 0x01)                     */
#define ANA_AMUX_CFG0_AMUX_BUF_BYPASS_EN_Pos (30U)                 /*!< AMUX_BUF_BYPASS_EN (Bit 30)                           */
#define ANA_AMUX_CFG0_AMUX_BUF_BYPASS_EN_Msk (0x40000000U)         /*!< AMUX_BUF_BYPASS_EN (Bitfield-Mask: 0x01)              */
#define ANA_AMUX_CFG0_AMUX_BUF_EN_Pos     (31U)                    /*!< AMUX_BUF_EN (Bit 31)                                  */
#define ANA_AMUX_CFG0_AMUX_BUF_EN_Msk     (0x80000000U)            /*!< AMUX_BUF_EN (Bitfield-Mask: 0x01)                     */
/* =======================================================  LVR_CFG0  ======================================================== */
#define ANA_LVR_CFG0_LVRDIS_Pos           (29U)                    /*!< LVRDIS (Bit 29)                                       */
#define ANA_LVR_CFG0_LVRDIS_Msk           (0x20000000U)            /*!< LVRDIS (Bitfield-Mask: 0x01)                          */
/* =======================================================  BG_CFG0  ======================================================== */
#define ANA_BG_CFG0_BG_MPX_SEL_Pos        (29U)                    /*!< BG_MPX_SEL (Bit 29)                                   */
#define ANA_BG_CFG0_BG_MPX_SEL_Msk        (0xe0000000U)            /*!< BG_MPX_SEL (Bitfield-Mask: 0x07)                      */
/* =======================================================  SPLL_CFG0  ======================================================= */
#define ANA_SPLL_CFG0_FBKDIV_Pos          (8U)                     /*!< FBKDIV (Bit 8)                                        */
#define ANA_SPLL_CFG0_FBKDIV_Msk          (0x1ff00U)               /*!< FBKDIV (Bitfield-Mask: 0x1ff)                         */
#define ANA_SPLL_CFG0_POSDIV_Pos          (23U)                    /*!< POSDIV (Bit 23)                                       */
#define ANA_SPLL_CFG0_POSDIV_Msk          (0xf800000U)             /*!< POSDIV (Bitfield-Mask: 0x1f)                          */
#define ANA_SPLL_CFG0_PREDIV_Pos          (28U)                    /*!< PREDIV (Bit 28)                                       */
#define ANA_SPLL_CFG0_PREDIV_Msk          (0xf0000000U)            /*!< PREDIV (Bitfield-Mask: 0x0f)                          */
/* =======================================================  SPLL_CFG1  ======================================================= */
#define ANA_SPLL_CFG1_POSDIV1_Pos         (0U)                     /*!< POSDIV1 (Bit 0)                                       */
#define ANA_SPLL_CFG1_POSDIV1_Msk         (0x1fU)                  /*!< POSDIV1 (Bitfield-Mask: 0x1f)                         */
#define ANA_SPLL_CFG1_LD_DLY_SEL_Pos      (23U)                    /*!< LD_DLY_SEL (Bit 23)                                   */
#define ANA_SPLL_CFG1_LD_DLY_SEL_Msk      (0x3800000U)             /*!< LD_DLY_SEL (Bitfield-Mask: 0x07)                      */
#define ANA_SPLL_CFG1_LD_EN_Pos           (27U)                    /*!< LD_EN (Bit 27)                                        */
#define ANA_SPLL_CFG1_LD_EN_Msk           (0x8000000U)             /*!< LD_EN (Bitfield-Mask: 0x01)                           */

/* =========================================================================================================================== */
/* ===============                                           ADC                                             ================= */
/* =========================================================================================================================== */

/* ==========================================================  STR  ========================================================== */
#define ADC_STR_AMO_Pos                   (0U)                     /*!< AMO (Bit 0)                                           */
#define ADC_STR_AMO_Msk                   (0x1U)                   /*!< AMO (Bitfield-Mask: 0x01)                             */
#define ADC_STR_IDLE_Pos                  (4U)                     /*!< IDLE (Bit 4)                                          */
#define ADC_STR_IDLE_Msk                  (0x10U)                  /*!< IDLE (Bitfield-Mask: 0x01)                            */
#define ADC_STR_NAMO_Pos                  (5U)                     /*!< NAMO (Bit 5)                                          */
#define ADC_STR_NAMO_Msk                  (0x20U)                  /*!< NAMO (Bitfield-Mask: 0x01)                            */
#define ADC_STR_AAMO_Pos                  (6U)                     /*!< AAMO (Bit 6)                                          */
#define ADC_STR_AAMO_Msk                  (0x40U)                  /*!< AAMO (Bitfield-Mask: 0x01)                            */
#define ADC_STR_DMARS_Pos                 (8U)                     /*!< DMARS (Bit 8)                                         */
#define ADC_STR_DMARS_Msk                 (0x100U)                 /*!< DMARS (Bitfield-Mask: 0x01)                           */
#define ADC_STR_COVCFT_Pos                (9U)                     /*!< COVCFT (Bit 9)                                        */
#define ADC_STR_COVCFT_Msk                (0x200U)                 /*!< COVCFT (Bitfield-Mask: 0x01)                          */
#define ADC_STR_ADCLDORDY_Pos             (10U)                    /*!< ADCLDORDY (Bit 10)                                    */
#define ADC_STR_ADCLDORDY_Msk             (0x400U)                 /*!< ADCLDORDY (Bitfield-Mask: 0x01)                       */
/* =========================================================  CTRL0  ========================================================= */
#define ADC_CTRL0_AMOCH_Pos               (0U)                     /*!< AMOCH (Bit 0)                                         */
#define ADC_CTRL0_AMOCH_Msk               (0x3fU)                  /*!< AMOCH (Bitfield-Mask: 0x3f)                           */
#define ADC_CTRL0_AMOSGL_Pos              (6U)                     /*!< AMOSGL (Bit 6)                                        */
#define ADC_CTRL0_AMOSGL_Msk              (0x40U)                  /*!< AMOSGL (Bitfield-Mask: 0x01)                          */
#define ADC_CTRL0_IAMOEN_Pos              (7U)                     /*!< IAMOEN (Bit 7)                                        */
#define ADC_CTRL0_IAMOEN_Msk              (0x80U)                  /*!< IAMOEN (Bitfield-Mask: 0x01)                          */
#define ADC_CTRL0_AMOEN_Pos               (8U)                     /*!< AMOEN (Bit 8)                                         */
#define ADC_CTRL0_AMOEN_Msk               (0x100U)                 /*!< AMOEN (Bitfield-Mask: 0x01)                           */
#define ADC_CTRL0_DISCNUM_Pos             (9U)                     /*!< DISCNUM (Bit 9)                                       */
#define ADC_CTRL0_DISCNUM_Msk             (0xe00U)                 /*!< DISCNUM (Bitfield-Mask: 0x07)                         */
#define ADC_CTRL0_IAUTO_Pos               (12U)                    /*!< IAUTO (Bit 12)                                        */
#define ADC_CTRL0_IAUTO_Msk               (0x1000U)                /*!< IAUTO (Bitfield-Mask: 0x01)                           */
#define ADC_CTRL0_IDISCEN_Pos             (13U)                    /*!< IDISCEN (Bit 13)                                      */
#define ADC_CTRL0_IDISCEN_Msk             (0x2000U)                /*!< IDISCEN (Bitfield-Mask: 0x01)                         */
#define ADC_CTRL0_DISCEN_Pos              (14U)                    /*!< DISCEN (Bit 14)                                       */
#define ADC_CTRL0_DISCEN_Msk              (0x4000U)                /*!< DISCEN (Bitfield-Mask: 0x01)                          */
#define ADC_CTRL0_CONT_Pos                (15U)                    /*!< CONT (Bit 15)                                         */
#define ADC_CTRL0_CONT_Msk                (0x8000U)                /*!< CONT (Bitfield-Mask: 0x01)                            */
#define ADC_CTRL0_SCAN_Pos                (16U)                    /*!< SCAN (Bit 16)                                         */
#define ADC_CTRL0_SCAN_Msk                (0x10000U)               /*!< SCAN (Bitfield-Mask: 0x01)                            */
#define ADC_CTRL0_AMOIE_Pos               (19U)                    /*!< AMOIE (Bit 19)                                        */
#define ADC_CTRL0_AMOIE_Msk               (0x80000U)               /*!< AMOIE (Bitfield-Mask: 0x01)                           */
#define ADC_CTRL0_DMAEN_Pos               (20U)                    /*!< DMAEN (Bit 20)                                        */
#define ADC_CTRL0_DMAEN_Msk               (0x100000U)              /*!< DMAEN (Bitfield-Mask: 0x01)                           */
#define ADC_CTRL0_EXTTRIG_Pos             (21U)                    /*!< EXTTRIG (Bit 21)                                      */
#define ADC_CTRL0_EXTTRIG_Msk             (0x200000U)              /*!< EXTTRIG (Bitfield-Mask: 0x01)                         */
#define ADC_CTRL0_IEXTTRIG_Pos            (22U)                    /*!< IEXTTRIG (Bit 22)                                     */
#define ADC_CTRL0_IEXTTRIG_Msk            (0x400000U)              /*!< IEXTTRIG (Bitfield-Mask: 0x01)                        */
#define ADC_CTRL0_ALIGN_Pos               (23U)                    /*!< ALIGN (Bit 23)                                        */
#define ADC_CTRL0_ALIGN_Msk               (0x800000U)              /*!< ALIGN (Bitfield-Mask: 0x01)                           */
#define ADC_CTRL0_AMOMODE_Pos             (24U)                    /*!< AMOMODE (Bit 24)                                      */
#define ADC_CTRL0_AMOMODE_Msk             (0x1000000U)             /*!< AMOMODE (Bitfield-Mask: 0x01)                         */
#define ADC_CTRL0_INTERVAL_Pos            (25U)                    /*!< INTERVAL (Bit 25)                                     */
#define ADC_CTRL0_INTERVAL_Msk            (0x2000000U)             /*!< INTERVAL (Bitfield-Mask: 0x01)                        */
#define ADC_CTRL0_ISWSTART_Pos            (30U)                    /*!< ISWSTART (Bit 30)                                     */
#define ADC_CTRL0_ISWSTART_Msk            (0x40000000U)            /*!< ISWSTART (Bitfield-Mask: 0x01)                        */
#define ADC_CTRL0_SWSTART_Pos             (31U)                    /*!< SWSTART (Bit 31)                                      */
#define ADC_CTRL0_SWSTART_Msk             (0x80000000U)            /*!< SWSTART (Bitfield-Mask: 0x01)                         */
/* =========================================================  CTRL1  ========================================================= */
#define ADC_CTRL1_ADON_Pos                (0U)                     /*!< ADON (Bit 0)                                          */
#define ADC_CTRL1_ADON_Msk                (0x1U)                   /*!< ADON (Bitfield-Mask: 0x01)                            */
#define ADC_CTRL1_CALEN_Pos               (1U)                     /*!< CALEN (Bit 1)                                         */
#define ADC_CTRL1_CALEN_Msk               (0x2U)                   /*!< CALEN (Bitfield-Mask: 0x01)                           */
#define ADC_CTRL1_AVGS_Pos                (8U)                     /*!< AVGS (Bit 8)                                          */
#define ADC_CTRL1_AVGS_Msk                (0x300U)                 /*!< AVGS (Bitfield-Mask: 0x03)                            */
#define ADC_CTRL1_AVGE_Pos                (10U)                    /*!< AVGE (Bit 10)                                         */
#define ADC_CTRL1_AVGE_Msk                (0x400U)                 /*!< AVGE (Bitfield-Mask: 0x01)                            */
#define ADC_CTRL1_PSC_Pos                 (12U)                    /*!< PSC (Bit 12)                                          */
#define ADC_CTRL1_PSC_Msk                 (0xf000U)                /*!< PSC (Bitfield-Mask: 0x0f)                             */
/* =========================================================  IOFR  ========================================================== */
#define ADC_IOFR_IOFR_Pos                 (0U)                     /*!< IOFR (Bit 0)                                          */
#define ADC_IOFR_IOFR_Msk                 (0xfffU)                 /*!< IOFR (Bitfield-Mask: 0xfff)                           */
/* =========================================================  AMOHR  ========================================================= */
#define ADC_AMOHR_AMOHT_Pos               (0U)                     /*!< AMOHT (Bit 0)                                         */
#define ADC_AMOHR_AMOHT_Msk               (0xfffU)                 /*!< AMOHT (Bitfield-Mask: 0xfff)                          */
#define ADC_AMOHR_AMOHO_Pos               (16U)                    /*!< AMOHO (Bit 16)                                        */
#define ADC_AMOHR_AMOHO_Msk               (0xfff0000U)             /*!< AMOHO (Bitfield-Mask: 0xfff)                          */
/* =========================================================  AMOLR  ========================================================= */
#define ADC_AMOLR_AMOLT_Pos               (0U)                     /*!< AMOLT (Bit 0)                                         */
#define ADC_AMOLR_AMOLT_Msk               (0xfffU)                 /*!< AMOLT (Bitfield-Mask: 0xfff)                          */
#define ADC_AMOLR_AMOLO_Pos               (16U)                    /*!< AMOLO (Bit 16)                                        */
#define ADC_AMOLR_AMOLO_Msk               (0xfff0000U)             /*!< AMOLO (Bitfield-Mask: 0xfff)                          */
/* =========================================================  SPT   ========================================================== */
#define ADC_SPT_Msk                       (0x7U)                   /*!< SPT  (Bitfield-Mask: 0x07)                            */
#define ADC_SPT_Width                     (3U)                     /*!< SPT width (3bit)                                      */
/* =========================================================   SQR   ========================================================= */
#define ADC_SQR_SEQ_Pos                   (0U)                     /*!< SEQ (Bit 0)                                           */
#define ADC_SQR_SEQ_Msk                   (0x3fU)                  /*!< SEQ (Bitfield-Mask: 0x3f)                             */
#define ADC_SQR_IE_Pos                    (6U)                     /*!< IE (Bit 6)                                            */
#define ADC_SQR_IE_Msk                    (0x40U)                  /*!< IE (Bitfield-Mask: 0x01)                              */
/* ==========================================================  SQL  ========================================================== */
#define ADC_SQL_RSQL_Pos                  (0U)                     /*!< RSQL (Bit 0)                                          */
#define ADC_SQL_RSQL_Msk                  (0x3fU)                  /*!< RSQL (Bitfield-Mask: 0x3f)                            */
#define ADC_SQL_ISQL_Pos                  (8U)                     /*!< ISQL (Bit 8)                                          */
#define ADC_SQL_ISQL_Msk                  (0x300U)                 /*!< ISQL (Bitfield-Mask: 0x03)                            */
#define ADC_SQL_SUPPLY_Pos                (16U)                    /*!< SUPPLY (Bit 16)                                       */
#define ADC_SQL_SUPPLY_Msk                (0x70000U)               /*!< SUPPLY (Bitfield-Mask: 0x07)                          */
#define ADC_SQL_REFSEL_Pos                (24U)                    /*!< REFSEL (Bit 24)                                       */
#define ADC_SQL_REFSEL_Msk                (0x1000000U)             /*!< REFSEL (Bitfield-Mask: 0x01)                          */
/* =========================================================  CALI0  ========================================================= */
#define ADC_CALI0_GE0_Pos                 (0U)                     /*!< GE0 (Bit 0)                                           */
#define ADC_CALI0_GE0_Msk                 (0xfffU)                 /*!< GE0 (Bitfield-Mask: 0xfff)                            */
#define ADC_CALI0_GE1_Pos                 (16U)                    /*!< GE1 (Bit 16)                                          */
#define ADC_CALI0_GE1_Msk                 (0xfff0000U)             /*!< GE1 (Bitfield-Mask: 0xfff)                            */
/* =========================================================  CALI1  ========================================================= */
#define ADC_CALI1_OE0_Pos                 (0U)                     /*!< OE0 (Bit 0)                                           */
#define ADC_CALI1_OE0_Msk                 (0x7ffU)                 /*!< OE0 (Bitfield-Mask: 0x7ff)                            */
#define ADC_CALI1_OE1_Pos                 (16U)                    /*!< OE1 (Bit 16)                                          */
#define ADC_CALI1_OE1_Msk                 (0x7ff0000U)             /*!< OE1 (Bitfield-Mask: 0x7ff)                            */
/* ========================================================  RDR/IDR  ======================================================== */
#define ADC_DR_DATA_Pos                   (0U)                     /*!< DR (Bit 0)                                            */
#define ADC_DR_DATA_Msk                   (0xffffU)                /*!< DR (Bitfield-Mask: 0xfff)                             */
#define ADC_DR_CH_Pos                     (16U)                    /*!< CH (Bit 16)                                           */
#define ADC_DR_CH_Msk                     (0x3f0000U)              /*!< CH (Bitfield-Mask: 0x3f)                              */
#define ADC_DR_EVEN_PARITY_Pos            (22U)                    /*!< EVEN_PARITY (Bit 22)                                  */
#define ADC_DR_EVEN_PARITY_Msk            (0x400000U)              /*!< EVEN_PARITY (Bitfield-Mask: 0x01)                     */
/* =========================================================  REOC  ========================================================== */
#define ADC0_REOC_REOC_Pos                (0U)                     /*!< REOC (Bit 0)                                          */
#define ADC0_REOC_REOC_Msk                (0xffffffffU)            /*!< REOC (Bitfield-Mask: 0xffffffff)                      */
/* =========================================================  IEOC  ========================================================== */
#define ADC0_IEOC_IEOC_Pos                (0U)                     /*!< IEOC (Bit 0)                                          */
#define ADC0_IEOC_IEOC_Msk                (0xfU)                   /*!< IEOC (Bitfield-Mask: 0x0f)                            */

/* =========================================================================================================================== */
/* ================                                           ACMP                                            ================ */
/* =========================================================================================================================== */

/* ==========================================================  CR0  ========================================================== */
#define ACMP_CR0_MOD_Pos                  (0U)                     /*!< MOD (Bit 0)                                           */
#define ACMP_CR0_MOD_Msk                  (0x3U)                   /*!< MOD (Bitfield-Mask: 0x03)                             */
#define ACMP_CR0_OPE_Pos                  (2U)                     /*!< OPE (Bit 2)                                           */
#define ACMP_CR0_OPE_Msk                  (0x4U)                   /*!< OPE (Bitfield-Mask: 0x01)                             */
#define ACMP_CR0_OUTEN_Pos                (3U)                     /*!< OUTEN (Bit 3)                                         */
#define ACMP_CR0_OUTEN_Msk                (0x8U)                   /*!< OUTEN (Bitfield-Mask: 0x01)                           */
#define ACMP_CR0_IE_Pos                   (4U)                     /*!< IE (Bit 4)                                            */
#define ACMP_CR0_IE_Msk                   (0x10U)                  /*!< IE (Bitfield-Mask: 0x01)                              */
#define ACMP_CR0_HYSTVOL_Pos              (6U)                     /*!< HYSTVOL (Bit 6)                                       */
#define ACMP_CR0_HYSTVOL_Msk              (0xc0U)                  /*!< HYSTVOL (Bitfield-Mask: 0x03)                         */
#define ACMP_CR0_HYSTMOD_Pos              (8U)                     /*!< HYSTMOD (Bit 8)                                       */
#define ACMP_CR0_HYSTMOD_Msk              (0x100U)                 /*!< HYSTMOD (Bitfield-Mask: 0x01)                         */
#define ACMP_CR0_EN_Pos                   (9U)                     /*!< EN (Bit 9)                                            */
#define ACMP_CR0_EN_Msk                   (0x200U)                 /*!< EN (Bitfield-Mask: 0x01)                              */
#define ACMP_CR0_LPF_Pos                  (10U)                    /*!< LPF (Bit 10)                                          */
#define ACMP_CR0_LPF_Msk                  (0xc00U)                 /*!< LPF (Bitfield-Mask: 0x03)                             */
#define ACMP_CR0_FILTEN_Pos               (16U)                    /*!< FILTEN (Bit 16)                                       */
#define ACMP_CR0_FILTEN_Msk               (0x10000U)               /*!< FILTEN (Bitfield-Mask: 0x01)                          */
#define ACMP_CR0_WINEN_Pos                (17U)                    /*!< WINEN (Bit 17)                                        */
#define ACMP_CR0_WINEN_Msk                (0x20000U)               /*!< WINEN (Bitfield-Mask: 0x01)                           */
#define ACMP_CR0_INVT_Pos                 (18U)                    /*!< INVT (Bit 18)                                         */
#define ACMP_CR0_INVT_Msk                 (0x40000U)               /*!< INVT (Bitfield-Mask: 0x01)                            */
#define ACMP_CR0_FILTER_CNT_Pos           (19U)                    /*!< FILTER_CNT (Bit 19)                                   */
#define ACMP_CR0_FILTER_CNT_Msk           (0x7f80000U)             /*!< FILTER_CNT (Bitfield-Mask: 0xff)                      */
#define ACMP_CR0_COS_Pos                  (27U)                    /*!< COS (Bit 27)                                          */
#define ACMP_CR0_COS_Msk                  (0x8000000U)             /*!< COS (Bitfield-Mask: 0x01)                             */
/* ==========================================================  CR1  ========================================================== */
#define ACMP_CR1_NSEL_Pos                 (0U)                     /*!< NSEL (Bit 0)                                          */
#define ACMP_CR1_NSEL_Msk                 (0xfU)                   /*!< NSEL (Bitfield-Mask: 0x0f)                            */
#define ACMP_CR1_PSEL_Pos                 (4U)                     /*!< PSEL (Bit 4)                                          */
#define ACMP_CR1_PSEL_Msk                 (0xf0U)                  /*!< PSEL (Bitfield-Mask: 0x0f)                            */
/* ==========================================================  CR2  ========================================================== */
#define ACMP_CR2_DACVAL_Pos               (0U)                     /*!< DACVAL (Bit 0)                                        */
#define ACMP_CR2_DACVAL_Msk               (0xffU)                  /*!< DACVAL (Bitfield-Mask: 0xff)                          */
#define ACMP_CR2_DACEN_Pos                (8U)                     /*!< DACEN (Bit 8)                                         */
#define ACMP_CR2_DACEN_Msk                (0x100U)                 /*!< DACEN (Bitfield-Mask: 0x01)                           */
#define ACMP_CR2_DACREF_Pos               (9U)                     /*!< DACREF (Bit 9)                                        */
#define ACMP_CR2_DACREF_Msk               (0x200U)                 /*!< DACREF (Bitfield-Mask: 0x01)                          */
/* ==========================================================  CR3  ========================================================== */
#define ACMP_CR3_NSPLEN_Pos               (3U)                     /*!< NSPLEN (Bit 3)                                        */
#define ACMP_CR3_NSPLEN_Msk               (0x8U)                   /*!< NSPLEN (Bitfield-Mask: 0x01)                          */
#define ACMP_CR3_PSPLEN_Pos               (7U)                     /*!< PSPLEN (Bit 7)                                        */
#define ACMP_CR3_PSPLEN_Msk               (0x80U)                  /*!< PSPLEN (Bitfield-Mask: 0x01)                          */
/* ==========================================================  CR4  ========================================================== */
#define ACMP_CR4_PLSEQ_Pos                (0U)                     /*!< PLSEQ (Bit 0)                                         */
#define ACMP_CR4_PLSEQ_Msk                (0x1ffU)                 /*!< PLSEQ (Bitfield-Mask: 0x1ff)                          */
/* ==========================================================  DR  =========================================================== */
#define ACMP_DR_O0_Pos                    (0U)                     /*!< O0 (Bit 0)                                            */
#define ACMP_DR_O0_Msk                    (0x1U)                   /*!< O0 (Bitfield-Mask: 0x01)                              */
#define ACMP_DR_O1_Pos                    (1U)                     /*!< O1 (Bit 1)                                            */
#define ACMP_DR_O1_Msk                    (0x2U)                   /*!< O1 (Bitfield-Mask: 0x01)                              */
#define ACMP_DR_O2_Pos                    (2U)                     /*!< O2 (Bit 2)                                            */
#define ACMP_DR_O2_Msk                    (0x4U)                   /*!< O2 (Bitfield-Mask: 0x01)                              */
#define ACMP_DR_O3_Pos                    (3U)                     /*!< O3 (Bit 3)                                            */
#define ACMP_DR_O3_Msk                    (0x8U)                   /*!< O3 (Bitfield-Mask: 0x01)                              */
#define ACMP_DR_O4_Pos                    (4U)                     /*!< O4 (Bit 4)                                            */
#define ACMP_DR_O4_Msk                    (0x10U)                  /*!< O4 (Bitfield-Mask: 0x01)                              */
#define ACMP_DR_O5_Pos                    (5U)                     /*!< O5 (Bit 5)                                            */
#define ACMP_DR_O5_Msk                    (0x20U)                  /*!< O5 (Bitfield-Mask: 0x01)                              */
#define ACMP_DR_O6_Pos                    (6U)                     /*!< O6 (Bit 6)                                            */
#define ACMP_DR_O6_Msk                    (0x40U)                  /*!< O6 (Bitfield-Mask: 0x01)                              */
#define ACMP_DR_O7_Pos                    (7U)                     /*!< O7 (Bit 7)                                            */
#define ACMP_DR_O7_Msk                    (0x80U)                  /*!< O7 (Bitfield-Mask: 0x01)                              */
#define ACMP_DR_O8_Pos                    (8U)                     /*!< O8 (Bit 8)                                            */
#define ACMP_DR_O8_Msk                    (0x100U)                 /*!< O8 (Bitfield-Mask: 0x01)                              */
#define ACMP_DR_O_Pos                     (9U)                     /*!< O (Bit 9)                                             */
#define ACMP_DR_O_Msk                     (0x200U)                 /*!< O (Bitfield-Mask: 0x01)                               */
#define ACMP_DR_POLLING_O_Msk             (0x1ffU)                 /*!< POLLING O (Bitfield-Mask: 0x1ff)                      */
/* ==========================================================  SR  =========================================================== */
#define ACMP_SR_F0_Pos                    (0U)                     /*!< F0 (Bit 0)                                            */
#define ACMP_SR_F0_Msk                    (0x1U)                   /*!< F0 (Bitfield-Mask: 0x01)                              */
#define ACMP_SR_F1_Pos                    (1U)                     /*!< F1 (Bit 1)                                            */
#define ACMP_SR_F1_Msk                    (0x2U)                   /*!< F1 (Bitfield-Mask: 0x01)                              */
#define ACMP_SR_F2_Pos                    (2U)                     /*!< F2 (Bit 2)                                            */
#define ACMP_SR_F2_Msk                    (0x4U)                   /*!< F2 (Bitfield-Mask: 0x01)                              */
#define ACMP_SR_F3_Pos                    (3U)                     /*!< F3 (Bit 3)                                            */
#define ACMP_SR_F3_Msk                    (0x8U)                   /*!< F3 (Bitfield-Mask: 0x01)                              */
#define ACMP_SR_F4_Pos                    (4U)                     /*!< F4 (Bit 4)                                            */
#define ACMP_SR_F4_Msk                    (0x10U)                  /*!< F4 (Bitfield-Mask: 0x01)                              */
#define ACMP_SR_F5_Pos                    (5U)                     /*!< F5 (Bit 5)                                            */
#define ACMP_SR_F5_Msk                    (0x20U)                  /*!< F5 (Bitfield-Mask: 0x01)                              */
#define ACMP_SR_F6_Pos                    (6U)                     /*!< F6 (Bit 6)                                            */
#define ACMP_SR_F6_Msk                    (0x40U)                  /*!< F6 (Bitfield-Mask: 0x01)                              */
#define ACMP_SR_F7_Pos                    (7U)                     /*!< F7 (Bit 7)                                            */
#define ACMP_SR_F7_Msk                    (0x80U)                  /*!< F7 (Bitfield-Mask: 0x01)                              */
#define ACMP_SR_F8_Pos                    (8U)                     /*!< F8 (Bit 8)                                            */
#define ACMP_SR_F8_Msk                    (0x100U)                 /*!< F8 (Bitfield-Mask: 0x01)                              */
#define ACMP_SR_F_Pos                     (9U)                     /*!< F (Bit 9)                                             */
#define ACMP_SR_F_Msk                     (0x200U)                 /*!< F (Bitfield-Mask: 0x01)                               */
#define ACMP_SR_POLLING_F_Msk             (0x1FFU)                 /*!< POLLING STATUS (Bitfield-Mask: 0x1ff)                 */
/* ==========================================================  FD  =========================================================== */
#define ACMP_FD_PLFD_Pos                  (0U)                     /*!< PLFD (Bit 0)                                          */
#define ACMP_FD_PLFD_Msk                  (0x3U)                   /*!< PLFD (Bitfield-Mask: 0x03)                            */
/* ==========================================================  OPA  ========================================================== */
#define ACMP_OPA_OPASEL_Pos               (0U)                     /*!< OPASEL (Bit 0)                                        */
#define ACMP_OPA_OPASEL_Msk               (0xFU)                   /*!< OPASEL (Bitfield-Mask: 0x0F)                          */
/* ==========================================================  OPB  ========================================================== */
#define ACMP_OPB_OPBSEL_Pos               (0U)                     /*!< OPBSEL (Bit 0)                                        */
#define ACMP_OPB_OPBSEL_Msk               (0xFU)                   /*!< OPBSEL (Bitfield-Mask: 0x0F)                          */
/* ==========================================================  OPC  ========================================================== */
#define ACMP_OPC_OPCSEL_Pos               (0U)                     /*!< OPCSEL (Bit 0)                                        */
#define ACMP_OPC_OPCSEL_Msk               (0xFU)                   /*!< OPCSEL (Bitfield-Mask: 0x0F)                          */
/* ==========================================================  CLK  ========================================================== */
#define ACMP_CLK_PSC_Pos                  (0U)                     /*!< PSC (Bit 0)                                           */
#define ACMP_CLK_PSC_Msk                  (0x7U)                   /*!< PSC (Bitfield-Mask: 0x07)                             */

/* =========================================================================================================================== */
/* ================                                           ABIST                                           ================ */
/* =========================================================================================================================== */

/* ==========================================================  CR  =========================================================== */
#define ABIST_CR_SOFT_START_Pos           (0U)                     /*!< SOFT_START (Bit 0)                                    */
#define ABIST_CR_SOFT_START_Msk           (0x7U)                   /*!< SOFT_START (Bitfield-Mask: 0x07)                      */
/* ==========================================================  SR  =========================================================== */
#define ABIST_SR_DLVDS_Pos                (0U)                     /*!< DLVDS (Bit 0)                                         */
#define ABIST_SR_DLVDS_Msk                (0x1U)                   /*!< DLVDS (Bitfield-Mask: 0x01)                           */
#define ABIST_SR_DLVRS_TST_Pos            (1U)                     /*!< DLVRS_TST (Bit 1)                                     */
#define ABIST_SR_DLVRS_TST_Msk            (0x2U)                   /*!< DLVRS_TST (Bitfield-Mask: 0x01)                       */
#define ABIST_SR_FLHLVDS_Pos              (2U)                     /*!< FLHLVDS (Bit 2)                                       */
#define ABIST_SR_FLHLVDS_Msk              (0x4U)                   /*!< FLHLVDS (Bitfield-Mask: 0x01)                         */
#define ABIST_SR_FLHOVDS_Pos              (3U)                     /*!< FLHOVDS (Bit 3)                                       */
#define ABIST_SR_FLHOVDS_Msk              (0x8U)                   /*!< FLHOVDS (Bitfield-Mask: 0x01)                         */
#define ABIST_SR_DIGLVDS_TST_Pos          (4U)                     /*!< DIGLVDS_TST (Bit 4)                                   */
#define ABIST_SR_DIGLVDS_TST_Msk          (0x10U)                  /*!< DIGLVDS_TST (Bitfield-Mask: 0x01)                     */
#define ABIST_SR_DIGOVDS_Pos              (5U)                     /*!< DIGOVDS (Bit 5)                                       */
#define ABIST_SR_DIGOVDS_Msk              (0x20U)                  /*!< DIGOVDS (Bitfield-Mask: 0x01)                         */
#define ABIST_SR_DFLHLVDS_Pos             (6U)                     /*!< DFLHLVDS (Bit 6)                                      */
#define ABIST_SR_DFLHLVDS_Msk             (0x40U)                  /*!< DFLHLVDS (Bitfield-Mask: 0x01)                        */
#define ABIST_SR_VVDDLVDS_Pos             (7U)                     /*!< VVDDLVDS (Bit 7)                                      */
#define ABIST_SR_VVDDLVDS_Msk             (0x80U)                  /*!< VVDDLVDS (Bitfield-Mask: 0x01)                        */
/* ==========================================================  ER  =========================================================== */
#define ABIST_ER_DLVDE_Pos                (0U)                     /*!< DLVDE (Bit 0)                                         */
#define ABIST_ER_DLVDE_Msk                (0x1U)                   /*!< DLVDE (Bitfield-Mask: 0x01)                           */
#define ABIST_ER_FLHLVD_Pos               (2U)                     /*!< FLHLVD (Bit 2)                                        */
#define ABIST_ER_FLHLVD_Msk               (0x4U)                   /*!< FLHLVD (Bitfield-Mask: 0x01)                          */
#define ABIST_ER_FLHOVD_Pos               (3U)                     /*!< FLHOVD (Bit 3)                                        */
#define ABIST_ER_FLHOVD_Msk               (0x8U)                   /*!< FLHOVD (Bitfield-Mask: 0x01)                          */
#define ABIST_ER_DIGOVD_Pos               (5U)                     /*!< DIGOVD (Bit 5)                                        */
#define ABIST_ER_DIGOVD_Msk               (0x20U)                  /*!< DIGOVD (Bitfield-Mask: 0x01)                          */
#define ABIST_ER_DFLHLVD_Pos              (6U)                     /*!< DFLHLVD (Bit 6)                                       */
#define ABIST_ER_DFLHLVD_Msk              (0x40U)                  /*!< DFLHLVD (Bitfield-Mask: 0x01)                         */
#define ABIST_ER_VVDDLVD_Pos              (7U)                     /*!< VVDDLVD (Bit 7)                                       */
#define ABIST_ER_VVDDLVD_Msk              (0x80U)                  /*!< VVDDLVD (Bitfield-Mask: 0x01)                         */

/* =========================================================================================================================== */
/* ===============                                           PWM                                             ================= */
/* =========================================================================================================================== */

/* =========================================================  INIT  ========================================================== */
#define PWM_INIT_COFE_Pos                 (0U)                     /*!< COFE (Bit 0)                                          */
#define PWM_INIT_COFE_Msk                 (0x1U)                   /*!< COFE (Bitfield-Mask: 0x01)                            */
#define PWM_INIT_CUFE_Pos                 (1U)                     /*!< CUFE (Bit 1)                                          */
#define PWM_INIT_CUFE_Msk                 (0x2U)                   /*!< CUFE (Bitfield-Mask: 0x01)                            */
#define PWM_INIT_OFDIR_Pos                (2U)                     /*!< OFDIR (Bit 2)                                         */
#define PWM_INIT_OFDIR_Msk                (0x4U)                   /*!< OFDIR (Bitfield-Mask: 0x01)                           */
#define PWM_INIT_CLKSRC_Pos               (3U)                     /*!< CLKSRC (Bit 3)                                        */
#define PWM_INIT_CLKSRC_Msk               (0x18U)                  /*!< CLKSRC (Bitfield-Mask: 0x03)                          */
#define PWM_INIT_CNTMODE_Pos              (5U)                     /*!< CNTMODE (Bit 5)                                       */
#define PWM_INIT_CNTMODE_Msk              (0x20U)                  /*!< CNTMODE (Bitfield-Mask: 0x01)                         */
#define PWM_INIT_CNTOIE_Pos               (6U)                     /*!< CNTOIE (Bit 6)                                        */
#define PWM_INIT_CNTOIE_Msk               (0x40U)                  /*!< CNTOIE (Bitfield-Mask: 0x01)                          */
#define PWM_INIT_CNTOF_Pos                (7U)                     /*!< CNTOF (Bit 7)                                         */
#define PWM_INIT_CNTOF_Msk                (0x80U)                  /*!< CNTOF (Bitfield-Mask: 0x01)                           */
#define PWM_INIT_CLKPSC_Pos               (8U)                     /*!< CLKPSC (Bit 8)                                        */
#define PWM_INIT_CLKPSC_Msk               (0xffff00U)              /*!< CLKPSC (Bitfield-Mask: 0xffff)                        */
#define PWM_INIT_OFUFDMAEN_Pos            (28U)                    /*!< OFUFDMAEN (Bit 28)                                    */
#define PWM_INIT_OFUFDMAEN_Msk            (0x10000000U)            /*!< OFUFDMAEN (Bitfield-Mask: 0x01)                       */
/* ==========================================================  CNT  ========================================================== */
#define PWM_CNT_COUNT_Pos                 (0U)                     /*!< COUNT (Bit 0)                                         */
#define PWM_CNT_COUNT_Msk                 (0xffffU)                /*!< COUNT (Bitfield-Mask: 0xffff)                         */
/* =========================================================  MCVR  ========================================================== */
#define PWM_MCVR_MCVR_Pos                 (0U)                     /*!< MCVR (Bit 0)                                          */
#define PWM_MCVR_MCVR_Msk                 (0xffffU)                /*!< MCVR (Bitfield-Mask: 0xffff)                          */
/* ========================================================  CH0SCR  ========================================================= */
#define PWM_CH0SCR_CHRSTEN_Pos            (0U)                     /*!< CHRSTEN (Bit 0)                                       */
#define PWM_CH0SCR_CHRSTEN_Msk            (0x1U)                   /*!< CHRSTEN (Bitfield-Mask: 0x01)                         */
#define PWM_CH0SCR_DIR_Pos                (1U)                     /*!< DIR (Bit 1)                                           */
#define PWM_CH0SCR_DIR_Msk                (0x2U)                   /*!< DIR (Bitfield-Mask: 0x01)                             */
#define PWM_CH0SCR_ELSR0_Pos              (2U)                     /*!< ELSR0 (Bit 2)                                         */
#define PWM_CH0SCR_ELSR0_Msk              (0x4U)                   /*!< ELSR0 (Bitfield-Mask: 0x01)                           */
#define PWM_CH0SCR_ELSR1_Pos              (3U)                     /*!< ELSR1 (Bit 3)                                         */
#define PWM_CH0SCR_ELSR1_Msk              (0x8U)                   /*!< ELSR1 (Bitfield-Mask: 0x01)                           */
#define PWM_CH0SCR_MSR0_Pos               (4U)                     /*!< MSR0 (Bit 4)                                          */
#define PWM_CH0SCR_MSR0_Msk               (0x10U)                  /*!< MSR0 (Bitfield-Mask: 0x01)                            */
#define PWM_CH0SCR_MSR1_Pos               (5U)                     /*!< MSR1 (Bit 5)                                          */
#define PWM_CH0SCR_MSR1_Msk               (0x20U)                  /*!< MSR1 (Bitfield-Mask: 0x01)                            */
#define PWM_CH0SCR_CHIE_Pos               (6U)                     /*!< CHIE (Bit 6)                                          */
#define PWM_CH0SCR_CHIE_Msk               (0x40U)                  /*!< CHIE (Bitfield-Mask: 0x01)                            */
#define PWM_CH0SCR_CHIF_Pos               (7U)                     /*!< CHIF (Bit 7)                                          */
#define PWM_CH0SCR_CHIF_Msk               (0x80U)                  /*!< CHIF (Bitfield-Mask: 0x01)                            */
#define PWM_CH0SCR_DMAEN_Pos              (8U)                     /*!< DMAEN (Bit 8)                                         */
#define PWM_CH0SCR_DMAEN_Msk              (0x100U)                 /*!< DMAEN (Bitfield-Mask: 0x01)                           */
/* ========================================================  CH1SCR  ========================================================= */
#define PWM_CH1SCR_CHRSTEN_Pos            (0U)                     /*!< CHRSTEN (Bit 0)                                       */
#define PWM_CH1SCR_CHRSTEN_Msk            (0x1U)                   /*!< CHRSTEN (Bitfield-Mask: 0x01)                         */
#define PWM_CH1SCR_DIR_Pos                (1U)                     /*!< DIR (Bit 1)                                           */
#define PWM_CH1SCR_DIR_Msk                (0x2U)                   /*!< DIR (Bitfield-Mask: 0x01)                             */
#define PWM_CH1SCR_ELSR0_Pos              (2U)                     /*!< ELSR0 (Bit 2)                                         */
#define PWM_CH1SCR_ELSR0_Msk              (0x4U)                   /*!< ELSR0 (Bitfield-Mask: 0x01)                           */
#define PWM_CH1SCR_ELSR1_Pos              (3U)                     /*!< ELSR1 (Bit 3)                                         */
#define PWM_CH1SCR_ELSR1_Msk              (0x8U)                   /*!< ELSR1 (Bitfield-Mask: 0x01)                           */
#define PWM_CH1SCR_MSR0_Pos               (4U)                     /*!< MSR0 (Bit 4)                                          */
#define PWM_CH1SCR_MSR0_Msk               (0x10U)                  /*!< MSR0 (Bitfield-Mask: 0x01)                            */
#define PWM_CH1SCR_MSR1_Pos               (5U)                     /*!< MSR1 (Bit 5)                                          */
#define PWM_CH1SCR_MSR1_Msk               (0x20U)                  /*!< MSR1 (Bitfield-Mask: 0x01)                            */
#define PWM_CH1SCR_CHIE_Pos               (6U)                     /*!< CHIE (Bit 6)                                          */
#define PWM_CH1SCR_CHIE_Msk               (0x40U)                  /*!< CHIE (Bitfield-Mask: 0x01)                            */
#define PWM_CH1SCR_CHIF_Pos               (7U)                     /*!< CHIF (Bit 7)                                          */
#define PWM_CH1SCR_CHIF_Msk               (0x80U)                  /*!< CHIF (Bitfield-Mask: 0x01)                            */
#define PWM_CH1SCR_DMAEN_Pos              (8U)                     /*!< DMAEN (Bit 8)                                         */
#define PWM_CH1SCR_DMAEN_Msk              (0x100U)                 /*!< DMAEN (Bitfield-Mask: 0x01)                           */
/* ========================================================  CH2SCR  ========================================================= */
#define PWM_CH2SCR_CHRSTEN_Pos            (0U)                     /*!< CHRSTEN (Bit 0)                                       */
#define PWM_CH2SCR_CHRSTEN_Msk            (0x1U)                   /*!< CHRSTEN (Bitfield-Mask: 0x01)                         */
#define PWM_CH2SCR_DIR_Pos                (1U)                     /*!< DIR (Bit 1)                                           */
#define PWM_CH2SCR_DIR_Msk                (0x2U)                   /*!< DIR (Bitfield-Mask: 0x01)                             */
#define PWM_CH2SCR_ELSR0_Pos              (2U)                     /*!< ELSR0 (Bit 2)                                         */
#define PWM_CH2SCR_ELSR0_Msk              (0x4U)                   /*!< ELSR0 (Bitfield-Mask: 0x01)                           */
#define PWM_CH2SCR_ELSR1_Pos              (3U)                     /*!< ELSR1 (Bit 3)                                         */
#define PWM_CH2SCR_ELSR1_Msk              (0x8U)                   /*!< ELSR1 (Bitfield-Mask: 0x01)                           */
#define PWM_CH2SCR_MSR0_Pos               (4U)                     /*!< MSR0 (Bit 4)                                          */
#define PWM_CH2SCR_MSR0_Msk               (0x10U)                  /*!< MSR0 (Bitfield-Mask: 0x01)                            */
#define PWM_CH2SCR_MSR1_Pos               (5U)                     /*!< MSR1 (Bit 5)                                          */
#define PWM_CH2SCR_MSR1_Msk               (0x20U)                  /*!< MSR1 (Bitfield-Mask: 0x01)                            */
#define PWM_CH2SCR_CHIE_Pos               (6U)                     /*!< CHIE (Bit 6)                                          */
#define PWM_CH2SCR_CHIE_Msk               (0x40U)                  /*!< CHIE (Bitfield-Mask: 0x01)                            */
#define PWM_CH2SCR_CHIF_Pos               (7U)                     /*!< CHIF (Bit 7)                                          */
#define PWM_CH2SCR_CHIF_Msk               (0x80U)                  /*!< CHIF (Bitfield-Mask: 0x01)                            */
#define PWM_CH2SCR_DMAEN_Pos              (8U)                     /*!< DMAEN (Bit 8)                                         */
#define PWM_CH2SCR_DMAEN_Msk              (0x100U)                 /*!< DMAEN (Bitfield-Mask: 0x01)                           */
/* ========================================================  CH3SCR  ========================================================= */
#define PWM_CH3SCR_CHRSTEN_Pos            (0U)                     /*!< CHRSTEN (Bit 0)                                       */
#define PWM_CH3SCR_CHRSTEN_Msk            (0x1U)                   /*!< CHRSTEN (Bitfield-Mask: 0x01)                         */
#define PWM_CH3SCR_DIR_Pos                (1U)                     /*!< DIR (Bit 1)                                           */
#define PWM_CH3SCR_DIR_Msk                (0x2U)                   /*!< DIR (Bitfield-Mask: 0x01)                             */
#define PWM_CH3SCR_ELSR0_Pos              (2U)                     /*!< ELSR0 (Bit 2)                                         */
#define PWM_CH3SCR_ELSR0_Msk              (0x4U)                   /*!< ELSR0 (Bitfield-Mask: 0x01)                           */
#define PWM_CH3SCR_ELSR1_Pos              (3U)                     /*!< ELSR1 (Bit 3)                                         */
#define PWM_CH3SCR_ELSR1_Msk              (0x8U)                   /*!< ELSR1 (Bitfield-Mask: 0x01)                           */
#define PWM_CH3SCR_MSR0_Pos               (4U)                     /*!< MSR0 (Bit 4)                                          */
#define PWM_CH3SCR_MSR0_Msk               (0x10U)                  /*!< MSR0 (Bitfield-Mask: 0x01)                            */
#define PWM_CH3SCR_MSR1_Pos               (5U)                     /*!< MSR1 (Bit 5)                                          */
#define PWM_CH3SCR_MSR1_Msk               (0x20U)                  /*!< MSR1 (Bitfield-Mask: 0x01)                            */
#define PWM_CH3SCR_CHIE_Pos               (6U)                     /*!< CHIE (Bit 6)                                          */
#define PWM_CH3SCR_CHIE_Msk               (0x40U)                  /*!< CHIE (Bitfield-Mask: 0x01)                            */
#define PWM_CH3SCR_CHIF_Pos               (7U)                     /*!< CHIF (Bit 7)                                          */
#define PWM_CH3SCR_CHIF_Msk               (0x80U)                  /*!< CHIF (Bitfield-Mask: 0x01)                            */
#define PWM_CH3SCR_DMAEN_Pos              (8U)                     /*!< DMAEN (Bit 8)                                         */
#define PWM_CH3SCR_DMAEN_Msk              (0x100U)                 /*!< DMAEN (Bitfield-Mask: 0x01)                           */
/* ========================================================  CH4SCR  ========================================================= */
#define PWM_CH4SCR_CHRSTEN_Pos            (0U)                     /*!< CHRSTEN (Bit 0)                                       */
#define PWM_CH4SCR_CHRSTEN_Msk            (0x1U)                   /*!< CHRSTEN (Bitfield-Mask: 0x01)                         */
#define PWM_CH4SCR_DIR_Pos                (1U)                     /*!< DIR (Bit 1)                                           */
#define PWM_CH4SCR_DIR_Msk                (0x2U)                   /*!< DIR (Bitfield-Mask: 0x01)                             */
#define PWM_CH4SCR_ELSR0_Pos              (2U)                     /*!< ELSR0 (Bit 2)                                         */
#define PWM_CH4SCR_ELSR0_Msk              (0x4U)                   /*!< ELSR0 (Bitfield-Mask: 0x01)                           */
#define PWM_CH4SCR_ELSR1_Pos              (3U)                     /*!< ELSR1 (Bit 3)                                         */
#define PWM_CH4SCR_ELSR1_Msk              (0x8U)                   /*!< ELSR1 (Bitfield-Mask: 0x01)                           */
#define PWM_CH4SCR_MSR0_Pos               (4U)                     /*!< MSR0 (Bit 4)                                          */
#define PWM_CH4SCR_MSR0_Msk               (0x10U)                  /*!< MSR0 (Bitfield-Mask: 0x01)                            */
#define PWM_CH4SCR_MSR1_Pos               (5U)                     /*!< MSR1 (Bit 5)                                          */
#define PWM_CH4SCR_MSR1_Msk               (0x20U)                  /*!< MSR1 (Bitfield-Mask: 0x01)                            */
#define PWM_CH4SCR_CHIE_Pos               (6U)                     /*!< CHIE (Bit 6)                                          */
#define PWM_CH4SCR_CHIE_Msk               (0x40U)                  /*!< CHIE (Bitfield-Mask: 0x01)                            */
#define PWM_CH4SCR_CHIF_Pos               (7U)                     /*!< CHIF (Bit 7)                                          */
#define PWM_CH4SCR_CHIF_Msk               (0x80U)                  /*!< CHIF (Bitfield-Mask: 0x01)                            */
#define PWM_CH4SCR_DMAEN_Pos              (8U)                     /*!< DMAEN (Bit 8)                                         */
#define PWM_CH4SCR_DMAEN_Msk              (0x100U)                 /*!< DMAEN (Bitfield-Mask: 0x01)                           */
/* ========================================================  CH5SCR  ========================================================= */
#define PWM_CH5SCR_CHRSTEN_Pos            (0U)                     /*!< CHRSTEN (Bit 0)                                       */
#define PWM_CH5SCR_CHRSTEN_Msk            (0x1U)                   /*!< CHRSTEN (Bitfield-Mask: 0x01)                         */
#define PWM_CH5SCR_DIR_Pos                (1U)                     /*!< DIR (Bit 1)                                           */
#define PWM_CH5SCR_DIR_Msk                (0x2U)                   /*!< DIR (Bitfield-Mask: 0x01)                             */
#define PWM_CH5SCR_ELSR0_Pos              (2U)                     /*!< ELSR0 (Bit 2)                                         */
#define PWM_CH5SCR_ELSR0_Msk              (0x4U)                   /*!< ELSR0 (Bitfield-Mask: 0x01)                           */
#define PWM_CH5SCR_ELSR1_Pos              (3U)                     /*!< ELSR1 (Bit 3)                                         */
#define PWM_CH5SCR_ELSR1_Msk              (0x8U)                   /*!< ELSR1 (Bitfield-Mask: 0x01)                           */
#define PWM_CH5SCR_MSR0_Pos               (4U)                     /*!< MSR0 (Bit 4)                                          */
#define PWM_CH5SCR_MSR0_Msk               (0x10U)                  /*!< MSR0 (Bitfield-Mask: 0x01)                            */
#define PWM_CH5SCR_MSR1_Pos               (5U)                     /*!< MSR1 (Bit 5)                                          */
#define PWM_CH5SCR_MSR1_Msk               (0x20U)                  /*!< MSR1 (Bitfield-Mask: 0x01)                            */
#define PWM_CH5SCR_CHIE_Pos               (6U)                     /*!< CHIE (Bit 6)                                          */
#define PWM_CH5SCR_CHIE_Msk               (0x40U)                  /*!< CHIE (Bitfield-Mask: 0x01)                            */
#define PWM_CH5SCR_CHIF_Pos               (7U)                     /*!< CHIF (Bit 7)                                          */
#define PWM_CH5SCR_CHIF_Msk               (0x80U)                  /*!< CHIF (Bitfield-Mask: 0x01)                            */
#define PWM_CH5SCR_DMAEN_Pos              (8U)                     /*!< DMAEN (Bit 8)                                         */
#define PWM_CH5SCR_DMAEN_Msk              (0x100U)                 /*!< DMAEN (Bitfield-Mask: 0x01)                           */
/* ========================================================  CH6SCR  ========================================================= */
#define PWM_CH6SCR_CHRSTEN_Pos            (0U)                     /*!< CHRSTEN (Bit 0)                                       */
#define PWM_CH6SCR_CHRSTEN_Msk            (0x1U)                   /*!< CHRSTEN (Bitfield-Mask: 0x01)                         */
#define PWM_CH6SCR_DIR_Pos                (1U)                     /*!< DIR (Bit 1)                                           */
#define PWM_CH6SCR_DIR_Msk                (0x2U)                   /*!< DIR (Bitfield-Mask: 0x01)                             */
#define PWM_CH6SCR_ELSR0_Pos              (2U)                     /*!< ELSR0 (Bit 2)                                         */
#define PWM_CH6SCR_ELSR0_Msk              (0x4U)                   /*!< ELSR0 (Bitfield-Mask: 0x01)                           */
#define PWM_CH6SCR_ELSR1_Pos              (3U)                     /*!< ELSR1 (Bit 3)                                         */
#define PWM_CH6SCR_ELSR1_Msk              (0x8U)                   /*!< ELSR1 (Bitfield-Mask: 0x01)                           */
#define PWM_CH6SCR_MSR0_Pos               (4U)                     /*!< MSR0 (Bit 4)                                          */
#define PWM_CH6SCR_MSR0_Msk               (0x10U)                  /*!< MSR0 (Bitfield-Mask: 0x01)                            */
#define PWM_CH6SCR_MSR1_Pos               (5U)                     /*!< MSR1 (Bit 5)                                          */
#define PWM_CH6SCR_MSR1_Msk               (0x20U)                  /*!< MSR1 (Bitfield-Mask: 0x01)                            */
#define PWM_CH6SCR_CHIE_Pos               (6U)                     /*!< CHIE (Bit 6)                                          */
#define PWM_CH6SCR_CHIE_Msk               (0x40U)                  /*!< CHIE (Bitfield-Mask: 0x01)                            */
#define PWM_CH6SCR_CHIF_Pos               (7U)                     /*!< CHIF (Bit 7)                                          */
#define PWM_CH6SCR_CHIF_Msk               (0x80U)                  /*!< CHIF (Bitfield-Mask: 0x01)                            */
#define PWM_CH6SCR_DMAEN_Pos              (8U)                     /*!< DMAEN (Bit 8)                                         */
#define PWM_CH6SCR_DMAEN_Msk              (0x100U)                 /*!< DMAEN (Bitfield-Mask: 0x01)                           */
/* ========================================================  CH7SCR  ========================================================= */
#define PWM_CH7SCR_CHRSTEN_Pos            (0U)                     /*!< CHRSTEN (Bit 0)                                       */
#define PWM_CH7SCR_CHRSTEN_Msk            (0x1U)                   /*!< CHRSTEN (Bitfield-Mask: 0x01)                         */
#define PWM_CH7SCR_DIR_Pos                (1U)                     /*!< DIR (Bit 1)                                           */
#define PWM_CH7SCR_DIR_Msk                (0x2U)                   /*!< DIR (Bitfield-Mask: 0x01)                             */
#define PWM_CH7SCR_ELSR0_Pos              (2U)                     /*!< ELSR0 (Bit 2)                                         */
#define PWM_CH7SCR_ELSR0_Msk              (0x4U)                   /*!< ELSR0 (Bitfield-Mask: 0x01)                           */
#define PWM_CH7SCR_ELSR1_Pos              (3U)                     /*!< ELSR1 (Bit 3)                                         */
#define PWM_CH7SCR_ELSR1_Msk              (0x8U)                   /*!< ELSR1 (Bitfield-Mask: 0x01)                           */
#define PWM_CH7SCR_MSR0_Pos               (4U)                     /*!< MSR0 (Bit 4)                                          */
#define PWM_CH7SCR_MSR0_Msk               (0x10U)                  /*!< MSR0 (Bitfield-Mask: 0x01)                            */
#define PWM_CH7SCR_MSR1_Pos               (5U)                     /*!< MSR1 (Bit 5)                                          */
#define PWM_CH7SCR_MSR1_Msk               (0x20U)                  /*!< MSR1 (Bitfield-Mask: 0x01)                            */
#define PWM_CH7SCR_CHIE_Pos               (6U)                     /*!< CHIE (Bit 6)                                          */
#define PWM_CH7SCR_CHIE_Msk               (0x40U)                  /*!< CHIE (Bitfield-Mask: 0x01)                            */
#define PWM_CH7SCR_CHIF_Pos               (7U)                     /*!< CHIF (Bit 7)                                          */
#define PWM_CH7SCR_CHIF_Msk               (0x80U)                  /*!< CHIF (Bitfield-Mask: 0x01)                            */
#define PWM_CH7SCR_DMAEN_Pos              (8U)                     /*!< DMAEN (Bit 8)                                         */
#define PWM_CH7SCR_DMAEN_Msk              (0x100U)                 /*!< DMAEN (Bitfield-Mask: 0x01)                           */
/* =========================================================  CH0V  ========================================================== */
#define PWM_CH0V_CHCVAL_Pos               (0U)                     /*!< CHCVAL (Bit 0)                                        */
#define PWM_CH0V_CHCVAL_Msk               (0xffffU)                /*!< CHCVAL (Bitfield-Mask: 0xffff)                        */
/* =========================================================  CH1V  ========================================================== */
#define PWM_CH1V_CHCVAL_Pos               (0U)                     /*!< CHCVAL (Bit 0)                                        */
#define PWM_CH1V_CHCVAL_Msk               (0xffffU)                /*!< CHCVAL (Bitfield-Mask: 0xffff)                        */
/* =========================================================  CH2V  ========================================================== */
#define PWM_CH2V_CHCVAL_Pos               (0U)                     /*!< CHCVAL (Bit 0)                                        */
#define PWM_CH2V_CHCVAL_Msk               (0xffffU)                /*!< CHCVAL (Bitfield-Mask: 0xffff)                        */
/* =========================================================  CH3V  ========================================================== */
#define PWM_CH3V_CHCVAL_Pos               (0U)                     /*!< CHCVAL (Bit 0)                                        */
#define PWM_CH3V_CHCVAL_Msk               (0xffffU)                /*!< CHCVAL (Bitfield-Mask: 0xffff)                        */
/* =========================================================  CH4V  ========================================================== */
#define PWM_CH4V_CHCVAL_Pos               (0U)                     /*!< CHCVAL (Bit 0)                                        */
#define PWM_CH4V_CHCVAL_Msk               (0xffffU)                /*!< CHCVAL (Bitfield-Mask: 0xffff)                        */
/* =========================================================  CH5V  ========================================================== */
#define PWM_CH5V_CHCVAL_Pos               (0U)                     /*!< CHCVAL (Bit 0)                                        */
#define PWM_CH5V_CHCVAL_Msk               (0xffffU)                /*!< CHCVAL (Bitfield-Mask: 0xffff)                        */
/* =========================================================  CH6V  ========================================================== */
#define PWM_CH6V_CHCVAL_Pos               (0U)                     /*!< CHCVAL (Bit 0)                                        */
#define PWM_CH6V_CHCVAL_Msk               (0xffffU)                /*!< CHCVAL (Bitfield-Mask: 0xffff)                        */
/* =========================================================  CH7V  ========================================================== */
#define PWM_CH7V_CHCVAL_Pos               (0U)                     /*!< CHCVAL (Bit 0)                                        */
#define PWM_CH7V_CHCVAL_Msk               (0xffffU)                /*!< CHCVAL (Bitfield-Mask: 0xffff)                        */
/* =========================================================  CNTIN  ========================================================= */
#define PWM_CNTIN_CNTINIT_Pos             (0U)                     /*!< CNTINIT (Bit 0)                                       */
#define PWM_CNTIN_CNTINIT_Msk             (0xffffU)                /*!< CNTINIT (Bitfield-Mask: 0xffff)                       */
/* ==========================================================  STR  ========================================================== */
#define PWM_STR_CH0SF_Pos                 (0U)                     /*!< CH0SF (Bit 0)                                         */
#define PWM_STR_CH0SF_Msk                 (0x1U)                   /*!< CH0SF (Bitfield-Mask: 0x01)                           */
#define PWM_STR_CH1SF_Pos                 (1U)                     /*!< CH1SF (Bit 1)                                         */
#define PWM_STR_CH1SF_Msk                 (0x2U)                   /*!< CH1SF (Bitfield-Mask: 0x01)                           */
#define PWM_STR_CH2SF_Pos                 (2U)                     /*!< CH2SF (Bit 2)                                         */
#define PWM_STR_CH2SF_Msk                 (0x4U)                   /*!< CH2SF (Bitfield-Mask: 0x01)                           */
#define PWM_STR_CH3SF_Pos                 (3U)                     /*!< CH3SF (Bit 3)                                         */
#define PWM_STR_CH3SF_Msk                 (0x8U)                   /*!< CH3SF (Bitfield-Mask: 0x01)                           */
#define PWM_STR_CH4SF_Pos                 (4U)                     /*!< CH4SF (Bit 4)                                         */
#define PWM_STR_CH4SF_Msk                 (0x10U)                  /*!< CH4SF (Bitfield-Mask: 0x01)                           */
#define PWM_STR_CH5SF_Pos                 (5U)                     /*!< CH5SF (Bit 5)                                         */
#define PWM_STR_CH5SF_Msk                 (0x20U)                  /*!< CH5SF (Bitfield-Mask: 0x01)                           */
#define PWM_STR_CH6SF_Pos                 (6U)                     /*!< CH6SF (Bit 6)                                         */
#define PWM_STR_CH6SF_Msk                 (0x40U)                  /*!< CH6SF (Bitfield-Mask: 0x01)                           */
#define PWM_STR_CH7SF_Pos                 (7U)                     /*!< CH7SF (Bit 7)                                         */
#define PWM_STR_CH7SF_Msk                 (0x80U)                  /*!< CH7SF (Bitfield-Mask: 0x01)                           */
#define PWM_STR_CH0STS_Pos                (16U)                    /*!< CH0STS (Bit 16)                                       */
#define PWM_STR_CH0STS_Msk                (0x10000U)               /*!< CH0STS (Bitfield-Mask: 0x01)                          */
#define PWM_STR_CH1STS_Pos                (17U)                    /*!< CH1STS (Bit 17)                                       */
#define PWM_STR_CH1STS_Msk                (0x20000U)               /*!< CH1STS (Bitfield-Mask: 0x01)                          */
#define PWM_STR_CH2STS_Pos                (18U)                    /*!< CH2STS (Bit 18)                                       */
#define PWM_STR_CH2STS_Msk                (0x40000U)               /*!< CH2STS (Bitfield-Mask: 0x01)                          */
#define PWM_STR_CH3STS_Pos                (19U)                    /*!< CH3STS (Bit 19)                                       */
#define PWM_STR_CH3STS_Msk                (0x80000U)               /*!< CH3STS (Bitfield-Mask: 0x01)                          */
#define PWM_STR_CH4STS_Pos                (20U)                    /*!< CH4STS (Bit 20)                                       */
#define PWM_STR_CH4STS_Msk                (0x100000U)              /*!< CH4STS (Bitfield-Mask: 0x01)                          */
#define PWM_STR_CH5STS_Pos                (21U)                    /*!< CH5STS (Bit 21)                                       */
#define PWM_STR_CH5STS_Msk                (0x200000U)              /*!< CH5STS (Bitfield-Mask: 0x01)                          */
#define PWM_STR_CH6STS_Pos                (22U)                    /*!< CH6STS (Bit 22)                                       */
#define PWM_STR_CH6STS_Msk                (0x400000U)              /*!< CH6STS (Bitfield-Mask: 0x01)                          */
#define PWM_STR_CH7STS_Pos                (23U)                    /*!< CH7STS (Bit 23)                                       */
#define PWM_STR_CH7STS_Msk                (0x800000U)              /*!< CH7STS (Bitfield-Mask: 0x01)                          */
#define PWM_STR_HALL_STATUS_Pos           (24U)                    /*!< HALL_STATUS (Bit 24)                                  */
#define PWM_STR_HALL_STATUS_Msk           (0x7000000U)             /*!< HALL_STATUS (Bitfield-Mask: 0x07)                     */
#define PWM_STR_CH_EVENT_Msk              (0xFFU)                  /*!< PWM EVENT STATUS                                      */
#define PWM_STR_CH_INPUT_Msk              (0xFF0000U)              /*!< PWM EVENT STATUS                                      */
/* ========================================================  FUNCSEL  ======================================================== */
#define PWM_FUNCSEL_PWMSYNCEN_Pos         (0U)                     /*!< PWMSYNCEN (Bit 0)                                     */
#define PWM_FUNCSEL_PWMSYNCEN_Msk         (0x1U)                   /*!< PWMSYNCEN (Bitfield-Mask: 0x01)                       */
#define PWM_FUNCSEL_INIT_Pos              (1U)                     /*!< INIT (Bit 1)                                          */
#define PWM_FUNCSEL_INIT_Msk              (0x2U)                   /*!< INIT (Bitfield-Mask: 0x01)                            */
#define PWM_FUNCSEL_WPDIS_Pos             (2U)                     /*!< WPDIS (Bit 2)                                         */
#define PWM_FUNCSEL_WPDIS_Msk             (0x4U)                   /*!< WPDIS (Bitfield-Mask: 0x01)                           */
#define PWM_FUNCSEL_PWMSYNC_Pos           (3U)                     /*!< PWMSYNC (Bit 3)                                       */
#define PWM_FUNCSEL_PWMSYNC_Msk           (0x8U)                   /*!< PWMSYNC (Bitfield-Mask: 0x01)                         */
#define PWM_FUNCSEL_FAULTMODE_Pos         (5U)                     /*!< FAULTMODE (Bit 5)                                     */
#define PWM_FUNCSEL_FAULTMODE_Msk         (0x60U)                  /*!< FAULTMODE (Bitfield-Mask: 0x03)                       */
#define PWM_FUNCSEL_FAULTIE_Pos           (7U)                     /*!< FAULTIE (Bit 7)                                       */
#define PWM_FUNCSEL_FAULTIE_Msk           (0x80U)                  /*!< FAULTIE (Bitfield-Mask: 0x01)                         */
#define PWM_FUNCSEL_FAULTHIZEN_Pos        (8U)                     /*!< FAULTHIZEN (Bit 8)                                    */
#define PWM_FUNCSEL_FAULTHIZEN_Msk        (0x100U)                 /*!< FAULTHIZEN (Bitfield-Mask: 0x01)                      */
#define PWM_FUNCSEL_HALLEN_Pos            (9U)                     /*!< HALLEN (Bit 9)                                        */
#define PWM_FUNCSEL_HALLEN_Msk            (0x200U)                 /*!< HALLEN (Bitfield-Mask: 0x01)                          */
#define PWM_FUNCSEL_CHP0PNWEN_Pos         (12U)                    /*!< CHP0PNWEN (Bit 12)                                    */
#define PWM_FUNCSEL_CHP0PNWEN_Msk         (0x1000U)                /*!< CHP0PNWEN (Bitfield-Mask: 0x01)                       */
#define PWM_FUNCSEL_CHP1PNWEN_Pos         (13U)                    /*!< CHP1PNWEN (Bit 13)                                    */
#define PWM_FUNCSEL_CHP1PNWEN_Msk         (0x2000U)                /*!< CHP1PNWEN (Bitfield-Mask: 0x01)                       */
#define PWM_FUNCSEL_CHP2PNWEN_Pos         (14U)                    /*!< CHP2PNWEN (Bit 14)                                    */
#define PWM_FUNCSEL_CHP2PNWEN_Msk         (0x4000U)                /*!< CHP2PNWEN (Bitfield-Mask: 0x01)                       */
#define PWM_FUNCSEL_CHP3PNWEN_Pos         (15U)                    /*!< CHP3PNWEN (Bit 15)                                    */
#define PWM_FUNCSEL_CHP3PNWEN_Msk         (0x8000U)                /*!< CHP3PNWEN (Bitfield-Mask: 0x01)                       */
#define PWM_FUNCSEL_CH0HIZEN_Pos          (16U)                    /*!< CH0HIZEN (Bit 16)                                     */
#define PWM_FUNCSEL_CH0HIZEN_Msk          (0x10000U)               /*!< CH0HIZEN (Bitfield-Mask: 0x01)                        */
#define PWM_FUNCSEL_CH1HIZEN_Pos          (17U)                    /*!< CH1HIZEN (Bit 17)                                     */
#define PWM_FUNCSEL_CH1HIZEN_Msk          (0x20000U)               /*!< CH1HIZEN (Bitfield-Mask: 0x01)                        */
#define PWM_FUNCSEL_CH2HIZEN_Pos          (18U)                    /*!< CH2HIZEN (Bit 18)                                     */
#define PWM_FUNCSEL_CH2HIZEN_Msk          (0x40000U)               /*!< CH2HIZEN (Bitfield-Mask: 0x01)                        */
#define PWM_FUNCSEL_CH3HIZEN_Pos          (19U)                    /*!< CH3HIZEN (Bit 19)                                     */
#define PWM_FUNCSEL_CH3HIZEN_Msk          (0x80000U)               /*!< CH3HIZEN (Bitfield-Mask: 0x01)                        */
#define PWM_FUNCSEL_CH4HIZEN_Pos          (20U)                    /*!< CH4HIZEN (Bit 20)                                     */
#define PWM_FUNCSEL_CH4HIZEN_Msk          (0x100000U)              /*!< CH4HIZEN (Bitfield-Mask: 0x01)                        */
#define PWM_FUNCSEL_CH5HIZEN_Pos          (21U)                    /*!< CH5HIZEN (Bit 21)                                     */
#define PWM_FUNCSEL_CH5HIZEN_Msk          (0x200000U)              /*!< CH5HIZEN (Bitfield-Mask: 0x01)                        */
#define PWM_FUNCSEL_CH6HIZEN_Pos          (22U)                    /*!< CH6HIZEN (Bit 22)                                     */
#define PWM_FUNCSEL_CH6HIZEN_Msk          (0x400000U)              /*!< CH6HIZEN (Bitfield-Mask: 0x01)                        */
#define PWM_FUNCSEL_CH7HIZEN_Pos          (23U)                    /*!< CH7HIZEN (Bit 23)                                     */
#define PWM_FUNCSEL_CH7HIZEN_Msk          (0x800000U)              /*!< CH7HIZEN (Bitfield-Mask: 0x01)                        */
#define PWM_FUNCSEL_PDYTPEN_Pos           (24U)                    /*!< PDYTPEN (Bit 24)                                      */
#define PWM_FUNCSEL_PDYTPEN_Msk           (0x1000000U)             /*!< PDYTPEN (Bitfield-Mask: 0x01)                         */
#define PWM_FUNCSEL_FAULTCTRLEN_Pos       (25U)                    /*!< FAULTCTRLEN (Bit 25)                                  */
#define PWM_FUNCSEL_FAULTCTRLEN_Msk       (0x2000000U)             /*!< FAULTCTRLEN (Bitfield-Mask: 0x01)                     */
/* =========================================================  SYNC  ========================================================== */
#define PWM_SYNC_MINSYNCP_Pos             (0U)                     /*!< MINSYNCP (Bit 0)                                      */
#define PWM_SYNC_MINSYNCP_Msk             (0x1U)                   /*!< MINSYNCP (Bitfield-Mask: 0x01)                        */
#define PWM_SYNC_MAXSYNCP_Pos             (1U)                     /*!< MAXSYNCP (Bit 1)                                      */
#define PWM_SYNC_MAXSYNCP_Msk             (0x2U)                   /*!< MAXSYNCP (Bitfield-Mask: 0x01)                        */
#define PWM_SYNC_OMSYNCP_Pos              (3U)                     /*!< OMSYNCP (Bit 3)                                       */
#define PWM_SYNC_OMSYNCP_Msk              (0x8U)                   /*!< OMSYNCP (Bitfield-Mask: 0x01)                         */
#define PWM_SYNC_TRIG0_Pos                (4U)                     /*!< TRIG0 (Bit 4)                                         */
#define PWM_SYNC_TRIG0_Msk                (0x10U)                  /*!< TRIG0 (Bitfield-Mask: 0x01)                           */
#define PWM_SYNC_TRIG1_Pos                (5U)                     /*!< TRIG1 (Bit 5)                                         */
#define PWM_SYNC_TRIG1_Msk                (0x20U)                  /*!< TRIG1 (Bitfield-Mask: 0x01)                           */
#define PWM_SYNC_TRIG2_Pos                (6U)                     /*!< TRIG2 (Bit 6)                                         */
#define PWM_SYNC_TRIG2_Msk                (0x40U)                  /*!< TRIG2 (Bitfield-Mask: 0x01)                           */
#define PWM_SYNC_SWSYNC_Pos               (7U)                     /*!< SWSYNC (Bit 7)                                        */
#define PWM_SYNC_SWSYNC_Msk               (0x80U)                  /*!< SWSYNC (Bitfield-Mask: 0x01)                          */
#define PWM_SYNC_PWM_SYNCPOL_Pos          (11U)                    /*!< PWM_SYNCPOL (Bit 11)                                  */
#define PWM_SYNC_PWM_SYNCPOL_Msk          (0x800U)                 /*!< PWM_SYNCPOL (Bitfield-Mask: 0x01)                     */
#define PWM_SYNC_PWM_SYNC_BYPEN_Pos       (16U)                    /*!< PWM_SYNC_BYPEN (Bit 16)                               */
#define PWM_SYNC_PWM_SYNC_BYPEN_Msk       (0x10000U)               /*!< PWM_SYNC_BYPEN (Bitfield-Mask: 0x01)                  */
#define PWM_SYNC_PWM_BFRGCHSEN_Pos        (17U)                    /*!< PWM_BFRGCHSEN (Bit 17)                                */
#define PWM_SYNC_PWM_BFRGCHSEN_Msk        (0x20000U)               /*!< PWM_BFRGCHSEN (Bitfield-Mask: 0x01)                   */
/* ========================================================  OUTINIT  ======================================================== */
#define PWM_OUTINIT_CH0OIV_Pos            (0U)                     /*!< CH0OIV (Bit 0)                                        */
#define PWM_OUTINIT_CH0OIV_Msk            (0x1U)                   /*!< CH0OIV (Bitfield-Mask: 0x01)                          */
#define PWM_OUTINIT_CH1OIV_Pos            (1U)                     /*!< CH1OIV (Bit 1)                                        */
#define PWM_OUTINIT_CH1OIV_Msk            (0x2U)                   /*!< CH1OIV (Bitfield-Mask: 0x01)                          */
#define PWM_OUTINIT_CH2OIV_Pos            (2U)                     /*!< CH2OIV (Bit 2)                                        */
#define PWM_OUTINIT_CH2OIV_Msk            (0x4U)                   /*!< CH2OIV (Bitfield-Mask: 0x01)                          */
#define PWM_OUTINIT_CH3OIV_Pos            (3U)                     /*!< CH3OIV (Bit 3)                                        */
#define PWM_OUTINIT_CH3OIV_Msk            (0x8U)                   /*!< CH3OIV (Bitfield-Mask: 0x01)                          */
#define PWM_OUTINIT_CH4OIV_Pos            (4U)                     /*!< CH4OIV (Bit 4)                                        */
#define PWM_OUTINIT_CH4OIV_Msk            (0x10U)                  /*!< CH4OIV (Bitfield-Mask: 0x01)                          */
#define PWM_OUTINIT_CH5OIV_Pos            (5U)                     /*!< CH5OIV (Bit 5)                                        */
#define PWM_OUTINIT_CH5OIV_Msk            (0x20U)                  /*!< CH5OIV (Bitfield-Mask: 0x01)                          */
#define PWM_OUTINIT_CH6OIV_Pos            (6U)                     /*!< CH6OIV (Bit 6)                                        */
#define PWM_OUTINIT_CH6OIV_Msk            (0x40U)                  /*!< CH6OIV (Bitfield-Mask: 0x01)                          */
#define PWM_OUTINIT_CH7OIV_Pos            (7U)                     /*!< CH7OIV (Bit 7)                                        */
#define PWM_OUTINIT_CH7OIV_Msk            (0x80U)                  /*!< CH7OIV (Bitfield-Mask: 0x01)                          */
/* =========================================================  OMCR  ========================================================== */
#define PWM_OMCR_CH0OMEN_Pos              (0U)                     /*!< CH0OMEN (Bit 0)                                       */
#define PWM_OMCR_CH0OMEN_Msk              (0x1U)                   /*!< CH0OMEN (Bitfield-Mask: 0x01)                         */
#define PWM_OMCR_CH1OMEN_Pos              (1U)                     /*!< CH1OMEN (Bit 1)                                       */
#define PWM_OMCR_CH1OMEN_Msk              (0x2U)                   /*!< CH1OMEN (Bitfield-Mask: 0x01)                         */
#define PWM_OMCR_CH2OMEN_Pos              (2U)                     /*!< CH2OMEN (Bit 2)                                       */
#define PWM_OMCR_CH2OMEN_Msk              (0x4U)                   /*!< CH2OMEN (Bitfield-Mask: 0x01)                         */
#define PWM_OMCR_CH3OMEN_Pos              (3U)                     /*!< CH3OMEN (Bit 3)                                       */
#define PWM_OMCR_CH3OMEN_Msk              (0x8U)                   /*!< CH3OMEN (Bitfield-Mask: 0x01)                         */
#define PWM_OMCR_CH4OMEN_Pos              (4U)                     /*!< CH4OMEN (Bit 4)                                       */
#define PWM_OMCR_CH4OMEN_Msk              (0x10U)                  /*!< CH4OMEN (Bitfield-Mask: 0x01)                         */
#define PWM_OMCR_CH5OMEN_Pos              (5U)                     /*!< CH5OMEN (Bit 5)                                       */
#define PWM_OMCR_CH5OMEN_Msk              (0x20U)                  /*!< CH5OMEN (Bitfield-Mask: 0x01)                         */
#define PWM_OMCR_CH6OMEN_Pos              (6U)                     /*!< CH6OMEN (Bit 6)                                       */
#define PWM_OMCR_CH6OMEN_Msk              (0x40U)                  /*!< CH6OMEN (Bitfield-Mask: 0x01)                         */
#define PWM_OMCR_CH7OMEN_Pos              (7U)                     /*!< CH7OMEN (Bit 7)                                       */
#define PWM_OMCR_CH7OMEN_Msk              (0x80U)                  /*!< CH7OMEN (Bitfield-Mask: 0x01)                         */
#define PWM_OMCR_CH_Msk                   (0xFFU)                  /*!< ALL CH Mask                                           */
/* ========================================================  MODESEL  ======================================================== */
#define PWM_MODESEL_PAIR0COMBINEN_Pos     (0U)                     /*!< PAIR0COMBINEN (Bit 0)                                 */
#define PWM_MODESEL_PAIR0COMBINEN_Msk     (0x1U)                   /*!< PAIR0COMBINEN (Bitfield-Mask: 0x01)                   */
#define PWM_MODESEL_PAIR0COMPEN_Pos       (1U)                     /*!< PAIR0COMPEN (Bit 1)                                   */
#define PWM_MODESEL_PAIR0COMPEN_Msk       (0x2U)                   /*!< PAIR0COMPEN (Bitfield-Mask: 0x01)                     */
#define PWM_MODESEL_PAIR0DECAPEN_Pos      (2U)                     /*!< PAIR0DECAPEN (Bit 2)                                  */
#define PWM_MODESEL_PAIR0DECAPEN_Msk      (0x4U)                   /*!< PAIR0DECAPEN (Bitfield-Mask: 0x01)                    */
#define PWM_MODESEL_PAIR0DECAP_Pos        (3U)                     /*!< PAIR0DECAP (Bit 3)                                    */
#define PWM_MODESEL_PAIR0DECAP_Msk        (0x8U)                   /*!< PAIR0DECAP (Bitfield-Mask: 0x01)                      */
#define PWM_MODESEL_PAIR0DTEN_Pos         (4U)                     /*!< PAIR0DTEN (Bit 4)                                     */
#define PWM_MODESEL_PAIR0DTEN_Msk         (0x10U)                  /*!< PAIR0DTEN (Bitfield-Mask: 0x01)                       */
#define PWM_MODESEL_PAIR0SYNCEN_Pos       (5U)                     /*!< PAIR0SYNCEN (Bit 5)                                   */
#define PWM_MODESEL_PAIR0SYNCEN_Msk       (0x20U)                  /*!< PAIR0SYNCEN (Bitfield-Mask: 0x01)                     */
#define PWM_MODESEL_PAIR0FAULTEN_Pos      (6U)                     /*!< PAIR0FAULTEN (Bit 6)                                  */
#define PWM_MODESEL_PAIR0FAULTEN_Msk      (0x40U)                  /*!< PAIR0FAULTEN (Bitfield-Mask: 0x01)                    */
#define PWM_MODESEL_PAIR0SYMEN_Pos        (7U)                     /*!< PAIR0SYMEN (Bit 7)                                    */
#define PWM_MODESEL_PAIR0SYMEN_Msk        (0x80U)                  /*!< PAIR0SYMEN (Bitfield-Mask: 0x01)                      */
#define PWM_MODESEL_PAIR1COMBINEN_Pos     (8U)                     /*!< PAIR1COMBINEN (Bit 8)                                 */
#define PWM_MODESEL_PAIR1COMBINEN_Msk     (0x100U)                 /*!< PAIR1COMBINEN (Bitfield-Mask: 0x01)                   */
#define PWM_MODESEL_PAIR1COMPEN_Pos       (9U)                     /*!< PAIR1COMPEN (Bit 9)                                   */
#define PWM_MODESEL_PAIR1COMPEN_Msk       (0x200U)                 /*!< PAIR1COMPEN (Bitfield-Mask: 0x01)                     */
#define PWM_MODESEL_PAIR1DECAPEN_Pos      (10U)                    /*!< PAIR1DECAPEN (Bit 10)                                 */
#define PWM_MODESEL_PAIR1DECAPEN_Msk      (0x400U)                 /*!< PAIR1DECAPEN (Bitfield-Mask: 0x01)                    */
#define PWM_MODESEL_PAIR1DECAP_Pos        (11U)                    /*!< PAIR1DECAP (Bit 11)                                   */
#define PWM_MODESEL_PAIR1DECAP_Msk        (0x800U)                 /*!< PAIR1DECAP (Bitfield-Mask: 0x01)                      */
#define PWM_MODESEL_PAIR1DTEN_Pos         (12U)                    /*!< PAIR1DTEN (Bit 12)                                    */
#define PWM_MODESEL_PAIR1DTEN_Msk         (0x1000U)                /*!< PAIR1DTEN (Bitfield-Mask: 0x01)                       */
#define PWM_MODESEL_PAIR1SYNCEN_Pos       (13U)                    /*!< PAIR1SYNCEN (Bit 13)                                  */
#define PWM_MODESEL_PAIR1SYNCEN_Msk       (0x2000U)                /*!< PAIR1SYNCEN (Bitfield-Mask: 0x01)                     */
#define PWM_MODESEL_PAIR1FAULTEN_Pos      (14U)                    /*!< PAIR1FAULTEN (Bit 14)                                 */
#define PWM_MODESEL_PAIR1FAULTEN_Msk      (0x4000U)                /*!< PAIR1FAULTEN (Bitfield-Mask: 0x01)                    */
#define PWM_MODESEL_PAIR1SYMEN_Pos        (15U)                    /*!< PAIR1SYMEN (Bit 15)                                   */
#define PWM_MODESEL_PAIR1SYMEN_Msk        (0x8000U)                /*!< PAIR1SYMEN (Bitfield-Mask: 0x01)                      */
#define PWM_MODESEL_PAIR2COMBINEN_Pos     (16U)                    /*!< PAIR2COMBINEN (Bit 16)                                */
#define PWM_MODESEL_PAIR2COMBINEN_Msk     (0x10000U)               /*!< PAIR2COMBINEN (Bitfield-Mask: 0x01)                   */
#define PWM_MODESEL_PAIR2COMPEN_Pos       (17U)                    /*!< PAIR2COMPEN (Bit 17)                                  */
#define PWM_MODESEL_PAIR2COMPEN_Msk       (0x20000U)               /*!< PAIR2COMPEN (Bitfield-Mask: 0x01)                     */
#define PWM_MODESEL_PAIR2DECAPEN_Pos      (18U)                    /*!< PAIR2DECAPEN (Bit 18)                                 */
#define PWM_MODESEL_PAIR2DECAPEN_Msk      (0x40000U)               /*!< PAIR2DECAPEN (Bitfield-Mask: 0x01)                    */
#define PWM_MODESEL_PAIR2DECAP_Pos        (19U)                    /*!< PAIR2DECAP (Bit 19)                                   */
#define PWM_MODESEL_PAIR2DECAP_Msk        (0x80000U)               /*!< PAIR2DECAP (Bitfield-Mask: 0x01)                      */
#define PWM_MODESEL_PAIR2DTEN_Pos         (20U)                    /*!< PAIR2DTEN (Bit 20)                                    */
#define PWM_MODESEL_PAIR2DTEN_Msk         (0x100000U)              /*!< PAIR2DTEN (Bitfield-Mask: 0x01)                       */
#define PWM_MODESEL_PAIR2SYNCEN_Pos       (21U)                    /*!< PAIR2SYNCEN (Bit 21)                                  */
#define PWM_MODESEL_PAIR2SYNCEN_Msk       (0x200000U)              /*!< PAIR2SYNCEN (Bitfield-Mask: 0x01)                     */
#define PWM_MODESEL_PAIR2FAULTEN_Pos      (22U)                    /*!< PAIR2FAULTEN (Bit 22)                                 */
#define PWM_MODESEL_PAIR2FAULTEN_Msk      (0x400000U)              /*!< PAIR2FAULTEN (Bitfield-Mask: 0x01)                    */
#define PWM_MODESEL_PAIR2SYMEN_Pos        (23U)                    /*!< PAIR2SYMEN (Bit 23)                                   */
#define PWM_MODESEL_PAIR2SYMEN_Msk        (0x800000U)              /*!< PAIR2SYMEN (Bitfield-Mask: 0x01)                      */
#define PWM_MODESEL_PAIR3COMBINEN_Pos     (24U)                    /*!< PAIR3COMBINEN (Bit 24)                                */
#define PWM_MODESEL_PAIR3COMBINEN_Msk     (0x1000000U)             /*!< PAIR3COMBINEN (Bitfield-Mask: 0x01)                   */
#define PWM_MODESEL_PAIR3COMPEN_Pos       (25U)                    /*!< PAIR3COMPEN (Bit 25)                                  */
#define PWM_MODESEL_PAIR3COMPEN_Msk       (0x2000000U)             /*!< PAIR3COMPEN (Bitfield-Mask: 0x01)                     */
#define PWM_MODESEL_PAIR3DECAPEN_Pos      (26U)                    /*!< PAIR3DECAPEN (Bit 26)                                 */
#define PWM_MODESEL_PAIR3DECAPEN_Msk      (0x4000000U)             /*!< PAIR3DECAPEN (Bitfield-Mask: 0x01)                    */
#define PWM_MODESEL_PAIR3DECAP_Pos        (27U)                    /*!< PAIR3DECAP (Bit 27)                                   */
#define PWM_MODESEL_PAIR3DECAP_Msk        (0x8000000U)             /*!< PAIR3DECAP (Bitfield-Mask: 0x01)                      */
#define PWM_MODESEL_PAIR3DTEN_Pos         (28U)                    /*!< PAIR3DTEN (Bit 28)                                    */
#define PWM_MODESEL_PAIR3DTEN_Msk         (0x10000000U)            /*!< PAIR3DTEN (Bitfield-Mask: 0x01)                       */
#define PWM_MODESEL_PAIR3SYNCEN_Pos       (29U)                    /*!< PAIR3SYNCEN (Bit 29)                                  */
#define PWM_MODESEL_PAIR3SYNCEN_Msk       (0x20000000U)            /*!< PAIR3SYNCEN (Bitfield-Mask: 0x01)                     */
#define PWM_MODESEL_PAIR3FAULTEN_Pos      (30U)                    /*!< PAIR3FAULTEN (Bit 30)                                 */
#define PWM_MODESEL_PAIR3FAULTEN_Msk      (0x40000000U)            /*!< PAIR3FAULTEN (Bitfield-Mask: 0x01)                    */
#define PWM_MODESEL_PAIR3SYMEN_Pos        (31U)                    /*!< PAIR3SYMEN (Bit 31)                                   */
#define PWM_MODESEL_PAIR3SYMEN_Msk        (0x80000000U)            /*!< PAIR3SYMEN (Bitfield-Mask: 0x01)                      */
#define PWM_COMBINE_PAIR_CONFIG_WIDTH     (8U)                     /*!< PAIRCONFIG                                            */
/* ========================================================  DTSET0  ========================================================= */
#define PWM_DTSET0_DTVAL0_Pos             (0U)                     /*!< DTVAL0 (Bit 0)                                        */
#define PWM_DTSET0_DTVAL0_Msk             (0x3ffU)                 /*!< DTVAL0 (Bitfield-Mask: 0x3ff)                         */
#define PWM_DTSET0_DTPSC0_Pos             (10U)                    /*!< DTPSC0 (Bit 10)                                       */
#define PWM_DTSET0_DTPSC0_Msk             (0xc00U)                 /*!< DTPSC0 (Bitfield-Mask: 0x03)                          */
#define PWM_DTSET0_DTVAL1_Pos             (16U)                    /*!< DTVAL1 (Bit 16)                                       */
#define PWM_DTSET0_DTVAL1_Msk             (0x3ff0000U)             /*!< DTVAL1 (Bitfield-Mask: 0x3ff)                         */
#define PWM_DTSET0_DTPSC1_Pos             (26U)                    /*!< DTPSC1 (Bit 26)                                       */
#define PWM_DTSET0_DTPSC1_Msk             (0xc000000U)             /*!< DTPSC1 (Bitfield-Mask: 0x03)                          */
#define PWM_DTSET_CONFIG_WIDTH            (16U)                    /*!< deadtime Config Width                                 */
/* ========================================================  DTSET1  ========================================================= */
#define PWM_DTSET1_DTVAL2_Pos             (0U)                     /*!< DTVAL2 (Bit 0)                                        */
#define PWM_DTSET1_DTVAL2_Msk             (0x3ffU)                 /*!< DTVAL2 (Bitfield-Mask: 0x3ff)                         */
#define PWM_DTSET1_DTPS2_Pos              (10U)                    /*!< DTPS2 (Bit 10)                                        */
#define PWM_DTSET1_DTPS2_Msk              (0xc00U)                 /*!< DTPS2 (Bitfield-Mask: 0x03)                           */
#define PWM_DTSET1_DTVAL3_Pos             (16U)                    /*!< DTVAL3 (Bit 16)                                       */
#define PWM_DTSET1_DTVAL3_Msk             (0x3ff0000U)             /*!< DTVAL3 (Bitfield-Mask: 0x3ff)                         */
#define PWM_DTSET1_DTPS3_Pos              (26U)                    /*!< DTPS3 (Bit 26)                                        */
#define PWM_DTSET1_DTPS3_Msk              (0xc000000U)             /*!< DTPS3 (Bitfield-Mask: 0x03)                           */
/* ========================================================  EXTTRIG  ======================================================== */
#define PWM_EXTTRIG_CH0TRIG_Pos           (0U)                     /*!< CH0TRIG (Bit 0)                                       */
#define PWM_EXTTRIG_CH0TRIG_Msk           (0x1U)                   /*!< CH0TRIG (Bitfield-Mask: 0x01)                         */
#define PWM_EXTTRIG_CH1TRIG_Pos           (1U)                     /*!< CH1TRIG (Bit 1)                                       */
#define PWM_EXTTRIG_CH1TRIG_Msk           (0x2U)                   /*!< CH1TRIG (Bitfield-Mask: 0x01)                         */
#define PWM_EXTTRIG_CH2TRIG_Pos           (2U)                     /*!< CH2TRIG (Bit 2)                                       */
#define PWM_EXTTRIG_CH2TRIG_Msk           (0x4U)                   /*!< CH2TRIG (Bitfield-Mask: 0x01)                         */
#define PWM_EXTTRIG_CH3TRIG_Pos           (3U)                     /*!< CH3TRIG (Bit 3)                                       */
#define PWM_EXTTRIG_CH3TRIG_Msk           (0x8U)                   /*!< CH3TRIG (Bitfield-Mask: 0x01)                         */
#define PWM_EXTTRIG_CH4TRIG_Pos           (4U)                     /*!< CH4TRIG (Bit 4)                                       */
#define PWM_EXTTRIG_CH4TRIG_Msk           (0x10U)                  /*!< CH4TRIG (Bitfield-Mask: 0x01)                         */
#define PWM_EXTTRIG_CH5TRIG_Pos           (5U)                     /*!< CH5TRIG (Bit 5)                                       */
#define PWM_EXTTRIG_CH5TRIG_Msk           (0x20U)                  /*!< CH5TRIG (Bitfield-Mask: 0x01)                         */
#define PWM_EXTTRIG_CH6TRIG_Pos           (6U)                     /*!< CH6TRIG (Bit 6)                                       */
#define PWM_EXTTRIG_CH6TRIG_Msk           (0x40U)                  /*!< CH6TRIG (Bitfield-Mask: 0x01)                         */
#define PWM_EXTTRIG_CH7TRIG_Pos           (7U)                     /*!< CH7TRIG (Bit 7)                                       */
#define PWM_EXTTRIG_CH7TRIG_Msk           (0x80U)                  /*!< CH7TRIG (Bitfield-Mask: 0x01)                         */
#define PWM_EXTTRIG_INITTRIGEN_Pos        (8U)                     /*!< INITTRIGEN (Bit 8)                                    */
#define PWM_EXTTRIG_INITTRIGEN_Msk        (0x100U)                 /*!< INITTRIGEN (Bitfield-Mask: 0x01)                      */
#define PWM_EXTTRIG_TRIGF_Pos             (9U)                     /*!< TRIGF (Bit 9)                                         */
#define PWM_EXTTRIG_TRIGF_Msk             (0x200U)                 /*!< TRIGF (Bitfield-Mask: 0x01)                           */
#define PWM_EXTTRIG_MAXTRIGEN_Pos         (10U)                    /*!< MAXTRIGEN (Bit 10)                                    */
#define PWM_EXTTRIG_MAXTRIGEN_Msk         (0x400U)                 /*!< MAXTRIGEN (Bitfield-Mask: 0x01)                       */
#define PWM_EXTTRIG_TRIGRATIO_Pos         (12U)                    /*!< TRIGRATIO (Bit 12)                                    */
#define PWM_EXTTRIG_TRIGRATIO_Msk         (0x7000U)                /*!< TRIGRATIO (Bitfield-Mask: 0x07)                       */
/* =======================================================  CHOPOLCR  ======================================================== */
#define PWM_CHOPOLCR_CH0POL_Pos           (0U)                     /*!< CH0POL (Bit 0)                                        */
#define PWM_CHOPOLCR_CH0POL_Msk           (0x1U)                   /*!< CH0POL (Bitfield-Mask: 0x01)                          */
#define PWM_CHOPOLCR_CH1POL_Pos           (1U)                     /*!< CH1POL (Bit 1)                                        */
#define PWM_CHOPOLCR_CH1POL_Msk           (0x2U)                   /*!< CH1POL (Bitfield-Mask: 0x01)                          */
#define PWM_CHOPOLCR_CH2POL_Pos           (2U)                     /*!< CH2POL (Bit 2)                                        */
#define PWM_CHOPOLCR_CH2POL_Msk           (0x4U)                   /*!< CH2POL (Bitfield-Mask: 0x01)                          */
#define PWM_CHOPOLCR_CH3POL_Pos           (3U)                     /*!< CH3POL (Bit 3)                                        */
#define PWM_CHOPOLCR_CH3POL_Msk           (0x8U)                   /*!< CH3POL (Bitfield-Mask: 0x01)                          */
#define PWM_CHOPOLCR_CH4POL_Pos           (4U)                     /*!< CH4POL (Bit 4)                                        */
#define PWM_CHOPOLCR_CH4POL_Msk           (0x10U)                  /*!< CH4POL (Bitfield-Mask: 0x01)                          */
#define PWM_CHOPOLCR_CH5POL_Pos           (5U)                     /*!< CH5POL (Bit 5)                                        */
#define PWM_CHOPOLCR_CH5POL_Msk           (0x20U)                  /*!< CH5POL (Bitfield-Mask: 0x01)                          */
#define PWM_CHOPOLCR_CH6POL_Pos           (6U)                     /*!< CH6POL (Bit 6)                                        */
#define PWM_CHOPOLCR_CH6POL_Msk           (0x40U)                  /*!< CH6POL (Bitfield-Mask: 0x01)                          */
#define PWM_CHOPOLCR_CH7POL_Pos           (7U)                     /*!< CH7POL (Bit 7)                                        */
#define PWM_CHOPOLCR_CH7POL_Msk           (0x80U)                  /*!< CH7POL (Bitfield-Mask: 0x01)                          */
/* =========================================================  FDSR  ========================================================== */
#define PWM_FDSR_FAULTDF0_Pos             (0U)                     /*!< FAULTDF0 (Bit 0)                                      */
#define PWM_FDSR_FAULTDF0_Msk             (0x1U)                   /*!< FAULTDF0 (Bitfield-Mask: 0x01)                        */
#define PWM_FDSR_FAULTDF1_Pos             (1U)                     /*!< FAULTDF1 (Bit 1)                                      */
#define PWM_FDSR_FAULTDF1_Msk             (0x2U)                   /*!< FAULTDF1 (Bitfield-Mask: 0x01)                        */
#define PWM_FDSR_FAULTDF2_Pos             (2U)                     /*!< FAULTDF2 (Bit 2)                                      */
#define PWM_FDSR_FAULTDF2_Msk             (0x4U)                   /*!< FAULTDF2 (Bitfield-Mask: 0x01)                        */
#define PWM_FDSR_FAULTDF3_Pos             (3U)                     /*!< FAULTDF3 (Bit 3)                                      */
#define PWM_FDSR_FAULTDF3_Msk             (0x8U)                   /*!< FAULTDF3 (Bitfield-Mask: 0x01)                        */
#define PWM_FDSR_FAULTIN_Pos              (5U)                     /*!< FAULTIN (Bit 5)                                       */
#define PWM_FDSR_FAULTIN_Msk              (0x20U)                  /*!< FAULTIN (Bitfield-Mask: 0x01)                         */
#define PWM_FDSR_WPEN_Pos                 (6U)                     /*!< WPEN (Bit 6)                                          */
#define PWM_FDSR_WPEN_Msk                 (0x40U)                  /*!< WPEN (Bitfield-Mask: 0x01)                            */
#define PWM_FDSR_FAULTDF_Pos              (7U)                     /*!< FAULTDF (Bit 7)                                       */
#define PWM_FDSR_FAULTDF_Msk              (0x80U)                  /*!< FAULTDF (Bitfield-Mask: 0x01)                         */
/* =======================================================  CAPFILTER  ======================================================= */
#define PWM_CAPFILTER_CH0CAPFVAL_Pos      (0U)                     /*!< CH0CAPFVAL (Bit 0)                                    */
#define PWM_CAPFILTER_CH0CAPFVAL_Msk      (0xfU)                   /*!< CH0CAPFVAL (Bitfield-Mask: 0x0f)                      */
#define PWM_CAPFILTER_CH1CAPFVAL_Pos      (5U)                     /*!< CH1CAPFVAL (Bit 5)                                    */
#define PWM_CAPFILTER_CH1CAPFVAL_Msk      (0x1e0U)                 /*!< CH1CAPFVAL (Bitfield-Mask: 0x0f)                      */
#define PWM_CAPFILTER_CH2CAPFVAL_Pos      (10U)                    /*!< CH2CAPFVAL (Bit 10)                                   */
#define PWM_CAPFILTER_CH2CAPFVAL_Msk      (0x3c00U)                /*!< CH2CAPFVAL (Bitfield-Mask: 0x0f)                      */
#define PWM_CAPFILTER_CH3CAPFVAL_Pos      (15U)                    /*!< CH3CAPFVAL (Bit 15)                                   */
#define PWM_CAPFILTER_CH3CAPFVAL_Msk      (0x78000U)               /*!< CH3CAPFVAL (Bitfield-Mask: 0x0f)                      */
#define PWM_CAPFILTER_CAPFPSC_Pos         (24U)                    /*!< CAPFPSC (Bit 24)                                      */
#define PWM_CAPFILTER_CAPFPSC_Msk         (0xf000000U)             /*!< CAPFPSC (Bitfield-Mask: 0x0f)                         */
#define PWM_CAPFILTER_WIDTH               (5U)                     /*!< Channel input filter width                            */
/* ========================================================  FFAFER  ========================================================= */
#define PWM_FFAFER_FER0EN_Pos             (0U)                     /*!< FER0EN (Bit 0)                                        */
#define PWM_FFAFER_FER0EN_Msk             (0x1U)                   /*!< FER0EN (Bitfield-Mask: 0x01)                          */
#define PWM_FFAFER_FER1EN_Pos             (1U)                     /*!< FER1EN (Bit 1)                                        */
#define PWM_FFAFER_FER1EN_Msk             (0x2U)                   /*!< FER1EN (Bitfield-Mask: 0x01)                          */
#define PWM_FFAFER_FER2EN_Pos             (2U)                     /*!< FER2EN (Bit 2)                                        */
#define PWM_FFAFER_FER2EN_Msk             (0x4U)                   /*!< FER2EN (Bitfield-Mask: 0x01)                          */
#define PWM_FFAFER_FER3EN_Pos             (3U)                     /*!< FER3EN (Bit 3)                                        */
#define PWM_FFAFER_FER3EN_Msk             (0x8U)                   /*!< FER3EN (Bitfield-Mask: 0x01)                          */
#define PWM_FFAFER_FF0EN_Pos              (4U)                     /*!< FF0EN (Bit 4)                                         */
#define PWM_FFAFER_FF0EN_Msk              (0x10U)                  /*!< FF0EN (Bitfield-Mask: 0x01)                           */
#define PWM_FFAFER_FF1EN_Pos              (5U)                     /*!< FF1EN (Bit 5)                                         */
#define PWM_FFAFER_FF1EN_Msk              (0x20U)                  /*!< FF1EN (Bitfield-Mask: 0x01)                           */
#define PWM_FFAFER_FF2EN_Pos              (6U)                     /*!< FF2EN (Bit 6)                                         */
#define PWM_FFAFER_FF2EN_Msk              (0x40U)                  /*!< FF2EN (Bitfield-Mask: 0x01)                           */
#define PWM_FFAFER_FF3EN_Pos              (7U)                     /*!< FF3EN (Bit 7)                                         */
#define PWM_FFAFER_FF3EN_Msk              (0x80U)                  /*!< FF3EN (Bitfield-Mask: 0x01)                           */
#define PWM_FFAFER_FFVAL_Pos              (8U)                     /*!< FFVAL (Bit 8)                                         */
#define PWM_FFAFER_FFVAL_Msk              (0xf00U)                 /*!< FFVAL (Bitfield-Mask: 0x0f)                           */
/* ==========================================================  QDI  ========================================================== */
#define PWM_QDI_QDIEN_Pos                 (0U)                     /*!< QDIEN (Bit 0)                                         */
#define PWM_QDI_QDIEN_Msk                 (0x1U)                   /*!< QDIEN (Bitfield-Mask: 0x01)                           */
#define PWM_QDI_CNTOFDIR_Pos              (1U)                     /*!< CNTOFDIR (Bit 1)                                      */
#define PWM_QDI_CNTOFDIR_Msk              (0x2U)                   /*!< CNTOFDIR (Bitfield-Mask: 0x01)                        */
#define PWM_QDI_QUADIR_Pos                (2U)                     /*!< QUADIR (Bit 2)                                        */
#define PWM_QDI_QUADIR_Msk                (0x4U)                   /*!< QUADIR (Bitfield-Mask: 0x01)                          */
#define PWM_QDI_QUADMODE_Pos              (3U)                     /*!< QUADMODE (Bit 3)                                      */
#define PWM_QDI_QUADMODE_Msk              (0x8U)                   /*!< QUADMODE (Bitfield-Mask: 0x01)                        */
#define PWM_QDI_PHBPOL_Pos                (4U)                     /*!< PHBPOL (Bit 4)                                        */
#define PWM_QDI_PHBPOL_Msk                (0x10U)                  /*!< PHBPOL (Bitfield-Mask: 0x01)                          */
#define PWM_QDI_PHAPOL_Pos                (5U)                     /*!< PHAPOL (Bit 5)                                        */
#define PWM_QDI_PHAPOL_Msk                (0x20U)                  /*!< PHAPOL (Bitfield-Mask: 0x01)                          */
#define PWM_QDI_PHZRSTEN_Pos              (8U)                     /*!< PHZRSTEN (Bit 8)                                      */
#define PWM_QDI_PHZRSTEN_Msk              (0x100U)                 /*!< PHZRSTEN (Bitfield-Mask: 0x01)                        */
#define PWM_QDI_PHZPOL_Pos                (9U)                     /*!< PHZPOL (Bit 9)                                        */
#define PWM_QDI_PHZPOL_Msk                (0x200U)                 /*!< PHZPOL (Bitfield-Mask: 0x01)                          */
#define PWM_QDI_PHZSTS_Pos                (11U)                    /*!< PHZSTS (Bit 11)                                       */
#define PWM_QDI_PHZSTS_Msk                (0x800U)                 /*!< PHZSTS (Bitfield-Mask: 0x01)                          */
/* =========================================================  CONF  ========================================================== */
#define PWM_CONF_CNTOFNUM_Pos             (0U)                     /*!< CNTOFNUM (Bit 0)                                      */
#define PWM_CONF_CNTOFNUM_Msk             (0x1fU)                  /*!< CNTOFNUM (Bitfield-Mask: 0x1f)                        */
#define PWM_CONF_GTBEEN_Pos               (9U)                     /*!< GTBEEN (Bit 9)                                        */
#define PWM_CONF_GTBEEN_Msk               (0x200U)                 /*!< GTBEEN (Bitfield-Mask: 0x01)                          */
#define PWM_CONF_GTBEOUT_Pos              (10U)                    /*!< GTBEOUT (Bit 10)                                      */
#define PWM_CONF_GTBEOUT_Msk              (0x400U)                 /*!< GTBEOUT (Bitfield-Mask: 0x01)                         */
#define PWM_CONF_EVENT0_PSC_Pos           (16U)                    /*!< EVENT0_PSC (Bit 16)                                   */
#define PWM_CONF_EVENT0_PSC_Msk           (0x30000U)               /*!< EVENT0_PSC (Bitfield-Mask: 0x03)                      */
#define PWM_CONF_EVENT1_PSC_Pos           (18U)                    /*!< EVENT1_PSC (Bit 18)                                   */
#define PWM_CONF_EVENT1_PSC_Msk           (0xc0000U)               /*!< EVENT1_PSC (Bitfield-Mask: 0x03)                      */
#define PWM_CONF_EVENT2_PSC_Pos           (20U)                    /*!< EVENT2_PSC (Bit 20)                                   */
#define PWM_CONF_EVENT2_PSC_Msk           (0x300000U)              /*!< EVENT2_PSC (Bitfield-Mask: 0x03)                      */
#define PWM_CONF_EVENT3_PSC_Pos           (22U)                    /*!< EVENT3_PSC (Bit 22)                                   */
#define PWM_CONF_EVENT3_PSC_Msk           (0xc00000U)              /*!< EVENT3_PSC (Bitfield-Mask: 0x03)                      */
#define PWM_CONF_EVENT4_PSC_Pos           (24U)                    /*!< EVENT4_PSC (Bit 24)                                   */
#define PWM_CONF_EVENT4_PSC_Msk           (0x3000000U)             /*!< EVENT4_PSC (Bitfield-Mask: 0x03)                      */
#define PWM_CONF_EVENT5_PSC_Pos           (26U)                    /*!< EVENT5_PSC (Bit 26)                                   */
#define PWM_CONF_EVENT5_PSC_Msk           (0xc000000U)             /*!< EVENT5_PSC (Bitfield-Mask: 0x03)                      */
#define PWM_CONF_EVENT6_PSC_Pos           (28U)                    /*!< EVENT6_PSC (Bit 28)                                   */
#define PWM_CONF_EVENT6_PSC_Msk           (0x30000000U)            /*!< EVENT6_PSC (Bitfield-Mask: 0x03)                      */
#define PWM_CONF_EVENT7_PSC_Pos           (30U)                    /*!< EVENT7_PSC (Bit 30)                                   */
#define PWM_CONF_EVENT7_PSC_Msk           (0xc0000000U)            /*!< EVENT7_PSC (Bitfield-Mask: 0x03)                      */
#define PWM_CONF_EVENTPSC_WIDTH           (2U)                     /*!< EVENTPSC (Bitfield-Mask: 0x03)                        */
/* ========================================================  FLTPOL  ========================================================= */
#define PWM_FLTPOL_FLT0POL_Pos            (0U)                     /*!< FLT0POL (Bit 0)                                       */
#define PWM_FLTPOL_FLT0POL_Msk            (0x1U)                   /*!< FLT0POL (Bitfield-Mask: 0x01)                         */
#define PWM_FLTPOL_FLT1POL_Pos            (1U)                     /*!< FLT1POL (Bit 1)                                       */
#define PWM_FLTPOL_FLT1POL_Msk            (0x2U)                   /*!< FLT1POL (Bitfield-Mask: 0x01)                         */
#define PWM_FLTPOL_FLT2POL_Pos            (2U)                     /*!< FLT2POL (Bit 2)                                       */
#define PWM_FLTPOL_FLT2POL_Msk            (0x4U)                   /*!< FLT2POL (Bitfield-Mask: 0x01)                         */
#define PWM_FLTPOL_FLT3POL_Pos            (3U)                     /*!< FLT3POL (Bit 3)                                       */
#define PWM_FLTPOL_FLT3POL_Msk            (0x8U)                   /*!< FLT3POL (Bitfield-Mask: 0x01)                         */
/* ========================================================  SYNCONF  ======================================================== */
#define PWM_SYNCONF_HWTRIGMODE_Pos        (0U)                     /*!< HWTRIGMODE (Bit 0)                                    */
#define PWM_SYNCONF_HWTRIGMODE_Msk        (0x1U)                   /*!< HWTRIGMODE (Bitfield-Mask: 0x01)                      */
#define PWM_SYNCONF_CNTINC_Pos            (2U)                     /*!< CNTINC (Bit 2)                                        */
#define PWM_SYNCONF_CNTINC_Msk            (0x4U)                   /*!< CNTINC (Bitfield-Mask: 0x01)                          */
#define PWM_SYNCONF_INVC_Pos              (4U)                     /*!< INVC (Bit 4)                                          */
#define PWM_SYNCONF_INVC_Msk              (0x10U)                  /*!< INVC (Bitfield-Mask: 0x01)                            */
#define PWM_SYNCONF_SWOC_Pos              (5U)                     /*!< SWOC (Bit 5)                                          */
#define PWM_SYNCONF_SWOC_Msk              (0x20U)                  /*!< SWOC (Bitfield-Mask: 0x01)                            */
#define PWM_SYNCONF_SYNCMODE_Pos          (7U)                     /*!< SYNCMODE (Bit 7)                                      */
#define PWM_SYNCONF_SYNCMODE_Msk          (0x80U)                  /*!< SYNCMODE (Bitfield-Mask: 0x01)                        */
#define PWM_SYNCONF_CNTVSWSYNC_Pos        (8U)                     /*!< CNTVSWSYNC (Bit 8)                                    */
#define PWM_SYNCONF_CNTVSWSYNC_Msk        (0x100U)                 /*!< CNTVSWSYNC (Bitfield-Mask: 0x01)                      */
#define PWM_SYNCONF_PWMSVSWSYNC_Pos       (9U)                     /*!< PWMSVSWSYNC (Bit 9)                                   */
#define PWM_SYNCONF_PWMSVSWSYNC_Msk       (0x200U)                 /*!< PWMSVSWSYNC (Bitfield-Mask: 0x01)                     */
#define PWM_SYNCONF_OMVSWSYNC_Pos         (10U)                    /*!< OMVSWSYNC (Bit 10)                                    */
#define PWM_SYNCONF_OMVSWSYNC_Msk         (0x400U)                 /*!< OMVSWSYNC (Bitfield-Mask: 0x01)                       */
#define PWM_SYNCONF_INVSWSYNC_Pos         (11U)                    /*!< INVSWSYNC (Bit 11)                                    */
#define PWM_SYNCONF_INVSWSYNC_Msk         (0x800U)                 /*!< INVSWSYNC (Bitfield-Mask: 0x01)                       */
#define PWM_SYNCONF_SWVSWSYNC_Pos         (12U)                    /*!< SWVSWSYNC (Bit 12)                                    */
#define PWM_SYNCONF_SWVSWSYNC_Msk         (0x1000U)                /*!< SWVSWSYNC (Bitfield-Mask: 0x01)                       */
#define PWM_SYNCONF_CNTVHWSYNC_Pos        (16U)                    /*!< CNTVHWSYNC (Bit 16)                                   */
#define PWM_SYNCONF_CNTVHWSYNC_Msk        (0x10000U)               /*!< CNTVHWSYNC (Bitfield-Mask: 0x01)                      */
#define PWM_SYNCONF_PWMSVHWSYNC_Pos       (17U)                    /*!< PWMSVHWSYNC (Bit 17)                                  */
#define PWM_SYNCONF_PWMSVHWSYNC_Msk       (0x20000U)               /*!< PWMSVHWSYNC (Bitfield-Mask: 0x01)                     */
#define PWM_SYNCONF_OMVHWSYNC_Pos         (18U)                    /*!< OMVHWSYNC (Bit 18)                                    */
#define PWM_SYNCONF_OMVHWSYNC_Msk         (0x40000U)               /*!< OMVHWSYNC (Bitfield-Mask: 0x01)                       */
#define PWM_SYNCONF_INVHWSYNC_Pos         (19U)                    /*!< INVHWSYNC (Bit 19)                                    */
#define PWM_SYNCONF_INVHWSYNC_Msk         (0x80000U)               /*!< INVHWSYNC (Bitfield-Mask: 0x01)                       */
#define PWM_SYNCONF_SWVHWSYNC_Pos         (20U)                    /*!< SWVHWSYNC (Bit 20)                                    */
#define PWM_SYNCONF_SWVHWSYNC_Msk         (0x100000U)              /*!< SWVHWSYNC (Bitfield-Mask: 0x01)                       */
#define PWM_SYNCONF_SWPOL_Pos             (21U)                    /*!< SWPOL (Bit 21)                                        */
#define PWM_SYNCONF_SWPOL_Msk             (0x200000U)              /*!< SWPOL (Bitfield-Mask: 0x01)                           */
#define PWM_SYNCONF_HWPOL_Pos             (22U)                    /*!< HWPOL (Bit 22)                                        */
#define PWM_SYNCONF_HWPOL_Msk             (0x400000U)              /*!< HWPOL (Bitfield-Mask: 0x01)                           */
/* =========================================================  INVCR  ========================================================= */
#define PWM_INVCR_PAIR0INVEN_Pos          (0U)                     /*!< PAIR0INVEN (Bit 0)                                    */
#define PWM_INVCR_PAIR0INVEN_Msk          (0x1U)                   /*!< PAIR0INVEN (Bitfield-Mask: 0x01)                      */
#define PWM_INVCR_PAIR1INVEN_Pos          (1U)                     /*!< PAIR1INVEN (Bit 1)                                    */
#define PWM_INVCR_PAIR1INVEN_Msk          (0x2U)                   /*!< PAIR1INVEN (Bitfield-Mask: 0x01)                      */
#define PWM_INVCR_PAIR2INVEN_Pos          (2U)                     /*!< PAIR2INVEN (Bit 2)                                    */
#define PWM_INVCR_PAIR2INVEN_Msk          (0x4U)                   /*!< PAIR2INVEN (Bitfield-Mask: 0x01)                      */
#define PWM_INVCR_PAIR3INVEN_Pos          (3U)                     /*!< PAIR3INVEN (Bit 3)                                    */
#define PWM_INVCR_PAIR3INVEN_Msk          (0x8U)                   /*!< PAIR3INVEN (Bitfield-Mask: 0x01)                      */
/* ========================================================  CHOSWCR  ======================================================== */
#define PWM_CHOSWCR_CHNSWEN_Msk           (0xFFU)                  /*!< Channnel output enable Mask                           */
#define PWM_CHOSWCR_CH0SWEN_Pos           (0U)                     /*!< CH0SWEN (Bit 0)                                       */
#define PWM_CHOSWCR_CH0SWEN_Msk           (0x1U)                   /*!< CH0SWEN (Bitfield-Mask: 0x01)                         */
#define PWM_CHOSWCR_CH1SWEN_Pos           (1U)                     /*!< CH1SWEN (Bit 1)                                       */
#define PWM_CHOSWCR_CH1SWEN_Msk           (0x2U)                   /*!< CH1SWEN (Bitfield-Mask: 0x01)                         */
#define PWM_CHOSWCR_CH2SWEN_Pos           (2U)                     /*!< CH2SWEN (Bit 2)                                       */
#define PWM_CHOSWCR_CH2SWEN_Msk           (0x4U)                   /*!< CH2SWEN (Bitfield-Mask: 0x01)                         */
#define PWM_CHOSWCR_CH3SWEN_Pos           (3U)                     /*!< CH3SWEN (Bit 3)                                       */
#define PWM_CHOSWCR_CH3SWEN_Msk           (0x8U)                   /*!< CH3SWEN (Bitfield-Mask: 0x01)                         */
#define PWM_CHOSWCR_CH4SWEN_Pos           (4U)                     /*!< CH4SWEN (Bit 4)                                       */
#define PWM_CHOSWCR_CH4SWEN_Msk           (0x10U)                  /*!< CH4SWEN (Bitfield-Mask: 0x01)                         */
#define PWM_CHOSWCR_CH5SWEN_Pos           (5U)                     /*!< CH5SWEN (Bit 5)                                       */
#define PWM_CHOSWCR_CH5SWEN_Msk           (0x20U)                  /*!< CH5SWEN (Bitfield-Mask: 0x01)                         */
#define PWM_CHOSWCR_CH6SWEN_Pos           (6U)                     /*!< CH6SWEN (Bit 6)                                       */
#define PWM_CHOSWCR_CH6SWEN_Msk           (0x40U)                  /*!< CH6SWEN (Bitfield-Mask: 0x01)                         */
#define PWM_CHOSWCR_CH7SWEN_Pos           (7U)                     /*!< CH7SWEN (Bit 7)                                       */
#define PWM_CHOSWCR_CH7SWEN_Msk           (0x80U)                  /*!< CH7SWEN (Bitfield-Mask: 0x01)                         */
#define PWM_CHOSWCR_CH0SWCV_Pos           (8U)                     /*!< CH0SWCV (Bit 8)                                       */
#define PWM_CHOSWCR_CH0SWCV_Msk           (0x100U)                 /*!< CH0SWCV (Bitfield-Mask: 0x01)                         */
#define PWM_CHOSWCR_CH1SWCV_Pos           (9U)                     /*!< CH1SWCV (Bit 9)                                       */
#define PWM_CHOSWCR_CH1SWCV_Msk           (0x200U)                 /*!< CH1SWCV (Bitfield-Mask: 0x01)                         */
#define PWM_CHOSWCR_CH2SWCV_Pos           (10U)                    /*!< CH2SWCV (Bit 10)                                      */
#define PWM_CHOSWCR_CH2SWCV_Msk           (0x400U)                 /*!< CH2SWCV (Bitfield-Mask: 0x01)                         */
#define PWM_CHOSWCR_CH3SWCV_Pos           (11U)                    /*!< CH3SWCV (Bit 11)                                      */
#define PWM_CHOSWCR_CH3SWCV_Msk           (0x800U)                 /*!< CH3SWCV (Bitfield-Mask: 0x01)                         */
#define PWM_CHOSWCR_CH4SWCV_Pos           (12U)                    /*!< CH4SWCV (Bit 12)                                      */
#define PWM_CHOSWCR_CH4SWCV_Msk           (0x1000U)                /*!< CH4SWCV (Bitfield-Mask: 0x01)                         */
#define PWM_CHOSWCR_CH5SWCV_Pos           (13U)                    /*!< CH5SWCV (Bit 13)                                      */
#define PWM_CHOSWCR_CH5SWCV_Msk           (0x2000U)                /*!< CH5SWCV (Bitfield-Mask: 0x01)                         */
#define PWM_CHOSWCR_CH6SWCV_Pos           (14U)                    /*!< CH6SWCV (Bit 14)                                      */
#define PWM_CHOSWCR_CH6SWCV_Msk           (0x4000U)                /*!< CH6SWCV (Bitfield-Mask: 0x01)                         */
#define PWM_CHOSWCR_CH7SWCV_Pos           (15U)                    /*!< CH7SWCV (Bit 15)                                      */
#define PWM_CHOSWCR_CH7SWCV_Msk           (0x8000U)                /*!< CH7SWCV (Bitfield-Mask: 0x01)                         */
#define PWM_CHOSWCR_CHNSWCV_Msk           (0xFF00U)                /*!< Channnel output value Mask                            */
/* ========================================================  DITHER0  ======================================================== */
#define PWM_DITHER0_C0DHR_Pos             (0U)                     /*!< C0DHR (Bit 0)                                         */
#define PWM_DITHER0_C0DHR_Msk             (0x1fU)                  /*!< C0DHR (Bitfield-Mask: 0x1f)                           */
#define PWM_DITHER0_C1DHR_Pos             (8U)                     /*!< C1DHR (Bit 8)                                         */
#define PWM_DITHER0_C1DHR_Msk             (0x1f00U)                /*!< C1DHR (Bitfield-Mask: 0x1f)                           */
#define PWM_DITHER0_C2DHR_Pos             (16U)                    /*!< C2DHR (Bit 16)                                        */
#define PWM_DITHER0_C2DHR_Msk             (0x1f0000U)              /*!< C2DHR (Bitfield-Mask: 0x1f)                           */
#define PWM_DITHER0_C3DHR_Pos             (24U)                    /*!< C3DHR (Bit 24)                                        */
#define PWM_DITHER0_C3DHR_Msk             (0x1f000000U)            /*!< C3DHR (Bitfield-Mask: 0x1f)                           */
#define PWM_DITHER_VALUE_WIDTH            (8U)                     /*!< Channel Dither Value                                  */
/* ========================================================  DITHER1  ======================================================== */
#define PWM_DITHER1_C4DHR_Pos             (0U)                     /*!< C4DHR (Bit 0)                                         */
#define PWM_DITHER1_C4DHR_Msk             (0x1fU)                  /*!< C4DHR (Bitfield-Mask: 0x1f)                           */
#define PWM_DITHER1_C5DHR_Pos             (8U)                     /*!< C5DHR (Bit 8)                                         */
#define PWM_DITHER1_C5DHR_Msk             (0x1f00U)                /*!< C5DHR (Bitfield-Mask: 0x1f)                           */
#define PWM_DITHER1_C6DHR_Pos             (16U)                    /*!< C6DHR (Bit 16)                                        */
#define PWM_DITHER1_C6DHR_Msk             (0x1f0000U)              /*!< C6DHR (Bitfield-Mask: 0x1f)                           */
#define PWM_DITHER1_C7DHR_Pos             (24U)                    /*!< C7DHR (Bit 24)                                        */
#define PWM_DITHER1_C7DHR_Msk             (0x1f000000U)            /*!< C7DHR (Bitfield-Mask: 0x1f)                           */
/* ========================================================  DITHER2  ======================================================== */
#define PWM_DITHER2_PDHR_Pos              (0U)                     /*!< PDHR (Bit 0)                                          */
#define PWM_DITHER2_PDHR_Msk              (0x1fU)                  /*!< PDHR (Bitfield-Mask: 0x1f)                            */
#define PWM_DITHER2_DEBUGCTRL_Pos         (30U)                    /*!< DEBUGCTRL (Bit 30)                                    */
#define PWM_DITHER2_DEBUGCTRL_Msk         (0xc0000000U)            /*!< DEBUGCTRL (Bitfield-Mask: 0x03)                       */
/* ========================================================  DMACTRL  ======================================================== */
#define PWM_DMACTRL_TRANS_LEN_Pos         (0U)                     /*!< TRANS_LEN (Bit 0)                                     */
#define PWM_DMACTRL_TRANS_LEN_Msk         (0x1fU)                  /*!< TRANS_LEN (Bitfield-Mask: 0x1f)                       */
#define PWM_DMACTRL_TRANS_EN_Pos          (7U)                     /*!< TRANS_EN (Bit 7)                                      */
#define PWM_DMACTRL_TRANS_EN_Msk          (0x80U)                  /*!< TRANS_EN (Bitfield-Mask: 0x01)                        */

/* =========================================================================================================================== */
/* ================                                         SENT_CTRL                                         ================ */
/* =========================================================================================================================== */

/* ==========================================================  FDR  ========================================================== */
#define SENT_CTRL_FDR_STEP_Pos            (0U)                     /*!< STEP (Bit 0)                                          */
#define SENT_CTRL_FDR_STEP_Msk            (0x3ffU)                 /*!< STEP (Bitfield-Mask: 0x3ff)                           */
/* ==========================================================  CTS  ========================================================== */
#define SENT_CTRL_CTS_CTS_Pos             (0U)                     /*!< CTS (Bit 0)                                           */
#define SENT_CTRL_CTS_CTS_Msk             (0xffffffffU)            /*!< CTS (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  TDIV  ========================================================== */
#define SENT_CTRL_TDIV_TDIV_Pos           (0U)                     /*!< TDIV (Bit 0)                                          */
#define SENT_CTRL_TDIV_TDIV_Msk           (0xfffffU)               /*!< TDIV (Bitfield-Mask: 0xfffff)                         */

/* =========================================================================================================================== */
/* ================                                       SENT_CHANNEL                                        ================ */
/* =========================================================================================================================== */

/* =========================================================  CPDR  ========================================================== */
#define SENT_CHANNEL_CPDR_PDIV_Pos        (0U)                     /*!< PDIV (Bit 0)                                          */
#define SENT_CHANNEL_CPDR_PDIV_Msk        (0xfffU)                 /*!< PDIV (Bitfield-Mask: 0xfff)                           */
/* =========================================================  CFDR  ========================================================== */
#define SENT_CHANNEL_CFDR_DIV_Pos         (0U)                     /*!< DIV (Bit 0)                                           */
#define SENT_CHANNEL_CFDR_DIV_Msk         (0xffffU)                /*!< DIV (Bitfield-Mask: 0xffff)                           */
#define SENT_CHANNEL_CFDR_DIVM_Pos        (16U)                    /*!< DIVM (Bit 16)                                         */
#define SENT_CHANNEL_CFDR_DIVM_Msk        (0xffff0000U)            /*!< DIVM (Bitfield-Mask: 0xffff)                          */
/* ==========================================================  RCR  ========================================================== */
#define SENT_CHANNEL_RCR_CHEN_Pos         (0U)                     /*!< CHEN (Bit 0)                                          */
#define SENT_CHANNEL_RCR_CHEN_Msk         (0x1U)                   /*!< CHEN (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_RCR_PAUSE_Pos        (1U)                     /*!< PAUSE (Bit 1)                                         */
#define SENT_CHANNEL_RCR_PAUSE_Msk        (0x2U)                   /*!< PAUSE (Bitfield-Mask: 0x01)                           */
#define SENT_CHANNEL_RCR_ACE_Pos          (2U)                     /*!< ACE (Bit 2)                                           */
#define SENT_CHANNEL_RCR_ACE_Msk          (0x4U)                   /*!< ACE (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_RCR_SNI_Pos          (3U)                     /*!< SNI (Bit 3)                                           */
#define SENT_CHANNEL_RCR_SNI_Msk          (0x8U)                   /*!< SNI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_RCR_SPEN_Pos         (4U)                     /*!< SPEN (Bit 4)                                          */
#define SENT_CHANNEL_RCR_SPEN_Msk         (0x10U)                  /*!< SPEN (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_RCR_SCRC_Pos         (5U)                     /*!< SCRC (Bit 5)                                          */
#define SENT_CHANNEL_RCR_SCRC_Msk         (0x20U)                  /*!< SCRC (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_RCR_FCRC_Pos         (6U)                     /*!< FCRC (Bit 6)                                          */
#define SENT_CHANNEL_RCR_FCRC_Msk         (0x40U)                  /*!< FCRC (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_RCR_FRAME_CHECK_Pos  (7U)                     /*!< FRAME_CHECK (Bit 7)                                   */
#define SENT_CHANNEL_RCR_FRAME_CHECK_Msk  (0x80U)                  /*!< FRAME_CHECK (Bitfield-Mask: 0x01)                     */
#define SENT_CHANNEL_RCR_FRAME_LENGTH_Pos (8U)                     /*!< FRAME_LENGTH (Bit 8)                                  */
#define SENT_CHANNEL_RCR_FRAME_LENGTH_Msk (0xff00U)                /*!< FRAME_LENGTH (Bitfield-Mask: 0xff)                    */
#define SENT_CHANNEL_RCR_CRZ_Pos          (16U)                    /*!< CRZ (Bit 16)                                          */
#define SENT_CHANNEL_RCR_CRZ_Msk          (0x10000U)               /*!< CRZ (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_RCR_SMOD_Pos         (17U)                    /*!< SMOD (Bit 17)                                         */
#define SENT_CHANNEL_RCR_SMOD_Msk         (0x20000U)               /*!< SMOD (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_RCR_DED_Pos          (18U)                    /*!< DED (Bit 18)                                          */
#define SENT_CHANNEL_RCR_DED_Msk          (0x40000U)               /*!< DED (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_RCR_FDFL_Pos         (20U)                    /*!< FDFL (Bit 20)                                         */
#define SENT_CHANNEL_RCR_FDFL_Msk         (0x100000U)              /*!< FDFL (Bitfield-Mask: 0x01)                            */
/* ==========================================================  RSR  ========================================================== */
#define SENT_CHANNEL_RSR_CRC_Pos          (0U)                     /*!< CRC (Bit 0)                                           */
#define SENT_CHANNEL_RSR_CRC_Msk          (0xfU)                   /*!< CRC (Bitfield-Mask: 0x0f)                             */
#define SENT_CHANNEL_RSR_CSTATUS_Pos      (4U)                     /*!< CSTATUS (Bit 4)                                       */
#define SENT_CHANNEL_RSR_CSTATUS_Msk      (0x30U)                  /*!< CSTATUS (Bitfield-Mask: 0x03)                         */
#define SENT_CHANNEL_RSR_FSTATUS_Pos      (8U)                     /*!< FSTATUS (Bit 8)                                       */
#define SENT_CHANNEL_RSR_FSTATUS_Msk      (0xf00U)                 /*!< FSTATUS (Bitfield-Mask: 0x0f)                         */
#define SENT_CHANNEL_RSR_FRLEN_Pos        (16U)                    /*!< FRLEN (Bit 16)                                        */
#define SENT_CHANNEL_RSR_FRLEN_Msk        (0xffff0000U)            /*!< FRLEN (Bitfield-Mask: 0xffff)                         */
/* ==========================================================  SDS  ========================================================== */
#define SENT_CHANNEL_SDS_SD_Pos           (0U)                     /*!< SD (Bit 0)                                            */
#define SENT_CHANNEL_SDS_SD_Msk           (0xffffU)                /*!< SD (Bitfield-Mask: 0xffff)                            */
#define SENT_CHANNEL_SDS_MID_Pos          (16U)                    /*!< MID (Bit 16)                                          */
#define SENT_CHANNEL_SDS_MID_Msk          (0xff0000U)              /*!< MID (Bitfield-Mask: 0xff)                             */
#define SENT_CHANNEL_SDS_SCRC_Pos         (24U)                    /*!< SCRC (Bit 24)                                         */
#define SENT_CHANNEL_SDS_SCRC_Msk         (0x3f000000U)            /*!< SCRC (Bitfield-Mask: 0x3f)                            */
#define SENT_CHANNEL_SDS_CON_Pos          (31U)                    /*!< CON (Bit 31)                                          */
#define SENT_CHANNEL_SDS_CON_Msk          (0x80000000U)            /*!< CON (Bitfield-Mask: 0x01)                             */
/* =========================================================  IOCR  ========================================================== */
#define SENT_CHANNEL_IOCR_INSEL_Pos       (0U)                     /*!< INSEL (Bit 0)                                         */
#define SENT_CHANNEL_IOCR_INSEL_Msk       (0x3U)                   /*!< INSEL (Bitfield-Mask: 0x03)                           */
#define SENT_CHANNEL_IOCR_DEPTH_Pos       (4U)                     /*!< DEPTH (Bit 4)                                         */
#define SENT_CHANNEL_IOCR_DEPTH_Msk       (0xf0U)                  /*!< DEPTH (Bitfield-Mask: 0x0f)                           */
#define SENT_CHANNEL_IOCR_OIE_Pos         (8U)                     /*!< OIE (Bit 8)                                           */
#define SENT_CHANNEL_IOCR_OIE_Msk         (0x100U)                 /*!< OIE (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_IOCR_IIE_Pos         (9U)                     /*!< IIE (Bit 9)                                           */
#define SENT_CHANNEL_IOCR_IIE_Msk         (0x200U)                 /*!< IIE (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_IOCR_ETS_Pos         (16U)                    /*!< ETS (Bit 16)                                          */
#define SENT_CHANNEL_IOCR_ETS_Msk         (0xf0000U)               /*!< ETS (Bitfield-Mask: 0x0f)                             */
/* ==========================================================  SCR  ========================================================== */
#define SENT_CHANNEL_SCR_PLEN_Pos         (0U)                     /*!< PLEN (Bit 0)                                          */
#define SENT_CHANNEL_SCR_PLEN_Msk         (0x3fU)                  /*!< PLEN (Bitfield-Mask: 0x3f)                            */
#define SENT_CHANNEL_SCR_TRIG_Pos         (6U)                     /*!< TRIG (Bit 6)                                          */
#define SENT_CHANNEL_SCR_TRIG_Msk         (0xc0U)                  /*!< TRIG (Bitfield-Mask: 0x03)                            */
#define SENT_CHANNEL_SCR_DEL_Pos          (8U)                     /*!< DEL (Bit 8)                                           */
#define SENT_CHANNEL_SCR_DEL_Msk          (0x3f00U)                /*!< DEL (Bitfield-Mask: 0x3f)                             */
#define SENT_CHANNEL_SCR_BASE_Pos         (14U)                    /*!< BASE (Bit 14)                                         */
#define SENT_CHANNEL_SCR_BASE_Msk         (0x4000U)                /*!< BASE (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_SCR_TRQ_Pos          (15U)                    /*!< TRQ (Bit 15)                                          */
#define SENT_CHANNEL_SCR_TRQ_Msk          (0x8000U)                /*!< TRQ (Bitfield-Mask: 0x01)                             */
/* =========================================================  VIEW  ========================================================== */
#define SENT_CHANNEL_VIEW_RDNP0_Pos       (0U)                     /*!< RDNP0 (Bit 0)                                         */
#define SENT_CHANNEL_VIEW_RDNP0_Msk       (0x7U)                   /*!< RDNP0 (Bitfield-Mask: 0x07)                           */
#define SENT_CHANNEL_VIEW_RDNP1_Pos       (4U)                     /*!< RDNP1 (Bit 4)                                         */
#define SENT_CHANNEL_VIEW_RDNP1_Msk       (0x70U)                  /*!< RDNP1 (Bitfield-Mask: 0x07)                           */
#define SENT_CHANNEL_VIEW_RDNP2_Pos       (8U)                     /*!< RDNP2 (Bit 8)                                         */
#define SENT_CHANNEL_VIEW_RDNP2_Msk       (0x700U)                 /*!< RDNP2 (Bitfield-Mask: 0x07)                           */
#define SENT_CHANNEL_VIEW_RDNP3_Pos       (12U)                    /*!< RDNP3 (Bit 12)                                        */
#define SENT_CHANNEL_VIEW_RDNP3_Msk       (0x7000U)                /*!< RDNP3 (Bitfield-Mask: 0x07)                           */
#define SENT_CHANNEL_VIEW_RDNP4_Pos       (16U)                    /*!< RDNP4 (Bit 16)                                        */
#define SENT_CHANNEL_VIEW_RDNP4_Msk       (0x70000U)               /*!< RDNP4 (Bitfield-Mask: 0x07)                           */
#define SENT_CHANNEL_VIEW_RDNP5_Pos       (20U)                    /*!< RDNP5 (Bit 20)                                        */
#define SENT_CHANNEL_VIEW_RDNP5_Msk       (0x700000U)              /*!< RDNP5 (Bitfield-Mask: 0x07)                           */
#define SENT_CHANNEL_VIEW_RDNP6_Pos       (24U)                    /*!< RDNP6 (Bit 24)                                        */
#define SENT_CHANNEL_VIEW_RDNP6_Msk       (0x7000000U)             /*!< RDNP6 (Bitfield-Mask: 0x07)                           */
#define SENT_CHANNEL_VIEW_RDNP7_Pos       (28U)                    /*!< RDNP7 (Bit 28)                                        */
#define SENT_CHANNEL_VIEW_RDNP7_Msk       (0x70000000U)            /*!< RDNP7 (Bitfield-Mask: 0x07)                           */
/* ========================================================  INTSTAT  ======================================================== */
#define SENT_CHANNEL_INTSTAT_RSI_Pos      (0U)                     /*!< RSI (Bit 0)                                           */
#define SENT_CHANNEL_INTSTAT_RSI_Msk      (0x1U)                   /*!< RSI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSTAT_RDI_Pos      (1U)                     /*!< RDI (Bit 1)                                           */
#define SENT_CHANNEL_INTSTAT_RDI_Msk      (0x2U)                   /*!< RDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSTAT_RFI_Pos      (2U)                     /*!< RFI (Bit 2)                                           */
#define SENT_CHANNEL_INTSTAT_RFI_Msk      (0x4U)                   /*!< RFI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSTAT_TDI_Pos      (3U)                     /*!< TDI (Bit 3)                                           */
#define SENT_CHANNEL_INTSTAT_TDI_Msk      (0x8U)                   /*!< TDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSTAT_TBI_Pos      (4U)                     /*!< TBI (Bit 4)                                           */
#define SENT_CHANNEL_INTSTAT_TBI_Msk      (0x10U)                  /*!< TBI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSTAT_FRI_Pos      (5U)                     /*!< FRI (Bit 5)                                           */
#define SENT_CHANNEL_INTSTAT_FRI_Msk      (0x20U)                  /*!< FRI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSTAT_FDI_Pos      (6U)                     /*!< FDI (Bit 6)                                           */
#define SENT_CHANNEL_INTSTAT_FDI_Msk      (0x40U)                  /*!< FDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSTAT_NNI_Pos      (7U)                     /*!< NNI (Bit 7)                                           */
#define SENT_CHANNEL_INTSTAT_NNI_Msk      (0x80U)                  /*!< NNI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSTAT_NVI_Pos      (8U)                     /*!< NVI (Bit 8)                                           */
#define SENT_CHANNEL_INTSTAT_NVI_Msk      (0x100U)                 /*!< NVI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSTAT_CRCI_Pos     (9U)                     /*!< CRCI (Bit 9)                                          */
#define SENT_CHANNEL_INTSTAT_CRCI_Msk     (0x200U)                 /*!< CRCI (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_INTSTAT_WSI_Pos      (10U)                    /*!< WSI (Bit 10)                                          */
#define SENT_CHANNEL_INTSTAT_WSI_Msk      (0x400U)                 /*!< WSI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSTAT_SDI_Pos      (11U)                    /*!< SDI (Bit 11)                                          */
#define SENT_CHANNEL_INTSTAT_SDI_Msk      (0x800U)                 /*!< SDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSTAT_SCRI_Pos     (12U)                    /*!< SCRI (Bit 12)                                         */
#define SENT_CHANNEL_INTSTAT_SCRI_Msk     (0x1000U)                /*!< SCRI (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_INTSTAT_WDI_Pos      (13U)                    /*!< WDI (Bit 13)                                          */
#define SENT_CHANNEL_INTSTAT_WDI_Msk      (0x2000U)                /*!< WDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSTAT_ESI_Pos      (14U)                    /*!< ESI (Bit 14)                                          */
#define SENT_CHANNEL_INTSTAT_ESI_Msk      (0x4000U)                /*!< ESI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSTAT_HFLL_Pos     (15U)                    /*!< HFLL (Bit 15)                                         */
#define SENT_CHANNEL_INTSTAT_HFLL_Msk     (0x8000U)                /*!< HFLL (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_INTSTAT_FDONE_Pos    (16U)                    /*!< FDONE (Bit 16)                                        */
#define SENT_CHANNEL_INTSTAT_FDONE_Msk    (0x10000U)               /*!< FDONE (Bitfield-Mask: 0x01)                           */
/* ========================================================  INTSET  ========================================================= */
#define SENT_CHANNEL_INTSET_RSI_Pos       (0U)                     /*!< RSI (Bit 0)                                           */
#define SENT_CHANNEL_INTSET_RSI_Msk       (0x1U)                   /*!< RSI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSET_RDI_Pos       (1U)                     /*!< RDI (Bit 1)                                           */
#define SENT_CHANNEL_INTSET_RDI_Msk       (0x2U)                   /*!< RDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSET_RFI_Pos       (2U)                     /*!< RFI (Bit 2)                                           */
#define SENT_CHANNEL_INTSET_RFI_Msk       (0x4U)                   /*!< RFI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSET_TDI_Pos       (3U)                     /*!< TDI (Bit 3)                                           */
#define SENT_CHANNEL_INTSET_TDI_Msk       (0x8U)                   /*!< TDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSET_TBI_Pos       (4U)                     /*!< TBI (Bit 4)                                           */
#define SENT_CHANNEL_INTSET_TBI_Msk       (0x10U)                  /*!< TBI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSET_FRI_Pos       (5U)                     /*!< FRI (Bit 5)                                           */
#define SENT_CHANNEL_INTSET_FRI_Msk       (0x20U)                  /*!< FRI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSET_FDI_Pos       (6U)                     /*!< FDI (Bit 6)                                           */
#define SENT_CHANNEL_INTSET_FDI_Msk       (0x40U)                  /*!< FDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSET_NNI_Pos       (7U)                     /*!< NNI (Bit 7)                                           */
#define SENT_CHANNEL_INTSET_NNI_Msk       (0x80U)                  /*!< NNI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSET_NVI_Pos       (8U)                     /*!< NVI (Bit 8)                                           */
#define SENT_CHANNEL_INTSET_NVI_Msk       (0x100U)                 /*!< NVI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSET_CRCI_Pos      (9U)                     /*!< CRCI (Bit 9)                                          */
#define SENT_CHANNEL_INTSET_CRCI_Msk      (0x200U)                 /*!< CRCI (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_INTSET_WSI_Pos       (10U)                    /*!< WSI (Bit 10)                                          */
#define SENT_CHANNEL_INTSET_WSI_Msk       (0x400U)                 /*!< WSI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSET_SDI_Pos       (11U)                    /*!< SDI (Bit 11)                                          */
#define SENT_CHANNEL_INTSET_SDI_Msk       (0x800U)                 /*!< SDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSET_SCRI_Pos      (12U)                    /*!< SCRI (Bit 12)                                         */
#define SENT_CHANNEL_INTSET_SCRI_Msk      (0x1000U)                /*!< SCRI (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_INTSET_WDI_Pos       (13U)                    /*!< WDI (Bit 13)                                          */
#define SENT_CHANNEL_INTSET_WDI_Msk       (0x2000U)                /*!< WDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSET_ESI_Pos       (14U)                    /*!< ESI (Bit 14)                                          */
#define SENT_CHANNEL_INTSET_ESI_Msk       (0x4000U)                /*!< ESI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTSET_HFLL_Pos      (15U)                    /*!< HFLL (Bit 15)                                         */
#define SENT_CHANNEL_INTSET_HFLL_Msk      (0x8000U)                /*!< HFLL (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_INTSET_FDONE_Pos     (16U)                    /*!< FDONE (Bit 16)                                        */
#define SENT_CHANNEL_INTSET_FDONE_Msk     (0x10000U)               /*!< FDONE (Bitfield-Mask: 0x01)                           */
/* ========================================================  INTCLR  ========================================================= */
#define SENT_CHANNEL_INTCLR_RSI_Pos       (0U)                     /*!< RSI (Bit 0)                                           */
#define SENT_CHANNEL_INTCLR_RSI_Msk       (0x1U)                   /*!< RSI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTCLR_RDI_Pos       (1U)                     /*!< RDI (Bit 1)                                           */
#define SENT_CHANNEL_INTCLR_RDI_Msk       (0x2U)                   /*!< RDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTCLR_RFI_Pos       (2U)                     /*!< RFI (Bit 2)                                           */
#define SENT_CHANNEL_INTCLR_RFI_Msk       (0x4U)                   /*!< RFI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTCLR_TDI_Pos       (3U)                     /*!< TDI (Bit 3)                                           */
#define SENT_CHANNEL_INTCLR_TDI_Msk       (0x8U)                   /*!< TDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTCLR_TBI_Pos       (4U)                     /*!< TBI (Bit 4)                                           */
#define SENT_CHANNEL_INTCLR_TBI_Msk       (0x10U)                  /*!< TBI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTCLR_FRI_Pos       (5U)                     /*!< FRI (Bit 5)                                           */
#define SENT_CHANNEL_INTCLR_FRI_Msk       (0x20U)                  /*!< FRI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTCLR_FDI_Pos       (6U)                     /*!< FDI (Bit 6)                                           */
#define SENT_CHANNEL_INTCLR_FDI_Msk       (0x40U)                  /*!< FDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTCLR_NNI_Pos       (7U)                     /*!< NNI (Bit 7)                                           */
#define SENT_CHANNEL_INTCLR_NNI_Msk       (0x80U)                  /*!< NNI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTCLR_NVI_Pos       (8U)                     /*!< NVI (Bit 8)                                           */
#define SENT_CHANNEL_INTCLR_NVI_Msk       (0x100U)                 /*!< NVI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTCLR_CRCI_Pos      (9U)                     /*!< CRCI (Bit 9)                                          */
#define SENT_CHANNEL_INTCLR_CRCI_Msk      (0x200U)                 /*!< CRCI (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_INTCLR_WSI_Pos       (10U)                    /*!< WSI (Bit 10)                                          */
#define SENT_CHANNEL_INTCLR_WSI_Msk       (0x400U)                 /*!< WSI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTCLR_SDI_Pos       (11U)                    /*!< SDI (Bit 11)                                          */
#define SENT_CHANNEL_INTCLR_SDI_Msk       (0x800U)                 /*!< SDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTCLR_SCRI_Pos      (12U)                    /*!< SCRI (Bit 12)                                         */
#define SENT_CHANNEL_INTCLR_SCRI_Msk      (0x1000U)                /*!< SCRI (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_INTCLR_WDI_Pos       (13U)                    /*!< WDI (Bit 13)                                          */
#define SENT_CHANNEL_INTCLR_WDI_Msk       (0x2000U)                /*!< WDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTCLR_ESI_Pos       (14U)                    /*!< ESI (Bit 14)                                          */
#define SENT_CHANNEL_INTCLR_ESI_Msk       (0x4000U)                /*!< ESI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTCLR_HFLL_Pos      (15U)                    /*!< HFLL (Bit 15)                                         */
#define SENT_CHANNEL_INTCLR_HFLL_Msk      (0x8000U)                /*!< HFLL (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_INTCLR_FDONE_Pos     (16U)                    /*!< FDONE (Bit 16)                                        */
#define SENT_CHANNEL_INTCLR_FDONE_Msk     (0x10000U)               /*!< FDONE (Bitfield-Mask: 0x01)                           */
/* =========================================================  INTEN  ========================================================= */
#define SENT_CHANNEL_INTEN_RSI_Pos        (0U)                     /*!< RSI (Bit 0)                                           */
#define SENT_CHANNEL_INTEN_RSI_Msk        (0x1U)                   /*!< RSI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTEN_RDI_Pos        (1U)                     /*!< RDI (Bit 1)                                           */
#define SENT_CHANNEL_INTEN_RDI_Msk        (0x2U)                   /*!< RDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTEN_RFI_Pos        (2U)                     /*!< RFI (Bit 2)                                           */
#define SENT_CHANNEL_INTEN_RFI_Msk        (0x4U)                   /*!< RFI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTEN_TDI_Pos        (3U)                     /*!< TDI (Bit 3)                                           */
#define SENT_CHANNEL_INTEN_TDI_Msk        (0x8U)                   /*!< TDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTEN_TBI_Pos        (4U)                     /*!< TBI (Bit 4)                                           */
#define SENT_CHANNEL_INTEN_TBI_Msk        (0x10U)                  /*!< TBI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTEN_FRI_Pos        (5U)                     /*!< FRI (Bit 5)                                           */
#define SENT_CHANNEL_INTEN_FRI_Msk        (0x20U)                  /*!< FRI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTEN_FDI_Pos        (6U)                     /*!< FDI (Bit 6)                                           */
#define SENT_CHANNEL_INTEN_FDI_Msk        (0x40U)                  /*!< FDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTEN_NNI_Pos        (7U)                     /*!< NNI (Bit 7)                                           */
#define SENT_CHANNEL_INTEN_NNI_Msk        (0x80U)                  /*!< NNI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTEN_NVI_Pos        (8U)                     /*!< NVI (Bit 8)                                           */
#define SENT_CHANNEL_INTEN_NVI_Msk        (0x100U)                 /*!< NVI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTEN_CRCI_Pos       (9U)                     /*!< CRCI (Bit 9)                                          */
#define SENT_CHANNEL_INTEN_CRCI_Msk       (0x200U)                 /*!< CRCI (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_INTEN_WSI_Pos        (10U)                    /*!< WSI (Bit 10)                                          */
#define SENT_CHANNEL_INTEN_WSI_Msk        (0x400U)                 /*!< WSI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTEN_SDI_Pos        (11U)                    /*!< SDI (Bit 11)                                          */
#define SENT_CHANNEL_INTEN_SDI_Msk        (0x800U)                 /*!< SDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTEN_SCRI_Pos       (12U)                    /*!< SCRI (Bit 12)                                         */
#define SENT_CHANNEL_INTEN_SCRI_Msk       (0x1000U)                /*!< SCRI (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_INTEN_WDI_Pos        (13U)                    /*!< WDI (Bit 13)                                          */
#define SENT_CHANNEL_INTEN_WDI_Msk        (0x2000U)                /*!< WDI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTEN_ESI_Pos        (14U)                    /*!< ESI (Bit 14)                                          */
#define SENT_CHANNEL_INTEN_ESI_Msk        (0x4000U)                /*!< ESI (Bitfield-Mask: 0x01)                             */
#define SENT_CHANNEL_INTEN_HFLL_Pos       (15U)                    /*!< HFLL (Bit 15)                                         */
#define SENT_CHANNEL_INTEN_HFLL_Msk       (0x8000U)                /*!< HFLL (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_INTEN_FDONE_Pos      (16U)                    /*!< FDONE (Bit 16)                                        */
#define SENT_CHANNEL_INTEN_FDONE_Msk      (0x10000U)               /*!< FDONE (Bitfield-Mask: 0x01)                           */
/* =========================================================  FIFO  ========================================================== */
#define SENT_CHANNEL_FIFO_EMPTY_Pos       (0U)                     /*!< EMPTY (Bit 0)                                         */
#define SENT_CHANNEL_FIFO_EMPTY_Msk       (0x1U)                   /*!< EMPTY (Bitfield-Mask: 0x01)                           */
#define SENT_CHANNEL_FIFO_HALF_FULL_Pos   (1U)                     /*!< HALF_FULL (Bit 1)                                     */
#define SENT_CHANNEL_FIFO_HALF_FULL_Msk   (0x2U)                   /*!< HALF_FULL (Bitfield-Mask: 0x01)                       */
#define SENT_CHANNEL_FIFO_FULL_Pos        (2U)                     /*!< FULL (Bit 2)                                          */
#define SENT_CHANNEL_FIFO_FULL_Msk        (0x4U)                   /*!< FULL (Bitfield-Mask: 0x01)                            */
#define SENT_CHANNEL_FIFO_FIFO_AVAIL_Pos  (3U)                     /*!< FIFO_AVAIL (Bit 3)                                    */
#define SENT_CHANNEL_FIFO_FIFO_AVAIL_Msk  (0x78U)                  /*!< FIFO_AVAIL (Bitfield-Mask: 0x0f)                      */
#define SENT_CHANNEL_FIFO_FLUSH_Pos       (7U)                     /*!< FLUSH (Bit 7)                                         */
#define SENT_CHANNEL_FIFO_FLUSH_Msk       (0x80U)                  /*!< FLUSH (Bitfield-Mask: 0x01)                           */
/* ==========================================================  WDT  ========================================================== */
#define SENT_CHANNEL_WDT_WDL_Pos          (0U)                     /*!< WDL (Bit 0)                                           */
#define SENT_CHANNEL_WDT_WDL_Msk          (0xffffU)                /*!< WDL (Bitfield-Mask: 0xffff)                           */
/* ==========================================================  RDR  ========================================================== */
#define SENT_CHANNEL_RDR_RDR_Pos          (0U)                     /*!< RDR (Bit 0)                                           */
#define SENT_CHANNEL_RDR_RDR_Msk          (0xffffffffU)            /*!< RDR (Bitfield-Mask: 0xffffffff)                       */

/* =========================================================================================================================== */
/* ================                                        TIMER_CTRL                                         ================ */
/* =========================================================================================================================== */

/* ==========================================================  CR  =========================================================== */
#define TIMER_CTRL_CR_MC_EN_Pos           (0U)                     /*!< MC_EN (Bit 0)                                         */
#define TIMER_CTRL_CR_MC_EN_Msk           (0x1U)                   /*!< MC_EN (Bitfield-Mask: 0x01)                           */
#define TIMER_CTRL_CR_DBG_EN_Pos          (1U)                     /*!< DBG_EN (Bit 1)                                        */
#define TIMER_CTRL_CR_DBG_EN_Msk          (0x2U)                   /*!< DBG_EN (Bitfield-Mask: 0x01)                          */
/* ==========================================================  SR  =========================================================== */
#define TIMER_CTRL_SR_TIF0_Pos            (0U)                     /*!< TIF0 (Bit 0)                                          */
#define TIMER_CTRL_SR_TIF0_Msk            (0x1U)                   /*!< TIF0 (Bitfield-Mask: 0x01)                            */
#define TIMER_CTRL_SR_TIF1_Pos            (1U)                     /*!< TIF1 (Bit 1)                                          */
#define TIMER_CTRL_SR_TIF1_Msk            (0x2U)                   /*!< TIF1 (Bitfield-Mask: 0x01)                            */
#define TIMER_CTRL_SR_TIF2_Pos            (2U)                     /*!< TIF2 (Bit 2)                                          */
#define TIMER_CTRL_SR_TIF2_Msk            (0x4U)                   /*!< TIF2 (Bitfield-Mask: 0x01)                            */
#define TIMER_CTRL_SR_TIF3_Pos            (3U)                     /*!< TIF3 (Bit 3)                                          */
#define TIMER_CTRL_SR_TIF3_Msk            (0x8U)                   /*!< TIF3 (Bitfield-Mask: 0x01)                            */
#define TIMER_CTRL_SR_TMIF0_Pos           (4U)                     /*!< TMIF0 (Bit 4)                                         */
#define TIMER_CTRL_SR_TMIF0_Msk           (0x10U)                  /*!< TMIF0 (Bitfield-Mask: 0x01)                           */
#define TIMER_CTRL_SR_TMIF1_Pos           (5U)                     /*!< TMIF1 (Bit 5)                                         */
#define TIMER_CTRL_SR_TMIF1_Msk           (0x20U)                  /*!< TMIF1 (Bitfield-Mask: 0x01)                           */
#define TIMER_CTRL_SR_TMIF2_Pos           (6U)                     /*!< TMIF2 (Bit 6)                                         */
#define TIMER_CTRL_SR_TMIF2_Msk           (0x40U)                  /*!< TMIF2 (Bitfield-Mask: 0x01)                           */
#define TIMER_CTRL_SR_TMIF3_Pos           (7U)                     /*!< TMIF3 (Bit 7)                                         */
#define TIMER_CTRL_SR_TMIF3_Msk           (0x80U)                  /*!< TMIF3 (Bitfield-Mask: 0x01)                           */
/* ==========================================================  IER  ========================================================== */
#define TIMER_CTRL_IER_TIE0_Pos           (0U)                     /*!< TIE0 (Bit 0)                                          */
#define TIMER_CTRL_IER_TIE0_Msk           (0x1U)                   /*!< TIE0 (Bitfield-Mask: 0x01)                            */
#define TIMER_CTRL_IER_TIE1_Pos           (1U)                     /*!< TIE1 (Bit 1)                                          */
#define TIMER_CTRL_IER_TIE1_Msk           (0x2U)                   /*!< TIE1 (Bitfield-Mask: 0x01)                            */
#define TIMER_CTRL_IER_TIE2_Pos           (2U)                     /*!< TIE2 (Bit 2)                                          */
#define TIMER_CTRL_IER_TIE2_Msk           (0x4U)                   /*!< TIE2 (Bitfield-Mask: 0x01)                            */
#define TIMER_CTRL_IER_TIE3_Pos           (3U)                     /*!< TIE3 (Bit 3)                                          */
#define TIMER_CTRL_IER_TIE3_Msk           (0x8U)                   /*!< TIE3 (Bitfield-Mask: 0x01)                            */
#define TIMER_CTRL_IER_TMIE0_Pos          (4U)                     /*!< TMIE0 (Bit 4)                                         */
#define TIMER_CTRL_IER_TMIE0_Msk          (0x10U)                  /*!< TMIE0 (Bitfield-Mask: 0x01)                           */
#define TIMER_CTRL_IER_TMIE1_Pos          (5U)                     /*!< TMIE1 (Bit 5)                                         */
#define TIMER_CTRL_IER_TMIE1_Msk          (0x20U)                  /*!< TMIE1 (Bitfield-Mask: 0x01)                           */
#define TIMER_CTRL_IER_TMIE2_Pos          (6U)                     /*!< TMIE2 (Bit 6)                                         */
#define TIMER_CTRL_IER_TMIE2_Msk          (0x40U)                  /*!< TMIE2 (Bitfield-Mask: 0x01)                           */
#define TIMER_CTRL_IER_TMIE3_Pos          (7U)                     /*!< TMIE3 (Bit 7)                                         */
#define TIMER_CTRL_IER_TMIE3_Msk          (0x80U)                  /*!< TMIE3 (Bitfield-Mask: 0x01)                           */
/* ==========================================================  ENR  ========================================================== */
#define TIMER_CTRL_ENR_TEN0_Pos           (0U)                     /*!< TEN0 (Bit 0)                                          */
#define TIMER_CTRL_ENR_TEN0_Msk           (0x1U)                   /*!< TEN0 (Bitfield-Mask: 0x01)                            */
#define TIMER_CTRL_ENR_TEN1_Pos           (1U)                     /*!< TEN1 (Bit 1)                                          */
#define TIMER_CTRL_ENR_TEN1_Msk           (0x2U)                   /*!< TEN1 (Bitfield-Mask: 0x01)                            */
#define TIMER_CTRL_ENR_TEN2_Pos           (2U)                     /*!< TEN2 (Bit 2)                                          */
#define TIMER_CTRL_ENR_TEN2_Msk           (0x4U)                   /*!< TEN2 (Bitfield-Mask: 0x01)                            */
#define TIMER_CTRL_ENR_TEN3_Pos           (3U)                     /*!< TEN3 (Bit 3)                                          */
#define TIMER_CTRL_ENR_TEN3_Msk           (0x8U)                   /*!< TEN3 (Bitfield-Mask: 0x01)                            */

/* =========================================================================================================================== */
/* ================                                       TIMER_CHANNEL                                       ================ */
/* =========================================================================================================================== */

/* =========================================================  TVAL  ========================================================== */
#define TIMER_CHANNEL_TVAL_TVAL_Pos       (0U)                     /*!< TVAL (Bit 0)                                          */
#define TIMER_CHANNEL_TVAL_TVAL_Msk       (0xffffffffU)            /*!< TVAL (Bitfield-Mask: 0xffffffff)                      */
/* =========================================================  CVAL  ========================================================== */
#define TIMER_CHANNEL_CVAL_CVAL_Pos       (0U)                     /*!< CVAL (Bit 0)                                          */
#define TIMER_CHANNEL_CVAL_CVAL_Msk       (0xffffffffU)            /*!< CVAL (Bitfield-Mask: 0xffffffff)                      */
/* =========================================================  CTRL  ========================================================== */
#define TIMER_CHANNEL_CTRL_CHN_EN_Pos     (0U)                     /*!< CHN_EN (Bit 0)                                        */
#define TIMER_CHANNEL_CTRL_CHN_EN_Msk     (0x1U)                   /*!< CHN_EN (Bitfield-Mask: 0x01)                          */
#define TIMER_CHANNEL_CTRL_TVAL_UP_Pos    (1U)                     /*!< TVAL_UP (Bit 1)                                       */
#define TIMER_CHANNEL_CTRL_TVAL_UP_Msk    (0x2U)                   /*!< TVAL_UP (Bitfield-Mask: 0x01)                         */
#define TIMER_CHANNEL_CTRL_CVAL_SEL_Pos   (2U)                     /*!< CVAL_SEL (Bit 2)                                      */
#define TIMER_CHANNEL_CTRL_CVAL_SEL_Msk   (0xcU)                   /*!< CVAL_SEL (Bitfield-Mask: 0x03)                        */
#define TIMER_CHANNEL_CTRL_MODE_Pos       (4U)                     /*!< MODE (Bit 4)                                          */
#define TIMER_CHANNEL_CTRL_MODE_Msk       (0x30U)                  /*!< MODE (Bitfield-Mask: 0x03)                            */
#define TIMER_CHANNEL_CTRL_TSOT_Pos       (8U)                     /*!< TSOT (Bit 8)                                          */
#define TIMER_CHANNEL_CTRL_TSOT_Msk       (0x100U)                 /*!< TSOT (Bitfield-Mask: 0x01)                            */
#define TIMER_CHANNEL_CTRL_TSOI_Pos       (9U)                     /*!< TSOI (Bit 9)                                          */
#define TIMER_CHANNEL_CTRL_TSOI_Msk       (0x200U)                 /*!< TSOI (Bitfield-Mask: 0x01)                            */
#define TIMER_CHANNEL_CTRL_TROT_Pos       (10U)                    /*!< TROT (Bit 10)                                         */
#define TIMER_CHANNEL_CTRL_TROT_Msk       (0x400U)                 /*!< TROT (Bitfield-Mask: 0x01)                            */
#define TIMER_CHANNEL_CTRL_TTRG_SEL_Pos   (12U)                    /*!< TTRG_SEL (Bit 12)                                     */
#define TIMER_CHANNEL_CTRL_TTRG_SEL_Msk   (0xf000U)                /*!< TTRG_SEL (Bitfield-Mask: 0x0f)                        */
#define TIMER_CHANNEL_CTRL_TRG_SRC_Pos    (16U)                    /*!< TRG_SRC (Bit 16)                                      */
#define TIMER_CHANNEL_CTRL_TRG_SRC_Msk    (0x10000U)               /*!< TRG_SRC (Bitfield-Mask: 0x01)                         */
/* =========================================================  TMVAL  ========================================================= */
#define TIMER_CHANNEL_TMVAL_TMVAL_Pos    (0U)                     /*!< TMVAL (Bit 0)                                          */
#define TIMER_CHANNEL_TMVAL_TMVAL_Msk    (0xffffffffU)            /*!< TMVAL (Bitfield-Mask: 0xffffffff)                      */

/* =========================================================================================================================== */
/* ================                                            CTU                                            ================ */
/* =========================================================================================================================== */
#define CTU_TRGMUX_SEL_Msk                (0x7fU)
#define CTU_TRGMUX_SEL_SHIFT              (8U)
/* ======================================================  TRGMUX_DMA  ======================================================= */
#define CTU_TRGMUX_DMA_SEL_DMA_CH0_Pos    (0U)                     /*!< SEL_DMA_CH0 (Bit 0)                                   */
#define CTU_TRGMUX_DMA_SEL_DMA_CH0_Msk    (0x7fU)                  /*!< SEL_DMA_CH0 (Bitfield-Mask: 0x7f)                     */
#define CTU_TRGMUX_DMA_SEL_DMA_CH1_Pos    (8U)                     /*!< SEL_DMA_CH1 (Bit 8)                                   */
#define CTU_TRGMUX_DMA_SEL_DMA_CH1_Msk    (0x7f00U)                /*!< SEL_DMA_CH1 (Bitfield-Mask: 0x7f)                     */
#define CTU_TRGMUX_DMA_SEL_DMA_CH2_Pos    (16U)                    /*!< SEL_DMA_CH2 (Bit 16)                                  */
#define CTU_TRGMUX_DMA_SEL_DMA_CH2_Msk    (0x7f0000U)              /*!< SEL_DMA_CH2 (Bitfield-Mask: 0x7f)                     */
#define CTU_TRGMUX_DMA_SEL_DMA_CH3_Pos    (24U)                    /*!< SEL_DMA_CH3 (Bit 24)                                  */
#define CTU_TRGMUX_DMA_SEL_DMA_CH3_Msk    (0x7f000000U)            /*!< SEL_DMA_CH3 (Bitfield-Mask: 0x7f)                     */
#define CTU_TRGMUX_DMA_LK1_Pos            (31U)                    /*!< LK1 (Bit 31)                                          */
#define CTU_TRGMUX_DMA_LK1_Msk            (0x80000000U)            /*!< LK1 (Bitfield-Mask: 0x01)                             */
/* ====================================================  TRGMUX_EXTOUT0  ===================================================== */
#define CTU_TRGMUX_EXTOUT0_SEL_EXT_OUT0_TR_Pos (0U)                /*!< SEL_EXT_OUT0_TR (Bit 0)                               */
#define CTU_TRGMUX_EXTOUT0_SEL_EXT_OUT0_TR_Msk (0x7fU)             /*!< SEL_EXT_OUT0_TR (Bitfield-Mask: 0x7f)                 */
#define CTU_TRGMUX_EXTOUT0_SEL_EXT_OUT1_TR_Pos (8U)                /*!< SEL_EXT_OUT1_TR (Bit 8)                               */
#define CTU_TRGMUX_EXTOUT0_SEL_EXT_OUT1_TR_Msk (0x7f00U)           /*!< SEL_EXT_OUT1_TR (Bitfield-Mask: 0x7f)                 */
#define CTU_TRGMUX_EXTOUT0_SEL_EXT_OUT2_TR_Pos (16U)               /*!< SEL_EXT_OUT2_TR (Bit 16)                              */
#define CTU_TRGMUX_EXTOUT0_SEL_EXT_OUT2_TR_Msk (0x7f0000U)         /*!< SEL_EXT_OUT2_TR (Bitfield-Mask: 0x7f)                 */
#define CTU_TRGMUX_EXTOUT0_SEL_EXT_OUT3_TR_Pos (24U)               /*!< SEL_EXT_OUT3_TR (Bit 24)                              */
#define CTU_TRGMUX_EXTOUT0_SEL_EXT_OUT3_TR_Msk (0x7f000000U)       /*!< SEL_EXT_OUT3_TR (Bitfield-Mask: 0x7f)                 */
#define CTU_TRGMUX_EXTOUT0_LK2_Pos        (31U)                    /*!< LK2 (Bit 31)                                          */
#define CTU_TRGMUX_EXTOUT0_LK2_Msk        (0x80000000U)            /*!< LK2 (Bitfield-Mask: 0x01)                             */
/* ====================================================  TRGMUX_EXTOUT1  ===================================================== */
#define CTU_TRGMUX_EXTOUT1_SEL_EXT_OUT4_TR_Pos (0U)                /*!< SEL_EXT_OUT4_TR (Bit 0)                               */
#define CTU_TRGMUX_EXTOUT1_SEL_EXT_OUT4_TR_Msk (0x7fU)             /*!< SEL_EXT_OUT4_TR (Bitfield-Mask: 0x7f)                 */
#define CTU_TRGMUX_EXTOUT1_SEL_EXT_OUT5_TR_Pos (8U)                /*!< SEL_EXT_OUT5_TR (Bit 8)                               */
#define CTU_TRGMUX_EXTOUT1_SEL_EXT_OUT5_TR_Msk (0x7f00U)           /*!< SEL_EXT_OUT5_TR (Bitfield-Mask: 0x7f)                 */
#define CTU_TRGMUX_EXTOUT1_SEL_EXT_OUT6_TR_Pos (16U)               /*!< SEL_EXT_OUT6_TR (Bit 16)                              */
#define CTU_TRGMUX_EXTOUT1_SEL_EXT_OUT6_TR_Msk (0x7f0000U)         /*!< SEL_EXT_OUT6_TR (Bitfield-Mask: 0x7f)                 */
#define CTU_TRGMUX_EXTOUT1_SEL_EXT_OUT7_TR_Pos (24U)               /*!< SEL_EXT_OUT7_TR (Bit 24)                              */
#define CTU_TRGMUX_EXTOUT1_SEL_EXT_OUT7_TR_Msk (0x7f000000U)       /*!< SEL_EXT_OUT7_TR (Bitfield-Mask: 0x7f)                 */
#define CTU_TRGMUX_EXTOUT1_LK3_Pos        (31U)                    /*!< LK3 (Bit 31)                                          */
#define CTU_TRGMUX_EXTOUT1_LK3_Msk        (0x80000000U)            /*!< LK3 (Bitfield-Mask: 0x01)                             */
/* ====================================================  TRGMUX_ADC0_REG  ==================================================== */
#define CTU_TRGMUX_ADC0_REG_SEL_ADC0_REGULAR0_TR_Pos (0U)          /*!< SEL_ADC0_REGULAR0_TR (Bit 0)                          */
#define CTU_TRGMUX_ADC0_REG_SEL_ADC0_REGULAR0_TR_Msk (0x7fU)       /*!< SEL_ADC0_REGULAR0_TR (Bitfield-Mask: 0x7f)            */
#define CTU_TRGMUX_ADC0_REG_SEL_ADC0_REGULAR1_TR_Pos (8U)          /*!< SEL_ADC0_REGULAR1_TR (Bit 8)                          */
#define CTU_TRGMUX_ADC0_REG_SEL_ADC0_REGULAR1_TR_Msk (0x7f00U)     /*!< SEL_ADC0_REGULAR1_TR (Bitfield-Mask: 0x7f)            */
#define CTU_TRGMUX_ADC0_REG_SEL_ADC0_REGULAR2_TR_Pos (16U)         /*!< SEL_ADC0_REGULAR2_TR (Bit 16)                         */
#define CTU_TRGMUX_ADC0_REG_SEL_ADC0_REGULAR2_TR_Msk (0x7f0000U)   /*!< SEL_ADC0_REGULAR2_TR (Bitfield-Mask: 0x7f)            */
#define CTU_TRGMUX_ADC0_REG_SEL_ADC0_REGULAR3_TR_Pos (24U)         /*!< SEL_ADC0_REGULAR3_TR (Bit 24)                         */
#define CTU_TRGMUX_ADC0_REG_SEL_ADC0_REGULAR3_TR_Msk (0x7f000000U) /*!< SEL_ADC0_REGULAR3_TR (Bitfield-Mask: 0x7f)            */
#define CTU_TRGMUX_ADC0_REG_LK4_Pos       (31U)                    /*!< LK4 (Bit 31)                                          */
#define CTU_TRGMUX_ADC0_REG_LK4_Msk       (0x80000000U)            /*!< LK4 (Bitfield-Mask: 0x01)                             */
/* ====================================================  TRGMUX_ADC0_INJ  ==================================================== */
#define CTU_TRGMUX_ADC0_INJ_SEL_ADC0_INJECTION0_TR_Pos (0U)        /*!< SEL_ADC0_INJECTION0_TR (Bit 0)                        */
#define CTU_TRGMUX_ADC0_INJ_SEL_ADC0_INJECTION0_TR_Msk (0x7fU)     /*!< SEL_ADC0_INJECTION0_TR (Bitfield-Mask: 0x7f)          */
#define CTU_TRGMUX_ADC0_INJ_SEL_ADC0_INJECTION1_TR_Pos (8U)        /*!< SEL_ADC0_INJECTION1_TR (Bit 8)                        */
#define CTU_TRGMUX_ADC0_INJ_SEL_ADC0_INJECTION1_TR_Msk (0x7f00U)   /*!< SEL_ADC0_INJECTION1_TR (Bitfield-Mask: 0x7f)          */
#define CTU_TRGMUX_ADC0_INJ_SEL_ADC0_INJECTION2_TR_Pos (16U)       /*!< SEL_ADC0_INJECTION2_TR (Bit 16)                       */
#define CTU_TRGMUX_ADC0_INJ_SEL_ADC0_INJECTION2_TR_Msk (0x7f0000U) /*!< SEL_ADC0_INJECTION2_TR (Bitfield-Mask: 0x7f)          */
#define CTU_TRGMUX_ADC0_INJ_SEL_ADC0_INJECTION3_TR_Pos (24U)       /*!< SEL_ADC0_INJECTION3_TR (Bit 24)                       */
#define CTU_TRGMUX_ADC0_INJ_SEL_ADC0_INJECTION3_TR_Msk (0x7f000000U) /*!< SEL_ADC0_INJECTION3_TR (Bitfield-Mask: 0x7f)        */
#define CTU_TRGMUX_ADC0_INJ_LK5_Pos       (31U)                    /*!< LK5 (Bit 31)                                          */
#define CTU_TRGMUX_ADC0_INJ_LK5_Msk       (0x80000000U)            /*!< LK5 (Bitfield-Mask: 0x01)                             */
/* ======================================================  TRGMUX_ACMP  ====================================================== */
#define CTU_TRGMUX_ACMP_SEL_ACMP0_TR_Pos  (0U)                     /*!< SEL_ACMP0_TR (Bit 0)                                  */
#define CTU_TRGMUX_ACMP_SEL_ACMP0_TR_Msk  (0x7fU)                  /*!< SEL_ACMP0_TR (Bitfield-Mask: 0x7f)                    */
/* ======================================================  TRGMUX_PWM0  ====================================================== */
#define CTU_TRGMUX_PWM0_SEL_PWM0_TRIG0_TR_Pos (0U)                 /*!< SEL_PWM0_TRIG0_TR (Bit 0)                             */
#define CTU_TRGMUX_PWM0_SEL_PWM0_TRIG0_TR_Msk (0x7fU)              /*!< SEL_PWM0_TRIG0_TR (Bitfield-Mask: 0x7f)               */
#define CTU_TRGMUX_PWM0_SEL_PWM0_FAULT0_TR_Pos (8U)                /*!< SEL_PWM0_FAULT0_TR (Bit 8)                            */
#define CTU_TRGMUX_PWM0_SEL_PWM0_FAULT0_TR_Msk (0x7f00U)           /*!< SEL_PWM0_FAULT0_TR (Bitfield-Mask: 0x7f)              */
#define CTU_TRGMUX_PWM0_SEL_PWM0_FAULT1_TR_Pos (16U)               /*!< SEL_PWM0_FAULT1_TR (Bit 16)                           */
#define CTU_TRGMUX_PWM0_SEL_PWM0_FAULT1_TR_Msk (0x7f0000U)         /*!< SEL_PWM0_FAULT1_TR (Bitfield-Mask: 0x7f)              */
#define CTU_TRGMUX_PWM0_SEL_PWM0_FAULT2_TR_Pos (24U)               /*!< SEL_PWM0_FAULT2_TR (Bit 24)                           */
#define CTU_TRGMUX_PWM0_SEL_PWM0_FAULT2_TR_Msk (0x7f000000U)       /*!< SEL_PWM0_FAULT2_TR (Bitfield-Mask: 0x7f)              */
#define CTU_TRGMUX_PWM0_LK9_Pos           (31U)                    /*!< LK9 (Bit 31)                                          */
#define CTU_TRGMUX_PWM0_LK9_Msk           (0x80000000U)            /*!< LK9 (Bitfield-Mask: 0x01)                             */
/* ======================================================  TRGMUX_PWM1  ====================================================== */
#define CTU_TRGMUX_PWM1_SEL_PWM1_TRIG0_TR_Pos (0U)                 /*!< SEL_PWM1_TRIG0_TR (Bit 0)                             */
#define CTU_TRGMUX_PWM1_SEL_PWM1_TRIG0_TR_Msk (0x7fU)              /*!< SEL_PWM1_TRIG0_TR (Bitfield-Mask: 0x7f)               */
#define CTU_TRGMUX_PWM1_SEL_PWM1_FAULT0_TR_Pos (8U)                /*!< SEL_PWM1_FAULT0_TR (Bit 8)                            */
#define CTU_TRGMUX_PWM1_SEL_PWM1_FAULT0_TR_Msk (0x7f00U)           /*!< SEL_PWM1_FAULT0_TR (Bitfield-Mask: 0x7f)              */
#define CTU_TRGMUX_PWM1_SEL_PWM1_FAULT1_TR_Pos (16U)               /*!< SEL_PWM1_FAULT1_TR (Bit 16)                           */
#define CTU_TRGMUX_PWM1_SEL_PWM1_FAULT1_TR_Msk (0x7f0000U)         /*!< SEL_PWM1_FAULT1_TR (Bitfield-Mask: 0x7f)              */
#define CTU_TRGMUX_PWM1_SEL_PWM1_FAULT2_TR_Pos (24U)               /*!< SEL_PWM1_FAULT2_TR (Bit 24)                           */
#define CTU_TRGMUX_PWM1_SEL_PWM1_FAULT2_TR_Msk (0x7f000000U)       /*!< SEL_PWM1_FAULT2_TR (Bitfield-Mask: 0x7f)              */
#define CTU_TRGMUX_PWM1_LK10_Pos          (31U)                    /*!< LK10 (Bit 31)                                         */
#define CTU_TRGMUX_PWM1_LK10_Msk          (0x80000000U)            /*!< LK10 (Bitfield-Mask: 0x01)                            */
/* ======================================================  TRGMUX_PWM2  ====================================================== */
#define CTU_TRGMUX_PWM2_SEL_PWM2_TRIG0_TR_Pos (0U)                 /*!< SEL_PWM2_TRIG0_TR (Bit 0)                             */
#define CTU_TRGMUX_PWM2_SEL_PWM2_TRIG0_TR_Msk (0x7fU)              /*!< SEL_PWM2_TRIG0_TR (Bitfield-Mask: 0x7f)               */
#define CTU_TRGMUX_PWM2_SEL_PWM2_FAULT0_TR_Pos (8U)                /*!< SEL_PWM2_FAULT0_TR (Bit 8)                            */
#define CTU_TRGMUX_PWM2_SEL_PWM2_FAULT0_TR_Msk (0x7f00U)           /*!< SEL_PWM2_FAULT0_TR (Bitfield-Mask: 0x7f)              */
#define CTU_TRGMUX_PWM2_SEL_PWM2_FAULT1_TR_Pos (16U)               /*!< SEL_PWM2_FAULT1_TR (Bit 16)                           */
#define CTU_TRGMUX_PWM2_SEL_PWM2_FAULT1_TR_Msk (0x7f0000U)         /*!< SEL_PWM2_FAULT1_TR (Bitfield-Mask: 0x7f)              */
#define CTU_TRGMUX_PWM2_SEL_PWM2_FAULT2_TR_Pos (24U)               /*!< SEL_PWM2_FAULT2_TR (Bit 24)                           */
#define CTU_TRGMUX_PWM2_SEL_PWM2_FAULT2_TR_Msk (0x7f000000U)       /*!< SEL_PWM2_FAULT2_TR (Bitfield-Mask: 0x7f)              */
#define CTU_TRGMUX_PWM2_LK11_Pos          (31U)                    /*!< LK11 (Bit 31)                                         */
#define CTU_TRGMUX_PWM2_LK11_Msk          (0x80000000U)            /*!< LK11 (Bitfield-Mask: 0x01)                            */
/* ======================================================  TRGMUX_PWM3  ====================================================== */
#define CTU_TRGMUX_PWM3_SEL_PWM3_TRIG0_TR_Pos (0U)                 /*!< SEL_PWM3_TRIG0_TR (Bit 0)                             */
#define CTU_TRGMUX_PWM3_SEL_PWM3_TRIG0_TR_Msk (0x7fU)              /*!< SEL_PWM3_TRIG0_TR (Bitfield-Mask: 0x7f)               */
#define CTU_TRGMUX_PWM3_SEL_PWM3_FAULT0_TR_Pos (8U)                /*!< SEL_PWM3_FAULT0_TR (Bit 8)                            */
#define CTU_TRGMUX_PWM3_SEL_PWM3_FAULT0_TR_Msk (0x7f00U)           /*!< SEL_PWM3_FAULT0_TR (Bitfield-Mask: 0x7f)              */
#define CTU_TRGMUX_PWM3_SEL_PWM3_FAULT1_TR_Pos (16U)               /*!< SEL_PWM3_FAULT1_TR (Bit 16)                           */
#define CTU_TRGMUX_PWM3_SEL_PWM3_FAULT1_TR_Msk (0x7f0000U)         /*!< SEL_PWM3_FAULT1_TR (Bitfield-Mask: 0x7f)              */
#define CTU_TRGMUX_PWM3_SEL_PWM3_FAULT2_TR_Pos (24U)               /*!< SEL_PWM3_FAULT2_TR (Bit 24)                           */
#define CTU_TRGMUX_PWM3_SEL_PWM3_FAULT2_TR_Msk (0x7f000000U)       /*!< SEL_PWM3_FAULT2_TR (Bitfield-Mask: 0x7f)              */
#define CTU_TRGMUX_PWM3_LK12_Pos          (31U)                    /*!< LK12 (Bit 31)                                         */
#define CTU_TRGMUX_PWM3_LK12_Msk          (0x80000000U)            /*!< LK12 (Bitfield-Mask: 0x01)                            */
/* =====================================================  TRGMUX_TIMER  ====================================================== */
#define CTU_TRGMUX_TIMER_SEL_TIMER_CH0_TR_Pos (0U)                 /*!< SEL_TIMER_CH0_TR (Bit 0)                              */
#define CTU_TRGMUX_TIMER_SEL_TIMER_CH0_TR_Msk (0x7fU)              /*!< SEL_TIMER_CH0_TR (Bitfield-Mask: 0x7f)                */
#define CTU_TRGMUX_TIMER_SEL_TIMER_CH1_TR_Pos (8U)                 /*!< SEL_TIMER_CH1_TR (Bit 8)                              */
#define CTU_TRGMUX_TIMER_SEL_TIMER_CH1_TR_Msk (0x7f00U)            /*!< SEL_TIMER_CH1_TR (Bitfield-Mask: 0x7f)                */
#define CTU_TRGMUX_TIMER_SEL_TIMER_CH2_TR_Pos (16U)                /*!< SEL_TIMER_CH2_TR (Bit 16)                             */
#define CTU_TRGMUX_TIMER_SEL_TIMER_CH2_TR_Msk (0x7f0000U)          /*!< SEL_TIMER_CH2_TR (Bitfield-Mask: 0x7f)                */
#define CTU_TRGMUX_TIMER_SEL_TIMER_CH3_TR_Pos (24U)                /*!< SEL_TIMER_CH3_TR (Bit 24)                             */
#define CTU_TRGMUX_TIMER_SEL_TIMER_CH3_TR_Msk (0x7f000000U)        /*!< SEL_TIMER_CH3_TR (Bitfield-Mask: 0x7f)                */
#define CTU_TRGMUX_TIMER_LK15_Pos         (31U)                    /*!< LK15 (Bit 31)                                         */
#define CTU_TRGMUX_TIMER_LK15_Msk         (0x80000000U)            /*!< LK15 (Bitfield-Mask: 0x01)                            */
/* =====================================================  TRGMUX_UART0  ====================================================== */
#define CTU_TRGMUX_UART0_SEL_UART0_TR_Pos (0U)                     /*!< SEL_UART0_TR (Bit 0)                                  */
#define CTU_TRGMUX_UART0_SEL_UART0_TR_Msk (0x7fU)                  /*!< SEL_UART0_TR (Bitfield-Mask: 0x7f)                    */
#define CTU_TRGMUX_UART0_LK17_Pos         (31U)                    /*!< LK17 (Bit 31)                                         */
#define CTU_TRGMUX_UART0_LK17_Msk         (0x80000000U)            /*!< LK17 (Bitfield-Mask: 0x01)                            */
/* =====================================================  TRGMUX_UART1  ====================================================== */
#define CTU_TRGMUX_UART1_SEL_UART1_TR_Pos (0U)                     /*!< SEL_UART1_TR (Bit 0)                                  */
#define CTU_TRGMUX_UART1_SEL_UART1_TR_Msk (0x7fU)                  /*!< SEL_UART1_TR (Bitfield-Mask: 0x7f)                    */
#define CTU_TRGMUX_UART1_LK18_Pos         (31U)                    /*!< LK18 (Bit 31)                                         */
#define CTU_TRGMUX_UART1_LK18_Msk         (0x80000000U)            /*!< LK18 (Bitfield-Mask: 0x01)                            */
/* ======================================================  TRGMUX_EIO  ======================================================= */
#define CTU_TRGMUX_EIO_SEL_EIO_TIMER0_TR_Pos (0U)                  /*!< SEL_EIO_TIMER0_TR (Bit 0)                             */
#define CTU_TRGMUX_EIO_SEL_EIO_TIMER0_TR_Msk (0x7fU)               /*!< SEL_EIO_TIMER0_TR (Bitfield-Mask: 0x7f)               */
#define CTU_TRGMUX_EIO_SEL_EIO_TIMER1_TR_Pos (8U)                  /*!< SEL_EIO_TIMER1_TR (Bit 8)                             */
#define CTU_TRGMUX_EIO_SEL_EIO_TIMER1_TR_Msk (0x7f00U)             /*!< SEL_EIO_TIMER1_TR (Bitfield-Mask: 0x7f)               */
#define CTU_TRGMUX_EIO_SEL_EIO_TIMER2_TR_Pos (16U)                 /*!< SEL_EIO_TIMER2_TR (Bit 16)                            */
#define CTU_TRGMUX_EIO_SEL_EIO_TIMER2_TR_Msk (0x7f0000U)           /*!< SEL_EIO_TIMER2_TR (Bitfield-Mask: 0x7f)               */
#define CTU_TRGMUX_EIO_SEL_EIO_TIMER3_TR_Pos (24U)                 /*!< SEL_EIO_TIMER3_TR (Bit 24)                            */
#define CTU_TRGMUX_EIO_SEL_EIO_TIMER3_TR_Msk (0x7f000000U)         /*!< SEL_EIO_TIMER3_TR (Bitfield-Mask: 0x7f)               */
#define CTU_TRGMUX_EIO_LK21_Pos           (31U)                    /*!< LK21 (Bit 31)                                         */
#define CTU_TRGMUX_EIO_LK21_Msk           (0x80000000U)            /*!< LK21 (Bitfield-Mask: 0x01)                            */
/* ========================================================  CTU_CFG  ======================================================== */
#define CTU_CTU_CFG_SEL_UART0_Pos         (0U)                     /*!< SEL_UART0 (Bit 0)                                     */
#define CTU_CTU_CFG_SEL_UART0_Msk         (0x1U)                   /*!< SEL_UART0 (Bitfield-Mask: 0x01)                       */
#define CTU_CTU_CFG_SEL_UART1_Pos         (1U)                     /*!< SEL_UART1 (Bit 1)                                     */
#define CTU_CTU_CFG_SEL_UART1_Msk         (0x2U)                   /*!< SEL_UART1 (Bitfield-Mask: 0x01)                       */
#define CTU_CTU_CFG_SEL_UART0_TX_Pos      (2U)                     /*!< SEL_UART0_TX (Bit 2)                                  */
#define CTU_CTU_CFG_SEL_UART0_TX_Msk      (0x4U)                   /*!< SEL_UART0_TX (Bitfield-Mask: 0x01)                    */
#define CTU_CTU_CFG_SEL_UART1_TX_Pos      (3U)                     /*!< SEL_UART1_TX (Bit 3)                                  */
#define CTU_CTU_CFG_SEL_UART1_TX_Msk      (0x8U)                   /*!< SEL_UART1_TX (Bitfield-Mask: 0x01)                    */
#define CTU_CTU_CFG_SEL_PWM1_CH0_Pos      (4U)                     /*!< SEL_PWM1_CH0 (Bit 4)                                  */
#define CTU_CTU_CFG_SEL_PWM1_CH0_Msk      (0x10U)                  /*!< SEL_PWM1_CH0 (Bitfield-Mask: 0x01)                    */
#define CTU_CTU_CFG_SEL_PWM3_CH0_Pos      (5U)                     /*!< SEL_PWM3_CH0 (Bit 5)                                  */
#define CTU_CTU_CFG_SEL_PWM3_CH0_Msk      (0x20U)                  /*!< SEL_PWM3_CH0 (Bitfield-Mask: 0x01)                    */
#define CTU_CTU_CFG_SEL_PWM2_HALL_Pos     (6U)                     /*!< SEL_PWM2_HALL (Bit 6)                                 */
#define CTU_CTU_CFG_SEL_PWM2_HALL_Msk     (0x40U)                  /*!< SEL_PWM2_HALL (Bitfield-Mask: 0x01)                   */
#define CTU_CTU_CFG_SEL_PWM0SYNC_Pos      (9U)                     /*!< SEL_PWM0SYNC (Bit 9)                                  */
#define CTU_CTU_CFG_SEL_PWM0SYNC_Msk      (0x200U)                 /*!< SEL_PWM0SYNC (Bitfield-Mask: 0x01)                    */
#define CTU_CTU_CFG_SEL_PWM1SYNC_Pos      (10U)                    /*!< SEL_PWM1SYNC (Bit 10)                                 */
#define CTU_CTU_CFG_SEL_PWM1SYNC_Msk      (0x400U)                 /*!< SEL_PWM1SYNC (Bitfield-Mask: 0x01)                    */
#define CTU_CTU_CFG_SEL_PWM2SYNC_Pos      (11U)                    /*!< SEL_PWM2SYNC (Bit 11)                                 */
#define CTU_CTU_CFG_SEL_PWM2SYNC_Msk      (0x800U)                 /*!< SEL_PWM2SYNC (Bitfield-Mask: 0x01)                    */
#define CTU_CTU_CFG_SEL_PWM3SYNC_Pos      (12U)                    /*!< SEL_PWM3SYNC (Bit 12)                                 */
#define CTU_CTU_CFG_SEL_PWM3SYNC_Msk      (0x1000U)                /*!< SEL_PWM3SYNC (Bitfield-Mask: 0x01)                    */
#define CTU_CTU_CFG_SEL_PWM0_FAULT0_Pos   (15U)                    /*!< SEL_PWM0_FAULT0 (Bit 15)                              */
#define CTU_CTU_CFG_SEL_PWM0_FAULT0_Msk   (0x8000U)                /*!< SEL_PWM0_FAULT0 (Bitfield-Mask: 0x01)                 */
#define CTU_CTU_CFG_SEL_PWM0_FAULT1_Pos   (16U)                    /*!< SEL_PWM0_FAULT1 (Bit 16)                              */
#define CTU_CTU_CFG_SEL_PWM0_FAULT1_Msk   (0x10000U)               /*!< SEL_PWM0_FAULT1 (Bitfield-Mask: 0x01)                 */
#define CTU_CTU_CFG_SEL_PWM0_FAULT2_Pos   (17U)                    /*!< SEL_PWM0_FAULT2 (Bit 17)                              */
#define CTU_CTU_CFG_SEL_PWM0_FAULT2_Msk   (0x20000U)               /*!< SEL_PWM0_FAULT2 (Bitfield-Mask: 0x01)                 */
#define CTU_CTU_CFG_SEL_PWM1_FAULT0_Pos   (19U)                    /*!< SEL_PWM1_FAULT0 (Bit 19)                              */
#define CTU_CTU_CFG_SEL_PWM1_FAULT0_Msk   (0x80000U)               /*!< SEL_PWM1_FAULT0 (Bitfield-Mask: 0x01)                 */
#define CTU_CTU_CFG_SEL_PWM1_FAULT1_Pos   (20U)                    /*!< SEL_PWM1_FAULT1 (Bit 20)                              */
#define CTU_CTU_CFG_SEL_PWM1_FAULT1_Msk   (0x100000U)              /*!< SEL_PWM1_FAULT1 (Bitfield-Mask: 0x01)                 */
#define CTU_CTU_CFG_SEL_PWM1_FAULT2_Pos   (21U)                    /*!< SEL_PWM1_FAULT2 (Bit 21)                              */
#define CTU_CTU_CFG_SEL_PWM1_FAULT2_Msk   (0x200000U)              /*!< SEL_PWM1_FAULT2 (Bitfield-Mask: 0x01)                 */
#define CTU_CTU_CFG_SEL_PWM2_FAULT0_Pos   (23U)                    /*!< SEL_PWM2_FAULT0 (Bit 23)                              */
#define CTU_CTU_CFG_SEL_PWM2_FAULT0_Msk   (0x800000U)              /*!< SEL_PWM2_FAULT0 (Bitfield-Mask: 0x01)                 */
#define CTU_CTU_CFG_SEL_PWM2_FAULT1_Pos   (24U)                    /*!< SEL_PWM2_FAULT1 (Bit 24)                              */
#define CTU_CTU_CFG_SEL_PWM2_FAULT1_Msk   (0x1000000U)             /*!< SEL_PWM2_FAULT1 (Bitfield-Mask: 0x01)                 */
#define CTU_CTU_CFG_SEL_PWM2_FAULT2_Pos   (25U)                    /*!< SEL_PWM2_FAULT2 (Bit 25)                              */
#define CTU_CTU_CFG_SEL_PWM2_FAULT2_Msk   (0x2000000U)             /*!< SEL_PWM2_FAULT2 (Bitfield-Mask: 0x01)                 */
#define CTU_CTU_CFG_SEL_PWM3_FAULT0_Pos   (27U)                    /*!< SEL_PWM3_FAULT0 (Bit 27)                              */
#define CTU_CTU_CFG_SEL_PWM3_FAULT0_Msk   (0x8000000U)             /*!< SEL_PWM3_FAULT0 (Bitfield-Mask: 0x01)                 */
#define CTU_CTU_CFG_SEL_PWM3_FAULT1_Pos   (28U)                    /*!< SEL_PWM3_FAULT1 (Bit 28)                              */
#define CTU_CTU_CFG_SEL_PWM3_FAULT1_Msk   (0x10000000U)            /*!< SEL_PWM3_FAULT1 (Bitfield-Mask: 0x01)                 */
#define CTU_CTU_CFG_SEL_PWM3_FAULT2_Pos   (29U)                    /*!< SEL_PWM3_FAULT2 (Bit 29)                              */
#define CTU_CTU_CFG_SEL_PWM3_FAULT2_Msk   (0x20000000U)            /*!< SEL_PWM3_FAULT2 (Bitfield-Mask: 0x01)                 */
#define CTU_CTU_CFG_RTC_CLK_CAP_Pos       (31U)                    /*!< RTC_CLK_CAP (Bit 31)                                  */
#define CTU_CTU_CFG_RTC_CLK_CAP_Msk       (0x80000000U)            /*!< RTC_CLK_CAP (Bitfield-Mask: 0x01)                     */
/* ========================================================  CTU_SW  ========================================================= */
#define CTU_CTU_SW_SW_TRIG1_TS_Pos        (0U)                     /*!< SW_TRIG1_TS (Bit 0)                                   */
#define CTU_CTU_SW_SW_TRIG1_TS_Msk        (0x1U)                   /*!< SW_TRIG1_TS (Bitfield-Mask: 0x01)                     */
#define CTU_CTU_SW_SW_TRIG2_TS_Pos        (1U)                     /*!< SW_TRIG2_TS (Bit 1)                                   */
#define CTU_CTU_SW_SW_TRIG2_TS_Msk        (0x2U)                   /*!< SW_TRIG2_TS (Bitfield-Mask: 0x01)                     */
#define CTU_CTU_SW_SW_TRIG3_TS_Pos        (2U)                     /*!< SW_TRIG3_TS (Bit 2)                                   */
#define CTU_CTU_SW_SW_TRIG3_TS_Msk        (0x4U)                   /*!< SW_TRIG3_TS (Bitfield-Mask: 0x01)                     */
#define CTU_CTU_SW_SW_TRIG4_TS_Pos        (3U)                     /*!< SW_TRIG4_TS (Bit 3)                                   */
#define CTU_CTU_SW_SW_TRIG4_TS_Msk        (0x8U)                   /*!< SW_TRIG4_TS (Bitfield-Mask: 0x01)                     */
/* ====================================================  PWM_MODULATION  ===================================================== */
#define CTU_PWM_MODULATION_PWM0_OUTSEL_Pos (0U)                    /*!< PWM0_OUTSEL (Bit 0)                                   */
#define CTU_PWM_MODULATION_PWM0_OUTSEL_Msk (0xffU)                 /*!< PWM0_OUTSEL (Bitfield-Mask: 0xff)                     */
#define CTU_PWM_MODULATION_PWM3_OUTSEL_Pos (8U)                    /*!< PWM3_OUTSEL (Bit 8)                                   */
#define CTU_PWM_MODULATION_PWM3_OUTSEL_Msk (0xff00U)               /*!< PWM3_OUTSEL (Bitfield-Mask: 0xff)                     */

/* =========================================================================================================================== */
/* ================                                            DMA                                            ================ */
/* =========================================================================================================================== */

/* ========================================================  TOP_RST  ======================================================== */
#define DMA_TOP_RST_WARM_RST_Pos          (0U)                     /*!< WARM_RST (Bit 0)                                      */
#define DMA_TOP_RST_WARM_RST_Msk          (0x1U)                   /*!< WARM_RST (Bitfield-Mask: 0x01)                        */
#define DMA_TOP_RST_HARD_RST_Pos          (1U)                     /*!< HARD_RST (Bit 1)                                      */
#define DMA_TOP_RST_HARD_RST_Msk          (0x2U)                   /*!< HARD_RST (Bitfield-Mask: 0x01)                        */
#define DMA_TOP_RST_TOP_RST_DMA_IDLE_Pos  (2U)                     /*!< DMA_IDLE (Bit 2)                                      */
#define DMA_TOP_RST_TOP_RST_DMA_IDLE_Msk  (0x4U)                   /*!< DMA_IDLE (Bitfield-Mask: 0x01)                        */

/* =========================================================================================================================== */
/* ================                                       DMA_Channel                                         ================ */
/* =========================================================================================================================== */

/* ========================================================  STATUS  ========================================================= */
#define DMA_CHANNEL_STATUS_FINISH_Pos     (0U)                     /*!< FINISH (Bit 0)                                        */
#define DMA_CHANNEL_STATUS_FINISH_Msk     (0x1U)                   /*!< FINISH (Bitfield-Mask: 0x01)                          */
#define DMA_CHANNEL_STATUS_HALF_FINISH_Pos (1U)                    /*!< HALF_FINISH (Bit 1)                                   */
#define DMA_CHANNEL_STATUS_HALF_FINISH_Msk (0x2U)                  /*!< HALF_FINISH (Bitfield-Mask: 0x01)                     */
#define DMA_CHANNEL_STATUS_DBE_Pos        (2U)                     /*!< DBE (Bit 2)                                           */
#define DMA_CHANNEL_STATUS_DBE_Msk        (0x4U)                   /*!< DBE (Bitfield-Mask: 0x01)                             */
#define DMA_CHANNEL_STATUS_SBE_Pos        (3U)                     /*!< SBE (Bit 3)                                           */
#define DMA_CHANNEL_STATUS_SBE_Msk        (0x8U)                   /*!< SBE (Bitfield-Mask: 0x01)                             */
#define DMA_CHANNEL_STATUS_DOE_Pos        (4U)                     /*!< DOE (Bit 4)                                           */
#define DMA_CHANNEL_STATUS_DOE_Msk        (0x10U)                  /*!< DOE (Bitfield-Mask: 0x01)                             */
#define DMA_CHANNEL_STATUS_SOE_Pos        (5U)                     /*!< SOE (Bit 5)                                           */
#define DMA_CHANNEL_STATUS_SOE_Msk        (0x20U)                  /*!< SOE (Bitfield-Mask: 0x01)                             */
#define DMA_CHANNEL_STATUS_DAE_Pos        (6U)                     /*!< DAE (Bit 6)                                           */
#define DMA_CHANNEL_STATUS_DAE_Msk        (0x40U)                  /*!< DAE (Bitfield-Mask: 0x01)                             */
#define DMA_CHANNEL_STATUS_SAE_Pos        (7U)                     /*!< SAE (Bit 7)                                           */
#define DMA_CHANNEL_STATUS_SAE_Msk        (0x80U)                  /*!< SAE (Bitfield-Mask: 0x01)                             */
#define DMA_CHANNEL_STATUS_CLE_Pos        (8U)                     /*!< CLE (Bit 8)                                           */
#define DMA_CHANNEL_STATUS_CLE_Msk        (0x100U)                 /*!< CLE (Bitfield-Mask: 0x01)                             */
/* =========================================================  INTEN  ========================================================= */
#define DMA_CHANNEL_INTEN_FINISH_INTERRUPT_ENABLE_Pos (0U)         /*!< FINISH_INTERRUPT_ENABLE (Bit 0)                       */
#define DMA_CHANNEL_INTEN_FINISH_INTERRUPT_ENABLE_Msk (0x1U)       /*!< FINISH_INTERRUPT_ENABLE (Bitfield-Mask: 0x01)         */
#define DMA_CHANNEL_INTEN_HALF_FINISH_INTERRUPT_ENABLE_Pos (1U)    /*!< HALF_FINISH_INTERRUPT_ENABLE (Bit 1)                  */
#define DMA_CHANNEL_INTEN_HALF_FINISH_INTERRUPT_ENABLE_Msk (0x2U)  /*!< HALF_FINISH_INTERRUPT_ENABLE (Bitfield-Mask: 0x01)    */
#define DMA_CHANNEL_INTEN_TRANS_ERROR_INTERRUPT_ENABLE_Pos (2U)    /*!< TRANS_ERROR_INTERRUPT_ENABLE (Bit 2)                  */
#define DMA_CHANNEL_INTEN_TRANS_ERROR_INTERRUPT_ENABLE_Msk (0x4U)  /*!< TRANS_ERROR_INTERRUPT_ENABLE (Bitfield-Mask: 0x01)    */
/* ==========================================================  RST  ========================================================== */
#define DMA_CHANNEL_RST_WARM_RST_Pos      (0U)                     /*!< WARM_RST (Bit 0)                                      */
#define DMA_CHANNEL_RST_WARM_RST_Msk      (0x1U)                   /*!< WARM_RST (Bitfield-Mask: 0x01)                        */
#define DMA_CHANNEL_RST_HARD_RST_Pos      (1U)                     /*!< HARD_RST (Bit 1)                                      */
#define DMA_CHANNEL_RST_HARD_RST_Msk      (0x2U)                   /*!< HARD_RST (Bitfield-Mask: 0x01)                        */
#define DMA_CHANNEL_RST_FLUSH_Pos         (2U)                     /*!< FLUSH (Bit 2)                                         */
#define DMA_CHANNEL_RST_FLUSH_Msk         (0x4U)                   /*!< FLUSH (Bitfield-Mask: 0x01)                           */
/* =========================================================  STOP  ========================================================== */
#define DMA_CHANNEL_STOP_STOP_Pos         (0U)                     /*!< STOP (Bit 0)                                          */
#define DMA_CHANNEL_STOP_STOP_Msk         (0x1U)                   /*!< STOP (Bitfield-Mask: 0x01)                            */
/* ========================================================  CONFIG  ========================================================= */
#define DMA_CHANNEL_CONFIG_CHAN_PRIORITY_Pos (0U)                  /*!< CHAN_PRIORITY (Bit 0)                                 */
#define DMA_CHANNEL_CONFIG_CHAN_PRIORITY_Msk (0x3U)                /*!< CHAN_PRIORITY (Bitfield-Mask: 0x03)                   */
#define DMA_CHANNEL_CONFIG_SSIZE_Pos      (2U)                     /*!< SSIZE (Bit 2)                                         */
#define DMA_CHANNEL_CONFIG_SSIZE_Msk      (0xcU)                   /*!< SSIZE (Bitfield-Mask: 0x03)                           */
#define DMA_CHANNEL_CONFIG_DSIZE_Pos      (4U)                     /*!< DSIZE (Bit 4)                                         */
#define DMA_CHANNEL_CONFIG_DSIZE_Msk      (0x30U)                  /*!< DSIZE (Bitfield-Mask: 0x03)                           */
#define DMA_CHANNEL_CONFIG_CHAN_CIRCULAR_Pos (6U)                  /*!< CHAN_CIRCULAR (Bit 6)                                 */
#define DMA_CHANNEL_CONFIG_CHAN_CIRCULAR_Msk (0x40U)               /*!< CHAN_CIRCULAR (Bitfield-Mask: 0x01)                   */
#define DMA_CHANNEL_CONFIG_CHAN_FIFO_Pos   (7U)                    /*!< CHAN_FIFO (Bit 7)                                     */
#define DMA_CHANNEL_CONFIG_CHAN_FIFO_Msk   (0x80U)                 /*!< CHAN_FIFO (Bitfield-Mask: 0x01)                       */
/* ======================================================  CHAN_LENGTH  ====================================================== */
#define DMA_CHANNEL_CHAN_LENGTH_CHAN_LENGTH_Pos (0U)               /*!< CHAN_LENGTH (Bit 0)                                   */
#define DMA_CHANNEL_CHAN_LENGTH_CHAN_LENGTH_Msk (0xfffffU)         /*!< CHAN_LENGTH (Bitfield-Mask: 0xfffff)                  */
/* ======================================================  SSTART_ADDR  ====================================================== */
#define DMA_CHANNEL_SSTART_ADDR_SSTART_ADDR_Pos (0U)               /*!< SSTART_ADDR (Bit 0)                                   */
#define DMA_CHANNEL_SSTART_ADDR_SSTART_ADDR_Msk (0xffffffffU)      /*!< SSTART_ADDR (Bitfield-Mask: 0xffffffff)               */
/* =======================================================  SEND_ADDR  ======================================================= */
#define DMA_CHANNEL_SEND_ADDR_SEND_ADDR_Pos (0U)                   /*!< SEND_ADDR (Bit 0)                                     */
#define DMA_CHANNEL_SEND_ADDR_SEND_ADDR_Msk (0xffffffffU)          /*!< SEND_ADDR (Bitfield-Mask: 0xffffffff)                 */
/* ======================================================  DSTART_ADDR  ====================================================== */
#define DMA_CHANNEL_DSTART_ADDR_DSTART_ADDR_Pos (0U)               /*!< DSTART_ADDR (Bit 0)                                   */
#define DMA_CHANNEL_DSTART_ADDR_DSTART_ADDR_Msk (0xffffffffU)      /*!< DSTART_ADDR (Bitfield-Mask: 0xffffffff)               */
/* ======================================================  CHAN_ENABLE  ====================================================== */
#define DMA_CHANNEL_CHAN_ENABLE_CHAN_ENABLE_Pos (0U)               /*!< CHAN_ENABLE (Bit 0)                                   */
#define DMA_CHANNEL_CHAN_ENABLE_CHAN_ENABLE_Msk (0x1U)             /*!< CHAN_ENABLE (Bitfield-Mask: 0x01)                     */
#define DMA_CHANNEL_CHAN_ENABLE_EDBG_Pos  (1U)                     /*!< EDBG (Bit 1)                                          */
#define DMA_CHANNEL_CHAN_ENABLE_EDBG_Msk  (0x2U)                   /*!< EDBG (Bitfield-Mask: 0x01)                            */
/* ====================================================  DATA_TRANS_NUM  ===================================================== */
#define DMA_CHANNEL_DATA_TRANS_NUM_DATA_TRANS_NUM_Pos (0U)         /*!< DATA_TRANS_NUM (Bit 0)                                */
#define DMA_CHANNEL_DATA_TRANS_NUM_DATA_TRANS_NUM_Msk (0xfffffU)   /*!< DATA_TRANS_NUM (Bitfield-Mask: 0xffff)                */
/* =====================================================  FIFO_LEFT_NUM  ===================================================== */
#define DMA_CHANNEL_FIFO_LEFT_NUM_FIFO_LEFT_NUM_Pos (0U)           /*!< FIFO_LEFT_NUM (Bit 0)                                 */
#define DMA_CHANNEL_FIFO_LEFT_NUM_FIFO_LEFT_NUM_Msk (0x3fU)        /*!< FIFO_LEFT_NUM (Bitfield-Mask: 0x3f)                   */
/* =======================================================  DEND_ADDR  ======================================================= */
#define DMA_CHANNEL_DEND_ADDR_DEND_ADDR_Pos (0U)                   /*!< DEND_ADDR (Bit 0)                                     */
#define DMA_CHANNEL_DEND_ADDR_DEND_ADDR_Msk (0xffffffffU)          /*!< DEND_ADDR (Bitfield-Mask: 0xffffffff)                 */
/* ======================================================  ADDR_OFFSET  ====================================================== */
#define DMA_CHANNEL_ADDR_OFFSET_SOFFSET_Pos (0U)                   /*!< SOFFSET (Bit 0)                                       */
#define DMA_CHANNEL_ADDR_OFFSET_SOFFSET_Msk (0xffffU)              /*!< SOFFSET (Bitfield-Mask: 0xffff)                       */
#define DMA_CHANNEL_ADDR_OFFSET_DOFFSET_Pos (16U)                  /*!< DOFFSET (Bit 16)                                      */
#define DMA_CHANNEL_ADDR_OFFSET_DOFFSET_Msk (0xffff0000U)          /*!< DOFFSET (Bitfield-Mask: 0xffff)                       */
/* ======================================================  DMAMUX_CFG  ======================================================= */
#define DMA_CHANNEL_DMAMUX_CFG_REQ_ID_Pos (0U)                     /*!< REQ_ID (Bit 0)                                        */
#define DMA_CHANNEL_DMAMUX_CFG_REQ_ID_Msk (0x7fU)                  /*!< REQ_ID (Bitfield-Mask: 0x7f)                          */
#define DMA_CHANNEL_DMAMUX_CFG_TRIG_EN_Pos (7U)                    /*!< TRIG_EN (Bit 7)                                       */
#define DMA_CHANNEL_DMAMUX_CFG_TRIG_EN_Msk (0x80U)                 /*!< TRIG_EN (Bitfield-Mask: 0x01)                         */
#define DMA_CHANNEL_DMAMUX_CFG_TRIG_OUT_SEL_Pos (8U)               /*!< TRIG_OUT_SEL (Bit 8)                                  */
#define DMA_CHANNEL_DMAMUX_CFG_TRIG_OUT_SEL_Msk (0x300U)           /*!< TRIG_OUT_SEL (Bitfield-Mask: 0x03)                    */
#define DMA_CHANNEL_DMAMUX_CFG_TRIG_DAISY_Pos (10U)                /*!< TRIG_DAISY (Bit 10)                                   */
#define DMA_CHANNEL_DMAMUX_CFG_TRIG_DAISY_Msk (0x400U)             /*!< TRIG_DAISY (Bitfield-Mask: 0x01)                      */

/* =========================================================================================================================== */
/* ================                                            WDG                                            ================ */
/* =========================================================================================================================== */

/* ==========================================================  CS0  ========================================================== */
#define WDG_CS0_STOP_Pos                  (0U)                     /*!< STOP (Bit 0)                                          */
#define WDG_CS0_STOP_Msk                  (0x1U)                   /*!< STOP (Bitfield-Mask: 0x01)                            */
#define WDG_CS0_DBG_Pos                   (1U)                     /*!< DBG (Bit 1)                                           */
#define WDG_CS0_DBG_Msk                   (0x2U)                   /*!< DBG (Bitfield-Mask: 0x01)                             */
#define WDG_CS0_ULK_Pos                   (2U)                     /*!< ULK (Bit 2)                                           */
#define WDG_CS0_ULK_Msk                   (0x4U)                   /*!< ULK (Bitfield-Mask: 0x01)                             */
#define WDG_CS0_TST_Pos                   (3U)                     /*!< TST (Bit 3)                                           */
#define WDG_CS0_TST_Msk                   (0x18U)                  /*!< TST (Bitfield-Mask: 0x03)                             */
#define WDG_CS0_UPDATE_Pos                (5U)                     /*!< UPDATE (Bit 5)                                        */
#define WDG_CS0_UPDATE_Msk                (0x20U)                  /*!< UPDATE (Bitfield-Mask: 0x01)                          */
#define WDG_CS0_INT_Pos                   (6U)                     /*!< INT (Bit 6)                                           */
#define WDG_CS0_INT_Msk                   (0x40U)                  /*!< INT (Bitfield-Mask: 0x01)                             */
#define WDG_CS0_EN_Pos                    (7U)                     /*!< EN (Bit 7)                                            */
#define WDG_CS0_EN_Msk                    (0x80U)                  /*!< EN (Bitfield-Mask: 0x01)                              */
/* ==========================================================  CS1  ========================================================== */
#define WDG_CS1_CLK_Pos                   (0U)                     /*!< CLK (Bit 0)                                           */
#define WDG_CS1_CLK_Msk                   (0x3U)                   /*!< CLK (Bitfield-Mask: 0x03)                             */
#define WDG_CS1_PRES_Pos                  (4U)                     /*!< PRES (Bit 4)                                          */
#define WDG_CS1_PRES_Msk                  (0x10U)                  /*!< PRES (Bitfield-Mask: 0x01)                            */
#define WDG_CS1_FLG_Pos                   (6U)                     /*!< FLG (Bit 6)                                           */
#define WDG_CS1_FLG_Msk                   (0x40U)                  /*!< FLG (Bitfield-Mask: 0x01)                             */
#define WDG_CS1_WIN_Pos                   (7U)                     /*!< WIN (Bit 7)                                           */
#define WDG_CS1_WIN_Msk                   (0x80U)                  /*!< WIN (Bitfield-Mask: 0x01)                             */
/* ==========================================================  CNT  ========================================================== */
#define WDG_CNT_CNT_Pos                   (0U)                     /*!< CNT (Bit 0)                                           */
#define WDG_CNT_CNT_Msk                   (0xffffffffU)            /*!< CNT (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  TOVAL  ========================================================= */
#define WDG_TOVAL_TOVAL_Pos               (0U)                     /*!< TOVAL (Bit 0)                                         */
#define WDG_TOVAL_TOVAL_Msk               (0xffffU)                /*!< TOVAL (Bitfield-Mask: 0xffff)                         */
/* ==========================================================  WIN  ========================================================== */
#define WDG_WIN_WIN_Pos                   (0U)                     /*!< WIN (Bit 0)                                           */
#define WDG_WIN_WIN_Msk                   (0xffffU)                /*!< WIN (Bitfield-Mask: 0xffff)                           */

/* =========================================================================================================================== */
/* ================                                            RTC                                            ================ */
/* =========================================================================================================================== */

/* =========================================================  CTRL  ========================================================== */
#define RTC_CTRL_SWR_Pos                  (0U)                     /*!< SWR (Bit 0)                                           */
#define RTC_CTRL_SWR_Msk                  (0x1U)                   /*!< SWR (Bitfield-Mask: 0x01)                             */
#define RTC_CTRL_UPM_Pos                  (1U)                     /*!< UPM (Bit 1)                                           */
#define RTC_CTRL_UPM_Msk                  (0x2U)                   /*!< UPM (Bitfield-Mask: 0x01)                             */
#define RTC_CTRL_COS_Pos                  (2U)                     /*!< COS (Bit 2)                                           */
#define RTC_CTRL_COS_Msk                  (0x4U)                   /*!< COS (Bitfield-Mask: 0x01)                             */
#define RTC_CTRL_COE_Pos                  (3U)                     /*!< COE (Bit 3)                                           */
#define RTC_CTRL_COE_Msk                  (0x8U)                   /*!< COE (Bitfield-Mask: 0x01)                             */
#define RTC_CTRL_TIIE_Pos                 (4U)                     /*!< TIIE (Bit 4)                                          */
#define RTC_CTRL_TIIE_Msk                 (0x10U)                  /*!< TIIE (Bitfield-Mask: 0x01)                            */
#define RTC_CTRL_POIE_Pos                 (5U)                     /*!< POIE (Bit 5)                                          */
#define RTC_CTRL_POIE_Msk                 (0x20U)                  /*!< POIE (Bitfield-Mask: 0x01)                            */
#define RTC_CTRL_TOIE_Pos                 (6U)                     /*!< TOIE (Bit 6)                                          */
#define RTC_CTRL_TOIE_Msk                 (0x40U)                  /*!< TOIE (Bitfield-Mask: 0x01)                            */
#define RTC_CTRL_TAIE_Pos                 (7U)                     /*!< TAIE (Bit 7)                                          */
#define RTC_CTRL_TAIE_Msk                 (0x80U)                  /*!< TAIE (Bitfield-Mask: 0x01)                            */
#define RTC_CTRL_CLKSEL_Pos               (16U)                    /*!< CLKSEL (Bit 16)                                       */
#define RTC_CTRL_CLKSEL_Msk               (0x30000U)               /*!< CLKSEL (Bitfield-Mask: 0x03)                          */
/* ==========================================================  TAR  ========================================================== */
#define RTC_TAR_TAR_Pos                   (0U)                     /*!< TAR (Bit 0)                                           */
#define RTC_TAR_TAR_Msk                   (0xffffffffU)            /*!< TAR (Bitfield-Mask: 0xffffffff)                       */
/* ==========================================================  TC  =========================================================== */
#define RTC_TC_TC_Pos                     (0U)                     /*!< TC (Bit 0)                                            */
#define RTC_TC_TC_Msk                     (0xffffffffU)            /*!< TC (Bitfield-Mask: 0xffffffff)                        */
/* ==========================================================  PSR  ========================================================== */
#define RTC_PSR_PSR_Pos                   (0U)                     /*!< PSR (Bit 0)                                           */
#define RTC_PSR_PSR_Msk                   (0xfffffU)               /*!< PSR (Bitfield-Mask: 0xfffff)                          */
/* ==========================================================  PSC  ========================================================== */
#define RTC_PSC_PSC_Pos                   (0U)                     /*!< PSC (Bit 0)                                           */
#define RTC_PSC_PSC_Msk                   (0xfffffU)               /*!< PSC (Bitfield-Mask: 0xfffff)                          */
/* ==========================================================  SR  =========================================================== */
#define RTC_SR_TIF_Pos                    (0U)                     /*!< TIF (Bit 0)                                           */
#define RTC_SR_TIF_Msk                    (0x1U)                   /*!< TIF (Bitfield-Mask: 0x01)                             */
#define RTC_SR_POF_Pos                    (1U)                     /*!< POF (Bit 1)                                           */
#define RTC_SR_POF_Msk                    (0x2U)                   /*!< POF (Bitfield-Mask: 0x01)                             */
#define RTC_SR_TOF_Pos                    (2U)                     /*!< TOF (Bit 2)                                           */
#define RTC_SR_TOF_Msk                    (0x4U)                   /*!< TOF (Bitfield-Mask: 0x01)                             */
#define RTC_SR_TAF_Pos                    (3U)                     /*!< TAF (Bit 3)                                           */
#define RTC_SR_TAF_Msk                    (0x8U)                   /*!< TAF (Bitfield-Mask: 0x01)                             */
#define RTC_SR_TCE_Pos                    (4U)                     /*!< TCE (Bit 4)                                           */
#define RTC_SR_TCE_Msk                    (0x10U)                  /*!< TCE (Bitfield-Mask: 0x01)                             */
#define RTC_SR_TCUF_Pos                   (5U)                     /*!< TCUF (Bit 5)                                          */
#define RTC_SR_TCUF_Msk                   (0x20U)                  /*!< TCUF (Bitfield-Mask: 0x01)                            */
#define RTC_SR_PSCUF_Pos                  (6U)                     /*!< PSCUF (Bit 6)                                         */
#define RTC_SR_PSCUF_Msk                  (0x40U)                  /*!< PSCUF (Bitfield-Mask: 0x01)                           */
/* ==========================================================  LR  =========================================================== */
#define RTC_LR_CRL_Pos                    (0U)                     /*!< CRL (Bit 0)                                           */
#define RTC_LR_CRL_Msk                    (0x1U)                   /*!< CRL (Bitfield-Mask: 0x01)                             */
#define RTC_LR_SRL_Pos                    (1U)                     /*!< SRL (Bit 1)                                           */
#define RTC_LR_SRL_Msk                    (0x2U)                   /*!< SRL (Bitfield-Mask: 0x01)                             */
#define RTC_LR_LRL_Pos                    (2U)                     /*!< LRL (Bit 2)                                           */
#define RTC_LR_LRL_Msk                    (0x4U)                   /*!< LRL (Bitfield-Mask: 0x01)                             */

/* =========================================================================================================================== */
/* ================                                            CRC                                            ================ */
/* =========================================================================================================================== */

/* ========================================================  DATA32  ========================================================= */
#define CRC_DATA32_DATA_Pos               (0U)                     /*!< DATA (Bit 0)                                          */
#define CRC_DATA32_DATA_Msk               (0xffffffffU)            /*!< DATA (Bitfield-Mask: 0xffffffff)                      */
/* =========================================================  POLY  ========================================================== */
#define CRC_POLY_POLY_Pos                 (0U)                     /*!< POLY (Bit 0)                                          */
#define CRC_POLY_POLY_Msk                 (0xffffffffU)            /*!< POLY (Bitfield-Mask: 0xffffffff)                      */
/* =========================================================  CTRL  ========================================================== */
#define CRC_CTRL_TCRC_Pos                 (0U)                     /*!< TCRC (Bit 0)                                          */
#define CRC_CTRL_TCRC_Msk                 (0x1U)                   /*!< TCRC (Bitfield-Mask: 0x01)                            */
#define CRC_CTRL_WAS_Pos                  (1U)                     /*!< WAS (Bit 1)                                           */
#define CRC_CTRL_WAS_Msk                  (0x2U)                   /*!< WAS (Bitfield-Mask: 0x01)                             */
#define CRC_CTRL_FXOR_Pos                 (2U)                     /*!< FXOR (Bit 2)                                          */
#define CRC_CTRL_FXOR_Msk                 (0x4U)                   /*!< FXOR (Bitfield-Mask: 0x01)                            */
#define CRC_CTRL_TOTR_Pos                 (4U)                     /*!< TOTR (Bit 4)                                          */
#define CRC_CTRL_TOTR_Msk                 (0x30U)                  /*!< TOTR (Bitfield-Mask: 0x03)                            */
#define CRC_CTRL_TOTW_Pos                 (6U)                     /*!< TOTW (Bit 6)                                          */
#define CRC_CTRL_TOTW_Msk                 (0xc0U)                  /*!< TOTW (Bitfield-Mask: 0x03)                            */

/* =========================================================================================================================== */
/* ================                                         EIM_CTRL                                          ================ */
/* =========================================================================================================================== */

/* ====================================================  EIM_GLB_ENABLE  ===================================================== */
#define EIM_CTRL_EIM_GLB_ENABLE_GLB_EN_Pos (0U)                    /*!< GLB_EN (Bit 0)                                        */
#define EIM_CTRL_EIM_GLB_ENABLE_GLB_EN_Msk (0x1U)                  /*!< GLB_EN (Bitfield-Mask: 0x01)                          */
/* =======================================================  EIM_CHEN  ======================================================== */
#define EIM_CTRL_EIM_CHEN_EIM0_EN_Pos     (0U)                     /*!< EIM0_EN (Bit 0)                                       */
#define EIM_CTRL_EIM_CHEN_EIM0_EN_Msk     (0x1U)                   /*!< EIM0_EN (Bitfield-Mask: 0x01)                         */
#define EIM_CTRL_EIM_CHEN_EIM1_EN_Pos     (1U)                     /*!< EIM1_EN (Bit 1)                                       */
#define EIM_CTRL_EIM_CHEN_EIM1_EN_Msk     (0x2U)                   /*!< EIM1_EN (Bitfield-Mask: 0x01)                         */

/* =========================================================================================================================== */
/* ================                                       EIM_CHANNEL                                         ================ */
/* =========================================================================================================================== */

/* =====================================================  EIM_INJ_DATA  ====================================================== */
#define EIM_CHANNEL_EIM_INJ_DATA_EIM_DATA_INJ_Pos (0U)             /*!< EIM_DATA_INJ (Bit 0)                                  */
#define EIM_CHANNEL_EIM_INJ_DATA_EIM_DATA_INJ_Msk (0xffffffffU)    /*!< EIM_DATA_INJ (Bitfield-Mask: 0xffffffff)              */
/* =====================================================  EIM_INJ_ADDR  ====================================================== */
#define EIM_CHANNEL_EIM_INJ_ADDR_EIM_ADDR_INJ_Pos (0U)             /*!< EIM_ADDR_INJ (Bit 0)                                  */
#define EIM_CHANNEL_EIM_INJ_ADDR_EIM_ADDR_INJ_Msk (0x1fffU)        /*!< EIM_ADDR_INJ (Bitfield-Mask: 0x1fff)                  */
/* =====================================================  EIM_INJ_ECC  ======================================================= */
#define EIM_CHANNEL_EIM_INJ_ECC_EIM_ECC_INJ_Pos (0U)               /*!< EIM_ECC_INJ (Bit 0)                                   */
#define EIM_CHANNEL_EIM_INJ_ECC_EIM_ECC_INJ_Msk (0x7fU)            /*!< EIM_ECC_INJ (Bitfield-Mask: 0x7f)                     */

/* =========================================================================================================================== */
/* ================                                         ECC_SRAM                                          ================ */
/* =========================================================================================================================== */

/* ======================================================  CH0_STATUS0  ====================================================== */
#define ECC_SRAM_CH0_STATUS0_ERR_ADDR0_Pos (0U)                    /*!< ERR_ADDR0 (Bit 0)                                     */
#define ECC_SRAM_CH0_STATUS0_ERR_ADDR0_Msk (0x1fffU)               /*!< ERR_ADDR0 (Bitfield-Mask: 0x1fff)                     */
#define ECC_SRAM_CH0_STATUS0_ERR_ST_Pos   (16U)                    /*!< ERR_ST (Bit 16)                                       */
#define ECC_SRAM_CH0_STATUS0_ERR_ST_Msk   (0x70000U)               /*!< ERR_ST (Bitfield-Mask: 0x07)                          */
/* ======================================================  CH0_STATUS1  ====================================================== */
#define ECC_SRAM_CH0_STATUS1_ERR_ADDR1_Pos (0U)                    /*!< ERR_ADDR1 (Bit 0)                                     */
#define ECC_SRAM_CH0_STATUS1_ERR_ADDR1_Msk (0x1fffU)               /*!< ERR_ADDR1 (Bitfield-Mask: 0x1fff)                     */
#define ECC_SRAM_CH0_STATUS1_ERR_STS0_Pos (16U)                    /*!< ERR_STS0 (Bit 16)                                     */
#define ECC_SRAM_CH0_STATUS1_ERR_STS0_Msk (0x10000U)               /*!< ERR_STS0 (Bitfield-Mask: 0x01)                        */
#define ECC_SRAM_CH0_STATUS1_ERR_STS1_Pos (17U)                    /*!< ERR_STS1 (Bit 17)                                     */
#define ECC_SRAM_CH0_STATUS1_ERR_STS1_Msk (0x20000U)               /*!< ERR_STS1 (Bitfield-Mask: 0x01)                        */
/* ======================================================  CH1_STATUS0  ====================================================== */
#define ECC_SRAM_CH1_STATUS0_ERR_ADDR0_Pos (0U)                    /*!< ERR_ADDR0 (Bit 0)                                     */
#define ECC_SRAM_CH1_STATUS0_ERR_ADDR0_Msk (0x1fffU)               /*!< ERR_ADDR0 (Bitfield-Mask: 0x1fff)                     */
#define ECC_SRAM_CH1_STATUS0_ERR_ST_Pos   (16U)                    /*!< ERR_ST (Bit 16)                                       */
#define ECC_SRAM_CH1_STATUS0_ERR_ST_Msk   (0x70000U)               /*!< ERR_ST (Bitfield-Mask: 0x07)                          */
/* ======================================================  CH1_STATUS1  ====================================================== */
#define ECC_SRAM_CH1_STATUS1_ERR_ADDR1_Pos (0U)                    /*!< ERR_ADDR1 (Bit 0)                                     */
#define ECC_SRAM_CH1_STATUS1_ERR_ADDR1_Msk (0x1fffU)               /*!< ERR_ADDR1 (Bitfield-Mask: 0x1fff)                     */
#define ECC_SRAM_CH1_STATUS1_ERR_STS0_Pos (16U)                    /*!< ERR_STS0 (Bit 16)                                     */
#define ECC_SRAM_CH1_STATUS1_ERR_STS0_Msk (0x10000U)               /*!< ERR_STS0 (Bitfield-Mask: 0x01)                        */
#define ECC_SRAM_CH1_STATUS1_ERR_STS1_Pos (17U)                    /*!< ERR_STS1 (Bit 17)                                     */
#define ECC_SRAM_CH1_STATUS1_ERR_STS1_Msk (0x20000U)               /*!< ERR_STS1 (Bitfield-Mask: 0x01)                        */
/* =====================================================  ECC_ERR_CTRL  ====================================================== */
#define ECC_SRAM_ECC_ERR_CTRL_ECC0_1BIT_IRQ_EN_Pos (0U)            /*!< ECC0_1BIT_IRQ_EN (Bit 0)                              */
#define ECC_SRAM_ECC_ERR_CTRL_ECC0_1BIT_IRQ_EN_Msk (0x1U)          /*!< ECC0_1BIT_IRQ_EN (Bitfield-Mask: 0x01)                */
#define ECC_SRAM_ECC_ERR_CTRL_ECC0_2BIT_IRQ_EN_Pos (1U)            /*!< ECC0_2BIT_IRQ_EN (Bit 1)                              */
#define ECC_SRAM_ECC_ERR_CTRL_ECC0_2BIT_IRQ_EN_Msk (0x2U)          /*!< ECC0_2BIT_IRQ_EN (Bitfield-Mask: 0x01)                */
#define ECC_SRAM_ECC_ERR_CTRL_ECC0_ST_CLR1_Pos (2U)                /*!< ECC0_ST_CLR1 (Bit 2)                                  */
#define ECC_SRAM_ECC_ERR_CTRL_ECC0_ST_CLR1_Msk (0x4U)              /*!< ECC0_ST_CLR1 (Bitfield-Mask: 0x01)                    */
#define ECC_SRAM_ECC_ERR_CTRL_ECC0_ST_CLR2_Pos (3U)                /*!< ECC0_ST_CLR2 (Bit 3)                                  */
#define ECC_SRAM_ECC_ERR_CTRL_ECC0_ST_CLR2_Msk (0x8U)              /*!< ECC0_ST_CLR2 (Bitfield-Mask: 0x01)                    */
#define ECC_SRAM_ECC_ERR_CTRL_ECC1_1BIT_IRQ_EN_Pos (8U)            /*!< ECC1_1BIT_IRQ_EN (Bit 8)                              */
#define ECC_SRAM_ECC_ERR_CTRL_ECC1_1BIT_IRQ_EN_Msk (0x100U)        /*!< ECC1_1BIT_IRQ_EN (Bitfield-Mask: 0x01)                */
#define ECC_SRAM_ECC_ERR_CTRL_ECC1_2BIT_IRQ_EN_Pos (9U)            /*!< ECC1_2BIT_IRQ_EN (Bit 9)                              */
#define ECC_SRAM_ECC_ERR_CTRL_ECC1_2BIT_IRQ_EN_Msk (0x200U)        /*!< ECC1_2BIT_IRQ_EN (Bitfield-Mask: 0x01)                */
#define ECC_SRAM_ECC_ERR_CTRL_ECC1_ST_CLR1_Pos (10U)               /*!< ECC1_ST_CLR1 (Bit 10)                                 */
#define ECC_SRAM_ECC_ERR_CTRL_ECC1_ST_CLR1_Msk (0x400U)            /*!< ECC1_ST_CLR1 (Bitfield-Mask: 0x01)                    */
#define ECC_SRAM_ECC_ERR_CTRL_ECC1_ST_CLR2_Pos (11U)               /*!< ECC1_ST_CLR2 (Bit 11)                                 */
#define ECC_SRAM_ECC_ERR_CTRL_ECC1_ST_CLR2_Msk (0x800U)            /*!< ECC1_ST_CLR2 (Bitfield-Mask: 0x01)                    */

/* =========================================================================================================================== */
/* ================                                           FLASH                                           ================ */
/* =========================================================================================================================== */

/* =========================================================  STAT  ========================================================== */
#define FLASH_STAT_CMDONOFF_Pos           (0U)                     /*!< DONEF (Bit 0)                                         */
#define FLASH_STAT_CMDONOFF_Msk           (0x1U)                   /*!< DONEF (Bitfield-Mask: 0x01)                           */
#define FLASH_STAT_ECCSERRF_Pos           (7U)                     /*!< ECCSERRF (Bit 7)                                      */
#define FLASH_STAT_ECCSERRF_Msk           (0x80U)                  /*!< ECCSERRF (Bitfield-Mask: 0x01)                        */
#define FLASH_STAT_DFDIF_Pos              (8U)                     /*!< ECCDERRF (Bit 8)                                      */
#define FLASH_STAT_DFDIF_Msk              (0x100U)                 /*!< ECCDERRF (Bitfield-Mask: 0x01)                        */
#define FLASH_STAT_PVIOLF_Pos             (9U)                     /*!< PVIOLF (Bit 9)                                        */
#define FLASH_STAT_PVIOLF_Msk             (0x200U)                 /*!< PVIOLF (Bitfield-Mask: 0x01)                          */
#define FLASH_STAT_VERIFYERR_Pos          (10U)                    /*!< VERIFYERR (Bit 10)                                    */
#define FLASH_STAT_VERIFYERR_Msk          (0x400U)                 /*!< VERIFYERR (Bitfield-Mask: 0x01)                       */
#define FLASH_STAT_ACCERRF_Pos            (11U)                    /*!< ACCERRF (Bit 11)                                      */
#define FLASH_STAT_ACCERRF_Msk            (0x800U)                 /*!< ACCERRF (Bitfield-Mask: 0x01)                         */
#define FLASH_STAT_COLLERRF_Pos           (12U)                    /*!< COLLERRF (Bit 12)                                     */
#define FLASH_STAT_COLLERRF_Msk           (0x1000U)                /*!< COLLERRF (Bitfield-Mask: 0x01)                        */
#define FLASH_STAT_CCIF_Pos               (13U)                    /*!< CCIF (Bit 13)                                         */
#define FLASH_STAT_CCIF_Msk               (0x2000U)                /*!< CCIF (Bitfield-Mask: 0x01)                            */
#define FLASH_STAT_LOCK_Pos               (14U)                    /*!< LOCK (Bit 14)                                         */
#define FLASH_STAT_LOCK_Msk               (0x4000U)                /*!< LOCK (Bitfield-Mask: 0x01)                            */
/* ==========================================================  SEC  ========================================================== */
#define FLASH_SEC_RDPROT_Pos              (8U)                     /*!< RDPROT (Bit 8)                                        */
#define FLASH_SEC_RDPROT_Msk              (0x300U)                 /*!< RDPROT (Bitfield-Mask: 0x03)                          */
/* ========================================================  KEYUNLK  ======================================================== */
#define FLASH_KEYUNLK_KEY_Pos             (0U)                     /*!< KEY (Bit 0)                                           */
#define FLASH_KEYUNLK_KEY_Msk             (0xffffffffU)            /*!< KEY (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  CNFG  ========================================================== */
#define FLASH_CNFG_DFDIE_Pos              (0U)                     /*!< DFDIE (Bit 0)                                         */
#define FLASH_CNFG_DFDIE_Msk              (0x1U)                   /*!< DFDIE (Bitfield-Mask: 0x01)                           */
#define FLASH_CNFG_COLLE_Pos              (1U)                     /*!< COLLE (Bit 1)                                         */
#define FLASH_CNFG_COLLE_Msk              (0x2U)                   /*!< COLLE (Bitfield-Mask: 0x01)                           */
#define FLASH_CNFG_CCIE_Pos               (2U)                     /*!< CCIE (Bit 2)                                          */
#define FLASH_CNFG_CCIE_Msk               (0x4U)                   /*!< CCIE (Bitfield-Mask: 0x01)                            */
#define FLASH_CNFG_EMDFIE_Pos             (4U)                     /*!< EMDFIE (Bit 4)                                        */
#define FLASH_CNFG_EMDFIE_Msk             (0x10U)                  /*!< EMDFIE (Bitfield-Mask: 0x01)                          */
#define FLASH_CNFG_LVDDETEN_Pos           (5U)                     /*!< LVDDETEN (Bit 5)                                      */
#define FLASH_CNFG_LVDDETEN_Msk           (0x20U)                  /*!< LVDDETEN (Bitfield-Mask: 0x01)                        */
#define FLASH_CNFG_PERABORT_Pos           (7U)                     /*!< PERABORT (Bit 7)                                      */
#define FLASH_CNFG_PERABORT_Msk           (0x80U)                  /*!< PERABORT (Bitfield-Mask: 0x01)                        */
#define FLASH_CNFG_CLKFREQ_Pos            (8U)                     /*!< CLKFREQ (Bit 8)                                       */
#define FLASH_CNFG_CLKFREQ_Msk            (0xff00U)                /*!< CLKFREQ (Bitfield-Mask: 0xff)                         */
#define FLASH_CNFG_LATENCY_Pos            (16U)                    /*!< LATENCY (Bit 16)                                      */
#define FLASH_CNFG_LATENCY_Msk            (0x70000U)               /*!< LATENCY (Bitfield-Mask: 0x07)                         */
#define FLASH_CNFG_LATENCY1_Pos           (20U)                    /*!< LATENCY1 (Bit 20)                                     */
#define FLASH_CNFG_LATENCY1_Msk           (0xf00000U)              /*!< LATENCY1 (Bitfield-Mask: 0x0f)                        */
/* ==========================================================  CMD  ========================================================== */
#define FLASH_CMD_CMD_Pos                 (0U)                     /*!< CMD (Bit 0)                                           */
#define FLASH_CMD_CMD_Msk                 (0xffffffffU)            /*!< CMD (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  ADDR  ========================================================== */
#define FLASH_ADDR_ADDR_Pos               (0U)                     /*!< ADDR (Bit 0)                                          */
#define FLASH_ADDR_ADDR_Msk               (0xffffffffU)            /*!< ADDR (Bitfield-Mask: 0xffffffff)                      */
/* =========================================================  DATA0  ========================================================= */
#define FLASH_DATA0_DATA0_Pos             (0U)                     /*!< DATA0 (Bit 0)                                         */
#define FLASH_DATA0_DATA0_Msk             (0xffffffffU)            /*!< DATA0 (Bitfield-Mask: 0xffffffff)                     */
/* =========================================================  DATA1  ========================================================= */
#define FLASH_DATA1_DATA1_Pos             (0U)                     /*!< DATA1 (Bit 0)                                         */
#define FLASH_DATA1_DATA1_Msk             (0xffffffffU)            /*!< DATA1 (Bitfield-Mask: 0xffffffff)                     */
/* ==========================================================  LEN  ========================================================== */
#define FLASH_LEN_LEN_Pos                 (0U)                     /*!< LEN (Bit 0)                                           */
#define FLASH_LEN_LEN_Msk                 (0xffffffffU)            /*!< LEN (Bitfield-Mask: 0xffffffff)                       */
/* ========================================================  CMDCFG  ========================================================= */
#define FLASH_CMDCFG_CMDEN_Pos            (0U)                     /*!< CMDEN (Bit 0)                                         */
#define FLASH_CMDCFG_CMDEN_Msk            (0x1U)                   /*!< CMDEN (Bitfield-Mask: 0x01)                           */
/* ==========================================================  CST  ========================================================== */
#define FLASH_CST_START_Pos               (0U)                     /*!< START (Bit 0)                                         */
#define FLASH_CST_START_Msk               (0x1U)                   /*!< START (Bitfield-Mask: 0x01)                           */
/* ========================================================  PPROT0  ========================================================= */
#define FLASH_PPROT_PPROT_Pos             (0U)                     /*!< PPROT (Bit 0)                                         */
#define FLASH_PPROT_PPROT_Msk             (0xffffffffU)            /*!< PPROT (Bitfield-Mask: 0xffffffff)                     */
/* ========================================================  PPROT1  ========================================================= */
#define FLASH_PPROT1_PPROT1_Pos           (0U)                     /*!< PPROT1 (Bit 0)                                        */
#define FLASH_PPROT1_PPROT1_Msk           (0xfU)                   /*!< PPROT1 (Bitfield-Mask: 0x0f)                          */
/* =========================================================  DPROT  ========================================================= */
#define FLASH_DPROT_DPROT_Pos             (0U)                     /*!< DPROT (Bit 0)                                         */
#define FLASH_DPROT_DPROT_Msk             (0xffffffffU)            /*!< DPROT (Bitfield-Mask: 0xffffffff)                     */
/* =========================================================  DFADR  ========================================================= */
#define FLASH_DFADR_DFADR_Pos             (0U)                     /*!< DFADR (Bit 0)                                         */
#define FLASH_DFADR_DFADR_Msk             (0xffffffffU)            /*!< DFADR (Bitfield-Mask: 0xffffffff)                     */
/* ========================================================  SWAPST  ========================================================= */
#define FLASH_SWAPST_FLAG_Pos             (0U)                     /*!< FLAG (Bit 0)                                          */
#define FLASH_SWAPST_FLAG_Msk             (0x1U)                   /*!< FLAG (Bitfield-Mask: 0x01)                            */
#define FLASH_SWAPST_EN_Pos               (1U)                     /*!< EN (Bit 1)                                            */
#define FLASH_SWAPST_EN_Msk               (0x2U)                   /*!< EN (Bitfield-Mask: 0x01)                              */
#define FLASH_SWAPST_DECERR_Pos           (2U)                     /*!< DECERR (Bit 2)                                        */
#define FLASH_SWAPST_DECERR_Msk           (0x4U)                   /*!< DECERR (Bitfield-Mask: 0x01)                          */
#define FLASH_SWAPST_PAGEEN_Pos           (4U)                     /*!< PAGEEN (Bit 4)                                        */
#define FLASH_SWAPST_PAGEEN_Msk           (0x30U)                  /*!< PAGEEN (Bitfield-Mask: 0x03)                          */
#define FLASH_SWAPST_RESNUN_Pos           (16U)                    /*!< RESNUN (Bit 16)                                       */
#define FLASH_SWAPST_RESNUN_Msk           (0xff0000U)              /*!< RESNUN (Bitfield-Mask: 0xff)                          */
/* =========================================================  LKSTS  ========================================================= */
#define FLASH_LKSTS_LOCK_Pos              (0U)                     /*!< LOCK (Bit 0)                                          */
#define FLASH_LKSTS_LOCK_Msk              (0x1U)                   /*!< LOCK (Bitfield-Mask: 0x01)                            */
#define FLASH_LKSTS_DECERR_Pos            (2U)                     /*!< DECERR (Bit 2)                                        */
#define FLASH_LKSTS_DECERR_Msk            (0x4U)                   /*!< DECERR (Bitfield-Mask: 0x01)                          */
#define FLASH_LKSTS_RESNUN_Pos            (16U)                    /*!< RESNUN (Bit 16)                                       */
#define FLASH_LKSTS_RESNUN_Msk            (0xff0000U)              /*!< RESNUN (Bitfield-Mask: 0xff)                          */
/* ========================================================  CFGDEC  ========================================================= */
#define FLASH_CFGDEC_ROMCODEEN_Pos        (8U)                     /*!< ROMCODEEN (Bit 8)                                     */
#define FLASH_CFGDEC_ROMCODEEN_Msk        (0x100U)                 /*!< ROMCODEEN (Bitfield-Mask: 0x01)                       */
#define FLASH_CFGDEC_BLEN_Pos             (9U)                     /*!< BLEN (Bit 9)                                          */
#define FLASH_CFGDEC_BLEN_Msk             (0x200U)                 /*!< BLEN (Bitfield-Mask: 0x01)                            */
#define FLASH_CFGDEC_FWEN_Pos             (10U)                    /*!< FWEN (Bit 10)                                         */
#define FLASH_CFGDEC_FWEN_Msk             (0x400U)                 /*!< FWEN (Bitfield-Mask: 0x01)                            */
#define FLASH_CFGDEC_BLASOTP_Pos          (12U)                    /*!< BLASOTP (Bit 12)                                      */
#define FLASH_CFGDEC_BLASOTP_Msk          (0x1000U)                /*!< BLASOTP (Bitfield-Mask: 0x01)                         */
#define FLASH_CFGDEC_FWASOTP_Pos          (13U)                    /*!< FWASOTP (Bit 13)                                      */
#define FLASH_CFGDEC_FWASOTP_Msk          (0x2000U)                /*!< FWASOTP (Bitfield-Mask: 0x01)                         */
#define FLASH_CFGDEC_SECBOOTEN_Pos        (16U)                    /*!< SECBOOTEN (Bit 16)                                    */
#define FLASH_CFGDEC_SECBOOTEN_Msk        (0x10000U)               /*!< SECBOOTEN (Bitfield-Mask: 0x01)                       */
#define FLASH_CFGDEC_SECBOOTBYP_Pos       (17U)                    /*!< SECBOOTBYP (Bit 17)                                   */
#define FLASH_CFGDEC_SECBOOTBYP_Msk       (0x20000U)               /*!< SECBOOTBYP (Bitfield-Mask: 0x01)                      */
#define FLASH_CFGDEC_DBGAUTHWAY_Pos       (18U)                    /*!< DBGAUTHWAY (Bit 18)                                   */
#define FLASH_CFGDEC_DBGAUTHWAY_Msk       (0x40000U)               /*!< DBGAUTHWAY (Bitfield-Mask: 0x01)                      */
#define FLASH_CFGDEC_HSMEN_Pos            (24U)                    /*!< HSMEN (Bit 24)                                        */
#define FLASH_CFGDEC_HSMEN_Msk            (0x1000000U)             /*!< HSMEN (Bitfield-Mask: 0x01)                           */
/* ========================================================  SECCTL ========================================================= */
#define FLASH_SECCTL_EMLK_Pos             (0U)                     /*!< EMLK (Bit 0)                                          */
#define FLASH_SECCTL_EMLK_Msk             (0x1U)                   /*!< EMLK (Bitfield-Mask: 0x01)                            */
#define FLASH_SECCTL_OTPSEL_Pos           (1U)                     /*!< OTPSEL (Bit 1)                                        */
#define FLASH_SECCTL_OTPSEL_Msk           (0x2U)                   /*!< OTPSEL (Bitfield-Mask: 0x01)                          */
#define FLASH_SECCTL_FSK_Pos              (2U)                     /*!< FSK (Bit 2)                                           */
#define FLASH_SECCTL_FSK_Msk              (0x4U)                   /*!< FSK (Bitfield-Mask: 0x01)                             */
/* ========================================================  CFGSEL ========================================================= */
#define FLASH_CFGSEL_CFG_SEL_Pos          (0U)                     /*!< CFG_SEL (Bit 0)                                       */
#define FLASH_CFGSEL_CFG_SEL_Msk          (0xFU)                   /*!< CFG_SEL (Bitfield-Mask: 0x0F)                         */
/* ========================================================  CFGVIEW ======================================================== */
#define FLASH_CFGVIEW_CFG_VIEW_Pos        (0U)                     /*!< CFG_VIEW (Bit 0)                                      */
#define FLASH_CFGVIEW_CFG_VIEW_Msk        (0xFFFFFFFFU)            /*!< CFG_VIEW (Bitfield-Mask: 0xFFFFFFFF)                  */

/* =========================================================================================================================== */
/* ================                                            EIO                                            ================ */
/* =========================================================================================================================== */

/* =========================================================  CTRL  ========================================================== */
#define EIO_CTRL_ENHANCED_EN_Pos          (0U)                     /*!< ENHANCED_EN (Bit 0)                                   */
#define EIO_CTRL_ENHANCED_EN_Msk          (0x1U)                   /*!< ENHANCED_EN (Bitfield-Mask: 0x01)                     */
#define EIO_CTRL_SWRST_Pos                (1U)                     /*!< SWRST (Bit 1)                                         */
#define EIO_CTRL_SWRST_Msk                (0x2U)                   /*!< SWRST (Bitfield-Mask: 0x01)                           */
#define EIO_CTRL_DBGE_Pos                 (2U)                     /*!< DBGE (Bit 2)                                          */
#define EIO_CTRL_DBGE_Msk                 (0x4U)                   /*!< DBGE (Bitfield-Mask: 0x01)                            */
/* ==========================================================  PIN  ========================================================== */
#define EIO_PIN_PDI_Pos                   (0U)                     /*!< PDI (Bit 0)                                           */
#define EIO_PIN_PDI_Msk                   (0xffU)                  /*!< PDI (Bitfield-Mask: 0xff)                             */
/* =======================================================  SHIFTSTAT  ======================================================= */
#define EIO_SHIFTSTAT_SSF_Pos             (0U)                     /*!< SSF (Bit 0)                                           */
#define EIO_SHIFTSTAT_SSF_Msk             (0xfU)                   /*!< SSF (Bitfield-Mask: 0x0f)                             */
/* =======================================================  SHIFTERR  ======================================================== */
#define EIO_SHIFTERR_SEF_Pos              (0U)                     /*!< SEF (Bit 0)                                           */
#define EIO_SHIFTERR_SEF_Msk              (0xfU)                   /*!< SEF (Bitfield-Mask: 0x0f)                             */
/* ========================================================  TIMSTAT  ======================================================== */
#define EIO_TIMSTAT_TSF_Pos               (0U)                     /*!< TSF (Bit 0)                                           */
#define EIO_TIMSTAT_TSF_Msk               (0xfU)                   /*!< TSF (Bitfield-Mask: 0x0f)                             */
/* =======================================================  SHIFTSIEN  ======================================================= */
#define EIO_SHIFTSIEN_SSIE_Pos            (0U)                     /*!< SSIE (Bit 0)                                          */
#define EIO_SHIFTSIEN_SSIE_Msk            (0xfU)                   /*!< SSIE (Bitfield-Mask: 0x0f)                            */
/* =======================================================  SHIFTEIEN  ======================================================= */
#define EIO_SHIFTEIEN_SEIE_Pos            (0U)                     /*!< SEIE (Bit 0)                                          */
#define EIO_SHIFTEIEN_SEIE_Msk            (0xfU)                   /*!< SEIE (Bitfield-Mask: 0x0f)                            */
/* ========================================================  TIMIEN  ========================================================= */
#define EIO_TIMIEN_TEIE_Pos               (0U)                     /*!< TEIE (Bit 0)                                          */
#define EIO_TIMIEN_TEIE_Msk               (0xfU)                   /*!< TEIE (Bitfield-Mask: 0x0f)                            */
/* =======================================================  SHIFTSDEN  ======================================================= */
#define EIO_SHIFTSDEN_SSDE_Pos            (0U)                     /*!< SSDE (Bit 0)                                          */
#define EIO_SHIFTSDEN_SSDE_Msk            (0xfU)                   /*!< SSDE (Bitfield-Mask: 0x0f)                            */
/* =======================================================  SHIFTCTL0  ======================================================= */
#define EIO_SHIFTCTL0_SMOD_Pos            (0U)                     /*!< SMOD (Bit 0)                                          */
#define EIO_SHIFTCTL0_SMOD_Msk            (0x7U)                   /*!< SMOD (Bitfield-Mask: 0x07)                            */
#define EIO_SHIFTCTL0_PINPOL_Pos          (7U)                     /*!< PINPOL (Bit 7)                                        */
#define EIO_SHIFTCTL0_PINPOL_Msk          (0x80U)                  /*!< PINPOL (Bitfield-Mask: 0x01)                          */
#define EIO_SHIFTCTL0_PINSEL_Pos          (8U)                     /*!< PINSEL (Bit 8)                                        */
#define EIO_SHIFTCTL0_PINSEL_Msk          (0x700U)                 /*!< PINSEL (Bitfield-Mask: 0x07)                          */
#define EIO_SHIFTCTL0_PINCFG_Pos          (16U)                    /*!< PINCFG (Bit 16)                                       */
#define EIO_SHIFTCTL0_PINCFG_Msk          (0x30000U)               /*!< PINCFG (Bitfield-Mask: 0x03)                          */
#define EIO_SHIFTCTL0_TIMPOL_Pos          (23U)                    /*!< TIMPOL (Bit 23)                                       */
#define EIO_SHIFTCTL0_TIMPOL_Msk          (0x800000U)              /*!< TIMPOL (Bitfield-Mask: 0x01)                          */
#define EIO_SHIFTCTL0_TIMSEL_Pos          (24U)                    /*!< TIMSEL (Bit 24)                                       */
#define EIO_SHIFTCTL0_TIMSEL_Msk          (0x3000000U)             /*!< TIMSEL (Bitfield-Mask: 0x03)                          */
/* =======================================================  SHIFTCTL1  ======================================================= */
#define EIO_SHIFTCTL1_SMOD_Pos            (0U)                     /*!< SMOD (Bit 0)                                          */
#define EIO_SHIFTCTL1_SMOD_Msk            (0x7U)                   /*!< SMOD (Bitfield-Mask: 0x07)                            */
#define EIO_SHIFTCTL1_PINPOL_Pos          (7U)                     /*!< PINPOL (Bit 7)                                        */
#define EIO_SHIFTCTL1_PINPOL_Msk          (0x80U)                  /*!< PINPOL (Bitfield-Mask: 0x01)                          */
#define EIO_SHIFTCTL1_PINSEL_Pos          (8U)                     /*!< PINSEL (Bit 8)                                        */
#define EIO_SHIFTCTL1_PINSEL_Msk          (0x700U)                 /*!< PINSEL (Bitfield-Mask: 0x07)                          */
#define EIO_SHIFTCTL1_PINCFG_Pos          (16U)                    /*!< PINCFG (Bit 16)                                       */
#define EIO_SHIFTCTL1_PINCFG_Msk          (0x30000U)               /*!< PINCFG (Bitfield-Mask: 0x03)                          */
#define EIO_SHIFTCTL1_TIMPOL_Pos          (23U)                    /*!< TIMPOL (Bit 23)                                       */
#define EIO_SHIFTCTL1_TIMPOL_Msk          (0x800000U)              /*!< TIMPOL (Bitfield-Mask: 0x01)                          */
#define EIO_SHIFTCTL1_TIMSEL_Pos          (24U)                    /*!< TIMSEL (Bit 24)                                       */
#define EIO_SHIFTCTL1_TIMSEL_Msk          (0x3000000U)             /*!< TIMSEL (Bitfield-Mask: 0x03)                          */
/* =======================================================  SHIFTCTL2  ======================================================= */
#define EIO_SHIFTCTL2_SMOD_Pos            (0U)                     /*!< SMOD (Bit 0)                                          */
#define EIO_SHIFTCTL2_SMOD_Msk            (0x7U)                   /*!< SMOD (Bitfield-Mask: 0x07)                            */
#define EIO_SHIFTCTL2_PINPOL_Pos          (7U)                     /*!< PINPOL (Bit 7)                                        */
#define EIO_SHIFTCTL2_PINPOL_Msk          (0x80U)                  /*!< PINPOL (Bitfield-Mask: 0x01)                          */
#define EIO_SHIFTCTL2_PINSEL_Pos          (8U)                     /*!< PINSEL (Bit 8)                                        */
#define EIO_SHIFTCTL2_PINSEL_Msk          (0x700U)                 /*!< PINSEL (Bitfield-Mask: 0x07)                          */
#define EIO_SHIFTCTL2_PINCFG_Pos          (16U)                    /*!< PINCFG (Bit 16)                                       */
#define EIO_SHIFTCTL2_PINCFG_Msk          (0x30000U)               /*!< PINCFG (Bitfield-Mask: 0x03)                          */
#define EIO_SHIFTCTL2_TIMPOL_Pos          (23U)                    /*!< TIMPOL (Bit 23)                                       */
#define EIO_SHIFTCTL2_TIMPOL_Msk          (0x800000U)              /*!< TIMPOL (Bitfield-Mask: 0x01)                          */
#define EIO_SHIFTCTL2_TIMSEL_Pos          (24U)                    /*!< TIMSEL (Bit 24)                                       */
#define EIO_SHIFTCTL2_TIMSEL_Msk          (0x3000000U)             /*!< TIMSEL (Bitfield-Mask: 0x03)                          */
/* =======================================================  SHIFTCTL3  ======================================================= */
#define EIO_SHIFTCTL3_SMOD_Pos            (0U)                     /*!< SMOD (Bit 0)                                          */
#define EIO_SHIFTCTL3_SMOD_Msk            (0x7U)                   /*!< SMOD (Bitfield-Mask: 0x07)                            */
#define EIO_SHIFTCTL3_PINPOL_Pos          (7U)                     /*!< PINPOL (Bit 7)                                        */
#define EIO_SHIFTCTL3_PINPOL_Msk          (0x80U)                  /*!< PINPOL (Bitfield-Mask: 0x01)                          */
#define EIO_SHIFTCTL3_PINSEL_Pos          (8U)                     /*!< PINSEL (Bit 8)                                        */
#define EIO_SHIFTCTL3_PINSEL_Msk          (0x700U)                 /*!< PINSEL (Bitfield-Mask: 0x07)                          */
#define EIO_SHIFTCTL3_PINCFG_Pos          (16U)                    /*!< PINCFG (Bit 16)                                       */
#define EIO_SHIFTCTL3_PINCFG_Msk          (0x30000U)               /*!< PINCFG (Bitfield-Mask: 0x03)                          */
#define EIO_SHIFTCTL3_TIMPOL_Pos          (23U)                    /*!< TIMPOL (Bit 23)                                       */
#define EIO_SHIFTCTL3_TIMPOL_Msk          (0x800000U)              /*!< TIMPOL (Bitfield-Mask: 0x01)                          */
#define EIO_SHIFTCTL3_TIMSEL_Pos          (24U)                    /*!< TIMSEL (Bit 24)                                       */
#define EIO_SHIFTCTL3_TIMSEL_Msk          (0x3000000U)             /*!< TIMSEL (Bitfield-Mask: 0x03)                          */
/* =======================================================  SHIFTCFG0  ======================================================= */
#define EIO_SHIFTCFG0_SSTART_Pos          (0U)                     /*!< SSTART (Bit 0)                                        */
#define EIO_SHIFTCFG0_SSTART_Msk          (0x3U)                   /*!< SSTART (Bitfield-Mask: 0x03)                          */
#define EIO_SHIFTCFG0_SSTOP_Pos           (4U)                     /*!< SSTOP (Bit 4)                                         */
#define EIO_SHIFTCFG0_SSTOP_Msk           (0x30U)                  /*!< SSTOP (Bitfield-Mask: 0x03)                           */
#define EIO_SHIFTCFG0_INSRC_Pos           (8U)                     /*!< INSRC (Bit 8)                                         */
#define EIO_SHIFTCFG0_INSRC_Msk           (0x100U)                 /*!< INSRC (Bitfield-Mask: 0x01)                           */
/* =======================================================  SHIFTCFG1  ======================================================= */
#define EIO_SHIFTCFG1_SSTART_Pos          (0U)                     /*!< SSTART (Bit 0)                                        */
#define EIO_SHIFTCFG1_SSTART_Msk          (0x3U)                   /*!< SSTART (Bitfield-Mask: 0x03)                          */
#define EIO_SHIFTCFG1_SSTOP_Pos           (4U)                     /*!< SSTOP (Bit 4)                                         */
#define EIO_SHIFTCFG1_SSTOP_Msk           (0x30U)                  /*!< SSTOP (Bitfield-Mask: 0x03)                           */
#define EIO_SHIFTCFG1_INSRC_Pos           (8U)                     /*!< INSRC (Bit 8)                                         */
#define EIO_SHIFTCFG1_INSRC_Msk           (0x100U)                 /*!< INSRC (Bitfield-Mask: 0x01)                           */
/* =======================================================  SHIFTCFG2  ======================================================= */
#define EIO_SHIFTCFG2_SSTART_Pos          (0U)                     /*!< SSTART (Bit 0)                                        */
#define EIO_SHIFTCFG2_SSTART_Msk          (0x3U)                   /*!< SSTART (Bitfield-Mask: 0x03)                          */
#define EIO_SHIFTCFG2_SSTOP_Pos           (4U)                     /*!< SSTOP (Bit 4)                                         */
#define EIO_SHIFTCFG2_SSTOP_Msk           (0x30U)                  /*!< SSTOP (Bitfield-Mask: 0x03)                           */
#define EIO_SHIFTCFG2_INSRC_Pos           (8U)                     /*!< INSRC (Bit 8)                                         */
#define EIO_SHIFTCFG2_INSRC_Msk           (0x100U)                 /*!< INSRC (Bitfield-Mask: 0x01)                           */
/* =======================================================  SHIFTCFG3  ======================================================= */
#define EIO_SHIFTCFG3_SSTART_Pos          (0U)                     /*!< SSTART (Bit 0)                                        */
#define EIO_SHIFTCFG3_SSTART_Msk          (0x3U)                   /*!< SSTART (Bitfield-Mask: 0x03)                          */
#define EIO_SHIFTCFG3_SSTOP_Pos           (4U)                     /*!< SSTOP (Bit 4)                                         */
#define EIO_SHIFTCFG3_SSTOP_Msk           (0x30U)                  /*!< SSTOP (Bitfield-Mask: 0x03)                           */
#define EIO_SHIFTCFG3_INSRC_Pos           (8U)                     /*!< INSRC (Bit 8)                                         */
#define EIO_SHIFTCFG3_INSRC_Msk           (0x100U)                 /*!< INSRC (Bitfield-Mask: 0x01)                           */
/* =======================================================  SHIFTBUF0  ======================================================= */
#define EIO_SHIFTBUF0_SHIFTBUF_Pos        (0U)                     /*!< SHIFTBUF (Bit 0)                                      */
#define EIO_SHIFTBUF0_SHIFTBUF_Msk        (0xffffffffU)            /*!< SHIFTBUF (Bitfield-Mask: 0xffffffff)                  */
/* =======================================================  SHIFTBUF1  ======================================================= */
#define EIO_SHIFTBUF1_SHIFTBUF_Pos        (0U)                     /*!< SHIFTBUF (Bit 0)                                      */
#define EIO_SHIFTBUF1_SHIFTBUF_Msk        (0xffffffffU)            /*!< SHIFTBUF (Bitfield-Mask: 0xffffffff)                  */
/* =======================================================  SHIFTBUF2  ======================================================= */
#define EIO_SHIFTBUF2_SHIFTBUF_Pos        (0U)                     /*!< SHIFTBUF (Bit 0)                                      */
#define EIO_SHIFTBUF2_SHIFTBUF_Msk        (0xffffffffU)            /*!< SHIFTBUF (Bitfield-Mask: 0xffffffff)                  */
/* =======================================================  SHIFTBUF3  ======================================================= */
#define EIO_SHIFTBUF3_SHIFTBUF_Pos        (0U)                     /*!< SHIFTBUF (Bit 0)                                      */
#define EIO_SHIFTBUF3_SHIFTBUF_Msk        (0xffffffffU)            /*!< SHIFTBUF (Bitfield-Mask: 0xffffffff)                  */
/* =====================================================  SHIFTBUFBIS0  ====================================================== */
#define EIO_SHIFTBUFBIS0_SHIFTBUFBIS_Pos  (0U)                     /*!< SHIFTBUFBIS (Bit 0)                                   */
#define EIO_SHIFTBUFBIS0_SHIFTBUFBIS_Msk  (0xffffffffU)            /*!< SHIFTBUFBIS (Bitfield-Mask: 0xffffffff)               */
/* =====================================================  SHIFTBUFBIS1  ====================================================== */
#define EIO_SHIFTBUFBIS1_SHIFTBUFBIS_Pos  (0U)                     /*!< SHIFTBUFBIS (Bit 0)                                   */
#define EIO_SHIFTBUFBIS1_SHIFTBUFBIS_Msk  (0xffffffffU)            /*!< SHIFTBUFBIS (Bitfield-Mask: 0xffffffff)               */
/* =====================================================  SHIFTBUFBIS2  ====================================================== */
#define EIO_SHIFTBUFBIS2_SHIFTBUFBIS_Pos  (0U)                     /*!< SHIFTBUFBIS (Bit 0)                                   */
#define EIO_SHIFTBUFBIS2_SHIFTBUFBIS_Msk  (0xffffffffU)            /*!< SHIFTBUFBIS (Bitfield-Mask: 0xffffffff)               */
/* =====================================================  SHIFTBUFBIS3  ====================================================== */
#define EIO_SHIFTBUFBIS3_SHIFTBUFBIS_Pos  (0U)                     /*!< SHIFTBUFBIS (Bit 0)                                   */
#define EIO_SHIFTBUFBIS3_SHIFTBUFBIS_Msk  (0xffffffffU)            /*!< SHIFTBUFBIS (Bitfield-Mask: 0xffffffff)               */
/* =====================================================  SHIFTBUFBYS0  ====================================================== */
#define EIO_SHIFTBUFBYS0_SHIFTBUFBYS_Pos  (0U)                     /*!< SHIFTBUFBYS (Bit 0)                                   */
#define EIO_SHIFTBUFBYS0_SHIFTBUFBYS_Msk  (0xffffffffU)            /*!< SHIFTBUFBYS (Bitfield-Mask: 0xffffffff)               */
/* =====================================================  SHIFTBUFBYS1  ====================================================== */
#define EIO_SHIFTBUFBYS1_SHIFTBUFBYS_Pos  (0U)                     /*!< SHIFTBUFBYS (Bit 0)                                   */
#define EIO_SHIFTBUFBYS1_SHIFTBUFBYS_Msk  (0xffffffffU)            /*!< SHIFTBUFBYS (Bitfield-Mask: 0xffffffff)               */
/* =====================================================  SHIFTBUFBYS2  ====================================================== */
#define EIO_SHIFTBUFBYS2_SHIFTBUFBYS_Pos  (0U)                     /*!< SHIFTBUFBYS (Bit 0)                                   */
#define EIO_SHIFTBUFBYS2_SHIFTBUFBYS_Msk  (0xffffffffU)            /*!< SHIFTBUFBYS (Bitfield-Mask: 0xffffffff)               */
/* =====================================================  SHIFTBUFBYS3  ====================================================== */
#define EIO_SHIFTBUFBYS3_SHIFTBUFBYS_Pos  (0U)                     /*!< SHIFTBUFBYS (Bit 0)                                   */
#define EIO_SHIFTBUFBYS3_SHIFTBUFBYS_Msk  (0xffffffffU)            /*!< SHIFTBUFBYS (Bitfield-Mask: 0xffffffff)               */
/* =====================================================  SHIFTBUFBBS0  ====================================================== */
#define EIO_SHIFTBUFBBS0_SHIFTBUFBBS_Pos  (0U)                     /*!< SHIFTBUFBBS (Bit 0)                                   */
#define EIO_SHIFTBUFBBS0_SHIFTBUFBBS_Msk  (0xffffffffU)            /*!< SHIFTBUFBBS (Bitfield-Mask: 0xffffffff)               */
/* =====================================================  SHIFTBUFBBS1  ====================================================== */
#define EIO_SHIFTBUFBBS1_SHIFTBUFBBS_Pos  (0U)                     /*!< SHIFTBUFBBS (Bit 0)                                   */
#define EIO_SHIFTBUFBBS1_SHIFTBUFBBS_Msk  (0xffffffffU)            /*!< SHIFTBUFBBS (Bitfield-Mask: 0xffffffff)               */
/* =====================================================  SHIFTBUFBBS2  ====================================================== */
#define EIO_SHIFTBUFBBS2_SHIFTBUFBBS_Pos  (0U)                     /*!< SHIFTBUFBBS (Bit 0)                                   */
#define EIO_SHIFTBUFBBS2_SHIFTBUFBBS_Msk  (0xffffffffU)            /*!< SHIFTBUFBBS (Bitfield-Mask: 0xffffffff)               */
/* =====================================================  SHIFTBUFBBS3  ====================================================== */
#define EIO_SHIFTBUFBBS3_SHIFTBUFBBS_Pos  (0U)                     /*!< SHIFTBUFBBS (Bit 0)                                   */
#define EIO_SHIFTBUFBBS3_SHIFTBUFBBS_Msk  (0xffffffffU)            /*!< SHIFTBUFBBS (Bitfield-Mask: 0xffffffff)               */
/* ========================================================  TIMCTL0  ======================================================== */
#define EIO_TIMCTL0_TIMOD_Pos             (0U)                     /*!< TIMOD (Bit 0)                                         */
#define EIO_TIMCTL0_TIMOD_Msk             (0x3U)                   /*!< TIMOD (Bitfield-Mask: 0x03)                           */
#define EIO_TIMCTL0_PINPOL_Pos            (7U)                     /*!< PINPOL (Bit 7)                                        */
#define EIO_TIMCTL0_PINPOL_Msk            (0x80U)                  /*!< PINPOL (Bitfield-Mask: 0x01)                          */
#define EIO_TIMCTL0_PINSEL_Pos            (8U)                     /*!< PINSEL (Bit 8)                                        */
#define EIO_TIMCTL0_PINSEL_Msk            (0x700U)                 /*!< PINSEL (Bitfield-Mask: 0x07)                          */
#define EIO_TIMCTL0_PINCFG_Pos            (16U)                    /*!< PINCFG (Bit 16)                                       */
#define EIO_TIMCTL0_PINCFG_Msk            (0x30000U)               /*!< PINCFG (Bitfield-Mask: 0x03)                          */
#define EIO_TIMCTL0_TRGSRC_Pos            (22U)                    /*!< TRGSRC (Bit 22)                                       */
#define EIO_TIMCTL0_TRGSRC_Msk            (0x400000U)              /*!< TRGSRC (Bitfield-Mask: 0x01)                          */
#define EIO_TIMCTL0_TRGPOL_Pos            (23U)                    /*!< TRGPOL (Bit 23)                                       */
#define EIO_TIMCTL0_TRGPOL_Msk            (0x800000U)              /*!< TRGPOL (Bitfield-Mask: 0x01)                          */
#define EIO_TIMCTL0_TRGSEL_Pos            (24U)                    /*!< TRGSEL (Bit 24)                                       */
#define EIO_TIMCTL0_TRGSEL_Msk            (0xf000000U)             /*!< TRGSEL (Bitfield-Mask: 0x0f)                          */
/* ========================================================  TIMCTL1  ======================================================== */
#define EIO_TIMCTL1_TIMOD_Pos             (0U)                     /*!< TIMOD (Bit 0)                                         */
#define EIO_TIMCTL1_TIMOD_Msk             (0x3U)                   /*!< TIMOD (Bitfield-Mask: 0x03)                           */
#define EIO_TIMCTL1_PINPOL_Pos            (7U)                     /*!< PINPOL (Bit 7)                                        */
#define EIO_TIMCTL1_PINPOL_Msk            (0x80U)                  /*!< PINPOL (Bitfield-Mask: 0x01)                          */
#define EIO_TIMCTL1_PINSEL_Pos            (8U)                     /*!< PINSEL (Bit 8)                                        */
#define EIO_TIMCTL1_PINSEL_Msk            (0x700U)                 /*!< PINSEL (Bitfield-Mask: 0x07)                          */
#define EIO_TIMCTL1_PINCFG_Pos            (16U)                    /*!< PINCFG (Bit 16)                                       */
#define EIO_TIMCTL1_PINCFG_Msk            (0x30000U)               /*!< PINCFG (Bitfield-Mask: 0x03)                          */
#define EIO_TIMCTL1_TRGSRC_Pos            (22U)                    /*!< TRGSRC (Bit 22)                                       */
#define EIO_TIMCTL1_TRGSRC_Msk            (0x400000U)              /*!< TRGSRC (Bitfield-Mask: 0x01)                          */
#define EIO_TIMCTL1_TRGPOL_Pos            (23U)                    /*!< TRGPOL (Bit 23)                                       */
#define EIO_TIMCTL1_TRGPOL_Msk            (0x800000U)              /*!< TRGPOL (Bitfield-Mask: 0x01)                          */
#define EIO_TIMCTL1_TRGSEL_Pos            (24U)                    /*!< TRGSEL (Bit 24)                                       */
#define EIO_TIMCTL1_TRGSEL_Msk            (0xf000000U)             /*!< TRGSEL (Bitfield-Mask: 0x0f)                          */
/* ========================================================  TIMCTL2  ======================================================== */
#define EIO_TIMCTL2_TIMOD_Pos             (0U)                     /*!< TIMOD (Bit 0)                                         */
#define EIO_TIMCTL2_TIMOD_Msk             (0x3U)                   /*!< TIMOD (Bitfield-Mask: 0x03)                           */
#define EIO_TIMCTL2_PINPOL_Pos            (7U)                     /*!< PINPOL (Bit 7)                                        */
#define EIO_TIMCTL2_PINPOL_Msk            (0x80U)                  /*!< PINPOL (Bitfield-Mask: 0x01)                          */
#define EIO_TIMCTL2_PINSEL_Pos            (8U)                     /*!< PINSEL (Bit 8)                                        */
#define EIO_TIMCTL2_PINSEL_Msk            (0x700U)                 /*!< PINSEL (Bitfield-Mask: 0x07)                          */
#define EIO_TIMCTL2_PINCFG_Pos            (16U)                    /*!< PINCFG (Bit 16)                                       */
#define EIO_TIMCTL2_PINCFG_Msk            (0x30000U)               /*!< PINCFG (Bitfield-Mask: 0x03)                          */
#define EIO_TIMCTL2_TRGSRC_Pos            (22U)                    /*!< TRGSRC (Bit 22)                                       */
#define EIO_TIMCTL2_TRGSRC_Msk            (0x400000U)              /*!< TRGSRC (Bitfield-Mask: 0x01)                          */
#define EIO_TIMCTL2_TRGPOL_Pos            (23U)                    /*!< TRGPOL (Bit 23)                                       */
#define EIO_TIMCTL2_TRGPOL_Msk            (0x800000U)              /*!< TRGPOL (Bitfield-Mask: 0x01)                          */
#define EIO_TIMCTL2_TRGSEL_Pos            (24U)                    /*!< TRGSEL (Bit 24)                                       */
#define EIO_TIMCTL2_TRGSEL_Msk            (0xf000000U)             /*!< TRGSEL (Bitfield-Mask: 0x0f)                          */
/* ========================================================  TIMCTL3  ======================================================== */
#define EIO_TIMCTL3_TIMOD_Pos             (0U)                     /*!< TIMOD (Bit 0)                                         */
#define EIO_TIMCTL3_TIMOD_Msk             (0x3U)                   /*!< TIMOD (Bitfield-Mask: 0x03)                           */
#define EIO_TIMCTL3_PINPOL_Pos            (7U)                     /*!< PINPOL (Bit 7)                                        */
#define EIO_TIMCTL3_PINPOL_Msk            (0x80U)                  /*!< PINPOL (Bitfield-Mask: 0x01)                          */
#define EIO_TIMCTL3_PINSEL_Pos            (8U)                     /*!< PINSEL (Bit 8)                                        */
#define EIO_TIMCTL3_PINSEL_Msk            (0x700U)                 /*!< PINSEL (Bitfield-Mask: 0x07)                          */
#define EIO_TIMCTL3_PINCFG_Pos            (16U)                    /*!< PINCFG (Bit 16)                                       */
#define EIO_TIMCTL3_PINCFG_Msk            (0x30000U)               /*!< PINCFG (Bitfield-Mask: 0x03)                          */
#define EIO_TIMCTL3_TRGSRC_Pos            (22U)                    /*!< TRGSRC (Bit 22)                                       */
#define EIO_TIMCTL3_TRGSRC_Msk            (0x400000U)              /*!< TRGSRC (Bitfield-Mask: 0x01)                          */
#define EIO_TIMCTL3_TRGPOL_Pos            (23U)                    /*!< TRGPOL (Bit 23)                                       */
#define EIO_TIMCTL3_TRGPOL_Msk            (0x800000U)              /*!< TRGPOL (Bitfield-Mask: 0x01)                          */
#define EIO_TIMCTL3_TRGSEL_Pos            (24U)                    /*!< TRGSEL (Bit 24)                                       */
#define EIO_TIMCTL3_TRGSEL_Msk            (0xf000000U)             /*!< TRGSEL (Bitfield-Mask: 0x0f)                          */
/* ========================================================  TIMCFG0  ======================================================== */
#define EIO_TIMCFG0_TSTART_Pos            (1U)                     /*!< TSTART (Bit 1)                                        */
#define EIO_TIMCFG0_TSTART_Msk            (0x2U)                   /*!< TSTART (Bitfield-Mask: 0x01)                          */
#define EIO_TIMCFG0_TSTOP_Pos             (4U)                     /*!< TSTOP (Bit 4)                                         */
#define EIO_TIMCFG0_TSTOP_Msk             (0x30U)                  /*!< TSTOP (Bitfield-Mask: 0x03)                           */
#define EIO_TIMCFG0_TIMENA_Pos            (8U)                     /*!< TIMENA (Bit 8)                                        */
#define EIO_TIMCFG0_TIMENA_Msk            (0x700U)                 /*!< TIMENA (Bitfield-Mask: 0x07)                          */
#define EIO_TIMCFG0_TIMDIS_Pos            (12U)                    /*!< TIMDIS (Bit 12)                                       */
#define EIO_TIMCFG0_TIMDIS_Msk            (0x7000U)                /*!< TIMDIS (Bitfield-Mask: 0x07)                          */
#define EIO_TIMCFG0_TIMRST_Pos            (16U)                    /*!< TIMRST (Bit 16)                                       */
#define EIO_TIMCFG0_TIMRST_Msk            (0x70000U)               /*!< TIMRST (Bitfield-Mask: 0x07)                          */
#define EIO_TIMCFG0_TIMDEC_Pos            (20U)                    /*!< TIMDEC (Bit 20)                                       */
#define EIO_TIMCFG0_TIMDEC_Msk            (0x300000U)              /*!< TIMDEC (Bitfield-Mask: 0x03)                          */
#define EIO_TIMCFG0_TIMOUT_Pos            (24U)                    /*!< TIMOUT (Bit 24)                                       */
#define EIO_TIMCFG0_TIMOUT_Msk            (0x3000000U)             /*!< TIMOUT (Bitfield-Mask: 0x03)                          */
/* ========================================================  TIMCFG1  ======================================================== */
#define EIO_TIMCFG1_TSTART_Pos            (1U)                     /*!< TSTART (Bit 1)                                        */
#define EIO_TIMCFG1_TSTART_Msk            (0x2U)                   /*!< TSTART (Bitfield-Mask: 0x01)                          */
#define EIO_TIMCFG1_TSTOP_Pos             (4U)                     /*!< TSTOP (Bit 4)                                         */
#define EIO_TIMCFG1_TSTOP_Msk             (0x30U)                  /*!< TSTOP (Bitfield-Mask: 0x03)                           */
#define EIO_TIMCFG1_TIMENA_Pos            (8U)                     /*!< TIMENA (Bit 8)                                        */
#define EIO_TIMCFG1_TIMENA_Msk            (0x700U)                 /*!< TIMENA (Bitfield-Mask: 0x07)                          */
#define EIO_TIMCFG1_TIMDIS_Pos            (12U)                    /*!< TIMDIS (Bit 12)                                       */
#define EIO_TIMCFG1_TIMDIS_Msk            (0x7000U)                /*!< TIMDIS (Bitfield-Mask: 0x07)                          */
#define EIO_TIMCFG1_TIMRST_Pos            (16U)                    /*!< TIMRST (Bit 16)                                       */
#define EIO_TIMCFG1_TIMRST_Msk            (0x70000U)               /*!< TIMRST (Bitfield-Mask: 0x07)                          */
#define EIO_TIMCFG1_TIMDEC_Pos            (20U)                    /*!< TIMDEC (Bit 20)                                       */
#define EIO_TIMCFG1_TIMDEC_Msk            (0x300000U)              /*!< TIMDEC (Bitfield-Mask: 0x03)                          */
#define EIO_TIMCFG1_TIMOUT_Pos            (24U)                    /*!< TIMOUT (Bit 24)                                       */
#define EIO_TIMCFG1_TIMOUT_Msk            (0x3000000U)             /*!< TIMOUT (Bitfield-Mask: 0x03)                          */
/* ========================================================  TIMCFG2  ======================================================== */
#define EIO_TIMCFG2_TSTART_Pos            (1U)                     /*!< TSTART (Bit 1)                                        */
#define EIO_TIMCFG2_TSTART_Msk            (0x2U)                   /*!< TSTART (Bitfield-Mask: 0x01)                          */
#define EIO_TIMCFG2_TSTOP_Pos             (4U)                     /*!< TSTOP (Bit 4)                                         */
#define EIO_TIMCFG2_TSTOP_Msk             (0x30U)                  /*!< TSTOP (Bitfield-Mask: 0x03)                           */
#define EIO_TIMCFG2_TIMENA_Pos            (8U)                     /*!< TIMENA (Bit 8)                                        */
#define EIO_TIMCFG2_TIMENA_Msk            (0x700U)                 /*!< TIMENA (Bitfield-Mask: 0x07)                          */
#define EIO_TIMCFG2_TIMDIS_Pos            (12U)                    /*!< TIMDIS (Bit 12)                                       */
#define EIO_TIMCFG2_TIMDIS_Msk            (0x7000U)                /*!< TIMDIS (Bitfield-Mask: 0x07)                          */
#define EIO_TIMCFG2_TIMRST_Pos            (16U)                    /*!< TIMRST (Bit 16)                                       */
#define EIO_TIMCFG2_TIMRST_Msk            (0x70000U)               /*!< TIMRST (Bitfield-Mask: 0x07)                          */
#define EIO_TIMCFG2_TIMDEC_Pos            (20U)                    /*!< TIMDEC (Bit 20)                                       */
#define EIO_TIMCFG2_TIMDEC_Msk            (0x300000U)              /*!< TIMDEC (Bitfield-Mask: 0x03)                          */
#define EIO_TIMCFG2_TIMOUT_Pos            (24U)                    /*!< TIMOUT (Bit 24)                                       */
#define EIO_TIMCFG2_TIMOUT_Msk            (0x3000000U)             /*!< TIMOUT (Bitfield-Mask: 0x03)                          */
/* ========================================================  TIMCFG3  ======================================================== */
#define EIO_TIMCFG3_TSTART_Pos            (1U)                     /*!< TSTART (Bit 1)                                        */
#define EIO_TIMCFG3_TSTART_Msk            (0x2U)                   /*!< TSTART (Bitfield-Mask: 0x01)                          */
#define EIO_TIMCFG3_TSTOP_Pos             (4U)                     /*!< TSTOP (Bit 4)                                         */
#define EIO_TIMCFG3_TSTOP_Msk             (0x30U)                  /*!< TSTOP (Bitfield-Mask: 0x03)                           */
#define EIO_TIMCFG3_TIMENA_Pos            (8U)                     /*!< TIMENA (Bit 8)                                        */
#define EIO_TIMCFG3_TIMENA_Msk            (0x700U)                 /*!< TIMENA (Bitfield-Mask: 0x07)                          */
#define EIO_TIMCFG3_TIMDIS_Pos            (12U)                    /*!< TIMDIS (Bit 12)                                       */
#define EIO_TIMCFG3_TIMDIS_Msk            (0x7000U)                /*!< TIMDIS (Bitfield-Mask: 0x07)                          */
#define EIO_TIMCFG3_TIMRST_Pos            (16U)                    /*!< TIMRST (Bit 16)                                       */
#define EIO_TIMCFG3_TIMRST_Msk            (0x70000U)               /*!< TIMRST (Bitfield-Mask: 0x07)                          */
#define EIO_TIMCFG3_TIMDEC_Pos            (20U)                    /*!< TIMDEC (Bit 20)                                       */
#define EIO_TIMCFG3_TIMDEC_Msk            (0x300000U)              /*!< TIMDEC (Bitfield-Mask: 0x03)                          */
#define EIO_TIMCFG3_TIMOUT_Pos            (24U)                    /*!< TIMOUT (Bit 24)                                       */
#define EIO_TIMCFG3_TIMOUT_Msk            (0x3000000U)             /*!< TIMOUT (Bitfield-Mask: 0x03)                          */
/* ========================================================  TIMCMP0  ======================================================== */
#define EIO_TIMCMP0_CMP_Pos               (0U)                     /*!< CMP (Bit 0)                                           */
#define EIO_TIMCMP0_CMP_Msk               (0xffffU)                /*!< CMP (Bitfield-Mask: 0xffff)                           */
/* ========================================================  TIMCMP1  ======================================================== */
#define EIO_TIMCMP1_CMP_Pos               (0U)                     /*!< CMP (Bit 0)                                           */
#define EIO_TIMCMP1_CMP_Msk               (0xffffU)                /*!< CMP (Bitfield-Mask: 0xffff)                           */
/* ========================================================  TIMCMP2  ======================================================== */
#define EIO_TIMCMP2_CMP_Pos               (0U)                     /*!< CMP (Bit 0)                                           */
#define EIO_TIMCMP2_CMP_Msk               (0xffffU)                /*!< CMP (Bitfield-Mask: 0xffff)                           */
/* ========================================================  TIMCMP3  ======================================================== */
#define EIO_TIMCMP3_CMP_Pos               (0U)                     /*!< CMP (Bit 0)                                           */
#define EIO_TIMCMP3_CMP_Msk               (0xffffU)                /*!< CMP (Bitfield-Mask: 0xffff)                           */

/* =========================================================================================================================== */
/* ================                                            AC784X_SysTick                                 ================ */
/* =========================================================================================================================== */

/* =======================================================  AC784X_SysTick_CSR  ============================================== */
#define AC784X_SysTick_CSR_ENABLE_Pos     (0U)                     /*!< ENABLE (Bit 0)                                        */
#define AC784X_SysTick_CSR_ENABLE_Msk     (0x1U)                   /*!< ENABLE (Bitfield-Mask: 0x01)                          */
#define AC784X_SysTick_CSR_TICKINT_Pos    (1U)                     /*!< TICKINT (Bit 1)                                       */
#define AC784X_SysTick_CSR_TICKINT_Msk    (0x2U)                   /*!< TICKINT (Bitfield-Mask: 0x01)                         */
#define AC784X_SysTick_CSR_CLKSOURCE_Pos  (2U)                     /*!< CLKSOURCE (Bit 2)                                     */
#define AC784X_SysTick_CSR_CLKSOURCE_Msk  (0x4U)                   /*!< CLKSOURCE (Bitfield-Mask: 0x01)                       */
#define AC784X_SysTick_CSR_COUNTFLAG_Pos  (16U)                    /*!< COUNTFLAG (Bit 16)                                    */
#define AC784X_SysTick_CSR_COUNTFLAG_Msk  (0x0x10000U)             /*!< COUNTFLAG (Bitfield-Mask: 0x01)                       */
/* =======================================================  AC784X_SysTick_RVR  ============================================== */
#define AC784X_SysTick_RVR_RELOAD_Pos     (0U)                     /*!< RELOAD (Bit 0)                                        */
#define AC784X_SysTick_RVR_RELOAD_Msk     (0xFFFFFFU)              /*!< RELOAD (Bitfield-Mask: 0xFFFFFF)                      */
/* =======================================================  AC784X_SysTick_CVR  ============================================== */
#define AC784X_SysTick_CVR_CURRENT_Pos    (0U)                     /*!< CURRENT (Bit 0)                                       */
#define AC784X_SysTick_CVR_CURRENT_Msk    (0xFFFFFFU)              /*!< CURRENT (Bitfield-Mask: 0xFFFFFF)                     */
/* =======================================================  AC784X_SysTick_CALIB  ============================================ */
#define AC784X_SysTick_CALIB_TENMS_Pos    (0U)                     /*!< TENMS (Bit 0)                                         */
#define AC784X_SysTick_CALIB_TENMS_Msk    (0xFFFFFFU)              /*!< TENMS (Bitfield-Mask: 0x0FFFFFF)                      */
#define AC784X_SysTick_CALIB_SKEW_Pos     (30U)                    /*!< SKEW (Bit 30)                                         */
#define AC784X_SysTick_CALIB_SKEW_Msk     (0x40000000L)             /*!< SKEW (Bitfield-Mask: 0x01)                            */
#define AC784X_SysTick_CALIB_NOREF_Pos    (31U)                    /*!< NOREF (Bit 31)                                        */
#define AC784X_SysTick_CALIB_NOREF_Msk    (0x80000000U)            /*!< NOREF (Bitfield-Mask: 0x01)                           */

/* =========================================================================================================================== */
/* ================                                            SMU                                            ================ */
/* =========================================================================================================================== */

/* =========================================================  SFES  ========================================================== */
#define SMU_SFES_HWSFES_Pos               (0U)                     /*!< HWSFES (Bit 0)                                        */
#define SMU_SFES_HWSFES_Msk               (0x3ffffffU)             /*!< HWSFES (Bitfield-Mask: 0x3ffffff)                     */
#define SMU_SFES_HWSFES0_Pos              (0U)                     /*!< HWSFES0 (Bit 0)                                       */
#define SMU_SFES_HWSFES0_Msk              (0x1U)                   /*!< HWSFES0 (Bitfield-Mask: 0x01)                         */
#define SMU_SFES_HWSFES1_Pos              (1U)                     /*!< HWSFES1 (Bit 1)                                       */
#define SMU_SFES_HWSFES1_Msk              (0x2U)                   /*!< HWSFES1 (Bitfield-Mask: 0x01)                         */
#define SMU_SFES_HWSFES2_Pos              (2U)                     /*!< HWSFES2 (Bit 2)                                       */
#define SMU_SFES_HWSFES2_Msk              (0x4U)                   /*!< HWSFES2 (Bitfield-Mask: 0x01)                         */
#define SMU_SFES_HWSFES3_Pos              (3U)                     /*!< HWSFES3 (Bit 3)                                       */
#define SMU_SFES_HWSFES3_Msk              (0x8U)                   /*!< HWSFES3 (Bitfield-Mask: 0x01)                         */
#define SMU_SFES_HWSFES4_Pos              (4U)                     /*!< HWSFES4 (Bit 4)                                       */
#define SMU_SFES_HWSFES4_Msk              (0x10U)                  /*!< HWSFES4 (Bitfield-Mask: 0x01)                         */
#define SMU_SFES_HWSFES5_Pos              (5U)                     /*!< HWSFES5 (Bit 5)                                       */
#define SMU_SFES_HWSFES5_Msk              (0x20U)                  /*!< HWSFES5 (Bitfield-Mask: 0x01)                         */
#define SMU_SFES_HWSFES6_Pos              (6U)                     /*!< HWSFES6 (Bit 6)                                       */
#define SMU_SFES_HWSFES6_Msk              (0x40U)                  /*!< HWSFES6 (Bitfield-Mask: 0x01)                         */
#define SMU_SFES_HWSFES7_Pos              (7U)                     /*!< HWSFES7 (Bit 7)                                       */
#define SMU_SFES_HWSFES7_Msk              (0x80U)                  /*!< HWSFES7 (Bitfield-Mask: 0x01)                         */
#define SMU_SFES_HWSFES8_Pos              (8U)                     /*!< HWSFES8 (Bit 8)                                       */
#define SMU_SFES_HWSFES8_Msk              (0x100U)                 /*!< HWSFES8 (Bitfield-Mask: 0x01)                         */
#define SMU_SFES_HWSFES9_Pos              (9U)                     /*!< HWSFES9 (Bit 9)                                       */
#define SMU_SFES_HWSFES9_Msk              (0x200U)                 /*!< HWSFES9 (Bitfield-Mask: 0x01)                         */
#define SMU_SFES_HWSFES10_Pos             (10U)                    /*!< HWSFES10 (Bit 10)                                     */
#define SMU_SFES_HWSFES10_Msk             (0x400U)                 /*!< HWSFES10 (Bitfield-Mask: 0x01)                        */
#define SMU_SFES_HWSFES11_Pos             (11U)                    /*!< HWSFES11 (Bit 11)                                     */
#define SMU_SFES_HWSFES11_Msk             (0x800U)                 /*!< HWSFES11 (Bitfield-Mask: 0x01)                        */
#define SMU_SFES_HWSFES12_Pos             (12U)                    /*!< HWSFES12 (Bit 12)                                     */
#define SMU_SFES_HWSFES12_Msk             (0x1000U)                /*!< HWSFES12 (Bitfield-Mask: 0x01)                        */
#define SMU_SFES_HWSFES13_Pos             (13U)                    /*!< HWSFES13 (Bit 13)                                     */
#define SMU_SFES_HWSFES13_Msk             (0x2000U)                /*!< HWSFES13 (Bitfield-Mask: 0x01)                        */
#define SMU_SFES_HWSFES14_Pos             (14U)                    /*!< HWSFES14 (Bit 14)                                     */
#define SMU_SFES_HWSFES14_Msk             (0x4000U)                /*!< HWSFES14 (Bitfield-Mask: 0x01)                        */
#define SMU_SFES_HWSFES15_Pos             (15U)                    /*!< HWSFES15 (Bit 15)                                     */
#define SMU_SFES_HWSFES15_Msk             (0x8000U)                /*!< HWSFES15 (Bitfield-Mask: 0x01)                        */
#define SMU_SFES_HWSFES16_Pos             (16U)                    /*!< HWSFES16 (Bit 16)                                     */
#define SMU_SFES_HWSFES16_Msk             (0x10000U)               /*!< HWSFES16 (Bitfield-Mask: 0x01)                        */
#define SMU_SFES_HWSFES18_Pos             (18U)                    /*!< HWSFES18 (Bit 18)                                     */
#define SMU_SFES_HWSFES18_Msk             (0x40000U)               /*!< HWSFES18 (Bitfield-Mask: 0x01)                        */
#define SMU_SFES_HWSFES19_Pos             (19U)                    /*!< HWSFES19 (Bit 19)                                     */
#define SMU_SFES_HWSFES19_Msk             (0x80000U)               /*!< HWSFES19 (Bitfield-Mask: 0x01)                        */
#define SMU_SFES_HWSFES20_Pos             (20U)                    /*!< HWSFES20 (Bit 20)                                     */
#define SMU_SFES_HWSFES20_Msk             (0x100000U)              /*!< HWSFES20 (Bitfield-Mask: 0x01)                        */
#define SMU_SFES_HWSFES21_Pos             (21U)                    /*!< HWSFES21 (Bit 21)                                     */
#define SMU_SFES_HWSFES21_Msk             (0x200000U)              /*!< HWSFES21 (Bitfield-Mask: 0x01)                        */
#define SMU_SFES_HWSFES22_Pos             (22U)                    /*!< HWSFES22 (Bit 22)                                     */
#define SMU_SFES_HWSFES22_Msk             (0x400000U)              /*!< HWSFES22 (Bitfield-Mask: 0x01)                        */
#define SMU_SFES_HWSFES23_Pos             (23U)                    /*!< HWSFES23 (Bit 23)                                     */
#define SMU_SFES_HWSFES23_Msk             (0x800000U)              /*!< HWSFES23 (Bitfield-Mask: 0x01)                        */
#define SMU_SFES_HWSFES24_Pos             (24U)                    /*!< HWSFES24 (Bit 24)                                     */
#define SMU_SFES_HWSFES24_Msk             (0x1000000U)             /*!< HWSFES24 (Bitfield-Mask: 0x01)                        */
#define SMU_SFES_HWSFES25_Pos             (25U)                    /*!< HWSFES25 (Bit 25)                                     */
#define SMU_SFES_HWSFES25_Msk             (0x2000000U)             /*!< HWSFES25 (Bitfield-Mask: 0x01)                        */
#define SMU_SFES_SWSFES_Pos               (26U)                    /*!< SWSFES (Bit 26)                                       */
#define SMU_SFES_SWSFES_Msk               (0x4000000U)             /*!< SWSFES (Bitfield-Mask: 0x01)                          */
/* =========================================================  SFESS  ========================================================= */
#define SMU_SFESS_HWSFES_Pos              (0U)                     /*!< HWSFES (Bit 0)                                        */
#define SMU_SFESS_HWSFES_Msk              (0x3ffffffU)             /*!< HWSFES (Bitfield-Mask: 0x3ffffff)                     */
#define SMU_SFESS_HWSFES0_Pos             (0U)                     /*!< HWSFES0 (Bit 0)                                       */
#define SMU_SFESS_HWSFES0_Msk             (0x1U)                   /*!< HWSFES0 (Bitfield-Mask: 0x01)                         */
#define SMU_SFESS_HWSFES1_Pos             (1U)                     /*!< HWSFES1 (Bit 1)                                       */
#define SMU_SFESS_HWSFES1_Msk             (0x2U)                   /*!< HWSFES1 (Bitfield-Mask: 0x01)                         */
#define SMU_SFESS_HWSFES2_Pos             (2U)                     /*!< HWSFES2 (Bit 2)                                       */
#define SMU_SFESS_HWSFES2_Msk             (0x4U)                   /*!< HWSFES2 (Bitfield-Mask: 0x01)                         */
#define SMU_SFESS_HWSFES3_Pos             (3U)                     /*!< HWSFES3 (Bit 3)                                       */
#define SMU_SFESS_HWSFES3_Msk             (0x8U)                   /*!< HWSFES3 (Bitfield-Mask: 0x01)                         */
#define SMU_SFESS_HWSFES4_Pos             (4U)                     /*!< HWSFES4 (Bit 4)                                       */
#define SMU_SFESS_HWSFES4_Msk             (0x10U)                  /*!< HWSFES4 (Bitfield-Mask: 0x01)                         */
#define SMU_SFESS_HWSFES5_Pos             (5U)                     /*!< HWSFES5 (Bit 5)                                       */
#define SMU_SFESS_HWSFES5_Msk             (0x20U)                  /*!< HWSFES5 (Bitfield-Mask: 0x01)                         */
#define SMU_SFESS_HWSFES6_Pos             (6U)                     /*!< HWSFES6 (Bit 6)                                       */
#define SMU_SFESS_HWSFES6_Msk             (0x40U)                  /*!< HWSFES6 (Bitfield-Mask: 0x01)                         */
#define SMU_SFESS_HWSFES7_Pos             (7U)                     /*!< HWSFES7 (Bit 7)                                       */
#define SMU_SFESS_HWSFES7_Msk             (0x80U)                  /*!< HWSFES7 (Bitfield-Mask: 0x01)                         */
#define SMU_SFESS_HWSFES8_Pos             (8U)                     /*!< HWSFES8 (Bit 8)                                       */
#define SMU_SFESS_HWSFES8_Msk             (0x100U)                 /*!< HWSFES8 (Bitfield-Mask: 0x01)                         */
#define SMU_SFESS_HWSFES9_Pos             (9U)                     /*!< HWSFES9 (Bit 9)                                       */
#define SMU_SFESS_HWSFES9_Msk             (0x200U)                 /*!< HWSFES9 (Bitfield-Mask: 0x01)                         */
#define SMU_SFESS_HWSFES10_Pos            (10U)                    /*!< HWSFES10 (Bit 10)                                     */
#define SMU_SFESS_HWSFES10_Msk            (0x400U)                 /*!< HWSFES10 (Bitfield-Mask: 0x01)                        */
#define SMU_SFESS_HWSFES11_Pos            (11U)                    /*!< HWSFES11 (Bit 11)                                     */
#define SMU_SFESS_HWSFES11_Msk            (0x800U)                 /*!< HWSFES11 (Bitfield-Mask: 0x01)                        */
#define SMU_SFESS_HWSFES12_Pos            (12U)                    /*!< HWSFES12 (Bit 12)                                     */
#define SMU_SFESS_HWSFES12_Msk            (0x1000U)                /*!< HWSFES12 (Bitfield-Mask: 0x01)                        */
#define SMU_SFESS_HWSFES13_Pos            (13U)                    /*!< HWSFES13 (Bit 13)                                     */
#define SMU_SFESS_HWSFES13_Msk            (0x2000U)                /*!< HWSFES13 (Bitfield-Mask: 0x01)                        */
#define SMU_SFESS_HWSFES14_Pos            (14U)                    /*!< HWSFES14 (Bit 14)                                     */
#define SMU_SFESS_HWSFES14_Msk            (0x4000U)                /*!< HWSFES14 (Bitfield-Mask: 0x01)                        */
#define SMU_SFESS_HWSFES15_Pos            (15U)                    /*!< HWSFES15 (Bit 15)                                     */
#define SMU_SFESS_HWSFES15_Msk            (0x8000U)                /*!< HWSFES15 (Bitfield-Mask: 0x01)                        */
#define SMU_SFESS_HWSFES16_Pos            (16U)                    /*!< HWSFES16 (Bit 16)                                     */
#define SMU_SFESS_HWSFES16_Msk            (0x10000U)               /*!< HWSFES16 (Bitfield-Mask: 0x01)                        */
#define SMU_SFESS_HWSFES18_Pos            (18U)                    /*!< HWSFES18 (Bit 18)                                     */
#define SMU_SFESS_HWSFES18_Msk            (0x40000U)               /*!< HWSFES18 (Bitfield-Mask: 0x01)                        */
#define SMU_SFESS_HWSFES19_Pos            (19U)                    /*!< HWSFES19 (Bit 19)                                     */
#define SMU_SFESS_HWSFES19_Msk            (0x80000U)               /*!< HWSFES19 (Bitfield-Mask: 0x01)                        */
#define SMU_SFESS_HWSFES20_Pos            (20U)                    /*!< HWSFES20 (Bit 20)                                     */
#define SMU_SFESS_HWSFES20_Msk            (0x100000U)              /*!< HWSFES20 (Bitfield-Mask: 0x01)                        */
#define SMU_SFESS_HWSFES21_Pos            (21U)                    /*!< HWSFES21 (Bit 21)                                     */
#define SMU_SFESS_HWSFES21_Msk            (0x200000U)              /*!< HWSFES21 (Bitfield-Mask: 0x01)                        */
#define SMU_SFESS_HWSFES22_Pos            (22U)                    /*!< HWSFES22 (Bit 22)                                     */
#define SMU_SFESS_HWSFES22_Msk            (0x400000U)              /*!< HWSFES22 (Bitfield-Mask: 0x01)                        */
#define SMU_SFESS_HWSFES23_Pos            (23U)                    /*!< HWSFES23 (Bit 23)                                     */
#define SMU_SFESS_HWSFES23_Msk            (0x800000U)              /*!< HWSFES23 (Bitfield-Mask: 0x01)                        */
#define SMU_SFESS_HWSFES24_Pos            (24U)                    /*!< HWSFES24 (Bit 24)                                     */
#define SMU_SFESS_HWSFES24_Msk            (0x1000000U)             /*!< HWSFES24 (Bitfield-Mask: 0x01)                        */
#define SMU_SFESS_HWSFES25_Pos            (25U)                    /*!< HWSFES25 (Bit 25)                                     */
#define SMU_SFESS_HWSFES25_Msk            (0x2000000U)             /*!< HWSFES25 (Bitfield-Mask: 0x01)                        */
#define SMU_SFESS_SWSFES_Pos              (26U)                    /*!< SWSFES (Bit 26)                                       */
#define SMU_SFESS_SWSFES_Msk              (0x4000000U)             /*!< SWSFES (Bitfield-Mask: 0x01)                          */
/* =========================================================  LFES  ========================================================== */
#define SMU_LFES_HWLFES_Pos               (0U)                     /*!< HWLFES (Bit 0)                                        */
#define SMU_LFES_HWLFES_Msk               (0x1fU)                  /*!< HWLFES (Bitfield-Mask: 0x1f)                          */
#define SMU_LFES_HWLFES0_Pos              (0U)                     /*!< HWLFES0 (Bit 0)                                       */
#define SMU_LFES_HWLFES0_Msk              (0x1U)                   /*!< HWLFES0 (Bitfield-Mask: 0x01)                         */
#define SMU_LFES_HWLFES1_Pos              (1U)                     /*!< HWLFES1 (Bit 1)                                       */
#define SMU_LFES_HWLFES1_Msk              (0x2U)                   /*!< HWLFES1 (Bitfield-Mask: 0x01)                         */
#define SMU_LFES_HWLFES2_Pos              (2U)                     /*!< HWLFES2 (Bit 2)                                       */
#define SMU_LFES_HWLFES2_Msk              (0x4U)                   /*!< HWLFES2 (Bitfield-Mask: 0x01)                         */
#define SMU_LFES_HWLFES3_Pos              (3U)                     /*!< HWLFES3 (Bit 3)                                       */
#define SMU_LFES_HWLFES3_Msk              (0x8U)                   /*!< HWLFES3 (Bitfield-Mask: 0x01)                         */
#define SMU_LFES_HWLFES4_Pos              (4U)                     /*!< HWLFES4 (Bit 4)                                       */
#define SMU_LFES_HWLFES4_Msk              (0x10U)                  /*!< HWLFES4 (Bitfield-Mask: 0x01)                         */
#define SMU_LFES_SWLFES_Pos               (5U)                     /*!< SWLFES (Bit 5)                                        */
#define SMU_LFES_SWLFES_Msk               (0x20U)                  /*!< SWLFES (Bitfield-Mask: 0x01)                          */
/* =========================================================  LFESS  ========================================================= */
#define SMU_LFESS_HWLFES_Pos              (0U)                     /*!< HWLFES (Bit 0)                                        */
#define SMU_LFESS_HWLFES_Msk              (0x1fU)                  /*!< HWLFES (Bitfield-Mask: 0x1f)                          */
#define SMU_LFESS_HWLFES0_Pos             (0U)                     /*!< HWLFES0 (Bit 0)                                       */
#define SMU_LFESS_HWLFES0_Msk             (0x1U)                   /*!< HWLFES0 (Bitfield-Mask: 0x01)                         */
#define SMU_LFESS_HWLFES1_Pos             (1U)                     /*!< HWLFES1 (Bit 1)                                       */
#define SMU_LFESS_HWLFES1_Msk             (0x2U)                   /*!< HWLFES1 (Bitfield-Mask: 0x01)                         */
#define SMU_LFESS_HWLFES2_Pos             (2U)                     /*!< HWLFES2 (Bit 2)                                       */
#define SMU_LFESS_HWLFES2_Msk             (0x4U)                   /*!< HWLFES2 (Bitfield-Mask: 0x01)                         */
#define SMU_LFESS_HWLFES3_Pos             (3U)                     /*!< HWLFES3 (Bit 3)                                       */
#define SMU_LFESS_HWLFES3_Msk             (0x8U)                   /*!< HWLFES3 (Bitfield-Mask: 0x01)                         */
#define SMU_LFESS_HWLFES4_Pos             (4U)                     /*!< HWLFES4 (Bit 4)                                       */
#define SMU_LFESS_HWLFES4_Msk             (0x10U)                  /*!< HWLFES4 (Bitfield-Mask: 0x01)                         */
#define SMU_LFESS_SWLFES_Pos              (5U)                     /*!< SWLFES (Bit 5)                                        */
#define SMU_LFESS_SWLFES_Msk              (0x20U)                  /*!< SWLFES (Bitfield-Mask: 0x01)                          */
/* ========================================================  LKSEQ0  ========================================================= */
#define SMU_LKSEQ0_LKSEQ0_Pos             (0U)                     /*!< LKSEQ0 (Bit 0)                                        */
#define SMU_LKSEQ0_LKSEQ0_Msk             (0xffU)                  /*!< LKSEQ0 (Bitfield-Mask: 0xff)                          */
/* ========================================================  LKSEQ1  ========================================================= */
#define SMU_LKSEQ1_LKSEQ1_Pos             (0U)                     /*!< LKSEQ1 (Bit 0)                                        */
#define SMU_LKSEQ1_LKSEQ1_Msk             (0xffU)                  /*!< LKSEQ1 (Bitfield-Mask: 0xff)                          */
/* =========================================================  SWSFE  ========================================================= */
#define SMU_SWSFE_SWSFE_Pos               (0U)                     /*!< SWSFE (Bit 0)                                         */
#define SMU_SWSFE_SWSFE_Msk               (0xffffU)                /*!< SWSFE (Bitfield-Mask: 0xffff)                         */
#define SMU_SWSFE_SWSFE0_Pos              (0U)                     /*!< SWSFE0 (Bit 0)                                        */
#define SMU_SWSFE_SWSFE0_Msk              (0x1U)                   /*!< SWSFE0 (Bitfield-Mask: 0x01)                          */
#define SMU_SWSFE_SWSFE1_Pos              (1U)                     /*!< SWSFE1 (Bit 1)                                        */
#define SMU_SWSFE_SWSFE1_Msk              (0x2U)                   /*!< SWSFE1 (Bitfield-Mask: 0x01)                          */
#define SMU_SWSFE_SWSFE2_Pos              (2U)                     /*!< SWSFE2 (Bit 2)                                        */
#define SMU_SWSFE_SWSFE2_Msk              (0x4U)                   /*!< SWSFE2 (Bitfield-Mask: 0x01)                          */
#define SMU_SWSFE_SWSFE3_Pos              (3U)                     /*!< SWSFE3 (Bit 3)                                        */
#define SMU_SWSFE_SWSFE3_Msk              (0x8U)                   /*!< SWSFE3 (Bitfield-Mask: 0x01)                          */
#define SMU_SWSFE_SWSFE4_Pos              (4U)                     /*!< SWSFE4 (Bit 4)                                        */
#define SMU_SWSFE_SWSFE4_Msk              (0x10U)                  /*!< SWSFE4 (Bitfield-Mask: 0x01)                          */
#define SMU_SWSFE_SWSFE5_Pos              (5U)                     /*!< SWSFE5 (Bit 5)                                        */
#define SMU_SWSFE_SWSFE5_Msk              (0x20U)                  /*!< SWSFE5 (Bitfield-Mask: 0x01)                          */
#define SMU_SWSFE_SWSFE6_Pos              (6U)                     /*!< SWSFE6 (Bit 6)                                        */
#define SMU_SWSFE_SWSFE6_Msk              (0x40U)                  /*!< SWSFE6 (Bitfield-Mask: 0x01)                          */
#define SMU_SWSFE_SWSFE7_Pos              (7U)                     /*!< SWSFE7 (Bit 7)                                        */
#define SMU_SWSFE_SWSFE7_Msk              (0x80U)                  /*!< SWSFE7 (Bitfield-Mask: 0x01)                          */
#define SMU_SWSFE_SWSFE8_Pos              (8U)                     /*!< SWSFE8 (Bit 8)                                        */
#define SMU_SWSFE_SWSFE8_Msk              (0x100U)                 /*!< SWSFE8 (Bitfield-Mask: 0x01)                          */
#define SMU_SWSFE_SWSFE9_Pos              (9U)                     /*!< SWSFE9 (Bit 9)                                        */
#define SMU_SWSFE_SWSFE9_Msk              (0x200U)                 /*!< SWSFE9 (Bitfield-Mask: 0x01)                          */
#define SMU_SWSFE_SWSFE10_Pos             (10U)                    /*!< SWSFE10 (Bit 10)                                      */
#define SMU_SWSFE_SWSFE10_Msk             (0x400U)                 /*!< SWSFE10 (Bitfield-Mask: 0x01)                         */
#define SMU_SWSFE_SWSFE11_Pos             (11U)                    /*!< SWSFE11 (Bit 11)                                      */
#define SMU_SWSFE_SWSFE11_Msk             (0x800U)                 /*!< SWSFE11 (Bitfield-Mask: 0x01)                         */
#define SMU_SWSFE_SWSFE12_Pos             (12U)                    /*!< SWSFE12 (Bit 12)                                      */
#define SMU_SWSFE_SWSFE12_Msk             (0x1000U)                /*!< SWSFE12 (Bitfield-Mask: 0x01)                         */
#define SMU_SWSFE_SWSFE13_Pos             (13U)                    /*!< SWSFE13 (Bit 13)                                      */
#define SMU_SWSFE_SWSFE13_Msk             (0x2000U)                /*!< SWSFE13 (Bitfield-Mask: 0x01)                         */
#define SMU_SWSFE_SWSFE14_Pos             (14U)                    /*!< SWSFE14 (Bit 14)                                      */
#define SMU_SWSFE_SWSFE14_Msk             (0x4000U)                /*!< SWSFE14 (Bitfield-Mask: 0x01)                         */
#define SMU_SWSFE_SWSFE15_Pos             (15U)                    /*!< SWSFE15 (Bit 15)                                      */
#define SMU_SWSFE_SWSFE15_Msk             (0x8000U)                /*!< SWSFE15 (Bitfield-Mask: 0x01)                         */
/* ========================================================  SWSFES  ========================================================= */
#define SMU_SWSFES_SWSFES_Pos             (0U)                     /*!< SWSFES (Bit 0)                                        */
#define SMU_SWSFES_SWSFES_Msk             (0xffffU)                /*!< SWSFES (Bitfield-Mask: 0xffff)                        */
#define SMU_SWSFES_SWSFES0_Pos            (0U)                     /*!< SWSFES0 (Bit 0)                                       */
#define SMU_SWSFES_SWSFES0_Msk            (0x1U)                   /*!< SWSFES0 (Bitfield-Mask: 0x01)                         */
#define SMU_SWSFES_SWSFES1_Pos            (1U)                     /*!< SWSFES1 (Bit 1)                                       */
#define SMU_SWSFES_SWSFES1_Msk            (0x2U)                   /*!< SWSFES1 (Bitfield-Mask: 0x01)                         */
#define SMU_SWSFES_SWSFES2_Pos            (2U)                     /*!< SWSFES2 (Bit 2)                                       */
#define SMU_SWSFES_SWSFES2_Msk            (0x4U)                   /*!< SWSFES2 (Bitfield-Mask: 0x01)                         */
#define SMU_SWSFES_SWSFES3_Pos            (3U)                     /*!< SWSFES3 (Bit 3)                                       */
#define SMU_SWSFES_SWSFES3_Msk            (0x8U)                   /*!< SWSFES3 (Bitfield-Mask: 0x01)                         */
#define SMU_SWSFES_SWSFES4_Pos            (4U)                     /*!< SWSFES4 (Bit 4)                                       */
#define SMU_SWSFES_SWSFES4_Msk            (0x10U)                  /*!< SWSFES4 (Bitfield-Mask: 0x01)                         */
#define SMU_SWSFES_SWSFES5_Pos            (5U)                     /*!< SWSFES5 (Bit 5)                                       */
#define SMU_SWSFES_SWSFES5_Msk            (0x20U)                  /*!< SWSFES5 (Bitfield-Mask: 0x01)                         */
#define SMU_SWSFES_SWSFES6_Pos            (6U)                     /*!< SWSFES6 (Bit 6)                                       */
#define SMU_SWSFES_SWSFES6_Msk            (0x40U)                  /*!< SWSFES6 (Bitfield-Mask: 0x01)                         */
#define SMU_SWSFES_SWSFES7_Pos            (7U)                     /*!< SWSFES7 (Bit 7)                                       */
#define SMU_SWSFES_SWSFES7_Msk            (0x80U)                  /*!< SWSFES7 (Bitfield-Mask: 0x01)                         */
#define SMU_SWSFES_SWSFES8_Pos            (8U)                     /*!< SWSFES8 (Bit 8)                                       */
#define SMU_SWSFES_SWSFES8_Msk            (0x100U)                 /*!< SWSFES8 (Bitfield-Mask: 0x01)                         */
#define SMU_SWSFES_SWSFES9_Pos            (9U)                     /*!< SWSFES9 (Bit 9)                                       */
#define SMU_SWSFES_SWSFES9_Msk            (0x200U)                 /*!< SWSFES9 (Bitfield-Mask: 0x01)                         */
#define SMU_SWSFES_SWSFES10_Pos           (10U)                    /*!< SWSFES10 (Bit 10)                                     */
#define SMU_SWSFES_SWSFES10_Msk           (0x400U)                 /*!< SWSFES10 (Bitfield-Mask: 0x01)                        */
#define SMU_SWSFES_SWSFES11_Pos           (11U)                    /*!< SWSFES11 (Bit 11)                                     */
#define SMU_SWSFES_SWSFES11_Msk           (0x800U)                 /*!< SWSFES11 (Bitfield-Mask: 0x01)                        */
#define SMU_SWSFES_SWSFES12_Pos           (12U)                    /*!< SWSFES12 (Bit 12)                                     */
#define SMU_SWSFES_SWSFES12_Msk           (0x1000U)                /*!< SWSFES12 (Bitfield-Mask: 0x01)                        */
#define SMU_SWSFES_SWSFES13_Pos           (13U)                    /*!< SWSFES13 (Bit 13)                                     */
#define SMU_SWSFES_SWSFES13_Msk           (0x2000U)                /*!< SWSFES13 (Bitfield-Mask: 0x01)                        */
#define SMU_SWSFES_SWSFES14_Pos           (14U)                    /*!< SWSFES14 (Bit 14)                                     */
#define SMU_SWSFES_SWSFES14_Msk           (0x4000U)                /*!< SWSFES14 (Bitfield-Mask: 0x01)                        */
#define SMU_SWSFES_SWSFES15_Pos           (15U)                    /*!< SWSFES15 (Bit 15)                                     */
#define SMU_SWSFES_SWSFES15_Msk           (0x8000U)                /*!< SWSFES15 (Bitfield-Mask: 0x01)                        */
/* =========================================================  SWLFE  ========================================================= */
#define SMU_SWLFE_SWLFE_Pos               (0U)                     /*!< SWLFE (Bit 0)                                         */
#define SMU_SWLFE_SWLFE_Msk               (0xffffU)                /*!< SWLFE (Bitfield-Mask: 0xffff)                         */
#define SMU_SWLFE_SWLFE0_Pos              (0U)                     /*!< SWLFE0 (Bit 0)                                        */
#define SMU_SWLFE_SWLFE0_Msk              (0x1U)                   /*!< SWLFE0 (Bitfield-Mask: 0x01)                          */
#define SMU_SWLFE_SWSLE1_Pos              (1U)                     /*!< SWSLE1 (Bit 1)                                        */
#define SMU_SWLFE_SWSLE1_Msk              (0x2U)                   /*!< SWSLE1 (Bitfield-Mask: 0x01)                          */
#define SMU_SWLFE_SWLFE2_Pos              (2U)                     /*!< SWLFE2 (Bit 2)                                        */
#define SMU_SWLFE_SWLFE2_Msk              (0x4U)                   /*!< SWLFE2 (Bitfield-Mask: 0x01)                          */
#define SMU_SWLFE_SWLFE3_Pos              (3U)                     /*!< SWLFE3 (Bit 3)                                        */
#define SMU_SWLFE_SWLFE3_Msk              (0x8U)                   /*!< SWLFE3 (Bitfield-Mask: 0x01)                          */
#define SMU_SWLFE_SWLFE4_Pos              (4U)                     /*!< SWLFE4 (Bit 4)                                        */
#define SMU_SWLFE_SWLFE4_Msk              (0x10U)                  /*!< SWLFE4 (Bitfield-Mask: 0x01)                          */
#define SMU_SWLFE_SWLFE5_Pos              (5U)                     /*!< SWLFE5 (Bit 5)                                        */
#define SMU_SWLFE_SWLFE5_Msk              (0x20U)                  /*!< SWLFE5 (Bitfield-Mask: 0x01)                          */
#define SMU_SWLFE_SWLFE6_Pos              (6U)                     /*!< SWLFE6 (Bit 6)                                        */
#define SMU_SWLFE_SWLFE6_Msk              (0x40U)                  /*!< SWLFE6 (Bitfield-Mask: 0x01)                          */
#define SMU_SWLFE_SWLFE7_Pos              (7U)                     /*!< SWLFE7 (Bit 7)                                        */
#define SMU_SWLFE_SWLFE7_Msk              (0x80U)                  /*!< SWLFE7 (Bitfield-Mask: 0x01)                          */
#define SMU_SWLFE_SWLFE8_Pos              (8U)                     /*!< SWLFE8 (Bit 8)                                        */
#define SMU_SWLFE_SWLFE8_Msk              (0x100U)                 /*!< SWLFE8 (Bitfield-Mask: 0x01)                          */
#define SMU_SWLFE_SWLFE9_Pos              (9U)                     /*!< SWLFE9 (Bit 9)                                        */
#define SMU_SWLFE_SWLFE9_Msk              (0x200U)                 /*!< SWLFE9 (Bitfield-Mask: 0x01)                          */
#define SMU_SWLFE_SWLFE10_Pos             (10U)                    /*!< SWLFE10 (Bit 10)                                      */
#define SMU_SWLFE_SWLFE10_Msk             (0x400U)                 /*!< SWLFE10 (Bitfield-Mask: 0x01)                         */
#define SMU_SWLFE_SWLFE11_Pos             (11U)                    /*!< SWLFE11 (Bit 11)                                      */
#define SMU_SWLFE_SWLFE11_Msk             (0x800U)                 /*!< SWLFE11 (Bitfield-Mask: 0x01)                         */
#define SMU_SWLFE_SWLFE12_Pos             (12U)                    /*!< SWLFE12 (Bit 12)                                      */
#define SMU_SWLFE_SWLFE12_Msk             (0x1000U)                /*!< SWLFE12 (Bitfield-Mask: 0x01)                         */
#define SMU_SWLFE_SWLFE13_Pos             (13U)                    /*!< SWLFE13 (Bit 13)                                      */
#define SMU_SWLFE_SWLFE13_Msk             (0x2000U)                /*!< SWLFE13 (Bitfield-Mask: 0x01)                         */
#define SMU_SWLFE_SWLFE14_Pos             (14U)                    /*!< SWLFE14 (Bit 14)                                      */
#define SMU_SWLFE_SWLFE14_Msk             (0x4000U)                /*!< SWLFE14 (Bitfield-Mask: 0x01)                         */
#define SMU_SWLFE_SWLFE15_Pos             (15U)                    /*!< SWLFE15 (Bit 15)                                      */
#define SMU_SWLFE_SWLFE15_Msk             (0x8000U)                /*!< SWLFE15 (Bitfield-Mask: 0x01)                         */
/* ========================================================  SWLFES  ========================================================= */
#define SMU_SWLFES_SWLFES_Pos             (0U)                     /*!< SWLFES (Bit 0)                                        */
#define SMU_SWLFES_SWLFES_Msk             (0xffffU)                /*!< SWLFES (Bitfield-Mask: 0xffff)                        */
#define SMU_SWLFES_SWLFES0_Pos            (0U)                     /*!< SWLFES0 (Bit 0)                                       */
#define SMU_SWLFES_SWLFES0_Msk            (0x1U)                   /*!< SWLFES0 (Bitfield-Mask: 0x01)                         */
#define SMU_SWLFES_SWLFES1_Pos            (1U)                     /*!< SWLFES1 (Bit 1)                                       */
#define SMU_SWLFES_SWLFES1_Msk            (0x2U)                   /*!< SWLFES1 (Bitfield-Mask: 0x01)                         */
#define SMU_SWLFES_SWLFES2_Pos            (2U)                     /*!< SWLFES2 (Bit 2)                                       */
#define SMU_SWLFES_SWLFES2_Msk            (0x4U)                   /*!< SWLFES2 (Bitfield-Mask: 0x01)                         */
#define SMU_SWLFES_SWLFES3_Pos            (3U)                     /*!< SWLFES3 (Bit 3)                                       */
#define SMU_SWLFES_SWLFES3_Msk            (0x8U)                   /*!< SWLFES3 (Bitfield-Mask: 0x01)                         */
#define SMU_SWLFES_SWLFES4_Pos            (4U)                     /*!< SWLFES4 (Bit 4)                                       */
#define SMU_SWLFES_SWLFES4_Msk            (0x10U)                  /*!< SWLFES4 (Bitfield-Mask: 0x01)                         */
#define SMU_SWLFES_SWLFES5_Pos            (5U)                     /*!< SWLFES5 (Bit 5)                                       */
#define SMU_SWLFES_SWLFES5_Msk            (0x20U)                  /*!< SWLFES5 (Bitfield-Mask: 0x01)                         */
#define SMU_SWLFES_SWLFES6_Pos            (6U)                     /*!< SWLFES6 (Bit 6)                                       */
#define SMU_SWLFES_SWLFES6_Msk            (0x40U)                  /*!< SWLFES6 (Bitfield-Mask: 0x01)                         */
#define SMU_SWLFES_SWLFES7_Pos            (7U)                     /*!< SWLFES7 (Bit 7)                                       */
#define SMU_SWLFES_SWLFES7_Msk            (0x80U)                  /*!< SWLFES7 (Bitfield-Mask: 0x01)                         */
#define SMU_SWLFES_SWLFES8_Pos            (8U)                     /*!< SWLFES8 (Bit 8)                                       */
#define SMU_SWLFES_SWLFES8_Msk            (0x100U)                 /*!< SWLFES8 (Bitfield-Mask: 0x01)                         */
#define SMU_SWLFES_SWLFES9_Pos            (9U)                     /*!< SWLFES9 (Bit 9)                                       */
#define SMU_SWLFES_SWLFES9_Msk            (0x200U)                 /*!< SWLFES9 (Bitfield-Mask: 0x01)                         */
#define SMU_SWLFES_SWLFES10_Pos           (10U)                    /*!< SWLFES10 (Bit 10)                                     */
#define SMU_SWLFES_SWLFES10_Msk           (0x400U)                 /*!< SWLFES10 (Bitfield-Mask: 0x01)                        */
#define SMU_SWLFES_SWLFES11_Pos           (11U)                    /*!< SWLFES11 (Bit 11)                                     */
#define SMU_SWLFES_SWLFES11_Msk           (0x800U)                 /*!< SWLFES11 (Bitfield-Mask: 0x01)                        */
#define SMU_SWLFES_SWLFES12_Pos           (12U)                    /*!< SWLFES12 (Bit 12)                                     */
#define SMU_SWLFES_SWLFES12_Msk           (0x1000U)                /*!< SWLFES12 (Bitfield-Mask: 0x01)                        */
#define SMU_SWLFES_SWLFES13_Pos           (13U)                    /*!< SWLFES13 (Bit 13)                                     */
#define SMU_SWLFES_SWLFES13_Msk           (0x2000U)                /*!< SWLFES13 (Bitfield-Mask: 0x01)                        */
#define SMU_SWLFES_SWLFES14_Pos           (14U)                    /*!< SWLFES14 (Bit 14)                                     */
#define SMU_SWLFES_SWLFES14_Msk           (0x4000U)                /*!< SWLFES14 (Bitfield-Mask: 0x01)                        */
#define SMU_SWLFES_SWLFES15_Pos           (15U)                    /*!< SWLFES15 (Bit 15)                                     */
#define SMU_SWLFES_SWLFES15_Msk           (0x8000U)                /*!< SWLFES15 (Bitfield-Mask: 0x01)                        */
/* ========================================================  SFINTEN  ======================================================== */
#define SMU_SFINTEN_HWSFINTE_Pos          (0U)                     /*!< HWSFINTE (Bit 0)                                      */
#define SMU_SFINTEN_HWSFINTE_Msk          (0x3ffffffU)             /*!< HWSFINTE (Bitfield-Mask: 0x3ffffff)                   */
#define SMU_SFINTEN_HWSFINTE0_Pos         (0U)                     /*!< HWSFINTE0 (Bit 0)                                     */
#define SMU_SFINTEN_HWSFINTE0_Msk         (0x1U)                   /*!< HWSFINTE0 (Bitfield-Mask: 0x01)                       */
#define SMU_SFINTEN_HWSFINTE1_Pos         (1U)                     /*!< HWSFINTE1 (Bit 1)                                     */
#define SMU_SFINTEN_HWSFINTE1_Msk         (0x2U)                   /*!< HWSFINTE1 (Bitfield-Mask: 0x01)                       */
#define SMU_SFINTEN_HWSFINTE2_Pos         (2U)                     /*!< HWSFINTE2 (Bit 2)                                     */
#define SMU_SFINTEN_HWSFINTE2_Msk         (0x4U)                   /*!< HWSFINTE2 (Bitfield-Mask: 0x01)                       */
#define SMU_SFINTEN_HWSFINTE3_Pos         (3U)                     /*!< HWSFINTE3 (Bit 3)                                     */
#define SMU_SFINTEN_HWSFINTE3_Msk         (0x8U)                   /*!< HWSFINTE3 (Bitfield-Mask: 0x01)                       */
#define SMU_SFINTEN_HWSFINTE4_Pos         (4U)                     /*!< HWSFINTE4 (Bit 4)                                     */
#define SMU_SFINTEN_HWSFINTE4_Msk         (0x10U)                  /*!< HWSFINTE4 (Bitfield-Mask: 0x01)                       */
#define SMU_SFINTEN_HWSFINTE5_Pos         (5U)                     /*!< HWSFINTE5 (Bit 5)                                     */
#define SMU_SFINTEN_HWSFINTE5_Msk         (0x20U)                  /*!< HWSFINTE5 (Bitfield-Mask: 0x01)                       */
#define SMU_SFINTEN_HWSFINTE6_Pos         (6U)                     /*!< HWSFINTE6 (Bit 6)                                     */
#define SMU_SFINTEN_HWSFINTE6_Msk         (0x40U)                  /*!< HWSFINTE6 (Bitfield-Mask: 0x01)                       */
#define SMU_SFINTEN_HWSFINTE7_Pos         (7U)                     /*!< HWSFINTE7 (Bit 7)                                     */
#define SMU_SFINTEN_HWSFINTE7_Msk         (0x80U)                  /*!< HWSFINTE7 (Bitfield-Mask: 0x01)                       */
#define SMU_SFINTEN_HWSFINTE8_Pos         (8U)                     /*!< HWSFINTE8 (Bit 8)                                     */
#define SMU_SFINTEN_HWSFINTE8_Msk         (0x100U)                 /*!< HWSFINTE8 (Bitfield-Mask: 0x01)                       */
#define SMU_SFINTEN_HWSFINTE9_Pos         (9U)                     /*!< HWSFINTE9 (Bit 9)                                     */
#define SMU_SFINTEN_HWSFINTE9_Msk         (0x200U)                 /*!< HWSFINTE9 (Bitfield-Mask: 0x01)                       */
#define SMU_SFINTEN_HWSFINTE10_Pos        (10U)                    /*!< HWSFINTE10 (Bit 10)                                   */
#define SMU_SFINTEN_HWSFINTE10_Msk        (0x400U)                 /*!< HWSFINTE10 (Bitfield-Mask: 0x01)                      */
#define SMU_SFINTEN_HWSFINTE11_Pos        (11U)                    /*!< HWSFINTE11 (Bit 11)                                   */
#define SMU_SFINTEN_HWSFINTE11_Msk        (0x800U)                 /*!< HWSFINTE11 (Bitfield-Mask: 0x01)                      */
#define SMU_SFINTEN_HWSFINTE12_Pos        (12U)                    /*!< HWSFINTE12 (Bit 12)                                   */
#define SMU_SFINTEN_HWSFINTE12_Msk        (0x1000U)                /*!< HWSFINTE12 (Bitfield-Mask: 0x01)                      */
#define SMU_SFINTEN_HWSFINTE13_Pos        (13U)                    /*!< HWSFINTE13 (Bit 13)                                   */
#define SMU_SFINTEN_HWSFINTE13_Msk        (0x2000U)                /*!< HWSFINTE13 (Bitfield-Mask: 0x01)                      */
#define SMU_SFINTEN_HWSFINTE14_Pos        (14U)                    /*!< HWSFINTE14 (Bit 14)                                   */
#define SMU_SFINTEN_HWSFINTE14_Msk        (0x4000U)                /*!< HWSFINTE14 (Bitfield-Mask: 0x01)                      */
#define SMU_SFINTEN_HWSFINTE15_Pos        (15U)                    /*!< HWSFINTE15 (Bit 15)                                   */
#define SMU_SFINTEN_HWSFINTE15_Msk        (0x8000U)                /*!< HWSFINTE15 (Bitfield-Mask: 0x01)                      */
#define SMU_SFINTEN_HWSFINTE16_Pos        (16U)                    /*!< HWSFINTE16 (Bit 16)                                   */
#define SMU_SFINTEN_HWSFINTE16_Msk        (0x10000U)               /*!< HWSFINTE16 (Bitfield-Mask: 0x01)                      */
#define SMU_SFINTEN_HWSFINTE18_Pos        (18U)                    /*!< HWSFINTE18 (Bit 18)                                   */
#define SMU_SFINTEN_HWSFINTE18_Msk        (0x40000U)               /*!< HWSFINTE18 (Bitfield-Mask: 0x01)                      */
#define SMU_SFINTEN_HWSFINTE19_Pos        (19U)                    /*!< HWSFINTE19 (Bit 19)                                   */
#define SMU_SFINTEN_HWSFINTE19_Msk        (0x80000U)               /*!< HWSFINTE19 (Bitfield-Mask: 0x01)                      */
#define SMU_SFINTEN_HWSFINTE20_Pos        (20U)                    /*!< HWSFINTE20 (Bit 20)                                   */
#define SMU_SFINTEN_HWSFINTE20_Msk        (0x100000U)              /*!< HWSFINTE20 (Bitfield-Mask: 0x01)                      */
#define SMU_SFINTEN_HWSFINTE21_Pos        (21U)                    /*!< HWSFINTE21 (Bit 21)                                   */
#define SMU_SFINTEN_HWSFINTE21_Msk        (0x200000U)              /*!< HWSFINTE21 (Bitfield-Mask: 0x01)                      */
#define SMU_SFINTEN_HWSFINTE22_Pos        (22U)                    /*!< HWSFINTE22 (Bit 22)                                   */
#define SMU_SFINTEN_HWSFINTE22_Msk        (0x400000U)              /*!< HWSFINTE22 (Bitfield-Mask: 0x01)                      */
#define SMU_SFINTEN_HWSFINTE23_Pos        (23U)                    /*!< HWSFINTE23 (Bit 23)                                   */
#define SMU_SFINTEN_HWSFINTE23_Msk        (0x800000U)              /*!< HWSFINTE23 (Bitfield-Mask: 0x01)                      */
#define SMU_SFINTEN_HWSFINTE24_Pos        (24U)                    /*!< HWSFINTE24 (Bit 24)                                   */
#define SMU_SFINTEN_HWSFINTE24_Msk        (0x1000000U)             /*!< HWSFINTE24 (Bitfield-Mask: 0x01)                      */
#define SMU_SFINTEN_HWSFINTE25_Pos        (25U)                    /*!< HWSFINTE25 (Bit 25)                                   */
#define SMU_SFINTEN_HWSFINTE25_Msk        (0x2000000U)             /*!< HWSFINTE25 (Bitfield-Mask: 0x01)                      */
#define SMU_SFINTEN_SWSFINTE_Pos          (26U)                    /*!< SWSFINTE (Bit 26)                                     */
#define SMU_SFINTEN_SWSFINTE_Msk          (0x4000000U)             /*!< SWSFINTE (Bitfield-Mask: 0x01)                        */
/* ========================================================  LFINTEN  ======================================================== */
#define SMU_LFINTEN_HWLFINTE_Pos          (0U)                     /*!< HWLFINTE (Bit 0)                                      */
#define SMU_LFINTEN_HWLFINTE_Msk          (0x1fU)                  /*!< HWLFINTE (Bitfield-Mask: 0x1f)                        */
#define SMU_LFINTEN_HWLFINTE0_Pos         (0U)                     /*!< HWLFINTE0 (Bit 0)                                     */
#define SMU_LFINTEN_HWLFINTE0_Msk         (0x1U)                   /*!< HWLFINTE0 (Bitfield-Mask: 0x01)                       */
#define SMU_LFINTEN_HWLFINTE1_Pos         (1U)                     /*!< HWLFINTE1 (Bit 1)                                     */
#define SMU_LFINTEN_HWLFINTE1_Msk         (0x2U)                   /*!< HWLFINTE1 (Bitfield-Mask: 0x01)                       */
#define SMU_LFINTEN_HWLFINTE2_Pos         (2U)                     /*!< HWLFINTE2 (Bit 2)                                     */
#define SMU_LFINTEN_HWLFINTE2_Msk         (0x4U)                   /*!< HWLFINTE2 (Bitfield-Mask: 0x01)                       */
#define SMU_LFINTEN_HWLFINTE3_Pos         (3U)                     /*!< HWLFINTE3 (Bit 3)                                     */
#define SMU_LFINTEN_HWLFINTE3_Msk         (0x8U)                   /*!< HWLFINTE3 (Bitfield-Mask: 0x01)                       */
#define SMU_LFINTEN_HWLFINTE4_Pos         (4U)                     /*!< HWLFINTE4 (Bit 4)                                     */
#define SMU_LFINTEN_HWLFINTE4_Msk         (0x10U)                  /*!< HWLFINTE4 (Bitfield-Mask: 0x01)                       */
#define SMU_LFINTEN_SWLFINTE_Pos          (5U)                     /*!< SWLFINTE (Bit 5)                                      */
#define SMU_LFINTEN_SWLFINTE_Msk          (0x20U)                  /*!< SWLFINTE (Bitfield-Mask: 0x01)                        */
/* ========================================================  SFRSTEN  ======================================================== */
#define SMU_SFRSTEN_HWSFRSTE_Pos          (0U)                     /*!< HWSFRSTE (Bit 0)                                      */
#define SMU_SFRSTEN_HWSFRSTE_Msk          (0x3ffffffU)             /*!< HWSFRSTE (Bitfield-Mask: 0x3ffffff)                   */
#define SMU_SFRSTEN_HWSFRSTE0_Pos         (0U)                     /*!< HWSFRSTE0 (Bit 0)                                     */
#define SMU_SFRSTEN_HWSFRSTE0_Msk         (0x1U)                   /*!< HWSFRSTE0 (Bitfield-Mask: 0x01)                       */
#define SMU_SFRSTEN_HWSFRSTE1_Pos         (1U)                     /*!< HWSFRSTE1 (Bit 1)                                     */
#define SMU_SFRSTEN_HWSFRSTE1_Msk         (0x2U)                   /*!< HWSFRSTE1 (Bitfield-Mask: 0x01)                       */
#define SMU_SFRSTEN_HWSFRSTE2_Pos         (2U)                     /*!< HWSFRSTE2 (Bit 2)                                     */
#define SMU_SFRSTEN_HWSFRSTE2_Msk         (0x4U)                   /*!< HWSFRSTE2 (Bitfield-Mask: 0x01)                       */
#define SMU_SFRSTEN_HWSFRSTE3_Pos         (3U)                     /*!< HWSFRSTE3 (Bit 3)                                     */
#define SMU_SFRSTEN_HWSFRSTE3_Msk         (0x8U)                   /*!< HWSFRSTE3 (Bitfield-Mask: 0x01)                       */
#define SMU_SFRSTEN_HWSFRSTE4_Pos         (4U)                     /*!< HWSFRSTE4 (Bit 4)                                     */
#define SMU_SFRSTEN_HWSFRSTE4_Msk         (0x10U)                  /*!< HWSFRSTE4 (Bitfield-Mask: 0x01)                       */
#define SMU_SFRSTEN_HWSFRSTE5_Pos         (5U)                     /*!< HWSFRSTE5 (Bit 5)                                     */
#define SMU_SFRSTEN_HWSFRSTE5_Msk         (0x20U)                  /*!< HWSFRSTE5 (Bitfield-Mask: 0x01)                       */
#define SMU_SFRSTEN_HWSFRSTE6_Pos         (6U)                     /*!< HWSFRSTE6 (Bit 6)                                     */
#define SMU_SFRSTEN_HWSFRSTE6_Msk         (0x40U)                  /*!< HWSFRSTE6 (Bitfield-Mask: 0x01)                       */
#define SMU_SFRSTEN_HWSFRSTE7_Pos         (7U)                     /*!< HWSFRSTE7 (Bit 7)                                     */
#define SMU_SFRSTEN_HWSFRSTE7_Msk         (0x80U)                  /*!< HWSFRSTE7 (Bitfield-Mask: 0x01)                       */
#define SMU_SFRSTEN_HWSFRSTE8_Pos         (8U)                     /*!< HWSFRSTE8 (Bit 8)                                     */
#define SMU_SFRSTEN_HWSFRSTE8_Msk         (0x100U)                 /*!< HWSFRSTE8 (Bitfield-Mask: 0x01)                       */
#define SMU_SFRSTEN_HWSFRSTE9_Pos         (9U)                     /*!< HWSFRSTE9 (Bit 9)                                     */
#define SMU_SFRSTEN_HWSFRSTE9_Msk         (0x200U)                 /*!< HWSFRSTE9 (Bitfield-Mask: 0x01)                       */
#define SMU_SFRSTEN_HWSFRSTE10_Pos        (10U)                    /*!< HWSFRSTE10 (Bit 10)                                   */
#define SMU_SFRSTEN_HWSFRSTE10_Msk        (0x400U)                 /*!< HWSFRSTE10 (Bitfield-Mask: 0x01)                      */
#define SMU_SFRSTEN_HWSFRSTE11_Pos        (11U)                    /*!< HWSFRSTE11 (Bit 11)                                   */
#define SMU_SFRSTEN_HWSFRSTE11_Msk        (0x800U)                 /*!< HWSFRSTE11 (Bitfield-Mask: 0x01)                      */
#define SMU_SFRSTEN_HWSFRSTE12_Pos        (12U)                    /*!< HWSFRSTE12 (Bit 12)                                   */
#define SMU_SFRSTEN_HWSFRSTE12_Msk        (0x1000U)                /*!< HWSFRSTE12 (Bitfield-Mask: 0x01)                      */
#define SMU_SFRSTEN_HWSFRSTE13_Pos        (13U)                    /*!< HWSFRSTE13 (Bit 13)                                   */
#define SMU_SFRSTEN_HWSFRSTE13_Msk        (0x2000U)                /*!< HWSFRSTE13 (Bitfield-Mask: 0x01)                      */
#define SMU_SFRSTEN_HWSFRSTE14_Pos        (14U)                    /*!< HWSFRSTE14 (Bit 14)                                   */
#define SMU_SFRSTEN_HWSFRSTE14_Msk        (0x4000U)                /*!< HWSFRSTE14 (Bitfield-Mask: 0x01)                      */
#define SMU_SFRSTEN_HWSFRSTE15_Pos        (15U)                    /*!< HWSFRSTE15 (Bit 15)                                   */
#define SMU_SFRSTEN_HWSFRSTE15_Msk        (0x8000U)                /*!< HWSFRSTE15 (Bitfield-Mask: 0x01)                      */
#define SMU_SFRSTEN_HWSFRSTE16_Pos        (16U)                    /*!< HWSFRSTE16 (Bit 16)                                   */
#define SMU_SFRSTEN_HWSFRSTE16_Msk        (0x10000U)               /*!< HWSFRSTE16 (Bitfield-Mask: 0x01)                      */
#define SMU_SFRSTEN_HWSFRSTE18_Pos        (18U)                    /*!< HWSFRSTE18 (Bit 18)                                   */
#define SMU_SFRSTEN_HWSFRSTE18_Msk        (0x40000U)               /*!< HWSFRSTE18 (Bitfield-Mask: 0x01)                      */
#define SMU_SFRSTEN_HWSFRSTE19_Pos        (19U)                    /*!< HWSFRSTE19 (Bit 19)                                   */
#define SMU_SFRSTEN_HWSFRSTE19_Msk        (0x80000U)               /*!< HWSFRSTE19 (Bitfield-Mask: 0x01)                      */
#define SMU_SFRSTEN_HWSFRSTE20_Pos        (20U)                    /*!< HWSFRSTE20 (Bit 20)                                   */
#define SMU_SFRSTEN_HWSFRSTE20_Msk        (0x100000U)              /*!< HWSFRSTE20 (Bitfield-Mask: 0x01)                      */
#define SMU_SFRSTEN_HWSFRSTE21_Pos        (21U)                    /*!< HWSFRSTE21 (Bit 21)                                   */
#define SMU_SFRSTEN_HWSFRSTE21_Msk        (0x200000U)              /*!< HWSFRSTE21 (Bitfield-Mask: 0x01)                      */
#define SMU_SFRSTEN_HWSFRSTE22_Pos        (22U)                    /*!< HWSFRSTE22 (Bit 22)                                   */
#define SMU_SFRSTEN_HWSFRSTE22_Msk        (0x400000U)              /*!< HWSFRSTE22 (Bitfield-Mask: 0x01)                      */
#define SMU_SFRSTEN_HWSFRSTE23_Pos        (23U)                    /*!< HWSFRSTE23 (Bit 23)                                   */
#define SMU_SFRSTEN_HWSFRSTE23_Msk        (0x800000U)              /*!< HWSFRSTE23 (Bitfield-Mask: 0x01)                      */
#define SMU_SFRSTEN_HWSFRSTE24_Pos        (24U)                    /*!< HWSFRSTE24 (Bit 24)                                   */
#define SMU_SFRSTEN_HWSFRSTE24_Msk        (0x1000000U)             /*!< HWSFRSTE24 (Bitfield-Mask: 0x01)                      */
#define SMU_SFRSTEN_HWSFRSTE25_Pos        (25U)                    /*!< HWSFRSTE25 (Bit 25)                                   */
#define SMU_SFRSTEN_HWSFRSTE25_Msk        (0x2000000U)             /*!< HWSFRSTE25 (Bitfield-Mask: 0x01)                      */
#define SMU_SFRSTEN_SWSFRSTE_Pos          (26U)                    /*!< SWSFRSTE (Bit 26)                                     */
#define SMU_SFRSTEN_SWSFRSTE_Msk          (0x4000000U)             /*!< SWSFRSTE (Bitfield-Mask: 0x01)                        */
/* ========================================================  LFRSTEN  ======================================================== */
#define SMU_LFRSTEN_HWLFRSTE_Pos          (0U)                     /*!< HWLFRSTE (Bit 0)                                      */
#define SMU_LFRSTEN_HWLFRSTE_Msk          (0x1fU)                  /*!< HWLFRSTE (Bitfield-Mask: 0x1f)                        */
#define SMU_LFRSTEN_HWLFRSTE0_Pos         (0U)                     /*!< HWLFRSTE0 (Bit 0)                                     */
#define SMU_LFRSTEN_HWLFRSTE0_Msk         (0x1U)                   /*!< HWLFRSTE0 (Bitfield-Mask: 0x01)                       */
#define SMU_LFRSTEN_HWLFRSTE1_Pos         (1U)                     /*!< HWLFRSTE1 (Bit 1)                                     */
#define SMU_LFRSTEN_HWLFRSTE1_Msk         (0x2U)                   /*!< HWLFRSTE1 (Bitfield-Mask: 0x01)                       */
#define SMU_LFRSTEN_HWLFRSTE2_Pos         (2U)                     /*!< HWLFRSTE2 (Bit 2)                                     */
#define SMU_LFRSTEN_HWLFRSTE2_Msk         (0x4U)                   /*!< HWLFRSTE2 (Bitfield-Mask: 0x01)                       */
#define SMU_LFRSTEN_HWLFRSTE3_Pos         (3U)                     /*!< HWLFRSTE3 (Bit 3)                                     */
#define SMU_LFRSTEN_HWLFRSTE3_Msk         (0x8U)                   /*!< HWLFRSTE3 (Bitfield-Mask: 0x01)                       */
#define SMU_LFRSTEN_HWLFRSTE4_Pos         (4U)                     /*!< HWLFRSTE4 (Bit 4)                                     */
#define SMU_LFRSTEN_HWLFRSTE4_Msk         (0x10U)                  /*!< HWLFRSTE4 (Bitfield-Mask: 0x01)                       */
#define SMU_LFRSTEN_SWLFRSTE_Pos          (5U)                     /*!< SWLFRSTE (Bit 5)                                      */
#define SMU_LFRSTEN_SWLFRSTE_Msk          (0x20U)                  /*!< SWLFRSTE (Bitfield-Mask: 0x01)                        */
/* ======================================================  SRSTCNTVAL  ======================================================= */
#define SMU_SRSTCNTVAL_SRSTCNTVAL_Pos     (0U)                     /*!< SRSTCNTVAL (Bit 0)                                    */
#define SMU_SRSTCNTVAL_SRSTCNTVAL_Msk     (0xfU)                   /*!< SRSTCNTVAL (Bitfield-Mask: 0x0f)                      */
/* ========================================================  SRSTCNT  ======================================================== */
#define SMU_SRSTCNT_SRSTCNT_Pos           (0U)                     /*!< SRSTCNT (Bit 0)                                       */
#define SMU_SRSTCNT_SRSTCNT_Msk           (0xfU)                   /*!< SRSTCNT (Bitfield-Mask: 0x0f)                         */
/* =======================================================  SRSTCNTS  ======================================================== */
#define SMU_SRSTCNTS_SRSTCNTS_Pos         (0U)                     /*!< SRSTCNTS (Bit 0)                                      */
#define SMU_SRSTCNTS_SRSTCNTS_Msk         (0xfU)                   /*!< SRSTCNTS (Bitfield-Mask: 0x0f)                        */
/* =======================================================  PATHCHK0  ======================================================== */
#define SMU_PATHCHK0_SEL_Pos              (0U)                     /*!< SEL (Bit 0)                                           */
#define SMU_PATHCHK0_SEL_Msk              (0x3ffffffU)             /*!< SEL (Bitfield-Mask: 0x3ffffff)                        */
#define SMU_PATHCHK0_SEL0_Pos             (0U)                     /*!< SEL0 (Bit 0)                                          */
#define SMU_PATHCHK0_SEL0_Msk             (0x1U)                   /*!< SEL0 (Bitfield-Mask: 0x01)                            */
#define SMU_PATHCHK0_SEL1_Pos             (1U)                     /*!< SEL1 (Bit 1)                                          */
#define SMU_PATHCHK0_SEL1_Msk             (0x2U)                   /*!< SEL1 (Bitfield-Mask: 0x01)                            */
#define SMU_PATHCHK0_SEL2_Pos             (2U)                     /*!< SEL2 (Bit 2)                                          */
#define SMU_PATHCHK0_SEL2_Msk             (0x4U)                   /*!< SEL2 (Bitfield-Mask: 0x01)                            */
#define SMU_PATHCHK0_SEL3_Pos             (3U)                     /*!< SEL3 (Bit 3)                                          */
#define SMU_PATHCHK0_SEL3_Msk             (0x8U)                   /*!< SEL3 (Bitfield-Mask: 0x01)                            */
#define SMU_PATHCHK0_SEL4_Pos             (4U)                     /*!< SEL4 (Bit 4)                                          */
#define SMU_PATHCHK0_SEL4_Msk             (0x10U)                  /*!< SEL4 (Bitfield-Mask: 0x01)                            */
#define SMU_PATHCHK0_SEL5_Pos             (5U)                     /*!< SEL5 (Bit 5)                                          */
#define SMU_PATHCHK0_SEL5_Msk             (0x20U)                  /*!< SEL5 (Bitfield-Mask: 0x01)                            */
#define SMU_PATHCHK0_SEL6_Pos             (6U)                     /*!< SEL6 (Bit 6)                                          */
#define SMU_PATHCHK0_SEL6_Msk             (0x40U)                  /*!< SEL6 (Bitfield-Mask: 0x01)                            */
#define SMU_PATHCHK0_SEL7_Pos             (7U)                     /*!< SEL7 (Bit 7)                                          */
#define SMU_PATHCHK0_SEL7_Msk             (0x80U)                  /*!< SEL7 (Bitfield-Mask: 0x01)                            */
#define SMU_PATHCHK0_SEL8_Pos             (8U)                     /*!< SEL8 (Bit 8)                                          */
#define SMU_PATHCHK0_SEL8_Msk             (0x100U)                 /*!< SEL8 (Bitfield-Mask: 0x01)                            */
#define SMU_PATHCHK0_SEL9_Pos             (9U)                     /*!< SEL9 (Bit 9)                                          */
#define SMU_PATHCHK0_SEL9_Msk             (0x200U)                 /*!< SEL9 (Bitfield-Mask: 0x01)                            */
#define SMU_PATHCHK0_SEL10_Pos            (10U)                    /*!< SEL10 (Bit 10)                                        */
#define SMU_PATHCHK0_SEL10_Msk            (0x400U)                 /*!< SEL10 (Bitfield-Mask: 0x01)                           */
#define SMU_PATHCHK0_SEL11_Pos            (11U)                    /*!< SEL11 (Bit 11)                                        */
#define SMU_PATHCHK0_SEL11_Msk            (0x800U)                 /*!< SEL11 (Bitfield-Mask: 0x01)                           */
#define SMU_PATHCHK0_SEL12_Pos            (12U)                    /*!< SEL12 (Bit 12)                                        */
#define SMU_PATHCHK0_SEL12_Msk            (0x1000U)                /*!< SEL12 (Bitfield-Mask: 0x01)                           */
#define SMU_PATHCHK0_SEL13_Pos            (13U)                    /*!< SEL13 (Bit 13)                                        */
#define SMU_PATHCHK0_SEL13_Msk            (0x2000U)                /*!< SEL13 (Bitfield-Mask: 0x01)                           */
#define SMU_PATHCHK0_SEL14_Pos            (14U)                    /*!< SEL14 (Bit 14)                                        */
#define SMU_PATHCHK0_SEL14_Msk            (0x4000U)                /*!< SEL14 (Bitfield-Mask: 0x01)                           */
#define SMU_PATHCHK0_SEL15_Pos            (15U)                    /*!< SEL15 (Bit 15)                                        */
#define SMU_PATHCHK0_SEL15_Msk            (0x8000U)                /*!< SEL15 (Bitfield-Mask: 0x01)                           */
#define SMU_PATHCHK0_SEL16_Pos            (16U)                    /*!< SEL16 (Bit 16)                                        */
#define SMU_PATHCHK0_SEL16_Msk            (0x10000U)               /*!< SEL16 (Bitfield-Mask: 0x01)                           */
#define SMU_PATHCHK0_SEL18_Pos            (18U)                    /*!< SEL18 (Bit 18)                                        */
#define SMU_PATHCHK0_SEL18_Msk            (0x40000U)               /*!< SEL18 (Bitfield-Mask: 0x01)                           */
#define SMU_PATHCHK0_SEL19_Pos            (19U)                    /*!< SEL19 (Bit 19)                                        */
#define SMU_PATHCHK0_SEL19_Msk            (0x80000U)               /*!< SEL19 (Bitfield-Mask: 0x01)                           */
#define SMU_PATHCHK0_SEL20_Pos            (20U)                    /*!< SEL20 (Bit 20)                                        */
#define SMU_PATHCHK0_SEL20_Msk            (0x100000U)              /*!< SEL20 (Bitfield-Mask: 0x01)                           */
#define SMU_PATHCHK0_SEL21_Pos            (21U)                    /*!< SEL21 (Bit 21)                                        */
#define SMU_PATHCHK0_SEL21_Msk            (0x200000U)              /*!< SEL21 (Bitfield-Mask: 0x01)                           */
#define SMU_PATHCHK0_SEL22_Pos            (22U)                    /*!< SEL22 (Bit 22)                                        */
#define SMU_PATHCHK0_SEL22_Msk            (0x400000U)              /*!< SEL22 (Bitfield-Mask: 0x01)                           */
#define SMU_PATHCHK0_SEL23_Pos            (23U)                    /*!< SEL23 (Bit 23)                                        */
#define SMU_PATHCHK0_SEL23_Msk            (0x800000U)              /*!< SEL23 (Bitfield-Mask: 0x01)                           */
#define SMU_PATHCHK0_SEL24_Pos            (24U)                    /*!< SEL24 (Bit 24)                                        */
#define SMU_PATHCHK0_SEL24_Msk            (0x1000000U)             /*!< SEL24 (Bitfield-Mask: 0x01)                           */
#define SMU_PATHCHK0_SEL25_Pos            (25U)                    /*!< SEL25 (Bit 25)                                        */
#define SMU_PATHCHK0_SEL25_Msk            (0x2000000U)             /*!< SEL25 (Bitfield-Mask: 0x01)                           */
#define SMU_PATHCHK0_EN_Pos               (28U)                    /*!< EN (Bit 28)                                           */
#define SMU_PATHCHK0_EN_Msk               (0xf0000000U)            /*!< EN (Bitfield-Mask: 0x0f)                              */
/* =======================================================  PATHCHK1  ======================================================== */
#define SMU_PATHCHK1_SEL_Pos              (0U)                     /*!< SEL (Bit 0)                                           */
#define SMU_PATHCHK1_SEL_Msk              (0x1fU)                  /*!< SEL (Bitfield-Mask: 0x1f)                             */
#define SMU_PATHCHK1_SEL0_Pos             (0U)                     /*!< SEL0 (Bit 0)                                          */
#define SMU_PATHCHK1_SEL0_Msk             (0x1U)                   /*!< SEL0 (Bitfield-Mask: 0x01)                            */
#define SMU_PATHCHK1_SEL1_Pos             (1U)                     /*!< SEL1 (Bit 1)                                          */
#define SMU_PATHCHK1_SEL1_Msk             (0x2U)                   /*!< SEL1 (Bitfield-Mask: 0x01)                            */
#define SMU_PATHCHK1_SEL2_Pos             (2U)                     /*!< SEL2 (Bit 2)                                          */
#define SMU_PATHCHK1_SEL2_Msk             (0x4U)                   /*!< SEL2 (Bitfield-Mask: 0x01)                            */
#define SMU_PATHCHK1_SEL3_Pos             (3U)                     /*!< SEL3 (Bit 3)                                          */
#define SMU_PATHCHK1_SEL3_Msk             (0x8U)                   /*!< SEL3 (Bitfield-Mask: 0x01)                            */
#define SMU_PATHCHK1_SEL4_Pos             (4U)                     /*!< SEL4 (Bit 4)                                          */
#define SMU_PATHCHK1_SEL4_Msk             (0x10U)                  /*!< SEL4 (Bitfield-Mask: 0x01)                            */
#define SMU_PATHCHK1_EN_Pos               (28U)                    /*!< EN (Bit 28)                                           */
#define SMU_PATHCHK1_EN_Msk               (0xf0000000U)            /*!< EN (Bitfield-Mask: 0x0f)                              */
/* =========================================================  FSPCR  ========================================================= */
#define SMU_FSPCR_FSPEN_Pos               (0U)                     /*!< FSPEN (Bit 0)                                         */
#define SMU_FSPCR_FSPEN_Msk               (0x1U)                   /*!< FSPEN (Bitfield-Mask: 0x01)                           */
#define SMU_FSPCR_FSPOUT0EN_Pos           (1U)                     /*!< FSPOUT0EN (Bit 1)                                     */
#define SMU_FSPCR_FSPOUT0EN_Msk           (0x2U)                   /*!< FSPOUT0EN (Bitfield-Mask: 0x01)                       */
#define SMU_FSPCR_FSPOUT1EN_Pos           (2U)                     /*!< FSPOUT1EN (Bit 2)                                     */
#define SMU_FSPCR_FSPOUT1EN_Msk           (0x4U)                   /*!< FSPOUT1EN (Bitfield-Mask: 0x01)                       */
#define SMU_FSPCR_FSPMODE_Pos             (3U)                     /*!< FSPMODE (Bit 3)                                       */
#define SMU_FSPCR_FSPMODE_Msk             (0x8U)                   /*!< FSPMODE (Bitfield-Mask: 0x01)                         */
#define SMU_FSPCR_FSPRELEASE_Pos          (4U)                     /*!< FSPRELEASE (Bit 4)                                    */
#define SMU_FSPCR_FSPRELEASE_Msk          (0x10U)                  /*!< FSPRELEASE (Bitfield-Mask: 0x01)                      */
#define SMU_FSPCR_FSPSTATE_Pos            (8U)                     /*!< FSPSTATE (Bit 8)                                      */
#define SMU_FSPCR_FSPSTATE_Msk            (0x100U)                 /*!< FSPSTATE (Bitfield-Mask: 0x01)                        */
/* ========================================================  FSPSFCR  ======================================================== */
#define SMU_FSPSFCR_FSPHWSFE_Pos          (0U)                     /*!< FSPHWSFE (Bit 0)                                      */
#define SMU_FSPSFCR_FSPHWSFE_Msk          (0x3ffffffU)             /*!< FSPHWSFE (Bitfield-Mask: 0x3ffffff)                   */
#define SMU_FSPSFCR_FSPHWSFE0_Pos         (0U)                     /*!< FSPHWSFE0 (Bit 0)                                     */
#define SMU_FSPSFCR_FSPHWSFE0_Msk         (0x1U)                   /*!< FSPHWSFE0 (Bitfield-Mask: 0x01)                       */
#define SMU_FSPSFCR_FSPHWSFE1_Pos         (1U)                     /*!< FSPHWSFE1 (Bit 1)                                     */
#define SMU_FSPSFCR_FSPHWSFE1_Msk         (0x2U)                   /*!< FSPHWSFE1 (Bitfield-Mask: 0x01)                       */
#define SMU_FSPSFCR_FSPHWSFE2_Pos         (2U)                     /*!< FSPHWSFE2 (Bit 2)                                     */
#define SMU_FSPSFCR_FSPHWSFE2_Msk         (0x4U)                   /*!< FSPHWSFE2 (Bitfield-Mask: 0x01)                       */
#define SMU_FSPSFCR_FSPHWSFE3_Pos         (3U)                     /*!< FSPHWSFE3 (Bit 3)                                     */
#define SMU_FSPSFCR_FSPHWSFE3_Msk         (0x8U)                   /*!< FSPHWSFE3 (Bitfield-Mask: 0x01)                       */
#define SMU_FSPSFCR_FSPHWSFE4_Pos         (4U)                     /*!< FSPHWSFE4 (Bit 4)                                     */
#define SMU_FSPSFCR_FSPHWSFE4_Msk         (0x10U)                  /*!< FSPHWSFE4 (Bitfield-Mask: 0x01)                       */
#define SMU_FSPSFCR_FSPHWSFE5_Pos         (5U)                     /*!< FSPHWSFE5 (Bit 5)                                     */
#define SMU_FSPSFCR_FSPHWSFE5_Msk         (0x20U)                  /*!< FSPHWSFE5 (Bitfield-Mask: 0x01)                       */
#define SMU_FSPSFCR_FSPHWSFE6_Pos         (6U)                     /*!< FSPHWSFE6 (Bit 6)                                     */
#define SMU_FSPSFCR_FSPHWSFE6_Msk         (0x40U)                  /*!< FSPHWSFE6 (Bitfield-Mask: 0x01)                       */
#define SMU_FSPSFCR_FSPHWSFE7_Pos         (7U)                     /*!< FSPHWSFE7 (Bit 7)                                     */
#define SMU_FSPSFCR_FSPHWSFE7_Msk         (0x80U)                  /*!< FSPHWSFE7 (Bitfield-Mask: 0x01)                       */
#define SMU_FSPSFCR_FSPHWSFE8_Pos         (8U)                     /*!< FSPHWSFE8 (Bit 8)                                     */
#define SMU_FSPSFCR_FSPHWSFE8_Msk         (0x100U)                 /*!< FSPHWSFE8 (Bitfield-Mask: 0x01)                       */
#define SMU_FSPSFCR_FSPHWSFE9_Pos         (9U)                     /*!< FSPHWSFE9 (Bit 9)                                     */
#define SMU_FSPSFCR_FSPHWSFE9_Msk         (0x200U)                 /*!< FSPHWSFE9 (Bitfield-Mask: 0x01)                       */
#define SMU_FSPSFCR_FSPHWSFE10_Pos        (10U)                    /*!< FSPHWSFE10 (Bit 10)                                   */
#define SMU_FSPSFCR_FSPHWSFE10_Msk        (0x400U)                 /*!< FSPHWSFE10 (Bitfield-Mask: 0x01)                      */
#define SMU_FSPSFCR_FSPHWSFE11_Pos        (11U)                    /*!< FSPHWSFE11 (Bit 11)                                   */
#define SMU_FSPSFCR_FSPHWSFE11_Msk        (0x800U)                 /*!< FSPHWSFE11 (Bitfield-Mask: 0x01)                      */
#define SMU_FSPSFCR_FSPHWSFE12_Pos        (12U)                    /*!< FSPHWSFE12 (Bit 12)                                   */
#define SMU_FSPSFCR_FSPHWSFE12_Msk        (0x1000U)                /*!< FSPHWSFE12 (Bitfield-Mask: 0x01)                      */
#define SMU_FSPSFCR_FSPHWSFE13_Pos        (13U)                    /*!< FSPHWSFE13 (Bit 13)                                   */
#define SMU_FSPSFCR_FSPHWSFE13_Msk        (0x2000U)                /*!< FSPHWSFE13 (Bitfield-Mask: 0x01)                      */
#define SMU_FSPSFCR_FSPHWSFE14_Pos        (14U)                    /*!< FSPHWSFE14 (Bit 14)                                   */
#define SMU_FSPSFCR_FSPHWSFE14_Msk        (0x4000U)                /*!< FSPHWSFE14 (Bitfield-Mask: 0x01)                      */
#define SMU_FSPSFCR_FSPHWSFE15_Pos        (15U)                    /*!< FSPHWSFE15 (Bit 15)                                   */
#define SMU_FSPSFCR_FSPHWSFE15_Msk        (0x8000U)                /*!< FSPHWSFE15 (Bitfield-Mask: 0x01)                      */
#define SMU_FSPSFCR_FSPHWSFE16_Pos        (16U)                    /*!< FSPHWSFE16 (Bit 16)                                   */
#define SMU_FSPSFCR_FSPHWSFE16_Msk        (0x10000U)               /*!< FSPHWSFE16 (Bitfield-Mask: 0x01)                      */
#define SMU_FSPSFCR_FSPHWSFE17_Pos        (17U)                    /*!< FSPHWSFE17 (Bit 17)                                   */
#define SMU_FSPSFCR_FSPHWSFE17_Msk        (0x20000U)               /*!< FSPHWSFE17 (Bitfield-Mask: 0x01)                      */
#define SMU_FSPSFCR_FSPHWSFE18_Pos        (18U)                    /*!< FSPHWSFE18 (Bit 18)                                   */
#define SMU_FSPSFCR_FSPHWSFE18_Msk        (0x40000U)               /*!< FSPHWSFE18 (Bitfield-Mask: 0x01)                      */
#define SMU_FSPSFCR_FSPHWSFE19_Pos        (19U)                    /*!< FSPHWSFE19 (Bit 19)                                   */
#define SMU_FSPSFCR_FSPHWSFE19_Msk        (0x80000U)               /*!< FSPHWSFE19 (Bitfield-Mask: 0x01)                      */
#define SMU_FSPSFCR_FSPHWSFE20_Pos        (20U)                    /*!< FSPHWSFE20 (Bit 20)                                   */
#define SMU_FSPSFCR_FSPHWSFE20_Msk        (0x100000U)              /*!< FSPHWSFE20 (Bitfield-Mask: 0x01)                      */
#define SMU_FSPSFCR_FSPHWSFE21_Pos        (21U)                    /*!< FSPHWSFE21 (Bit 21)                                   */
#define SMU_FSPSFCR_FSPHWSFE21_Msk        (0x200000U)              /*!< FSPHWSFE21 (Bitfield-Mask: 0x01)                      */
#define SMU_FSPSFCR_FSPHWSFE22_Pos        (22U)                    /*!< FSPHWSFE22 (Bit 22)                                   */
#define SMU_FSPSFCR_FSPHWSFE22_Msk        (0x400000U)              /*!< FSPHWSFE22 (Bitfield-Mask: 0x01)                      */
#define SMU_FSPSFCR_FSPHWSFE23_Pos        (23U)                    /*!< FSPHWSFE23 (Bit 23)                                   */
#define SMU_FSPSFCR_FSPHWSFE23_Msk        (0x800000U)              /*!< FSPHWSFE23 (Bitfield-Mask: 0x01)                      */
#define SMU_FSPSFCR_FSPHWSFE24_Pos        (24U)                    /*!< FSPHWSFE24 (Bit 24)                                   */
#define SMU_FSPSFCR_FSPHWSFE24_Msk        (0x1000000U)             /*!< FSPHWSFE24 (Bitfield-Mask: 0x01)                      */
#define SMU_FSPSFCR_FSPHWSFE25_Pos        (25U)                    /*!< FSPHWSFE25 (Bit 25)                                   */
#define SMU_FSPSFCR_FSPHWSFE25_Msk        (0x2000000U)             /*!< FSPHWSFE25 (Bitfield-Mask: 0x01)                      */
#define SMU_FSPSFCR_FSPSWSFE_Pos          (26U)                    /*!< FSPSWSFE (Bit 26)                                     */
#define SMU_FSPSFCR_FSPSWSFE_Msk          (0x4000000U)             /*!< FSPSWSFE (Bitfield-Mask: 0x01)                        */
/* ========================================================  FSPLFCR  ======================================================== */
#define SMU_FSPLFCR_FSPHWLFE_Pos          (0U)                     /*!< FSPHWLFE (Bit 0)                                      */
#define SMU_FSPLFCR_FSPHWLFE_Msk          (0x1fU)                  /*!< FSPHWLFE (Bitfield-Mask: 0x1f)                        */
#define SMU_FSPLFCR_FSPHWLFE0_Pos         (0U)                     /*!< FSPHWLFE0 (Bit 0)                                     */
#define SMU_FSPLFCR_FSPHWLFE0_Msk         (0x1U)                   /*!< FSPHWLFE0 (Bitfield-Mask: 0x01)                       */
#define SMU_FSPLFCR_FSPHWLFE1_Pos         (1U)                     /*!< FSPHWLFE1 (Bit 1)                                     */
#define SMU_FSPLFCR_FSPHWLFE1_Msk         (0x2U)                   /*!< FSPHWLFE1 (Bitfield-Mask: 0x01)                       */
#define SMU_FSPLFCR_FSPHWLFE2_Pos         (2U)                     /*!< FSPHWLFE2 (Bit 2)                                     */
#define SMU_FSPLFCR_FSPHWLFE2_Msk         (0x4U)                   /*!< FSPHWLFE2 (Bitfield-Mask: 0x01)                       */
#define SMU_FSPLFCR_FSPHWLFE3_Pos         (3U)                     /*!< FSPHWLFE3 (Bit 3)                                     */
#define SMU_FSPLFCR_FSPHWLFE3_Msk         (0x8U)                   /*!< FSPHWLFE3 (Bitfield-Mask: 0x01)                       */
#define SMU_FSPLFCR_FSPHWLFE4_Pos         (4U)                     /*!< FSPHWLFE4 (Bit 4)                                     */
#define SMU_FSPLFCR_FSPHWLFE4_Msk         (0x10U)                  /*!< FSPHWLFE4 (Bitfield-Mask: 0x01)                       */
#define SMU_FSPLFCR_FSPSWLFE_Pos          (5U)                     /*!< FSPSWLFE (Bit 5)                                      */
#define SMU_FSPLFCR_FSPSWLFE_Msk          (0x20U)                  /*!< FSPSWLFE (Bitfield-Mask: 0x01)                        */

/* =========================================================================================================================== */
/* ================                                           CMU                                             ================ */
/* =========================================================================================================================== */

/* ==========================================================  CR  =========================================================== */
#define CMU_CR_MONE_Pos                   (0U)                     /*!< MONE (Bit 0)                                          */
#define CMU_CR_MONE_Msk                   (0x7U)                   /*!< MONE (Bitfield-Mask: 0x07)                            */
/* =========================================================  RCCR  ========================================================== */
#define CMU_RCCR_WINLEN_Pos               (0U)                     /*!< WINLEN (Bit 0)                                        */
#define CMU_RCCR_WINLEN_Msk               (0xffffU)                /*!< WINLEN (Bitfield-Mask: 0xffff)                        */
/* =========================================================  HTCR  ========================================================== */
#define CMU_HTCR_HIGHTHR_Pos              (0U)                     /*!< HIGHTHR (Bit 0)                                       */
#define CMU_HTCR_HIGHTHR_Msk              (0xffffffU)              /*!< HIGHTHR (Bitfield-Mask: 0xffffff)                     */
/* =========================================================  LTCR  ========================================================== */
#define CMU_LTCR_LOWTHR_Pos               (0U)                     /*!< LOWTHR (Bit 0)                                        */
#define CMU_LTCR_LOWTHR_Msk               (0xffffffU)              /*!< LOWTHR (Bitfield-Mask: 0xffffff)                      */
/* ==========================================================  SR  =========================================================== */
#define CMU_SR_FLL_Pos                    (0U)                     /*!< FLL (Bit 0)                                           */
#define CMU_SR_FLL_Msk                    (0x1U)                   /*!< FLL (Bitfield-Mask: 0x01)                             */
#define CMU_SR_FHH_Pos                    (1U)                     /*!< FHH (Bit 1)                                           */
#define CMU_SR_FHH_Msk                    (0x2U)                   /*!< FHH (Bitfield-Mask: 0x01)                             */
#define CMU_SR_CLKLOSS_Pos                (2U)                     /*!< CLKLOSS (Bit 2)                                       */
#define CMU_SR_CLKLOSS_Msk                (0x4U)                   /*!< CLKLOSS (Bitfield-Mask: 0x01)                         */
#define CMU_SR_CHECKDONE_Pos              (3U)                     /*!< CHECKDONE (Bit 3)                                     */
#define CMU_SR_CHECKDONE_Msk              (0x8U)                   /*!< CHECKDONE (Bitfield-Mask: 0x01)                       */
#define CMU_SR_WORK_Pos                   (4U)                     /*!< WORK (Bit 4)                                          */
#define CMU_SR_WORK_Msk                   (0x10U)                  /*!< WORK (Bitfield-Mask: 0x01)                            */
/* ========================================================  REFCNT  ========================================================= */
#define CMU_REFCNT_WINLEN_Pos             (0U)                     /*!< WINLEN (Bit 0)                                        */
#define CMU_REFCNT_WINLEN_Msk             (0xffffU)                /*!< WINLEN (Bitfield-Mask: 0xffff)                        */
/* ========================================================  MONCNT  ========================================================= */
#define CMU_MONCNT_moncnt_Pos             (0U)                     /*!< moncnt (Bit 0)                                        */
#define CMU_MONCNT_moncnt_Msk             (0xffffffU)              /*!< moncnt (Bitfield-Mask: 0xffffff)                      */

/* =========================================================================================================================== */
/* ================                                            HSM                                            ================ */
/* =========================================================================================================================== */

/* =========================================================  UID0  ========================================================== */
#define HSM_UID0_UID0_Pos                 (0U)                     /*!< UID0 (Bit 0)                                          */
#define HSM_UID0_UID0_Msk                 (0xffffffffU)            /*!< UID0 (Bitfield-Mask: 0xffffffff)                      */
/* =========================================================  UID1  ========================================================== */
#define HSM_UID1_UID1_Pos                 (0U)                     /*!< UID1 (Bit 0)                                          */
#define HSM_UID1_UID1_Msk                 (0xffffffffU)            /*!< UID1 (Bitfield-Mask: 0xffffffff)                      */
/* =========================================================  UID2  ========================================================== */
#define HSM_UID2_UID2_Pos                 (0U)                     /*!< UID2 (Bit 0)                                          */
#define HSM_UID2_UID2_Msk                 (0xffffffffU)            /*!< UID2 (Bitfield-Mask: 0xffffffff)                      */
/* =========================================================  UID3  ========================================================== */
#define HSM_UID3_UID3_Pos                 (0U)                     /*!< UID3 (Bit 0)                                          */
#define HSM_UID3_UID3_Msk                 (0xffffffffU)            /*!< UID3 (Bitfield-Mask: 0xffffffff)                      */
/* ==========================================================  CMD  ========================================================== */
#define HSM_CMD_TXLEN_Pos                 (0U)                     /*!< TXLEN (Bit 0)                                         */
#define HSM_CMD_TXLEN_Msk                 (0xfffffU)               /*!< TXLEN (Bitfield-Mask: 0xfffff)                        */
#define HSM_CMD_AES256EN_Pos              (20U)                    /*!< AES256EN (Bit 20)                                     */
#define HSM_CMD_AES256EN_Msk              (0x100000U)              /*!< AES256EN (Bitfield-Mask: 0x01)                        */
#define HSM_CMD_SM4EN_Pos                 (21U)                    /*!< SM4EN (Bit 21)                                        */
#define HSM_CMD_SM4EN_Msk                 (0x200000U)              /*!< SM4EN (Bitfield-Mask: 0x01)                           */
#define HSM_CMD_KEYID_Pos                 (22U)                    /*!< KEYID (Bit 22)                                        */
#define HSM_CMD_KEYID_Msk                 (0x7c00000U)             /*!< KEYID (Bitfield-Mask: 0x1f)                           */
#define HSM_CMD_DEC_Pos                   (27U)                    /*!< DEC (Bit 27)                                          */
#define HSM_CMD_DEC_Msk                   (0x8000000U)             /*!< DEC (Bitfield-Mask: 0x01)                             */
#define HSM_CMD_CIPMODE_Pos               (28U)                    /*!< CIPMODE (Bit 28)                                      */
#define HSM_CMD_CIPMODE_Msk               (0xf0000000U)            /*!< CIPMODE (Bitfield-Mask: 0x0f)                         */
/* ==========================================================  CST  ========================================================== */
#define HSM_CST_START_Pos                 (0U)                     /*!< START (Bit 0)                                         */
#define HSM_CST_START_Msk                 (0x1U)                   /*!< START (Bitfield-Mask: 0x01)                           */
/* =======================================================  SYS_CTRL  ======================================================== */
#define HSM_SYS_CTRL_DMAEN_Pos            (0U)                     /*!< DMAEN (Bit 0)                                         */
#define HSM_SYS_CTRL_DMAEN_Msk            (0x1U)                   /*!< DMAEN (Bitfield-Mask: 0x01)                           */
#define HSM_SYS_CTRL_IRQEN_Pos            (1U)                     /*!< IRQEN (Bit 1)                                         */
#define HSM_SYS_CTRL_IRQEN_Msk            (0x2U)                   /*!< IRQEN (Bitfield-Mask: 0x01)                           */
/* =======================================================  SYS_STAT  ======================================================== */
#define HSM_SYS_STAT_CMDDONE_Pos          (0U)                     /*!< CMDDONE (Bit 0)                                       */
#define HSM_SYS_STAT_CMDDONE_Msk          (0x3U)                   /*!< CMDDONE (Bitfield-Mask: 0x03)                         */
#define HSM_SYS_STAT_IDLE_Pos             (2U)                     /*!< IDLE (Bit 2)                                          */
#define HSM_SYS_STAT_IDLE_Msk             (0x4U)                   /*!< IDLE (Bitfield-Mask: 0x01)                            */
#define HSM_SYS_STAT_CRU_Pos              (3U)                     /*!< CRU (Bit 3)                                           */
#define HSM_SYS_STAT_CRU_Msk              (0x8U)                   /*!< CRU (Bitfield-Mask: 0x01)                             */
#define HSM_SYS_STAT_COG_Pos              (4U)                     /*!< COG (Bit 4)                                           */
#define HSM_SYS_STAT_COG_Msk              (0x10U)                  /*!< COG (Bitfield-Mask: 0x01)                             */
#define HSM_SYS_STAT_RF_Pos               (5U)                     /*!< RF (Bit 5)                                            */
#define HSM_SYS_STAT_RF_Msk               (0x20U)                  /*!< RF (Bitfield-Mask: 0x01)                              */
#define HSM_SYS_STAT_RE_Pos               (6U)                     /*!< RE (Bit 6)                                            */
#define HSM_SYS_STAT_RE_Msk               (0x40U)                  /*!< RE (Bitfield-Mask: 0x01)                              */
#define HSM_SYS_STAT_TF_Pos               (7U)                     /*!< TF (Bit 7)                                            */
#define HSM_SYS_STAT_TF_Msk               (0x80U)                  /*!< TF (Bitfield-Mask: 0x01)                              */
#define HSM_SYS_STAT_TE_Pos               (8U)                     /*!< TE (Bit 8)                                            */
#define HSM_SYS_STAT_TE_Msk               (0x100U)                 /*!< TE (Bitfield-Mask: 0x01)                              */
/* =======================================================  VRY_STAT  ======================================================== */
#define HSM_VRY_STAT_BVD_Pos              (0U)                     /*!< BVD (Bit 0)                                           */
#define HSM_VRY_STAT_BVD_Msk              (0x1U)                   /*!< BVD (Bitfield-Mask: 0x01)                             */
#define HSM_VRY_STAT_BVP_Pos              (1U)                     /*!< BVP (Bit 1)                                           */
#define HSM_VRY_STAT_BVP_Msk              (0x2U)                   /*!< BVP (Bitfield-Mask: 0x01)                             */
/* =====================================================  VRY_STAT_CLR  ====================================================== */
#define HSM_VRY_STAT_CLR_VSC_Pos          (0U)                     /*!< VSC (Bit 0)                                           */
#define HSM_VRY_STAT_CLR_VSC_Msk          (0x1U)                   /*!< VSC (Bitfield-Mask: 0x01)                             */
/* =======================================================  AUTH_CTRL  ======================================================= */
#define HSM_AUTH_CTRL_ERV_Pos             (0U)                     /*!< ERV (Bit 0)                                           */
#define HSM_AUTH_CTRL_ERV_Msk             (0x1U)                   /*!< ERV (Bitfield-Mask: 0x01)                             */
/* =======================================================  AUTH_STAT  ======================================================= */
#define HSM_AUTH_STAT_IAD_Pos             (0U)                     /*!< IAD (Bit 0)                                           */
#define HSM_AUTH_STAT_IAD_Msk             (0x1U)                   /*!< IAD (Bitfield-Mask: 0x01)                             */
#define HSM_AUTH_STAT_IAT_Pos             (1U)                     /*!< IAT (Bit 1)                                           */
#define HSM_AUTH_STAT_IAT_Msk             (0x2U)                   /*!< IAT (Bitfield-Mask: 0x01)                             */
#define HSM_AUTH_STAT_CAP_Pos             (2U)                     /*!< CAP (Bit 2)                                           */
#define HSM_AUTH_STAT_CAP_Msk             (0x4U)                   /*!< CAP (Bitfield-Mask: 0x01)                             */
#define HSM_AUTH_STAT_BAP_Pos             (3U)                     /*!< BAP (Bit 3)                                           */
#define HSM_AUTH_STAT_BAP_Msk             (0x8U)                   /*!< BAP (Bitfield-Mask: 0x01)                             */
#define HSM_AUTH_STAT_SFAP_Pos            (4U)                     /*!< SFAP (Bit 4)                                          */
#define HSM_AUTH_STAT_SFAP_Msk            (0x10U)                  /*!< SFAP (Bitfield-Mask: 0x01)                            */
#define HSM_AUTH_STAT_SRAP_Pos            (5U)                     /*!< SRAP (Bit 5)                                          */
#define HSM_AUTH_STAT_SRAP_Msk            (0x20U)                  /*!< SRAP (Bitfield-Mask: 0x01)                            */
#define HSM_AUTH_STAT_SUAP_Pos            (6U)                     /*!< SUAP (Bit 6)                                          */
#define HSM_AUTH_STAT_SUAP_Msk            (0x40U)                  /*!< SUAP (Bitfield-Mask: 0x01)                            */
#define HSM_AUTH_STAT_AUAP_Pos            (7U)                     /*!< AUAP (Bit 7)                                          */
#define HSM_AUTH_STAT_AUAP_Msk            (0x80U)                  /*!< AUAP (Bitfield-Mask: 0x01)                            */
#define HSM_AUTH_STAT_SFUAP_Pos           (8U)                     /*!< SFUAP (Bit 8)                                         */
#define HSM_AUTH_STAT_SFUAP_Msk           (0x100U)                 /*!< SFUAP (Bitfield-Mask: 0x01)                           */
#define HSM_AUTH_STAT_DAD_Pos             (9U)                     /*!< DAD (Bit 9)                                           */
#define HSM_AUTH_STAT_DAD_Msk             (0x200U)                 /*!< DAD (Bitfield-Mask: 0x01)                             */
#define HSM_AUTH_STAT_DAF_Pos             (10U)                    /*!< DAF (Bit 10)                                          */
#define HSM_AUTH_STAT_DAF_Msk             (0x400U)                 /*!< DAF (Bitfield-Mask: 0x01)                             */
#define HSM_AUTH_STAT_DAP_Pos             (11U)                    /*!< DAP (Bit 11)                                          */
#define HSM_AUTH_STAT_DAP_Msk             (0x800U)                 /*!< DAP (Bitfield-Mask: 0x01)                             */
#define HSM_AUTH_STAT_ACV_Pos             (16U)                    /*!< ACV (Bit 16)                                          */
#define HSM_AUTH_STAT_ACV_Msk             (0x10000U)               /*!< ACV (Bitfield-Mask: 0x01)                             */
/* =====================================================  AUTH_STAT_CLR  ===================================================== */
#define HSM_AUTH_STAT_CLR_IAD_Pos         (0U)                     /*!< IAD (Bit 0)                                           */
#define HSM_AUTH_STAT_CLR_IAD_Msk         (0x1U)                   /*!< IAD (Bitfield-Mask: 0x01)                             */
#define HSM_AUTH_STAT_CLR_IAT_Pos         (1U)                     /*!< IAT (Bit 1)                                           */
#define HSM_AUTH_STAT_CLR_IAT_Msk         (0x2U)                   /*!< IAT (Bitfield-Mask: 0x01)                             */
#define HSM_AUTH_STAT_CLR_CAP_Pos         (2U)                     /*!< CAP (Bit 2)                                           */
#define HSM_AUTH_STAT_CLR_CAP_Msk         (0x4U)                   /*!< CAP (Bitfield-Mask: 0x01)                             */
#define HSM_AUTH_STAT_CLR_BAP_Pos         (3U)                     /*!< BAP (Bit 3)                                           */
#define HSM_AUTH_STAT_CLR_BAP_Msk         (0x8U)                   /*!< BAP (Bitfield-Mask: 0x01)                             */
#define HSM_AUTH_STAT_CLR_SFAP_Pos        (4U)                     /*!< SFAP (Bit 4)                                          */
#define HSM_AUTH_STAT_CLR_SFAP_Msk        (0x10U)                  /*!< SFAP (Bitfield-Mask: 0x01)                            */
#define HSM_AUTH_STAT_CLR_SRAP_Pos        (5U)                     /*!< SRAP (Bit 5)                                          */
#define HSM_AUTH_STAT_CLR_SRAP_Msk        (0x20U)                  /*!< SRAP (Bitfield-Mask: 0x01)                            */
#define HSM_AUTH_STAT_CLR_SUAP_Pos        (6U)                     /*!< SUAP (Bit 6)                                          */
#define HSM_AUTH_STAT_CLR_SUAP_Msk        (0x40U)                  /*!< SUAP (Bitfield-Mask: 0x01)                            */
#define HSM_AUTH_STAT_CLR_AUAP_Pos        (7U)                     /*!< AUAP (Bit 7)                                          */
#define HSM_AUTH_STAT_CLR_AUAP_Msk        (0x80U)                  /*!< AUAP (Bitfield-Mask: 0x01)                            */
#define HSM_AUTH_STAT_CLR_SFUAP_Pos       (8U)                     /*!< SFUAP (Bit 8)                                         */
#define HSM_AUTH_STAT_CLR_SFUAP_Msk       (0x100U)                 /*!< SFUAP (Bitfield-Mask: 0x01)                           */
/* =========================================================  CIPIN  ========================================================= */
#define HSM_CIPIN_IN_Pos                  (0U)                     /*!< IN (Bit 0)                                            */
#define HSM_CIPIN_IN_Msk                  (0xffffffffU)            /*!< IN (Bitfield-Mask: 0xffffffff)                        */
/* ========================================================  CIPOUT  ========================================================= */
#define HSM_CIPOUT_OUT_Pos                (0U)                     /*!< OUT (Bit 0)                                           */
#define HSM_CIPOUT_OUT_Msk                (0xffffffffU)            /*!< OUT (Bitfield-Mask: 0xffffffff)                       */
/* ==========================================================  IV0  ========================================================== */
#define HSM_IV0_IV_Pos                    (0U)                     /*!< IV (Bit 0)                                            */
#define HSM_IV0_IV_Msk                    (0xffffffffU)            /*!< IV (Bitfield-Mask: 0xffffffff)                        */
/* ==========================================================  IV1  ========================================================== */
#define HSM_IV1_IV_Pos                    (0U)                     /*!< IV (Bit 0)                                            */
#define HSM_IV1_IV_Msk                    (0xffffffffU)            /*!< IV (Bitfield-Mask: 0xffffffff)                        */
/* ==========================================================  IV2  ========================================================== */
#define HSM_IV2_IV_Pos                    (0U)                     /*!< IV (Bit 0)                                            */
#define HSM_IV2_IV_Msk                    (0xffffffffU)            /*!< IV (Bitfield-Mask: 0xffffffff)                        */
/* ==========================================================  IV3  ========================================================== */
#define HSM_IV3_IV_Pos                    (0U)                     /*!< IV (Bit 0)                                            */
#define HSM_IV3_IV_Msk                    (0xffffffffU)            /*!< IV (Bitfield-Mask: 0xffffffff)                        */
/* =======================================================  RAM_KEY0  ======================================================== */
#define HSM_RAM_KEY0_KEY_Pos              (0U)                     /*!< KEY (Bit 0)                                           */
#define HSM_RAM_KEY0_KEY_Msk              (0xffffffffU)            /*!< KEY (Bitfield-Mask: 0xffffffff)                       */
/* =======================================================  RAM_KEY1  ======================================================== */
#define HSM_RAM_KEY1_KEY_Pos              (0U)                     /*!< KEY (Bit 0)                                           */
#define HSM_RAM_KEY1_KEY_Msk              (0xffffffffU)            /*!< KEY (Bitfield-Mask: 0xffffffff)                       */
/* =======================================================  RAM_KEY2  ======================================================== */
#define HSM_RAM_KEY2_KEY_Pos              (0U)                     /*!< KEY (Bit 0)                                           */
#define HSM_RAM_KEY2_KEY_Msk              (0xffffffffU)            /*!< KEY (Bitfield-Mask: 0xffffffff)                       */
/* =======================================================  RAM_KEY3  ======================================================== */
#define HSM_RAM_KEY3_KEY_Pos              (0U)                     /*!< KEY (Bit 0)                                           */
#define HSM_RAM_KEY3_KEY_Msk              (0xffffffffU)            /*!< KEY (Bitfield-Mask: 0xffffffff)                       */
/* =======================================================  RAM_KEY4  ======================================================== */
#define HSM_RAM_KEY4_KEY_Pos              (0U)                     /*!< KEY (Bit 0)                                           */
#define HSM_RAM_KEY4_KEY_Msk              (0xffffffffU)            /*!< KEY (Bitfield-Mask: 0xffffffff)                       */
/* =======================================================  RAM_KEY5  ======================================================== */
#define HSM_RAM_KEY5_KEY_Pos              (0U)                     /*!< KEY (Bit 0)                                           */
#define HSM_RAM_KEY5_KEY_Msk              (0xffffffffU)            /*!< KEY (Bitfield-Mask: 0xffffffff)                       */
/* =======================================================  RAM_KEY6  ======================================================== */
#define HSM_RAM_KEY6_KEY_Pos              (0U)                     /*!< KEY (Bit 0)                                           */
#define HSM_RAM_KEY6_KEY_Msk              (0xffffffffU)            /*!< KEY (Bitfield-Mask: 0xffffffff)                       */
/* =======================================================  RAM_KEY7  ======================================================== */
#define HSM_RAM_KEY7_KEY_Pos              (0U)                     /*!< KEY (Bit 0)                                           */
#define HSM_RAM_KEY7_KEY_Msk              (0xffffffffU)            /*!< KEY (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  CHA0  ========================================================== */
#define HSM_CHA0_CHALLENGE_Pos            (0U)                     /*!< CHALLENGE (Bit 0)                                     */
#define HSM_CHA0_CHALLENGE_Msk            (0xffffffffU)            /*!< CHALLENGE (Bitfield-Mask: 0xffffffff)                 */
/* =========================================================  CHA1  ========================================================== */
#define HSM_CHA1_CHALLENGE_Pos            (0U)                     /*!< CHALLENGE (Bit 0)                                     */
#define HSM_CHA1_CHALLENGE_Msk            (0xffffffffU)            /*!< CHALLENGE (Bitfield-Mask: 0xffffffff)                 */
/* =========================================================  CHA2  ========================================================== */
#define HSM_CHA2_CHALLENGE_Pos            (0U)                     /*!< CHALLENGE (Bit 0)                                     */
#define HSM_CHA2_CHALLENGE_Msk            (0xffffffffU)            /*!< CHALLENGE (Bitfield-Mask: 0xffffffff)                 */
/* =========================================================  CHA3  ========================================================== */
#define HSM_CHA3_CHALLENGE_Pos            (0U)                     /*!< CHALLENGE (Bit 0)                                     */
#define HSM_CHA3_CHALLENGE_Msk            (0xffffffffU)            /*!< CHALLENGE (Bitfield-Mask: 0xffffffff)                 */
/* =========================================================  RSP0  ========================================================== */
#define HSM_RSP0_RESPONSE_Pos             (0U)                     /*!< RESPONSE (Bit 0)                                      */
#define HSM_RSP0_RESPONSE_Msk             (0xffffffffU)            /*!< RESPONSE (Bitfield-Mask: 0xffffffff)                  */
/* =========================================================  RSP1  ========================================================== */
#define HSM_RSP1_RESPONSE_Pos             (0U)                     /*!< RESPONSE (Bit 0)                                      */
#define HSM_RSP1_RESPONSE_Msk             (0xffffffffU)            /*!< RESPONSE (Bitfield-Mask: 0xffffffff)                  */
/* =========================================================  RSP2  ========================================================== */
#define HSM_RSP2_RESPONSE_Pos             (0U)                     /*!< RESPONSE (Bit 0)                                      */
#define HSM_RSP2_RESPONSE_Msk             (0xffffffffU)            /*!< RESPONSE (Bitfield-Mask: 0xffffffff)                  */
/* =========================================================  RSP3  ========================================================== */
#define HSM_RSP3_RESPONSE_Pos             (0U)                     /*!< RESPONSE (Bit 0)                                      */
#define HSM_RSP3_RESPONSE_Msk             (0xffffffffU)            /*!< RESPONSE (Bitfield-Mask: 0xffffffff)                  */
/* =========================================================  MAC0  ========================================================== */
#define HSM_MAC0_MAC_Pos                  (0U)                     /*!< MAC (Bit 0)                                           */
#define HSM_MAC0_MAC_Msk                  (0xffffffffU)            /*!< MAC (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  MAC1  ========================================================== */
#define HSM_MAC1_MAC_Pos                  (0U)                     /*!< MAC (Bit 0)                                           */
#define HSM_MAC1_MAC_Msk                  (0xffffffffU)            /*!< MAC (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  MAC2  ========================================================== */
#define HSM_MAC2_MAC_Pos                  (0U)                     /*!< MAC (Bit 0)                                           */
#define HSM_MAC2_MAC_Msk                  (0xffffffffU)            /*!< MAC (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  MAC3  ========================================================== */
#define HSM_MAC3_MAC_Pos                  (0U)                     /*!< MAC (Bit 0)                                           */
#define HSM_MAC3_MAC_Msk                  (0xffffffffU)            /*!< MAC (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  KAC0  ========================================================== */
#define HSM_KAC0_KAC_Pos                  (0U)                     /*!< KAC (Bit 0)                                           */
#define HSM_KAC0_KAC_Msk                  (0xffffffffU)            /*!< KAC (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  KAC1  ========================================================== */
#define HSM_KAC1_KAC_Pos                  (0U)                     /*!< KAC (Bit 0)                                           */
#define HSM_KAC1_KAC_Msk                  (0xffffffffU)            /*!< KAC (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  KAC2  ========================================================== */
#define HSM_KAC2_KAC_Pos                  (0U)                     /*!< KAC (Bit 0)                                           */
#define HSM_KAC2_KAC_Msk                  (0xffffffffU)            /*!< KAC (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  KAC3  ========================================================== */
#define HSM_KAC3_KAC_Pos                  (0U)                     /*!< KAC (Bit 0)                                           */
#define HSM_KAC3_KAC_Msk                  (0xffffffffU)            /*!< KAC (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  KAC4  ========================================================== */
#define HSM_KAC4_KAC_Pos                  (0U)                     /*!< KAC (Bit 0)                                           */
#define HSM_KAC4_KAC_Msk                  (0xffffffffU)            /*!< KAC (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  KAC5  ========================================================== */
#define HSM_KAC5_KAC_Pos                  (0U)                     /*!< KAC (Bit 0)                                           */
#define HSM_KAC5_KAC_Msk                  (0xffffffffU)            /*!< KAC (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  KAC6  ========================================================== */
#define HSM_KAC6_KAC_Pos                  (0U)                     /*!< KAC (Bit 0)                                           */
#define HSM_KAC6_KAC_Msk                  (0xffffffffU)            /*!< KAC (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  KAC7  ========================================================== */
#define HSM_KAC7_KAC_Pos                  (0U)                     /*!< KAC (Bit 0)                                           */
#define HSM_KAC7_KAC_Msk                  (0xffffffffU)            /*!< KAC (Bitfield-Mask: 0xffffffff)                       */
/* =========================================================  STAT  ========================================================== */
#define HSM_STAT_S0_Pos                   (0U)                     /*!< S0 (Bit 0)                                            */
#define HSM_STAT_S0_Msk                   (0x1U)                   /*!< S0 (Bitfield-Mask: 0x01)                              */
#define HSM_STAT_S1_Pos                   (1U)                     /*!< S1 (Bit 1)                                            */
#define HSM_STAT_S1_Msk                   (0x2U)                   /*!< S1 (Bitfield-Mask: 0x01)                              */
#define HSM_STAT_S2_Pos                   (2U)                     /*!< S2 (Bit 2)                                            */
#define HSM_STAT_S2_Msk                   (0x4U)                   /*!< S2 (Bitfield-Mask: 0x01)                              */
#define HSM_STAT_S3_Pos                   (3U)                     /*!< S3 (Bit 3)                                            */
#define HSM_STAT_S3_Msk                   (0x8U)                   /*!< S3 (Bitfield-Mask: 0x01)                              */
#define HSM_STAT_S4_Pos                   (4U)                     /*!< S4 (Bit 4)                                            */
#define HSM_STAT_S4_Msk                   (0x10U)                  /*!< S4 (Bitfield-Mask: 0x01)                              */
#define HSM_STAT_S5_Pos                   (5U)                     /*!< S5 (Bit 5)                                            */
#define HSM_STAT_S5_Msk                   (0x20U)                  /*!< S5 (Bitfield-Mask: 0x01)                              */
#define HSM_STAT_S6_Pos                   (6U)                     /*!< S6 (Bit 6)                                            */
#define HSM_STAT_S6_Msk                   (0x40U)                  /*!< S6 (Bitfield-Mask: 0x01)                              */
#define HSM_STAT_S7_Pos                   (7U)                     /*!< S7 (Bit 7)                                            */
#define HSM_STAT_S7_Msk                   (0x80U)                  /*!< S7 (Bitfield-Mask: 0x01)                              */
#define HSM_STAT_S8_Pos                   (8U)                     /*!< S8 (Bit 8)                                            */
#define HSM_STAT_S8_Msk                   (0x100U)                 /*!< S8 (Bitfield-Mask: 0x01)                              */
#define HSM_STAT_S9_Pos                   (9U)                     /*!< S9 (Bit 9)                                            */
#define HSM_STAT_S9_Msk                   (0x200U)                 /*!< S9 (Bitfield-Mask: 0x01)                              */
#define HSM_STAT_S10_Pos                  (10U)                    /*!< S10 (Bit 10)                                          */
#define HSM_STAT_S10_Msk                  (0x400U)                 /*!< S10 (Bitfield-Mask: 0x01)                             */
#define HSM_STAT_S11_Pos                  (11U)                    /*!< S11 (Bit 11)                                          */
#define HSM_STAT_S11_Msk                  (0x800U)                 /*!< S11 (Bitfield-Mask: 0x01)                             */
#define HSM_STAT_S12_Pos                  (12U)                    /*!< S12 (Bit 12)                                          */
#define HSM_STAT_S12_Msk                  (0x1000U)                /*!< S12 (Bitfield-Mask: 0x01)                             */
#define HSM_STAT_S13_Pos                  (13U)                    /*!< S13 (Bit 13)                                          */
#define HSM_STAT_S13_Msk                  (0x2000U)                /*!< S13 (Bitfield-Mask: 0x01)                             */
#define HSM_STAT_S14_Pos                  (14U)                    /*!< S14 (Bit 14)                                          */
#define HSM_STAT_S14_Msk                  (0x4000U)                /*!< S14 (Bitfield-Mask: 0x01)                             */
#define HSM_STAT_S15_Pos                  (15U)                    /*!< S15 (Bit 15)                                          */
#define HSM_STAT_S15_Msk                  (0x8000U)                /*!< S15 (Bitfield-Mask: 0x01)                             */
#define HSM_STAT_S16_Pos                  (16U)                    /*!< S16 (Bit 16)                                          */
#define HSM_STAT_S16_Msk                  (0x10000U)               /*!< S16 (Bitfield-Mask: 0x01)                             */
#define HSM_STAT_S17_Pos                  (17U)                    /*!< S17 (Bit 17)                                          */
#define HSM_STAT_S17_Msk                  (0x20000U)               /*!< S17 (Bitfield-Mask: 0x01)                             */
#define HSM_STAT_S18_Pos                  (18U)                    /*!< S18 (Bit 18)                                          */
#define HSM_STAT_S18_Msk                  (0x40000U)               /*!< S18 (Bitfield-Mask: 0x01)                             */
#define HSM_STAT_S19_Pos                  (19U)                    /*!< S19 (Bit 19)                                          */
#define HSM_STAT_S19_Msk                  (0x80000U)               /*!< S19 (Bitfield-Mask: 0x01)                             */
#define HSM_STAT_S20_Pos                  (20U)                    /*!< S20 (Bit 20)                                          */
#define HSM_STAT_S20_Msk                  (0x100000U)              /*!< S20 (Bitfield-Mask: 0x01)                             */
/* =======================================================  STAT_CLR  ======================================================== */
#define HSM_STAT_CLR_CLR_Pos              (0U)                     /*!< CLR (Bit 0)                                           */
#define HSM_STAT_CLR_CLR_Msk              (0x1U)                   /*!< CLR (Bitfield-Mask: 0x01)                             */
/* =======================================================  CHIP_STAT  ======================================================= */
#define HSM_CHIP_STAT_STAT_Pos            (0U)                     /*!< STAT (Bit 0)                                          */
#define HSM_CHIP_STAT_STAT_Msk            (0x1U)                   /*!< STAT (Bitfield-Mask: 0x01)                            */

/** @} */ /* End of group PosMask_peripherals */

#ifdef __cplusplus
}
#endif

#endif /* AC7840E_H */
/** @} */ /* End of group AC7840e */

/** @} */ /* End of group AutoChips */
