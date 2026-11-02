/****************************** include ***************************************/
#include "taskEcallProcess.h"
#include "taskPowerManage.h"
#include "vehicleSignalApp.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "task.h"
#include "string.h"
#include "logHal.h"
#include "mpuHal.h"
#include "peripheralHal.h"
#include "alarmSdk.h"
#include "batterySdk.h"
#include "mpuPowerSyncSdk.h"
#include "canParseSdk.h"
#include "stateSyncSdk.h"
#include "taskDtcProcess.h"
#include "task.h"
#include "ecuNodeMissingCheck.h"

/****************************** Macro Definitions ******************************/
#define ECALL_PROCESS_CYCLE_TIME            10 //ms
#define	SOS_LED_TRIG_MSG_QUEUE_DEPTH	    ( 5 )
#define SOS_KEY_DEBANCE_TIME                ( 100 )                         /*按键消除去抖动的时间*/
#define SOS_KEY_PRESS_MIN_TIME              ( 500 )                         /*按键被按下最短时间*/
#define SOS_KEY_PRESS_MAX_TIME              ( 10000 )                        /*按键被按下最长时间*/
#define SOS_KEY_RELEASED_TIME               ( 2000 )                        /*按键被释放的时间*/
#define SOS_KEY_ECALL_TEST_TIME             ( 10000 )                       /*按键ECALL测试模式的时间*/
#define SOS_KEY_RESET_TBOX_TIME             ( 20000 )                       /*按键复位TBOX的时间*/    
#define SOS_KEY_HARD_FAULT_TIME             ( 30000 )                       /*按键硬件故障的时间*/    

#define RULE_NORMAL                         0
#define RULE_CRASH                          1
#define SYSTEM_CHECK_RUN_TIME	            ( 80 * 1000 )	    /*硬件上电后连续60秒钟*/
/****************************** Type Definitions ******************************/
typedef struct
{
    uint16_t minHigh;
    uint16_t maxHigh;
    uint16_t minLow;
    uint16_t maxLow;
    uint8_t  needCycles;
} PwmRule_t;

typedef struct
{
    uint32_t lastTick;
    uint16_t lastHigh;
    uint16_t lastLow;
    uint8_t  okCount[2];    /* normal / crash 分别计数 */
    AirbagPwmState_e stableState;
    uint8_t lastLevel;
} PwmRuntime_t;

/****************************** Global Variables ******************************/
static SosLledState_e g_SosLedState;        
static SosButtonClickMsg_t g_SosButtonClickMsg;
#if(0)
static const uint8_t g_CanSignalFormat = VEHICLE_CAN_UNPACK_FORMAT_MOTO_LSB;
static uint8_t selfCheckStatusPrintFlag = 0;
#endif
static const PwmRule_t g_pwmRules[] =
{
    /* 规则0：正常 66.7% (高 20ms±2, 低 10ms±2) */
    { .minHigh = 18u, .maxHigh = 22u,
      .minLow  = 8u,  .maxLow  = 12u,
      .needCycles = 3u },

    /* 规则1：碰撞 33.3% (高 10ms±2, 低 20ms±2) */
    { .minHigh = 8u,  .maxHigh = 12u,
      .minLow  = 18u, .maxLow  = 22u,
      .needCycles = 3u },
};
static volatile PwmRuntime_t g_pwm;
static TelematicsMode_e g_telemataticsMode = TELEMATICS_MODE_NOT_ACTIVE;
static PhoneCallState_e g_PhoneCallState = E_ECALL_STATE_NOT_ACTIVE;
/****************************** Function Declarations *************************/
static void AirbagPwmInit(void);
static AirbagPwmState_e AirbagPwmGetState(void);
static void BcallSignalProcess(void);
static void AirbagSingleProcess(void);
static void XCallCloseSignalProcess(void);
/****************************** Public Function Implementations ***************/

/** ****************************************************************************
* @remarks       static void SetSosLedState( ecall_led_flash_e st )
* @brief         设置SOS led灯显示的状态
* @param[in]     st led flash 显示状态
* @param[out]    无
* @return        无
* @attention     无
*******************************************************************************/
static void SetSosLedState( SosLledState_e st )
{
    g_SosLedState = st;
}

/** ****************************************************************************
* @remarks       static SosLledState_e GetSosLedState( void )
* @brief         获取SOS led灯显示的状态
* @param[in]     无
* @param[out]    无
* @return        LED显示的状态
* @attention     无
*******************************************************************************/
static SosLledState_e GetSosLedState( void )
{
    return g_SosLedState;
}

