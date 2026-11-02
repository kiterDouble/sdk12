/*************************************************
 Copyright © 2025 SiRun (AnHui) . All rights reserved.
 File Name: canHal.c
 Author:
 Created Time:
 Description:
 Others:
*************************************************/
/****************************** include ***************************************/
#include "stdbool.h"
#include "canHal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "logHal.h"
#include <string.h>
#include "event_groups.h"
#include "autosarNmSdk.h"
#include "taskDiagnostic.h"
#include "Can_Hal.h"
#include "Gpio_Hal.h"
#include "Core_Hal.h"

/****************************** Macro Definitions ******************************/
#define CAN_DRIVER_HAL_HANDLE_INSTANSE_MAX  (10)
#define CAN_CHANNEL_NUMBER_MAX              (4)
#define ALL_CAN_RX_BUFFER_SIZE              (64)
#define ALL_CAN_TX_BUFFER_SIZE              (120U)
#define CAN_SEND_MAIL_MAX_NUMBER            (4)
#define CAN_TX_BUFFER_SIZE                  (32)
#define CANFD_ENABLE_CONFIG                 (1)
#define CAN_TX_EVT_INIT_DONE                (1U << 0)
#define CAN_NM_WAKEUP_ID_MIN                (0x500)
#define CAN_NM_WAKEUP_ID_MAX                (0x5FF)
#define CAN_TX_ENQUEUE_TIMEOUT_MS           (5U)
#define CAN_TX_BUSY_MAX_RETRY               (2U)
#define CAN_TX_BUSY_RETRY_DELAY_MS          (1U)
#define CAN_OTA_TX_MB_IDX                   (0U)
#define CAN_TX_MB_IDLE_POLL_DELAY_MS        (1U)
#define CAN_TX_MB_IDLE_TIMEOUT_MS           (10U) 

#ifndef BUSY
#define BUSY -6
#endif

#define CAN_USER_IRQ_EN_MASK     (CAN_CTRL1_EIE_Msk | CAN_CTRL1_BEIE_Msk | \
    CAN_CTRL1_RIE_Msk | CAN_CTRL1_TPIE_Msk | CAN_CTRL1_TSIE_Msk)

/****************************** Type Definitions ******************************/
typedef enum
{
    E_CAN_MODE_STANDBY = 0,
    E_CAN_MODE_NORMAL = 1,
} CanModeType_e;

typedef struct {
    uint32_t reqId;
    uint8_t  channel;
} EcuMonitorNode_t;

typedef struct
{
    int8_t txMsgEnable;
    int8_t AppMsgEnable;
    uint8_t BusErrorState;
    uint8_t busOffCount;
    uint8_t transmitFinishedFlag;
    uint8_t busErrorAppDiableFlag;
    uint8_t busNoAckErrorState; 
    uint8_t txFailState;
    uint8_t loadHighFlag;
    uint8_t noAckError;
    uint8_t stuffError;
    uint8_t nmNoackState;
    uint32_t noAckTimeCount;
    uint32_t txStartTimeCount;
    uint32_t txTimeOutCount;
    uint32_t loadMsgCount;
    uint32_t loadTimeCount;
    typeCanBusOffEventCallBack busOffEventFun;
    typeCanTxRxHalCallBackPtr canRxCallFun;
    typeCanTxRxHalCallBackPtr canTxCallFun;
    typeCanRxHalFilterCallBackPtr rxFilterFun;
    typeCanNoAckStuffErrorRecoverEvent errorRecoverFun;
} CanManage_t;

typedef struct
{
    uint8_t canUseFlag;
    uint8_t canRxFlag;
    uint8_t enableSendLoopBack;
    CanHalMsg_t *pMsgBufferRx;
    uint16_t msgRxBufferSize;
    uint8_t msgRxIndexIn;
    QueueHandle_t RxQueueHandle;
    CanHalMsgFilter_t filter;
    typeCanTxHalCallBackPtr pTxCallBackFun;
    typeCanRxHalFilterCallBackPtr pFilerFun;
} CanBufferHal_t;

typedef struct
{
    bool canFDFormatBrs;
    bool canFDFormat;
    bool extended;
    uint32_t canId;
    uint8_t data[64];
    uint8_t dlc;
    uint32_t hwTimeStamp;
} stc_canfd_msg_t;

typedef struct
{
    uint8_t msgRxIndexIn;
    QueueHandle_t rxQueueHandle;
    CanHalMsg_t msgRxBuffer[ALL_CAN_RX_BUFFER_SIZE];
} AllCanRxBuffer_t;

typedef struct
{
    uint16_t txIndexIn;
    uint16_t txIndexOut;
    stc_canfd_msg_t canMsg[CAN_TX_BUFFER_SIZE];
} can_tx_msg_buffer_t;

typedef struct
{
    CanHalMsg_t   msgTxBuffer[ALL_CAN_TX_BUFFER_SIZE];
    QueueHandle_t txQueueHandle;
    QueueHandle_t freeIdxQueueHandle;
    uint8_t       inited;
} CanHalTxBuffer_t;

/****************************** Global Variables ******************************/
CanManage_t g_driverCanManage[CAN_CHANNEL_NUMBER_MAX];
static uint8_t g_canWakeUpFlag[CAN_CHANNEL_NUMBER_MAX];
static CanBufferHal_t g_can0DriverBuffer[CAN_DRIVER_HAL_HANDLE_INSTANSE_MAX];
static CanBufferHal_t g_can1DriverBuffer[CAN_DRIVER_HAL_HANDLE_INSTANSE_MAX];
static CanBufferHal_t g_can2DriverBuffer[CAN_DRIVER_HAL_HANDLE_INSTANSE_MAX];
static CanBufferHal_t g_can3DriverBuffer[CAN_DRIVER_HAL_HANDLE_INSTANSE_MAX];
static SemaphoreHandle_t g_canTxMutex[CAN_CHANNEL_NUMBER_MAX];
/* RX队列满丢包统计，用于调试和监控 */
static volatile uint32_t g_canRxDropCount[CAN_CHANNEL_NUMBER_MAX] = {0};

static CanBufferHal_t *g_canDriverBufferList[] = {
    g_can0DriverBuffer,
    g_can1DriverBuffer,
    g_can2DriverBuffer,
    g_can3DriverBuffer,
};

static AllCanRxBuffer_t g_allCanRxBuffer;
/* 全局RX队列初始化完成标志，用于确保中断处理前队列已就绪 */
static volatile uint8_t g_allCanRxBufferInited = 0;
uint8_t g_canTestModeFlag[CAN_CHANNEL_NUMBER_MAX] = {0};
static volatile unsigned int CAN0_BusOffIntFlag = 0U; // 定义CAN0 的BusOff中断进入标志
static volatile unsigned int CAN1_BusOffIntFlag = 0U; // 定义CAN1 的BusOff中断进入标志
static volatile unsigned int CAN2_BusOffIntFlag = 0U; // 定义CAN2 的BusOff中断进入标志
static volatile unsigned int CAN3_BusOffIntFlag = 0U; // 定义CAN3 的BusOff中断进入标志
static uint32_t g_receiveCanNmFlag = 0U;
static uint8_t g_canOtaMode = 0U;

static const EcuMonitorNode_t g_monitorEcuList[] = {
    // PCAN
    {0x727, TBOX_CAN_CHANNEL_2}, // SRS
    {0x7F1, TBOX_CAN_CHANNEL_2}, 
    {0x710, TBOX_CAN_CHANNEL_2}, // BCS
    {0x7E0, TBOX_CAN_CHANNEL_2}, // EMS
    {0x765, TBOX_CAN_CHANNEL_2}, // MFS
    {0x7E1, TBOX_CAN_CHANNEL_2}, // TCU
    // ACAN
    {0x731, TBOX_CAN_CHANNEL_2}, // PAS
    {0x714, TBOX_CAN_CHANNEL_2}, // EPS
    {0x73E, TBOX_CAN_CHANNEL_2}, // FR
    {0x740, TBOX_CAN_CHANNEL_2}, // IFC
    // BCAN
    {0x72C, TBOX_CAN_CHANNEL_2}, // HVACF
    {0x72B, TBOX_CAN_CHANNEL_2}, // AVNT
    {0x755, TBOX_CAN_CHANNEL_2}, // ETC
    {0x728, TBOX_CAN_CHANNEL_2}, // IID
    {0x720, TBOX_CAN_CHANNEL_2}, // IBCM
    {0x748, TBOX_CAN_CHANNEL_2}, // WCM
    {0x718, TBOX_CAN_CHANNEL_2}, // FLDCM
    {0x719, TBOX_CAN_CHANNEL_2}, // FRDCM
    {0x73B, TBOX_CAN_CHANNEL_2}, // HVSM
    {0x73A, TBOX_CAN_CHANNEL_2}, // RCP
    {0x724, TBOX_CAN_CHANNEL_2}, // PLGM
    // TCAN & DCAN
    {0x72D, TBOX_CAN_CHANNEL_2}, // TBOX
    {0x74F, TBOX_CAN_CHANNEL_2}, // GWM
    {0x67F, TBOX_CAN_CHANNEL_2}, // ECU_BCAN_ICM}
    {0x73C, TBOX_CAN_CHANNEL_2}, // DCU
    {0x7DF, TBOX_CAN_CHANNEL_2},
};

static EventGroupHandle_t g_canTxEvt = NULL;
static CanHalTxBuffer_t g_allCanTxBuffer;

/****************************** Function Declarations *************************/
void CanRxInterruptProcessMsg(uint8_t canChannel, stc_canfd_msg_t *pstcCanFDmsg, uint8_t txState);
static void CanHalSetReceiveCanNmFlag(uint32_t canId);
static uint8_t CanHandleToChannel(int16_t canHandle, uint8_t *pOutCh);
static void CanHalTxInit(void);
static int16_t CanHalTxWaitInitDone(TickType_t waitTicks);
static void CheckAndSetExternalTester(uint32_t canId, uint8_t channel, uint8_t txFlag);
static int16_t CanInit(uint8_t u8Channel, uint8_t canfdFlag, CanBaudType_e u8BaudType, CanBaudType_e dataBandType, uint8_t mode);
static void CAN_IRQCallback(void *Args);

