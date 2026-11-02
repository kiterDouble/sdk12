/**
 * @file sr_param.h
 * @author jiajiangze (jiangze.jia@sirun.net)
 * @brief param sdk层接口头文件
 * @version 0.1
 * @date 2023-12-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef _RPC_PARAM_SDK_H
#define _RPC_PARAM_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief param rpc server对外的位置信息RPC调用subcmd
 * 
 */
#define PARAMSERVER_MSGTYPE_SUBCMD_GETPARAM (1)
#define PARAMSERVER_MSGTYPE_SUBCMD_UPDATEPARAM (2)
#define PARAMSERVER_MSGTYPE_SUBCMD_ADDUSERPARAM (3)
#define PARAMSERVER_MSGTYPE_SUBCMD_DELETEPARAM (4)

#define PARAM_SOURCES_MODEM 0
#define PARAM_SOURCES_MCU 1
#define PARAM_SOURCES_DB 2
#define PARAM_SOURCES_USER 3

    typedef enum
    {
        E_ParamId_ICCID = 0,        /* ICCID */
        E_ParamId_IMEI,             /* IMEI */
        E_ParamId_IMSI,             /* IMSI */
        E_ParamId_SN,               /* Tbox Sn号 */
        E_ParamId_VIN,              /* 车辆VIN码 */
        E_ParamId_TSPAddr,          /* 平台地址 */
        E_ParamId_TSPPort,          /* 平台端口号 */
        E_ParamId_GB32960Addr,      /* GB32960平台地址 */
        E_ParamId_GB329060Port,     /* GB32960平台端口 */
        E_ParamId_HJ1239Addr,       /* HJ1239平台地址 */
        E_ParamId_HJ1239Port,       /* HJ1239平台端口 */
        E_ParamId_ECallNumber,      /* E-CALL电话号码 */
        E_ParamId_BCallNumber,      /* B-CALL电话号码 */
        E_ParamId_ICallNumber,      /* I-CALL电话号码 */
        E_ParamId_SW_Version,       /* 斯润软件版本号 */
        E_ParamId_HW_Version,       /* 斯润硬件版本号 */
        E_ParamId_CustomSW_Version, /* 客户软件版本号 */
        E_ParamId_CarVersion,       /* 整车版本号 */
        E_ParamId_ManufactureData,  /* Tbox生产日期 */
        E_ParamId_ParatNumber,      /* 零件号 */
        E_ParamId_SupIdentifier,    /* 供应商标识号 */
        E_ParamId_ecallConfigEnable,
        E_ParamId_ecallConfigVoiceConf,
        E_ParamId_ecallConfigMode,
        E_ParamId_ecallConfigProcessinfo,
        E_ParamId_ecallConfigStartTimer,
        E_ParamId_ecallConfigHackTimer,
        E_ParamId_ecallConfigMsdTransmission,
        E_ParamId_ecallConfigMoFailRedial,
        E_ParamId_ecallConfigDropRedial,
        E_ParamId_ecallConfigCleardownTimer,
        E_ParamId_ecallConfigInitiationTimer,
        E_ParamId_ecallConfigNadRegistrationTimer,
        E_ParamId_ecallConfigNadUnregistrationTimer,
        E_ParamId_ecallConfigSystem,
        E_ParamId_ecallConfigEraGlonassRedialTimer,
        E_ParamId_ecallConfigEraGlonassAutoAnswerTimer,
        E_ParamId_ecallConfigRedialDurationTimer,
        E_ParamId_ecallConfigTestMode,
        E_ParamId_ecallConfigTestNum,
        E_ParamId_netAllowDial,
        /**
         * @brief 是否对关键数据进行加密 0:不加密 1:需要加密且暂时未加密 2:已加密 
         * 在制作oemapp镜像时，此值设置为0，其余所有参数设置为明文数据。则数据库中所有数据在运行时会以明文形式保存
         * 在制作oemapp镜像时，此值设置为1，其余所有参数设置为明文数据。在下一次参数管理服务运行时，会将关键参数进行加密并保存到数据库，同时将此值设置为2
         * 在制作oemapp镜像时，不要将此值设置为2
         * 
         */
        E_ParamId_paramEncryptStatus,
        E_ParamId_ecallConfigCallAutoAnswerTime,
        E_ParamId_ecallConfigPosTestRegistrationTime,
        E_ParamId_ecallConfigTestModeEndDistance,
        E_ParamId_ecallConfigIntMemTransmitInterval,
        E_ParamId_ecallConfigIntMemTransmitAttempts,
        E_ParamId_ecallConfigCCFT,
        E_ParamId_ecallConfigMSDMaxTransmissionTime,
        E_ParamId_ecallConfigNADDeregistrationTime,
        E_ParamId_ecallConfigEcallNoAutomaticTriggering,
        E_ParamId_ecallConfigEcallDialDiration,
        E_ParamId_ecallConfigEcallAutoDialAttempts,
        E_ParamId_ecallConfigEcallManualDialAttempts,
        E_ParamId_ecallConfigEcallManualCanCancel,
        E_ParamId_ecallConfigEcallSMSFallbackNumber,
        E_ParamId_ecallConfigTestRegistrationPeriod,
        E_ParamId_ecallConfigGNSSDataRate,
        E_ParamId_ecallConfigVehicleType,
        E_ParamId_ecallConfigVehiclePropulsionStorageType,
        E_ParamId_enumParamEnd,    /*始末标志请勿删除*/
        E_ParamId_BaseEnd = 63,    /*始末标志请勿删除*/
        E_ParamId_UserStart = 100, /*始末标志请勿删除*/
        E_ParamId_UserEnd = 255,   /*始末标志请勿删除*/
        E_ParamId_InvalidId = 300  /*始末标志请勿删除*/
    } DeviceParamEnum;

