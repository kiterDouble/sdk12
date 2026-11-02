/*************************************************
 Copyright © 2026 SiRun (AnHui) . All rights reserved.
 File Name: mpuHal.c
 Author: 
 Created Time: 2026年7月7日
 Description:
 Others:
*************************************************/
/****************************** include ***************************************/
#include "mpuHal.h"
#include "crc8_16_32.h"
#include <string.h>
#include "Spi_Hal.h"
#include "Gpio_Hal.h"
#include "Ckgen_Hal.h"
#include "Spm_Hal.h"
#include "Dma_Hal.h"
#include "logHal.h"
#include "Rcm_Hal.h"
#include "Core_Hal.h"
#include "peripheralHal.h"

/****************************** Macro Definitions ******************************/
#define MPU_DEBUG_QUEUE                         0
#define MPU_HAL_HANDLE_INSTANSE_MAX             15
#define MPU_PROTOCAL_HEADER_LEN                 8
#define MPU_PROTOCAL_SPI_HEADER_LEN             6
#define MPU_HAL_PROC_BUFFER_LEN                 4096
#define MPU_HAL_MAX_BUFFSIZE                    (4096*2)
#define MPU_HAL_UART_BUFFSIZE                   (4096)
#define MPU_HAL_PACK_HEADER_BUF_LEN             20
#define MPU_HAL_TXMSG_MAX_LEN                   4000
#define MPU_HAL_8BIT_MASK                       0xFF
#define MPU_HAL_16BIT_MASK                      0xFFFF
#define MPU_HAL_8BIT_SHIFT                      8
#define MPU_HAL_16BIT_SHIFT                     16
#define MPU_HAL_WAKEOUT_TIME_INIT               10
#define MPU_HAL_WAKEOUT_TIME_WAKEVALUE          2
#define MPU_HAL_WAKEOUT_TIME_SLEEPVALUE         0
#define MPU_HAL_UART_DELAY_CNT                  500
#define MPU_HAL_UART_RX_BUF_LEB                 11
#define MPU_HAL_WAKE_MODE_POWER_OFF             0
#define MPU_HAL_WAKE_MODE_MPU_SLEEP             1
#define MPU_HAL_WAKE_MODE_MPU_WAKE              2
#define MPU_HAL_PACK_FIX_HEADER_0X55            0x55
#define MPU_HAL_PACK_FIX_HEADER_0XAA            0xAA
#define MPU_HAL_PACK_FIX_HEADER_0XBB            0xBB // SPI 6字节头帧
#define MPU_HAL_PACK_HEADER_RESERVED            0x00
#define MPU_HAL_CRC_DEFAULT_VALUE               0x0000
#define MPU_HAL_CYC_TIME_CNT_55MS               500
#define MPU_HAL_CYC_TIME_CNT_400MS              4000
#define MPU_HAL_CYC_TIME_CNT_50MS               500
#define MPU_HAL_UART_TX_ONCE_LEN                40
#define MPU_HAL_UART_TX_TEMP_LEN                50
#define MPU_HAL_SEND_IRQ_ENABLE                 (1U) 
#define MPU_HAL_SEND_QUEUE_WAIT_CNT             (10U)
#define MPU_HAL_RING_BUFFER                     (4096 * 2)

/* 环形缓冲区大小 - 8KB可缓存多帧数据 */
#define MPU_HAL_RX_RING_SIZE  8192

/* 临时缓冲区大小 - 最大一帧数据长度（DMA缓冲区大小） */
#define MPU_HAL_RX_TEMP_SIZE  4096

/* 数据块队列大小 */
#define RX_BLOCK_QUEUE_SIZE  30 // 升级包一次2048，其他数据不超过200，30为合理值

/* --- Hardware Instances Mapping --- */
#define UART_BAUDRATE                           (115200*4)
#define MPU_HAL_UART_ID                         0  // Map to UART0
#define UART_DMA_TX_REQ                         DMA_REQ_UART0_TX
#define UART_DMA_RX_REQ                         DMA_REQ_UART0_RX
#define MPU_HAL_UART_RX_PORT                    PORTID_C
#define MPU_HAL_UART_RX_PIN                     2
#define MPU_HAL_UART_RX_GPIO                    PORT_MUX_AS_GPIO
#define MPU_HAL_UART_RX_MUX                     PORT_MUX_ALT4
#define MPU_HAL_UART_TX_PORT                    PORTID_C
#define MPU_HAL_UART_TX_PIN                     3 
#define MPU_HAL_UART_TX_GPIO                    PORT_MUX_AS_GPIO
#define MPU_HAL_UART_TX_MUX                     PORT_MUX_ALT4

#define UART_DMA_TX_CHANNEL                     1
#define UART_DMA_RX_CHANNEL                     0

#define MAX_FRAME_QUEUE_SIZE  4
#define BOOT_UART_TX_MAX_LEN  1024 

/****************************** Type Definitions ******************************/

/* 数据块信息 - 记录每帧数据在环形缓冲区中的位置 */
typedef struct {
    uint16_t startIdx;  /* 数据起始位置 */
    uint16_t len;       /* 数据长度 */
} RxBlockInfo_t;

/* 数据块队列 - 用于中断和主循环间传递数据块信息 */
typedef struct {
    RxBlockInfo_t blocks[RX_BLOCK_QUEUE_SIZE];
    volatile uint8_t writeIdx;
    volatile uint8_t readIdx;
    volatile uint8_t count;
} RxBlockQueue_t;

typedef struct 
{
    uint8_t data[2200];
    uint16_t dataCount;
    uint16_t dataLength;
}MpuUartProtocalBuffer_t;

typedef struct {
    uint16_t startIdx;
    uint16_t len;
} FrameInfo_t;

typedef struct
{
    uint8_t useFlag;
    uint8_t useRxFilter;
    uint8_t* pDataBufferRx;
    uint16_t dataBufferSize;
    uint16_t rxIndexIn;
    FrameInfo_t      frameQueue[MAX_FRAME_QUEUE_SIZE];
    volatile uint8_t queueHead;
    volatile uint8_t queueTail;
    volatile uint8_t frameCount;
    MpuHalFilter_t rxFilterConfig;
}MpuHalHandle_t;

