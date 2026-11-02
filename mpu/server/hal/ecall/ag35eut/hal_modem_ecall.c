#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_ecall.h"
#include "ql_type.h"
#include "ql_ms_voice.h"
#include "ql_ecall.h"
#include "ql_audio_pcm.h"
#include "ql_audio_cfg.h"


typedef struct
{
    int32_t inited;
    EcallVoiceCallbak ecallVoiceCbk;
} strHalEcallVoiceSet;

static strHalEcallVoiceSet g_HalEcallVoiceSet = {0};


/**
* @brief 通用状态处理函数
*
* @param msgId: 消息 ID
* @param msgData: 消息数据
* @param msgDataLen: 消息数据长度
*
* @return 无返回值
*/
#if 0
void CommonStateHandler(int ind,void *ind_data,void *userdata)
{    
    if (NULL == g_HalEcallVoiceSet.ecallVoiceCbk)
    {
        LOG_Error("%s faild,  ecallVoiceCbk is not init ! g_HalEcallVoiceSet.ecallVoiceCbk is NULL.",__FUNCTION__);
        return;
    }

    switch(ind){
        case QL_ECALL_EVENT_SENDING_START:
            LOG_Info("QL_ECALL_EVENT_SENDING_START.");
            break;
        case QL_ECALL_EVENT_SENDING_MSD:
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_SNED_MSD);
            LOG_Info("QL_ECALL_EVENT_SENDING_MSD.");
            break;
        case QL_ECALL_EVENT_LLACK_RECEIVED:
            LOG_Info("QL_ECALL_EVENT_LLACK_RECEIVED.");
            break;
        case QL_ECALL_EVENT_ALLACK_POSITIVE_RECEIVED:
            LOG_Info("QL_ECALL_EVENT_ALLACK_POSITIVE_RECEIVED.");
            break;
        case QL_ECALL_EVENT_ALLACK_CLEARDOWN_RECEIVED:
            LOG_Info("QL_ECALL_EVENT_ALLACK_CLEARDOWN_RECEIVED.");
            break;
        case QL_ECALL_EVENT_ACTIVE:
            LOG_Info("QL_ECALL_EVENT_ACTIVE.");
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_ACTIVE);
            break;
        case QL_ECALL_EVENT_DISCONNECTED:
            LOG_Info("QL_ECALL_EVENT_DISCONNECTED.");
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_END);
            break;
        case QL_ECALL_EVENT_ABNORMAL_HANGUP:
            LOG_Info("QL_ECALL_EVENT_ABNORMAL_HANGUP.");
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_ABNORMAL_END);
            break;
        case QL_ECALL_EVENT_ONLY_DEREGISTRATION:
            LOG_Info("QL_ECALL_EVENT_ONLY_DEREGISTRATION.");
            break;
        case QL_ECALL_EVENT_MAY_DEREGISTRATION:
            LOG_Info("QL_ECALL_EVENT_MAY_DEREGISTRATION.");
            break;
        case QL_ECALL_EVENT_PSAP_CALLBACK_START:
            LOG_Info("QL_ECALL_EVENT_PSAP_CALLBACK_START.");
            break;
        case QL_ECALL_EVENT_T2_TIMEOUT:
            LOG_Info("QL_ECALL_EVENT_T2_TIMEOUT.");
            break;
        case QL_ECALL_EVENT_T5_TIMEOUT:
            LOG_Info("QL_ECALL_EVENT_T5_TIMEOUT.");
            break;
        case QL_ECALL_EVENT_T6_TIMEOUT:
            LOG_Info("QL_ECALL_EVENT_T6_TIMEOUT.");
            break;
        case QL_ECALL_EVENT_T7_TIMEOUT:
            LOG_Info("QL_ECALL_EVENT_T7_TIMEOUT.");
            break;
        case QL_ECALL_EVENT_ECALL_STARTED:
            LOG_Info("QL_ECALL_EVENT_ECALL_STARTED.");
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_CALLING);
            break;
        case QL_ECALL_EVENT_INCOMING_CALL:
            LOG_Info("QL_ECALL_EVENT_INCOMING_CALL.");
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_INCOMING);
            break;
        case QL_ECALL_EVENT_DIAL_DURATION_TIMEOUT:
            LOG_Info("QL_ECALL_EVENT_DIAL_DURATION_TIMEOUT.");
            break;
        case QL_ECALL_EVENT_INTERVAL_TIMEOUT:
            LOG_Info("QL_ECALL_EVENT_INTERVAL_TIMEOUT.");
            break;
        case QL_ECALL_EVENT_AUTO_ANSWER_TIMEOUT:
            LOG_Info("QL_ECALL_EVENT_AUTO_ANSWER_TIMEOUT.");
            break;
        default:
            LOG_Info("UNKNOWN.");
            break;
    }

}

