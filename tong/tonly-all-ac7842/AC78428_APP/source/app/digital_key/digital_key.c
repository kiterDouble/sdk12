#include "digital_key.h"
#include "canPeriodTask.h"
#include "bleHal.h"
#include "logHal.h"
#include <string.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "crc8_16_32.h"


#define UART_RX_BUF_MAX_SIZE 512
#define UART_TX_BUF_MAX_SIZE 512

typedef struct {
    int16_t canHandler;
    CanHalMsg_t canDriverRxBuffer[DIGITAL_KEY_CAN_RX_BUFFER_SIZE];
} DigitalKeyCtx_t;

static DigitalKeyCtx_t g_DigitalKeyCtx = {0};

/**
 * @brief DigitalKey Initialize
 *
 */
static void DigitalKey_Init(void)
{
    CanHalMsgFilter_t canFilter;
    g_DigitalKeyCtx.canHandler = CanHalOpen(DIGITAL_KEY_CAN_CHANNEL);
    if (g_DigitalKeyCtx.canHandler < 0) // invalid handle
    {
        for (;;) {
            TBOX_PRINT("vehile data to cpu : Bcan handle open error\r\n");
            vTaskDelay(1000);
            return;
        }
    }
    canFilter.useStdFilter = 0x01;
    canFilter.canIdStdMin = 0x00;
    canFilter.canIdStdMax = 0xFFF;
    canFilter.useExdtFilter = 0x00;
    canFilter.canIdExtdMin = 0x00;
    canFilter.canIdExtdMax = 0xFFFFFFFF;
    CanHalSetFilter(g_DigitalKeyCtx.canHandler, &canFilter);
    CanHalEnableTransmitLoopBack(g_DigitalKeyCtx.canHandler);
    CanHalSetRxBuffer(g_DigitalKeyCtx.canHandler, g_DigitalKeyCtx.canDriverRxBuffer, DIGITAL_KEY_CAN_RX_BUFFER_SIZE);
}

/**
 * @brief Pack business data into specified frame format
 * @param channel Channel number (0~0xFF)
 * @param data Business data buffer (NULL means no data)
 * @param dataLen Business data length (0~512)
 * @param outBuf Output frame buffer (pre-allocated)
 * @param outBufLen Output buffer length
 * @return int Success: frame total length, Fail: -1
 * @note outBuf minimum length: FRAME_FIX_LEN + dataLen
 */
static int FramePack(uint8_t channel, const uint8_t *data, uint16_t dataLen, uint8_t *outBuf, uint16_t outBufLen)
{
    // All variables defined at function head and initialized
    uint16_t framePos = 0;
    uint8_t crc = 0;
    uint16_t needLen = 0;

    // 1. Parameter validation
    if (outBuf == NULL) {
        TBOX_PRINT("outBuf is NULL");
        return -1;
    }

    needLen = BLE_UATY_FRAME_FIX_LEN + dataLen; // Calculate required buffer length
    if (outBufLen < needLen) {
        TBOX_PRINT("outBufLen(%d) < need len(%d)", outBufLen, needLen);
        return -1;
    }

    if (dataLen > UART_RX_BUF_MAX_SIZE) {
        TBOX_PRINT("dataLen(%d) > MAX_DATA_LEN(%d)", dataLen, UART_RX_BUF_MAX_SIZE);
        return -1;
    }

    if (dataLen > 0 && data == NULL) {
        TBOX_PRINT("data is NULL but dataLen(%d) > 0", dataLen);
        return -1;
    }

    // 2. Pack frame fields (strictly follow format)
    // 2.1 Frame header
    outBuf[framePos++] = BLE_UART_FRAME_HEADER;
    // 2.2 DataLen (little-endian: low byte first)
    outBuf[framePos++] = (uint8_t)(dataLen & 0xFF);        // Low byte
    outBuf[framePos++] = (uint8_t)((dataLen >> 8) & 0xFF); // High byte
    // 2.3 Channel number
    outBuf[framePos++] = channel;
    // 2.4 Business data
    if (dataLen > 0) {
        memcpy(&outBuf[framePos], data, dataLen);
        framePos += dataLen;
    }
    // 2.5 Frame tail
    outBuf[framePos++] = BLE_UART_FRAME_TAILER;

    // 3. Calculate CRC8 (range: Header~Tailer)
    crc = cal_crc8(outBuf, framePos);
    outBuf[framePos++] = crc;

    return framePos; // Return packed frame total length
}

