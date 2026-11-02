/**
 * @file blf_writer.cpp
 * @brief BlfWriter 实现：BLF 序列化、zlib 压缩、文件轮转
 *
 * @details
 * 关键设计：
 * - pending_ 缓冲达 128 KB 触发 flushContainer，把缓冲打包为单个
 *   LOG_CONTAINER（zlib 压缩）写入磁盘；
 * - 文件大小超阈值触发 rotateFile，按字典序保留最新 maxFileCount_-1 个；
 * - 每个 BLF 文件名使用自身创建时间，sessionSeq_ 全局递增；同秒内
 *   轮转时通过序号避免冲突。
 */
#include "blf_writer.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>
#include <zlib.h>

#include "blf_format.h"
#include "sr_log.h"

namespace {

/** @brief 按小端序写入 2 字节到 p */
inline void putU16LE(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
}

/** @brief 按小端序写入 4 字节到 p */
inline void putU32LE(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8) & 0xFFu);
    p[2] = (uint8_t)((v >> 16) & 0xFFu);
    p[3] = (uint8_t)((v >> 24) & 0xFFu);
}

/** @brief 按小端序写入 8 字节到 p */
inline void putU64LE(uint8_t *p, uint64_t v)
{
    for (int i = 0; i < 8; ++i) {
        p[i] = (uint8_t)((v >> (8 * i)) & 0xFFu);
    }
}

inline uint16_t beToHost16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

inline uint32_t beToHost32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  |  (uint32_t)p[3];
}

/**
 * @brief 粗略判断系统时间是否可信（year >= 2020）
 *
 * @details 用于检测板子断电重启后 RTC 未校时的情形，触发目录扫描兜底。
 */
inline bool isSystemTimeValid()
{
    time_t now = time(NULL);
    struct tm lt;
    localtime_r(&now, &lt);
    return (lt.tm_year + 1900) >= 2020;
}

/** @brief 把 time_t 格式化为 "YYYYMMDD_HHMMSS"（本地时区） */
inline std::string formatSessionTimestamp(time_t t)
{
    struct tm lt;
    localtime_r(&t, &lt);
    char buf[32];
    snprintf(buf, sizeof(buf), "%04d%02d%02d_%02d%02d%02d",
             lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday,
             lt.tm_hour, lt.tm_min, lt.tm_sec);
    return std::string(buf);
}

/**
 * @brief 反向解析 "YYYYMMDD_HHMMSS" 为 time_t
 * @return 解析失败返回 (time_t)-1
 */
inline time_t parseSessionTimestamp(const std::string &ts)
{
    if (ts.size() != 15u || ts[8] != '_') return (time_t)-1;
    struct tm lt;
    std::memset(&lt, 0, sizeof(lt));
    char buf[16];
    std::memcpy(buf, ts.c_str(), 15);
    buf[15] = '\0';
    char *r = strptime(buf, "%Y%m%d_%H%M%S", &lt);
    if (!r || *r != '\0') return (time_t)-1;
    return mktime(&lt);
}

/**
 * @brief CAN-FD 数据字节长度转 BLF DLC 编码
 *
 * @details CAN-FD 的 dlc 字段是编码值，与字节长度映射（ISO 11898-1）：
 * 字节 0~8 → dlc 0~8；12→9；16→10；20→11；24→12；32→13；48→14；64→15。
 */
inline uint8_t len2dlc(uint8_t len)
{
    if (len <= 8u)  return len;
    if (len <= 12u) return 9u;
    if (len <= 16u) return 10u;
    if (len <= 20u) return 11u;
    if (len <= 24u) return 12u;
    if (len <= 32u) return 13u;
    if (len <= 48u) return 14u;
    return 15u;
}

inline void captureTimeT(time_t t, uint16_t out[8])
{
    struct tm lt;
    localtime_r(&t, &lt);

    out[0] = (uint16_t)(lt.tm_year + 1900);
    out[1] = (uint16_t)(lt.tm_mon + 1);
    out[2] = (uint16_t)((lt.tm_wday + 6) % 7 + 1);
    out[3] = (uint16_t)lt.tm_mday;
    out[4] = (uint16_t)lt.tm_hour;
    out[5] = (uint16_t)lt.tm_min;
    out[6] = (uint16_t)lt.tm_sec;
    out[7] = 0;
}

}

