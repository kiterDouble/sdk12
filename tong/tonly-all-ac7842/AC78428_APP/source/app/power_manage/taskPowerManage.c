/*************************************************
 Copyright © 2025 SiRun (AnHui). All rights reserved.
 File Name: taskPowerManage.c
 Author:
 Created Time:
 Description:
 Others:
 *************************************************/
/****************************** include ***************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "logHal.h"
#include "timerHal.h"
#include "peripheralHal.h"
#include "mpuHal.h"
#include "powerManageHal.h"
// #include "bleHal.h"
#include "batterySdk.h"
#include "nmConfig.h"
#if (NM_PROTOCOL_TYPE == NM_PROTOCOL_OSEK)
#include "osekNmSdk.h"
#else
#include "autosarNmSdk.h"
#endif
#include "powerManageSdk.h"
#include "taskPowerManage.h"
#include "taskDtcProcess.h"
#include "canPeriodTask.h"

/* J1939 Protocol */
#include "j1939Sdk.h"
#include "j1939App.h"

#include <stdbool.h>

/****************************** Macro Definitions ******************************/

/* LED Heartbeat Blink Parameters - 2 seconds cycle */
#define LED_ON_TIME         20      /* 100ms / 5ms cycle = 20 counts */
#define LED_OFF_SHORT       80      /* 400ms / 5ms cycle = 80 counts */
#define LED_OFF_LONG        280     /* 1400ms / 5ms cycle = 280 counts */

/****************************** Type Definitions ******************************/
typedef enum
{
    E_KL30_DETECT_NORMAL,
    E_KL30_DETECT_HIGH,
    E_KL30_DETECT_LOW,
} kl30DetectState_e;

typedef enum
{
    E_VOLTAGE_STATUS_NORMAL = 0,    // Normal voltage status
    E_VOLTAGE_STATUS_GENERAL_ERROR, // General error status
    E_VOLTAGE_STATUS_CRITICAL_ERROR // Critical error status
} VoltageStatus_e;

/* LED Heartbeat State Machine */
typedef enum
{
    LED_STATE_ON_1,        /* First ON period (100ms) */
    LED_STATE_OFF_SHORT,   /* Short OFF period (400ms) */
    LED_STATE_ON_2,        /* Second ON period (100ms) */
    LED_STATE_OFF_LONG     /* Long OFF period (1400ms) */
} LedHeartbeatState_e;
/****************************** Function Declarations *************************/
static void Can1BusErrorEvent(uint8_t flag);
// static void LedHeartbeatProcess(void);

/****************************** Global Variables ******************************/
static uint8_t g_lastMcuWakeupSource = PM_HAL_WAKEUP_SOURCE_NONE; // 存储最近一次的唤醒源 0xB245_cxl
const BatteryConfigure_t g_batterConfig =
    {
        .tempHighErrorValue = 85, // ℃
        .tempHighAlarmValue = 70,
        .voltageHighErrorValue = 5000, // mv
        .voltageLowErrorValue = 200,
        .batterySocAlarmValue = 30, // 电池电量%
        .batteryTemperatureHighErrorFun = NULL,
        .batteryNoPlugInErrorFun = NULL,
        .batteryVoltageHighErrorFun = NULL,
        .batteryVoltageLowErrorFun = NULL,
        .batteryShortErrorFun = NULL,
};
#if (NM_PROTOCOL_TYPE == NM_PROTOCOL_OSEK)
static void Can1LimpHomeEvent(uint8_t flag)
{
  TBOX_PRINT("can limphome event flag is %d\r\n",flag);
	if(flag)
	{
		//SetDtcFaultState(E_DTC_ITEM_PCAN_LIMPHOME);
	}
	else
	{
		//ClearDtcFaultState(E_DTC_ITEM_PCAN_LIMPHOME);
	}
}

//ECAN
static void Can2LimpHomeEvent(uint8_t flag)
{
  TBOX_PRINT("can limphome event flag is %d\r\n",flag);
	if(flag)
	{
		//SetDtcFaultState(E_DTC_ITEM_ECAN_LIMPHOME);
	}
	else
	{
		//ClearDtcFaultState(E_DTC_ITEM_ECAN_LIMPHOME);
	}
}
static void Can2BusErrorEvent(uint8_t flag)
{
  //TBOX_PRINT("can bus event flag is %d\r\n",flag);
	if(flag)
	{
		SetDtcFaultState(E_DTC_ITEM_ECAN_BUSOFF);
	}
	else
	{
		ClearDtcFaultState(E_DTC_ITEM_ECAN_BUSOFF);
	}
}

