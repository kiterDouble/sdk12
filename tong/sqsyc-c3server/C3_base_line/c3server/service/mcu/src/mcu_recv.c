#include <string.h>

#include "crcTool.h"
#include "err_code.h"
#include "sr_log.h"
#include "mcu_bus.h"
#include "mcu_recv.h"
#include "mcu_can_filter.h"


#define MCU_FRAME_HEADER_LEN (8U)
#define MCU_FRAME_CRC_LEN (2U)
#define MCU_FRAME_PAYLOAD_OFF (MCU_FRAME_HEADER_LEN)
#define MCU_RECV_CRC_ERR (-2)
#define MCU_RECOVER_END (1)
#define MCU_RECOVER_RETRY (2)

typedef enum
{
    MCU_RECV_CHANNEL_UART = 0,
    MCU_RECV_CHANNEL_SPI
} McuRecvChannel_e;

/* 前向声明：McuSrvFindFrameHeader 定义在本文件后部 */
static uint32_t McuSrvFindFrameHeader(const uint8_t *data, uint32_t dataLen, uint32_t offset);

/**
 * @name McuSrvRecvFillCache
 * @brief 从总线批量读取数据到接收缓存
 * @param srvData 指向McuSrvData_t结构体的指针
 * @return 成功返回读取字节数，失败返回-1
 */
static int32_t McuSrvRecvFillCache(McuSrvData_t *srvData)
{
    int32_t readn = 0;

    if (NULL == srvData)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return -1;
    }

    if (srvData->recvLen >= MCU_RECV_BUFFER_LEN)
    {
        LOG_Error("%s recv buffer full, discard all.", __FUNCTION__);
        srvData->recvLen = 0U;
    }

    readn = McuSrvBusRecv(srvData, &srvData->recvBuf[srvData->recvLen],
                          MCU_RECV_BUFFER_LEN - srvData->recvLen);
    if (readn <= 0)
    {
        LOG_Error("%s bus recv error or eof, readn=%d.", __FUNCTION__, readn);
        LOG_Debug("%s deinit bus.", __FUNCTION__);
        (void)McuSrvBusDeinit(srvData);
        srvData->dataBusFd = -1;
        srvData->recvLen = 0U;
        return -1;
    }

    srvData->recvLen += (uint32_t)readn;
    return readn;
}

/**
 * @name McuSrvRecvData
 * @brief 接收MCU服务数据（缓存式解析）
 * @param srvData 服务数据结构体指针
 * @param data 接收的数据缓冲区指针
 * @param dataLen 数据长度指针，输入参数为期望接收的数据长度，输出参数为实际接收的数据长度
 * @param channel 报文来源通道（UART/SPI），用于区分日志策略
 * @return int32_t 返回操作结果，RET_OK表示成功，RET_ERR表示失败，MCU_RECV_CRC_ERR表示CRC校验失败
 */