#else

void CommonStateHandler(int ind,void *ind_data,void *userdata)
{    
    if (NULL == g_HalEcallVoiceSet.ecallVoiceCbk)
    {
        LOG_Error("%s faild,  ecallVoiceCbk is not init ! g_HalEcallVoiceSet.ecallVoiceCbk is NULL.",__FUNCTION__);
        return;
    }

    switch(ind){
        case QL_ECALL_EVENT_SENDING_START: //MSD准备发送
            LOG_Info("QL_ECALL_EVENT_SENDING_START.");
            break;
        case QL_ECALL_EVENT_SENDING_MSD: //MSD发送中
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_SNED_MSD);
            LOG_Info("QL_ECALL_EVENT_SENDING_MSD.");
            break;
        case QL_ECALL_EVENT_LLACK_RECEIVED: //IVS收到LLACK
            LOG_Info("QL_ECALL_EVENT_LLACK_RECEIVED.");
            break;
        case QL_ECALL_EVENT_ALLACK_POSITIVE_RECEIVED:  //IVS收到ALACK
            LOG_Info("QL_ECALL_EVENT_ALLACK_POSITIVE_RECEIVED.");
            break;
        case QL_ECALL_EVENT_ALLACK_CLEARDOWN_RECEIVED: //收到PSAP挂断请求
            LOG_Info("QL_ECALL_EVENT_ALLACK_CLEARDOWN_RECEIVED.");
            break;
        case QL_ECALL_EVENT_MSD_TRANS_STATUS_SUCCESS: //MSD发送成功
            LOG_Info("QL_ECALL_EVENT_MSD_TRANS_STATUS_SUCCESS.");
            break;
        case QL_ECALL_EVENT_MSD_TRANS_STATUS_FAILURE: //MSD发送失败
            LOG_Info("QL_ECALL_EVENT_MSD_TRANS_STATUS_FAILURE.");
            break;
        case QL_ECALL_EVENT_MSDUPDATE_PSAP_REQURE_UPDATE_MSD: //PSAP请求MSD更新
            LOG_Info("QL_ECALL_EVENT_MSDUPDATE_PSAP_REQURE_UPDATE_MSD.");
            break;
        case QL_ECALL_EVENT_MSDUPDATE_PSAP_REQURE_TIMEOUT: //等待更新超时，推送旧的MSD
            LOG_Info("QL_ECALL_EVENT_MSDUPDATE_PSAP_REQURE_TIMEOUT.");
            break;
        case QL_ECALL_EVENT_ACTIVE: //ECALL接通
            LOG_Info("QL_ECALL_EVENT_ACTIVE .");
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_ACTIVE);
            break;
        case QL_ECALL_EVENT_DISCONNECTED: //ECALL断开
            LOG_Info("QL_ECALL_EVENT_DISCONNECTED .");
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_END);
            break;
        case QL_ECALL_EVENT_ABNORMAL_HANGUP: //ECALL异常挂断
            LOG_Info("QL_ECALL_EVENT_ABNORMAL_HANGUP.");
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_ABNORMAL_END);
            break;
        case QL_ECALL_EVENT_ONLY_DEREGISTRATION: //去注网
            LOG_Info("QL_ECALL_EVENT_ONLY_DEREGISTRATION.");
            break;
        case QL_ECALL_EVENT_MAY_DEREGISTRATION: //暂未使用
            LOG_Info("QL_ECALL_EVENT_MAY_DEREGISTRATION.");
            break;
        case QL_ECALL_EVENT_PSAP_CALLBACK_START: //IVS开启自动接听，等待PSAP回拨
            LOG_Info("QL_ECALL_EVENT_PSAP_CALLBACK_START.");
            break;
        case QL_ECALL_EVENT_ORI_REDIAL_TIME:  //拨打失败定时器超时
            LOG_Info("QL_ECALL_EVENT_ORI_REDIAL_TIME.");
            break;
        case QL_ECALL_EVENT_DROP_REDIAL_TIME: //中途挂断重播定时器超时
            LOG_Info("QL_ECALL_EVENT_DROP_REDIAL_TIME.");
            break;
        case QL_ECALL_EVENT_ECALL_STARTED: //ECALL开始
            LOG_Info("QL_ECALL_EVENT_ECALL_STARTED.");
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_CALLING);
            break;
        case QL_ECALL_EVENT_INCOMING_CALL: //PSAP打入
            LOG_Info("QL_ECALL_EVENT_INCOMING_CALL.");
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_INCOMING);
            break;
        case QL_ECALL_EVENT_T2_TIMEOUT: //T2超时
            LOG_Info("QL_ECALL_EVENT_T2_TIMEOUT.");
            break;
        case QL_ECALL_EVENT_T5_TIMEOUT: //T5超时
            LOG_Info("QL_ECALL_EVENT_T5_TIMEOUT.");
            break;   
        case QL_ECALL_EVENT_T6_TIMEOUT: //T6超时
            LOG_Info("QL_ECALL_EVENT_T6_TIMEOUT.");
            break;  
        case QL_ECALL_EVENT_T7_TIMEOUT: //T7超时
            LOG_Info("QL_ECALL_EVENT_T7_TIMEOUT.");
            break;     
        case QL_ECALL_EVENT_DIAL_DURATION_TIMEOUT: //拨号时间超时
            LOG_Info("QL_ECALL_EVENT_DIAL_DURATION_TIMEOUT.");
            break;  
        case QL_ECALL_EVENT_INTERVAL_TIMEOUT: //内部定时器，客户不需要处理
            LOG_Info("QL_ECALL_EVENT_INTERVAL_TIMEOUT.");
            break;   
        case QL_ECALL_EVENT_AUTO_ANSWER_TIMEOUT: //自动接听定时器超时
            LOG_Info("QL_ECALL_EVENT_AUTO_ANSWER_TIMEOUT.");
            break;                
        default:
            LOG_Info("UNKNOWN.");
            break;
    }

}
#endif

