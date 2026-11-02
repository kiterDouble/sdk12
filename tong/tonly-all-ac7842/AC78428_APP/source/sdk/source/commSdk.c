#include "commSdk.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "mpuHal.h"
#include "logSdk.h"

#include <string.h>

#define MPU_HAL_HANDLE_INSTANSE_MAX         15U

#define MPU_PROTOCAL_HEADER_LEN             8U
#define MPU_PROTOCAL_SPI_HEADER_LEN         6U

#define MPU_HAL_RX_QUEUE_LEN                10U
#define MPU_HAL_TX_QUEUE_LEN                40U

#define MPU_HAL_8BIT_SHIFT                  8U
#define MPU_HAL_16BIT_SHIFT                 16U

#define MPU_HAL_8BIT_MASK                   0xFF
#define MPU_HAL_16BIT_MASK                  0xFFFF

#define MPU_HAL_RX_ONE_FRAME_MAX_SIZE       (4096U)
#define MPU_HAL_RING_BUFFER                 (4096 * 2)

#define MPU_HAL_MAX_BUFFSIZE                (4096 * 2)

#define MPU_HAL_PROC_BUFFER_LEN             (4096U)
#define MPU_HAL_UART_BUFFSIZE               (4096U)
#define MPU_HAL_TXMSG_MAX_LEN               (4000U)




#define MPU_HAL_PACK_HEADER_BUF_LEN             20
#define MPU_HAL_PACK_FIX_HEADER_0X55            0x55
#define MPU_HAL_PACK_FIX_HEADER_0XAA            0xAA
#define MPU_HAL_PACK_FIX_HEADER_0XBB            0xBB // SPI 6字节头帧
#define MPU_HAL_PACK_HEADER_RESERVED            0x00

#define MPU_HAL_SPI_HEADER_FIRST_BYTE       (0x55u)
#define MPU_HAL_SPI_HEADER_SECOND_BYTE      (0xBBu)
#define MPU_HAL_SPI_HEADER_SIZE             (6u)  

#define MPU_HAL_CRC_DEFAULT_VALUE               0x0000


#define MPU_HAL_CYC_TIME_CNT_50MS               500 // base cycleTime = 10
#define MPU_HAL_CYC_TIME_CNT_40MS               400 // base cycleTime = 10
#define MPU_HAL_CYC_TIME_CNT_20MS               200 // base cycleTime = 10  

#define MPU_HAL_WAKEOUT_TIME_INIT               10
#define MPU_HAL_WAKEOUT_TIME_WAKEVALUE          2
#define MPU_HAL_WAKEOUT_TIME_SLEEPVALUE         0


#define MPU_HAL_SEND_IRQ_ENABLE                 (1U)



#define MPU_HAL_WAKE_MODE_MPU_SLEEP             1


typedef enum {
    PROTOCOL_STATE_IDLE,                // Idle: wait for 1st header byte (0x55)
    PROTOCOL_STATE_WAIT_HEADER2,        // Wait for 2nd header byte (0xAA) or (0xBB)
    PROTOCOL_STATE_WAIT_HEADER,         // Wait for remaining header fields
    PROTOCOL_STATE_WAIT_SPI_HEADER,     // Wait for remaining header fields (SPI)
    PROTOCOL_STATE_WAIT_DATA            // Wait for payload and checksum
} ProtocolStateType;





/****************************** Type Definitions ******************************/

typedef struct {
    uint8_t data[MPU_HAL_RX_ONE_FRAME_MAX_SIZE];
    uint16_t dataCount;
    uint16_t dataLength;
} MpuUartProtocalBuffer_t;

typedef struct {
    uint8_t buf[MPU_HAL_RING_BUFFER];     // Buffer storage
    volatile uint16_t head;             // Write index (updated by interrupt)
    volatile uint16_t tail;             // Read index (updated by main loop)
} RingBufferType;

typedef struct {
    RingBufferType rx_ring_buf;         // RX ring buffer instance
    ProtocolStateType proto_state;      // Current state of protocol parser
    MpuUartProtocalBuffer_t proto_buf;  // Buffer for storing protocol data
    //uint8_t init_success;               // Init status (1 = success, 0 = failure)
    uint16_t err_cnt;                   // Error counter (buffer full, invalid protocol)
} CommHandler_t;


typedef struct {
    QueueHandle_t txQueueHandle;
    uint8_t buffer[MPU_HAL_MAX_BUFFSIZE];
    uint16_t index;
} MpuQueueTxBuffer_t;

