/**
 * tsp_can_data.c - CAN原始报文采集与缓存模块实现
 *
 * 架构:
 * - 订阅 SYS_QUE_TYPE_MCU_VEHICLE_CAN_RAW (0x00020400)
 * - IPC回调中按1s限频采样, 筛选匹配的CAN ID并缓存最新8字节数据
 * - 提供 tsp_can_data_lookup() 查找接口, 供 tsp_app 实时上报组包
 *
 * 数据格式(参考 canloger/RecvCanRawFrameCallback):
 *   [0..1]  BE16 帧数量 num
 *   [2..N]  定长模式: CanFrameRecord_t[num] (每帧76字节, 对齐后80字节)
 *           变长模式: (1B ch + 1B flags + 1B dlc + 1B reserved + 4B canId(BE)
 *                      + 4B tsMs(BE) + 4B tsUs(BE) + dlcB data)[num]
 */

#include "tsp_can_data.h"
#include "tsp_log.h"
#include "tsp_blf.h"
#include "tsp_common.h"
#include "tsp_vin.h"

#include "sr_ipc.h"
#include "queue_msg.h"

#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>
#include <glob.h>
#include <errno.h>
#include <inttypes.h>

/* ======================== 本地定义 ======================== */

#define CAN_FRAME_MAX_DATA       (64u)

/** @brief CAN原始帧记录(与MCU IPC协议对齐, 大端字段需ntohl转换) */
typedef struct {
    uint8_t  channel;       /**< CAN通道号 */
    uint8_t  flags;         /**< 帧标志位 */
    uint8_t  dlc;           /**< 实际有效字节数(0~64) */
    uint8_t  reserved;      /**< 保留 */
    uint32_t canId;         /**< 仲裁ID (大端) */
    uint32_t timestampMs;   /**< MCU时间戳高位, 毫秒 (大端) */
    uint32_t timestampUs;   /**< MCU时间戳低位, 微秒 (大端) */
    uint8_t  canData[CAN_FRAME_MAX_DATA]; /**< 数据字段 */
} can_frame_record_t;

/** @brief 变长模式帧头固定大小 */
#define VARLEN_FRAME_HDR_SIZE   (16u)

/** @brief 采样限频间隔(毫秒), 满足"10ms生产→1s采集"需求 */
#define SAMPLING_INTERVAL_MS    (1000u)

/** @brief 数据超时时间(毫秒), 超过此时间认为数据已过期(10秒,与上报周期一致) */
#define CAN_DATA_TIMEOUT_MS     (10000u)

/* ======================== 哈希表配置 ======================== */

/** @brief 哈希表大小（必须是2的幂，用于位运算取模） */
#define HASH_TABLE_SIZE         512

/** @brief 哈希表掩码 */
#define HASH_MASK               (HASH_TABLE_SIZE - 1)

/** @brief 哈希表特殊标记值 */
#define HASH_ENTRY_EMPTY        0xFFFF   /**< 空槽标记 */
#define HASH_ENTRY_DELETED      0xFFFE   /**< 已删除槽标记（tombstone） */

/**
 * @brief 改进的哈希函数 - 使用FNV-1a算法变体
 * 提供更好的分布性和雪崩效应
 */
static inline uint32_t hash_fnv1a(uint32_t can_id, uint8_t channel)
{
    /* FNV-1a 32-bit hash */
    uint32_t hash = 2166136261u;  /* FNV offset basis */
    
    /* 混合 can_id 的四个字节 */
    hash ^= (can_id & 0xFF);
    hash *= 16777619u;  /* FNV prime */
    hash ^= ((can_id >> 8) & 0xFF);
    hash *= 16777619u;
    hash ^= ((can_id >> 16) & 0xFF);
    hash *= 16777619u;
    hash ^= ((can_id >> 24) & 0xFF);
    hash *= 16777619u;
    
    /* 混合 channel */
    hash ^= channel;
    hash *= 16777619u;
    
    return hash & HASH_MASK;
}

#define HASH_KEY(can_id, channel)   hash_fnv1a(can_id, channel)

/* ======================== 缓存条目 ======================== */

/** @brief 单个CAN帧历史记录 */
typedef struct {
    uint8_t  data[8];       /**< CAN报文数据 */
    uint8_t  dlc;           /**< 数据长度 */
    uint64_t timestamp_ms;  /**< 采集时间戳（毫秒） */
    bool     valid;         /**< 是否有效 */
} can_history_frame_t;

/** @brief 单个CAN ID的环形缓冲区 */
typedef struct {
    uint32_t can_id;        /**< CAN标识符 */
    uint8_t  channel;       /**< 通道号 */
    can_history_frame_t history[CAN_HISTORY_FRAME_COUNT]; /**< 历史帧数组 */
    uint16_t write_idx;     /**< 当前写入位置 */
    uint16_t count;         /**< 有效帧数 */
} can_cache_entry_t;

/** @brief 哈希表条目（支持开放寻址法处理冲突） */
typedef struct {
    uint32_t can_id;        /**< CAN ID，0xFFFFFFFF表示空槽 */
    uint8_t  channel;       /**< 通道号 */
    uint16_t entry_idx;     /**< 缓存条目索引，HASH_ENTRY_EMPTY表示空，HASH_ENTRY_DELETED表示已删除 */
} hash_entry_t;

/* ======================== 模块全局状态 ======================== */

