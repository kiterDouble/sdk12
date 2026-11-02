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
 * @file Compiler_Cfg.h
 *
 * @brief the file used for add module specific memory classes and pointer classes.
 *
 */

#ifndef COMPILER_CFG_H
#define COMPILER_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/
/**
@{
* @brief ADC memory and pointer classes.
*/
#define ADC_PUBLIC_CODE
#define ADC_PRIVATE_CODE
#define ADC_CONST
#define ADC_APPL_DATA
#define ADC_APPL_CONST
#define ADC_APPL_CODE
#define ADC_CALLOUT_CODE
#define ADC_VAR_NOINIT
#define ADC_VAR_POWER_ON_INIT
#define ADC_VAR_FAST
#define ADC_VAR
/**@}*/

/**
@{
* @brief CAN memory and pointer classes.
*/
#define CAN_PUBLIC_CODE
#define CAN_PRIVATE_CODE
#define CAN_CONST
#define CAN_APPL_DATA
#define CAN_APPL_CONST
#define CAN_APPL_CODE
#define CAN_CALLOUT_CODE
#define CAN_VAR_NOINIT
#define CAN_VAR_POWER_ON_INIT
#define CAN_VAR_FAST
#define CAN_VAR
/**@}*/

/**
@{
* @brief CANIF memory and pointer classes.
*/
#define CANIF_PUBLIC_CODE
#define CANIF_PRIVATE_CODE
#define CANIF_CONST
#define CANIF_APPL_DATA
#define CANIF_APPL_CONST
#define CANIF_APPL_CODE
#define CANIF_CALLOUT_CODE
#define CANIF_VAR_NOINIT
#define CANIF_VAR_POWER_ON_INIT
#define CANIF_VAR_FAST
#define CANIF_VAR
/**@}*/

/**
@{
* @brief CORTST memory and pointer classes.
*/
#define CORTST_PUBLIC_CODE
#define CORTST_PRIVATE_CODE
#define CORTST_CONST
#define CORTST_APPL_DATA
#define CORTST_APPL_CONST
#define CORTST_APPL_CODE
#define CORTST_CALLOUT_CODE
#define CORTST_VAR_NOINIT
#define CORTST_VAR_POWER_ON_INIT
#define CORTST_VAR_FAST
#define CORTST_VAR
/**@}*/

/**
@{
* @brief CRYIF memory and pointer classes.
*/
#define CRYIF_PUBLIC_CODE
#define CRYIF_PRIVATE_CODE
#define CRYIF_CONST
#define CRYIF_APPL_DATA
#define CRYIF_APPL_CONST
#define CRYIF_APPL_CODE
#define CRYIF_CALLOUT_CODE
#define CRYIF_VAR_NOINIT
#define CRYIF_VAR_POWER_ON_INIT
#define CRYIF_VAR_FAST
#define CRYIF_VAR
/**@}*/

/**
@{
* @brief CRYPTO memory and pointer classes.
*/
#define CRYPTO_PUBLIC_CODE
#define CRYPTO_PRIVATE_CODE
#define CRYPTO_CONST
#define CRYPTO_APPL_DATA
#define CRYPTO_APPL_CONST
#define CRYPTO_APPL_CODE
#define CRYPTO_CALLOUT_CODE
#define CRYPTO_VAR_NOINIT
#define CRYPTO_VAR_POWER_ON_INIT
#define CRYPTO_VAR_FAST
#define CRYPTO_VAR
/**@}*/

/**
@{
* @brief CSM memory and pointer classes.
*/
#define CSM_PUBLIC_CODE
#define CSM_PRIVATE_CODE
#define CSM_CONST
#define CSM_APPL_DATA
#define CSM_APPL_CONST
#define CSM_APPL_CODE
#define CSM_CALLOUT_CODE
#define CSM_VAR_NOINIT
#define CSM_VAR_POWER_ON_INIT
#define CSM_VAR_FAST
#define CSM_VAR
/**@}*/

