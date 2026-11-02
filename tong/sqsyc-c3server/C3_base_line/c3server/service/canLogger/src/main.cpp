/**
 * @file main.cpp
 * @brief canLogger 进程入口：IPC 订阅、回调解析、写线程与主循环
 *
 * @details
 * 进程模型：
 * - Main 线程：日志初始化、读配置、启动 IPC/写线程、跑 GMainLoop 与心跳；
 * - IPC 接收线程（sr-ipc 内部创建）：触发 RecvCanRawFrameCallback；
 * - BlfWriterThread：批量取帧、字节序转换、调 BlfWriter::appendFrame。
 *
 * 数据通路：MCU → IPC → 回调 → FrameQueue → 写线程 → BlfWriter → 磁盘。
 */
#include <glib.h>
#include <endian.h>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <vector>
#include <atomic>

#include "err_code.h"
#include "sr_log.h"
#include "sr_ipc.h"
#include "queue_msg.h"

#include "blf_writer.h"
#include "can_raw_frame.h"
#include "frame_queue.h"
#include "logger_cfg.h"

#define DEFAULT_CFG_PATH     "/oemdata/config/canLogger.ini"  ///< 默认配置文件路径
#define FALLBACK_CFG_PATH    "/oemapp/config/canLogger.ini"   ///< 默认路径不存在时的回退路径
#define BATCH_POP_MAX        (1024u)                          ///< 写线程单次最多出队的帧数
#define POP_TIMEOUT_MS       (200u)                           ///< 出队超时，超时后触发兜底 flush
#define HEARTBEAT_INTERVAL_S (10u)                            ///< 心跳日志间隔（秒）

static BlfWriter g_writer;                                    ///< 全局唯一 BlfWriter 实例
static GMainLoop *g_loop = NULL;                              ///< GLib 主循环
static std::atomic<uint64_t> g_recvCbCount{0};                ///< IPC 回调累计次数（诊断用）
static std::atomic<uint64_t> g_recvFrameCount{0};             ///< IPC 累计接收帧数（诊断用）

/**
 * @brief CAN 原始报文 IPC 回调
 *
 * @details 处理流程：
 * 1. 头 2 字节大端解析为本批次帧数 num；
 * 2. 优先按"定长 CanFrameRecord_t (76B/帧)"格式解析（need ≤ len 即采用）；
 * 3. 长度不匹配时回退到"变长 16B 头 + dlc 字节数据"格式；
 * 4. 解析成功的帧批量 pushBatch 到 FrameQueue。
 *
 * 诊断日志：前 3 次回调 + 每 1000 次，打印前 48 字节十六进制 dump。
 *
 * @param[in] data      IPC payload，前 2 字节为 num（大端）
 * @param[in] len       payload 字节数
 * @param[in] userdata  未使用
 */