/** @brief CAN数据模块全局状态结构体 */
typedef struct {
    can_cache_entry_t *cache;           /**< 动态分配的缓存数组 */
    uint16_t           cache_count;     /**< 缓存条目数量 */
    uint64_t           last_sample_ms;    /**< 上次采样时间戳(毫秒) */
    bool               initialized;       /**< 模块初始化标志 */
    hash_entry_t       hash_table[HASH_TABLE_SIZE]; /**< 哈希表，快速查找 */
    pthread_mutex_t    hash_mutex;        /**< 哈希表操作互斥锁 */
    uint32_t           hash_count;        /**< 当前哈希表中的有效条目数 */
} can_data_ctx_t;

static can_data_ctx_t g_ctx = {
    .cache          = NULL,
    .cache_count    = 0,
    .last_sample_ms = 0,
    .initialized    = false,
    .hash_table     = {{0}},  /* 将在hash_table_init中完整初始化 */
    .hash_mutex     = PTHREAD_MUTEX_INITIALIZER,
    .hash_count     = 0
};

/* ======================== 哈希表操作 ======================== */

/**
 * @brief 初始化哈希表
 * @note 调用者需持有hash_mutex锁
 */
static void hash_table_init(void)
{
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        g_ctx.hash_table[i].can_id = 0xFFFFFFFF;
        g_ctx.hash_table[i].channel = 0;
        g_ctx.hash_table[i].entry_idx = HASH_ENTRY_EMPTY;
    }
    g_ctx.hash_count = 0;
}

/**
 * @brief 在哈希表中查找条目
 * @param can_id CAN标识符
 * @param channel 通道号
 * @return 找到返回缓存索引，未找到返回HASH_ENTRY_EMPTY
 * @note 调用者需持有hash_mutex锁
 */
static uint16_t hash_table_lookup(uint32_t can_id, uint8_t channel)
{
    uint32_t hash = HASH_KEY(can_id, channel);
    
    /* 线性探测处理冲突 */
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        uint32_t probe = (hash + i) & HASH_MASK;
        hash_entry_t *entry = &g_ctx.hash_table[probe];
        
        /* 空槽表示未找到（遇到tombstone继续探测） */
        if (entry->entry_idx == HASH_ENTRY_EMPTY) {
            return HASH_ENTRY_EMPTY;
        }
        
        /* 跳过已删除的槽位，继续探测 */
        if (entry->entry_idx == HASH_ENTRY_DELETED) {
            continue;
        }
        
        /* 检查是否匹配 */
        if (entry->can_id == can_id && entry->channel == channel) {
            return entry->entry_idx;
        }
    }
    
    return HASH_ENTRY_EMPTY;  /* 未找到 */
}

/**
 * @brief 向哈希表插入条目
 * @param can_id CAN标识符
 * @param channel 通道号
 * @param entry_idx 缓存条目索引
 * @return 0成功，-1失败（哈希表满）
 * @note 调用者需持有hash_mutex锁
 */
static int hash_table_insert(uint32_t can_id, uint8_t channel, uint16_t entry_idx)
{
    /* 检查负载因子，如果超过75%则拒绝插入（预留空间给删除后的tombstone） */
    if (g_ctx.hash_count >= (HASH_TABLE_SIZE * 3 / 4)) {
        TLY_LOG_E("CAN_DATA: hash table load factor too high, cannot insert can_id=0x%08X", can_id);
        return -1;
    }
    
    uint32_t hash = HASH_KEY(can_id, channel);
    int first_tombstone = -1;  /* 记录第一个遇到的tombstone位置 */
    
    /* 线性探测处理冲突 */
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        uint32_t probe = (hash + i) & HASH_MASK;
        hash_entry_t *entry = &g_ctx.hash_table[probe];
        
        /* 检查是否已存在相同的键 */
        if (entry->entry_idx != HASH_ENTRY_EMPTY &&
            entry->entry_idx != HASH_ENTRY_DELETED &&
            entry->can_id == can_id && entry->channel == channel) {
            /* 更新现有条目 */
            entry->entry_idx = entry_idx;
            return 0;
        }
        
        /* 记录第一个tombstone位置用于复用 */
        if (first_tombstone < 0 && entry->entry_idx == HASH_ENTRY_DELETED) {
            first_tombstone = probe;
            continue;
        }
        
        /* 找到空槽 */
        if (entry->entry_idx == HASH_ENTRY_EMPTY) {
            /* 如果有tombstone，优先使用tombstone位置 */
            if (first_tombstone >= 0) {
                probe = first_tombstone;
                entry = &g_ctx.hash_table[probe];
            }
            entry->can_id = can_id;
            entry->channel = channel;
            entry->entry_idx = entry_idx;
            g_ctx.hash_count++;
            return 0;
        }
    }
    
    /* 哈希表已满，但可能还有tombstone可以复用 */
    if (first_tombstone >= 0) {
        hash_entry_t *entry = &g_ctx.hash_table[first_tombstone];
        entry->can_id = can_id;
        entry->channel = channel;
        entry->entry_idx = entry_idx;
        g_ctx.hash_count++;
        return 0;
    }
    
    TLY_LOG_E("CAN_DATA: hash table full, cannot insert can_id=0x%08X", can_id);
    return -1;
}

/* ======================== 内部辅助 ======================== */