typedef struct {
    uint8_t buffer[MPU_HAL_PROC_BUFFER_LEN];
    uint16_t txLength;
} MpuRxBuffer_t;

typedef struct {
    uint8_t buffer[MPU_HAL_PROC_BUFFER_LEN];
    uint16_t dataCount;
} MpuProtocolBuffer_t;

typedef struct {
    uint8_t useFlag;
    uint8_t useRxFilter;
    uint8_t *pDataBufferRx;
    uint16_t dataBufferSize;
    uint16_t rxIndexIn;
    QueueHandle_t rxQueueHandle;
    MpuHalFilter_t rxFilterConfig;
} MpuHalHandle_t;

typedef struct {
    uint8_t wakeMode; /* 0:MPU power off,1:Mpu sleep,2:Mpu wake */
    uint8_t wakeoutTimeCount;
    MpuHalStartState_e startState;
    MpuHalHandle_t rxHandle[MPU_HAL_HANDLE_INSTANSE_MAX];
    uint8_t mcuRequest; /* 0:no request,1:requested */
} MpuHalManage_t;



static uint32_t PacketSend(MpuHalTXMode_e txMode, uint32_t queueData, MpuQueueTxBuffer_t *pTxBuffer);

static uint8_t MpuHalReadRingBuf(RingBufferType *rb, uint8_t *data);
static void MpuHalSetRingBuf(uint8_t *recv_data, uint16_t length, MpuHalTXMode_e txMode);
static void MpuDataDispatch(const uint8_t *pData, uint16_t length, uint8_t irq);

static MpuQueueTxBuffer_t g_mpuTxBuf[MPU_HAL_TX_MODE_MAX];
static MpuHalManage_t g_mpuManage;

static CommHandler_t g_mpuUartRingBuf = {
    .rx_ring_buf = {.head = 0, .tail = 0},
    .proto_state = PROTOCOL_STATE_IDLE,
    //.init_success = 0,
    .err_cnt = 0
};

static CommHandler_t g_mpuSpiRingBuf = {
    .rx_ring_buf = {.head = 0, .tail = 0},
    .proto_state = PROTOCOL_STATE_IDLE,
    //.init_success = 0,
    .err_cnt = 0
};

void CommSdk_Init(void)
{
    memset(&g_mpuManage, 0, sizeof(g_mpuManage));

    g_mpuManage.wakeoutTimeCount = MPU_HAL_WAKEOUT_TIME_INIT;
    g_mpuManage.wakeMode = MPU_HAL_WAKE_MODE_MPU_SLEEP;

    MpuHalInit();
}

void CommSdk_DeInit(void)
{
    /* TODO: DeInit not available on target platform, MpuHal does not support de-init */
}

void CommSdk_Start(void)
{
    g_mpuManage.wakeoutTimeCount = MPU_HAL_WAKEOUT_TIME_INIT;
    MpuHalReset();
    MpuHalSetPower(1);
    g_mpuManage.startState = E_MPU_HAL_START_STATE_POWER_ON;
}

void CommSdk_Stop(void)
{
    MpuHalPowerOff();
    g_mpuManage.startState = E_MPU_HAL_START_STATE_IDLE;
}

/*************************************************
 Function: CommSdk_SetRxFilter
 Description: Set the RX filter for a given handle
 Input: int16_t handle: handle index
        const MpuHalFilter_t* pFilter: pointer to the filter configuration, NULL to disable filter
 Output: void
 Return: int16_t: COMM_SDK_OK if successful, COMM_SDK_ERR_STATUS if error
 Others:
*************************************************/
int16_t CommSdk_SetRxFilter(int16_t handle, const MpuHalFilter_t *pFilter)
{
    if ((handle >= 0) && (handle < MPU_HAL_HANDLE_INSTANSE_MAX)) {
        if (pFilter != NULL) {
            g_mpuManage.rxHandle[handle].useRxFilter = 1;
            memcpy(&(g_mpuManage.rxHandle[handle].rxFilterConfig), pFilter, sizeof(MpuHalFilter_t));
        } else {
            g_mpuManage.rxHandle[handle].useRxFilter = 0;
        }
        return COMM_SDK_OK;
    }

    return COMM_SDK_ERR_STATUS;
}

