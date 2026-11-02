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

 * @file Can_Hal.c
 *
 * @brief This file provides extern Can Hal API implement.
 *
 */
/*==============================================INCLUDE FILES=======================================*/
#include "OsIf.h"
#include "OsIf_Time.h"
#include "Ckgen_Hal.h"
#include "Rcm_Hal.h"
#include "Dma_Hal.h"
#include "AC784xx_Can_Reg.h"
#include "Can_Hal.h"
#include "Core_Hal.h"

/*============================================DEFINES AND MACROS====================================*/
#if defined (AC7840X)
/**< @def CAN_BASE_PTRS
*  @brief CAN base address pointer array for AC7840X series
*  @note DES_CAN_MACRO_051
*/
#define CAN_BASE_PTRS              {CAN0, CAN1, CAN2, CAN3}
#endif
/**< @def CAN_WAIT_TIMEOUT
*  @brief CAN wait timeout in milliseconds
*  @note DES_CAN_MACRO_052
*/
#define CAN_WAIT_TIMEOUT           (50U)
/* ============================================= TYPEDEFS ================================================ */
#if defined (AC7843X)
/**
 * @def CAN_STANDARD_FILTER
 * @brief Standard filter type definition
 * @note DES_CAN_MACRO_053
 */
#define CAN_STANDARD_FILTER 0U

/**
 * @def CAN_EXTEND_FILTER
 * @brief Extended filter type definition
 * @note DES_CAN_MACRO_054
 */
#define CAN_EXTEND_FILTER   1U

/**
 * @def CAN_RX_FIFO0_DMA_OFFSET_ADDRESS
 * @brief DMU RX FIFO0 DMA offset address
 * @note DES_CAN_MACRO_055
 */
#define CAN_RX_FIFO0_DMA_OFFSET_ADDRESS         (0x280U)

/**
 * @def CAN_RX_FIFO1_DMA_OFFSET_ADDRESS
 * @brief DMU RX FIFO1 DMA offset address
 * @note DES_CAN_MACRO_056
 */
#define CAN_RX_FIFO1_DMA_OFFSET_ADDRESS         (0x300U)

/**
 * @brief CAN group enumeration
 * @note DES_CAN_TYP_051
 */
enum
{
    CAN_GROUP0 = 0U, /*!< Group 0: First CAN group */
    CAN_GROUP1,      /*!< Group 1: Second CAN group */
    CAN_GROUP_ALL    /*!< Group ALL: Number of CAN groups */
};

/**
 * @brief CAN controller state structure
 * @note DES_CAN_TYP_052
 */
typedef struct
{
    uint8 GroupInitRef[CAN_GROUP_ALL];        /*!< Reference count for group CAN bus clock and reset */
    uint8 ExtTimestampEnMask[CAN_GROUP_ALL];  /*!< Reference count for external timestamp enable */
} Can_CtrlStateType;
#endif

/**
 * @brief CAN device argument structure
 * @note DES_CAN_TYP_053
 */
typedef struct
{
    Hal_CallbackType IrqCallback;                /*!< IRQ callback function pointer */
    Hal_CallbackType WakeupIrqCallback;          /*!< Wakeup IRQ callback function pointer */
    boolean FdEn;                                /*!< FD mode enable flag */
    Can_ExtendModeType Mode;                     /*!< Current CAN extended mode */
    Dma_ChannelAddrType RxFifoDmaChannelAddr[CAN_RX_FIFO_NUM_MAX]; /*!< RX FIFO DMA channel address array */
    uint8 RxFifoDmaChannel[CAN_RX_FIFO_NUM_MAX]; /*!< RX FIFO DMA channel array */
#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
    Can_TxSecAmountType TxSecAmount;             /*!< CAN TX secondary buffer amount */
#elif defined (AC7843X)
    uint8 RxBuffersNum;                          /*!< Number of RX buffers */
    uint8 TxBuffersNum;                          /*!< Number of TX buffers */
    Hal_CallbackType DmuIrqCallback;             /*!< DMU IRQ callback function pointer */
    Can_BufferInfoType *TxBuffersInfoPtr;        /*!< Pointer to TX buffers info */
    Can_BufferInfoType *RxBuffersInfoPtr;        /*!< Pointer to RX buffers info */
    uint32 TxPendingMask;                        /*!< TX pending mask for buffer index */
    Can_MramConfigType mramSizeStatus;           /*!< Message RAM distribute state */
    Can_MramAddressType *mramAddresss;           /*!< Message RAM element start address */
    uint32 ErrorCode;                            /*!< Temporary error code storage */
    uint8 GroupIndex;                            /*!< CAN group index */
#endif
} Can_DeviceType;

/**
 * @brief CAN IRQ event process function.
 * @details Handles CAN interrupt events for the specified CAN hardware unit instance.
 * @param[in] Instance: Specify CAN HW Unit.
 * @return void
 * @pre None
 * @post Interrupt event is processed.
 * @note DES_CAN_API_050
 */
static void Can_Hal_ISR(uint8 Instance);

/**
 * @brief CAN WAKEUP IRQ event process function.
 * @details Handles CAN wakeup interrupt events for the specified CAN hardware unit instance.
 * @param[in] Instance: Specify CAN HW Unit.
 * @return void
 * @pre None
 * @post Wakeup interrupt event is processed.
 * @note DES_CAN_API_051
 */
static void Can_Hal_WakeupISR(uint8 Instance);

/**
 * @brief Configure CAN DMA arguments.
 * @details Configures DMA for CAN RX FIFO with the given parameters.
 * @param[in] Instance: Specify CAN HW Unit.
 * @param[in] SrcAddr: DMA read data from the address.
 * @param[in] Length: Read data length.
 * @param[in] RxFifoBufDmaConfigPtr: DMA config arguments.
 * @return void
 * @pre None
 * @post DMA is configured for RX FIFO.
 * @note DES_CAN_API_052
 */
static void Can_Hal_CfgDma(uint8 Instance, uint32 SrcAddr,
                           uint32 Length, Can_RxFifoBufDmaConfigType const *RxFifoBufDmaConfigPtr);

/**
 * @brief Deinitialize CAN used DMA channel.
 * @details Deinitializes the DMA channel used by the specified CAN instance.
 * @param[in] Instance: Specify CAN HW Unit.
 * @return void
 * @pre None
 * @post DMA channel is deinitialized.
 * @note DES_CAN_API_053
 */
static void Can_Hal_DmaDeinit(uint8 Instance);

/**
 * @brief Get the CAN base address.
 * @details Returns the base address pointer for the specified CAN hardware unit instance.
 * @param[in] Instance: Specify CAN HW Unit.
 * @return CAN base address pointer.
 * @pre None
 * @post None
 * @note DES_CAN_API_054
 */
LOCAL_INLINE CAN_Type *Can_Hal_GetBaseLocal(uint8 Instance);

/**
 * @brief Get the CAN device pointer.
 * @details Returns the device pointer for the specified CAN hardware unit instance.
 * @param[in] Instance: Specify CAN HW Unit.
 * @return CAN device pointer.
 * @pre None
 * @post None
 * @note DES_CAN_API_055
 */
LOCAL_INLINE Can_DeviceType *Can_Hal_GetDevice(uint8 Instance);

#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
/**
 * @brief Set CAN filter enable controller bits.
 * @details Enable or disable the filter for the specified index.
 * @param[in] Instance: Specify CAN HW Unit.
 * @param[in] Index: CAN index value to be set.
 * @param[in] Enable: Enable or disable flag.
 * @return void
 * @pre None
 * @post Filter enable state is updated.
 * @note DES_CAN_API_056
 */
static void Can_Hal_SetFilterEnable(uint8 Instance, uint8 Index, boolean Enable);

/**
 * @brief Check if CAN transmission is busy.
 * @details Returns whether the specified buffer is currently busy transmitting.
 * @param[in] Instance: Specify CAN HW Unit.
 * @param[in] BufferId: Transmission buffer ID.
 * @return 1 if busy, 0 otherwise.
 * @pre None
 * @post None
 * @note DES_CAN_API_057
 */
static uint32 Can_Hal_IsTransmitBusy(uint8 Instance, uint8 BufferId);

/**
 * @brief Check if CAN is transmitting.
 * @details Returns whether the specified buffer is currently transmitting.
 * @param[in] Instance: Specify CAN HW Unit.
 * @param[in] BufferId: Transmission buffer ID.
 * @return 1 if transmitting, 0 otherwise.
 * @pre None
 * @post None
 * @note DES_CAN_API_058
 */
static uint32 Can_Hal_IsTransmitting(uint8 Instance, uint8 BufferId);

/**
 * @brief Configure CAN controller to extended mode in hardware.
 * @details Sets the CAN controller to the specified extended mode in hardware.
 * @param[in] Instance: Specify CAN HW Unit.
 * @param[in] Mode: Extended mode to configure.
 * @return void
 * @pre None
 * @post Hardware mode is updated.
 * @note DES_CAN_API_059
 */
static void Can_Hal_ConfigExtendModeToHw(uint8 Instance, Can_ExtendModeType Mode);

/**
 * @brief Read frame info from hardware.
 * @details Reads the message info from the specified CAN hardware module.
 * @param[in] CanModuleNx: Given CAN module base.
 * @param[out] InfoPtr: Message info pointer.
 * @return void
 * @pre None
 * @post InfoPtr is updated with frame information.
 * @note DES_CAN_API_060
 */
static void Can_Hal_GetMsgInfoHw(CAN_Type const * const CanModuleNx, Can_MessageInfoType *InfoPtr);

/**
 * @brief Set CAN filter parameter.
 * @details Configure the filter for the specified index and enable state.
 * @param[in] Instance: Specify CAN HW Unit.
 * @param[in] Index: Filter index.
 * @param[in] Enable: Enable filter or not.
 * @param[in] ConfigPtr: Filter configuration pointer.
 * @return void
 * @pre None
 * @post Filter configuration is updated.
 * @note DES_CAN_API_061
 */
static void Can_Hal_SetFilter(uint8 Instance, uint8 Index, boolean Enable, const Can_FilterParamsType *ConfigPtr);

#elif defined (AC7843X)
/**
 * @brief Set CAN normal bitrate
 * @details Configures the normal (arbitration) bitrate for the specified CAN module hardware.
 * @param[in] Base: Pointer to the CAN hardware module.
 * @param[in] Bitrate: Pointer to the bitrate configuration structure.
 * @return void
 * @pre The CAN module must be initialized and in configuration mode.
 * @post The normal bitrate is set in hardware.
 * @note DES_CAN_API_062
 */
static void Can_Hal_SetSlowBitrate(CAN_Type *Base, const Can_BitrateParamsType *Bitrate);

/**
 * @brief Set CAN FD data bitrate
 * @details Configures the CAN FD data phase bitrate for the specified CAN module hardware.
 * @param[in] Base: Pointer to the CAN hardware module.
 * @param[in] Bitrate: Pointer to the data phase bitrate configuration structure.
 * @return void
 * @pre The CAN module must be initialized and in configuration mode.
 * @post The FD data bitrate is set in hardware.
 * @note DES_CAN_API_063
 */
static void Can_Hal_SetFastBitrate(CAN_Type *Base, const Can_BitrateParamsType *Bitrate);

/**
 * @brief Enable or disable CAN module initialization
 * @details Enables or disables the initialization mode for the specified CAN hardware module.
 * @param[in] Base: Pointer to the CAN hardware module.
 * @param[in] En: TRUE to enable initialization, FALSE to disable.
 * @return void
 * @pre The CAN module must be powered and accessible.
 * @post The initialization mode is set accordingly.
 * @note DES_CAN_API_064
 */
static void Can_Hal_InitEnable(CAN_Type *Base, boolean En);

/**
 * @brief Release receive buffer
 * @details Releases the specified receive buffer in the CAN message RAM.
 * @param[in] Base: Pointer to the CAN hardware module.
 * @param[in] BufferIndex: Index of the receive buffer to release.
 * @param[in] Location: Location in message RAM to release.
 * @return void
 * @pre The buffer must be allocated and valid.
 * @post The specified buffer is released and available for new messages.
 * @note DES_CAN_API_065
 */
static void Can_Hal_ReleaseBuffer(CAN_Type *Base, uint8 BufferIndex, Can_MramType Location);

/**
 * @brief Calculate element RAM real address
 * @details Calculates the real address of a message RAM element for the given CAN instance and index.
 * @param[in] Instance: CAN module instance number.
 * @param[in] Index: Element index within the message RAM.
 * @param[in] Location: Message RAM type/location.
 * @return uint32* Pointer to the calculated element address.
 * @pre The CAN instance and index must be valid.
 * @post None
 * @note DES_CAN_API_066
 */
static uint32 *Can_Hal_MRAMCalculate(uint8 Instance, uint8 Index, Can_MramType Location);

/**
 * @brief Configure CAN message RAM registers
 * @details Configures the message RAM registers for the specified CAN module instance.
 * @param[in] Instance: CAN module instance number.
 * @return void
 * @pre The CAN module must be initialized and in configuration mode.
 * @post Message RAM registers are configured.
 * @note DES_CAN_API_067
 */
static void Can_Hal_SetMRAMConfig(uint8 Instance);

/**
 * @brief Initialize CAN message RAM
 * @details Initializes the message RAM for the specified CAN module instance.
 * @param[in] Instance: CAN module instance number.
 * @param[in] mramAddresss: Pointer to the message RAM address structure.
 * @return void
 * @pre The CAN module must be initialized.
 * @post Message RAM is initialized and ready for use.
 * @note DES_CAN_API_068
 */
static void Can_Hal_MRAMInit(uint8 Instance, const Can_MramAddressType *const mramAddresss);

/**
 * @brief Get CAN message RAM configuration
 * @details Retrieves and checks the message RAM configuration for the specified CAN module instance.
 * @param[in] Instance: CAN module instance number.
 * @param[in] ConfigPtr: Pointer to the CAN configuration structure.
 * @return void
 * @pre The CAN module must be initialized.
 * @post The configuration is checked and updated if necessary.
 * @note DES_CAN_API_069
 */
static void Can_Hal_GetMRAMConfig(uint8 Instance, const Can_HalConfigType *ConfigPtr);

/**
 * @brief Initialize the CAN controller
 * @details Initializes the CAN controller hardware for the specified instance with the given configuration.
 * @param[in] Instance: CAN module instance number.
 * @param[in] ConfigPtr: Pointer to the CAN configuration structure.
 * @return void
 * @pre The CAN module must be powered and accessible.
 * @post The controller is initialized and ready for operation.
 * @note DES_CAN_API_070
 */
static void Can_Hal_CtrlInit(uint8 Instance, const Can_HalConfigType *ConfigPtr);

/**
 * @brief Deinitialize the CAN controller
 * @details Deinitializes the CAN controller hardware for the specified instance.
 * @param[in] Instance: CAN module instance number.
 * @return void
 * @pre The CAN controller must be initialized.
 * @post The controller is deinitialized and hardware resources are released.
 * @note DES_CAN_API_071
 */
static void Can_Hal_CtrlDeInit(uint8 Instance);

/**
 * @brief Read frame info from hardware
 * @details Reads CAN frame information from the specified hardware buffer and fills the Info structure.
 * @param[out] Info: Pointer to the message information structure to be filled.
 * @param[in] Buf: Pointer to the hardware frame buffer.
 * @param[in] DmaEn: DMA enable flag, TRUE if DMA is used.
 * @return void
 * @pre Info and Buf must be valid pointers.
 * @post Info is updated with the received frame information.
 * @note DES_CAN_API_072
 */
static void Can_Hal_GetMsgInfoHw(Can_MessageInfoType *Info, const uint32 *Buf, boolean DmaEn);

/**
 * @brief Set CAN standard filter parameter
 * @details Configures the standard ID filter for the specified CAN instance and index.
 * @param[in] Instance: CAN module instance number.
 * @param[in] Index: Standard ID filter index.
 * @param[in] ConfigPtr: Pointer to the filter configuration structure.
 * @return void
 * @pre Instance, Index, and ConfigPtr must be valid.
 * @post The standard filter is configured in hardware.
 * @note DES_CAN_API_073
 */
static void Can_Hal_SetStdFilter(uint8 Instance, uint8 Index, const Can_FilterParamsType *ConfigPtr);

/**
 * @brief Set CAN extended filter parameter
 * @details Configures the extended ID filter for the specified CAN instance and index.
 * @param[in] Instance: CAN module instance number.
 * @param[in] Index: Extended ID filter index.
 * @param[in] ConfigPtr: Pointer to the filter configuration structure.
 * @return void
 * @pre Instance, Index, and ConfigPtr must be valid.
 * @post The extended filter is configured in hardware.
 * @note DES_CAN_API_074
 */
static void Can_Hal_SetExtFilter(uint8 Instance, uint8 Index, const Can_FilterParamsType *ConfigPtr);

/**
 * @brief Set CAN RX and TX configuration to hardware
 * @details Configures the receive and transmit settings in hardware for the specified CAN instance.
 * @param[in] Instance: CAN module instance number.
 * @param[in] ConfigPtr: Pointer to the CAN configuration structure.
 * @return void
 * @pre Instance and ConfigPtr must be valid.
 * @post RX and TX configuration is applied to hardware.
 * @note DES_CAN_API_075
 */
static void Can_Hal_SetRxTxConfig(uint8 Instance, const Can_HalConfigType *ConfigPtr);

