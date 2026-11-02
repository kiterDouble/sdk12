/*************************************************
 Copyright © 2025 SiRun (WuXi) . All rights reserved.
 File Name: bleHal.c
 Author: 
 Created Time: 
 Description: BLE UART HAL - Ported from Z20K148MS to AC7842x APIs
 Others: 
  NOTE: AC7842x only has UART0-UART3. BLE_UART_INSTANCE=4 may need 
        hardware reassignment if used on actual AC7842x silicon.
*************************************************/

/****************************** include ***************************************/
#include "bleHal.h"
#include "mpuHal.h"
#include "logHal.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#include "Core_AC784xx.h"   /* for NVIC_xxx, __disable_irq, __enable_irq */

/****************************** Macro Definitions ******************************/


/****************************** Type Definitions ******************************/
/****************************** Global Variables ******************************/


static BleUartHandleTypeDef g_bleUartRingBuf = {
    .uart_rx_rb = {.head = 0, .tail = 0},
    .proto_state = PARSE_STATE_HEADER,
    .init_success = 0,
    .err_cnt = 0
};

/* AC7842x UART channel config (replaces Z20K148MS Uart_ConfigType + Uart_FifoConfigType) */
static const Uart_ChannelConfigType g_Uart4Config = {
    .BaudRate = BLE_UART_BAUDRATE,
    .ParityMode = UART_PARITY_DISABLED,
    .StopBitCount = UART_ONE_STOP_BIT,
    .BitCountPerChar = UART_8_BITS_PER_CHAR,
#if (BLE_UART_DMA == 1)
    .TransferType = UART_USING_DMA,
#else
    .TransferType = UART_USING_INTERRUPTS,
#endif
    .SampleCnt = UART_SMP_CNT16,
    .RxDmaChannel = BLE_UART_DMA_RX_CHANNEL,
    .TxDmaChannel = BLE_UART_DMA_TX_CHANNEL,
    .RxCallback = NULL,
    .TxCallback = NULL,
};

#if (BLE_UART_DMA == 1)
/* AC7842x DMA channel config array */
static const Dma_ChannelConfigType g_dmaChannelCfg[] = {
    {
        .HwChannelId = BLE_UART_DMA_TX_CHANNEL,
        .VirtualChannelId = BLE_UART_DMA_TX_CHANNEL,
        .Priority = DMA_CHANNEL_PRIORITY_MEDIUM,
        .ReqSrc = DMA_REQ_UART3_TX   /* NOTE: AC7842x has no UART4 DMA req; adjust per actual UART instance */
    },
    {
        .HwChannelId = BLE_UART_DMA_RX_CHANNEL,
        .VirtualChannelId = BLE_UART_DMA_RX_CHANNEL,
        .Priority = DMA_CHANNEL_PRIORITY_MEDIUM,
        .ReqSrc = DMA_REQ_UART3_RX   /* NOTE: AC7842x has no UART4 DMA req; adjust per actual UART instance */
    },
};

/* AC7842x DMA controller config (replaces Z20K148MS Dma_ConfigType) */
static const Dma_ConfigType dmaConfig = {
    .ChannelCnt = 2U,
    .ChannelCfg = g_dmaChannelCfg,
};

/* Forward declarations for DMA callbacks */
static void UartDma_RxCallback(void *Args);
static void UartDma_TxCallback(void *Args);

/* AC7842x DMA TX transfer config (replaces Z20K148MS Dma_TransferConfigType) */
static Dma_TransferConfigType txTransferConfig = {
    .TriggerMode = FALSE,
    .CircularMode = FALSE,
    .IrqSrc = 0U,                          /* IRQ enabled via Dma_Hal_EnableChIrq */
    .SrcOffset = 1U,
    .DestOffset = 0U,
    .Length = BLE_UART_BUFFER_SIZE,
    .SrcUnit = DMA_TRANSFER_UNIT_1B,
    .DestUnit = DMA_TRANSFER_UNIT_1B,
    .Type = DMA_TRANSFER_MEM2PERIPH,
    .SrcStartAddr = 0U,                    /* Set dynamically before each transfer */
    .DestStartAddr = 0U,                   /* Set to UART TX data register */
    .SrcEndAddr = 0U,
    .DestEndAddr = 0U,
    .Callback = UartDma_TxCallback,
    .UserArgs = NULL,
};