/****************************** Public Function Implementations ***************/
/*****************************************************************************
 * Function:        CAN0_Init
 * Description:     Configure and initialize CAN module according to canIndex
 * Input:           canIndex          - CAN instance index (0: CAN0, 1: CAN1, 2: CAN2)
 *                  canfdFlag         - CAN FD enable flag
 *                  idBandrate        - Arbitration(ID) phase baud rate type
 *                  dataBandrateRate  - Data phase baud rate type (CAN FD only)
 * Output:          None
 * Return:          None
 * Others:          1) CAN clock source uses external 40 MHz crystal (PLL derived as configured)
 *                  2) CAN error counter behavior:
 *                     - 96 < error_count <= 127 : Error warning
 *                     - error_count > 127       : Error passive; if transmitting, may enter Bus-Off
 *                     - error_count reaches 255 : Counter wraps to 0
 ****************************************************************************/
void CAN0_Init(uint8_t canIndex, uint8_t canfdFlag, CanBaudType_e idBandrate, CanBaudType_e dataBandrateRate)
{
    Can_HalConfigType canCfg = {0};
    Can_BaudrateConfigType baudrateCfg = {0};
    Can_FiltersConfigType filtersCfg = {0};

    /* 500K, 80%, NQ=80, TSEG2=16, SJW=16 @ 40MHz */
    baudrateCfg.NormalBitrate.Presc = 0;
    baudrateCfg.NormalBitrate.Seg1 = 88;
    baudrateCfg.NormalBitrate.Seg2 = 29;
    baudrateCfg.NormalBitrate.Sjw = 29;

    if (canfdFlag) {
        baudrateCfg.DataBitrate.Presc = 0;
        baudrateCfg.DataBitrate.Seg1 = 19;
        baudrateCfg.DataBitrate.Seg2 = 8;
        baudrateCfg.DataBitrate.Sjw = 8;
        baudrateCfg.SspOffset = 18U;
        canCfg.FdEn = TRUE;
        canCfg.FdIsoEn = TRUE;
    } else {
        canCfg.FdEn = FALSE;
    }
    
    canCfg.BaudrateConfigPtr = &baudrateCfg;
    canCfg.Filters = filtersCfg;
    canCfg.Filters.FiltersNum = 0U;
    canCfg.IrqCallback = CAN_IRQCallback;
    canCfg.CommonIrqEnMasks = CAN_USER_IRQ_EN_MASK; /* 开启中断 */
    canCfg.EccEn = TRUE;                            /* 使能消息RAM ECC*/
    canCfg.RxOverWrite = TRUE;                      /* RX FIFO消息覆盖使能配置*/
    canCfg.BOREC = FALSE;                           /* FALSE表示不禁止自恢复*/
    canCfg.TxSecAmount = CAN_TX_SEC_ALL;

    /* Enable RX hardware timestamping (internal TSC source).
     * Must be configured while controller is in STOP state (before
     * Can_Hal_SetControllerState(CAN_STATE_RUNNING)). */
    Can_TimeStampType tsCfg;
    tsCfg.En = TRUE;
    tsCfg.ExtClkSrc = FALSE;
    tsCfg.PosEnd = TRUE;       /* timestamp at frame EOF */
    tsCfg.ExtClkDiv = 0U;

    switch (canIndex)
    {
    case 0:
        /* GPIO 初始化：使用 Gpio_Hal */
        Gpio_Hal_SetMuxMode(CANFD1_RX_PORT, CANFD1_RX_PIN, CANFD1_RX_MUX);
        Gpio_Hal_SetMuxMode(CANFD1_TX_PORT, CANFD1_TX_PIN, CANFD1_TX_MUX);
        Gpio_Hal_SetMuxMode(CAN1FD_STB_PORT, CAN1FD_STB_PIN, CAN1FD_STB_PIN_MUX);
        Gpio_Hal_SetPinDirection(CAN1FD_STB_PORT, CAN1FD_STB_PIN, GPIO_OUTPUT_DIRECTION);
        Gpio_Hal_WritePin(CAN1FD_STB_PORT, CAN1FD_STB_PIN, GPIO_HIGH_LEVEL);

        Can_Hal_Init(0, &canCfg);
        Can_Hal_ConfigTimeStamp(0, &tsCfg);
        Can_Hal_SetControllerState(0, CAN_STATE_RUNNING);
        Core_Hal_SetIrqPriority(CAN0_IRQn, 0x1);
        Core_Hal_EnableIrq(CAN0_IRQn);
        break;
    case 1:
        Gpio_Hal_SetMuxMode(CANFD2_RX_PORT, CANFD2_RX_PIN, CANFD2_RX_MUX);
        Gpio_Hal_SetMuxMode(CANFD2_TX_PORT, CANFD2_TX_PIN, CANFD2_TX_MUX);
        Gpio_Hal_SetMuxMode(CAN2FD_STB_PORT, CAN2FD_STB_PIN, CAN2FD_STB_PIN_MUX);
        Gpio_Hal_SetPinDirection(CAN2FD_STB_PORT, CAN2FD_STB_PIN, GPIO_OUTPUT_DIRECTION);
        Gpio_Hal_WritePin(CAN2FD_STB_PORT, CAN2FD_STB_PIN, GPIO_HIGH_LEVEL);

        Can_Hal_Init(1, &canCfg);
        Can_Hal_ConfigTimeStamp(1, &tsCfg);
        Can_Hal_SetControllerState(1, CAN_STATE_RUNNING);
        Core_Hal_SetIrqPriority(CAN1_IRQn, 0x1);
        Core_Hal_EnableIrq(CAN1_IRQn);
        break;
    case 2:
        /* CANFD3 - PB12_RX, PB13_TX, PTD2_STB */
        Gpio_Hal_SetMuxMode(CANFD3_RX_PORT, CANFD3_RX_PIN, CANFD3_RX_MUX);
        Gpio_Hal_SetMuxMode(CANFD3_TX_PORT, CANFD3_TX_PIN, CANFD3_TX_MUX);
        Gpio_Hal_SetMuxMode(CAN3FD_STB_PORT, CAN3FD_STB_PIN, CAN3FD_STB_PIN_MUX);
        Gpio_Hal_SetPinDirection(CAN3FD_STB_PORT, CAN3FD_STB_PIN, GPIO_OUTPUT_DIRECTION);
        Gpio_Hal_WritePin(CAN3FD_STB_PORT, CAN3FD_STB_PIN, GPIO_HIGH_LEVEL);

        Can_Hal_Init(2, &canCfg);
        Can_Hal_ConfigTimeStamp(2, &tsCfg);
        Can_Hal_SetControllerState(2, CAN_STATE_RUNNING);
        Core_Hal_SetIrqPriority(CAN2_IRQn, 0x1);
        Core_Hal_EnableIrq(CAN2_IRQn);
        break;
    case 3:
        /* CANFD4 - PE12_RX, PE3_TX(CAN3_TX), PTD2_STB */
        Gpio_Hal_SetMuxMode(CANFD4_RX_PORT, CANFD4_RX_PIN, CANFD4_RX_MUX);
        Gpio_Hal_SetMuxMode(CANFD4_TX_PORT, CANFD4_TX_PIN, CANFD4_TX_MUX);
        Gpio_Hal_SetMuxMode(CAN4FD_STB_PORT, CAN4FD_STB_PIN, CAN4FD_STB_PIN_MUX);
        Gpio_Hal_SetPinDirection(CAN4FD_STB_PORT, CAN4FD_STB_PIN, GPIO_OUTPUT_DIRECTION);
        Gpio_Hal_WritePin(CAN4FD_STB_PORT, CAN4FD_STB_PIN, GPIO_HIGH_LEVEL);

        Can_Hal_Init(3, &canCfg);
        Can_Hal_ConfigTimeStamp(3, &tsCfg);
        Can_Hal_SetControllerState(3, CAN_STATE_RUNNING);
        Core_Hal_SetIrqPriority(CAN3_IRQn, 0x1);
        Core_Hal_EnableIrq(CAN3_IRQn);
        break;
    default:
        break;
    }
}

/*****************************************************************************
 * Function:        CAN_IRQCallback
 * Description:     CAN_IRQCallback
 * Input:           None
 * Output:          None
 * Return:          None
 * Others:
 ****************************************************************************/
static void CAN_IRQCallback(void *Args)
{
    Can_IrqCBParams *irqParams = (Can_IrqCBParams *)Args;
    uint32_t interruptFlag = irqParams->CommonIrqFlagMasks;
    uint8_t instance = irqParams->Instance;
    
    Can_MessageInfoType recvMsg = {0};
    uint8_t recvData[64] = {0};
    stc_canfd_msg_t stcCanFDmsg = {0};

    /* ---------- BUS OFF & Error 检测 ---------- */
    if (0U != (interruptFlag & CAN_CTRL1_EIF_Msk))  /* BUSOFF状态发生更改 */
    {
        /* 检查当前是否处于BUSOFF状态 */
        if (CAN_ERR_STATE_BUSOFF == Can_Hal_GetErrorState(instance))   
        {
            g_driverCanManage[instance].busOffCount++;
            if (instance == 0)
            {
                CAN0_BusOffIntFlag = 1;
            }
            else if (instance == 1)
            {
                CAN1_BusOffIntFlag = 1;
                /* AutosarNmSdkStartBusOffTimer(); */
            }
            else if (instance == 2)
            {
                CAN2_BusOffIntFlag = 1;
            }
            else if (instance == 3)
            {
                CAN3_BusOffIntFlag = 1;
            }
        }
        else
        {
            if (instance == 0)
            {
                CAN0_BusOffIntFlag = 0;
            }
            else if (instance == 1)
            {
                CAN1_BusOffIntFlag = 0;
            }
            else if (instance == 2)
            {
                CAN2_BusOffIntFlag = 0;
            }
            else if (instance == 3)
            {
                CAN3_BusOffIntFlag = 0;
            }
        }
    }
    
    if (0U != (interruptFlag & CAN_CTRL1_BEIE_Msk)) /* 总线错误标志检测 */
    {
        // do nothing
    }

    /* CAN 发送完成中断 */
    if ((interruptFlag & CAN_CTRL1_TPIE_Msk) != 0U)  
    {
        g_driverCanManage[instance].busErrorAppDiableFlag = 0x00;
    }

    /* ---------- 接收处理 ---------- */
    if (0U != (interruptFlag & CAN_CTRL1_RIF_Msk))  /* 有新消息写入RX FIFO */
    {
        /* 循环读取直至FIFO为空 */
        while (1) 
        {
            recvMsg.MsgDataPtr = recvData;
            /* 读取接收缓冲区，若不成功则说明FIFO已空 */
            if (STATUS_SUCCESS != Can_Hal_ReadRxBuffer(instance, 0, &recvMsg)) 
            {
                break;  
            }

            stcCanFDmsg.canFDFormatBrs = recvMsg.MsgBrs;
            stcCanFDmsg.canFDFormat = recvMsg.MsgFdf;
            stcCanFDmsg.extended = (recvMsg.MsgIdType == CAN_EXTENDED_MSG);
            stcCanFDmsg.canId = recvMsg.MsgId;
            stcCanFDmsg.dlc = recvMsg.MsgDataLen;
            memcpy(stcCanFDmsg.data, recvMsg.MsgDataPtr, recvMsg.MsgDataLen);
            stcCanFDmsg.hwTimeStamp = recvMsg.MsgRts;

            if (instance == 1) {
                CanHalSetReceiveCanNmFlag(recvMsg.MsgId);
            }
            CanRxInterruptProcessMsg(instance, &stcCanFDmsg, 0);
        }
    }
}