extern "C" void RecvCanRawFrameCallback(const uint8_t *data, uint16_t len, void *userdata)
{
    (void)userdata;
    g_recvCbCount++;

    if (!data || len < 2u) {
        LOG_Warn("CAN raw cb #%llu: too short len=%u",
                 (unsigned long long)g_recvCbCount.load(), len);
        return;
    }
    uint16_t num = (uint16_t)(((uint16_t)data[0] << 8) | (uint16_t)data[1]);

    if (g_recvCbCount.load() <= 3u ||
        (g_recvCbCount.load() % 1000u) == 0u) {
        char hex[160];
        size_t dumpLen = (len < 48u) ? (size_t)len : 48u;
        size_t p = 0;
        for (size_t i = 0; i < dumpLen; ++i) {
            int w = snprintf(&hex[p], sizeof(hex) - p, "%02X ", data[i]);
            if (w <= 0 || (size_t)w >= sizeof(hex) - p) break;
            p += (size_t)w;
        }
        if (p > 0 && hex[p - 1] == ' ') hex[p - 1] = '\0';
        else hex[p] = '\0';

        LOG_Notice("CAN raw cb #%llu: len=%u num=%u hex_first_%zuB=[%s]",
                   (unsigned long long)g_recvCbCount.load(), len, num,
                   dumpLen, hex);
    }

    size_t need = 2u + (size_t)num * sizeof(CanFrameRecord_t);
    bool fixedLenOk = (need <= (size_t)len);

    if (fixedLenOk) {
        g_recvFrameCount += num;
        const CanFrameRecord_t *frames = (const CanFrameRecord_t *)(data + 2);
        FrameQueue::instance().pushBatch(frames, num);
        return;
    }

    if (num == 0u) {
        return;
    }

    const size_t FRAME_FIXED_SIZE = 16u;
    size_t off = 2u;
    std::vector<CanFrameRecord_t> parsed;
    parsed.reserve(num);
    bool parseOk = true;

    for (uint16_t i = 0; i < num; ++i) {
        if (off + FRAME_FIXED_SIZE > (size_t)len) {
            parseOk = false;
            break;
        }
        CanFrameRecord_t f;
        std::memset(&f, 0, sizeof(f));
        f.channel  = data[off + 0];
        f.flags    = data[off + 1];
        f.dlc      = data[off + 2];
        f.reserved = data[off + 3];
        std::memcpy(&f.canId,       &data[off + 4],  4);
        std::memcpy(&f.timestampMs, &data[off + 8],  4);
        std::memcpy(&f.timestampUs, &data[off + 12], 4);
        off += FRAME_FIXED_SIZE;

        if (f.dlc > (uint8_t)CAN_FRAME_MAX_DATA_SIZE) {
            if ((g_recvCbCount.load() <= 3u) ||
                ((g_recvCbCount.load() % 100u) == 1u)) {
                LOG_Warn("cb #%llu var-len parse: frame[%u] dlc=%u > %u — format guess wrong, "
                         "see hex dump above",
                         (unsigned long long)g_recvCbCount.load(), i,
                         f.dlc, (unsigned)CAN_FRAME_MAX_DATA_SIZE);
            }
            parseOk = false;
            break;
        }
        if (off + f.dlc > (size_t)len) {
            parseOk = false;
            break;
        }
        std::memcpy(f.canData, &data[off], f.dlc);
        off += f.dlc;
        parsed.push_back(f);
    }

    if (!parseOk || parsed.empty()) {
        if ((g_recvCbCount.load() % 100u) == 1u) {
            LOG_Warn("cb #%llu: both formats failed (fixed-80B need=%zu, var-len parsed=%zu/%u "
                     "off=%zu len=%u) — see hex dump to determine real format",
                     (unsigned long long)g_recvCbCount.load(),
                     need, parsed.size(), num, off, len);
        }
        return;
    }

    if (g_recvCbCount.load() <= 3u) {
        const CanFrameRecord_t &f0 = parsed[0];
        LOG_Notice("cb #%llu var-len OK: parsed=%zu off_end=%zu len=%u "
                   "first(ch=%u id=0x%x dlc=%u flags=0x%02x rsv=%u ts=%u.%06u)",
                   (unsigned long long)g_recvCbCount.load(),
                   parsed.size(), off, len,
                   f0.channel, be32toh(f0.canId), f0.dlc, f0.flags, f0.reserved,
                   be32toh(f0.timestampMs), be32toh(f0.timestampUs));
    }

    if (off != (size_t)len && ((g_recvCbCount.load() % 1000u) == 0u)) {
        LOG_Info("cb #%llu: trailing %zu bytes after %zu frames",
                 (unsigned long long)g_recvCbCount.load(),
                 (size_t)len - off, parsed.size());
    }

    g_recvFrameCount += parsed.size();
    FrameQueue::instance().pushBatch(parsed.data(), parsed.size());
}

/**
 * @brief BLF 写线程入口
 *
 * @details 循环从 FrameQueue 批量取帧：
 * - 每帧做 be32toh 字节序转换（canId / timestampMs / timestampUs）；
 * - 调 BlfWriter::appendFrame 写入；
 * - 当 popBatch 返回 0（超时无数据）触发一次 flush，避免数据滞留；
 * - FrameQueue 进入 shutdown 状态后退出循环，做最后一次 flush。
 *
 * @param[in] arg 未使用
 * @return 固定 NULL
 */
static gpointer BlfWriterThread(gpointer arg)
{
    (void)arg;
    std::vector<CanFrameRecord_t> batch;
    batch.reserve(BATCH_POP_MAX);
    LOG_Info("BlfWriterThread started");

    while (!FrameQueue::instance().isShutdown()) {
        size_t n = FrameQueue::instance().popBatch(batch, BATCH_POP_MAX, POP_TIMEOUT_MS);
        /* n is authoritative; batch is cleared by popBatch on timeout. */
        for (size_t i = 0; i < n; ++i) {
            CanFrameRecord_t f = batch[i];
            f.canId       = be32toh(f.canId);
            f.timestampMs = be32toh(f.timestampMs);
            f.timestampUs = be32toh(f.timestampUs);
            g_writer.appendFrame(f);
        }
        if (n == 0u) {
            g_writer.flush();
        }
    }
    g_writer.flush();
    LOG_Info("BlfWriterThread exit");
    return NULL;
}

/**
 * @brief 10 秒心跳定时器回调
 *
 * @details Debug 级别打印 recv_cb / recv_frames / appended / flushed /
 *          pending / 当前文件路径与字节数，用于运行期观测。
 *
 * @param[in] userdata 未使用
 * @return G_SOURCE_CONTINUE 保持定时器持续
 */
static gboolean HeartbeatTimer(gpointer userdata)
{
    (void)userdata;
    BlfWriter::Stats s = g_writer.getStats();
    LOG_Debug("heartbeat: recv_cb=%llu recv_frames=%llu appended=%llu flushed=%llu "
               "pending=%llu file[%u]=%s bytes=%llu",
               (unsigned long long)g_recvCbCount.load(),
               (unsigned long long)g_recvFrameCount.load(),
               (unsigned long long)s.framesAppended,
               (unsigned long long)s.containersFlushed,
               (unsigned long long)s.pendingObjects,
               s.currentFileIndex, s.currentFilePath.c_str(),
               (unsigned long long)s.currentFileBytes);
    return G_SOURCE_CONTINUE;
}

