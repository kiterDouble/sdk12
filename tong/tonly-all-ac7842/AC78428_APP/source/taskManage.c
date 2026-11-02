/*************************************************
 Copyright © 2026 SiRun (AnHui). All rights reserved.
 File Name: taskManage.c
 Author: 
 Created Time: 
 Description: 
 Others: 
 *************************************************/
/****************************** include ***************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "taskManage.h"
#include "batteryHal.h"
#include "canHal.h"
#include "logHal.h"
#include "mpuHal.h"
#include "peripheralHal.h"
#include "timerHal.h"
#include "flashHal.h"
#include "PowerManageHal.h"
#include "ecallHal.h"
#include "mcuMpuSyncTask.h"
#include "remoteDiagnosticTask.h"
#include "taskPowerManage.h"
#include "taskVehicleDataToCpu.h"
#include "taskDiagnostic.h"
#include "taskDtcProcess.h"
#include "taskEcallProcess.h"
#include "canPeriodTask.h"
#include "remoteControl.h"
#include "string.h"
#include "Rcm_Hal.h"

/****************************** Macro Definitions ******************************/
/* TASK_TEST_ENABLE is defined in taskManage.h */
#define LOWEST_TASK_PRIORITY                            (tskIDLE_PRIORITY+1)
#define REMOTE_UPDATE_TASK_PRIORITY                     (tskIDLE_PRIORITY+2)
#define POWERMANAGE_TASK_PRIORITY                       (tskIDLE_PRIORITY+3)
#define MCU_CPU_SYNC_TASK_PRIORITY                      (tskIDLE_PRIORITY+3)
#define UDS_DIG_TASK_PRIORITY                           (tskIDLE_PRIORITY+3)
#define UPLOAD_DATA_TASK_PRIORITY                       (tskIDLE_PRIORITY+3)
#define MONITOR_TASK_PRIORITY	                        (tskIDLE_PRIORITY+3)
#define NET_MANAGE_TASK_PRIORITY                        (tskIDLE_PRIORITY+4)    
#define REMOTE_CONTROL_TASK_PRIORITY                    (tskIDLE_PRIORITY+4)
#define CAN_TX_CYCLE_TASK_PRIORITY                      (tskIDLE_PRIORITY+4)
#define BLUETOOTH_TASK_PRIORITY                         (tskIDLE_PRIORITY+2)
#define DRIVER_TASK_PRIORITY                            (tskIDLE_PRIORITY+6)
#define MPU_UART_TASK_PRIORITY                          (tskIDLE_PRIORITY+7)
#define TASK_TEST_STATIC_SIZE                           (256*1) 
#define TASK_MPU_HAL_TX_STATIC_SIZE                     (configMINIMAL_STACK_SIZE*3)
#define TASK_CAN_HAL_TX_STATIC_SIZE                     (configMINIMAL_STACK_SIZE*3)
#define TASK_CAN_HAL_RX_STATIC_SIZE                     (configMINIMAL_STACK_SIZE*3)
#define TASK_POWER_MANAGE_STATIC_SIZE					(configMINIMAL_STACK_SIZE*2)
#define TASK_NET_MANAGE_STATIC_SIZE                     (configMINIMAL_STACK_SIZE*2)
#define TASK_DIAGNOSTIC_STATIC_SIZE                     (configMINIMAL_STACK_SIZE*4)
#define TASK_DIAGNOSTIC_DTC_STATIC_SIZE                 (configMINIMAL_STACK_SIZE*2)
#define TASK_ECALL_STATIC_SIZE                          (configMINIMAL_STACK_SIZE*6)
#define TASK_MCU_CPU_SYNC_STATIC_SIZE                   (configMINIMAL_STACK_SIZE*4)
#define TASK_CAN_DATA_TO_CPU_STATIC_SIZE                (configMINIMAL_STACK_SIZE*6)
#define TASK_TEST_PRESENT_STATIC_SIZE	                (configMINIMAL_STACK_SIZE*2)
#define TASK_REMOTE_DIAGNOSTIC_STATIC_SIZE              (configMINIMAL_STACK_SIZE*2)
#define TASK_REMOTE_CONTROL_STATIC_SIZE                 (configMINIMAL_STACK_SIZE*8)
#define TASK_CAN_PERIOD_STATIC_SIZE   	                (configMINIMAL_STACK_SIZE*3)
#define TASK_MONITOR_STATIC_SIZE					    (configMINIMAL_STACK_SIZE)

