/**
 * @file can_raw_frame.h
 * @brief CAN 帧在 SOC 侧的内存表示
 *
 * @details
 * 定义了 IPC 接收回调与 BLF 写入线程之间传递的单帧结构体
 * CanFrameRecord_t，以及帧标志位掩码。结构体大小固定 76 字节，
 * 其中数据区为定长 64 字节，便于在队列中按值拷贝。
 *
 * @note IPC 线上协议可能是变长（16B 头 + dlc 字节数据），由接收回调
 * 解析后归一化为本结构体；内存中的 canId / timestampMs / timestampUs
 * 保持大端序，由写线程做 be32toh 转换后再喂给 BlfWriter。
 */
#ifndef _CAN_RAW_FRAME_H_
#define _CAN_RAW_FRAME_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief CAN-FD 最大数据字节数 */
#define CAN_FRAME_MAX_DATA_SIZE (64u)

/**
 * @brief 单个 CAN 帧的内存表示（76 字节）
 *
 * @details 字段顺序与对齐与 IPC 线上协议定长头一致；canData 固定 64B。
 */
typedef struct
{
    uint8_t  channel;      ///< CAN 通道号（MCU 透传，BLF 中原样写出）
    uint8_t  flags;        ///< 帧标志位，见 CAN_FLAG_* 宏
    uint8_t  dlc;          ///< 实际有效字节数（0~64）
    uint8_t  reserved;     ///< 预留对齐字节，必须为 0
    uint32_t canId;        ///< 仲裁 ID（大端），bit31=扩展帧标识
    uint32_t timestampMs;  ///< MCU timestamp milliseconds (big-endian)
    uint32_t timestampUs;  ///< MCU sub-millisecond microseconds (big-endian)
    uint8_t  canData[CAN_FRAME_MAX_DATA_SIZE]; ///< 数据字段，dlc 决定有效长度
} CanFrameRecord_t;

/** @brief flags bit0：扩展帧（29bit ID） */
#define CAN_FLAG_EXT_ID (0x01u)

/** @brief flags bit1：CAN-FD 帧 */
#define CAN_FLAG_FD     (0x02u)

/** @brief flags bit2：CAN-FD 比特率切换（BRS） */
#define CAN_FLAG_BRS    (0x04u)

/** @brief flags bit3：TX frame direction */
#define CAN_FLAG_TX     (0x08u)

#ifdef __cplusplus
}
#endif

#endif
