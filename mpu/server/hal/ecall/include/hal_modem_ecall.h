#ifndef _HAL_MODEM_ECALL_H_
#define _HAL_MODEM_ECALL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ECALL_MSD (140)
#define ECALL_PHONE_NUM_LEN (20)
typedef enum
{
    ECALL_TEST = 1,            /**< Test eCall */
    ECALL_EMERGENCY = 2,       /**< Emergency eCall */
    ECALL_RECONFIG = 3,        /**< Reconfig eCall */
} EnuEcallModeType;
    
typedef enum
{
    TRIGGER_MANUAL = 0,
    TRIGGER_AUTO = 1,            
} EnuEcallTriggerType;
    
typedef enum
{
    U_SIM_1,
    U_SIM_2,
} EnuEcallSimId;
    
typedef struct
{
    int8_t ecall_msd[MAX_ECALL_MSD]; //max msd length 140
    uint32_t msd_len;
    int32_t manual; // 0:manual 1:auto
    EnuEcallModeType eCallModeType;
    EnuEcallSimId simId;
    int8_t PhoneNumber[ECALL_PHONE_NUM_LEN];
} StrEcallVoiceInfo;

typedef struct
{
    uint16_t ecallConfigEnable;  // eCall 使能配置项，0表示禁用，1表示使能

    uint16_t ecallConfigVoiceConf;  // MSD 传输时的自动静音使能配置项，0表示禁用，1表示使能

    uint16_t ecallConfigMode;  // eCall 网络模式配置项，0表示正常模式，1表示eCall only模式，2表示默认模式

    uint16_t ecallConfigProcessinfo;  // eCall 状态事件上报终端使能配置项，0表示禁用，1表示使能

    uint16_t ecallConfigStartTimer;  // eCall 定时器 T5 配置项；单位：秒

    uint16_t ecallConfigHackTimer;  // eCall 定时器 T6 配置项；单位：秒

    uint16_t ecallConfigMsdTransmission;  // eCall 定时器 T7 配置项；单位：秒

    uint16_t ecallConfigMoFailRedial;  // eCall 拨号失败重拨次数配置项；单位：次

    uint16_t ecallConfigDropRedial;  // eCall 断线重拨次数配置项；单位：次

    uint16_t ecallConfigCleardownTimer;  // eCall 定时器 T2 配置项；单位：秒

    uint16_t ecallConfigInitiationTimer;  // eCall 定时器 T3 配置项；单位：秒

    uint16_t ecallConfigNadRegistrationTimer;  // eCall 定时器 T9 配置项；单位：秒

    uint16_t ecallConfigNadUnregistrationTimer;  // eCall 定时器 T10 配置项；单位：秒

    uint16_t ecallConfigSystem;  // eCall 标准配置项，0表示European eCall，1表示ERA-GLONASS eCall

    uint16_t ecallConfigEraGlonassRedialTimer;  // ERA-GLONASS eCall 重拨间隔时间配置项；单位：秒

    uint16_t ecallConfigEraGlonassAutoAnswerTimer;  // ERA-GLONASS eCall 自动应答时间配置项；单位：秒

    uint16_t ecallConfigRedialDurationTimer;  // 重拨持续时间 单位：秒              待移远确认
} StrEcallConfig;