static int32_t McuSrvRecvData(McuSrvData_t *srvData, uint8_t *data,
                              uint32_t *dataLen, McuRecvChannel_e channel)
{
    uint32_t frameLen = 0U;
    uint16_t srvLen = 0U;
    uint16_t crc16Val = 0U;
    uint32_t headerPos = 0U;
    uint32_t skipLen = 0U;
    uint32_t frameType = 0U;

    if ((NULL == srvData) || (NULL == data) || (NULL == dataLen)
        || (*dataLen == 0U)
        || (*dataLen < (MCU_FRAME_HEADER_LEN + MCU_FRAME_CRC_LEN)))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return RET_ERR;
    }

    while (1)
    {
        /* 1. 在缓存中查找 0x55 0xAA 帧头 */
        headerPos = (srvData->recvLen > 0U)
                    ? McuSrvFindFrameHeader(srvData->recvBuf, srvData->recvLen, 0U)
                    : 0U;

        /* 2. 没有完整帧头，补充数据或保留末尾不完整的 0x55 */
        if (headerPos + 1U >= srvData->recvLen)
        {
            if ((headerPos < srvData->recvLen)
                && (srvData->recvBuf[headerPos] == 0x55U))
            {
                srvData->recvBuf[0U] = 0x55U;
                srvData->recvLen = 1U;
            }
            else
            {
                srvData->recvLen = 0U;
            }

            if (McuSrvRecvFillCache(srvData) < 0)
            {
                return RET_ERR;
            }
            continue;
        }

        /* 3. 丢弃帧头前的噪声字节 */
        if (headerPos > 0U)
        {
            (void)memmove(srvData->recvBuf, &srvData->recvBuf[headerPos],
                          srvData->recvLen - headerPos);
            srvData->recvLen -= headerPos;
        }

        /* 4. 至少需要完整帧头 */
        if (srvData->recvLen < MCU_FRAME_HEADER_LEN)
        {
            if (McuSrvRecvFillCache(srvData) < 0)
            {
                return RET_ERR;
            }
            continue;
        }

        /* 5. 解析 payload 长度和完整帧长 */
        srvLen = ((uint16_t)srvData->recvBuf[5U] << 8U) + srvData->recvBuf[6U];
        frameLen = MCU_FRAME_HEADER_LEN + (uint32_t)srvLen + MCU_FRAME_CRC_LEN;

        /* 6. 帧长异常则按坏包丢弃当前 0x55，继续寻找下一帧 */
        if ((frameLen > *dataLen) || (frameLen > MCU_RECV_BUFFER_LEN))
        {
            LOG_Error("%s invalid frameLen=%u, skip header.", __FUNCTION__,
                      (unsigned int)frameLen);
            skipLen = 1U;
        }
        /* 7. 缓存中已有完整帧，提取并校验 */
        else if (srvData->recvLen >= frameLen)
        {
            (void)memcpy(data, srvData->recvBuf, frameLen);
            (void)memmove(srvData->recvBuf, &srvData->recvBuf[frameLen],
                          srvData->recvLen - frameLen);
            srvData->recvLen -= frameLen;
            *dataLen = frameLen;

            crc16Val = ((uint16_t)data[frameLen - 2U] << 8U) + data[frameLen - 1U];
            if (crc16Val != Crc16((void*)data,
                                  (uint32_t)srvLen + MCU_FRAME_HEADER_LEN,
                                  Crc16Ccitt))
            {
                if (channel != MCU_RECV_CHANNEL_SPI)
                {
                    LOG_Error("%s crc check fail.", __FUNCTION__);
                    if (srvLen < 128U)
                    {
                        LOG_DebugHex(data, frameLen, "McuSrvBusRecv crc check fail");
                    }
                }
                return MCU_RECV_CRC_ERR;
            }

            /* CAN数据类报文数据量大，不打印 */
            frameType = (((uint32_t)data[2] & 0xffU) << 16U)
                        | (((uint32_t)data[3] & 0xffU) << 8U)
                        | ((uint32_t)data[4] & 0x7fU);
            if ((frameType != (uint32_t)SYS_QUE_TYPE_MCU_VEHICLE_CAN_RAW)
                && (frameType != (uint32_t)SYS_QUE_TYPE_MCU_VEHICLE_CAN_DATA))
            {
                LOG_DebugHex(data, frameLen, "McuSrvBusRecv");
            }
            return RET_OK;
        }
        /* 8. 帧未收完，继续补充数据 */
        else
        {
            if (McuSrvRecvFillCache(srvData) < 0)
            {
                return RET_ERR;
            }
            continue;
        }

        /* 9. 按 skipLen 丢弃已处理/坏包字节 */
        if (skipLen > 0U)
        {
            if (skipLen < srvData->recvLen)
            {
                (void)memmove(srvData->recvBuf, &srvData->recvBuf[skipLen],
                              srvData->recvLen - skipLen);
                srvData->recvLen -= skipLen;
            }
            else
            {
                srvData->recvLen = 0U;
            }
            skipLen = 0U;
        }
    }
}

/**
 * @name McuSrvDispatchFrame
 * @brief 根据来源通道处理已通过CRC校验的MCU报文
 * @param data 完整MCU报文的首地址
 * @param channel 报文来源通道
 * @return RET_OK表示报文已处理，RET_ERR表示报文不属于当前通道并已丢弃
 */