/**
 * @brief CAN DMU interrupt service routine
 * @details Handles the DMU interrupt for the specified CAN module instance.
 * @param[in] Instance: CAN module instance number.
 * @return void
 * @pre The CAN module and DMU must be initialized.
 * @post DMU interrupt is processed and cleared.
 * @note DES_CAN_API_076
 */
static void Can_Hal_DmuISR(uint8 Instance);
#endif

/* =========================================== LOCAL VARIABLES ============================================== */
/**
 * @brief Table of CAN base address pointers
 * @note DES_CAN_VAR_001
 */
static CAN_Type *const CanBase[CAN_INSTANCE_MAX] = CAN_BASE_PTRS;

/**
 * @brief Table of CAN IRQ IDs
 * @note DES_CAN_VAR_002
 */
static const IRQn_Type CanIrqs[CAN_INSTANCE_MAX] = CAN_IRQS;

/**
 * @brief Table of CAN wakeup IRQ IDs
 * @note DES_CAN_VAR_003
 */
static const IRQn_Type CanWakeupIrqs[CAN_INSTANCE_MAX] = CAN_WAKEUP_IRQS;

/*PRQA S 3218 ++ # AMCE_CAN_3218.*/
/**
 * @brief CAN DLC to bytes mapping table
 * @note DES_CAN_VAR_004
 */
static const uint8 s_dlcToBytes[] = {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 12U, 16U, 20U, 24U, 32U, 48U, 64U};
/*PRQA S 3218 -- # AMCE_CAN_3218.*/

/* Table of CAN ckgen interface clocks */
static const Ckgen_BusClkIdType CanCkgenBusClock[CAN_INSTANCE_MAX] = \
{
    CKGEN_CAN0_BUS_CLK, CKGEN_CAN1_BUS_CLK
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
    , CKGEN_CAN2_BUS_CLK, CKGEN_CAN3_BUS_CLK
#if defined (AC7842X) || defined (AC7843X)
    , CKGEN_CAN4_BUS_CLK, CKGEN_CAN5_BUS_CLK
#endif
#endif
};

/* Table of CAN soft resets */
static const Rcm_ResetIDType CanClockReset[CAN_INSTANCE_MAX] = \
{
    RCM_RESET_ID_CAN0, RCM_RESET_ID_CAN1
#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
    , RCM_RESET_ID_CAN2, RCM_RESET_ID_CAN3
#if defined (AC7842X) || defined (AC7843X)
    , RCM_RESET_ID_CAN4, RCM_RESET_ID_CAN5
#endif
#endif
};

#if defined (AC7843X)
/*PRQA S 3218 ++ # AMCE_CAN_3218.*/
/**
 * @brief Table of CAN DMU IRQ IDs
 * @note DES_CAN_VAR_005
 */
static const IRQn_Type CanDMUIrq[CAN_INSTANCE_MAX] = CAN_DMU_IRQS;
/*PRQA S 3218 -- # AMCE_CAN_3218.*/

/**
 * @brief CAN controller state variable
 * @note DES_CAN_VAR_008
 */
static Can_CtrlStateType CanCtrlState;
/**
 * @brief CAN data size change table
 * @note DES_CAN_VAR_009
 */
static const uint8 CanDataSizeChange[8] = {8U, 12U, 16U, 20U, 24U, 32U, 48U, 64U};
#endif

/**
 * @brief CAN0 device information
 * @note DES_CAN_VAR_010
 */
#if (CONFIG_CAN0_ENABLE)
static Can_DeviceType Can0Device;
#endif

/**
 * @brief CAN1 device information
 * @note DES_CAN_VAR_011
 */
#if (CONFIG_CAN1_ENABLE)
static Can_DeviceType Can1Device;
#endif

#if defined (AC7840X) || defined (AC7842X) || defined (AC7843X)
/**
 * @brief CAN2 device information
 * @note DES_CAN_VAR_012
 */
#if (CONFIG_CAN2_ENABLE)
static Can_DeviceType Can2Device;
#endif

/**
 * @brief CAN3 device information
 * @note DES_CAN_VAR_013
 */
#if (CONFIG_CAN3_ENABLE)
static Can_DeviceType Can3Device;
#endif

#if defined (AC7842X)  || defined (AC7843X)
/**
 * @brief CAN4 device information
 * @note DES_CAN_VAR_014
 */
#if (CONFIG_CAN4_ENABLE)
static Can_DeviceType Can4Device;
#endif

/**
 * @brief CAN5 device information
 * @note DES_CAN_VAR_015
 */
#if (CONFIG_CAN5_ENABLE)
static Can_DeviceType Can5Device;
#endif

#endif
#endif

#if defined (AC7843X)
/**
 * @brief CAN0 MRAM address information
 * @note DES_CAN_VAR_016
 */
#if (CONFIG_CAN0_ENABLE)
static Can_MramAddressType Can0MramAddresss;
#endif

/**
 * @brief CAN1 MRAM address information
 * @note DES_CAN_VAR_017
 */
#if (CONFIG_CAN1_ENABLE)
static Can_MramAddressType Can1MramAddresss;
#endif

/**
 * @brief CAN2 MRAM address information
 * @note DES_CAN_VAR_018
 */
#if (CONFIG_CAN2_ENABLE)
static Can_MramAddressType Can2MramAddresss;
#endif

/**
 * @brief CAN3 MRAM address information
 * @note DES_CAN_VAR_019
 */
#if (CONFIG_CAN3_ENABLE)
static Can_MramAddressType Can3MramAddresss;
#endif

/**
 * @brief CAN4 MRAM address information
 * @note DES_CAN_VAR_020
 */
#if (CONFIG_CAN4_ENABLE)
static Can_MramAddressType Can4MramAddresss;
#endif

/**
 * @brief CAN5 MRAM address information
 * @note DES_CAN_VAR_021
 */
#if (CONFIG_CAN5_ENABLE)
static Can_MramAddressType Can5MramAddresss;
#endif

/*PRQA S 3218 ++ # AMCE_CAN_3218.*/
/**
 * @brief Table of CAN MRAM address pointers
 * @note DES_CAN_VAR_022
 */
static Can_MramAddressType *const  CanMramAddresss[CAN_INSTANCE_MAX] =
{
#if (CONFIG_CAN0_ENABLE)
    &Can0MramAddresss,
#else
    NULL_PTR,
#endif

#if (CONFIG_CAN1_ENABLE)
    &Can1MramAddresss,
#else
    NULL_PTR,
#endif

#if (CONFIG_CAN2_ENABLE)
    &Can2MramAddresss,
#else
    NULL_PTR,
#endif

#if (CONFIG_CAN3_ENABLE)
    &Can3MramAddresss,
#else
    NULL_PTR,
#endif

#if (CONFIG_CAN4_ENABLE)
    &Can4MramAddresss,
#else
    NULL_PTR,
#endif

#if (CONFIG_CAN5_ENABLE)
    &Can5MramAddresss,
#else
    NULL_PTR,
#endif
};
#endif
/*PRQA S 3218 -- # AMCE_CAN_3218.*/

/**
 * @brief Table of CAN device pointers
 * @note DES_CAN_VAR_023
 */
static Can_DeviceType *const CanDevice[CAN_INSTANCE_MAX] =
{
#if (CONFIG_CAN0_ENABLE)
    &Can0Device,
#else
    NULL_PTR,
#endif

#if (CONFIG_CAN1_ENABLE)
    &Can1Device,
#else
    NULL_PTR,
#endif

#if defined (AC7840X) || defined (AC7842X)  || defined (AC7843X)

#if (CONFIG_CAN2_ENABLE)
    &Can2Device,
#else
    NULL_PTR,
#endif

#if (CONFIG_CAN3_ENABLE)
    &Can3Device,
#else
    NULL_PTR,
#endif

#if defined (AC7842X)  || defined (AC7843X)

#if (CONFIG_CAN4_ENABLE)
    &Can4Device,
#else
    NULL_PTR,
#endif

#if (CONFIG_CAN5_ENABLE)
    &Can5Device,
#else
    NULL_PTR,
#endif

#endif
#endif
};
/**
 * @brief DLC to data length mapping table
 * @note DES_CAN_VAR_024
 */
/*PRQA S 3218 ++ # AMCE_CAN_3218.*/
static const uint8 DlcMapTab[65] =
{
    0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, /*0~8*/
    9U, 9U, 9U, 9U, /*9~12*/
    10U, 10U, 10U, 10U, /*13~16*/
    11U, 11U, 11U, 11U, /*17~20*/
    12U, 12U, 12U, 12U, /*21~24*/
    13U, 13U, 13U, 13U, 13U, 13U, 13U, 13U, /*25~32*/
    14U, 14U, 14U, 14U, 14U, 14U, 14U, 14U, 14U, 14U, 14U, 14U, 14U, 14U, 14U, 14U, /*33~48*/
    15U, 15U, 15U, 15U, 15U, 15U, 15U, 15U, 15U, 15U, 15U, 15U, 15U, 15U, 15U, 15U, /*49~64*/
};
/*PRQA S 3218 -- # AMCE_CAN_3218.*/

/*=================================GLOBAL FUNCTION IMPLEMENTATIONS=================================*/
#if defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
void Can_Hal_Init(uint8 Instance, const Can_HalConfigType *ConfigPtr)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    DEVICE_ASSERT(ConfigPtr != NULL_PTR);
    CAN_Type *Base;
    const Can_BitrateParamsType *Bitrate;
    uint32 TicksDelayDuration;
    uint32 CurrentTicks;
    uint32 ElapsedTicks = 0U;
    uint32 CommonIntMasks = 0U;
    uint32 EccIntMasks = 0U;
    uint8 i = 0U;

    if ((Instance < CAN_INSTANCE_MAX) && (ConfigPtr != NULL_PTR) && (CanDevice[Instance] != NULL_PTR))
    {
#if defined (AC7842X)
        if (Instance > 3U)
        {
            DEVICE_ASSERT(FALSE == ConfigPtr->FdEn);
            DEVICE_ASSERT(ConfigPtr->Filters.FiltersNum <= CAN_FILTER_NUM_MAX);
        }
#endif
        Base = Can_Hal_GetBaseLocal(Instance);
        CanDevice[Instance]->TxSecAmount = ConfigPtr->TxSecAmount;
        (void)Ckgen_Hal_EnablePeriphClk(CanCkgenBusClock[Instance], TRUE);
        Rcm_Hal_SetResetState(CanClockReset[Instance], RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState(CanClockReset[Instance], RCM_RESET_STATE_DEASSERT);
        Can_Reg_SetReset(Base, TRUE);
        Can_Reg_SetMemEcc(Base, ConfigPtr->EccEn);
        Bitrate = &ConfigPtr->BaudrateConfigPtr->NormalBitrate;
        Can_Reg_SetSBitrate(Base, Bitrate->Presc, Bitrate->Sjw, Bitrate->Seg1, Bitrate->Seg2);

        CanDevice[Instance]->Mode = CAN_EXTMODE_OFF;
        /* FD mode is enable */
        if (TRUE == ConfigPtr->FdEn)
        {
            Can_Reg_SetFdIso(Base, ConfigPtr->FdIsoEn);
            Bitrate = &ConfigPtr->BaudrateConfigPtr->DataBitrate;
            Can_Reg_SetFBitrate(Base, Bitrate->Presc, Bitrate->Sjw, Bitrate->Seg1, Bitrate->Seg2);
            if (0U != ConfigPtr->BaudrateConfigPtr->SspOffset)
            {
                Can_Reg_SetTdc(Base, TRUE);
                Can_Reg_SetSspOff(Base, ConfigPtr->BaudrateConfigPtr->SspOffset);
            }
        }
        CanDevice[Instance]->FdEn = ConfigPtr->FdEn;

        /* ECC is enable */
        if (TRUE == ConfigPtr->EccEn)
        {
            TicksDelayDuration = OsIf_MicrosToTicks(CAN_WAIT_TIMEOUT);
            CurrentTicks = OsIf_GetCounter();
            /* Wait for Memort init done */
            while ((uint32)0 == Can_Reg_IsMemInitDone(Base))
            {
                ElapsedTicks += OsIf_GetElapsed(&CurrentTicks);
                /* Loop timeout */
                if (ElapsedTicks >= TicksDelayDuration)
                {
                    break;
                }
            }
            if (0U != ConfigPtr->EccIrqEnMasks)
            {
                EccIntMasks = ConfigPtr->EccIrqEnMasks & CAN_ECC_IRQ_EN_MASKS;
            }
        }
        for (i = 0; i < ConfigPtr->Filters.FiltersNum; i++)
        {
            Can_Hal_SetFilter(Instance, i, TRUE, \
                          &ConfigPtr->Filters.FiltersParamsPtr[i]);
        }
        Can_Reg_SetRom(Base, (ConfigPtr->RxOverWrite == TRUE) ? 0U : 1U);
        Can_Reg_SetBusOffRecDisable(Base, ConfigPtr->BOREC);
        if (ConfigPtr->RxFifoBufDmaConfigNum != 0U)
        {
            Can_Reg_SetDmaRecv(Base, TRUE);
            Can_Hal_CfgDma(Instance, (uint32)&Base->RBUF, 0x4cU, &ConfigPtr->RxFifoBufDmaConfigPtr[0]);
        }
        if (NULL_PTR != ConfigPtr->WakeupIrqCallback)
        {
            CanDevice[Instance]->WakeupIrqCallback = ConfigPtr->WakeupIrqCallback;
            Core_Hal_EnableIrq(CanWakeupIrqs[Instance]);
#if defined (AC7840X) || defined (AC7842X)
            Can_Reg_SetLpfen(Base, TRUE);
            Can_Regl_SetWuen(Base, TRUE);
#elif defined (AC7840E)
            if (Instance == 0U)
            {
                Can_Reg_SetLpfen0(SPM, TRUE);
                Can_Reg_SetWuen0(SPM, TRUE);
            }
            else
            {
                Can_Reg_SetLpfen1(SPM, TRUE);
                Can_Reg_SetWuen1(SPM, TRUE);
            }
#endif
        }
        if (0U != ConfigPtr->CommonIrqEnMasks)
        {
            CommonIntMasks = ConfigPtr->CommonIrqEnMasks & CAN_COMMON_IRQ_EN_MASKS;
            /* If enable DMA receive, the receive done interrupt is not needed */
            if (ConfigPtr->RxFifoBufDmaConfigNum != 0U)
            {
                CommonIntMasks &= ~CAN_CTRL1_RIE_Msk;
            }
        }

        if ((0U != CommonIntMasks) || (0U != EccIntMasks))
        {
            Can_Reg_SetIntEnable(Base, CommonIntMasks);
            Can_Reg_SetEccIntEnable(Base, EccIntMasks);
        }
        if (NULL_PTR != ConfigPtr->IrqCallback)
        {
            CanDevice[Instance]->IrqCallback = ConfigPtr->IrqCallback;
            Core_Hal_EnableIrq(CanIrqs[Instance]);
        }
    }
}

void Can_Hal_Deinit(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    if ((Instance < CAN_INSTANCE_MAX) && (CanDevice[Instance] != NULL_PTR))
    {
        Core_Hal_DisableIrq(CanIrqs[Instance]);
        Core_Hal_DisableIrq(CanWakeupIrqs[Instance]);
        CanDevice[Instance]->WakeupIrqCallback = NULL_PTR;
        CanDevice[Instance]->IrqCallback = NULL_PTR;
        /* Reset CAN*/
        Rcm_Hal_SetResetState(CanClockReset[Instance], RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState(CanClockReset[Instance], RCM_RESET_STATE_DEASSERT);
        (void)Ckgen_Hal_EnablePeriphClk(CanCkgenBusClock[Instance], FALSE);
        Can_Hal_DmaDeinit(Instance);
        Core_Hal_ClearPendingIrq(CanIrqs[Instance]);
        Core_Hal_ClearPendingIrq(CanWakeupIrqs[Instance]);
    }
}

Hal_StatusType Can_Hal_SetControllerState(uint8 Instance, Can_HalStateType State)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base;
    Hal_StatusType RetState = STATUS_SUCCESS;

    if ((Instance < CAN_INSTANCE_MAX) && (CanDevice[Instance] != NULL_PTR))
    {
        Base = Can_Hal_GetBaseLocal(Instance);
        if (CAN_STATE_STANDBY == State)
        {
            if (Can_Hal_GetControllerState(Instance) != CAN_STATE_STOP)
            {
                RetState = STATUS_ERROR;
            }
            Can_Reg_SetStandby(Base, TRUE);
        }
        else if (CAN_STATE_RUNNING == State)
        {
            if (Can_Hal_GetControllerState(Instance) != CAN_STATE_STOP)
            {
                RetState = STATUS_ERROR;
            }
            Can_Reg_SetReset(Base, FALSE);
            Can_Reg_ClearBusOff(Base);
            Can_Hal_ConfigExtendModeToHw(Instance, CanDevice[Instance]->Mode);
        }
        else
        {
            Can_Reg_SetReset(Base, TRUE);
            Can_Reg_SetStandby(Base, FALSE);
        }
    }
    else
    {
        RetState = STATUS_ERROR;
    }

    return RetState;
}