typedef enum{
	 ECALL_STATUS_INCOMING = 1,
	 ECALL_STATUS_CALLING,
  	 ECALL_STATUS_ACTIVE,
	 ECALL_STATUS_END,
	 ECALL_STATUS_ABNORMAL_END,
	 ECALL_STATUS_WAIT_PSAP,
	 ECALL_STATUS_SNED_MSD,
	 ECALL_STATUS_PUSH_MSD,
     ECALL_STATUS_UPDATE_MSD,
	 ECALL_STATUS_MSD_SNED_SUCCESS,
	 ECALL_STATUS_MSD_SNED_FAULT
}EnuEcallEventId_e;

 typedef enum
 {
   E_VOICE_ECALL_MODE_NOMAL   = 0, 
   E_VOICE_ECALL_MODE_ONLY    = 1, 
   E_VOICE_ECALL_MODE_DEFAULT = 2
 }EcallConfigMode_e;

 typedef struct 
 {
   uint8_t enable;           /*  Enable or disable ecall ;0 -- Disable; 1 -- Enable */
   uint8_t voiceconfig;      /*  Enable or disable to mute IVS speaker in MSD
                             0 -- Disable to mute IVS speaker automatical in MSD transmission
                             1 -- Enable to mute IVS speaker automatical in MSD transmission */
   EcallConfigMode_e ecallmode;  /*  The Ecall mode. */
   uint8_t processinfo;      /*  Enable or disable to report ecall event info.   0 -- Disable; 1 -- Enable */
   uint8_t T5;               /*  The timer of IVS waiting for "START". */
   uint8_t T6;               /*  The timer of IVS waiting for "HACK". */
   uint8_t T7;               /*  The timer of MSD transmission. */
   uint8_t mofailredial;     /*  The dial fail times. */
   uint8_t dropredial;
   uint16_t T2;              /*  The timer of IVS waiting for "ClearDown".*/
   uint16_t T3;              /*  The timer of IVS waiting for "InitIATION".*/
   uint16_t T9;              /*  The timer of IVS waiting for "NAD Registration".*/
   uint16_t T10;             /*  The timer of IVS waiting for "NAD Deregistration".*/
   uint8_t ecall_system;     /*  The ecall system standard. 0 -- EU; 1 -- ERA_GLONASS*/
   uint16_t era_glonass_redial_time;     /*  The timer of era_glonass redial times.*/
   uint16_t era_glonass_auto_answer_time;/*  The timer of era_glonass auto answer times.*/
   uint16_t redial_duration;
 }EcallConfigInfo_t;

typedef struct 
{
    int32_t latitude;
    int32_t longitude;
    int32_t direction;
    uint8_t isTrusted;   
} EcallPosition_t;

typedef struct
{
    int32_t latitudeDelta;
    int32_t longitudeDelta;
} EcallPositionDelta_t;

typedef struct
{
    uint8_t msdVersion;
    uint8_t vehicleType;
    uint8_t numberOfPassengers;
    uint8_t PropulsionType;
    EcallPositionDelta_t ecallPositionDeltaN1;
    EcallPositionDelta_t ecallPositionDeltaN2;
    EcallPosition_t ecallPostion;
    uint8_t vin[17];
    uint8_t testCall;
} EcallMsd_t;

/**
 * @brief  eCall 语音服务回调函数类型定义
 * @param[in]  EventId    eCall 事件ID
 */
typedef void (*EcallVoiceCallbak)(EnuEcallEventId_e EventId);

/**
 * @brief  初始化eCall语音模块
 * @param[in]  cbFunc      eCall语音服务事件回调函数
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalEcallVoiceInit(EcallVoiceCallbak cbFunc);

/**
 * @brief  启动eCall语音服务
 * @param[in]  EcallVoiceInfo  eCall语音服务配置
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalEcallVoiceStart(StrEcallVoiceInfo *const EcallVoiceInfo, EcallMsd_t *ecallMsdData);

/**
 * @brief   hangup eCall语音服务
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalEcallVoiceHangup(void);

/**
 * @brief  更新eCall语音服务MSD
 * @param[in]  msd           MSD数据
 * @param[in]  msdLen        MSD数据长度
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalEcallVoiceUpdateMsd(const int8_t *msd,uint32_t msdLen, EcallMsd_t *ecallMsdData);

/**
 * @brief  推送eCall语音服务MSD
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalEcallVoicePushMsd(void);

/**
 * @brief  释放eCall语音模块
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalEcallVoiceDeinit(void);

/**
 * @brief  设置eCall语音服务配置
 * @param[in]  ecallConfig  eCall语音服务配置
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalEcallVoiceSetConfigInfo(StrEcallConfig *ecallConfig);

/**
 * @brief  获取eCall语音服务配置
 * @param[out] ecallConfigInfo  eCall语音服务配置
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalEcallVoiceGetConfigInfo(EcallConfigInfo_t *ecallConfigInfo);

/**
 * @brief  启用eCall语音服务
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalEcallVoiceEnable(void);

/**
 * @brief 禁用eCall语音服务
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalEcallVoiceDisable(void);

extern void HalEcallAudioPrepare(void);


#ifdef __cplusplus
}
#endif

#endif //_HAL_MODEM_ECALL_H_