typedef struct {
    uint16_t headerError;   // 帧头错误
    uint16_t lengthError;   // 长度超出缓冲区
    uint16_t overflowError; // 队列发送失败
    uint16_t rxOverflowCnt;
    uint8_t  lastInvalidAid;       // 最近一次未匹配成功的 AID
    uint8_t  lastInvalidMid;       // 最近一次未匹配成功的 MID
    uint16_t lastFullHandleReqLen; // 报错时请求写入的长度
    uint16_t lastFullHandleFree;   // 报错时剩余的可用空间
} MpuHalErrorStats_t;

typedef enum
{
    E_MPU_HAL_START_STATE_IDLE = 0,
    E_MPU_HAL_START_STATE_POWER_OFF,
    E_MPU_HAL_START_STATE_OFF_DELAY,
    E_MPU_HAL_START_STATE_POWER_ON,
    E_MPU_HAL_START_STATE_POWER_DELAY,
    E_MPU_HAL_START_STATE_KEY_ON,
    E_MPU_HAL_START_STATE_KEY_ON_DELAY,
    E_MPU_HAL_START_STATE_KEY_OFF,
    E_MPU_HAL_START_STATE_FINISH,
} MpuHalStartState_e;

typedef struct 
{
    uint8_t wakeMode;/*0:MPU power off,1:Mpu sleep,2:Mpu wake*/
    uint8_t wakeoutTimeCount;
    MpuHalStartState_e startState;
    MpuHalHandle_t rxHandle[MPU_HAL_HANDLE_INSTANSE_MAX];
}MpuHalManage_t;

/****************************** Global Variables ******************************/
static MpuUartProtocalBuffer_t     g_mpuUartProtocalBuffer;
static MpuHalManage_t g_mpuManage;
static MpuHalErrorStats_t g_rxErrorStats = {0};
static uint8_t s_bootTxBuffer[BOOT_UART_TX_MAX_LEN];
static volatile uint8_t g_uartDmaTxFlag = 0;
static uint8_t g_uartDmaRxBufRaw[MPU_HAL_UART_BUFFSIZE];

/* 环形缓冲区 - 8KB */
static uint8_t s_rxRingBuf[MPU_HAL_RX_RING_SIZE];
static volatile uint16_t s_rxRingWriteIdx = 0;
static volatile uint16_t s_rxRingReadIdx = 0;
static RxBlockQueue_t s_rxBlockQueue = {0};
static uint8_t s_rxTempBuf[MPU_HAL_RX_TEMP_SIZE];

/****************************** Function Declarations *************************/
static void MpuHalMainUartInit(void);
static void MpuHalMainUartSleep(void);
static void MpuHalSetPowerkey(uint8_t flag);
static void MpuHalSetWakeOut(uint8_t flag);
static void MpuHalSetVbus(uint8_t flag);
static void MpuHalSetGpio(uint8_t port, uint8_t gpioNum, uint8_t flag);
static void MpuPackHeader(uint8_t headerBuffer[], const MpuHalDataPack_t* pMsg);
static uint16_t MpuPackGetCrc(uint8_t header[], const MpuHalDataPack_t *pMsg);
static void UartProtocalProcess(uint8_t *pData, uint16_t dataLength);
static void UartHalEventCallback(uint8 Instance, Uart_EventType Event);
static void MpuHal_UartDmaInit(void);
static int16_t MpuUartTransmit(const MpuHalDataPack_t* pTxMsg, const uint8_t* headerBuf, const uint8_t* crcBuf, uint16_t txLength);
static void MpuDataDispatch(const uint8_t* pData, uint16_t length);
static void MpuHalRxBuf_Init(void);

/****************************** Public Function Implementations ******************************/
/*************************************************
 Function: MpuHal_BootInit
 Description: Initialize the MPU HAL
 Input: void
 Output: void
 Return: void
 Others:
*************************************************/
void MpuHal_BootInit(void)
{
    memset(&g_mpuManage, 0, sizeof(g_mpuManage));
    g_mpuManage.wakeoutTimeCount = MPU_HAL_WAKEOUT_TIME_INIT;
    g_mpuManage.wakeMode = MPU_HAL_WAKE_MODE_MPU_SLEEP;
    MpuHalRxBuf_Init();  /* 初始化接收双缓冲区 */
    MpuHal_UartDmaInit();
    MpuHalMainUartInit();
}

/*************************************************
 Function: MpuHal_BootDeInit
 Description: Initialize the MPU HAL
 Input: void
 Output: void
 Return: void
 Others:
*************************************************/
void MpuHal_BootDeInit(void)
{
    Dma_Hal_Deinit();
    MpuHalMainUartSleep();
}

/*************************************************
  Function:       MpuHal_UartDmaInit
  Description:    Initialize UART DMA for communication
  Input:          None
  Output:         None
  Return:         None
  Others:         None
*************************************************/
static void MpuHal_UartDmaInit(void)
{
    static const Dma_ChannelConfigType dmaChannelConfigs[] = {
        {
            .HwChannelId = UART_DMA_TX_CHANNEL,
            .VirtualChannelId = UART_DMA_TX_CHANNEL,
            .Priority = DMA_CHANNEL_PRIORITY_LOW, 
            .ReqSrc = UART_DMA_TX_REQ
        },
        {
            .HwChannelId = UART_DMA_RX_CHANNEL,
            .VirtualChannelId = UART_DMA_RX_CHANNEL,
            .Priority = DMA_CHANNEL_PRIORITY_LOW,
            .ReqSrc = UART_DMA_RX_REQ
        }
    };
    Dma_ConfigType dmaCfg;
    memset(&dmaCfg, 0, sizeof(dmaCfg));
    dmaCfg.ChannelCnt = sizeof(dmaChannelConfigs) / sizeof(dmaChannelConfigs[0]);
    dmaCfg.ChannelCfg = dmaChannelConfigs;
    Dma_Hal_Init(&dmaCfg);
}