/** ****************************************************************************
* @remarks       static void SosLedControlProcess(void)
* @brief         SOS led闪烁控制函数
* @param[in]     flash LED灯闪烁方式
* @param[out]    无
* @return        无
* @attention     无
*******************************************************************************/
static void SosLedControlProcess(void)
{
    SosLledState_e sos_led_msg;

    if( EcallHalSosLedControlReceive( &sos_led_msg, 0) == pdPASS )
    {
        switch( sos_led_msg )
        {
            case E_SOS_LED_STATE_RING:         /*ECALL拨号中LED状态指示灯 200ms开800ms关*/
                EcallHalSetSosLedMode( E_ECALL_LED_MODE_FLASH , 100 , 100 );
                SetSosLedState( E_SOS_LED_STATE_RING );
            break;
            case E_SOS_LED_STATE_CALL:         /*ECALL正在通话中LED状态指示灯 800ms开200ms关*/
                EcallHalSetSosLedMode( E_ECALL_LED_MODE_FLASH , 100 , 100 );
                SetSosLedState( E_SOS_LED_STATE_CALL );
            break;
            case E_SOS_LED_STATE_WAIT_BACK:    /*ECALL等待PSAP应答时 500ms开500ms关*/
                EcallHalSetSosLedMode( E_ECALL_LED_MODE_FLASH , 500 , 500 );
                SetSosLedState( E_SOS_LED_STATE_WAIT_BACK );
            break;
            case E_SOS_LED_STATE_END:          /*ECALL事件结束 LED灯关闭时全部为 0*/
                EcallHalSetSosLedMode( E_ECALL_LED_MODE_KEEP_OFF , 0 , 0 );
                SetSosLedState( E_SOS_LED_STATE_END );
            break;

            /************************************************************************/
            case E_SOS_LED_STATE_SELFCHECK_ERR:      /*设备自检失败，LED状态指示灯 200ms开1800ms关*/
                EcallHalSetSosLedMode( E_ECALL_LED_MODE_FLASH , 200 , 1800 );
                SetSosLedState( E_SOS_LED_STATE_SELFCHECK_ERR );
            break;
            case E_SOS_LED_STATE_WARNING:         /*中断故障，LED状态指示灯 125ms开125ms关*/
                EcallHalSetSosLedMode( E_ECALL_LED_MODE_FLASH , 125 , 125 );
                SetSosLedState( E_SOS_LED_STATE_WARNING );
            break;  
            case E_SOS_LED_STATE_SELFCHECK_ON:           /*ECALL事件结束 LED灯打开时全部为 0*/
                EcallHalSetSosLedMode( E_ECALL_LED_MODE_KEEP_ON , 0 , 0 );
                SetSosLedState( E_SOS_LED_STATE_SELFCHECK_ON );
            break;
            default:
            break;
        }
    }
}

/** ****************************************************************************
* @remarks       static uint8_t SosButtonPressAction( uint32_t presstime, uint8_t* saveBtnState, SosLledState_e* saveLedState)
* @brief         SOS按键按下动作处理函数
* @param[in]     presstime - 按键按下的时间
* @param[out]    saveBtnState - 保存按键状态的指针
* @param[out]    saveLedState - 保存LED状态的指针
* @return        成功返回0
* @attention     根据按键按下的时间长度执行不同的动作
*******************************************************************************/
static uint8_t SosButtonPressAction( uint32_t presstime, uint8_t* saveBtnState, SosLledState_e* saveLedState)
{
    uint8_t btnState;
    SosLledState_e ledState;

    btnState = *saveBtnState;
    ledState = *saveLedState;

    if( presstime >= SOS_KEY_HARD_FAULT_TIME )
    {
        if(btnState == 2)
        {
            btnState = 3;
            ledState = GetSosLedState();
            TBOX_PRINT("SOS button press over 30S, cur led mode:%d.\r\n", ledState);
            EcallHalSosLedControlSend( E_SOS_LED_STATE_WARNING );
            g_SosButtonClickMsg.hardFault = 1;
        }
    }
    else if( presstime >= SOS_KEY_ECALL_TEST_TIME )
    {
        if(btnState == 0)
        {
            btnState = 1;
            ledState = GetSosLedState();
            TBOX_PRINT("SOS button press over 10S, cur led mode:%d.\r\n", ledState);
            EcallHalSosLedControlSend( E_SOS_LED_STATE_WARNING );
        }
    }

    *saveBtnState = btnState;
    *saveLedState = ledState;

    return 0;
}

/** ****************************************************************************
* @remarks       static uint8_t SosButtonReleaseAction( uint32_t presskeeptime, SosButtonState_e *st )
* @brief         SOS按键释放动作处理函数
* @param[in]     presskeeptime - 按键按下的持续时间
* @param[out]    st - 按键状态指针
* @return        成功返回0
* @attention     根据按键按下的时间长度执行不同的动作，如触发测试模式、重置TBOX等
*******************************************************************************/
static uint8_t SosButtonReleaseAction( uint32_t presskeeptime, SosButtonState_e *st )
{
    SosButtonState_e SosButtonState = *st;

    if( presskeeptime >= SOS_KEY_HARD_FAULT_TIME )
    {
        TBOX_PRINT("SOS button is released, Into idle mode: [E_SOS_BUTTON_STATE_IDLE]\r\n");
        SosButtonState = E_SOS_BUTTON_STATE_IDLE;
        memset( (uint8_t *)&g_SosButtonClickMsg, 0x00, sizeof( SosButtonClickMsg_t ));
    }
    else if( presskeeptime >= SOS_KEY_ECALL_TEST_TIME )
    {
        AlarmSdkEcallTriger(E_ECALL_TRIGGER_TEST_MODE);
        EcallHalSetVehicleMute(1);
        TBOX_PRINT("SOS button triggers the test mode, Into idle mode : [E_SOS_BUTTON_STATE_IDLE]\r\n");
        SosButtonState = E_SOS_BUTTON_STATE_IDLE;
        memset( (uint8_t *)&g_SosButtonClickMsg, 0x00, sizeof( SosButtonClickMsg_t ));
    }
    else if(( presskeeptime >= SOS_KEY_PRESS_MIN_TIME ) && ( presskeeptime <= SOS_KEY_PRESS_MAX_TIME ))
    {
        g_SosButtonClickMsg.releasedTime = xTaskGetTickCount();
        SosButtonState = E_SOS_BUTTON_STATE_RELEASED;
        TBOX_PRINT("SOS button is confirmed to be released : [E_SOS_BUTTON_STATE_RELEASED]\r\n");
    }
    else
    {
        SosButtonState = E_SOS_BUTTON_STATE_IDLE;
        TBOX_PRINT("SOS button is pressed less than 0.5S : [E_SOS_BUTTON_STATE_RELEASED]\r\n");
    }

    *st = SosButtonState;

    return 0;
}