void HalEcallAudioPrepare(void)
{
    int32_t ReturnValue = 0;
 /*   
    system("ql_cmd at AT+QCFG=\"nwscanmode\",1");
    for(int i = 0; i < 4; i++)
    {
        system("ql_cmd at AT+cops=1,2,00101,0");
        sleep(2);
    }
*/
    ReturnValue = ql_audio_init();
    if(0 != ReturnValue)
    {
        LOG_Info("%s Failed to init audio service, ret = %d", __FUNCTION__, ReturnValue);
    }
    ReturnValue = ql_audio_set_rx_voice_spkr_gain(65535);
    if (0 != ReturnValue)
    {
        LOG_Info("%s, Failed to set rx voice spkr gain, ret = %d", __FUNCTION__, ReturnValue);
    }

}

/**
* @brief 初始化 eCall 资源
*
* @param cbFunc: 回调函数，用于处理 eCall 事件
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEcallVoiceInit(EcallVoiceCallbak cbFunc)
{
    int32_t returnValue = 0;
    
    if (1 == g_HalEcallVoiceSet.inited)
    {
        LOG_Info("HalEcallVoiceInit already init");
        return RET_OK; // 已经启动
    }
    
    // 使用 ql_ecall_init 初始化 eCall 服务
    returnValue = ql_ecall_init();
    if(returnValue != QL_ERR_OK)
    {
        LOG_Error("ql_ecall_init failed. return value:%d.", returnValue);
        return RET_ERR; // 初始化失败
    }

    // 使用 ql_ecall_set_user_ind_cb 注册 eCall 状态回调函数
    ql_ecall_set_user_ind_cb(CommonStateHandler, NULL);

    // 设置回调函数
    g_HalEcallVoiceSet.ecallVoiceCbk = cbFunc;
    g_HalEcallVoiceSet.inited = 1;
    
    LOG_Info("%s success .",__FUNCTION__);
    return RET_OK;
}

/**
* @brief 释放 eCall 资源
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEcallVoiceDeinit() {
    if (1 == g_HalEcallVoiceSet.inited) {

        // 调用 ql_ecall_deinit 函数释放 eCall 语音服务
        ql_ecall_deinit();

        g_HalEcallVoiceSet.inited = 0;
        g_HalEcallVoiceSet.ecallVoiceCbk = NULL;
    }

    return RET_OK;
}

/**
* @brief 使能ECALL语音功能
*
* @return RET_OK表示ECALL语音功能使能成功，RET_ERR表示失败。
*/
int32_t HalEcallVoiceEnable(void)
{
    // CET保留
    return RET_OK;
}
/**
* @brief 禁用语音功能
*
* @return 成功返回0，失败返回非0
*/

