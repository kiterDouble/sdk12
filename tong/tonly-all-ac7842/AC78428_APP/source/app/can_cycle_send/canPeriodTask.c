/*************************************************
 Copyright © 2025 SiRun (Beijing) . All rights reserved.
 File Name: canPeriodTask.c
 Author:
 Created Time:
 Description: CAN periodic message sending task implementation
 Others:
*************************************************/
/****************************** include ***************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "peripheralHal.h"
#include "canHal.h"
#include "timeSyncSdk.h"
#include "powerManageSdk.h"
#include "canGateWay.h"
#include "canPeriodTask.h"
#include "secocSdk.h"
#include "string.h"
#include "logHal.h"
#include "stateSyncSdk.h"
#include "taskEcallProcess.h"
#include "remoteControl.h"

#if (SELFCHECK_RESULT_SEND == 1)
#include "ecallHal.h"
#endif


/****************************** Macro Definitions ******************************/
#define CYCLE_SEND_WITH_SECOC_SDK (0)
#define CYCLE_SEND_WITH_CANFD (1)
#define CYCLE_SEND_WITHOUT_CANFD (0)
#define SIZE_OF_ARRAY(arrayName) (sizeof(arrayName) / sizeof(arrayName)[0])

#define CAN_CYCLE_SEND_CONFIGURE_BEGIN(groupNum) const CanSendMsgConfigure_t m_group##groupNum##SendMsgConfigure[] = \
                                                     {
#define CAN_CYCLE_SEND_CONFIGURE_CAN(cycleTime, canId, fdFlag, length, msgProcessCallback) {cycleTime, canId, fdFlag, length, msgProcessCallback},

#define CAN_CYCLE_SEND_CONFIGURE_END(groupNum) \
    }                                          \
    ;                                          \
    CanSendMsgBuffer_t m_group##groupNum##SendMsgBuffer[SIZE_OF_ARRAY(m_group##groupNum##SendMsgConfigure)];

#define CYCLE_SEND_CAN_CONFIGURE(groupNum) m_group##groupNum##SendMsgConfigure                     // Cycle send configuration
#define CYCLE_SEND_CAN_BUFFER(groupNum) m_group##groupNum##SendMsgBuffer                           // Cycle send buffer
#define CYCLE_SEND_CAN_CONFIGURE_SIZE(groupNum) SIZE_OF_ARRAY(m_group##groupNum##SendMsgConfigure) // Cycle send configuration size

//static uint8_t g_TEL_MsgCounter = 0U;

/****************************** Type Definitions ******************************/
typedef int16_t (*pCanMsgProcessFun_t)(uint8_t *pMsgData);

// CAN send message configuration structure
typedef struct
{
    uint32_t cycleTime; // Cycle time
    uint32_t canId;     // CAN message ID
    uint8_t fdFlag;
    uint8_t dataLength; // Message length
    pCanMsgProcessFun_t msgProcessFun;
} CanSendMsgConfigure_t;

// CAN send message buffer structure
typedef struct
{
    uint32_t timeCount; // Time counter
    uint8_t canData[8]; // CAN data
} CanSendMsgBuffer_t;

// CAN channel cycle send configuration structure
typedef struct
{
    uint8_t canChannel;                               // CAN channel
    const CanSendMsgConfigure_t *pCycleConfigureList; // Cycle send configuration list
    uint8_t cycleConfigureListSize;                   // Cycle send configuration list size
    uint32_t msgStartTime;                            // Message start sending time
    CanSendMsgBuffer_t *pMsgBuffer;                   // Message buffer
} CanChannelCycleSendConfigure_t;

// CAN channel flag buffer structure
typedef struct
{
    int16_t canHandle;  // CAN channel handle
    int8_t enableFlag;  // Enable flag    0: Disable 1: Enable
    uint8_t resetFlag;  // Reset flag    0: Disable 1: Enable
    uint8_t busoffFlag; // Busoff flag  0: Disable 1: Enable
} CanChannelBuffer_t;

//typedef union TEL_11_A_T
//{
//    uint8_t data[8];
//    struct
//    {
//
//        uint64_t TEL_Latitude_1 : 8;
//        uint64_t TEL_Latitude_2 : 8;
//        uint64_t TEL_Latitude_3 : 8;
//        uint64_t TEL_Latitude_4 : 4;
//
//        uint64_t TEL_Longitude_1 : 4;
//        uint64_t TEL_Longitude_2 : 8;
//        uint64_t TEL_Longitude_3 : 8;
//        uint64_t TEL_Longitude_4 : 8;
//
//        uint64_t TEL_NaviPstVD : 1;
//        uint64_t TEL_11_Reserve : 7;
//    } DetailInfo;
//} TEL_11_A_T;

//typedef union TEL_3
//{
//    uint8_t data[8];
//    struct
//    {
//        uint64_t TEL_GPSCommSt : 2;
//        uint64_t TEL_GPRSOr3GCommSt : 2;
//        uint64_t TEL_TelematicsModeActSt : 3;
//        uint64_t TEL_Reserve1 : 1;
//
//        uint64_t TEL_MsgCounter : 4;
//
//        uint64_t TEL_BTHandfreeSt : 1;
//        uint64_t TEL_BTHisdel : 1;
//        uint64_t TEL_BTAutoconectingSt : 1;
//        uint64_t TEL_BTPowerSt : 1;
//
//        uint64_t TEL_BTAvrcpPlayMode : 3;
//        uint64_t TEL_BTAvrcpSt : 1;
//        uint64_t TEL_OTAState : 1;
//        uint64_t TEL_Reserve2 : 1;
//        uint64_t TEL_BTManualMode : 2;
//
//        uint64_t TEL_TelematicsRegisterSt : 3;
//        uint64_t TEL_Reserve3 : 5;
//
//        uint64_t TEL_Reserve4 : 32;
//    } DetailInfo;
//} TEL_3;