/*PRQA S 1505 ++ # AMCE_CAN_1505. */
void Can_Hal_SetMsgInfo(uint8 Instance, uint8 BufferIndex, const Can_MessageInfoType *InfoPtr)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *CanModuleNx;
    uint8 DataLength;
    uint8 DlcValue;
    uint8 Index;
    uint32 IdNu;
    uint32 CtrlAddr;
    uint32 FdPaddingValue = 0U;
    uint32 FdPaddingValueTmp = 0U;
    uint8 DataPos;

    if ((Instance < CAN_INSTANCE_MAX) && (CanDevice[Instance] != NULL_PTR))
    {
        CanModuleNx = CanBase[Instance];
        /* Select transmit buffer */
        Can_Reg_SetTbsel(CanModuleNx, BufferIndex);
        /* Set message ID, TTSEN, ESI bits */
        IdNu = (InfoPtr->MsgId & CAN_INFO_ID_Msk);
        /* Set TTSEN bit */
        if (0U != Can_Reg_GetTimeStamp(CanModuleNx))
        {
            IdNu |= CAN_INFO_TTSEN_Msk;
        }

        /* Set ESI bit */
        if (0U != Can_Reg_GetEpass(CanModuleNx))
        {
            IdNu |= CAN_INFO_ESI_Msk;
        }
        Can_Reg_SetIdEsi(CanModuleNx, IdNu);
        DlcValue = DlcMapTab[InfoPtr->MsgDataLen];
        /* Set message control bits */
        CtrlAddr = (uint32)DlcValue & CAN_INFO_DLC_Msk;
        if (TRUE == InfoPtr->MsgBrs)
        {
            CtrlAddr |= CAN_INFO_BRS_Msk;
        }
        if ((TRUE == InfoPtr->MsgFdf) && (TRUE == CanDevice[Instance]->FdEn))
        {
            CtrlAddr |= CAN_INFO_FDF_Msk;
        }
        else if (TRUE == InfoPtr->MsgRtr)
        {
            CtrlAddr |= CAN_INFO_RTR_Msk;
        }
        else
        {
            /* undefine */
        }
        if (CAN_EXTENDED_MSG == InfoPtr->MsgIdType)
        {
            CtrlAddr |= CAN_INFO_IDE_Msk;
        }
        Can_Reg_SetCtrlAddrVal(CanModuleNx, CtrlAddr);
        if (((uint8)CAN_CTRL_DATA_FRAME == InfoPtr->MsgRtr) && (0U != InfoPtr->MsgDataLen))
        {
            DataLength = s_dlcToBytes[DlcValue];
            DataPos = DataLength;
            if (DataLength != InfoPtr->MsgDataLen)
            {
                /*the data position of MsgDataPtr, MsgDataLen 4B downward-aligned */
                DataPos = InfoPtr->MsgDataLen & 0xFCU;
                FdPaddingValue = InfoPtr->FdPaddingValue;
                for (Index = 0U; Index < 4U; Index++)
                {
                    /*msg data merge with FdPaddingValue to 4B*/
                    if (Index < (InfoPtr->MsgDataLen - DataPos))
                    {
                        FdPaddingValueTmp |= ((uint32)InfoPtr->MsgDataPtr[DataPos + Index] << (Index * 8U));
                    }
                    else
                    {
                        FdPaddingValueTmp |= (FdPaddingValue << (Index * 8U));
                    }
                }
                FdPaddingValue = \
                (FdPaddingValue << 24U) | (FdPaddingValue << 16U) | (FdPaddingValue << 8U) | FdPaddingValue;
            }
            for (Index = 0U; Index < DataLength; Index += 4U)
            {
                if (Index < DataPos)
                {
                    /* PRQA S 0310,3305 ++ # AMCE_CAN_0310, AMCE_CAN_3305. */
                    /* Data buffer narrow is 8byte but the platform define data narrow is 32byte */
                    /* There is no issue to assign data value here. */
                    Can_Reg_SetTbufData(CanModuleNx, Index >> 2U, *(uint32 *)(&InfoPtr->MsgDataPtr[Index]));
                    /* PRQA S 0310,3305 -- # AMCE_CAN_0310, AMCE_CAN_3305. */
                }
                else if (Index == DataPos)
                {
                    Can_Reg_SetTbufData(CanModuleNx, Index >> 2U, FdPaddingValueTmp);
                }
                else
                {
                    Can_Reg_SetTbufData(CanModuleNx, Index >> 2U, FdPaddingValue);
                }
            }
        }
        if ((uint8)CAN_TX_BUF_SECONDARY == BufferIndex)
        {
            Can_Reg_SetTsnext(CanModuleNx);
        }
    }
}

void Can_Hal_StartTransmit(uint8 Instance, uint8 BufferId)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base;

    if ((Instance < CAN_INSTANCE_MAX) && (CanDevice[Instance] != NULL_PTR))
    {
        Base = Can_Hal_GetBaseLocal(Instance);
        /* the transmit mode is CAN_TRANSMIT_PRIMARY.*/
        if ((uint8)CAN_TX_BUF_PRIMARY == BufferId)
        {
            Can_Reg_SetTpe(Base, TRUE);
        }
        else /* the transmit mode is CAN_TRANSMIT_SECONDARY.*/
        {
            if (CAN_TX_SEC_ALL == CanDevice[Instance]->TxSecAmount)
            {
                Can_Reg_SetTsall(Base, TRUE);
            }
            else
            {
                Can_Reg_SetTsone(Base, TRUE);
            }
        }
    }
}

Hal_StatusType Can_Hal_WriteTxBufferMsgSetOnly(uint8 Instance, uint8 BufferId, const Can_MessageInfoType *MessagePtr)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    Hal_StatusType RetVal = STATUS_SUCCESS;

    if (CAN_STATE_RUNNING == Can_Hal_GetControllerState(Instance))
    {
        if ((uint8)CAN_TX_BUF_SECONDARY == BufferId)
        {
            /*the hardware SECONDARY transmit is busy*/
            if (0U != Can_Hal_IsTransmitBusy(Instance, BufferId))
            {
                RetVal = STATUS_BUSY;
            }
        }
        /* the hardware PRIMARY transmit is busy*/
        else if (0U != Can_Hal_IsTransmitting(Instance, BufferId))
        {
            RetVal = STATUS_BUSY;
        }
        else
        {
            /* Normal */
        }
        /* the hardware transmit fifo has free*/
        if (STATUS_SUCCESS == RetVal)
        {
            Can_Hal_SetMsgInfo(Instance, BufferId, MessagePtr);
        }
    }
    else
    {
        RetVal = STATUS_ERROR;
    }

    return RetVal;
}
/*PRQA S 1505 -- # AMCE_CAN_1505. */

Hal_StatusType Can_Hal_WriteTxBuffer(uint8 Instance, uint8 BufferId, const Can_MessageInfoType *MessagePtr)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    Hal_StatusType RetVal = STATUS_SUCCESS;

    RetVal = Can_Hal_WriteTxBufferMsgSetOnly(Instance, BufferId, MessagePtr);
    if (STATUS_SUCCESS == RetVal)
    {
        Can_Hal_StartTransmit(Instance, BufferId);
    }

    return RetVal;
}

Hal_StatusType Can_Hal_ReadRxBuffer(uint8 Instance, uint8 BufferId, Can_MessageInfoType *MessagePtr)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base;
    Hal_StatusType RetStatus = STATUS_ERROR;

    Base = Can_Hal_GetBaseLocal(Instance);
    if (CAN_STATE_RUNNING == Can_Hal_GetControllerState(Instance))
    {
        /* the hardware has recieve data */
        if (0U != Can_Reg_GetRStat(Base))
        {
            Can_Hal_GetMsgInfoHw(Base, MessagePtr);
            Can_Reg_SetRrel(Base);
            RetStatus = STATUS_SUCCESS;
        }
    }
    (void)BufferId;

    return RetStatus;
}

Hal_StatusType Can_Hal_AbortTransmit(uint8 Instance, uint8 BufferId)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base;
    Hal_StatusType RetStatus = STATUS_ERROR;

    Base = Can_Hal_GetBaseLocal(Instance);
    if (1U == Can_Hal_IsTransmitting(Instance, BufferId))
    {
        if ((uint8)CAN_TX_BUF_PRIMARY == BufferId)
        {
            Can_Reg_SetTpa(Base, TRUE);
        }
        else
        {
            Can_Reg_SetTsa(Base, TRUE);
        }
        RetStatus = STATUS_SUCCESS;
    }

    return RetStatus;
}

Hal_StatusType Can_Hal_GetTxStatus(uint8 Instance, uint8 BufferId)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base;
    Hal_StatusType Status = STATUS_ERROR;
    uint32 Val;

    Base = Can_Hal_GetBaseLocal(Instance);
    Val = Can_Reg_GetCommonIrqFlagMasks(Base);
    if ((uint8)CAN_TX_BUF_PRIMARY == BufferId)
    {
        if (0U != (Val & CAN_CTRL1_TPIF_Msk))
        {
            Can_Reg_ClearCommonIrqFlagMasks(Base, CAN_CTRL1_TPIF_Msk);
            Status = STATUS_SUCCESS;
        }
    }
    else
    {
        if (0U != (Val & CAN_CTRL1_TSIF_Msk))
        {
            Can_Reg_ClearCommonIrqFlagMasks(Base, CAN_CTRL1_TSIF_Msk);
            Status = STATUS_SUCCESS;
        }
    }

    return Status;
}

uint8 Can_Hal_GetTxErrorCount(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type const *Base;

    Base = Can_Hal_GetBaseLocal(Instance);
    return (uint8)Can_Reg_GetTeCnt(Base);
}

uint8 Can_Hal_GetRxErrorCount(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type const *Base;

    Base = Can_Hal_GetBaseLocal(Instance);
    return (uint8)Can_Reg_GetReCnt(Base);
}

Can_DevErrorStateType Can_Hal_GetErrorState(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type const *Base;
    Can_DevErrorStateType state;

    Base = Can_Hal_GetBaseLocal(Instance);
    if (0U != Can_Reg_IsBusOff(Base))
    {
        state = CAN_ERR_STATE_BUSOFF;
    }
    else
    {
        if (0U != Can_Reg_GetEpass(Base))
        {
            state = CAN_ERR_STATE_PASSIVE;
        }
        else
        {
            state = CAN_ERR_STATE_ACTIVE;
        }
    }

    return state;
}

uint32 Can_Hal_GetErrorsInfo(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base;
    uint32 RetVal = 0U;
    uint32 Flag;

    Base = Can_Hal_GetBaseLocal(Instance);
    Flag = Can_Reg_GetKoer(Base);
    switch (Flag)
    {
        case 1U:
            RetVal |= (uint32)CAN_BUSERR_BIT_MASK;
            break;
        case 2U:
            RetVal |= (uint32)CAN_BUSERR_FORM_MASK;
            break;
        case 3U:
            RetVal |= (uint32)CAN_BUSERR_STUFF_MASK;
            break;
        case 4U:
            RetVal |= (uint32)CAN_BUSERR_ACK_MASK;
            break;
        case 5U:
            RetVal |= (uint32)CAN_BUSERR_CRC_MASK;
            break;
        case 6U:
            RetVal |= (uint32)CAN_BUSERR_OTHER_MASK;
            break;
        default:
            /* Not support */
            break;
    }

    Flag = Can_Reg_GetCommonIrqFlagMasks(Base);
    if (0U != (Flag & CAN_CTRL1_ALIF_Msk))
    {
        Can_Reg_ClearCommonIrqFlagMasks(Base, CAN_CTRL1_ALIF_Msk);
        RetVal |= (uint32)CAN_ARBIT_ERR_MASK;
    }

    if (0U != (Flag & CAN_CTRL1_ROIF_Msk))
    {
        Can_Reg_ClearCommonIrqFlagMasks(Base, CAN_CTRL1_ROIF_Msk);
        RetVal |= (uint32)CAN_OVERFLOW_ERR_MASK;
    }

    Flag = Can_Reg_GetEccIrqFlagMasks(Base);
    if (0U != (Flag & CAN_VERMEM_MDWIF_Msk))
    {
        Can_Reg_ClearEccIrqFlagMasks(Base, CAN_VERMEM_MDWIF_Msk);
        RetVal |= (uint32)CAN_ECC_WARN_MASK;
    }

    if (0U != (Flag & CAN_VERMEM_MDEIF_Msk))
    {
        Can_Reg_ClearEccIrqFlagMasks(Base, CAN_VERMEM_MDEIF_Msk);
        RetVal |= (uint32)CAN_ECC_ERR_MASK;
    }

    return RetVal;
}

static void Can_Hal_ConfigExtendModeToHw(uint8 Instance, Can_ExtendModeType Mode)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base;

    Base = Can_Hal_GetBaseLocal(Instance);
    Can_Reg_SetLom(Base, FALSE);
    Can_Reg_SetLbmi(Base, FALSE);
    Can_Reg_SetLbme(Base, FALSE);
    Can_Reg_SetSack(Base, FALSE);
    switch (Mode)
    {
    case CAN_EXTMODE_LISTENING:
        {
            Can_Reg_SetLom(Base, TRUE);
            break;
        }
    case CAN_EXTMODE_LOOPBACK_INTERNEL:
        {
            Can_Reg_SetLbmi(Base, TRUE);
            break;
        }
    case CAN_EXTMODE_LOOPBACK_EXTERNEL:
        {
            Can_Reg_SetLbme(Base, TRUE);
            Can_Reg_SetSack(Base, TRUE);
            break;
        }
    default:
        {
            /* Not support */
            break;
        }
    }
}

#ifndef CAN_SDK_NON_EXTENDED_API
void Can_Hal_ConfigExtendMode(uint8 Instance, Can_ExtendModeType Mode)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    DEVICE_ASSERT(Can_Hal_GetControllerState(Instance) == CAN_STATE_STOP);
    if ((Instance < CAN_INSTANCE_MAX) && (CanDevice[Instance] != NULL_PTR))
    {
        CanDevice[Instance]->Mode = Mode;
    }
}

void Can_Hal_ConfigTimeStamp(uint8 Instance, const Can_TimeStampType *tsConfigPtr)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base;
    uint8 Val;

    Base = Can_Hal_GetBaseLocal(Instance);
    DEVICE_ASSERT(Can_Hal_GetControllerState(Instance) == CAN_STATE_STOP);
    Can_Reg_SetTimeStamp(Base, FALSE);
    if (TRUE == tsConfigPtr->En)
    {
        if (TRUE == tsConfigPtr->ExtClkSrc)
        {
            Val = 0U;
        }
        else
        {
            Val = 1U;
        }
        Can_Reg_SetTimeClockSrc(Base, Val);
        Can_RegTsClkDivCtrl(Instance, tsConfigPtr->ExtClkDiv);
        Can_Reg_SetTimePosition(Base, tsConfigPtr->PosEnd);
        Can_Reg_SetTimeStamp(Base, TRUE);
        Can_Reg_SetTimeCount(Base, TRUE);
    }
    else
    {
        Can_Reg_SetTimeCount(Base, FALSE);
    }
}

uint32 Can_Hal_GetTxTimeStamp(uint8 Instance, uint8 BufferId)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base;
    (void)BufferId;
    Base = Can_Hal_GetBaseLocal(Instance);
    return Can_Reg_GetTts(Base);
}

void Can_Hal_SetTxSecAmount(uint8 Instance, Can_TxSecAmountType amount)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);

    if ((Instance < CAN_INSTANCE_MAX) && (CanDevice[Instance] != NULL_PTR))
    {
        CanDevice[Instance]->TxSecAmount = amount;
    }
}

void Can_Hal_GetMsgInfo(Can_MessageInfoType *InfoPtr, const uint32 *Buf)
{
    DEVICE_ASSERT(InfoPtr != NULL_PTR);
    DEVICE_ASSERT(Buf != NULL_PTR);

    uint32 head1, head2;
    uint8 DlcValue;
    const uint32 *data;
    const uint32 *TmpBuf;
    if ((InfoPtr != NULL_PTR) && (Buf != NULL_PTR))
    {
        TmpBuf = Buf;
        head1 = *TmpBuf;
        InfoPtr->MsgId = (head1 & CAN_INFO_ID_Msk);
        TmpBuf++;
        head2 = *TmpBuf;
        /*PRQA S 4394,4342 ++ # AMCE_CAN_4394, AMCE_CAN_4342.*/
        InfoPtr->MsgIdType = (Can_MessageIdType)((head1 & CAN_INFO_IDE_Msk) >> CAN_INFO_IDE_Pos);
        /*PRQA S 4394,4342 -- # AMCE_CAN_4394, AMCE_CAN_4342.*/
        InfoPtr->MsgRtr = (boolean)((head1 & CAN_INFO_RTR_Msk) >> CAN_INFO_RTR_Pos);
        InfoPtr->MsgFdf = (boolean)((head2 & CAN_INFO_FDF_Msk) >> CAN_INFO_FDF_Pos);
        InfoPtr->MsgBrs = (boolean)((head2 & CAN_INFO_BRS_Msk) >> CAN_INFO_BRS_Pos);
        DlcValue = (uint8)(head2 & CAN_INFO_DLC_Msk);
        InfoPtr->MsgDataLen = s_dlcToBytes[DlcValue];
        if ((InfoPtr->MsgFdf == FALSE) && (InfoPtr->MsgDataLen > 8U))
        {
            InfoPtr->MsgDataLen = 8U;
        }
        TmpBuf++;
        data = TmpBuf;
        /* PRQA S 0311 ++ # AMCE_CAN_0311. */
        InfoPtr->MsgDataPtr = (uint8 *)data;
        /* PRQA S 0311 -- # AMCE_CAN_0311. */
        /*PRQA S 0488 ++ # AMCE_CAN_0488.*/
        TmpBuf += 0x10;
        /*PRQA S 0488 -- # AMCE_CAN_0488.*/
        InfoPtr->MsgRts = *TmpBuf;
    }
}

