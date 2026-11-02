#include <ql_oe.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_ecall.h"

typedef struct
{
    int32_t inited;
    voice_client_handle_type h_voice;
    EcallVoiceCallbak ecallVoiceCbk;
} strHalEcallVoiceSet;

static strHalEcallVoiceSet g_HalEcallVoiceSet = {0};

static void HandlerVoiceCallInd(void *msgData, uint32_t msgDataLen)
{
    int32_t ecallStatus = 0;
    if(msgDataLen != sizeof(ql_mcm_voice_call_ind))
    {
        return;
    }

    ql_mcm_voice_call_ind *pVoiceCallInd = (ql_mcm_voice_call_ind*)msgData;

    char *callState[] = {"INCOMING", "DIALING", "ALERTING", "ACTIVE", "HOLDING", "END", "WAITING"};

    int i = 0;
    for(i = 0; i < pVoiceCallInd->calls_len; i++)
    {
        int endReasonIdx = 0;
        LOG_Info("EU-ECALL Call id=%d, PhoneNum:%s, event=%s!  call_end_reason_valid:%d ,", 
        pVoiceCallInd->calls[i].call_id, pVoiceCallInd->calls[i].number, callState[pVoiceCallInd->calls[i].state], pVoiceCallInd->calls[i].call_end_reason_valid);
        ecallStatus = pVoiceCallInd->calls[i].state;

        if(ecallStatus == E_QL_MCM_VOICE_CALL_STATE_INCOMING)
        {
            LOG_Info("%s: STATUS VALUE: %d .", "ECALL_STATUS_INCOMING",ECALL_STATUS_INCOMING);
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_INCOMING);
        }

        if(pVoiceCallInd->calls[i].call_end_reason_valid)
        {
            endReasonIdx = pVoiceCallInd->calls[i].call_end_reason;
        }
        if((pVoiceCallInd->calls[i].call_end_reason_valid == 1) && (ecallStatus == E_QL_MCM_VOICE_CALL_STATE_END)
            && !((E_QL_MCM_VOICE_CALL_END_CAUSE_REL_NORMAL == endReasonIdx) //7
             || (E_QL_MCM_VOICE_CALL_END_CAUSE_CLIENT_END == endReasonIdx) //11
             || (E_QL_MCM_VOICE_CALL_END_CAUSE_NORMAL_CALL_CLEARING == endReasonIdx) //88
             || (E_QL_MCM_VOICE_CALL_END_CAUSE_USER_BUSY == endReasonIdx) //89
             || (E_QL_MCM_VOICE_CALL_END_CAUSE_NORMAL_UNSPECIFIED == endReasonIdx)/*99*/))
        {
            LOG_Info("%s: STATUS VALUE: %d STATUS REASON : %d .", "ECALL_STATUS_ABNORMAL_END",ECALL_STATUS_ABNORMAL_END,endReasonIdx);
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_ABNORMAL_END);

        }
        else if(ecallStatus == E_QL_MCM_VOICE_CALL_STATE_END)
        {
            LOG_Info("%s: STATUS VALUE: %d STATUS REASON : %d .", "ECALL_STATUS_END",ECALL_STATUS_END,endReasonIdx);
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_END);
            LOG_Info("%s: STATUS VALUE: %d .", "ECALL_STATUS_WAIT_PSAP",ECALL_STATUS_WAIT_PSAP);
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_WAIT_PSAP);
        }
        else if(ecallStatus == E_QL_MCM_VOICE_CALL_STATE_DIALING || ecallStatus == E_QL_MCM_VOICE_CALL_STATE_ALERTING)
        {
            LOG_Info("%s: STATUS VALUE: %d .", "ECALL_STATUS_CALLING",ECALL_STATUS_CALLING);
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_CALLING);
        }
        else if(ecallStatus == E_QL_MCM_VOICE_CALL_STATE_ACTIVE)
        {
            LOG_Info("%s: STATUS VALUE: %d .", "ECALL_STATUS_ACTIVE",ECALL_STATUS_ACTIVE);
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_ACTIVE);
        }
        else
        {
            // reserve
        }
    }
}