/**
 * @brief 将一帧CAN数据写入匹配的缓存条目（环形缓冲区）
 * @param timestamp_ms 当前时间戳（毫秒），由调用者统一获取
 */
static void cache_frame(uint32_t can_id, uint8_t channel,
                        const uint8_t *data, uint8_t dlc, uint64_t timestamp_ms)
{
    uint8_t copy_len = (dlc > 8u) ? 8u : dlc;

    /* 加锁保护哈希表查找 */
    pthread_mutex_lock(&g_ctx.hash_mutex);
    uint16_t idx = hash_table_lookup(can_id, channel);
    pthread_mutex_unlock(&g_ctx.hash_mutex);
    
    if (idx == HASH_ENTRY_EMPTY) {
        return;  /* 未找到匹配的缓存条目 */
    }
    
    can_cache_entry_t *entry = &g_ctx.cache[idx];
    
    /* 写入当前位置 */
    uint16_t write_pos = entry->write_idx;
    /* 优化：只在数据长度不足8字节时才清零剩余部分 */
    if (copy_len < 8) {
        memset(entry->history[write_pos].data + copy_len, 0, 8 - copy_len);
    }
    memcpy(entry->history[write_pos].data, data, copy_len);
    entry->history[write_pos].dlc = copy_len;
    entry->history[write_pos].timestamp_ms = timestamp_ms;
    entry->history[write_pos].valid = true;
    
    /* 更新写入位置（环形） */
    entry->write_idx = (write_pos + 1) % CAN_HISTORY_FRAME_COUNT;
    
    /* 更新有效帧数 */
    if (entry->count < CAN_HISTORY_FRAME_COUNT) {
        entry->count++;
    }
}

/**
 * @brief 处理定长模式CAN帧数组 (每帧 sizeof(can_frame_record_t))
 */
static void process_fixed_frames(const uint8_t *data, uint16_t num, uint64_t timestamp_ms)
{
    const can_frame_record_t *frames = (const can_frame_record_t *)data;

    for (uint16_t i = 0; i < num; i++) {
        uint32_t can_id  = ntohl(frames[i].canId);
        uint8_t  channel = frames[i].channel;
        uint8_t  dlc     = frames[i].dlc;

        cache_frame(can_id, channel, frames[i].canData, dlc, timestamp_ms);

        /* 更新VIN数据 */
        if (vin_is_vin_can_id(can_id)) {
            vin_update_frame(can_id, channel, frames[i].canData, dlc);
        }
    }
}

/**
 * @brief 处理变长模式CAN帧数组 (16B头 + dlc字节数据)
 */
static void process_varlen_frames(const uint8_t *data, uint32_t data_len,
                                   uint16_t num, uint64_t timestamp_ms)
{
    uint32_t off = 2u; /* 跳过2字节帧数量 */

    for (uint16_t i = 0; i < num; i++) {
        if (off + VARLEN_FRAME_HDR_SIZE > data_len) {
            break;
        }

        uint8_t  channel = data[off + 0];
        /* uint8_t  flags   = data[off + 1]; (未使用) */
        uint8_t  dlc     = data[off + 2];
        /* uint8_t  resv    = data[off + 3]; (未使用) */
        
        /* 使用逐字节读取避免非对齐内存访问，大端序 */
        uint32_t can_id = ((uint32_t)data[off + 4] << 24) |
                          ((uint32_t)data[off + 5] << 16) |
                          ((uint32_t)data[off + 6] << 8)  |
                          ((uint32_t)data[off + 7]);
        /* uint32_t tsMs    = ntohl(...); (未使用) */
        /* uint32_t tsUs    = ntohl(...); (未使用) */

        off += VARLEN_FRAME_HDR_SIZE;

        if (off + dlc > data_len) {
            break;
        }

        cache_frame(can_id, channel, data + off, dlc, timestamp_ms);

        /* 更新VIN数据 */
        if (vin_is_vin_can_id(can_id)) {
            vin_update_frame(can_id, channel, data + off, dlc);
        }

        off += dlc;
    }
}

/* ======================== IPC订阅回调 ======================== */

/**
 * @brief SYS_QUE_TYPE_MCU_VEHICLE_CAN_RAW 订阅回调
 *
 * @details
 * 1s限频: 距上次采样不足1秒则直接跳过, 满足"10ms生产→1s采集"需求
 * 数据格式: 2B帧数量(BE) + 定长/变长帧数组
 * 对每个帧, 查找匹配的(can_id, channel), 更新最新数据
 *
 * @param recvData  IPC payload
 * @param dataLen   payload长度
 * @param userdata  未使用
 */
static void can_raw_callback(const uint8_t *recvData, const uint16_t dataLen,
                              void *userdata)
{
    (void)userdata;

    if (!g_ctx.initialized || !g_ctx.cache || g_ctx.cache_count == 0) {
        return;
    }

    /* --- 1s限频 --- */
    uint64_t now_ms = tly_monotonic_time_ms();
    // if (now_ms - g_ctx.last_sample_ms < SAMPLING_INTERVAL_MS) {
    //     return;
    // }
    g_ctx.last_sample_ms = now_ms;

    /* --- 基础校验 --- */
    if (!recvData || dataLen < 2u) {
        return;
    }

    uint16_t num = (uint16_t)(((uint16_t)recvData[0] << 8) | (uint16_t)recvData[1]);
    if (num == 0u) {
        return;
    }

    /* --- 定长模式: 校验长度是否匹配 --- */
    uint32_t need_fixed = 2u + (uint32_t)num * (uint32_t)sizeof(can_frame_record_t);

    if (need_fixed <= (uint32_t)dataLen) {
        process_fixed_frames(recvData + 2, num, now_ms);
        return;
    }

    /* --- 变长模式: 16B固定头 + dlc字节数据 --- */
    process_varlen_frames(recvData, dataLen, num, now_ms);
}