Hal_StatusType Can_Hal_GetRxStatus(uint8 Instance, uint8 BufferId)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base;
    Hal_StatusType Status = STATUS_ERROR;
    uint32 Val;

    Base = Can_Hal_GetBaseLocal(Instance);
    Val = Can_Reg_GetCommonIrqFlagMasks(Base);
    if (0U != (Val & CAN_CTRL1_RIF_Msk))
    {
        Can_Reg_ClearCommonIrqFlagMasks(Base, CAN_CTRL1_RIF_Msk);
        Status = STATUS_SUCCESS;
    }
    (void)BufferId;

    return Status;
}
#endif

static uint32 Can_Hal_IsTransmitBusy(uint8 Instance, uint8 BufferId)
{
    CAN_Type const *Base;
    uint32 IsTransmitBusy = 0U;

    Base = Can_Hal_GetBaseLocal(Instance);
    if ((uint8)CAN_TX_BUF_PRIMARY == BufferId)
    {
        IsTransmitBusy = Can_Reg_GetTpe(Base);
    }
    else
    {
        if ((uint32)CAN_TSSTAT_FULL == Can_Reg_GetTsstat(Base))
        {
            IsTransmitBusy = 1U;
        }
    }

    return IsTransmitBusy;
}

static uint32 Can_Hal_IsTransmitting(uint8 Instance, uint8 BufferId)
{
    CAN_Type const *Base;
    uint32 IsTransmitting = 0U;

    Base = Can_Hal_GetBaseLocal(Instance);
    if ((uint8)CAN_TX_BUF_PRIMARY == BufferId)
    {
        IsTransmitting = Can_Reg_GetTpe(Base);
    }
    else
    {
        if ((Can_Reg_GetCtrl0(Base) & (CAN_CTRL0_TSALL_Msk | CAN_CTRL0_TSONE_Msk)) != 0U)
        {
            IsTransmitting = 1U;
        }
    }

    return IsTransmitting;
}

static void Can_Hal_SetFilterEnable(uint8 Instance, uint8 Index, boolean Enable)
{
    CAN_Type *Base;

    Base = Can_Hal_GetBaseLocal(Instance);
    if (Index < CAN_ACFCTRL1_OFFSET)
    {
        Can_Reg_SetFilter0AcfEn(Base, Index, Enable);
    }
    else if (Index < CAN_ACFCTRL2_OFFSET)
    {
        Can_Reg_SetFilter1AcfEn(Base, Index, Enable);
    }
#if defined (AC7840X)
    else if (Index < CAN_FILTER_NUM_MAX)
#else
    else if (Index < CANFD_FILTER_NUM_MAX)
#endif
    {
        Can_Reg_SetFilter2AcfEn(Base, Index, Enable);
    }
    else
    {
        /* index is invalid */
    }
}

/*PRQA S 1505 ++ # AMCE_CAN_1505. */
Can_HalStateType Can_Hal_GetControllerState(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type const *Base;
    Can_HalStateType State;

    Base = Can_Hal_GetBaseLocal(Instance);
    if (0U != Can_Reg_GetStandby(Base))
    {
        State = CAN_STATE_STANDBY;
    }
    /*PRQA S 3415 ++ # AMCE_CAN_3415.*/
    else if ((0U != Can_Reg_GetReset(Base)) || (0U != Can_Reg_IsBusOff(Base)))
    /*PRQA S 3415 -- # AMCE_CAN_3415.*/
    {
        State = CAN_STATE_STOP;
    }
    else
    {
        State = CAN_STATE_RUNNING;
    }

    return State;
}
/*PRQA S 1505 -- # AMCE_CAN_1505. */

static void Can_Hal_SetFilter
(
    uint8 Instance,
    uint8 Index,
    boolean Enable,
    const Can_FilterParamsType *ConfigPtr
)
{
    CAN_Type *Base;
    uint32 Mask;

    Base = Can_Hal_GetBaseLocal(Instance);
    /* Enable filter */
    Can_Hal_SetFilterEnable(Instance, Index, Enable);
    /* Configure filter */
    if (TRUE == Enable)
    {
        /* Select filter index */
        Can_Reg_SetFilterIndex(Base, Index);
        /* Set filter code */
        Can_Reg_SetFilterCode(Base, ConfigPtr->ID1);
        /* set ACF Mask */
        Mask = (~(ConfigPtr->ID2)) & CAN_ACF_ACODE_Msk;
        if (STD_FRAME == ConfigPtr->FrameType)
        {
            /* only receive  Standard*/
            Mask |= CAN_ACF_AIDEE_Msk;
        }
        else if (EXT_FRAME == ConfigPtr->FrameType)
        {
            /* only receive  extend*/
            Mask |= CAN_ACF_AIDEE_Msk;
            Mask |= CAN_ACF_AIDE_Msk;
        }
        else
        {
            /* receive Standard/extend*/
        }
        Can_Reg_SetFilterMask(Base, Mask);
    }
}

static void Can_Hal_GetMsgInfoHw(CAN_Type const * const CanModuleNx, Can_MessageInfoType *InfoPtr)
{
    uint8 DlcValue;
    uint8 DataLength;
    uint8 Index;
    uint32 CtrlAddr;

    InfoPtr->MsgId = Can_Reg_GetIdEsi(CanModuleNx) & CAN_INFO_ID_Msk;
    CtrlAddr = Can_Reg_GetCtrlAddrVal(CanModuleNx);
    InfoPtr->MsgBrs = (boolean)((CtrlAddr & CAN_INFO_BRS_Msk) >> CAN_INFO_BRS_Pos);
    InfoPtr->MsgRtr = (boolean)((CtrlAddr & CAN_INFO_RTR_Msk) >> CAN_INFO_RTR_Pos);
    InfoPtr->MsgFdf = (boolean)((CtrlAddr & CAN_INFO_FDF_Msk) >> CAN_INFO_FDF_Pos);
    /*PRQA S 4394,4342 ++ # AMCE_CAN_4394, AMCE_CAN_4342.*/
    InfoPtr->MsgIdType = (Can_MessageIdType)((CtrlAddr >> CAN_INFO_IDE_Pos) & 0x01U);
    /*PRQA S 4394,4342 -- # AMCE_CAN_4394, AMCE_CAN_4342.*/
    DlcValue = (uint8)(CtrlAddr & CAN_INFO_DLC_Msk);
    InfoPtr->MsgDataLen = s_dlcToBytes[DlcValue];

    /* recieve is data frame and length is not equal to 0U */
    if (((uint8)CAN_CTRL_DATA_FRAME == InfoPtr->MsgRtr) && (0U != InfoPtr->MsgDataLen))
    {
        DataLength = InfoPtr->MsgDataLen;
        /* not fd mode and data length out of 8u */
        if ((InfoPtr->MsgFdf == FALSE) && (DataLength > 8U))
        {
            DataLength = 8U;
        }
        /* Loop get data for hardware FIFO */
        for (Index = 0U; Index < DataLength; Index += 4U)
        {
            /* PRQA S 0310,3305 ++ # AMCE_CAN_0310, AMCE_CAN_3305.*/
            /* Data buffer narrow is 8byte but the platform define data narrow is 32byte */
            /* There is no issue to assign data value here. */
            *(uint32 *)(&InfoPtr->MsgDataPtr[Index]) = Can_Reg_GetTbufData(CanModuleNx, Index >> 2U);
            /* PRQA S 0310,3305 -- # AMCE_CAN_0310, AMCE_CAN_3305.*/
        }
    }
    InfoPtr->MsgRts = Can_Reg_GetRts(CanModuleNx);
}

static void Can_Hal_ISR(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base;
    Can_IrqCBParams CbArgs;
#if defined CAN_PERFORMANCE_TEST
    Can_PerformaceTestType *CanDevTest = &CanPerformaceTest;
    uint64 StartTime, StopTime, IntvlTime;

    StartTime = OsIf_GetCurrentTimeUS();
#endif
    if ((Instance < CAN_INSTANCE_MAX) && (CanDevice[Instance] != NULL_PTR))
    {
        Base = Can_Hal_GetBaseLocal(Instance);
        CbArgs.CommonIrqFlagMasks = Can_Reg_GetCommonIrqFlagMasks(Base);
        CbArgs.EccIrqFlagMasks = Can_Reg_GetEccIrqFlagMasks(Base);
        Can_Reg_ClearCommonIrqFlagMasks(Base, CbArgs.CommonIrqFlagMasks);
        Can_Reg_ClearEccIrqFlagMasks(Base, CbArgs.EccIrqFlagMasks);
        if (NULL_PTR != CanDevice[Instance]->IrqCallback)
        {
            CbArgs.Instance = Instance;
            CanDevice[Instance]->IrqCallback(&CbArgs);
        }
    }
#if defined CAN_PERFORMANCE_TEST
    if (0U != (CbArgs.CommonIrqFlagMasks & CAN_CTRL1_RIF_Msk))
    {
        StopTime = OsIf_GetCurrentTimeUS();
        IntvlTime = StopTime - StartTime;
        CanDevTest->TimeRxAvg = IntvlTime;
        if (IntvlTime < 200U)
        {
            if ((0U == CanDevTest->TimeRxMin) || (CanDevTest->TimeRxMin > IntvlTime))
            {
                CanDevTest->TimeRxMin = IntvlTime;
            }
            if (CanDevTest->TimeRxMax < IntvlTime)
            {
                CanDevTest->TimeRxMax = IntvlTime;
            }
        }
    }
#endif
}
#elif defined (AC7843X)
void Can_Hal_Init(uint8 Instance, const Can_HalConfigType *ConfigPtr)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    DEVICE_ASSERT(ConfigPtr != NULL_PTR);
    CAN_Type *Base;
    Can_DeviceType *CanDev;
    uint32 CommonIntMasks;
    uint32 DmaSrcAddr;
    uint32 Length;
    uint8 FifoId;
    uint8 i;

    if ((Instance < CAN_INSTANCE_MAX) && (ConfigPtr != NULL_PTR) && (CanDevice[Instance] != NULL_PTR))
    {
        Base = Can_Hal_GetBaseLocal(Instance);
        CanDev = Can_Hal_GetDevice(Instance);
        CanDev->IrqCallback = NULL_PTR;
        CanDev->WakeupIrqCallback = NULL_PTR;
        CanDev->DmuIrqCallback = NULL_PTR;
        CanDev->RxBuffersNum = ConfigPtr->RxBuffersNum;
        CanDev->TxBuffersNum = ConfigPtr->TxBuffersNum;
        CanDev->TxBuffersInfoPtr = ConfigPtr->TxBuffersInfoPtr;
        CanDev->RxBuffersInfoPtr = ConfigPtr->RxBuffersInfoPtr;
        CanDev->GroupIndex = (uint8)((Instance < CAN_GROUP0_INSTANCE_MAX) ? CAN_GROUP0 : CAN_GROUP1);
        Can_Hal_CtrlInit(Instance, ConfigPtr);
        DEVICE_ASSERT(TRUE != Can_Reg_GetClockStop(Base));
        Can_Hal_InitEnable(Base, TRUE);
        Can_Hal_SetSlowBitrate(Base, &ConfigPtr->BaudrateConfigPtr->NormalBitrate);
        /* FD mode is enable */
        if (TRUE == ConfigPtr->FdEn)
        {
            Can_Reg_SetFDOE(Base, TRUE);
            Can_Reg_SetNISO(Base, (TRUE != ConfigPtr->FdIsoEn) ? 1U : 0U);
            Can_Reg_SetBRSE(Base, TRUE);
            Can_Hal_SetFastBitrate(Base, &ConfigPtr->BaudrateConfigPtr->DataBitrate);
            if ((0U != ConfigPtr->BaudrateConfigPtr->SspOffset) || (0U != ConfigPtr->BaudrateConfigPtr->TDCFWL))
            {
                Can_Reg_SetTDC(Base, TRUE);
                Can_Reg_SetTDCOffset(Base, ConfigPtr->BaudrateConfigPtr->SspOffset);
                Can_Reg_SetTDCFilter(Base, ConfigPtr->BaudrateConfigPtr->TDCFWL);
            }
        }
        CanDev->FdEn = ConfigPtr->FdEn;
        /* set message RAM configuration*/
        Can_Hal_GetMRAMConfig(Instance, ConfigPtr);
        Can_Hal_SetMRAMConfig(Instance);
        /* set rx configuration */
        Can_Hal_SetRxTxConfig(Instance, ConfigPtr);
        Can_Reg_SetEventFifoWatermark(Base, ConfigPtr->TxEventFifoWm);
        CommonIntMasks = ConfigPtr->CommonIrqEnMasks;
        if (ConfigPtr->RxFifoBufDmaConfigNum != 0U)
        {
            for (i = 0; i < ConfigPtr->RxFifoBufDmaConfigNum; i++)
            {
                FifoId = ConfigPtr->RxFifoBufDmaConfigPtr[i].FifoId;
                if (0U == FifoId)
                {
                    DmaSrcAddr = (uint32)Base + CAN_RX_FIFO0_DMA_OFFSET_ADDRESS;
                    Length = (uint32)CanDataSizeChange[CanDev->mramSizeStatus.rxFifo0DataSize] + 8UL;
                    /* If enable DMA receive, the receive done interrupt is not needed */
                    CommonIntMasks &= ~CAN_IE_RF0NE_Msk;
                }
                else
                {
                    DmaSrcAddr = (uint32)Base + CAN_RX_FIFO1_DMA_OFFSET_ADDRESS;
                    Length = (uint32)CanDataSizeChange[CanDev->mramSizeStatus.rxFifo1DataSize] + 8UL;
                    /* If enable DMA receive, the receive done interrupt is not needed */
                    CommonIntMasks &= ~CAN_IE_RF1NE_Msk;
                }
                Can_Hal_CfgDma(Instance, DmaSrcAddr, Length, &ConfigPtr->RxFifoBufDmaConfigPtr[i]);
            }
            if (ConfigPtr->DmuIrqEnMasks != 0U)
            {
                DEVICE_ASSERT(NULL_PTR != ConfigPtr->DmuIrqCallback);
                Can_Reg_SetDMUIE(Base, ConfigPtr->DmuIrqEnMasks);
                CanDev->DmuIrqCallback = ConfigPtr->DmuIrqCallback;
                Core_Hal_EnableIrq(CanDMUIrq[Instance]);
            }
        }
        if (NULL_PTR != ConfigPtr->WakeupIrqCallback)
        {
            CanDev->WakeupIrqCallback = ConfigPtr->WakeupIrqCallback;
            Core_Hal_EnableIrq(CanWakeupIrqs[Instance]);
        }

        if (NULL_PTR != ConfigPtr->IrqCallback)
        {
            CanDev->IrqCallback = ConfigPtr->IrqCallback;
            CommonIntMasks = ConfigPtr->CommonIrqEnMasks;
            Can_Reg_SetIE(Base, CommonIntMasks);
            Core_Hal_EnableIrq(CanIrqs[Instance]);
        }
        CanDev->Mode = CAN_EXTMODE_OFF;
    }
}

void Can_Hal_Deinit(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    Can_DeviceType *CanDev;
    if (Instance < CAN_INSTANCE_MAX)
    {
        CanDev = Can_Hal_GetDevice(Instance);
        /* Reset CAN*/
        Can_Hal_CtrlDeInit(Instance);
        /* Disable CAN interrupt */
        Core_Hal_DisableIrq(CanIrqs[Instance]);
        Core_Hal_DisableIrq(CanWakeupIrqs[Instance]);
        CanDev->IrqCallback = NULL_PTR;
        CanDev->WakeupIrqCallback = NULL_PTR;
        Can_Hal_DmaDeinit(Instance);
        Core_Hal_ClearPendingIrq(CanIrqs[Instance]);
        Core_Hal_ClearPendingIrq(CanWakeupIrqs[Instance]);
    }
}

Hal_StatusType Can_Hal_SetControllerState(uint8 Instance, Can_HalStateType State)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base;
    Hal_StatusType RetState = STATUS_SUCCESS;

    Base = Can_Hal_GetBaseLocal(Instance);
    if (CAN_STATE_RUNNING == State)
    {
        if (Can_Hal_GetControllerState(Instance) != CAN_STATE_STOP)
        {
            RetState = STATUS_ERROR;
        }
        else
        {
            Can_Hal_InitEnable(Base, FALSE);
        }
    }
    else
    {
        Can_Hal_InitEnable(Base, TRUE);
    }

    return RetState;
}