static void HandlerVoiceCallStatueInd(void *msgData, uint32_t msgDataLen)
{
    if(msgDataLen != sizeof(ql_mcm_voice_ecall_status_ind))
    {
        return;
    }
    
    ql_mcm_voice_ecall_status_ind *pG_ecallStatusInd 
                                   = (ql_mcm_voice_ecall_status_ind*)msgData;
    
    if (pG_ecallStatusInd->ecall_msd_tx_status_valid)
    {
        if (pG_ecallStatusInd->ecall_msd_tx_status == E_QL_MCM_VOICE_ECALL_MSD_TRANSMISSION_STATUS_SUCCESS)
        {
            LOG_Info("%s: STATUS VALUE: %d .", "ECALL_STATUS_MSD_SNED_SUCCESS",ECALL_STATUS_MSD_SNED_SUCCESS);
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_MSD_SNED_SUCCESS);
        }
        else if(pG_ecallStatusInd->ecall_msd_tx_status == E_QL_MCM_VOICE_ECALL_MSD_TRANSMISSION_STATUS_FAILURE)
        {
            LOG_Info("%s: STATUS VALUE: %d .", "ECALL_STATUS_MSD_SNED_FAULT",ECALL_STATUS_MSD_SNED_FAULT);
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_MSD_SNED_FAULT);
        }
    }
    else
    {
        LOG_Info("========== Ecall status  call_id =%d  ", pG_ecallStatusInd->call_id);
    }
    
}