/** ****************************************************************************
* @remarks       static void SosButtonDetection( void )
* @brief         SOS 按键信号周期性处理
* @param[in]     无
* @param[out]    无
* @return        pdPASS 成功，pdFAIL 失败
* @attention     无
*******************************************************************************/
static void SosButtonDetection( void )
{
    uint32_t key_time = 0;
    static SosLledState_e ledState = E_SOS_LED_STATE_INIT;
    static uint8_t btnState = 0;
    static SosButtonState_e SosButtonState = E_SOS_BUTTON_STATE_IDLE;

    if( SosButtonState == E_SOS_BUTTON_STATE_IDLE )            /*检测按键*/
    {
        if( EcallHalGetSosButtonStatus() == 1 )
        {
            g_SosButtonClickMsg.clickTime = xTaskGetTickCount();
            SosButtonState = E_SOS_BUTTON_STATE_CLICK;
        }
    }
    else if(SosButtonState == E_SOS_BUTTON_STATE_CLICK )            /*按键去抖*/
    {
        if( EcallHalGetSosButtonStatus() == 1 )
        {
            g_SosButtonClickMsg.pressTime = xTaskGetTickCount();
            key_time = osElapsedTimeGet( g_SosButtonClickMsg.pressTime, g_SosButtonClickMsg.clickTime );
            if( key_time >= SOS_KEY_DEBANCE_TIME )
            {
                SosButtonState = E_SOS_BUTTON_STATE_PRESS;           /*按键确认被按下*/
                TBOX_PRINT("SOS button is confirmed to be pressed : [E_SOS_BUTTON_STATE_PRESS]\r\n", 0);
            }
        }
        else
        {
            TBOX_PRINT("SOS button jitters, Into idle mode : [E_SOS_BUTTON_STATE_IDLE]\r\n");
            SosButtonState = E_SOS_BUTTON_STATE_IDLE;
            memset( (uint8_t *)&g_SosButtonClickMsg, 0x00, sizeof( SosButtonClickMsg_t ));
        }
    }
    else if(SosButtonState == E_SOS_BUTTON_STATE_PRESS )           /*按键确认被按下*/
    {
        g_SosButtonClickMsg.pressTime = xTaskGetTickCount();
        if( EcallHalGetSosButtonStatus() == 1 )
        {
            key_time = osElapsedTimeGet( g_SosButtonClickMsg.pressTime, g_SosButtonClickMsg.clickTime );
            SosButtonPressAction(key_time, &btnState, &ledState);
        }
        else
        {
            btnState = 0;
            EcallHalSosLedControlSend( ledState );
            key_time = osElapsedTimeGet( g_SosButtonClickMsg.pressTime, g_SosButtonClickMsg.clickTime );
            SosButtonReleaseAction(key_time, &SosButtonState);
        }
    }
    else if(SosButtonState == E_SOS_BUTTON_STATE_RELEASED )
    {
        g_SosButtonClickMsg.releasedTime = xTaskGetTickCount();
        if( EcallHalGetSosButtonStatus() == 1 )
        {
            g_SosButtonClickMsg.cancelTime = xTaskGetTickCount();
            key_time = osElapsedTimeGet( g_SosButtonClickMsg.cancelTime, g_SosButtonClickMsg.pressTime );
            if( key_time >= SOS_KEY_DEBANCE_TIME )
            {
                key_time = osElapsedTimeGet( g_SosButtonClickMsg.releasedTime, g_SosButtonClickMsg.pressTime );
                if( key_time < SOS_KEY_RELEASED_TIME )                  /*规定时间内再次按压按键进行取消*/
                {
                    SosButtonState = E_SOS_BUTTON_STATE_CANCELLED;
                    TBOX_PRINT("SOS button ECALL has been cancelled : [E_SOS_BUTTON_STATE_CANCELLED]\r\n");
                }
            }
        }
        else
        {
            key_time = osElapsedTimeGet( g_SosButtonClickMsg.releasedTime, g_SosButtonClickMsg.pressTime );
            if( key_time >= SOS_KEY_RELEASED_TIME )                     /*触发按键成功*/    
            {
                AlarmSdkEcallTriger(E_ECALL_TRIGGER_BTN_MANN);
                EcallHalSetVehicleMute(1);
                TBOX_PRINT("SOS button triggers ECALL, Into idle mode: [E_SOS_BUTTON_STATE_IDLE]\r\n");
                SosButtonState = E_SOS_BUTTON_STATE_IDLE;
                memset( (uint8_t *)&g_SosButtonClickMsg, 0x00, sizeof( SosButtonClickMsg_t ));
            }
            if(AlarmSdkGetEcallCallState() != 0)
            {
                if( key_time <= SOS_KEY_RELEASED_TIME )                     /*触发按键成功*/    
                {
                    AlarmSdkEcallClose(E_ECALL_TRIGGER_BTN_MANN);
                    TBOX_PRINT("SOS button close ECALL, Into idle mode: [E_SOS_BUTTON_STATE_IDLE]\r\n");
                    SosButtonState = E_SOS_BUTTON_STATE_IDLE;
                    memset( (uint8_t *)&g_SosButtonClickMsg, 0x00, sizeof( SosButtonClickMsg_t ));
                }
            }
        }
    }
    else if(SosButtonState == E_SOS_BUTTON_STATE_CANCELLED )
    {
        if( EcallHalGetSosButtonStatus() == 0 )
        {
            TBOX_PRINT("SOS button is released, Into idle mode: [E_SOS_BUTTON_STATE_IDLE]\r\n");
            SosButtonState = E_SOS_BUTTON_STATE_IDLE;
            memset( (uint8_t *)&g_SosButtonClickMsg, 0x00, sizeof( SosButtonClickMsg_t ));
        }
    }
}
#if(0)
/** ****************************************************************************
* @remarks       static uint32_t HardwareSelfcheckResult( void )
* @brief         系统硬件状态自检
* @param[in]     无
* @param[out]    无
* @return        pdPASS 成功，pdFAIL 失败；
* @attention     无
*******************************************************************************/
static uint32_t HardwareSelfcheckResult( void )
{
    uint32_t ret = pdPASS;
    LocationInfoSync_t localtionInfo;
    NetInfoSync_t netInfo;

    /*MPU模块运行状态*/
    if(MpuPowerSyncSdkGetNadModuleStatus() == 0)
    {
        AlarmSdkSetMcuSelfcheckResult(E_NAD_FAULT, 1);         /*MPU 模块运行正常*/
    }
    else
    {
        AlarmSdkSetMcuSelfcheckResult(E_NAD_FAULT, 0);         /*MPU 模块运行异常*/
        ret = pdFAIL;
    }
    /*备用电池运行状态*/
    if(BatterySdkGetBatteyStatus() == 1)
    {
        AlarmSdkSetMcuSelfcheckResult(E_BATTERY_FAULT, 0);     /*备用电池运行异常*/
        ret = pdFAIL;
    }
    else if(BatterySdkGetBatteyVoltageError() != 0)
    {
        AlarmSdkSetMcuSelfcheckResult(E_BATTERY_FAULT, 2);     /*备用电池运行异常*/
        ret = pdFAIL;
    }
    else
    {
        AlarmSdkSetMcuSelfcheckResult(E_BATTERY_FAULT, 1);     /*备用电池运行正常*/
    }
    /*气囊传感器连接状态*/
#if(0)
    if( EcuNodeMissingStatus(TBOX_CAN_CHANNEL_1, 0x310) == 0 )
#else
    if( EcuNodeMissingStatus(TBOX_CAN_CHANNEL_2, 0x310) == 0 )
#endif
    {
        AlarmSdkSetMcuSelfcheckResult(E_ABM_FAULT, 1);        /*气囊连接状态正常*/
    }
    else
    {
        AlarmSdkSetMcuSelfcheckResult(E_ABM_FAULT, 0);        /*气囊连接状态异常*/
        ret = pdFAIL;
    }
    /*CAN连接状态*/
    if(OsekNmSdkGetBusState() == 0)
    {
        AlarmSdkSetMcuSelfcheckResult(E_CAN_FAULT, 1);         /*CAN连接正常*/
    }
    else
    {
        AlarmSdkSetMcuSelfcheckResult(E_CAN_FAULT, 0);         /*CAN连接异常*/
        ret = pdFAIL;
    }
    /*SOS按键状态*/
#if 0   // BTN_CON 启用
    if( EcallHalGetSosButtonConnectStatus() == 1 )        /*按键安装*/
    {
        if((EcallHalGetSosButtonStatus() != 2) && (g_SosButtonClickMsg.hardFault == 0))
        {
            AlarmSdkSetMcuSelfcheckResult(E_SOS_FAULT, 1);         /*按键正常*/
        }
        else
        {
            AlarmSdkSetMcuSelfcheckResult(E_SOS_FAULT, 0);         /*按键异常*/
            ret = pdFAIL;
        }
    }
#else
    if((EcallHalGetSosButtonStatus() != 2) && (g_SosButtonClickMsg.hardFault == 0))
    {
        AlarmSdkSetMcuSelfcheckResult(E_SOS_FAULT, 1);         /*按键正常*/
    }
#endif
    else
    {
        AlarmSdkSetMcuSelfcheckResult(E_SOS_FAULT, 0);         /*按键异常*/
        ret = pdFAIL;
    }
    /*MIC状态检测*/
    if( EcallHalGetMicConnectStatus() == 0 )
    {
        AlarmSdkSetMcuSelfcheckResult(E_MIC_FAULT, 1);     // MIC连接正常
    }
    else
    {
        AlarmSdkSetMcuSelfcheckResult(E_MIC_FAULT, 0);     // MIC连接异常
        ret = pdFAIL;
    }
    if(Kl30VoltageState() == 0)
    {
        AlarmSdkSetMcuSelfcheckResult(E_KL30_FAULT, 1);              /*外部电源电压正常*/
    }
    else 
    {
        AlarmSdkSetMcuSelfcheckResult(E_KL30_FAULT, 0);              /*外部电源电压超压或者欠压*/
        ret = pdFAIL;
    }
    /*填充硬件检测状态*****************************************/
    if( ret == pdFAIL )
    {
        AlarmSdkSetMcuSelfcheckResult(E_SELF_TEST_STATE, 0x02);              /*设备自检异常*/
    }
    else
    {
        AlarmSdkSetMcuSelfcheckResult(E_SELF_TEST_STATE, 0x01);              /*设备自检正常*/
    }

#if 1
    /*扬声器连接异常*/
    // if( EcallHalGetAmpFaultStatus() == 0 )
    if( (EcallHalGetAmpDiagnosticStatus() & 0x0F) == 0 )
    {
        AlarmSdkSetMcuSelfcheckResult(E_SPEAKER_FAULT, 0x01);
    }
    else
    {
        AlarmSdkSetMcuSelfcheckResult(E_SPEAKER_FAULT, 0x00);
        ret = pdFAIL;
    }
#endif
    /*GNSS天线连接异常*/
    if(StateSyncGetLocationInfo(&localtionInfo) == 0)
    {
        // 0: 正常 1：电线断路 2：电线地短路 3：电线电源短路
        if(localtionInfo.wireState == 0)
        {
            AlarmSdkSetMpuSelfcheckResult(E_GPS_ANT_FAULT, 0x01);
        }
        else
        {
            AlarmSdkSetMpuSelfcheckResult(E_GPS_ANT_FAULT, localtionInfo.wireState + 1);
            ret = pdFAIL;
        }
    }
    
    if(StateSyncGetNetInfo(&netInfo) == 0)
    { 
        /*SIM卡状态检测异常*/ 
        // if(netInfo.simCardStatus != 1)
        // {
        //     AlarmSdkSetMpuSelfcheckResult(E_SIM_CARD_FAULT, netInfo.simCardStatus);
        //     ret = pdFAIL;
        // }
        /*4G主天线连接异常*/
        if(netInfo.mainAntennaStatus == 1)   // 1正常 2短路 3断路
        {
            AlarmSdkSetMpuSelfcheckResult(E_GSM_MAIN_ANT_FAULT, 0x01);
        }
        else
        {
            AlarmSdkSetMpuSelfcheckResult(E_GSM_MAIN_ANT_FAULT, netInfo.mainAntennaStatus);
            ret = pdFAIL;
        }
        // /*4G辅助天线*/
        // if(netInfo.slaveAntennaStatus == 1)
        // { 
        //     AlarmSdkSetMpuSelfcheckResult(E_GSM_SEC_ANT_FAULT, 0x01);
        // }
        // else
        // { 
        //     AlarmSdkSetMpuSelfcheckResult(E_GSM_SEC_ANT_FAULT, netInfo.slaveAntennaStatus);
        //     ret = pdFAIL;
        // }
    }

    if( selfCheckStatusPrintFlag == 0 )
    {
        McuSelfcheckMsg_t* res = NULL;
        MpuSelfcheckMsg_t* res1 = NULL;

        res = AlarmSdkGetMcuSelfcheckResult();
        res1 = AlarmSdkGetMpuSelfcheckResult();
        TBOX_PRINT("check state:%02x, check result:%08x, %08x\r\n", res->self_test, res->fault_st, res1->fault_st);
        selfCheckStatusPrintFlag = 1;  //信息打印完毕
    }

    return ret;
}

