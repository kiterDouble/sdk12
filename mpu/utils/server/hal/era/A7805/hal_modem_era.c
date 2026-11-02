#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <Sunsea_Ecall.h>
#include <Sunsea_AT.h>

#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_era.h"


typedef struct
{
    int32_t inited;
    EraVoiceCallbak ecallVoiceCbk;
} strHalEraVoiceSet;

typedef struct {
    char* name;
    uint8_t *valid;
    uint32_t *value;
} RegMapping_t;

static strHalEraVoiceSet g_HalEraVoiceSet = {0};

const char *SunSeaEventToString(sunsea_ecall_event event)
{
    static const char *eventStringTable[] =
    {
    "SUNSEA_ECALL_EVENT_AL_ACK_RECEIVED",              //= 0,  /*AL-ACK signal received */
    "SUNSEA_ECALL_EVENT_EIM_LOST",                     //= 1,  /* eIM lost synchronization with PSAP*/
    "SUNSEA_ECALL_EVENT_NACK_RECEIVED",                //= 2,  /* NACK signal received */
    "SUNSEA_ECALL_EVENT_THREE_RESERVED",               //= 3,  /*Reserved*/
    "SUNSEA_ECALL_EVENT_FOUR_RESERVED",                //= 4,  /*Reserved*/
    "SUNSEA_ECALL_EVENT_FIVE_RESERVED",                //= 5,  /*Reserved*/
    "SUNSEA_ECALL_EVENT_EIM_TERMINATED_FATAL_ERROR",   //= 6,  /*eIM is terminated: fatal error*/
    "SUNSEA_ECALL_EVENT_EIM_TERMINATED_CALL_DROP",     //= 7,  /* eIM is terminated: call drop*/
    "SUNSEA_ECALL_EVENT_GENERIC_ERROR",                //= 8,  /*Generic error*/
    "SUNSEA_ECALL_EVENT_REQUEST_MSD",                  //= 9,  /*PSAP request MSD*/
    "SUNSEA_ECALL_EVENT_START_SEND_MSD",               //= 10, /* Start Send MSD: eIM starts sending MSD*/
    "SUNSEA_ECALL_EVENT_PUSH_MODE_EIM_SEND_SIGNAL",    //= 11, /*The eIM starts sending the Initiation signal (in PUSH mode only)*/
    "SUNSEA_ECALL_EVENT_SEND_MSD_SIGNAL_RECEIVED",     //= 12, /*SEND MSD signal received*/
    "SUNSEA_ECALL_EVENT_SEND_MSD_SIGNAL_TOO_OFTEN",    //= 13, /*SEND MSD signal received too often, PSAP sync failure assumed */
    "SUNSEA_ECALL_EVENT_IVS_SYNCED_PSAP",              //= 14, /*IVS eIM is synced with PSAP */
    "SUNSEA_ECALL_EVENT_IVS_SYNCED_PSAP_EIM",          //= 15, /* IVS eIM is synced with PSAP eIM */
    "SUNSEA_ECALL_EVENT_ACK_RECEIVED",                 //= 16, /*Link layer ACK received*/
    "SUNSEA_ECALL_EVENT_ALACK_RECEIVED",               //= 17, /* Higher Layer ACK received (similar to AL-ACK(#0) but without data) */
    "SUNSEA_ECALL_EVENT_EIM_RESET",                    //= 18, /*The eIM transmitter idles after a full reset*/
    "SUNSEA_ECALL_EVENT_PUSH_MODE_EIM_IDLES",          //= 19, /*The eIM transmitter idles after the transmission of the Initiation signal (push mode) */
    "SUNSEA_ECALL_EVENT_MSD_UPDATE_REQUEST_RECEIVED",  //= 20, /*MSD update request is received.*/
    "SUNSEA_ECALL_EVENT_MSD_UPDATE_DONE",              //= 21, /*MSD update is done*/
    "SUNSEA_ECALL_EVENT_MSD_UPDATE_FAILED",            //= 22, /*MSD update request cannot be processed.*/
    "SUNSEA_ECALL_EVENT_T3_EXPIRED",                   //= 23, /*T3 expired*/
    "SUNSEA_ECALL_EVENT_T5_EXPIRED",                   //= 24, /*T5 expired*/
    "SUNSEA_ECALL_EVENT_T6_EXPIRED",                   //= 25, /*T6 expired*/
    "SUNSEA_ECALL_EVENT_T7_EXPIRED",                   //= 26, /*T7 expired*/
    "SUNSEA_ECALL_EVENT_TH_RESERVED",                  //= 27, /*TH expired, eCall application could use this URC as a flag to play notification sound if needed.*/
    "SUNSEA_ECALL_EVENT_TWENTY_EIGHT_RESERVED",        //= 28, /*Reserved*/
    "SUNSEA_ECALL_EVENT_TWENTY_NINE_RESERVED",         //= 29, /*Reserved*/
    "SUNSEA_ECALL_EVENT_THIRTY_RESERVED",              //= 30, /*Reserved*/
    "SUNSEA_ECALL_EVENT_IVS_EVENT_TIMEOUT",            //= 31, /*IVS event timeout*/
    "SUNSEA_ECALL_EVENT_PSAP_REQUEST_MSD",             //= 32, /*PSAP request MSD*/
    };
    static const char *eventStringTable2[] =
    {
    "SUNSEA_ECALL_STATE_ORIGINATING",                  //= 40, /*call originaing*/
    "SUNSEA_ECALL_STATE_ALERTING",                     //= 41, /*call alerting*/
    "SUNSEA_ECALL_STATE_CONVESATION",                  //= 42, /*call convesation*/
    "SUNSEA_ECALL_STATE_END",                          //= 43, /*call end*/
    "SUNSEA_ECALL_STATE_EXCEPTION_END",                //= 44, /*call exception end*/
    };
    if(event >= SUNSEA_ECALL_STATE_ORIGINATING && event <= SUNSEA_ECALL_STATE_EXCEPTION_END)
        return eventStringTable2[event-SUNSEA_ECALL_STATE_ORIGINATING];
    else if(event >= SUNSEA_ECALL_EVENT_AL_ACK_RECEIVED && event <= SUNSEA_ECALL_EVENT_PSAP_REQUEST_MSD)
        return eventStringTable[event];

    return "unknown";
}

