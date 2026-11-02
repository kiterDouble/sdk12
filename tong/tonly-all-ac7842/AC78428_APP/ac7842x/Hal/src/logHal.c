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
static uint8_t g_logHalUploadLogBuf[LOG_UPLOAD_LOG_BUF_SIZE];
static int16_t g_logHalhandle = -1; // MPU通信句柄
static uint8_t g_logInitFlag = 0;

#if (DEBUG_UART_REV_ENABLE == 1)
uint8_t UART2_RxBuf[PRINT_MAX_LEN];
uint32_t UART2_RxDataLen = 0;
uint8_t UART2_RxComplete = 0;
#endif

/****************************** Function Declarations *************************/
static void LoghalPackageLogData(char *log_str, MpuHalDataPack_t *packet);
static void LogHalUploadLogInit(void);
#if (DEBUG_UART_REV_ENABLE == 1)
static void LogHal_UartReceiveCallback(uint8 Instance, Uart_EventType Event);
#endif

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
#if (DEBUG_UART_REV_ENABLE == 1)
    userConfig.RxCallback      = LogHal_UartReceiveCallback;
#else
    userConfig.RxCallback      = NULL;
#endif
    userConfig.TxCallback      = NULL;

    (void)Uart_Hal_Init(LOG_SCB_DEBUG_TYPE, &userConfig);
#if (DEBUG_UART_REV_ENABLE == 1)
    Core_Hal_EnableIrq(UART2_IRQn);
    Core_Hal_SetIrqPriority(UART2_IRQn, 0x3);
    (void)Uart_Hal_ReceiveData(LOG_SCB_DEBUG_TYPE, UART2_RxBuf, 1);
#endif

    LogHalUploadLogInit();

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

void LogHalRttPrint(const char *format, ...)
{
#if(TBOX_PRINT_RTT_ENABLE)
    va_list ParamList;
    va_start(ParamList, format);
    SEGGER_RTT_vprintf(0, format, &ParamList);
    va_end(ParamList);
#endif
}

void LogHalSetMode(uint8_t mode)
{
    if (0u == mode)
    {
        g_logInitFlag = 0;
#if (DEBUG_UART_REV_ENABLE == 1)
        Core_Hal_DisableIrq(UART2_IRQn);
#endif
        Uart_Hal_DeInit(LOG_SCB_DEBUG_TYPE);
        Gpio_Hal_SetMuxMode(LOG_SCB_DEBUG_UART_RX_PORT, LOG_SCB_DEBUG_UART_RX_PIN, PORT_MUX_AS_GPIO);
        Gpio_Hal_SetMuxMode(LOG_SCB_DEBUG_UART_TX_PORT, LOG_SCB_DEBUG_UART_TX_PIN, PORT_MUX_AS_GPIO);
        Gpio_Hal_SetPullSel(LOG_SCB_DEBUG_UART_RX_PORT, LOG_SCB_DEBUG_UART_RX_PIN, PORT_INTERNAL_PULL_NOT_ENABLED);
        Gpio_Hal_SetPullSel(LOG_SCB_DEBUG_UART_TX_PORT, LOG_SCB_DEBUG_UART_TX_PIN, PORT_INTERNAL_PULL_NOT_ENABLED);
    }
    else if(1u == mode)
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
#if (DEBUG_UART_REV_ENABLE == 1)
    userConfig.RxCallback      = LogHal_UartReceiveCallback;
#else
    userConfig.RxCallback      = NULL;
#endif
        userConfig.TxCallback      = NULL;

        (void)Uart_Hal_Init(LOG_SCB_DEBUG_TYPE, &userConfig);
#if (DEBUG_UART_REV_ENABLE == 1)
        Core_Hal_EnableIrq(UART2_IRQn);
        Core_Hal_SetIrqPriority(UART2_IRQn, 0x3);
        (void)Uart_Hal_ReceiveData(LOG_SCB_DEBUG_TYPE, UART2_RxBuf, 1);
#endif
        g_logInitFlag = 1;
    }  
}