static void HandlerVoiceCallEventInd(void *msgData, uint32_t msgDataLen)
{
    if(msgDataLen != sizeof(ql_mcm_voice_ecall_event_ind))
    {
        return;
    }
    ql_mcm_voice_ecall_event_ind *pEcallEventInd = (ql_mcm_voice_ecall_event_ind*)msgData;
    
    if(pEcallEventInd->ecall_event_fails_valid)
    {
        
        LOG_Info("%s: STATUS VALUE: %d STATUS REASON:%d .", "ECALL_STATUS_MSD_SNED_FAULT",ECALL_STATUS_MSD_SNED_FAULT,pEcallEventInd->ecall_event_fails);
        g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_MSD_SNED_FAULT);
    
    }
    
    if(pEcallEventInd->ecall_event_process_valid)
    {
        if(pEcallEventInd->ecall_event_process == E_QL_MCM_VOICE_ECALL_EVENT_PROCESS_IVS_START_RECEIVED_MSD)
        {
            LOG_Info("%s: STATUS VALUE: %d .", "ECALL_STATUS_SNED_MSD",ECALL_STATUS_SNED_MSD);
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_SNED_MSD);
        }
        LOG_Info("========== ecall_event_process_valid:%d   ==========",pEcallEventInd->ecall_event_process);                
    }
    
    if(pEcallEventInd->ecall_event_msdupdate_valid)
    {
        if(pEcallEventInd->ecall_event_msdupdate == E_QL_MCM_VOICE_ECALL_EVENT_MSDUPDATE_IVS_UPDATING_MSD)
            LOG_Info("========== ecall_event_msdupdate: %d ==========  ",pEcallEventInd->ecall_event_msdupdate);
        if(pEcallEventInd->ecall_event_msdupdate == E_QL_MCM_VOICE_ECALL_EVENT_MSDUPDATE_PSAP_REQURE_UPDATE_MSD)
        {
            LOG_Info("%s: STATUS VALUE: %d .", "ECALL_STATUS_PUSH_MSD",ECALL_STATUS_UPDATE_MSD);
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_UPDATE_MSD);
        }
        if(pEcallEventInd->ecall_event_msdupdate == E_QL_MCM_VOICE_ECALL_EVENT_MSDUPDATE_IVS_UPDATE_MSD_TIMEOUT)
        {
            LOG_Info("%s: STATUS VALUE: %d .", "ECALL_STATUS_PUSH_MSD",ECALL_STATUS_PUSH_MSD);
            g_HalEcallVoiceSet.ecallVoiceCbk(ECALL_STATUS_PUSH_MSD);
        }
    }
    if(pEcallEventInd->ecall_event_establish_valid)
    {
        LOG_Info("========== ecall_event_establish: %d ==========  ",pEcallEventInd->ecall_event_establish);
    }
    
    if(pEcallEventInd->ecall_event_hackcode_valid)
    {
        LOG_Info("========== Ecall IND EVENT:   ecall_event_hackcode:%d  ========== ", pEcallEventInd->ecall_event_hackcode.hack_code);
    }
    
    if(pEcallEventInd->ecall_event_ori_redial_valid)
    {
        LOG_Info("========== Ecall IND EVENT:  ori_remainder_times:%d     time:%d  ========== ",
        pEcallEventInd->ecall_event_ori_redial.ori_remainder_times,pEcallEventInd->ecall_event_ori_redial.time);
    }
    
    if(pEcallEventInd->ecall_event_drop_redial_valid)
    {
        LOG_Info("========== Ecall IND EVENT:  drop_remainder_times:%d    time:%d  ========== ",
        pEcallEventInd->ecall_event_drop_redial.drop_remainder_times,pEcallEventInd->ecall_event_drop_redial.time);
    }
    if(pEcallEventInd->ecall_event_extend_state_valid)
    {
        LOG_Info("========== ecall_event_extend_state: %d ==========  ",pEcallEventInd->ecall_event_extend_state);
    }
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
void CommonStateHandler(unsigned long msgId, void *msgData, uint32_t msgDataLen)
{    
    if(NULL == msgData)
    {
        LOG_Error("%s faild,  Unavailable parameters ! msgData is NULL ",__FUNCTION__);
        return;
    }
    if (NULL == g_HalEcallVoiceSet.ecallVoiceCbk)
    {
        LOG_Error("%s faild,  ecallVoiceCbk is not init ! g_HalEcallVoiceSet.ecallVoiceCbk is NULL ",__FUNCTION__);
        return;
    }

    switch(msgId)
    {
        case E_QL_MCM_VOICE_CALL_IND:
        {
            HandlerVoiceCallInd(msgData,msgDataLen);
            break;
        }

        case E_QL_MCM_VOICE_ECALL_STATUE_IND:
        {
            HandlerVoiceCallStatueInd(msgData,msgDataLen);
            break;
        }
        case E_QL_MCM_VOICE_ECALL_EVENT_IND:
        {
            HandlerVoiceCallEventInd(msgData,msgDataLen);
            break;
        }
        case E_QL_MCM_VOICE_UNKOWN_IND:
            LOG_Info("========== Ecall IND EVENT ==========  ");
        default:
            break;
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
    
    // 使用 QL_Voice_Call_Client_Init 初始化语音服务句柄
    returnValue = QL_Voice_Call_Client_Init(&g_HalEcallVoiceSet.h_voice);
    if(returnValue < 0)
    {
        LOG_Error("QL_Voice_Call_Client_Init faild. return value:%d ",returnValue);
        return RET_ERR; // 初始化失败
    }

    // 注册 eCall 状态回调函数
    returnValue = QL_Voice_Call_AddCommonStateHandler(g_HalEcallVoiceSet.h_voice, CommonStateHandler);
    if(returnValue < 0)
    {
        QL_Voice_Call_Client_Deinit(g_HalEcallVoiceSet.h_voice); // 初始化失败，需要反初始化
        LOG_Error("QL_Voice_Call_AddCommonStateHandler faild. return value:%d ",returnValue);
        return RET_ERR; // 注册失败
    }

    // 设置回调函数
    g_HalEcallVoiceSet.ecallVoiceCbk = cbFunc;
    g_HalEcallVoiceSet.inited = 1;

    return RET_OK;
}
/**
* @brief 释放 eCall 资源
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEcallVoiceDeinit()
{
    // Deinitialize the voice service handle
    if (1 == g_HalEcallVoiceSet.inited)
    {
        // Remove the common state handler before deinitializing
        QL_Voice_Call_RemoveCommonStateHandler(g_HalEcallVoiceSet.h_voice);

        QL_Voice_Call_Client_Deinit(g_HalEcallVoiceSet.h_voice);
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
    if (0 == g_HalEcallVoiceSet.inited)
    {
        LOG_Error("%s faild,Ecall Voice not init",__FUNCTION__);
        return RET_ERR; 
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_ENABLE, 1) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_ENABLE");
        return RET_ERR; // 设置失败
    }

    LOG_Info("%s success,set %s success. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_ENABLE");
    return RET_OK;
}
/**
* @brief 禁用语音功能
*
* @return 成功返回0，失败返回非0
*/
int32_t HalEcallVoiceDisable(void)
{
    if (0 == g_HalEcallVoiceSet.inited)
    {
        LOG_Error("%s faild,Ecall Voice not init",__FUNCTION__);
        return RET_OK;
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_ENABLE, 0) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_ENABLE");
        return RET_ERR; // 设置失败
    }

    LOG_Info("%s success,set %s success. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_ENABLE");
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
    
    // 实现设置 IVS 的 eCall 参数逻辑
    if (0 == g_HalEcallVoiceSet.inited)
    {
        LOG_Error("%s faild,Ecall Voice not init",__FUNCTION__);
        returnValue = RET_ERR; // 未启动
    }

    // 使用 QL_Voice_Call_Ecall_SetConfigInfo 进行设置

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_ENABLE, ecallConfig->ecallConfigEnable) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_ENABLE");
        returnValue = RET_ERR; // 设置失败
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_VIOCECONF, ecallConfig->ecallConfigVoiceConf) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_VIOCECONF");
        returnValue = RET_ERR; // 设置失败
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_MODE, ecallConfig->ecallConfigMode) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_MODE");
        returnValue = RET_ERR; 
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_PROCESSINFO, ecallConfig->ecallConfigProcessinfo) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_PROCESSINFO");
        returnValue = RET_ERR; 
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_START_TIMER, ecallConfig->ecallConfigStartTimer) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_START_TIMER");
        returnValue = RET_ERR; 
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_HACK_TIMER, ecallConfig->ecallConfigHackTimer) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_HACK_TIMER");
        returnValue = RET_ERR; 
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_MSD_TRANSMISSION, ecallConfig->ecallConfigMsdTransmission) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_MSD_TRANSMISSION");
        returnValue = RET_ERR; 
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_MO_FAILR_REDIAL, ecallConfig->ecallConfigMoFailRedial) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_MO_FAILR_REDIAL");
        returnValue = RET_ERR; 
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_DROP_REDIAL, ecallConfig->ecallConfigDropRedial) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_DROP_REDIAL");
        returnValue = RET_ERR; 
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_CLEARDOWN_TIMER, ecallConfig->ecallConfigCleardownTimer) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_CLEARDOWN_TIMER");
        returnValue = RET_ERR;
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_INITIATION_TIMER, ecallConfig->ecallConfigInitiationTimer) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_INITIATION_TIMER");
        returnValue = RET_ERR;
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_NAD_REGISTRATION_TIMER, ecallConfig->ecallConfigNadRegistrationTimer) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_NAD_REGISTRATION_TIMER");
        returnValue = RET_ERR;
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_NAD_UNREGISTRATION_TIMER, ecallConfig->ecallConfigNadUnregistrationTimer) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_NAD_UNREGISTRATION_TIMER");
        returnValue = RET_ERR;
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_ECALL_SYSTEM, ecallConfig->ecallConfigSystem) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_ECALL_SYSTEM");
        returnValue = RET_ERR;
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_ERA_GLONASS_REDIAL_TIMER, ecallConfig->ecallConfigEraGlonassRedialTimer) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_ERA_GLONASS_REDIAL_TIMER");
        returnValue = RET_ERR;
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_ERA_GLONASS_AUTO_ANSWER_TIMER, ecallConfig->ecallConfigEraGlonassAutoAnswerTimer) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_ERA_GLONASS_AUTO_ANSWER_TIMER");
        returnValue = RET_ERR;
    }

    if (QL_Voice_Call_Ecall_SetConfigInfo(g_HalEcallVoiceSet.h_voice, E_QL_MCM_VOICE_ECALL_CONFIG_REDIAL_DURATION_TIMER, ecallConfig->ecallConfigRedialDurationTimer) != 0)
    {
        LOG_Error("%s faild,set %s failed. ",__FUNCTION__,"E_QL_MCM_VOICE_ECALL_CONFIG_REDIAL_DURATION_TIMER");
        returnValue = RET_ERR;
    }
    
    LOG_Info("%s success. ",__FUNCTION__);
    return returnValue;
}
/**
* @brief 启动语音功能
*
* @param EcallVoiceInfo: 语音信息结构体指针
*
* @return 成功返回0，失败返回非0
*/
int32_t HalEcallVoiceStart(StrEcallVoiceInfo *const EcallVoiceInfo, EcallMsd_t *ecallMsdData)
{
    // 实现发起 eCall 通话逻辑
    if (0 == g_HalEcallVoiceSet.inited)
    {
        LOG_Error("%s faild,Ecall Voice not init",__FUNCTION__);
        return RET_ERR; // 未启动
    }
    ql_mcm_ecall_info ecallInfo = {0};
    int callId = 0;
    int32_t returnValue = 0;

    ecallInfo.msd_len = EcallVoiceInfo->msd_len;
    if(TRIGGER_MANUAL == EcallVoiceInfo->manual)
        ecallInfo.manual = TRIGGER_AUTO;
    else if(TRIGGER_AUTO == EcallVoiceInfo->manual)
    {
        ecallInfo.manual = TRIGGER_MANUAL;
    }
    else
    {
        ecallInfo.manual = TRIGGER_AUTO;
        LOG_Warn("%s : wrong manual value. set default AUTO",__FUNCTION__);
    }
    ecallInfo.eCallModeType = EcallVoiceInfo->eCallModeType;
    memcpy(ecallInfo.ecall_msd,EcallVoiceInfo->ecall_msd,MAX_ECALL_MSD);
    
    // 使用 EcallVoiceInfo 参数进行发起 eCall 通话
    returnValue = QL_Voice_Call_Ecall(g_HalEcallVoiceSet.h_voice, EcallVoiceInfo->simId, (char *)EcallVoiceInfo->PhoneNumber,
                            ecallInfo,&callId);
    if(0 != returnValue)                      
    {
        LOG_Error("%s faild, return value:%d . ",__FUNCTION__,returnValue);
        return RET_ERR;
    }
    LOG_Info("%s success, return value:%d . ",__FUNCTION__,returnValue);
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
        LOG_Error("%s faild,Ecall Voice not init",__FUNCTION__);
        return RET_ERR; // 未启动
    }
    
    uint32_t returnValue = 0;
    
    returnValue = QL_Voice_Call_Ecall_HangUp(g_HalEcallVoiceSet.h_voice);
    if(0 !=  returnValue)
    {
        LOG_Error("%s faild, return value:%d . ",__FUNCTION__,returnValue);
        return RET_ERR;
    }
    LOG_Info("%s success, return value:%d . ",__FUNCTION__,returnValue);
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
    // 实现更新 MSD 逻辑
    if (0 == g_HalEcallVoiceSet.inited)
    {
        LOG_Error("%s faild,Ecall Voice not init",__FUNCTION__);
        return RET_ERR; // 未启动
    }
    uint32_t returnValue = 0;
    returnValue = QL_Voice_Call_Ecall_UpdateMsd(g_HalEcallVoiceSet.h_voice, (const char *)msd, msdLen);
    if(0 !=  returnValue)
    {
        LOG_Error("%s faild, return value:%d . ",__FUNCTION__,returnValue);
        return RET_ERR;
    }
    LOG_Info("%s success, return value:%d . ",__FUNCTION__,returnValue);
    return RET_OK;
}
/**
* @brief 发送 MSD 以 PUSH 方式
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEcallVoicePushMsd(void)
{
    // 实现以 PUSH 方式发送更新的 MSD 逻辑
    if (0 == g_HalEcallVoiceSet.inited)
    {
        LOG_Error("%s faild,Ecall Voice not init",__FUNCTION__);
        return RET_ERR; // 未启动
    }
    
    uint32_t returnValue = 0;
    E_QL_MCM_ECALL_STATE_T ecall_state;
    returnValue = QL_Voice_Call_Ecall_MsdPush(g_HalEcallVoiceSet.h_voice, &ecall_state);
    if(0 !=  returnValue)
    {
        LOG_Error("%s faild, return value:%d . ",__FUNCTION__,returnValue);
        return RET_ERR;
    }

    LOG_Info("%s success, return value:%d . ",__FUNCTION__,returnValue);
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

    if (0 == g_HalEcallVoiceSet.inited)
    {
        LOG_Error("%s faild,Ecall Voice not init",__FUNCTION__);
        return RET_ERR; // 未启动
    }

    if(NULL == ecallConfigInfo)
    {
        LOG_Error("%s faild,ecallConfigInfo is NULL",__FUNCTION__);
        return RET_ERR; 
    }
 
    uint32_t returnValue = 0;
    returnValue = QL_Voice_Call_Ecall_GetConfigInfo(g_HalEcallVoiceSet.h_voice,(ql_mcm_ecall_config_info *)ecallConfigInfo);
    if(0 !=  returnValue)
    {
        LOG_Error("%s faild, return value:%d . ",__FUNCTION__,returnValue);
        return RET_ERR;
    }

    LOG_Info("%s success, return value:%d . ",__FUNCTION__,returnValue);
    return RET_OK; 
}

void HalEcallAudioPrepare(void)
{

}