/**
* @brief 通用状态处理函数
*
* @param msgId: 消息 ID
* @param msgData: 消息数据
* @param msgDataLen: 消息数据长度
*
* @return 无返回值
*/
void EraCommonStateHandler(sunsea_ecall_event event, sunsea_ecall_event_data *data)/*具体状态含义需要对接一下*/
{
    LOG_Warn("simcom enter event <%s>(%d)", SunSeaEventToString(event), event);

    if (NULL == g_HalEraVoiceSet.ecallVoiceCbk)
    {
        LOG_Error("%s failed, ecallVoiceCbk is not init! g_HalEraVoiceSet.ecallVoiceCbk is NULL.", __FUNCTION__);
        return;
    }

    switch(event) {
        case SUNSEA_ECALL_EVENT_AL_ACK_RECEIVED:
            break;
        case SUNSEA_ECALL_EVENT_EIM_LOST:
            break;
        case SUNSEA_ECALL_EVENT_NACK_RECEIVED:
            break;
        case SUNSEA_ECALL_EVENT_THREE_RESERVED:
            break;
        case SUNSEA_ECALL_EVENT_FOUR_RESERVED:
            break;
        case SUNSEA_ECALL_EVENT_FIVE_RESERVED:
            break;
        case SUNSEA_ECALL_EVENT_EIM_TERMINATED_FATAL_ERROR:
            break;
        case SUNSEA_ECALL_EVENT_EIM_TERMINATED_CALL_DROP:
            break;
        case SUNSEA_ECALL_EVENT_GENERIC_ERROR:
            break;
        case SUNSEA_ECALL_EVENT_REQUEST_MSD:
            g_HalEraVoiceSet.ecallVoiceCbk(ECALL_STATUS_UPDATE_MSD);
            break;
        case SUNSEA_ECALL_EVENT_START_SEND_MSD:
            break;
        case SUNSEA_ECALL_EVENT_PUSH_MODE_EIM_SEND_SIGNAL:
            //g_HalEraVoiceSet.ecallVoiceCbk(ECALL_STATUS_SNED_MSD);
            break;
        case SUNSEA_ECALL_EVENT_SEND_MSD_SIGNAL_RECEIVED:
            break;
        case SUNSEA_ECALL_EVENT_SEND_MSD_SIGNAL_TOO_OFTEN:
            break;
        case SUNSEA_ECALL_EVENT_IVS_SYNCED_PSAP:
            break;
        case SUNSEA_ECALL_EVENT_IVS_SYNCED_PSAP_EIM:
            break;
        case SUNSEA_ECALL_EVENT_ACK_RECEIVED:
            break;
        case SUNSEA_ECALL_EVENT_ALACK_RECEIVED:
            break;
        case SUNSEA_ECALL_EVENT_EIM_RESET:
            break;
        case SUNSEA_ECALL_EVENT_PUSH_MODE_EIM_IDLES:
            break;
        case SUNSEA_ECALL_EVENT_MSD_UPDATE_REQUEST_RECEIVED:
            break;
        case SUNSEA_ECALL_EVENT_MSD_UPDATE_DONE:
//            g_HalEraVoiceSet.ecallVoiceCbk(ECALL_STATUS_PUSH_MSD);
            break;
        case SUNSEA_ECALL_EVENT_MSD_UPDATE_FAILED:
            break;
        case SUNSEA_ECALL_EVENT_T3_EXPIRED:
            break;
        case SUNSEA_ECALL_EVENT_T5_EXPIRED:
            break;
        case SUNSEA_ECALL_EVENT_T6_EXPIRED:
            break;
        case SUNSEA_ECALL_EVENT_T7_EXPIRED:
            break;
        case SUNSEA_ECALL_EVENT_TH_RESERVED:
            break;
        case SUNSEA_ECALL_EVENT_TWENTY_EIGHT_RESERVED:
            break;
        case SUNSEA_ECALL_EVENT_TWENTY_NINE_RESERVED:
            break;
        case SUNSEA_ECALL_EVENT_THIRTY_RESERVED:
            break;
        case SUNSEA_ECALL_EVENT_IVS_EVENT_TIMEOUT:
            break;
        case SUNSEA_ECALL_STATE_ORIGINATING:
            g_HalEraVoiceSet.ecallVoiceCbk(ECALL_STATUS_CALLING);
            break;
        case SUNSEA_ECALL_STATE_ALERTING:               /* 对方振铃 */
            //g_HalEraVoiceSet.ecallVoiceCbk(ECALL_STATUS_ALERTING);
            break;
        case SUNSEA_ECALL_STATE_CONVESATION:  //回拨
            g_HalEraVoiceSet.ecallVoiceCbk(ECALL_STATUS_ACTIVE);
            break;
        case SUNSEA_ECALL_STATE_END: //正常结束
            g_HalEraVoiceSet.ecallVoiceCbk(ECALL_STATUS_END);
            break;
        case SUNSEA_ECALL_STATE_EXCEPTION_END:   //通话异常结束
            g_HalEraVoiceSet.ecallVoiceCbk(ECALL_STATUS_ABNORMAL_END);
            break;
        default:
            LOG_Info("UNKNOWN.");
            break;
    }
    LOG_Warn("simcom exit event: %d", event);
}