int32_t HalEcallVoiceDisable(void)
{
    // CET保留
    return RET_OK;
}

/**
* @brief 设置语音配置信息
*
* @param ecallConfig: 语音配置信息结构体指针
*
* @return 成功返回0，失败返回非0
*/
int32_t HalEcallVoiceSetConfigInfo(StrEcallConfig *ecallConfig)
{
    int32_t returnValue = RET_OK;
    ql_ecall_config_t ecall_context_info = {0};

    if (0 == g_HalEcallVoiceSet.inited)
    {
        LOG_Error("%s faild,Ecall Voice not init.",__FUNCTION__);
        return RET_ERR; // 未启动
    }
    
    if(NULL == ecallConfig)
    {
        LOG_Error("%s faild,ecallConfig is NULL.",__FUNCTION__);
        return RET_ERR; 
    }
    
    // 设置 eCall 配置信息
    ecall_context_info.t5_timeout_ms_valid = 1;
    ecall_context_info.t5_timeout_ms = ecallConfig->ecallConfigStartTimer;
    ecall_context_info.t6_timeout_ms_valid = 1;
    ecall_context_info.t6_timeout_ms = ecallConfig->ecallConfigHackTimer;
    ecall_context_info.t7_timeout_ms_valid = 1;
    ecall_context_info.t7_timeout_ms = ecallConfig->ecallConfigMsdTransmission;
    ecall_context_info.dialDurationTimer_timout_ms_valid = 1;
    ecall_context_info.dialDurationTimer_timout_ms = ecallConfig->ecallConfigRedialDurationTimer;
    
    int ret = ql_ecall_set_system_std(ecallConfig->ecallConfigSystem);
    if (ret != QL_ERR_OK) {
        LOG_Error("ql_ecall_set_system_std faild, return value:%d ConfigSystem : %s .",
            ret,ecallConfig->ecallConfigSystem ? "GLONASS" : "EUROPEAN");
        returnValue = RET_ERR;
    }
    // 调用 ql_ecall_set_config_info 函数进行设置
    ql_ecall_set_config_info(ecall_context_info);

    LOG_Info("%s success \n",__FUNCTION__);
    return returnValue;
}

