#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ql_type.h"
#include "ql_voice.h"
#include "ql_audio_pcm.h"
#include "ql_audio_cfg.h"

#include "hal_modem_bcall.h"
#include "sr_log.h"
#include "err_code.h"

typedef struct
{
    int32_t initialized;        // 模块初始化标志
    BcallVoiceCallbak bcallCbk; // 用户回调函数
    uint32_t currentCallId;     // 当前通话ID
} strHalBcallVoiceSet;

static strHalBcallVoiceSet g_HalBcallVoiceSet = {0};

/*-------------------------------------
 * SDK状态到BCALL事件映射
 *-------------------------------------*/
static EnuBcallEventId_e MapSdkStateToBcallEvent(QL_VOICE_STATE_E state)
{
    switch (state)
    {
        case QL_VOICE_STATE_INCOMING: return BCALL_STATUS_INCOMING;
        case QL_VOICE_STATE_DIALING:  return BCALL_STATUS_CALLING;
        case QL_VOICE_STATE_ACTIVE:   return BCALL_STATUS_ACTIVE;
        case QL_VOICE_STATE_END:      return BCALL_STATUS_END;
        default:                      return BCALL_STATUS_SAVE;
    }
}

/*-------------------------------------
 * SDK回调处理 (内置自动接听逻辑)
 *-------------------------------------*/
static void BcallSdkCallback(ql_voice_record_array_t *pRecords)
{
    if ((NULL == g_HalBcallVoiceSet.bcallCbk) || (NULL == pRecords) || (0 == pRecords->len))
    {
        LOG_Error("%s: call bak failed.paramater is wrong.", __FUNCTION__);
        return;
    }

    for (int i = 0; i < pRecords->len; i++)
    {
        ql_voice_record_t *pRec = &pRecords->records[i];

        /* 触发用户回调 */
        if(BCALL_STATUS_SAVE != MapSdkStateToBcallEvent(pRec->state))
        {
            g_HalBcallVoiceSet.bcallCbk(MapSdkStateToBcallEvent(pRec->state));
        }
        if(pRec->state == QL_VOICE_STATE_END)
        {
            LOG_Debug("%s : voice call end - reason : %d",__FUNCTION__,pRec->end_reason);
        }
    }
}

/*-------------------------------------
 * 接口实现
 *-------------------------------------*/
int32_t HalBcallVoiceInit(BcallVoiceCallbak cbFunc)
{
    if (1 == g_HalBcallVoiceSet.initialized)
    {
        LOG_Info("%s: Already initialized", __FUNCTION__);
        return RET_OK;
    }

    /* 初始化SDK语音服务 */
    if (QL_ERR_OK != ql_voice_init())
    {
        LOG_Error("%s: SDK voice init failed", __FUNCTION__);
        return RET_ERR;
    }

    /* 配置自动接听（内部实现，不对外暴露）*/
    if (QL_ERR_OK != ql_voice_autoanswer(1, 0))
    {
        LOG_Warn("%s: AutoAnswer config failed (proceed)", __FUNCTION__);
    }

    /* 注册回调函数 */
    if (QL_ERR_OK != ql_voice_set_call_cb(BcallSdkCallback))
    {
        LOG_Error("%s: Register callback failed", __FUNCTION__);
        ql_voice_deinit();
        return RET_ERR;
    }

    g_HalBcallVoiceSet.initialized = 1;
    g_HalBcallVoiceSet.bcallCbk = cbFunc;
    LOG_Info("%s: Initialized with AutoAnswer.", __FUNCTION__);
    return RET_OK;
}

int32_t HalBcallVoiceDeinit(void)
{
    if (0 == g_HalBcallVoiceSet.initialized)
    {
        return RET_OK;
    }

    /* 关闭自动接听 */
    ql_voice_autoanswer(0, 0);

    /* 释放SDK资源 */
    ql_voice_deinit();
    memset(&g_HalBcallVoiceSet, 0, sizeof(strHalBcallVoiceSet));
    LOG_Info("%s: Deinitialized", __FUNCTION__);
    return RET_OK;
}

int32_t HalBcallVoiceStart(StrBcallVoiceInfo *const pInfo)
{
    if ((NULL == pInfo) || (0 == g_HalBcallVoiceSet.initialized))
    {
        LOG_Error("%s: Invalid params or uninitialized", __FUNCTION__);
        return RET_ERR;
    }

    /* 拨号 */
    int32_t ret = ql_voice_dial((char*)pInfo->PhoneNumber, strlen((char*)pInfo->PhoneNumber), 
                               &g_HalBcallVoiceSet.currentCallId);
    if (QL_ERR_OK != ret)
    {
        LOG_Error("%s: Dial failed (ret=%d)", __FUNCTION__, ret);
        return RET_ERR;
    }

    /* 发送MSD数据（通过DTMF模拟）*/
    #if 0
    for (int i = 0; i < pInfo->msd_len; i++)
    {
        if (QL_ERR_OK != ql_voice_send_dtmf_char(g_HalBcallVoiceSet.currentCallId, pInfo->bcall_msd[i]))
        {
            LOG_Error("%s: MSD send failed at byte %d", __FUNCTION__, i);
            return RET_ERR;
        }
    }
    #endif
    LOG_Info("%s: Call started (ID=%u),call num : [%s] ", __FUNCTION__, g_HalBcallVoiceSet.currentCallId,(char*)pInfo->PhoneNumber);
    return RET_OK;
}

int32_t HalBcallVoiceHangup(void)
{
    if (0 == g_HalBcallVoiceSet.initialized)
    {
        LOG_Error("%s: uninitialized", __FUNCTION__);
        return RET_ERR;
    }

    int32_t ret = ql_voice_hangup_all();
    if (QL_ERR_OK != ret)
    {
        LOG_Error("%s: Hangup failed (ret=%d)", __FUNCTION__, ret);
        return RET_ERR;
    }

    g_HalBcallVoiceSet.currentCallId = 0;
    LOG_Info("%s: Call hung up", __FUNCTION__);
    return RET_OK;
}

int32_t HalBcallVoiceUpdateMsd(StrBcallVoiceInfo *const pInfo)
{
    if ((NULL == pInfo) || (0 == g_HalBcallVoiceSet.initialized))
    {
        LOG_Error("%s: Invalid params", __FUNCTION__);
        return RET_ERR;
    }

    #if 0
    for (int i = 0; i < msdLen; i++)
    {
        if (QL_ERR_OK != ql_voice_send_dtmf_char(g_HalBcallVoiceSet.currentCallId, msd[i]))
        {
            LOG_Error("%s: MSD update failed at byte %d", __FUNCTION__, i);
            return RET_ERR;
        }
    }
    #endif
    LOG_Info("%s: MSD updated", __FUNCTION__);
    return RET_OK;
}

int32_t HalBcallVoicePushMsd(void)
{
    if (0 == g_HalBcallVoiceSet.initialized)
    {
        return RET_ERR;
    }
    
    //GB接口实现
    
    LOG_Info("%s: MSD pushed", __FUNCTION__);
    return RET_OK;
}

int32_t HalBcallVoiceSetConfigInfo(StrBcallConfig *pConfig)
{
    if ((NULL == pConfig) || (0 == g_HalBcallVoiceSet.initialized))
    {
        LOG_Error("%s: Invalid params", __FUNCTION__);
        return RET_ERR;
    }

    //GB接口实现
    
    LOG_Info("%s: Config updated", __FUNCTION__);
    return RET_OK;
}

void HalBcallAudioPrepare(void)
{
    int32_t ReturnValue = 0;
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