/**
@{
* @brief CRC memory and pointer classes.
*/
#define CRC_PUBLIC_CODE
#define CRC_PRIVATE_CODE
#define CRC_CONST
#define CRC_APPL_DATA
#define CRC_APPL_CONST
#define CRC_APPL_CODE
#define CRC_CALLOUT_CODE
#define CRC_VAR_NOINIT
#define CRC_VAR_POWER_ON_INIT
#define CRC_VAR_FAST
#define CRC_VAR
/**@}*/

/**
@{
* @brief DMA memory and pointer classes.
*/
#define DMA_PUBLIC_CODE
#define DMA_PRIVATE_CODE
#define DMA_CONST
#define DMA_APPL_DATA
#define DMA_APPL_CONST
#define DMA_APPL_CODE
#define DMA_CALLOUT_CODE
#define DMA_VAR_NOINIT
#define DMA_VAR_POWER_ON_INIT
#define DMA_VAR_FAST
#define DMA_VAR
/**@}*/

/**
@{
* @brief DEM memory and pointer classes.
*/
#define DEM_PUBLIC_CODE
#define DEM_PRIVATE_CODE
#define DEM_CONST
#define DEM_APPL_DATA
#define DEM_APPL_CONST
#define DEM_APPL_CODE
#define DEM_CALLOUT_CODE
#define DEM_VAR_NOINIT
#define DEM_VAR_POWER_ON_INIT
#define DEM_VAR_FAST
#define DEM_VAR
/**@}*/

/**
@{
* @brief DET memory and pointer classes.
*/
#define DET_PUBLIC_CODE
#define DET_PRIVATE_CODE
#define DET_CONST
#define DET_APPL_DATA
#define DET_APPL_CONST
#define DET_APPL_CODE
#define DET_CALLOUT_CODE
#define DET_VAR_NOINIT
#define DET_VAR_POWER_ON_INIT
#define DET_VAR_FAST
#define DET_VAR
/**@}*/

/**
@{
* @brief DIO memory and pointer classes.
*/
#define DIO_PUBLIC_CODE
#define DIO_PRIVATE_CODE
#define DIO_CONST
#define DIO_APPL_DATA
#define DIO_APPL_CONST
#define DIO_APPL_CODE
#define DIO_CALLOUT_CODE
#define DIO_VAR_NOINIT
#define DIO_VAR_POWER_ON_INIT
#define DIO_VAR_FAST
#define DIO_VAR
/**@}*/

/**
@{
* @brief ECUM memory and pointer classes.
*/
#define ECUM_PUBLIC_CODE
#define ECUM_PRIVATE_CODE
#define ECUM_CONST
#define ECUM_APPL_DATA
#define ECUM_APPL_CONST
#define ECUM_APPL_CODE
#define ECUM_CALLOUT_CODE
#define ECUM_VAR_NOINIT
#define ECUM_VAR_POWER_ON_INIT
#define ECUM_VAR_FAST
#define ECUM_VAR
/**@}*/

/**
@{
* @brief EEP memory and pointer classes.
*/
#define EEP_PUBLIC_CODE
#define EEP_PRIVATE_CODE
#define EEP_CONST
#define EEP_APPL_DATA
#define EEP_APPL_CONST
#define EEP_APPL_CODE
#define EEP_CALLOUT_CODE
#define EEP_VAR_NOINIT
#define EEP_VAR_POWER_ON_INIT
#define EEP_VAR_FAST
#define EEP_VAR
/**@}*/

/**
@{
* @brief FEE memory and pointer classes.
*/
#define FEE_PUBLIC_CODE
#define FEE_PRIVATE_CODE
#define FEE_CONST
#define FEE_APPL_DATA
#define FEE_APPL_CONST
#define FEE_APPL_CODE
#define FEE_CALLOUT_CODE
#define FEE_VAR_NOINIT
#define FEE_VAR_POWER_ON_INIT
#define FEE_VAR_FAST
#define FEE_VAR
/**@}*/

/**
@{
* @brief FLS memory and pointer classes.
*/
#define FLS_PUBLIC_CODE
#define FLS_PRIVATE_CODE
#define FLS_CONST
#define FLS_APPL_DATA
#define FLS_APPL_CONST
#define FLS_APPL_CODE
#define FLS_CALLOUT_CODE
#define FLS_VAR_NOINIT
#define FLS_VAR_POWER_ON_INIT
#define FLS_VAR_FAST
#define FLS_VAR
/**@}*/