/*************************************************
  Function:     LogHalUploadLogInit
  Description:  Log upload log init
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
static void LogHalUploadLogInit(void)
{
    MpuHalFilter_t filter;
    g_logHalhandle = MpuHalOpen();

    filter.aid = LOG_UPLOAD_LOG_AID;
    filter.midMin = LOG_UPLOAD_LOG_AID;
    filter.midMax = LOG_UPLOAD_LOG_AID;

    MpuHalSetRxFilter(g_logHalhandle, &filter);
    MpuHalSetRxBuffer(g_logHalhandle, g_logHalUploadLogBuf, sizeof(g_logHalUploadLogBuf));

}

/*************************************************
  Function:     LogHalUpLoadLog
  Description:  Log upload log
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void LogHalUpLoadLog(const char *format, ...) 
{
    static char log_str[LOG_UPLOAD_LOG_BUF_SIZE];
    static MpuHalDataPack_t packet;
    int16_t ret = 0U;
    va_list args;
    va_start(args, format);
    
    int len = vsnprintf(log_str, LOG_UPLOAD_LOG_BUF_SIZE, format, args);

    if (len < 0 || len >= LOG_UPLOAD_LOG_BUF_SIZE) {
        return;
    }
    va_end(args);

    LoghalPackageLogData(log_str, &packet);

    ret = MpuHalTransmit(g_logHalhandle, &packet, MPU_HAL_SPI_MODE);
    if(ret != MPU_HAL_STATUS_OK)
    {
        LogHalPrint("LogHalUpLoadLog failed, ret=%d", ret, g_logHalhandle);
    }
}

/*************************************************
  Function:     LoghalPackageLogData
  Description:  Log package log data
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
static void LoghalPackageLogData(char *log_str, MpuHalDataPack_t *packet)
{
    packet->aid = LOG_UPLOAD_LOG_AID;
    packet->mid = LOG_UPLOAD_LOG_MID;
    packet->dataBufferSize = sizeof(g_logHalUploadLogBuf);
    packet->pDataBuffer = g_logHalUploadLogBuf;
    memset(g_logHalUploadLogBuf, 0, sizeof(g_logHalUploadLogBuf));

    packet->dataLength = strlen(log_str);
    memcpy(g_logHalUploadLogBuf, log_str, packet->dataLength);
}


#if (DEBUG_UART_REV_ENABLE == 1)
/*************************************************
  Function:     LogHal_UartReceiveCallback
  Description:  UART2 receive data valid interrupt handler
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
static void LogHal_UartReceiveCallback(uint8 Instance, Uart_EventType Event)
{
    (void)Uart_Hal_ReceiveData(Instance, UART2_RxBuf, 1);
    UART2_RxComplete = 1;
}

/*************************************************
  Function:     LogHal_MainFunc
  Description:  MPU uart or debug uart main function
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void mcu_gotosleep(void);
void FlashHal_TestSuite(void);
void PeripheralHalMcuHardReset(void);
void LogHal_MainFunc(void)
{
    if (UART2_RxComplete == 1)
    {
        if (strstr((char*)UART2_RxBuf, "s") != NULL)
        {
            mcu_gotosleep();
            TBOX_PRINT("sleep test done\n");
        }
        else if (strstr((char*)UART2_RxBuf, "f") != NULL)
        {
            FlashHal_TestSuite();
            TBOX_PRINT("flash test done\n");
        }
        else if (strstr((char*)UART2_RxBuf, "r") != NULL)
        {
            TBOX_PRINT("reset test done\n");
            uint16_t count = 1000;
            while (count--)
            {
                __NOP();
            }
            PeripheralHalMcuHardReset();
        }
        else
        {
          TBOX_PRINT("debug not func : %s\n", UART2_RxBuf);
        }

        /* 命令处理完毕，清空状态，允许接收下一帧 */
        memset((void*)UART2_RxBuf, 0, PRINT_MAX_LEN);
        UART2_RxDataLen = 0;
        UART2_RxComplete = 0;
    }
}
#endif