BlfWriter::BlfWriter()
    : maxFileBytes_(DEFAULT_MAX_FILE_MB * 1024u * 1024u),
      maxFileCount_(DEFAULT_MAX_FILE_CNT),
      maxTotalBytes_((uint64_t)DEFAULT_MAX_TOTAL_MB * 1024ULL * 1024ULL),
      channelMask_(DEFAULT_CHANNEL_MASK),
      fileIndex_(0),
      sessionSeq_(0),
      pendingObjectCount_(0)
{
    f_ = FileState{};
    f_.fp = NULL;
    stats_ = Stats{};
    pending_.reserve(64u * 1024u);
}

BlfWriter::~BlfWriter()
{
    close();
}

void BlfWriter::setMaxFileSizeMB(uint32_t mb)
{
    maxFileBytes_ = mb * 1024u * 1024u;
}

void BlfWriter::setMaxFileCount(uint32_t count)
{
    maxFileCount_ = count < 1u ? 1u : count;
}

void BlfWriter::setMaxTotalSizeMB(uint32_t mb)
{
    maxTotalBytes_ = (uint64_t)mb * 1024ULL * 1024ULL;
}

void BlfWriter::setChannelMask(uint32_t mask)
{
    channelMask_ = mask;
}

int BlfWriter::open(const std::string &dir, const std::string &prefix)
{
    dir_   = dir;
    prefix_ = prefix;
    fileIndex_ = 0;
    sessionSeq_ = 0;

    if (dir_.empty() || prefix_.empty()) {
        LOG_Error("BlfWriter::open invalid dir/prefix");
        return -1;
    }

    if (dir_[dir_.size() - 1] != '/') {
        dir_.push_back('/');
    }

    struct stat st;
    if (stat(dir_.c_str(), &st) != 0) {
        if (mkdir(dir_.c_str(), 0755) != 0 && errno != EEXIST) {
            LOG_Error("mkdir %s failed: %s", dir_.c_str(), strerror(errno));
            return -1;
        }
    } else if (!S_ISDIR(st.st_mode)) {
        LOG_Error("%s is not a directory", dir_.c_str());
        return -1;
    }

    sessionSeq_ = 0;

    return openNewFile();
}

