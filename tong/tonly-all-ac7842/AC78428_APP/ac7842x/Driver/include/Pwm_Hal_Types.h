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

#ifndef PWM_HAL_TYPES_H
#define PWM_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================INCLUDE FILES=======================================*/
#include "Device_Register.h"

/*===================================================ENUMS==========================================*/
/**
 * @brief PWM hardware module enumeration
 * @note DES ID: DES_PWM_TYP_201
 */
typedef enum
{
    PWM_INSTANCE_0 = 0U,  /*!< Hardware unit ID 0 */
    PWM_INSTANCE_1,       /*!< Hardware unit ID 1 */
    PWM_INSTANCE_2,       /*!< Hardware unit ID 2 */
    PWM_INSTANCE_3,       /*!< Hardware unit ID 3 */
#if (PWM_INSTANCE_MAX > 4U)
    PWM_INSTANCE_4,       /*!< Hardware unit ID 4 */
#endif
#if (PWM_INSTANCE_MAX > 5U)
    PWM_INSTANCE_5,       /*!< Hardware unit ID 5 */
#endif
#if (PWM_INSTANCE_MAX > 6U)
    PWM_INSTANCE_6,       /*!< Hardware unit ID 6 */
#endif
#if (PWM_INSTANCE_MAX > 7U)
    PWM_INSTANCE_7,       /*!< Hardware unit ID 7 */
#endif
} Pwm_Hal_InstanceType;

/**
 * @brief PWM hardware channel enumeration
 * @note DES ID: DES_PWM_TYP_202
 */
typedef enum
{
    PWM_CHANNEL_0 = 0U,  /*!< Hardware channel ID 0 */
    PWM_CHANNEL_1,       /*!< Hardware channel ID 1 */
    PWM_CHANNEL_2,       /*!< Hardware channel ID 2 */
    PWM_CHANNEL_3,       /*!< Hardware channel ID 3 */
    PWM_CHANNEL_4,       /*!< Hardware channel ID 4 */
    PWM_CHANNEL_5,       /*!< Hardware channel ID 5 */
    PWM_CHANNEL_6,       /*!< Hardware channel ID 6 */
    PWM_CHANNEL_7,       /*!< Hardware channel ID 7 */
} Pwm_Hal_ChannelType;

/**
 * @brief PWM hardware channel pair enumeration
 * @note DES ID: DES_PWM_TYP_203
 */
typedef enum
{
    PWM_CHANNEL_PAIR_0 = 0U,  /*!< Hardware channel pair 0 */
    PWM_CHANNEL_PAIR_1,       /*!< Hardware channel pair 1 */
    PWM_CHANNEL_PAIR_2,       /*!< Hardware channel pair 2 */
    PWM_CHANNEL_PAIR_3,       /*!< Hardware channel pair 3 */
    PWM_CHANNEL_PAIR_NUM      /*!< Hardware channel pair maximum number */
} Pwm_Hal_ChannelPairType;

/**
 * @brief PWM clock source enumeration
 * @note DES ID: DES_PWM_TYP_204
 */
typedef enum
{
    PWM_CLK_SOURCE_NONE = 0U,       /*!< No clock selected, in effect disables the counter */
#if defined (AC7843X)
    PWM_CLK_SOURCE_BUS,             /*!< Bus clock */
#else
    PWM_CLK_SOURCE_SYSTEM,          /*!< System clock */
#endif
    PWM_CLK_SOURCE_FIXED_FREQUENCY, /*!< Fixed frequency clock */
    PWM_CLK_SOURCE_EXTERNAL,        /*!< External clock */
} Pwm_Hal_ClockSourceType;

/**
 * @brief PWM count mode enumeration
 * @note DES ID: DES_PWM_TYP_205
 */
typedef enum
{
    UP_COUNT = 0U,  /*!< Up count only mode */
    UP_DOWN_COUNT,  /*!< Up down count mode */
} Pwm_Hal_CountModeType;

/**
 * @brief PWM output channel running mode enumeration
 * @note DES ID: DES_PWM_TYP_206
 */
typedef enum
{
    OUTPUT_NONE = 0U,    /*!< No output */
    OUTPUT_INDEPENDENT,  /*!< Independent mode */
    OUTPUT_COMBINE,      /*!< Combine mode */
    OUTPUT_COMPARE,      /*!< Output compare mode */
} Pwm_Hal_OutputChnModeType;

