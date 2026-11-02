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
#include "FreeRTOS.h"
#include "queue.h"
#include "crc8_16_32.h"
#include <string.h>
#include "Spi_Hal.h"
#include "Gpio_Hal.h"
#include "Ckgen_Hal.h"
#include "Spm_Hal.h"
#include "Dma_Hal.h"
#include "logHal.h"
#include "timerHal.h"
#include "Rcm_Hal.h"
#include "Core_Hal.h"
#include "semphr.h"
#include "event_groups.h"
// #include "peripheralHal.h"

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
#define MPU_HAL_RX_QUEUE_LEN                    10
#define MPU_HAL_TX_QUEUE_LEN                    40
#define MPU_HAL_UART_TX_ONCE_LEN                40
#define MPU_HAL_UART_TX_TEMP_LEN                50
#define MPU_HAL_SEND_IRQ_ENABLE                 (1U) 
#define MPU_HAL_SEND_QUEUE_WAIT_CNT             (10U)
#define MPU_HAL_RING_BUFFER                     (4096 * 2)

/* --- Hardware Instances Mapping --- */
#if (MPU_HAL_UART2_DEBUG == 0)
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
#else
#define UART_BAUDRATE                           (115200)
#define MPU_HAL_UART_ID                         2  // Map to UART2
#define UART_DMA_TX_REQ                         DMA_REQ_UART2_TX
#define UART_DMA_RX_REQ                         DMA_REQ_UART2_RX
#define MPU_HAL_UART_RX_PORT                    PORTID_A
#define MPU_HAL_UART_RX_PIN                     8
#define MPU_HAL_UART_RX_GPIO                    PORT_MUX_AS_GPIO
#define MPU_HAL_UART_RX_MUX                     PORT_MUX_ALT2
#define MPU_HAL_UART_TX_PORT                    PORTID_A
#define MPU_HAL_UART_TX_PIN                     9
#define MPU_HAL_UART_TX_GPIO                    PORT_MUX_AS_GPIO
#define MPU_HAL_UART_TX_MUX                     PORT_MUX_ALT2
#endif

#define MPU_HAL_SPI_ID                          0  // Map to SPI0
#define SPI_DMA_TX_REQ                          DMA_REQ_SPI0_TX
#define SPI_DMA_RX_REQ                          DMA_REQ_SPI0_RX
#define MPU_HAL_SPI_CLK_PORT                    PORTID_D
#define MPU_HAL_SPI_CLK_PIN                     15U
#define MPU_HAL_SPI_CLK_MUX                     PORT_MUX_ALT4
#define MPU_HAL_SPI_MOSI_PORT                   PORTID_B
#define MPU_HAL_SPI_MOSI_PIN                    4U
#define MPU_HAL_SPI_MOSI_MUX                    PORT_MUX_ALT3
#define MPU_HAL_SPI_MISO_PORT                   PORTID_D
#define MPU_HAL_MISO_PIN                        16U
#define MPU_HAL_SPI_MISO_MUX                    PORT_MUX_ALT4
#define MPU_HAL_SPI_CS_PORT                     PORTID_B
#define MPU_HAL_SPI_CS_PIN                      5U
#define MPU_HAL_SPI_CS_MUX                      PORT_MUX_ALT4


#define MPU_HAL_SPI_IEQ_PORT                   PORTID_E
#define MPU_HAL_SPI_IEQ_PORT_PIN               16U
#define MPU_HAL_SPI_IEQ_PORT_MUX               PORT_MUX_AS_GPIO

#define MPU_HAL_SPI_DMA_BUFFSIZE                2048
#define UART_DMA_TX_CHANNEL                     1
#define UART_DMA_RX_CHANNEL                     0
#define SPI_DMA_TX_CHANNEL                      2
#define SPI_DMA_RX_CHANNEL                      3

#define MPU_HAL_SPI_HEADER_FIRST_BYTE           (0x55u)
#define MPU_HAL_SPI_HEADER_SECOND_BYTE          (0xBBu)
#define MPU_HAL_SPI_HEADER_SIZE                 (6u)  

#define MPU_EVENT_UART_INIT_DONE                (1 << 0)

/****************************** Type Definitions ******************************/

typedef struct
{
    QueueHandle_t txQueueHandle;
    uint8_t buffer[MPU_HAL_MAX_BUFFSIZE];
    uint16_t index;
} MpuQueueTxBuffer_t;

typedef struct
{
    uint8_t useFlag;
    uint8_t useRxFilter;
    uint8_t *pDataBufferRx;
    uint16_t dataBufferSize;
    uint16_t rxIndexIn;
    QueueHandle_t rxQueueHandle;
    MpuHalFilter_t rxFilterConfig;
} MpuHalHandle_t;

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
    uint8_t wakeMode; 
    uint8_t wakeoutTimeCount;
    MpuHalStartState_e startState;
    MpuHalHandle_t rxHandle[MPU_HAL_HANDLE_INSTANSE_MAX];
    uint8_t mcuRequest; 
} MpuHalManage_t;

typedef struct 
{
    uint8_t data[MPU_HAL_RX_ONE_FRAME_MAX_SIZE];
    uint16_t dataCount;
    uint16_t dataLength;
} MpuUartProtocalBuffer_t;

typedef enum {
    PROTOCOL_STATE_IDLE,                
    PROTOCOL_STATE_WAIT_HEADER2,        
    PROTOCOL_STATE_WAIT_HEADER,         
    PROTOCOL_STATE_WAIT_SPI_HEADER,     
    PROTOCOL_STATE_WAIT_DATA            
} ProtocolStateType;

typedef struct {
    uint8_t buf[MPU_HAL_RING_BUFFER];     
    volatile uint16_t head;             
    volatile uint16_t tail;             
} RingBufferType;

typedef struct {
    RingBufferType rx_ring_buf;
    ProtocolStateType proto_state;
    MpuUartProtocalBuffer_t proto_buf;
    uint8_t init_success;
    uint16_t err_cnt;
} MpuUartHandleTypeDef;


/****************************** Global Variables ******************************/
static MpuHalManage_t g_mpuManage;

static MpuUartHandleTypeDef g_mpuUartRingBuf = {
    .rx_ring_buf = {.head = 0, .tail = 0},
    .proto_state = PROTOCOL_STATE_IDLE,
    .init_success = 0,
    .err_cnt = 0
};

static MpuUartHandleTypeDef g_mpuSpiRingBuf = {
    .rx_ring_buf = {.head = 0, .tail = 0},
    .proto_state = PROTOCOL_STATE_IDLE,
    .init_success = 0,
    .err_cnt = 0
};
static uint8_t g_uartDmaRxBufRaw[MPU_HAL_UART_BUFFSIZE];
static uint8_t spi_ready = 0u;
static volatile uint8_t g_spi_sleeping = 0u;
static volatile uint8_t g_uart_sleeping = 0u;
static MpuQueueTxBuffer_t g_mpuTxBuf[MPU_HAL_TX_MODE_MAX];


static SemaphoreHandle_t   g_uartTxCompleteSem = NULL;
static volatile uint32_t g_sleepGenerationCnt = 0;
static EventGroupHandle_t g_mpuInitEvent = NULL;

