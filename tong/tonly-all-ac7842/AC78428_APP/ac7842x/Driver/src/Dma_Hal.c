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
* @file Dma_Hal.c
*
* @brief This file provides extern Dma Hal API implement
*
*/

/*==============================================INCLUDE FILES=======================================*/
#include "Ckgen_Hal.h"
#include "Core_Hal.h"
#include "Dma_Hal.h"
#include "OsIf_Critical.h"

/*=====================================SOURCE FILE VERSION INFORMATION==============================*/

/*============================================FILE VERSION CHECKS===================================*/

/*=================================================CONSTANTS========================================*/

/*============================================DEFINES AND MACROS====================================*/

/*===================================================ENUMS==========================================*/
/*!
 * @brief Dma state machine
 */
typedef enum
{
    DMA_STATE_INITED = 0x0U,
    DMA_STATE_UNINITED,
    DMA_STATE_CONFIGURED,
    DMA_STATE_RUNNING,
    DMA_STATE_ERROR
} Dma_StateType;
/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/**
 * @brief Dma channel information type
 */
typedef struct
{
    Dma_RequestSourceType ReqSrc;
    Dma_StateType State;
    uint8 VirtualChannelId; /**< Dma channel ID*/
    Hal_CallbackType Callback;
    void *UserArgs;
} Dma_ChannelInfoType;
/*===========================================VARIABLE DECLARATIONS==================================*/

static Dma_ChannelConfigType DmaVCConfig[DMA_VIRTUAL_CH_MAX];

#if (CONFIG_DMA_CHANNEL0_ENABLE)
/** @brief DMA channel0 information */
static Dma_ChannelInfoType Dma0ChInfo =
{
    DMA_REQ_DISABLE, DMA_STATE_UNINITED, DMA_VIRTUAL_CH_MAX, NULL_PTR, NULL_PTR
};
#endif /* CONFIG_DMA_CHANNEL0_ENABLE */

#if (CONFIG_DMA_CHANNEL1_ENABLE)
/** @brief DMA channel1 information */
static Dma_ChannelInfoType Dma1ChInfo =
{
    DMA_REQ_DISABLE, DMA_STATE_UNINITED, DMA_VIRTUAL_CH_MAX, NULL_PTR, NULL_PTR
};
#endif /* CONFIG_DMA_CHANNEL1_ENABLE */

#if (CONFIG_DMA_CHANNEL2_ENABLE)
/** @brief DMA channel2 information */
static Dma_ChannelInfoType Dma2ChInfo =
{
    DMA_REQ_DISABLE, DMA_STATE_UNINITED, DMA_VIRTUAL_CH_MAX, NULL_PTR, NULL_PTR
};
#endif /* CONFIG_DMA_CHANNEL2_ENABLE */

#if (CONFIG_DMA_CHANNEL3_ENABLE)
/** @brief DMA channel3 information */
static Dma_ChannelInfoType Dma3ChInfo =
{
    DMA_REQ_DISABLE, DMA_STATE_UNINITED, DMA_VIRTUAL_CH_MAX, NULL_PTR, NULL_PTR
};
#endif /* CONFIG_DMA_CHANNEL3_ENABLE */

#if (CONFIG_DMA_CHANNEL4_ENABLE)
/** @brief DMA channel4 information */
static Dma_ChannelInfoType Dma4ChInfo =
{
    DMA_REQ_DISABLE, DMA_STATE_UNINITED, DMA_VIRTUAL_CH_MAX, NULL_PTR, NULL_PTR
};
#endif /* CONFIG_DMA_CHANNEL4_ENABLE */

#if (CONFIG_DMA_CHANNEL5_ENABLE)
/** @brief DMA channel5 information */
static Dma_ChannelInfoType Dma5ChInfo =
{
    DMA_REQ_DISABLE, DMA_STATE_UNINITED, DMA_VIRTUAL_CH_MAX, NULL_PTR, NULL_PTR
};
#endif /* CONFIG_DMA_CHANNEL5_ENABLE */

#if (CONFIG_DMA_CHANNEL6_ENABLE)
/** @brief DMA channel6 information */
static Dma_ChannelInfoType Dma6ChInfo =
{
    DMA_REQ_DISABLE, DMA_STATE_UNINITED, DMA_VIRTUAL_CH_MAX, NULL_PTR, NULL_PTR
};
#endif /* CONFIG_DMA_CHANNEL6_ENABLE */

#if (CONFIG_DMA_CHANNEL7_ENABLE)
/** @brief DMA channel7 information */
static Dma_ChannelInfoType Dma7ChInfo =
{
    DMA_REQ_DISABLE, DMA_STATE_UNINITED, DMA_VIRTUAL_CH_MAX, NULL_PTR, NULL_PTR
};
#endif /* CONFIG_DMA_CHANNEL7_ENABLE */

#if (CONFIG_DMA_CHANNEL8_ENABLE)
/** @brief DMA channel8 information */
static Dma_ChannelInfoType Dma8ChInfo =
{
    DMA_REQ_DISABLE, DMA_STATE_UNINITED, DMA_VIRTUAL_CH_MAX, NULL_PTR, NULL_PTR
};
#endif /* CONFIG_DMA_CHANNEL8_ENABLE */

#if (CONFIG_DMA_CHANNEL9_ENABLE)
/** @brief DMA channel9 information */
static Dma_ChannelInfoType Dma9ChInfo =
{
    DMA_REQ_DISABLE, DMA_STATE_UNINITED, DMA_VIRTUAL_CH_MAX, NULL_PTR, NULL_PTR
};
#endif /* CONFIG_DMA_CHANNEL9_ENABLE */