/*PRQA S 1505 ++ # AMCE_CAN_1505. */
void Can_Hal_SetMsgInfo(uint8 Instance, uint8 BufferIndex, const Can_MessageInfoType *InfoPtr)
{
    DEVICE_ASSERT(InfoPtr != NULL_PTR);
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    uint32 id, ctrl;
    uint32 *Address;
    uint32 FdPaddingValue = 0U;
    uint32 FdPaddingValueTmp = 0U;
    uint8 DataPos;
    uint8 DataLength;
    uint8 DlcValue;
    uint8 i;

    if ((Instance < CAN_INSTANCE_MAX) && (InfoPtr != NULL_PTR))
    {
        DlcValue = DlcMapTab[InfoPtr->MsgDataLen];
        Address = Can_Hal_MRAMCalculate(Instance, BufferIndex, TX_BUFFER);
        /* Set message ID, TTSEN, ESI bits */
        if ((uint8)InfoPtr->MsgIdType != 0U)
        {
            id = (InfoPtr->MsgId & CAN_INFO_EXT_ID_Msk);
            id |= CAN_INFO_XTD_Msk;
        }
        else
        {
            id = ((InfoPtr->MsgId << CAN_INFO_STD_ID_Pos) & CAN_INFO_STD_ID_Msk);
        }
        /* Set ESI bit */
        if (1U == Can_Reg_GetEP(CanBase[Instance]))
        {
            id |= CAN_INFO_ESI_Msk;
        }
        if (TRUE == InfoPtr->MsgRtr)
        {
            id |= CAN_INFO_RTR_Msk;
        }
        *Address = id;

        /* Set message control bits */
        ctrl = ((((uint32)DlcValue << CAN_INFO_DLC_Pos) & CAN_INFO_DLC_Msk) |\
                ((uint32)InfoPtr->MmFidx << CAN_INFO_MM_Pos));
        if (TRUE == InfoPtr->MsgFdf)
        {
            ctrl |= CAN_INFO_FDF_Msk;
            if (TRUE == InfoPtr->MsgBrs)
            {
                ctrl |= CAN_INFO_BRS_Msk;
            }
        }
        if (TRUE == InfoPtr->EfcAnmf)
        {
            ctrl |= CAN_INFO_EFC_Msk;
        }
        Address++;
        *Address = ctrl;

        /* Set message data bits */
        if (((uint8)CAN_CTRL_DATA_FRAME == InfoPtr->MsgRtr) && (DlcValue > 0U))
        {
            DEVICE_ASSERT(InfoPtr->MsgDataPtr != NULL_PTR);
            if (InfoPtr->MsgDataPtr != NULL_PTR)
            {
                DataLength = s_dlcToBytes[DlcValue];
                DataPos = DataLength;
                if (DataLength != InfoPtr->MsgDataLen)
                {
                    /*the data position of MsgDataPtr, MsgDataLen 4B downward-aligned */
                    DataPos = InfoPtr->MsgDataLen & 0xFCU;
                    FdPaddingValue = InfoPtr->FdPaddingValue;
                    for (i = 0U; i < 4U; i++)
                    {
                        /*msg data merge with FdPaddingValue to 4B*/
                        if (i < (InfoPtr->MsgDataLen - DataPos))
                        {
                            FdPaddingValueTmp |= ((uint32)InfoPtr->MsgDataPtr[DataPos + i] << (i * 8U));
                        }
                        else
                        {
                            FdPaddingValueTmp |= (FdPaddingValue << (i * 8U));
                        }
                    }
                    FdPaddingValue = \
                    (FdPaddingValue << 24U) | (FdPaddingValue << 16U) | (FdPaddingValue << 8U) | FdPaddingValue;
                }
                Address++;
                /* PRQA S 0310,3305 ++ # AMCE_CAN_0310, AMCE_CAN_3305. */
                for (i = 0U; i < DataLength; i += 4U)
                {
                    if (i < DataPos)
                    {
                        *Address = *(uint32 *)(&InfoPtr->MsgDataPtr[i]);
                    }
                    else if (i == DataPos)
                    {
                        *Address = FdPaddingValueTmp;
                    }
                    else
                    {
                        *Address = FdPaddingValue;
                    }
                    Address++;
                }
                /* PRQA S 0310,3305 -- # AMCE_CAN_0310, AMCE_CAN_3305. */
            }
        }
    }
}
/*PRQA S 1505 -- # AMCE_CAN_1505. */

Hal_StatusType Can_Hal_WriteTxBuffer(uint8 Instance, uint8 BufferId,
                                     Can_MessageInfoType *MessagePtr)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base;
    Can_DeviceType *CanDev;
    Hal_StatusType RetVal = STATUS_SUCCESS;
    uint8 BufferIndex;
    uint8 BufferType;

    Base = Can_Hal_GetBaseLocal(Instance);
    CanDev = Can_Hal_GetDevice(Instance);
    DEVICE_ASSERT(BufferId < CanDev->TxBuffersNum);
    if (CAN_STATE_RUNNING == Can_Hal_GetControllerState(Instance))
    {
        BufferIndex = CanDev->TxBuffersInfoPtr[BufferId].BufferIndex;
        BufferType = CanDev->TxBuffersInfoPtr[BufferId].BufferType;
        /* check that index is in range */
        if (BufferType == (uint8)TX_BUF_TYPE_FIFO)
        {
            if (TRUE == Can_Reg_GetTxFQFull(Base))
            {
                RetVal = STATUS_BUSY;
            }
            else
            {
                BufferIndex = Can_Reg_GetTxFQPutIndex(Base);
            }
        }

        /* Fill transmit buffer, than enable transmission */
        /* Check the tx buffer pending status */
        if (RetVal == STATUS_SUCCESS)
        {
            if (TRUE == Can_Reg_IsTransmitPending(Base, BufferIndex))
            {
                RetVal = STATUS_BUSY;
            }
            else
            {
                Can_Hal_SetMsgInfo(Instance, BufferIndex, MessagePtr);
                Can_Reg_StartTransmit(Base, BIT(BufferIndex));
                MessagePtr->HwBufIdx = BufferIndex;
                CanDev->TxPendingMask |= BIT(BufferIndex);
            }
        }
    }
    else
    {
        RetVal = STATUS_ERROR;
    }

    return RetVal;
}

Hal_StatusType Can_Hal_ReadRxBuffer(uint8 Instance, uint8 BufferId, Can_MessageInfoType *MessagePtr)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base;
    const Can_DeviceType *CanDev;
    Hal_StatusType RetStatus = STATUS_SUCCESS;
    const uint32 *rxAddress;
    uint8 BufferType;
    uint8 BufferIndex;
    Can_MramType MramType;

    Base = Can_Hal_GetBaseLocal(Instance);
    CanDev = Can_Hal_GetDevice(Instance);
    /* Check if there is a message in the receive buffer */
    DEVICE_ASSERT(BufferId < CanDev->RxBuffersNum);
    BufferType = CanDev->RxBuffersInfoPtr[BufferId].BufferType;
    switch (BufferType)
    {
    case (uint8)RX_BUF_TYPE_DEDICATED:
        BufferIndex = CanDev->RxBuffersInfoPtr[BufferId].BufferIndex;
        if (BufferIndex > 31U)
        {
            if ((Can_Reg_GetNewData2(Base) & (1UL << (BufferIndex - 32U))) == 0U)
            {
                RetStatus = STATUS_ERROR;
            }
        }
        else
        {
            if ((Can_Reg_GetNewData1(Base) & (1UL << BufferIndex)) == 0U)
            {
                RetStatus = STATUS_ERROR;
            }
        }
        if (RetStatus == STATUS_SUCCESS)
        {
            rxAddress = Can_Hal_MRAMCalculate(Instance, BufferIndex, DEDICATED_RX_BUFFER);
            MramType = DEDICATED_RX_BUFFER;
        }
        break;

    case (uint8)RX_BUF_TYPE_FIFO0:
        if (Can_Reg_GetRxFifo0Fill(Base) != 0U)
        {
            BufferIndex = Can_Reg_GetRxFifo0GetIndex(Base);
            rxAddress = Can_Hal_MRAMCalculate(Instance, BufferIndex, RX_FIFO0);
            MramType = RX_FIFO0;
        }
        else
        {
            RetStatus = STATUS_ERROR;
        }
        break;

    case (uint8)RX_BUF_TYPE_FIFO1:
        if (Can_Reg_GetRxFifo1Fill(Base) != 0U)
        {
            BufferIndex = Can_Reg_GetRxFifo1GetIndex(Base);
            rxAddress = Can_Hal_MRAMCalculate(Instance, BufferIndex, RX_FIFO1);
            MramType = RX_FIFO1;
        }
        else
        {
            RetStatus = STATUS_ERROR;
        }
        break;

    default:
        RetStatus = STATUS_ERROR;
        break;
    }

    if (RetStatus == STATUS_SUCCESS)
    {
        /* Get message information */
        Can_Hal_GetMsgInfoHw(MessagePtr, rxAddress, FALSE);
        /* Release receive buffer */
        Can_Hal_ReleaseBuffer(Base, BufferIndex, MramType);
    }

    return RetStatus;
}

Hal_StatusType Can_Hal_AbortTransmit(uint8 Instance, uint8 BufferId)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    Hal_StatusType Status = STATUS_ERROR;
    CAN_Type *Base;
    Can_DeviceType *CanDev;
    uint8 BufferIndex;
    uint8 BufferType;
    uint32 RegVal;
    uint32 TxPendingMaskSw;

    Base = Can_Hal_GetBaseLocal(Instance);
    CanDev = Can_Hal_GetDevice(Instance);
    BufferIndex = CanDev->TxBuffersInfoPtr[BufferId].BufferIndex;
    BufferType = CanDev->TxBuffersInfoPtr[BufferId].BufferType;
    TxPendingMaskSw = CanDev->TxPendingMask;
    if ((uint8)TX_BUF_TYPE_FIFO == BufferType)
    {
        BufferIndex = Can_Reg_GetTxFifoGetIndex(Base);
    }
    RegVal = Can_Reg_AllTransmitPending(Base);
    if (((0U != (TxPendingMaskSw & BIT(BufferIndex)))) && (0U != (RegVal & BIT(BufferIndex))))
    {
        Can_Reg_AbortTransmit(Base, BIT(BufferIndex));
        CanDev->TxPendingMask &= ~BIT(BufferIndex);
        Status = STATUS_SUCCESS;
    }

    return Status;
}

uint8 Can_Hal_GetTxErrorCount(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type const *Base;

    Base = Can_Hal_GetBaseLocal(Instance);
    return (uint8)Can_Reg_GetTEC(Base);
}

uint8 Can_Hal_GetRxErrorCount(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type const *Base;

    Base = Can_Hal_GetBaseLocal(Instance);
    return (uint8)Can_Reg_GetREC(Base);
}

Can_DevErrorStateType Can_Hal_GetErrorState(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type const *Base;
    Can_DeviceType *CanDev;
    Can_DevErrorStateType state;

    Base = Can_Hal_GetBaseLocal(Instance);
    CanDev = Can_Hal_GetDevice(Instance);
    /*record errorcode because its reg will be clear on the follow*/
    CanDev->ErrorCode = Can_Reg_GetErrorCode(Base);

    if (0U != Can_Reg_IsBusOff(Base))
    {
        state = CAN_ERR_STATE_BUSOFF;
    }
    else if (0U != Can_Reg_GetErrorPassiveFlag(Base))
    {
        state = CAN_ERR_STATE_PASSIVE;
    }
    else
    {
        state = CAN_ERR_STATE_ACTIVE;
    }

    return state;
}

uint32 Can_Hal_GetErrorsInfo(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type const *Base;
    Can_DeviceType *CanDev;
    uint32 RetVal = 0U;
    uint32 Flag;

    Base = Can_Hal_GetBaseLocal(Instance);
    CanDev = Can_Hal_GetDevice(Instance);
    if (0U != CanDev->ErrorCode)
    {
        Flag = CanDev->ErrorCode;
        CanDev->ErrorCode = 0U;
    }
    else
    {
        Flag = Can_Reg_GetErrorCode(Base);
    }
    Flag &= CAN_PSR_LEC_Msk;
    switch (Flag)
    {
        case 1U:
            RetVal |= (uint32)CAN_BUSERR_STUFF_MASK;
            break;
        case 2U:
            RetVal |= (uint32)CAN_BUSERR_FORM_MASK;
            break;
        case 3U:
            RetVal |= (uint32)CAN_BUSERR_ACK_MASK;
            break;
        case 4U:
        case 5U:
            RetVal |= (uint32)CAN_BUSERR_BIT_MASK;
            break;
        case 6U:
            RetVal |= (uint32)CAN_BUSERR_CRC_MASK;
            break;
        default:
            /* Not support */
            break;
    }

    Flag = Can_Reg_GetIR(Base);

    if (0U != (Flag & CAN_IR_PEA_Msk))
    {
        RetVal |= (uint32)CAN_ARBIT_ERR_MASK;
    }
    if (0U != (Flag & (CAN_IR_RF1L_Msk | CAN_IR_RF0L_Msk)))
    {
        RetVal |= (uint32)CAN_OVERFLOW_ERR_MASK;
    }

    if (0U != (Flag & CAN_IR_BEC_Msk))
    {
        RetVal |= (uint32)CAN_ECC_WARN_MASK;
    }

    if (0U != (Flag & CAN_IR_BEU_Msk))
    {
        RetVal |= (uint32)CAN_ECC_ERR_MASK;
    }

    return RetVal;
}

#ifndef CAN_SDK_NON_EXTENDED_API
void Can_Hal_ConfigExtendMode(uint8 Instance, Can_ExtendModeType Mode)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    DEVICE_ASSERT(Can_Hal_GetControllerState(Instance) == CAN_STATE_STOP);
    CAN_Type *Base;

    Base = Can_Hal_GetBaseLocal(Instance);
    /* CAN mode configure */
    switch (Mode)
    {
    case CAN_EXTMODE_OFF:
        Can_Reg_SetBusMonitor(Base, FALSE);
        Can_Reg_SetTest(Base, FALSE);
        Can_Reg_SetASM(Base, FALSE);
        break;
    case CAN_EXTMODE_LISTENING:
        {
            Can_Reg_SetBusMonitor(Base, TRUE);
            Can_Reg_SetTest(Base, FALSE);
            Can_Reg_SetASM(Base, FALSE);
            break;
        }
    case CAN_EXTMODE_LOOPBACK_INTERNEL:
        {
            Can_Reg_SetBusMonitor(Base, TRUE);
            Can_Reg_SetTest(Base, TRUE);
            Can_Reg_SetASM(Base, FALSE);
            Can_Reg_SetLoopBack(Base, TRUE);
            break;
        }
    case CAN_EXTMODE_LOOPBACK_EXTERNEL:
        {
            Can_Reg_SetBusMonitor(Base, FALSE);
            Can_Reg_SetTest(Base, TRUE);
            Can_Reg_SetASM(Base, FALSE);
            Can_Reg_SetLoopBack(Base, TRUE);
            break;
        }
    default:
        {
            /* Not support */
            break;
        }
    }
}

void Can_Hal_ConfigTimeStamp(uint8 Instance, const Can_TimeStampType *tsConfigPtr)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base;
    uint32 Val;

    Base = Can_Hal_GetBaseLocal(Instance);
    DEVICE_ASSERT(Can_Hal_GetControllerState(Instance) == CAN_STATE_STOP);
    if (TRUE == tsConfigPtr->En)
    {
        /* extern timestamp enable */
        Val = Can_Reg_GetExtTimestampEn(CAN_CTRL);
        if (TRUE == tsConfigPtr->ExtClkSrc)
        {
            Can_Reg_SetTimeStamp(Base, 2U, 0U);
            if (Instance < CAN_GROUP0_INSTANCE_MAX)
            {
                Can_Reg_SetExtTimestamp0(CAN_CTRL, tsConfigPtr->ExtClkDiv);
                if (0U == CanCtrlState.ExtTimestampEnMask[CAN_GROUP0])
                {
                    Val |= BIT(CAN_GROUP0);
                }
                CanCtrlState.ExtTimestampEnMask[CAN_GROUP0] |= (uint8)BIT(Instance);
            }
            else
            {
                Can_Reg_SetExtTimestamp1(CAN_CTRL, tsConfigPtr->ExtClkDiv);
                if (0U == CanCtrlState.ExtTimestampEnMask[CAN_GROUP1])
                {
                    Val |= BIT(CAN_GROUP1);
                }
                CanCtrlState.ExtTimestampEnMask[CAN_GROUP1] |= (uint8)BIT(Instance);
            }
        }
        else
        {
            Can_Reg_SetTimeStamp(Base, 1U, 0U);
            if (Instance < CAN_GROUP0_INSTANCE_MAX)
            {
                CanCtrlState.ExtTimestampEnMask[CAN_GROUP0] &= (uint8)~BIT(Instance);
                if (0U == CanCtrlState.ExtTimestampEnMask[CAN_GROUP0])
                {
                    Val &= ~BIT(CAN_GROUP0);
                }
            }
            else
            {
                CanCtrlState.ExtTimestampEnMask[CAN_GROUP1] |= (uint8)BIT(Instance);
                if (0U == CanCtrlState.ExtTimestampEnMask[CAN_GROUP1])
                {
                    Val &= ~BIT(CAN_GROUP1);
                }
            }
        }
        Can_Reg_SetExtTimestampEn(CAN_CTRL, (uint8)Val);
        /* extern timestamp prescaler */
    }
    else
    {
        Can_Reg_SetTimeStamp(Base, 0U, 0U);
    }
}