/****************************** Function Declarations *************************/
static void MpuHalGpioInit(void);
static void MpuHalGpioSetMode(uint8_t mode);
static void MpuHalMainUartInit(void);
static void MpuHalMainUartSleep(void);
static void MpuHalSetPowerkey(uint8_t flag);
static void MpuHalSetWakeOut(uint8_t flag);
static void MpuHalSetVbus(uint8_t flag);
static void MpuHalSetGpio(uint8_t port, uint8_t gpioNum, uint8_t flag);
static void MpuDataDispatch(const uint8_t *pData, uint16_t length, uint8_t irq);
static uint8_t MpuPackHeader(uint8_t headerBuffer[], const MpuHalDataPack_t *pMsg);
static uint16_t MpuPackGetCrc(uint8_t header[], const MpuHalDataPack_t *pMsg);
static uint8_t MpuHalReadRingBuf(RingBufferType *rb, uint8_t *data);
static void UartProtocalProcess(void);
static void SpiProtocalProcess(void);
static void MpuHalSetRingBuf(uint8_t *recv_data, uint16_t length, MpuHalTXMode_e txMode);
static int16_t MpuHalQueueSend(MpuQueueTxBuffer_t *pTxBuffer, const uint8_t *pTxData, uint16_t txLength, MpuHalTXMode_e txMode);
static uint32_t MpuHalPacketSend(MpuHalTXMode_e txMode, uint32_t queueData, MpuQueueTxBuffer_t *pTxBuffer);

/* SDK Specific Callbacks */
static void UartHalEventCallback(uint8 Instance, Uart_EventType Event);
static void SpiHalEventCallback(uint8 Instance, uint16 Status);
static void MpuHal_SpiInit(void);
static void MpuHal_UartDmaInit(void);
static void MpuHalMainSpiSleep(void);

/****************************** Public Function Implementations ******************************/
/*************************************************
 Function: MpuHalInit
 Description: Initialize the MPU HAL
 Input: void
 Output: void
 Return: void
 Others:
*************************************************/
void MpuHalInit(void)
{
    memset(&g_mpuManage, 0, sizeof(g_mpuManage));
    g_mpuManage.wakeoutTimeCount = MPU_HAL_WAKEOUT_TIME_INIT;
    g_mpuManage.wakeMode = MPU_HAL_WAKE_MODE_MPU_SLEEP;
    g_mpuTxBuf[MPU_HAL_UART_MODE].txQueueHandle = xQueueCreate(MPU_HAL_TX_QUEUE_LEN, sizeof(uint32_t));
    g_mpuTxBuf[MPU_HAL_SPI_MODE].txQueueHandle = xQueueCreate(MPU_HAL_TX_QUEUE_LEN, sizeof(uint32_t));
    if (g_uartTxCompleteSem == NULL) {
        g_uartTxCompleteSem = xSemaphoreCreateBinary();
    }
    if (g_mpuInitEvent == NULL) {
        g_mpuInitEvent = xEventGroupCreate();
    }
    MpuHalGpioInit();
    MpuHal_UartDmaInit();
    MpuHalMainUartInit();
    MpuHal_SpiInit();
    /* 初始化完成事件置位 */
    xEventGroupSetBits(g_mpuInitEvent, MPU_EVENT_UART_INIT_DONE);
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
        ,
        {
            .HwChannelId = SPI_DMA_TX_CHANNEL,
            .VirtualChannelId = SPI_DMA_TX_CHANNEL,
            .Priority = DMA_CHANNEL_PRIORITY_LOW,
            .ReqSrc = SPI_DMA_TX_REQ
        },
        {
            .HwChannelId = SPI_DMA_RX_CHANNEL,
            .VirtualChannelId = SPI_DMA_RX_CHANNEL,
            .Priority = DMA_CHANNEL_PRIORITY_LOW,
            .ReqSrc = SPI_DMA_RX_REQ
        }
    };
    Dma_ConfigType dmaCfg;
    memset(&dmaCfg, 0, sizeof(dmaCfg));
    dmaCfg.ChannelCnt = sizeof(dmaChannelConfigs) / sizeof(dmaChannelConfigs[0]);
    dmaCfg.ChannelCfg = dmaChannelConfigs;
    Dma_Hal_Init(&dmaCfg);
}

/*************************************************
 Function: MpuHalGpioInit
 Description: Initialize the GPIO used by MPU
 Input: void
 Output: void
Return:void
 Others:
*************************************************/
static void MpuHalGpioInit(void)
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

/*************************************************
 Function: MpuHalGpioSetMode
 Description: Set the GPIO mode for MPU
 Input: void
 Output: void
 Return:void
 Others:
*************************************************/
static void MpuHalGpioSetMode(uint8_t mode)
{
    if (mode == 1)
    {
        Gpio_Hal_WritePin(PORTID_E, 5, GPIO_HIGH_LEVEL);
        Gpio_Hal_WritePin(PORTID_D, 5, GPIO_HIGH_LEVEL);
        Gpio_Hal_WritePin(PORTID_D, 8, GPIO_HIGH_LEVEL);                  //ECALL_LED_R
    }
    else
    {
        Gpio_Hal_WritePin(PORTID_E, 5, GPIO_LOW_LEVEL);
        Gpio_Hal_WritePin(PORTID_D, 5, GPIO_LOW_LEVEL);
        Gpio_Hal_WritePin(PORTID_D, 8, GPIO_LOW_LEVEL);                   //ECALL_LED_R
    }
}