int32_t HalSetMsdData(EcallMsd_t *ecallMsdData)
{
    int ret = QL_ERR_OK;
    msd_Vin_t vin;

    if(!ecallMsdData)
    {
        LOG_Error("%s invalid param", __FUNCTION__);
        return -1;
    }

    LOG_Debug("%s set msd msdVersion: %d", __FUNCTION__, ecallMsdData->msdVersion);
    ret = ql_ecall_set_msd_version(ecallMsdData->msdVersion);
    LOG_Debug("%s ql_ecall_get_msd_version ret = %d", __FUNCTION__, ret);
    if(ret != QL_ERR_OK)
    {
        goto end;
    }

    memset(&vin, 0, sizeof(msd_Vin_t));
    memcpy(&vin.isowmi, ecallMsdData->vin, 3);
    memcpy(&vin.isovds, &ecallMsdData->vin[3], 6);
    memcpy(&vin.isovisModelyear, &ecallMsdData->vin[9], 1);
    memcpy(&vin.isovisSeqPlant, &ecallMsdData->vin[10], 7);

   LOG_Debug("%s set msd vin: %s", __FUNCTION__, ecallMsdData->vin);
    ret = ql_ecall_set_msd_vin(vin);
    LOG_Debug("%s ql_ecall_set_msd_vin ret = %d", __FUNCTION__, ret);
    if(ret != QL_ERR_OK)
    {
        goto end;
    }

    LOG_Debug("%s set msd position isTrusted: %d, latitude: %d, longitude: %d, direction: %d", __FUNCTION__, ecallMsdData->ecallPostion.isTrusted, 
        ecallMsdData->ecallPostion.latitude, ecallMsdData->ecallPostion.longitude, ecallMsdData->ecallPostion.direction);
    ret = ql_ecall_set_msd_position(ecallMsdData->ecallPostion.isTrusted, ecallMsdData->ecallPostion.latitude, 
        ecallMsdData->ecallPostion.longitude, ecallMsdData->ecallPostion.direction);
    LOG_Debug("%s ql_ecall_set_msd_position ret = %d", __FUNCTION__, ret);
    if(ret != QL_ERR_OK)
    {
        goto end;
    }

    LOG_Debug("%s set msd N1 latitudeDelta: %d, longitudeDelta: %d", __FUNCTION__, ecallMsdData->ecallPositionDeltaN1.latitudeDelta, ecallMsdData->ecallPositionDeltaN1.longitudeDelta);
    ret = ql_ecall_set_msd_position_n1(ecallMsdData->ecallPositionDeltaN1.latitudeDelta, ecallMsdData->ecallPositionDeltaN1.longitudeDelta);
    LOG_Debug("%s ql_ecall_set_msd_position_n1 ret = %d", __FUNCTION__, ret);
    if(ret != QL_ERR_OK)
    {
        goto end;
    }

    LOG_Debug("%s set msd N2 latitudeDelta: %d, longitudeDelta: %d", __FUNCTION__, ecallMsdData->ecallPositionDeltaN2.latitudeDelta, ecallMsdData->ecallPositionDeltaN2.longitudeDelta);
    ret = ql_ecall_set_msd_position_n2(ecallMsdData->ecallPositionDeltaN2.latitudeDelta, ecallMsdData->ecallPositionDeltaN2.longitudeDelta);
    LOG_Debug("%s ql_ecall_set_msd_position_n2 ret = %d", __FUNCTION__, ret);
    if(ret != QL_ERR_OK)
    {
        goto end;
    }

    LOG_Debug("%s set msd vehicleType: %d", __FUNCTION__, ecallMsdData->vehicleType);
    ret = ql_ecall_set_msd_vehicle_type(ecallMsdData->vehicleType);
    LOG_Debug("%s ql_ecall_set_msd_vehicle_type ret = %d", __FUNCTION__, ret);
    if(ret != QL_ERR_OK)
    {
        goto end;
    }

    LOG_Debug("%s set msd numberOfPassengers: %d", __FUNCTION__, ecallMsdData->numberOfPassengers);
    ql_ecall_set_msd_passengers_count(ecallMsdData->numberOfPassengers);

    LOG_Debug("%s set msd PropulsionType: %d", __FUNCTION__, ecallMsdData->PropulsionType);
    ql_ecall_set_msd_propulsion_type(ecallMsdData->PropulsionType);

    LOG_Debug("%s set msd testCall: %d", __FUNCTION__, ecallMsdData->testCall);
    ql_ecall_set_msd_call_type(ecallMsdData->testCall);

    ret = ql_ecall_update_msd();
    if(ret != QL_ERR_OK)
    {
        goto end;
    }
    LOG_Debug("%s ql_ecall_update_msd ret = %d", __FUNCTION__, ret);
    
end:
    return ret;
}