/**
* @brief 初始化 eCall 资源
*
* @param cbFunc: 回调函数，用于处理 eCall 事件
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEraVoiceInit(EraVoiceCallbak cbFunc)
{
    int32_t returnValue = 0;
    
    if (1 == g_HalEraVoiceSet.inited)
    {
        LOG_Info("HalEraVoiceInit already init");
        return RET_OK; // 已经启动
    }
    Sunsea_AT_Init();
    // 使用 ql_ecall_init 初始化 eCall 服务
    returnValue = Sunsea_Ecall_Init(EraCommonStateHandler);
    if(returnValue != 0)
    {
        LOG_Error("ql_ecall_init failed. return value:%d.", returnValue);
        return RET_ERR; // 初始化失败
    }

    // 设置回调函数
    g_HalEraVoiceSet.ecallVoiceCbk = cbFunc;
    g_HalEraVoiceSet.inited = 1;
    
    LOG_Info("%s success .",__FUNCTION__);
    return RET_OK;
}

/**
* @brief 释放 eCall 资源
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEraVoiceDeinit() {
    if (1 == g_HalEraVoiceSet.inited) {

        // 调用 ql_ecall_deinit 函数释放 eCall 语音服务
        Sunsea_Ecall_Deinit();
        Sunsea_AT_Deinit();
        g_HalEraVoiceSet.inited = 0;
        g_HalEraVoiceSet.ecallVoiceCbk = NULL;
    }

    return RET_OK;
}

/**
* @brief 使能ECALL语音功能
*
* @return RET_OK表示ECALL语音功能使能成功，RET_ERR表示失败。
*/
int32_t HalEraVoiceEnable(void)
{
    //A7805保留
    return RET_OK;
}
/**
* @brief 禁用语音功能
*
* @return 成功返回0，失败返回非0
*/