#if (CONFIG_DMA_CHANNEL10_ENABLE)
/** @brief DMA channel10 information */
static Dma_ChannelInfoType Dma10ChInfo =
{
    DMA_REQ_DISABLE, DMA_STATE_UNINITED, DMA_VIRTUAL_CH_MAX, NULL_PTR, NULL_PTR
};
#endif /* CONFIG_DMA_CHANNEL10_ENABLE */

#if (CONFIG_DMA_CHANNEL11_ENABLE)
/** @brief DMA channel11 information */
static Dma_ChannelInfoType Dma11ChInfo =
{
    DMA_REQ_DISABLE, DMA_STATE_UNINITED, DMA_VIRTUAL_CH_MAX, NULL_PTR, NULL_PTR
};
#endif /* CONFIG_DMA_CHANNEL11_ENABLE */

#if (CONFIG_DMA_CHANNEL12_ENABLE)
/** @brief DMA channel12 information */
static Dma_ChannelInfoType Dma12ChInfo =
{
    DMA_REQ_DISABLE, DMA_STATE_UNINITED, DMA_VIRTUAL_CH_MAX, NULL_PTR, NULL_PTR
};
#endif /* CONFIG_DMA_CHANNEL12_ENABLE */

#if (CONFIG_DMA_CHANNEL13_ENABLE)
/** @brief DMA channel13 information */
static Dma_ChannelInfoType Dma13ChInfo =
{
    DMA_REQ_DISABLE, DMA_STATE_UNINITED, DMA_VIRTUAL_CH_MAX, NULL_PTR, NULL_PTR
};
#endif /* CONFIG_DMA_CHANNEL13_ENABLE */

#if (CONFIG_DMA_CHANNEL14_ENABLE)
/** @brief DMA channel14 information */
static Dma_ChannelInfoType Dma14ChInfo =
{
    DMA_REQ_DISABLE, DMA_STATE_UNINITED, DMA_VIRTUAL_CH_MAX, NULL_PTR, NULL_PTR
};
#endif /* CONFIG_DMA_CHANNEL14_ENABLE */

#if (CONFIG_DMA_CHANNEL15_ENABLE)
/** @brief DMA channel15 information */
static Dma_ChannelInfoType Dma15ChInfo =
{
    DMA_REQ_DISABLE, DMA_STATE_UNINITED, DMA_VIRTUAL_CH_MAX, NULL_PTR, NULL_PTR
};
#endif /* CONFIG_DMA_CHANNEL15_ENABLE */
/** @brief DMA global structure for all channel */
static Dma_ChannelInfoType *const DmaChannelInfo[DMA_CH_MAX] =
{
#if (CONFIG_DMA_CHANNEL0_ENABLE)
    &Dma0ChInfo,
#else
    NULL_PTR,
#endif /* CONFIG_DMA_CHANNEL0_ENABLE */
#if (CONFIG_DMA_CHANNEL1_ENABLE)
    &Dma1ChInfo,
#else
    NULL_PTR,
#endif /* CONFIG_DMA_CHANNEL1_ENABLE */
#if (CONFIG_DMA_CHANNEL2_ENABLE)
    &Dma2ChInfo,
#else
    NULL_PTR,
#endif /* CONFIG_DMA_CHANNEL2_ENABLE */
#if (CONFIG_DMA_CHANNEL3_ENABLE)
    &Dma3ChInfo,
#else
    NULL_PTR,
#endif /* CONFIG_DMA_CHANNEL3_ENABLE */
#if (CONFIG_DMA_CHANNEL4_ENABLE)
    &Dma4ChInfo,
#else
    NULL_PTR,
#endif /* CONFIG_DMA_CHANNEL4_ENABLE */
#if (CONFIG_DMA_CHANNEL5_ENABLE)
    &Dma5ChInfo,
#else
    NULL_PTR,
#endif /* CONFIG_DMA_CHANNEL5_ENABLE */
#if (CONFIG_DMA_CHANNEL6_ENABLE)
    &Dma6ChInfo,
#else
    NULL_PTR,
#endif /* CONFIG_DMA_CHANNEL6_ENABLE */
#if (CONFIG_DMA_CHANNEL7_ENABLE)
    &Dma7ChInfo,
#else
    NULL_PTR,
#endif /* CONFIG_DMA_CHANNEL7_ENABLE */
#if (CONFIG_DMA_CHANNEL8_ENABLE)
    &Dma8ChInfo,
#else
    NULL_PTR,
#endif /* CONFIG_DMA_CHANNEL8_ENABLE */
#if (CONFIG_DMA_CHANNEL9_ENABLE)
    &Dma9ChInfo,
#else
    NULL_PTR,
#endif /* CONFIG_DMA_CHANNEL9_ENABLE */
#if (CONFIG_DMA_CHANNEL10_ENABLE)
    &Dma10ChInfo,
#else
    NULL_PTR,
#endif /* CONFIG_DMA_CHANNEL10_ENABLE */
#if (CONFIG_DMA_CHANNEL11_ENABLE)
    &Dma11ChInfo,
#else
    NULL_PTR,
#endif /* CONFIG_DMA_CHANNEL11_ENABLE */
#if (CONFIG_DMA_CHANNEL12_ENABLE)
    &Dma12ChInfo,
#else
    NULL_PTR,
#endif /* CONFIG_DMA_CHANNEL12_ENABLE */
#if (CONFIG_DMA_CHANNEL13_ENABLE)
    &Dma13ChInfo,
#else
    NULL_PTR,
#endif /* CONFIG_DMA_CHANNEL13_ENABLE */
#if (CONFIG_DMA_CHANNEL14_ENABLE)
    &Dma14ChInfo,
#else
    NULL_PTR,
#endif /* CONFIG_DMA_CHANNEL14_ENABLE */
#if (CONFIG_DMA_CHANNEL15_ENABLE)
    &Dma15ChInfo,
#else
    NULL_PTR,
#endif /* CONFIG_DMA_CHANNEL15_ENABLE */
};