/* AC7842x DMA RX transfer config (replaces Z20K148MS Dma_TransferConfigType) */
static Dma_TransferConfigType rxTransferConfig = {
    .TriggerMode = FALSE,
    .CircularMode = FALSE,                 /* Software double-buffering, not hardware circular */
    .IrqSrc = 0U,
    .SrcOffset = 0U,                       /* UART RX register is fixed */
    .DestOffset = 1U,
    .Length = BLE_UART_DMA_RX_THRESHOLD,
    .SrcUnit = DMA_TRANSFER_UNIT_1B,
    .DestUnit = DMA_TRANSFER_UNIT_1B,
    .Type = DMA_TRANSFER_PERIPH2MEM,
    .SrcStartAddr = 0U,                    /* Set to UART RX data register */
    .DestStartAddr = 0U,                   /* Set dynamically (ping-pong buffer) */
    .SrcEndAddr = 0U,
    .DestEndAddr = 0U,
    .Callback = UartDma_RxCallback,
    .UserArgs = NULL,
};
#endif /* BLE_UART_DMA == 1 */

/* DMA接收缓冲区管理器 - 双缓冲机制 (unchanged from original) */
static UartDmaRxManager_t g_dmaRxManager = {
    .ActiveBufIndex = 0,
    .ReadyBufIndex = 0,
    .ReadyBufLen = 0,
    .BufState = {DMA_BUF_STATE_IDLE, DMA_BUF_STATE_IDLE},
    .TotalRxCount = 0
};

/* DMA发送缓冲区管理器 (unchanged from original) */
static UartDmaTxManager_t g_dmaTxManager = {
    .BufferLen = 0,
    .dmaBufStatus = DMA_BUF_STATE_IDLE
};

static uint8_t g_BleInitFlag = 0;

/****************************** Function Declarations *************************/
#if (BLE_UART_DMA == 0)
static void UART4_RxCallback(uint8_t Instance, Uart_EventType Event);
static Hal_StatusType UART4_SendData(uint8_t *data, uint32_t dataLen);
#endif


void RingBuffer_Init(void)
{

}

bool RingBuffer_Write(RingBuffer_t *rb, uint8_t data)
{
    /* cal the next position of the write pointer */
    uint16_t next_head = (rb->head + 1) % BLE_UART_RING_BUFFER_SIZE;

    /* If full, drop */
    if (next_head == rb->tail) {
        return false;
    }

    /* store the data and move the pointer */
    rb->buffer[rb->head] = data;
    rb->head = next_head;

    return true;
}

/**
 * @brief 批量写入环形缓冲区（用于中断中高效写入）
 *
 * @param rb 环形缓冲区指针
 * @param data 数据缓冲区
 * @param len 数据长度
 * @return uint16_t 实际写入的字节数
 */
uint16_t RingBuffer_WriteBatch(RingBuffer_t *rb, const uint8_t *data, uint16_t len)
{
    uint16_t write_count = 0;
    uint16_t available_space;
    
    /* 计算可用空间 */
    if (rb->head >= rb->tail) {
        available_space = BLE_UART_RING_BUFFER_SIZE - rb->head + rb->tail - 1;
    } else {
        available_space = rb->tail - rb->head - 1;
    }
    
    /* 限制写入长度不超过可用空间 */
    if (len > available_space) {
        len = available_space;
    }
    
    /* 批量写入数据 */
    while (write_count < len) {
        rb->buffer[rb->head] = data[write_count];
        write_count++;
        rb->head = (rb->head + 1) % BLE_UART_RING_BUFFER_SIZE;
    }
    
    return write_count;
}

bool RingBuffer_Read(RingBuffer_t *rb, uint8_t *data)
{
    /* If empty, return false */
    if (rb->head == rb->tail) {
        return false;
    }

    /* Get the data and move the pointer */
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % BLE_UART_RING_BUFFER_SIZE;

    return true;
}

/**
 * @brief 批量读取
 * 
 * @param rb 环形缓冲区指针
 * @param data 数据缓冲区
 * @param maxLen 最大读取数据长度
 * @return uint16_t 实际读取的字节数
 */