/*****************************************************************************
 * Function:        CheckCanIdIsSelfSend
 * Description:     Check whether the CAN ID is in the self-transmit filter list
 * Input:           canId - CAN identifier (standard ID assumed)
 * Output:          None
 * Return:          0 - CAN ID is in self-send list
 *                  1 - CAN ID is NOT in self-send list
 * Others:          Current self-send list includes: 0x316, 0x317, 0x36C, 0x406, 0x425,
 *                  0x536, 0x537, 0x53C, 0x53D, 0x53E, 0x53F, 0x591, 0x686
 ****************************************************************************/
static int16_t CheckCanIdIsSelfSend(uint32_t canId)
{
    if (canId == 0x316)
    {
        return 0;
    }
    if (canId == 0x317)
    {
        return 0;
    }
    if (canId == 0x36C)
    {
        return 0;
    }
    if (canId == 0x406)
    {
        return 0;
    }
    if (canId == 0x425)
    {
        return 0;
    }
    if (canId == 0x536)
    {
        return 0;
    }
    if (canId == 0x537)
    {
        return 0;
    }
    if (canId == 0x53C)
    {
        return 0;
    }
    if (canId == 0x53D)
    {
        return 0;
    }
    if (canId == 0x53E)
    {
        return 0;
    }
    if (canId == 0x53F)
    {
        return 0;
    }
    if (canId == 0x591)
    {
        return 0;
    }
    if (canId == 0x686)
    {
        return 0;
    }
    return 1;
}

/*************************************************
   Function:        CanRxInterruptProcessMsg
   Description:     CAN RX/TX interrupt message processing (ISR context)
   Input:           canChannel   - CAN channel index
                    pstcCanFDmsg - Pointer to received CANFD message structure
                    txState      - 0: RX frame, non-zero: TX event/loopback indication
   Output:          None
   Return:          None
   Others:          Builds unified CAN ID format (bit31 indicates extended ID)
                    Copies payload into local buffer (max 64 bytes)
                    RX path:
                      - Updates load message counter
                      - Applies optional channel RX filter callback
                      - Invokes channel RX callback if registered
                    TX path:
                      - Invokes channel TX callback if registered
                      - Invokes per-handle TX finished callbacks
                    Enqueues message into global RX queue for task dispatching
                    In TX path, self-send frames may be filtered by CheckCanIdIsSelfSend()
 *************************************************/
void CanRxInterruptProcessMsg(uint8_t canChannel, stc_canfd_msg_t *pstcCanFDmsg, uint8_t txState)
{
    uint16_t data;
    uint8_t canData[64];
    uint8_t dlc;
    BaseType_t xHigherPriorityTaskWoken;
    uint32_t canId;
    uint8_t i;
    uint16_t j;

    canId = pstcCanFDmsg->canId;
    if (pstcCanFDmsg->extended)
    {
        canId |= (((uint32_t)0x0000001) << 31);
    }
    {
        dlc = pstcCanFDmsg->dlc;
    }
    for (j = 0; j < pstcCanFDmsg->dlc; j++)
    {
        canData[j] = pstcCanFDmsg->data[j];
    }

    xHigherPriorityTaskWoken = pdFALSE;

    if (txState == 0)
    {
        g_driverCanManage[canChannel].loadMsgCount++;
        if (g_driverCanManage[canChannel].rxFilterFun != NULL)
        {
            if (g_driverCanManage[canChannel].rxFilterFun(canId, canData, dlc) != 0)
            {
                return;
            }
        }

        if (g_driverCanManage[canChannel].canRxCallFun != NULL)
        {
            g_driverCanManage[canChannel].canRxCallFun(canId, canData, dlc);
        }
    }
    else
    {
        // tx call back
        if (g_driverCanManage[canChannel].canTxCallFun != NULL)
        {
            g_driverCanManage[canChannel].canTxCallFun(canId, canData, dlc);
        }
        for (i = 0; i < CAN_DRIVER_HAL_HANDLE_INSTANSE_MAX; i++)
        {
            if (g_canDriverBufferList[canChannel][i].pTxCallBackFun != NULL)
            {
                g_canDriverBufferList[canChannel][i].pTxCallBackFun();
            }
        }
    }
    /* 检查全局RX队列是否已初始化完成，避免中断处理时队列未就绪 */
    if ((g_allCanRxBufferInited != 0) && (g_allCanRxBuffer.rxQueueHandle != NULL))
    {
        if (txState)
        {
            if (CheckCanIdIsSelfSend(canId) != 0)
            {
                return;
            }
        }
        g_allCanRxBuffer.msgRxBuffer[g_allCanRxBuffer.msgRxIndexIn].canId = canId;
        g_allCanRxBuffer.msgRxBuffer[g_allCanRxBuffer.msgRxIndexIn].dlc = dlc;
        memcpy(g_allCanRxBuffer.msgRxBuffer[g_allCanRxBuffer.msgRxIndexIn].canData, canData, dlc);
        g_allCanRxBuffer.msgRxBuffer[g_allCanRxBuffer.msgRxIndexIn].txFlag = txState;
        g_allCanRxBuffer.msgRxBuffer[g_allCanRxBuffer.msgRxIndexIn].hwTimeStamp = pstcCanFDmsg->hwTimeStamp;
        data = (canChannel << 8) + g_allCanRxBuffer.msgRxIndexIn;
        g_allCanRxBuffer.msgRxIndexIn++;
        if (g_allCanRxBuffer.msgRxIndexIn >= ALL_CAN_RX_BUFFER_SIZE)
        {
            g_allCanRxBuffer.msgRxIndexIn = 0;
        }
        xQueueSendFromISR(g_allCanRxBuffer.rxQueueHandle, &data, &xHigherPriorityTaskWoken);
    }
}

/*************************************************
   Function:        CanHalSetCANMode
   Description:     Set CAN transceiver mode for specified channel
   Input:           u8Channel - CAN channel index
                    mode      - CAN mode (E_CAN_MODE_NORMAL / E_CAN_MODE_STANDBY)
   Output:          None
   Return:          0  - Success
                    -1 - Invalid parameter
   Others:          Controls transceiver standby pin via GPIO
                    Mapping between channel and STB pin is platform dependent
 *************************************************/
static int16_t CanHalSetCANMode(uint8_t u8Channel, uint8_t mode)
{
    if ((u8Channel >= CAN_CHANNEL_NUMBER_MAX) || ((mode != E_CAN_MODE_NORMAL) && (mode != E_CAN_MODE_STANDBY)))
    {
        return -1;
    }

    switch (u8Channel)
    {
    case 0:
        Gpio_Hal_WritePin(CAN1FD_STB_PORT, CAN1FD_STB_PIN, (mode == E_CAN_MODE_NORMAL) ? GPIO_LOW_LEVEL : GPIO_HIGH_LEVEL);
        break;
    case 1:
        Gpio_Hal_WritePin(CAN2FD_STB_PORT, CAN2FD_STB_PIN, (mode == E_CAN_MODE_NORMAL) ? GPIO_LOW_LEVEL : GPIO_HIGH_LEVEL);
        break;
    case 2:
        Gpio_Hal_WritePin(CAN3FD_STB_PORT, CAN3FD_STB_PIN, (mode == E_CAN_MODE_NORMAL) ? GPIO_LOW_LEVEL : GPIO_HIGH_LEVEL);
        break;
    case 3:
        Gpio_Hal_WritePin(CAN4FD_STB_PORT, CAN4FD_STB_PIN, (mode == E_CAN_MODE_NORMAL) ? GPIO_LOW_LEVEL : GPIO_HIGH_LEVEL);
        break;
    default:
        break;
    }
    return 0;
}

/*************************************************
   Function:        CanHalSetSleep
   Description:     Apply CAN transceiver mode to all channels
   Input:           mode - Target mode (E_CAN_MODE_NORMAL / E_CAN_MODE_STANDBY)
   Output:          None
   Return:          0  - Success
                    -1 - One or more channels failed to switch mode
   Others:          Iterates all channels and calls CanHalSetCANMode()
                    Returns aggregated result (does not early-exit on failure)
 *************************************************/
static int16_t CanHalSetSleep(uint8_t mode)
{
    uint8_t i;
    int16_t ret = 0, tem;

    for (i = 0; i < CAN_CHANNEL_NUMBER_MAX; i++)
    {
        tem = CanHalSetCANMode(i, mode);
        if (tem != 0)
        {
            ret = -1;
        }
    }
    return ret;
}


/*************************************************
   Function:        SetCanControllerStop
   Description:     Stop CAN controller and disable related interrupts for a channel
   Input:           canChannel - CAN channel index
   Output:          None
   Return:          None
   Others:          Clears CAN interrupts, disables CAN module clock/power
                    Reconfigures RX pin mux for wake-up interrupt (platform specific)
 *************************************************/
static void SetCanControllerStop(uint8_t canChannel)
{
    Can_Hal_SetControllerState(canChannel, CAN_STATE_STANDBY);
}

/*************************************************
   Function:        SetCanControllerStart
   Description:     Start CAN controller and configure pins/modules for a channel
   Input:           canChannel - CAN channel index
   Output:          None
   Return:          None
   Others:          Configures RX/TX pin mux and disables RX pin interrupt flags
                    Resets and enables CAN module in system controller
                    Enables CAN controller instance (CAN_Enable)
                    Platform-specific initialization for multi-channel variants
 *************************************************/
