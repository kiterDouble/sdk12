/*************************************************
 Copyright © 2025 SiRun (AnHui). All rights reserved.
 File Name: remoteControl.c
 Author: Simon
 Created Time: 2025-09-24
 Description: Vehicle Remote Control Module
              This module handles remote control functionality for vehicle systems,
              including command processing, state machine management,
              certification process, and communication with various ECUs.
              It supports multiple remote control operations such as door locking,
              engine starting, HVAC control, and other vehicle functions.
 Others: 
 *************************************************/
 /****************************** include ***************************************/
#include "stdlib.h"
#include "canHal.h"
#include "mpuHal.h"
#include "remoteControlCanSignal.h"
#include "remoteControl.h"
#include "peripheralHal.h"
#include "canParseSdk.h"
#include "vehicleSignalApp.h" 
#include "mpuHal.h"
#include "timerHal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "logHal.h"
#include "udsDidFunction.h"
#include "osekNmSdk.h"
#include "canPeriodTask.h"
#include "ParameterStoreManageApp.h"
#include "taskVehicleDataToCpu.h"
#include "md5.h"
#include "taskVehicleDataToCpu.h"
/****************************** Macro Definitions ******************************/
#define REMOTE_CONTROL_ECU_MAX_NUM                  (7U)
#define REMOTE_CONTROL_ECU_CMD_MAX_NUM              (20U)
#define REMOTE_CONTROL_HVAC_CMD_NUM                 (9U)
#define REMOTE_CONTROL_PEPS_CMD_NUM                 (4U)
#define REMOTE_CONTROL_BCM_CMD_NUM                  (12U)
#define REMOTE_CONTROL_HVSM_CMD_NUM                 (4U)
#define REMOTE_CONTROL_PLGM_CMD_NUM                 (1U)
#define REMOTE_CONTROL_HOD_CMD_NUM                  (1U)
#define REMOTE_CONTROL_SEND_CAN_SIZE                (8U)
#define REMOTE_CONTROL_DATA_UART_RECEIVE_BUF_SIZE   (200U)
#define REMOTE_CONTROL_UART_SEND_BUF_SIZE           (5U)

#define REMOTE_CONTROL_AID                          (0x02U)
#define REMOTE_CONTROL_MID                          (0x40U)
#define REMOTE_CONTROL_SUBCMD                       (0x01U)
#define REMOTE_CONTROL_SUBCOMMAND_ACK               (0x6EU)
#define REMOTE_CONTROL_SUBCOMMAND_RESULT            (0x6FU)
#define REMOTE_CONTROL_ACK_CTRL_CMD                 (0x02U)
//cmd type
#define REMOTE_CONTROL_CMD_CONTROL_TYPE             (0x0U)
#define REMOTE_CONTROL_CMD_WAKE_TYPE                (0x1U)
#define REMOTE_CONTROL_CMD_OTA_TYPE                 (0x2U)

#define REMOTE_CONTROL_BCM_REQ_MAX_CNT              (2U)
#define REMOTE_CONTROL_PEPS_REQ_MAX_CNT             (2U)
#define REMOTE_CONTROL_TRANS_MAX_COUNT              (3U)

#define ESK_LENGTH                                  (16U)
#define RANDOM_LENGTH                               (8U)
#define REMOTE_CONTROL_TASK_PERIOD_TIME             (5U)

#define CAN_WATCH_NUM        (5U)   
//error code
//#define REMOTE_CONTROL_ERR_CODE_SUCCESS             (0x0U)
//#define REMOTE_CONTROL_ERR_CODE_TIME_OUT            (0x1U)
//#define REMOTE_CONTROL_ERR_CODE_INVALID_CONDITION   (0x2U)
//#define REMOTE_CONTROL_ERR_CODE_INVALID_CMD         (0x4U)
//#define REMOTE_CONTROL_ERR_CODE_EXCUTE_ERROR        (0x6U)
//#define REMOTE_CONTROL_ERR_CODE_NO_NAME             (0xAU)
//#define REMOTE_CONTROL_ERR_CODE_FUC_FORBID          (0xFU)
//#define REMOTE_CONTROL_ERR_HAZARD_LAMP_ON           (0x15U)
//#define REMOTE_CONTROL_ERR_HAZARD_LAMP_OFF          (0x16U)

/*Error code*/
#define REMOTE_CONTROL_ERR_CODE_SOC                 (0x1U) /* SOC<= 30% */
#define REMOTE_CONTROL_ERR_CODE_SET_MODE            (0x2U) /* BMS请求BTMS工作状态为非制热 */
#define REMOTE_CONTROL_ERR_CODE_MAX_ERR             (0x4U) /* 故障等级 >= 3 */
#define REMOTE_CONTROL_ERR_CODE_ACTIVE_ERR          (0x8U) /* 激活失败 */
#define REMOTE_CONTROL_ERR_CODE_INACTIVE_ERR        (0x10U) /* 取消激活失败 */
#define REMOTE_CONTROL_ERR_CODE_HANDSHAKE_ERR       (0x20U) /* 握手失败 */
#define REMOTE_CONTROL_ERR_CODE_LOCK_ERR            (0x40U) /* 锁车失败 */
#define REMOTE_CONTROL_ERR_CODE_UNLOCK_ERR          (0x80U) /* 解锁失败 */
#define REMOTE_CONTROL_ERR_CODE_WAKE_ERR            (0x100U) /* 唤醒失败 */

#define REMOTE_CONTROL_OTA_AND_WAKE_RESULT          (0U)

/*Vehicle type*/
#define REMOTE_CONTROL_VEHICLE_KANGMINGSI           (0x70U) /* 康明斯锁车 */
#define REMOTE_CONTROL_VEHICLE_YUCHAI               (0x92U) /* 玉柴锁车 */
#define REMOTE_CONTROL_VEHICLE_MAN                  (0x93U) /* 曼锁车 */
#define REMOTE_CONTROL_VEHICLE_WEICHAI              (0x90U) /* 潍柴锁车 */
#define REMOTE_CONTROL_VEHICLE_XICHAI               (0x94U) /* 锡柴锁车 */
#define REMOTE_CONTROL_VEHICLE_ELEC                 (0x80U) /* 电车锁车 */