/** ****************************************************************************
* @remarks       static void SelfcheckCycleProcess( void )
* @brief         检测硬件系统的状态
* @param[in]     无
* @param[out]    无
* @return        无
* @attention     无
*******************************************************************************/
static void SelfcheckCycleProcess( void )
{
    static uint32_t startTime = 0;
    static uint32_t currTime = 0;

    /*设备自检开始******************************************************/
    switch( AlarmSdkGetSelfcheckState() )
    {
        case E_SELFCHECK_RUN_INIT:
            selfCheckStatusPrintFlag = 0;
            AlarmSdkSetMcuSelfcheckResult(E_MCU_FAULT_STATUS, 0x00);
            AlarmSdkSetMpuSelfcheckResult(E_MPU_FAULT_STATUS, 0x00);
            startTime = (uint32_t)xTaskGetTickCount();//RTOS_HalGetTickCount();	
            EcallHalSosLedControlSend(  E_SOS_LED_STATE_SELFCHECK_ON ); 
            AlarmSdkSetSelfcheckState(E_SELFCHECK_RUN_START);
            TBOX_PRINT("enter E_SELFCHECK_RUN_START\r\n" );
            break;
        case E_SELFCHECK_RUN_START:
            currTime = (uint32_t)xTaskGetTickCount();
            if(( currTime - startTime ) >= SYSTEM_CHECK_RUN_TIME )
            {
                AlarmSdkSetSelfcheckState(E_SELFCHECK_RUN_STOP);
                TBOX_PRINT("timeout enter E_SELFCHECK_RUN_STOP\r\n" );
            }
            else
            {   /*MPU检测时间比较长，当MPU检测成功时快速结束自检*/
                if(MpuPowerSyncSdkGetNadModuleStatus() == 0)
                {
                    AlarmSdkSetSelfcheckState(E_SELFCHECK_RUN_STOP);
                    TBOX_PRINT("recv mpu heartbeat enter E_SELFCHECK_RUN_STOP\r\n" );
                }
            }
            break;
        case E_SELFCHECK_RUN_STOP:
            if( GetSosLedState() == E_SOS_LED_STATE_SELFCHECK_ON )              /*设备停止自检*/
            {   
                EcallHalSosLedControlSend( E_SOS_LED_STATE_END );
            }
            AlarmSdkSetSelfcheckState(E_SELFCHECK_RUN_END);
            TBOX_PRINT("enter E_SELFCHECK_RUN_END\r\n" );
            break;
        case E_SELFCHECK_RUN_END:
            if( HardwareSelfcheckResult() == pdPASS )                    /*系统硬件状态检测成功*/
            {      
                if(( GetSosLedState() == E_SOS_LED_STATE_SELFCHECK_ON ) || ( GetSosLedState() == E_SOS_LED_STATE_SELFCHECK_ERR ))
                {   
                    EcallHalSosLedControlSend( E_SOS_LED_STATE_END );
                }         
            }
            else
            {
                if( GetSosLedState() == E_SOS_LED_STATE_END )
                {
                    EcallHalSosLedControlSend( E_SOS_LED_STATE_SELFCHECK_ERR );
                }
            }
            break;
        default:
            break;
    }
}
#endif

