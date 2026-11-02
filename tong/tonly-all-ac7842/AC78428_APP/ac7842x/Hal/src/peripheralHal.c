/*************************************************
 Copyright © 2026 SiRun (AnHui) . All rights reserved.
 File Name: peripheralHal.c
 Author: HuShangrui
 Created Time: 2026年7月10日
 Description: 
 Others: 
*************************************************/
/****************************** include ***************************************/
#include "peripheralHal.h"
#include "Core_Hal.h"
#include "Gpio_Hal.h" 
#include "Wdg_Hal.h" 
#include "Timer_Hal.h"
#include "Adc_Hal.h"
#include "AC7842x_Features.h"
#include "batteryHal.h"
#include "logHal.h"
#include "timerHal.h"
#include "canHal.h"
#include "mpuHal.h"
#include "ecallHal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "taskEcallProcess.h"
#include "powerManageSdk.h"

/****************************** Macro Definitions ******************************/
#define WAKEUP_SOURCE_NONE            (0U)     
#define WAKEUP_SOURCE_CAN1            (1U)     
#define WAKEUP_SOURCE_CAN2            (2U)     
#define WAKEUP_SOURCE_CAN3            (3U)     
#define WAKEUP_SOURCE_CAN4            (4U)      
#define WAKEUP_SOURCE_CAN5            (5U)     
#define WAKEUP_SOURCE_CAN6            (6U)     
#define WAKEUP_SOURCE_CAN7            (7U)     
#define WAKEUP_SOURCE_CAN8            (8U)     
#define WAKEUP_SOURCE_MPU             (9U)     
#define WAKEUP_SOURCE_KL15            (10U)     
#define WAKEUP_SOURCE_KL30            (11U)  
#define WAKEUP_SOURCE_BLE             (12U)     
#define WAKEUP_SOURCE_RTC             (13U)
#define WAKEUP_SOURCE_RTC_MCU         (14U)
#define WAKEUP_SOURCE_GSENSOR         (15U)
#define WAKEUP_SOURCE_CHARGE_FAST     (16U)
#define WAKEUP_SOURCE_CHARGE_SLOW     (17U)
#define WAKEUP_SOURCE_SPI             (20U)

#define WDOG_TIMEOUT_MS               (2000U)

#define KL15_IRQ_PORT                 PORTID_C
#define KL15_IRQ_PIN                  9
#define KL15_IRQ_PIN_MUX              PORT_MUX_AS_GPIO
#define KL15_IRQ_IRQN                 PORTC_IRQn

#define KL30_IRQ_PORT                 PORTID_E
#define KL30_IRQ_PIN                  2
#define KL30_IRQ_PIN_MUX              PORT_MUX_AS_GPIO
#define KL30_IRQ_IRQN                 PORTE_IRQn

#define GSM_IRQ_PORT                  PORTID_B                    
#define GSM_IRQ_PIN                   9
#define GSM_IRQ_PIN_MUX               PORT_MUX_AS_GPIO
#define GSM_IRQ_IRQN                  PORTB_IRQn

#define Gsensor_IRQ_PORT              PORTID_B
#define Gsensor_IRQ_PIN               11
#define Gsensor_IRQ_PIN_MUX           PORT_MUX_AS_GPIO
#define Gsensor_IRQ_IRQN              PORTB_IRQn

#define CHARGE_FAST_IRQ_PORT          PORTID_A
#define CHARGE_FAST_IRQ_PIN           11
#define CHARGE_FAST_IRQ_PIN_MUX       PORT_MUX_AS_GPIO
#define CHARGE_FAST_IRQ_IRQN          PORTA_IRQn

#define CHARGE_SLOW_IRQ_PORT          PORTID_A
#define CHARGE_SLOW_IRQ_PIN           6
#define CHARGE_SLOW_IRQ_PIN_MUX       PORT_MUX_AS_GPIO
#define CHARGE_SLOW_IRQ_IRQN          PORTA_IRQn

#define SYS_ON_PORT                   PORTID_D
#define SYS_ON_PIN                    10
#define SYS_ON_PIN_MUX                PORT_MUX_AS_GPIO

#define ADC_KL30_PORT                 PORTID_B
#define ADC_KL30_PIN                  3U
#define ADC_KL30_PIN_MUX              PORT_MUX_ALT0

#define ADC_BATT_PORT                 PORTID_E
#define ADC_BATT_PIN                  6U
#define ADC_BATT_PIN_MUX              PORT_MUX_ALT0

#define ADC_NTC_PORT                  PORTID_E
#define ADC_NTC_PIN                   2U
#define ADC_NTC_PIN_MUX               PORT_MUX_ALT0

#define ADC_BTN_PORT                  PORTID_D
#define ADC_BTN_PIN                   3U
#define ADC_BTN_PIN_MUX               PORT_MUX_ALT0

#define ADC_MIC_PORT                  PORTID_A
#define ADC_MIC_PIN                   6U
#define ADC_MIC_PIN_MUX               PORT_MUX_ALT0

#define SYS_LED_PORT                  PORTID_D
#define SYS_LED_PIN                   11
#define SYS_LED_PIN_MUX               PORT_MUX_AS_GPIO

#define PERIPHERAL_TIMER_INDEX        1

#define MAX_SOFTWARE_TIMERS           (5U)
#define TIMER_UNUSED                  (0U)
#define TIMER_RUNNING                 (1U)
#define TIMER_STOPPED                 (2U)

#define KL30_SLOPE_Q20                (9172681UL)
#define KL30_OFFSET_mV                (347L)

// 定义校准点的数量
#define KL30_CAL_POINT_NUM  5

#define Q20_SHIFT 20
#define CALC_Q20(v_diff, raw_diff) (uint32_t)((((uint64_t)(v_diff)) << Q20_SHIFT) / (raw_diff))

// 使用位掩码代替取模，使用滑动和代替 for 循环累加
#define FILTER_SHIFT 3                    // 2^3 = 8
#define FILTER_LEN   (1 << FILTER_SHIFT)  // 滤波器长度必须是 2 的整数次幂
#define FILTER_MASK  (FILTER_LEN - 1)     // 即 0x07

/****************************** Type Definitions ******************************/
typedef struct
{
  uint16_t adValue;
  uint8_t adValid;  
} AdConvertBuffer_t;

typedef struct {
    uint8_t                 state;
    uint32_t                interval;
    uint32_t                counter;
    SoftwareTimerCallback_t callback;
} SoftwareTimer_t;

typedef struct {
    uint16_t adcRaw;    
    uint32_t voltage_mV;
    uint32_t slope_Q20; // 预存该段的 Q20 斜率
} Kl30CalPoint_t;

/****************************** Global Variables ******************************/
static SoftwareTimer_t g_softwareTimers[MAX_SOFTWARE_TIMERS];
AdConvertBuffer_t g_adBuffer[17];  /* expanded to accommodate AD_CHANNEL_HW_VER (index 16) */
typeSetWakeupSourceCallbackPtr g_wakeupSourceFun;

static uint8_t g_ledSleepFlag = 0;
static uint8_t g_adcInitSatate = 0;
static uint8_t g_adcCompletedFlag = 0;