/****************************** Type Definitions ******************************/
typedef enum
{
    E_REMOTE_CONTROL_LOCKCMD_UNLOCK    = 0U,     /*解锁*/ 
    E_REMOTE_CONTROL_LOCKCMD_LOCK      = 1U,     /*锁车*/  
    E_REMOTE_CONTROL_LOCKCMD_TORQUE    = 2U,     /*限扭矩  暂没用到*/
    E_REMOTE_CONTROL_LOCKCMD_ACTIVE    = 3U,     /*激活*/
    E_REMOTE_CONTROL_LOCKCMD_INACTIVE  = 4U,     /*取消激活*/
}RemoteControlLockCmd_e;

typedef enum
{
    E_REMOTE_CONTROL_BUSSINESS_NO             = 0U,     /*没有业务*/ 
    E_REMOTE_CONTROL_BUSSINESS_LOCK           = 1U,     /*远程锁车*/ 
    E_REMOTE_CONTROL_BUSSINESS_BATT_TEMP      = 2U,     /*远程电池加热*/  
}RemoteControlBussiness_e;

typedef enum
{
    E_REMOTE_CONTROL_AuthState_Idle         = 0U,     /*空闲*/ 
    E_REMOTE_CONTROL_AuthState_RN           = 1U,     /*接收vcu的随机数*/ 
    E_REMOTE_CONTROL_AuthState_SEND_KEY     = 2U,     /*发送计算出的KEY*/  
}RemoteControlAuthState_e;

typedef enum
{   
    ECU_NULL_E  = 0u,
    ECU_HVAC_E  = 1U,
    ECU_PEPS_E  = 2U,
    ECU_BCM_E   = 3U,
    ECU_HVSM_E  = 4U,
    ECU_PLGM_E  = 7U,
    ECU_HOD_E   = 13U,
}RemoteControlEcuId_t;

typedef enum
{
    RemoteControlResult_Success_e       = 0U,    
    RemoteControlResult_Fail_e          = 1U,      
    RemoteControlResult_Processing_e    = 2U,  
}RemoteControlProcessResult_t;

typedef enum
{
    RemoteControlWakeUpFlag_NotKeep_e   = 0U,
    RemoteControlWakeUpFlag_Keep_e      = 1U,
}RemoteControlWakeUpFlag_t;
typedef enum
{
    REMOTE_CONTROL_CAN_FD_USE = 1U,
    REMOTE_CONTROL_CAN_FD_NOT_USE = 0U,
}RemoteControlCanFdUse_t;

typedef enum
{
    REMOTE_CONTROL_BCM_REQ_AHTU_TIME    = 199U,
    REMOTE_CONTROL_BCM_CHECK_AHTU_TIME  = 299U,
    REMOTE_CONTROL_PEPS_REQ_AHTU_TIME   = 199U,
    REMOTE_CONTROL_PEPS_CHECK_AHTU_TIME = 299U,
    REMOTE_CONTROL_TRANS_CYCLE_TIME     = 39U, //40ms
    REMOTE_CONTROL_TRANS_SEPE_TIME      = 119U,
    REMOTE_CONTROL_PLGM_SEPE_TIME       = 99U,
    REMOTE_CONTROL_PLGM_CLOSE_WAIT_TIME = 199U,
    REMOTE_CONTROL_CHECK_EXCUTE_TIME    = 15999U,
    REMOTE_CONTROL_SLEEP_FORBID_TIME    = 30000U,
}RemoteControlTimeout_t;
typedef enum
{
    REMOTE_CONTROL_GW_BCM_E                 = 0x046,
    REMOTE_CONTROL_GW_PEPS_E                = 0x04D,
}RemoteControlReceiveCanId_t;

typedef RemoteControlProcessResult_t (*RemoteControlPreCheckFunc)(void);
typedef RemoteControlProcessResult_t (*RemoteControlCertificationFunc)(void);

typedef struct
{
    RemoteControlCmdId_t cmdId;
    RemoteControlPreCheckFunc checkFunc; 
    RemoteControlCertificationFunc certificationFunc;
} RemoteControlEntry_t;

typedef struct 
{
    RemoteControlEcuId_t ecuId;
    RemoteControlEntry_t cmdTable[REMOTE_CONTROL_ECU_CMD_MAX_NUM];
    uint8_t cmdCount;
}RemoteControlTotalTable_t;

typedef enum
{
    PROCESS_SIGNAL_STATE_IDLE,
    PROCESS_SIGNAL_STATE_NORMAL_SEND,   
    PROCESS_SIGNAL_STATE_NORMAL_WAIT,
    PROCESS_SIGNAL_STATE_NORMAL_RESULT,            
} ProcessSignalState_t;

typedef enum
{
    REMOTE_CONTROL_REQ_WAKE_STATUS_IDLE,
    REMOTE_CONTROL_REQ_WAKE_STATUS_NORMAL,          
} RemoteControlReqWakeStatus;

typedef enum
{
    REMOTE_CONTROL_OTA_FLAG_IDLE,
    REMOTE_CONTROL_OTA_FLAG_ING,
} RemoteControlOtaFlag_t;

typedef struct 
{
    uint8_t RxFlag;
    uint32_t CanId;
    uint8_t CanData[8];
//    uint16_t TimeOut;
//    uint8_t TimeOutFlag;
}EventMessageInfo_t;

typedef struct {
    uint32_t canId;
    uint16_t tickCnt;    // 超时计时
    uint8_t isAlive;     // true=周期收到报文，正常
} CanWatch_t;