static void SetCanControllerStart(uint8_t canChannel)
{
    CanInit(canChannel, CANFD_ENABLE_CONFIG, E_CAN_500K, E_CAN_2000K, E_CAN_MODE_NORMAL);
}

/*************************************************
   Function:        CanHalSetMode
   Description:     Set CAN system mode (sleep or awake) with optional wake-up interrupt
   Input:           mode - 0: sleep, 1: awake
   Output:          None
   Return:          0  - Success
                    -1 - Invalid mode parameter
   Others:          Sleep (mode=0):
                      - Sets transceiver to standby for all channels
                      - Stops CAN controller modules
                      - If wake-up is enabled per channel, configures RX pin as GPIO
                        and enables falling-edge interrupt for wake-up detection
                    Awake (mode=1):
                      - Starts CAN controller modules for all channels
                      - Sets transceiver to normal mode for all channels
 *************************************************/
int16_t CanHalSetMode(uint8_t mode)
{
    int i = 0;
    if (mode == 0)
    {
        CanHalSetSleep(0);
        for (; i < CAN_CHANNEL_NUMBER_MAX; i++)
        {
            SetCanControllerStop(i);
            Can_Hal_Deinit(i);
        }

        /* 唤醒配置 */
        if (g_canWakeUpFlag[0])
        {
            Gpio_Hal_SetPinDirection(CANFD1_RX_PORT, CANFD1_RX_PIN, GPIO_INPUT_DIRECTION);
            Gpio_Hal_SetPinIntSel(CANFD1_RX_PORT, CANFD1_RX_PIN, PORT_INT_FALLING_EDGE);
            Core_Hal_SetIrqPriority(CANFD1_IRQN, 0x3);
            Core_Hal_EnableIrq(CANFD1_IRQN);
        }
        if (g_canWakeUpFlag[1])
        {
            Gpio_Hal_SetPinDirection(CANFD2_RX_PORT, CANFD2_RX_PIN, GPIO_INPUT_DIRECTION);
            Gpio_Hal_SetPinIntSel(CANFD2_RX_PORT, CANFD2_RX_PIN, PORT_INT_FALLING_EDGE);
            Core_Hal_SetIrqPriority(CANFD2_IRQN, 0x3);
            Core_Hal_EnableIrq(CANFD2_IRQN);
        }
        if (g_canWakeUpFlag[2])
        {
            Gpio_Hal_SetPinDirection(CANFD3_RX_PORT, CANFD3_RX_PIN, GPIO_INPUT_DIRECTION);
            Gpio_Hal_SetPinIntSel(CANFD3_RX_PORT, CANFD3_RX_PIN, PORT_INT_FALLING_EDGE);
            Core_Hal_SetIrqPriority(CANFD3_IRQN, 0x3);
            Core_Hal_EnableIrq(CANFD3_IRQN);
        }
        if (g_canWakeUpFlag[3])
        {
            Gpio_Hal_SetPinDirection(CANFD4_RX_PORT, CANFD4_RX_PIN, GPIO_INPUT_DIRECTION);
            Gpio_Hal_SetPinIntSel(CANFD4_RX_PORT, CANFD4_RX_PIN, PORT_INT_FALLING_EDGE);
            Core_Hal_SetIrqPriority(CANFD4_IRQN, 0x3);
            Core_Hal_EnableIrq(CANFD4_IRQN);
        }
        return 0;
    }
    else if (mode == 1)
    {
        for (; i < CAN_CHANNEL_NUMBER_MAX; i++)
        {
            SetCanControllerStart(i);
        }
        CanHalSetSleep(1);
        return 0;
    }
    return -1;
}

/*************************************************
   Function:        CanInit
   Description:     Initialize CAN controller instance with baudrate and mode
   Input:           u8Channel     - CAN channel index
                    canfdFlag     - CAN FD enable flag
                    u8BaudType    - Arbitration phase baudrate selection
                    dataBandType  - Data phase baudrate selection (CAN FD)
                    mode          - CAN mode (E_CAN_MODE_NORMAL / E_CAN_MODE_STANDBY)
   Output:          None
   Return:          0  - Success
                    -1 - Invalid parameter
   Others:          Calls platform-specific CAN init function per channel
                    Configures transceiver mode via CanHalSetCANMode()
                    Some channels may be not implemented depending on platform
 *************************************************/
static int16_t CanInit(uint8_t u8Channel, uint8_t canfdFlag, CanBaudType_e u8BaudType, CanBaudType_e dataBandType, uint8_t mode)
{
    if (u8Channel >= CAN_CHANNEL_NUMBER_MAX) return -1;
    
    CAN0_Init(u8Channel, canfdFlag, u8BaudType, dataBandType);
    CanHalSetCANMode(u8Channel, mode);

    return 0;
}

/*************************************************
   Function:        CanHalInit
   Description:     Initialize CAN HAL driver management and hardware channels
   Input:           pCanConfig - CAN configuration table (array)
                    CanNum     - Number of CAN configurations in table
   Output:          None
   Return:          0  - Success
                    -1 - Invalid parameter (NULL config or invalid channel)
                    -2 - TX mutex creation failure
   Others:          Initializes per-channel management flags and callback pointers
                    Creates per-channel TX mutex for thread-safe transmission
                    Initializes CAN controller hardware using provided configuration
                    Sets bus-off processing enable flag and bus-off event callback
                    Initializes txStartTimeCount to 0xFFFFFFFF (timer stopped state)
 *************************************************/
int16_t CanHalInit(const CanConfigure_t *pCanConfig, uint8_t CanNum)
{
    uint8_t i;

    for (i = 0; i < CAN_CHANNEL_NUMBER_MAX; i++)
    {
        g_driverCanManage[i].AppMsgEnable = 0;
        g_driverCanManage[i].txMsgEnable = 1;
        g_driverCanManage[i].canRxCallFun = NULL;
        g_driverCanManage[i].rxFilterFun = NULL;
        g_driverCanManage[i].noAckError = 0;
        g_driverCanManage[i].stuffError = 0;
        g_driverCanManage[i].errorRecoverFun = NULL;
        g_canTxMutex[i] = xSemaphoreCreateMutex();
        if (g_canTxMutex[i] == NULL)
        {
            return -2;
        }
        g_canWakeUpFlag[i] = 0;
    }
    g_allCanRxBuffer.rxQueueHandle = NULL;
    if (pCanConfig == NULL) return -1;
    
    for (i = 0; i < CanNum; i++)
    {
        if (pCanConfig[i].canChannel > (CAN_CHANNEL_NUMBER_MAX - 1)) return -1;
        CanInit(pCanConfig[i].canChannel, pCanConfig[i].canFdMode, pCanConfig[i].bandrate, pCanConfig[i].dataBandrate, E_CAN_MODE_NORMAL);
        g_driverCanManage[pCanConfig[i].canChannel].AppMsgEnable = 0;
        g_driverCanManage[pCanConfig[i].canChannel].busOffEventFun = pCanConfig[i].busOffEventFun;
        g_driverCanManage[pCanConfig[i].canChannel].txStartTimeCount = 0xFFFFFFFF;
    }
    return 0;
}


/*************************************************
   Function:        CanHalOpen
   Description:     Open CAN HAL instance and allocate handle for specified channel
   Input:           canChannel - CAN channel index
   Output:          None
   Return:          >=0 - Valid CAN handle (high byte: channel, low byte: instance index)
                    -1  - Failed (invalid channel or no free instance)
   Others:          Searches free instance in g_canDriverBufferList[canChannel][]
                    Uses critical section to protect instance allocation
                    Initializes instance fields (queue handle, loopback flag, filter callback)
 *************************************************/
int16_t CanHalOpen(uint8_t canChannel)
{
    int16_t canHandle;
    uint16_t i;

    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return -1;
    }
    canHandle = -1;
    taskENTER_CRITICAL();
    for (i = 0; i < CAN_DRIVER_HAL_HANDLE_INSTANSE_MAX; i++)
    {
        if (g_canDriverBufferList[canChannel][i].canUseFlag == 0)
        {
            g_canDriverBufferList[canChannel][i].canUseFlag = 1;
            g_canDriverBufferList[canChannel][i].RxQueueHandle = NULL;
            g_canDriverBufferList[canChannel][i].enableSendLoopBack = 0x00;
            g_canDriverBufferList[canChannel][i].pFilerFun = NULL;
            canHandle = i + (256 * canChannel);
            break;
        }
    }
    taskEXIT_CRITICAL();
    return canHandle;
}

/*************************************************
   Function:        CanHalSetRxBuffer
   Description:     Configure RX message buffer and RX queue for specified CAN handle
   Input:           canHandle    - CAN driver handle
                    pMsgBuf      - Message buffer array provided by caller
                    bufMsgCount  - Number of messages in buffer
   Output:          None
   Return:          0  - Success
                    -1 - Invalid handle or channel
                    -2 - Invalid buffer size (bufMsgCount == 0)
                    -3 - Queue creation failure
   Others:          Creates per-instance RX queue with item size of uint8_t index token
                    Initializes RX ring buffer index and assigns caller-provided buffer
                    Caller is responsible for ensuring pMsgBuf has bufMsgCount entries
 *************************************************/
int16_t CanHalSetRxBuffer(int16_t canHandle, CanHalMsg_t *pMsgBuf, uint16_t bufMsgCount)
{
    uint8_t canChannel;
    QueueHandle_t queHandle;
    if (canHandle < 0)
    {
        return -1;
    }

    canChannel = canHandle >> 8;
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX) // invalid channel
    {
        return -1;
    }
    if (bufMsgCount == 0)
    {
        return -2;
    }

    queHandle = xQueueCreate(bufMsgCount,      // The number of items the queue can hold.
                             sizeof(uint8_t)); // The buffer that will hold the queue structure.
    if (0 == queHandle)
    {
        return -3;
    }

    g_canDriverBufferList[canChannel][(canHandle & 0xFF)].RxQueueHandle = queHandle;
    g_canDriverBufferList[canChannel][(canHandle & 0xFF)].msgRxIndexIn = 0;
    g_canDriverBufferList[canChannel][(canHandle & 0xFF)].msgRxBufferSize = bufMsgCount;
    g_canDriverBufferList[canChannel][(canHandle & 0xFF)].pMsgBufferRx = pMsgBuf;
    return 0;
}

