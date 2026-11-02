#include "logSdk.h"
#include "commSdk.h"
#include "logHal.h"
#include "mpuHal.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define LOG_UPLOAD_LOG_BUF_SIZE            (256U)
#define PRINT_MAX_LEN                      (200U)

//command ID
#define LOG_UPLOAD_LOG_AID                 (0x00)
#define LOG_UPLOAD_LOG_MID                 (0x00)

static int16_t g_logHalhandle = -1; // MPU通信句柄
static uint8_t g_logHalUploadLogBuf[LOG_UPLOAD_LOG_BUF_SIZE];
static char g_debugBuffer[PRINT_MAX_LEN];
static uint8_t g_debugMode;

static void PackLog(char *log_str, MpuHalDataPack_t *packet);


/*************************************************
  Function:     LogSdk_Init
  Description:  Log upload log init
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void LogSdk_Init(uint8_t mode)
{
    MpuHalFilter_t filter;
    g_logHalhandle = MpuHalOpen();

    filter.aid = LOG_UPLOAD_LOG_AID;
    filter.midMin = LOG_UPLOAD_LOG_AID;
    filter.midMax = LOG_UPLOAD_LOG_AID;

    MpuHalSetRxFilter(g_logHalhandle, &filter);
    MpuHalSetRxBuffer(g_logHalhandle, g_logHalUploadLogBuf, sizeof(g_logHalUploadLogBuf));

    LogHalInit(mode);

    g_debugMode = mode;
}

void LogSdk_DeInit(void)
{
    //TODO
}

void LogSdk_SetMode(uint8_t mode)
{
    if (mode == 0) {
        if (g_debugMode == 1 || g_debugMode == 3) {
            LogHalSetMode(0);
        }
    } else if (1 == mode) {
        // LogHal_Init(3);
        LogHalSetMode(1);
    }
}



void LogSdk_Sleep(void)
{
    LogHalSetMode(0);
}

void LogSdk_Wakeup(void)
{
    LogHalSetMode(1);
}


/*************************************************
  Function:     PackLog
  Description:  Log package log data
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
static void PackLog(char *log_str, MpuHalDataPack_t *packet)
{
    packet->aid = LOG_UPLOAD_LOG_AID;
    packet->mid = LOG_UPLOAD_LOG_MID;
    packet->dataBufferSize = sizeof(g_logHalUploadLogBuf);
    packet->pDataBuffer = g_logHalUploadLogBuf;
    memset(g_logHalUploadLogBuf, 0, sizeof(g_logHalUploadLogBuf));

    packet->dataLength = strlen(log_str);
    memcpy(g_logHalUploadLogBuf, log_str, packet->dataLength);
}


/*************************************************
  Function:     LogSdk_Print
  Description:  MPU uart or debug uart print
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void LogSdk_Print(const char *format, ...)
{
    va_list v_args;
    va_start(v_args, format);
    vsprintf(g_debugBuffer, (char const *)format, v_args);
    va_end(v_args);

    if (g_debugMode == 1 || g_debugMode == 3) {
        // Common_DisableInterrupts();
        // UART2_SendData((unsigned char *)g_debugBuffer, strlen(g_debugBuffer));
        // Common_EnableInterrupts();
        LogHalPrint("%s", g_debugBuffer);
    }

#if (TBOX_PRINT_RTT_ENABLE)
    SEGGER_RTT_printf(0, RTT_CTRL_TEXT_WHITE);
    va_list ParamList;
    va_start(ParamList, format);
    SEGGER_RTT_vprintf(0, format, &ParamList);
    va_end(ParamList);
#endif
}


/*************************************************
  Function:     LogSdk_Send
  Description:  Log upload log
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void LogSdk_Send(const char *format, ...)
{
    static char log_str[LOG_UPLOAD_LOG_BUF_SIZE];
    static MpuHalDataPack_t packet;
    uint8_t ret = 0U;
    va_list args;
    va_start(args, format);

    int len = vsnprintf(log_str, LOG_UPLOAD_LOG_BUF_SIZE, format, args);

    if (len < 0 || len >= LOG_UPLOAD_LOG_BUF_SIZE) {
        return;
    }
    va_end(args);

    PackLog(log_str, &packet);

    ret = MpuHalTransmit(g_logHalhandle, &packet, MPU_HAL_SPI_MODE);
    if (ret != 0) {
        LogSdk_Print("LogSdk_Send failed, log_str: %s", log_str);
    }
}