//typedef union TEL_4
//{
//    uint8_t data[8];
//    struct
//    {
//        uint64_t TEL_PhoneNumber : 48;
//        uint64_t TEL_BTDisplayAct : 1;
//        uint64_t TEL_BTConnectingAct : 1;
//        uint64_t TEL_BTConnectingSt : 1;
//        uint64_t TEL_PhoneCallSt : 3;
//        uint64_t TEL_A2DPSupportSt : 1;
//        uint64_t TEL_AVRCPSupportSt : 1;
//        uint64_t TEL_MobilePowerSt : 3;
//        uint64_t TEL_MobileSignalSt : 3;
//        uint64_t TEL_Reserve : 2;
//    } DetailInfo;
//} TEL_4;

//typedef __packed union TEL_12
//{
//    uint8_t data[8];
//    struct
//    {
//        uint64_t TEL_Time_Year : 8;
//        // uint64_t TEL_Reserve1 : 4;
//        uint64_t TEL_Time_Month : 4;
//        uint64_t TEL_Time_Day : 5;
//
//        uint64_t TEL_Time_Hour : 5;
//        // uint64_t TEL_Reserve2 : 2;
//
//        uint64_t TEL_Time_Min : 6;
//        uint64_t TEL_Time : 6;
//
//        uint64_t TEL_BookChrgSt : 3;
//        uint64_t TEL_TimeVD : 1;
//
//        uint64_t TEL_Reserve3 : 26;
//    } DetailInfo;
//} TEL_12;


/* CANID: 0x18FEE6EE, Time and Date, Periodic: 1000ms */
typedef __packed union EVT_TD {
    uint8_t data[8];
    
    struct {
        uint64_t EVT_Sec : 8;           /* 秒 */
        uint64_t EVT_Min : 8;           /* 分 */
        uint64_t EVT_Hours : 8;         /* 时 */
        uint64_t EVT_Month : 8;         /* 月 */
        uint64_t EVT_Day : 8;           /* 日 */
        uint64_t EVT_Year : 8;          /* 年 */
        uint64_t EVT_LclMinOfs : 8;     /* 当地分偏量 */
        uint64_t EVT_LclHourOfs : 8;    /* 当地时偏量 */
    } DetailInfo;
} EVT_TD;


//typedef union TEL_18
//{
//    uint8_t data[8];
//    struct
//    {
//        uint64_t TEL_ACU_GNSSclletAuthorStartMon : 4;
//        uint64_t TEL_ACU_GNSSclletAuthorStartDay : 5;
//        uint64_t TEL_Reserve1 : 2;
//        uint64_t TEL_ACU_GNSSclletAuthorStopMon : 4;
//        uint64_t TEL_ACU_GNSSclletAuthorStopDay : 5;
//        uint64_t TEL_ACU_GNSSclletAuthorStartYear : 8;
//        uint64_t TEL_ACU_GNSSclletAuthorStopYear : 8;
//        uint64_t TEL_ACU_GNSSclletAuthorMode : 2;
//        uint64_t TEL_GNSSAPPTypeSt : 24;
//        uint64_t TEL_Reserve2 : 2;
//    } DetailInfo;
//} TEL_18;

typedef enum
{
    E_TEL_TIME_INVALID = 0U,
    E_TEL_TIME_VALID = 1U,
} TEL_TimeVD_e;

typedef enum
{
    E_BookMode_NotBooking = 0x00,        /* 0x0 = Not booking */
    E_BookMode_Cycle_NotStarted = 0x01,  /* 0x1 = Not started in cycle mode */
    E_BookMode_Cycle_InTime = 0x00,      /* 0x2 = In the time of cycle mode */
    E_BookMode_Signal_NotStarted = 0x03, /* 0x3 = Not started in signal mode */
    E_BookMode_Signal_InTime = 0x04,     /* 0x4 = In the time of signal mode */
    E_BookMode_Single_End = 0x05,        /* 0x5 = End in single mode */
} TEL_BookChrgSt_r;


/****************************** Function Declarations *************************/
static int16_t CanPeriodCycleInit(uint32_t cycleTime);

//static int16_t CanPeriodMessage3C5(uint8_t *pCanData);
//static int16_t CanPeriodMessage35F(uint8_t *pCanData);
//static int16_t CanPeriodMessage35C(uint8_t *pCanData);
//static int16_t CanPeriodMessage39E(uint8_t *pCanData);
//static int16_t CanPeriodMessage273(uint8_t *pCanData);


// 外发报文
static int16_t CanPeriodMessage_18FEE6EE(uint8_t *pCanData);
//static int16_t CanPeriodMessage_1820F03A(uint8_t *pCanData);
static void CanPeriodSetCanConfigureWakeUpStartTime(uint32_t startTime_ms, const CanSendMsgConfigure_t *pMsgConfigure, CanSendMsgBuffer_t *pMsgBuffer, uint32_t elementSize);
static void CanPeriodSetCanConfigureInitializeStartTime(uint32_t startTime_ms, const CanSendMsgConfigure_t *pMsgConfigure, CanSendMsgBuffer_t *pMsgBuffer, uint32_t elementSize);
static void CanPeriodSendCycleProcess(int16_t canHandle, const CanSendMsgConfigure_t *pMsgConfigure, CanSendMsgBuffer_t *pMsgBuffer, uint32_t elementSize);
/****************************** Global Variables ******************************/
static uint8_t g_sleepFlag = 0;
static uint8_t g_powerManageHandle = -1;
static uint32_t g_cycleTime = 10;

//static TEL_11_A_T g_tbox11Message;
//static TEL_3 g_tbox3Message;
//static TEL_4 g_tbox4Message;
//static TEL_12 g_tbox12Message;
//static TEL_18 g_tbox18Message;

static EVT_TD g_EvtTd;


//static TEL_TimeVD_e g_timeVdValue = E_TEL_TIME_INVALID;