/*************************************************
   Function:        CanHalSetFilter
   Description:     Set CAN ID filter configuration for specified CAN handle instance
   Input:           canHandle   - CAN driver handle
                    pMsgFilter  - Filter configuration structure
   Output:          None
   Return:          0  - Success
                    -1 - Invalid handle or channel
                    -2 - NULL filter pointer
   Others:          Copies filter configuration into instance context
                    Filtering is applied in dispatch path when no custom filter function is set
 *************************************************/
int16_t CanHalSetFilter(int16_t canHandle, CanHalMsgFilter_t *pMsgFilter)
{
    uint8_t canChannel;

    if (canHandle < 0)
    {
        return -1;
    }
    canChannel = canHandle >> 8;
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX) // invalid channel
    {
        return -1;
    }
    if (pMsgFilter == NULL)
    {
        return -2;
    }
    memcpy(&(g_canDriverBufferList[canChannel][(canHandle & 0xFF)].filter), pMsgFilter, sizeof(CanHalMsgFilter_t));

    return 0;
}

/*************************************************
   Function:        CanHalSetFilterFun
   Description:     Register custom CAN RX filter callback for specified handle instance
   Input:           canHandle  - CAN driver handle
                    pFilterFun - Filter callback function pointer
   Output:          None
   Return:          0  - Success
                    -1 - Invalid handle or channel
                    -2 - NULL callback pointer
   Others:          When registered, callback is invoked to decide whether to accept message
                    Custom filter has higher priority than static filter range configuration
 *************************************************/
int16_t CanHalSetFilterFun(int16_t canHandle, typeCanRxHalFilterCallBackPtr pFilterFun)
{
    uint8_t canChannel;

    if (canHandle < 0)
    {
        return -1;
    }
    canChannel = canHandle >> 8;
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX) // invalid channel
    {
        return -1;
    }

    if (pFilterFun == NULL)
    {
        return -2;
    }
    g_canDriverBufferList[canChannel][(canHandle & 0xFF)].pFilerFun = pFilterFun;

    return 0;
}

/*************************************************
    Function:        CanHalRegisterTxFinishedCallBackFunction
    Description:     Register TX finished callback for NM (OSEK NM protocol)
    Input:           canHandle     - CAN driver handle
                     pCallBackFun  - Callback function pointer
    Output:          None
    Return:          0  - Success
                     -1 - Invalid handle or channel
    Others:          Used by OSEK NM SDK for TX completion notification
  *************************************************/
int16_t CanHalRegisterTxFinishedCallBackFunction(int16_t canHandle, typeCanTxHalCallBackPtr pCallBackFun)
{
    uint8_t canChannel;

    if (canHandle < 0)
    {
        return -1;
    }

    canChannel = canHandle >> 8;
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return -1;
    }
    g_canDriverBufferList[canChannel][(canHandle & 0xFF)].pTxCallBackFun = pCallBackFun;
    return 0;
}

/*************************************************
    Function:        CanTransmit
    Description:     Transmit CAN message through specified channel
    Input:           u8Channel - CAN channel number
                     id - CAN message ID
                     u8Len - Data length
                     pu8CmdData - Pointer to data buffer
                     fdFlag - CAN FD configuration flag (bit0: FD enable, bit1: BRS enable)
    Output:          None
    Return:          0 on success, negative error code on failure
    Others:          Uses independent mail counter for each channel
  *************************************************/
static int16_t CanTransmit(uint8_t u8Channel, uint32_t id, uint8_t u8Len, const uint8_t *pu8CmdData, uint8_t fdFlag, uint8_t isOta)
{
    Can_MessageInfoType txInfo = {0};
    int16_t ret = -1;

    if ((pu8CmdData == NULL) || (u8Channel >= CAN_CHANNEL_NUMBER_MAX)) return -1;

    txInfo.MsgId = id;
    txInfo.MsgDataLen = u8Len;
    txInfo.MsgDataPtr = (uint8_t *)pu8CmdData;
    txInfo.MsgIdType = (id > 0x7FF) ? CAN_EXTENDED_MSG : CAN_STANDARD_MSG; /* 支持扩展帧类型判断 */
    txInfo.MsgRtr = FALSE;
    txInfo.MsgFdf = (fdFlag & 0x01U) ? TRUE : FALSE;
    txInfo.MsgBrs = (fdFlag & 0x02U) ? TRUE : FALSE;

    taskENTER_CRITICAL();
    if (Can_Hal_WriteTxBuffer(u8Channel, CAN_TX_BUF_SECONDARY, &txInfo) == STATUS_SUCCESS)
    {
        g_driverCanManage[u8Channel].txTimeOutCount = 0;
        ret = 0;
    }
    else
    {
        ret = BUSY;
    }
    taskEXIT_CRITICAL();

    return ret;
}

/*************************************************
   Function:        CanHalGetTimestampTickHz
   Description:     Get the timestamp counter tick rate for a CAN handle
   Input:           canHandle - CAN driver handle
   Output:          None
   Return:          Timestamp tick rate in Hz, or 0 for an invalid handle
   Others:          The AC7842x internal timestamp counter tracks nominal bit
                    time. Keep this value aligned with CAN0_Init bitrate setup.
 *************************************************/
uint32_t CanHalGetTimestampTickHz(int16_t canHandle)
{
    uint8_t canChannel;

    if (canHandle < 0)
    {
        return 0UL;
    }

    canChannel = (uint8_t)(canHandle >> 8);
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return 0UL;
    }

    /* All product CAN channels currently use a fixed 500K nominal rate. */
    return CAN_HAL_TIMESTAMP_TICK_HZ;
}

/*************************************************
   Function:        CanHalReceive
   Description:     Receive CAN message from per-instance RX queue
   Input:           canHandle  - CAN driver handle (high byte: channel, low byte: instance index)
                    timeOut_ms - Receive timeout in milliseconds (portMAX_DELAY for infinite wait)
   Output:          pMsg       - Output buffer for received CAN message
   Return:          0  - Receive successful
                    CAN_ERROR_INVALID_HANDLE - Invalid handle or channel
                    CAN_ERROR_TIMEOUT        - Timeout or queue not ready
   Others:          Receives an index token from RxQueueHandle, then copies the message
                    from instance RX buffer into pMsg
 *************************************************/
int16_t CanHalReceive(int16_t canHandle, CanHalMsg_t *pMsg, uint32_t timeOut_ms)
{
    uint8_t canChannel, index;
    QueueHandle_t queHandle;
    TickType_t tickWait;
    uint8_t data;
    if (canHandle < 0)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    canChannel = canHandle >> 8;
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    // receive
    index = canHandle & 0xFF;
    queHandle = g_canDriverBufferList[canChannel][index].RxQueueHandle;
    if (queHandle == NULL)
    {
        return CAN_ERROR_TIMEOUT;
    }
    if (timeOut_ms != portMAX_DELAY)
    {
        tickWait = pdMS_TO_TICKS(timeOut_ms);
    }
    else
    {
        tickWait = portMAX_DELAY;
    }

    if (xQueueReceive(queHandle, &data, tickWait) != pdPASS)
    {
        return CAN_ERROR_TIMEOUT;
    }

    memcpy(pMsg, (g_canDriverBufferList[canChannel][index].pMsgBufferRx + data), sizeof(CanHalMsg_t));
    return 0;
}

/*************************************************
   Function:        CanHalNmReceive
   Description:     Receive CAN Network Management (NM) message
   Input:           canHandle  - CAN driver handle
                    timeOut_ms - Receive timeout in milliseconds
   Output:          pMsg       - Output buffer for received NM message
   Return:          0  - Receive successful
                    CAN_ERROR_INVALID_HANDLE - Invalid handle or channel
                    CAN_ERROR_TIMEOUT        - Timeout or queue not ready
   Others:          Receives message index from RX queue and copies
                    message from internal RX buffer
 *************************************************/
int16_t CanHalNmReceive(int16_t canHandle, CanHalMsg_t *pMsg, uint32_t timeOut_ms)
{
    uint8_t canChannel;
    uint8_t index;
    QueueHandle_t queHandle;
    TickType_t tickWait;
    uint8_t data;

    if (canHandle < 0)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    canChannel = canHandle >> 8;
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    // receive
    index = canHandle & 0xFF;
    queHandle = g_canDriverBufferList[canChannel][index].RxQueueHandle;
    if (queHandle == NULL)
    {
        return CAN_ERROR_TIMEOUT;
    }
    if (timeOut_ms != portMAX_DELAY)
    {
        tickWait = pdMS_TO_TICKS(timeOut_ms);
    }
    else
    {
        tickWait = portMAX_DELAY;
    }
    if (xQueueReceive(queHandle, &data, tickWait) != pdPASS)
    {
        return CAN_ERROR_TIMEOUT;
    }
    memcpy(pMsg, &(g_canDriverBufferList[canChannel][index].pMsgBufferRx[data]), sizeof(CanHalMsg_t));
    return 0;
}

/*************************************************
   Function:        CanHalNmTransmit
   Description:     Transmit CAN NM message directly via CanTransmit
   Input:           canHandle - CAN driver handle (high byte: channel)
                    canId     - CAN message ID
                    canData   - Pointer to data buffer
                    dlc       - Data length
                    fdFlag    - CAN FD configuration flag
   Output:          None
   Return:          0 on success, negative error code on failure
   Others:          Used by OSEK NM SDK for NM message transmission
                    Adapted from Z20K148 to AC78428 CanTransmit interface
  *************************************************/
int16_t CanHalNmTransmit(int16_t canHandle, uint32_t canId, uint8_t *canData, uint8_t dlc, uint8_t fdFlag)
{
    uint8_t canChannel;
    int16_t ret;

    if (canHandle < 0)
    {
        return -1;
    }
    canChannel = canHandle >> 8;
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return -1;
    }
    if (g_driverCanManage[canChannel].txMsgEnable <= 0)
    {
        return CAN_ERROR_TX_DISABLE;
    }
    if (g_driverCanManage[canChannel].BusErrorState)
    {
        return CAN_ERROR_BUS_ERROR;
    }

    if (g_driverCanManage[canChannel].busNoAckErrorState != 0x00)
    {
        return CAN_ERROR_BUS_ERROR;
    }

    if (g_canTestModeFlag[canChannel])
    {
        return CAN_ERROR_TEST_MODE;
    }
    ret = CanTransmit(canChannel, canId, dlc, canData, fdFlag, 0);
    if (ret == BUSY)
    {
        vTaskDelay(pdMS_TO_TICKS(2));
        ret = CanTransmit(canChannel, canId, dlc, canData, fdFlag, 0);
    }
    if (ret == 0)
    {
        taskENTER_CRITICAL();
        if (g_driverCanManage[canChannel].txStartTimeCount == 0xffffffff)
        {
            g_driverCanManage[canChannel].txStartTimeCount = 0x00;
        }
        taskEXIT_CRITICAL();
    }
    return ret;
}

