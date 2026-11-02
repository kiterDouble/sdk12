/* ========================================================================== */
/*  文件名称  : didDefaults.c                                                 */
/*  模块名称  : UDS DID 默认值提供器                                           */
/*  版本      : v1.0.0                                                         */
/*  作者      : <SIRUN HF / Jaxsen>                                           */
/*  创建日期  : 2025-09-01                                                     */
/* -------------------------------------------------------------------------- */
/*  摘要说明  :                                                                */
/*    本模块按“DID（非数据库 id）”返回各项的出厂默认值，用于冷启动时将缺失项       */
/*    下沉到持久化存储并回灌 RAM 槽。默认值以静态只读缓冲区形式提供，调用方只读     */
/*    使用，不得修改或长期持有其指针。                                          */
/*                                                                            */
/*  设计要点  :                                                                */
/*    - 接口：int DefaultGetByDid(uint16_t did, const uint8_t **pp,            */
/*            uint16_t *plen)，返回 1 表示找到默认值并写回 *pp,*plen；返回 0 表示   */
/*            无显式默认（上层可用 min_len 个 0 兜底）。                          */
/*    - 边界：仅给出推荐长度 *plen（在 min..max 范围内），最终裁剪由上层完成。        */
/*    - 一致：端口类 U16 采用大端 (BE)；文本类不包含终止符 '\0'。                   */
/*    - 可维护：所有常量集中定义，避免散布 magic number。                         */
/*                                                                            */
/*  MISRA 注意 :                                                               */
/*    - 所有外部可见函数进行空指针参数校验；                                     */
/*    - 仅返回 0/1 的布尔式结果，不与 NRC 混用；                                 */
/*    - switch 分支内不定义非常量对象；                                         */
/*    - 常量均为 file-scope static const，避免可变全局状态。                      */
/*                                                                            */
/*  变更记录  :                                                                */
/*    v1.0.0  初版                                                             */
/* ========================================================================== */

#include "didDefaults.h"
#include "udsCfg.h"      /* LEN_VIN / LEN_IPv4_STR / LEN_ASCII_32 ... */
#include "udsTypes.h"    /* DID_* 枚举、UDS_MAX_DID_PAYLOAD           */
#include "dtcRpcApi.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "sr_param.h"
#include "sr_log.h"
#include "udsCallbackEntry.h"
#include "appRpcType.h"
#include "didRegistry.h"
/* ========================================================================== */
/*  宏名称  : U16_BE, U32_BE                                                  */
/*  功能描述: 将数值展开为大端字节数组常量                                     */
/* ========================================================================== */
#define U16_BE(v)   { (uint8_t)(((v) >> 8) & 0xFFu), (uint8_t)((v) & 0xFFu) }
#define U32_BE(v)   { (uint8_t)(((v) >> 24) & 0xFFu), \
                      (uint8_t)(((v) >> 16) & 0xFFu), \
                      (uint8_t)(((v) >> 8) & 0xFFu),  \
                      (uint8_t)((v) & 0xFFu) }
/* ============================== 共享零缓冲 ================================ */
/* 说明：用于“默认=全 0”的 DID；上层会按 min_len 做最终限幅。 */
static const uint8_t DEFAULT_ZEROS[UDS_MAX_DID_PAYLOAD] = { 0x00 };

/* ============================== 语义默认值 ================================ */
/* 短文本/号码缺省占位（若字段为“1..N”且无默认内容时，给出 1 字节占位）。 */
static const uint8_t DEF_EMPTY1[1]                   = { 0x00u };

static const uint8_t DEF_VEHICLE_MODE                = 0x01U;
static const uint8_t DEF_GAC_DIAG_PARAM_VERSION[4]   = {1, 35, 9, 1};
static const uint8_t ETHERNET_ID[5]   = {0x4f, 0x51, 0xeb, 0x02, 0X00}; // 0x4f51eb02
static const char hwVer[32] = "v1.0.0";
static const char gacSwVer[17] = "T08-YT1.01E0";
static const uint8_t gnssGalaxy = 0x01u;
static uint8_t FunConfigBuf[64] = {0x00};
static const uint8_t otaDomain[50] = "pre-ota.gacicv.com";
static const uint8_t otaPort[8] = "9082";
static const uint8_t logDomain[100] = "https://pre-fileupload.gacicv.com";
static const uint8_t logPort[8] = "4432";