CAN_CYCLE_SEND_CONFIGURE_BEGIN(1)
/****************************    Time,  Id,       FdFlag    Length  CanllBack******/
//CAN_CYCLE_SEND_CONFIGURE_CAN(500, 0x3C5, 1, 8, CanPeriodMessage3C5)
//CAN_CYCLE_SEND_CONFIGURE_CAN(100, 0x35F, 1, 8, CanPeriodMessage35F)
//CAN_CYCLE_SEND_CONFIGURE_CAN(200, 0x35C, 1, 8, CanPeriodMessage35C)
//CAN_CYCLE_SEND_CONFIGURE_CAN(500, 0x39E, 1, 8, CanPeriodMessage39E)
//CAN_CYCLE_SEND_CONFIGURE_CAN(500, 0x273, 1, 8, CanPeriodMessage273)
CAN_CYCLE_SEND_CONFIGURE_CAN(1000, 0x18FEE6EE, 0, 8, CanPeriodMessage_18FEE6EE)
//CAN_CYCLE_SEND_CONFIGURE_CAN(1000, 0x1820F03A, 0, 8, CanPeriodMessage_1820F03A)
CAN_CYCLE_SEND_CONFIGURE_END(1)



const CanChannelCycleSendConfigure_t g_canCycleConfigureList[] =
{
    {
        .canChannel = TBOX_CAN_CHANNEL_1,
        .msgStartTime = 90,
        .pCycleConfigureList = CYCLE_SEND_CAN_CONFIGURE(1),
        .cycleConfigureListSize = CYCLE_SEND_CAN_CONFIGURE_SIZE(1),
        .pMsgBuffer = CYCLE_SEND_CAN_BUFFER(1)
    },
};

#pragma default_variable_attributes = @ "SYS_RAM"

static CanChannelBuffer_t g_canChannelBufferList[SIZE_OF_ARRAY(g_canCycleConfigureList)];

#pragma default_variable_attributes =


/****************************** Public Function Implementations ******************************/
/*************************************************
   Function:        CanPeriodCycleStart
   Description:     Start CAN message periodic sending
   Input:           None
   Output:          None
   Return:          0 on success, -1 on failure
   Others:
 *************************************************/
int16_t CanPeriodCycleStart(void)
{
    uint8_t i = 0;
    for (i = 0; i < SIZE_OF_ARRAY(g_canCycleConfigureList); i++)
    {
        g_canChannelBufferList[i].enableFlag = 0x01;
    }
    return 0;
}

/*************************************************
   Function:        CanPeriodSendEnable
   Description:     Enable periodic sending on specific CAN channel
   Input:           canChannel - CAN channel number
   Output:          None
   Return:          0 on success, -1 on failure
   Others:
 *************************************************/
int16_t CanPeriodSendEnable(uint8_t canChannel)
{
    uint8_t i = 0, j = 0;
    int16_t ret = -1;
    for (i = 0; i < SIZE_OF_ARRAY(g_canCycleConfigureList); i++)
    {
        if (g_canCycleConfigureList[i].canChannel == canChannel)
        {
            g_canChannelBufferList[i].enableFlag = 0x01;
            for (j = 0; j < g_canCycleConfigureList[i].cycleConfigureListSize; j++)
            {
                g_canCycleConfigureList[i].pMsgBuffer[j].timeCount = (g_canCycleConfigureList[i].pCycleConfigureList[j].cycleTime - 7);
            }
            ret = 0;
            break;
        }
    }
    return ret;
}

/*************************************************
   Function:        CanGetAppMsgEnable
   Description:     Get application message enable status for specific CAN channel
   Input:           canChannel - CAN channel number
   Output:          None
   Return:          appEnableFlag - enable status flag (0 or 1)
   Others:
 *************************************************/
uint8_t CanGetAppMsgEnable(uint8_t canChannel)
{
    uint8_t i = 0U;
    uint8_t appEnableFlag = 0U;
    for (i = 0; i < SIZE_OF_ARRAY(g_canCycleConfigureList); i++)
    {
        if (g_canCycleConfigureList[i].canChannel == canChannel)
        {
            appEnableFlag = g_canChannelBufferList[i].enableFlag;
        }
    }
    return appEnableFlag;
}

/*************************************************
   Function:        CanPeriodSendDisable
   Description:     Disable periodic sending on specific CAN channel
   Input:           canChannel - CAN channel number
   Output:          None
   Return:          0 on success, -1 on failure
   Others:
 *************************************************/
int16_t CanPeriodSendDisable(uint8_t canChannel)
{
    uint8_t i, j;
    int16_t ret = -1;
    for (i = 0; i < SIZE_OF_ARRAY(g_canCycleConfigureList); i++)
    {
        if (g_canCycleConfigureList[i].canChannel == canChannel)
        {
            for (j = 0; j < g_canCycleConfigureList[i].cycleConfigureListSize; j++)
            {
                g_canCycleConfigureList[i].pMsgBuffer[j].timeCount = (g_canCycleConfigureList[i].pCycleConfigureList[j].cycleTime - 3);
            }
            g_canChannelBufferList[i].enableFlag = 0x00;
            ret = 0;
            break;
        }
    }
    return ret;
}

/*************************************************
   Function:        CanPeriodSendEnableAll
   Description:     Enable periodic sending on all CAN channels
   Input:           None
   Output:          None
   Return:          0 on success, -1 on failure
   Others:
 *************************************************/
int16_t CanPeriodSendEnableAll(void)
{
    uint8_t i = 0, j = 0;
    for (i = 0; i < SIZE_OF_ARRAY(g_canCycleConfigureList); i++)
    {
        g_canChannelBufferList[i].enableFlag = 0x01;
        for (j = 0; j < g_canCycleConfigureList[i].cycleConfigureListSize; j++)
        {
            g_canCycleConfigureList[i].pMsgBuffer[j].timeCount = (g_canCycleConfigureList[i].pCycleConfigureList[j].cycleTime - 7);
        }
    }
    return 0;
}

