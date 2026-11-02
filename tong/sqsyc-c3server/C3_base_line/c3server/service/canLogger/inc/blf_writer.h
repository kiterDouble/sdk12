/**
 * @file blf_writer.h
 * @brief BLF 文件 Writer 类，负责序列化、压缩、文件轮转
 *
 * @details
 * BlfWriter 把 CanFrameRecord_t 序列化为 BLF 对象，累积到内部
 * pending_ 缓冲；达到 128 KB 或显式 flush 时用 zlib 压缩为一个
 * LOG_CONTAINER 落盘。单个 .blf 文件达到 maxFileBytes_ 时轮转：
 * 关闭当前文件、自增 session 序号、按字典序删除最旧文件、新建文件。
 *
 * 文件名格式：`<prefix><YYYYMMDD_HHMMSS>_<NNNN>.blf`，时间戳在每次
 * open() 时确定，NNNN 为本次 session 内的递增序号；同秒重启时若文件已
 * 存在则自动递增序号兜底。
 */
#ifndef _BLF_WRITER_H_
#define _BLF_WRITER_H_

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

#include "can_raw_frame.h"

namespace blf_timestamp {

const uint64_t MCU_RESET_BACKWARD_THRESHOLD_US = 1000000ULL;
const uint64_t MCU_BOOT_WINDOW_US = 5000000ULL;
const uint64_t MCU_LARGE_BACKWARD_US = 60000000ULL;

inline uint64_t frameTickUs(const CanFrameRecord_t &frame)
{
    const uint64_t tickUs = (uint64_t)frame.timestampMs * 1000ULL +
                            (uint64_t)frame.timestampUs;
    return tickUs;
}

inline uint64_t relativeNs(uint64_t tickUs, uint64_t firstTickUs)
{
    return (uint64_t)(tickUs - firstTickUs) * 1000ULL;
}

inline bool isMcuTimestampReset(uint64_t tickUs, uint64_t lastTickUs)
{
    /* A real MCU restart begins in the boot window. A frame that is only a
     * few seconds late is transport reordering, not a new time domain. */
    if (lastTickUs <= tickUs) {
        return false;
    }
    const uint64_t backwardUs = lastTickUs - tickUs;
    return (backwardUs > MCU_LARGE_BACKWARD_US) ||
           ((tickUs <= MCU_BOOT_WINDOW_US) &&
            (backwardUs > MCU_RESET_BACKWARD_THRESHOLD_US));
}

}

/**
 * @class BlfWriter
 * @brief 单文件 Writer（不可拷贝，单实例对应一组 .blf 文件）
 */
class BlfWriter
{
public:
    BlfWriter();
    ~BlfWriter();

    BlfWriter(const BlfWriter &) = delete;
    BlfWriter &operator=(const BlfWriter &) = delete;

    /**
     * @brief 打开输出目录并创建首个 BLF 文件
     * @param[in] dir    输出目录，不存在会自动 mkdir 0755
     * @param[in] prefix 文件名前缀，最终格式 prefix+时间戳+序号.blf
     * @return 0 成功，-1 失败（参数空 / 目录不可写 / 创建文件失败）
     *
     * @details 调用时生成 session 时间戳：系统时间有效则取当前时间，
     *          RTC 未校时则扫描目录取最大时间戳 + 1s，全失败回退到 1970。
     */
    int  open(const std::string &dir, const std::string &prefix);

    /**
     * @brief 关闭 Writer，刷盘并回写当前文件 FILE_HEADER
     *
     * @note 析构函数会自动调用，单实例生命周期内可重复 open/close。
     */
    void close();

    /**
     * @brief 追加一帧到 pending_ 缓冲
     * @param[in] frame 已做过字节序转换的 CAN 帧
     * @return 0 成功，-1 Writer 未 open；通道未命中 channelMask_ 时返回 0 但不写
     *
     * @details 帧会被序列化为 OBJ_CAN_MESSAGE2 或 OBJ_CAN_FD_MESSAGE64，
     *          追加到 pending_；累计达 128 KB 时触发 flushContainer，
     *          若文件大小超 maxFileBytes_ 进一步触发 rotateFile。
     */
    int  appendFrame(const CanFrameRecord_t &frame);