static const Kl30CalPoint_t kl30_cal_table[] = {
    {648,  6000,  CALC_Q20(8000-6000,   875-648)},    // 0: 6V->8V 段
    {875,  8000,  CALC_Q20(12000-8000,  1335-875)},   // 1: 8V->12V 段
    {1335, 12000, CALC_Q20(15000-12000, 1676-1335)},  // 2: 12V->15V 段
    {1676, 15000, CALC_Q20(18000-15000, 2017-1676)},  // 3: 15V->18V 段
    {2015, 18000, CALC_Q20(18000-15000, 2015-1676)}   // 4: 18V+外推段 (复用上一段斜率)
};


/****************************** Function Declarations *************************/
static void PeripheralHalSysOnInit(void);
static void PeripheralHalAdc0Init(void);
static void PeripheralHalAdc1Init(void);
static void Timer1Handler(void *Args);
static void PeripheralHal1msTimerInit(void);
static void PeripheralStartSysLed(void);
static void PeripheralHalStopSysLed(void);
static void PeripheralHalRestartSysLed(void);
static void MCWDTInit(void);
static void IrqPinInit(void);
static void SoftwareTimerProcess(void);
static void PeripheralHalAdcHalSetMode(uint8_t mode);
static void PeripheralHalAdc0PinDeinit(void);
static void PeripheralHalAdc1PinDeinit(void);
static uint32_t PeripheralHalKl30RawTo_mV(uint16_t raw);

/* 新版 ADC 回调声明 */
static void ADC0_EventCallback(const Adc_InterruptInfoType *Info);
static void ADC1_EventCallback(const Adc_InterruptInfoType *Info);

/****************************** Public Function Implementations ***************/
/*************************************************
  Function:       PeripheralHalRegisterSetWakeupSourceCallback
  Description:    Register callback function for wakeup source notification
  Input:          pCallBackFun - Pointer to callback function
  Output:         none
  Return:         none
  Others:         Stores callback function for later use when wakeup events occur
*************************************************/
void PeripheralHalRegisterSetWakeupSourceCallback(typeSetWakeupSourceCallbackPtr pCallBackFun)
{
    g_wakeupSourceFun = pCallBackFun;
}

/*************************************************
  Function:       PeripheralHal_PORTAIRQHandler
  Description:    Interrupt handler for PORT A peripheral
  Input:          none
  Output:         none
  Return:         none
  Others:         Handles CANFD2 RX interrupt and notifies wakeup source
*************************************************/
void PeripheralHal_PORTAIRQHandler(uint8 PortId, uint32 Status)
{
    if ((Status & (1 << CANFD2_RX_PIN))!= 0u)
    {
        if (g_wakeupSourceFun != NULL)
        {
            g_wakeupSourceFun(WAKEUP_SOURCE_CAN2);
        }
    }
    else if ((Status & (1 << CHARGE_FAST_IRQ_PIN))!= 0u)
    {
        if (g_wakeupSourceFun != NULL)
        {
            g_wakeupSourceFun(WAKEUP_SOURCE_CHARGE_FAST);
        }
    }
    else if ((Status & (1 << CHARGE_SLOW_IRQ_PIN))!= 0u)
    {
        if (g_wakeupSourceFun != NULL)
        {
            g_wakeupSourceFun(WAKEUP_SOURCE_CHARGE_SLOW);
        }
    }

}

/*************************************************
  Function:       PeripheralHal_PORTBIRQHandler
  Description:    Interrupt handler for PORT B peripheral
  Input:          none
  Output:         none
  Return:         none
  Others:         Handles G-sensor interrupt and notifies wakeup source
*************************************************/
void PeripheralHal_PORTBIRQHandler(uint8 PortId, uint32 Status)
{
    if ((Status & (1 << CANFD1_RX_PIN))!= 0u)
    {
        if (g_wakeupSourceFun != NULL)
        {
            g_wakeupSourceFun(WAKEUP_SOURCE_CAN1);
        }
    }
    else if ((Status & (1 << CANFD3_RX_PIN))!= 0u)
    {
        if (g_wakeupSourceFun != NULL)
        {
            g_wakeupSourceFun(WAKEUP_SOURCE_CAN3);
        }
    }
    else if ((Status & (1 << GSM_IRQ_PIN))!= 0u)
    {
        PowerManageSdkSetMpuWakeUpFlag(0x01);
        if (g_wakeupSourceFun != NULL)
        {
            g_wakeupSourceFun(WAKEUP_SOURCE_MPU);
        }
    }
}

/*************************************************
  Function:       PeripheralHal_PORTCIRQHandler
  Description:    Interrupt handler for PORT C peripheral
  Input:          none
  Output:         none
  Return:         none
  Others:         Currently empty implementation
*************************************************/
void PeripheralHal_PORTCIRQHandler(uint8 PortId, uint32 Status)
{
    if ((Status & (1 << KL15_IRQ_PIN))!= 0u)
    {
        if (g_wakeupSourceFun != NULL)
        {
            g_wakeupSourceFun(WAKEUP_SOURCE_KL15);
        }
    }
}

/*************************************************
  Function:       PeripheralHal_PORTDIRQHandler
  Description:    Interrupt handler for PORT D peripheral
  Input:          none
  Output:         none
  Return:         none
  Others:         Handles button and SRS interrupts
                  Note: Wakeup source notifications are currently commented out
*************************************************/
void PeripheralHal_PORTDIRQHandler(uint8 PortId, uint32 Status)
{
    uint8_t srsIrqLevel = 0;
    if ((Status & (1 << SRS_STATE_PIN))!= 0u)
    {
        srsIrqLevel = Gpio_Hal_GetPinOutputLevel(SRS_STATE_PORT, SRS_STATE_PIN);
        AirbagPwmIsrHandler(srsIrqLevel);
    }
}

/*************************************************
  Function:       PeripheralHal_PORTEIRQHandler
  Description:    Interrupt handler for PORT E peripheral
  Input:          none
  Output:         none
  Return:         none
  Others:         Handles KL30, CANFD1 RX and GSM interrupts
                  Controls battery output based on KL30 state
                  Notifies wakeup sources accordingly
*************************************************/
void PeripheralHal_PORTEIRQHandler(uint8 PortId, uint32 Status)
{
    if ((Status & (1 << MPU_HAL_SPI_REQ_PORT_PIN))!= 0u)
    {
#if (SPI_DMA_ENABLE == 1)
        MpuHal_SpiIrqCallback();
#endif
        if (g_wakeupSourceFun != NULL)
        {
            g_wakeupSourceFun(WAKEUP_SOURCE_SPI);
        }
    }
    else if ((Status & (1 << CANFD4_RX_PIN))!= 0u)
    {
        if (g_wakeupSourceFun != NULL)
        {
            g_wakeupSourceFun(WAKEUP_SOURCE_CAN4);
        }
    }
    else if ((Status & (1 << KL30_IRQ_PIN))!= 0u)
    {
        // do nothing
    }
}

/*************************************************
  Function:       PeripheralHalGetKl15Status
  Description:    Read the status of KL15 ignition signal
  Input:          none
  Output:         none
  Return:         1 if KL15 is active , 0 if inactive
  Others:         Returns 1 in EVB board test mode regardless of actual pin state
*************************************************/
int16_t PeripheralHalGetKl15Status (void)
{
    return (Gpio_Hal_ReadPin(KL15_IRQ_PORT, KL15_IRQ_PIN) == GPIO_HIGH_LEVEL) ? 1 : 0;
}