Hal_StatusType Can_Hal_ReadTxEvent(uint8 Instance, Can_MsgEventType *Msg)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    DEVICE_ASSERT(Msg != NULL_PTR);
    CAN_Type *Base;
    Hal_StatusType RetVal = STATUS_ERROR;
    uint32 const*readAddress;
    uint32 eventData1, eventData2;
    uint8 Index;

    if ((Instance < CAN_INSTANCE_MAX) && (Msg != NULL_PTR))
    {
        Base = Can_Hal_GetBaseLocal(Instance);
        /* tx event fifo fill status */
        if (Can_Reg_GetTxEventFifoFill(Base) != 0U)
        {
            Index = Can_Reg_GetTxEventFifoGetIndex(Base);
            readAddress = Can_Hal_MRAMCalculate(Instance, Index, TX_EVENT_FIFO);
            eventData1 = *readAddress;
            Msg->ESI = (uint8)((eventData1 & CAN_INFO_ESI_Msk) >> CAN_INFO_ESI_Pos);
            Msg->XTD = (uint8)((eventData1 & CAN_INFO_XTD_Msk) >> CAN_INFO_XTD_Pos);
            Msg->RTR = (uint8)((eventData1 & CAN_INFO_RTR_Msk) >> CAN_INFO_RTR_Pos);
            if (Msg->XTD != 0U)
            {
                Msg->ID = eventData1 & CAN_INFO_EXT_ID_Msk;
            }
            else
            {
                Msg->ID = ((eventData1 & CAN_INFO_STD_ID_Msk) >> CAN_INFO_STD_ID_Pos);
            }
            readAddress++;
            eventData2 = *readAddress;
            Msg->MM = (uint8)((eventData2 & CAN_INFO_MM_Msk) >> CAN_INFO_MM_Pos);
            Msg->EFC = (uint8)((eventData2 & CAN_INFO_ET_Msk) >> CAN_INFO_ET_Pos);
            Msg->FDF = (uint8)((eventData2 & CAN_INFO_FDF_Msk) >> CAN_INFO_FDF_Pos);
            Msg->BRS = (uint8)((eventData2 & CAN_INFO_BRS_Msk) >> CAN_INFO_BRS_Pos);
            Msg->DLC = (uint8)((eventData2 & CAN_INFO_DLC_Msk) >> CAN_INFO_DLC_Pos);
            Msg->TXTS = (uint16)(eventData2 & CAN_INFO_TXTS_Msk);
            /* Release tx event fifo */
            Can_Hal_ReleaseBuffer(Base, Index, TX_EVENT_FIFO);
            RetVal = STATUS_SUCCESS;
        }
    }

    return RetVal;
}

void Can_Hal_GetMsgInfo(Can_MessageInfoType *InfoPtr, const uint32 *Buf)
{
    Can_Hal_GetMsgInfoHw(InfoPtr, Buf, TRUE);
}

Hal_StatusType Can_Hal_GetRxStatus(uint8 Instance, uint8 BufferId)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    Hal_StatusType Status = STATUS_ERROR;
    CAN_Type *Base;
    const Can_DeviceType *CanDev;
    uint8 BufferIndex;
    uint8 BufferType;

    Base = Can_Hal_GetBaseLocal(Instance);
    CanDev = Can_Hal_GetDevice(Instance);
    BufferIndex = CanDev->RxBuffersInfoPtr[BufferId].BufferIndex;
    BufferType = CanDev->RxBuffersInfoPtr[BufferId].BufferType;
    if ((uint8)RX_BUF_TYPE_DEDICATED == BufferType)
    {
        if (BufferIndex < 32U)
        {
            if (0U != (Can_Reg_GetNewData1(Base) & BIT(BufferIndex)))
            {
                Status = STATUS_SUCCESS;
            }
        }
        else
        {
            if (0U != (Can_Reg_GetNewData2(Base) & BIT((uint32)BufferIndex - 32UL)))
            {
                Status = STATUS_SUCCESS;
            }
        }
    }
    else if ((uint8)RX_BUF_TYPE_FIFO0 == BufferType)
    {
        if (Can_Reg_GetRxFifo0Fill(Base) != 0U)
        {
            Status = STATUS_SUCCESS;
        }
    }
    else
    {
        if (Can_Reg_GetRxFifo1Fill(Base) != 0U)
        {
            Status = STATUS_SUCCESS;
        }
    }

    return Status;
}

Hal_StatusType Can_Hal_GetTxStatus(uint8 Instance, uint8 HwBufIdx)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    Hal_StatusType Status = STATUS_ERROR;
    CAN_Type *Base;
    Can_DeviceType *CanDev;
    uint32 RegVal;

    Base = Can_Hal_GetBaseLocal(Instance);
    CanDev = Can_Hal_GetDevice(Instance);
    RegVal = Can_Reg_TransmitOccurred(Base);
    if ((0U != (CanDev->TxPendingMask & BIT(HwBufIdx))) && (0U != (RegVal & BIT(HwBufIdx))))
    {
        CanDev->TxPendingMask &= ~ BIT(HwBufIdx);
        Status = STATUS_SUCCESS;
    }
    return Status;
}
#endif

static void Can_Hal_SetSlowBitrate(CAN_Type *Base, const Can_BitrateParamsType *Bitrate)
{
    if ((Bitrate != NULL_PTR) && (Bitrate->Seg2 <= CAN_MAX_S_SEG_2) &&\
        (Bitrate->Sjw <= CAN_MAX_S_SJW) && (Bitrate->Presc <= CAN_MAX_S_PRESC))
    {
        Can_Reg_SetNBTP(Base, ((uint32)Bitrate->Sjw << CAN_NBTP_NSJW_Pos) |
            ((uint32)Bitrate->Presc << CAN_NBTP_NBRP_Pos) |
            ((uint32)Bitrate->Seg1 << CAN_NBTP_NTSEG1_Pos) | (uint32)Bitrate->Seg2);
    }
    else
    {
        DEVICE_ASSERT(Bitrate != NULL_PTR);
        if (Bitrate != NULL_PTR)
        {
            DEVICE_ASSERT(Bitrate->Seg2 <= CAN_MAX_S_SEG_2);
            DEVICE_ASSERT(Bitrate->Sjw <= CAN_MAX_S_SJW);
            DEVICE_ASSERT(Bitrate->Presc <= CAN_MAX_S_PRESC);
        }
    }
}

static void Can_Hal_SetFastBitrate(CAN_Type *Base, const Can_BitrateParamsType *Bitrate)
{
    if ((Bitrate != NULL_PTR) && (Bitrate->Seg1 <= CAN_MAX_F_SEG_1) &&\
        (Bitrate->Seg2 <= CAN_MAX_F_SEG_2) && (Bitrate->Sjw <= CAN_MAX_F_SJW))
    {
        Can_Reg_SetDBTP(Base, ((uint32)Bitrate->Presc << CAN_DBTP_DBRP_Pos) |
            ((uint32)Bitrate->Seg1 << CAN_DBTP_DTSEG1_Pos) |
            ((uint32)Bitrate->Seg2 << CAN_DBTP_DTSEG2_Pos) | (uint32)Bitrate->Sjw);
    }
    else
    {
        DEVICE_ASSERT(Bitrate != NULL_PTR);
        if (Bitrate != NULL_PTR)
        {
            DEVICE_ASSERT(Bitrate->Seg1 <= CAN_MAX_F_SEG_1);
            DEVICE_ASSERT(Bitrate->Seg2 <= CAN_MAX_F_SEG_2);
            DEVICE_ASSERT(Bitrate->Sjw <= CAN_MAX_F_SJW);
        }
    }
}

static void Can_Hal_ReleaseBuffer(CAN_Type *Base, uint8 BufferIndex, Can_MramType Location)
{
    uint32 Val;

    switch (Location)
    {
    case DEDICATED_RX_BUFFER:
        if (BufferIndex >= 32U)
        {
            Val = (1UL << (BufferIndex - 32U));
            Can_Reg_SetNewData2(Base, Val);
        }
        else
        {
            Val = (1UL << BufferIndex);
            Can_Reg_SetNewData1(Base, Val);
        }
        break;

    case RX_FIFO0:
        Can_Reg_SetRXF0A(Base, BufferIndex);
        break;

    case RX_FIFO1:
        Can_Reg_SetRXF1A(Base, BufferIndex);
        break;

    case TX_EVENT_FIFO:
        Can_Reg_SetTXEFA(Base, BufferIndex);
        break;

    default:
        /* Not run here */
        break;
    }
}

static void Can_Hal_InitEnable(CAN_Type *Base, boolean En)
{
    uint32 TicksDelayDuration;
    uint32 CurrentTicks;
    uint32 ElapsedTicks = 0U;

    if (TRUE == En)
    {
        Can_Reg_SetINIT(Base, TRUE);
        TicksDelayDuration = OsIf_MicrosToTicks(CAN_WAIT_TIMEOUT);
        CurrentTicks = OsIf_GetCounter();
        while (TRUE != Can_Reg_GetInitState(Base))
        {
            ElapsedTicks += OsIf_GetElapsed(&CurrentTicks);
            /* Loop timeout */
            if (ElapsedTicks >= TicksDelayDuration)
            {
                break;
            }
        }
        DEVICE_ASSERT(ElapsedTicks < TicksDelayDuration);
        /* enable write access to the protected configuration registers */
        Can_Reg_SetCCE(Base, TRUE);
    }
    else
    {
        Can_Reg_SetCCE(Base, FALSE);
        Can_Reg_SetINIT(Base, FALSE);
    }
}

/*PRQA S 1505 ++ # AMCE_CAN_1505. */
Can_HalStateType Can_Hal_GetControllerState(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type const *Base;
    Can_HalStateType State;
    boolean Flag;

    Base = Can_Hal_GetBaseLocal(Instance);
    Flag = Can_Reg_GetInitState(Base);
    Flag |= Can_Reg_IsBusOff(Base);
    if (TRUE == Flag)
    {
        State = CAN_STATE_STOP;
    }
    else
    {
        State = CAN_STATE_RUNNING;
    }

    return State;
}
/*PRQA S 1505 -- # AMCE_CAN_1505. */

static uint32 *Can_Hal_MRAMCalculate(uint8 Instance, uint8 Index, Can_MramType Location)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    /* Table of message RAM base address */
    const uint32 RamBases[CAN_GROUP_MAX] = CAN_MRAM_BASE;
    Can_DeviceType const *CanDev;
    CAN_Type const *Base;
    uint32 RamBase;
    uint32 Address = 0xFFFFFFFFU;

    Base = Can_Hal_GetBaseLocal(Instance);
    CanDev = Can_Hal_GetDevice(Instance);
    if (Instance < CAN_GROUP0_INSTANCE_MAX)
    {
        RamBase = RamBases[0];
    }
    else
    {
        RamBase = RamBases[1];
    }

    switch (Location)
    {
    case DEDICATED_RX_BUFFER:
        Address = (Base->RXBC & CAN_RXBC_RBSA_Msk) \
                  + (((uint32)CanDataSizeChange[CanDev->mramSizeStatus.rxDbufferDataSize] + 8U) * Index);
        break;

    case RX_FIFO0:
        Address = (Base->RXF0C & CAN_RXF0C_F0SA_Msk) \
                  + (((uint32)CanDataSizeChange[CanDev->mramSizeStatus.rxFifo0DataSize] + 8U) * Index);
        break;

    case RX_FIFO1:
        Address = (Base->RXF1C & CAN_RXF1C_F1SA_Msk) \
                  + (((uint32)CanDataSizeChange[CanDev->mramSizeStatus.rxFifo1DataSize] + 8U) * Index);
        break;

    case TX_BUFFER:
        Address = (Base->TXBC & CAN_TXBC_TBSA_Msk) \
                  + (((uint32)CanDataSizeChange[CanDev->mramSizeStatus.txBufferDataSize] + 8U) * Index);
        break;

    case TX_EVENT_FIFO:
        Address = (Base->TXEFC & CAN_TXEFC_EFSA_Msk) + ((uint32)Index * 8U);
        break;

    case STD_FILTER:
        Address = (Base->SIDFC & CAN_SIDFC_FLSSA_Msk) + ((uint32)Index * 4U);
        break;

    case EXT_FILTER:
        Address = (Base->XIDFC & CAN_XIDFC_FLESA_Msk) + ((uint32)Index * 8U);
        break;

    default:
        /* can not run here */
        break;
    }
    Address = Address + RamBase;

    return (uint32 *)Address;
}

static void Can_Hal_SetMRAMConfig(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);

    Can_DeviceType const *CanDev;
    CAN_Type *Base;

    Base = Can_Hal_GetBaseLocal(Instance);
    CanDev = Can_Hal_GetDevice(Instance);
    /*set element number register*/
    Can_Reg_SetMRAMSize(Base, &CanDev->mramSizeStatus);
    /*set element start address register*/
    Can_Reg_SetMRAMAdress(Base, CanDev->mramAddresss);
}

static void Can_Hal_MRAMInit(uint8 Instance, const Can_MramAddressType *const mramAddresss)
{
    uint32 endAddress;
    uint32 offset;
    uint32 i;
    uint32 *addr;

    offset = mramAddresss->stdFilterStartAddress;
    endAddress = mramAddresss->endAddress;
   /* check can mram valid and memset to 0 */
    if (Instance < CAN_GROUP0_INSTANCE_MAX)
    {
        DEVICE_ASSERT((endAddress * 4UL) <= CAN_MRAM0_MAX);
        /*PRQA S 0488,3440,3387 ++ # AMCE_CAN_0488, AMCE_CAN_3440, AMCE_CAN_3387. */
        addr = (uint32 *)CAN_MRAM0_BASE + offset;
        for (i = 0U; i < (endAddress - offset); i++)
        {
            *addr++ = 0x00000000U;
        }
        /*PRQA S 0488,3440,3387 -- # AMCE_CAN_0488, AMCE_CAN_3440, AMCE_CAN_3387. */
    }
    else
    {
        DEVICE_ASSERT((endAddress * 4UL) <= CAN_MRAM1_MAX);
        /*PRQA S 0488,3440,3387 ++ # AMCE_CAN_0488, AMCE_CAN_3440, AMCE_CAN_3387.*/
        addr = (uint32 *)CAN_MRAM1_BASE + offset;
        for (i = 0U; i < (endAddress - offset); i++)
        {
            *addr++ = 0x00000000U;
        }
        /*PRQA S 0488,3440,3387 -- # AMCE_CAN_0488, AMCE_CAN_3440, AMCE_CAN_3387.*/
    }
}

