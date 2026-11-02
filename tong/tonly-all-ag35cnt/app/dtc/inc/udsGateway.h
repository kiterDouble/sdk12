#ifndef UDS_GATEWAY_H
#define UDS_GATEWAY_H

/* ========================================================================== */
/*  文件名称  : uds_gateway.h                                                 */
/*  模块名称  : UDS ServiceData 网关（依赖外部 MCU 服务）                       */
/*  版本      : v1.0.0                                                         */
/*  作者      : <SIRUN HF / Jaxsen>                                           */
/*  日期      : 2025-09-09                                                     */
/* -------------------------------------------------------------------------- */
/*  摘要说明  :                                                                */
/*    - 接收 MCU 服务已解析的消息 (ServiceData)；                              */
/*    - 判断 0x22/0x2E，调用 UdsDidRead/UdsDidWrite；                           */
/*    - 打包 62/6E 或 7F 的 ServiceData 并通过 McuSrvSendMesg 发送响应。         */
/*                                                                            */
/*  关键约定  :                                                                */
/*    - McuMesg_t.payload 指向的就是 UDS ServiceData（首字节=SID）。             */
/*    - NRC 规范：0=成功，非0为 UDS 否定应答码（你方 DID 层需遵守）。             */
/* ========================================================================== */

#include <stdint.h>
#include <stddef.h>
#include <glib.h>
#include "mcu_api.h"

/**
 * @brief 向 MCU 发送消息。由你方服务提供实现。
 * @return 0 成功；<0 失败
 */
int32_t McuSrvSendMesg(const McuMesg_t *msg);

/* ========== UDS 常量（本模块用到的子集） ========== */
typedef enum
{
    UdsSidReadRequest_e       = 0x22u,  /**< ReadDataByIdentifier 请求 */
    UdsSidWriteRequest_e      = 0x2Eu,  /**< WriteDataByIdentifier 请求 */
    UdsSidReadPositive_e      = 0x62u,  /**< ReadDataByIdentifier 响应 */
    UdsSidWritePositive_e     = 0x6Eu,  /**< WriteDataByIdentifier 响应 */
    UdsSidNegativeResponse_e  = 0x7Fu,  /**< 负响应 */

    UdsSidReadDtcInfo_e              = 0x19u,  /**< ReadDTCInformation 请求 */
    UdsSidRoutineControlRequest_e    = 0x31u   /**< RoutineControl 请求 */
} UdsSid_e;

/* ========== 返回码（网关内部） ========== */
/**
 * @enum  UdsRet_e
 * @brief 网关内部函数的统一返回值枚举。
 *
 * 说明：
 * - 这些返回码仅在 **网关内部函数之间** 使用，用来标记本地执行结果。
 * - 对外给 MCU 的诊断响应不使用这些枚举，而是通过 NRC (0x11/0x13/0x31/0x74...)。
 */
typedef enum
{
    UdsRetOk_e     =  0,  /**< 成功：
                           *   - 函数正常完成；
                           *   - 无论最终打包的是正响应还是否定响应，
                           *     只要能正确生成到缓冲区里，就算成功。 */

    UdsRetErr_e    = -1, /**< 一般错误：
                           *   - 内部逻辑出错；
                           *   - 或调用了某个外部依赖失败（非参数问题）。 */

    UdsRetEinval_e = -2, /**< 参数无效：
                           *   - 传入指针为空；
                           *   - 长度字段非法；
                           *   - 等明显的入参错误。 */

    UdsRetEbuf_e   = -3  /**< 缓冲不足：
                           *   - 生成响应时缓冲区空间不够；
                           *   - 连最小的否定响应 (7F) 都装不下。 */
} UdsRet_e;

/* ========== 配置与句柄 ========== */
typedef struct
{
    SYSQueMsgType_e  reqType;               /* “下行诊断请求” 消息类型 */
    SYSQueMsgType_e  respType;              /* “上行诊断响应” 消息类型 */
    gboolean         useExtendedNegWithDid; /* TRUE: 内部 7F 带 DID；FALSE: 标准 7F 三字节 */
    uint32_t         maxServiceData;        /* 上行响应缓冲上限（字节） */
} UdsGatewayConfig_t;

typedef struct
{
    UdsGatewayConfig_t cfg;
} UdsGatewayHandle_t;

/* ========== 对外 API ========== */

/**
 * @brief  初始化网关（填入配置即可；无 I/O 资源占用）。
 */
void     UdsGatewayInit(UdsGatewayHandle_t *handle, const UdsGatewayConfig_t *cfg);

/**
 * @brief  处理一条来自 MCU 服务的“下行诊断请求”消息，并立即通过 McuSrvSendMesg 回响应。
 * @param  handle   网关句柄
 * @param  inMsg    MCU 服务传入的消息（payload=ServiceData）
 * @return UdsRetOk_e 表示已尝试生成并发送响应；负值为本地错误（参数/内存等）
 */
UdsRet_e UdsGatewayOnMcuMessage(UdsGatewayHandle_t *handle, const McuMesg_t *inMsg);

#endif /* UDS_GATEWAY_H */