/* ======================== 生命周期 ======================== */

int tsp_can_data_init(void)
{
    TLY_LOG_I("CAN_DATA: init start");
    
    if (g_ctx.initialized) {
        TLY_LOG_W("CAN_DATA: already initialized");
        return 0;
    }

    /* 检查全局状态 */
    if (g_ctx.cache != NULL) {
        TLY_LOG_E("CAN_DATA: cache not NULL before init, possible memory leak");
        free(g_ctx.cache);
        g_ctx.cache = NULL;
    }
    g_ctx.cache_count = 0;
    g_ctx.last_sample_ms = 0;

    /* 订阅队列 (IPC由tsp_app.c:InitSysQueTree/StartQueueMsgRcvThread初始化) */
    TLY_LOG_I("CAN_DATA: subscribing queue 0x%08X", (unsigned)SYS_QUE_TYPE_MCU_VEHICLE_CAN_RAW);
    int32_t ret = SubscriptionQueueMsg(
        (SYSQueMsgType_e)SYS_QUE_TYPE_MCU_VEHICLE_CAN_RAW,
        can_raw_callback, NULL);
    if (ret != 0) {
        TLY_LOG_E("CAN_DATA: SubscriptionQueueMsg failed, ret=%d, IPC may not be ready yet", ret);
    }

    g_ctx.last_sample_ms = tly_monotonic_time_ms();
    g_ctx.initialized = true;
    TLY_LOG_I("CAN_DATA: init success, subscribed 0x%08X, history_frames=%d",
              (unsigned)SYS_QUE_TYPE_MCU_VEHICLE_CAN_RAW, CAN_HISTORY_FRAME_COUNT);
    return 0;
}

void tsp_can_data_deinit(void)
{
    g_ctx.initialized = false;

    /* 加锁保护，确保没有正在进行的哈希表操作 */
    pthread_mutex_lock(&g_ctx.hash_mutex);
    
    if (g_ctx.cache) {
        free(g_ctx.cache);
        g_ctx.cache = NULL;
    }
    g_ctx.cache_count = 0;
    g_ctx.last_sample_ms = 0;
    
    /* 清空哈希表 */
    hash_table_init();
    
    pthread_mutex_unlock(&g_ctx.hash_mutex);
    
    /* 销毁互斥锁 */
    pthread_mutex_destroy(&g_ctx.hash_mutex);

    TLY_LOG_I("CAN_DATA: deinitialized");
}

/* ======================== 筛选更新 ======================== */

void tsp_can_data_update_filter(const tly_canid_list_resp_t *canid_cache)
{
    if (!canid_cache) {
        TLY_LOG_W("CAN_DATA: update_filter with NULL cache");
        return;
    }

    uint16_t total = canid_cache->fast_count + canid_cache->slow_count;
    if (total > TLY_CANID_MAX) {
        total = TLY_CANID_MAX;
    }

    /* 加锁保护哈希表和缓存操作 */
    pthread_mutex_lock(&g_ctx.hash_mutex);

    /* 释放旧缓存并清空状态 */
    if (g_ctx.cache) {
        free(g_ctx.cache);
        g_ctx.cache = NULL;
    }
    g_ctx.cache_count = 0;
    hash_table_init();  /* 统一初始化哈希表 */

    /* 处理空列表情况 */
    if (total == 0) {
        pthread_mutex_unlock(&g_ctx.hash_mutex);
        TLY_LOG_W("CAN_DATA: update_filter with empty list");
        return;
    }

    /* 分配新缓存 */
    g_ctx.cache = (can_cache_entry_t *)calloc(total, sizeof(can_cache_entry_t));
    if (!g_ctx.cache) {
        pthread_mutex_unlock(&g_ctx.hash_mutex);
        TLY_LOG_E("CAN_DATA: update_filter alloc failed");
        return;
    }

    /* 填充 CAN ID 列表（calloc 已将 valid 初始化为 0）*/
    uint16_t idx = 0;
    const uint16_t *src_counts[2] = { &canid_cache->fast_count, &canid_cache->slow_count };
    const tly_canid_entry_t *src_ids[2] = { canid_cache->fast_ids, canid_cache->slow_ids };
    
    for (int src = 0; src < 2 && idx < total; src++) {
        for (uint16_t i = 0; i < *src_counts[src] && idx < total; i++) {
            g_ctx.cache[idx].can_id  = src_ids[src][i].can_id;
            g_ctx.cache[idx].channel = src_ids[src][i].channel;
            /* 插入哈希表 */
            if (hash_table_insert(g_ctx.cache[idx].can_id, g_ctx.cache[idx].channel, idx) != 0) {
                TLY_LOG_W("CAN_DATA: failed to insert can_id=0x%08X ch=%u to hash table",
                          g_ctx.cache[idx].can_id, g_ctx.cache[idx].channel);
            }
            idx++;
        }
    }
    g_ctx.cache_count = idx;

    pthread_mutex_unlock(&g_ctx.hash_mutex);

    TLY_LOG_I("CAN_DATA: filter updated, %u entries (fast=%u slow=%u)",
              g_ctx.cache_count, canid_cache->fast_count, canid_cache->slow_count);
}