/*************************************************
   Function:        CanPeriodSendDisableAll
   Description:     Disable periodic sending on all CAN channels
   Input:           None
   Output:          None
   Return:          0 on success, -1 on failure
   Others:
 *************************************************/
int16_t CanPeriodSendDisableAll(void)
{
    uint8_t i, j;
    for (i = 0; i < SIZE_OF_ARRAY(g_canCycleConfigureList); i++)
    {
        for (j = 0; j < g_canCycleConfigureList[i].cycleConfigureListSize; j++)
        {
            g_canCycleConfigureList[i].pMsgBuffer[j].timeCount = (g_canCycleConfigureList[i].pCycleConfigureList[j].cycleTime - 3);
        }
        g_canChannelBufferList[i].enableFlag = 0x00;
    }
    return 0;
}

/*************************************************
   Function:        CanPeriodSendReset
   Description:     Reset periodic sending on specific CAN channel
   Input:           canChannel - CAN channel number
   Output:          None
   Return:          0 on success, -1 on failure
   Others:
 *************************************************/
int16_t CanPeriodSendReset(uint8_t canChannel)
{
    uint8_t i = 0;
    int16_t ret = -1;
    for (i = 0; i < SIZE_OF_ARRAY(g_canCycleConfigureList); i++)
    {
        if (g_canCycleConfigureList[i].canChannel == canChannel)
        {
            if (g_canChannelBufferList[i].enableFlag == 1)
            {
                g_canChannelBufferList[i].resetFlag = 0x01;
                ret = 0;
                break;
            }
        }
    }
    return ret;
}

/*************************************************
   Function:        CanPeriodCycleProcess
   Description:     Main process function for periodic CAN message sending
   Input:           None
   Output:          None
   Return:          0 on success, -1 on failure
   Others:          Handles sleep state detection and message sending logic
 *************************************************/
int16_t CanPeriodCycleProcess(void)
{
    int16_t sleepState = 0;
    uint16_t i = 0;
    sleepState = PowerManageSdkGetSleepState(g_powerManageHandle);
    if (sleepState == 0)
    {
        PowerManageSdkSetSleepAck(g_powerManageHandle);
        g_sleepFlag = 1;
    }
    else
    {
        if (g_sleepFlag)
        {
            // set the time  delayed after can wakeup , app can message send
            for (i = 0; i < SIZE_OF_ARRAY(g_canCycleConfigureList); i++)
            {
                CanPeriodSetCanConfigureWakeUpStartTime(g_canCycleConfigureList[i].msgStartTime, g_canCycleConfigureList[i].pCycleConfigureList, g_canCycleConfigureList[i].pMsgBuffer, g_canCycleConfigureList[i].cycleConfigureListSize);
            }
        }
        for (i = 0; i < SIZE_OF_ARRAY(g_canCycleConfigureList); i++)
        {
            if (g_canChannelBufferList[i].enableFlag)
            {
                CanPeriodSendCycleProcess(g_canChannelBufferList[i].canHandle, g_canCycleConfigureList[i].pCycleConfigureList, g_canCycleConfigureList[i].pMsgBuffer, g_canCycleConfigureList[i].cycleConfigureListSize);
            }
        }
        g_sleepFlag = 0x00;
    }
    return 0;
}

/*************************************************
   Function:        canPeriodTaskMain
   Description:     Main task function for CAN periodic sending
   Input:           pvParameters - Task parameters (not used)
   Output:          None
   Return:          None
   Others:          Runs periodically based on FreeRTOS task delay
 *************************************************/
void canPeriodTaskMain(void *pvParameters)
{
    TickType_t xLastWakeTime;
    CanPeriodCycleInit(5);
    xLastWakeTime = xTaskGetTickCount();
    while (1)
    {
        CanPeriodCycleProcess();
        vTaskDelayUntil(&xLastWakeTime, 5);
    }
}

/****************************** Private Function Implementations ***************/
/*************************************************
   Function:        CanPeriodMessage3C5
   Description:     Process and prepare CAN message with ID 0x3C5
   Input:           pCanData - Pointer to CAN data buffer
   Output:          pCanData - Filled with message data
   Return:          0 on success, -1 on failure
   Others:          Sets latitude, longitude and navigation position valid data
 *************************************************/
//static int16_t CanPeriodMessage3C5(uint8_t *pCanData)
//{
//    uint16_t ret = 0U;
//    if (pCanData != NULL)
//    {
//        memset(pCanData, 0x00, 8); 
//        
//        const ftyCircleDataToMcu_t *ftyData = StateSyncGetFtyData();
//
//        if (ftyData != NULL)
//        {
//            const GpsPosition_t *gpsPos = &ftyData->gpsPosition;
//
//            if (gpsPos->fixState > 0)
//            {
//                g_tbox11Message.DetailInfo.TEL_NaviPstVD = 1U;
//            }
//            else
//            {
//                g_tbox11Message.DetailInfo.TEL_NaviPstVD = 0U;
//            }
//
//            uint32_t lat_abs_1e6 = gpsPos->latitude / 10;
//            uint32_t lon_abs_1e6 = gpsPos->longitude / 10;
//
//            int32_t lat_signed = (gpsPos->northSouth == 0) ? (int32_t)lat_abs_1e6 : -(int32_t)lat_abs_1e6;
//            int32_t lon_signed = (gpsPos->eastWest == 0) ? (int32_t)lon_abs_1e6 : -(int32_t)lon_abs_1e6;
//
//            //TBOX_PRINT("lat_signed=0X%X, lon_signed=0X%X\r\n", lat_signed, lon_signed);
//
//            pCanData[0] = (uint8_t)((lon_signed >> 20) & 0xFF); // Bit 27-20
//            pCanData[1] = (uint8_t)((lon_signed >> 12) & 0xFF); // Bit 19-12
//            pCanData[2] = (uint8_t)((lon_signed >> 4) & 0xFF);  // Bit 11-4
//            
//            pCanData[3] = (uint8_t)((lon_signed << 4) & 0xF0); 
//
//            pCanData[3] |= (uint8_t)((lat_signed >> 24) & 0x0F);
//
//            pCanData[4] = (uint8_t)((lat_signed >> 16) & 0xFF); // Bit 23-16
//            pCanData[5] = (uint8_t)((lat_signed >> 8) & 0xFF);  // Bit 15-8
//            pCanData[6] = (uint8_t)(lat_signed & 0xFF);         // Bit 7-0
//
//            pCanData[7] = (uint8_t)(g_tbox11Message.DetailInfo.TEL_NaviPstVD & 0x01);
//        }
//    }
//    else
//    {
//        ret = -1;
//        TBOX_PRINT("can3C5 period send ptr is null\r\n");
//    }
//    return ret;
//}