/** ****************************************************************************
* @remarks       void AirbagSingleProcess(void)
* @brief         气囊信号处理函数
* @param[in]     无
* @param[out]    无
* @return        无
* @attention     处理CAN总线和硬件PWM两种气囊信号，检测到碰撞信号时触发ECALL
*******************************************************************************/
static void AirbagSingleProcess(void)
{
    double dataVaule = 0;
    uint8_t airbagSingal = 0;
    AirbagPwmState_e airBagState = AIRBAG_PWM_UNKNOWN;
    static uint8_t airbagCanFlag = 0;
    static uint8_t airbagHardwareFlag = 0;
    #if 0
    const can0_signal_configure_t *pCan0SignalConfigure = NULL;    
    //const can1_signal_configure_t *pCan1SignalConfigure = NULL;   
    pCan0SignalConfigure = GetCan0SignalConfigure();
    CanParseSdkReadSignal(g_CanSignalFormat,&pCan0SignalConfigure->SRS_CrashOutputSt,&dataVaule);  
    #endif 
    airBagState = AirbagPwmGetState();

    airbagSingal = (uint8_t)dataVaule;
    if(airbagSingal != 0)
    {
        if(airbagCanFlag == 0)
        {
            AlarmSdkEcallTriger(E_ECALL_TRIGGER_CAN_AUTO);
            EcallHalSetVehicleMute(1);
            TBOX_PRINT("Airbag can triggers ECALL, signal = %02x\r\n", airbagSingal);
            airbagCanFlag = 1;
        }
    }
    else
    {
        if(airbagCanFlag == 1)
        {
            airbagCanFlag = 0;
        }
    }

    if(airBagState == AIRBAG_PWM_CRASH)
    {
        if(airbagHardwareFlag == 0)
        {
          AlarmSdkEcallTriger(E_ECALL_TRIGGER_SRS_AUTO);
          EcallHalSetVehicleMute(1);
          TBOX_PRINT("Airbag srs triggers ECALL, signal = %02x\r\n", airbagSingal);
          airbagHardwareFlag = 1;
        }
      }
    else if(airBagState == AIRBAG_PWM_NORMAL)
    {
        airbagHardwareFlag = 0;
    }
}