int32_t HalEraVoiceDisable(void)
{
    //A7805保留
    return RET_OK;
}

/**
* @brief 设置语音配置信息
*
* @param ecallConfig: 语音配置信息结构体指针
*
* @return 成功返回0，失败返回非0
*/
int32_t HalEraVoiceSetConfigInfo(StrEraConfig *ecallConfig)
{
    int32_t returnValue = RET_OK;
    sunsea_ecall_config ecall_context_info = {0};

    if (0 == g_HalEraVoiceSet.inited)
    {
        LOG_Error("%s faild,Era Voice not init.",__FUNCTION__);
        return RET_ERR; // 未启动
    }
    
    if(NULL == ecallConfig)
    {
        LOG_Error("%s faild,ecallConfig is NULL.",__FUNCTION__);
        return RET_ERR; 
    }
    
    // 设置 eCall 配置信息
    if(ecallConfig->ecallConfigMode == 1)
    {
        ecall_context_info.ecallonly_valid = 1;
        ecall_context_info.ecallonly = 1;            /*1：only 0: normal*/
    }
    ecall_context_info.ecallmode_valid = 1;
    if(ecallConfig->ecallConfigSystem == 0) //EU
    {
        ecall_context_info.ecallmode = SUNSEA_ECALL_EU_MODE;
    }
    else //ERA
    {
        ecall_context_info.ecallmode = SUNSEA_ECALL_ERA_MODE;
    }
    ecall_context_info.cleardownTimer_timeout_ms_valid = 1;  //t2
    ecall_context_info.cleardownTimer_timeout_ms = ecallConfig->ecallConfigCleardownTimer;
    ecall_context_info.t5_timeout_ms_valid = 1;
    ecall_context_info.t5_timeout_ms = ecallConfig->ecallConfigStartTimer;
    ecall_context_info.t6_timeout_ms_valid = 1;
    ecall_context_info.t6_timeout_ms = ecallConfig->ecallConfigHackTimer;
    ecall_context_info.t7_timeout_ms_valid = 1;
    ecall_context_info.t7_timeout_ms = ecallConfig->ecallConfigMsdTransmission;
    ecall_context_info.autoAnswer_timeout_ms_valid = 1;
    ecall_context_info.autoAnswer_timeout_ms = ecallConfig->ecallConfigNadRegistrationTimer;
    ecall_context_info.deregister_network_timeout_ms_valid = 1;
    ecall_context_info.deregister_network_timeout_ms = ecallConfig->ecallConfigNadUnregistrationTimer;
    ecall_context_info.dialDurationTimer_timout_ms_valid = 1;
    ecall_context_info.dialDurationTimer_timout_ms = ecallConfig->ecallConfigRedialDurationTimer;
    
    //其他保留
    
    // 调用 ql_ecall_set_config_info 函数进行设置
    LOG_Info("ecall_context_info.ecallonly : %d \n",ecall_context_info.ecallonly);
    Sunsea_Ecall_Set_Config_Info(ecall_context_info);

    LOG_Info("%s success \n",__FUNCTION__);
    return returnValue;
}
/**
* @brief 根据法规字段设置ECALL语音配置信息
*
* @param ecallConfig: 语音配置信息结构体指针
*
* @return 成功返回0，失败返回非0
*/
int32_t HalEraVoiceSetRegulationsInfo(char *name,char *value)
{
    sunsea_ecall_config ecall_context_info = {0};
    int i = 0;
    
    RegMapping_t configMapping[] = {
        {"CALL_AUTO_ANSWER_TIME",&ecall_context_info.autoAnswer_timeout_ms_valid,&ecall_context_info.autoAnswer_timeout_ms},
        {"POST_TEST_REGISTRATION_TIME", &ecall_context_info.deregister_network_timeout_ms_valid,&ecall_context_info.deregister_network_timeout_ms},
        {"INT_MEM_TRANSMIT_INTERVAL",&ecall_context_info.intervalSmsBetweenAttempts_valid, (uint32_t *)&ecall_context_info.intervalSmsBetweenAttempts},
        {"INT_MEM_TRANSMIT_ATTEMPTS",&ecall_context_info.maxSmsSendAttempts_valid, &ecall_context_info.maxSmsSendAttempts},
        {"CCFT",&ecall_context_info.cleardownTimer_timeout_ms_valid, &ecall_context_info.cleardownTimer_timeout_ms},
        {"MSD_MAX_TRANSMISSION_TIME",&ecall_context_info.t7_timeout_ms_valid, (uint32_t *)&ecall_context_info.t7_timeout_ms},
        {"NAD_DEREGISTRATION_TIME",&ecall_context_info.deregister_network_timeout_ms_valid, &ecall_context_info.deregister_network_timeout_ms},
        {"ECALL_AUTO_DIAL_ATTEMPTS",&ecall_context_info.maxDialAttempts_valid, &ecall_context_info.maxDialAttempts},
    };
    int num_configs = sizeof(configMapping) / sizeof(configMapping[0]);
    
    for (i = 0; i < num_configs; i++)
    {
        if (strcmp(name, configMapping[i].name) == 0)
        {
            *configMapping[i].valid = 1;
            *configMapping[i].value = (uint32_t)atoi(value);
            break;
        }
    }
    
    if (i == num_configs)
    {
        LOG_Warn("%s : can not find %s.",__FUNCTION__,name);
        return RET_ERR;
    }
    
    Sunsea_Ecall_Set_Config_Info(ecall_context_info);

    LOG_Debug("%s : set %s SUCCESS.",__FUNCTION__,name);
    return RET_OK;
}