static void Can_Hal_GetMRAMConfig(uint8 Instance, const Can_HalConfigType *ConfigPtr)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    /* can group mram address offset */
    static uint32 CanRamAddrOffset[CAN_GROUP_MAX];
    Can_DeviceType *CanDev;
    Can_MramAddressType mramAddresss;
    uint32 i;

    if (Instance < CAN_INSTANCE_MAX)
    {
        CanDev = Can_Hal_GetDevice(Instance);
        CanDev->mramSizeStatus.txFifoNumber = 0U;
        CanDev->mramSizeStatus.txDbufferNumber = 0U;
        CanDev->mramSizeStatus.stdFilterNumber = 0U;
        CanDev->mramSizeStatus.extFilterNumber = 0U;
        CanDev->mramSizeStatus.rxFifo0Number = 0U;
        CanDev->mramSizeStatus.rxFifo1Number = 0U;
        CanDev->mramSizeStatus.rxDbufferNumber = 0U;
        CanDev->mramSizeStatus.txEventFifoNumber = ConfigPtr->TxEventFifoNum;
        CanDev->mramSizeStatus.rxDbufferDataSize = (uint8)ConfigPtr->DataSize.rxDbufferDataSize;
        CanDev->mramSizeStatus.rxFifo0DataSize = (uint8)ConfigPtr->DataSize.rxFifo0DataSize;
        CanDev->mramSizeStatus.rxFifo1DataSize = (uint8)ConfigPtr->DataSize.rxFifo1DataSize;
        CanDev->mramSizeStatus.txBufferDataSize = (uint8)ConfigPtr->DataSize.txBufferDataSize;
        /* 1.get element number */
        /* 2.bufferid map to hw buffer index && hw buffer type*/
        for (i = 0U; i < ConfigPtr->Filters.FiltersNum; i++)
        {
            if (STD_FRAME == ConfigPtr->Filters.FiltersParamsPtr[i].FrameType)
            {
                CanDev->mramSizeStatus.stdFilterNumber++;
            }
            else
            {
                CanDev->mramSizeStatus.extFilterNumber++;
            }
        }
        for (i = 0U; i < CanDev->RxBuffersNum; i++)
        {
            if (ConfigPtr->RxBufferConfigPtr[i].DepthNum > CAN_DEDICATED_BUFFER_DEPTH_NUM)
            {
                if (0U == CanDev->mramSizeStatus.rxFifo0Number)
                {
                    CanDev->RxBuffersInfoPtr[i].BufferType = (uint8)RX_BUF_TYPE_FIFO0;
                    CanDev->RxBuffersInfoPtr[i].BufferIndex = 0;
                    CanDev->mramSizeStatus.rxFifo0Number = ConfigPtr->RxBufferConfigPtr[i].DepthNum;
                }
                else
                {
                    CanDev->RxBuffersInfoPtr[i].BufferType = (uint8)RX_BUF_TYPE_FIFO1;
                    CanDev->RxBuffersInfoPtr[i].BufferIndex = 0;
                    CanDev->mramSizeStatus.rxFifo1Number = ConfigPtr->RxBufferConfigPtr[i].DepthNum;
                }
            }
            else
            {
                CanDev->RxBuffersInfoPtr[i].BufferType = (uint8)RX_BUF_TYPE_DEDICATED;
                CanDev->RxBuffersInfoPtr[i].BufferIndex = CanDev->mramSizeStatus.rxDbufferNumber;
                CanDev->mramSizeStatus.rxDbufferNumber++;
            }
        }

        for (i = 0U; i < CanDev->TxBuffersNum; i++)
        {
            if (CAN_DEDICATED_BUFFER_DEPTH_NUM == ConfigPtr->TxBufferConfigPtr[i].DepthNum)
            {
                CanDev->TxBuffersInfoPtr[i].BufferType = (uint8)TX_BUF_TYPE_DEDICATED;
                CanDev->TxBuffersInfoPtr[i].BufferIndex = CanDev->mramSizeStatus.txDbufferNumber;
                CanDev->mramSizeStatus.txDbufferNumber++;
            }
            else
            {
                CanDev->TxBuffersInfoPtr[i].BufferType = (uint8)TX_BUF_TYPE_FIFO;
                CanDev->mramSizeStatus.txFifoNumber = ConfigPtr->TxBufferConfigPtr[i].DepthNum;
            }
        }

        for (i = 0U; i < CanDev->TxBuffersNum; i++)
        {
            if ((uint8)TX_BUF_TYPE_FIFO == CanDev->TxBuffersInfoPtr[i].BufferType)
            {
                CanDev->TxBuffersInfoPtr[i].BufferIndex = CanDev->mramSizeStatus.txDbufferNumber;
            }
        }

        /* get element start address */
        CanDev->mramAddresss = CanMramAddresss[Instance];
        if (0U != CanDev->mramAddresss->endAddress)
        {
            mramAddresss.stdFilterStartAddress = CanDev->mramAddresss->stdFilterStartAddress;
        }
        else
        {
            mramAddresss.stdFilterStartAddress = CanRamAddrOffset[CanDev->GroupIndex];
        }
        mramAddresss.extFilterStartAddress = mramAddresss.stdFilterStartAddress \
                                             + CanDev->mramSizeStatus.stdFilterNumber;
        if (CanDev->mramSizeStatus.rxDbufferNumber != 0U)
        {
            mramAddresss.rxDbufferStartAddress = mramAddresss.extFilterStartAddress \
                                                 + ((uint32) CanDev->mramSizeStatus.extFilterNumber * 2UL);
            mramAddresss.rxFifo0StartAddress = mramAddresss.rxDbufferStartAddress \
                 + ((((uint32)CanDataSizeChange[CanDev->mramSizeStatus.rxDbufferDataSize] / 4UL) + 2UL) \
                                                  * CanDev->mramSizeStatus.rxDbufferNumber);
        }
        else
        {
            mramAddresss.rxDbufferStartAddress = 0U;
            mramAddresss.rxFifo0StartAddress = mramAddresss.extFilterStartAddress \
                                               + ((uint32)CanDev->mramSizeStatus.extFilterNumber * 2UL);
        }
        mramAddresss.rxFifo1StartAddress = mramAddresss.rxFifo0StartAddress \
                                 + ((((uint32)CanDataSizeChange[CanDev->mramSizeStatus.rxFifo0DataSize] / 4UL) + 2UL) \
                                              * CanDev->mramSizeStatus.rxFifo0Number);
        mramAddresss.txBufferStartAddress = mramAddresss.rxFifo1StartAddress \
                                 + ((((uint32)CanDataSizeChange[CanDev->mramSizeStatus.rxFifo1DataSize] / 4UL) + 2UL) \
                                               * CanDev->mramSizeStatus.rxFifo1Number);
        mramAddresss.txEventfifoStartAddress = mramAddresss.txBufferStartAddress \
                                + ((((uint32)CanDataSizeChange[CanDev->mramSizeStatus.txBufferDataSize] / 4UL) + 2UL) \
                     * ((uint32)CanDev->mramSizeStatus.txDbufferNumber + (uint32)CanDev->mramSizeStatus.txFifoNumber));
        mramAddresss.endAddress = mramAddresss.txEventfifoStartAddress + \
            ((uint32)CanDev->mramSizeStatus.txEventFifoNumber * 2UL);
        if (0U != CanDev->mramAddresss->endAddress)
        {
            DEVICE_ASSERT(!((CanDev->mramAddresss->stdFilterStartAddress != mramAddresss.stdFilterStartAddress) || \
                    (CanDev->mramAddresss->extFilterStartAddress != mramAddresss.extFilterStartAddress) || \
                    (CanDev->mramAddresss->rxDbufferStartAddress != mramAddresss.rxDbufferStartAddress) || \
                    (CanDev->mramAddresss->rxFifo0StartAddress != mramAddresss.rxFifo0StartAddress) || \
                    (CanDev->mramAddresss->rxFifo1StartAddress != mramAddresss.rxFifo1StartAddress) || \
                    (CanDev->mramAddresss->txBufferStartAddress != mramAddresss.txBufferStartAddress) || \
                    (CanDev->mramAddresss->txEventfifoStartAddress != mramAddresss.txEventfifoStartAddress) || \
                    ((CanDev->mramSizeStatus.txEventFifoNumber != 0U) && \
                     (CanDev->mramAddresss->endAddress != mramAddresss.endAddress))));
        }
        else
        {
            CanDev->mramAddresss->stdFilterStartAddress = mramAddresss.stdFilterStartAddress;
            CanDev->mramAddresss->extFilterStartAddress = mramAddresss.extFilterStartAddress;
            CanDev->mramAddresss->rxDbufferStartAddress = mramAddresss.rxDbufferStartAddress;
            CanDev->mramAddresss->rxFifo0StartAddress = mramAddresss.rxFifo0StartAddress;
            CanDev->mramAddresss->rxFifo1StartAddress = mramAddresss.rxFifo1StartAddress;
            CanDev->mramAddresss->txBufferStartAddress = mramAddresss.txBufferStartAddress;
            CanDev->mramAddresss->txEventfifoStartAddress = mramAddresss.txEventfifoStartAddress;
            CanDev->mramAddresss->endAddress = mramAddresss.endAddress;
            CanRamAddrOffset[CanDev->GroupIndex] = mramAddresss.endAddress;
        }
        Can_Hal_MRAMInit(Instance, &mramAddresss);
    }
}

static void Can_Hal_GetMsgInfoHw(Can_MessageInfoType *Info, const uint32 *Buf, boolean DmaEn)
{
    DEVICE_ASSERT(Info != NULL_PTR);
    DEVICE_ASSERT(Buf != NULL_PTR);

    uint32 head1, head2;
    uint8 DataLength = 0U, i;
    uint8 DlcValue;
    const uint32 *data = Buf;
    if ((Info != NULL_PTR) && (Buf != NULL_PTR))
    {
        head1 = *data;
        /*PRQA S 4394,4342 ++ # AMCE_CAN_4394, AMCE_CAN_4342.*/
        Info->MsgIdType = (Can_MessageIdType)((head1 & CAN_INFO_XTD_Msk) >> CAN_INFO_XTD_Pos);
        /*PRQA S 4394,4342 -- # AMCE_CAN_4394, AMCE_CAN_4342.*/
        if (Info->MsgIdType != CAN_STANDARD_MSG)
        {
            Info->MsgId = head1 & CAN_INFO_EXT_ID_Msk;
        }
        else
        {
            Info->MsgId = (head1 & CAN_INFO_STD_ID_Msk) >> CAN_INFO_STD_ID_Pos;
        }
        Info->MsgRtr = (boolean)((head1 & CAN_INFO_RTR_Msk) >> CAN_INFO_RTR_Pos);
        data++;
        head2 = *data;
        Info->EfcAnmf = (uint8)((head2 & CAN_INFO_ANMF_Msk) >> CAN_INFO_ANMF_Pos);
        Info->MmFidx = (uint8)((head2 & CAN_INFO_FIDX_Msk) >> CAN_INFO_FIDX_Pos);
        Info->MsgFdf = (boolean)((head2 & CAN_INFO_FDF_Msk) >> CAN_INFO_FDF_Pos);
        Info->MsgBrs = (boolean)((head2 & CAN_INFO_BRS_Msk) >> CAN_INFO_BRS_Pos);
        DlcValue = (uint8)((head2 & CAN_INFO_DLC_Msk) >> CAN_INFO_DLC_Pos);
        Info->MsgRts = (uint16)head2;
        data++;
        if (((uint8)CAN_CTRL_DATA_FRAME == Info->MsgRtr) && (DlcValue > 0U))
        {
            DEVICE_ASSERT(Info->MsgDataPtr != NULL_PTR);
            DataLength = s_dlcToBytes[DlcValue];
            if ((Info->MsgFdf == FALSE) && (DataLength > 8U))
            {
                DataLength = 8U;
            }
            Info->MsgDataLen = DataLength;
        }
        if (TRUE != DmaEn)
        {
            /* PRQA S 0310,3305 ++ # AMCE_CAN_0310, AMCE_CAN_3305. */
            for (i = 0U; i < DataLength; i += 4U)
            {
                *(uint32 *)(&Info->MsgDataPtr[i]) = *data;
                data++;
            }
            /* PRQA S 0310,3305 -- # AMCE_CAN_0310, AMCE_CAN_3305. */
        }
        else
        {
            /* PRQA S 0311 ++ # AMCE_CAN_0311. */
            Info->MsgDataPtr = (uint8 *)data;
            /* PRQA S 0311 -- # AMCE_CAN_0311. */
        }
    }
}