/*************************************************
 Function: MpuHal_BootGpioInit
 Description: Initialize the GPIO used by MPU
 Input: void
 Output: void
Return:void
 Others:
*************************************************/
void MpuHal_BootGpioInit(void)
{
    /* Power En: PTB10 */
    Gpio_Hal_SetMuxMode(PORTID_B, 10, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(PORTID_B, 10, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(PORTID_B, 10, GPIO_HIGH_LEVEL);
    
    /* Power Key: PTE11 */
    Gpio_Hal_SetMuxMode(PORTID_E, 11, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(PORTID_E, 11, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(PORTID_E, 11, GPIO_LOW_LEVEL);
    
    /* Vbus: PTA14 */
    Gpio_Hal_SetMuxMode(PORTID_A, 14, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(PORTID_A, 14, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(PORTID_A, 14, GPIO_LOW_LEVEL);
    
    /* Wake Out: PTB8 */
    Gpio_Hal_SetMuxMode(PORTID_B, 8, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(PORTID_B, 8, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(PORTID_B, 8, GPIO_LOW_LEVEL);
    
    /* MCU_RESET_4G: PTD12 */
    Gpio_Hal_SetMuxMode(PORTID_D, 12, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(PORTID_D, 12, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(PORTID_D, 12, GPIO_LOW_LEVEL);
    
    /* GPS EN: PE5 */
    Gpio_Hal_SetMuxMode(PORTID_E, 5, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(PORTID_E, 5, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(PORTID_E, 5, GPIO_HIGH_LEVEL);
}

static void UartHalEventCallback(uint8 Instance, Uart_EventType Event)
{
    uint32_t remainBytes = 0;
    uint16_t receivedLen = 0;
    uint16_t availableSpace = 0;
    uint16_t writeIdx = 0;
    uint16_t i = 0;
    
    if ((Event == UART_EVENT_IDLE_LINE) || (Event == UART_EVENT_END_TRANSFER))
    {
        Uart_Hal_GetReceiveStatus(Instance, &remainBytes);
        receivedLen = MPU_HAL_UART_BUFFSIZE - remainBytes;
        
        if ((receivedLen > 0) && (receivedLen <= MPU_HAL_UART_BUFFSIZE)) {
            /* 计算环形缓冲区可用空间（预留1字节用于区分满/空） */
            writeIdx = s_rxRingWriteIdx;
            if (writeIdx >= s_rxRingReadIdx) {
                availableSpace = MPU_HAL_RX_RING_SIZE - 1 - (writeIdx - s_rxRingReadIdx);
            } else {
                availableSpace = s_rxRingReadIdx - writeIdx - 1;
            }
            
            /* 检查是否有足够空间和队列槽位 */
            if ((receivedLen <= availableSpace) && (s_rxBlockQueue.count < RX_BLOCK_QUEUE_SIZE)) {
                /* 计算数据起始位置 */
                uint16_t startIdx = writeIdx;
                
                /* 拷贝数据到环形缓冲区（处理环形边界） */
                for (i = 0; i < receivedLen; i++) {
                    s_rxRingBuf[writeIdx] = g_uartDmaRxBufRaw[i];
                    writeIdx++;
                    if (writeIdx >= MPU_HAL_RX_RING_SIZE) {
                        writeIdx = 0;
                    }
                }
                
                /* 更新写索引 */
                s_rxRingWriteIdx = writeIdx;
                
                /* 推送数据块信息到队列 */
                __disable_irq();
                s_rxBlockQueue.blocks[s_rxBlockQueue.writeIdx].startIdx = startIdx;
                s_rxBlockQueue.blocks[s_rxBlockQueue.writeIdx].len = receivedLen;
                s_rxBlockQueue.writeIdx = (s_rxBlockQueue.writeIdx + 1) % RX_BLOCK_QUEUE_SIZE;
                s_rxBlockQueue.count++;
                __enable_irq();
            } else {
                /* 溢出计数增加 */
                g_rxErrorStats.rxOverflowCnt++;
            }
        }
        
        /* Restart RX Reception */
        Uart_Hal_AbortReceivingData(MPU_HAL_UART_ID);
        Uart_Hal_ReceiveData(MPU_HAL_UART_ID, g_uartDmaRxBufRaw, MPU_HAL_UART_BUFFSIZE);
    }
    else if (Event == UART_EVENT_ERROR)
    {
        Uart_Hal_AbortReceivingData(MPU_HAL_UART_ID);
        Uart_Hal_ReceiveData(MPU_HAL_UART_ID, g_uartDmaRxBufRaw, MPU_HAL_UART_BUFFSIZE);
    }
    else
    {
        // do nothing
    }
}

static void UartHalTxEventCallback(uint8 Instance, Uart_EventType Event)
{
    if (Event == UART_EVENT_ERROR || Event == UART_EVENT_END_TRANSFER)
    {
        g_uartDmaTxFlag = 0;
    }
}


/*************************************************
 Function: MpuHalMainUartInit
 Description: Initialize the main UART interface for MPU communication
 Input: void
 Output: void
 Return: void
 Others:
*************************************************/
static void MpuHalMainUartInit(void)
{
    Gpio_Hal_SetMuxMode(MPU_HAL_UART_RX_PORT, MPU_HAL_UART_RX_PIN, MPU_HAL_UART_RX_MUX);
    Gpio_Hal_SetMuxMode(MPU_HAL_UART_TX_PORT, MPU_HAL_UART_TX_PIN, MPU_HAL_UART_TX_MUX);

    Gpio_Hal_SetPullSel(MPU_HAL_UART_RX_PORT, MPU_HAL_UART_RX_PIN, PORT_INTERNAL_PULL_UP_ENABLED);
    Gpio_Hal_SetPullSel(MPU_HAL_UART_TX_PORT, MPU_HAL_UART_TX_PIN, PORT_INTERNAL_PULL_UP_ENABLED);

    Uart_ChannelConfigType uartCfg;
    memset(&uartCfg, 0, sizeof(uartCfg));
    uartCfg.BaudRate = UART_BAUDRATE;
    uartCfg.ParityMode = UART_PARITY_DISABLED;
    uartCfg.StopBitCount = UART_ONE_STOP_BIT;
    uartCfg.BitCountPerChar = UART_8_BITS_PER_CHAR;
    uartCfg.TransferType = UART_USING_DMA;
    uartCfg.SampleCnt = UART_SMP_CNT16;
    uartCfg.RxDmaChannel = UART_DMA_RX_CHANNEL;
    uartCfg.TxDmaChannel = UART_DMA_TX_CHANNEL;
    uartCfg.RxCallback = UartHalEventCallback;
    uartCfg.TxCallback = UartHalTxEventCallback;

    Uart_Hal_Init(MPU_HAL_UART_ID, &uartCfg);
    Uart_Hal_SetIdleInterrupt(MPU_HAL_UART_ID, TRUE);

    Core_Hal_EnableIrq(UART0_IRQn);
    Core_Hal_SetIrqPriority(UART0_IRQn, 0x3);
    
    /* Start initial DMA RX */
    Uart_Hal_ReceiveData(MPU_HAL_UART_ID, g_uartDmaRxBufRaw, MPU_HAL_UART_BUFFSIZE);
    TBOX_PRINT("UART init\n");
}

/*************************************************
 Function: MpuHalSetGpio
 Description: Set the GPIO output level
 Input:  PORT_Id_t port: port ID
        PORT_GpioNum_t gpioNum: GPIO number
        uint8_t flag: 0:low level,1:high level
 Output: void
 Return: void
 Others:
*************************************************/
static void MpuHalSetGpio(uint8_t port, uint8_t gpioNum, uint8_t flag)
{
    Gpio_Hal_WritePin(port, gpioNum, flag ? GPIO_HIGH_LEVEL : GPIO_LOW_LEVEL);
}

/*************************************************
 Function: MpuHalSetPower
 Description: Set the power output level
 Input:  uint8_t flag: 0:low level,1:high level
 Output: void
 Return: void
 Others:
*************************************************/
void MpuHalSetPower(uint8_t flag)
{
    MpuHalSetGpio(PORTID_B, 10, flag);
}

/*************************************************
 Function: MpuHalSetPowerkey
 Description: Set the powerkey output level
 Input: uint8_t flag: 0:low level,1:high level
 Output: void
 Return: void
 Others:
*************************************************/
static void MpuHalSetPowerkey(uint8_t flag)
{
    MpuHalSetGpio(PORTID_E, 11, flag);
}

/*************************************************
 Function: MpuHalSetVbus
 Description: Set the Vbus output level
 Input: uint8_t flag: 0:low level,1:high level
 Output: void
 Return: void
 Others:
*************************************************/
static void MpuHalSetVbus(uint8_t flag)
{
    MpuHalSetGpio(PORTID_A, 1, flag);
}

/*************************************************
 Function: MpuHalSetWakeOut
 Description: Set the WakeOut output level
 Input: uint8_t flag: 1:low level,0:high level
 Output: void
 Return: void
 Others:
*************************************************/
static void MpuHalSetWakeOut(uint8_t flag)
{
    MpuHalSetGpio(PORTID_B, 8, flag);
}

/*************************************************
 Function: MpuDataDispatch
 Description: Dispatch the received data to the registered handle
 Input: const uint8_t* pData: pointer to the received data
        uint16_t length: length of the received data
        uint8_t irq: 1: called in IRQ,0: called in task
 Output: void
 Return: void
 Others:
*************************************************/
static void MpuDataDispatch(const uint8_t* pData, uint16_t length)
{
    uint16_t i = 0u, j = 0u;
    MpuHalHandle_t* pHandleInstance;

    for (i = 0u; i < MPU_HAL_HANDLE_INSTANSE_MAX; i++) {

        pHandleInstance = g_mpuManage.rxHandle + i;
        if ((pHandleInstance->useFlag == 0) || (pHandleInstance->pDataBufferRx == NULL)) {
            continue;
        }
        if ((pHandleInstance->useRxFilter == 0) || (pData[2] != pHandleInstance->rxFilterConfig.aid)) {
            continue;
        }
        if ((pData[3] < pHandleInstance->rxFilterConfig.midMin) || (pData[3] > pHandleInstance->rxFilterConfig.midMax)) {
            continue;
        }

        uint16_t startIndex = pHandleInstance->rxIndexIn;

        for (j = 0; j < length; j++)
        {
            pHandleInstance->pDataBufferRx[pHandleInstance->rxIndexIn] = pData[j];
            pHandleInstance->rxIndexIn++;
            if (pHandleInstance->rxIndexIn >= pHandleInstance->dataBufferSize) 
            {
                pHandleInstance->rxIndexIn = 0;
            }
        }

        if (pHandleInstance->frameCount < MAX_FRAME_QUEUE_SIZE) 
        {
            pHandleInstance->frameQueue[pHandleInstance->queueTail].startIdx = startIndex;
            pHandleInstance->frameQueue[pHandleInstance->queueTail].len = length;
            pHandleInstance->queueTail = (pHandleInstance->queueTail + 1) % MAX_FRAME_QUEUE_SIZE;
            pHandleInstance->frameCount++; 
        } 
        else 
        {
            g_rxErrorStats.overflowError++;
        }
    }
}

/*************************************************
 Function: MpuPackHeader
 Description: Pack MPU data pack header
 Input: headerBuffer - Buffer to store the packed header
        pMsg - Pointer to the MpuHalDataPack_t structure containing the data pack information
 Output: void
 Return: void
 Others:
*************************************************/
static void MpuPackHeader(uint8_t headerBuffer[], const MpuHalDataPack_t* pMsg)
{
    headerBuffer[0] = 0x55;
    headerBuffer[1] = 0xAA;    
    headerBuffer[2] = pMsg->aid;
    headerBuffer[3] = pMsg->mid; 
    headerBuffer[4] = pMsg->subcommand;
    headerBuffer[5] = (pMsg->dataLength>>8)&0xFF; 
    headerBuffer[6] = pMsg->dataLength&0xFF;
    headerBuffer[7] = 0x00;    /*reserved*/ 
}

/*************************************************
 Function: MpuPackGetCrc
 Description: Calculate the CRC of the packed message
 Input: uint8_t header[]: pointer to the header
        const MpuHalDataPack_t* pMsg: pointer to the message
 Output: void
 Return: uint16_t: calculated CRC value
 Others:
*************************************************/
static uint16_t MpuPackGetCrc(uint8_t header[], const MpuHalDataPack_t *pMsg)
{
    uint16_t crc = MPU_HAL_CRC_DEFAULT_VALUE;
    crc = CcittCrc16(crc, header, MPU_PROTOCAL_HEADER_LEN);
    crc = CcittCrc16(crc, pMsg->pDataBuffer, pMsg->dataLength);
    return crc;
}

/*************************************************
 Function: MpuHalCycleProcess
 Description: Process the MPU HAL cycle
 Input: uint32_t cycleTime: cycle time in milliseconds
 Output: void
 Return: void
 Others:
*************************************************/
void MpuHalCycleProcess(uint32_t cycleTime)
{
    static uint32_t timeCount = 0u;
    
    if (E_MPU_HAL_START_STATE_POWER_OFF == g_mpuManage.startState)
    {
        timeCount = 0;
        g_mpuManage.startState = E_MPU_HAL_START_STATE_OFF_DELAY;
    }
    else if (E_MPU_HAL_START_STATE_OFF_DELAY == g_mpuManage.startState)
    {
        timeCount += cycleTime;
        if (timeCount >= MPU_HAL_CYC_TIME_CNT_50MS)
        {
            g_mpuManage.startState = E_MPU_HAL_START_STATE_POWER_ON;
            MpuHalSetPower(1);
        }
    }
    else if (E_MPU_HAL_START_STATE_POWER_ON == g_mpuManage.startState)
    {
        timeCount = 0;
        g_mpuManage.startState = E_MPU_HAL_START_STATE_POWER_DELAY;
    }
    else if (E_MPU_HAL_START_STATE_POWER_DELAY == g_mpuManage.startState)
    {
        timeCount += cycleTime;
        if (timeCount >= (MPU_HAL_CYC_TIME_CNT_400MS - cycleTime))
        {
            g_mpuManage.startState = E_MPU_HAL_START_STATE_KEY_ON;
        }
    }
    else if (E_MPU_HAL_START_STATE_KEY_ON == g_mpuManage.startState)
    {
        g_mpuManage.startState = E_MPU_HAL_START_STATE_KEY_ON_DELAY;
        MpuHalSetPowerkey(1);
        timeCount = 0;
    }
    else if (E_MPU_HAL_START_STATE_KEY_ON_DELAY == g_mpuManage.startState)
    {
        timeCount += cycleTime;
        if (timeCount > (MPU_HAL_CYC_TIME_CNT_55MS - cycleTime))
        {
            g_mpuManage.startState = E_MPU_HAL_START_STATE_KEY_OFF;
            timeCount = 0U;
            MpuHalSetPowerkey(0);
        }
    }
    else if (E_MPU_HAL_START_STATE_KEY_OFF == g_mpuManage.startState)
    {
        timeCount += cycleTime;
        if (timeCount > (MPU_HAL_CYC_TIME_CNT_50MS - cycleTime))
        {
            MpuHalSetVbus(1);
            g_mpuManage.startState = E_MPU_HAL_START_STATE_FINISH;
        }
    }

    if (g_mpuManage.wakeoutTimeCount < MPU_HAL_WAKEOUT_TIME_WAKEVALUE)
    {
        g_mpuManage.wakeoutTimeCount++;
        if (MPU_HAL_WAKEOUT_TIME_WAKEVALUE == g_mpuManage.wakeoutTimeCount)
        {
            MpuHalSetWakeOut(0);
        }
    }
}

/*************************************************
 Function: MpuHalOpen
 Description: Open a MPU HAL handle
 Input: void
 Output: void
 Return: int16_t: handle index, -1 if no available handle
 Others:
*************************************************/
int16_t MpuHalOpen(void)
{
    int16_t handle = -1, index = 0;
    for (index = 0; index < MPU_HAL_HANDLE_INSTANSE_MAX; index++) {
        if (g_mpuManage.rxHandle[index].useFlag == 0) {
            g_mpuManage.rxHandle[index].useFlag = 1;
            handle = index;
            break;
        }
    }
    return handle;
}

/*************************************************
 Function: MpuHalSetRxFilter
 Description: Set the RX filter for a given handle
 Input: int16_t handle: handle index
        const MpuHalFilter_t* pFilter: pointer to the filter configuration, NULL to disable filter
 Output: void
 Return: int16_t: MPU_HAL_STATUS_OK if successful, MPU_HAL_STATUS_ERR if error
 Others:
*************************************************/
int16_t MpuHalSetRxFilter(int16_t handle, const MpuHalFilter_t *pFilter)
{
    if ((handle >= 0) && (handle < MPU_HAL_HANDLE_INSTANSE_MAX))
    {
        if (pFilter != NULL)
        {
            g_mpuManage.rxHandle[handle].useRxFilter = 1;
            memcpy(&(g_mpuManage.rxHandle[handle].rxFilterConfig), pFilter, sizeof(MpuHalFilter_t));
        }
        else
        {
            g_mpuManage.rxHandle[handle].useRxFilter = 0;
        }
        return MPU_HAL_STATUS_OK;
    }
    return MPU_HAL_STATUS_ERR;
}

/*************************************************
 Function: MpuHalSetRxBuffer
 Description: Set RX buffer for MPU HAL handle
 Input: handle - MPU HAL handle index
        pBuffer - Pointer to the RX buffer
        bufferSize - Size of the RX buffer in bytes
 Output: void
 Return: int16_t - MPU_HAL_STATUS_OK if successful, MPU_HAL_STATUS_ERR if failed
 Others:
*************************************************/
int16_t MpuHalSetRxBuffer(int16_t handle,uint8_t* pBuffer, uint32_t bufferSize)
{
    if ((handle >= 0) && (handle < MPU_HAL_HANDLE_INSTANSE_MAX))
    {
        if (pBuffer != NULL)
        {
            g_mpuManage.rxHandle[handle].rxIndexIn = 0;
            g_mpuManage.rxHandle[handle].pDataBufferRx = pBuffer;
            g_mpuManage.rxHandle[handle].dataBufferSize = bufferSize;
            /* BOOT裸机配置 */
            g_mpuManage.rxHandle[handle].queueHead = 0;
            g_mpuManage.rxHandle[handle].queueTail = 0;
            g_mpuManage.rxHandle[handle].frameCount = 0;
            return MPU_HAL_STATUS_OK;
        }
        else
        {
            return MPU_HAL_STATUS_ERR;
        }        
    }
    else
    {
        return MPU_HAL_STATUS_ERR;
    }
}

/*************************************************
 Function: MpuHal_Transmit
 Description: Transmit MPU HAL data packet using UART
 Input: handle - MPU HAL handle index
        pTxMsg - Pointer to the MpuHalDataPack_t structure containing the data packet to be transmitted
 Output: void
 Return: int16_t - MPU_HAL_STATUS_OK if successful, MPU_HAL_STATUS_ERR if failed
 Others:
*************************************************/
int16_t MpuHal_Transmit(int16_t handle, const MpuHalDataPack_t* pTxMsg)
{
    int16_t ret = MPU_HAL_STATUS_ERR;
    uint8_t headerBuf[MPU_PROTOCAL_HEADER_LEN + 2];
    uint16_t crc;
    uint16_t txLength;
    uint8_t crcBuf[2];

    if (pTxMsg == NULL || pTxMsg->pDataBuffer == NULL) {
        TBOX_PRINT("UART HAL TX NULL ERROR\n");
        return MPU_HAL_STATUS_ERR;
    }

    if ((handle < 0) || (handle >= MPU_HAL_HANDLE_INSTANSE_MAX) || (pTxMsg->dataLength > 4000)) {
        TBOX_PRINT("UART HAL TX PARAM ERROR: handle=%d, dataLength=%d\n", handle, pTxMsg->dataLength);
        return MPU_HAL_STATUS_ERR;
    }

    MpuPackHeader(headerBuf, pTxMsg);
    crc = MpuPackGetCrc(headerBuf, pTxMsg);
    crcBuf[0] = (crc >> 8) & 0xFF;
    crcBuf[1] = crc & 0xFF;
    txLength = MPU_PROTOCAL_HEADER_LEN + pTxMsg->dataLength + 2;

    ret = MpuUartTransmit(pTxMsg, headerBuf, crcBuf, txLength);

    return ret;
}

/*************************************************
 Function: MpuHal_DiagnosticTask
 Description: Diagnostic task for MPU HAL errors
 Input: void
 Output: void
 Return: void
 Others:
*************************************************/
void MpuHal_PrintDiagnosticReport(void)
{
    static MpuHalErrorStats_t last = {0};
    
    // 检查是否有任何计数器发生了变化
    if (memcmp(&last, &g_rxErrorStats, sizeof(MpuHalErrorStats_t)) != 0) {
        TBOX_PRINT("\r\n====== UART RX ERROR ======\r\n");        
        if (g_rxErrorStats.headerError != last.headerError)
            TBOX_PRINT("[!] SYNC ERR: %u (Bad Header 0xAA)\r\n", g_rxErrorStats.headerError);
            
        if (g_rxErrorStats.lengthError != last.lengthError) {
            TBOX_PRINT("[! BUFFER FULL] PackLen:%u, Free:%u \r\n", 
                    g_rxErrorStats.lastFullHandleReqLen, g_rxErrorStats.lastFullHandleFree);
        }

        if (g_rxErrorStats.rxOverflowCnt != last.rxOverflowCnt) {
            TBOX_PRINT("[RX FIFO] Cnt:%u \r\n", g_rxErrorStats.rxOverflowCnt);
        }
            
        if (g_rxErrorStats.overflowError != last.overflowError)
            TBOX_PRINT("[!!!] CPU LAG: %u (Task Queue Full)\r\n", g_rxErrorStats.overflowError);
        last = g_rxErrorStats; // 更新快照
        TBOX_PRINT("==============================\r\n");
    }
}

/*************************************************
 Function: MpuHal_Receive
 Description: Receive MPU HAL data packet
 Input: handle - MPU HAL handle index
        pRxMsg - Pointer to the MpuHalDataPack_t structure where the received data packet will be stored
        waitTime - Maximum time to wait for a data packet to be received (in ticks)
 Output: void
 Return: int16_t - MPU_HAL_STATUS_OK if successful, MPU_HAL_STATUS_ERR if failed
 Others:
*************************************************/
int16_t MpuHal_Receive(int16_t handle,MpuHalDataPack_t* pRxMsg, uint32_t waitTime )
{
    uint16_t len = 0u, crc = 0u,crcRx = 0u, index = 0u, i = 0u;
    uint8_t packHeader[MPU_PROTOCAL_HEADER_LEN];
    int16_t ret = MPU_HAL_STATUS_ERR;

    if ((handle < 0) || (handle >= MPU_HAL_HANDLE_INSTANSE_MAX)) {
        TBOX_PRINT("UART HAL RX PARAM ERROR: handle=%d\n", handle);
        return MPU_HAL_STATUS_ERR;
    }

    if (pRxMsg == NULL) {
        TBOX_PRINT("UART HAL RX NULL ERROR\n");
        return MPU_HAL_STATUS_ERR;
    }
    

    index = handle & 0xFF;
    MpuHalHandle_t* pHandle = &g_mpuManage.rxHandle[index];


    uint32_t waitCount = waitTime;
    while ((pHandle->frameCount == 0) && (waitCount > 0))
    {
        Delay_ms(1);
        waitCount--;
    }

    if (pHandle->frameCount > 0)
    {
        index = pHandle->frameQueue[pHandle->queueHead].startIdx;
        len   = pHandle->frameQueue[pHandle->queueHead].len;

        if ((pRxMsg->dataBufferSize + MPU_PROTOCAL_HEADER_LEN + 2) < len)
        {
            TBOX_PRINT("UART HAL RX BUFFER SIZE ERROR\n");
        }
        else
        {
            for (i = 0u; i < MPU_PROTOCAL_HEADER_LEN; i++)
            {
                packHeader[i] = pHandle->pDataBufferRx[index];
                index = (index + 1) % pHandle->dataBufferSize;
            }
            for (i = 0u; i < (len - MPU_PROTOCAL_HEADER_LEN - 2); i++)
            {
                pRxMsg->pDataBuffer[i] = pHandle->pDataBufferRx[index];
                index = (index + 1) % pHandle->dataBufferSize;
            }
            crcRx = (pHandle->pDataBufferRx[index] << 8);
            index = (index + 1) % pHandle->dataBufferSize;
            crcRx |= pHandle->pDataBufferRx[index];
            pRxMsg->dataLength = len - (MPU_PROTOCAL_HEADER_LEN + 2);
            crc = MpuPackGetCrc(packHeader, pRxMsg);
            
            if (crcRx == crc)
            {
                // TBOX_PRINT("UART RX SUCCESS: AID=%02X MID=%02X Len=%d\r\n", packHeader[2], packHeader[3], pRxMsg->dataLength);
                pRxMsg->aid = packHeader[2];
                pRxMsg->mid = packHeader[3];
                pRxMsg->subcommand = packHeader[4];
                pRxMsg->dataLength = len - (MPU_PROTOCAL_HEADER_LEN + 2); 
                ret = MPU_HAL_STATUS_OK;
            } 
            else 
            {
                TBOX_PRINT("UART RX CRC ERROR:%04X Rx:%04X\r\n", crc, crcRx);
            }
        }

        pHandle->queueHead = (pHandle->queueHead + 1) % MAX_FRAME_QUEUE_SIZE;
        
        __disable_irq();
        pHandle->frameCount--;
        __enable_irq();
    }

    return ret;
}

/*************************************************
 Function: MpuHalStart
 Description: Start the MPU HAL
 Input: void
 Output: void
 Return: void
 Others:
*************************************************/
void MpuHalStart(void)
{
    g_mpuManage.wakeoutTimeCount = MPU_HAL_WAKEOUT_TIME_INIT;
    MpuHalSetWakeOut(0);
    MpuHalSetPower(1);
    g_mpuManage.startState = E_MPU_HAL_START_STATE_POWER_ON;
}

/*************************************************
 Function: MpuHalStartIsFinished
 Description: Check if the MPU HAL start process is finished
 Input: void
 Output: void
 Return: int16_t: MPU_HAL_STATUS_OK if finished, MPU_HAL_STATUS_ERR if not finished
 Others:
*************************************************/
int16_t MpuHalStartIsFinished(void)
{
    return (E_MPU_HAL_START_STATE_FINISH == g_mpuManage.startState) ? MPU_HAL_STATUS_OK : MPU_HAL_STATUS_ERR;
}

/*************************************************
 Function: MpuHalReset
 Description: Reset the MPU HAL
 Input: void
 Output: void
 Return: void
 Others:
*************************************************/
void MpuHalReset(void)
{
    MpuHalSetPower(0);
    g_mpuManage.startState = E_MPU_HAL_START_STATE_POWER_OFF;
}

/*************************************************
 Function: MpuHalResetIsFinished
 Description: Check if the MPU HAL reset process is finished
 Input: void
 Output: void
 Return: int16_t: MPU_HAL_STATUS_OK if finished, MPU_HAL_STATUS_ERR if not finished
 Others:
*************************************************/
int16_t MpuHalResetIsFinished(void)
{
    return (E_MPU_HAL_START_STATE_FINISH == g_mpuManage.startState) ? MPU_HAL_STATUS_OK : MPU_HAL_STATUS_ERR;
}

/*************************************************
 Function: MpuHalPowerOff
 Description: Power off the MPU HAL
 Input: void
 Output: void
 Return: void
 Others:
*************************************************/
void MpuHalPowerOff(void)
{
    MpuHalSetPower(0);
    g_mpuManage.startState = E_MPU_HAL_START_STATE_IDLE;
}


/*************************************************
 Function: MpuHalGetWakeMode
 Description: Get the current MPU HAL mode (wake or sleep)
 Input: void
 Output: void
 Return: int16_t: 0 for sleep mode, 1 for wake mode
 Others:
*************************************************/
int16_t MpuHalGetWakeMode(void)
{
    return g_mpuManage.wakeMode;
}
/*************************************************
 Function: UartProtocalProcess
 Description: Process UART protocol data
 Input: uint8_t *pData - pointer to protocol data
        uint16_t dataLength - length of protocol data
 Output: void
 Return: void
 Others:
*************************************************/
static void UartProtocalProcess(uint8_t *pData, uint16_t dataLength)
{
    uint16_t i = 0u;
    MpuUartProtocalBuffer_t* pProtocalData;

    pProtocalData = &g_mpuUartProtocalBuffer;
    for (i = 0; i < dataLength; i++)
    {
        if (0 == pProtocalData->dataCount)
        {
            if (0x55 == pData[i])
            {
                pProtocalData->data[pProtocalData->dataCount] = pData[i];
                pProtocalData->dataCount++;
            }      
        }
        else if (1 == pProtocalData->dataCount)
        {
            if (0xAA == pData[i])
            {
                pProtocalData->data[pProtocalData->dataCount] = pData[i];
                pProtocalData->dataCount++;
            }
            else if (0x55 == pData[i]) 
            {
                g_rxErrorStats.headerError++;
            }
            else
            {
                g_rxErrorStats.headerError++;
                pProtocalData->dataCount = 0;
            }
        }
        else if (pProtocalData->dataCount < MPU_PROTOCAL_HEADER_LEN)
        {
            pProtocalData->data[pProtocalData->dataCount] = pData[i];
            pProtocalData->dataCount++;
            if (MPU_PROTOCAL_HEADER_LEN == pProtocalData->dataCount)
            {
                pProtocalData->dataLength = ((uint16_t)(pProtocalData->data[5]) << 8) + pProtocalData->data[6];
                if (pProtocalData->dataLength > (sizeof(pProtocalData->data) - (MPU_PROTOCAL_HEADER_LEN + 2)))
                {
                    g_rxErrorStats.lastFullHandleReqLen = pProtocalData->dataLength;
                    g_rxErrorStats.lastFullHandleFree = sizeof(pProtocalData->data) - (MPU_PROTOCAL_HEADER_LEN + 2);
                    g_rxErrorStats.lengthError++;                    
                    pProtocalData->dataCount = 0;
                    pProtocalData->dataLength = 0;
                }
            }
        }
        else
        {
            pProtocalData->data[pProtocalData->dataCount] = pData[i];
            pProtocalData->dataCount++;
            if (pProtocalData->dataCount >= (pProtocalData->dataLength + (MPU_PROTOCAL_HEADER_LEN + 2)))
            {
                MpuDataDispatch(pProtocalData->data, pProtocalData->dataCount);
                pProtocalData->dataLength = 0;
                pProtocalData->dataCount = 0;
            }
        }
    }
}

/*************************************************
 Function: MpuHalMainUartSleep
 Description: Put the main UART interface for MPU communication into sleep mode
 Input: void
 Output: void
 Return: void
 Others:
*************************************************/
static void MpuHalMainUartSleep(void)
{
    Uart_Hal_DeInit(MPU_HAL_UART_ID); // 内部已禁用中断
    Gpio_Hal_SetMuxMode(MPU_HAL_UART_RX_PORT, MPU_HAL_UART_RX_PIN, MPU_HAL_UART_RX_GPIO);
    Gpio_Hal_SetPinDirection(MPU_HAL_UART_RX_PORT, MPU_HAL_UART_RX_PIN, GPIO_INPUT_DIRECTION);
    Gpio_Hal_SetMuxMode(MPU_HAL_UART_TX_PORT, MPU_HAL_UART_TX_PIN, MPU_HAL_UART_TX_GPIO);
    Gpio_Hal_SetPinDirection(MPU_HAL_UART_TX_PORT, MPU_HAL_UART_TX_PIN, GPIO_INPUT_DIRECTION);
    Gpio_Hal_SetPullSel(MPU_HAL_UART_TX_PORT, MPU_HAL_UART_TX_PIN, PORT_INTERNAL_PULL_DOWN_ENABLED);
}


/*************************************************
 Function: MpuUartTransmit
 Description: Transmit data via UART
 Input: const uint8_t* pTxData - Pointer to the data to transmit
        uint16_t txLength - Length of the data to transmit
 Output: void
 Return: int16_t - Status code (0 for success, -1 for error)
 Others:
*************************************************/
static int16_t MpuUartTransmit(const MpuHalDataPack_t* pTxMsg, const uint8_t* headerBuf, const uint8_t* crcBuf, uint16_t txLength)
{
    uint32_t waitMs = 20;

    if (txLength == 0)
    {
        TBOX_PRINT("UART TX LEN ERROR\n"); // 内部函数, 确保入参非空,此处不作非空判断提高性能
        return -1;
    }

    if (txLength > BOOT_UART_TX_MAX_LEN)
    {
        TBOX_PRINT("UART TX BUF OVERFLOW\n");
        return -1;
    }

    while (g_uartDmaTxFlag == 1)
    {
        Delay_ms(1);
        waitMs--;
        PeripheralHalFeedWatchDog();
        if (waitMs == 0)
        {
            TBOX_PRINT("UART TX DMA BUSY DEADLOCK\n");
            Uart_Hal_AbortSendingData(MPU_HAL_UART_ID);
            g_uartDmaTxFlag = 0;
            return -1;
        }
    }
    
    memcpy(s_bootTxBuffer, headerBuf, MPU_PROTOCAL_HEADER_LEN);
    memcpy(&s_bootTxBuffer[MPU_PROTOCAL_HEADER_LEN], pTxMsg->pDataBuffer, pTxMsg->dataLength);
    memcpy(&s_bootTxBuffer[MPU_PROTOCAL_HEADER_LEN + pTxMsg->dataLength], crcBuf, 2);

#if 0
    // 调试打印
    uint16_t k = 0u;
    TBOX_PRINT("[BOOT DMA TX] Len:%d Data:", txLength);
    for(k = 0; k < txLength; k++)
    {
        TBOX_PRINT("%02X ", s_bootTxBuffer[k]);
    }
    TBOX_PRINT("\r\n");
#endif

    Uart_Hal_SendData(MPU_HAL_UART_ID, s_bootTxBuffer, txLength);
    

    g_uartDmaTxFlag = 1;

    return 0;
}

/*************************************************
  Function: MpuHalRxBuf_Init
  Description: 初始化接收缓冲区
  Input: void
  Output: void
  Return: void
  Others:
*************************************************/
static void MpuHalRxBuf_Init(void)
{
    /* 初始化环形缓冲区索引 */
    s_rxRingWriteIdx = 0;
    s_rxRingReadIdx = 0;
    
    /* 初始化数据块队列 */
    s_rxBlockQueue.writeIdx = 0;
    s_rxBlockQueue.readIdx = 0;
    s_rxBlockQueue.count = 0;
    
    /* 清零缓冲区 */
    memset(s_rxRingBuf, 0, sizeof(s_rxRingBuf));
    memset(s_rxTempBuf, 0, sizeof(s_rxTempBuf));
}

/*************************************************
  Function: MpuHal_ProcessRxData
  Description: 主循环中处理接收到的UART数据
  Input: void
  Output: void
  Return: void
  Others: 
*************************************************/
void MpuHal_ProcessRxData(void)
{
    RxBlockInfo_t blockInfo;
    uint16_t readIdx = 0;
    uint16_t i = 0;
    
    /* while循环处理所有待处理数据块 */
    while (s_rxBlockQueue.count > 0) {
        /* 从队列获取数据块信息 */
        blockInfo = s_rxBlockQueue.blocks[s_rxBlockQueue.readIdx];
        
        /* 数据长度应该始终有效（在中断中已检查） */
        readIdx = blockInfo.startIdx;
        
        /* 从环形缓冲区读取数据到临时缓冲区（处理环形边界） */
        for (i = 0; i < blockInfo.len; i++) {
            s_rxTempBuf[i] = s_rxRingBuf[readIdx];
            readIdx++;
            if (readIdx >= MPU_HAL_RX_RING_SIZE) {
                readIdx = 0;
            }
        }
        
        /* 更新环形缓冲区读索引 */
        s_rxRingReadIdx = readIdx;
        
        /* 调用协议处理函数 */
        UartProtocalProcess(s_rxTempBuf, blockInfo.len);
        
        /* 更新队列读索引和计数 */
        s_rxBlockQueue.readIdx = (s_rxBlockQueue.readIdx + 1) % RX_BLOCK_QUEUE_SIZE;
        s_rxBlockQueue.count--;
    }
}