/* ======================== 数据查找 ======================== */

int tsp_can_data_lookup(uint32_t can_id, uint8_t channel,
                         uint8_t *data_out, uint8_t *dlc_out,
                         uint64_t current_time_ms)
{
    if (!g_ctx.initialized || !g_ctx.cache || !data_out) {
        return -1;
    }

    /* 加锁保护哈希表查找 */
    pthread_mutex_lock(&g_ctx.hash_mutex);
    uint16_t idx = hash_table_lookup(can_id, channel);
    pthread_mutex_unlock(&g_ctx.hash_mutex);
    
    if (idx == HASH_ENTRY_EMPTY) {
        return -1;  /* 未找到 */
    }
    
    can_cache_entry_t *entry = &g_ctx.cache[idx];
    uint8_t dlc = 0;
    
    /* 如果有缓存数据，检查是否过期 */
    if (entry->count > 0) {
        /* 获取最新一帧（write_idx 的上一位置） */
        uint16_t latest_idx = entry->write_idx == 0 ?
                              CAN_HISTORY_FRAME_COUNT - 1 :
                              entry->write_idx - 1;
        can_history_frame_t *frame = &entry->history[latest_idx];
        
        /* 检查数据是否过期（超过10秒未更新） */
        if (current_time_ms == 0 ||
            (current_time_ms >= frame->timestamp_ms &&
             (current_time_ms - frame->timestamp_ms) <= CAN_DATA_TIMEOUT_MS)) {
            /* 数据有效，复制到输出 */
            memcpy(data_out, frame->data, 8);
            dlc = frame->dlc;
            if (dlc_out) {
                *dlc_out = dlc;
            }
            return 0;
        }
    }
    
    /* 无数据或数据已过期，返回空 */
    memset(data_out, 0, 8);
    if (dlc_out) {
        *dlc_out = 0;
    }
    return 0;
}

/* ======================== 数据持久化 ======================== */

/**
 * @brief 将历史帧写入BLF文件
 * @param writer BLF writer
 * @param entry CAN ID缓存条目
 * @return 写入的帧数，失败返回-1
 */
static int write_entry_to_blf(tly_blf_writer_t *writer, can_cache_entry_t *entry)
{
    uint32_t frames_written = 0;
    
    for (uint16_t j = 0; j < entry->count; j++) {
        uint16_t idx;
        if (entry->count < CAN_HISTORY_FRAME_COUNT) {
            /* 缓冲区未满，从0开始 */
            idx = j;
        } else {
            /* 缓冲区已满，从 write_idx 开始 */
            idx = (entry->write_idx + j) % CAN_HISTORY_FRAME_COUNT;
        }
        
        can_history_frame_t *hist_frame = &entry->history[idx];
        
        /* 转换为BLF格式 */
        tly_can_frame_t blf_frame = {
            .id = entry->can_id,
            .channel = entry->channel,
            .dlc = hist_frame->dlc,
            .flags = TLY_CAN_FLAG_EXT,  /* 扩展帧标志，用于BLF解析器识别29位ID */
            .timestamp_us = hist_frame->timestamp_ms * 1000ULL  /* ms转us */
        };
        memcpy(blf_frame.data, hist_frame->data, 8);
        
        /* 写入BLF文件 */
        if (tly_blf_write_can(writer, &blf_frame) != 0) {
            TLY_LOG_E("CAN_DATA: blf_write_can failed");
            return -1;
        }
        frames_written++;
    }
    
    return frames_written;
}

int tsp_can_data_save_to_file(const char *filename)
{
    if (!g_ctx.initialized || !g_ctx.cache || g_ctx.cache_count == 0) {
        TLY_LOG_W("CAN_DATA: save_to_file, not initialized or empty cache");
        return -1;
    }

    if (!filename) {
        TLY_LOG_E("CAN_DATA: save_to_file, invalid filename");
        return -1;
    }

    /* 使用BLF writer打开文件 */
    tly_blf_writer_t *writer = tly_blf_writer_open(filename);
    if (!writer) {
        TLY_LOG_E("CAN_DATA: save_to_file, blf_writer_open failed: %s", filename);
        return -1;
    }

    uint32_t total_frames = 0;

    /* 写入每个CAN ID的历史帧数据（按时间顺序） */
    for (uint16_t i = 0; i < g_ctx.cache_count; i++) {
        int ret = write_entry_to_blf(writer, &g_ctx.cache[i]);
        if (ret < 0) {
            tly_blf_writer_close(writer);
            return -1;
        }
        total_frames += ret;
    }

    long file_size = tly_blf_writer_size(writer);
    tly_blf_writer_close(writer);
    
    TLY_LOG_I("CAN_DATA: saved BLF to %s, %u CAN IDs, %u frames, size=%ld bytes",
              filename, g_ctx.cache_count, total_frames, file_size);
    return 0;
}

void tsp_can_data_clear_cache(void)
{
    if (!g_ctx.initialized || !g_ctx.cache) {
        return;
    }

    for (uint16_t i = 0; i < g_ctx.cache_count; i++) {
        can_cache_entry_t *entry = &g_ctx.cache[i];
        
        /* 清空历史帧 */
        memset(entry->history, 0, sizeof(entry->history));
        entry->write_idx = 0;
        entry->count = 0;
    }

    g_ctx.last_sample_ms = tly_monotonic_time_ms();
}

