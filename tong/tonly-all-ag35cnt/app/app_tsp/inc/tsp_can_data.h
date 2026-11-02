/**
 * tsp_can_data.h - CAN原始报文采集与缓存模块
 *
 * 功能:
 * 1. 订阅 SYS_QUE_TYPE_MCU_VEHICLE_CAN_RAW 队列
 * 2. 根据平台下发的 0x7B CANID列表(fast_ids)筛选需要采集的CAN帧
 * 3. 缓存每个CAN ID的最新一帧数据(8字节)
 * 4. 1秒限频采样, 避免高频回调冲击
 * 5. 提供查找接口供实时上报组包使用
 * 6. 【扩展】支持缓存320秒历史数据（每秒1帧，共320帧）
 */

#ifndef __TSP_CAN_DATA_H__
#define __TSP_CAN_DATA_H__

#include "tsp_protocol.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== 配置常量 ======================== */

/** @brief 历史缓存时长（秒） */
#define CAN_HISTORY_DURATION_SEC    320

/** @brief 每秒采样次数 */
#define CAN_SAMPLES_PER_SEC         1

/** @brief 每个CAN ID缓存的帧数 */
#define CAN_HISTORY_FRAME_COUNT     (CAN_HISTORY_DURATION_SEC * CAN_SAMPLES_PER_SEC)

/* ======================== 生命周期 ======================== */

/**
 * @brief 初始化CAN数据采集模块
 * @details 订阅 SYS_QUE_TYPE_MCU_VEHICLE_CAN_RAW, 注册筛选回调
 * @param canid_cache  平台下发的CAN ID列表(来自0x7B), 用于筛选过滤
 * @return 0 成功, 负数 失败
 */
int  tsp_can_data_init(void);

/**
 * @brief 反初始化CAN数据采集模块
 */
void tsp_can_data_deinit(void);

/* ======================== 筛选更新 ======================== */

/**
 * @brief 更新CAN ID筛选列表(平台下发新的0x7B时调用)
 * @param canid_cache  新的CAN ID列表
 */
void tsp_can_data_update_filter(const tly_canid_list_resp_t *canid_cache);

/* ======================== 数据查找（实时） ======================== */

/**
 * @brief 查找指定CAN ID的最新缓存数据
 * @param can_id   CAN标识符
 * @param channel  CAN通道号
 * @param data_out 输出缓冲区(8字节), 存放最新CAN报文数据
 * @param dlc_out  输出实际数据长度(可为NULL)
 * @param current_time_ms 当前时间戳(毫秒),用于判断数据是否过期,为0时不检查超时
 * @return 0 找到缓存数据, -1 未找到
 * @note 如果数据超过10秒未更新,dlc_out将返回0
 */
int  tsp_can_data_lookup(uint32_t can_id, uint8_t channel,
                         uint8_t *data_out, uint8_t *dlc_out,
                         uint64_t current_time_ms);

/* ======================== 数据持久化 ======================== */

/**
 * @brief 将当前缓存的所有CAN数据保存到文件
 * @param filename  文件路径
 * @return 0 成功, -1 失败
 * @note 保存格式为二进制，包含文件头+数据块
 */
int  tsp_can_data_save_to_file(const char *filename);

/**
 * @brief 清空所有缓存数据（重新开始采集）
 * @note 清空后count归零，write_idx归零，但CAN ID列表保留
 */
void tsp_can_data_clear_cache(void);
/**
 * @brief 触发保存并清空缓存（原子操作）
 * @param filename  文件路径
 * @return 0 成功, -1 失败
 * @note 先保存到文件，成功后清空缓存
 */
int  tsp_can_data_trigger_save(const char *filename);

/* ======================== 故障存储（前后5分钟） ======================== */

/**
 * @brief 故障存储状态
 */
typedef enum {
    FAULT_STORAGE_IDLE = 0,      /**< 空闲状态 */
    FAULT_STORAGE_RECORDING = 1, /**< 正在记录（已保存前5分钟，等待后5分钟） */
} fault_storage_state_t;

/**
 * @brief 故障存储轻量索引结构
 * @note 内存中维护，避免频繁遍历目录
 */
typedef struct {
    uint64_t maxIndex;      /**< 当前最大序号 */
    uint64_t oldestIndex;   /**< 当前最旧序号，用于清理 */
    uint64_t totalSize;     /**< 目录总大小（字节） */
    uint32_t fileCount;     /**< 文件总数 */
} fault_storage_idx_t;

/**
 * @brief 启动故障存储（保存前后5分钟）
 * @param base_filename  保留参数，可为NULL（文件名自动生成：fault-序号-时间戳.blf）
 * @return 0 成功启动, -1 失败, -2 已在记录中
 * @note 调用后立即保存前5分钟，5分钟后自动保存后5分钟
 * @note 支持512M存储上限，满了自动删除旧文件
 */
int  tsp_can_data_start_fault_storage(const char *base_filename);

/**
 * @brief 获取当前故障存储状态
 * @return 当前状态
 */
fault_storage_state_t tsp_can_data_get_fault_storage_state(void);



#ifdef __cplusplus
}
#endif

#endif /* __TSP_CAN_DATA_H__ */