/**
 * @brief 信号处理函数（SIGTERM / SIGINT）
 *
 * @details 不在信号上下文中做任何 IO，仅 quit 主循环，由主线程接管清理。
 *
 * @param[in] signo 收到的信号编号
 */
static void OnSignal(int signo)
{
    if (g_loop) {
        g_main_loop_quit(g_loop);
    }
    fprintf(stderr, "[canLogger] received signal %d, shutting down\n", signo);
}

/**
 * @brief 进程入口
 *
 * @details 启动顺序：
 * 1. LOG_INIT 初始化日志；
 * 2. 加载配置（默认路径优先，回退到 /oemapp 路径），cfg.enable=false 直接退出；
 * 3. BlfWriter::open 创建输出目录与首个 BLF 文件，写入文件大小/数量/通道配置；
 * 4. StartQueueMsgRcvThread 启动 IPC 接收线程，SubscriptionQueueMsg 订阅 CAN_RAW；
 * 5. g_thread_new 启动 BlfWriterThread；
 * 6. 注册 SIGTERM/SIGINT 处理函数，进入 g_main_loop_run + 10s 心跳。
 *
 * 退出顺序：信号触发 quit → shutdown 队列 → join 写线程 → 释放主循环 → close Writer。
 *
 * @return 0 正常退出，-1 关键初始化失败
 */
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    int rc = LOG_INIT("canLoggerServer");
    if (rc != 0) {
        fprintf(stderr, "[canLogger] LOG_INIT failed rc=%d, conf=%s rule=canLogger\n",
                rc, ZLOG_CONF_FILE_PATH);
    }

    LOG_Debug("canLoggerServer start!!!");

    LoggerCfg cfg;
    const char *cfgPath = DEFAULT_CFG_PATH;
    if (access(FALLBACK_CFG_PATH, F_OK) == 0) {
        cfgPath = FALLBACK_CFG_PATH;
    }
    if (LoadLoggerConfig(cfg, cfgPath) != RET_OK) {
        LOG_Warn("config load %s failed, using defaults", cfgPath);
    }
    if (!cfg.enable) {
        LOG_Debug("canLogger disabled by config, exit");
        return 0;
    }

    LOG_Debug("canLogger starting: dir=%s prefix=%s maxSizeMB=%u maxCount=%u maxTotalSizeMB=%u flushMs=%u",
               cfg.filePath.c_str(), cfg.filePrefix.c_str(),
               cfg.maxSizeMB, cfg.maxFileCount, cfg.maxTotalSizeMB, cfg.flushIntervalMs);

    g_writer.setMaxFileSizeMB(cfg.maxSizeMB);
    g_writer.setMaxFileCount(cfg.maxFileCount);
    g_writer.setMaxTotalSizeMB(cfg.maxTotalSizeMB);
    g_writer.setChannelMask(cfg.channelMask);

    if (g_writer.open(cfg.filePath, cfg.filePrefix) != 0) {
        LOG_Error("BlfWriter open failed");
        return -1;
    }

    if (StartQueueMsgRcvThread("canlogger_ipc") != RET_OK) {
        LOG_Error("StartQueueMsgRcvThread failed");
        return -1;
    }
    LOG_Debug("IPC recv thread started");

    if (SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_VEHICLE_CAN_RAW,
                             RecvCanRawFrameCallback, NULL) != RET_OK) {
        LOG_Error("SubscriptionQueueMsg CAN_RAW (0x%08x) failed",
                  (unsigned)SYS_QUE_TYPE_MCU_VEHICLE_CAN_RAW);
        return -1;
    }
    LOG_Debug("subscribed SYS_QUE_TYPE_MCU_VEHICLE_CAN_RAW (0x%08x), waiting for frames...",
               (unsigned)SYS_QUE_TYPE_MCU_VEHICLE_CAN_RAW);

    GThread *writer = g_thread_new("BlfWriter", BlfWriterThread, NULL);

    signal(SIGTERM, OnSignal);
    signal(SIGINT,  OnSignal);

    g_loop = g_main_loop_new(NULL, FALSE);
    g_timeout_add_seconds(HEARTBEAT_INTERVAL_S, HeartbeatTimer, NULL);
    g_main_loop_run(g_loop);

    LOG_Debug("canLogger main loop exit, shutting down...");

    FrameQueue::instance().shutdown();
    if (writer) {
        g_thread_join(writer);
        g_thread_unref(writer);
    }
    g_main_loop_unref(g_loop);
    g_loop = NULL;

    g_writer.close();
    LOG_Debug("canLogger exited");
    return 0;
}