/** ****************************************************************************
* @remarks       static void BcallSignalProcess(void)
* @brief         B-Call信号处理函数
* @param[in]     无
* @param[out]    无
* @return        无
* @attention     从CAN总线读取B-Call请求信号，在未触发ECALL的情况下触发B-Call
*******************************************************************************/
static void BcallSignalProcess(void)
{
    double dataVaule = 0;
    uint8_t bcallSingal = 0;
    static uint8_t bcallCanFlag = 0;
#if 0
    const can0_signal_configure_t *pCan0SignalConfigure = NULL;    
    pCan0SignalConfigure = GetCan0SignalConfigure();
    CanParseSdkReadSignal(g_CanSignalFormat,&pCan0SignalConfigure->ACU_TELBcallReq,&dataVaule);   
#endif
    bcallSingal = (uint8_t)dataVaule;
    if(bcallSingal == 1)
    {
        if((bcallCanFlag == 0)&&(AlarmSdkGetEcallCallState() == 0)&&(AlarmSdkGetBcallCallState() == 0))
        {
            AlarmSdkBcallTriger(E_ECALL_TRIGGER_CAN_AUTO);
            EcallHalSetVehicleMute(1);
            bcallCanFlag = 1;
            TBOX_PRINT("Bcall can triggers BCALL, signal = %02x\r\n", bcallSingal);
        }
        else if((bcallCanFlag == 0)&&(AlarmSdkGetEcallCallState() != 0))
        {
            TBOX_PRINT("Ecall still run,bcall is forbiden\r\n");
        }
    }
    else
    {
        if(bcallCanFlag == 1)
        {
            bcallCanFlag = 0;
        }
    }
}