/* ====================================  FUNCTION PROTOTYPES  ===================================== */

/*============================================FUNCTION PROTOTYPES===================================*/

/* ======================================  Functions definition  ==================================== */
/**
 * @brief Initializes the dma channel.
 * @note  Function ID : DES_MCL_API_140
 * @param[in] ConfigPtr: pointer of dma channel configration.
 * @param[out]  None
 * @return None.
 */
static void Dma_Hal_InitVCChannel(const Dma_ChannelConfigType *ConfigPtr)
{
    uint8 VCChannelId; /* virtual dma channel id */
    uint8 HwChannelId; /* hardware dma channel id */

    DEVICE_ASSERT(ConfigPtr != NULL_PTR);
    /* PRQA S 2812 ++ # AMCE_MCL_2812*/
    DEVICE_ASSERT(ConfigPtr->Priority <= DMA_CHANNEL_PRIORITY_VERY_HIGH);
    /* PRQA S 2812 -- # AMCE_MCL_2812*/
    VCChannelId = ConfigPtr->VirtualChannelId;
    HwChannelId = ConfigPtr->HwChannelId;
    DEVICE_ASSERT(VCChannelId < DMA_VIRTUAL_CH_MAX);
    if (VCChannelId < DMA_VIRTUAL_CH_MAX)
    {
        /* Install the user callback and set status to idle */
        DmaVCConfig[VCChannelId].VirtualChannelId = VCChannelId;
        DmaVCConfig[VCChannelId].HwChannelId = HwChannelId;
        DmaVCConfig[VCChannelId].Priority = ConfigPtr->Priority;
        DmaVCConfig[VCChannelId].ReqSrc = ConfigPtr->ReqSrc;
    }
}

/**
 * @brief Initializes the dma channel.
 * @note  Function ID : DES_MCL_API_140
 * @param[in] ConfigPtr: pointer of dma channel configration.
 * @param[out]  None
 * @return None.
 */
static void Dma_Hal_InitHWChannel(const Dma_ChannelConfigType *ConfigPtr)
{
    uint8 HwChannelId; /* hardware dma channel id */

    DEVICE_ASSERT(ConfigPtr != NULL_PTR);
    /* PRQA S 2812 ++ # AMCE_MCL_2812*/
    DEVICE_ASSERT(ConfigPtr->Priority <= DMA_CHANNEL_PRIORITY_VERY_HIGH);
    /* PRQA S 2812 -- # AMCE_MCL_2812*/
    HwChannelId = ConfigPtr->HwChannelId;
    DEVICE_ASSERT(HwChannelId < DMA_CH_MAX);
    if (HwChannelId < DMA_CH_MAX)
    {
        DEVICE_ASSERT(DmaChannelInfo[HwChannelId] != NULL_PTR);
        /* Set status to idle and set default param */
        DmaChannelInfo[HwChannelId]->State = DMA_STATE_INITED;
    }
}

/**
 * @brief De-initializes the dma channel.
 * @note  Function ID : DES_MCL_API_141
 * @param[in] HwChannelId: dma hardware channel id.
 * @param[out]  None
 * @return None.
 */
static void Dma_Hal_DeinitVCChannel(uint8 VCChannelId)
{
    DmaVCConfig[VCChannelId].VirtualChannelId = DMA_VIRTUAL_CH_MAX;
    DmaVCConfig[VCChannelId].HwChannelId = DMA_CH_MAX;
    DmaVCConfig[VCChannelId].ReqSrc = DMA_REQ_DISABLE;
}

/**
 * @brief De-initializes the dma channel.
 * @note  Function ID : DES_MCL_API_141
 * @param[in] HwChannelId: dma hardware channel id.
 * @param[out]  None
 * @return None.
 */
static void Dma_Hal_DeinitHWChannel(uint8 HwChannelId)
{
    if (DmaChannelInfo[HwChannelId] != NULL_PTR)
    {
        if (DMA_STATE_RUNNING == DmaChannelInfo[HwChannelId]->State)
        {
            (void)Dma_Hal_StopCh(HwChannelId);
        }

        OSIF_ENTER_CRITICAL(DMA_HAL_ID2);
        DmaChannelInfo[HwChannelId]->State = DMA_STATE_UNINITED;
        DmaChannelInfo[HwChannelId]->Callback = NULL_PTR;
        DmaChannelInfo[HwChannelId]->UserArgs = NULL_PTR;
        OSIF_EXIT_CRITICAL(DMA_HAL_ID2);
    }
}