    /**
     * @brief 强制把 pending_ 中残留对象 flush 到磁盘
     *
     * @details 由写线程在 popBatch 超时或退出前调用，避免数据滞留。
     */
    void flush();

    /** @brief 设置单文件大小上限，单位 MB（open 之后调用生效） */
    void setMaxFileSizeMB(uint32_t mb);

    /** @brief 设置目录最大保留文件数（小于 1 会被钳制为 1） */
    void setMaxFileCount(uint32_t count);

    /** @brief 设置目录内 BLF 总大小上限；0 表示不限制总容量 */
    void setMaxTotalSizeMB(uint32_t mb);

    /** @brief 设置通道过滤掩码，bit n=1 表示录通道 n（仅 channel<32 有效） */
    void setChannelMask(uint32_t mask);

    /**
     * @brief 运行期统计快照
     */
    struct Stats
    {
        uint64_t      framesAppended;     ///< 累计 appendFrame 成功次数
        uint64_t      containersFlushed;  ///< 累计 flush 出的 LOG_CONTAINER 数
        uint64_t      bytesWritten;       ///< 当前文件累计写入字节数
        uint32_t      currentFileIndex;   ///< 当前 session 内序号（与文件名 NNNN 一致）
        uint64_t      currentFileBytes;   ///< 当前文件当前字节数
        uint32_t      pendingObjects;     ///< pending_ 缓冲中尚未 flush 的对象数
        std::string   currentFilePath;    ///< 当前文件绝对路径
    };

    /**
     * @brief 获取统计快照（线程不安全，仅在写线程或主线程读）
     * @return 当前统计的值拷贝
     */
    Stats getStats() const;

private:
    struct FileState
    {
        FILE         *fp;                  ///< libc 文件指针
        uint64_t      fileBytes;           ///< 当前文件累计字节
        uint64_t      uncompressedTotal;   ///< 累计未压缩字节数（写回 FILE_HEADER）
        uint32_t      objectCount;         ///< 累计对象数（写回 FILE_HEADER）
        uint64_t      firstFrameTickUs;    ///< First MCU timestamp in microseconds.
        uint64_t      lastFrameTickUs;     ///< Last MCU timestamp in microseconds.
        uint64_t      lastEmittedNs;       ///< Last non-decreasing BLF timestamp.
        bool          firstFrameSeen;      ///< 是否已收到首帧
        uint16_t      startTime[8];        ///< 起始时间（年月日时分秒，写回 FILE_HEADER）
        uint16_t      stopTime[8];         ///< 结束时间
        std::string   path;                ///< 当前文件绝对路径
    };

    /** @brief 新建一个 BLF 文件并写 FILE_HEADER，文件名使用本文件创建时间 */
    int  openNewFile();

    /** @brief flush + 回写 FILE_HEADER + fclose，不动 sessionSeq_ */
    void closeCurrentFile();

    /**
     * @brief 关闭当前文件 + 自增 sessionSeq_ + 删除最旧文件 + 新建下一个
     * @return 0 成功，-1 新建失败
     */
    int  rotateFile();

    /**
     * @brief 生成本次 session 的时间戳串
     *
     * @details 优先用系统时间；RTC 看起来无效（year < 2020）时：
     *          - 扫描目录取最大时间戳 +1s
     *          - 目录无历史文件时回退 "19700101_000000"
     *
     * @return 形如 "YYYYMMDD_HHMMSS" 的 15 字符串
     */
    std::string generateSessionTimestamp();

    /**
     * @brief 扫描目录取最大时间戳与对应序号
     * @param[out] outMaxSeq 输出最大时间戳对应的序号字符串
     * @return 最大时间戳串；目录为空或打开失败时返回空串
     */
    std::string scanMaxTimestampInDir(std::string &outMaxSeq) const;

