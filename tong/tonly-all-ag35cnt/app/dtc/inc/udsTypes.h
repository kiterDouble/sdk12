/* ========================================================================== */
/*  文件名称  : uds_types.h                                                   */
/*  模块名称  : UDS 基础类型与常量定义                                         */
/*  版本      : v1.0.0                                                         */
/*  作者      : <SIRUN HF / Jaxsen>                                           */
/*  创建日期  : 2025-09-01                                                     */
/* -------------------------------------------------------------------------- */
/*  摘要说明  :                                                                */
/*    本头文件定义 UDS（Unified Diagnostic Services）相关的基础类型、常量与枚举：   */
/*    - 存储类型枚举（RAM/DFLASH/EEPROM 等抽象层）；                             */
/*    - DID 访问权限掩码；                                                      */
/*    - 常用的 UDS 负响应码（NRC）；                                            */
/*    - 安全等级占位枚举；                                                      */
/*    - 本项目所涉及的 DID 枚举表（覆盖数据库/协议字段）。                         */
/*                                                                              */
/*  设计要点  :                                                                */
/*    - 所有常量均以 `u` 后缀标注，遵循 MISRA C:2012 规范；                        */
/*    - DID 值统一由枚举 UDS_DID_IDS_e 提供，避免硬编码；                          */
/*    - NRC 返回码与诊断服务负响应对齐（0 表示成功，非 0 为错误码）。               */
/*                                                                              */
/*  使用约定  :                                                                */
/*    - 在数据库注册表 (did_registry.c) 与默认值表 (did_defaults.c) 中引用此枚举；  */
/*    - 外部函数接口 (UdsDidRead/UdsDidWrite) 使用 UDS_DID_IDS_e 标识 DID；         */
/*    - 存储类型 uds_store_t 仅作为抽象层，项目中可做裁剪。                          */
/*                                                                              */
/*  变更记录  :                                                                */
/*    v1.0.0  初版                                                             */
/* ========================================================================== */

#ifndef UDS_TYPES_H_
#define UDS_TYPES_H_

/* ================================== 依赖 =================================== */
#include <stdint.h>
#include "udsCfg.h"   /* 长度宏：LEN_VIN/LEN_IPv4_STR 等 */

/* ============================== 存储类型枚举 ============================== */
/* 注：如存储类型枚举未使用，可在实现中裁剪。 */
typedef enum
{
    UDS_STORE_RAM = 0,     /* 存放于 RAM，仅运行时有效 */
    UDS_STORE_DFLASH,      /* 存放于 Data Flash，掉电保持 */
    UDS_STORE_EEPROM       /* 存放于 EEPROM，掉电保持 */
} uds_store_t;

/* ============================== DID 访问权限 ============================== */
#define UDS_ACCESS_R    (1u << 0)   /* 可读 */
#define UDS_ACCESS_W    (1u << 1)   /* 可写 */
#define UDS_ACCESS_RW   (UDS_ACCESS_R | UDS_ACCESS_W) /* 可读写 */

/* ============================== 负响应码枚举 ============================== */
/* 说明：仅列出项目常用子集，完整 NRC 可参考 ISO 14229-1。 */
typedef enum
{
    NRC_NONE_E                      = 0x00, /* 无错误 */
    NRC_GENERAL_REJECT_E            = 0x10, /* 一般拒绝 */
    NRC_SERVICE_NOT_SUPPORTED_E     = 0x11, /* 服务不支持（SID 未实现） */
    NRC_SUBFUNCTION_NOT_SUPPORTED_E = 0x12, /* 子功能不支持 */
    NRC_INCORRECT_MESSAGE_LENGTH_E  = 0x13, /* 消息长度错误 */
    NRC_CONDITIONS_NOT_CORRECT_E    = 0x22, /* 条件不满足 */
    NRC_REQUEST_OUT_OF_RANGE_E      = 0x31, /* 请求超出范围（DID 不存在/值非法） */
    NRC_RESPONSE_TOO_LONG_E         = 0x74  /* 响应太长（缓冲区不足） */
} UdsNrc_e;

/* ============================== 安全等级枚举 ============================== */
/* 注：占位定义，可根据实际安全访问等级替换/扩展。 */
typedef enum
{
    UDS_SEC_L0 = 0,   /* 无安全要求 */
    UDS_SEC_L1,       /* 一级安全等级 */
    UDS_SEC_L2,       /* 二级安全等级 */
    UDS_SEC_L3        /* 三级安全等级 */
} uds_security_level_t;
#endif /* UDS_TYPES_H_ */