static void UartHalEventCallback(uint8 Instance, Uart_EventType Event)
{
    uint32_t remainBytes = 0;
    
    if (Event == UART_EVENT_RX_FULL)
    {
        MpuHalSetRingBuf(g_uartDmaRxBufRaw, MPU_HAL_UART_BUFFSIZE, MPU_HAL_UART_MODE);
        Uart_Hal_ReceiveData(MPU_HAL_UART_ID, g_uartDmaRxBufRaw, MPU_HAL_UART_BUFFSIZE);
    }
    else if ((Event == UART_EVENT_IDLE_LINE) || (Event == UART_EVENT_END_TRANSFER))
    {
        Uart_Hal_GetReceiveStatus(Instance, &remainBytes);
        uint16_t receivedLen = MPU_HAL_UART_BUFFSIZE - remainBytes;
        if ((receivedLen > 0) && (receivedLen <= MPU_HAL_UART_BUFFSIZE)) {
            MpuHalSetRingBuf(g_uartDmaRxBufRaw, receivedLen, MPU_HAL_UART_MODE);
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
}

static void UartHalTxEventCallback(uint8 Instance, Uart_EventType Event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (Event == UART_EVENT_ERROR || Event == UART_EVENT_END_TRANSFER)
    {
        if (g_uartTxCompleteSem != NULL)
        {
            xSemaphoreGiveFromISR(g_uartTxCompleteSem, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
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
    MpuUartHandleTypeDef *huart = &g_mpuUartRingBuf;
    huart->init_success = 0;

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
    
    huart->init_success = 1;
    TBOX_PRINT("UART init success with new HAL!\n");
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
    MpuHalSetGpio(PORTID_A, 14, flag);
    LogHalUpLoadLog("Set Vbus %d", flag);
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
static void MpuDataDispatch(const uint8_t *pData, uint16_t length, uint8_t irq)
{
    uint16_t i, j, instanceMax;
    uint32_t data;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    MpuHalHandle_t *pHandleInstance;

    instanceMax = MPU_HAL_HANDLE_INSTANSE_MAX;
    for (i = 0u; i < instanceMax; i++)
    {
        pHandleInstance = g_mpuManage.rxHandle + i;
        if ((pHandleInstance->useFlag == 1u) && (pHandleInstance->pDataBufferRx != NULL))
        {
            if ((pHandleInstance->useRxFilter == 1) && (pData[2] == pHandleInstance->rxFilterConfig.aid))
            {
                if ((pData[3] >= pHandleInstance->rxFilterConfig.midMin) && (pData[3] <= pHandleInstance->rxFilterConfig.midMax))
                {
                    data = (pHandleInstance->rxIndexIn << 16);
                    for (j = 0u; j < length; j++)
                    {
                        pHandleInstance->pDataBufferRx[pHandleInstance->rxIndexIn] = pData[j];
                        pHandleInstance->rxIndexIn++;
                        if (pHandleInstance->rxIndexIn >= pHandleInstance->dataBufferSize)
                        {
                            pHandleInstance->rxIndexIn = 0;
                        }
                    }
                    data += length;
                    if (irq != 0u)
                    {
                        xQueueSendFromISR(pHandleInstance->rxQueueHandle, &data, &xHigherPriorityTaskWoken);
                    }
                    else
                    {
                        xQueueSend(pHandleInstance->rxQueueHandle, &data, 0);
                    }
                }
            }
        }
    }
}

/*************************************************
 Function: MpuPackHeader
 Description: Pack the protocol header
 Input: uint8_t headerBuffer[]: buffer to store the packed header
        const MpuHalDataPack_t* pMsg: pointer to the message to be packed
 Output: void
 Return: void
 Others:
*************************************************/
static uint8_t MpuPackHeader(uint8_t headerBuffer[], const MpuHalDataPack_t *pMsg)
{
    headerBuffer[0] = MPU_HAL_PACK_FIX_HEADER_0X55;
    headerBuffer[1] = MPU_HAL_PACK_FIX_HEADER_0XAA;
    headerBuffer[2] = pMsg->aid;
    headerBuffer[3] = pMsg->mid;
    headerBuffer[4] = pMsg->subcommand;
    headerBuffer[5] = (pMsg->dataLength >> MPU_HAL_8BIT_SHIFT) & MPU_HAL_8BIT_MASK;
    headerBuffer[6] = pMsg->dataLength & MPU_HAL_8BIT_MASK;
    headerBuffer[7] = MPU_HAL_PACK_HEADER_RESERVED;
    return MPU_PROTOCAL_HEADER_LEN;
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
    taskENTER_CRITICAL();
    for (index = 0; index < MPU_HAL_HANDLE_INSTANSE_MAX; index++) {
        if (g_mpuManage.rxHandle[index].useFlag == 0) {
            g_mpuManage.rxHandle[index].useFlag = 1;
            handle = index;
            break;
        }
    }
    taskEXIT_CRITICAL();
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
 Description: Set the RX buffer for a given handle
 Input: int16_t handle: handle index
        uint8_t* pBuffer: pointer to the RX buffer
        uint32_t bufferSize: size of the RX buffer
 Output: void
 Return: int16_t: MPU_HAL_STATUS_OK if successful, MPU_HAL_STATUS_ERR if error
 Others:
*************************************************/
int16_t MpuHalSetRxBuffer(int16_t handle, uint8_t *pBuffer, uint32_t bufferSize)
{
    if ((handle >= 0) && (handle < MPU_HAL_HANDLE_INSTANSE_MAX))
    {
        if (pBuffer != NULL)
        {
            g_mpuManage.rxHandle[handle].rxQueueHandle = xQueueCreate(MPU_HAL_RX_QUEUE_LEN, sizeof(uint32_t));
            g_mpuManage.rxHandle[handle].rxIndexIn = 0;
            g_mpuManage.rxHandle[handle].pDataBufferRx = pBuffer;
            g_mpuManage.rxHandle[handle].dataBufferSize = bufferSize;
            return MPU_HAL_STATUS_OK;
        }
        else
        {
            return MPU_HAL_NULL_PTR_ERR;
        }
    }
    return MPU_HAL_STATUS_ERR;
}

/*************************************************
 Function: MpuHalQueueSend
 Description: Transmit data via UART
 Input: const uint8_t* pTxData: pointer to the data to be transmitted
        uint16_t txLength: length of the data to be transmitted
 Output: void
 Return: int16_t: 0 if successful, -1 if error
 Others:
*************************************************/
static int16_t MpuHalQueueSend(MpuQueueTxBuffer_t *pTxBuffer, const uint8_t *pTxData, uint16_t txLength, MpuHalTXMode_e txMode)
{
    uint32_t ret = 0u, data = 0u, resnum = 0u;
    uint16_t i = 0u;
    
    if ((pTxBuffer == NULL) || (pTxData == NULL)) return MPU_HAL_NULL_PTR_ERR;
    
    resnum = uxQueueMessagesWaiting(pTxBuffer->txQueueHandle);
    if (resnum >= MPU_HAL_TX_QUEUE_LEN) {
        return MPU_HAL_QUEUE_ERR;
    }
    
    data = pTxBuffer->index << MPU_HAL_16BIT_SHIFT;
    for (i = 0; i < txLength; i++) {
        pTxBuffer->buffer[pTxBuffer->index++] = pTxData[i];
        if (pTxBuffer->index >= MPU_HAL_MAX_BUFFSIZE) pTxBuffer->index = 0;
    }
    
    data |= txLength;
    if (pdPASS != xQueueSend(pTxBuffer->txQueueHandle, &data, 0))
    {
        if (pTxBuffer->index >= txLength) pTxBuffer->index -= txLength;
        else pTxBuffer->index = MPU_HAL_MAX_BUFFSIZE - (txLength - pTxBuffer->index);
        ret = MPU_HAL_QUEUE_ERR;
    }
    return ret;
}

/*************************************************
 Function: MpuHalTransmit
 Description: Transmit a data packet via MPU HAL
 Input: int16_t handle: handle index
        const MpuHalDataPack_t* pTxMsg: pointer to the data packet to be transmitted
        MpuHalTXMode_e txMode: transmission mode (UART or SPI)
 Output: void
 Return: int16_t: MPU_HAL_STATUS_OK if successful, MPU_HAL_STATUS_ERR if error
 Others:
*************************************************/
int16_t MpuHalTransmit(int16_t handle, const MpuHalDataPack_t *pTxMsg, MpuHalTXMode_e txMode)
{
    static uint8_t sTxBuffer[MPU_HAL_PROC_BUFFER_LEN];
    uint16_t crc, index;
    uint8_t packHeader[MPU_HAL_PACK_HEADER_BUF_LEN];
    int16_t ret = MPU_HAL_STATUS_ERR;
    
    if (txMode >= MPU_HAL_TX_MODE_MAX) return MPU_HAL_MODE_ERR;

    if ((handle >= 0) && (handle < MPU_HAL_HANDLE_INSTANSE_MAX) && pTxMsg != NULL)
    {
        if (pTxMsg->dataLength < MPU_HAL_TXMSG_MAX_LEN)
        {
            taskENTER_CRITICAL();
            MpuPackHeader(packHeader, pTxMsg);
            crc = MpuPackGetCrc(packHeader, pTxMsg);
            index = 0;
            memcpy(sTxBuffer + index, packHeader, MPU_PROTOCAL_HEADER_LEN);
            index += MPU_PROTOCAL_HEADER_LEN;
            memcpy(sTxBuffer + index, pTxMsg->pDataBuffer, pTxMsg->dataLength);
            index += pTxMsg->dataLength;
            sTxBuffer[index++] = (crc >> MPU_HAL_8BIT_SHIFT) & MPU_HAL_8BIT_MASK;
            sTxBuffer[index++] = crc & MPU_HAL_8BIT_MASK;
            ret = MpuHalQueueSend(&g_mpuTxBuf[txMode], sTxBuffer, index, txMode);
            taskEXIT_CRITICAL();
        }
        else
        {
            ret = MPU_HAL_LEN_ERR;
        }
    }
    else
    {
        ret = MPU_HAL_NULL_PTR_ERR;
    }
    return ret;
}

/*************************************************
 Function: MpuHalReceive
 Description: Receive a data packet via MPU HAL
 Input: int16_t handle: handle index
        MpuHalDataPack_t* pRxMsg: pointer to the data packet to be received
        uint32_t waitTime: maximum wait time in ticks
 Output: void
 Return: int16_t: MPU_HAL_STATUS_OK if successful, MPU_HAL_STATUS_ERR if error
 Others:
*************************************************/
int16_t MpuHalReceive(int16_t handle, MpuHalDataPack_t *pRxMsg, uint32_t waitTime)
{
    uint16_t len, crc, crcRx, index, i;
    uint32_t data;
    uint8_t packHeader[MPU_HAL_PACK_HEADER_BUF_LEN];
    QueueHandle_t queHandle;
    int16_t ret = MPU_HAL_STATUS_OK;

    if ((handle >= 0) && (handle < MPU_HAL_HANDLE_INSTANSE_MAX) && pRxMsg != NULL)
    {
        index = handle & MPU_HAL_8BIT_MASK;
        queHandle = g_mpuManage.rxHandle[index].rxQueueHandle;
        if (xQueueReceive(queHandle, &data, waitTime) == pdPASS)
        {
            index = (data >> MPU_HAL_16BIT_SHIFT) & MPU_HAL_16BIT_MASK;
            len = data & MPU_HAL_16BIT_MASK;
            if ((pRxMsg->dataBufferSize + MPU_PROTOCAL_HEADER_LEN + 2) >= len)
            {
                for (i = 0; i < MPU_PROTOCAL_HEADER_LEN; i++) {
                    packHeader[i] = g_mpuManage.rxHandle[handle].pDataBufferRx[index++];
                    if (index >= g_mpuManage.rxHandle[handle].dataBufferSize) index = 0;
                }
                for (i = 0; i < (len - MPU_PROTOCAL_HEADER_LEN - 2); i++) {
                    pRxMsg->pDataBuffer[i] = g_mpuManage.rxHandle[handle].pDataBufferRx[index++];
                    if (index >= g_mpuManage.rxHandle[handle].dataBufferSize) index = 0;
                }
                pRxMsg->dataLength = len - MPU_PROTOCAL_HEADER_LEN - 2;
                crcRx = (g_mpuManage.rxHandle[handle].pDataBufferRx[index++] << MPU_HAL_8BIT_SHIFT);
                if (index >= g_mpuManage.rxHandle[handle].dataBufferSize) index = 0;
                crcRx |= g_mpuManage.rxHandle[handle].pDataBufferRx[index];
                
                crc = MpuPackGetCrc(packHeader, pRxMsg);
                if (crcRx == crc) {
                    pRxMsg->aid = packHeader[2];
                    pRxMsg->mid = packHeader[3];
                    pRxMsg->subcommand = packHeader[4];
                } else {
                    ret = MPU_HAL_CRC_ERR;
                }
            } else {
                ret = MPU_HAL_LEN_ERR;
            }
        } else {
            ret = MPU_HAL_STATUS_ERR;
        }
    } else {
        ret = MPU_HAL_STATUS_ERR;
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
 Function: MpuHalSetMode
 Description: Set the MPU HAL mode (wake or sleep)
 Input: uint8_t wakeMode: 0 for sleep mode, 1 for wake mode
 Output: void
 Return: void
 Others:
*************************************************/
void MpuHalSetMode(uint8_t wakeMode)
{
    if (MPU_HAL_WAKE_MODE_POWER_OFF == wakeMode)
    {
        TBOX_PRINT("===============IN Sleep===============\r\n");
        taskENTER_CRITICAL();
        /* 标记SPI/UART进入休眠, 防止发送任务在外设 DeInit 后继续访问寄存器 */
        g_spi_sleeping = 1u;
        g_uart_sleeping = 1u;
        if (g_mpuInitEvent != NULL) {
            xEventGroupClearBits(g_mpuInitEvent, MPU_EVENT_UART_INIT_DONE);
        }
        MpuUartHandleTypeDef *huart = &g_mpuUartRingBuf;
        huart->init_success = 0;
        spi_ready = spi_ready & 1;
        Dma_Hal_Deinit();
        MpuHalMainUartSleep();
        MpuHalMainSpiSleep();
        MpuHalGpioSetMode(0u);
        g_sleepGenerationCnt++;
        /* 休眠强行释放信号量, 防止mpu发送线程阻塞在100ms等待信号量 */
        if (g_uartTxCompleteSem != NULL) {
            xSemaphoreGive(g_uartTxCompleteSem);
        }
        taskEXIT_CRITICAL();
        MpuHalSetVbus(0);
        g_mpuManage.wakeMode = wakeMode;
    } else if (MPU_HAL_WAKE_MODE_MPU_SLEEP == wakeMode)
    {
        taskENTER_CRITICAL();
        MpuHal_UartDmaInit();
        MpuHalMainUartInit();
        MpuHal_SpiInit();
        MpuHalGpioSetMode(1u);
        /* SPI/UART重新初始化完成, 清除休眠标志允许发送 */
        g_spi_sleeping = 0u;
        g_uart_sleeping = 0u;
        taskEXIT_CRITICAL();
        MpuHalSetVbus(1);
        g_mpuManage.wakeoutTimeCount = MPU_HAL_WAKEOUT_TIME_SLEEPVALUE;
        MpuHalSetWakeOut(1);
        g_mpuManage.wakeMode = wakeMode;
        xEventGroupSetBits(g_mpuInitEvent, MPU_EVENT_UART_INIT_DONE);
    }
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
 Description: Process the received UART data and extract complete protocol packets
 Input: uint8_t *pData: pointer to the received data
        uint16_t dataLength: length of the received data
        uint8_t IsrFlag: 1 if called in ISR, 0 if called in task
 Output: void
 Return: void
 Others:
*************************************************/
static void UartProtocalProcess(void)
{
    MpuUartHandleTypeDef *huart = &g_mpuUartRingBuf;
    uint8_t recv_data = 0U, read_ok = 0U;

    if (huart->init_success == 1)
    {
        for (uint8_t i = 0; i < 10; i++)
        {
            read_ok = MpuHalReadRingBuf(&huart->rx_ring_buf, &recv_data);
            if (read_ok == 0) return;
            switch (huart->proto_state)
            {
                case PROTOCOL_STATE_IDLE:
                    if (recv_data == MPU_HAL_PACK_FIX_HEADER_0X55) {
                        huart->proto_buf.data[huart->proto_buf.dataCount++] = recv_data;
                        huart->proto_state = PROTOCOL_STATE_WAIT_HEADER2;
                    }
                    break;
                case PROTOCOL_STATE_WAIT_HEADER2:
                    if (recv_data == MPU_HAL_PACK_FIX_HEADER_0XAA) {
                        huart->proto_buf.data[huart->proto_buf.dataCount++] = recv_data;
                        huart->proto_state = PROTOCOL_STATE_WAIT_HEADER;
                    } else if (recv_data == MPU_HAL_PACK_FIX_HEADER_0XBB) {
                        huart->proto_buf.data[huart->proto_buf.dataCount++] = recv_data;
                        huart->proto_state = PROTOCOL_STATE_WAIT_SPI_HEADER;
                    } else {
                        huart->proto_buf.dataCount = 0;
                        huart->proto_state = PROTOCOL_STATE_IDLE;
                    }
                    break;
                case PROTOCOL_STATE_WAIT_HEADER:
                    huart->proto_buf.data[huart->proto_buf.dataCount++] = recv_data;
                    if (huart->proto_buf.dataCount == MPU_PROTOCAL_HEADER_LEN) {
                        huart->proto_buf.dataLength = ((uint16_t)huart->proto_buf.data[5] << 8) | huart->proto_buf.data[6];
                        if (huart->proto_buf.dataLength > MPU_HAL_RX_ONE_FRAME_MAX_SIZE) {
                            huart->proto_buf.dataCount = 0;
                            huart->proto_buf.dataLength = 0;
                            huart->proto_state = PROTOCOL_STATE_IDLE;
                            huart->err_cnt++;
                        } else {
                            huart->proto_state = PROTOCOL_STATE_WAIT_DATA;
                        }
                    }
                    break;
                case PROTOCOL_STATE_WAIT_SPI_HEADER:
                    huart->proto_buf.data[huart->proto_buf.dataCount++] = recv_data;
                    if (huart->proto_buf.dataCount == MPU_PROTOCAL_SPI_HEADER_LEN) {
                        huart->proto_buf.dataLength = ((uint16_t)huart->proto_buf.data[2] << 8) | huart->proto_buf.data[3];
                        huart->proto_buf.dataCount = 0;
                        huart->proto_buf.dataLength = 0;
                        huart->proto_state = PROTOCOL_STATE_IDLE;
                    }
                    break;
                case PROTOCOL_STATE_WAIT_DATA:
                    huart->proto_buf.data[huart->proto_buf.dataCount++] = recv_data;
                    if (huart->proto_buf.dataCount >= (MPU_PROTOCAL_HEADER_LEN + huart->proto_buf.dataLength + 2)) {
                        MpuDataDispatch((uint8_t *)huart->proto_buf.data, huart->proto_buf.dataCount, MPU_HAL_SEND_IRQ_ENABLE);
                        huart->proto_buf.dataCount = 0;
                        huart->proto_buf.dataLength = 0;
                        huart->proto_state = PROTOCOL_STATE_IDLE;
                    }
                    break;
                default:
                    huart->proto_state = PROTOCOL_STATE_IDLE;
                    huart->proto_buf.dataCount = 0;
                    huart->err_cnt++;
                    break;
            }
        }
    }
}

static void SpiProtocalProcess(void)
{
    MpuUartHandleTypeDef *huart = &g_mpuSpiRingBuf;
    uint8_t recv_data = 0U;
    uint8_t read_ok = 0U;

    if (huart->init_success != 1)
    {
        //TBOX_PRINT(" Mpu hal uart init error,does not support protocal process\r\n");   //1ms print maybe reultl over loading
    }
    else
    {
        for (uint8_t i = 0; i < 5; i++)
        {
            read_ok = MpuHalReadRingBuf(&huart->rx_ring_buf, &recv_data);
            if (read_ok == 0) {
                return;
            }
            switch (huart->proto_state)
            {
                case PROTOCOL_STATE_IDLE:
                    // Wait for 1st header byte (0x55)
                    if (recv_data == MPU_HAL_PACK_FIX_HEADER_0X55)
                    {
                        huart->proto_buf.data[huart->proto_buf.dataCount] = recv_data;
                        huart->proto_buf.dataCount++;
                        huart->proto_state = PROTOCOL_STATE_WAIT_HEADER2;
                    }
                    break;

                case PROTOCOL_STATE_WAIT_HEADER2:
                    // Wait for 2nd header byte (0xAA)
                    if (recv_data == MPU_HAL_PACK_FIX_HEADER_0XAA)
                    {
                        huart->proto_buf.data[huart->proto_buf.dataCount] = recv_data;
                        huart->proto_buf.dataCount++;
                        huart->proto_state = PROTOCOL_STATE_WAIT_HEADER;
                    }
                    else if (recv_data == MPU_HAL_PACK_FIX_HEADER_0XBB)
                    {
                        huart->proto_buf.data[huart->proto_buf.dataCount] = recv_data;
                        huart->proto_buf.dataCount++;
                        huart->proto_state = PROTOCOL_STATE_WAIT_SPI_HEADER;
                    }
                    else
                    {
                        huart->proto_buf.dataCount = 0;
                        huart->proto_state = PROTOCOL_STATE_IDLE;
                        TBOX_PRINT("Proto 1: invalid header2 (0x%02x)\n", recv_data);
                        // huart->err_cnt++;
                    }
                    break;

                case PROTOCOL_STATE_WAIT_HEADER:
                    // Collect remaining header fields until header length is reached
                    huart->proto_buf.data[huart->proto_buf.dataCount] = recv_data;
                    huart->proto_buf.dataCount++;

                    if (huart->proto_buf.dataCount == MPU_PROTOCAL_HEADER_LEN)
                    {
                        huart->proto_buf.dataLength = ((uint16_t)huart->proto_buf.data[5] << 8) | huart->proto_buf.data[6];
                        // Check if payload length exceeds max buffer size
                        if (huart->proto_buf.dataLength > MPU_HAL_RX_ONE_FRAME_MAX_SIZE)
                        {
                            TBOX_PRINT("Proto 1: data len over max (%d > %d)\n",
                                    huart->proto_buf.dataLength, MPU_HAL_RX_ONE_FRAME_MAX_SIZE);
                            huart->proto_buf.dataCount = 0;
                            huart->proto_buf.dataLength = 0;
                            huart->proto_state = PROTOCOL_STATE_IDLE;
                            huart->err_cnt++;
                        }
                        else
                        {
                            huart->proto_state = PROTOCOL_STATE_WAIT_DATA;
                        }
                    }
                    break;

                case PROTOCOL_STATE_WAIT_SPI_HEADER:
                    huart->proto_buf.data[huart->proto_buf.dataCount] = recv_data;
                    huart->proto_buf.dataCount++;

                    if (huart->proto_buf.dataCount == MPU_PROTOCAL_SPI_HEADER_LEN)
                    {
                        huart->proto_buf.dataLength = ((uint16_t)huart->proto_buf.data[2] << 8) | huart->proto_buf.data[3];
                        if (huart->proto_buf.dataLength > MPU_HAL_RX_ONE_FRAME_MAX_SIZE)
                        {
                            TBOX_PRINT("Proto 1: data len over max (%d > %d)\n",
                                    huart->proto_buf.dataLength, MPU_HAL_RX_ONE_FRAME_MAX_SIZE);
                            huart->err_cnt++;
                        }
                        /* SPI头帧对于SPI SLAVE，只做故障后处理，不对接业务，此处重置数据 */
                        huart->proto_buf.dataCount = 0;
                        huart->proto_buf.dataLength = 0;
                        huart->proto_state = PROTOCOL_STATE_IDLE;
                    }
                    break;

                case PROTOCOL_STATE_WAIT_DATA:
                    // Collect payload and checksum (last 2 bytes)
                    huart->proto_buf.data[huart->proto_buf.dataCount] = recv_data;
                    huart->proto_buf.dataCount++;

                    // Check if full packet is received (header + payload + checksum)
                    if (huart->proto_buf.dataCount >= (MPU_PROTOCAL_HEADER_LEN + huart->proto_buf.dataLength + 2))
                    {
                        MpuDataDispatch((uint8_t *)huart->proto_buf.data, huart->proto_buf.dataCount, MPU_HAL_SEND_IRQ_ENABLE);
                        huart->proto_buf.dataCount = 0;
                        huart->proto_buf.dataLength = 0;
                        huart->proto_state = PROTOCOL_STATE_IDLE;
                    }
                    break;

                default:
                    huart->proto_state = PROTOCOL_STATE_IDLE;
                    huart->proto_buf.dataCount = 0;
                    huart->err_cnt++;
                    break;
            }
        }
    }
}

/*************************************************
  Function:       MpuHalSetRingBuf
  Description:    Set data to UART0 ring buffer
  Input:          recv_data: Data byte to be stored in the buffer
  Output:         None
  Return:         None
  Others:         None
*************************************************/
static void MpuHalSetRingBuf(uint8_t *recv_data, uint16_t length, MpuHalTXMode_e txMode)
{
    MpuUartHandleTypeDef* huart = NULL;
    uint16_t next_head = 0u;
    if (MPU_HAL_UART_MODE == txMode)
    {
        huart = &g_mpuUartRingBuf;
    }
    else if (MPU_HAL_SPI_MODE == txMode)
    {
        huart = &g_mpuSpiRingBuf;
    }
    else
    {
        TBOX_PRINT("MpuHalSetRingBuf: invalid txMode %d\n", txMode);
        return;
    }

    __disable_irq();
    for (uint16_t i = 0u; i < length; i++)
    {
        next_head = (huart->rx_ring_buf.head + 1) % MPU_HAL_RING_BUFFER;
        if (next_head != huart->rx_ring_buf.tail) 
        {
            huart->rx_ring_buf.buf[huart->rx_ring_buf.head] = recv_data[i];
            huart->rx_ring_buf.head = next_head; // Update write index
        }
        else 
        {
            huart->err_cnt++;
            if (MPU_HAL_UART_MODE == txMode)
            {
                TBOX_PRINT("Uart ring buf full! 1 cnt: %d\n", huart->err_cnt);
            }
            else if (MPU_HAL_SPI_MODE == txMode)
            {
                TBOX_PRINT("Spi ring buf full! 1 cnt: %d\n", huart->err_cnt);
            } else ;
            break;
        }
    }
    __enable_irq();
}

/*************************************************
  Function: MpuHalReadRingBuf
  Description: Read data from ring buffer
  Input: RingBufferType* rb: pointer to the ring buffer structure
         uint8_t* data: pointer to store the read data
  Output: data: contains the read data if successful
  Return: 1 if read successfully, 0 if buffer is empty or parameters invalid
*************************************************/
static uint8_t MpuHalReadRingBuf(RingBufferType* rb, uint8_t* data)
{
    uint8_t read_ok = 0;
    if (rb != NULL && data != NULL) {
        __disable_irq();
        uint16_t current_head = rb->head;
        uint16_t current_tail = rb->tail;
        
        if (current_head != current_tail)
        {
            if (current_tail < MPU_HAL_RING_BUFFER)
            {
                *data = rb->buf[current_tail];
                current_tail = (current_tail + 1) % MPU_HAL_RING_BUFFER;
                rb->tail = current_tail;
                read_ok = 1;
            }
        }
        __enable_irq();
    }
    return read_ok;
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
    MpuUartHandleTypeDef *huart = &g_mpuUartRingBuf;
    huart->init_success = 0;
    Uart_Hal_DeInit(MPU_HAL_UART_ID); // 内部已禁用中断
    Gpio_Hal_SetMuxMode(MPU_HAL_UART_RX_PORT, MPU_HAL_UART_RX_PIN, MPU_HAL_UART_RX_GPIO);
    Gpio_Hal_SetPinDirection(MPU_HAL_UART_RX_PORT, MPU_HAL_UART_RX_PIN, GPIO_INPUT_DIRECTION);
    Gpio_Hal_SetMuxMode(MPU_HAL_UART_TX_PORT, MPU_HAL_UART_TX_PIN, MPU_HAL_UART_TX_GPIO);
    Gpio_Hal_SetPinDirection(MPU_HAL_UART_TX_PORT, MPU_HAL_UART_TX_PIN, GPIO_INPUT_DIRECTION);
    Gpio_Hal_SetPullSel(MPU_HAL_UART_TX_PORT, MPU_HAL_UART_TX_PIN, PORT_INTERNAL_PULL_DOWN_ENABLED);
}

/*************************************************
 Function: MpuHalUartTxTask
 Description: Task to handle UART transmission for MPU HAL
 Input: void *pvParameters: task parameters (not used)
 Output: void
 Return: void
 Others:
*************************************************/
void MpuHalUartTxTask(void *pvParameters)
{
    uint32_t queueData = 0u;
    while (1) {
        xEventGroupWaitBits(g_mpuInitEvent, MPU_EVENT_UART_INIT_DONE, pdFALSE, pdTRUE, portMAX_DELAY);
        if (g_mpuUartRingBuf.init_success == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
            continue;
        }
        
        if (xQueueReceive(g_mpuTxBuf[MPU_HAL_UART_MODE].txQueueHandle, &queueData, portMAX_DELAY) == pdPASS) {
            xSemaphoreTake(g_uartTxCompleteSem, 0);
            uint32_t startSleepCnt = g_sleepGenerationCnt;
            MpuHalPacketSend(MPU_HAL_UART_MODE, queueData, &g_mpuTxBuf[MPU_HAL_UART_MODE]);
            BaseType_t waitRet = xSemaphoreTake(g_uartTxCompleteSem, pdMS_TO_TICKS(1000));
            /* 休眠过一次 */
            if (startSleepCnt != g_sleepGenerationCnt)
            {
                TBOX_PRINT("UART TX Aborted by Sleep!\r\n");
                Uart_Hal_AbortSendingData(MPU_HAL_UART_ID);
                continue; 
            }
            /* DMA TX 超时未完成 */
            if (waitRet != pdPASS)
            {
                TBOX_PRINT("UART TX DMA TIMEOUT\n");
                Uart_Hal_AbortSendingData(MPU_HAL_UART_ID);
                continue; 
            }
            /* UART 初始化失效 */
            if ((xEventGroupGetBits(g_mpuInitEvent) & MPU_EVENT_UART_INIT_DONE) == 0)
            {
                TBOX_PRINT("UART Init Error!\r\n");
                Uart_Hal_AbortSendingData(MPU_HAL_UART_ID);
                continue;
            }
        }
    }
}

/*************************************************
 Function: MpuHalPacketSend
 Description: Send a packet of data via UART or SPI DMA
 Input: MpuHalTXMode_e txMode: transmission mode (UART or SPI)
        uint32_t queueData: data to be sent (start address and length)
        MpuQueueTxBuffer_t *pTxBuffer: pointer to the transmission buffer
 Output: void
 Return: uint32_t: status code (OK or error code)
 Others:
*************************************************/
static uint32_t MpuHalPacketSend(MpuHalTXMode_e txMode, uint32_t queueData, MpuQueueTxBuffer_t *pTxBuffer)
{
    static uint8_t pTxData[MPU_HAL_MAX_BUFFSIZE + MPU_HAL_SPI_HEADER_SIZE] = {0};
    uint16_t startAddress = (queueData >> MPU_HAL_16BIT_SHIFT) & MPU_HAL_16BIT_MASK;
    uint16_t length = queueData & MPU_HAL_16BIT_MASK;
    uint16_t crc = MPU_HAL_CRC_DEFAULT_VALUE;

    if (txMode == MPU_HAL_UART_MODE) {
        __disable_irq();
        for (uint16_t i = 0u; i < length; i++) {
            pTxData[i] = pTxBuffer->buffer[startAddress++];
            if (startAddress >= MPU_HAL_MAX_BUFFSIZE) startAddress = 0;
        }
        __enable_irq();
        /* 发送启动阶段用临界区保护, 防止休眠任务在发送中途 DeInit UART 导致访问已关时钟的寄存器 */
        taskENTER_CRITICAL();
        if ((g_uart_sleeping == 0u) && (g_mpuUartRingBuf.init_success != 0u)) {
            Uart_Hal_SendData(MPU_HAL_UART_ID, pTxData, length);
        }
        taskEXIT_CRITICAL();
    }
#if (SPI_DMA_ENABLE == 1)
    else
    {
        __disable_irq();
        for (uint16_t i = 0u; i < length; i++) {
            pTxData[i + MPU_HAL_SPI_HEADER_SIZE] = pTxBuffer->buffer[startAddress++];
            if (startAddress >= MPU_HAL_MAX_BUFFSIZE) startAddress = 0;
        }
        __enable_irq();
        /* Build SPI 6-byte header: 0x55, 0xBB, lenH, lenL, crcH, crcL */
        pTxData[0] = MPU_HAL_SPI_HEADER_FIRST_BYTE;
        pTxData[1] = MPU_HAL_SPI_HEADER_SECOND_BYTE;
        pTxData[2] = (uint8_t)(length >> 8);
        pTxData[3] = (uint8_t)(length & 0xFF);
        crc = CcittCrc16(crc, &pTxData[MPU_HAL_SPI_HEADER_SIZE], length);
        pTxData[4] = (uint8_t)(crc >> 8);
        pTxData[5] = (uint8_t)(crc & 0xFF);
        Spi_Hal_TransceiveDma(MPU_HAL_SPI_ID, pTxData, NULL_PTR, (length + MPU_HAL_SPI_HEADER_SIZE));
        Gpio_Hal_WritePin(MPU_HAL_SPI_IEQ_PORT, MPU_HAL_SPI_IEQ_PORT_PIN, GPIO_HIGH_LEVEL);
    }
#endif
    return MPU_HAL_STATUS_OK;
}

#if (SPI_DMA_ENABLE == 1)
/*************************************************
  Function:       MpuHal_SpiIrqCallback
  Description:    Handle SPI REQ pin interrupt (dual-edge)
  Input:          None
  Output:         None
  Return:         None
  Others:         On first falling edge, set spi_ready bit0 (MPU alive detection).
                  Subsequent edges are handled by SPI HAL internally.
*************************************************/
void MpuHal_SpiIrqCallback(void)
{
    static uint8_t first_req_done = 0u;

    if ((first_req_done == 0u) && (Gpio_Hal_ReadPin(MPU_HAL_SPI_REQ_PORT, MPU_HAL_SPI_REQ_PORT_PIN) == GPIO_LOW_LEVEL)) {
        spi_ready = spi_ready | 1;
        first_req_done = 1u;
        TBOX_PRINT("MPU Spi INIT OK\n");
    }
}

/*************************************************
  Function:       SpiHalEventCallback
  Description:    SPI HAL event callback
  Input:          Instance - SPI instance
                  Status   - SPI status mask
  Output:         None
  Return:         None
  Others:         TX_FINISH → pull IEQ low to signal MPU transfer complete.
                  TX_UNDERFLOW → also pull IEQ low.
*************************************************/
static void SpiHalEventCallback(uint8 Instance, uint16 Status)
{
    (void)Instance;
    if (Status == SPI_STATUS_TX_FINISH_MASK)
    {
        Gpio_Hal_WritePin(MPU_HAL_SPI_IEQ_PORT, MPU_HAL_SPI_IEQ_PORT_PIN, GPIO_LOW_LEVEL);
    }
    else if (Status == SPI_STATUS_RX_FINISH_MASK)
    {
        /* do nothing */
    }
    else if (Status == SPI_STATUS_TX_UNDERFLOW_MASK)
    {
        Gpio_Hal_WritePin(MPU_HAL_SPI_IEQ_PORT, MPU_HAL_SPI_IEQ_PORT_PIN, GPIO_LOW_LEVEL);
    }
}

/*************************************************
  Function:       MpuHal_SpiInit
  Description:    Initialize SPI as slave
  Input:          None
  Output:         None
  Return:         None
  Others:         Uses Spi_Hal_TransceiveDma for TX/RX, SPI callback for IEQ control.
                  DMA channels are pre-initialized in MpuHal_UartDmaInit.
*************************************************/
static void MpuHal_SpiInit(void)
{
    MpuUartHandleTypeDef *huart = &g_mpuSpiRingBuf;
    huart->init_success = 0;

    /* Config SPI pin mux */
    Gpio_Hal_SetMuxMode(MPU_HAL_SPI_CLK_PORT, MPU_HAL_SPI_CLK_PIN, MPU_HAL_SPI_CLK_MUX);
    Gpio_Hal_SetMuxMode(MPU_HAL_SPI_MOSI_PORT, MPU_HAL_SPI_MOSI_PIN, MPU_HAL_SPI_MOSI_MUX);
    Gpio_Hal_SetMuxMode(MPU_HAL_SPI_MISO_PORT, MPU_HAL_MISO_PIN, MPU_HAL_SPI_MISO_MUX);
    Gpio_Hal_SetMuxMode(MPU_HAL_SPI_CS_PORT, MPU_HAL_SPI_CS_PIN, MPU_HAL_SPI_CS_MUX);

    /* IEQ and REQ int config */
    Gpio_Hal_SetMuxMode(MPU_HAL_SPI_IEQ_PORT, MPU_HAL_SPI_IEQ_PORT_PIN, MPU_HAL_SPI_IEQ_PORT_MUX);
    Gpio_Hal_SetMuxMode(MPU_HAL_SPI_REQ_PORT, MPU_HAL_SPI_REQ_PORT_PIN, MPU_HAL_SPI_REQ_PORT_MUX);

    Gpio_Hal_SetPullSel(MPU_HAL_SPI_REQ_PORT, MPU_HAL_SPI_REQ_PORT_PIN, PORT_INTERNAL_PULL_DOWN_ENABLED);
    Gpio_Hal_SetPinDirection(MPU_HAL_SPI_REQ_PORT, MPU_HAL_SPI_REQ_PORT_PIN, GPIO_INPUT_DIRECTION);
    Gpio_Hal_SetPinIntSel(MPU_HAL_SPI_REQ_PORT, MPU_HAL_SPI_REQ_PORT_PIN, PORT_INT_EITHER_EDGE);
    Core_Hal_EnableIrq(MPU_HAL_SPI_REQ_IRQN);

    Gpio_Hal_SetPinDirection(MPU_HAL_SPI_IEQ_PORT, MPU_HAL_SPI_IEQ_PORT_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(MPU_HAL_SPI_IEQ_PORT, MPU_HAL_SPI_IEQ_PORT_PIN, GPIO_LOW_LEVEL);

    Spi_HalConfigType spiCfg;
    memset(&spiCfg, 0, sizeof(spiCfg));

    spiCfg.CsSetup      = 5U;
    spiCfg.CsHold       = 5U;
    spiCfg.CsIdle       = 5U;
    spiCfg.FrmSize      = 8U;
    spiCfg.TxDmaChannel = SPI_DMA_TX_CHANNEL;
    spiCfg.RxDmaChannel = SPI_DMA_RX_CHANNEL;
    spiCfg.MsbFirst     = TRUE;
    spiCfg.CsOutputEn   = TRUE;
    spiCfg.ContinuousCs = TRUE;
    spiCfg.HreqEn       = FALSE;
    spiCfg.Mode         = SPI_SLAVE;
    spiCfg.Cpol         = SPI_CPOL_LOW;
    spiCfg.Cpha         = SPI_CPHA_1;
    spiCfg.PcsCfg       = SPI_PCS_0;
    spiCfg.PcsPol       = SPI_PCS_POLARITY_LOW;
    spiCfg.HreqPol      = SPI_HREQ_POLARITY_HIGH;
    spiCfg.Width        = SPI_DATA_WIDTH_1BIT;
    spiCfg.PinCfg       = SPI_SOUT_MISO_SIN_MOSI;
    spiCfg.Callback     = SpiHalEventCallback;
    spiCfg.BaudRate     = 2000000UL;

    Spi_Hal_Init(MPU_HAL_SPI_ID, &spiCfg);

    /* Enable SPI interrupts based on HAL patterns */
    Core_Hal_EnableIrq(SPI0_IRQn);
    Core_Hal_SetIrqPriority(SPI0_IRQn, 2);

    spi_ready = spi_ready | 2;
    huart->init_success = 1;
}

/*************************************************
  Function:       MpuHalMainSpiSleep
  Description:    Put SPI interface into sleep mode
  Input:          None
  Output:         None
  Return:         None
*************************************************/
static void MpuHalMainSpiSleep(void)
{
    MpuUartHandleTypeDef *huart = &g_mpuSpiRingBuf;
    huart->init_success = 0;
    spi_ready = spi_ready & 1;

    Spi_Hal_DeInit(MPU_HAL_SPI_ID);

    /* ===== 新增：处理 SPI REQ(PE8) 中断，防止进入 VLPS 时被误唤醒 ===== */
    /* 1. 先禁用 PORTE 中断并清除挂起的 NVIC 中断，避免残留触发 */
    Core_Hal_DisableIrq(MPU_HAL_SPI_REQ_IRQN);
    Core_Hal_ClearPendingIrq(MPU_HAL_SPI_REQ_IRQN);
    /* 2. 将 PE8 中断配置改为禁止中断/ DMA，从源头关闭边沿检测 */
    Gpio_Hal_SetPinIntSel(MPU_HAL_SPI_REQ_PORT, MPU_HAL_SPI_REQ_PORT_PIN, PORT_DMA_INT_DISABLED);
    /* 3. 清除 PE8 的 ISF 标志位（写 1 清零），避免唤醒后立即进中断 */
    Gpio_Hal_ClearIntStatus(MPU_HAL_SPI_REQ_PORT, MPU_HAL_SPI_REQ_PORT_PIN);
    /* 4. PE8 复用为普通 GPIO 输入 + 内部下拉，保持稳定电平 */
    Gpio_Hal_SetMuxMode(MPU_HAL_SPI_REQ_PORT, MPU_HAL_SPI_REQ_PORT_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(MPU_HAL_SPI_REQ_PORT, MPU_HAL_SPI_REQ_PORT_PIN, GPIO_INPUT_DIRECTION);
    Gpio_Hal_SetPullSel(MPU_HAL_SPI_REQ_PORT, MPU_HAL_SPI_REQ_PORT_PIN, PORT_INTERNAL_PULL_DOWN_ENABLED);
    /* ================================================================== */

    Gpio_Hal_SetMuxMode(MPU_HAL_SPI_CLK_PORT, MPU_HAL_SPI_CLK_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(MPU_HAL_SPI_CLK_PORT, MPU_HAL_SPI_CLK_PIN, GPIO_INPUT_DIRECTION);
    Gpio_Hal_SetMuxMode(MPU_HAL_SPI_MISO_PORT, MPU_HAL_MISO_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(MPU_HAL_SPI_MISO_PORT, MPU_HAL_MISO_PIN, GPIO_INPUT_DIRECTION);
    Gpio_Hal_SetMuxMode(MPU_HAL_SPI_MOSI_PORT, MPU_HAL_SPI_MOSI_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(MPU_HAL_SPI_MOSI_PORT, MPU_HAL_SPI_MOSI_PIN, GPIO_INPUT_DIRECTION);
    Gpio_Hal_SetMuxMode(MPU_HAL_SPI_CS_PORT, MPU_HAL_SPI_CS_PIN, PORT_MUX_AS_GPIO);
    Gpio_Hal_SetPinDirection(MPU_HAL_SPI_CS_PORT, MPU_HAL_SPI_CS_PIN, GPIO_INPUT_DIRECTION);
    /* IEQ output low */
    Gpio_Hal_WritePin(MPU_HAL_SPI_IEQ_PORT, MPU_HAL_SPI_IEQ_PORT_PIN, GPIO_LOW_LEVEL);
}

/*************************************************
  Function:       MpuHalSpiTxTask
  Description:    SPI TX task - waits for data in queue and sends via Spi_Hal_TransceiveDma
  Input:          pvParameters - task parameters (unused)
  Output:         None
  Return:         None
  Others:         Checks spi_ready and IEQ pin level.
                  IEQ high means MPU is transmitting, wait.
*************************************************/
void MpuHalSpiTxTask(void *pvParameters)
{
    uint32_t queueData = 0u;
    uint8_t ret = 0u;
    (void)pvParameters;

    while (1) {
        if (spi_ready != 3) {
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }
        if (Gpio_Hal_ReadPin(MPU_HAL_SPI_IEQ_PORT, MPU_HAL_SPI_IEQ_PORT_PIN) == GPIO_HIGH_LEVEL) {
            /* IEQ high means MPU is transmitting data, wait */
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        if (xQueueReceive(g_mpuTxBuf[MPU_HAL_SPI_MODE].txQueueHandle, &queueData, portMAX_DELAY) != pdPASS) {
            continue;
        }
        /* 检查SPI是否已进入休眠, 若是则丢弃数据避免访问已DeInit的SPI外设 */
        if (g_spi_sleeping != 0u) {
            continue;
        }
        ret = MpuHalPacketSend(MPU_HAL_SPI_MODE, queueData, &g_mpuTxBuf[MPU_HAL_SPI_MODE]);
        if (ret != MPU_HAL_STATUS_OK) {
            TBOX_PRINT("SPI PacketSend failed, ret = 0x%X\n", ret);
        }
    }
}
#endif /* SPI_DMA_ENABLE */

/*************************************************
  Function:       MpuHalUartTimerCallBack
  Description:    Timer callback for UART protocol processing
  Input:          None
  Output:         None
  Return:         None
*************************************************/
void MpuHalUartTimerCallBack(void)
{
    UartProtocalProcess();
}

/*************************************************
  Function:       MpuHalSpiTimerCallBack
  Description:    Timer callback for SPI protocol processing
  Input:          None
  Output:         None
  Return:         None
*************************************************/
void MpuHalSpiTimerCallBack(void)
{
    SpiProtocalProcess();
}

/*************************************************
  Function:       MpuHalSetSpiReady
  Description:    Set SPI ready status from external (power management)
  Input:          ready - 1 to set ready bit0, 0 to clear
  Output:         None
  Return:         None
*************************************************/
void MpuHalSetSpiReady(uint8_t ready)
{
    static uint32_t power_init_flag = 0;
    if (ready) {
        spi_ready = spi_ready | 1;
        if (power_init_flag == 0) {
            power_init_flag = 1;
        }
    } else {
        spi_ready = spi_ready & 0x2;
    }

    Gpio_Hal_WritePin(MPU_HAL_SPI_IEQ_PORT, MPU_HAL_SPI_IEQ_PORT_PIN, GPIO_LOW_LEVEL);
}