/****************************** Type Definitions ******************************/
/****************************** Global Variables ******************************/
StaticTask_t m_mpuHalUartTxTaskBuffer;
StackType_t m_mpuHalUartTxTaskStack[TASK_MPU_HAL_TX_STATIC_SIZE];

StaticTask_t m_mpuHalSpiTxTaskBuffer;
StackType_t m_mpuHalSpiTxTaskStack[TASK_MPU_HAL_TX_STATIC_SIZE];

StaticTask_t m_canHalRxTaskBuffer;
StackType_t m_canHalRxTaskStack[TASK_CAN_HAL_RX_STATIC_SIZE];

StaticTask_t m_canHalTxTaskBuffer;
StackType_t m_canHalTxTaskStack[TASK_CAN_HAL_TX_STATIC_SIZE];

StaticTask_t m_powerManageTaskBuffer;
StackType_t m_powerManageTaskStack[TASK_POWER_MANAGE_STATIC_SIZE];

#if(TASK_TEST_ENABLE == 1)
StaticTask_t m_testTaskBuffer;
StackType_t m_testTaskStack[TASK_TEST_STATIC_SIZE];
#endif

StaticTask_t m_mcuCpuSyncTaskBuffer;
StackType_t m_mcuCpuSyncTaskStack[TASK_MCU_CPU_SYNC_STATIC_SIZE];

StaticTask_t m_canToCpuTaskBuffer;
StackType_t m_canToCpuTaskStack[TASK_CAN_DATA_TO_CPU_STATIC_SIZE];

StaticTask_t m_diagnosticTaskBuffer;
StackType_t m_diagnosticTaskStack[TASK_DIAGNOSTIC_STATIC_SIZE];

StaticTask_t m_dtcTaskBuffer;
StackType_t m_dtcTaskStack[TASK_DIAGNOSTIC_DTC_STATIC_SIZE];

StaticTask_t m_ecallTaskBuffer;
StackType_t m_ecallTaskStack[TASK_ECALL_STATIC_SIZE];

StaticTask_t m_canPeriodTaskBuffer;
StackType_t m_canPeriodTaskStack[TASK_CAN_PERIOD_STATIC_SIZE];

StaticTask_t m_remoteDiagnostic1TaskBuffer;
StackType_t m_remoteDiagnostic1TaskStack[TASK_REMOTE_DIAGNOSTIC_STATIC_SIZE];

StaticTask_t m_remoteControlTaskBuffer;
StackType_t m_remoteControlTaskStack[TASK_REMOTE_CONTROL_STATIC_SIZE];

uint8_t ExampleStr1[] = "App Project is running...\r\n";
const CanConfigure_t g_globalCanfdConfig[] = 
{
    {
        .canChannel = TBOX_CAN_CHANNEL_1,
        .canFdMode = 1,
        .bandrate = E_CAN_500K,
        .dataBandrate = E_CAN_2000K,
        .busOffEventFun = NULL,
    },
    {
        .canChannel = TBOX_CAN_CHANNEL_2,
        .canFdMode = 1,
        .bandrate = E_CAN_500K,
        .dataBandrate = E_CAN_2000K,
        .busOffEventFun = NULL,
    },
    {
        .canChannel = TBOX_CAN_CHANNEL_3,
        .canFdMode = 1,
        .bandrate = E_CAN_500K,
        .dataBandrate = E_CAN_2000K,
        .busOffEventFun = NULL,
    },
    {
        .canChannel = TBOX_CAN_CHANNEL_4,
        .canFdMode = 1,
        .bandrate = E_CAN_500K,
        .dataBandrate = E_CAN_2000K,
        .busOffEventFun = NULL,
    },
};

/****************************** Function Declarations *************************/
static void start_tasks(void);
static void CreateDriverTasks(void);
static void CreateAppTasks(void);
static int16_t TasksManageInit(void);
static void MainTask(void *pvParameters);

