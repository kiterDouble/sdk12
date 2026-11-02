#ifndef __DOIP_ENTITY_H__
#define __DOIP_ENTITY_H__
#include <stdint.h>
//#include "entity_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif

#define DOIP_ENTITY_SUPPORT_TESTER_MAX (10)
#define DOIP_ENTITY_SUPPORT_PAYLOAD_lENGTH_MAX (0x40000U)

typedef struct
{
    uint32_t activeType;//tester请求类型，参考13400文档
    uint32_t testerNum;//tester数量
    uint32_t testerLas[DOIP_ENTITY_SUPPORT_TESTER_MAX];//支持的tester逻辑地址
    uint32_t entityLa;//逻辑地址
    uint32_t functionLa;//功能地址
    uint8_t eid[6];//EID
    uint8_t gid[6];//GID    
    uint8_t vin[17];//VIN
}DoipEntityCfg_t;

typedef void* DoipEntityHandle_t;//节点句柄

/**
 * @name UdsRequestCallBack
 * @brief tester uds请求回调
 * @param testerLa:输入参数，tester的逻辑地址
 * @param udsData:输入参数，uds请求数据
 * @param dataLen:输入参数，udsData长度
 * @return 函数无返回值
 */
typedef void (*UdsRequestCallBack)(uint32_t testerLa, const uint8_t *udsData, uint32_t dataLen);

/**
 * @name DoipEntityInit
 * @brief DOIP实体初始化
 *        本函数用于初始化DOIP实体，配置实体的基本属性、事件处理机制，并启动相关的线程。
 * @param hdl 指向DoipEntityHandle_t的指针，用于返回初始化后的实体句柄。
 * @param cfg 指向DoipEntityCfg_t的常量指针，提供实体的配置信息。
 * @param reqCb UdsRequestCallBack类型的回调函数，用于处理UDS请求。
 * @return 返回RET_OK表示成功，其他错误码表示失败。
 */
int32_t DoipEntityInit(DoipEntityHandle_t *hdl, 
                    const DoipEntityCfg_t *cfg, 
                    UdsRequestCallBack reqCb);

/**
 * @name DoipEntityUdsResponse
 * @brief 处理UDS（UDS诊断服务）响应的函数。
 * @param hdl Doip实体的句柄，用于标识特定的DOIP实体。
 * @param testerLa 测试设备地址，用于标识发送UDS请求的测试者。
 * @param udsData 包含UDS响应数据的缓冲区。
 * @param dataLen UDS响应数据的长度。
 * @return 函数执行状态，如果执行成功返回RET_OK，否则返回RET_ERR。
 */
int32_t DoipEntityUdsResponse(DoipEntityHandle_t hdl, 
                            uint32_t testerLa, 
                            const uint8_t *udsData, 
                            uint32_t dataLen);

/**
 * @name DoipEntityDeinit
 * @brief 对DOIP实体进行反初始化
 *        该函数用于释放DOIP实体在初始化时分配的资源，包括诊断模块、注释模块的资源释放，以及事件系统的清理。
 * @param hdl DoipEntityHandle_t类型，DOIP实体的句柄，用于标识和访问具体的实体。
 * @return 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t DoipEntityDeinit(DoipEntityHandle_t hdl);
#ifdef __cplusplus
}
#endif

#endif