/**
@{
* @brief FLSTST memory and pointer classes.
*/
#define FLSTST_PUBLIC_CODE
#define FLSTST_PRIVATE_CODE
#define FLSTST_CONST
#define FLSTST_APPL_DATA
#define FLSTST_APPL_CONST
#define FLSTST_APPL_CODE
#define FLSTST_CALLOUT_CODE
#define FLSTST_VAR_NOINIT
#define FLSTST_VAR_POWER_ON_INIT
#define FLSTST_VAR_FAST
#define FLSTST_VAR
/**@}*/

/**
@{
* @brief GPT memory and pointer classes.
*/
#define GPT_PUBLIC_CODE
#define GPT_PRIVATE_CODE
#define GPT_CONST
#define GPT_APPL_DATA
#define GPT_APPL_CONST
#define GPT_APPL_CODE
#define GPT_CALLOUT_CODE
#define GPT_VAR_NOINIT
#define GPT_VAR_POWER_ON_INIT
#define GPT_VAR_FAST
#define GPT_VAR
/**@}*/

/**
@{
* @brief ICU memory and pointer classes.
*/
#define ICU_PUBLIC_CODE
#define ICU_PRIVATE_CODE
#define ICU_CONST
#define ICU_APPL_DATA
#define ICU_APPL_CONST
#define ICU_APPL_CODE
#define ICU_CALLOUT_CODE
#define ICU_VAR_NOINIT
#define ICU_VAR_POWER_ON_INIT
#define ICU_VAR_FAST
#define ICU_VAR
/**@}*/

/**
@{
* @brief I2C memory and pointer classes.
*/
#define I2C_PUBLIC_CODE
#define I2C_PRIVATE_CODE
#define I2C_CONST
#define I2C_APPL_DATA
#define I2C_APPL_CONST
#define I2C_APPL_CODE
#define I2C_CALLOUT_CODE
#define I2C_VAR_NOINIT
#define I2C_VAR_POWER_ON_INIT
#define I2C_VAR_FAST
#define I2C_VAR
/**@}*/

/**
@{
* @brief LIN memory and pointer classes.
*/
#define LIN_PUBLIC_CODE
#define LIN_PRIVATE_CODE
#define LIN_CONST
#define LIN_APPL_DATA
#define LIN_APPL_CONST
#define LIN_APPL_CODE
#define LIN_CALLOUT_CODE
#define LIN_VAR_NOINIT
#define LIN_VAR_POWER_ON_INIT
#define LIN_VAR_FAST
#define LIN_VAR
/**@}*/

/**
@{
* @brief LINIF memory and pointer classes.
*/
#define LINIF_PUBLIC_CODE
#define LINIF_PRIVATE_CODE
#define LINIF_CONST
#define LINIF_APPL_DATA
#define LINIF_APPL_CONST
#define LINIF_APPL_CODE
#define LINIF_CALLOUT_CODE
#define LINIF_VAR_NOINIT
#define LINIF_VAR_POWER_ON_INIT
#define LINIF_VAR_FAST
#define LINIF_VAR
/**@}*/

/**
@{
* @brief MCEM memory and pointer classes.
*/
#define MCEM_PUBLIC_CODE
#define MCEM_PRIVATE_CODE
#define MCEM_CONST
#define MCEM_APPL_DATA
#define MCEM_APPL_CONST
#define MCEM_APPL_CODE
#define MCEM_CALLOUT_CODE
#define MCEM_VAR_NOINIT
#define MCEM_VAR_POWER_ON_INIT
#define MCEM_VAR_FAST
#define MCEM_VAR
/**@}*/

/**
@{
* @brief MCL memory and pointer classes.
*/
#define MCL_PUBLIC_CODE
#define MCL_PRIVATE_CODE
#define MCL_CONST
#define MCL_APPL_DATA
#define MCL_APPL_CONST
#define MCL_APPL_CODE
#define MCL_CALLOUT_CODE
#define MCL_VAR_NOINIT
#define MCL_VAR_POWER_ON_INIT
#define MCL_VAR_FAST
#define MCL_VAR
/**@}*/