/*************************************************
  Function:       PeripheralHalGetKl30Status
  Description:    Read the status of KL30 power supply signal
  Input:          none
  Output:         none
  Return:         1 if KL30 is active (low), 0 if inactive (high)
  Others:         Returns 1 in EVB board test mode regardless of actual pin state
                  Note: KL30 uses active low logic
*************************************************/
int16_t PeripheralHalGetKl30Status (void)
{
    return (Gpio_Hal_ReadPin(KL30_IRQ_PORT, KL30_IRQ_PIN) == GPIO_HIGH_LEVEL) ? 0 : 1;
}

/*************************************************
  Function:       PeripheralHalSetSysOn
  Description:    Set the state of SYS_ON pin
  Input:          flag - 1 to set SYS_ON high, 0 to set SYS_ON low
  Output:         none
  Return:         none
  Others:         Controls system power enable signal
*************************************************/
void PeripheralHalSetSysOn (uint8_t flag)
{
    Gpio_Hal_WritePin(SYS_ON_PORT, SYS_ON_PIN, flag ? GPIO_HIGH_LEVEL : GPIO_LOW_LEVEL);
}

/*************************************************
  Function:       ADC0_EventCallback
  Description:    ADC0 FIFO Watermark interrupt service routine
  Input:          none
  Output:         none
  Return:         none
  Others:         Reads conversion results from ADC0 FIFO
                  Stores results in global buffer
                  Configures next ADC channel based on current channel
                  Marks KL30 conversion as complete
                  Clears FIFO after reading
*************************************************/
static void ADC0_EventCallback(const Adc_InterruptInfoType *Info)
{
    if (Info->Event & ADC_EVENT_EOC)
    {
        g_adBuffer[AD_CHANNEL_BCALL_LIGHT].adValue = Adc_Hal_GetSeqResult(0, ADC_RSEQ_0);
        g_adBuffer[AD_CHANNEL_SOS_KEY].adValue     = Adc_Hal_GetSeqResult(0, ADC_RSEQ_1);
        g_adBuffer[AD_CHANNEL_ECALL_LIGHT].adValue = Adc_Hal_GetSeqResult(0, ADC_RSEQ_2);
        g_adBuffer[AD_CHANNEL_MIC_POWER].adValue   = Adc_Hal_GetSeqResult(0, ADC_RSEQ_3);
    }
}

/*************************************************
  Function:       ADC1_EventCallback
  Description:    ADC1 FIFO Watermark interrupt service routine
  Input:          none
  Output:         none
  Return:         none
  Others:         Reads conversion results from ADC1 FIFO
                  Stores KL30 channel results in global buffer
                  Clears FIFO after reading
                  Includes memory barriers for proper execution order
*************************************************/
static void ADC1_EventCallback(const Adc_InterruptInfoType *Info)
{
    if (Info->Event & ADC_EVENT_EOC)
    {
        g_adBuffer[AD_CHANNEL_KL30].adValue      = Adc_Hal_GetSeqResult(1, ADC_RSEQ_0);
        g_adBuffer[AD_CHANNEL_4G].adValue        = Adc_Hal_GetSeqResult(1, ADC_RSEQ_1);
        g_adBuffer[AD_CHANNEL_SYS_5V].adValue    = Adc_Hal_GetSeqResult(1, ADC_RSEQ_2);
        g_adBuffer[AD_CHANNEL_GPS].adValue       = Adc_Hal_GetSeqResult(1, ADC_RSEQ_3);
        g_adBuffer[AD_CHANNEL_GPS1].adValue      = Adc_Hal_GetSeqResult(1, ADC_RSEQ_4);
        g_adBuffer[AD_CHANNEL_MIC].adValue       = Adc_Hal_GetSeqResult(1, ADC_RSEQ_5);
        g_adBuffer[AD_CHANNEL_HW_VER].adValue    = Adc_Hal_GetSeqResult(1, ADC_RSEQ_6);
        g_adBuffer[AD_CHANNEL_BATTERY].adValue   = Adc_Hal_GetSeqResult(1, ADC_RSEQ_7);
        g_adBuffer[AD_CHANNEL_NTC].adValue       = Adc_Hal_GetSeqResult(1, ADC_RSEQ_8);
    }
}

/*************************************************
  Function:       PeripheralHalAdcStart
  Description:    Start ADC conversion for specified ADC module
  Input:          sarNo - ADC module number (0, 1, 2)
  Output:         none
  Return:         none
  Others:         Triggers ADC conversion and marks corresponding channel as valid
*************************************************/
void PeripheralHalAdcStart(uint8_t sarNo)
{
    if (g_adcInitSatate != 0x3) return;

    if (sarNo == 0)
    {
        g_adBuffer[AD_CHANNEL_BCALL_LIGHT].adValid = 1;
        g_adBuffer[AD_CHANNEL_ECALL_LIGHT].adValid = 1;
        g_adBuffer[AD_CHANNEL_SOS_KEY].adValid = 1;
        g_adBuffer[AD_CHANNEL_MIC_POWER].adValid = 1;
        Adc_Hal_SwTriggerRegularConvert(0); // 触发 ADC0 规则组
    }
    else if (sarNo == 1)
    {
        g_adBuffer[AD_CHANNEL_KL30].adValid = 1;
        g_adBuffer[AD_CHANNEL_SYS_5V].adValid = 1;
        g_adBuffer[AD_CHANNEL_4G].adValid = 1;
        g_adBuffer[AD_CHANNEL_GPS].adValid = 1;
        g_adBuffer[AD_CHANNEL_GPS1].adValid = 1;
        g_adBuffer[AD_CHANNEL_MIC].adValid = 1;
        g_adBuffer[AD_CHANNEL_HW_VER].adValid = 1;
        g_adBuffer[AD_CHANNEL_BATTERY].adValid = 1;
        g_adBuffer[AD_CHANNEL_NTC].adValid = 1;
        Adc_Hal_SwTriggerRegularConvert(1); // 触发 ADC1 规则组
    }
}