/**
* @brief 启动语音功能
*
* @param EcallVoiceInfo: 语音信息结构体指针
*
* @return 成功返回0，失败返回非0
*/
int32_t HalEcallVoiceStart(StrEcallVoiceInfo *const EcallVoiceInfo, EcallMsd_t *ecallMsdData) {

    if (0 == g_HalEcallVoiceSet.inited)
    {
        LOG_Error("%s faild,Ecall Voice not init.",__FUNCTION__);
        return RET_ERR; // 未启动
    }
    
    if(NULL == EcallVoiceInfo)
    {
        LOG_Error("%s faild,EcallVoiceInfo is NULL.",__FUNCTION__);
        return RET_ERR; 
    }
    
    ql_voice_ecall_info_t ecall_info = {0};
    // uint32_t ecall_id = 0;

    ecall_info.msd_len = EcallVoiceInfo->msd_len;
    memcpy(ecall_info.msd, EcallVoiceInfo->ecall_msd, MAX_ECALL_MSD);
    ecall_info.type = (QL_VOICE_ECALL_TYPE_E)EcallVoiceInfo->eCallModeType;
    ecall_info.auto_trigger = EcallVoiceInfo->manual;
    memcpy(ecall_info.test_number, EcallVoiceInfo->PhoneNumber, ECALL_PHONE_NUM_LEN);

    int ret = ql_ecall_set_test_number(EcallVoiceInfo->simId + 1, ecall_info.test_number);
    if (ret != QL_ERR_OK) 
    {
        LOG_Error("ql_ecall_set_test_number faild, return value:%d ." ,ret);
        return RET_ERR;
    }

    ret = HalSetMsdData(ecallMsdData);
    if (ret != QL_ERR_OK) {
        LOG_Error("%s faild, return value:%d .",__FUNCTION__,ret);
        return RET_ERR;
    }

    // ret = ql_ecall_dial(EcallVoiceInfo->simId + 1, &ecall_info, &ecall_id);
    // if (ret != QL_ERR_OK) {
    //     LOG_Error("%s faild, return value:%d .",__FUNCTION__,ret);
    //     return RET_ERR;
    // }
    if(ecall_info.type == QL_VOICE_ECALL_TYPE_TEST)
    {
        ret = ql_ecall_start_test(EcallVoiceInfo->simId + 1);
        if (ret != QL_ERR_OK) 
        {
            LOG_Error("ql_ecall_start_test faild, return value: %d" ,ret);
            return RET_ERR;
        }
    }
    else
    {
       if(ecall_info.auto_trigger == 0)
       {    
            ret = ql_ecall_start_manual(EcallVoiceInfo->simId + 1);
            if (ret != QL_ERR_OK) 
            {
                LOG_Error("ql_ecall_start_manual faild, return value: %d" ,ret);
                return RET_ERR;
            }
       } 
       else
       {
            ret = ql_ecall_start_automatic(EcallVoiceInfo->simId + 1);
            if (ret != QL_ERR_OK) 
            {
                LOG_Error("ql_ecall_start_automatic faild, return value: %d" ,ret);
                return RET_ERR;
            }
       }
    }

    // ret = ql_ecall_update_msd();
    // LOG_Debug("%s ql_ecall_update_msd ret = %d", __FUNCTION__, ret);

    LOG_Info("%s success, return value:%d .",__FUNCTION__,ret);
    return RET_OK;
}