/**
* @brief 启动语音功能
*
* @param EraVoiceInfo: 语音信息结构体指针
*
* @return 成功返回0，失败返回非0
*/
int32_t HalEraVoiceStart(StrEraVoiceInfo *const EcallVoiceInfo)/*没有设置sim卡的接口*/
{

    if (0 == g_HalEraVoiceSet.inited)
    {
        LOG_Error("%s faild,Era Voice not init.",__FUNCTION__);
        return RET_ERR; // 未启动
    }
    
    if(NULL == EcallVoiceInfo)
    {
        LOG_Error("%s faild,EcallVoiceInfo is NULL.",__FUNCTION__);
        return RET_ERR; 
    }
    
    sunsea_voice_ecall_info ecall_info = {0};

    ecall_info.msd_len = EcallVoiceInfo->msd_len;
    memcpy(ecall_info.msd, EcallVoiceInfo->ecall_msd, MAX_ERA_MSD);
    if(EcallVoiceInfo->eCallModeType == 1 || EcallVoiceInfo->eCallModeType == 2)
    {
        ecall_info.type = (SUNSEA_VOICE_ECALL_TYPE)EcallVoiceInfo->eCallModeType;
    }
    else
    {
        ecall_info.type = SUNSEA_VOICE_ECALL_TYPE_EMERGENCY;
    }
    ecall_info.auto_trigger = EcallVoiceInfo->manual;
    memcpy(ecall_info.test_number, EcallVoiceInfo->PhoneNumber, ERA_PHONE_NUM_LEN);

    LOG_Info("ecall_info.type : %d",ecall_info.type);
    int ret = Sunsea_Ecall_Start(&ecall_info);
    if (ret != 0) {
        LOG_Error("ecall start faild, return value : %d",ret);
        return RET_ERR;
    }
    LOG_Info("ecall start success, return value : %d", ret);
    return RET_OK;
}

