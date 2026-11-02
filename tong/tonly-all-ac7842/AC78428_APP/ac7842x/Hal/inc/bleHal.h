/*************************************************
 Copyright © 2025 SiRun (WuXi) . All rights reserved.
 File Name: bleHal.h
 Author:
 Created Time:
 Description:
 Others:
*************************************************/

#ifndef __BLE_HAL_H__
#define __BLE_HAL_H__

#include <stdint.h>
#include <stdbool.h>

#include "Std_Types.h"
#include "Gpio_Hal.h"
#include "Gpio_Hal_Types.h"
#include "Ckgen_Hal.h"
#include "Uart_Hal.h"
#include "Uart_Hal_Types.h"
#include "Dma_Hal.h"


#define BLE_UART_DMA                1U

/* BLE UART Instance: UART4 */
#define BLE_UART_INSTANCE           4

/* BLE UART Pin Mux values (AC7842x PORT_MUX_ALTx) */
/* PTEx pins for UART4: PTE15(RX)=ALT2, PTE16(TX)=ALT2 on AC7842x */
#define BLE_UART_RX_MUX_VAL         PORT_MUX_ALT2
#define BLE_UART_TX_MUX_VAL         PORT_MUX_ALT2

/* DMA Channel Numbers for UART4 TX/RX on AC7842x */
#define BLE_UART_DMA_TX_CHANNEL     10
#define BLE_UART_DMA_RX_CHANNEL     11

#define BLE_UART_BUFFER_SIZE        512U

/* DMA双缓冲配置 */
#define BLE_UART_DMA_RX_BUF_NUM     2U              /* 双缓冲区数量 */
#define BLE_UART_DMA_RX_THRESHOLD   32U             /* DMA传输完成阈值（字节数） */

/* DMA缓冲区状态枚举 */
typedef enum {
    DMA_BUF_STATE_IDLE = 0,     /* 空闲，可用于DMA传输 */
    DMA_BUF_STATE_BUSY,         /* DMA正在使用 */
    DMA_BUF_STATE_READY,        /* 数据就绪，待处理 */
    DMA_BUF_STATE_ERROR         /* 错误状态 */
} DmaBufStateType;

/* DMA接收缓冲区管理结构 */
typedef struct {
    uint8_t Buffer[BLE_UART_DMA_RX_BUF_NUM][BLE_UART_BUFFER_SIZE];  /* 双缓冲区 */
    volatile uint8_t ActiveBufIndex;     /* 当前DMA使用的缓冲区索引 */
    volatile uint8_t ReadyBufIndex;      /* 数据就绪待处理的缓冲区索引 */
    volatile uint16_t ReadyBufLen;       /* 就绪缓冲区数据长度 */
    volatile DmaBufStateType BufState[BLE_UART_DMA_RX_BUF_NUM];  /* 缓冲区状态 */
    volatile uint32_t TotalRxCount;      /* 总接收计数（调试用） */
} UartDmaRxManager_t;

/* DMA发送缓冲区管理结构 */
typedef struct {
    uint8_t Buffer[BLE_UART_BUFFER_SIZE];    /* 发送缓冲区 */
    volatile uint16_t BufferLen;             /* 待发送数据长度 */
    volatile DmaBufStateType dmaBufStatus;   /* 缓冲区状态 */
} UartDmaTxManager_t;



/* BLE RX Pin: PTE15 */
#define BLE_UART_RX_PORT            PORTID_E
#define BLE_UART_RX_PIN             15
#define BLE_UART_RX_PIN_MUX         PORT_MUX_ALT2

/* BLE Tx Pin: PTE16 */
#define BLE_UART_TX_PORT            PORTID_E
#define BLE_UART_TX_PIN             16
#define BLE_UART_TX_PIN_MUX         PORT_MUX_ALT2

/* BLE Power Pin: PTD1 */
#define BLE_PWR_PORT                PORTID_D
#define BLE_PWR_PIN                 1
#define BLE_PWR_PIN_MUX             PORT_MUX_AS_GPIO

/* BLE Rst Pin: PTC11 */
#define BLE_RST_PORT                PORTID_C
#define BLE_RST_PIN                 11
#define BLE_RST_PIN_MUX             PORT_MUX_AS_GPIO
#define BLE_RST_IRQ                 PORTC_IRQn