static int32_t McuSrvDispatchFrame(const uint8_t *data, McuRecvChannel_e channel)
{
    uint32_t aid = data[2];
    uint32_t mid = data[3];
    uint32_t sub = (uint32_t)data[4] & 0x7fU;
    uint16_t srvLen = ((uint16_t)data[5] << 8U) + data[6];

    if ((aid == 0U) && (mid == 0U))
    {
        if (channel != MCU_RECV_CHANNEL_SPI)
        {
            LOG_Error("%s UART received SPI log frame, discard it.", __FUNCTION__);
            return RET_ERR;
        }
        uint32_t printLen = (uint32_t)srvLen;
        LOG_Debug("[MCU_DEBUG]:%.*s", (int)printLen, (const char *)&data[MCU_FRAME_PAYLOAD_OFF]);
        return RET_OK;
    }

    uint32_t u32type = ((aid & 0xffU) << 16U) + ((mid & 0xffU) << 8U) + sub;
    SYSQueMsgType_e type = (SYSQueMsgType_e)u32type;

    if ((type == SYS_QUE_TYPE_MCU_VEHICLE_CAN_RAW) && (srvLen >= 2U))
    {
        /* 过滤表就绪后，按表过滤并重组广播到 CAN_DATA 供 signalServer 使用 */
        McuCanFilterForwardRawToData(&data[MCU_FRAME_HEADER_LEN], srvLen);
    }

    (void)SendMsgToAllQueue(type, &data[MCU_FRAME_HEADER_LEN], srvLen);
    return RET_OK;
}

/**
 * @name McuSrvFindFrameHeader
 * @brief 从指定偏移位置开始查找下一个0x55 0xAA报文头
 * @param data 待查找的数据缓冲区
 * @param dataLen 数据缓冲区的有效长度
 * @param offset 开始查找的偏移位置
 * @return 找到报文头时返回报文头偏移，否则返回缓冲区末尾偏移
 */
static uint32_t McuSrvFindFrameHeader(const uint8_t *data, uint32_t dataLen, uint32_t offset)
{
    while ((offset + 1U < dataLen) && ((data[offset] != 0x55U) || (data[offset + 1U] != 0xAAU))) {
        offset++;
    }
    return offset;
}

/**
 * @name McuSrvPrepareCandidate
 * @brief 从当前缓存中查找一个完整候选报文，不根据坏包内容继续读取总线
 * @param data 接收数据缓冲区
 * @param dataLen 当前缓存有效数据长度
 * @param capacity 接收数据缓冲区容量
 * @param offset 输入开始查找的偏移，输出下次查找的偏移
 * @param candidate 输出候选报文的起始偏移
 * @param srvLen 输出候选报文的服务数据长度
 * @param frameLen 输出候选报文的完整帧长度
 * @return RET_OK表示候选报文准备完成，MCU_RECOVER_END表示查找结束，
 *         MCU_RECOVER_RETRY表示需要重新查找
 */
static int32_t McuSrvPrepareCandidate(const uint8_t *data, uint32_t dataLen,
                                      uint32_t capacity, uint32_t *offset, uint32_t *candidate,
                                      uint16_t *srvLen, uint32_t *frameLen)
{
    *candidate = McuSrvFindFrameHeader(data, dataLen, *offset);
    if (*candidate + 1U >= dataLen) {
        return MCU_RECOVER_END;
    }

    if (dataLen - *candidate < MCU_FRAME_HEADER_LEN) {
        LOG_Warn("%s discard incomplete candidate header at offset=%u.",
                 __FUNCTION__, (unsigned int)*candidate);
        return MCU_RECOVER_END;
    }

    *srvLen = ((uint16_t)data[*candidate + 5U] << 8U) + data[*candidate + 6U];
    *frameLen = MCU_FRAME_HEADER_LEN + (uint32_t)*srvLen + MCU_FRAME_CRC_LEN;
    if (*frameLen > capacity) {
        *offset = *candidate + 1U;
        return MCU_RECOVER_RETRY;
    }

    if (dataLen - *candidate < *frameLen) {
        uint32_t next = McuSrvFindFrameHeader(data, dataLen, *candidate + 2U);
        if (next + 1U < dataLen) {
            *offset = next;
            return MCU_RECOVER_RETRY;
        }
        LOG_Warn("%s discard incomplete candidate at offset=%u, need=%u, remain=%u.",
                 __FUNCTION__, (unsigned int)*candidate, (unsigned int)*frameLen,
                 (unsigned int)(dataLen - *candidate));
        return MCU_RECOVER_END;
    }
    return RET_OK;
}

