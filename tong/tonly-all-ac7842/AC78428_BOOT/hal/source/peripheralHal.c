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
#include "logHal.h"
#include "mpuHal.h"

/****************************** Macro Definitions ******************************/
#define WDOG_TIMEOUT_MS               (2000U)

#define KL15_IRQ_PORT                 PORTID_E
#define KL15_IRQ_PIN                  1
#define KL15_IRQ_PIN_MUX              PORT_MUX_AS_GPIO
#define KL15_IRQ_IRQN                 PORTE_IRQn

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

#define SYS_ON_PORT                   PORTID_D
#define SYS_ON_PIN                    10
#define SYS_ON_PIN_MUX                PORT_MUX_AS_GPIO

#define SYS_LED_PORT                  PORTID_D
#define SYS_LED_PIN                   11
#define SYS_LED_PIN_MUX               PORT_MUX_AS_GPIO

#define PERIPHERAL_TIMER_INDEX        1

#define MAX_SOFTWARE_TIMERS           (5U)
#define TIMER_UNUSED                  (0U)
#define TIMER_RUNNING                 (1U)
#define TIMER_STOPPED                 (2U)

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

/****************************** Global Variables ******************************/
/****************************** Function Declarations *************************/
static void PeripheralHalSysOnInit(void);
static void MCWDTInit(void);

/****************************** Public Function Implementations ***************/
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
    PeripheralHalSysOnInit();
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
        Delay_ms(10);
    }
    PeripheralHalMcuHardReset();
}