/**
@{
* @brief MCU memory and pointer classes.
*/
#define MCU_PUBLIC_CODE
#define MCU_PRIVATE_CODE
#define MCU_CONST
#define MCU_APPL_DATA
#define MCU_APPL_CONST
#define MCU_APPL_CODE
#define MCU_CALLOUT_CODE
#define MCU_VAR_NOINIT
#define MCU_VAR_POWER_ON_INIT
#define MCU_VAR_FAST
#define MCU_VAR
/**@}*/

/**
@{
* @brief OCU memory and pointer classes.
*/
#define OCU_PUBLIC_CODE
#define OCU_PRIVATE_CODE
#define OCU_CONST
#define OCU_APPL_DATA
#define OCU_APPL_CONST
#define OCU_APPL_CODE
#define OCU_CALLOUT_CODE
#define OCU_VAR_NOINIT
#define OCU_VAR_POWER_ON_INIT
#define OCU_VAR_FAST
#define OCU_VAR
/**@}*/

/**
@{
* @brief PORT memory and pointer classes.
*/
#define PORT_PUBLIC_CODE
#define PORT_PRIVATE_CODE
#define PORT_CONST
#define PORT_APPL_DATA
#define PORT_APPL_CONST
#define PORT_APPL_CODE
#define PORT_CALLOUT_CODE
#define PORT_VAR_NOINIT
#define PORT_VAR_POWER_ON_INIT
#define PORT_VAR_FAST
#define PORT_VAR
/**@}*/

/**
@{
* @brief PWM memory and pointer classes.
*/
#define PWM_PUBLIC_CODE
#define PWM_PRIVATE_CODE
#define PWM_CONST
#define PWM_APPL_DATA
#define PWM_APPL_CONST
#define PWM_APPL_CODE
#define PWM_CALLOUT_CODE
#define PWM_VAR_NOINIT
#define PWM_VAR_POWER_ON_INIT
#define PWM_VAR_FAST
#define PWM_VAR
/**@}*/

/**
@{
* @brief RamTST memory and pointer classes.
*/
#define RAMTST_PUBLIC_CODE
#define RAMTST_PRIVATE_CODE
#define RAMTST_CONST
#define RAMTST_APPL_DATA
#define RAMTST_APPL_CONST
#define RAMTST_APPL_CODE
#define RAMTST_CALLOUT_CODE
#define RAMTST_VAR_NOINIT
#define RAMTST_VAR_POWER_ON_INIT
#define RAMTST_VAR_FAST
#define RAMTST_VAR
/**@}*/

/**
@{
* @brief SchM memory and pointer classes.
*/
#define RTE_PUBLIC_CODE
#define RTE_PRIVATE_CODE
#define RTE_CONST
#define RTE_APPL_DATA
#define RTE_APPL_CONST
#define RTE_APPL_CODE
#define RTE_CALLOUT_CODE
#define RTE_VAR_NOINIT
#define RTE_VAR_POWER_ON_INIT
#define RTE_VAR_FAST
#define RTE_VAR
/**@}*/

/**
@{
* @brief SPI memory and pointer classes.
*/
#define SPI_PUBLIC_CODE
#define SPI_PRIVATE_CODE
#define SPI_CONST
#define SPI_APPL_DATA
#define SPI_APPL_CONST
#define SPI_APPL_CODE
#define SPI_CALLOUT_CODE
#define SPI_VAR_NOINIT
#define SPI_VAR_POWER_ON_INIT
#define SPI_VAR_FAST
#define SPI_VAR
/**@}*/

/**
@{
* @brief WDG memory and pointer classes.
*/
#define WDG_PUBLIC_CODE
#define WDG_PRIVATE_CODE
#define WDG_CONST
#define WDG_APPL_DATA
#define WDG_APPL_CONST
#define WDG_APPL_CODE
#define WDG_CALLOUT_CODE
#define WDG_VAR_NOINIT
#define WDG_VAR_POWER_ON_INIT
#define WDG_VAR_FAST
#define WDG_VAR
/**@}*/