/* —— 整型 DID 的“字节序”默认值（报文返回用，大端） —— */
static const uint8_t DEF_RTC_WAKEUP_INTERVAL_BE[2]         = U16_BE(1610u);   /* 0x064A */
static const uint8_t DEF_TSP_TIMEOUT_FOR_FLOWCHART_BE[1]   = { 0x28 };        /* 40s */
static const uint8_t DEF_TSP_ACK_TIME_BE[1]                = { 0x28 };        /* 40s */
static const uint8_t DEF_LONG_CONNECTION_CYCLE_BE[2]       = U16_BE(0x0E10u); /* 3600s */
static const uint8_t DEF_TSP_PLATFORM_CONN_SETTING_BE[1]   = { 0x03 };        /* APN1+APN2 连接 */
static const uint8_t DEF_4G_HW_RESET_TIME_BE[1]            = { 0x30 };        /* 48s */
static const uint8_t DEF_BURYING_POINT_SWITCH_BE[1]        = { 0x01 };        /* 开 */
/* 以原实现推导出的以位图组成的以太网配置（注意之前是局部数组，已改为静态常量） */
/* 原值: byte0:bit(0,1,2,4,6)=1 → 0x57, byte1:bit(0,1)=1 → 0x03, 其余 0 */
static const uint8_t DEF_ETHERNET_CONFIG_INFO_BYTES[4]     = { 0x57, 0x03, 0x00, 0x00 };
static       uint8_t DEF_BURIED_POINT_DOMAIN[128]          = "https://bc-v2c-servicedatasync.gacicv.com/data-synchronization/dataSync/v1/buriedPointData";
static       uint8_t DEF_BURIED_POINT_DOMAIN_1[64]         = {0};
static       uint8_t DEF_BURIED_POINT_DOMAIN_2[64]         = {0};

#if 0
/* 将整型写入 ParamInfo.value（十进制 ASCII，不带符号位扩展），自动补 '\0' */
static void ParamSetIntAscii(ParamInfo_t *p, int32_t v)
{
    /* sr_param.h 约定：type==1 时，value 为“整形的字符串形式” */
    /* 256 字节充足，这里用一个小临时缓冲再 memcpy */
    char buf[32];
    int n = snprintf(buf, sizeof(buf), "%d", v);
    if (n < 0) n = 0;
    if ((size_t)n > sizeof(p->value)) n = (int)sizeof(p->value);
    memset(p->value, 0, sizeof(p->value));
    memcpy(p->value, buf, (size_t)n);
}
#endif