/*************************************************
   Function:        CanHalRecieveIsTimeOut
   Description:     Check whether CAN message has been received since last check
   Input:           canHandle - CAN driver handle
   Output:          None
   Return:          1  - Message received
                    0  - No message received
                    CAN_ERROR_INVALID_HANDLE - Invalid handle or channel
   Others:          Clears internal canRxFlag after detection
                    Typically used for timeout supervision
 *************************************************/
int16_t CanHalRecieveIsTimeOut(int16_t canHandle)
{
    uint8_t canChannel;
    uint8_t index;
    if (canHandle < 0)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    canChannel = canHandle >> 8;
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    index = canHandle & 0xFF;
    if (g_canDriverBufferList[canChannel][index].canRxFlag)
    {
        g_canDriverBufferList[canChannel][index].canRxFlag = 0;
        return 1;
    }
    return 0;
}

/*************************************************
   Function:        CanHalAppMsgEnable
   Description:     Enable application-level CAN message processing
   Input:           canHandle - CAN driver handle
   Output:          None
   Return:          0  - Success
                    CAN_ERROR_INVALID_HANDLE - Invalid handle or channel
   Others:          Increments AppMsgEnable counter in critical section
                    Supports nested enable/disable usage
 *************************************************/
int16_t CanHalAppMsgEnable(int16_t canHandle)
{
    uint8_t canChannel;
    if (canHandle < 0)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    canChannel = canHandle >> 8;
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    taskENTER_CRITICAL();
    g_driverCanManage[canChannel].AppMsgEnable++;
    taskEXIT_CRITICAL();
    return 0;
}

/*************************************************
   Function:        CanHalAppMsgDisable
   Description:     Disable application-level CAN message processing
   Input:           canHandle - CAN driver handle
   Output:          None
   Return:          0  - Success
                    CAN_ERROR_INVALID_HANDLE - Invalid handle or channel
   Others:          Decrements AppMsgEnable counter in critical section
 *************************************************/
int16_t CanHalAppMsgDisable(int16_t canHandle)
{
    uint8_t canChannel;

    if (canHandle < 0)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    canChannel = canHandle >> 8;
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    taskENTER_CRITICAL();
    g_driverCanManage[canChannel].AppMsgEnable--;
    taskEXIT_CRITICAL();
    return 0;
}

/*************************************************
   Function:        CanHalTxMsgEnable
   Description:     Enable CAN transmit messages for specified channel
   Input:           canChannel - CAN channel index
   Output:          None
   Return:          0  - Success
                    CAN_ERROR_INVALID_HANDLE - Invalid channel
   Others:          Sets txMsgEnable flag in critical section
 *************************************************/
int16_t CanHalTxMsgEnable(uint8_t canChannel)
{
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    taskENTER_CRITICAL();
    g_driverCanManage[canChannel].txMsgEnable = 1;
    taskEXIT_CRITICAL();
    return 0;
}

/*************************************************
   Function:        CanHalTxMsgDisable
   Description:     Disable CAN transmit messages for specified channel
   Input:           canChannel - CAN channel index
   Output:          None
   Return:          0  - Success
                    CAN_ERROR_INVALID_HANDLE - Invalid channel
   Others:          Clears txMsgEnable flag in critical section
 *************************************************/
int16_t CanHalTxMsgDisable(uint8_t canChannel)
{
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    taskENTER_CRITICAL();
    g_driverCanManage[canChannel].txMsgEnable = 0;
    taskEXIT_CRITICAL();
    return 0;
}

/*************************************************
   Function:        CanControllerBusOffError
   Description:     Check CAN controller bus-off error status
   Input:           canChannel - CAN channel index
   Output:          None
   Return:          0 - Bus-off error detected
                    1 - No bus-off error
   Others:          Reads platform-specific BusOff interrupt flags
                    Sets BusErrorState and disables app messages on error
 *************************************************/
static int16_t CanControllerBusOffError(uint8_t canChannel)
{
    uint8_t busErr = 0x00;

    switch (canChannel)
    {
    case 0:
        busErr = CAN0_BusOffIntFlag;
        break;
    case 1:
        busErr = CAN1_BusOffIntFlag;
        break;
    case 2:
        busErr = CAN2_BusOffIntFlag;
        break;
    case 3:
        busErr = CAN3_BusOffIntFlag;
        break;
    default:
        break;
    }
    if (busErr)
    {
        g_driverCanManage[canChannel].busErrorAppDiableFlag = 0x01;
        g_driverCanManage[canChannel].BusErrorState = 0x01;
        return 0;
    }
    return 1;
}

/*************************************************
   Function:        CanHalClearBusoffAppDisableFlag
   Description:     Clear application disable flag caused by bus-off error
   Input:           canHandle - CAN driver handle
   Output:          None
   Return:          0  - Success
                    CAN_ERROR_INVALID_HANDLE - Invalid handle or channel
   Others:          Clears busErrorAppDiableFlag for the channel
 *************************************************/
uint8_t CanHalClearBusoffAppDisableFlag(int16_t canHandle)
{
    uint8_t canChannel;

    if (canHandle < 0)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    canChannel = canHandle >> 8;
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    g_driverCanManage[canChannel].busErrorAppDiableFlag = 0x00;
    return 0;
}

/*************************************************
   Function:        CanHalGetIsBusOffError
   Description:     Check whether CAN bus-off error exists
   Input:           canHandle - CAN driver handle
   Output:          None
   Return:          0 - Bus-off error detected
                    1 - No bus-off error
                    CAN_ERROR_INVALID_HANDLE - Invalid handle or channel
   Others:          Internally calls CanControllerBusOffError()
 *************************************************/
int16_t CanHalGetIsBusOffError(int16_t canHandle)
{
    uint8_t canChannel;

    if (canHandle < 0)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    canChannel = canHandle >> 8;
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }

    return CanControllerBusOffError(canChannel);
}

/*************************************************
   Function:        CanHalEnableTransmitLoopBack
   Description:     Enable transmit loopback for specified CAN handle instance
   Input:           canHandle - CAN driver handle (high byte: channel, low byte: instance index)
   Output:          None
   Return:          0  - Success
                    -1 - Invalid handle or channel out of range
   Others:          Enables send-loopback mode for the target CAN buffer instance
                    Loopback frames can be dispatched to local RX path depending on configuration
 *************************************************/
int16_t CanHalEnableTransmitLoopBack(int16_t canHandle)
{
    uint8_t canChannel;

    if (canHandle < 0)
    {
        return -1;
    }
    canChannel = canHandle >> 8;
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX) 
    {
        return -1;
    }
    g_canDriverBufferList[canChannel][(canHandle & 0xFF)].enableSendLoopBack = 0x01;
    return 0;
}

/*************************************************
   Function:        CanHalDiagnosticReceive
   Description:     Receive diagnostic CAN message from per-instance RX queue
   Input:           canHandle  - CAN driver handle (high byte: channel, low byte: instance index)
                    timeOut_ms - Receive timeout in milliseconds (portMAX_DELAY for infinite wait)
   Output:          pMsg       - Output buffer for received CAN message
   Return:          0  - Receive successful
                    CAN_ERROR_INVALID_HANDLE - Invalid handle or channel out of range
                    CAN_ERROR_TIMEOUT        - Timeout or queue not ready
   Others:          Receives an index token from RxQueueHandle, then copies the message
                    from the instance RX buffer (ring) into pMsg
 *************************************************/
int16_t CanHalDiagnosticReceive(int16_t canHandle, CanHalMsg_t *pMsg, uint32_t timeOut_ms)
{
    uint8_t canChannel;
    uint8_t index;
    QueueHandle_t queHandle;
    TickType_t tickWait;
    uint8_t data;

    if (canHandle < 0)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    canChannel = canHandle >> 8;
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }

    index = canHandle & 0xFF;
    queHandle = g_canDriverBufferList[canChannel][index].RxQueueHandle;
    if (queHandle == NULL)
    {
        return CAN_ERROR_TIMEOUT;
    }
    if (timeOut_ms != portMAX_DELAY)
    {
        tickWait = pdMS_TO_TICKS(timeOut_ms);
    }
    else
    {
        tickWait = portMAX_DELAY;
    }

    if (xQueueReceive(queHandle, &data, tickWait) != pdPASS)
    {
        return CAN_ERROR_TIMEOUT;
    }

    memcpy(pMsg, &(g_canDriverBufferList[canChannel][index].pMsgBufferRx[data]), sizeof(CanHalMsg_t));
    return 0;
}

/*************************************************
   Function:        CanControllerReset
   Description:     Reset and re-initialize CAN controller after error recovery
   Input:           canChannel - CAN channel index
   Output:          None
   Return:          None
   Others:          Re-initializes CAN controller with configured bitrates and mode
                    Clears bus-off interrupt flag (platform-specific)
                    Resets BusErrorState for the specified channel
                    Uploads "BusoffRecover" log for traceability
 *************************************************/
static void CanControllerReset(uint8_t canChannel)
{
    CanInit(canChannel, CANFD_ENABLE_CONFIG, E_CAN_500K, E_CAN_2000K, E_CAN_MODE_NORMAL);
    switch (canChannel)
    {
    case 0:
        CAN0_BusOffIntFlag = 0U;
        break;
    case 1:
        CAN1_BusOffIntFlag = 0U;
        break;
    case 2:
        CAN2_BusOffIntFlag = 0U;
        break;
    case 3:
        CAN3_BusOffIntFlag = 0U;
        break;
    default:
        break;
    }
    g_driverCanManage[canChannel].BusErrorState = 0x00;
    g_driverCanManage[canChannel].busErrorAppDiableFlag = 0x00;
    LogHalUpLoadLog("BusoffRecover");
}