uint16_t RingBuffer_ReadBatch(RingBuffer_t *rb, uint8_t *data, uint16_t maxLen)
{
    uint16_t count = 0;

    while (count < maxLen && rb->head != rb->tail) {
        data[count] = rb->buffer[rb->tail];
        rb->tail = (rb->tail + 1) % BLE_UART_RING_BUFFER_SIZE;
        count++;
    }
    
    return count;
}

#if (BLE_UART_DMA == 1)
/**
 * @brief DMA接收管理器初始化
 */
static void UartDma_RxManagerInit(void)
{
    g_dmaRxManager.ActiveBufIndex = 0;
    g_dmaRxManager.ReadyBufIndex = 0;
    g_dmaRxManager.ReadyBufLen = 0;
    g_dmaRxManager.BufState[0] = DMA_BUF_STATE_IDLE;
    g_dmaRxManager.BufState[1] = DMA_BUF_STATE_IDLE;
    g_dmaRxManager.TotalRxCount = 0;
}

/**
 * @brief DMA接收完成中断回调函数 (AC7842x signature: void (*)(void *Args))
 * @note  实现双缓冲切换和数据拷贝到环形缓冲区
 */
static void UartDma_RxCallback(void *Args)
{
    Dma_ChannelCBInfoType *cbInfo = (Dma_ChannelCBInfoType *)Args;
    uint8_t completed_buf_index;
    uint16_t transferred_len;
    uint8_t next_buf_index;
    uint16_t write_count;
    
    (void)cbInfo;  /* Channel info available via cbInfo->ChannelId, cbInfo->DmaEvent */

    /* 获取已完成的缓冲区索引 */
    completed_buf_index = g_dmaRxManager.ActiveBufIndex;
    
    /* 计算实际传输的数据长度 (AC7842x: Dma_Hal_GetTransBytes) */
    transferred_len = (uint16_t)Dma_Hal_GetTransBytes(BLE_UART_DMA_RX_CHANNEL);
    
    /* 如果传输长度为0，可能是错误，使用阈值作为默认值 */
    if (transferred_len == 0U) {
        transferred_len = BLE_UART_DMA_RX_THRESHOLD;
    }
    
    /* 标记当前缓冲区数据就绪 */
    g_dmaRxManager.BufState[completed_buf_index] = DMA_BUF_STATE_READY;
    g_dmaRxManager.ReadyBufLen = transferred_len;
    g_dmaRxManager.TotalRxCount += transferred_len;
    
    /* 将就绪数据拷贝到环形缓冲区 */
    if (g_dmaRxManager.ReadyBufLen > 0U) {
        write_count = RingBuffer_WriteBatch(&g_bleUartRingBuf.uart_rx_rb,
            g_dmaRxManager.Buffer[completed_buf_index],
            g_dmaRxManager.ReadyBufLen);
        
        /* 如果写入失败（环形缓冲区满），记录错误 */
        if (write_count < g_dmaRxManager.ReadyBufLen) {
            g_bleUartRingBuf.err_cnt++;
        }
        
        /* 标记缓冲区空闲 */
        g_dmaRxManager.BufState[completed_buf_index] = DMA_BUF_STATE_IDLE;
    }
    
    /* 切换到下一个缓冲区 */
    next_buf_index = (completed_buf_index + 1U) % BLE_UART_DMA_RX_BUF_NUM;
    
    /* 检查下一个缓冲区是否空闲 */
    if (g_dmaRxManager.BufState[next_buf_index] == DMA_BUF_STATE_IDLE) {
        Dma_ChannelAddrType chAddr;
        
        g_dmaRxManager.ActiveBufIndex = next_buf_index;
        g_dmaRxManager.BufState[next_buf_index] = DMA_BUF_STATE_BUSY;
        
        /* 设置新的目标地址 (AC7842x: Dma_Hal_UpdateChAddr) */
        chAddr.SrcOffset = 0U;
        chAddr.DestOffset = 1U;
        chAddr.Length = BLE_UART_DMA_RX_THRESHOLD;
        chAddr.SrcStartAddr = rxTransferConfig.SrcStartAddr;
        chAddr.DestStartAddr = (uint32_t)g_dmaRxManager.Buffer[next_buf_index];
        chAddr.SrcEndAddr = rxTransferConfig.SrcStartAddr;
        chAddr.DestEndAddr = (uint32_t)g_dmaRxManager.Buffer[next_buf_index] + BLE_UART_DMA_RX_THRESHOLD;
        Dma_Hal_UpdateChAddr(BLE_UART_DMA_RX_CHANNEL, &chAddr);
        
        /* 重新启动DMA通道 (AC7842x: Dma_Hal_StartCh replaces Dma_ChannelRequestEnable) */
        (void)Dma_Hal_StartCh(BLE_UART_DMA_RX_CHANNEL);
    } else {
        /* 下一个缓冲区不空闲，记录错误 */
        g_dmaRxManager.BufState[completed_buf_index] = DMA_BUF_STATE_ERROR;
        g_bleUartRingBuf.err_cnt++;
    }
}