/* Implementation of Dma_Hal_Init declared in Dma_Hal.h */
void Dma_Hal_Init(const Dma_ConfigType *ConfigPtr)
{
    uint8 ChIdx; /* dma channel idx */

    DEVICE_ASSERT(ConfigPtr != NULL_PTR);
    /* PRQA S 2812 ++ # AMCE_MCL_2812*/
    DEVICE_ASSERT(ConfigPtr->ChannelCnt <= DMA_VIRTUAL_CH_MAX);
    /* PRQA S 2812 -- # AMCE_MCL_2812*/
    DEVICE_ASSERT(ConfigPtr->ChannelCfg != NULL_PTR);
    if (ConfigPtr->ChannelCfg != NULL_PTR)
    {
        (void)Ckgen_Hal_EnablePeriphClk(CKGEN_DMA_BUS_CLK, TRUE);
        /*traverse all configuration channels to config each channel*/
        for (ChIdx = 0; ChIdx < ConfigPtr->ChannelCnt; ChIdx++)
        {
            Dma_Hal_InitVCChannel(&ConfigPtr->ChannelCfg[ChIdx]);
            Dma_Hal_InitHWChannel(&ConfigPtr->ChannelCfg[ChIdx]);
        }
    }
}

/* Implementation of Dma_Hal_Deinit declared in Dma_Hal.h */
void Dma_Hal_Deinit(void)
{
    uint8 ChIdx; /* dma channel idx */

    /*traverse all hardware channel to deinit*/
    for (ChIdx = 0; ChIdx < DMA_CH_MAX; ChIdx++)
    {
        Dma_Hal_DeinitHWChannel(ChIdx);
    }
    /*traverse all virtual channel to deinit*/
    for (ChIdx = 0; ChIdx < DMA_VIRTUAL_CH_MAX; ChIdx++)
    {
        Dma_Hal_DeinitVCChannel(ChIdx);
    }

    /*disable clock*/
    (void)Ckgen_Hal_EnablePeriphClk(CKGEN_DMA_BUS_CLK, FALSE);
}

#if defined(AC7840X)
/**
 * @brief Check the DMA config paramments
 * @note Function ID : DES_MCL_API_142
 * @param[in] ConfigPtr: The pointer to the DMA channel transfer config structure
 * @return paraments whether right
 */
static Hal_StatusType Hal_ParamsCheck(const Dma_TransferConfigType *ConfigPtr)
{
    Hal_StatusType Status = STATUS_SUCCESS;

    /*Check transfer Type whether is DMA_TRANSFER_PERIPH2MEM*/
    if (DMA_TRANSFER_PERIPH2MEM == (Dma_TransferType)ConfigPtr->Type)
    {
        /*check Dest address whether valid*/
        if ((ConfigPtr->DestStartAddr > SRAM_U_END) || (ConfigPtr->DestStartAddr < SRAM_U_BASE))
        {
            Status = STATUS_ADDRS_INVALID;
        }
    }
    else if (DMA_TRANSFER_MEM2PERIPH == ConfigPtr->Type) /*Check transfer Type whether is DMA_TRANSFER_MEM2PERIPH*/
    {
        /*check Src address whether valid*/
        if ((ConfigPtr->SrcStartAddr > SRAM_U_END) || (ConfigPtr->SrcStartAddr < SRAM_U_BASE))
        {
            Status = STATUS_ADDRS_INVALID;
        }
    }
    else if (DMA_TRANSFER_MEM2MEM == ConfigPtr->Type) /*Check transfer Type whether is DMA_TRANSFER_MEM2MEM*/
    {

        /*check Src & Dest address whether valid*/
        if (((ConfigPtr->SrcStartAddr > SRAM_U_END) || (ConfigPtr->SrcStartAddr < SRAM_U_BASE)) \
                || ((ConfigPtr->DestStartAddr > SRAM_U_END) || (ConfigPtr->DestStartAddr < SRAM_U_BASE)))
        {
            Status = STATUS_ADDRS_INVALID;
        }
    }
    else
    {
        /*do nothing*/
    }

    return Status;
}
#endif /* AC7840X */

