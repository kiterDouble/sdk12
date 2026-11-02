/**
 * tsp_vin.h - VIN数据采集与缓存模块
 *
 * 功能:
 * 1. 仅过滤 can_id=0x18FEEC10 和 can_id=0x18F117D1 的CAN帧
 * 2. 缓存VIN帧数据
 * 3. 1秒限频采样
 * 4. 检测 can_id_2 的触发字节，比对 VCU VIN 与 TBOX VIN
 */

#ifndef __TSP_VIN_H__
#define __TSP_VIN_H__

#include <stdint.h>
#include <stdbool.h>
#include "tsp_app.h"

#define VIN_CAN1_VIN_LENGTH 17  /* VIN字符串长度 */

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== VIN处理函数 ======================== */

/**
 * @brief 检查VIN是否全为0（空数据）
 * @param vin VIN字符串
 * @param len 字符串长度
 * @return true 全为0, false 非全0
 */
bool vin_is_all_zero(const char *vin, uint8_t len);

/**
 * @brief 检查CAN ID是否为VIN相关ID
 */
bool vin_is_vin_can_id(uint32_t can_id);

/**
 * @brief 更新VIN实时数据
 */
void vin_update_frame(uint32_t can_id, uint8_t channel, const uint8_t *data, uint8_t dlc);

/* ======================== 生命周期 ======================== */

/**
 * @brief 初始化VIN数据采集模块
 * @param app  应用上下文
 * @return 0 成功, 负数 失败
 */
int tsp_vin_data_init(tly_app_t *app);

/**
 * @brief 反初始化VIN数据采集模块
 */
void tsp_vin_data_deinit(void);


#ifdef __cplusplus
}
#endif

#endif /* __TSP_VIN_H__ */