static void Can_Hal_CtrlInit(uint8 Instance, const Can_HalConfigType *ConfigPtr)
{
    uint32 Val;
    uint8 i;

    /* Configure CAN module ctrl clock and reset */
    if ((0U == CanCtrlState.GroupInitRef[CAN_GROUP0]) && (0U == CanCtrlState.GroupInitRef[CAN_GROUP1]))
    {
        (void)Ckgen_Hal_EnablePeriphClk(CKGEN_CAN_CTRL_BUS_CLK, TRUE);
        Rcm_Hal_SetResetState(RCM_RESET_ID_CAN_CTRL, RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState(RCM_RESET_ID_CAN_CTRL, RCM_RESET_STATE_DEASSERT);
    }
    if (Instance < CAN_GROUP0_INSTANCE_MAX)
    {
        if (0U == CanCtrlState.GroupInitRef[CAN_GROUP0])
        {
            for (i = 0; i < CAN_GROUP0_INSTANCE_MAX; i++)
            {
                /* Enable CAN group clock */
                (void)Ckgen_Hal_EnablePeriphClk(CanCkgenBusClock[i], TRUE);
                Rcm_Hal_SetResetState(CanClockReset[i], RCM_RESET_STATE_ASSERT);
                Rcm_Hal_SetResetState(CanClockReset[i], RCM_RESET_STATE_DEASSERT);
            }
        }
        CanCtrlState.GroupInitRef[CAN_GROUP0] |= (uint8)BIT(Instance);
    }
    else
    {
        if (0U == CanCtrlState.GroupInitRef[CAN_GROUP1])
        {
            for (i = CAN_GROUP0_INSTANCE_MAX; i < CAN_INSTANCE_MAX; i++)
            {
                /* Enable CAN group clock */
                (void)Ckgen_Hal_EnablePeriphClk(CanCkgenBusClock[i], TRUE);
                Rcm_Hal_SetResetState(CanClockReset[i], RCM_RESET_STATE_ASSERT);
                Rcm_Hal_SetResetState(CanClockReset[i], RCM_RESET_STATE_DEASSERT);
            }
        }
        CanCtrlState.GroupInitRef[CAN_GROUP1] |= (uint8)BIT(Instance);
    }

    /* channele DMU enable */
    Val = Can_Reg_GetDMU(CAN_CTRL);
    if (ConfigPtr->RxFifoBufDmaConfigNum != 0U)
    {
        Val |= BIT(Instance);
    }
    else
    {
        Val &= ~BIT(Instance);
    }
    Can_Reg_SetDMU(CAN_CTRL, (uint8)Val);

    /* ECC is enable\disable */
    Val = Can_Reg_GetECC(CAN_CTRL);
    if (TRUE == ConfigPtr->EccEn)
    {
        Val |= BIT(Instance);
    }
    else
    {
        Val &= ~BIT(Instance);
    }
    Can_Reg_SetECC(CAN_CTRL, (uint8)Val);

    /* channele low power wake up enable */
    Val = Can_Reg_GetWakeup(CAN_CTRL);
    if (NULL_PTR != ConfigPtr->WakeupIrqCallback)
    {
        Val |= BIT(Instance);
    }
    else
    {
        Val &= ~BIT(Instance);
    }
    Can_Reg_SetWakeup(CAN_CTRL, (uint8)Val);

    /* channele low power filter enable */
    Val = Can_Reg_GetLowPassFilter(CAN_CTRL);
    if (NULL_PTR != ConfigPtr->WakeupIrqCallback)
    {
        Val |= BIT(Instance);
    }
    else
    {
        Val &= ~BIT(Instance);
    }
    Can_Reg_SetLowPassFilter(CAN_CTRL, (uint8)Val);
}

static void Can_Hal_CtrlDeInit(uint8 Instance)
{
    uint8 i;

    if (Instance < CAN_INSTANCE_MAX)
    {
        Rcm_Hal_SetResetState(CanClockReset[Instance], RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState(CanClockReset[Instance], RCM_RESET_STATE_DEASSERT);
    }
    if (Instance < CAN_GROUP0_INSTANCE_MAX)
    {
        CanCtrlState.GroupInitRef[CAN_GROUP0] &= ~(uint8)BIT(Instance);
        if (0U == CanCtrlState.GroupInitRef[CAN_GROUP0])
        {
            for (i = 0U; i < CAN_GROUP0_INSTANCE_MAX; i++)
            {
                /* Enable CAN group clock */
                (void)Ckgen_Hal_EnablePeriphClk(CanCkgenBusClock[i], FALSE);
            }
        }
    }
    else
    {
        CanCtrlState.GroupInitRef[CAN_GROUP1] &= ~(uint8)BIT(Instance);
        if (0U == CanCtrlState.GroupInitRef[CAN_GROUP1])
        {
            for (i = CAN_GROUP0_INSTANCE_MAX; i < CAN_INSTANCE_MAX; i++)
            {
                /* Enable CAN group clock */
                (void)Ckgen_Hal_EnablePeriphClk(CanCkgenBusClock[i], FALSE);
            }
        }
    }

    /* Configure CAN module ctrl clock and reset */
    if ((0U == CanCtrlState.GroupInitRef[CAN_GROUP0]) && (0U == CanCtrlState.GroupInitRef[CAN_GROUP1]))
    {
        Rcm_Hal_SetResetState(RCM_RESET_ID_CAN_CTRL, RCM_RESET_STATE_ASSERT);
        Rcm_Hal_SetResetState(RCM_RESET_ID_CAN_CTRL, RCM_RESET_STATE_DEASSERT);
        (void)Ckgen_Hal_EnablePeriphClk(CKGEN_CAN_CTRL_BUS_CLK, FALSE);
    }
}

static void Can_Hal_SetStdFilter(uint8 Instance, uint8 Index, const Can_FilterParamsType *ConfigPtr)
{
    uint32 *FilterAddress;
    uint32 filterValue1;
    uint8 fec = (uint8)ConfigPtr->FEC;

    if ((uint8)STORE_IN_RXBUFFER == fec)
    {
        DEVICE_ASSERT(CanDevice[Instance]->mramSizeStatus.rxDbufferNumber != 0U);
    }
    filterValue1 = ((((uint32)ConfigPtr->Type & 0x3UL) << 30UL) | (((uint32)fec & 0x7UL) << 27UL) | \
                    ((ConfigPtr->ID1 & 0x7FFUL) << 16UL) | (ConfigPtr->ID2 & 0x7FFUL));
    FilterAddress = Can_Hal_MRAMCalculate(Instance, Index, STD_FILTER);
    *FilterAddress = filterValue1;
}

static void Can_Hal_SetExtFilter(uint8 Instance, uint8 Index, const Can_FilterParamsType *ConfigPtr)
{
    uint32 *FilterAddress;
    uint32 filterValue1, filterValue2;
    uint8 fec = (uint8)ConfigPtr->FEC;

    if ((uint8)STORE_IN_RXBUFFER == fec)
    {
        DEVICE_ASSERT(CanDevice[Instance]->mramSizeStatus.rxDbufferNumber != 0U);
    }
    filterValue1 = ((((uint32)fec & 0x7UL) << 29UL) | (ConfigPtr->ID1 & 0x1FFFFFFFUL));
    filterValue2 = ((((uint32)ConfigPtr->Type & 0x3UL) << 30UL) | (ConfigPtr->ID2 & 0x1FFFFFFFUL));
    FilterAddress = Can_Hal_MRAMCalculate(Instance, Index, EXT_FILTER);
    *FilterAddress = filterValue1;
    FilterAddress++;
    *FilterAddress = filterValue2;
}

static void Can_Hal_SetFilters(uint8 Instance, const Can_HalConfigType *ConfigPtr)
{
    CAN_Type *Base;
    uint8 stdIndex = 0, extIndex = 0;
    uint8 i;
    const Can_GlobalFilterConfigType *GlobalFilter;

    Base = Can_Hal_GetBaseLocal(Instance);
    GlobalFilter = &ConfigPtr->Filters.GlobalFilter;
    Can_Reg_SetGlobalFilter(Base, (uint8)GlobalFilter->std, (uint8)GlobalFilter->ext, \
        (uint8)GlobalFilter->rejectStdRemote, (uint8)GlobalFilter->rejectExtRemote);
    if (0U != ConfigPtr->Filters.XIDAM)
    {
        Can_Reg_SetXIDAM(Base, ConfigPtr->Filters.XIDAM);
    }

    for (i = 0; i < ConfigPtr->Filters.FiltersNum; i++)
    {
        if (STD_FRAME == ConfigPtr->Filters.FiltersParamsPtr[i].FrameType)
        {
            Can_Hal_SetStdFilter(Instance, stdIndex, &ConfigPtr->Filters.FiltersParamsPtr[i]);
            stdIndex++;
        }
        else
        {
            Can_Hal_SetExtFilter(Instance, extIndex, &ConfigPtr->Filters.FiltersParamsPtr[i]);
            extIndex++;
        }
    }
}

static void Can_Hal_SetRxTxConfig(uint8 Instance, const Can_HalConfigType *ConfigPtr)
{
    const Can_DeviceType *CanDev;
    CAN_Type *Base;
    uint32 i, j;
    uint32 TXBTIE = 0U;
    uint32 TXBCIE = 0U;
    uint8 tmpBufferIndex;

    Base = Can_Hal_GetBaseLocal(Instance);
    CanDev = Can_Hal_GetDevice(Instance);
    Can_Hal_SetFilters(Instance, ConfigPtr);
    for (i = 0; i < CanDev->RxBuffersNum; i++)
    {
        if (CanDev->RxBuffersInfoPtr[i].BufferType == (uint8)RX_BUF_TYPE_FIFO0)
        {
            Can_Reg_SetRxFifo0Mode(Base, ConfigPtr->RxOverWrite);
        }
        else if (CanDev->RxBuffersInfoPtr[i].BufferType == (uint8)RX_BUF_TYPE_FIFO1)
        {
            Can_Reg_SetRxFifo1Mode(Base, ConfigPtr->RxOverWrite);
        }
        else
        {
            /* nothing to do*/
        }
    }

    for (i = 0; i < CanDev->TxBuffersNum; i++)
    {
        if ((uint8)TX_BUF_TYPE_DEDICATED == CanDev->TxBuffersInfoPtr[i].BufferType)
        {
            if (TRUE == ConfigPtr->TxBufferConfigPtr[i].TxCmpIrqEn)
            {
                TXBTIE |= BIT(CanDev->TxBuffersInfoPtr[i].BufferIndex);
            }
            if (TRUE == ConfigPtr->TxBufferConfigPtr[i].TxCnlIrqEn)
            {
                TXBCIE |= BIT(CanDev->TxBuffersInfoPtr[i].BufferIndex);
            }
        }
        else
        {
            tmpBufferIndex = CanDev->TxBuffersInfoPtr[i].BufferIndex;
            for (j = 0; j < ConfigPtr->TxBufferConfigPtr[i].DepthNum; j++)
            {
                if (TRUE == ConfigPtr->TxBufferConfigPtr[i].TxCmpIrqEn)
                {
                    TXBTIE |= BIT(tmpBufferIndex + j);
                }
                if (TRUE == ConfigPtr->TxBufferConfigPtr[i].TxCnlIrqEn)
                {
                    TXBCIE |= BIT(tmpBufferIndex + j);
                }
            }
        }
    }
    /* set tx intereupt enable masks */
    Can_Reg_SetTxIE(Base, TXBTIE);
    Can_Reg_SetTxCancelIE(Base, TXBCIE);
}

static void Can_Hal_ISR(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base;
    Can_DeviceType const *CanDev;
    Can_IrqCBParams CbArgs;
    uint32 Flag, IrqEn;

#if defined CAN_PERFORMANCE_TEST
    Can_PerformaceTestType *CanDevTest = &CanPerformaceTest;
    uint64 StartTime, StopTime, IntvlTime;

    StartTime = OsIf_GetCurrentTimeUS();
#endif
    Base = Can_Hal_GetBaseLocal(Instance);
    CanDev = Can_Hal_GetDevice(Instance);
    Flag = Can_Reg_GetIR(Base);
    IrqEn = Can_Reg_GetIE(Base);
    CbArgs.CommonIrqFlagMasks = Flag & IrqEn;
    Can_Reg_ClearIR(Base, CbArgs.CommonIrqFlagMasks);
    CbArgs.RxNewData1 = Can_Reg_GetNewData1(Base);
    CbArgs.RxNewData2 = Can_Reg_GetNewData2(Base);
    CbArgs.TxBTO = Can_Reg_TransmitOccurred(Base);

    if (NULL_PTR != CanDev->IrqCallback)
    {
        CbArgs.Instance = Instance;
        CanDev->IrqCallback(&CbArgs);
    }
#if defined CAN_PERFORMANCE_TEST
    if (CbArgs.CommonIrqFlagMasks & (CAN_IR_RF0N_Msk | CAN_IR_RF1N_Msk | CAN_IR_DRX_Msk))
    {
        StopTime = OsIf_GetCurrentTimeUS();
        IntvlTime = StopTime - StartTime;
        CanDevTest->TimeRxAvg = IntvlTime;
        if (IntvlTime < 200U)
        {
            if (!CanDevTest->TimeRxMin || CanDevTest->TimeRxMin > IntvlTime)
            {
                CanDevTest->TimeRxMin = IntvlTime;
            }
            if (CanDevTest->TimeRxMax < IntvlTime)
            {
                CanDevTest->TimeRxMax = IntvlTime;
            }
        }
    }
#endif
}

static void Can_Hal_DmuISR(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    Can_DeviceType const *CanDev;
    CAN_Type *Base;
    Can_DmuIrqCBParams CbArgs;

    Base = Can_Hal_GetBaseLocal(Instance);
    CanDev = Can_Hal_GetDevice(Instance);
    if (NULL_PTR != CanDev->DmuIrqCallback)
    {
        CbArgs.Instance = Instance;
        CbArgs.DmuIrqFlagMasks = Can_Reg_GetDMUIR(Base);
        Can_Reg_ClearDMUIR(Base, CbArgs.DmuIrqFlagMasks);
        CanDev->WakeupIrqCallback(&CbArgs);
    }
}
#endif

#ifndef CAN_SDK_NON_EXTENDED_API
void Can_Hal_StartNextDma(uint8 Instance, uint8 FifoId, uint32 DmaDstAddr)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    Can_DeviceType *CanDev;
    Dma_ChannelAddrType *ChannelAddr;
    uint8 ChannelId;

    CanDev = Can_Hal_GetDevice(Instance);
    ChannelAddr = &CanDev->RxFifoDmaChannelAddr[FifoId];
    ChannelId = CanDev->RxFifoDmaChannel[FifoId];
    ChannelAddr->DestStartAddr = DmaDstAddr;
    ChannelAddr->DestEndAddr = DmaDstAddr +\
        ((((uint32)ChannelAddr->Length) / (1U << (uint32)DMA_TRANSFER_UNIT_4B)) * (uint32)ChannelAddr->DestOffset);
    (void)Dma_Hal_StopCh(ChannelId);
    Dma_Hal_EnableChIrq(ChannelId, (uint8)DMA_IRQ_NONE);
    Dma_Hal_UpdateChAddr(ChannelId, ChannelAddr);
    Dma_Hal_EnableChIrq(ChannelId, (uint8)(DMA_FINISH_IRQ) | (uint8)(DMA_ERROR_IRQ));
    (void)Dma_Hal_StartCh(ChannelId);
}
#endif

CAN_Type *Can_Hal_GetBase(uint8 Instance)
{
    return Can_Hal_GetBaseLocal(Instance);
}

LOCAL_INLINE CAN_Type *Can_Hal_GetBaseLocal(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    CAN_Type *Base = NULL_PTR;

    if (Instance < CAN_INSTANCE_MAX)
    {
        Base = CanBase[Instance];
    }

    return Base;
}

LOCAL_INLINE Can_DeviceType *Can_Hal_GetDevice(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    Can_DeviceType *CanDev = NULL_PTR;

    if (Instance < CAN_INSTANCE_MAX)
    {
        CanDev = CanDevice[Instance];
    }

    return CanDev;
}

static void Can_Hal_CfgDma(uint8 Instance, uint32 SrcAddr,
                       uint32 Length, Can_RxFifoBufDmaConfigType const *RxFifoBufDmaConfigPtr)
{
    Can_DeviceType *CanDev;
    Dma_ChannelAddrType *ChannelAddr;
    Hal_StatusType Status;
    Dma_TransferConfigType TransferConfig;
    uint8 ChannelId = 0;
    Dma_RequestSourceType DmaReqSrc;
    uint8 FifoId;
    uint32 DstAddr;

    CanDev = Can_Hal_GetDevice(Instance);
    FifoId = RxFifoBufDmaConfigPtr->FifoId;
    DstAddr = RxFifoBufDmaConfigPtr->DmaDstAddr;
    ChannelAddr = &CanDev->RxFifoDmaChannelAddr[FifoId];
    TransferConfig.TriggerMode = FALSE;
    TransferConfig.CircularMode = FALSE;
#if defined (AC7840E)
    TransferConfig.DaisyMode = FALSE;
#endif
    TransferConfig.IrqSrc = (uint8)DMA_FINISH_IRQ | (uint8)DMA_ERROR_IRQ;
    TransferConfig.SrcUnit = DMA_TRANSFER_UNIT_4B;
    TransferConfig.DestUnit = DMA_TRANSFER_UNIT_4B;
    TransferConfig.Type = DMA_TRANSFER_PERIPH2MEM;
    TransferConfig.SrcOffset = 4U;
    TransferConfig.DestOffset = 4U;
    TransferConfig.SrcStartAddr = SrcAddr;
    TransferConfig.DestStartAddr = DstAddr;
    TransferConfig.Length = (uint16)Length;
    TransferConfig.SrcEndAddr = SrcAddr +
        ((Length / (1UL << (uint32)TransferConfig.SrcUnit)) * (uint32)TransferConfig.SrcOffset);
    TransferConfig.DestEndAddr = DstAddr +
        ((Length / (1UL << (uint32)TransferConfig.DestUnit)) * (uint32)TransferConfig.DestOffset);
    TransferConfig.Callback = RxFifoBufDmaConfigPtr->DmaCb;
    TransferConfig.UserArgs = RxFifoBufDmaConfigPtr->DmaCbArgs;
    /*PRQA S 4342,4394 ++ # AMCE_CAN_4342, AMCE_CAN_4394. */
#if defined (AC7843X)
    DmaReqSrc = (Dma_RequestSourceType)((uint8)DMA_REQ_CAN0_RX0 + FifoId + (Instance * 2U));
#elif defined (AC7840X) || defined (AC7842X) || defined (AC7840E)
    DmaReqSrc = (Dma_RequestSourceType)((uint32)DMA_REQ_CAN0_RX + Instance);
#endif
    /*PRQA S 4342,4394 -- # AMCE_CAN_4342, AMCE_CAN_4394. */
    ChannelId = Dma_Hal_GetChIdByReqSrc(DmaReqSrc);
    /* Configure the DMA transfer control */
    Status = Dma_Hal_ConfigCh(ChannelId, &TransferConfig);
    if (NULL_PTR != RxFifoBufDmaConfigPtr->DmaCb)
    {
        Dma_Hal_EnableChIrq(ChannelId, TransferConfig.IrqSrc);
    }
    if (STATUS_SUCCESS == Status)/* config dma success */
    {
        /* Start the DMA channel */
        (void)Dma_Hal_StartCh(ChannelId);
    }
    CanDev->RxFifoDmaChannel[FifoId] = ChannelId;
    ChannelAddr->SrcOffset = TransferConfig.SrcOffset;
    ChannelAddr->SrcStartAddr = TransferConfig.SrcStartAddr;
    ChannelAddr->SrcEndAddr = TransferConfig.SrcEndAddr;
    ChannelAddr->Length = TransferConfig.Length;
    ChannelAddr->DestOffset = TransferConfig.DestOffset;
    ChannelAddr->DestStartAddr = TransferConfig.DestStartAddr;
    ChannelAddr->DestEndAddr = TransferConfig.DestEndAddr;
    DEVICE_ASSERT(STATUS_SUCCESS == Status);
}

static void Can_Hal_DmaDeinit(uint8 Instance)
{
    Can_DeviceType const *CanDev;
    uint8 i;

    CanDev = Can_Hal_GetDevice(Instance);
    /* PRQA S 2877 ++ # AMCE_CAN_2877.*/
    for (i = 0; i < CAN_RX_FIFO_NUM_MAX; i++)
    {
        if (0U != CanDev->RxFifoDmaChannelAddr[i].Length)
        {
            (void)Dma_Hal_StopCh(CanDev->RxFifoDmaChannel[i]);
            Dma_Hal_EnableChIrq(CanDev->RxFifoDmaChannel[i], (uint8)DMA_IRQ_NONE);
        }
    }
    /* PRQA S 2877 -- # AMCE_CAN_2877.*/
}

static void Can_Hal_WakeupISR(uint8 Instance)
{
    DEVICE_ASSERT(Instance < CAN_INSTANCE_MAX);
    const Can_DeviceType *CanDev;
    uint8 CbArg;

    CanDev = Can_Hal_GetDevice(Instance);
    if (NULL_PTR != CanDev->WakeupIrqCallback)
    {
        CbArg = Instance;
        CanDev->WakeupIrqCallback(&CbArg);
    }
}

/*PRQA S 1503,3408 ++ # AMCE_CAN_1503, AMCE_CAN_3408. */
/**
* @brief CAN0 IRQ Handler function.
* @return None
*/
ISR(CAN0_IRQHandler)
{
    Can_Hal_ISR(0U);
}

/**
* @brief CAN0 wakeup IRQ Handler function.
* @return None
*/
ISR(CAN0_Wakeup_IRQHandler)
{
    Can_Hal_WakeupISR(0U);
}

/**
* @brief CAN1 IRQ Handler function.
* @return None
*/
ISR(CAN1_IRQHandler)
{
    Can_Hal_ISR(1U);
}

/**
* @brief CAN1 wakeup IRQ Handler function.
* @return None
*/
ISR(CAN1_Wakeup_IRQHandler)
{
    Can_Hal_WakeupISR(1U);
}

#if defined (AC7840X) || defined (AC7842X)  || defined (AC7843X)
/**
* @brief CAN2 IRQ Handler function.
* @return None
*/
ISR(CAN2_IRQHandler)
{
    Can_Hal_ISR(2U);
}

/**
* @brief CAN2 wakeup IRQ Handler function.
* @return None
*/
ISR(CAN2_Wakeup_IRQHandler)
{
    Can_Hal_WakeupISR(2U);
}

/**
* @brief CAN3 IRQ Handler function.
* @return None
*/
ISR(CAN3_IRQHandler)
{
    Can_Hal_ISR(3U);
}

/**
* @brief CAN3 wakeup IRQ Handler function.
* @return None
*/
ISR(CAN3_Wakeup_IRQHandler)
{
    Can_Hal_WakeupISR(3U);
}

#if defined (AC7842X)  || defined (AC7843X)
/**
* @brief CAN3 IRQ Handler function.
* @return None
*/
ISR(CAN4_IRQHandler)
{
    Can_Hal_ISR(4U);
}

/**
* @brief CAN3 wakeup IRQ Handler function.
* @return None
*/
ISR(CAN4_Wakeup_IRQHandler)
{
    Can_Hal_WakeupISR(4U);
}

/**
* @brief CAN3 IRQ Handler function.
* @return None
*/
ISR(CAN5_IRQHandler)
{
    Can_Hal_ISR(5U);
}

/**
* @brief CAN3 wakeup IRQ Handler function.
* @return None
*/
ISR(CAN5_Wakeup_IRQHandler)
{
    Can_Hal_WakeupISR(5U);
}
#endif
#endif

#if defined (AC7843X)
/**
* @brief CAN0 DMU IRQ Handler function.
* @return None
*/
ISR(CAN0_DMU_IRQHandler)
{
    Can_Hal_DmuISR(0U);
}

/**
* @brief CAN1 DMU IRQ Handler function.
* @return None
*/
ISR(CAN1_DMU_IRQHandler)
{
    Can_Hal_DmuISR(1U);
}

/**
* @brief CAN2 DMU IRQ Handler function.
* @return None
*/
ISR(CAN2_DMU_IRQHandler)
{
    Can_Hal_DmuISR(2U);
}

/**
* @brief CAN3 DMU IRQ Handler function.
* @return None
*/
ISR(CAN3_DMU_IRQHandler)
{
    Can_Hal_DmuISR(3U);
}

/**
* @brief CAN4 DMU IRQ Handler function.
* @return None
*/
ISR(CAN4_DMU_IRQHandler)
{
    Can_Hal_DmuISR(4U);
}

/**
* @brief CAN5 DMU IRQ Handler function.
* @return None
*/
ISR(CAN5_DMU_IRQHandler)
{
    Can_Hal_DmuISR(5U);
}
/*PRQA S 1503,3408 -- # AMCE_CAN_1503, AMCE_CAN_3408. */
#endif