/**
@{
* @brief WDGIF memory and pointer classes.
*/
#define WDGIF_PUBLIC_CODE
#define WDGIF_PRIVATE_CODE
#define WDGIF_CONST
#define WDGIF_APPL_DATA
#define WDGIF_APPL_CONST
#define WDGIF_APPL_CODE
#define WDGIF_CALLOUT_CODE
#define WDGIF_VAR_NOINIT
#define WDGIF_VAR_POWER_ON_INIT
#define WDGIF_VAR_FAST
#define WDGIF_VAR
/**@}*/

/**
* @brief Define for ComStack Data.
*/
#define AUTOSAR_COMSTACKDATA

/* eth */
#define ETH_VAR

/* ethif */
#define ETHIF_CODE
#define ETHIF_VAR

#define ETHIF_APPL_DATA

/* tcpip */
#define TCPIP_CODE
#define TCPIP_VAR

#define TCPIP_APPL_CONST
#define TCPIP_APPL_VAR
#define TCPIP_CONST

#define TCPIP_CONFIG_DATA

#define TCPIP_APPL_DATA
#define TCPIP_APPL_CODE
#define TCPIP_VAR_ZERO_INIT

/* csm */
#define CSM_APPL_DATA
#define CSM_CONST
#define CSM_CODE

/* keym */
#define KEYM_CONST_PBCFG
#define KEYM_APPL_CONST

#define KEYM_APPL_CODE
#define KEYM_CODE

#define KEYM_APPL_DATA
#define KEYM_CONST

/* det */
#define DET_CODE

/* ethsm */
#define ETHSM_CODE

/* StbM */
#define STBM_CODE
#define STBM_APPL_DATA
#define STBM_CONST_PBCFG
#define STBM_VAR_CLEARED_BOOLEAN
#define STBM_VAR
#define STBM_CONST

/* CanTSyn */
#define CANTSYN_CODE
#define CANTSYN_APPL_CONST
#define CANTSYN_VAR_NOINIT
#define CANTSYN_VAR_POWER_ON_INIT
#define CANTSYN_APPL_DATA
#define CANTSYN_CONST_PBCFG
#define CANTSYN_CONST

/* EthTSyn */
#define ETHTSYN_CODE
#define ETHTSYN_VAR
#define ETHTSYN_APPL_DATA
#define ETHTSYN_VAR_CLEARED_BOOLEAN

/* Tm */
#define TM_APPL_DATA
#define TM_APPL_CONST
#define TM_CODE

/* CanTrcv */
#define CANTRCV_CODE
#define CANTRCV_VAR_NOINIT
#define CANTRCV_CONST_PBCFG
#define CANTRCV_VAR_POWER_ON_INIT
#define CANTRCV_APPL_DATA

/* LinIf */
#define LINIF_CODE
#define LINIF_VAR
#define LINIF_APPL_CONST
#define LINIF_APPL_DATA
#define LINIF_CONST
#define LINIF_DATA

/* LinSM */
#define LINSM_CODE
#define LINSM_VAR_POWER_ON_INIT
#define LINSM_VAR
#define LINSM_CONST
#define LINSM_CONST_PBCFG
#define LinSM_APPL_DATA

/* SecOC */
#define SECOC_APPL_CONST
#define SECOC_APPL_DATA
#define SECOC_CFG_CONST
#define SECOC_CFG_VAR
#define SECOC_CODE
#define SECOC_CONST
#define SECOC_VAR_POWER_ON_INIT

/* Sd */
#define SD_CFG_CONST
#define SD_CFG_VAR
#define SD_CODE
#define SD_VAR
#define SD_CONST

/* SomeIpTp */
#define SOMEIPTP_APPL_CONST
#define SOMEIPTP_APPL_DATA
#define SOMEIPTP_CODE
#define SOMEIPTP_CONST
#define SOMEIPTP_VAR
#define SOMEIPTP_VAR_POWER_ON_INIT

/* EthSM */
#define ETHSM_APPL_DATA
#define ETHSM_CODE
#define ETHSM_CONST
#define ETHSM_VAR
#define ETHSM_VAR_POWER_ON_INIT

/* UdpNm */
#define UDPNM_APPL_CONST
#define UDPNM_APPL_DATA
#define UDPNM_CODE
#define UDPNM_VAR
#define UDPNM_VAR_POWER_ON_INIT

