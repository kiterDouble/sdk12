/**
 * @file blf_format.h
 * @brief BLF (Vector Binary Log Format) 文件磁盘格式定义
 *
 * @details
 * 本头文件定义 canLogger 写入 BLF 文件所使用的子集：
 * - 文件级常量（魔数、头部大小、压缩方式）
 * - 对象类型枚举（LOG_CONTAINER / CAN_MESSAGE2 / CAN_FD_MESSAGE64 等）
 * - 文件头与各对象负载的二进制结构体
 *
 * 所有字段为 **小端序**（除非另有说明），结构体按磁盘布局紧凑排列，
 * 不可随意增删字段。BLF 完整规范参见 Vector 官方文档。
 */
#ifndef _BLF_FORMAT_H_
#define _BLF_FORMAT_H_

#include <cstdint>

namespace blf {

/** @brief BLF 文件魔数 "LOGG"（小端四字节） */
static constexpr uint32_t FILE_SIGNATURE  = 0x47474F4Cu;

/** @brief BLF 对象魔数 "LOBJ"（小端四字节，每个对象开头） */
static constexpr uint32_t OBJ_SIGNATURE   = 0x4A424F4Cu;

/** @brief 容器对象（LOG_CONTAINER）使用的最小对象头大小，单位字节 */
static constexpr uint16_t OBJ_HEADER_BASE_SIZE = 16u;

/** @brief 普通对象（CAN 帧等）使用的完整对象头大小，含 8 字节时间戳 */
static constexpr uint16_t OBJ_HEADER_SIZE = 32u;

/** @brief 对象头版本号 */
static constexpr uint16_t OBJ_HEADER_VERSION = 1u;

/** @brief BLF 文件头总长度（固定 144 字节） */
static constexpr size_t   FILE_HEADER_SIZE = 144u;

/**
 * @brief 单个 LOG_CONTAINER 压缩前最大字节数
 *
 * @details
 * canLogger 把序列化后的 CAN 帧对象累积到 pending_ 缓冲中，
 * 累计达到此阈值（128 KB）即触发一次 zlib 压缩 + 落盘。
 */
static constexpr size_t   CONTAINER_MAX_UNCOMPRESSED = 128u * 1024u;

/** @brief 容器使用 zlib 压缩 */
static constexpr uint16_t COMPRESSION_ZLIB = 2u;

/** @brief 容器不压缩（canLogger 未使用） */
static constexpr uint16_t COMPRESSION_NONE = 0u;

/** @brief CAN message object flag bit0: transmitted frame direction */
static constexpr uint8_t CAN_MESSAGE_FLAG_TX = 0x01u;

/**
 * @brief BLF 对象类型枚举
 *
 * @details canLogger 实际仅写出 OBJ_LOG_CONTAINER 与 OBJ_CAN_MESSAGE2 /
 * OBJ_CAN_FD_MESSAGE64 三类，其余保留以便读取兼容。
 */
enum ObjectType : uint32_t {
    OBJ_CAN_MESSAGE      = 1u,    ///< 经典 CAN 帧（旧版，canLogger 不写出）
    OBJ_LOG_CONTAINER    = 10u,   ///< 压缩容器，包裹一批 CAN 帧对象
    OBJ_CAN_MESSAGE2     = 86u,   ///< 经典 CAN 帧（dlc ≤ 8）
    OBJ_CAN_FD_MESSAGE   = 100u,  ///< CAN-FD 帧（变长数据，canLogger 不写出）
    OBJ_CAN_FD_MESSAGE64 = 101u,  ///< CAN-FD 帧（固定 64 字节数据区，canLogger 使用）
};

/**
 * @brief BLF 文件头（前 144 字节）
 *
 * @details
 * 创建文件时占位写入，关闭时回写 fileSize / uncompressedSize /
 * objectCount / stopTime 等真实统计字段。startTime / stopTime 为
 * 8 个 uint16 元素，依次为 year, month, dayOfWeek, day, hour, min, sec, ms。
 */
struct FileHeader {
    uint32_t signature;          ///< 文件魔数 FILE_SIGNATURE
    uint32_t headerSize;         ///< 头部长度，固定 144
    uint8_t  appId;              ///< 应用标识
    uint8_t  appMajor;           ///< 应用主版本号
    uint8_t  appMinor;           ///< 应用次版本号
    uint8_t  appBuild;           ///< 应用构建号
    uint8_t  binLogMajor;        ///< BLF 格式主版本（canLogger 写 2）
    uint8_t  binLogMinor;        ///< BLF 格式次版本（canLogger 写 9）
    uint8_t  binLogBuild;        ///< BLF 格式构建号
    uint8_t  binLogPatch;        ///< BLF 格式补丁号
    uint64_t fileSize;           ///< 关闭时回写的文件总字节数
    uint64_t uncompressedSize;   ///< 累计未压缩字节数（用于校验）
    uint32_t objectCount;        ///< 累计对象数
    uint32_t objectsRead;        ///< 关闭时与 objectCount 一致
    uint16_t startTime[8];       ///< 起始时间（年月日时分秒）
    uint16_t stopTime[8];        ///< 结束时间（年月日时分秒）
};

/**
 * @brief 经典 CAN 帧负载（OBJ_CAN_MESSAGE2，16 字节）
 */
struct CanMessage2 {
    uint16_t channel;            ///< CAN 通道号（从 MCU 透传）
    uint8_t  flags;              ///< 对象标志位（canLogger 当前固定为 0）
    uint8_t  dlc;                ///< 数据长度码（经典帧 0~8）
    uint32_t arbitrationId;      ///< 仲裁 ID，bit31=1 表示扩展帧
    uint8_t  data[8];            ///< 数据字段，不足 8 字节补 0
};

/**
 * @brief CAN-FD 帧负载（OBJ_CAN_FD_MESSAGE64，72 字节）
 *
 * @details 数据区固定 64 字节，不足补 0；validDataBytes 记录真实字节数。
 */
struct CanFdMessage {
    uint16_t channel;            ///< CAN 通道号
    uint8_t  flags;              ///< 对象标志位
    uint8_t  dlc;                ///< 编码后的 DLC（len2dlc 转换，9~15）
    uint32_t arbitrationId;      ///< 仲裁 ID，bit31=1 表示扩展帧
    uint32_t frameLength;        ///< 帧长度（canLogger 当前固定为 0）
    uint8_t  bitCount;           ///< 总线位计数（canLogger 当前固定为 0）
    uint8_t  fdFlags;            ///< FD 标志：bit0=FDF, bit1=BRS
    uint8_t  validDataBytes;     ///< 实际有效数据字节数（0~64）
    uint8_t  reserved[5];        ///< 预留对齐字段
    uint8_t  data[64];           ///< 数据字段，不足 64 字节补 0
};

}

#endif