/*************************************************
   Function:        CanPeriodMessage35F
   Description:     Process and prepare CAN message with ID 0x35F
   Input:           pCanData - Pointer to CAN data buffer
   Output:          pCanData - Filled with message data
   Return:          0 on success, -1 on failure
   Others:          Sets various telematics status data including GPS, GPRS, Bluetooth
 *************************************************/

/*************************************************
   Function:        CanPeriodMessage35F
   Description:     Process and prepare CAN message with ID 0x35F
   Input:           pCanData - Pointer to CAN data buffer
   Output:          pCanData - Filled with message data
   Return:          0 on success, -1 on failure
   Others:          Sets various telematics status data including GPS, GPRS, Bluetooth
 *************************************************/
//static int16_t CanPeriodMessage35F(uint8_t *pCanData)
//{
//    uint16_t ret = 0U;
//
//    if (pCanData != NULL)
//    {
//        memset(pCanData, 0x00, 8);
//        const ftyCircleDataToMcu_t *ftyData = StateSyncGetFtyData();
//
//        if (ftyData != NULL)
//        {
//            if (ftyData->gnssModuleStatus.gpsEnable == 1)
//            {
//                g_tbox3Message.DetailInfo.TEL_GPSCommSt = 1U;
//            }
//            else
//            {
//                g_tbox3Message.DetailInfo.TEL_GPSCommSt = 0U;
//            }
//
//            g_tbox3Message.DetailInfo.TEL_TelematicsRegisterSt = (uint64_t)(ftyData->lteNetworkStatus.b2_3.netState & 0x07);
//        }
//        else
//        {
//            g_tbox3Message.DetailInfo.TEL_GPSCommSt = 0U;
//            g_tbox3Message.DetailInfo.TEL_TelematicsRegisterSt = 0U;
//        }
//
//        g_tbox3Message.DetailInfo.TEL_GPRSOr3GCommSt = 0U;
////        g_tbox3Message.DetailInfo.TEL_TelematicsModeActSt = XCallGetTelemataticsMode();
//        g_tbox3Message.DetailInfo.TEL_TelematicsModeActSt = 0U;     // SXSYC no eCall
//        g_tbox3Message.DetailInfo.TEL_BTPowerSt = 0U;
//        g_tbox3Message.DetailInfo.TEL_BTAutoconectingSt = 0U;
//        g_tbox3Message.DetailInfo.TEL_BTHisdel = 0U;
//        g_tbox3Message.DetailInfo.TEL_BTHandfreeSt = 0U;
//        g_tbox3Message.DetailInfo.TEL_BTAvrcpPlayMode = 0U;
//        g_tbox3Message.DetailInfo.TEL_BTAvrcpSt = 0U;
//        g_tbox3Message.DetailInfo.TEL_OTAState = RemoteControlGetOtaFlag();
//        g_tbox3Message.DetailInfo.TEL_BTManualMode = 0U;
//
//        g_tbox3Message.DetailInfo.TEL_MsgCounter = g_TEL_MsgCounter;
//        g_TEL_MsgCounter = (g_TEL_MsgCounter + 1U) % 16U;
//        memcpy(pCanData, &g_tbox3Message.data[0], sizeof(g_tbox3Message.data));
//    }
//    else
//    {
//        ret = -1;
//        TBOX_PRINT("can35F period send ptr is null\r\n");
//    }
//    return ret;
//}

/*************************************************
   Function:        CanPeriodMessage35C
   Description:     Process and prepare CAN message with ID 0x35C
   Input:           pCanData - Pointer to CAN data buffer
   Output:          pCanData - Filled with message data
   Return:          0 on success, -1 on failure
   Others:          Sets phone number and Bluetooth connection status data
 *************************************************/
//static int16_t CanPeriodMessage35C(uint8_t *pCanData)
//{
//    uint16_t ret = 0U;
//    if (pCanData != NULL)
//    {
//        memset(pCanData, 0x00, 8);
//        // Get for interface
//        g_tbox4Message.DetailInfo.TEL_PhoneNumber = 0U;
//        g_tbox4Message.DetailInfo.TEL_BTDisplayAct = 0U;
//        g_tbox4Message.DetailInfo.TEL_BTConnectingAct = 0U;
//        g_tbox4Message.DetailInfo.TEL_BTConnectingSt = 0U;
//        g_tbox4Message.DetailInfo.TEL_PhoneCallSt = 0U;
//        g_tbox4Message.DetailInfo.TEL_A2DPSupportSt = 0U;
//        g_tbox4Message.DetailInfo.TEL_AVRCPSupportSt = 0U;
//        g_tbox4Message.DetailInfo.TEL_MobilePowerSt = 0U;
//        g_tbox4Message.DetailInfo.TEL_MobileSignalSt = 0U;
//        memcpy(pCanData, &g_tbox4Message.data[0], sizeof(g_tbox4Message.data));
//    }
//    else
//    {
//        ret = -1;
//        TBOX_PRINT("can35C period send ptr is null\r\n");
//    }
//    return ret;
//}