/**
 * @brief PWM output channel level enumeration
 * @note DES ID: DES_PWM_TYP_207
 */
typedef enum
{
    PWM_LOW_LEVEL = 0U,   /*!< Low level */
    PWM_HIGH_LEVEL,       /*!< High level */
} Pwm_Hal_OutputLevelType;

/**
 * @brief PWM output channel level mode enumeration
 * @note DES ID: DES_PWM_TYP_208
 */
typedef enum
{
    PWM_LOW_TRUE = 0U,   /*!< ELSR value */
    PWM_HIGH_TRUE,       /*!< ELSR value */
} Pwm_Hal_OutputLevelModeType;

/**
 * @brief Enumeration of PWM duty cycle types in combination center alignment mode
 * @note DES ID: DES_PWM_TYP_209
 */
typedef enum
{
    PWM_DUTY_MODE_0 = 0U, /*!< CnV=Cn+1V=CNTIN,output 0% duty cycle;CnV=Cn+1V=MOD,output 100% duty cycle */
    PWM_DUTY_MODE_1       /*!< CnV=Cn+1V=CNTIN,output 100% duty cycle;CnV=Cn+1V=MOD,output 0% duty cycle */
} Pwm_Hal_CombineCenterDutyModeType;

/**
 * @brief PWM combine channel match direction enumeration
 * @note DES ID: DES_PWM_TYP_210
 */
typedef enum
{
    PWM_DIR_DOWN = 0U, /*!< Counter counting up */
    PWM_DIR_UP,        /*!< Counter counting down */
} Pwm_Hal_ChannelMatchDirType;

/**
 * @brief PWM combine channel dead time prescaler enumeration
 * @note DES ID: DES_PWM_TYP_211
 */
typedef enum
{
    PWM_DEADTIME_DIVID_1 = 1U, /*!< 1x frequency division */
    PWM_DEADTIME_DIVID_4,      /*!< 4x frequency division */
    PWM_DEADTIME_DIVID_16,     /*!< 16x frequency division */
} Pwm_Hal_DeadTimePscType;

/**
 * @brief PWM output compare mode enumeration
 * @note DES ID: DES_PWM_TYP_212
 */
typedef enum
{
    PWM_NONE_OUTPUT = 0U, /*!< No output */
    PWM_TOGGLE_OUTPUT,    /*!< Toggle output */
    PWM_CLEAR_OUTPUT,     /*!< Clear output */
    PWM_SET_OUTPUT        /*!< Set output */
} Pwm_Hal_CompareActionType;

/**
 * @brief PWM channel output polarity active enumeration
 * @note DES ID: DES_PWM_TYP_213
 */
typedef enum
{
    PWM_ACTIVE_POLARITY_HIGH = 0U, /*!< The channel output polarity is active high */
    PWM_ACTIVE_POLARITY_LOW        /*!< The channel output polarity is active low */
} Pwm_Hal_ActivePolarityType;

/**
 * @brief PWM detect edge enumeration
 * @note DES ID: DES_PWM_TYP_214
 */
typedef enum
{
    NONE_EDGE = 0U,      /*!< Capture none */
    RISING_EDGE_DETECT,  /*!< Capture rising edge */
    FALLING_EDGE_DETECT, /*!< Capture falling edge */
    BOTH_EDGES_DETECT,   /*!< Capture both edge */
} Pwm_Hal_EdgeType;

/**
 * @brief PWM input channel running mode enumeration
 * @note DES ID: DES_PWM_TYP_215
 */
typedef enum
{
    INPUT_NONE = 0U,  /*!< Capture none */
    INPUT_SINGLE,     /*!< Single edge capture */
    INPUT_DUAL,       /*!< Dual edge capture */
} Pwm_Hal_InputChnModeType;

/**
 * @brief Input event prescaler
 * @note DES ID: DES_PWM_TYP_216
 */
typedef enum
{
    INPUT_EVENT_PSC_1 = 0U,    /*!< 1x prescale */
    INPUT_EVENT_PSC_2,         /*!< 2x prescale */
    INPUT_EVENT_PSC_4,         /*!< 4x prescale */
    INPUT_EVENT_PSC_8,         /*!< 8x prescale */
} Pwm_Hal_InputEventPscType;