int tsp_can_data_trigger_save(const char *filename)
{
    int ret = tsp_can_data_save_to_file(filename);
    if (ret == 0) {
        tsp_can_data_clear_cache();
    }
    return ret;
}

/* ======================== 故障存储（前后5分钟） ======================== */

/** @brief 故障存储目录大小上限：512MB */
#define FAULT_STORAGE_MAX_SIZE (512ULL * 1024 * 1024)

/** @brief 故障存储目录 */
#define FAULT_STORAGE_DIR "/mnt/sdcard/fault_data"

/** @brief 故障文件名最大长度 */
#define FAULT_FILENAME_MAX_SIZE 256

/** @brief 故障文件名格式: fault-序号-时间戳.blf */
#define FAULT_FILENAME_FORMAT "fault-%010" PRIu64 "-%s.blf"
#define FAULT_FILENAME_PARSE_FORMAT "fault-%010" PRIu64 "-%*s.blf"

/** @brief 故障存储上下文（封装相关状态，提高内聚性） */
typedef struct {
    fault_storage_state_t state;        /**< 当前状态：空闲/记录中 */
    timer_t               post_timer;   /**< 后5分钟保存定时器 */
    char                  post_filename[256]; /**< 后5分钟数据文件名 */
    pthread_mutex_t       mutex;        /**< 状态保护锁（线程安全） */
    fault_storage_idx_t   idx;          /**< 轻量索引 */
    char                  baseDir[256]; /**< 存储目录 */
} fault_storage_ctx_t;

/** @brief 故障存储全局上下文（静态初始化） */
static fault_storage_ctx_t g_fault_ctx = {
    .state         = FAULT_STORAGE_IDLE,
    .post_timer    = NULL,
    .post_filename = {0},
    .mutex         = PTHREAD_MUTEX_INITIALIZER,
    .idx           = {0, 0, 0, 0},
    .baseDir       = FAULT_STORAGE_DIR
};

/** @brief 获取文件大小 */
static uint64_t get_file_size(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    return (uint64_t)st.st_size;
}

/** @brief 检查并创建目录 */
static int check_and_create_directory(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        if (mkdir(path, 0755) != 0 && errno != EEXIST) {
            TLY_LOG_E("CAN_DATA: failed to create directory %s: %s", path, strerror(errno));
            return -1;
        }
    }
    return 0;
}

/** @brief 获取当前时间戳字符串（用于文件名） */
static void get_timestamp_str(char *buf, size_t bufSize)
{
    time_t now = time(NULL);
    struct tm tm_buf;
    localtime_r(&now, &tm_buf);
    strftime(buf, bufSize, "%Y%m%d%H%M%S", &tm_buf);
}

/** @brief 重建轻量索引（启动时或需要时调用）
 * 遍历目录，统计文件大小、数量，找出最旧和最新序号
 */
static int32_t fault_storage_rebuild_idx(fault_storage_ctx_t *ctx)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    uint64_t index = 0;
    char filePath[512] = {0};
    
    if (ctx == NULL || ctx->baseDir[0] == '\0') {
        return -1;
    }
    
    pthread_mutex_lock(&ctx->mutex);
    
    /* 重置索引 */
    ctx->idx.maxIndex = 0;
    ctx->idx.oldestIndex = UINT64_MAX;
    ctx->idx.totalSize = 0;
    ctx->idx.fileCount = 0;
    
    /* 打开目录 */
    dir = opendir(ctx->baseDir);
    if (dir == NULL) {
        /* 目录不存在，尝试创建 */
        if (check_and_create_directory(ctx->baseDir) != 0) {
            pthread_mutex_unlock(&ctx->mutex);
            return -1;
        }
        dir = opendir(ctx->baseDir);
        if (dir == NULL) {
            pthread_mutex_unlock(&ctx->mutex);
            return -1;
        }
    }
    
    /* 遍历目录 */
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        /* 解析文件名: fault-序号-时间戳.blf */
        if (sscanf(entry->d_name, FAULT_FILENAME_PARSE_FORMAT, &index) == 1) {
            snprintf(filePath, sizeof(filePath), "%s/%s", ctx->baseDir, entry->d_name);
            uint64_t size = get_file_size(filePath);
            
            ctx->idx.totalSize += size;
            ctx->idx.fileCount++;
            
            if (index > ctx->idx.maxIndex) {
                ctx->idx.maxIndex = index;
            }
            if (index < ctx->idx.oldestIndex) {
                ctx->idx.oldestIndex = index;
            }
        }
    }
    
    closedir(dir);
    
    /* 目录为空时，oldestIndex置为0 */
    if (ctx->idx.fileCount == 0) {
        ctx->idx.oldestIndex = 0;
    }
    
    TLY_LOG_I("CAN_DATA: fault storage idx rebuilt: maxIndex=%" PRIu64
              ", oldestIndex=%" PRIu64 ", totalSize=%" PRIu64 " bytes, fileCount=%u",
              ctx->idx.maxIndex, ctx->idx.oldestIndex, ctx->idx.totalSize, ctx->idx.fileCount);
    
    pthread_mutex_unlock(&ctx->mutex);
    return 0;
}