/*************************************************
 Function: CommSdk_SetRxBuffer
 Description: Set the RX buffer for a given handle
 Input: int16_t handle: handle index
        uint8_t* pBuffer: pointer to the RX buffer
        uint32_t bufferSize: size of the RX buffer
 Output: void
 Return: int16_t: COMM_SDK_OK if successful, COMM_SDK_ERR_STATUS if error
 Others:
*************************************************/
int16_t CommSdk_SetRxBuffer(int16_t handle, uint8_t *pBuffer, uint32_t bufferSize)
{
    if ((handle >= 0) && (handle < MPU_HAL_HANDLE_INSTANSE_MAX)) {
        if (pBuffer != NULL) {
            g_mpuManage.rxHandle[handle].rxQueueHandle = xQueueCreate(MPU_HAL_RX_QUEUE_LEN, sizeof(uint32_t));
            g_mpuManage.rxHandle[handle].rxIndexIn = 0;
            g_mpuManage.rxHandle[handle].pDataBufferRx = pBuffer;
            g_mpuManage.rxHandle[handle].dataBufferSize = bufferSize;
            return COMM_SDK_OK;
        } else {
            return COMM_SDK_ERR_NULL_PTR;
        }
    }

    return COMM_SDK_ERR_STATUS;
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
    CommHandler_t *huart = NULL;
    uint16_t next_head = 0u;

    if (MPU_HAL_UART_MODE == txMode) {
        huart = &g_mpuUartRingBuf;
    } else if (MPU_HAL_SPI_MODE == txMode) {
        huart = &g_mpuSpiRingBuf;
    } else {
        TBOX_PRINT("MpuHalSetRingBuf: invalid txMode %d\n", txMode);
        return;
    }

    taskENTER_CRITICAL();
    for (uint16_t i = 0u; i < length; i++) {
        next_head = (huart->rx_ring_buf.head + 1) % MPU_HAL_RING_BUFFER;
        if (next_head != huart->rx_ring_buf.tail) {
            huart->rx_ring_buf.buf[huart->rx_ring_buf.head] = recv_data[i];
            huart->rx_ring_buf.head = next_head; // Update write index
        } else {
            huart->err_cnt++;
            if (MPU_HAL_UART_MODE == txMode) {
                TBOX_PRINT("Uart ring buf full! Err cnt: %d\n", huart->err_cnt);
            } else if (MPU_HAL_SPI_MODE == txMode) {
                TBOX_PRINT("Spi ring buf full! Err cnt: %d\n", huart->err_cnt);
            } else {

            }
            break;
        }
    }
    taskEXIT_CRITICAL();
}

/*************************************************
  Function: MpuHalReadRingBuf
  Description: Read data from ring buffer
  Input: RingBufferType* rb: pointer to the ring buffer structure
         uint8_t* data: pointer to store the read data
  Output: data: contains the read data if successful
  Return: 1 if read successfully, 0 if buffer is empty or parameters invalid
*************************************************/
static uint8_t MpuHalReadRingBuf(RingBufferType *rb, uint8_t *data)
{
    uint8_t read_ok = 0;

    if (rb != NULL && data != NULL) {
        taskENTER_CRITICAL();
        uint16_t current_head = rb->head;
        uint16_t current_tail = rb->tail;

        if (current_head != current_tail) {
            if (current_tail < MPU_HAL_RING_BUFFER) {
                *data = rb->buf[current_tail];
                current_tail = (current_tail + 1) % MPU_HAL_RING_BUFFER;
                rb->tail = current_tail;
                read_ok = 1;
            }
        }
        taskEXIT_CRITICAL();
    }
    return read_ok;
}