/****************************** Function Declarations *************************/
static void RemoteControlStateMachine(void);
static void RemoteControlCmdProcess(void);
static RemoteControlProcessResult_t RemoteControlPreCheckProcess(void);
//static RemoteControlProcessResult_t RemoteControlCheckResCanSignal(void);
//static void RemoteControlCertificationProcess(void);
static void RemoteControlHandleSignalProcess(void);
static void RemoteControlSendResultProcess(void);
static void RemoteControlSendAck(void);
//static void RemoteControlNormalPackReqCanSignal(void);
static void RemoteControlSendResult(uint16_t ErrorCode);
//static void RemoteControlSpecialPackReqCanSignal(void);
static void RemoteControlStartStateMachine(void);
static void RemoteControlSetKeepWakeFlag(uint8_t keepWakeFlag);
//static void RemoteControlForbidSleepCheck(void);
//static uint8_t RemoteControlMapEcuIdToIndex(RemoteControlEcuId_t ecuId);
//static void BcmAuthCalcKey(const uint8_t canRandom[8], const uint8_t esk[16], uint8_t outKey[8]);
//static void PepsAuthCalcKey8(const uint8_t rnd[8], const uint8_t esk[16], uint8_t out[8]);
//static void RemoteCtrlSignalValSet(uint8_t *buf, RemoteControlReqSignalId_t sig, double value);
//static void RemoteConreolReqWakeStateMachine(void);
//static void RemoteControlSendReqWakeUpResult(void);
//static void RemoteControlSendIntoOtaModeResult(void);
static void RemoteControlSetTotalState(RemoteControlState_t state);
static void AuthenticationProcess(void);
static void WakeMaintainSendProcess(void);
static void CanWatch_Init(void);
static uint8_t CanCommExist(void);
//static uint64_t BytesToU64Le(const uint8_t in[8]);
/****************************** Global Variables ******************************/
//static uint8_t g_remoteControlCanBuf[REMOTE_CONTROL_SEND_CAN_SIZE] = {0};
static volatile RemoteControlEcuId_t g_remoteControlEcuId = ECU_NULL_E;
static volatile RemoteControlCmdId_t g_remoteControlCmdId = CMD_DEFAULT_E;
//static uint8_t g_remoteControlParamValue = 0U;
static RemoteControlState_t g_remoteControlTotalState = RemoteControlStateIdle;
static RemoteControlStatusSignalInfo_t g_remoteControlSignalInfo;
//static int16_t g_remoteControlAuthTimerHandle = -1;
//static int16_t g_remoteControlTransTimerHandle = -1;
//static int16_t g_remoteControlSleepForrbidHandle = -1;
static int16_t  g_remoteControlUartHandle = -1;
static uint8_t g_remoteControlDataRcevBuf[REMOTE_CONTROL_DATA_UART_RECEIVE_BUF_SIZE] = {0};
static uint8_t g_remoteControlDataBuffer[REMOTE_CONTROL_DATA_UART_RECEIVE_BUF_SIZE] = {0};
static uint8_t g_remoteControlSendBuffer[REMOTE_CONTROL_UART_SEND_BUF_SIZE] = {0};
static MpuHalDataPack_t g_remoteControlReceivePack;
static MpuHalDataPack_t g_remoteControlSendPack;
//static uint8_t g_remoteControlESK[ESK_LENGTH] = {0};
//static uint8_t g_remoteControlPlgmFrameTime = 0U;
static uint16_t g_remoteControlErrorCode = 0;
static uint8_t g_remoteControlKeepWakeFlag = 0U;
static int16_t g_remoteControlCan1Handle = -1;
//static uint8_t g_randomPepsArray[RANDOM_LENGTH] = {0};
//static uint8_t g_randomBcmArray[RANDOM_LENGTH] = {0};
//static uint8_t g_remoteControlHvacPepsCheck = 0U;
//static uint8_t g_remoteControlHvsmPepsCheck = 0U;
//static uint8_t g_hvacSignalSet[20] = {0U};
//static uint8_t g_remoteControlReqWakeByte = 0U;
//static uint32_t g_remoteControReqWakeTime = 0U;
//static RemoteControlReqWakeStatus g_remoteControlReqWakeStatus = REMOTE_CONTROL_REQ_WAKE_STATUS_IDLE;
static uint8_t g_remoteControlOTAFlag = 0U;
//static uint32_t g_remoteControlOTATime = 0U;