/****************************** Public Function Implementations ***************/
#if configCHECK_FOR_STACK_OVERFLOW == 2
/*************************************************
   Function:        vApplicationStackOverflowHook
   Description:     Callback function for FreeRTOS to detect stack overflow
   Input:           xTask - Handle of the task that caused the stack overflow
                    pcTaskName - Name of the task that caused the stack overflow
   Output:          None
   Return:          None
   Others:          Required by FreeRTOS when configCHECK_FOR_STACK_OVERFLOW is set to 1 or 2
  *************************************************/
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    /* Disable interrupts to prevent further damage */
    taskDISABLE_INTERRUPTS();
	for(uint32_t i = 0; i < 10; i++)
	{
		TBOX_PRINT("Stack overflow detected in task: %s\n", pcTaskName);
	}
    
    for( ;; )
    {
    }
}
#endif

/*************************************************
   Function:        TasksStart
   Description:     Public API to start the task management system
   Input:           None
   Output:          None
   Return:          None
   Others:          Calls the internal start_tasks function to initialize and start all tasks
 *************************************************/
void TasksStart(void)
{
    start_tasks();
}

/****************************** Private Function Implementations ***************/
/*************************************************
   Function:        CreateDriverTasks
   Description:     Create driver-level tasks for hardware communication
   Input:           None
   Output:          None
   Return:          None
   Others:          Creates tasks for MPU transmission and CAN reception
 *************************************************/
static void CreateDriverTasks(void)
{
    xTaskCreateStatic( MpuHalUartTxTask, "mpu_tx_t", TASK_MPU_HAL_TX_STATIC_SIZE, NULL, MPU_UART_TASK_PRIORITY, m_mpuHalUartTxTaskStack,&m_mpuHalUartTxTaskBuffer);
    xTaskCreateStatic( MpuHalSpiTxTask, "mpu_spi_tx_t", TASK_MPU_HAL_TX_STATIC_SIZE, NULL, DRIVER_TASK_PRIORITY, m_mpuHalSpiTxTaskStack,&m_mpuHalSpiTxTaskBuffer);
    xTaskCreateStatic( CanHalReceiveTask, "can_rx_t", TASK_CAN_HAL_RX_STATIC_SIZE, NULL, DRIVER_TASK_PRIORITY, m_canHalRxTaskStack,&m_canHalRxTaskBuffer); 
    xTaskCreateStatic( CanHalSendTask, "can_tx_t",TASK_CAN_HAL_TX_STATIC_SIZE, NULL,DRIVER_TASK_PRIORITY, m_canHalTxTaskStack, &m_canHalTxTaskBuffer);
}

/*************************************************
   Function:        CreateAppTasks
   Description:     Create application-level tasks for system functionality
   Input:           None
   Output:          None
   Return:          None
   Others:          Creates tasks for vehicle data processing, diagnostics, CAN communication, and remote control
 *************************************************/
static void CreateAppTasks(void)
{
    xTaskCreateStatic(TaskVehicleDataToCpu,"ToCpu_task",TASK_CAN_DATA_TO_CPU_STATIC_SIZE, NULL,UPLOAD_DATA_TASK_PRIORITY, m_canToCpuTaskStack, &m_canToCpuTaskBuffer ); 
    xTaskCreateStatic(McuMpuSyncTaskMain,"mcuMpuSync_task",TASK_MCU_CPU_SYNC_STATIC_SIZE, NULL,MCU_CPU_SYNC_TASK_PRIORITY, m_mcuCpuSyncTaskStack, &m_mcuCpuSyncTaskBuffer );
    xTaskCreateStatic(TaskEcuDiagnostic, "diagnostic_task", TASK_DIAGNOSTIC_STATIC_SIZE, NULL, UDS_DIG_TASK_PRIORITY, m_diagnosticTaskStack,&m_diagnosticTaskBuffer);
    xTaskCreateStatic(canPeriodTaskMain, "canPeriod_task", TASK_CAN_PERIOD_STATIC_SIZE, NULL, CAN_TX_CYCLE_TASK_PRIORITY, m_canPeriodTaskStack,&m_canPeriodTaskBuffer);
    xTaskCreateStatic(TaskDtcProcess, "dtc_task", TASK_DIAGNOSTIC_DTC_STATIC_SIZE, NULL, UDS_DIG_TASK_PRIORITY, m_dtcTaskStack,&m_dtcTaskBuffer);  
    xTaskCreateStatic(TaskEcallProcess, "ecall_task", TASK_ECALL_STATIC_SIZE, NULL, UDS_DIG_TASK_PRIORITY, m_ecallTaskStack,&m_ecallTaskBuffer);
    xTaskCreateStatic(RemoteDiagnosticTaskMain, "remoteDiagnostic_task",TASK_REMOTE_DIAGNOSTIC_STATIC_SIZE,NULL,REMOTE_CONTROL_TASK_PRIORITY,m_remoteDiagnostic1TaskStack,&m_remoteDiagnostic1TaskBuffer);
    xTaskCreateStatic(TaskAppVehicleRemoteControl, "remoteControl_task",TASK_REMOTE_CONTROL_STATIC_SIZE,NULL,REMOTE_CONTROL_TASK_PRIORITY,m_remoteControlTaskStack,&m_remoteControlTaskBuffer);
}