/*************************************************
  Function:       PeripheralHalAdGet
  Description:    Get ADC converted value from specified channel
  Input:          adChannel - ADC channel number (0-13)
                  pValue - Pointer to store converted voltage value
  Output:         Voltage value stored in pValue (unit: mV)
  Return:         0 - Success
                  -1 - Invalid channel number or other error
                  -2 - ADC conversion not valid
  Others:         Handles multiple ADC channels including KL30, battery, NTC, MICP and SOS key
                  Implements channel-specific voltage conversion calculations
                  Uses critical section to protect shared data access
                  Provides test mode with fixed return values
*************************************************/
int16_t PeripheralHalAdGet(uint8_t adChannel, uint32_t *pValue)
{
    uint32_t temData = 0;
    uint16_t adValue = 0;
    uint8_t adValid = 0;
    int16_t result = -2; // Default return value
    
    if ((pValue == NULL) ||
        (adChannel >= (uint8_t)(sizeof(g_adBuffer) / sizeof(g_adBuffer[0]))))
    {
        result = -1;
    }
    else
    {
        taskENTER_CRITICAL();
        adValid = g_adBuffer[adChannel].adValid;
        adValue = g_adBuffer[adChannel].adValue;
        taskEXIT_CRITICAL();     
        if(adValid != 0)
        {
            // Common calculation for all valid channels
            temData = adValue * AD_REF_VOLTAGE;
            temData = temData / 4095;
            
            // Apply channel-specific factor
            if(AD_CHANNEL_KL30 == adChannel)
            {
                temData = temData * AD_CHANNEL_KL30_FACTOR;
                *pValue = temData;
                result = 0;
            }
            else if (AD_CHANNEL_SOS_KEY == adChannel)
            {
                temData = temData * AD_CHANNEL_SOS_FACTOR;
                *pValue = temData;
                result = 0;
            }
            else if (AD_CHANNEL_4G == adChannel)
            {
                temData = temData * AD_CHANNEL_4G_FACTOR;
                *pValue = temData;
                result = 0;
            }
            else if (AD_CHANNEL_CAN_POWER == adChannel)
            {
                temData = temData * AD_CHANNEL_CAN_POWER_FACTOR;
                *pValue = temData;
                result = 0;
            }
            else if (AD_CHANNEL_GPS == adChannel)
            {
                temData = temData * AD_CHANNEL_GPS_FACTOR;
                *pValue = temData;
                result = 0;
            }
            else if (AD_CHANNEL_GPS1 == adChannel)
            {
                temData = temData * AD_CHANNEL_GPS1_FACTOR;
                *pValue = temData;
                result = 0;
            }
            else if (AD_CHANNEL_MIC == adChannel)
            {
                temData = temData * AD_CHANNEL_MIC_FACTOR;
                *pValue = temData;
                result = 0;
            }
            else if (AD_CHANNEL_MIC_POWER == adChannel)
            {
                temData = temData * AD_CHANNEL_MIC_POWER_FACTOR;
                *pValue = temData;
                result = 0;
            }
            else if (AD_CHANNEL_BCALL_LIGHT == adChannel)
            {
                *pValue = temData;
                result = 0;
            }
            else if (AD_CHANNEL_ECALL_LIGHT == adChannel)
            {
                *pValue = temData;
                result = 0;
            }
            else if (AD_CHANNEL_BATTERY == adChannel)
            {
                *pValue = temData;
                result = 0;
            }
            else if (AD_CHANNEL_NTC == adChannel)
            {
                *pValue = temData;
                result = 0;
            }
            else if (AD_CHANNEL_SYS_5V == adChannel)
            {
                temData = temData * AD_CHANNEL_SYS_5V_FACTOR / AD_CHANNEL_SYS_5V_DIVIDER;
                *pValue = temData;
                result = 0;
            }
            else if (AD_CHANNEL_HW_VER == adChannel)
            {
                temData = temData * AD_CHANNEL_HW_VER_FACTOR;
                *pValue = temData;
                result = 0;
            }
            else
            {
                result = -1;
            }
        }
        else 
        {
            result = -1;
        }
    }
    return result;
}

/*************************************************
  Function:       PeripheralHal1msTimerProcess
  Description:    1ms timer callback function for system operations
  Input:          none
  Output:         none
  Return:         none
  Others:         Toggles system LED every second when not in sleep mode
                  Uses static counters to track time intervals
                  Controlled by g_ledSleepFlag
*************************************************/
void PeripheralHal1msTimerProcess(void)
{
    static uint32_t sysCnt = 0;
    static uint8_t  flag = 0;
    sysCnt += 1;
    if (sysCnt < 1000) return;
    sysCnt = 0;
    
    if(!g_ledSleepFlag)
    {
        flag = !flag;
        Gpio_Hal_WritePin(SYS_LED_PORT, SYS_LED_PIN, flag ? GPIO_HIGH_LEVEL : GPIO_LOW_LEVEL);
    }   
}

/*****************************************************************************
 * Function:TIM1_Init
 * Description:Configure TIM1 module for timer interrupt function.
 * Input:
 * Output:
 * Others:1,
 *         2, Clock source selection description:
 *            TIM_CLK_SOURCE_SYSTEM--System clock source
 *            TIM_CLK_SOURCE_FUNCTION--Clock source selected by function "CLK_ModuleSrc"/PARCC;
 *            TIM_CLK_SOURCE_EXTERNAL--According to the content of function "TIM_ExternalCounterSelect", it is the external clock source input through pins;
 *        3, TIM module contains a 16-bit counter register, each output channel has a 16-bit comparison register. Up to 8 output channels,
 *            up to 2 fault signal input channels, up to 8 signal input channels.
****************************************************************************/
void TIM1_Init(void)
{
    /* 使用新版 Timer_Hal */
    Timer_Hal_Init(TIMER_CLOCK_SPLL);
    
    Timer_Channel_ConfigType timerCfg;
    timerCfg.Mode = TIMER_MODE_0;
    timerCfg.Config = TIMER_IRQ_EN;
    timerCfg.TriggerSrc = TRIG_SOURCE_TIMER_CH0;

    Timer_Hal_SetConfig(PERIPHERAL_TIMER_INDEX, &timerCfg);
    Timer_Hal_InstallCallback(PERIPHERAL_TIMER_INDEX, Timer1Handler, NULL);
    Timer_Hal_EnableInterrupt(PERIPHERAL_TIMER_INDEX, TIMER_INT_EN);
    
    // 配置为 1ms 触发
    uint32_t ticks = Timer_Hal_MicrosToTicks(1000); 
    Timer_Hal_Start(PERIPHERAL_TIMER_INDEX, ticks);
}

/*************************************************
  Function:       PeripheralHalSetMode
  Description:    Set peripheral operating mode (normal or low power)
  Input:          mode - Operating mode (non-zero for normal mode, zero for low power mode)
  Output:         none
  Return:         none
  Others:         In normal mode: enables system power, restarts system LED, enables audio amp, initializes timer
                  In low power mode: disables system power, stops system LED, disables audio amp, disables timer and clears interrupts
*************************************************/
void PeripheralHalSetMode(uint8_t mode)
{
    if (mode)
    {
        PeripheralHalSetSysOn(1);
        PeripheralHalRestartSysLed();
        Gpio_Hal_WritePin(AMP_STB_PORT, AMP_STB_PIN, GPIO_HIGH_LEVEL);
        Gpio_Hal_WritePin(AMP_MUTE_PORT, AMP_MUTE_PIN, GPIO_HIGH_LEVEL);
        TIM1_Init();
        PeripheralHalAdcHalSetMode(1);
    }
    else
    {
        PeripheralHalSetSysOn(0);
        PeripheralHalStopSysLed();
        Gpio_Hal_WritePin(AMP_STB_PORT, AMP_STB_PIN, GPIO_LOW_LEVEL);
        Gpio_Hal_WritePin(AMP_MUTE_PORT, AMP_MUTE_PIN, GPIO_LOW_LEVEL);
        Timer_Hal_Stop(PERIPHERAL_TIMER_INDEX);
        PeripheralHalAdcHalSetMode(0);
    }
}

/*************************************************
  Function:       PeripheralHalMcuHardReset
  Description:    Perform MCU hardware reset
  Input:          none
  Output:         none
  Return:         none
  Others:         Triggers system reset through NVIC
                  Execution does not return from this function
*************************************************/
void PeripheralHalMcuHardReset(void)
{
    Core_Hal_PerformReset();
}