/**
 * @brief DMA发送完成中断回调函数 (AC7842x signature)
 */
static void UartDma_TxCallback(void *Args)
{
    (void)Args;
    /* 标记发送缓冲区为空闲 */
    g_dmaTxManager.dmaBufStatus = DMA_BUF_STATE_IDLE;
}

/**
 * @brief DMA传输配置 (AC7842x: Dma_Hal_ConfigCh replaces Dma_ConfigTransfer)
 */
static void UartDma_TransferConfig(void)
{
    Hal_StatusType ret;

    /* 配置TX DMA */
    txTransferConfig.DestStartAddr = 0U;   /* Will be set to UART TX register address at init */
    txTransferConfig.DestEndAddr = 0U;
    ret = Dma_Hal_ConfigCh(BLE_UART_DMA_TX_CHANNEL, &txTransferConfig);
    if (ret != STATUS_SUCCESS) {
        TBOX_PRINT("DMA TX config failed\r\n");
        return;
    }

    /* 配置RX DMA */
    rxTransferConfig.SrcStartAddr = 0U;    /* Will be set to UART RX register address at init */
    rxTransferConfig.SrcEndAddr = 0U;
    rxTransferConfig.DestStartAddr = (uint32_t)g_dmaRxManager.Buffer[0];
    rxTransferConfig.DestEndAddr = (uint32_t)g_dmaRxManager.Buffer[0] + BLE_UART_DMA_RX_THRESHOLD;
    ret = Dma_Hal_ConfigCh(BLE_UART_DMA_RX_CHANNEL, &rxTransferConfig);
    if (ret != STATUS_SUCCESS) {
        TBOX_PRINT("DMA RX config failed\r\n");
        return;
    }
    
    /* Enable DMA interrupts for RX channel (done + error) */
    Dma_Hal_EnableChIrq(BLE_UART_DMA_RX_CHANNEL, 0x03U);  /* bits: done | error */
    
    /* Enable DMA interrupts for TX channel (done only) */
    Dma_Hal_EnableChIrq(BLE_UART_DMA_TX_CHANNEL, 0x01U);
}

/**
 * @brief UART4 DMA初始化 (AC7842x version)
 */
static void Uart4_DmaInit(void)
{
    /* 初始化DMA接收管理器 */
    UartDma_RxManagerInit();
    
    /* Reset and enable DMA peripheral clock (AC7842x: Ckgen_Hal_EnablePeriphClk replaces Sysctrl_ResetModule/EnableModule) */
    /* NOTE: DMA and DMAMUX are typically enabled by startup code; add explicit enable if needed */
    
    /* Initializes DMA (AC7842x: Dma_Hal_Init replaces Dma_Init) */
    Dma_Hal_Init(&dmaConfig);

    /* DMA transfer config (configures each channel) */
    UartDma_TransferConfig();

    /* AC7842x DMAMUX is configured via Dma_ChannelConfigType.ReqSrc - no separate DMAMUX API calls needed */

    /* Set DMA interrupt priorities and enable IRQs (AC7842x: NVIC_xxx replaces Int_xxx) */
    NVIC_SetPriority(DMA0_CHANNEL11_IRQn, 0x3U);   /* RX channel 11 */
    NVIC_EnableIRQ(DMA0_CHANNEL11_IRQn);
    
    NVIC_SetPriority(DMA0_CHANNEL10_IRQn, 0x3U);   /* TX channel 10 */
    NVIC_EnableIRQ(DMA0_CHANNEL10_IRQn);
    
    /* 标记第一个缓冲区为忙碌状态 */
    g_dmaRxManager.BufState[0] = DMA_BUF_STATE_BUSY;
    
    /* 启动RX DMA通道 (AC7842x: Dma_Hal_StartCh replaces Dma_ChannelRequestEnable) */
    (void)Dma_Hal_StartCh(BLE_UART_DMA_RX_CHANNEL);
}