/*************************************************
   Function:        CanHalCanBusOffProcessEnalbe
   Description:     Enable CAN bus-off recovery processing for specified channel
   Input:           canChannel - CAN channel index
   Output:          None
   Return:          None
   Others:          Sets bus-off process enable flag
                    Used by CAN error management logic
 *************************************************/
void CanHalCanBusOffProcessEnalbe(uint8_t canChannel)
{
    if (canChannel > (CAN_CHANNEL_NUMBER_MAX - 1))
    {
        return;
    }

    // g_canBusOffProcessFlag[canChannel] = 1;
}

/*************************************************
   Function:        CanHalCanBusOffProcessDisalbe
   Description:     Disable CAN bus-off recovery processing for specified channel
   Input:           canChannel - CAN channel index
   Output:          None
   Return:          None
   Others:          Clears bus-off process enable flag
                    Used by OSEK NM SDK during initialization
  *************************************************/
void CanHalCanBusOffProcessDisalbe(uint8_t canChannel)
{
    if (canChannel > (CAN_CHANNEL_NUMBER_MAX - 1))
    {
        return;
    }

    // g_canBusOffProcessFlag[canChannel] = 0;
}

/*************************************************
   Function:        CanHalSetCanTestMode
   Description:     Enable or disable CAN test mode for specified channel
   Input:           canChannel - CAN channel index
                    modeFlag   - Test mode flag (0:disable, 1:enable)
   Output:          None
   Return:          None
   Others:          Test mode disables normal CAN TX gating logic
 *************************************************/
void CanHalSetCanTestMode(uint8_t canChannel, uint8_t modeFlag)
{
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return;
    }
    g_canTestModeFlag[canChannel] = modeFlag;
}

/*************************************************
   Function:        CanMsgDispatch
   Description:     Dispatch received CAN message to registered handlers
   Input:           pMsg              - Received CAN message
                     pCanDriverBuffer  - CAN driver buffer list
                     maxCanInstanse    - Number of instances
                     canChannel        - CAN channel index for drop count
   Output:          None
   Return:          None
   Others:          Supports standard and extended ID filtering
                     Optional user filter callback supported
                     Copies message into RX ring buffer and notifies task
 *************************************************/
static void CanMsgDispatch(CanHalMsg_t *pMsg, CanBufferHal_t *pCanDriverBuffer, uint16_t maxCanInstanse, uint8_t canChannel)
{
    uint16_t i;
    uint8_t data;
    uint8_t extended;
    uint32_t canId;

    extended = (pMsg->canId >> 31) & 0x01;
    canId = pMsg->canId & 0x7FFFFFFF;

    for (i = 0; i < maxCanInstanse; i++)
    {
        if ((0 == pCanDriverBuffer[i].enableSendLoopBack) && pMsg->txFlag)
        {
            continue;
        }
        if (pCanDriverBuffer[i].pFilerFun != NULL)
        {
            if (pCanDriverBuffer[i].pFilerFun(pMsg->canId, pMsg->canData, pMsg->dlc) != 0)
            {
                continue;
            }
        }
        else
        {
            if (extended)
            {
                // filter process
                if (pCanDriverBuffer[i].filter.useExdtFilter)
                {
                    if (canId < pCanDriverBuffer[i].filter.canIdExtdMin)
                    {
                        continue;
                    }
                    if (canId > pCanDriverBuffer[i].filter.canIdExtdMax)
                    {
                        continue;
                    }
                }
                else
                {
                    continue;
                }
            }
            else
            {
                if (pCanDriverBuffer[i].filter.useStdFilter)
                {
                    if (canId < pCanDriverBuffer[i].filter.canIdStdMin)
                    {
                        continue;
                    }
                    if (canId > pCanDriverBuffer[i].filter.canIdStdMax)
                    {
                        continue;
                    }
                }
                else
                {
                    continue;
                }
            }
        }

        // dispatch data
        if (pCanDriverBuffer[i].canUseFlag)
        {
            pCanDriverBuffer[i].canRxFlag = 1;
            if (pCanDriverBuffer[i].pMsgBufferRx == NULL) //
            {
                continue;
            }
            memcpy(&(pCanDriverBuffer[i].pMsgBufferRx[pCanDriverBuffer[i].msgRxIndexIn]), pMsg, sizeof(CanHalMsg_t));
            data = pCanDriverBuffer[i].msgRxIndexIn;
            pCanDriverBuffer[i].msgRxIndexIn++;
            if (pCanDriverBuffer[i].msgRxIndexIn >= pCanDriverBuffer[i].msgRxBufferSize)
            {
                pCanDriverBuffer[i].msgRxIndexIn = 0;
            }
            /* 尝试发送队列，如果失败则记录丢包统计 */
            if (xQueueSend(pCanDriverBuffer[i].RxQueueHandle, &data, 0) != pdPASS)
            {
                /* 队列满，消息丢失，增加统计计数 */
                g_canRxDropCount[canChannel]++;
            }
        }
    }
}

/*************************************************
   Function:        CanHalReceiveTask
   Description:     CAN receive task for message dispatching
   Input:           pvParameters - FreeRTOS task parameter (unused)
   Output:          None
   Return:          None
   Others:          Receives messages from RX queue
                    Copies data from ISR buffer
                    Dispatches messages to application buffers
 *************************************************/
void CanHalReceiveTask(void *pvParameters)
{
    uint8_t canChannel;
    uint8_t msgIndex;
    CanHalMsg_t canMsg;
    uint16_t queueData;

    g_allCanRxBuffer.msgRxIndexIn = 0;
    g_allCanRxBuffer.rxQueueHandle = xQueueCreate(ALL_CAN_RX_BUFFER_SIZE, // The number of items the queue can hold.
                                                  sizeof(uint16_t));
    /* 标记全局RX队列初始化完成，允许中断处理 */
    if (g_allCanRxBuffer.rxQueueHandle != NULL)
    {
        g_allCanRxBufferInited = 1;
    }
    while (1)
    {
        if (xQueueReceive(g_allCanRxBuffer.rxQueueHandle, &queueData, 0xFFFFFFFF) != pdPASS)
        {
            continue;
        }
        canChannel = queueData >> 8;
        msgIndex = queueData & 0xFF;
        // receive from can interrupt
        memcpy(&canMsg, &(g_allCanRxBuffer.msgRxBuffer[msgIndex]), sizeof(CanHalMsg_t));
        CheckAndSetExternalTester(canMsg.canId, canChannel, canMsg.txFlag);
        if (canChannel < CAN_CHANNEL_NUMBER_MAX)
        {
            CanMsgDispatch(&canMsg, g_canDriverBufferList[canChannel], CAN_DRIVER_HAL_HANDLE_INSTANSE_MAX, canChannel);
        }
        else
        {

        }
    }
}

/*************************************************
   Function:        CanHalResetHardware
   Description:     Reset CAN controller hardware by handle
   Input:           canHandle - CAN driver handle
   Output:          None
   Return:          0  - Reset successful
                    CAN_ERROR_INVALID_HANDLE - Invalid handle
   Others:          Extracts CAN channel from handle internally
 *************************************************/
int16_t CanHalResetHardware(int16_t canHandle)
{
    uint8_t canChannel;

    if (canHandle < 0)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    canChannel = canHandle >> 8;
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return CAN_ERROR_INVALID_HANDLE;
    }
    CanControllerReset(canChannel);
    return 0;
}

/*************************************************
   Function:        CanHalEnableCanWakeup
   Description:     Enable CAN wake-up detection for specified channel
   Input:           canChannel - CAN channel index
   Output:          None
   Return:          None
   Others:          Sets internal wake-up enable flag
 *************************************************/
void CanHalEnableCanWakeup(uint8_t canChannel)
{
    if (canChannel >= CAN_CHANNEL_NUMBER_MAX)
    {
        return;
    }
    g_canWakeUpFlag[canChannel] = 1;
}

/*************************************************
   Function:        CanHalSetReceiveCanNmFlag
   Description:     Set CAN Network Management receive flag according to CAN ID
   Input:           canId - Received CAN message identifier
   Output:          None
   Return:          None
   Others:          If the received CAN ID falls within the NM wake-up ID range,
                    the global NM receive flag will be set.
                    Otherwise, the flag will be cleared.
                    This flag is typically used to indicate reception of
                    CAN NM wake-up or management frames.
 *************************************************/
static void CanHalSetReceiveCanNmFlag(uint32_t canId)
{
    if((canId >= CAN_NM_WAKEUP_ID_MIN) && (canId <= CAN_NM_WAKEUP_ID_MAX))
    {
        g_receiveCanNmFlag = 1U;
    }
    else
    {
        g_receiveCanNmFlag = 0U;
    }
}

/*************************************************
 Function:        CanHalClearReceiveCanNmFlag
 Description:     Clears the CAN NM (Network Management) receive flag
 Input:           None
 Output:          None
 Return:          None
 Others:          Resets the global flag g_receiveCanNmFlag to 0, typically used after processing NM messages to avoid repeated handling
 *************************************************/
void CanHalClearReceiveCanNmFlag(void)
{
    g_receiveCanNmFlag = 0U;
}

/*************************************************
   Function:        CanHalReceiveCanNmFlagCheck
   Description:     Check whether CAN NM message has been received
   Input:           None
   Output:          None
   Return:          1 - CAN NM message received
                    0 - No CAN NM message received
   Others:          Reads global CAN NM receive flag
                    Typically used by NM state machine or wake-up logic
 *************************************************/
uint8_t CanHalReceiveCanNmFlagCheck(void)
{
    uint8_t ret = 0U;

    if(g_receiveCanNmFlag != 0U)
    {
        ret = 1U;
    }
    return ret;
}

/*************************************************
   Function:        CanHandleToChannel
   Description:     Convert CAN driver handle to CAN channel index
   Input:           canHandle - CAN driver handle
   Output:          pOutCh    - Output CAN channel index
   Return:          1 - Conversion successful
                    0 - Invalid handle or channel out of range
   Others:          CAN handle high byte represents channel number
                    Used for TX/RX path channel resolution
 *************************************************/
static uint8_t CanHandleToChannel(int16_t canHandle, uint8_t *pOutCh)
{
    uint8_t ch;

    if (canHandle < 0)
    {
        return 0U;
    }

    ch = (uint8_t)(((uint16_t)canHandle) >> 8);
    if (ch >= (uint8_t)CAN_CHANNEL_NUMBER_MAX)
    {
        return 0U;
    }

    *pOutCh = ch;
    return 1U;
}