/** @brief 清理最旧的文件，直到总大小低于上限
 * 返回：删除的文件数；出错返回-1
 */
static int32_t fault_storage_cleanup_oldest(fault_storage_ctx_t *ctx)
{
    char pattern[512] = {0};
    char filePath[512] = {0};
    glob_t g;
    int globRet = 0;
    uint32_t deleted = 0;
    uint64_t index = 0;
    size_t k = 0;
    
    if (ctx == NULL || ctx->baseDir[0] == '\0') {
        return -1;
    }
    
    pthread_mutex_lock(&ctx->mutex);
    
    /* 如果未超过上限，直接返回 */
    if (ctx->idx.totalSize <= FAULT_STORAGE_MAX_SIZE) {
        pthread_mutex_unlock(&ctx->mutex);
        return 0;
    }
    
    TLY_LOG_W("CAN_DATA: fault storage cleanup start: totalSize=%" PRIu64
              " bytes, maxSize=%" PRIu64 " bytes, oldestIndex=%" PRIu64,
              ctx->idx.totalSize, FAULT_STORAGE_MAX_SIZE, ctx->idx.oldestIndex);
    
    /* 持续删除最旧序号的文件，直到总大小低于上限 */
    while (ctx->idx.totalSize > FAULT_STORAGE_MAX_SIZE &&
           ctx->idx.oldestIndex <= ctx->idx.maxIndex) {
        
        /* 构造glob模式查找最旧序号的文件 */
        snprintf(pattern, sizeof(pattern), "%s/fault-%010" PRIu64 "-*.blf",
                 ctx->baseDir, ctx->idx.oldestIndex);
        
        memset(&g, 0, sizeof(g));
        globRet = glob(pattern, 0, NULL, &g);
        
        if (globRet == 0 && g.gl_pathc > 0) {
            /* 删除匹配的文件 */
            for (k = 0; k < g.gl_pathc; k++) {
                uint64_t size = get_file_size(g.gl_pathv[k]);
                
                if (remove(g.gl_pathv[k]) == 0) {
                    ctx->idx.totalSize -= (size <= ctx->idx.totalSize) ? size : ctx->idx.totalSize;
                    if (ctx->idx.fileCount > 0) {
                        ctx->idx.fileCount--;
                    }
                    deleted++;
                    TLY_LOG_W("CAN_DATA: removed old fault file: %s, size=%" PRIu64
                              ", totalSize now=%" PRIu64,
                              g.gl_pathv[k], size, ctx->idx.totalSize);
                } else {
                    TLY_LOG_E("CAN_DATA: failed to remove file %s: %s",
                              g.gl_pathv[k], strerror(errno));
                }
            }
        }
        
        if (g.gl_pathc > 0) {
            globfree(&g);
        }
        
        /* 前进到下一个序号 */
        ctx->idx.oldestIndex++;
        
        /* 防止无限循环 */
        if (ctx->idx.oldestIndex > ctx->idx.maxIndex + 1000) {
            TLY_LOG_E("CAN_DATA: cleanup loop protection triggered");
            break;
        }
    }
    
    TLY_LOG_I("CAN_DATA: fault storage cleanup done: deleted %u files, "
              "totalSize=%" PRIu64 " bytes, oldestIndex=%" PRIu64,
              deleted, ctx->idx.totalSize, ctx->idx.oldestIndex);
    
    pthread_mutex_unlock(&ctx->mutex);
    return (int32_t)deleted;
}

/** @brief 生成故障文件名（带序号和时间戳） */
static void fault_storage_generate_filename(fault_storage_ctx_t *ctx,
                                             char *filename, size_t filenameSize,
                                             uint64_t index)
{
    char timestamp[32] = {0};
    get_timestamp_str(timestamp, sizeof(timestamp));
    snprintf(filename, filenameSize, FAULT_FILENAME_FORMAT, index, timestamp);
}

/** @brief 启动延迟保存定时器（5分钟后触发） */
static int start_post_fault_timer(void (*callback)(union sigval), timer_t *timerid)
{
    struct sigevent sev;
    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_value.sival_ptr = NULL;
    sev.sigev_notify_function = callback;
    sev.sigev_notify_attributes = NULL;
    
    if (timer_create(CLOCK_MONOTONIC, &sev, timerid) != 0) {
        TLY_LOG_E("CAN_DATA: timer_create failed");
        return -1;
    }
    
    struct itimerspec its;
    its.it_value.tv_sec = 5 * 60;  /* 5分钟 */
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;     /* 只触发一次 */
    its.it_interval.tv_nsec = 0;
    
    if (timer_settime(*timerid, 0, &its, NULL) != 0) {
        TLY_LOG_E("CAN_DATA: timer_settime failed");
        timer_delete(*timerid);
        *timerid = NULL;
        return -1;
    }
    
    return 0;
}

/** @brief 保存故障数据到文件（带索引更新和自动清理）
 * 返回：0成功，-1失败
 */