/* ========================================================================== */
/*  函数名  : DefaultGetByDid                                                 */
/*  语法    : int DefaultGetByDid(uint16_t did, const uint8_t **pp,           */
/*                                 uint16_t *plen)                            */
/*  功能    : 按 DID 返回默认值缓冲与长度。                                    */
/*  参数    : did   [IN ]  目标 DID                                           */
/*            pp    [OUT]  指向静态默认缓冲的指针地址                          */
/*            plen  [OUT]  默认缓冲的有效长度（推荐值）                        */
/*  返回值  : 1 = 找到默认值；0 = 无显式默认                                   */
/*  说明    : *pp 指向只读静态数据；调用方不得修改或长期缓存该指针。            */
/*            上层应再对 *plen 进行 min..max 限幅。                            */
/* ========================================================================== */
int DefaultGetByDid(uint16_t did, const uint8_t **pp, uint16_t *plen)
{
    /* STEP 1: 参数校验 */
    if ((pp == NULL) || (plen == NULL))
    {
        return 0;
    }

    /* STEP 2: DID 映射到默认值 */
    DeviceParamEnum paramId = 0;
    LOG_Debug("DefaultGetByDid: did = %02x", did);

    switch (did)
    {
     #if 0
        case DID_VEHICLE_MODE:
        {
            *pp = &DEF_VEHICLE_MODE;
            *plen = 1u;
            return 1;
        }
        case DID_TRANSPORT_MODE:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            ParamInfo_t param = {paramId, "transportMode", 1, 0, 255, PARAM_SOURCES_USER, true, "0"};
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_MANUFACTORY_MODE:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            ParamInfo_t param = {paramId, "factoryMode", 1, 0, 255, PARAM_SOURCES_USER, true, "255"};
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_GAC_DIAG_PARAM_VERSION:
        {
            *pp = DEF_GAC_DIAG_PARAM_VERSION;
            *plen = 4u;
            return 1;
        }
        case DID_TSP_DOMAIN_NAME:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            ParamInfo_t param = {paramId, "tspDomainName", 0, 0, 50, PARAM_SOURCES_USER, true, "\0"};
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_RTC_WAKEUP_INTERVAL:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            // ParamInfo_t param = {paramId, "RTCWakeupInterval", 1, 0, 65535, PARAM_SOURCES_USER, true, {0x06,0x4A}};
            ParamInfo_t param = {paramId, "RTCWakeupInterval", 1, 0, 65535, PARAM_SOURCES_USER, true, {0}};
            /* 1610 秒 → 写十进制 ASCII "1610" */
            ParamSetIntAscii(&param, 1610);
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_PORT1:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            if (paramId == (DeviceParamEnum)E_ParamId_UserPrivateTspPort)
            {
                // 专网TSP端口
                ParamInfo_t param = {
                .id = paramId,
                .name = "userPrivateTspPort",
                .type = 1,
                .min = 0,
                .max = 65535,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                AddUserParam(param);

                // 需要拆分成2个参数(参数管理服务整型最大支持4个字节)
                 ParamInfo_t param1 = {
                .id = E_ParamId_UserPrivateTspPort_1,
                .name = "userPrivateTspPort_1",
                .type = 1,
                .min = 0,
                .max = 0xffffffffu,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                AddUserParam(param1);

                ParamInfo_t param2 = {
                .id = E_ParamId_UserPrivateTspPort_2,
                .name = "userPrivateTspPort_2",
                .type = 1,
                .min = 0,
                .max = 0xffffffffu,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                AddUserParam(param2);
            }
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_TSP_CONN_TIMEOUT:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            ParamInfo_t param = {paramId, "tspConnectTimeout", 1, 0, 255, PARAM_SOURCES_USER, true, "5"};
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_TSP_TIMEOUT_FOR_FLOWCHART:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            // ParamInfo_t param = {paramId, "tspTimeoutForFlowchart", 1, 0, 255, PARAM_SOURCES_USER, true, {0x28}};
            ParamInfo_t param = {paramId, "tspTimeoutForFlowchart", 1, 0, 255, PARAM_SOURCES_USER, true, {0}};
            ParamSetIntAscii(&param, 40);
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_TSP_ACK_TIME:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            // ParamInfo_t param = {paramId, "tspAckTime", 1, 0, 255, PARAM_SOURCES_USER, true, {0x28}};
            ParamInfo_t param = {paramId, "tspAckTime", 1, 0, 255, PARAM_SOURCES_USER, true, {0}};
            ParamSetIntAscii(&param, 40);
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        // case DID_LONG_CONNECTION_CYCLE:
        // {
        //     if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
        //     {
        //         LOG_Debug("did:%d, ParamId not found", did);
        //         return 0;
        //     }
        //     // ParamInfo_t param = {paramId, "longConnectionCycle", 1, 0, 65535, PARAM_SOURCES_USER, true, {0x0e, 0x10}};
        //     ParamInfo_t param = {paramId, "longConnectionCycle", 1, 0, 65535, PARAM_SOURCES_USER, true, {0}};
        //     ParamSetIntAscii(&param, 3600);
        //     AddUserParam(param);
        //     *pp = DEFAULT_ZEROS;
        //     *plen = 1u;
        //     return 1;
        // }
        case DID_ETHERNET_CONFIG_INFO:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            uint8_t ethConfig[4] = {0};
            ethConfig[0] = 0x01u | (0x01u << 1) | (0x01u << 2) | (0x01u << 4) | (0x01u << 6);
            ethConfig[1] = 0x01u | (0x01u << 1);
            char ethConfigStr[32] = {0};
            snprintf(ethConfigStr, sizeof(ethConfigStr), "%u", ethConfig[0] | (ethConfig[1] << 8) | (ethConfig[2] << 16) | (ethConfig[3] << 24));
            ParamInfo_t param = {
                .id = paramId,
                .name = "ethernetConfigInfo",
                .type = 1,
                .min = 0,
                .max = 0xFFFFFFFF,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = {0},
            };
            size_t src_len = strlen(ethConfigStr);
            if (src_len > sizeof(param.value))
            {
                src_len = sizeof(param.value);
            }
            memcpy(param.value, (const uint8_t *)ethConfigStr, src_len);
            if (src_len < sizeof(param.value))
            {
                param.value[src_len] = '\0';
            }
            AddUserParam(param);
            *pp = ethConfig;
            *plen = 4u;
            return 1;
        }
        case DID_PUBLIC_TSP_DOMAIN_NAME:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            ParamInfo_t param = {paramId, "publicTspDomainName", 0, 0, 50, PARAM_SOURCES_USER, true, "\0"};
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_4G_MODULE_HW_VER:
        {
            *pp   = (const uint8_t*)hwVer;
            *plen = strlen(hwVer);
            return 1;
        }
        case DID_4G_APP_VER:
        {
            *pp   = (const uint8_t*)gacSwVer;
            *plen = strlen(gacSwVer);
            return 1;
        }
        case DID_GNSS_GALAXY:
        {
            *pp = &gnssGalaxy;
            *plen = 1;
            return 1;
        }
        case DID_ETHERNET_ID:
        {
            //UID 0x4f51eb02
            *pp = ETHERNET_ID;
            *plen = sizeof(ETHERNET_ID);
            // LOG_Debug("did:%d, %08x", did, *pp);
            return 1;
        }
        case DID_TSP_PLATFORM_CONN_SETTING:
        {
            // bit 0:0x00=APN1通道不连接 0x01=APN1通道连接
            // bit 1:0x00=APN2通道不连接 0x01=APN2通道连接
            // bit 2-7:保留
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            ParamInfo_t param = {
                .id = paramId,
                .name = "tspPlatformConnSetting",
                .type = 1,
                .min = 0,
                .max = 255,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = {0},
            };
            // param.value[0] = 0x01u | (0x01u << 1);
            ParamSetIntAscii(&param, 0x03);
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_TBOX_RUNNING_MODE:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            // 0=正常模式 1=维修模式 2=运输模式
            ParamInfo_t param = {paramId, "tboxRunningMode", 1, 0, 255, PARAM_SOURCES_USER, true, "0"};
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_4G_HW_RESET_TIME:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            // ParamInfo_t param = {paramId, "4gHwResetTime", 1, 0, 255, PARAM_SOURCES_USER, true, {0x30u}};
            ParamInfo_t param = {paramId, "4gHwResetTime", 1, 0, 255, PARAM_SOURCES_USER, true, {0}};
            ParamSetIntAscii(&param, 48);
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_4G_HW_RESET_COUNT:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            // ParamInfo_t param = {paramId, "4gHwResetTime", 1, 0, 255, PARAM_SOURCES_USER, true, {0x30u}};
            ParamInfo_t param = {paramId, "4gHwResetCount", 1, 0, 255, PARAM_SOURCES_USER, true, {0}};
            ParamSetIntAscii(&param, 1);
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
            break;
        }
        case DID_PKI_DOMAIN_NAME:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            ParamInfo_t param = {
                .id = paramId,
                .name = "PKI_DOMAIN_NAME",
                .type = 0,
                .min = 0,
                .max = 50,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = {0},
            };
            param.value[0] = '\0';
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_PKI_PORT:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            if (paramId == (DeviceParamEnum)E_ParamId_UserPkiPort)
            {
                // pki端口
                ParamInfo_t param = {
                .id = paramId,
                .name = "pkiPort",
                .type = 1,
                .min = 0,
                .max = 65535,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                AddUserParam(param);

                // 需要拆分成2个参数(参数管理服务整型最大支持4个字节)
                 ParamInfo_t param1 = {
                .id = E_ParamId_UserPkiPort_1,
                .name = "pkiPort1",
                .type = 1,
                .min = 0,
                .max = 0xffffffffu,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                AddUserParam(param1);

                ParamInfo_t param2 = {
                .id = E_ParamId_UserPkiPort_2,
                .name = "pkiPort2",
                .type = 1,
                .min = 0,
                .max = 0xffffffffu,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                AddUserParam(param2);
            }
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_PKI_MAINTENANCE_CYCLE:{
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
           ParamInfo_t param = {
                .id = paramId,
                .name = "PKI_CYCLE",
                .type = 1,
                .min = 0,
                .max = 255,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = {0},
            };
            // param.value[0] = 15U;
            ParamSetIntAscii(&param, 15);
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        };
        case DID_LOG_DOMAIN_NAME:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            ParamInfo_t param = {
                .id = paramId,
                .name = "LOG_DOMAIN_NAME",
                .type = 0,
                .min = 0,
                .max = 100,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = {0},
            };
            memcpy(param.value, logDomain, strlen((char *)logDomain));
            param.value[strlen((char *)logDomain)] = '\0';
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_LOG_PORT:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            if (paramId == (DeviceParamEnum)E_ParamId_UserLogUploadPort)
            {
                ParamInfo_t param = {
                .id = paramId,
                .name = "logPort",
                .type = 1,
                .min = 0,
                .max = 65535,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                ParamSetIntAscii(&param, 4432);
                AddUserParam(param);

                // 需要拆分成2个参数(参数管理服务整型最大支持4个字节)
                 ParamInfo_t param1 = {
                .id = E_ParamId_UserLogUploadPort_1,
                .name = "logPort1",
                .type = 1,
                .min = 0,
                .max = 0xffffffffu,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                ParamSetIntAscii(&param1, 0x34343332);
                AddUserParam(param1);

                ParamInfo_t param2 = {
                .id = E_ParamId_UserLogUploadPort_2,
                .name = "logPort2",
                .type = 1,
                .min = 0,
                .max = 0xffffffffu,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                AddUserParam(param2);
            }
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_LOG_ACCOUNT:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            ParamInfo_t param = {
                .id = paramId,
                .name = "LOG_ACCOUNT",
                .type = 0,
                .min = 0,
                .max = 50,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = {0},
            };
            param.value[0] = '\0';
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_LOG_PASSWORD:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            ParamInfo_t param = {
                .id = paramId,
                .name = "LOG_PASSWORD",
                .type = 0,
                .min = 0,
                .max = 50,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = {0},
            };
            param.value[0] = '\0';
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_LOG_INTERFACE:
        {
            /**
             * 0x00=专网通道
             * 0x01=公网通道
             * 0x02=车机USB通道
             */
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            ParamInfo_t param = {
                .id = paramId,
                .name = "LOG_INTERFACE",
                .type = 1,
                .min = 0,
                .max = 255,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
            };
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_LOG_PATH:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            ParamInfo_t param = {
                .id = paramId,
                .name = "LOG_PATH",
                .type = 0,
                .min = 0,
                .max = 255,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = {0},
            };
            param.value[0] = '\0';
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_MQTT_PORT_PUBLIC:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            if (paramId == (DeviceParamEnum)E_ParamId_UserPublicTspPort)
            {
                // pki端口
                ParamInfo_t param = {
                .id = paramId,
                .name = "mqttPort",
                .type = 1,
                .min = 0,
                .max = 65535,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                AddUserParam(param);

                // 需要拆分成2个参数(参数管理服务整型最大支持4个字节)
                 ParamInfo_t param1 = {
                .id = E_ParamId_UserPublicTspPort_1,
                .name = "mqttPort1",
                .type = 1,
                .min = 0,
                .max = 0xffffffffu,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                AddUserParam(param1);

                ParamInfo_t param2 = {
                .id = E_ParamId_UserPublicTspPort_2,
                .name = "mqttPort2",
                .type = 1,
                .min = 0,
                .max = 0xffffffffu,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                AddUserParam(param2);
            }
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_SENSITIVE_DATA_SET:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            ParamInfo_t param = {0};
            if (paramId == (DeviceParamEnum)E_ParamId_Sensitive_Data_Set)
            {
                // 需要拆分成4个参数(参数管理服务整型最大支持4个字节)
                 ParamInfo_t param1 = {
                .id = E_ParamId_Sensitive_Data_Set_1,
                .name = "SENSITIVE_DATA_SET_1",
                .type = 1,
                .min = 0,
                .max = 0xffffffffu,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                AddUserParam(param1);

                 ParamInfo_t param2 = {
                .id = E_ParamId_Sensitive_Data_Set_2,
                .name = "SENSITIVE_DATA_SET_2",
                .type = 1,
                .min = 0,
                .max = 0xffffffffu,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                AddUserParam(param2);

                 ParamInfo_t param3 = {
                .id = E_ParamId_Sensitive_Data_Set_3,
                .name = "SENSITIVE_DATA_SET_3",
                .type = 1,
                .min = 0,
                .max = 0xffffffffu,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                AddUserParam(param3);

                 ParamInfo_t param4 = {
                .id = E_ParamId_Sensitive_Data_Set_4,
                .name = "SENSITIVE_DATA_SET_4",
                .type = 1,
                .min = 0,
                .max = 0xffffffffu,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                AddUserParam(param4);
            }
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_OTA_RTC_WAKEUP_PARAM:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            if (paramId == (DeviceParamEnum)E_ParamId_OTA_RTC_Wakeup_Param)
            {
                // 需要拆分成2个参数(参数管理服务整型最大支持4个字节)
                 ParamInfo_t param1 = {
                .id = E_ParamId_OTA_RTC_Wakeup_Param_1,
                .name = "OTA_RTC_Wakeup_Param_1",
                .type = 1,
                .min = 0,
                .max = 0xffffffffu,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                AddUserParam(param1);

                ParamInfo_t param2 = {
                .id = E_ParamId_OTA_RTC_Wakeup_Param_2,
                .name = "OTA_RTC_Wakeup_Param_2",
                .type = 1,
                .min = 0,
                .max = 0xffffffffu,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = "0",
                };
                AddUserParam(param2);
            }
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
        case DID_BURYING_POINT_SWITCH:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            ParamInfo_t param = {
                .id = paramId,
                .name = "burying_point_switch",
                .type = 1,
                .min = 0,
                .max = 0xffffffffu,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = {0},
                };
            // param.value[0] = 0x01u;
            ParamSetIntAscii(&param, 1);
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1;
            return 1;
        }
        case DID_CONN_TEST_ADDR:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            ParamInfo_t param = {
                .id = paramId,
                .name = "conn_test_addr",
                .type = 0,
                .min = 0,
                .max = 50,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = {0},
                };
            // 默认值8.8.8.8
            memcpy(param.value, "8.8.8.8", strlen("8.8.8.8"));
            param.value[strlen("8.8.8.8")] = '\0';
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1;
            return 1;
        }
        case DID_BURIED_POINT_DOMAIN:
        {
            if (UdsGetParamIdFromMappingTable(did, &paramId) != 0)
            {
                LOG_Debug("did:%d, ParamId not found", did);
                return 0;
            }
            ParamInfo_t param = {
                .id = paramId,
                .name = "BuriedPointDomain",
                .type = 0,
                .min = 0,
                .max = 128,
                .paramSources = PARAM_SOURCES_USER,
                .rpcWriteEnable = true,
                .value = {0},
                };
            // 默认埋点数据量产地址 https://bc-v2c-sni-servicedatasync.gacicv.com:8443/data-buried-eea2/dataSync/v3/buriedPointData

            memcpy(param.value, "https://bc-v2c-sni-servicedatasync.gacicv.com:8443/data-buried-eea2/dataSync/v3/buriedPointData",
                                strlen("https://bc-v2c-sni-servicedatasync.gacicv.com:8443/data-buried-eea2/dataSync/v3/buriedPointData"));
            param.value[strlen("https://bc-v2c-sni-servicedatasync.gacicv.com:8443/data-buried-eea2/dataSync/v3/buriedPointData")] = '\0';
            AddUserParam(param);
            *pp = DEFAULT_ZEROS;
            *plen = 1;
            return 1;
        }
        case DID_BURIED_POINT_DOMAIN_1:
        {
            memcpy(DEF_BURIED_POINT_DOMAIN_1, DEF_BURIED_POINT_DOMAIN, 64);
            *pp = DEF_BURIED_POINT_DOMAIN_1;
            *plen = strlen((char *)DEF_BURIED_POINT_DOMAIN_1);
            return 1;
        }
        case DID_BURIED_POINT_DOMAIN_2:
        {
            memcpy(DEF_BURIED_POINT_DOMAIN_2, DEF_BURIED_POINT_DOMAIN + 64, strlen((char *)DEF_BURIED_POINT_DOMAIN)-64);
            *pp = DEF_BURIED_POINT_DOMAIN_2;
            *plen = strlen((char *)DEF_BURIED_POINT_DOMAIN_2);
            return 1;
        }
        case DID_FUNCTION_CONFIG:
        {
            uint8_t vehicleTypeForMqttTopic[10] = "T08-Y1";
            uint8_t vehicleTypeForPkiSDK[30] = "T08Y1-TBOX-SM2-021";
            uint8_t ecallAudioType = 0x02;
            uint8_t bcallAudioType = 0x02;
            // 1：以太网   0：非以太网
            uint8_t ethernetConfig = 0x01;
            uint8_t rtkConfig = 0x00;
            uint8_t srsPwmType = 0x00;
            uint8_t simCardNumber = 0x00;
            uint8_t config40 = (ecallAudioType & 0x03) |
                                ((bcallAudioType & 0x03) << 2) |
                                ((ethernetConfig & 0x01) << 4) |
                                ((rtkConfig & 0x01) << 5) |
                                ((srsPwmType & 0x01) << 6) |
                                ((simCardNumber & 0x01) << 7);
            uint8_t config41 = 0x00;
            uint8_t config42 = 0x00;
            uint8_t config43 = 0x00;
            memcpy(&FunConfigBuf[0], vehicleTypeForMqttTopic, 10);
            memcpy(&FunConfigBuf[10], vehicleTypeForPkiSDK, 30);
            FunConfigBuf[40] = config40;
            FunConfigBuf[41] = config41;
            FunConfigBuf[42] = config42;
            FunConfigBuf[43] = config43;
            *pp = FunConfigBuf;
            *plen = 64;
            return 1;
        }

        case DID_OTA_DOMAIN_NAME:
        {
            *pp = otaDomain;
            *plen = sizeof((char *)otaDomain);
            return 1;
        }

        case DID_OTA_PORT:
        {
            *pp = otaPort;
            *plen = sizeof((char *)otaPort);
            return 1;
        }

        case DID_ADB_STATUS:
        {
            *pp = DEFAULT_ZEROS;
            *plen = 1U;
            AdbClose();
            return 1;

        }
    #endif
        default:
        {
            *pp = DEFAULT_ZEROS;
            *plen = 1u;
            return 1;
        }
    }
}