/*************************************************
   Function:        CanHalTxCanSendHook
   Description:     Check whether CAN transmission is allowed for the channel
   Input:           canChannel - CAN channel index
   Output:          None
   Return:          1 - Transmission allowed
                    0 - Transmission forbidden
   Others:          Checks TX enable flag, test mode and bus error states
                    Can be overridden by project-specific logic
 *************************************************/
uint8_t CanHalTxCanSendHook(uint8_t canChannel)
{
    if (g_driverCanManage[canChannel].txMsgEnable <= 0) return 0U;
    if (g_canTestModeFlag[canChannel] != 0U) return 0U;
    if ((g_driverCanManage[canChannel].busNoAckErrorState != 0U) ||
        (g_driverCanManage[canChannel].busErrorAppDiableFlag != 0U) ||
        (g_driverCanManage[canChannel].BusErrorState != 0U)) return 0U;
    return 1U;
}

/*************************************************
   Function:        CanHalTxInit
   Description:     Initialize CAN transmit buffer pool and queues
   Input:           None
   Output:          None
   Return:          None
   Others:          Creates TX message queue and free index pool
                    Ensures one-time initialization
 *************************************************/
static void CanHalTxInit(void)
{
    uint8_t i;

    if (g_allCanTxBuffer.inited != 0U)
    {
        return;
    }

    g_allCanTxBuffer.txQueueHandle = xQueueCreate(ALL_CAN_TX_BUFFER_SIZE, sizeof(uint16_t));
    g_allCanTxBuffer.freeIdxQueueHandle = xQueueCreate(ALL_CAN_TX_BUFFER_SIZE, sizeof(uint8_t));

    /* 填充 free idx 池 */
    for (i = 0U; i < (uint8_t)ALL_CAN_TX_BUFFER_SIZE; i++)
    {
        (void)xQueueSend(g_allCanTxBuffer.freeIdxQueueHandle, &i, 0U);
    }

    g_allCanTxBuffer.inited = 1U;
}

/*************************************************
   Function:        CanHalTxWaitInitDone
   Description:     Wait for CAN TX task initialization completion
   Input:           waitTicks - Maximum wait ticks
   Output:          None
   Return:          0  - Initialization done
                    -1 - Timeout or failure
   Others:          Uses event group synchronization
 *************************************************/
static int16_t CanHalTxWaitInitDone(TickType_t waitTicks)
{
    EventBits_t bits;

    /* 事件组由CanHalSendTask创建，这里只等待，不创建，避免重复创建导致内存泄漏 */
    if (g_canTxEvt == NULL)
    {
        /* 事件组尚未创建，返回错误，调用者应稍后重试 */
        return -1;
    }

    bits = xEventGroupWaitBits(g_canTxEvt,
                              CAN_TX_EVT_INIT_DONE,
                              pdFALSE,
                              pdTRUE,
                              waitTicks);

    if ((bits & CAN_TX_EVT_INIT_DONE) == 0U)
    {
        return -1;
    }

    return 0;
}

/*************************************************
   Function:        CanHalTransmitQueued
   Description:     Enqueue CAN message for transmission (task context)
   Input:           canHandle - CAN driver handle
                    canId     - CAN identifier
                    canData   - CAN payload data
                    dlc       - Data length
                    txFlag    - TX attribute flags
                    prio      - Transmission priority
   Output:          None
   Return:          0  - Enqueue successful
                    <0 - Parameter or initialization error
                    CAN_ERROR_TX_BUFFER_FULL - Buffer exhausted
   Others:          Supports priority-based queue insertion
                    Blocks until TX task initialization completed
 *************************************************/
int16_t CanHalTransmitQueued(int16_t canHandle,
                             uint32_t canId,
                             const uint8_t *canData,
                             uint8_t dlc,
                             uint8_t txFlag,
                             CanTxPrio_e prio)
{
    uint8_t  ch;
    uint8_t  idx;
    uint16_t queueData;
    BaseType_t qret;

    if ((canData == NULL) || (dlc > 64U))
    {
        return -1;
    }

    if (CanHandleToChannel(canHandle, &ch) == 0U)
    {
        return -1;
    }

    if (CanHalTxWaitInitDone(pdMS_TO_TICKS(CAN_TX_ENQUEUE_TIMEOUT_MS)) != 0)
    {
        return -1;
    }

    if ((g_allCanTxBuffer.txQueueHandle == NULL) || (g_allCanTxBuffer.freeIdxQueueHandle == NULL))
    {
        return -1;
    }

    if (xQueueReceive(g_allCanTxBuffer.freeIdxQueueHandle,
                      &idx,
                      pdMS_TO_TICKS(CAN_TX_ENQUEUE_TIMEOUT_MS)) != pdPASS)
    {
        return CAN_ERROR_TX_BUFFER_FULL;
    }

    g_allCanTxBuffer.msgTxBuffer[idx].canId  = canId;
    g_allCanTxBuffer.msgTxBuffer[idx].dlc    = dlc;
    g_allCanTxBuffer.msgTxBuffer[idx].txFlag = txFlag;
    (void)memset(g_allCanTxBuffer.msgTxBuffer[idx].canData, 0, sizeof(g_allCanTxBuffer.msgTxBuffer[idx].canData));
    (void)memcpy(g_allCanTxBuffer.msgTxBuffer[idx].canData, canData, dlc);

    queueData = (uint16_t)(((uint16_t)ch << 8) | (uint16_t)idx);

    if (prio == CAN_TX_PRIO_HIGH)
    {
        qret = xQueueSendToFront(g_allCanTxBuffer.txQueueHandle,
                                 &queueData,
                                 pdMS_TO_TICKS(CAN_TX_ENQUEUE_TIMEOUT_MS));
    }
    else
    {
        qret = xQueueSendToBack(g_allCanTxBuffer.txQueueHandle,
                                &queueData,
                                pdMS_TO_TICKS(CAN_TX_ENQUEUE_TIMEOUT_MS));
    }

    if (qret != pdPASS)
    {
        /* 必须归还索引，使用portMAX_DELAY确保归还成功，避免索引泄漏 */
        BaseType_t giveRet;
        do {
            giveRet = xQueueSend(g_allCanTxBuffer.freeIdxQueueHandle, &idx, portMAX_DELAY);
        } while (giveRet != pdPASS);
        return CAN_ERROR_TX_BUFFER_FULL;
    }

    return 0;
}

/*************************************************
 Function:        CanHalSendTask
 Description:     CAN bus transmit task for handling message sending
 Input:           pvParameters - FreeRTOS task parameter (not used)
 Output:          None
 Return:          None
 Others:          Uses queue-based message management
                  Initializes event group and transmit buffer pool
                  Handles message transmission with retry mechanism
                  Manages free buffer index recycling
 *************************************************/
void CanHalSendTask(void *pvParameters)
{
    uint16_t    queueData;
    uint8_t     canChannel;
    uint8_t     msgIndex;
    CanHalMsg_t txMsg;
    int16_t     ret;
    uint32_t    retry;

    (void)pvParameters;

    /* 1) 初始化事件组 */
    if (g_canTxEvt == NULL)
    {
        g_canTxEvt = xEventGroupCreate();
        if (g_canTxEvt == NULL)
        {
            for (;;)
            {
                TBOX_PRINT("g_canTxEvt ERROR\n");
                vTaskDelay(pdMS_TO_TICKS(1000U));
            }
        }
    }

    /* 2) 初始化队列/空闲池 */
    CanHalTxInit();

    /* 3) 通知初始化完成 */
    (void)xEventGroupSetBits(g_canTxEvt, CAN_TX_EVT_INIT_DONE);

    while (1)
    {
        if (xQueueReceive(g_allCanTxBuffer.txQueueHandle, &queueData, portMAX_DELAY) != pdPASS)
        {
            continue;
        }

        canChannel = (uint8_t)(queueData >> 8);
        msgIndex   = (uint8_t)(queueData & 0xFFU);

        if ((canChannel >= CAN_CHANNEL_NUMBER_MAX) || (msgIndex >= ALL_CAN_TX_BUFFER_SIZE))
        {
            /* 归还 idx，避免泄露 */
            (void)xQueueSend(g_allCanTxBuffer.freeIdxQueueHandle, &msgIndex, 0U);
            continue;
        }

        /* memcpy 到局部变量（与 RX 同风格，避免槽位被覆盖） */
        (void)memcpy(&txMsg, &g_allCanTxBuffer.msgTxBuffer[msgIndex], sizeof(CanHalMsg_t));

        /* 发送条件检测 */
        if (CanHalTxCanSendHook(canChannel) == 0U)
        {
            (void)xQueueSend(g_allCanTxBuffer.freeIdxQueueHandle, &msgIndex, 0U);
            continue;
        }

        /* BUSY(-6) 重试发送 */
        retry = 0U;
        for (;;)
        {
            ret = CanTransmit(canChannel, txMsg.canId, txMsg.dlc, txMsg.canData, txMsg.txFlag , g_canOtaMode);

            if (ret == 0)
            {
                break;
            }
            else if (ret == (int16_t)CAN_ERROR_TX_BUFFER_FULL)
            {
                retry++;
                if (retry >= (uint32_t)CAN_TX_BUSY_MAX_RETRY)
                {
                    /* 超限丢弃 */
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(CAN_TX_BUSY_RETRY_DELAY_MS));
            }
            else
            {
                /* 其他错误：丢弃 */
                break;
            }
        }

        /* 归还槽位 idx（关键：否则很快耗尽导致“队列满/丢帧”） */
        (void)xQueueSend(g_allCanTxBuffer.freeIdxQueueHandle, &msgIndex, 0U);
    }
}

void CanHal_SetOtaMode(uint8_t enable)
{
    g_canOtaMode = (enable != 0U) ? 1U : 0U;
}



static void CheckAndSetExternalTester(uint32_t canId, uint8_t channel, uint8_t txFlag)
{
    uint32_t i;
    uint32_t listSize = sizeof(g_monitorEcuList) / sizeof(g_monitorEcuList[0]);

    for (i = 0; i < listSize; i++)
    {
        if ((g_monitorEcuList[i].reqId == canId) && (g_monitorEcuList[i].channel == channel))
        {
            /* SetEcuOnlineFlag(); */
            return;
        }
    }
}