int BlfWriter::openNewFile()
{
    if (!enforceMaxTotalSize(blf::FILE_HEADER_SIZE)) {
        LOG_Error("cannot reserve bytes for a new BLF file under total limit");
        return -1;
    }

    sessionTs_ = generateSessionTimestamp();
    char name[64];
    snprintf(name, sizeof(name), "%s%s_%04u.blf",
             prefix_.c_str(), sessionTs_.c_str(), sessionSeq_);

    std::string path = dir_ + name;

    struct stat st;
    while (stat(path.c_str(), &st) == 0) {
        if (sessionSeq_ >= 9999u) break;
        sessionSeq_++;
        snprintf(name, sizeof(name), "%s%s_%04u.blf",
                 prefix_.c_str(), sessionTs_.c_str(), sessionSeq_);
        path = dir_ + name;
    }

    FILE *fp = fopen(path.c_str(), "wb");
    if (!fp) {
        LOG_Error("fopen %s failed: %s", path.c_str(), strerror(errno));
        return -1;
    }

    f_ = FileState{};
    f_.fp = fp;
    f_.path = path;
    time_t startTime = parseSessionTimestamp(sessionTs_);
    if (startTime == (time_t)-1) {
        startTime = time(NULL);
    }
    captureTimeT(startTime, f_.startTime);
    f_.firstFrameSeen = false;
    f_.firstFrameTickUs = 0;

    uint8_t hdr[blf::FILE_HEADER_SIZE];
    std::memset(hdr, 0, sizeof(hdr));
    putU32LE(&hdr[0],  blf::FILE_SIGNATURE);
    putU32LE(&hdr[4],  (uint32_t)blf::FILE_HEADER_SIZE);
    hdr[8]  = 0;
    hdr[12] = 2;  hdr[13] = 9;  hdr[14] = 0;  hdr[15] = 0;
    for (size_t i = 0; i < 8; ++i) {
        putU16LE(&hdr[40 + 2 * i], f_.startTime[i]);
    }
    if (fwrite(hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        LOG_Error("write file header failed: %s", strerror(errno));
        fclose(fp);
        f_.fp = NULL;
        return -1;
    }
    fflush(fp);
    f_.fileBytes = sizeof(hdr);

    stats_.currentFileIndex = sessionSeq_;
    stats_.currentFileBytes = f_.fileBytes;
    stats_.currentFilePath  = path;

    LOG_Info("BlfWriter open %s (sessionTs=%s seq=%u, header flushed=%zu bytes)",
             path.c_str(), sessionTs_.c_str(), sessionSeq_,
             sizeof(hdr));
    return 0;
}

void BlfWriter::captureSystemTime(uint16_t out[8])
{
    captureTimeT(time(NULL), out);
}

void BlfWriter::close()
{
    if (!f_.fp) {
        return;
    }
    flushContainer();
    updateFileHeaderOnClose();
    fclose(f_.fp);
    f_.fp = NULL;
    LOG_Info("BlfWriter close %s objects=%u bytes=%llu",
             f_.path.c_str(), f_.objectCount,
             (unsigned long long)f_.fileBytes);
}

void BlfWriter::closeCurrentFile()
{
    if (!f_.fp) return;
    flushContainer();
    updateFileHeaderOnClose();
    fclose(f_.fp);
    f_.fp = NULL;
}

void BlfWriter::updateFileHeaderOnClose()
{
    captureSystemTime(f_.stopTime);

    uint8_t hdr[blf::FILE_HEADER_SIZE];
    std::memset(hdr, 0, sizeof(hdr));
    putU32LE(&hdr[0],  blf::FILE_SIGNATURE);
    putU32LE(&hdr[4],  (uint32_t)blf::FILE_HEADER_SIZE);
    hdr[8]  = 0;
    hdr[12] = 2;  hdr[13] = 9;  hdr[14] = 0;  hdr[15] = 0;

    putU64LE(&hdr[16], f_.fileBytes);
    putU64LE(&hdr[24], f_.uncompressedTotal);
    putU32LE(&hdr[32], f_.objectCount);
    putU32LE(&hdr[36], f_.objectCount);
    for (size_t i = 0; i < 8; ++i) {
        putU16LE(&hdr[40 + 2 * i], f_.startTime[i]);
    }
    for (size_t i = 0; i < 8; ++i) {
        putU16LE(&hdr[56 + 2 * i], f_.stopTime[i]);
    }

    if (fseek(f_.fp, 0, SEEK_SET) != 0) {
        LOG_Warn("fseek file header failed: %s", strerror(errno));
        return;
    }
    if (fwrite(hdr, 1, sizeof(hdr), f_.fp) != sizeof(hdr)) {
        LOG_Warn("write file header back failed: %s", strerror(errno));
    }
    fflush(f_.fp);
}

int BlfWriter::rotateFile()
{
    closeCurrentFile();

    sessionSeq_++;
    enforceMaxFileCount();

    return openNewFile();
}

bool BlfWriter::parseTimestampPart(const std::string &nameAfterPrefix,
                                   std::string &ts, std::string &seq)
{
    const size_t TS_LEN = 15u;
    const size_t SEQ_MIN = 4u;
    if (nameAfterPrefix.size() < TS_LEN + 1u + SEQ_MIN + 4u) return false;

    if (nameAfterPrefix[TS_LEN] != '_') return false;

    ts = nameAfterPrefix.substr(0, TS_LEN);
    if (ts[8] != '_') return false;
    for (size_t i = 0; i < TS_LEN; ++i) {
        if (i == 8) continue;
        if (ts[i] < '0' || ts[i] > '9') return false;
    }

    size_t dotPos = nameAfterPrefix.find('.', TS_LEN + 1u);
    if (dotPos == std::string::npos) return false;
    seq = nameAfterPrefix.substr(TS_LEN + 1u, dotPos - (TS_LEN + 1u));
    if (seq.empty()) return false;
    for (size_t i = 0; i < seq.size(); ++i) {
        if (seq[i] < '0' || seq[i] > '9') return false;
    }
    return true;
}

std::string BlfWriter::scanMaxTimestampInDir(std::string &outMaxSeq) const
{
    outMaxSeq.clear();
    DIR *dir = opendir(dir_.c_str());
    if (!dir) return "";

    std::string maxTs;
    std::string maxSeq;
    struct dirent *ent;
    const size_t pfxLen = prefix_.size();
    while ((ent = readdir(dir)) != NULL) {
        std::string name = ent->d_name;
        if (name.size() <= pfxLen) continue;
        if (name.compare(0, pfxLen, prefix_) != 0) continue;
        if (name.size() < 4 || name.compare(name.size() - 4, 4, ".blf") != 0) continue;

        std::string rest = name.substr(pfxLen);
        std::string ts, seq;
        if (!parseTimestampPart(rest, ts, seq)) continue;

        if (ts > maxTs || (ts == maxTs && seq > maxSeq)) {
            maxTs = ts;
            maxSeq = seq;
        }
    }
    closedir(dir);

    outMaxSeq = maxSeq;
    return maxTs;
}

std::string BlfWriter::generateSessionTimestamp()
{
    if (isSystemTimeValid()) {
        return formatSessionTimestamp(time(NULL));
    }

    LOG_Warn("BlfWriter: RTC time looks invalid (year < 2020), fallback to dir scan");

    std::string maxSeq;
    std::string maxTs = scanMaxTimestampInDir(maxSeq);
    if (!maxTs.empty()) {
        time_t t = parseSessionTimestamp(maxTs);
        if (t != (time_t)-1) {
            t += 1;
            std::string ts = formatSessionTimestamp(t);
            LOG_Warn("BlfWriter: RTC invalid, reuse dir max ts %s + 1s = %s",
                     maxTs.c_str(), ts.c_str());
            return ts;
        }
    }

    std::string ts = "19700101_000000";
    LOG_Warn("BlfWriter: RTC invalid and no history in dir, use epoch fallback %s",
             ts.c_str());
    return ts;
}

void BlfWriter::enforceMaxFileCount()
{
    if (maxFileCount_ == 0) return;

    DIR *dir = opendir(dir_.c_str());
    if (!dir) return;

    std::vector<std::string> files;
    struct dirent *ent;
    const size_t pfxLen = prefix_.size();
    while ((ent = readdir(dir)) != NULL) {
        std::string name = ent->d_name;
        if (name.size() <= pfxLen) continue;
        if (name.compare(0, pfxLen, prefix_) != 0) continue;
        if (name.size() < 4 || name.compare(name.size() - 4, 4, ".blf") != 0) continue;
        std::string rest = name.substr(pfxLen);
        std::string ts, seq;
        if (!parseTimestampPart(rest, ts, seq)) continue;
        files.push_back(name);
    }
    closedir(dir);

    if (files.size() < maxFileCount_) return;

    std::sort(files.begin(), files.end());

    size_t keep = maxFileCount_ - 1u;
    if (keep >= files.size()) return;

    for (size_t i = 0; i + keep < files.size(); ++i) {
        std::string path = dir_ + files[i];
        if (remove(path.c_str()) == 0) {
            LOG_Info("rotate removed %s", path.c_str());
        } else {
            LOG_Warn("rotate remove %s failed: %s", path.c_str(), strerror(errno));
        }
    }
}

bool BlfWriter::enforceMaxTotalSize(uint64_t incomingBytes)
{
    if (maxTotalBytes_ == 0ULL) {
        return true;
    }

    struct ManagedFile
    {
        std::string name;
        uint64_t size;
    };

    DIR *dir = opendir(dir_.c_str());
    if (!dir) {
        LOG_Error("open %s for total-size check failed: %s",
                  dir_.c_str(), strerror(errno));
        return false;
    }

    std::vector<ManagedFile> files;
    struct dirent *ent;
    const size_t prefixLen = prefix_.size();
    while ((ent = readdir(dir)) != NULL) {
        std::string name = ent->d_name;
        if (name.size() <= prefixLen ||
            name.compare(0, prefixLen, prefix_) != 0 ||
            name.compare(name.size() - 4, 4, ".blf") != 0) {
            continue;
        }

        const std::string path = dir_ + name;
        if (f_.fp != NULL && path == f_.path) {
            files.push_back({name, f_.fileBytes});
            continue;
        }

        struct stat st;
        if (stat(path.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) {
            LOG_Error("stat %s for total-size check failed: %s",
                      path.c_str(), strerror(errno));
            closedir(dir);
            return false;
        }
        files.push_back({name, (uint64_t)st.st_size});
    }
    closedir(dir);

    std::sort(files.begin(), files.end(),
              [](const ManagedFile &a, const ManagedFile &b) {
                  return a.name < b.name;
              });

    uint64_t totalBytes = 0;
    for (size_t i = 0; i < files.size(); ++i) {
        totalBytes += files[i].size;
    }
    for (size_t i = 0; i < files.size(); ++i) {
        if ((totalBytes + incomingBytes) <= maxTotalBytes_) {
            break;
        }

        const std::string path = dir_ + files[i].name;
        if (f_.fp != NULL && path == f_.path) {
            continue;
        }

        if (remove(path.c_str()) != 0) {
            LOG_Error("remove %s for BLF total-size limit failed: %s",
                      path.c_str(), strerror(errno));
            continue;
        }

        LOG_Info("removed %s (%llu bytes) for BLF total-size limit",
                 path.c_str(), (unsigned long long)files[i].size);
        totalBytes -= files[i].size;
    }

    const bool withinLimit = (totalBytes + incomingBytes) <= maxTotalBytes_;
    if (!withinLimit) {
        LOG_Error("BLF total-size limit cannot be enforced: total=%llu incoming=%llu limit=%llu",
                  (unsigned long long)totalBytes,
                  (unsigned long long)incomingBytes,
                  (unsigned long long)maxTotalBytes_);
    }
    return withinLimit;
}

void BlfWriter::serializeFrame(const CanFrameRecord_t &frame, std::vector<uint8_t> &out)
{
    uint64_t ts = frameToBlfTimestamp(frame);

    uint8_t  dlcBytes  = frame.dlc;
    if (dlcBytes > (uint8_t)CAN_FRAME_MAX_DATA_SIZE) {
        dlcBytes = (uint8_t)CAN_FRAME_MAX_DATA_SIZE;
    }

    const bool isFd  = ((frame.flags & CAN_FLAG_FD) != 0) || (dlcBytes > 8u);
    const bool isExt = (frame.flags & CAN_FLAG_EXT_ID) != 0;
    const bool isBrs = (frame.flags & CAN_FLAG_BRS) != 0;
    const bool isTx = (frame.flags & CAN_FLAG_TX) != 0;

    if (dlcBytes > 8u && (frame.flags & CAN_FLAG_FD) == 0) {
        static uint64_t fdAutoDetectCount = 0;
        if ((++fdAutoDetectCount <= 3u) || ((fdAutoDetectCount % 10000u) == 0u)) {
            LOG_Info("FD auto-detect: dlc=%u without CAN_FLAG_FD bit, MCU may have "
                     "omitted FD flag (count=%llu)",
                     dlcBytes, (unsigned long long)fdAutoDetectCount);
        }
    }

    uint8_t  objFlags = 0;
    uint8_t  fdFlags  = 0;
    if (isTx) objFlags |= blf::CAN_MESSAGE_FLAG_TX;
    if (isFd)  fdFlags |= 0x01u;
    if (isBrs) fdFlags |= 0x02u;

    uint32_t canId = frame.canId & 0x1FFFFFFFu;
    if (isExt) canId |= 0x80000000u;

    uint8_t rawDlc;
    if (!isFd) {
        rawDlc = dlcBytes;
    } else {
        rawDlc = len2dlc(dlcBytes);
    }

    uint32_t objType;
    uint32_t payloadSize;
    if (isFd) {
        objType    = blf::OBJ_CAN_FD_MESSAGE;
        payloadSize = (uint32_t)sizeof(blf::CanFdMessage);
    } else {
        objType    = blf::OBJ_CAN_MESSAGE2;
        payloadSize = (uint32_t)sizeof(blf::CanMessage2);
    }
    uint32_t objSize = blf::OBJ_HEADER_SIZE + payloadSize;

    size_t cur = out.size();
    out.resize(cur + objSize);
    uint8_t *p = &out[cur];

    putU32LE(&p[0],  blf::OBJ_SIGNATURE);
    putU16LE(&p[4],  blf::OBJ_HEADER_SIZE);
    putU16LE(&p[6],  blf::OBJ_HEADER_VERSION);
    putU32LE(&p[8],  objSize);
    putU32LE(&p[12], objType);

    putU32LE(&p[16], 0u);
    putU16LE(&p[20], 0u);
    putU16LE(&p[22], 0u);
    putU64LE(&p[24], ts);

    uint8_t *body = p + blf::OBJ_HEADER_SIZE;
    if (isFd) {
        putU16LE(&body[0],  (uint16_t)frame.channel);
        body[2] = objFlags;
        body[3] = rawDlc;
        putU32LE(&body[4],  canId);
        putU32LE(&body[8],  0u);
        body[12] = 0u;
        body[13] = fdFlags;
        body[14] = dlcBytes;
        std::memset(&body[15], 0, 5);
        std::memset(&body[20], 0, 64);
        std::memcpy(&body[20], frame.canData, dlcBytes);
    } else {
        putU16LE(&body[0],  (uint16_t)frame.channel);
        body[2] = objFlags;
        body[3] = rawDlc;
        putU32LE(&body[4],  canId);
        std::memset(&body[8], 0, 8);
        std::memcpy(&body[8], frame.canData, dlcBytes);
    }
}

uint64_t BlfWriter::frameToBlfTimestamp(const CanFrameRecord_t &frame)
{
    const uint64_t tickUs = blf_timestamp::frameTickUs(frame);

    if (!f_.firstFrameSeen) {
        f_.firstFrameTickUs = tickUs;
        f_.firstFrameSeen   = true;
    }

    const uint64_t deltaUs =
        (tickUs > f_.firstFrameTickUs) ? (tickUs - f_.firstFrameTickUs) : 0ULL;
    const uint64_t timestampNs = deltaUs * 1000ULL;
    if (timestampNs < f_.lastEmittedNs) {
        return f_.lastEmittedNs;
    }
    f_.lastEmittedNs = timestampNs;
    return timestampNs;
}

int BlfWriter::appendFrame(const CanFrameRecord_t &frame)
{
    if (!f_.fp) {
        LOG_Error("BlfWriter not opened");
        return -1;
    }
    if ((channelMask_ & (1u << frame.channel)) == 0u) {
        return 0;
    }

    const uint64_t tickUs = blf_timestamp::frameTickUs(frame);
    if (f_.firstFrameSeen && blf_timestamp::isMcuTimestampReset(tickUs, f_.firstFrameTickUs)) {
        LOG_Warn("MCU timestamp is earlier than file base, rotate BLF: "
                 "base=%lluus last=%lluus new=%lluus",
                 (unsigned long long)f_.firstFrameTickUs,
                 (unsigned long long)f_.lastFrameTickUs,
                 (unsigned long long)tickUs);
        if (rotateFile() != 0) {
            LOG_Error("rotate BLF after late MCU frame failed");
            return -1;
        }
    } else if (f_.firstFrameSeen &&
        blf_timestamp::isMcuTimestampReset(tickUs, f_.lastFrameTickUs)) {
        LOG_Warn("MCU timestamp moved backward, rotate BLF: last=%lluus new=%lluus",
                 (unsigned long long)f_.lastFrameTickUs,
                 (unsigned long long)tickUs);
        if (rotateFile() != 0) {
            LOG_Error("rotate BLF after MCU timestamp reset failed");
            return -1;
        }
    }

    serializeFrame(frame, pending_);
    f_.lastFrameTickUs = tickUs;
    f_.objectCount++;
    pendingObjectCount_++;
    stats_.framesAppended++;
    stats_.pendingObjects = pendingObjectCount_;

    if (pending_.size() >= blf::CONTAINER_MAX_UNCOMPRESSED) {
        flushContainer();
        if (f_.fileBytes >= maxFileBytes_) {
            rotateFile();
        }
    }
    return 0;
}

void BlfWriter::flush()
{
    flushContainer();
}

void BlfWriter::flushContainer()
{
    if (pending_.empty() || !f_.fp) {
        return;
    }

    uLong srcLen = (uLong)pending_.size();
    uLong bound  = compressBound(srcLen);
    std::vector<uint8_t> compressed;
    compressed.resize(bound);

    int zret = compress2(compressed.data(), &bound,
                         pending_.data(), srcLen, Z_DEFAULT_COMPRESSION);
    if (zret != Z_OK) {
        LOG_Error("zlib compress failed: %d", zret);
        pending_.clear();
        pendingObjectCount_ = 0;
        return;
    }
    compressed.resize(bound);

    const uint32_t containerHeaderSize = (uint32_t)blf::OBJ_HEADER_BASE_SIZE + 16u;
    const uint32_t objSize = containerHeaderSize + (uint32_t)compressed.size();
    const uint32_t padSize = objSize % 4u;
    const uint64_t incomingBytes =
        (uint64_t)containerHeaderSize + compressed.size() + padSize;

    if (!enforceMaxTotalSize(incomingBytes)) {
        LOG_Error("drop %llu pending bytes: BLF total-size limit cannot be enforced",
                  (unsigned long long)pending_.size());
        pending_.clear();
        pendingObjectCount_ = 0;
        stats_.pendingObjects = 0;
        return;
    }

    uint8_t hdr[containerHeaderSize];
    std::memset(hdr, 0, sizeof(hdr));
    putU32LE(&hdr[0],  blf::OBJ_SIGNATURE);
    putU16LE(&hdr[4],  blf::OBJ_HEADER_BASE_SIZE);
    putU16LE(&hdr[6],  blf::OBJ_HEADER_VERSION);
    putU32LE(&hdr[8],  objSize);
    putU32LE(&hdr[12], blf::OBJ_LOG_CONTAINER);

    putU16LE(&hdr[16], blf::COMPRESSION_ZLIB);
    std::memset(&hdr[18], 0, 6);
    putU32LE(&hdr[24], (uint32_t)srcLen);
    std::memset(&hdr[28], 0, 4);

    if (fwrite(hdr, 1, sizeof(hdr), f_.fp) != sizeof(hdr)) {
        LOG_Error("write container header failed: %s", strerror(errno));
        pending_.clear();
        pendingObjectCount_ = 0;
        return;
    }
    f_.fileBytes += sizeof(hdr);

    if (!compressed.empty()) {
        if (fwrite(compressed.data(), 1, compressed.size(), f_.fp) != compressed.size()) {
            LOG_Error("write container payload failed: %s", strerror(errno));
            pending_.clear();
            pendingObjectCount_ = 0;
            return;
        }
        f_.fileBytes += compressed.size();
    }

    if (padSize > 0u) {
        static const uint8_t ZEROS[4] = {0, 0, 0, 0};
        if (fwrite(ZEROS, 1, padSize, f_.fp) != padSize) {
            LOG_Error("write container padding failed: %s", strerror(errno));
            pending_.clear();
            pendingObjectCount_ = 0;
            return;
        }
        f_.fileBytes += padSize;
    }

    f_.uncompressedTotal += srcLen;

    fflush(f_.fp);

    stats_.containersFlushed++;
    stats_.bytesWritten = f_.fileBytes;
    stats_.currentFileBytes = f_.fileBytes;
    stats_.pendingObjects = 0;

    LOG_Info("flush container: objects=%llu bytes_in=%lu bytes_out=%lu file_bytes=%llu file=%s",
             (unsigned long long)pendingObjectCount_, srcLen, (unsigned long)compressed.size(),
             (unsigned long long)f_.fileBytes, f_.path.c_str());

    pending_.clear();
    pendingObjectCount_ = 0;
}

BlfWriter::Stats BlfWriter::getStats() const
{
    Stats s = stats_;
    s.currentFileIndex = sessionSeq_;
    s.currentFileBytes = f_.fileBytes;
    s.currentFilePath  = f_.path;
    s.pendingObjects   = pendingObjectCount_;
    return s;
}