/** ****************************************************************************
* @remarks       static void XCallCloseSignalProcess(void)
* @brief         eCall/bCall关闭信号处理函数
* @param[in]     无
* @param[out]    无
* @return        无
* @attention     从CAN总线读取ACU_Key2St信号，用于关闭正在进行的eCall或bCall
*******************************************************************************/
static void XCallCloseSignalProcess(void)
{
    double dataVaule = 0;
    uint8_t acuKeySingal = 0U;
    static uint8_t acuCanFlag = 0U;
    uint8_t ecallStatus = 0U;
    uint8_t bcallStatus = 0U;
    uint8_t triggerType = 0U;
#if 0
    const can0_signal_configure_t *pCan0SignalConfigure = NULL;    
    pCan0SignalConfigure = GetCan0SignalConfigure();
    CanParseSdkReadSignal(g_CanSignalFormat,&pCan0SignalConfigure->ACU_Key2St,&dataVaule);  
#endif
    ecallStatus = AlarmSdkGetEcallCallState();
    bcallStatus = AlarmSdkGetBcallCallState(); 
    acuKeySingal = (uint8_t)dataVaule;
    if(acuKeySingal == 1)
    {
        if(acuCanFlag == 0)
        {
            if(ecallStatus != 0)
            {
                triggerType = AlarmSdkGetEcallTriggerType();
                AlarmSdkEcallClose(triggerType -1);    
                acuCanFlag = 1;
            }
            else if(bcallStatus != 0)
            {
                triggerType = AlarmSdkGetBcallTriggerType();
                AlarmSdkBcallClose(triggerType -1);    
                acuCanFlag = 1;
            }
        }
    }
    else
    {
        if(acuCanFlag == 1)
        {
            acuCanFlag = 0;
        }
    }
}

/** ****************************************************************************
* @remarks       void TaskEcallProcess( void *pvParameters )
* @brief         ECALL功能主任务函数
* @param[in]     pvParameters - FreeRTOS任务参数
* @param[out]    无
* @return        无
* @attention     周期性处理SOS按键、LED控制、ECALL、气囊信号和B-Call信号等功能
*******************************************************************************/
void TaskEcallProcess( void *pvParameters )
{
    uint16_t cycleTimeCount = 0;
    AlarmSdkInit();
    AirbagPwmInit();
    SetSosLedState(E_SOS_LED_STATE_INIT);
    AlarmSdkSetSelfcheckState(E_SELFCHECK_RUN_INIT);
    memset( (uint8_t *)&g_SosButtonClickMsg, 0x00, sizeof( SosButtonClickMsg_t ));

    while(1)
    {
        /* 按键处理 */
        SosButtonDetection();
        /* 顶灯处理 */
        SosLedControlProcess();
        /* ecall */
        AlarmSdkCycleProcess();
        /* 气囊信号处理 */
        AirbagSingleProcess();
        /* B-Call信号处理 */
        BcallSignalProcess();
        /* eCall/bCall关闭信号处理 */
        XCallCloseSignalProcess();

        cycleTimeCount++;
        if(cycleTimeCount == (100 / ECALL_PROCESS_CYCLE_TIME))
        {
            if(AlarmSdkGetEcallTriggerType() == 0)
            {
                //EcallHalRestartAmpDiagnostic();
            }
        }
        if(cycleTimeCount >= (1000 / ECALL_PROCESS_CYCLE_TIME))
        {
            cycleTimeCount = 0;
            EcallHalGetAmpFaultStatus();
            EcallHalGetAmpDiagnosticStatus();
    
            /* 上电自检 */
            //SelfcheckCycleProcess();
            AlarmSdkSelfchackPeriSend();
            //EcallHalRestartAmpClose();
        }
        vTaskDelay(ECALL_PROCESS_CYCLE_TIME);
    }
}

/** ****************************************************************************
 * @remarks   初始化气囊PWM信号检测相关变量
 * @brief     初始化气囊PWM信号状态跟踪结构体中的所有成员变量
 * @param     无
 * @return    无
 * @attention 此函数应在系统初始化时调用，确保气囊PWM信号检测的初始状态正确
 * ****************************************************************************/