/* Implementation of Dma_Hal_ConfigCh declared in Dma_Hal.h */
Hal_StatusType Dma_Hal_ConfigCh(uint8 ChannelId, const Dma_TransferConfigType *ConfigPtr)
{
    Hal_StatusType Status = STATUS_SUCCESS; /* function return value */
    uint8 HwChannelId = DMA_CH_MAX;         /* dma hardware channel id */
    IRQn_Type IRQNum;

    DEVICE_ASSERT(ConfigPtr != NULL_PTR);
    /* PRQA S 2812 ++ # AMCE_MCL_2812*/
    DEVICE_ASSERT(ConfigPtr->Length <= DMA_TRANSFER_LENGTH_MAX);
    /* PRQA S 2812 -- # AMCE_MCL_2812*/
    DEVICE_ASSERT(ChannelId < DMA_VIRTUAL_CH_MAX);
    if (ChannelId < DMA_VIRTUAL_CH_MAX)
    {
        HwChannelId = DmaVCConfig[ChannelId].HwChannelId;
        DmaChannelInfo[HwChannelId]->VirtualChannelId = ChannelId;
    }
    DEVICE_ASSERT(HwChannelId < DMA_CH_MAX);
    if (HwChannelId < DMA_CH_MAX)
    {
        DEVICE_ASSERT(((DMA_STATE_CONFIGURED == DmaChannelInfo[HwChannelId]->State) \
                       || (DMA_STATE_INITED == DmaChannelInfo[HwChannelId]->State)));
#if defined (AC7840X)
        Status = Hal_ParamsCheck(ConfigPtr);
        DEVICE_ASSERT(STATUS_SUCCESS == Status);
        /*if Status=STATUS_SUCCESS means Params Check is Pass*/
        if (STATUS_SUCCESS == Status)
        {
#endif
            /* Configure source and destination addresses */
            Dma_Reg_SetSrcStartAddr(HwChannelId, ConfigPtr->SrcStartAddr);
            Dma_Reg_SetSrcEndAddr(HwChannelId, ConfigPtr->SrcEndAddr);
            Dma_Reg_SetDestStartAddr(HwChannelId, ConfigPtr->DestStartAddr);
            Dma_Reg_SetDestEndAddr(HwChannelId, ConfigPtr->DestEndAddr);
            /* Set transfer size (1, 2 or 4 bytes) */
            Dma_Reg_SetSrcTransferSize(HwChannelId, (uint8)ConfigPtr->SrcUnit);
            Dma_Reg_SetDstTransferSize(HwChannelId, (uint8)ConfigPtr->DestUnit);
            /* Configure source/destination offset address. */
            Dma_Reg_SetSrcAddrOffSet(HwChannelId, ConfigPtr->SrcOffset);
            Dma_Reg_SetDestAddrOffSet(HwChannelId, ConfigPtr->DestOffset);
            Dma_Reg_SetFIFOFastFunction(HwChannelId, FALSE);
            /* Set the total number of bytes to be transfered */
            Dma_Reg_SetTransferLength(HwChannelId, ConfigPtr->Length);
            /* Enable/disalbe trigger mode */
            Dma_Reg_SetChannelTrigger(HwChannelId, ConfigPtr->TriggerMode);
            /* Enable/disalbe Circular mode */
            Dma_Reg_SetCircularMode(HwChannelId, ConfigPtr->CircularMode);
            /* Set DMA channel source request and trigger */
            if (DmaVCConfig[ChannelId].ReqSrc < DMA_REQ_CRC)
            {
                Dma_Reg_SetChannelSource(HwChannelId, (uint32)DmaVCConfig[ChannelId].ReqSrc);
            }
            else
            {
                Dma_Reg_SetChannelSource(HwChannelId, (uint32)DMA_REQ_ALWAYS_ENABLED);
            }
#if defined (AC7840E)
            Dma_Reg_EnChannelDaisy(HwChannelId, ConfigPtr->DaisyMode);
            if (TRUE == ConfigPtr->DaisyMode)
            {
                Dma_Reg_SetChannelTriggerOutSel(HwChannelId, (uint8)ConfigPtr->DaisyTrigger);
            }
#endif
            /*PRQA S 4394 ++ # AMCE_MCL_4394*/
            /*PRQA S 4342 ++ # AMCE_MCL_4342*/
            /*PRQA S 2985 ++ # AMCE_MCL_2985*/
            IRQNum = (IRQn_Type)(HwChannelId + (uint8)DMA0_CHANNEL0_IRQn);
            /*PRQA S 4394 -- */
            /*PRQA S 4342 -- */
            /*PRQA S 2985 -- */
            /* Clear interrupt status */
            Dma_Reg_ClearDmaStatus(HwChannelId);
            /* set dma irq source */
            Dma_Reg_SetInterrupt(HwChannelId, ConfigPtr->IrqSrc);
            if ((uint8)DMA_IRQ_NONE != ConfigPtr->IrqSrc)
            {
                Core_Hal_EnableIrq(IRQNum);
            }
            else
            {
                Core_Hal_DisableIrq(IRQNum);
            }
            /* Set the channel priority */
            Dma_Reg_SetChannelPriority(HwChannelId, (uint32)DmaVCConfig[ChannelId].Priority);
            /*set current to CONFIG*/
            DmaChannelInfo[HwChannelId]->State = DMA_STATE_CONFIGURED;
            DmaChannelInfo[HwChannelId]->UserArgs = ConfigPtr->UserArgs;
            DmaChannelInfo[HwChannelId]->Callback = ConfigPtr->Callback;
#if defined(AC7840X)
        }
#endif
    }
    else
    {
        Status = STATUS_ERROR;
    }

    return Status;
}

/* Implementation of Dma_Hal_UpdateChAddr declared in Dma_Hal.h */
void Dma_Hal_UpdateChAddr(uint8 ChannelId, const Dma_ChannelAddrType *ChannelAddr)
{
    uint8 HwChannelId = DMA_CH_MAX; /* dma hardware channel id */

    DEVICE_ASSERT(ChannelId < DMA_VIRTUAL_CH_MAX);
    DEVICE_ASSERT(ChannelAddr != NULL_PTR);
    /* PRQA S 2812 ++ # AMCE_MCL_2812*/
    DEVICE_ASSERT(ChannelAddr->Length <= DMA_TRANSFER_LENGTH_MAX);
    /* PRQA S 2812 -- # AMCE_MCL_2812*/
    if (ChannelId < DMA_VIRTUAL_CH_MAX)
    {
        HwChannelId = DmaVCConfig[ChannelId].HwChannelId;
    }
    DEVICE_ASSERT(HwChannelId < DMA_CH_MAX);
    if (HwChannelId < DMA_CH_MAX)
    {
        /* Configure source and destination addresses */
        Dma_Reg_SetSrcStartAddr(HwChannelId, ChannelAddr->SrcStartAddr);
        Dma_Reg_SetSrcEndAddr(HwChannelId, ChannelAddr->SrcEndAddr);
        Dma_Reg_SetDestStartAddr(HwChannelId, ChannelAddr->DestStartAddr);
        Dma_Reg_SetDestEndAddr(HwChannelId, ChannelAddr->DestEndAddr);
        Dma_Reg_SetTransferLength(HwChannelId, ChannelAddr->Length);
        Dma_Reg_SetSrcAddrOffSet(HwChannelId, ChannelAddr->SrcOffset);
        Dma_Reg_SetDestAddrOffSet(HwChannelId, ChannelAddr->DestOffset);
    }
}