static const struc_netManageOsekConfigure m_netManageOsekConfigure[] = 
{
  {
    .canChannel = TBOX_CAN_CHANNEL_1,//PCAN
    .netManageBaseAddress = 0x18FFF000,
    .netManageNodeIdMax = 0x7FF,
    .netManageNodeId = 0xEB,
    .netManageTtype = 200,
    .netManageTmax = 260,
    .netManageTerror = 1000,
    .netManageTwaitBusSleep = 5000,
    .netManageTtx = 10,
    .rx_limit = 4,
    .tx_limit = 8,  
    .busOffQuickTime = 0,
    .busOffSlowTime = 200,
    .busOffQuickTimes = 0,    
    .limpHomeEventCallBack = Can1LimpHomeEvent,
    .busOffErrorEventLimitCount = 5,
    .canBusErrorCallBack = Can1BusErrorEvent,
  },
	{
    .canChannel = TBOX_CAN_CHANNEL_2,//ECAN
    .netManageBaseAddress = 0x18FFF000,
    .netManageNodeIdMax = 0x7FF,
    .netManageNodeId = 0xEB,
    .netManageTtype = 200,
    .netManageTmax = 260,
    .netManageTerror = 1000,
    .netManageTwaitBusSleep = 5000,
    .netManageTtx = 10,
    .rx_limit = 4,
    .tx_limit = 8,  
    .busOffQuickTime = 0,
    .busOffSlowTime = 200,
    .busOffQuickTimes = 0,    
    .limpHomeEventCallBack = Can2LimpHomeEvent,
    .busOffErrorEventLimitCount = 5,
    .canBusErrorCallBack = Can2BusErrorEvent,
  },
};
#else
static const AutosarNmParameter_t g_netManageAutosarConfigure[] =
    {
        {
            .canChannel = TBOX_CAN_CHANNEL_1,
            .NodeId = 0xEB,
            .BaseAddress = 0x18FFF000,        // SQ CAN network management messages address
            .NodeIdMin = 0x00,               // Minimum node value for CAN network management messages
            .NodeIdMax = 0xFF,               // Maximum node value for CAN network management messages
            .startUpTime = 1000,             // Maximum time the node stays in RMS (Start Up State) mode. T_REPEAT_MESSAGE unit: ms
            .nmTimeOutTime = 1000,           // Maximum time the node stays in NM (Network Mode) state. T_NM_TIMEOUT unit: ms.
            .waitBusSleepTime = 5000 - 80,  // Ensure all nodes have time to stop their network activities. T_WAIT_BUS_SLEEP unit: ms
            .startTXTime = 10,               // Maximum time interval from leaving BSM (Bus Sleep Mode) to entering RMS (Repeat Message State) mode and starting to send the first NM message
            .startxAppFrameTime = 20,        // Maximum time interval to start sending application frames after successfully sending the first NM message
            .immediateCycleTime = 20,        // Period for sending NM messages rapidly in the sub-state mode, unit: ms
            .msgCycleTime = 200 - 8,             // Normal cycle time for sending network management messages in the sub-state or regular operation mode
            .wakeupTime = 100,               // Maximum time to start sending network management messages repeatedly after transitioning from sleep mode to network mode
            .immediateTimes = 10,            // Number of network management messages sent at immediateCycleTime in the sub-state mode
            .busOffQuickTime = 0,            // Time for quick recovery after bus-off in milliseconds
            .busOffSlowTime = 180,             // Time for slow recovery after bus-off in milliseconds
            .busOffQuickTimes = 0,           // Number of quick recovery attempts after bus-off
            .busOffErrorEventLimitCount = 2, // Minimum number of bus-off DTC occurrences required
            .canBusErrorCallBack = Can1BusErrorEvent,
        },
};
#endif
static uint8_t g_communicationStartFlag = 0;
static uint8_t g_powerOnState = 0;
static kl30DetectState_e g_voltageComState = E_KL30_DETECT_LOW;
static kl30DetectState_e g_voltageComTargetState = E_KL30_DETECT_LOW; // Target state to transition to
static uint32_t g_voltageStateChangeCount = 0; // Counter for state hold time
static const uint32_t g_voltageStateHoldTime = 20; // 20 cycles = 200ms (since each cycle is 10ms)
static uint32_t g_diagRecoverTimerCount = 0;
static uint8_t g_kl30DtcResetFlag = 0;
static uint32_t g_diagFaultTimerCount = 0;
static const uint16_t g_communicationMaxVoltage = 18000; // mv
static const uint16_t g_communicationMinVoltage = 7000;  // mv
static const uint16_t g_diagCriticalMaxVoltage = 18000;  // mv
static const uint16_t g_diagCriticalMinVoltage = 7000;   // mv
static const uint16_t g_diagMaxVoltage = 16000;          // mv
static const uint16_t g_diagMinVoltage = 9000;           // mv
static const uint32_t g_diagFaultTime = 2000;             // ms

/* LED Heartbeat State Variables */
// static LedHeartbeatState_e g_ledHeartbeatState = LED_STATE_OFF_LONG;
// static uint16_t g_ledHeartbeatCount = 0;

/****************************** Public Function Implementations ***************/