#pragma pack(1)
typedef struct
{
    uint32_t id;          // 参数ID：0~63，100~255，0~63为基础参数，100~255为用户参数
    uint8_t name[64];     // 参数名称数组，长度固定为64字节，不足64时补0
    uint8_t type;         // 参数值的类型：0 表示value字段为字符串类型，1 表示value字段为整型
    int32_t min;         // 参数值的最小值：type为0时，表示字符串的最小长度；type为1时，表示数据的最小值
    int32_t max;         // 参数值的最大值：type为0时，表示字符串的最大长度，应小于256；type为1时，表示数据的最大值，max取值大于等于min
    uint8_t paramSources; // 参数来源：PARAM_SOURCES_MODEM, PARAM_SOURCES_MCU, PARAM_SOURCES_DB, PARAM_SOURCES_USER
    bool rpcWriteEnable;  // RPC调用时是否允许可写：true 表示RPC调用时允许覆写参数值，false 表示RPC调用时不允许覆写参数值
    uint8_t value[256];   // 参数值数组，长度固定为256字节，默认值为全0。当type==0时，格式为字符串如“www.baidu.com”;当type==1时，格式为整形的字符串形式如“-1”
} ParamInfo_t;
#pragma pack()

/**
 * @brief Get the Param
 * 
 * @param param IO 1.接口调用时，填充param.id字段，用户请求对应ID的参数。2.接口调用结束后，用于返回ID对应的具体参数
 * @return int32_t 接口执行结果 RET_OK：成功 RER_ERR：失败（如果请求的参数不存在也会返回失败）
 */
int32_t GetParam(ParamInfo_t *param);

/**
 * @brief Update the Param
 * 
 * @param id 待更新参数的ID
 * @param value 待更新参数的具体内容，如果该参数为整型，value需要为int转为的字符串比如“8888”
 *                                  如果该参数为字符串，value需要为字符串的首地址
 * @param dataLen value 的实际字节长度
 * @return int32_t 接口执行结果 RET_OK：成功 RER_ERR：失败
 */
int32_t UpdateParam(uint32_t id, uint8_t *value, uint16_t dataLen);

/**
 * @brief 添加用户参数
 * 
 * @param param 待新增参数的详细内容 (param.id范围条件：100~255)
 * @return int32_t 接口执行结果 RET_OK：成功 RER_ERR：失败
 */
int32_t AddUserParam(ParamInfo_t param);

/**
 * @brief 删除用户参数
 * 
 * @param id 待删除参数的ID (范围条件：100~255)
 * @return int32_t 接口执行结果 RET_OK：成功 RER_ERR：失败
 */
int32_t DeleteUserParam(uint32_t id);

#ifdef __cplusplus
}
#endif
#endif