/* Implementation of Dma_Hal_EnableChIrq declared in Dma_Hal.h */
void Dma_Hal_EnableChIrq(uint8 ChannelId, uint8 IrqSrc)
{
    uint8 HwChannelId = DMA_CH_MAX; /* dma hardware channel id */
    IRQn_Type IRQNum;

    DEVICE_ASSERT(ChannelId < DMA_VIRTUAL_CH_MAX);
    if (ChannelId < DMA_VIRTUAL_CH_MAX)
    {
        HwChannelId = DmaVCConfig[ChannelId].HwChannelId;
    }
    DEVICE_ASSERT(HwChannelId < DMA_CH_MAX);
    if (HwChannelId < DMA_CH_MAX)
    {
        if (DMA_STATE_UNINITED != DmaChannelInfo[HwChannelId]->State)
        {
            /*PRQA S 4394 ++ # AMCE_MCL_4394 */
            /*PRQA S 4342 ++ # AMCE_MCL_4342*/
            /*PRQA S 2985 ++ # AMCE_MCL_2985*/
            IRQNum = (IRQn_Type)(HwChannelId + (uint8)DMA0_CHANNEL0_IRQn);
            /*PRQA S 2985 -- # AMCE_MCL_2985*/
            /*PRQA S 4342 -- */
            /*PRQA S 4394 -- */
            Dma_Reg_SetInterrupt(HwChannelId, IrqSrc);
            if ((uint8)DMA_IRQ_NONE != IrqSrc)
            {
                Core_Hal_EnableIrq(IRQNum);
            }
            else
            {
                Core_Hal_DisableIrq(IRQNum);
                Core_Hal_ClearPendingIrq(IRQNum);
            }
        }
    }
}

/* Implementation of Dma_Hal_GetChStatus declared in Dma_Hal.h */
Hal_StatusType Dma_Hal_GetChStatus(uint8 ChannelId)
{
    Dma_StateType State;                  /* dma state machine */
    Hal_StatusType Status = STATUS_ERROR; /* dma status */
    uint8 HwChannelId = DMA_CH_MAX;       /* dma hardware channel id */

    DEVICE_ASSERT(ChannelId < DMA_VIRTUAL_CH_MAX);
    if (ChannelId < DMA_VIRTUAL_CH_MAX)
    {
        HwChannelId = DmaVCConfig[ChannelId].HwChannelId;
    }
    DEVICE_ASSERT(HwChannelId < DMA_CH_MAX);
    if (HwChannelId < DMA_CH_MAX)
    {
        State = DmaChannelInfo[HwChannelId]->State;
        if ((DMA_STATE_INITED == State) || (DMA_STATE_UNINITED == State))
        {
            Status = STATUS_IDLE;
        }
        else if (DMA_STATE_RUNNING == State)
        {
            Status = STATUS_BUSY;
        }
        else if (DMA_STATE_CONFIGURED == State)
        {
            Status = STATUS_CONFIG;
        }
        else
        {
            Status = STATUS_ABORT;
        }
    }

    return Status;
}

/* Implementation of Dma_Hal_StartCh declared in Dma_Hal.h */
Hal_StatusType Dma_Hal_StartCh(uint8 ChannelId)
{
    Hal_StatusType Status = STATUS_ERROR; /* function return value */
    Dma_StateType State;                  /* dma state machine */
    Dma_ChannelInfoType *DmaChInfo;
    uint8 HwChannelId = DMA_CH_MAX; /* dma hardware channel id */

    DEVICE_ASSERT(ChannelId < DMA_VIRTUAL_CH_MAX);
    if (ChannelId < DMA_VIRTUAL_CH_MAX)
    {
        HwChannelId = DmaVCConfig[ChannelId].HwChannelId;
        DmaChannelInfo[HwChannelId]->VirtualChannelId = ChannelId;
    }
    DEVICE_ASSERT(HwChannelId < DMA_CH_MAX);
    if (HwChannelId < DMA_CH_MAX)
    {
        DmaChInfo = DmaChannelInfo[HwChannelId];
        State = DmaChInfo->State;
        if (DMA_STATE_CONFIGURED == State)
        {
            /* Check DMA channel error status */
            DmaChInfo->State = DMA_STATE_RUNNING;
            /* Start working */
#if defined (AC7840E)
            if (0U == Dma_Reg_GetChannelDaisy(HwChannelId))
#endif
            {
                Dma_Reg_SetChannel(HwChannelId, TRUE);
            }

            Status = STATUS_SUCCESS;
        }
        else if (DMA_STATE_RUNNING == State)
        {
            Status = STATUS_SUCCESS;
        }
        else
        {
            Status = STATUS_ERROR;
        }
    }

    return Status;
}