/****************************** Private Function Implementations ***************/
static void PowerWakeUpProcess(uint8_t mcuWakeUpSource, uint8_t cpuWakeUpSource, uint32_t *pDelayTime_ms)
{
    // uint8_t MapWakeUpSource = 0;
    g_lastMcuWakeupSource = mcuWakeUpSource; // 记录`唤醒源 0xB245_cxl
    if ((mcuWakeUpSource >= PM_HAL_WAKEUP_SOURCE_CAN1) && (mcuWakeUpSource <= PM_HAL_WAKEUP_SOURCE_CAN6))
    {
        *pDelayTime_ms = 0;
        // MapWakeUpSource = mcuWakeUpSource;
    }
    else if (mcuWakeUpSource == PM_HAL_WAKEUP_SOURCE_MPU)
    {
        if (cpuWakeUpSource == 0x02)
        {
            // MapWakeUpSource = MAP_WAKE_UP_SOURCE_CPU_TCP;
            *pDelayTime_ms = 45 * 1000; //
        }
        else if (cpuWakeUpSource == 0x08)
        {
            // MapWakeUpSource = MAP_WAKE_UP_SOURCE_CPU_RTC;
            *pDelayTime_ms = 45 * 1000; //
        }
        else
        {
            // MapWakeUpSource = MAP_WAKE_UP_SOURCE_NONE;
            *pDelayTime_ms = 45 * 1000; // 20200702 WHL
        }
    }
    else if (mcuWakeUpSource == PM_HAL_WAKEUP_SOURCE_KL15)
    {
        *pDelayTime_ms = 0;
        // MapWakeUpSource = MAP_WAKE_UP_SOURCE_MCU_KL15;
    }
    else if (mcuWakeUpSource == PM_HAL_WAKEUP_SOURCE_BLE)
    {
        *pDelayTime_ms = 45 * 1000;
        // MapWakeUpSource = MAP_WAKE_UP_SOURCE_CPU_TCP;
    }
    else if (mcuWakeUpSource == PM_HAL_WAKEUP_SOURCE_KL30)
    {
        *pDelayTime_ms = 2 * 60 * 1000;
        // MapWakeUpSource = MAP_WAKE_UP_SOURCE_MCU_KL30;
    }
    else
    {
        *pDelayTime_ms = 0;
        // MapWakeUpSource = MAP_WAKE_UP_SOURCE_NONE;
    }
}

const PmSdkConfig_t g_pmCondg =
    {
        .degInfo = 1,
    #if (NM_PROTOCOL_TYPE == NM_PROTOCOL_OSEK)
        .canNmType = 2,
    #else
        .canNmType = 1,
    #endif
        .wakeDelayTime = 20,
        .wakeupFun = PowerWakeUpProcess,
        .kl30OffWakeDelay = (60 * 10),
        .deepSleepConfig =
            {
                .mpuDeepSleep = 1,
                .gSensorDeepSleep = 1,
            },
        .customSleepConfig =
            {
                .mpuDeepSleep = 1,
                .gSensorDeepSleep = 1,
            },
};

/*################################# KL30 detection process ############################################*/
static void StartCommunication(void)
{
    if (0 == g_communicationStartFlag)
    {
        g_communicationStartFlag = 1;
        if (0 == g_powerOnState)
        {
            g_powerOnState = 1;
            PowerManageSdkPowerOn();
        }
        else
        {
            CanHalTxMsgEnable(TBOX_CAN_CHANNEL_1);
        }
        // TBOX_PRINT("CycleSendEnableAll---\r\n");
    }
}

static void StopCommunication(void)
{
    if (g_communicationStartFlag)
    {
        CanHalTxMsgDisable(TBOX_CAN_CHANNEL_1);
        g_communicationStartFlag = 0;
        // TBOX_PRINT("CycleSendDisableAll---\r\n");
    }
}