void CommSdk_UpdateState(MpuHalStartState_e state)
{
    g_mpuManage.startState = state;
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
    for (i = 0u; i < instanceMax; i++) {
        pHandleInstance = g_mpuManage.rxHandle + i;
        if ((pHandleInstance->useFlag == 1u) && (pHandleInstance->pDataBufferRx != NULL)) {
            if ((pHandleInstance->useRxFilter == 1) && (pData[2] == pHandleInstance->rxFilterConfig.aid)) {
                if ((pData[3] >= pHandleInstance->rxFilterConfig.midMin) && (pData[3] <= pHandleInstance->rxFilterConfig.midMax)) {
                    /*  copy data to receive buffer */
                    data = (pHandleInstance->rxIndexIn << 16);
                    for (j = 0u; j < length; j++) {
                        pHandleInstance->pDataBufferRx[pHandleInstance->rxIndexIn] = pData[j];
                        pHandleInstance->rxIndexIn++;
                        if (pHandleInstance->rxIndexIn >= pHandleInstance->dataBufferSize) {
                            pHandleInstance->rxIndexIn = 0;
                        }
                    }
                    /* dispatch data */
                    data += length;
                    if (irq != 0u) {
                        xQueueSendFromISR(pHandleInstance->rxQueueHandle, &data, &xHigherPriorityTaskWoken);
                    } else {
                        xQueueSend(pHandleInstance->rxQueueHandle, &data, 0);
                    }
                }
            }
        }
    }
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
    CommHandler_t *huart = &g_mpuUartRingBuf;
    uint8_t recv_data = 0U;
    uint8_t read_ok = 0U;

    // if (huart->init_success != 1) {
    //     // TBOX_PRINT(" Mpu hal uart init error,does not support protocal process\r\n");   //1ms print maybe reultl over loading
    // } else {
        for (uint8_t i = 0; i < 10; i++) {
            read_ok = MpuHalReadRingBuf(&huart->rx_ring_buf, &recv_data);
            if (read_ok == 0) {
                return;
            }
            switch (huart->proto_state) {
            case PROTOCOL_STATE_IDLE:
                // Wait for 1st header byte (0x55)
                if (recv_data == MPU_HAL_PACK_FIX_HEADER_0X55) {
                    huart->proto_buf.data[huart->proto_buf.dataCount] = recv_data;
                    huart->proto_buf.dataCount++;
                    huart->proto_state = PROTOCOL_STATE_WAIT_HEADER2;
                }
                break;

            case PROTOCOL_STATE_WAIT_HEADER2:
                // Wait for 2nd header byte (0xAA)
                if (recv_data == MPU_HAL_PACK_FIX_HEADER_0XAA) {
                    huart->proto_buf.data[huart->proto_buf.dataCount] = recv_data;
                    huart->proto_buf.dataCount++;
                    huart->proto_state = PROTOCOL_STATE_WAIT_HEADER;
                } else if (recv_data == MPU_HAL_PACK_FIX_HEADER_0XBB) {
                    huart->proto_buf.data[huart->proto_buf.dataCount] = recv_data;
                    huart->proto_buf.dataCount++;
                    huart->proto_state = PROTOCOL_STATE_WAIT_SPI_HEADER;
                } else {
                    huart->proto_buf.dataCount = 0;
                    huart->proto_state = PROTOCOL_STATE_IDLE;
                    TBOX_PRINT("Proto err: invalid header2 (0x%02x)\n", recv_data);
                    // huart->err_cnt++;
                }
                break;

            case PROTOCOL_STATE_WAIT_HEADER:
                // Collect remaining header fields until header length is reached
                huart->proto_buf.data[huart->proto_buf.dataCount] = recv_data;
                huart->proto_buf.dataCount++;

                if (huart->proto_buf.dataCount == MPU_PROTOCAL_HEADER_LEN) {
                    huart->proto_buf.dataLength = ((uint16_t)huart->proto_buf.data[5] << 8) | huart->proto_buf.data[6];
                    // Check if payload length exceeds max buffer size
                    if (huart->proto_buf.dataLength > MPU_HAL_RX_ONE_FRAME_MAX_SIZE) {
                        TBOX_PRINT("Proto err: data len over max (%d > %d)\n",
                                   huart->proto_buf.dataLength, MPU_HAL_RX_ONE_FRAME_MAX_SIZE);
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
                huart->proto_buf.data[huart->proto_buf.dataCount] = recv_data;
                huart->proto_buf.dataCount++;

                if (huart->proto_buf.dataCount == MPU_PROTOCAL_SPI_HEADER_LEN) {
                    huart->proto_buf.dataLength = ((uint16_t)huart->proto_buf.data[2] << 8) | huart->proto_buf.data[3];
                    if (huart->proto_buf.dataLength > MPU_HAL_RX_ONE_FRAME_MAX_SIZE) {
                        TBOX_PRINT("Proto err: data len over max (%d > %d)\n",
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
    // }
}

static void SpiProtocalProcess(void)
{
    CommHandler_t *huart = &g_mpuSpiRingBuf;
    uint8_t recv_data = 0U;
    uint8_t read_ok = 0U;

    // if (huart->init_success != 1) {
    //     // TBOX_PRINT(" Mpu hal uart init error,does not support protocal process\r\n");   //1ms print maybe reultl over loading
    // } else {
        for (uint8_t i = 0; i < 5; i++) {
            read_ok = MpuHalReadRingBuf(&huart->rx_ring_buf, &recv_data);
            if (read_ok == 0) {
                return;
            }
            switch (huart->proto_state) {
            case PROTOCOL_STATE_IDLE:
                // Wait for 1st header byte (0x55)
                if (recv_data == MPU_HAL_PACK_FIX_HEADER_0X55) {
                    huart->proto_buf.data[huart->proto_buf.dataCount] = recv_data;
                    huart->proto_buf.dataCount++;
                    huart->proto_state = PROTOCOL_STATE_WAIT_HEADER2;
                }
                break;

            case PROTOCOL_STATE_WAIT_HEADER2:
                // Wait for 2nd header byte (0xAA)
                if (recv_data == MPU_HAL_PACK_FIX_HEADER_0XAA) {
                    huart->proto_buf.data[huart->proto_buf.dataCount] = recv_data;
                    huart->proto_buf.dataCount++;
                    huart->proto_state = PROTOCOL_STATE_WAIT_HEADER;
                } else if (recv_data == MPU_HAL_PACK_FIX_HEADER_0XBB) {
                    huart->proto_buf.data[huart->proto_buf.dataCount] = recv_data;
                    huart->proto_buf.dataCount++;
                    huart->proto_state = PROTOCOL_STATE_WAIT_SPI_HEADER;
                } else {
                    huart->proto_buf.dataCount = 0;
                    huart->proto_state = PROTOCOL_STATE_IDLE;
                    TBOX_PRINT("Proto err: invalid header2 (0x%02x)\n", recv_data);
                    // huart->err_cnt++;
                }
                break;

            case PROTOCOL_STATE_WAIT_HEADER:
                // Collect remaining header fields until header length is reached
                huart->proto_buf.data[huart->proto_buf.dataCount] = recv_data;
                huart->proto_buf.dataCount++;

                if (huart->proto_buf.dataCount == MPU_PROTOCAL_HEADER_LEN) {
                    huart->proto_buf.dataLength = ((uint16_t)huart->proto_buf.data[5] << 8) | huart->proto_buf.data[6];
                    // Check if payload length exceeds max buffer size
                    if (huart->proto_buf.dataLength > MPU_HAL_RX_ONE_FRAME_MAX_SIZE) {
                        TBOX_PRINT("Proto err: data len over max (%d > %d)\n",
                                   huart->proto_buf.dataLength, MPU_HAL_RX_ONE_FRAME_MAX_SIZE);
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
                huart->proto_buf.data[huart->proto_buf.dataCount] = recv_data;
                huart->proto_buf.dataCount++;

                if (huart->proto_buf.dataCount == MPU_PROTOCAL_SPI_HEADER_LEN) {
                    huart->proto_buf.dataLength = ((uint16_t)huart->proto_buf.data[2] << 8) | huart->proto_buf.data[3];
                    if (huart->proto_buf.dataLength > MPU_HAL_RX_ONE_FRAME_MAX_SIZE) {
                        TBOX_PRINT("Proto err: data len over max (%d > %d)\n",
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
    // }
}


/*************************************************
 Function: PacketSend
 Description: Send a packet of data via UART or SPI DMA
 Input: MpuHalTXMode_e txMode: transmission mode (UART or SPI)
        uint32_t queueData: data to be sent (start address and length)
        MpuQueueTxBuffer_t *pTxBuffer: pointer to the transmission buffer
 Output: void
 Return: uint32_t: status code (OK or error code)
 Others:
*************************************************/
static uint32_t PacketSend(MpuHalTXMode_e txMode, uint32_t queueData, MpuQueueTxBuffer_t *pTxBuffer)
{
    uint32_t ret = COMM_SDK_OK;
    static uint8_t pTxData[MPU_HAL_MAX_BUFFSIZE] = {0};
    uint16_t startAddress = 0u;
    uint16_t length = 0u;
    uint8_t spiPackData[MPU_PROTOCAL_SPI_HEADER_LEN + 2048];
    uint16_t spiPackDataLen = 0;

    do {
        if (txMode >= MPU_HAL_TX_MODE_MAX) {
            ret = COMM_SDK_ERR_MODE;
            break;
        }
        // Common_DisableInterrupts();
        startAddress = (queueData >> MPU_HAL_16BIT_SHIFT) & MPU_HAL_16BIT_MASK;
        length = queueData & MPU_HAL_16BIT_MASK;
        for (uint16_t i = 0u; i < length; i++) {
            pTxData[i] = pTxBuffer->buffer[startAddress++];
            if (startAddress >= MPU_HAL_MAX_BUFFSIZE) {
                startAddress = 0;
            }
        }
        if (txMode == MPU_HAL_UART_MODE) {
            /* Construct MpuHalDataPack_t from raw data: [AID][MID][payload...] */
            MpuHalDataPack_t txPacket;
            txPacket.aid = pTxData[0];
            txPacket.mid = pTxData[1];
            txPacket.subcommand = 0;
            txPacket.pDataBuffer = (length > 2) ? (pTxData + 2) : NULL;
            txPacket.dataLength = (length > 2) ? (length - 2) : 0;
            txPacket.dataBufferSize = txPacket.dataLength;
            ret = MpuHalTransmit(0, &txPacket, MPU_HAL_UART_MODE);
        } else {
            /* Construct MpuHalDataPack_t from SPI packed data */
            MpuHalDataPack_t txPacket;
            txPacket.aid = pTxData[0];
            txPacket.mid = pTxData[1];
            txPacket.subcommand = 0;
            txPacket.pDataBuffer = (length > 2) ? (pTxData + 2) : NULL;
            txPacket.dataLength = (length > 2) ? (length - 2) : 0;
            txPacket.dataBufferSize = txPacket.dataLength;
            ret = MpuHalTransmit(0, &txPacket, MPU_HAL_SPI_MODE);
        }
        // Common_EnableInterrupts();
    } while (0);

    return ret;
}

/*************************************************
 Function: CommSdk_UartTxTask
 Description: Task to handle UART transmission for MPU HAL
 Input: void *pvParameters: task parameters (not used)
 Output: void
 Return: void
 Others:
*************************************************/
void CommSdk_UartTxTask(void *pvParameters)
{
    uint32_t ret = COMM_SDK_OK;
    uint32_t queueData = 0u;
    CommHandler_t *huart = &g_mpuUartRingBuf;

    g_mpuTxBuf[MPU_HAL_UART_MODE].txQueueHandle = xQueueCreate(MPU_HAL_TX_QUEUE_LEN, sizeof(uint32_t));

    while (1) {
        // if (huart->init_success == 0) {
        //     vTaskDelay(pdMS_TO_TICKS(100));
        //     continue;
        // }
        // FIXME


        /* Target platform mpuHal manages TX buffer internally, skip this check */
        if (xQueueReceive(g_mpuTxBuf[MPU_HAL_UART_MODE].txQueueHandle, &queueData, 0xFFFFFFFF) != pdPASS) {
            continue;
        }
        ret = PacketSend(MPU_HAL_UART_MODE, queueData, &g_mpuTxBuf[MPU_HAL_UART_MODE]);
        if (ret != COMM_SDK_OK) {
            TBOX_PRINT("UART PacketSend failed, ret = 0x%X\n", ret);
        }
    }
}

/*************************************************
 Function: CommSdk_SpiTxTask
 Description: Task to handle SPI transmission for MPU HAL
 Input: void *pvParameters: task parameters (not used)
 Output: void
 Return: void
 Others:
*************************************************/
void CommSdk_SpiTxTask(void *pvParameters)
{
    uint32_t ret = COMM_SDK_OK;
    uint32_t queueData = 0u;

    g_mpuTxBuf[MPU_HAL_SPI_MODE].txQueueHandle = xQueueCreate(MPU_HAL_TX_QUEUE_LEN, sizeof(uint32_t));

    while (1) {
        // if (spi_ready != 3) {
        //     vTaskDelay(pdMS_TO_TICKS(500));
        //     continue;
        // }
        //FIXME


        /* Check if buffer is available */
        // if (g_spiDmaTxBuf.dmaBufStatus != DMA_BUF_STATE_IDLE) {
        /* Target platform mpuHal manages TX buffer and SPI ready state internally */
        if (xQueueReceive(g_mpuTxBuf[MPU_HAL_SPI_MODE].txQueueHandle, &queueData, 0xFFFFFFFF) != pdPASS) {
            continue;
        }
        ret = PacketSend(MPU_HAL_SPI_MODE, queueData, &g_mpuTxBuf[MPU_HAL_SPI_MODE]);
        if (ret != COMM_SDK_OK) {
            TBOX_PRINT("SPi PacketSend failed, ret = 0x%X\n", ret);
        }
    }
}

void CommSdk_Sleep(void)
{
    MpuHal_Sleep();
    g_mpuManage.wakeMode = 0;
}


void CommSdk_Wakeup(void)
{
    MpuHal_Wakeup();
    g_mpuManage.wakeoutTimeCount = MPU_HAL_WAKEOUT_TIME_SLEEPVALUE;
    g_mpuManage.wakeMode = 1;
}