static int fault_storage_save_with_idx(fault_storage_ctx_t *ctx)
{
    char filename[256] = {0};
    char fullpath[512] = {0};
    int ret = 0;
    
    if (ctx == NULL) {
        return -1;
    }
    
    /* 确保目录存在 */
    if (check_and_create_directory(ctx->baseDir) != 0) {
        return -1;
    }
    
    pthread_mutex_lock(&ctx->mutex);
    
    /* 生成带序号的文件名 */
    ctx->idx.maxIndex++;
    fault_storage_generate_filename(ctx, filename, sizeof(filename), ctx->idx.maxIndex);
    snprintf(fullpath, sizeof(fullpath), "%s/%s", ctx->baseDir, filename);
    
    /* 保存post_filename用于定时器回调 */
    strncpy(ctx->post_filename, fullpath, sizeof(ctx->post_filename) - 1);
    ctx->post_filename[sizeof(ctx->post_filename) - 1] = '\0';
    
    pthread_mutex_unlock(&ctx->mutex);
    
    /* 保存数据到文件 */
    ret = tsp_can_data_save_to_file(fullpath);
    if (ret != 0) {
        TLY_LOG_E("CAN_DATA: failed to save fault data to %s", fullpath);
        return -1;
    }
    
    /* 更新索引 */
    pthread_mutex_lock(&ctx->mutex);
    
    uint64_t newSize = get_file_size(fullpath);
    ctx->idx.totalSize += newSize;
    ctx->idx.fileCount++;
    
    /* 第一个文件或oldestIndex为0时，设置oldestIndex */
    if (ctx->idx.fileCount == 1 || ctx->idx.oldestIndex == 0) {
        ctx->idx.oldestIndex = ctx->idx.maxIndex;
    }
    
    TLY_LOG_I("CAN_DATA: fault data saved: %s, size=%" PRIu64
              ", totalSize=%" PRIu64 ", fileCount=%u",
              filename, newSize, ctx->idx.totalSize, ctx->idx.fileCount);
    
    pthread_mutex_unlock(&ctx->mutex);
    
    /* 检查并清理旧文件（如果超过512M） */
    if (ctx->idx.totalSize > FAULT_STORAGE_MAX_SIZE) {
        fault_storage_cleanup_oldest(ctx);
    }
    
    return 0;
}

/** @brief 定时器回调：保存后5分钟 */
static void fault_timer_callback(union sigval val)
{
    (void)val;
    
    pthread_mutex_lock(&g_fault_ctx.mutex);
    
    if (g_fault_ctx.state != FAULT_STORAGE_RECORDING) {
        pthread_mutex_unlock(&g_fault_ctx.mutex);
        return;
    }
    
    pthread_mutex_unlock(&g_fault_ctx.mutex);
    
    TLY_LOG_I("CAN_DATA: saving post-fault data to %s", g_fault_ctx.post_filename);
    
    /* 使用新的保存函数，支持索引更新和自动清理 */
    int ret = fault_storage_save_with_idx(&g_fault_ctx);
    if (ret == 0) {
        TLY_LOG_I("CAN_DATA: fault storage completed");
    } else {
        TLY_LOG_E("CAN_DATA: failed to save post-fault data");
    }
    
    pthread_mutex_lock(&g_fault_ctx.mutex);
    g_fault_ctx.state = FAULT_STORAGE_IDLE;
    g_fault_ctx.post_timer = NULL;
    pthread_mutex_unlock(&g_fault_ctx.mutex);
}

int tsp_can_data_start_fault_storage(const char *base_filename)
{
    /* base_filename可以为NULL，使用内部自动生成的文件名 */
    (void)base_filename;  /* 暂时忽略传入的文件名，使用内部生成的序号文件名 */
    
    /* 首次调用时重建索引 */
    static int idx_initialized = 0;
    if (!idx_initialized) {
        fault_storage_rebuild_idx(&g_fault_ctx);
        idx_initialized = 1;
    }
    
    pthread_mutex_lock(&g_fault_ctx.mutex);
    
    /* 检查是否已在记录中 */
    if (g_fault_ctx.state == FAULT_STORAGE_RECORDING) {
        pthread_mutex_unlock(&g_fault_ctx.mutex);
        TLY_LOG_W("CAN_DATA: fault storage already in progress");
        return -2;
    }
    
    pthread_mutex_unlock(&g_fault_ctx.mutex);
    
    /* 1. 立即保存前5分钟（使用新的保存函数） */
    TLY_LOG_I("CAN_DATA: saving pre-fault data");
    int ret = fault_storage_save_with_idx(&g_fault_ctx);
    if (ret != 0) {
        TLY_LOG_E("CAN_DATA: failed to save pre-fault data");
        return -1;
    }
    
    pthread_mutex_lock(&g_fault_ctx.mutex);
    
    /* 2. 设置5分钟定时器保存后5分钟 */
    if (start_post_fault_timer(fault_timer_callback, &g_fault_ctx.post_timer) != 0) {
        /* 定时器启动失败，只保存前5分钟数据，后5分钟不保存 */
        TLY_LOG_W("CAN_DATA: failed to start post-fault timer, only pre-fault data saved");
        pthread_mutex_unlock(&g_fault_ctx.mutex);
        return 0;  /* 前5分钟已成功保存，返回成功 */
    }
    
    g_fault_ctx.state = FAULT_STORAGE_RECORDING;
    TLY_LOG_I("CAN_DATA: fault storage started, will save post data in 5 minutes");
    
    pthread_mutex_unlock(&g_fault_ctx.mutex);
    
    return 0;
}

fault_storage_state_t tsp_can_data_get_fault_storage_state(void)
{
    pthread_mutex_lock(&g_fault_ctx.mutex);
    fault_storage_state_t state = g_fault_ctx.state;
    pthread_mutex_unlock(&g_fault_ctx.mutex);
    return state;
}