static void McuVoltageDtcCheckProcess(void)
{
    // uint32_t kl30_voltage_mv = 0;
    uint32_t v_4g_mv = 0;
    uint32_t v_can_mv = 0;
    uint32_t v_battery_mv = 0;
    // uint32_t v_mic_mv = 0;
    uint32_t v_gps_mv = 0;
    uint32_t v_gps1_mv = 0;
    // uint32_t v_bcall_led_mv = 0;
    // uint32_t v_ecall_led_mv = 0;
    // uint32_t v_bcall_adc_mv = 0;
    // uint32_t v_ecall_adc_mv = 0;
    // 1. 读取主电源电压 (KL30)
    // if (PeripheralHalAdGet(AD_CHANNEL_KL30, &kl30_voltage_mv) == 0)
    // {
    //     //   检查 B11001C: 主电源超限
    //     kl30_voltage_mv = kl30_voltage_mv / 11;
    //     if (kl30_voltage_mv < 800 || kl30_voltage_mv > 1480)
    //     {
    //         SetDtcFaultState(E_DTC_ITEM_MAIN_POWER_OVER_RANGE);
    //     }
    //     else
    //     {
    //         ClearDtcFaultState(E_DTC_ITEM_MAIN_POWER_OVER_RANGE);
    //     }

    //     // 检查 B110091: 主电源超极限
    //     if (kl30_voltage_mv < 630 || kl30_voltage_mv > 1670)
    //     {
    //         SetDtcFaultState(E_DTC_ITEM_MAIN_POWER_OVER_MAX_RANGE);
    //     }
    //     else
    //     {
    //         ClearDtcFaultState(E_DTC_ITEM_MAIN_POWER_OVER_MAX_RANGE);
    //     }
    // }
    // 2. 读取4G模块电压
    if (PeripheralHalAdGet(AD_CHANNEL_4G, &v_4g_mv) == 0)
    {
        // TBOX_PRINT("4g voltage is %d mv\r\n", v_4g_mv);
        //  检查 0x190217: 4G 模块电压异常
        if (v_4g_mv < 1800 || v_4g_mv > 2050)
        {
            SetDtcFaultState(E_DTC_ITEM_4G_MODULE_VOLTAGE);
        }
        else
        {
            ClearDtcFaultState(E_DTC_ITEM_4G_MODULE_VOLTAGE);
        }
    }

    // 3. 读取CAN收发器电源电压（通过SYS5V_V_DET判断）
    if (PeripheralHalAdGet(AD_CHANNEL_SYS_5V, &v_can_mv) == 0)
    {
        // TBOX_PRINT("can power voltage is %d mv\r\n", v_can_mv);
        // 检查 190219: CAN 电源电压超限
        if (v_can_mv < 4800 || v_can_mv > 5200)
        {
            SetDtcFaultState(E_DTC_ITEM_CAN_POWER_ABNORMAL);
        }
        else
        {
            ClearDtcFaultState(E_DTC_ITEM_CAN_POWER_ABNORMAL);
        }
    }

    
    // 读取后备电池电压（通过BATTERY_V_DET判断）
    if (PeripheralHalAdGet(AD_CHANNEL_BATTERY, &v_battery_mv) == 0)
    {
        // TBOX_PRINT("battery voltage is %d mv\r\n", v_battery_mv);
        // 检查 190218: 后备电池电量低
        if (v_battery_mv < 3500)
        {
            SetDtcFaultState(E_DTC_ITEM_CAN_POWER_ABNORMAL);
        }
        else
        {
            ClearDtcFaultState(E_DTC_ITEM_CAN_POWER_ABNORMAL);
        }
    }
#if 0
    // 4. 读取麦克风电源电压
    if (PeripheralHalAdGet(AD_CHANNEL_MIC, &v_mic_mv) == 0)
    {
        // TBOX_PRINT("mic voltage is %d mv\r\n", v_mic_mv);
        //  检查 B32031F: 麦克风信号输入故障
        //  正常范围: 10mV < V < 550mV
        if (v_mic_mv <= 10 || v_mic_mv >= 550)
        {
            SetDtcFaultState(E_DTC_ITEM_MPU_MIC_SIGNAL_FAULT);
        }
        else
        {
            ClearDtcFaultState(E_DTC_ITEM_MPU_MIC_SIGNAL_FAULT);
        }
    }
#endif
    // 5. 读取GPS天线电压
    if (PeripheralHalAdGet(AD_CHANNEL_GPS, &v_gps_mv) == 0 &&
        PeripheralHalAdGet(AD_CHANNEL_GPS1, &v_gps1_mv) == 0)
    {
        // TBOX_PRINT("gps voltage is %d mv, gps1 voltage is %d mv\r\n", v_gps_mv, v_gps1_mv);
        // 检查 190317: GPS天线对地短路
        if (v_gps_mv < 100 && v_gps1_mv < 100)
        {
            SetDtcFaultState(E_DTC_ITEM_MPU_GPS_ANTENNA_SHORT_GND);
        }
        else
        {
            ClearDtcFaultState(E_DTC_ITEM_MPU_GPS_ANTENNA_SHORT_GND);
        }

        // 检查 190315: GPS天线开路或对电源短路
        bool isOpen = (v_gps_mv > 700 && v_gps_mv < 1200) && (v_gps1_mv > 700 && v_gps1_mv < 1200);
        bool isShortToBat = (v_gps_mv > 1900 && v_gps1_mv > 1900);

        if (isOpen || isShortToBat)
        {
            // TBOX_PRINT("set E_DTC_ITEM_MPU_GPS_ANTENNA_OPEN_SHORT\r\n");
            SetDtcFaultState(E_DTC_ITEM_MPU_GPS_ANTENNA_OPEN_SHORT);
        }
        else
        {
            ClearDtcFaultState(E_DTC_ITEM_MPU_GPS_ANTENNA_OPEN_SHORT);
        }
    }
#if 0
    // 6. 读取 B-Call LED 电压
    if (PeripheralHalAdGet(AD_CHANNEL_BCALL_LIGHT, &v_bcall_led_mv) == 0)
    {
        // TBOX_PRINT("bcall led voltage is %d mv\r\n", v_bcall_led_mv);
        //  检查 0x190714: B_Call指示灯短路到地
        if (v_bcall_led_mv < 100) // 阈值 0.1V = 100mV
        {
            SetDtcFaultState(E_DTC_ITEM_BCALL_LIGHT_SHORT_GND);
        }
        else
        {
            ClearDtcFaultState(E_DTC_ITEM_BCALL_LIGHT_SHORT_GND);
        }
    }

    // 7. 读取 E-Call LED 电压
    if (PeripheralHalAdGet(AD_CHANNEL_ECALL_LIGHT, &v_ecall_led_mv) == 0)
    {
        if (v_ecall_led_mv < 100)
        {
            SetDtcFaultState(E_DTC_ITEM_ECALL_LIGHT_SHORT_GND);
        }
        else
        {
            ClearDtcFaultState(E_DTC_ITEM_ECALL_LIGHT_SHORT_GND);
        }
    }

    // 8. B-Call 按键电气故障诊断 (DTC B320707)
    if (PeripheralHalAdGet(AD_CHANNEL_BCALL, &v_bcall_adc_mv) == 0)
    {
        if (v_bcall_adc_mv > 2400 || v_bcall_adc_mv < 450)
        {
            SetDtcFaultState(E_DTC_ITEM_MPU_BCALL_KEY_STUCK);
        }
        else
        {
            ClearDtcFaultState(E_DTC_ITEM_MPU_BCALL_KEY_STUCK);
        }
    }

    // 9. E-Call 按键电气故障诊断 (DTC B320807)
    if (PeripheralHalAdGet(AD_CHANNEL_ECALL, &v_ecall_adc_mv) == 0)
    {
        if (v_ecall_adc_mv > 2400 || v_ecall_adc_mv < 450)
        {
            SetDtcFaultState(E_DTC_ITEM_MPU_ECALL_KEY_STUCK);
        }
        else
        {
            ClearDtcFaultState(E_DTC_ITEM_MPU_ECALL_KEY_STUCK);
        }
    }
#endif
    // 10. BLE 天线电压
    //FIXME
}