/* cantp */
#define CANTP_VAR_POWER_ON_INIT
#define CANTP_CODE
#define CANTP_CONST
#define CANTP_APPL_CONST
#define CANTP_APPL_DATA
#define CANTP_APPL_CODE
#define CANTP_VAR_NOINIT
#define CANTP_VAR

/* dcm */
#define DCM_VAR_POWER_ON_INIT
#define DCM_CODE
#define DCM_CONST_PBCFG
#define DCM_VAR
#define DCM_APPL_CODE
#define DCM_CONST
#define DCM_VAR_NOINIT
#define DCM_APPL_CONST
#define DCM_APPL_DATA

/* Dlt */
#define DLT_VAR_POWER_ON_INIT
#define DLT_CODE
#define DLT_CONST_PBCFG
#define DLT_VAR
#define DLT_APPL_CODE
#define DLT_CONST
#define DLT_VAR_NOINIT
#define DLT_APPL_CONST
#define DLT_APPL_DATA

/* Det */
#define DET_VAR_POWER_ON_INIT
#define DET_CODE
#define DET_CONST_PBCFG
#define DET_VAR
#define DET_APPL_CODE
#define DET_CONST
#define DET_VAR_NOINIT
#define DET_APPL_CONST
#define DET_APPL_DATA

/* Wdg */
#define WDG_VAR_POWER_ON_INIT
#define WDG_CODE
#define WDG_CONST_PBCFG
#define WDG_VAR
#define WDG_APPL_CODE
#define WDG_CONST
#define WDG_VAR_NOINIT
#define WDG_APPL_CONST
#define WDG_APPL_DATA

/* WdgIf */
#define WDGIF_VAR_POWER_ON_INIT
#define WDGIF_CODE
#define WDGIF_CONST_PBCFG
#define WDGIF_VAR
#define WDGIF_APPL_CODE
#define WDGIF_CONST
#define WDGIF_VAR_NOINIT
#define WDGIF_APPL_CONST
#define WDGIF_APPL_DATA

/* doip */
#define DOIP_VAR
#define DOIP_CODE
#define DOIP_VAR_CLEARED
#define DOIP_VAR_INIT
#define DOIP_APPL_CONST
#define DOIP_CONST
#define DOIP_APPL_CODE

/* nvm */
#define NVM_CODE
#define NVM_VAR
#define NVM_CONST
#define NVM_VAR_NOINIT
#define NVM_APPL_DATA
#define NVM_VAR_POWER_ON_INIT
#define NVM_APPL_CODE

/* MemIf */
#define MEMIF_CODE
#define MEMIF_VAR
#define MEMIF_CONST
#define MEMIF_VAR_NOINIT
#define MEMIF_APPL_DATA
#define MEMIF_VAR_POWER_ON_INIT
#define MEMIF_APPL_CODE

/* Fee */
#define FEE_CODE
#define FEE_VAR
#define FEE_CONST
#define FEE_VAR_NOINIT
#define FEE_APPL_DATA
#define FEE_VAR_POWER_ON_INIT
#define FEE_APPL_CODE

/* Fls */
#define FLS_CODE
#define FLS_VAR
#define FLS_CONST
#define FLS_VAR_NOINIT
#define FLS_APPL_DATA
#define FLS_VAR_POWER_ON_INIT
#define FLS_APPL_CODE

/* EA */
#define EA_CODE
#define EA_VAR
#define EA_CONST
#define EA_VAR_NOINIT
#define EA_APPL_DATA
#define EA_VAR_POWER_ON_INIT
#define EA_APPL_CODE

/* Eep */
#define EEP_CODE
#define EEP_VAR
#define EEP_CONST
#define EEP_VAR_NOINIT
#define EEP_APPL_DATA
#define EEP_VAR_POWER_ON_INIT
#define EEP_APPL_CODE

/* dem */
#define DEM_CODE
#define DEM_CONST
#define DEM_VAR
#define DEM_CONST_PBCFG
#define DEM_APPL_DATA