/**
 * @name McuSrvFrameCrcValid
 * @brief 校验完整MCU报文的CRC16
 * @param frame 完整MCU报文的首地址
 * @param frameLen 完整MCU报文长度，包含报文头、服务数据和CRC
 * @return RET_OK表示CRC校验通过，RET_ERR表示CRC校验失败
 */
static int32_t McuSrvFrameCrcValid(const uint8_t *frame, uint32_t frameLen)
{
    uint16_t recvCrc = ((uint16_t)frame[frameLen - 2U] << 8U) + frame[frameLen - 1U];
    uint16_t calcCrc = Crc16((void *)frame, frameLen - MCU_FRAME_CRC_LEN, Crc16Ccitt);
    return (recvCrc == calcCrc) ? RET_OK : RET_ERR;
}

/**
 * @name McuSrvRecoverFrames
 * @brief 从CRC校验失败的报文中重新提取、校验并分发后续完整粘包报文
 * @param data CRC校验失败报文及其粘包数据的缓冲区
 * @param dataLen 缓冲区有效长度
 * @param capacity 数据缓冲区容量
 * @param channel 报文来源通道
 * @return RET_OK表示至少恢复一个有效报文，RET_ERR表示未恢复有效报文
 */
static int32_t McuSrvRecoverFrames(const uint8_t *data, uint32_t dataLen,
                                   uint32_t capacity, McuRecvChannel_e channel)
{
    uint32_t offset = 1U;
    uint32_t recovered = 0U;

    while (offset < dataLen) {
        uint32_t candidate = 0U;
        uint32_t frameLen = 0U;
        uint16_t srvLen = 0U;
        int32_t status = McuSrvPrepareCandidate(data, dataLen, capacity, &offset,
                                                &candidate, &srvLen, &frameLen);
        if (status == MCU_RECOVER_END) {
            LOG_Warn("%s no more candidates found, recovered %u frame(s).", __FUNCTION__, (unsigned int)recovered);
            break;
        }
        if (status == MCU_RECOVER_RETRY) {
            continue;
        }
        if (McuSrvFrameCrcValid(&data[candidate], frameLen) != RET_OK) {
            LOG_Warn("%s skip invalid frame candidate at offset=%u, srvLen=%u.", __FUNCTION__,
                     (unsigned int)candidate, (unsigned int)srvLen);
            offset = candidate + 1U;
            continue;
        }

        const uint8_t *frame = &data[candidate];
        LOG_DebugHex(frame, frameLen, "McuSrvBusRecv recovered frame");
        if (McuSrvDispatchFrame(frame, channel) != RET_OK) {
            offset = candidate + frameLen;
            continue;
        }
        recovered++;
        offset = candidate + frameLen;
    }

    if (recovered == 0U) {
        LOG_Error("%s no valid embedded frame found.", __FUNCTION__);
        return RET_ERR;
    }

    LOG_Info("%s recovered %u frame(s).", __FUNCTION__, (unsigned int)recovered);
    return RET_OK;
}

/**
 * @brief Process one MCU service message received from data bus.
 *
 * Frame format (byte index):
 *   [0..1]  : 0x55 0xAA (header)
 *   [2]     : AID
 *   [3]     : MID
 *   [4]     : SUB (bit7 reserved, use sub & 0x7F)
 *   [5..6]  : LEN (payload length, big-endian)
 *   [7]     : reserved (placeholder)
 *   [8..]   : payload (LEN bytes)
 *   [end-2..end-1] : CRC (2 bytes)
 *
 * @param[in] srvData  MCU service context.
 * @return RET_OK on success, otherwise RET_ERR.
 */