/**
* @brief 挂断 eCall 通话
*
* @return 成功返回 0，失败返回非 0
*/

int32_t HalEraVoiceHangup(void)
{
    // 实现挂断 eCall 通话逻辑
    if (0 == g_HalEraVoiceSet.inited)
    {
        LOG_Error("%s faild,Era Voice not init.",__FUNCTION__);
        return RET_ERR; // 未启动
    }

    uint32_t returnValue = 0;
    
    returnValue = Sunsea_Ecall_End();
    if(0 !=  returnValue)
    {
        LOG_Error("%s faild, return value:%d .",__FUNCTION__,returnValue);
        return RET_ERR;
    }
    LOG_Info("%s success, return value:%d .",__FUNCTION__,returnValue);
    return RET_OK;
}

/**
* @brief 更新 MSD
*
* @param msd: 要更新的 MSD 数据
* @param msdLen: MSD 数据长度
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEraVoiceUpdateMsd(const int8_t *msd, uint32_t msdLen)
{
    if (0 == g_HalEraVoiceSet.inited)
    {
        LOG_Error("%s faild,Era Voice not init.",__FUNCTION__);
        return RET_ERR; // 未启动
    }

    if (0 == msdLen || NULL == msd)
    {
        LOG_Error("%s faild,wrong pramater.",__FUNCTION__);
        return RET_ERR;
    }
    
    uint32_t returnValue = 0;
    returnValue = Sunsea_Ecall_UpdateMSD((uint8_t *)msd, msdLen);
    if(returnValue !=  0)
    {
        LOG_Error("%s faild, return value:%d .",__FUNCTION__,returnValue);
        return RET_ERR;
    }
    LOG_Info("%s success, return value:%d .",__FUNCTION__,returnValue);
    return RET_OK;
}

/**
* @brief 发送 MSD 以 PUSH 方式
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEraVoicePushMsd(void)
{
    if (0 == g_HalEraVoiceSet.inited)
    {
        LOG_Error("%s faild,Era Voice not init",__FUNCTION__);
        return RET_ERR; // 未启动
    }
    uint32_t returnValue = 0;
    returnValue = Sunsea_Ecall_PushMSD();
    if(0 !=  returnValue)
    {
        LOG_Error("%s faild, return value:%d .",__FUNCTION__,returnValue);
        return RET_ERR;
    }
    LOG_Info("%s success, return value:%d .",__FUNCTION__,returnValue);
    return RET_OK;
}
/**
* @brief 获取 eCall 配置信息
*
* @param EraConfigInfo: 用于存储 eCall 配置信息的结构体指针
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEraVoiceGetConfigInfo(EraConfigInfo_t *ecallConfigInfo)
{
    // A7805 保留
    return RET_OK; 
}

