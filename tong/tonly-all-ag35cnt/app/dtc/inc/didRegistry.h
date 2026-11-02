/* ========================================================================== */
/*  文件名称  : did_registry.h                                                */
/*  模块名称  : UDS DID 注册表与默认读写接口                                   */
/*  版本      : v1.0.0                                                         */
/*  作者      : <SIRUN HF / Jaxsen>                                           */
/*  创建日期  : 2025-09-01                                                     */
/* -------------------------------------------------------------------------- */
/*  摘要说明  :                                                                */
/*    本头文件定义 UDS DID（Data Identifier）注册表的对外接口：                  */
/*    - 通过固定 db_id ↔ DID 的静态映射与 SQLite 持久化保持一致；                 */
/*    - 提供默认读/写路径（长度校验 + DB 同步 + RAM 缓存）；                      */
/*    - 暴露只读的注册表查询、初始化与关闭接口。                                  */
/*                                                                              */
/*  设计要点  :                                                                */
/*    - 函数名采用大驼峰；结构体名以 *_t 结尾；回调使用 uds_read_cb_t /            */
/*      uds_write_cb_t 原型，参数与返回值遵循 UDS NRC 语义（0=成功）。             */
/*    - RAM 槽为内部实现细节；外部仅通过 API 读写，避免越权访问。                  */
/*    - db_id 为数据库固定主键（非自增），永不改变；DID 为协议编号。               */
/*                                                                              */
/*  使用约定  :                                                                */
/*    - 上电顺序：                                                              */
/*        STEP1: 调用 UdsStorageInit() 打开/建库 → 下沉默认值 → 回灌 RAM；          */
/*        STEP2: 业务通过 UdsDidRead/UdsDidWrite 进行读写；                         */
/*        STEP3: 下电前调用 UdsStorageClose() 关闭数据库。                          */
/*    - 读写接口均会执行长度边界检查（min_len ≤ len ≤ max_len）。                   */
/*    - 若某 DID 有专用回调，可在注册表项中设置 read_cb / write_cb；               */
/*      未设置时走默认通道（RAM/DB）。                                            */
/*                                                                              */
/*  依赖说明  :                                                                */
/*    - 需要 udsTypes.h 提供：                                                   */
/*        · UDS_DID_IDS_e（DID 枚举）                                            */
/*        · NRC 常量（0=成功，其它为负响应码）                                    */
/*    - 需要 did_defaults.c 提供 DefaultGetByDid() 默认值查询。                   */
/*                                                                              */
/*  变更记录  :                                                                */
/*    v1.0.0  初版                                                             */
/* ========================================================================== */

#ifndef DID_REGISTRY_H
#define DID_REGISTRY_H

/* ================================== 依赖 =================================== */
#include <stdint.h>
#include <udsTypes.h>   /* UDS_DID_IDS_e / NRC_* */
#include "dtcRpcApi.h"

/* ============================== 访问权限位 ================================ */
/* 若已在 udsTypes.h 定义，请在编译脚本中移除以下兜底定义，避免重复定义。 */
#define UDS_ACCESS_R    (1u << 0)
#define UDS_ACCESS_W    (1u << 1)
#define UDS_ACCESS_RW   (UDS_ACCESS_R | UDS_ACCESS_W)
/* 新增：Control 权限位（例：触发动作/控制型 DID） */
#define UDS_ACCESS_C   (0x04u)
#define UDS_ACCESS_RWC   (UDS_ACCESS_R | UDS_ACCESS_W | UDS_ACCESS_C)