/**
 * @brief DMA方式发送数据 (AC7842x version)
 * @param data 发送数据指针
 * @param dataLen 数据长度
 * @return STATUS_SUCCESS 成功, STATUS_ERROR 失败
 */
static Hal_StatusType Uart4_DmaSendData(uint8_t *data, uint32_t dataLen)
{
    Dma_ChannelAddrType chAddr;
    
    if (data == NULL || dataLen == 0U) {
        return STATUS_ERROR;
    }

    if (g_BleInitFlag == 0U) {
        return STATUS_ERROR;
    }

    /* 检查发送缓冲区是否空闲 */
    if (g_dmaTxManager.dmaBufStatus == DMA_BUF_STATE_BUSY) {
        return STATUS_ERROR;
    }

    /* 限制最大发送长度 */
    if (dataLen > BLE_UART_BUFFER_SIZE) {
        dataLen = BLE_UART_BUFFER_SIZE;
    }

    /* 拷贝数据到DMA发送缓冲区 */
    (void)memcpy(g_dmaTxManager.Buffer, data, (size_t)dataLen);
    g_dmaTxManager.BufferLen = (uint16_t)dataLen;

    /* 停止DMA通道 (AC7842x: Dma_Hal_StopCh replaces Dma_HaltControl) */
    (void)Dma_Hal_StopCh(BLE_UART_DMA_TX_CHANNEL);
    
    /* Wait for channel to become idle */
    {
        uint32 timeout = 10000U;
        while (Dma_Hal_GetChStatus(BLE_UART_DMA_TX_CHANNEL) != STATUS_IDLE) {
            if (--timeout == 0U) {
                break;
            }
        }
    }

    /* 更新源地址和传输长度 (AC7842x: Dma_Hal_UpdateChAddr replaces Dma_SetSrcAddr/Dma_SetMinorLoopNum) */
    chAddr.SrcOffset = 1U;
    chAddr.DestOffset = 0U;
    chAddr.Length = (uint16)dataLen;
    chAddr.SrcStartAddr = (uint32_t)g_dmaTxManager.Buffer;
    chAddr.DestStartAddr = txTransferConfig.DestStartAddr;
    chAddr.SrcEndAddr = (uint32_t)g_dmaTxManager.Buffer + dataLen;
    chAddr.DestEndAddr = txTransferConfig.DestStartAddr;
    Dma_Hal_UpdateChAddr(BLE_UART_DMA_TX_CHANNEL, &chAddr);

    /* 标记为忙碌 */
    g_dmaTxManager.dmaBufStatus = DMA_BUF_STATE_BUSY;

    /* 启动TX DMA通道 (AC7842x: Dma_Hal_StartCh replaces Dma_ChannelRequestEnable) */
    (void)Dma_Hal_StartCh(BLE_UART_DMA_TX_CHANNEL);

    return STATUS_SUCCESS;
}
#endif /* BLE_UART_DMA == 1 */