/*************************************************
   Function:        TasksManageInit
   Description:     Initialize all hardware abstraction layer modules
   Input:           None
   Output:          None
   Return:          0 on successful initialization
   Others:          Initializes flash, peripheral, logging, power management, CAN, MPU, and timer modules
 *************************************************/
static int16_t  TasksManageInit(void)
{
/*
    BIT0 (1 << 0): RCM_RESET_STATUS_POR_RST (上电复位)
    BIT1 (1 << 1): RCM_RESET_STATUS_LVR_RST (低压复位)
    BIT2 (1 << 2): RCM_RESET_STATUS_SW_RST (软件复位)
    BIT3 (1 << 3): RCM_RESET_STATUS_LOCKUP_RST (CPU锁死复位)
    BIT4 (1 << 4): RCM_RESET_STATUS_ACK_ERR_RST (应答错误复位)
    BIT5 (1 << 5): RCM_RESET_STATUS_WDG_RST (看门狗复位)
    BIT7 (1 << 7): RCM_RESET_STATUS_XOSC_LOSS_RST (外部晶振丢失复位)
    BIT8 (1 << 8): RCM_RESET_STATUS_PLL_UNLOCK_RST (锁相环失锁复位)
    BIT9 (1 << 9): RCM_RESET_STATUS_VHSI_LOSS_RST (内部高频时钟丢失复位)
    BIT10 (1 << 10): RCM_RESET_STATUS_EXT_RST (外部引脚复位)
    BIT11 (1 << 11): RCM_RESET_STATUS_SMU_ERR_RST (系统管理单元错误复位)
    BIT12 (1 << 12): RCM_RESET_STATUS_ECC2_ERR_RST (Flash ECC双位错误复位)
*/
    LogHalInit(3);
    uint32 rawStatus = Rcm_Hal_GetResetStatus();
    uint32 resetStatus = (rawStatus >> 16) | (rawStatus << 16);
    if (resetStatus & (1UL << 2))  TBOX_PRINT("-> [SW] Software Reset\r\n");
    else if (resetStatus & (1UL << 5))  TBOX_PRINT("-> [WDG] Watchdog Reset !!!\r\n");
    TBOX_PRINT("Reset Status: 0x%08X\r\n", resetStatus);
    Rcm_Hal_ClearResetStatus();
    FlashHalInit();
    PeripheralHalInit();
    PowerManageHalInit(1,570,15000,9000);
    CanHalInit(g_globalCanfdConfig,sizeof(g_globalCanfdConfig)/sizeof(g_globalCanfdConfig[0]));    
    EcallHalInit();
    MpuHalInit();
    LogHalUpLoadLog("System reset cause: 0x%08X", resetStatus);
    TimerHalInit();
    TBOX_PRINT("Task init complete! VERSION: %s\r\n", "027");
    return 0;
}

/*************************************************
   Function:        MainTask
   Description:     Main task function that initializes system and creates all tasks
   Input:           pvParameters - Task parameters (unused)
   Output:          None
   Return:          None
   Others:          Initializes tasks, creates driver and application tasks, and starts power management
 *************************************************/
static void MainTask(void *pvParameters)
{
    TasksManageInit();
    CreateDriverTasks();
    CreateAppTasks(); 
    TaskPowerManage(NULL);
}

/*************************************************
   Function:        start_tasks
   Description:     Start the main task and initialize the FreeRTOS scheduler
   Input:           None
   Output:          None
   Return:          None
   Others:          Creates the main task with appropriate priority and starts the scheduler
 *************************************************/
static void start_tasks(void)
{
    xTaskCreate( MainTask, "main_task", configMINIMAL_STACK_SIZE*4, NULL, POWERMANAGE_TASK_PRIORITY, NULL );
    vTaskStartScheduler();
    while(1);
}