static int32_t McuSrvRecvMesgProcessByChannel(McuSrvData_t *srvData,
                                              McuRecvChannel_e channel)
{
    /* ---- local constants (avoid magic numbers) ---- */
    enum
    {
        MCU_FRAME_MIN_HEAD_SIZE = 8U,  /* 55 AA + AID + MID + SUB + LEN(2) + RSV */
        MCU_FRAME_CRC_SIZE      = 2U,

        MCU_FRAME_HDR0          = 0x55U,
        MCU_FRAME_HDR1          = 0xAAU
    };

    int32_t  ret = RET_ERR;
    uint8_t  data[MCU_RECV_BUFFER_LEN] = { 0U };
    uint32_t dataLen = (uint32_t)sizeof(data);

    if (NULL == srvData)
    {
        LOG_Error("%s param invalid.\n", __FUNCTION__);
        return ret;
    }

    if (srvData->dataBusFd == -1)
    {
        LOG_Error("%s %s data bus have not inited.\n", __FUNCTION__, srvData->cfg.device);
        return ret;
    }

    ret = McuSrvRecvData(srvData, data, &dataLen, channel);
    if (ret == MCU_RECV_CRC_ERR)
    {
        if (channel == MCU_RECV_CHANNEL_SPI)
        {
            // LOG_Warn("%s discard SPI CRC/sticky frame, dataLen=%u.", __FUNCTION__, (unsigned int)dataLen);
            return RET_ERR;
        }
        return McuSrvRecoverFrames(data, dataLen, sizeof(data), channel);
    }
    if (ret != RET_OK)
    {
        LOG_Error("%s McuSrvRecvData failed.", __FUNCTION__);
        return RET_ERR;
    }

    /* Basic length check */
    if (dataLen < (uint32_t)(MCU_FRAME_MIN_HEAD_SIZE + MCU_FRAME_CRC_SIZE))
    {
        LOG_Error("%s invalid frame length=%u.", __FUNCTION__, dataLen);
        return RET_ERR;
    }

    /* Header check */
    if ((data[0] != MCU_FRAME_HDR0) || (data[1] != MCU_FRAME_HDR1))
    {
        LOG_Error("%s invalid header: %02X %02X.", __FUNCTION__, data[0], data[1]);
        return RET_ERR;
    }

    {
        const uint16_t srvLen = (uint16_t)(((uint16_t)data[5] << 8U) | (uint16_t)data[6]);
        const uint32_t neededLen = (uint32_t)MCU_FRAME_PAYLOAD_OFF +
                                   (uint32_t)srvLen + (uint32_t)MCU_FRAME_CRC_SIZE;
        if (neededLen > dataLen)
        {
            LOG_Error("%s invalid payload len: dataLen=%u srvLen=%u need=%u.",
                      __FUNCTION__, dataLen, (uint32_t)srvLen, neededLen);
            return RET_ERR;
        }
    }

    return McuSrvDispatchFrame(data, channel);
}

gpointer UartDataHandleTask(gpointer arg)
{
    McuSrvData_t *uartData = (McuSrvData_t *)arg;

    while (1)
    {

        if (uartData->dataBusFd == -1)
        {
            (void)McuSrvBusInit(uartData);
            (void)g_usleep(500000);
        }
        else
        {
            (void)McuSrvRecvMesgProcessByChannel(uartData, MCU_RECV_CHANNEL_UART);
        }
    }
}

gpointer SpiDataHandleTask(gpointer arg)
{
    McuSrvData_t *spiData = (McuSrvData_t *)arg;

    while (1)
    {
        if (spiData->dataBusFd == -1)
        {
            (void)McuSrvBusInit(spiData);
            (void)g_usleep(500000);
        }
        else
        {
            (void)McuSrvRecvMesgProcessByChannel(spiData, MCU_RECV_CHANNEL_SPI);
        }
    }
}