/****************************** Public Function Implementations ***************/
/*************************************************
  Function:     UART4_Init
  Description:  UART4 module init (AC7842x version)
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
static void UART4_Init(void)
{
    Hal_StatusType retStatus;

    /* Disable and reset UART4 module clock (AC7842x: Ckgen_Hal_EnablePeriphClk replaces Sysctrl_xxx) */
    (void)Ckgen_Hal_EnablePeriphClk(CKGEN_UART3_BUS_CLK, FALSE);  /* NOTE: using UART3 IDs as placeholder for UART4 */
    
    /* Set UART clock source (AC7842x: Ckgen_Hal_SetPeriphClkMux replaces Clk_SetModuleSrc) */
    /* NOTE: CKGEN_UART4_CLK may not exist on AC7842x; adjust Ckgen_ClkIdType per actual hardware */
    retStatus = Ckgen_Hal_SetPeriphClkMux(CKGEN_UART3_CLK, CKGEN_CORE_CLK);
    if (retStatus != STATUS_SUCCESS) {
        TBOX_PRINT("Ckgen_Hal_SetPeriphClkMux fail!\r\n");
        return;
    }
    
    /* Set clock divider to 1 (AC7842x: Ckgen_Hal_SetPeriphClkDiv replaces Clk_SetClkDivider) */
    (void)Ckgen_Hal_SetPeriphClkDiv(CKGEN_UART3_CLK, 1U);
    
    /* Enable UART module clock */
    (void)Ckgen_Hal_EnablePeriphClk(CKGEN_UART3_BUS_CLK, TRUE);

    /* Init UART4 (AC7842x: Uart_Hal_Init replaces Uart_Init + Uart_Enable + Uart_ConfigFifo) */
    Uart_Hal_Init(BLE_UART_INSTANCE, &g_Uart4Config);

#if (BLE_UART_DMA == 1)
    /* DMA模式: 使能UART空闲中断用于检测数据流结束 (AC7842x: Uart_Hal_SetIdleInterrupt replaces Uart_ConfigIdleDetect) */
    Uart_Hal_SetIdleInterrupt(BLE_UART_INSTANCE, TRUE);
#else
    /* 中断模式: 配置接收回调 (AC7842x: callback in Uart_Hal_ReceiveData replaces Uart_InstallCallBackFunc + Uart_IntMask) */
    /* Callback is passed when Uart_Hal_ReceiveData is called */
    NVIC_SetPriority(UART4_IRQn, 0x3U);     /* NOTE: UART4_IRQn may not exist on AC7842x */
    NVIC_EnableIRQ(UART4_IRQn);
#endif

    /* Enable GPIO clock (AC7842x: Ckgen_Hal_xxx replaces Clk_SetModuleSrc for GPIO) */
    /* GPIO port clocks are typically enabled at system init */

    /* Set PORT pin mux for UART (AC7842x: Gpio_Hal_SetMuxMode replaces Port_ConfigPinmux) */
    Gpio_Hal_SetMuxMode(BLE_UART_RX_PORT, BLE_UART_RX_PIN, BLE_UART_RX_PIN_MUX);
    Gpio_Hal_SetMuxMode(BLE_UART_TX_PORT, BLE_UART_TX_PIN, BLE_UART_TX_PIN_MUX);

    g_BleInitFlag = 1;
}



#if (BLE_UART_DMA == 0)
/*************************************************
  Function:     UART4_RxCallback
  Description:  UART4 receive callback (AC7842x Uart_CallbackType signature)
  Input:        Instance - UART instance, Event - callback event type
  Output:       None
  Return:       None
  Others:       None
*************************************************/
static void UART4_RxCallback(uint8_t Instance, Uart_EventType Event)
{
    BleUartHandleTypeDef *huart = &g_bleUartRingBuf;
    uint8_t rx_batch_buf[32];
    uint16_t write_count;
    
    (void)Instance;
    
    if (Event == UART_EVENT_RX_FULL) {
        /* AC7842x UART HAL fills its internal buffer; 
         * for ring-buffer integration, read available data via polling */
        uint8_t i;
        for (i = 0; i < 32U; i++) {
            /* Read one byte at a time from UART data register */
            rx_batch_buf[i] = UART_Type_ReadByte(BLE_UART_INSTANCE);
        }
        
        write_count = RingBuffer_WriteBatch(&huart->uart_rx_rb, rx_batch_buf, 32U);
        
        if (write_count < 32U) {
            huart->err_cnt++;
        }
    }
}

/*************************************************
  Function:     UART4_SendData
  Description:  Send data through UART4 (AC7842x polling mode)
  Input:        data - pointer to data, dataLen - length
  Output:       None
  Return:       STATUS_SUCCESS or STATUS_ERROR
  Others:       None
*************************************************/
static Hal_StatusType UART4_SendData(uint8_t *data, uint32_t dataLen)
{
    Hal_StatusType ret;

    if (g_BleInitFlag == 0U) {
        return STATUS_ERROR;
    }

    /* AC7842x: Uart_Hal_SendDataPolling replaces Uart_SendByte + Uart_GetLineStatus loop */
    ret = Uart_Hal_SendDataPolling(BLE_UART_INSTANCE, data, dataLen);
    
    return ret;
}
#endif /* BLE_UART_DMA == 0 */