/* Implementation of Dma_Hal_StopCh declared in Dma_Hal.h */
Hal_StatusType Dma_Hal_StopCh(uint8 ChannelId)
{
    Hal_StatusType Status = STATUS_SUCCESS; /* function return value */
    Dma_StateType State;                    /* dma state machine */
    IRQn_Type IRQNum;
    uint8 HwChannelId = DMA_CH_MAX; /* dma hardware channel id */

    DEVICE_ASSERT(ChannelId < DMA_VIRTUAL_CH_MAX);
    if (ChannelId < DMA_VIRTUAL_CH_MAX)
    {
        HwChannelId = DmaVCConfig[ChannelId].HwChannelId;
    }
    DEVICE_ASSERT(HwChannelId < DMA_CH_MAX);
    if (HwChannelId < DMA_CH_MAX)
    {
        OSIF_ENTER_CRITICAL(DMA_HAL_ID1);
        State = DmaChannelInfo[HwChannelId]->State;

        if ((DMA_STATE_RUNNING == State) || (DMA_STATE_CONFIGURED == State) || (DMA_STATE_ERROR == State))
        {
            Dma_Reg_SetCircularMode(HwChannelId, FALSE);
            /*PRQA S 4394 ++ # AMCE_MCL_4394 */
            /*PRQA S 4342 ++ # AMCE_MCL_4342*/
            /*PRQA S 2985 ++ # AMCE_MCL_2985*/
            IRQNum = (IRQn_Type)(HwChannelId + (uint8)DMA0_CHANNEL0_IRQn);
            /*PRQA S 2985 -- # AMCE_MCL_2985*/
            /*PRQA S 4342 -- */
            /*PRQA S 4394 -- */
            /* Stop working */
            Dma_Reg_SetChannel(HwChannelId, FALSE);
            /* Clear interrupt status */
            Dma_Reg_ClearDmaStatus(HwChannelId);
            /*Disable and clear nvic dma irq*/
            Core_Hal_DisableIrq(IRQNum);
            Core_Hal_ClearPendingIrq(IRQNum);
            DmaChannelInfo[HwChannelId]->State = DMA_STATE_CONFIGURED;
        }
        else
        {
            Status = STATUS_ERROR;
        }
        OSIF_EXIT_CRITICAL(DMA_HAL_ID1);
    }

    return Status;
}

/* Implementation of Dma_Hal_ChReset declared in Dma_Hal.h */
void Dma_Hal_ChReset(uint8 ChannelId, Dma_ChannelResetType Reset) //PRQA S 1503 # This external HAL API is kept for upper-layer integration even when the current project does not call it directly.
{
    Dma_StateType State;            /* dma state machine */
    uint8 HwChannelId = DMA_CH_MAX; /* dma hardware channel id */

    DEVICE_ASSERT(ChannelId < DMA_VIRTUAL_CH_MAX);
    if (ChannelId < DMA_VIRTUAL_CH_MAX)
    {
        HwChannelId = DmaVCConfig[ChannelId].HwChannelId;
    }
    DEVICE_ASSERT(HwChannelId < DMA_CH_MAX);
    if (HwChannelId < DMA_CH_MAX)
    {
        State = DmaChannelInfo[HwChannelId]->State;
        DEVICE_ASSERT(DMA_STATE_UNINITED != State);
        if (DMA_STATE_UNINITED != State)
        {
            if (DMA_CHANNEL_SW_RESET == Reset)
            {
                Dma_Reg_ChannelWarmRst(HwChannelId);
            }
            else
            {
                Dma_Reg_ChannelHardRst(HwChannelId);
            }
            DmaChannelInfo[HwChannelId]->State = DMA_STATE_INITED;
        }
    }
}

/* Implementation of Dma_Hal_GetTransBytes declared in Dma_Hal.h */
uint32 Dma_Hal_GetTransBytes(uint8 ChannelId)
{
    uint32 TransedLength = 0U;
    uint8 HwChannelId = DMA_CH_MAX; /* dma hardware channel id */

    DEVICE_ASSERT(ChannelId < DMA_VIRTUAL_CH_MAX);
    if (ChannelId < DMA_VIRTUAL_CH_MAX)
    {
        HwChannelId = DmaVCConfig[ChannelId].HwChannelId;
    }
    DEVICE_ASSERT(HwChannelId < DMA_CH_MAX);
    if (HwChannelId < DMA_CH_MAX)
    {
        DEVICE_ASSERT(DMA_STATE_UNINITED != DmaChannelInfo[HwChannelId]->State);

        TransedLength = Dma_Reg_GetTransferedBytes(HwChannelId);
    }

    return TransedLength;
}

/* Implementation of Dma_Hal_GetChIdByReqSrc declared in Dma_Hal.h */
uint8 Dma_Hal_GetChIdByReqSrc(Dma_RequestSourceType ReqSrc)
{
    uint8 ChIdx;

    /*traverse all channel reqSrc to find matching reqSrc */
    for (ChIdx = 0; ChIdx < DMA_VIRTUAL_CH_MAX; ChIdx++)
    {
        if (ReqSrc == DmaVCConfig[ChIdx].ReqSrc)
        {
            break;
        }
    }
    DEVICE_ASSERT(ChIdx < DMA_VIRTUAL_CH_MAX);

    return ChIdx;
}

/**
 * @brief Dma interrupt request handler function.
 * @details
 * This function handles the DMA channel interrupt. It checks the interrupt status flags,
 * clears the status, updates the DMA channel state, and invokes the user-defined callback
 * function with interrupt event information.
 *
 * @param[in] HwChannelId: DMA hardware channel id (0-15)
 * @return void
 *
 * @pre
 *     The DMA channel must be initialized and configured.
 *     The callback function pointer must be properly set if interrupt handling is needed.
 *
 * @post
 *     Interrupt status is cleared, channel state is updated based on the event,
 *     and user callback is invoked if registered.
 *
 * @note
 *     This is an internal static function used by DMA interrupt handlers.
 *     Reentrancy: Not reentrant.
 *     DES_MCL_VAR_201
 */