/**
* @brief 挂断 eCall 通话
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEcallVoiceHangup(void)
{
    // 实现挂断 eCall 通话逻辑
    if (0 == g_HalEcallVoiceSet.inited)
    {
        LOG_Error("%s faild,Ecall Voice not init.",__FUNCTION__);
        return RET_ERR; // 未启动
    }

    uint32_t returnValue = 0;
    
    returnValue = ql_ecall_hangup();
    if(QL_ERR_OK !=  returnValue)
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
int32_t HalEcallVoiceUpdateMsd(const int8_t *msd, uint32_t msdLen, EcallMsd_t *ecallMsdData)
{
    if (0 == g_HalEcallVoiceSet.inited)
    {
        LOG_Error("%s faild,Ecall Voice not init.",__FUNCTION__);
        return RET_ERR; // 未启动
    }

    if (0 == msdLen || NULL == msd)
    {
        LOG_Error("%s faild,wrong pramater.",__FUNCTION__);
        return RET_ERR;
    }
    
    int ret = HalSetMsdData(ecallMsdData);
    if (ret != QL_ERR_OK) {
        LOG_Error("%s faild, return value:%d .",__FUNCTION__,ret);
        return RET_ERR;
    }
 //   returnValue = ql_ecall_update_msd_raw((uint8_t *)msd, msdLen);
    // ret = ql_ecall_update_msd();
    // if(QL_ERR_OK !=  ret)
    // {
    //     LOG_Error("%s faild, return value:%d .",__FUNCTION__,ret);
    //     return RET_ERR;
    // }
    LOG_Info("%s success, return value:%d .",__FUNCTION__,ret);
    return RET_OK;
}

/**
* @brief 发送 MSD 以 PUSH 方式
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEcallVoicePushMsd(void)
{
    if (0 == g_HalEcallVoiceSet.inited)
    {
        LOG_Error("%s faild,Ecall Voice not init",__FUNCTION__);
        return RET_ERR; // 未启动
    }
    uint32_t returnValue = 0;
    returnValue = ql_ecall_update_msd();
    if(QL_ERR_OK !=  returnValue)
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
* @param EcallConfigInfo: 用于存储 eCall 配置信息的结构体指针
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEcallVoiceGetConfigInfo(EcallConfigInfo_t *ecallConfigInfo)
{
    ql_ecall_config_t ecall_context_info = {0};
    ECALL_SYSTEM_STD_E system_std;

    if (0 == g_HalEcallVoiceSet.inited)
    {
        LOG_Error("%s faild,Ecall Voice not init.",__FUNCTION__);
        return RET_ERR; // 未启动
    }

    if(NULL == ecallConfigInfo)
    {
        LOG_Error("%s faild,ecallConfigInfo is NULL.",__FUNCTION__);
        return RET_ERR; 
    }
    
    ql_ecall_get_system_std(&system_std);
    ecallConfigInfo->ecall_system = system_std;
    
    // 调用 ql_ecall_get_config_info 函数获取 eCall 配置信息
    ql_ecall_get_config_info(&ecall_context_info);

    // 将获取到的 eCall 配置信息赋值给输出参数
    ecallConfigInfo->T5 = ecall_context_info.t5_timeout_ms;
    ecallConfigInfo->T6 = ecall_context_info.t6_timeout_ms;
    ecallConfigInfo->T7 = ecall_context_info.t7_timeout_ms;
    ecallConfigInfo->redial_duration = ecall_context_info.dialDurationTimer_timout_ms;

    // CET没有接口的参数不赋值

    // 返回成功
    LOG_Info("%s success.",__FUNCTION__);
    return RET_OK; 
}