static void ProcessDtcReset(void)
{
    if (g_kl30DtcResetFlag != 0)
    {
        g_kl30DtcResetFlag = 0;
        g_diagFaultTimerCount = 0;
        // ClearDtcFaultState(E_DTC_ITEM_MAIN_POWER_OVER_RANGE);
        // ClearDtcFaultState(E_DTC_ITEM_MAIN_POWER_OVER_MAX_RANGE);
    }
}

static void KL30DetectDtcProcess(uint32_t kl30Voltage)
{
    static VoltageStatus_e lastVoltageStatus = E_VOLTAGE_STATUS_NORMAL;
    VoltageStatus_e currentVoltageStatus = E_VOLTAGE_STATUS_NORMAL;

    // Determine current voltage status
    if (kl30Voltage >= g_diagMinVoltage && kl30Voltage <= g_diagMaxVoltage) // Normal voltage range (9-16V)
    {
        currentVoltageStatus = E_VOLTAGE_STATUS_NORMAL;
    }
    else if (kl30Voltage < g_diagCriticalMinVoltage || kl30Voltage > g_diagCriticalMaxVoltage) // Critical error
    {
        currentVoltageStatus = E_VOLTAGE_STATUS_CRITICAL_ERROR;
    }
    else // E_VOLTAGE_STATUS_GENERAL_ERROR - General error (7-9V or 16-18V)
    {
        currentVoltageStatus = E_VOLTAGE_STATUS_GENERAL_ERROR;
    }

    // Reset corresponding timers when status changes
    if (currentVoltageStatus != lastVoltageStatus)
    {
        if (currentVoltageStatus == E_VOLTAGE_STATUS_NORMAL) // Enter normal status
        {
            g_diagFaultTimerCount = 0;   // Clear fault timer
            g_diagRecoverTimerCount = 0; // Reset recovery timer and restart timing
        }
        else // Enter error status
        {
            g_diagRecoverTimerCount = 0; // Clear recovery timer
            g_diagFaultTimerCount = 0;   // Reset fault timer to ensure re-timing when switching between different error types
        }
        lastVoltageStatus = currentVoltageStatus;
    }

    // Execute corresponding logic based on current status
    if (currentVoltageStatus == E_VOLTAGE_STATUS_NORMAL) // Normal voltage range (9-16V)
    {
        // Clear DTC after continuous normal operation for 2000ms
        if (g_diagRecoverTimerCount >= (g_diagFaultTime / 10))
        {
            // ClearDtcFaultState(E_DTC_ITEM_CONTROLLER_VOLTAGE_HIGH);
            ClearDtcFaultState(E_DTC_ITEM_KL30_VOLTAGE_HIGH);
            ClearDtcFaultState(E_DTC_ITEM_KL30_VOLTAGE_LOW);
            g_diagRecoverTimerCount = 0; // Reset recovery timer
        }
        else
        {
            g_diagRecoverTimerCount++;
        }
    }
    else if (currentVoltageStatus == E_VOLTAGE_STATUS_CRITICAL_ERROR) // Critical error (<7V or >18V)
    {
        // Increase critical error timer
        g_diagFaultTimerCount++;

        // Set DTC after critical error persists for 2000ms
        if (g_diagFaultTimerCount >= (g_diagFaultTime / 10))
        {
            // Set U300316/U300317 for voltage out of normal range (<9V or >16V)
            if (kl30Voltage < g_diagCriticalMinVoltage) // < 7V
            {
                    SetDtcFaultState(E_DTC_ITEM_KL30_VOLTAGE_LOW);  // U300316
                    ClearDtcFaultState(E_DTC_ITEM_KL30_VOLTAGE_HIGH);
                }
            else if (kl30Voltage > g_diagCriticalMaxVoltage) // > 18V
            {
                    SetDtcFaultState(E_DTC_ITEM_KL30_VOLTAGE_HIGH); // U300317
                    ClearDtcFaultState(E_DTC_ITEM_KL30_VOLTAGE_LOW);
                }
            // Clear general error DTC
            // ClearDtcFaultState(E_DTC_ITEM_CONTROLLER_VOLTAGE_HIGH); // P190017 已屏蔽
        }
    }
    else // General error (7-9V or 16-18V)
    {
        // Increase general error timer
        g_diagFaultTimerCount++;

        // Set DTC after general error persists for 2000ms
        if (g_diagFaultTimerCount >= (g_diagFaultTime / 10))
        {
            // Set U300316/U300317 for voltage out of normal range (<9V or >16V)
            if (kl30Voltage < g_diagMinVoltage) // 7-9V
            {
                    SetDtcFaultState(E_DTC_ITEM_KL30_VOLTAGE_LOW);  // U300316
                    ClearDtcFaultState(E_DTC_ITEM_KL30_VOLTAGE_HIGH);
                }
            else if (kl30Voltage > g_diagMaxVoltage) // 16-18V
            {
                    SetDtcFaultState(E_DTC_ITEM_KL30_VOLTAGE_HIGH); // U300317
                    ClearDtcFaultState(E_DTC_ITEM_KL30_VOLTAGE_LOW);
                    // Also set P190017 for general error range
                    // SetDtcFaultState(E_DTC_ITEM_CONTROLLER_VOLTAGE_HIGH); // P190017 已屏蔽
                }
            }
        }

    // Process DTC reset logic
    ProcessDtcReset();
}