/**
 * @brief Input filter prescaler
 * @note DES ID: DES_PWM_TYP_217
 */
typedef enum
{
    INPUT_FILTER_PSC_1 = 0U,    /*!< 1x prescale */
    INPUT_FILTER_PSC_2,         /*!< 2x prescale */
    INPUT_FILTER_PSC_4,         /*!< 4x prescale */
    INPUT_FILTER_PSC_8,         /*!< 8x prescale */
    INPUT_FILTER_PSC_16,        /*!< 16x prescale */
    INPUT_FILTER_PSC_32,        /*!< 32x prescale */
    INPUT_FILTER_PSC_64,        /*!< 64x prescale */
    INPUT_FILTER_PSC_128,       /*!< 128x prescale */
    INPUT_FILTER_PSC_256,       /*!< 256x prescale */
    INPUT_FILTER_PSC_1024,      /*!< 1024x prescale */
    INPUT_FILTER_PSC_2048,      /*!< 2048x prescale */
    INPUT_FILTER_PSC_4096,      /*!< 4096x prescale */
} Pwm_Hal_InputFilterPscType;

/**
 * @brief PWM measure type enumeration
 * @note DES ID: DES_PWM_TYP_218
 */
typedef enum
{
    PWM_POSITIVE_PLUSE_WIDTH_MEASURE = 0U,    /*!< Measure positive pulse width */
    PWM_NEGATIVE_PLUSE_WIDTH_MEASURE,         /*!< Measure negative pulse width */
    PWM_RISING_EDGE_PERIOD_MEASURE,           /*!< Measure period between two rising edge */
    PWM_FALLING_EDGE_PERIOD_MEASURE,          /*!< Measure period between two falling edge */
    PWM_BOTH_EDGE_DUTY_CYCLE_MEASURE          /*!< Measure dutycycle between any edge */
} Pwm_Hal_DualInputMeasureType;

/**
 * @brief PWM continuous mode enumeration
 * @note DES ID: DES_PWM_TYP_219
 */
typedef enum
{
    PWM_INPUTCAP_ONESHOT = 0U,    /*!< One shot capture */
    PWM_INPUTCAP_CONTINUOUS,      /*!< Continuous capture */
} Pwm_Hal_DualInputContinuousModeType;

/**
 * @brief PWM sync trigger enumeration
 * @note DES ID: DES_PWM_TYP_220
 */
typedef enum
{
    PWM_SYNC_TRIGGER_SOFTWARE = 0U,    /*!< Software sync */
    PWM_SYNC_TRIGGER_HARDWARE,         /*!< Hardware sync */
} Pwm_Hal_SyncTriggerMethodType;

/*PRQA S 1535 ++ # AMCE_PWM_1535. */
/**
 * @brief PWM sync mode enumeration
 * @note DES ID: DES_PWM_TYP_221
 */
typedef enum
{
    PWM_SYNC_MODE_LEGACY = 0U,    /*!< Legacy sync mode */
    PWM_SYNC_MODE_ENHANCED,       /*!< Enhanced sync mode */
} Pwm_Hal_SyncModeType;

/**
 * @brief PWM fault input pin enumeration
 * @note DES ID: DES_PWM_TYP_222
 */
typedef enum
{
    PWM_FAULT_INPUT_0 = 0U, /*!< PWM fault input input ID 0 */
    PWM_FAULT_INPUT_1,      /*!< PWM fault input input ID 1 */
    PWM_FAULT_INPUT_2,      /*!< PWM fault input input ID 2 */
    PWM_FAULT_INPUT_3,      /*!< PWM fault input input ID 3 */
    PWM_FAULT_INPUT_MAX     /*!< Invalid fault input ID */
} Pwm_Hal_FaultInputIdType;
/*PRQA S 1535 -- # AMCE_PWM_1535. */

/**
 * @brief PWM channel fault control mode enumeration
 * @note DES ID: DES_PWM_TYP_223
 */
typedef enum
{
    PWM_FAULT_CTRL_NONE = 0U,   /*!< No Fault control */
    PWM_FAULT_CTRL_MANUAL_EVEN, /*!< Fault control is enabled for even channels and manual fault clearing */
    PWM_FAULT_CTRL_MANUAL_ALL,  /*!< Fault control is enabled for all channels and manual fault clearing */
    PWM_FAULT_CTRL_AUTO         /*!< Fault control is enabled for all channels and automatic fault clearing */
} Pwm_Hal_FaultCtrlModeType;

