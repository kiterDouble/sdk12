#ifndef __ENTITY_ANNOUNCE_H__
#define __ENTITY_ANNOUNCE_H__
#include <event2/event.h>
#include "doip_entity.h"
#include "message_proc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* EntityAnnoHandle_t;

/**
 * @brief 实体announce初始化函数。
 * @param hdl 指向实体注解句柄的指针，初始化后将包含操作实体注解所需的所有资源。
 * @param cfg 指向DOIP实体配置的指针，用于配置实体注解的行为。
 * @param base 指向事件基础结构的指针，用于管理实体注解的事件处理。
 * @param establisted 指向一个布尔值的指针，初始化后将指示实体注解是否已建立连接。
 * @return 成功返回RET_OK，否则返回RET_ERR。
 */
int32_t EntityAnnoInit(EntityAnnoHandle_t *hdl, 
                    const DoipEntityCfg_t *cfg, 
                    struct event_base *base, 
                    uint8_t *establisted);

/**
 * @name EntityAnnoDeinit
 * @brief 实体announce反初始化函数
 *        该函数用于释放实体注解相关的资源，包括事件和文件描述符等。
 * @param hdl 实体注解句柄，用于标识特定的实体注解实例
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t EntityAnnoDeinit(EntityAnnoHandle_t hdl);
#ifdef __cplusplus
}
#endif
#endif