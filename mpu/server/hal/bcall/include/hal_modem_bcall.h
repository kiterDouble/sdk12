#ifndef _HAL_MODEM_BCALL_H_
#define _HAL_MODEM_BCALL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_BCALL_MSD (140)
#define BCALL_PHONE_NUM_LEN (20)
typedef enum
{
    BCALL_TEST = 1,            /**< Test eCall */
    BCALL_EMERGENCY = 2,       /**< Emergency eCall */
    BCALL_RECONFIG = 3,        /**< Reconfig eCall */
} EnuBcallModeType;
    
typedef enum
{
    TRIGGER_MANUAL = 0,
    TRIGGER_AUTO = 1,            
} EnuBcallTriggerType;
    
typedef enum
{
    U_SIM_1,
    U_SIM_2,
} EnuBcallSimId;
typedef struct 
{
    int32_t latitude;
    int32_t longitude;
    int32_t direction;
    uint8_t isTrusted;   
} BcallPosition_t;

typedef struct
{
    int32_t latitudeDelta;
    int32_t longitudeDelta;
} BcallPositionDelta_t;

typedef struct
{
    uint8_t msdVersion;
    uint8_t vehicleType;
    uint8_t numberOfPassengers;
    uint8_t PropulsionType;
    BcallPositionDelta_t bcallPositionDeltaN1;
    BcallPositionDelta_t bcallPositionDeltaN2;
    BcallPosition_t bcallPostion;
    uint8_t vin[17];
    uint8_t testCall;
} BcallMsd_t;
   
typedef struct
{
    int8_t bcall_msd[MAX_BCALL_MSD]; //max msd length 140
    uint32_t msd_len;
    BcallMsd_t bcallMsd;
    int32_t manual; // 0:manual 1:auto
    EnuBcallModeType bCallModeType;
    EnuBcallSimId simId;
    int8_t PhoneNumber[BCALL_PHONE_NUM_LEN];
} StrBcallVoiceInfo;

typedef struct
{
    int keepAlivable;
} StrBcallConfig;

typedef enum{
	 BCALL_STATUS_INCOMING = 1,
	 BCALL_STATUS_CALLING,
  	 BCALL_STATUS_ACTIVE,
	 BCALL_STATUS_END,
	 BCALL_STATUS_ABNORMAL_END,
	 BCALL_STATUS_WAIT_PSAP,
	 BCALL_STATUS_SNED_MSD,
	 BCALL_STATUS_PUSH_MSD,
     BCALL_STATUS_UPDATE_MSD,
	 BCALL_STATUS_MSD_SNED_SUCCESS,
	 BCALL_STATUS_MSD_SNED_FAULT,
	 BCALL_STATUS_SAVE
}EnuBcallEventId_e;



/**
 * @brief  Bcall 语音服务回调函数类型定义
 * @param[in]  EventId    Bcall 事件ID
 */
typedef void (*BcallVoiceCallbak)(EnuBcallEventId_e EventId);

/**
 * @brief  初始化Bcall语音模块
 * @param[in]  cbFunc      Bcall语音服务事件回调函数
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalBcallVoiceInit(BcallVoiceCallbak cbFunc);

/**
 * @brief  启动Bcall语音服务
 * @param[in]  BcallVoiceInfo  Bcall语音服务配置
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalBcallVoiceStart(StrBcallVoiceInfo *const BcallVoiceInfo);

/**
 * @brief   hangup Bcall语音服务
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalBcallVoiceHangup(void);

/**
 * @brief  更新Bcall语音服务MSD
 * @param[in]  msd           MSD数据
 * @param[in]  msdLen        MSD数据长度
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalBcallVoiceUpdateMsd(StrBcallVoiceInfo *const pInfo);

/**
 * @brief  推送Bcall语音服务MSD
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalBcallVoicePushMsd(void);

/**
 * @brief  释放Bcall语音模块
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalBcallVoiceDeinit(void);

/**
 * @brief  设置Bcall语音服务配置
 * @param[in]  BcallConfig  Bcall语音服务配置
 * @return     0表示成功,其他表示失败
 */
extern int32_t HalBcallVoiceSetConfigInfo(StrBcallConfig *BcallConfig);

extern void HalBcallAudioPrepare(void);

#ifdef __cplusplus
}
#endif

#endif //_HAL_MODEM_BCALL_H_