/**
 * @brief PWM quadrature decode mode enumeration
 * @note DES ID: DES_PWM_TYP_224
 */
typedef enum
{
    PWM_QUAD_PHASE_ENCODE = 0U, /*!< Phase encoding mode */
    PWM_QUAD_COUNT_DIR_ENCODE   /*!< Counter and direction encoding mode */
} Pwm_Hal_QuadModeType;

/**
 * @brief PWM quadrature phase polarity enumeration
 * @note DES ID: DES_PWM_TYP_225
 */
typedef enum
{
    PWM_QUAD_PHASE_NORMAL = 0U, /*!< Phase input signal is not inverted */
    PWM_QUAD_PHASE_INVERT       /*!< Phase input signal is inverted */
} Pwm_Hal_QuadPhasePolarityType;

#if defined (AC7843X)
/**
 * @brief PWM quadrature Z index reset mode enumeration
 * @note DES ID: DES_PWM_TYP_226
 */
typedef enum
{
    PWM_QUAD_RESET_AT_NEXT_VAILD_EDGE = 0U, /*!< Counter reset at next valid edge */
    PWM_QUAD_RESET_IMMEDIATELY = 1U         /*!< Counter reset immediately */
} Pwm_Hal_QuadPhaseZResetMode;
#endif

/*PRQA S 1535 ++ # AMCE_PWM_1535. */
/**
 * @brief PWM debug mode enumeration
 * @note DES ID: DES_PWM_TYP_227
 */
typedef enum
{
    PWM_DEBUG_OFF = 0U,        /*!< No effect for counter, channel output, CNTIN/MOD/CnV/Cn+1V buffer and registers */
    PWM_DEBUG_OUTPUT_PREVIOUS, /*!< Counter stopped, channel output remains previous value, write CNTIN/MOD/CnV/Cn+1V buffer and immediately update to registers */
    PWM_DEBUG_OUTPUT_HIZ       /*!< Counter stopped, channel output remains high resistance value, write CNTIN/MOD/CnV/Cn+1V buffer and immediately update to registers */
} Pwm_Hal_DebugModeType;
/*PRQA S 1535 -- # AMCE_PWM_1535. */

/*=======================================STRUCTURES AND OTHER TYPEDEFS==============================*/
/**
 * @brief PWM callback function pointer type
 * @note DES ID: DES_PWM_TYP_228
 */
typedef void (*Pwm_Hal_Callback)(Pwm_Hal_InstanceType Instance, uint32 Status, const void *UserInfo);

/*PRQA S 3630 ++ # AMCE_QAC_3630 */
/**
 * @brief PWM common base configuration structure
 * @note DES ID: DES_PWM_TYP_229
 */
typedef struct
{
    Pwm_Hal_ClockSourceType ClockSource;  /*!< Select clock source */
    uint16 Prescaler;                     /*!< Set clock prescaler */
    uint16 MinCount;                      /*!< Initial counter value */
    uint16 MaxCount;                      /*!< Maximum counter value */
    uint8 PeriodDither;                   /*!< Period dither value */
    boolean EnableDmaTransLen;            /*!< Enable when DMA transmission length exceeding 32b */
    uint8 DmaTransLen;                    /*!< DMA transmission length */
    boolean EnableOverflowEvent;          /*!< Enable overflow event */
    boolean EnableUnderflowEvent;         /*!< Enable underflow event */
    boolean EnableOverflowInterrupt;      /*!< Enable overflow interrupt */
    boolean EnableOverflowDmaReq;         /*!< Request DMA when overflow */
    uint8 OverflowFreq;                   /*!< Overflow ratio */
    Pwm_Hal_Callback OverflowCallback;    /*!< Overflow callback handler */
    Pwm_Hal_Callback ChannelCallback;     /*!< Channel callback handler */
} Pwm_Hal_CommonCfg;

/**
 * @brief PWM independent channel configuration structure
 * @note DES ID: DES_PWM_TYP_230
 */