/*************************************************
  Function:       PeripheralHalFeedWatchDog
  Description:    Feed system watchdog to prevent reset
  Input:          none
  Output:         none
  Return:         none
  Others:         Calls WDOG_Refresh() to reset watchdog counter
*************************************************/
void PeripheralHalFeedWatchDog(void)
{
    Wdg_Hal_Feed();    
}

/*************************************************
  Function:       PeripheralHalInit
  Description:    Initialize all system peripherals
  Input:          none
  Output:         none
  Return:         none
  Others:         Calls initialization functions for:
                  - Interrupt pins (KL15, KL30, GSM)
                  - Audio amplifier
                  - System power control
                  - ADC1 module
                  - System LED and 1ms timer
                  - Watchdog module
                  Contains commented-out ADC channel initialization functions
*************************************************/
void PeripheralHalInit(void)
{
    IrqPinInit();
    PeripheralHalSysOnInit();
    PeripheralHalAdc0Init();
    PeripheralHalAdc1Init();
    PeripheralStartSysLed();
    MCWDTInit();
}

/*************************************************
  Function:       PeripheralHalSysOnInit
  Description:    Initialize SYS_ON pin configuration
  Input:          none
  Output:         none
  Return:         none
  Others:         Configures SYS_ON pin as output and sets it high
*************************************************/
static void PeripheralHalSysOnInit (void)
{
    Gpio_Hal_SetMuxMode(SYS_ON_PORT, SYS_ON_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(SYS_ON_PORT, SYS_ON_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(SYS_ON_PORT, SYS_ON_PIN, GPIO_HIGH_LEVEL);
}

/*************************************************
  Function:       PeripheralHalAdc0PinDeinit
  Description:    Deinitialize ADC0 related pins by restoring to GPIO mode
  Input:          none
  Output:         none
  Return:         none
*************************************************/
static void PeripheralHalAdc0PinDeinit(void)
{
    // E_call 指示灯短路到地 (PORTC_15)
    Gpio_Hal_SetMuxMode(PORTID_C, 15U, PORT_MUX_AS_GPIO);
    // B_call 指示灯短路到地 (PORTC_14)
    Gpio_Hal_SetMuxMode(PORTID_C, 14U, PORT_MUX_AS_GPIO);
    // E-CALL 按键监测 (PORTC_17)
    Gpio_Hal_SetMuxMode(PORTID_C, 17U, PORT_MUX_AS_GPIO);
    /* PTA6 (ADC_MIC) 已改作 S_CHARGE 唤醒输入，屏蔽 ADC 引脚反初始化 */
    // Gpio_Hal_SetMuxMode(ADC_MIC_PORT, ADC_MIC_PIN, PORT_MUX_AS_GPIO);
}

/*************************************************
  Function:       PeripheralHalAdc0PinConfig
  Description:    Configure all ADC0 related pins
  Input:          none
  Output:         none
  Return:         none
  Others:         none
*************************************************/
static void PeripheralHalAdc0PinConfig(void)
{
    Gpio_Hal_SetMuxMode(PORTID_C, 15, PORT_MUX_ALT0);
    Gpio_Hal_SetMuxMode(PORTID_C, 14, PORT_MUX_ALT0);
    Gpio_Hal_SetMuxMode(PORTID_C, 17, PORT_MUX_ALT0);
    /* PTA6 (ADC_MIC) 已改作 S_CHARGE 唤醒输入，屏蔽 ADC 引脚配置 */
    // Gpio_Hal_SetMuxMode(ADC_MIC_PORT,  ADC_MIC_PIN, ADC_MIC_PIN_MUX);
}


/*************************************************
  Function:       PeripheralHalAdc1PinConfig
  Description:    Configure all ADC1 related pins
  Input:          none
  Output:         none
*************************************************/
static void PeripheralHalAdc1PinConfig(void)
{
    Gpio_Hal_SetMuxMode(PORTID_A, 15, PORT_MUX_ALT0);
    Gpio_Hal_SetMuxMode(PORTID_C,  6, PORT_MUX_ALT0);
    Gpio_Hal_SetMuxMode(PORTID_C,  7, PORT_MUX_ALT0);
    Gpio_Hal_SetMuxMode(PORTID_A, 16, PORT_MUX_ALT0);
    /* PTD2 (ADC1_IN2): GNSS 天线 2 检测通道 (CAN STB 已迁移至 PC10) */
    Gpio_Hal_SetMuxMode(PORTID_D,  2, PORT_MUX_ALT0);
    /* PTD4 (ADC1_IN6): 硬件版本检测通道 AD_CHANNEL_HW_VER */
    Gpio_Hal_SetMuxMode(PORTID_D,  4, PORT_MUX_ALT0);
    /* PTE6 (ADC1_IN11): 备用电池电压采样通道 AD_CHANNEL_BATTERY */
    Gpio_Hal_SetMuxMode(ADC_BATT_PORT, ADC_BATT_PIN, ADC_BATT_PIN_MUX);
    /* PTE2 (ADC1_IN10): 备用电池 NTC 温度采样通道 AD_CHANNEL_NTC */
    Gpio_Hal_SetMuxMode(ADC_NTC_PORT, ADC_NTC_PIN, ADC_NTC_PIN_MUX);
}

/*************************************************
  Function:       PeripheralHalAdc1PinDeinit
  Description:    Deinitialize all ADC1 related pins by restoring to GPIO mode
  Input:          none
  Output:         none
*************************************************/
static void PeripheralHalAdc1PinDeinit(void)
{    
    /* KL30 电压通道 */
    Gpio_Hal_SetMuxMode(PORTID_A, 15U, PORT_MUX_AS_GPIO);

    /* 4G 电源通道 */
    Gpio_Hal_SetMuxMode(PORTID_C, 6U, PORT_MUX_AS_GPIO);

    /* CAN 电源通道 */
    Gpio_Hal_SetMuxMode(PORTID_C, 7U, PORT_MUX_AS_GPIO);

    /* GPS 电源通道 */
    Gpio_Hal_SetMuxMode(PORTID_A, 16U, PORT_MUX_AS_GPIO);

    /* GNSS 天线 2 检测通道 - PTD2 (CAN STB 已迁移至 PC10) */
    Gpio_Hal_SetMuxMode(PORTID_D, 2U, PORT_MUX_AS_GPIO);

    /* 硬件版本检测通道 - PTD4 */
    Gpio_Hal_SetMuxMode(PORTID_D, 4U, PORT_MUX_AS_GPIO);

    /* 备用电池电压采样通道 - PTE6 (ADC1_IN11) */
    Gpio_Hal_SetMuxMode(ADC_BATT_PORT, ADC_BATT_PIN, PORT_MUX_AS_GPIO);

    /* 备用电池 NTC 温度采样通道 - PTE2 (ADC1_IN10) */
    Gpio_Hal_SetMuxMode(ADC_NTC_PORT, ADC_NTC_PIN, PORT_MUX_AS_GPIO);
}

/*************************************************
  Function:       PeripheralHalAdc0Init
  Description:    Initialize ADC0 module for multiple analog inputs (KL30, battery, NTC)
  Input:          none
  Output:         none
  Return:         none
  Others:         Configures ADC0 hardware with FIFO and interrupt settings
*************************************************/
static void PeripheralHalAdc0Init(void)
{
    PeripheralHalAdc0PinConfig();

    Adc_InitConfigType initCfg = { .Interrupt = { .Callback = ADC0_EventCallback } };
    Adc_Hal_Init(0, &initCfg);

    Adc_ConverterConfigType convCfg;
    Adc_Hal_InitConverterStruct(&convCfg);
    convCfg.Resolution = ADC_RESOLUTION_12BIT;
    convCfg.HwAverage.Enable = TRUE;
    convCfg.HwAverage.Value = ADC_AVERAGE_16;
    convCfg.PowerEn = TRUE;
    Adc_Hal_ConfigConverter(0, &convCfg);

    /* 序列分配 */
    Adc_ChanConfigType chanCfg;
    Adc_Hal_InitChanStruct(&chanCfg);
    chanCfg.Spt = ADC_SPT_CLK_85;
    
    chanCfg.Channel = ADC_CH_12; chanCfg.SeqIndex = ADC_RSEQ_0; Adc_Hal_ConfigChannel(0, &chanCfg);
    chanCfg.Channel = ADC_CH_15; chanCfg.SeqIndex = ADC_RSEQ_1; Adc_Hal_ConfigChannel(0, &chanCfg);
    chanCfg.Channel = ADC_CH_13; chanCfg.SeqIndex = ADC_RSEQ_2; Adc_Hal_ConfigChannel(0, &chanCfg);
    chanCfg.Channel = ADC_CH_2;  chanCfg.SeqIndex = ADC_RSEQ_3; 
    chanCfg.InterruptEn = TRUE; // 仅最后一个序列开启 EOC 中断
    Adc_Hal_ConfigChannel(0, &chanCfg);

    Adc_GroupConfigType groupCfg;
    Adc_Hal_InitGroupStruct(&groupCfg);
    groupCfg.RegularSequenceLength = 4;
    groupCfg.RegularTrigger = ADC_TRIGG_SRC_SW;
    groupCfg.ScanModeEn = TRUE;
    Adc_Hal_ConfigRegularGroup(0, &groupCfg);

    g_adcInitSatate |= 0x1;
}

/*************************************************
  Function:       PeripheralHalAdc1Init
  Description:    Initialize ADC1 module for KL30 voltage measurement
  Input:          none
  Output:         none
  Return:         none
  Others:         Configures ADC1 hardware with FIFO and interrupt settings
*************************************************/
static void PeripheralHalAdc1Init(void)
{
    PeripheralHalAdc1PinConfig();

    Adc_InitConfigType initCfg = { .Interrupt = { .Callback = ADC1_EventCallback } };
    Adc_Hal_Init(1, &initCfg);

    Adc_ConverterConfigType convCfg;
    Adc_Hal_InitConverterStruct(&convCfg);
    convCfg.Resolution = ADC_RESOLUTION_12BIT;
    convCfg.HwAverage.Enable = TRUE;
    convCfg.HwAverage.Value = ADC_AVERAGE_16;
    convCfg.PowerEn = TRUE;
    Adc_Hal_ConfigConverter(1, &convCfg);

    /* 序列分配 */
    Adc_ChanConfigType chanCfg;
    Adc_Hal_InitChanStruct(&chanCfg);
    chanCfg.Spt = ADC_SPT_CLK_85;
    
    chanCfg.Channel = ADC_CH_12; chanCfg.SeqIndex = ADC_RSEQ_0; Adc_Hal_ConfigChannel(1, &chanCfg);
    chanCfg.Channel = ADC_CH_4;  chanCfg.SeqIndex = ADC_RSEQ_1; Adc_Hal_ConfigChannel(1, &chanCfg);
    chanCfg.Channel = ADC_CH_5;  chanCfg.SeqIndex = ADC_RSEQ_2; Adc_Hal_ConfigChannel(1, &chanCfg);
    chanCfg.Channel = ADC_CH_13; chanCfg.SeqIndex = ADC_RSEQ_3; Adc_Hal_ConfigChannel(1, &chanCfg);
    chanCfg.Channel = ADC_CH_2;  chanCfg.SeqIndex = ADC_RSEQ_4; Adc_Hal_ConfigChannel(1, &chanCfg);
    chanCfg.Channel = ADC_CH_2;  chanCfg.SeqIndex = ADC_RSEQ_5; Adc_Hal_ConfigChannel(1, &chanCfg);
    chanCfg.Channel = ADC_CH_6;  chanCfg.SeqIndex = ADC_RSEQ_6; // PTD4 (ADC1_IN6) HW_VER
    Adc_Hal_ConfigChannel(1, &chanCfg);
    chanCfg.Channel = ADC_CH_11; chanCfg.SeqIndex = ADC_RSEQ_7; // PTE6 (ADC1_IN11) 备用电池电压
    Adc_Hal_ConfigChannel(1, &chanCfg);
    chanCfg.Channel = ADC_CH_10; chanCfg.SeqIndex = ADC_RSEQ_8; // PTE2 (ADC1_IN10) 备用电池 NTC
    chanCfg.InterruptEn = TRUE; // 仅最后一个序列开启 EOC 中断
    Adc_Hal_ConfigChannel(1, &chanCfg);

    Adc_GroupConfigType groupCfg;
    Adc_Hal_InitGroupStruct(&groupCfg);
    groupCfg.RegularSequenceLength = 9;
    groupCfg.RegularTrigger = ADC_TRIGG_SRC_SW;
    groupCfg.ScanModeEn = TRUE;
    Adc_Hal_ConfigRegularGroup(1, &groupCfg);

    g_adcInitSatate |= 0x2;
}

/*************************************************
  Function:       PeripheralHalAdcHalSetMode
  Description:    Initialize or disable ADC0 and ADC1 modules
  Input:          mode - 1: Enable, 0: Disable
  Output:         none
  Return:         none
  Others:         新版 SDK 通过 Adc_Hal_Deinit 统一管理模块关闭，
                  内部会自动处理时钟、中断屏蔽及复位。
*************************************************/
static void PeripheralHalAdcHalSetMode(uint8_t mode)
{
    if (mode != 0)
    {
        PeripheralHalAdc0Init();
        PeripheralHalAdc1Init();
    }
    else
    {
        /* 1. 关闭 ADC0 模块 */
        Adc_Hal_Deinit(0); 
        PeripheralHalAdc0PinDeinit();
        
        /* 2. 关闭 ADC1 模块 */
        Adc_Hal_Deinit(1);
        PeripheralHalAdc1PinDeinit();
        
        g_adcInitSatate = 0;
    }        
}

/*****************************************************************************
 * Function:Timer1Handler
 * Description:TIM1 timer interrupt handler function, performs 1ms timing related processing
 * Input:
 * Output:
 * Others:1, Timer 1 interrupt handler function
 *     2, Calls PeripheralHal1msTimerProcess function to handle timing related tasks
****************************************************************************/
static void Timer1Handler(void *Args)
{
    (void)Args;
    PeripheralHal1msTimerProcess();
    MpuHalSpiTimerCallBack();
    SoftwareTimerProcess();
}

/*************************************************
  Function:       PeripheralHal1msTimerInit
  Description:    Initialize 1ms timer for system operations
  Input:          none
  Output:         none
  Return:         none
  Others:         Initializes TIM1 module for timing operations
*************************************************/
static void PeripheralHal1msTimerInit(void)
{
    for (uint8_t i = 0; i < MAX_SOFTWARE_TIMERS; i++) {
        g_softwareTimers[i].state = TIMER_UNUSED;
        g_softwareTimers[i].interval = 0;
        g_softwareTimers[i].counter = 0;
        g_softwareTimers[i].callback = NULL;
    }
    TIM1_Init();  
}

/*************************************************
  Function:     SoftwareTimerCreate
  Description:  Create a software timer
  Input:        callback: Timer callback function
                interval: Timer interval in milliseconds
  Output:       None
  Return:       Timer ID (0-4) on success, -1 on failure
  Others:       None
*************************************************/
uint32_t PeripheralHal_TimerCreate(SoftwareTimerCallback_t callback, uint32_t interval)
{
    if (callback == NULL) {
        return PERIPHERALHAL_ERR_INVALID_PARAM;
    }

    for (uint8_t i = 0; i < MAX_SOFTWARE_TIMERS; i++) {
        if (g_softwareTimers[i].state == TIMER_UNUSED) {
            g_softwareTimers[i].state = TIMER_STOPPED;
            g_softwareTimers[i].interval = interval;
            g_softwareTimers[i].counter = 0;
            g_softwareTimers[i].callback = callback;
            return i;  // 返回定时器ID
        }
    }
    return PERIPHERALHAL_ERR_NO_RESOURCE;
}

/*************************************************
  Function:     PeripheralHal_TimerStart
  Description:  Start a software timer
  Input:        timerId: Timer ID
  Output:       None
  Return:       PERIPHERALHAL_OK on success, PERIPHERALHAL_ERR_INVALID_PARAM on failure
  Others:       None
*************************************************/
uint32_t PeripheralHal_TimerStart(uint8_t timerId)
{
    if (timerId >= MAX_SOFTWARE_TIMERS || g_softwareTimers[timerId].state == TIMER_UNUSED) {
        return PERIPHERALHAL_ERR_INVALID_PARAM;  // 无效的定时器ID
    }
    
    g_softwareTimers[timerId].counter = g_softwareTimers[timerId].interval;
    g_softwareTimers[timerId].state = TIMER_RUNNING;
    return PERIPHERALHAL_OK;
}

/*************************************************
  Function:     PeripheralHal_TimerStop
  Description:  Stop a software timer
  Input:        timerId: Timer ID
  Output:       None
  Return:       PERIPHERALHAL_OK on success, PERIPHERALHAL_ERR_INVALID_PARAM on failure
  Others:       None
*************************************************/
uint32_t PeripheralHal_TimerStop(uint8_t timerId)
{
    if (timerId >= MAX_SOFTWARE_TIMERS || g_softwareTimers[timerId].state == TIMER_UNUSED) {
        return PERIPHERALHAL_ERR_INVALID_PARAM;  // 无效的定时器ID
    }
    g_softwareTimers[timerId].state = TIMER_STOPPED;
    return PERIPHERALHAL_OK;
}

/*************************************************
  Function:       PeripheralHal_SetTimerVal
  Description:    Set the interval of a software timer
  Input:          timerId - Timer ID (0-4)
                  interval - New interval in milliseconds
  Output:         none
  Return:         0 - Success
                 -1 - Invalid timer ID
  Others:         none
*************************************************/
uint32_t PeripheralHal_SetTimerVal(uint8_t timerId, uint32_t interval)
{
    if (timerId >= MAX_SOFTWARE_TIMERS || g_softwareTimers[timerId].state == TIMER_UNUSED) {
        return PERIPHERALHAL_ERR_INVALID_PARAM;
    }
    
    g_softwareTimers[timerId].interval = interval;
    
    // 如果定时器正在运行，重置计数器
    if (g_softwareTimers[timerId].state == TIMER_RUNNING) {
        g_softwareTimers[timerId].counter = interval;
    }
    
    return PERIPHERALHAL_OK;
}

/*************************************************
  Function:       SoftwareTimerProcess
  Description:    Process software timers, handling counting and timeouts
  Input:          none
  Output:         none
  Return:         none
  Others:         none
*************************************************/
static void SoftwareTimerProcess(void)
{
    for (uint8_t i = 0; i < MAX_SOFTWARE_TIMERS; i++) {
        if (g_softwareTimers[i].state == TIMER_RUNNING) {
            if (g_softwareTimers[i].counter > 0) g_softwareTimers[i].counter--;
            if (g_softwareTimers[i].counter == 0) {
                if (g_softwareTimers[i].callback != NULL) g_softwareTimers[i].callback();
            }
        }
    }
}

/*************************************************
  Function:       PeripheralStartSysLed
  Description:    Initialize and start system LED operation
  Input:          none
  Output:         none
  Return:         none
  Others:         Initializes 1ms timer for LED control
                  Configures LED pin muxing and direction
                  Sets LED to initial state (ON)
*************************************************/
static void PeripheralStartSysLed(void)
{
    PeripheralHal1msTimerInit();
    Gpio_Hal_SetMuxMode(SYS_LED_PORT, SYS_LED_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(SYS_LED_PORT, SYS_LED_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(SYS_LED_PORT, SYS_LED_PIN, GPIO_HIGH_LEVEL);
}

/*************************************************
  Function:       PeripheralHalStopSysLed
  Description:    Stop system LED operation
  Input:          none
  Output:         none
  Return:         none
  Others:         Sets sleep flag to disable LED blinking
                  Turns off system LED
*************************************************/
static void PeripheralHalStopSysLed(void)
{
    g_ledSleepFlag = 1;
    Gpio_Hal_WritePin(SYS_LED_PORT, SYS_LED_PIN, GPIO_LOW_LEVEL);
}

/*************************************************
  Function:       PeripheralHalRestartSysLed
  Description:    Restart system LED operation
  Input:          none
  Output:         none
  Return:         none
  Others:         Reconfigures LED pin muxing and direction
                  Sets LED to ON state
                  Clears sleep flag to enable LED blinking
*************************************************/
static void PeripheralHalRestartSysLed(void)
{
    /* 1. 配置引脚复用为 GPIO 模式 */
    Gpio_Hal_SetMuxMode(SYS_LED_PORT, SYS_LED_PIN, PORT_MUX_AS_GPIO);

    /* 2. 配置引脚方向为输出 */
    Gpio_Hal_SetPinDirection(SYS_LED_PORT, SYS_LED_PIN, GPIO_OUTPUT_DIRECTION);

    /* 3. 设置初始输出电平为高 (GPIO_HIGH_LEVEL) */
    Gpio_Hal_WritePin(SYS_LED_PORT, SYS_LED_PIN, GPIO_HIGH_LEVEL);
    g_ledSleepFlag = 0;
}

/*************************************************
  Function:       MCWDTInit
  Description:    Initialize watchdog module
  Input:          none
  Output:         none
  Return:         none
  Others:         Resets and enables watchdog module
                  Configures watchdog settings using WDOG_Config
                  Disables watchdog interrupts and clears status flags
                  Sets watchdog interrupt priority
                  Enables watchdog and performs initial refresh
*************************************************/
static void MCWDTInit(void)
{
    Wdg_HalConfigType wdtCfg;
    wdtCfg.ClockSource = WDG_CLOCK_LSI; // 使用 LSI 作为内部低频时钟源
    wdtCfg.WindowValue = 0;
    wdtCfg.TimeoutValue = WDOG_TIMEOUT_MS; // ms 单位
    wdtCfg.Config = WDG_CONFIG_PRESCALER_EN;
    
    Wdg_Hal_Init(&wdtCfg);
    Wdg_Hal_Feed();
}

/*************************************************
  Function:       IrqPinInit
  Description:    Initialize interrupt pins for various system peripherals
  Input:          none
  Output:         none
  Return:         none
  Others:         Configures KL15, KL30 and GSM interrupt pins
                  Sets pin muxing, direction and interrupt configuration
                  Enables NVIC interrupts with priority level 3
                  Contains commented-out code for CAN, Bluetooth, button, SRS and G-sensor interrupts
*************************************************/
static void IrqPinInit(void)
{
    Gpio_Hal_InstallCallback(PORTID_A, PeripheralHal_PORTAIRQHandler);
    Gpio_Hal_InstallCallback(PORTID_B, PeripheralHal_PORTBIRQHandler);
    Gpio_Hal_InstallCallback(PORTID_C, PeripheralHal_PORTCIRQHandler);
    Gpio_Hal_InstallCallback(PORTID_D, PeripheralHal_PORTDIRQHandler);
    Gpio_Hal_InstallCallback(PORTID_E, PeripheralHal_PORTEIRQHandler);
    /* KL15 */
    Gpio_Hal_SetMuxMode(KL15_IRQ_PORT, KL15_IRQ_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(KL15_IRQ_PORT, KL15_IRQ_PIN, GPIO_INPUT_DIRECTION);
    Gpio_Hal_SetPinIntSel(KL15_IRQ_PORT, KL15_IRQ_PIN, PORT_INT_RISING_EDGE);
    
    /* kl30 */
    Gpio_Hal_SetMuxMode(KL30_IRQ_PORT, KL30_IRQ_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(KL30_IRQ_PORT, KL30_IRQ_PIN, GPIO_INPUT_DIRECTION);
    Gpio_Hal_SetPinIntSel(KL30_IRQ_PORT, KL30_IRQ_PIN, PORT_INT_FALLING_EDGE);
    
    /* GSM */
    Gpio_Hal_SetMuxMode(GSM_IRQ_PORT, GSM_IRQ_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(GSM_IRQ_PORT, GSM_IRQ_PIN, GPIO_INPUT_DIRECTION);
    Gpio_Hal_SetPinIntSel(GSM_IRQ_PORT, GSM_IRQ_PIN, PORT_INT_FALLING_EDGE);

    /* Charge Fast (F_CHARGE, PTA11) */
    Gpio_Hal_SetMuxMode(CHARGE_FAST_IRQ_PORT, CHARGE_FAST_IRQ_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(CHARGE_FAST_IRQ_PORT, CHARGE_FAST_IRQ_PIN, GPIO_INPUT_DIRECTION);
    Gpio_Hal_SetPinIntSel(CHARGE_FAST_IRQ_PORT, CHARGE_FAST_IRQ_PIN, PORT_INT_FALLING_EDGE);

    /* Charge Slow (S_CHARGE, PTA6) */
    Gpio_Hal_SetMuxMode(CHARGE_SLOW_IRQ_PORT, CHARGE_SLOW_IRQ_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(CHARGE_SLOW_IRQ_PORT, CHARGE_SLOW_IRQ_PIN, GPIO_INPUT_DIRECTION);
    Gpio_Hal_SetPinIntSel(CHARGE_SLOW_IRQ_PORT, CHARGE_SLOW_IRQ_PIN, PORT_INT_FALLING_EDGE);
}

/*************************************************
 Function    : PeripheralHalKl30RawTo_mV
 Description : Convert KL30 raw ADC value to millivolt (mV)
 Input       : raw - Raw ADC value from KL30 measurement
 Output      : None
 Return      : uint32_t - Converted voltage value in millivolt (mV)
 Others      : Uses linear conversion formula: v = raw*slope + offset
               KL30_SLOPE_Q20 and KL30_OFFSET_mV are used for the conversion
*************************************************/
static uint32_t PeripheralHalKl30RawTo_mV(uint16_t raw)
{
    uint32_t base_mV, slope_Q20;
    int32_t raw_diff;

    if (raw < kl30_cal_table[1].adcRaw) { 
        // 第一段 & 低压外推
        raw_diff = raw - kl30_cal_table[0].adcRaw;
        base_mV = kl30_cal_table[0].voltage_mV;
        slope_Q20 = kl30_cal_table[0].slope_Q20;
    } 
    else if (raw < kl30_cal_table[2].adcRaw) {
        // 第二段
        raw_diff = raw - kl30_cal_table[1].adcRaw;
        base_mV = kl30_cal_table[1].voltage_mV;
        slope_Q20 = kl30_cal_table[1].slope_Q20;
    }
    else if (raw < kl30_cal_table[3].adcRaw) {
        // 第三段
        raw_diff = raw - kl30_cal_table[2].adcRaw;
        base_mV = kl30_cal_table[2].voltage_mV;
        slope_Q20 = kl30_cal_table[2].slope_Q20;
    }
    else {
        // 第四段 & 高压外推
        raw_diff = raw - kl30_cal_table[3].adcRaw;
        base_mV = kl30_cal_table[3].voltage_mV;
        slope_Q20 = kl30_cal_table[3].slope_Q20;
    }

    // 核心计算：注意这里将 raw_diff 强转为 int64_t 防止溢出，然后再移位
    int32_t delta_mV = (int32_t)(((int64_t)raw_diff * slope_Q20) >> Q20_SHIFT);
    int32_t final_mV = (int32_t)base_mV + delta_mV;

    // 防止负压兜底
    return (final_mV > 0) ? (uint32_t)final_mV : 0; 
}

/*************************************************
  Function:       RtcWakeUpProcess
  Description:    After waking up, handle according to the voltage value
  Input:          maxVoltage : Maximum wake-up voltage
                  minVoltage : Minimum wake-up voltage
  Output:         none
  Return:         1 : wake up
                  2 : keep sleep
  Others:         none
*************************************************/
int16_t RtcWakeUpProcess(uint32_t maxVoltage, uint32_t minVoltage)
{
    uint32_t adValue;
    uint32_t timeCount = 100000;
    int16_t result = 0; 
    g_adcCompletedFlag = 0;
    TimerHalSetMode(1);
    while(g_adcCompletedFlag == 0)
    {
        timeCount --;
        if(timeCount == 0)
        {
            break;
        }
    }
    if(g_adcCompletedFlag == 0)
    {
        TimerHalSetMode(0);
        result = 0;
    }
    else
    {
        TBOX_PRINT("KL30 voltage conversion complete!\r\n");
        PeripheralHalAdGet(AD_CHANNEL_KL30, &adValue);
        TBOX_PRINT("KL30 voltage is : %d \r\n", adValue);
        if(adValue > maxVoltage || adValue < minVoltage)
        {
            result = 1;
        }
        else
        {
            TimerHalSetMode(0);
            result = 0;
        }
    }
    return result;
}

/*************************************************
  Function:       HardFault_Custom_Callback
  Description:    HardFault_Custom_Callback
  Output:         none
  Return:         void
  Others:         none
*************************************************/
void HardFault_Custom_Callback(void)
{
    uint32_t cnt = 50;
    LogHalInit(1); // 防止串口未初始化
    while (cnt--)
    {
        TBOX_PRINT("HardFault\n");
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    PeripheralHalMcuHardReset();
}