/*************************************************
   Function:        CanPeriodMessage39E
   Description:     Process and prepare CAN message with ID 0x39E
   Input:           pCanData - Pointer to CAN data buffer
   Output:          pCanData - Filled with message data
   Return:          0 on success, -1 on failure
   Others:          Sets real-time clock data including year, month, day, hour, minute, second
 *************************************************/
//static int16_t CanPeriodMessage39E(uint8_t *pCanData)
//{
//    uint16_t ret = 0U;
//    TimeData_t Beijing_Time;
//    if (pCanData != NULL)
//    {
//        memset(pCanData, 0x00, 8);
//        const ftyCircleDataToMcu_t *ftyData = StateSyncGetFtyData();
//
//        if (ftyData != NULL)
//        {
//            g_timeVdValue = E_TEL_TIME_VALID;
//            pCanData[0] = (ftyData->timeData.year - 2000) & 0xFF;
//            pCanData[1] = ((ftyData->timeData.month << 4) | (ftyData->timeData.day) >> 1) & 0xFF;
//            pCanData[2] = ((ftyData->timeData.day << 7) | (ftyData->timeData.hour << 2) | ((ftyData->timeData.minute >> 4) & 0x3)) & 0xFF;
//            pCanData[3] = ((ftyData->timeData.minute << 4) | (ftyData->timeData.second >> 2) & 0xF) & 0xFF;
//            pCanData[4] = ((ftyData->timeData.second << 6) & 0xFF) | ((E_BookMode_Cycle_InTime & 0x7) << 3) | ((g_timeVdValue & 0x1) << 2);
//            pCanData[5] = 0;
//            pCanData[6] = 0;
//            pCanData[7] = 0;
//
//            // g_timeVdValue = E_TEL_TIME_VALID;
//            // Beijing_Time.year = ftyData->timeData.year;
//            // Beijing_Time.month = ftyData->timeData.month;
//            // Beijing_Time.day = ftyData->timeData.day;
//            // Beijing_Time.hour = ftyData->timeData.hour;
//            // Beijing_Time.minute = ftyData->timeData.minute;
//            // Beijing_Time.second = ftyData->timeData.second;
//
//            // g_tbox12Message.DetailInfo.TEL_Time_Year = (Beijing_Time.year - 2000);
//            // g_tbox12Message.DetailInfo.TEL_Time_Month = Beijing_Time.month;
//            // g_tbox12Message.DetailInfo.TEL_Time_Hour = Beijing_Time.hour;
//            // g_tbox12Message.DetailInfo.TEL_Time_Day = Beijing_Time.day;
//            // g_tbox12Message.DetailInfo.TEL_Time_Min = Beijing_Time.minute;
//            // g_tbox12Message.DetailInfo.TEL_TimeVD = g_timeVdValue;
//            // g_tbox12Message.DetailInfo.TEL_BookChrgSt = E_BookMode_Cycle_InTime;
//
//            // g_tbox12Message.DetailInfo.TEL_Time = Beijing_Time.second;
//
//            // TBOX_PRINT("时间:g_tbox12Message.DetailInfo.TEL_Time_Year=%d, TEL_Time_Month=%d, TEL_Time_Day=%d, TEL_Time_Hour=%d, TEL_Time_Min=%d, TEL_Time=%d\r\n",
//            //            g_tbox12Message.DetailInfo.TEL_Time_Year,
//            //            g_tbox12Message.DetailInfo.TEL_Time_Month,
//            //            g_tbox12Message.DetailInfo.TEL_Time_Day,
//            //            g_tbox12Message.DetailInfo.TEL_Time_Hour,
//            //            g_tbox12Message.DetailInfo.TEL_Time_Min,
//            //            g_tbox12Message.DetailInfo.TEL_Time);
//        }
//
//        else
//        {
//            TimerHal_GetRtcTime(&Beijing_Time);
//            g_timeVdValue = E_TEL_TIME_INVALID;
//            pCanData[0] = (Beijing_Time.year - 2000) & 0xFF;
//            pCanData[1] = ((Beijing_Time.month << 4) | (Beijing_Time.day >> 1)) & 0xFF;
//            pCanData[2] = ((Beijing_Time.day << 7) | (Beijing_Time.hour << 2) | ((Beijing_Time.minute >> 4) & 0x3)) & 0xFF;
//            pCanData[3] = ((Beijing_Time.minute << 4) | (Beijing_Time.second >> 2) & 0xF) & 0xFF;
//            pCanData[4] = ((Beijing_Time.second << 6) & 0xFF) | ((E_BookMode_Cycle_InTime & 0x7) << 3) | ((g_timeVdValue & 0x1) << 2);
//            pCanData[5] = 0;
//            pCanData[6] = 0;
//            pCanData[7] = 0;
//            // TimerHal_RtcRemoteTimeSync(&Beijing_Time);
//        }
//
//        // if (TimeSyncSdkGetRealTime(8, &Beijing_Time) == 0)
//        // {
//        //     g_timeVdValue = E_TEL_TIME_VALID;
//        //     if (Beijing_Time.year >= 2025)
//        //     {
//        //         g_tbox12Message.DetailInfo.TEL_Time_Year = (Beijing_Time.year - 2000);
//        //         g_tbox12Message.DetailInfo.TEL_Time_Month = (Beijing_Time.month << 4) & (0xF0);
//        //         g_tbox12Message.DetailInfo.TEL_Time_Hour = (Beijing_Time.hour << 3) & (0xF8);
//        //         g_tbox12Message.DetailInfo.TEL_Time_Day = (Beijing_Time.day << 3) & (0xF8);
//        //         g_tbox12Message.DetailInfo.TEL_Time_Min = (Beijing_Time.minute << 2) & (0xFC);
//        //         g_tbox12Message.DetailInfo.TEL_TimeVD = g_timeVdValue;
//        //         g_tbox12Message.DetailInfo.TEL_BookChrgSt = E_BookMode_Cycle_InTime;
//        //         g_tbox12Message.DetailInfo.TEL_Time = Beijing_Time.second << 2;
//        //         TBOX_PRINT("can39E period send time: %04d-%02d-%02d %02d:%02d:%02d\r\n",
//        //                    Beijing_Time.year,
//        //                    Beijing_Time.month,
//        //                    Beijing_Time.day,
//        //                    Beijing_Time.hour,
//        //                    Beijing_Time.minute,
//        //                    Beijing_Time.second);
//        //     }
//        //     else
//        //     {
//        //         g_timeVdValue = E_TEL_TIME_INVALID;
//        //         g_tbox12Message.DetailInfo.TEL_TimeVD = g_timeVdValue;
//        //         TBOX_PRINT("can39E period send year < 2025\r\n");
//        //     }
//        // }
//        // else
//        // {
//        //     g_timeVdValue = E_TEL_TIME_INVALID;
//        //     g_tbox12Message.DetailInfo.TEL_TimeVD = g_timeVdValue;
//        //     TBOX_PRINT("can39E period send get time error\r\n");
//        // }
//        // memcpy(pCanData, &g_tbox12Message.data[0], sizeof(g_tbox12Message.data));
//    }
//    else
//    {
//        ret = -1;
//        TBOX_PRINT("can39E period send ptr is null\r\n");
//    }
//    return ret;
//}