typedef struct
{
    uint16 ChnValue;                             /*!< Channel value */
    uint8 ChnDither;                             /*!< Channel dither value */
    boolean EnableMatchTrigger;                  /*!< Channel match trigger enable state */
    boolean EnableInterrupt;                     /*!< Channel interrupt enable state */
    boolean EnableChnEventDmaReq;                /*!< Channel interrupt DMA request enable state */
    Pwm_Hal_OutputLevelType InitLevel;           /*!< Channel initial level value */
    Pwm_Hal_ActivePolarityType ActivePolarity;   /*!< Channel active polarity */
    Pwm_Hal_OutputLevelModeType LevelMode;       /*!< Channel level mode */
} Pwm_Hal_IndependentChnCfg;

/**
 * @brief PWM combine mode configuration structure
 * @note DES ID: DES_PWM_TYP_231
 */
typedef struct
{
    uint16 Ch1stValue;                             /*!< Even channel value */
    uint8 Ch1stDither;                             /*!< Even channel dither value */
    boolean EnableCh1stMatchTrigger;               /*!< Even channel match trigger enable state */
    boolean EnableCh1stInterrupt;                  /*!< Even channel interrupt enable state */
    boolean EnableCh1stEventDmaReq;                /*!< Even channel interrupt DMA request enable state */
    Pwm_Hal_OutputLevelType Ch1stInitLevel;        /*!< Even channel initial level value */
    Pwm_Hal_ActivePolarityType Ch1stActivePolarity;/*!< Even channel active polarity */
    Pwm_Hal_ChannelMatchDirType Ch1stMatchDir;     /*!< Up or down counting to match */
    uint16 Ch2ndValue;                             /*!< Odd channel value */
    uint8 Ch2ndDither;                             /*!< Odd channel dither value */
    boolean EnableCh2ndMatchTrigger;               /*!< Odd channel match trigger enable state */
    boolean EnableCh2ndInterrupt;                  /*!< Odd channel interrupt enable state */
    boolean EnableCh2ndEventDmaReq;                /*!< Odd channel interrupt DMA request enable state */
    Pwm_Hal_OutputLevelType Ch2ndInitLevel;        /*!< Odd channel initial level value */
    Pwm_Hal_ActivePolarityType Ch2ndActivePolarity;/*!< Even channel active polarity */
    Pwm_Hal_ChannelMatchDirType Ch2ndMatchDir;     /*!< Up or down counting to match */
    Pwm_Hal_OutputLevelModeType LevelMode;         /*!< Level mode */
    boolean EnableChComplementation;               /*!< Enable channel level complementation */
    boolean EnableSymmetric;                       /*!< Enable channel level symmetric */
    boolean EnableDeadTime;                        /*!< Enable deadtime */
    uint16 DeadTimeValue;                          /*!< Deadtime value */
    Pwm_Hal_DeadTimePscType TimePsc;               /*!< Deadtime prescaler */
} Pwm_Hal_CombineChnPairCfg;

/**
 * @brief PWM compare mode configuration structure
 * @note DES ID: DES_PWM_TYP_232
 */
typedef struct
{
    uint16 ChnValue;                           /*!< Channel value */
    boolean EnableMatchTrigger;                /*!< Channel match trigger enable state */
    boolean EnableInterrupt;                   /*!< Channel interrupt enable state */
    boolean EnableChnEventDmaReq;              /*!< Channel interrupt DMA request enable state */
    Pwm_Hal_OutputLevelType InitLevel;         /*!< Channel initial level value */
    Pwm_Hal_ActivePolarityType ActivePolarity; /*!< Channel active polarity */
    Pwm_Hal_CompareActionType Action;          /*!< Channel value match action */
} Pwm_Hal_CompareModeChnCfg;

/**
 * @brief PWM output mode common configuration structure
 * @note DES ID: DES_PWM_TYP_233
 */
typedef struct
{
    Pwm_Hal_CommonCfg *BaseCfg;                             /*!< Common configuration */
    Pwm_Hal_CountModeType CountMode;                        /*!< Counter mode, up or up-down */
    boolean EnableInitTrigger;                              /*!< Initial value match trigger */
    boolean EnableMaxTrigger;                               /*!< Maximum value match trigger */
    uint8 TriggerRatio;                                     /*!< Trigger ratio */
    boolean InitOutput;                                     /*!< Initial level */
    Pwm_Hal_CombineCenterDutyModeType CombineCenterDutyMode;/*!< Combine center duty mode */
} Pwm_Hal_OutputCommonCfg;