static RemoteControlBussiness_e g_remoteControlCmdBussiness = E_REMOTE_CONTROL_BUSSINESS_NO; /*远控任务项*/
static uint8_t g_remoteControlCmd = 0; /*远控命令*/
static RemoteControlAuthState_e g_authenticationState = E_REMOTE_CONTROL_AuthState_Idle;
static EventMessageInfo_t g_authenticationBuff;
static uint8_t g_authSeed[3] = {0};
static uint8_t g_key[8] = {0};
static uint8_t g_wakeMaintainFlag = 0; /*唤醒保持标志*/
static const uint32_t g_wakeMaintainCanid = 0x1820F03A;
static CanWatch_t g_CanWatch[CAN_WATCH_NUM];
static const uint32_t g_WatchCanId[5] = {
    0x18F117D1UL,
    0x1830B021UL,
    0x18F101D0UL,
    0xCFF0008UL,
    0xCFFD003UL
};
/****************************** Public Function Implementations ***************/
/*************************************************
 Function:        TaskAppVehicleRemoteControl
 Description:     Task for handling vehicle remote control functionality
 Input:           pvParameters - Task parameters (not used)
 Output:          None
 Return:          None
 Others:          Manages MPU communication for remote control
                 Handles receive buffer and filters configuration
                 Implements main control loop for remote commands
*************************************************/
void TaskAppVehicleRemoteControl( void *pvParameters )  
{
    int16_t ret = -1;
    uint32_t DataLength = 0;
    uint8_t TempFlag = 0;
    int16_t TempRet = 0;
    MpuHalFilter_t filter;
    g_remoteControlUartHandle = MpuHalOpen();
    g_remoteControlCan1Handle = CanHalOpen(TBOX_CAN_CHANNEL_1);
//    g_remoteControlAuthTimerHandle = TimerHalOpen();
//    g_remoteControlTransTimerHandle = TimerHalOpen();
//    g_remoteControlSleepForrbidHandle = TimerHalOpen();
    filter.aid = 0x02;
    filter.midMin = 0x40;
    filter.midMax = 0x40;
    MpuHalSetRxFilter(g_remoteControlUartHandle,&filter);
    MpuHalSetRxBuffer(g_remoteControlUartHandle,g_remoteControlDataRcevBuf,sizeof(g_remoteControlDataRcevBuf));
    g_remoteControlReceivePack.pDataBuffer = g_remoteControlDataBuffer;
    g_remoteControlReceivePack.dataBufferSize = sizeof(g_remoteControlDataBuffer);
    TempRet = WorkFlashVehicleInforRead(E_PARAMETER_INFO_REMOTE_WAKE_MAINTAIN,&TempFlag,&DataLength);
    if(TempRet == 0)
    {
        g_wakeMaintainFlag = TempFlag;
    }
    CanWatch_Init();
    while(1)
    {
        ret = MpuHalReceive(g_remoteControlUartHandle,&g_remoteControlReceivePack,0);
        if(ret == MPU_HAL_STATUS_OK)
        {
            RemoteControlCmdProcess();
        }
        RemoteControlStateMachine();
        AuthenticationProcess();
        WakeMaintainSendProcess();
        
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

/***************************************************************************//**
 * @Function    RemoteCtrlSignalTransmit
 * @Description SignalTransmit
 * @Input       /
 * @Output      /
 * @Return      None
 * @Others      /
 ******************************************************************************/
static void RemoteCtrlSignalTransmit(uint32_t canId, const uint8_t *pTxData,uint16_t dlc)
{    
    CanHalTransmitQueued(g_remoteControlCan1Handle, canId, pTxData, dlc, REMOTE_CONTROL_CAN_FD_USE, CAN_TX_PRIO_NORMAL);        
}
/*************************************************
 Function:        WakeMaintainSendProcess
 Description:     wake info maintain send
 Input:           None
 Output:          None
 Return:          None
 Others:          /
*************************************************/
static void WakeMaintainSendProcess(void)
{
  uint8_t data[8] = {0x00};
  static uint16_t SendPeriodCnt = 0;
  static uint16_t StopSendDelayCnt = 0;
  static uint16_t StopSendFlag = 0;
  if(g_wakeMaintainFlag == 1)
  {  
      SendPeriodCnt++;
      if(SendPeriodCnt >= 500/REMOTE_CONTROL_TASK_PERIOD_TIME)  /*唤醒帧500ms周期发送*/
      {
        SendPeriodCnt = 0;
        RemoteCtrlSignalTransmit(g_wakeMaintainCanid,data,sizeof(data));
      } 
      
      if((g_remoteControlSignalInfo.BMS_ST2_SOC <= 30) ||\
      (g_remoteControlSignalInfo.B2T_RQ_SetMode_A != 0x02) ||\
      (g_remoteControlSignalInfo.Tx_DB_MaxErrLevel >= 0x03) ||\
      (PeripheralHalGetKl15Status() == 1) ||
      (CanCommExist() == 0))
      {
          StopSendFlag = 1;
      }    
  }
  
  if(StopSendFlag == 1)
  {
    if(g_wakeMaintainFlag == 1)
    {
        StopSendDelayCnt++;
        if(StopSendDelayCnt >= 5000/REMOTE_CONTROL_TASK_PERIOD_TIME) /*延时5s停止发送*/
        {
          StopSendDelayCnt = 0;
          SendPeriodCnt = 0;
          g_wakeMaintainFlag = 0;
          StopSendFlag = 0;
          RemoteControlSetKeepWakeFlag(RemoteControlWakeUpFlag_NotKeep_e);
          WorkFlashVehicleInforStore(E_PARAMETER_INFO_REMOTE_WAKE_MAINTAIN, &g_wakeMaintainFlag, sizeof(g_wakeMaintainFlag));
        }
    }
  }
}
/*************************************************
 Function:        ReceiveSeedMessage
 Description:     receive seed from vcu and store
 Input:           canMsg
 Output:          None
 Return:          None
 Others:          /
*************************************************/
void ReceiveSeedMessage(CanHalMsg_t canMsg)
{
    g_authenticationBuff.CanId = 0x18FC0100;
    canMsg.canId &= 0x7FFFFFFF;
    if(canMsg.canId == g_authenticationBuff.CanId)
    {
        g_authenticationBuff.RxFlag = 1;
        memcpy(g_authenticationBuff.CanData,canMsg.canData,8);
    }
}
/*************************************************
 Function:        ReceivePeriodMessage
 Description:     receive heartbeat message from vcu and judge if lost
 Input:           canMsg
 Output:          None
 Return:          None
 Others:          /
*************************************************/
void ReceivePeriodMessage(CanHalMsg_t canMsg)
{
    canMsg.canId &= 0x7FFFFFFF;
    for(uint8_t i = 0; i < CAN_WATCH_NUM; i++)
    {
        if(g_CanWatch[i].canId == canMsg.canId)
        {
            g_CanWatch[i].tickCnt = 0;
            g_CanWatch[i].isAlive = 1;
            break;
        }
        
        if(g_CanWatch[i].isAlive)
        {
            g_CanWatch[i].tickCnt ++;
            if(g_CanWatch[i].tickCnt >= 1000/5)
            {
                g_CanWatch[i].isAlive = 0;
            }
        }
    }
}
/*************************************************
 Function:        CanWatch_Init
 Description:     init heartbeat message
 Input:           None
 Output:          None
 Return:          None
 Others:          /
*************************************************/
static void CanWatch_Init(void)
{
    uint8_t i;
    for(i = 0; i < CAN_WATCH_NUM; i++)
    {
        g_CanWatch[i].canId = g_WatchCanId[i];
        g_CanWatch[i].tickCnt = 0;
        g_CanWatch[i].isAlive = 0;
    }
}
/*************************************************
 Function:        CanCommExist
 Description:     result of whether lost heartbeat message
 Input:           None
 Output:          None
 Return:          0:lost  1:not lost
 Others:          /
*************************************************/
static uint8_t CanCommExist(void)
{
    uint8_t i;
    for(i = 0; i < CAN_WATCH_NUM; i++)
    {
        if(g_CanWatch[i].isAlive == 0)
        {
            return 0;
        }
    }
    return 1;
}
/*************************************************
 Function:        AuthenticationProcess
 Description:     Authentication with vcu
 Input:           None
 Output:          None
 Return:          None
 Others:          /
*************************************************/
static void AuthenticationProcess(void)
{
    uint8_t Vin[17] = {0};
    uint8_t CombinedData[8] = {0};
//    uint8_t CombinedData[8] = {0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0XBB};
    uint16_t VinLength = 17;
    uint8_t ComLength = 8;
    uint8_t TempMd5[16] = {0};
    uint32_t KeyCanId = 0x18FFD5FC;
    
    switch(g_authenticationState)
    {
        case E_REMOTE_CONTROL_AuthState_Idle:
          {
            g_authenticationState = E_REMOTE_CONTROL_AuthState_RN;
          }
            break;

        case E_REMOTE_CONTROL_AuthState_RN:
          {
            if(g_authenticationBuff.RxFlag == 1) /*收到了数据*/
            {
                memcpy(&g_authSeed[0], g_authenticationBuff.CanData, 3);
                Service22ReadVINF190(Vin,&VinLength);
                memcpy(&CombinedData[0], &Vin[12], 5);
                memcpy(&CombinedData[5], &g_authSeed[0], 3);
                /*md5加密出key值*/
                md5_calc(CombinedData,ComLength,TempMd5);
                memcpy(g_key,TempMd5,8);
                g_authenticationBuff.RxFlag = 0;
                memset(g_authenticationBuff.CanData, 0 , 8);
                g_authenticationState = E_REMOTE_CONTROL_AuthState_SEND_KEY; 
            }
          }
            break;
 
        case E_REMOTE_CONTROL_AuthState_SEND_KEY:
          {
              RemoteCtrlSignalTransmit(KeyCanId,g_key,sizeof(g_key));
              g_authenticationState = E_REMOTE_CONTROL_AuthState_Idle;
          }
            break;

        default:
            break;
    }
}

/*************************************************
 Function:        RemoteControlStateMachine
 Description:     Implements the remote control state machine logic
 Input:           None
 Output:          None
 Return:          None
 Others:          Handles different states of remote control process
                 Transitions between idle, pre-check, certification, signal processing and result sending
                 Calls corresponding handler functions based on current state
*************************************************/
static void RemoteControlStateMachine(void)
{
#if 1  // TODO: 适配新的远控信号
    RemoteControlGetSignalValue(&g_remoteControlSignalInfo);
#endif
//    RemoteControlForbidSleepCheck();
    switch(g_remoteControlTotalState)
    {
        case RemoteControlStateIdle:
            break;

        case RemoteControlStatePreCheck:
            RemoteControlPreCheckProcess();
            break;

        case RemoteControlStateProcessSignal:
            RemoteControlHandleSignalProcess();
            break;

        case RemoteControlStateSendResult:
            RemoteControlSendResultProcess();
            break;

        default:
            break;
    }
}

/*************************************************
 Function:        RemoteControlCmdProcess
 Description:     Processes incoming remote control commands
 Input:           None
 Output:          None
 Return:          None
 Others:          Validates received command packets
                 Handles different command types (control, wake, OTA)
                 Initiates state machine for valid control commands
                 Sends acknowledgment and error responses
*************************************************/
static void RemoteControlCmdProcess(void)
{
    if((g_remoteControlReceivePack.aid == REMOTE_CONTROL_AID) && 
    (g_remoteControlReceivePack.mid == REMOTE_CONTROL_MID)&&
    ((g_remoteControlReceivePack.subcommand & 0x7f)== REMOTE_CONTROL_SUBCMD))
    {
        RemoteControlSendAck();
        RemoteControlStartStateMachine();
    }
    return;
}



/*************************************************
 Function:        RemoteControlSetTotalState
 Description:     Sets the total state of remote control system
 Input:           state - New state to be set
 Output:          None
 Return:          None
 Others:          Updates the global remote control state variable
                 Used to transition between different operational states
                 Supports state machine workflow control
*************************************************/
static void RemoteControlSetTotalState(RemoteControlState_t state)
{
    g_remoteControlTotalState = state;
}

/*************************************************
 Function:        RemoteControlGetTotalState
 Description:     Gets the current total state of remote control system
 Input:           None
 Output:          None
 Return:          RemoteControlState_t - Current state of the remote control system
 Others:          Returns the global remote control state variable
                 Used by other components to check current operational state
                 Supports state monitoring and conditional execution
*************************************************/
RemoteControlState_t RemoteControlGetTotalState(void)
{
    return g_remoteControlTotalState;
}

/*******************************************************************************
* Function    : RemoteControlStartStateMachine
* Description : Initiates the remote control state machine
*               Checks if the system is in idle state, and if so, prepares for
*               command processing by copying received data, extracting ECU ID
*               and command ID, then transitioning to pre-check state.
*               If not in idle state, sets error code and sends result.
* Input       : None
* Output      : None
* Return      : None
* Others      : Modifies global state variables and local buffer
*******************************************************************************/
static void RemoteControlStartStateMachine(void)
{
  if(g_remoteControlTotalState == RemoteControlStateIdle)
  {
        g_remoteControlCmd = g_remoteControlDataBuffer[2];
        
        if(((g_remoteControlDataBuffer[0] == REMOTE_CONTROL_VEHICLE_KANGMINGSI) ||\
        (g_remoteControlDataBuffer[0] == REMOTE_CONTROL_VEHICLE_YUCHAI) ||\
        (g_remoteControlDataBuffer[0] == REMOTE_CONTROL_VEHICLE_MAN) ||\
        (g_remoteControlDataBuffer[0] == REMOTE_CONTROL_VEHICLE_WEICHAI) ||\
        (g_remoteControlDataBuffer[0] == REMOTE_CONTROL_VEHICLE_XICHAI) ||\
        (g_remoteControlDataBuffer[0] == REMOTE_CONTROL_VEHICLE_ELEC)) &&\
        (g_remoteControlDataBuffer[1] == 0x01)) 
        {
            g_remoteControlCmdBussiness = E_REMOTE_CONTROL_BUSSINESS_LOCK;
        }
        else if ((g_remoteControlDataBuffer[0] == 0xCA) && (g_remoteControlDataBuffer[1] == 0x01))
        {
            g_remoteControlCmdBussiness = E_REMOTE_CONTROL_BUSSINESS_BATT_TEMP;
        }
        else
        {
            g_remoteControlCmdBussiness = E_REMOTE_CONTROL_BUSSINESS_NO;
        }
        TBOX_PRINT("valid cmd, Bussiness: %x-%x\r\n", g_remoteControlCmd, g_remoteControlCmdBussiness);
        RemoteControlSetKeepWakeFlag(RemoteControlWakeUpFlag_Keep_e);
        RemoteControlSetTotalState(RemoteControlStatePreCheck);
  }
  else
  {
      RemoteControlSetTotalState(RemoteControlStateSendResult);
  }
        
}
/*******************************************************************************
* Function    : RemoteControlPreCheckProcess
* Description : Performs pre-check for remote control commands
*               Finds the command entry based on ECU ID and command ID,
*               executes the corresponding check function if available,
*               and transitions to certification state if check passes.
* Input       : None
* Output      : None
* Return      : RemoteControlProcessResult_t - Success, Fail, or Processing status
* Others      : Sets global state based on check result
*******************************************************************************/
static RemoteControlProcessResult_t RemoteControlPreCheckProcess(void)
{
    RemoteControlProcessResult_t result = RemoteControlResult_Success_e;
    switch(g_remoteControlCmdBussiness)
    {
      case E_REMOTE_CONTROL_BUSSINESS_LOCK:
          {
            if(g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_ACTIVE) /*激活指令*/
            {
                if(g_remoteControlSignalInfo.Rx_ActiveState == 0x01) /*实车已激活*/
                {
                    g_remoteControlErrorCode = 0;
                    RemoteControlSetTotalState(RemoteControlStateSendResult);
                }
                else
                {
                    RemoteControlSetTotalState(RemoteControlStateProcessSignal);
                    TBOX_PRINT("go RemoteControlStateProcessSignal\r\n");
                } 
            }
            else if(g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_INACTIVE) /*取消激活指令*/
            {
                if(g_remoteControlSignalInfo.Rx_ActiveState == 0x00) /*实车已是未激活*/
                {
                    g_remoteControlErrorCode = 0;
                    RemoteControlSetTotalState(RemoteControlStateSendResult);
                }
                else
                {
                    RemoteControlSetTotalState(RemoteControlStateProcessSignal);
                } 
            }
            else if(g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_LOCK) /*锁车指令*/
            {   
                if(g_remoteControlSignalInfo.Rx_LockState == 0x01) /*实车已是锁车*/
                {
                    g_remoteControlErrorCode = 0;
                    RemoteControlSetTotalState(RemoteControlStateSendResult);
                }
                else if((g_remoteControlSignalInfo.Rx_VcuResponse == 0x03) && (g_remoteControlSignalInfo.Rx_ActiveState == 0x01)) /*实车握手成功*/
                {
                    RemoteControlSetTotalState(RemoteControlStateProcessSignal);
                }
                else
                {
                      /*上报失败原因*/
                      if(g_remoteControlSignalInfo.Rx_VcuResponse != 0x03)
                      {
                        g_remoteControlErrorCode |= REMOTE_CONTROL_ERR_CODE_HANDSHAKE_ERR;
                      }
                      if(g_remoteControlSignalInfo.Rx_ActiveState != 0x01)
                      {
                        g_remoteControlErrorCode |= REMOTE_CONTROL_ERR_CODE_ACTIVE_ERR;
                      }
                      RemoteControlSetTotalState(RemoteControlStateSendResult);
                } 
            }
            else if(g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_UNLOCK) /*解锁指令*/
            {
                if(g_remoteControlSignalInfo.Rx_LockState == 0x00) /*实车已是解锁*/
                {
                    g_remoteControlErrorCode = 0;
                    RemoteControlSetTotalState(RemoteControlStateSendResult);
                }
                else if((g_remoteControlSignalInfo.Rx_VcuResponse == 0x03) && (g_remoteControlSignalInfo.Rx_ActiveState == 0x01)) /*实车握手成功*/
                {
                    RemoteControlSetTotalState(RemoteControlStateProcessSignal);
                }
                else
                {
                      /*上报失败原因*/
                      if(g_remoteControlSignalInfo.Rx_VcuResponse != 0x03)
                      {
                        g_remoteControlErrorCode |= REMOTE_CONTROL_ERR_CODE_HANDSHAKE_ERR;
                      }
                      if(g_remoteControlSignalInfo.Rx_ActiveState != 0x01)
                      {
                        g_remoteControlErrorCode |= REMOTE_CONTROL_ERR_CODE_ACTIVE_ERR;
                      }
                      RemoteControlSetTotalState(RemoteControlStateSendResult);
                } 
            }
            break;
          }
      case E_REMOTE_CONTROL_BUSSINESS_BATT_TEMP:
          {
            if(g_remoteControlCmd == 0x01)  /*只有开启*/
            {
                /*判断进入的前置条件*/
                if((g_remoteControlSignalInfo.BMS_ST2_SOC > 30) &&\
                   (g_remoteControlSignalInfo.B2T_RQ_SetMode_A == 0x02) &&\
                   (g_remoteControlSignalInfo.Tx_DB_MaxErrLevel < 0x03))
                {
                    RemoteControlSetTotalState(RemoteControlStateProcessSignal);
                }
                else
                {
                   /*上报不满足原因*/
                  if(g_remoteControlSignalInfo.BMS_ST2_SOC <= 30)
                  {
                    g_remoteControlErrorCode |= REMOTE_CONTROL_ERR_CODE_SOC;
                  }
                  
                  if(g_remoteControlSignalInfo.B2T_RQ_SetMode_A != 0x02)
                  {
                    g_remoteControlErrorCode |= REMOTE_CONTROL_ERR_CODE_SET_MODE;
                  }
                  
                  if(g_remoteControlSignalInfo.Tx_DB_MaxErrLevel >= 0x03)
                  {
                    g_remoteControlErrorCode |= REMOTE_CONTROL_ERR_CODE_MAX_ERR;
                  }
                  RemoteControlSetTotalState(RemoteControlStateSendResult);
                }
            }
            break;
          }
      default:
          {
            break; 
          }
    }
    return result;
}



/*******************************************************************************
* Function    : RemoteControlHandleSignalProcess
* Description : Handles the state machine flow for remote control signals
*               Manages the CAN signal transmission and response checking
*               for remote control commands in the state machine process.
* Input       : None
* Output      : None
* Return      : None
* Others      : Controls the signal processing phase of the remote control workflow
*******************************************************************************/
static void RemoteControlHandleSignalProcess(void)
{
    uint32_t s_canId = 0U;
    uint8_t VcuResult = 0;
    static uint16_t s_transCount = 0U;             
    static ProcessSignalState_t s_state = PROCESS_SIGNAL_STATE_IDLE; 
    static uint8_t CanBuf[REMOTE_CONTROL_SEND_CAN_SIZE] = {0};
    
    const can0_signal_configure_t *pCan0SignalConfigure = NULL;
    pCan0SignalConfigure = GetCan0SignalConfigure();
 
    switch(s_state)
    {
        case PROCESS_SIGNAL_STATE_IDLE:
          {
            s_canId = 0U;
            s_transCount = 0U;
            s_state = PROCESS_SIGNAL_STATE_NORMAL_SEND;
          }
          break;
        case PROCESS_SIGNAL_STATE_NORMAL_SEND: 
          {
                switch(g_remoteControlCmdBussiness)
                {
                    case E_REMOTE_CONTROL_BUSSINESS_LOCK:
                    {
                       
                        if(g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_ACTIVE)
                        {
                             RemoteCtrlSignalValToCanFrame(CanBuf, &pCan0SignalConfigure->T_Box_ActiveCmd, 0xDC1F);
                        }
                        else  if(g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_INACTIVE)
                        {
                             RemoteCtrlSignalValToCanFrame(CanBuf, &pCan0SignalConfigure->T_Box_ActiveCmd, 0x9836);
                        }
                        else if(g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_LOCK)
                        {
                             RemoteCtrlSignalValToCanFrame(CanBuf, &pCan0SignalConfigure->T_Box_LockCmd, 0x1900);
                        }
                        else if(g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_UNLOCK)
                        {
                             RemoteCtrlSignalValToCanFrame(CanBuf, &pCan0SignalConfigure->T_Box_LockCmd, 0x6D60);
                        }
                        else
                        {
                           g_remoteControlErrorCode = 0xFF;
                        }
                        
                        if((g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_ACTIVE) || (g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_INACTIVE))
                        {
                            s_canId = 0x18FFD4FC;
                            RemoteCtrlSignalTransmit(s_canId,CanBuf,sizeof(CanBuf));
                        }
                        else if((g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_LOCK) || (g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_UNLOCK))
                        {
                            s_canId = 0x18FFD6FC;
                            RemoteCtrlSignalTransmit(s_canId,CanBuf,sizeof(CanBuf));
                        }
                        else
                        {
                            g_remoteControlErrorCode = 0xFF;
                        }
                    }
                    break; 
                    case E_REMOTE_CONTROL_BUSSINESS_BATT_TEMP:
                    {
                        if(g_remoteControlCmd == 0x01)
                        {
                            g_wakeMaintainFlag = 1;/*开始周期发送唤醒保持报文*/
                            WorkFlashVehicleInforStore(E_PARAMETER_INFO_REMOTE_WAKE_MAINTAIN, &g_wakeMaintainFlag, sizeof(g_wakeMaintainFlag));
                        }
                        else
                        {
                            g_remoteControlErrorCode = 0xFF;
                        }
                    }
                    break;    
                    default:
                      break;
                }
                s_state = PROCESS_SIGNAL_STATE_NORMAL_RESULT;
           }
           break;
           case PROCESS_SIGNAL_STATE_NORMAL_RESULT:
              {
                    s_transCount++;
                    switch(g_remoteControlCmdBussiness)
                    {
                        case E_REMOTE_CONTROL_BUSSINESS_LOCK:
                        {
                            if(g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_ACTIVE)
                            {
                                 if(g_remoteControlSignalInfo.Rx_ActiveState == 1)
                                 {
                                     VcuResult = 1;
                                 }
                            }
                            else  if(g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_INACTIVE)
                            {
                                 if(g_remoteControlSignalInfo.Rx_ActiveState == 0)
                                 {
                                     VcuResult = 1;
                                 }
                            }
                            else if(g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_LOCK)
                            {
                                 if(g_remoteControlSignalInfo.Rx_LockState == 1)
                                 {
                                     VcuResult = 1;
                                 }
                            }
                            else if(g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_UNLOCK)
                            {
                                 if(g_remoteControlSignalInfo.Rx_LockState == 0)
                                 {
                                     VcuResult = 1;
                                 }
                            }
                            else
                            {
                               g_remoteControlErrorCode = 0xFF;
                            }
                        }
                        break; 
                        case E_REMOTE_CONTROL_BUSSINESS_BATT_TEMP:
                        {
                            if(g_remoteControlCmd == 0x01)
                            {
                                /*判定如果能收到实车周期报文，则成功*/
                              if(CanCommExist() == 1)
                              {
                                  VcuResult = 1;
                              }
                            }
                            else
                            {
                                g_remoteControlErrorCode = 0xFF;
                            }
                        }
                        break;    
                        default:
                          break;
                    }
                    
                    if(VcuResult == 1)
                    {
                        s_transCount = 0;
                        g_remoteControlErrorCode = 0;
                        RemoteControlSetTotalState(RemoteControlStateSendResult);
                        s_state = PROCESS_SIGNAL_STATE_IDLE;
                    }
                    else if(s_transCount >= 5000/REMOTE_CONTROL_TASK_PERIOD_TIME) /*5s超时*/
                    {
                      s_transCount = 0;
                      if(g_remoteControlCmdBussiness == E_REMOTE_CONTROL_BUSSINESS_LOCK)
                      {
                            if(g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_ACTIVE)
                            {
                                g_remoteControlErrorCode |= REMOTE_CONTROL_ERR_CODE_ACTIVE_ERR;
                            }
                            else  if(g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_INACTIVE)
                            {
                                g_remoteControlErrorCode |= REMOTE_CONTROL_ERR_CODE_INACTIVE_ERR;
                            }
                            else if(g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_LOCK)
                            {
                                g_remoteControlErrorCode |= REMOTE_CONTROL_ERR_CODE_LOCK_ERR;
                            }
                            else if(g_remoteControlCmd == E_REMOTE_CONTROL_LOCKCMD_UNLOCK)
                            {
                                g_remoteControlErrorCode |= REMOTE_CONTROL_ERR_CODE_UNLOCK_ERR;
                            }
                      }
                      else if(g_remoteControlCmdBussiness == E_REMOTE_CONTROL_BUSSINESS_BATT_TEMP)
                      {
                          if(g_remoteControlCmd == 0x01)
                          {
                              g_remoteControlErrorCode |= REMOTE_CONTROL_ERR_CODE_WAKE_ERR;
                              TBOX_PRINT("g_remoteControlErrorCode: %x\r\n", g_remoteControlErrorCode);
                          }
                          else
                          {
                              g_remoteControlErrorCode = 0xFF;
                          }
                      }
                      else
                      {
                        g_remoteControlErrorCode = 0xFF;
                      }
                      s_state = PROCESS_SIGNAL_STATE_IDLE;
                      RemoteControlSetTotalState(RemoteControlStateSendResult);
                    }
                  
                  }
               break;
               default:
                    break;
    
    
    }
    
}
/*******************************************************************************
* Function    : RemoteControlSendResultProcess
* Description : Sends the execution result of remote control commands
*               Prepares response messages based on execution results and
*               transmits them to the remote client.
* Input       : None
* Output      : None
* Return      : None
* Others      : Resets system state after completing the result transmission
*******************************************************************************/
static void RemoteControlSendResultProcess(void)
{
    RemoteControlSendResult(g_remoteControlErrorCode);
    g_remoteControlErrorCode = 0;
    if(g_wakeMaintainFlag == 0)
    {
        RemoteControlSetKeepWakeFlag(RemoteControlWakeUpFlag_NotKeep_e);
    }
   
    RemoteControlSetTotalState(RemoteControlStateIdle);
}

/*******************************************************************************
 * Function    RemoteControlSendResult
 * Description Send remote control command execution result to the remote client
 * Input       None
 * Output      None
 * Return      None
 * Others      This function prepares the response packet with error code, ECU ID and command ID,
 *              then transmits it through MPU HAL UART interface
*******************************************************************************/
static void RemoteControlSendResult(uint16_t ErrorCode)
{
    g_remoteControlSendPack.aid = REMOTE_CONTROL_AID;
    g_remoteControlSendPack.mid = REMOTE_CONTROL_MID;
    g_remoteControlSendPack.subcommand = REMOTE_CONTROL_SUBCOMMAND_RESULT;

    memset(g_remoteControlSendBuffer,0,sizeof(g_remoteControlSendBuffer));
    g_remoteControlSendPack.dataBufferSize = sizeof(g_remoteControlSendBuffer);
    g_remoteControlSendBuffer[0] = g_remoteControlDataBuffer[0];
    g_remoteControlSendBuffer[1] = 0x03; 
    g_remoteControlSendBuffer[2] = (uint8_t)ErrorCode;        
    g_remoteControlSendBuffer[3] = (uint8_t)(ErrorCode >> 8); 
    TBOX_PRINT("RemoteControlSendResult: %x-%x-%x\r\n", g_remoteControlSendPack.subcommand,g_remoteControlSendBuffer[2],g_remoteControlSendBuffer[3]);

    g_remoteControlSendPack.pDataBuffer = g_remoteControlSendBuffer;
    g_remoteControlSendPack.dataLength = 4U;
    MpuHalTransmit(g_remoteControlUartHandle,&g_remoteControlSendPack,MPU_HAL_UART_MODE);
}

/*******************************************************************************
 * Function    RemoteControlSendAck
 * Description Send acknowledgment response to the remote client
 * Input       cmdType - Command type to be acknowledged
 * Output      None
 * Return      None
 * Others      This function prepares and transmits an acknowledgment packet
 *              with the specified command type, then transitions the system to idle state
*******************************************************************************/
static void RemoteControlSendAck(void)
{ 
    g_remoteControlSendPack.aid = REMOTE_CONTROL_AID;
    g_remoteControlSendPack.mid = REMOTE_CONTROL_MID;
    g_remoteControlSendPack.subcommand = REMOTE_CONTROL_SUBCOMMAND_ACK;

    memset(g_remoteControlSendBuffer,0,sizeof(g_remoteControlSendBuffer));
    g_remoteControlSendPack.dataBufferSize = sizeof(g_remoteControlSendBuffer);
    g_remoteControlSendBuffer[0] = g_remoteControlDataBuffer[0];
    g_remoteControlSendBuffer[1] = REMOTE_CONTROL_ACK_CTRL_CMD;
    g_remoteControlSendBuffer[2] = 0x00;
    g_remoteControlSendBuffer[3] = 0x00;
    g_remoteControlSendPack.pDataBuffer = g_remoteControlSendBuffer;
    g_remoteControlSendPack.dataLength = 4U;
    MpuHalTransmit(g_remoteControlUartHandle,&g_remoteControlSendPack,MPU_HAL_UART_MODE);
}

/*************************************************
 Function:        RemoteControlSetKeepWakeFlag
 Description:     Set the keep wake flag for remote control module
 Input:           keepWakeFlag - Flag value to set
 Output:          None
 Return:          None
 Others:          Updates the global keep wake flag variable
 *************************************************/
static void RemoteControlSetKeepWakeFlag(uint8_t keepWakeFlag)
{
    g_remoteControlKeepWakeFlag = keepWakeFlag;
}

/*************************************************
 Function:        RemoteControlGetKeepWakeFlag
 Description:     Get the current keep wake flag value
 Input:           None
 Output:          None
 Return:          uint8_t - Current keep wake flag value
 Others:          Returns the global keep wake flag variable
 *************************************************/
uint8_t RemoteControlGetKeepWakeFlag(void)
{
    return g_remoteControlKeepWakeFlag;
}

/*************************************************
  Function:        RemoteControlGetOtaFlag
  Description:     Get the current OTA flag value
  Input:           None
  Output:          None
  Return:          uint8_t - Current OTA flag value
  Others:          Returns the global remote control OTA flag variable
 *************************************************/
uint8_t RemoteControlGetOtaFlag(void)
{
    return g_remoteControlOTAFlag;
}