/*************************************************
   Function:        CanPeriodMessage273
   Description:     Process and prepare CAN message with ID 0x273
   Input:           pCanData - Pointer to CAN data buffer
   Output:          pCanData - Filled with message data
   Return:          0 on success, -1 on failure
   Others:          Sets GNSS collection authorization data
 *************************************************/
//static int16_t CanPeriodMessage273(uint8_t *pCanData)
//{
//    uint16_t ret = 0U;
//    if (pCanData != NULL)
//    {
//        memset(pCanData, 0x00, 8);
//        // Get for interface
//        g_tbox18Message.DetailInfo.TEL_ACU_GNSSclletAuthorStartMon = 0U;
//        g_tbox18Message.DetailInfo.TEL_ACU_GNSSclletAuthorStartDay = 0U;
//        g_tbox18Message.DetailInfo.TEL_ACU_GNSSclletAuthorStopMon = 0U;
//        g_tbox18Message.DetailInfo.TEL_ACU_GNSSclletAuthorStopDay = 0U;
//        g_tbox18Message.DetailInfo.TEL_ACU_GNSSclletAuthorStartYear = 0U;
//        g_tbox18Message.DetailInfo.TEL_ACU_GNSSclletAuthorStopYear = 0U;
//        g_tbox18Message.DetailInfo.TEL_ACU_GNSSclletAuthorMode = 0U;
//        g_tbox18Message.DetailInfo.TEL_GNSSAPPTypeSt = 0U;
//        memcpy(pCanData, &g_tbox18Message.data[0], sizeof(g_tbox18Message.data));
//    }
//    else
//    {
//        ret = -1;
//        TBOX_PRINT("can273 period send ptr is null\r\n");
//    }
//    return ret;
//}

static int16_t CanPeriodMessage_18FEE6EE(uint8_t *pCanData)
{
    uint16_t ret = 0U;
    TimeSevice_t tim = {0};
    
    if (pCanData != NULL) {
        memset(pCanData, 0x00, 8);

        if (TimeSyncSdkGetRealTime(0, &tim) == 0) {
            g_EvtTd.DetailInfo.EVT_Sec = tim.second;
            g_EvtTd.DetailInfo.EVT_Min = tim.minute;
            g_EvtTd.DetailInfo.EVT_Hours = tim.hour;
            g_EvtTd.DetailInfo.EVT_Day = tim.day;
            g_EvtTd.DetailInfo.EVT_Month= tim.month;
            g_EvtTd.DetailInfo.EVT_Year = tim.year - 2000;
            g_EvtTd.DetailInfo.EVT_LclMinOfs = 0U;
            g_EvtTd.DetailInfo.EVT_LclHourOfs = 0U;
            memcpy(pCanData, g_EvtTd.data, sizeof(g_EvtTd.data));
        } else {
            g_EvtTd.DetailInfo.EVT_Sec = 0U;
            g_EvtTd.DetailInfo.EVT_Min = 0U;
            g_EvtTd.DetailInfo.EVT_Hours = 0U;
            g_EvtTd.DetailInfo.EVT_Day = 0U;
            g_EvtTd.DetailInfo.EVT_Month= 0U;
            g_EvtTd.DetailInfo.EVT_Year = 0U;
            g_EvtTd.DetailInfo.EVT_LclMinOfs = 0U;
            g_EvtTd.DetailInfo.EVT_LclHourOfs = 0U;
            memcpy(pCanData, g_EvtTd.data, sizeof(g_EvtTd.data));
        }
    } else {
        ret = -1;
        TBOX_PRINT("can 0x18FEE6EE period send ptr is null\r\n");
    }
    
    return ret;
}

//static int16_t CanPeriodMessage_1820F03A(uint8_t *pCanData)
//{
//    uint16_t ret = 0U;
//    
//    for(uint8_t i = 0; i < 8; i++)
//    {
//       pCanData[i] = 0x00;
//    }
//    
//    return ret;
//}
/*************************************************
   Function:        CanPeriodSetCanConfigureWakeUpStartTime
   Description:     Set initial time counts for messages after wakeup
   Input:           startTime_ms - Start time in milliseconds
                    pMsgConfigure - Pointer to message configuration
                    pMsgBuffer - Pointer to message buffer
                    elementSize - Number of elements in configuration
   Output:          pMsgBuffer - Updated with initial time counts
   Return:          None
   Others:          Adds compensation time for network process delay
 *************************************************/