static void AirbagPwmInit(void)
{
    g_pwm.lastTick  = 0;
    g_pwm.lastHigh  = 0;
    g_pwm.lastLow   = 0;
    g_pwm.okCount[0] = 0;
    g_pwm.okCount[1] = 0;
    g_pwm.stableState = AIRBAG_PWM_UNKNOWN;
    g_pwm.lastLevel = 0;
}

/** ****************************************************************************
 * @remarks   气囊PWM信号中断处理函数
 * @brief     处理气囊PWM信号的电平变化中断，记录高低电平时间并判断PWM状态
 * @param     level - 当前PWM信号的电平状态(0表示低电平，非0表示高电平)
 * @return    无
 * @attention 此函数在中断上下文中执行，应保持轻量级以避免长时间占用CPU
 * ****************************************************************************/
void AirbagPwmIsrHandler(uint8_t level)
{
    uint32_t now;
    uint32_t dt;

    /* 只有电平变化时才视为真正的边沿 */
    if (level != g_pwm.lastLevel)
    {
        now = xTaskGetTickCountFromISR();
        //now = xTaskGetTickCount();
        dt  = now - g_pwm.lastTick;
        g_pwm.lastTick = now;

        if (level != 0u)
        {
            /* 上升沿：低电平结束 → 记录低电平时间 */
            g_pwm.lastLow = (uint16_t)dt;
        }
        else
        {
            /* 下降沿：高电平结束 → 记录高电平时间 */
            g_pwm.lastHigh = (uint16_t)dt;
        }

        g_pwm.lastLevel = level;

        /* 必须已经采到一段高、一段低，才有完整一个周期 */
        if ((g_pwm.lastHigh > 0u) && (g_pwm.lastLow > 0u))
        {
            uint8_t i;

            for (i = 0u; i < 2u; i++)
            {
                const PwmRule_t *r = &g_pwmRules[i];

                if ((g_pwm.lastHigh >= r->minHigh) && (g_pwm.lastHigh <= r->maxHigh) &&
                    (g_pwm.lastLow  >= r->minLow)  && (g_pwm.lastLow  <= r->maxLow))
                {
                    if (g_pwm.okCount[i] < 255u)
                    {
                        g_pwm.okCount[i]++;
                    }
                    else
                    {
                        /* already at max, do nothing */
                    }
                }
                else
                {
                    g_pwm.okCount[i] = 0u;
                }
            }

            if (g_pwm.okCount[RULE_CRASH] >= g_pwmRules[RULE_CRASH].needCycles)
            {
                g_pwm.stableState = AIRBAG_PWM_NORMAL;
            }
            else if (g_pwm.okCount[RULE_NORMAL] >= g_pwmRules[RULE_NORMAL].needCycles)
            {
                g_pwm.stableState = AIRBAG_PWM_CRASH;
            }
            else
            {
                g_pwm.stableState = AIRBAG_PWM_UNKNOWN;
            }
        }
        else
        {
            /* 尚未获得完整一个高低周期：不做判定 */
        }
    }
    else
    {
        /* 同一电平的重复中断：忽略 */
    }
}

/** ****************************************************************************
 * @remarks   获取气囊PWM信号状态
 * @brief     返回当前检测到的气囊PWM信号稳定状态
 * @param     无
 * @return    当前气囊PWM信号状态枚举值
 * @attention 此函数仅读取状态，不修改任何变量
 ****************************************************************************/
static AirbagPwmState_e AirbagPwmGetState(void)
{
    return g_pwm.stableState;
}

/** ****************************************************************************
 * @remarks   设置远程通信模式
 * @brief     更新全局远程通信模式状态变量
 * @param     mode - 要设置的远程通信模式枚举值
 * @return    无
 * @attention 此函数修改全局状态，可能需要考虑线程安全性
 * ****************************************************************************/
void XCallSetTelemataticsMode(TelematicsMode_e mode)
{
    g_telemataticsMode = mode;
}

/** ****************************************************************************
 * @remarks   获取远程通信模式
 * @brief     返回当前全局远程通信模式状态
 * @param     无
 * @return    当前的远程通信模式枚举值
 * @attention 此函数仅返回状态，不修改任何全局变量
 * ****************************************************************************/
TelematicsMode_e XCallGetTelemataticsMode(void)
{
    return g_telemataticsMode;
}

/** ****************************************************************************
 * @remarks   设置电话通话状态
 * @brief     更新全局电话通话状态变量
 * @param     state - 要设置的电话通话状态枚举值
 * @return    无
 * @attention 此函数修改全局状态，可能需要考虑线程安全性
 * ****************************************************************************/
void XCallSetPhoneCallState(PhoneCallState_e state)
{
    g_PhoneCallState = state;
}

/** ****************************************************************************
 * @remarks   获取电话通话状态
 * @brief     返回当前全局电话通话状态
 * @param     无
 * @return    当前的电话通话状态枚举值
 * @attention 此函数仅返回状态，不修改任何全局变量
 * ****************************************************************************/
PhoneCallState_e XCallGetPhoneCallState(void)
{
    return g_PhoneCallState;
}