    /**
     * @brief 从 "<ts>_<seq>.blf" 后缀中解析时间戳与序号
     * @param[in]  nameAfterPrefix 去掉 prefix 之后的文件名
     * @param[out] ts   输出时间戳串（YYYYMMDD_HHMMSS，15 字符）
     * @param[out] seq  输出序号串（纯数字）
     * @return true 解析成功，false 文件名不符合预期格式
     */
    static bool parseTimestampPart(const std::string &nameAfterPrefix,
                                   std::string &ts, std::string &seq);

    /**
     * @brief 按字典序保留最新 maxFileCount_-1 个文件，删除其余
     *
     * @note 时间戳格式保证字典序与时间顺序一致；调用时机为新建文件前。
     */
    void enforceMaxFileCount();

    /** @brief 写入前按实际 BLF 文件大小清理最旧文件，保证不超过总容量 */
    bool enforceMaxTotalSize(uint64_t incomingBytes);

    /**
     * @brief 把一帧序列化为 BLF 对象字节流，追加到 out
     *
     * @details 根据 dlc/flags 选择 OBJ_CAN_MESSAGE2 (≤8B) 或
     *          OBJ_CAN_FD_MESSAGE64 (>8B 或 CAN_FLAG_FD 置位)，
     *          时间戳通过 frameToBlfTimestamp 换算。
     */
    void serializeFrame(const CanFrameRecord_t &frame, std::vector<uint8_t> &out);

    /**
     * @brief 把 pending_ 当前内容 zlib 压缩为一个 LOG_CONTAINER 写入磁盘
     *
     * @details 失败时清空 pending_ 并打 Error 日志，不抛异常；
     *          成功后清空 pending_、更新 fileBytes/objectCount/stats_。
     */
    void flushContainer();

    /** @brief 创建文件时写初始 FILE_HEADER（多数字段填占位 0） */
    void writeFileHeader();

    /** @brief 关闭文件前回写 fileSize/objectCount/stopTime 等真实字段 */
    void updateFileHeaderOnClose();

    /**
     * @brief 抓取当前系统时间到 8 个 u16 数组
     *
     * @details 输出格式：[year, month, dayOfWeek, day, hour, min, sec, 0]，
     *          用于 FILE_HEADER.startTime / stopTime。
     */
    void captureSystemTime(uint16_t out[8]);

    /**
     * @brief Convert an MCU timestamp to a BLF relative nanosecond timestamp.
     */
    uint64_t frameToBlfTimestamp(const CanFrameRecord_t &frame);

    static const uint32_t DEFAULT_MAX_FILE_MB   = 100u;          ///< 默认单文件 100 MB
    static const uint32_t DEFAULT_MAX_FILE_CNT  = 10u;           ///< 默认保留 10 个文件
    static const uint32_t DEFAULT_MAX_TOTAL_MB  = 5120u;         ///< 默认 BLF 总容量 5 GiB
    static const uint32_t DEFAULT_CHANNEL_MASK  = 0xFFFFFFFFu;   ///< 默认全通道录制

    std::string         dir_;                ///< 输出目录（确保以 '/' 结尾）
    std::string         prefix_;             ///< 文件名前缀
    uint32_t            maxFileBytes_;       ///< 单文件字节上限
    uint32_t            maxFileCount_;       ///< 最大保留文件数
    uint64_t            maxTotalBytes_;       ///< BLF 总容量上限，0 表示不限制
    uint32_t            channelMask_;        ///< 通道过滤掩码
    uint32_t            fileIndex_;          ///< （已废弃，保留字段）

    std::string         sessionTs_;          ///< 当前文件创建时间串（YYYYMMDD_HHMMSS）
    uint32_t            sessionSeq_;         ///< 本次运行内全局递增序号（文件名 NNNN）

    FileState           f_;                  ///< 当前文件状态
    std::vector<uint8_t> pending_;           ///< 待压缩的对象字节流缓冲（预分配 64 KB）
    uint64_t            pendingObjectCount_; ///< pending_ 中的对象数
    Stats               stats_;              ///< 对外可观测统计
};

#endif