static void KL30DetectCommunicationProcess(uint32_t kl30Voltage)
{
    kl30DetectState_e targetState = g_voltageComState;
    
    // Determine target state based on current voltage
    if (E_KL30_DETECT_NORMAL == g_voltageComState)
    {
        if (kl30Voltage > (g_communicationMaxVoltage))
        {
            targetState = E_KL30_DETECT_HIGH;
        }
        else if (kl30Voltage < (g_communicationMinVoltage))
        {
            targetState = E_KL30_DETECT_LOW;
        }
    }
    else if (E_KL30_DETECT_HIGH == g_voltageComState)
    {
        if (kl30Voltage < g_communicationMaxVoltage) // recover
        {
            targetState = E_KL30_DETECT_NORMAL;
        }
    }
    else if (E_KL30_DETECT_LOW == g_voltageComState)
    {
        if (kl30Voltage > g_communicationMinVoltage) // recover
        {
            targetState = E_KL30_DETECT_NORMAL;
        }
    }
    
    // Check if target state is different from current state
    if (targetState != g_voltageComState)
    {
        // Check if we're already tracking this state change
        if (targetState != g_voltageComTargetState)
        {
            // New state change detected, start counting
            g_voltageComTargetState = targetState;
            g_voltageStateChangeCount = 0;
        }
        else
        {
            // Continue counting for the same state change
            g_voltageStateChangeCount++;
            
            // Check if state has been stable for required time (200ms)
            if (g_voltageStateChangeCount >= g_voltageStateHoldTime)
            {
                // State has been stable long enough, perform state change
                g_voltageComState = targetState;
                
                // Reset counters
                g_voltageStateChangeCount = 0;
                
                // Execute actions based on new state
                if (E_KL30_DETECT_NORMAL == g_voltageComState)
                {
                    // start communication
                    // Nm_AllEnableCommunication();
                    StartCommunication();
                }
                else
                {
                    // to do stop communication
                    // Ng_AllDisableCommunication();
                    StopCommunication();
                }
            }
        }
    }
    else
    {
        // Target state is same as current state, reset counters
        g_voltageStateChangeCount = 0;
        g_voltageComTargetState = targetState;
        
        // If in abnormal state, ensure communication is stopped
        if (E_KL30_DETECT_NORMAL != g_voltageComState)
        {
            StopCommunication();
        }
    }
}

static void Kl30DetectCycleProcess(void)
{
    uint32_t u16BatVol;

    PeripheralHalAdGet(AD_CHANNEL_KL30, &u16BatVol);
    /********communication control ************************************/
    KL30DetectCommunicationProcess(u16BatVol);

    /****************diagnostic voltage detecting ************/
    KL30DetectDtcProcess(u16BatVol);
}