/**
 * @brief PWM output channel configuration structure
 * @note DES ID: DES_PWM_TYP_234
 */
typedef struct
{
    Pwm_Hal_ChannelType Channel;                  /*!< Channel ID */
    Pwm_Hal_OutputChnModeType ChannelMode;        /*!< Channel output mode */
    Pwm_Hal_IndependentChnCfg *IndependentChnCfg; /*!< Independent mode configuration */
    Pwm_Hal_CombineChnPairCfg *ChnPairCfg;        /*!< Combine mode configuration */
    Pwm_Hal_CompareModeChnCfg *CompareChnCfg;     /*!< Output compare mode configuration */
} Pwm_Hal_OutputChannelCfg;

/**
 * @brief PWM output mode configuration structure
 * @note DES ID: DES_PWM_TYP_235
 */
typedef struct
{
    Pwm_Hal_OutputCommonCfg CommonCfg;    /*!< Hardware unit common configuration */
    Pwm_Hal_OutputChannelCfg *ChannelCfg; /*!< Channel configuration */
    uint8 ChannelNum;                     /*!< Channel number */
} Pwm_Hal_OutputCfg;

/**
 * @brief PWM input mode common configuration structure
 * @note DES ID: DES_PWM_TYP_236
 */
typedef struct
{
    Pwm_Hal_CommonCfg *BaseCfg;             /*!< Common configuration */
    boolean EnableHall;                     /*!< Enable hall */
    Pwm_Hal_InputFilterPscType FilterPsc;   /*!< Filter prescaler */
} Pwm_Hal_InputCommonCfg;

/**
 * @brief PWM input channel configuration structure
 * @note DES ID: DES_PWM_TYP_237
 */
typedef struct
{
    Pwm_Hal_ChannelType Channel;                                /*!< Channel ID */
    Pwm_Hal_InputChnModeType ChannelMode;                       /*!< Channel mode */
    Pwm_Hal_EdgeType CaptureEdge;                               /*!< Edge to capture */
    Pwm_Hal_InputEventPscType EventPsc;                         /*!< Event prescaler */
    Pwm_Hal_DualInputContinuousModeType DualInputContinuousMode;/*!< Continuous capture or not */
    boolean EnablePulseWidthMeasure;                            /*!< Enable pulse width measure or not */
    Pwm_Hal_DualInputMeasureType DualInputMeasureType;          /*!< Measure type for dual capture mode */
    boolean EnableCounterReset;                                 /*!< Reset counter after captured */
    boolean EnableInterrupt;                                    /*!< Enable capture interrupt */
    boolean EnableChnEventDmaReq;                               /*!< Enable capture DMA request */
    uint8 FilterValue;                                          /*!< Input filter value */
} Pwm_Hal_InputChannelCfg;

/**
 * @brief PWM input mode configuration structure
 * @note DES ID: DES_PWM_TYP_238
 */
typedef struct
{
    Pwm_Hal_InputCommonCfg CommonCfg;    /*!< Hardware unit common configuration */
    Pwm_Hal_InputChannelCfg *ChannelCfg; /*!< Channel configuration */
    uint8 ChannelNum;                    /*!< Channel number */
} Pwm_Hal_InputCfg;

/**
 * @brief PWM synchronization configuration structure
 * @details Please don't use software and hardware trigger simultaneously
 * @note DES ID: DES_PWM_TYP_239
 */
typedef struct
{
    Pwm_Hal_SyncTriggerMethodType TriggerType; /*!< Synchronization trigger mode */
    boolean EnableHwSync0;                     /*!< Enable/disable hardware sync trigger source 0 */
    boolean EnableHwSync1;                     /*!< Enable/disable hardware sync trigger source 1 */
    boolean EnableHwSync2;                     /*!< Enable/disable hardware sync trigger source 2 */
    boolean DisableHwTriggerAfterTrigged;      /*!< Available only for hardware trigger */
    boolean EnableCounterInitSync;             /*!< Enable/disable CNTIN sync */
    boolean EnableSwOutputCtrlSync;            /*!< Enable/disable CHOSWCR sync */
    boolean EnableDualChannelInvertSync;       /*!< Enable/disable INVCR sync */
    boolean EnableOutputMaskSync;              /*!< Enable/disable OMCR sync */
    boolean EnablePolaritySync;                /*!< Enable/disable CHOPOLCR sync */
    boolean EnablePairedChnValSync[PWM_CHANNEL_PAIR_NUM]; /*!< Enable/disable dual channel CHV sync */
    boolean EnableMaxLoadingPoint;             /*!< Enable/disable maximum loading point */
    boolean EnableMinLoadingPoint;             /*!< Enable/disable minimum loading point */
    boolean EnableSyncBypass;                  /*!< Enable/disable Synchronization bypass */
    boolean EnableSync;                        /*!< PWM Enable/disable */
} Pwm_Hal_SyncCfg;