static void Ble_GpioInit(void)
{
    /* Init Power Pin (AC7842x: Gpio_Hal_xxx replaces Port_ConfigPinmux/Gpio_SetPinDir/Gpio_WritePinOutput) */
    Gpio_Hal_SetMuxMode(BLE_PWR_PORT, BLE_PWR_PIN, BLE_PWR_PIN_MUX);
    Gpio_Hal_SetPinDirection(BLE_PWR_PORT, BLE_PWR_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(BLE_PWR_PORT, BLE_PWR_PIN, GPIO_HIGH_LEVEL);

    /* Init BLE RST Pin */
    Gpio_Hal_SetMuxMode(BLE_RST_PORT, BLE_RST_PIN, BLE_RST_PIN_MUX);
    Gpio_Hal_SetPinDirection(BLE_RST_PORT, BLE_RST_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(BLE_RST_PORT, BLE_RST_PIN, GPIO_HIGH_LEVEL);

    /* Init BLE锚点 5V Enable Pin */
    Gpio_Hal_SetMuxMode(BLE_ANCHOR_EN_PORT, BLE_ANCHOR_EN_PIN, BLE_ANCHOR_EN_PIN_MUX);
    Gpio_Hal_SetPinDirection(BLE_ANCHOR_EN_PORT, BLE_ANCHOR_EN_PIN, GPIO_OUTPUT_DIRECTION);
    Gpio_Hal_WritePin(BLE_ANCHOR_EN_PORT, BLE_ANCHOR_EN_PIN, GPIO_HIGH_LEVEL);

    /* Init BLE锚点状态检测 Pin */
    Gpio_Hal_SetMuxMode(BLE_ANCHOR_STATUS_PORT, BLE_ANCHOR_STATUS_PIN, BLE_ANCHOR_STATUS_PIN_MUX);
    Gpio_Hal_SetPinDirection(BLE_ANCHOR_STATUS_PORT, BLE_ANCHOR_STATUS_PIN, GPIO_INPUT_DIRECTION);

    /* Configure GPIO interrupt on falling edge (AC7842x APIs replace Z20K148MS Port_ConfigPinInt + Int_xxx) */
    Gpio_Hal_SetPullSel(BLE_ANCHOR_STATUS_PORT, BLE_ANCHOR_STATUS_PIN, PORT_INTERNAL_PULL_NOT_ENABLED);
    Gpio_Hal_SetPinIntSel(BLE_ANCHOR_STATUS_PORT, BLE_ANCHOR_STATUS_PIN, PORT_INT_FALLING_EDGE);
    
    NVIC_SetPriority(BLE_ANCHOR_STATUS_IRQ_IRQN, 0x3U);
    NVIC_EnableIRQ(BLE_ANCHOR_STATUS_IRQ_IRQN);
}

/*************************************************
  Function:     BleHal_Init
  Description:  BLE uart init
  Input:        None
  Output:       None
  Return:       None
  Others:       None
*************************************************/
void BleHal_Init(void)
{
    BleUartHandleTypeDef *huart = &g_bleUartRingBuf;

    Ble_GpioInit();

    UART4_Init();

#if (BLE_UART_DMA == 1)
    /* DMA模式：UART初始化后再初始化DMA */
    Uart4_DmaInit();
#endif
    
    huart->init_success = 1;
}

void BleHal_PowerOn(void)
{
    Gpio_Hal_WritePin(BLE_PWR_PORT, BLE_PWR_PIN, 1U);
}

void BleHal_PowerOff(void)
{
    Gpio_Hal_WritePin(BLE_PWR_PORT, BLE_PWR_PIN, 0U);
}

Hal_StatusType BleHalTransmit(uint8_t *data, uint32_t dataLen)
{
#if (BLE_UART_DMA == 1)
    return Uart4_DmaSendData(data, dataLen);
#else
    return UART4_SendData(data, dataLen);
#endif
}

BleUartHandleTypeDef *BleHal_GetInstance(void)
{
    return &g_bleUartRingBuf;
}