#ifdef __cplusplus
extern "C" {
#endif

/* ============================== 回调原型定义 ============================== */
/* 说明：回调返回值与 UDS NRC 对齐：0=NRC_NONE 成功；其他为各类拒绝码。 */
typedef UdsNrc_e (*uds_read_cb_t)(uint16_t did, uint8_t *buf, uint16_t *io_len);
typedef UdsNrc_e (*uds_write_cb_t)(uint16_t did, const uint8_t *buf, uint16_t len);
/* 新增：Control 回调。
 * - subFunc：子功能码（类似 0x31 的子功能；如果你的控制无子功能，可传 0）。
 * - in/inLen：控制输入数据。
 * - out/io_len：可选返回状态记录（如无返回，把 *io_len 置 0）。
 * - pReturnStatus：TRUE=正响应应携带 out（长度 *io_len），FALSE=不携带。
 */
typedef UdsNrc_e (*uds_control_cb_t)(uint16_t did, uint8_t subFunc,
                                     const uint8_t *in, uint16_t inLen,
                                     uint8_t *out, uint16_t *io_len,
                                     gboolean *pReturnStatus);


/* ============================== 注册表项结构 ============================== */
/* 说明：db_id 为数据库固定主键（非自增、不可变）；did 为协议 DID。 */
typedef struct
{
    UDS_DID_IDS_e  did;       /* 协议 DID（数据标识符）                         */
    const char    *name;      /* 仅用于日志/调试打印                            */
    uint8_t        access;    /* UDS_ACCESS_R / UDS_ACCESS_W / UDS_ACCESS_RW    */
    uint16_t       min_len;   /* 允许的最小长度（含变长文本最小 1 字节等）       */
    uint16_t       max_len;   /* 允许的最大长度                                  */
    uds_read_cb_t  read_cb;   /* 可选：专用读回调；NULL 则走默认读路径           */
    uds_write_cb_t write_cb;  /* 可选：专用写回调；NULL 则走默认写路径           */
    uds_control_cb_t control_cb;/* 新增：专用控制回调；仅走回调，不落库 */
} UdsDidEntry_t;

/* ============================== 注册表/查询接口 =========================== */
/* -------------------------------------------------------------------------- */
/*  函数名称  : UdsDidName                                                    */
/*  功能描述  : 根据 DID 返回可读名称字符串（未命中返回 "UNKNOWN_DID"）。        */
/*  参数说明  : did  [in]  目标 DID。                                          */
/*  返回值    : const char*（静态只读）。                                       */
/*  执行步骤  :                                                                */
/*    STEP1: 在线性表中查找 DID；                                               */
/*    STEP2: 命中返回 name；未命中返回 "UNKNOWN_DID"。                          */
/* -------------------------------------------------------------------------- */
const char * UdsDidName(UDS_DID_IDS_e did);

/* -------------------------------------------------------------------------- */
/*  函数名称  : UdsGetDidRegistry                                             */
/*  功能描述  : 获取 DID 注册表起始指针与项数（只读视图）。                       */
/*  参数说明  : count [out] 若非 NULL，写入表项个数。                           */
/*  返回值    : 指向静态只读注册表的指针。                                       */
/*  执行步骤  :                                                                */
/*    STEP1: 将静态数组首地址与项数返回给调用方只读使用。                        */
/* -------------------------------------------------------------------------- */
const UdsDidEntry_t * UdsGetDidRegistry(uint32_t *count);

/* ============================== 初始化/关闭接口 =========================== */
/* -------------------------------------------------------------------------- */
/*  函数名称  : UdsStorageInit                                                */
/*  功能描述  : 打开/创建数据库 → 建表 → 按 db_id 下沉默认值 → 回灌 RAM。         */
/*  参数说明  : path [in]  数据库文件路径（UTF-8），为 NULL 时使用实现默认路径。  */
/*  返回值    : 0 成功；非 0 失败（见实现的错误码/日志）。                         */
/*  执行步骤  :                                                                */
/*    STEP1: 打开或创建数据库文件；                                             */
/*    STEP2: 确保表结构存在（id 主键、did 唯一、value BLOB）；                    */
/*    STEP3: 遍历注册表，缺失项用 DefaultGetByDid() 或 min_len 零填入库；          */
/*    STEP4: 全量从 DB 读取覆盖 RAM 槽（含长度限幅至 [min,max]）；                 */
/*    STEP5: 返回状态码。                                                       */
/* -------------------------------------------------------------------------- */
int  UdsStorageInit(const char *path);

/* -------------------------------------------------------------------------- */
/*  函数名称  : UdsStorageClose                                               */
/*  功能描述  : 关闭数据库连接，释放与持久化相关的内部资源。                      */
/*  参数说明  : 无。                                                            */
/*  返回值    : 无。                                                            */
/*  执行步骤  :                                                                */
/*    STEP1: 若 DB 句柄非空则关闭；                                             */
/*    STEP2: 清理内部状态。                                                     */
/* -------------------------------------------------------------------------- */
void UdsStorageClose(void);

/* ============================== 默认读/写接口 ============================= */
/* -------------------------------------------------------------------------- */
/*  函数名称  : UdsDidRead                                                    */
/*  功能描述  : 读取指定 DID 的当前值。默认从 RAM 槽返回（专用回调优先）。          */
/*  参数说明  :                                                                */
/*    did    [in]   目标 DID；                                                  */
/*    buf    [out]  输出缓冲区指针；                                             */
/*    io_len [in/out] 入参=缓冲区容量；出参=实际字节数。                          */
/*  返回值    : NRC（0=成功；NRC_RESPONSE_TOO_LONG 等）。                         */
/*  执行步骤  :                                                                */
/*    STEP1: 校验入参与访问权限；                                               */
/*    STEP2: 若注册了 read_cb，则直接调用并返回；                                 */
/*    STEP3: 否则取 RAM 槽长度，比较 *io_len；不足则返回 NRC_RESPONSE_TOO_LONG；   */
/*    STEP4: 拷贝数据，写回 *io_len，返回成功。                                   */
/* -------------------------------------------------------------------------- */
UdsNrc_e UdsDidRead(UDS_DID_IDS_e did, uint8_t *buf, uint16_t *io_len);

/* -------------------------------------------------------------------------- */
/*  函数名称  : UdsDidWrite                                                   */
/*  功能描述  : 写入指定 DID 的值。默认路径会做长度校验，先写 DB 成功后再更新 RAM。  */
/*  参数说明  :                                                                */
/*    did   [in]  目标 DID；                                                    */
/*    buf   [in]  输入数据指针；                                                */
/*    len   [in]  输入数据长度。                                                */
/*  返回值    : NRC（0=成功；NRC_INCORRECT_MESSAGE_LENGTH 等）。                   */
/*  执行步骤  :                                                                */
/*    STEP1: 校验入参与访问权限，并检查 len ∈ [min_len,max_len]；                 */
/*    STEP2: 若注册了 write_cb，则先调用（用于额外校验/联动）；                    */
/*    STEP3: 走默认路径：写 DB（按固定 db_id），成功后更新 RAM 槽；                 */
/*    STEP4: 返回状态码。                                                       */
/* -------------------------------------------------------------------------- */
UdsNrc_e UdsDidWrite(UDS_DID_IDS_e did, const uint8_t *buf, uint16_t len);

UdsNrc_e UdsDidWriteForOwner(UDS_DID_IDS_e did, const uint8_t *buf, uint16_t len);

UdsNrc_e UdsDidControl(UDS_DID_IDS_e did, uint8_t subFunc,
                       const uint8_t *in, uint16_t inLen,
                       uint8_t *out, uint16_t *io_len,
                       gboolean *pReturnStatus);

#ifdef __cplusplus
}
#endif

#endif /* DID_REGISTRY_H */