uint8_t Kl30VoltageState(void)
{
    return g_voltageComState;
}

void Kl30DtcCheckReset(void)
{
    g_kl30DtcResetFlag = 1;
}

static void Can1BusErrorEvent(uint8_t flag)
{
    /* if(flag)
     {
         SetBusOffDtc();
     }
     else
     {
         ClearBusOffDtc();
    }*/
    if (flag)
    {
        SetDtcFaultState(E_DTC_ITEM_PCAN_BUSOFF);
    }
    else
    {
        ClearDtcFaultState(E_DTC_ITEM_PCAN_BUSOFF);
    }
}
# if 0
/*************************************************
  Function:       LedHeartbeatProcess
  Description:    LED heartbeat blink control on PTD11
                  Double-flash pattern: ON 100ms -> OFF 400ms -> ON 100ms -> OFF 1400ms (2s cycle)
                  Only active when KL15 is ON and system is in normal operation mode
                  LED is HIGH-active (high level = ON)
  Input:          None
  Output:         None
  Return:         None
  Others:         Called every 5ms from TaskPowerManage
 *************************************************/
static void LedHeartbeatProcess(void)
{
    uint8_t pmState;
    uint8_t wakeupSource;
    uint32_t wakeCount;
    
    /* Get current power management state */
    PowerManageSdkGetPowerInfo(&pmState, &wakeupSource, &wakeCount);
    
    /* Check if KL15 is ON (PeripheralHalGetKl15Status returns 1 when KL15 is ON) 
       and system is in normal operation (not sleeping) */
    if ((PeripheralHalGetKl15Status() == 0) || (pmState == E_PM_STATE_MCU_SLEEP))
    {
        /* KL15 OFF or system sleeping: Turn LED OFF */
        PeripheralHalSetLedState(0);
        g_ledHeartbeatState = LED_STATE_OFF_LONG;
        g_ledHeartbeatCount = 0;
        return;
    }
    
    /* KL15 is ON and system is running: Perform heartbeat blink */
    g_ledHeartbeatCount++;
    
    switch (g_ledHeartbeatState)
    {
        case LED_STATE_ON_1:
            /* First ON period - LED is ON (HIGH) */
            PeripheralHalSetLedState(1);
            if (g_ledHeartbeatCount >= LED_ON_TIME)
            {
                g_ledHeartbeatState = LED_STATE_OFF_SHORT;
                g_ledHeartbeatCount = 0;
            }
            break;
            
        case LED_STATE_OFF_SHORT:
            /* Short OFF period - LED is OFF (LOW) */
            PeripheralHalSetLedState(0);
            if (g_ledHeartbeatCount >= LED_OFF_SHORT)
            {
                g_ledHeartbeatState = LED_STATE_ON_2;
                g_ledHeartbeatCount = 0;
            }
            break;
            
        case LED_STATE_ON_2:
            /* Second ON period - LED is ON (HIGH) */
            PeripheralHalSetLedState(1);
            if (g_ledHeartbeatCount >= LED_ON_TIME)
            {
                g_ledHeartbeatState = LED_STATE_OFF_LONG;
                g_ledHeartbeatCount = 0;
            }
            break;
            
        case LED_STATE_OFF_LONG:
        default:
            /* Long OFF period - LED is OFF (LOW) */
            PeripheralHalSetLedState(0);
            if (g_ledHeartbeatCount >= LED_OFF_LONG)
            {
                g_ledHeartbeatState = LED_STATE_ON_1;
                g_ledHeartbeatCount = 0;
            }
            break;
    }
}
    