/* CDD_ASIC_DI */
#define CDD_ASIC_DI_CODE
#define CDD_ASIC_DI_CONST
#define CDD_ASIC_DI_VAR
#define CDD_ASIC_DI_CONST_PBCFG
#define CDD_ASIC_DI_APPL_DATA

/* fim */
#define FIM_VAR_INIT_8
#define FIM_APPL_DATA
#define FIM_VAR_NO_INIT_UNSPECIFIED
#define FIM_CODE

/* canif */
#define CANIF_VAR_POWER_ON_INIT
#define CANIF_CONST
#define CANIF_VAR
#define CANIF_CODE
#define CANIF_APPL_DATA
#define CANIF_CONST_PBCFG
#define CANIF_APPL_CONST
#define CANIF_APPL_CODE

/* com */
#define COM_VAR_POWER_ON_INIT
#define COM_VAR
#define COM_CODE
#define COM_APPL_CONST
#define COM_APPL_DATA
#define COM_CONST_PBCFG
#define COM_CONST
#define COM_APPL_CODE

/* comxf */
#define COMXF_CODE
#define COMXF_CONST
#define COMXF_VAR
#define COMXF_VAR_POWER_ON_INIT
#define COMXF_CONST_PBCFG

/* ipdum */
#define IPDUM_VAR_POWER_ON_INIT
#define IPDUM_CONST
#define IPDUM_VAR_NOINIT
#define IPDUM_CODE
#define IPDUM_APPL_DATA
#define IPDUM_APPL_CONST
#define IPDUM_RXINDICATION_CODE
#define IPDUM_TXCONFIRMATION_CODE
#define IPDUM_TRIGGERTRANSMIT_CODE

/* ldcom */
#define LDCOM_VAR_POWER_ON_INIT
#define LDCOM_CONST
#define LDCOM_APPL_DATA
#define LDCOM_CODE
#define LDCOM_COPYTXDATA_CODE
#define LDCOM_TPTXCONFIRMATION_CODE
#define LDCOM_STARTOFRECEPTION_CODE
#define LDCOM_COPYRXDATA_CODE
#define LDCOM_TPRXINDICATION_CODE
#define LDCOM_RXINDICATION_CODE
#define LDCOM_TXCONFIRMATION_CODE
#define LDCOM_TRIGGERTRANSMIT_CODE
#define LDCOM_APPL_CODE
#define LDCOM_CONST_PBCFG

/* pdur */
#define PDUR_VAR_POWER_ON_INIT
#define PDUR_VAR
#define PDUR_APPL_DATA
#define PDUR_CODE
#define PDUR_CONST
#define PDUR_CONST_PBCFG
#define PDUR_APPL_CODE

/* soad */
#define SOAD_APPL_DATA
#define SOAD_APPL_CONST
#define SOAD_VAR_POWER_ON_INIT
#define SOAD_VAR
#define SOAD_CODE
#define SOAD_CONST
#define SOAD_CALLBACK_CODE
#define SOAD_APPL_CODE
#define SOAD_CONST_PBCFG

/* comm */
#define COMM_VAR_POWER_ON_INIT
#define COMM_APPL_CONST
#define COMM_APPL_DATA
#define COMM_CODE
#define COMM_COM_CODE
#define COMM_DCM_CODE
#define COMM_ECUM_USED
#define COMM_CONST
#define COMM_VAR
#define COMM_NM_CODE
#define ComM_CONFIG_DATA

/*nm*/
#define NM_CODE
#define NM_APPL_DATA
#define NM_VAR_CLEARED_BOOLEAN
#define NM_VAR_NO_INIT_UNSPECIFIED

/*cannm*/
#define CANNM_APPL_CONST
#define CANNM_APPL_DATA
#define CANNM_CODE
#define CANNM_VAR
#define CANNM_VAR_POWER_ON_INIT