static void Dma_Hal_IRQHandler(uint8 HwChannelId)
{
    uint32 Status;
    Dma_ChannelCBInfoType IrqInfo =
    {
        .DmaEvent = 0U,
    };
    Dma_ChannelInfoType *ChInfo;

    ChInfo = DmaChannelInfo[HwChannelId];
    /* get interrupt status */
    Status = Dma_Reg_GetDmaStatus(HwChannelId);
    /* Clear interrupt status */
    Dma_Reg_ClearDmaStatus(HwChannelId);
    if ((Status & DMA_FINISH_EVENT) != 0U) /* Check DMA channel finish status */
    {
        /* if channel is circular mode, don't change state*/
        if (0U == Dma_Reg_GetCircularMode(HwChannelId))
        {
            ChInfo->State = DMA_STATE_CONFIGURED;
        }
    }
#if defined(AC7842X) || defined(AC7843X)
    else if ((Status & DMA_HALF_FINISH_EVENT) != 0U) /* Check DMA channel half finish status */
    {
        /* if channel is circular mode, don't change state*/
        if (0U == Dma_Reg_GetCircularMode(HwChannelId))
        {
            ChInfo->State = DMA_STATE_CONFIGURED;
        }
    }
#endif
    else if ((Status & DMA_ERROR_EVENT) != 0U) /* Check DMA channel error status */
    {
        /* Set DMA channel error status */
        ChInfo->State = DMA_STATE_ERROR;
    }
    else /* invalid dma irq */
    {
        /* nothing */
    }

    if (0U != Status) /* valid dma irq */
    {
        IrqInfo.DmaEvent = Status;
        IrqInfo.UserArgs = ChInfo->UserArgs;
        /*PRQA S 4342 ++ # AMCE_MCL_4342 */
        IrqInfo.ChannelId = DmaChannelInfo[HwChannelId]->VirtualChannelId;
        /*PRQA S 4342 -- */
        /* Calling user interrupt callback function */
        if (ChInfo->Callback != NULL_PTR)
        {
            ChInfo->Callback((void *)&IrqInfo);
        }
    }
}

/* DMA0 Channel0 interrupt handler - delegates to Dma_Hal_IRQHandler */
ISR(DMA0_Channel0_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    Dma_Hal_IRQHandler(0U);
}

/* DMA0 Channel1 interrupt handler - delegates to Dma_Hal_IRQHandler */
ISR(DMA0_Channel1_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    Dma_Hal_IRQHandler(1U);
}

/* DMA0 Channel2 interrupt handler - delegates to Dma_Hal_IRQHandler */
ISR(DMA0_Channel2_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    Dma_Hal_IRQHandler(2U);
}

/* DMA0 Channel3 interrupt handler - delegates to Dma_Hal_IRQHandler */
ISR(DMA0_Channel3_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    Dma_Hal_IRQHandler(3U);
}

/* DMA0 Channel4 interrupt handler - delegates to Dma_Hal_IRQHandler */
ISR(DMA0_Channel4_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    Dma_Hal_IRQHandler(4U);
}

/* DMA0 Channel5 interrupt handler - delegates to Dma_Hal_IRQHandler */
ISR(DMA0_Channel5_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    Dma_Hal_IRQHandler(5U);
}

/* DMA0 Channel6 interrupt handler - delegates to Dma_Hal_IRQHandler */
ISR(DMA0_Channel6_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    Dma_Hal_IRQHandler(6U);
}

/* DMA0 Channel7 interrupt handler - delegates to Dma_Hal_IRQHandler */
ISR(DMA0_Channel7_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    Dma_Hal_IRQHandler(7U);
}

/* DMA0 Channel8 interrupt handler - delegates to Dma_Hal_IRQHandler */
ISR(DMA0_Channel8_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    Dma_Hal_IRQHandler(8U);
}

/* DMA0 Channel9 interrupt handler - delegates to Dma_Hal_IRQHandler */
ISR(DMA0_Channel9_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    Dma_Hal_IRQHandler(9U);
}

/* DMA0 Channel10 interrupt handler - delegates to Dma_Hal_IRQHandler */
ISR(DMA0_Channel10_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    Dma_Hal_IRQHandler(10U);
}

/* DMA0 Channel11 interrupt handler - delegates to Dma_Hal_IRQHandler */
ISR(DMA0_Channel11_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    Dma_Hal_IRQHandler(11U);
}

/* DMA0 Channel12 interrupt handler - delegates to Dma_Hal_IRQHandler */
ISR(DMA0_Channel12_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    Dma_Hal_IRQHandler(12U);
}

/* DMA0 Channel13 interrupt handler - delegates to Dma_Hal_IRQHandler */
ISR(DMA0_Channel13_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    Dma_Hal_IRQHandler(13U);
}

/* DMA0 Channel14 interrupt handler - delegates to Dma_Hal_IRQHandler */
ISR(DMA0_Channel14_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    Dma_Hal_IRQHandler(14U);
}

/* DMA0 Channel15 interrupt handler - delegates to Dma_Hal_IRQHandler */
ISR(DMA0_Channel15_IRQHandler) //PRQA S 1503 # This interrupt entry is referenced by the vector table or hardware, so it may not appear in the static call graph.
{
    Dma_Hal_IRQHandler(15U);
}