#endif
void PmDebugPrint(void)
{
    static uint32_t count = 0;
    uint32_t wakeCount;
    uint8_t pmState, wakeupSource;
    static uint8_t lastPmStat = 0;
    static uint8_t lastWakeupSource = 0;
    static uint32_t lastWakeupCount = 0;

    count++;
    PowerManageSdkGetPowerInfo(&pmState, &wakeupSource, &wakeCount);
    if ((pmState != lastPmStat) || (wakeupSource != lastWakeupSource) || (wakeCount != lastWakeupCount))
    {
        lastPmStat = pmState;
        lastWakeupSource = wakeupSource;
        lastWakeupCount = wakeCount;
        TBOX_PRINT("powerstate is %d,wakesoure is %d,wakecount is %d\r\n", pmState, wakeupSource, wakeCount);
        LogHalUpLoadLog("powerstate is %d,wakesoure is %d,wakecount is %d\r\n", pmState, wakeupSource, wakeCount);
        return;
    }
    else if (count < 200)
    {
        return;
    }
    count = 0;
    TBOX_PRINT("powerstate is %d,wakesoure is %d,wakecount is %d\r\n", pmState, wakeupSource, wakeCount);
    LogHalUpLoadLog("powerstate is %d,wakesoure is %d,wakecount is %d\r\n", pmState, wakeupSource, wakeCount);
    uint32_t voltage = 0;
    BatterySdkGetVoltage(&voltage);

    uint32_t level = BatterySdkGetBatteySoc();
    uint32_t voltageKl30 = 0;
    PeripheralHalAdGet(AD_CHANNEL_KL30, &voltageKl30);

    TBOX_PRINT("KL30: %dmV, battery: %dmV, level %d\r\n", voltageKl30, voltage, level);
    // TBOX_PRINT("KL30 voltage %d",voltageKl30);
    /*TimerHalDebugPrint(&wakeCount);
    TBOX_PRINT("timer hal test %d \r\n",wakeCount);*/

    uint32_t hwVer = 0;
    PeripheralHalAdGet(AD_CHANNEL_HW_VER, &hwVer);
    TBOX_PRINT("hwVer: %dmV\r\n", hwVer);
    
    uint32_t gps1 = 0, gps2 = 0;
    PeripheralHalAdGet(AD_CHANNEL_GPS, &gps1);
    PeripheralHalAdGet(AD_CHANNEL_GPS1, &gps2);
    TBOX_PRINT("gps1: %dmV, gps2: %dmV\r\n", gps1, gps2);

    uint32_t moduleVol = 0;
    PeripheralHalAdGet(AD_CHANNEL_4G, &moduleVol);
    TBOX_PRINT("4G Voltage: %dmV\r\n", moduleVol);

    uint32_t sys5V = 0;
    PeripheralHalAdGet(AD_CHANNEL_SYS_5V, &sys5V);
    TBOX_PRINT("sys5V: %dmV\r\n", sys5V);

    uint32_t ntc = 0;
    PeripheralHalAdGet(AD_CHANNEL_BACKUP_BATTERY_NTC, &ntc);
    TBOX_PRINT("ntc: %d\r\n", ntc);

    PeripheralHalAdGet(AD_CHANNEL_BATTERY, &ntc);
    TBOX_PRINT("backup battery: %dmV\r\n", ntc);
}

void WatchDogCycleProcess(void)
{
    static uint32_t count = 0;
    count++;
    if (count < 10)
    {
        return;
    }
    count = 0;
    PeripheralHalFeedWatchDog();
    // TBOX_PRINT("feedDog");
}

void TaskPowerManage(void *pvParameters)
{
    TickType_t xLastWakeTime;
    uint8_t timeCount;
#if (NM_PROTOCOL_TYPE == NM_PROTOCOL_OSEK)    
    NetManageOsekInit(m_netManageOsekConfigure,sizeof(m_netManageOsekConfigure)/sizeof(m_netManageOsekConfigure[0])); 
#else
    AutosarNmSdkConfig(g_netManageAutosarConfigure, sizeof(g_netManageAutosarConfigure) / sizeof(g_netManageAutosarConfigure[0]));
#endif
    PowerManageSdkInit(&g_pmCondg);
    BatterySdkInit(&g_batterConfig,10,E_BATTERY_XYSR);

#if 0
    /* Initialize J1939 Protocol */
    {
        J1939DeviceConfig_t j1939Config = {
            .sourceAddress = 0x21,    /* Body Control Module */
            .deviceName = 0x0000000000000021ULL,
            .preferredAddress = 0x21,
            .priority = 3
        };
        J1939Sdk_Init(&j1939Config);
        J1939App_Init();
    }
#endif

    //TBOX_PRINT("[OTA_DEBUG] Force MPU reset before start, lastResetCause=0x400\r\n");
    //MpuHalReset();  // PWR_EN = LOW
    //vTaskDelay(pdMS_TO_TICKS(500));
    MpuHalStart();
    xLastWakeTime = xTaskGetTickCount();
    timeCount = 0;

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, 5);
        PowerManageSdkCycleProcess(5);

        //FIXME
        // CanPeriodSendEnable(TBOX_CAN_CHANNEL_1); // 会影响28 03 03禁用报文失效
#if (NM_PROTOCOL_TYPE == NM_PROTOCOL_OSEK)
        NetManageOsekCycleProcess();
#else
        NetManageAutosarCycleProcess();
#endif
        WatchDogCycleProcess();
        // LedHeartbeatProcess();  /* LED heartbeat control - called every 5ms */
        
        /* J1939 Protocol Processing */
        // J1939Sdk_CycleProcess(5);
        // J1939App_CycleProcess();
        
        timeCount++;
        if (2 == timeCount)
        {
            timeCount = 0;
            MpuHalCycleProcess(10);
            BatterySdkCycleProcess();
            Kl30DetectCycleProcess();
            McuVoltageDtcCheckProcess(); // set dtc
            PmDebugPrint();
        }
        xLastWakeTime = xTaskGetTickCount();
    }
}
/*************************************************
  Function:       PowerManageGetLastWakeupSource
  Description:    获取系统最近一次的唤醒源
  Input:          无
  Output:         无
  Return:         唤醒源ID
*************************************************/
uint8_t PowerManageGetLastWakeupSource(void) // 0xB245_cxl
{
    return g_lastMcuWakeupSource;
}