/**
 * @brief 接收BLE瞄点数据并通过UART转发给BLE主节点
 * 
 */
static void CanToUart_Forward(void)
{
    int16_t ret = 0U;
    CanHalMsg_t canMsg;
    uint8_t buffer[128] = {0};
    uint32_t bufferLen = 0;
    uint8_t out[128] = {0};
    uint16_t outLen = 0;

    while (1) {
        memset(&canMsg, 0, sizeof(CanHalMsg_t));
        ret = CanHalReceive(g_DigitalKeyCtx.canHandler, &canMsg, 0);
        if (ret == 0) {
            // CAN ID
            buffer[bufferLen++] = (uint8_t)(canMsg.canId);
            buffer[bufferLen++] = (uint8_t)(canMsg.canId >> 8);
            buffer[bufferLen++] = (uint8_t)(canMsg.canId >> 16);
            buffer[bufferLen++] = (uint8_t)(canMsg.canId >> 24);
            // length
            buffer[bufferLen++] = canMsg.dlc;
            // data
            memcpy(buffer + bufferLen, canMsg.canData, canMsg.dlc);
            bufferLen += canMsg.dlc;
            outLen = FramePack(0, buffer, bufferLen, out, sizeof(out));
            // TBOX_BUF_PRINT("[Ble]", out, outLen);
            BleHalTransmit(out, outLen);
        } else {
            break;
        }
    }
}

static CanBleData_t *ParseSingleCanFrame(uint8_t* data_buf, uint16_t data_len)
{
    // 1. 检查长度是否足够（至少 4字节ID + 1字节DLC = 5字节）
    if (data_len < 5) {
        return NULL;
    }

    CanBleData_t *can = (CanBleData_t *)data_buf;

    // 3. 检查数据是否完整（防止残缺包）
    if ((5 + can->dlc) > data_len) {
        return NULL;
    }

    // 4. 校验合法 → 返回CAN帧指针
    return can;
}