/**
 * @brief PWM fault channel configuration structure
 * @note DES ID: DES_PWM_TYP_240
 */
typedef struct
{
    boolean EnableFaultInput;                 /*!< Fault input channel state */
    boolean EnableFaultFilter;                /*!< Fault channel filter state */
    Pwm_Hal_ActivePolarityType FaultPolarity; /*!< Fault channel input polarity active */
} Pwm_Hal_FaultPinCfg;

/*PRQA S 1535 ++ # AMCE_PWM_1535. */
/**
 * @brief PWM fault configuration structure
 * @note DES ID: DES_PWM_TYP_241
 */
typedef struct
{
    Pwm_Hal_FaultCtrlModeType FaultCtrlMode;              /*!< Fault mode */
    Pwm_Hal_FaultPinCfg FaultPinCfg[PWM_FAULT_INPUT_MAX]; /*!< Fault input channels configuration */
    uint8 FilterValue;                                    /*!< Fault filter value */
    Pwm_Hal_InputFilterPscType FilterPsc;                 /*!< Fault filter prescaler */
    boolean EnableChannelOutputCtrl[PWM_CHANNEL_PAIR_NUM];/*!< Fault control channel output state */
    boolean EnableInterrupt;                              /*!< Enable PWM fault interrupt */
    boolean EnableHiz;                                    /*!< Enable PWM fault high impedance output */
    Pwm_Hal_Callback FaultCallback;
} Pwm_Hal_FaultCfg;
/*PRQA S 1535 -- # AMCE_PWM_1535. */

/**
 * @brief PWM quadrature decoder phase input configuration structure
 * @note DES ID: DES_PWM_TYP_242
 */
typedef struct
{
    Pwm_Hal_QuadPhasePolarityType Polarity; /*!< PhaseA Polarity */
    uint8 FilterValue;                      /*!< Filter value */
} Pwm_Hal_QuadPhaseCfg;

/*PRQA S 1535 ++ # AMCE_PWM_1535. */
/**
 * @brief PWM quadrature configuration structure
 * @note DES ID: DES_PWM_TYP_243
 */
typedef struct
{
    Pwm_Hal_CommonCfg *BaseCfg;        /*!< PWM common configuration */
    Pwm_Hal_QuadModeType Mode;         /*!< PWM quadrature decode mode */
    Pwm_Hal_QuadPhaseCfg PhaseAConfig; /*!< PhaseA configuration */
    Pwm_Hal_QuadPhaseCfg PhaseBConfig; /*!< PhaseB configuration */
    Pwm_Hal_QuadPhaseCfg PhaseZConfig; /*!< PhaseZ configuration */
    boolean EnableQuad;                /*!< Enable quadrature decode mode */
    Pwm_Hal_InputFilterPscType FilterPsc; /*!< Input Capture Filter prescaler */
    boolean EnablePhaseZReset;         /*!< Reset the counter when Z index event is detected */
#if defined (AC7843X)
    Pwm_Hal_QuadPhaseZResetMode PhaseZResetMode; /*!< Reset mode when Z index event is detected */
    boolean EnablePhaseZInterrupt;     /*!< Enable/disable interrupt when z index event is detected */
    Pwm_Hal_Callback ZDetectCallback;
#endif
} Pwm_Hal_QuadDecoderCfg;
/*PRQA S 1535 -- # AMCE_PWM_1535. */
/*PRQA S 3630 -- # AMCE_QAC_3630*/

#ifdef __cplusplus
}
#endif

#endif /* PWM_HAL_TYPES_H */