/*cansm*/
#define CANNM_VAR_POWER_ON_INIT
#define CANSM_CONFIG_DATA
#define CANSM_VAR
#define CANSM_CODE
#define CANSM_COMM_CODE
#define CANSM_ECUM_CODE
#define CANSM_CONTROLLERBUSOFF_CODE
#define CANSM_CONTROLLERNODEINDICATION_CODE
#define CANSM_TRANSCEIVERMODEINDICATION_CODE
#define CANSM_CLEARTRCVWUFFLAGINDICATION_CODE
#define CANSM_CHECKTRANSCEIVERWAKEFLAGINDICATION_CODE
#define CANSM_CONFIRMPNAVAILABILITY_CODE
#define CANSM_CURRENTICOMCONFIGURATION_CODE
#define CANSM_TXTIMEOUTEXCEPTION_CODE

/*EthTrcv*/
#define ETHTRCV_VAR_NOINIT
#define ETHTRCV_VAR_ZERO_INIT
#define ETHTRCV_CODE
#define ETHTRCV_CONFIG_DATA
#define ETHTRCV_CONST
#define ETHTRCV_APPL_DATA

/*crc*/
#define CRC_CODE
#define CRC_APPL_CONST

/*e2e*/
#define E2E_APPL_DATA
#define E2E_CODE

/*e2exf*/
#define E2EXF_CONST
#define E2EXF_CODE
#define E2EXF_VAR_POWER_ON_INIT
#define E2EXF_VAR

/* IoHwAbDrv */
#define IOHWABDRV_VAR_POWER_ON_INIT
#define IOHWABDRV_APPL_DATA
#define IOHWABDRV_CODE
#define IOHWABDRV_COM_CODE
#define IOHWABDRV_DCM_CODE
#define IOHWABDRV_ECUM_USED
#define IOHWABDRV_CONST
#define IOHWABDRV_VAR
#define IOHWABDRV_NM_CODE
#define IOHWABDRV_CONFIG_DATA

/* IoHwAb */
#define IOHWAB_VAR_POWER_ON_INIT
#define IOHWAB_APPL_DATA
#define IOHWAB_CODE
#define IOHWAB_COM_CODE
#define IOHWAB_DCM_CODE
#define IOHWAB_ECUM_USED
#define IOHWAB_CONST
#define IOHWAB_VAR
#define IOHWAB_NM_CODE
#define IOHWAB_CONFIG_DATA

/* Rte_IoHwAb */
#define RTEIOHWAB_VAR_POWER_ON_INIT
#define RTEIOHWAB_APPL_DATA
#define RTEIOHWAB_CODE
#define RTEIOHWAB_COM_CODE
#define RTEIOHWAB_DCM_CODE
#define RTEIOHWAB_ECUM_USED
#define RTEIOHWAB_CONST
#define RTEIOHWAB_VAR
#define RTEIOHWAB_NM_CODE
#define RTEIOHWAB_CONFIG_DATA

/* J193DCM */
#define J1939DCM_CONST_UNSPECIFIED
#define J1939DCM_CONST_BOOLEAN
#define J1939DCM_CONST_16
#define J1939DCM_VAR_NO_INIT_UNSPECIFIED
#define J1939DCM_APPL_DATA

/* J1939RM */
#define J1939RM_APPL_DATA
#define J1939RM_VAR_NO_INIT_UNSPECIFIED
#define J1939RM_VAR_CLEARED_UNSPECIFIED
#define J1939RM_CONFIG_DATA
#define J1939RM_CONST_UNSPECIFIED
#define J1939RM_CONST_16
#define J1939RM_CONST_BOOLEAN
#define J1939RM_PBCFG

/* J1939Nm */
#define J1939NM_APPL_DATA
#define J1939NM_CONST_BOOLEAN
#define J1939NM_CONST_8
#define J1939NM_CONST_16
#define J1939NM_CONST_BOOLEAN
#define J1939NM_CONST_UNSPECIFIED
#define J1939NM_VAR_CLEARED_32
#define J1939NM_VAR_UNSPECIFIED

/* J1939TP */
#define J1939TP_APPL_DATA
#define J1939TP_CONST_UNSPECIFIED
#define J1939TP_CONST_UNSPECIFIED
#define J1939TP_CONST_8

/*===================================================ENUMS==========================================*/

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/

/*===========================================VARIABLE DECLARATIONS==================================*/

/*============================================FUNCTION PROTOTYPES===================================*/

#ifdef __cplusplus
}
#endif

#endif /* COMPILER_CFG_H */