static void CanPeriodSetCanConfigureWakeUpStartTime(uint32_t startTime_ms, const CanSendMsgConfigure_t *pMsgConfigure, CanSendMsgBuffer_t *pMsgBuffer, uint32_t elementSize)
{
    uint32_t i;
    startTime_ms += 10; // compensate time for network process delay to enable can tx
    for (i = 0; i < elementSize; i++)
    {
        if (pMsgConfigure[i].cycleTime <= 0)
        {
            continue;
        }
        pMsgBuffer[i].timeCount = pMsgConfigure[i].cycleTime - startTime_ms;
    }
}

/*************************************************
   Function:        CanPeriodSetCanConfigureInitializeStartTime
   Description:     Set initial time counts for messages during initialization
   Input:           startTime_ms - Start time in milliseconds
                    pMsgConfigure - Pointer to message configuration
                    pMsgBuffer - Pointer to message buffer
                    elementSize - Number of elements in configuration
   Output:          pMsgBuffer - Updated with initial time counts
   Return:          None
   Others:          Adds compensation time for network process delay
 *************************************************/
static void CanPeriodSetCanConfigureInitializeStartTime(uint32_t startTime_ms, const CanSendMsgConfigure_t *pMsgConfigure, CanSendMsgBuffer_t *pMsgBuffer, uint32_t elementSize)
{
    uint32_t i;
    startTime_ms += 10; // compensate time for network process delay to enable can tx
    for (i = 0; i < elementSize; i++)
    {
        if (pMsgConfigure[i].cycleTime <= 0)
        {
            continue;
        }
        pMsgBuffer[i].timeCount = pMsgConfigure[i].cycleTime - startTime_ms;
    }
}

/*************************************************
   Function:        CanPeriodSendCycleProcess
   Description:     Process and send periodic CAN messages
   Input:           canHandle - CAN channel handle
                    pMsgConfigure - Pointer to message configuration
                    pMsgBuffer - Pointer to message buffer
                    elementSize - Number of elements in configuration
   Output:          None
   Return:          None
   Others:          Handles time counting, message processing and transmission
 *************************************************/
static void CanPeriodSendCycleProcess(int16_t canHandle, const CanSendMsgConfigure_t *pMsgConfigure, CanSendMsgBuffer_t *pMsgBuffer, uint32_t elementSize)
{
    uint32_t i;
    int16_t ret = 0U;
    for (i = 0; i < elementSize; i++)
    {
        if (pMsgConfigure[i].cycleTime <= 0)
        {
            continue;
        }
        pMsgBuffer[i].timeCount += g_cycleTime;
        if (pMsgBuffer[i].timeCount >= pMsgConfigure[i].cycleTime)
        {
            pMsgBuffer[i].timeCount = 0x00;
            if (pMsgConfigure[i].msgProcessFun != NULL)
            {
                ret = pMsgConfigure[i].msgProcessFun(pMsgBuffer[i].canData);
            }
            if (ret == 0)
            {
                if (pMsgConfigure[i].fdFlag)
                {
#if (CYCLE_SEND_WITH_SECOC_SDK)
                    SecocSdkTransmit(canHandle, pMsgConfigure[i].canId, pMsgBuffer[i].canData, pMsgConfigure[i].dataLength, 3);
#else
                    CanHalTransmitQueued(canHandle, pMsgConfigure[i].canId, pMsgBuffer[i].canData, pMsgConfigure[i].dataLength, CYCLE_SEND_WITH_CANFD, CAN_TX_PRIO_NORMAL);
#endif
                }
                else
                {
#if (CYCLE_SEND_WITH_SECOC_SDK)
                    SecocSdkTransmit(canHandle, pMsgConfigure[i].canId, pMsgBuffer[i].canData, pMsgConfigure[i].dataLength, 0);
#else
                    CanHalTransmitQueued(canHandle, pMsgConfigure[i].canId, pMsgBuffer[i].canData, pMsgConfigure[i].dataLength, CYCLE_SEND_WITHOUT_CANFD, CAN_TX_PRIO_NORMAL);
#endif
                }
            }
            else
            {
                TBOX_PRINT("can period send message preocess failed,canid:0x%x,ret:%d\r\n", pMsgConfigure[i].canId, ret);
            }
        }
    }
}

/*************************************************
   Function:        CanPeriodCycleInit
   Description:     Initialize CAN periodic message sending system
   Input:           cycleTime - Cycle time for the main processing loop
   Output:          None
   Return:          0 on success, -1 on failure
   Others:          Initializes CAN channels, buffers, and power management handle
 *************************************************/
static int16_t CanPeriodCycleInit(uint32_t cycleTime)
{
    const char moduleName[] = {'C', 'a', 'n', 'C', 'y', 'c', 'l', 'e', 'S', 'e', 'n', 'd', 'T', 'a', 's', 'k'};
    uint8_t i = 0;
    for (i = 0; i < SIZE_OF_ARRAY(g_canCycleConfigureList); i++)
    {
        // CAN channel flag buffer initialization
        g_canChannelBufferList[i].canHandle = CanHalOpen(g_canCycleConfigureList[i].canChannel);
        g_canChannelBufferList[i].enableFlag = 0x01;
        // Set initial time initialization
        CanPeriodSetCanConfigureInitializeStartTime(g_canCycleConfigureList[i].msgStartTime, g_canCycleConfigureList[i].pCycleConfigureList, g_canCycleConfigureList[i].pMsgBuffer, g_canCycleConfigureList[i].cycleConfigureListSize);
    }
    // Power management handle initialization
    g_powerManageHandle = PowerManageSdkOpenHandle(moduleName);
    if (cycleTime != 0)
    {
        g_cycleTime = cycleTime;
    }
    return 0;
}
