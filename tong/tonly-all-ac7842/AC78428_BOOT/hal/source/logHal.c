/*************************************************
 Copyright © 2026 SiRun (AnHui) . All rights reserved.
 File Name: logHal.c
 Author: HuShangrui
 Created Time: 2026年7月10日
 Description:
 Others:
*************************************************/
/****************************** include ***************************************/
#include "logHal.h"
#include "Gpio_Hal.h"
#include "Uart_Hal.h"
#include "string.h"
#include "stdarg.h"
#include <stdio.h>
#include "mpuHal.h"
#include "Core_Hal.h"

/****************************** Macro Definitions ******************************/
#define MCU_DELAY_1MS_COUNT                  7464U // 已校准

#define LOG_SCB_DEBUG_TYPE                 2
#define LOG_SCB_DEBUG_UART_RX_PORT         PORTID_A
#define LOG_SCB_DEBUG_UART_RX_PIN          8U
#define LOG_SCB_DEBUG_UART_RX_MUX          PORT_MUX_ALT2
#define LOG_SCB_DEBUG_UART_TX_PORT         PORTID_A
#define LOG_SCB_DEBUG_UART_TX_PIN          9U
#define LOG_SCB_DEBUG_UART_TX_MUX          PORT_MUX_ALT2
#define LOG_SCB_DEBUG_UART_IRQN            UART2_IRQn 

#define LOG_UPLOAD_LOG_BUF_SIZE            (256)
#define LOG_UPLOAD_LOG_AID                 (0x00)
#define LOG_UPLOAD_LOG_MID                 (0x00)

/****************************** Type Definitions ******************************/
/****************************** Global Variables ******************************/
static uint8_t g_logInitFlag = 0;

/****************************** Function Declarations *************************/

/****************************** Public Function Implementations ******************************/
/*************************************************
  Function:     LogHalInit
  Description:  UART2 module init
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void LogHalInit(uint8_t mode)
{
    Uart_ChannelConfigType userConfig; 
    memset(&userConfig, 0, sizeof(Uart_ChannelConfigType));

    Gpio_Hal_SetMuxMode(LOG_SCB_DEBUG_UART_RX_PORT, LOG_SCB_DEBUG_UART_RX_PIN, LOG_SCB_DEBUG_UART_RX_MUX);
    Gpio_Hal_SetMuxMode(LOG_SCB_DEBUG_UART_TX_PORT, LOG_SCB_DEBUG_UART_TX_PIN, LOG_SCB_DEBUG_UART_TX_MUX);

    Gpio_Hal_SetPullSel(LOG_SCB_DEBUG_UART_RX_PORT, LOG_SCB_DEBUG_UART_RX_PIN, PORT_INTERNAL_PULL_UP_ENABLED);
    Gpio_Hal_SetPullSel(LOG_SCB_DEBUG_UART_TX_PORT, LOG_SCB_DEBUG_UART_TX_PIN, PORT_INTERNAL_PULL_UP_ENABLED);

    userConfig.BaudRate        = 115200;                         
    userConfig.ParityMode      = UART_PARITY_DISABLED;         
    userConfig.StopBitCount    = UART_ONE_STOP_BIT;          
    userConfig.BitCountPerChar = UART_8_BITS_PER_CHAR;    
    userConfig.TransferType    = UART_USING_INTERRUPTS; 
    userConfig.SampleCnt       = UART_SMP_CNT16;
    userConfig.RxCallback      = NULL;
    userConfig.TxCallback      = NULL;

    (void)Uart_Hal_Init(LOG_SCB_DEBUG_TYPE, &userConfig);

    g_logInitFlag = 1;
}

/*************************************************
  Function:     LogHalPrint
  Description:  MPU uart or debug uart print
*************************************************/
void LogHalPrint(const char *format, ...)
{
    static uint8_t sendflag = 0;
    if (sendflag == 1)
    {
        return;
    }

    sendflag = 1;
    char g_debugBuffer[PRINT_MAX_LEN] = {0};
    va_list v_args;
    va_start(v_args, format);
    int len = vsnprintf(g_debugBuffer, PRINT_MAX_LEN, format, v_args);
    va_end(v_args);	

    if (g_logInitFlag != 0)
    {
        (void)Uart_Hal_SendDataPolling(LOG_SCB_DEBUG_TYPE, (const uint8_t*)g_debugBuffer, len);
    }
    sendflag = 0;
}


/*************************************************
  Function:     LogHalDeInit
  Description:  串口去初始化
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void LogHalDeInit(void)
{
    Delay_ms(2);
    g_logInitFlag = 0;
    Uart_Hal_DeInit(LOG_SCB_DEBUG_TYPE);
    Gpio_Hal_SetMuxMode(LOG_SCB_DEBUG_UART_RX_PORT, LOG_SCB_DEBUG_UART_RX_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetMuxMode(LOG_SCB_DEBUG_UART_TX_PORT, LOG_SCB_DEBUG_UART_TX_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPullSel(LOG_SCB_DEBUG_UART_RX_PORT, LOG_SCB_DEBUG_UART_RX_PIN, PORT_INTERNAL_PULL_NOT_ENABLED);
    Gpio_Hal_SetPullSel(LOG_SCB_DEBUG_UART_TX_PORT, LOG_SCB_DEBUG_UART_TX_PIN, PORT_INTERNAL_PULL_NOT_ENABLED);
}

/*****************************************************************************
 * Function Name: Delay_ms
 * Description  : Provides a blocking millisecond-level delay.
 * Parameters   : uint32_t ms - The number of milliseconds to delay.
 * Return Value : None
 * Note         : This is a blocking function. CPU will wait in a loop until 
                  the specified time has elapsed. Accuracy depends on the 
                  underlying implementation (SysTick timer or software loop) 
                  and system interrupt frequency.
****************************************************************************/
void Delay_ms(uint32_t ms)
{
    volatile uint32_t count; 
    
    while (ms--)
    {
        count = MCU_DELAY_1MS_COUNT;
        while (count--)
        {
            __NOP();
        }
    }
}