/* BLE Anchor 5V Enable Pin: PTA0 */
#define BLE_ANCHOR_EN_PORT          PORTID_A
#define BLE_ANCHOR_EN_PIN           0
#define BLE_ANCHOR_EN_PIN_MUX       PORT_MUX_AS_GPIO

/* BLE Anchor 5V Abnormal Pin: PTC17 */
#define BLE_ANCHOR_STATUS_PORT      PORTID_C
#define BLE_ANCHOR_STATUS_PIN       17
#define BLE_ANCHOR_STATUS_PIN_MUX   PORT_MUX_AS_GPIO
#define BLE_ANCHOR_STATUS_IRQ_IRQN  PORTC_IRQn



#define BLE_UART_BAUDRATE           460800U
#define BLE_UART_RING_BUFFER_SIZE   2048U
#define BLE_UART_FRAME_DATA_SIZE    512U
#define UART_BLE_RX_BUF_LEN 512


#define UART_WAIT_FOR_BUSY_CLEAR    7000U

typedef enum {
    PARSE_STATE_HEADER = 0,         /* 等待帧头 */
    PARSE_STATE_DATALEN_LOW = 1,    /* 接收数据长度低字节 */
    PARSE_STATE_DATALEN_HIGH = 2,   /* 接收数据长度高字节 */
    PARSE_STATE_CHANNEL = 3,        /* 接收通道号 */
    PARSE_STATE_DATA = 4,           /* 接收数据域 */
    PARSE_STATE_TAILER = 5,         /* 接收帧尾 */
    PARSE_STATE_CRC = 6,            /* 接收CRC校验 */
    PARSE_STATE_COMPLETE
} ParseStateType;

typedef struct {
    uint8_t buf[UART_BLE_RX_BUF_LEN];   // Buffer storage
    volatile uint16_t head;             // Write index (updated by interrupt)
    volatile uint16_t tail;             // Read index (updated by main loop)
} RingBufferType;

/* BLE CAN Data struct */
typedef struct {
    uint32_t    id;     /* Little Edition */
    uint8_t     dlc;    /* data length */
    uint8_t     data[0];
} CanBleData_t;

typedef struct {
    uint8_t  channel;
    uint16_t data_len;          /* Little Edition */
    uint8_t  data[BLE_UART_FRAME_DATA_SIZE];
    uint8_t  crc;               /* CRC received */
    uint8_t  calc_crc;          /* CRC Calculated */
} BleUartFrame_t;

typedef struct {
    uint8_t buffer[BLE_UART_RING_BUFFER_SIZE];
    volatile uint16_t head;                 /* 写指针 */
    volatile uint16_t tail;                 /* 读指针 */
} RingBuffer_t;

typedef struct {
    // RingBufferType rx_ring_buf;         // RX ring buffer instance
    RingBuffer_t uart_rx_rb;
    ParseStateType proto_state;         // Current state of protocol parser
    BleUartFrame_t frame;               // Buffer for storing protocol data
    uint8_t init_success;               // Init status (1 = success, 0 = failure)
    uint16_t err_cnt;                   // Error counter (buffer full, invalid protocol)
} BleUartHandleTypeDef;



/****************************** include ***************************************/
/****************************** Macro Definitions *****************************/
/****************************** Type Definitions ******************************/
/****************************** Function Declarations *************************/
void BleHal_Init(void);
void BleHal_PowerOn(void);
void BleHal_PowerOff(void);
Hal_StatusType BleHalTransmit(uint8_t *pData,uint32_t txLength);
// int16_t BleHalRead(uint8_t *pData,uint32_t readCount,uint32_t *pLength);
void BleHalSetMode(uint8_t BleHalRead);

bool RingBuffer_Read(RingBuffer_t *rb, uint8_t *data);
uint16_t RingBuffer_ReadBatch(RingBuffer_t *rb, uint8_t *data, uint16_t maxLen);
uint16_t RingBuffer_WriteBatch(RingBuffer_t *rb, const uint8_t *data, uint16_t len);

BleUartHandleTypeDef *BleHal_GetInstance(void);

#endif  /* __BLE_HAL_H__ */