static void UartToCan_Forward(void)
{
    BleUartHandleTypeDef *huart = NULL;
    static uint8_t calc_buf[BLE_UART_FRAME_DATA_SIZE + 6];
    static uint8_t batch_buf[64];
    static uint16_t calc_len = 0;   // CRC数据长度
    static uint16_t recv_cnt = 0;   // 数据接收计数器

    huart = BleHal_GetInstance();
    if (!huart) {
        return;
    }

    if (huart->init_success != 1) {
        // TBOX_PRINT(" Ble hal uart init error,does not support protocal process\r\n");   //1ms print maybe reultl over loading
    } else {
        uint16_t batch_len = RingBuffer_ReadBatch(&huart->uart_rx_rb, batch_buf, 64);
        for (uint16_t i = 0; i < batch_len; i++) {
            uint8_t recv_data = batch_buf[i];
            switch (huart->proto_state) {
                case PARSE_STATE_HEADER:
                    calc_len = 0;
                    // memset(calc_buf, 0, sizeof(calc_buf));
                    // Wait for 1st header byte (0x3C)
                    if (recv_data == BLE_UART_FRAME_HEADER) {
                        calc_buf[calc_len++] = recv_data;
                        huart->proto_state = PARSE_STATE_DATALEN_LOW;
                    }
                    break;
    
                case PARSE_STATE_DATALEN_LOW:
                    // Wait for data length low byte
                    calc_buf[calc_len++] = recv_data;
                    huart->frame.data_len = recv_data;  // Low Byte
                    huart->proto_state = PARSE_STATE_DATALEN_HIGH;
                    break;
    
                case PARSE_STATE_DATALEN_HIGH:
                    // Wait for data length high byte
                    calc_buf[calc_len++] = recv_data;
                    huart->frame.data_len |= (uint16_t)(recv_data << 8);  // High Byte
                    // check length valid
                    if (huart->frame.data_len > BLE_UART_FRAME_DATA_SIZE) {
                        // parse_reset();  // 长度非法，重置
                        huart->proto_state = PARSE_STATE_HEADER;
                        recv_cnt = 0;
                        calc_len = 0;
                        memset(&huart->frame, 0, sizeof(BleUartFrame_t));
                        return;
                    }
                    huart->proto_state = PARSE_STATE_CHANNEL;
                    break;
                
                case PARSE_STATE_CHANNEL:
                    calc_buf[calc_len++] = recv_data;
                    huart->frame.channel = recv_data;
                    recv_cnt = 0;
                    if (huart->frame.data_len == 0)
                        huart->proto_state = PARSE_STATE_TAILER;
                    else
                        huart->proto_state = PARSE_STATE_DATA;
                    break;
    
                case PARSE_STATE_DATA:
                    calc_buf[calc_len++] = recv_data;
                    huart->frame.data[recv_cnt++] = recv_data;
    
                    // 数据接收完成
                    if (recv_cnt >= huart->frame.data_len) {
                        huart->proto_state = PARSE_STATE_TAILER;
                    }
                    break;
                
                case PARSE_STATE_TAILER:
                    // Wait for 1st header byte (0xC3)
                    if (recv_data == BLE_UART_FRAME_TAILER) {
                        calc_buf[calc_len++] = recv_data;
                        huart->proto_state = PARSE_STATE_CRC;
                    } else {
                        huart->proto_state = PARSE_STATE_HEADER;
                        recv_cnt = 0;
                        calc_len = 0;
                        memset(&huart->frame, 0, sizeof(BleUartFrame_t));
                    }
                    break;
    
                case PARSE_STATE_CRC:
                    huart->frame.crc = recv_data;
    
                    // Cal CRC
                    huart->frame.calc_crc = cal_crc8(calc_buf, calc_len);
                    if (huart->frame.crc == huart->frame.calc_crc) {
                        huart->proto_state = PARSE_STATE_COMPLETE;
                    } else {
                        // parse_reset();
                        huart->proto_state = PARSE_STATE_HEADER;
                        recv_cnt = 0;
                        calc_len = 0;
                        memset(&huart->frame, 0, sizeof(BleUartFrame_t));
                    }
                    break;
                
                case PARSE_STATE_COMPLETE:
                    {
                        // TBOX_BUF_PRINT("ble", calc_buf, calc_len);
                        // TBOX_PRINT("CanId: 0x%x, dlc: %d\n", can->id, can->dlc);
                        // TBOX_BUF_PRINT("[Ble]", can->data, can->dlc);
                        CanBleData_t *can = ParseSingleCanFrame(huart->frame.data, huart->frame.data_len);
                        if (can != NULL) {
                            // 需要按CANFD标准帧发送，启用FD和BRS（可变波特率）
                            // fdFlag: bit0=1(FD enable), bit1=1(BRS enable) -> fdFlag=3
                            CanHalTransmitQueued(g_DigitalKeyCtx.canHandler, can->id, can->data, can->dlc, 0x03, CAN_TX_PRIO_NORMAL);
                            // TBOX_PRINT("CanId: 0x%x, dlc: %d\n", can->id, can->dlc);
                        }
                        huart->proto_state = PARSE_STATE_HEADER;
                    }
                    break;
    
                default:
                    // parse_reset();
                    huart->proto_state = PARSE_STATE_HEADER;
                    recv_cnt = 0;
                    calc_len = 0;
                    memset(&huart->frame, 0, sizeof(BleUartFrame_t));
                    break;
                }
        }
    }
}

/**
 * @brief DigitalKey Task
 * 
 * @param pvParameters 
 */
void DigitalKey_Task(void *pvParameters)
{
    uint32_t timeCount;

    DigitalKey_Init();

    timeCount = xTaskGetTickCount();

    while (1) {
        UartToCan_Forward();
        CanToUart_Forward();
        vTaskDelayUntil(&timeCount, 1);
    }
}
