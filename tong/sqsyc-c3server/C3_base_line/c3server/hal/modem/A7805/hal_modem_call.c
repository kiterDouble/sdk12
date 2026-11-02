#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_call.h"
#include "hal_modem_comm.h"
#include "Sunsea_VCall.h"

#define HAL_MODEM_CALL_INITIALED (0)
#define HAL_MODEM_CALL_UNINITIALED (-1)

typedef struct
{
    CallStatusHandleCb callCb;
    int32_t initialed;
}HalModemCallPriv_t;

static HalModemCallPriv_t gCallPriv = 
{
    .callCb = NULL,
    .initialed = HAL_MODEM_CALL_UNINITIALED,
};

static HalModemCallStatus_e convertState(vcall_state_e state)
{
    switch (state)
    {
    case VCALL_STATE_ORIGINATING:
        return HAL_MODEM_CALL_STATUS_DIALING;
    case VCALL_STATE_INCOMING:
        return HAL_MODEM_CALL_STATUS_INCOMING;
    case VCALL_STATE_CONVESATION:
        return HAL_MODEM_CALL_STATUS_ACTIVE;
    case VCALL_STATE_CC_IN_PROGRESS:
        return HAL_MODEM_CALL_STATUS_ACTIVE;
    case VCALL_STATE_ALERTING:
        return HAL_MODEM_CALL_STATUS_ALERTING;
    case VCALL_STATE_HOLD:
        return HAL_MODEM_CALL_STATUS_HOLDING;
    case VCALL_STATE_WAITING:
        return HAL_MODEM_CALL_STATUS_WAITING;
    case VCALL_STATE_DISCONNECTING:
        return HAL_MODEM_CALL_STATUS_DISCONNECTING;
    case VCALL_STATE_END:
        return HAL_MODEM_CALL_STATUS_DISCONNECTED;
    case VCALL_STATE_SETUP:
        return HAL_MODEM_CALL_STATUS_ACTIVE;
    default:
        return HAL_MODEM_CALL_STATUS_DISCONNECTED;
    }
}

static void CallStateHandleCb(vcall_info_s *infos)
{
    HalModemCallInfoList_t list = {0};
    int32_t i = 0;

    if (HAL_MODEM_CALL_INITIALED != gCallPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return;
    }

    if (!infos)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;      
    }

    memset(&list, 0, sizeof(HalModemCallInfoList_t));

    for (i = 0; i < infos->call_num; i++)
    {
        strcpy((char*)list.callInfo[i].phoneNum, infos->call_info[i].phoneNum);
        list.callInfo[i].callId = infos->call_info[i].h_vcall;
        list.callInfo[i].status = convertState(infos->call_info[i].call_state);
        list.infoNum++;
    }

    if (gCallPriv.callCb)
    {
        gCallPriv.callCb(&list);
    }
}

int32_t HalModemCallInit(CallStatusHandleCb Cb)
{
    int32_t ret = RET_ERR;

    if (HAL_MODEM_CALL_INITIALED == gCallPriv.initialed)
    {
        LOG_Debug("%s have initialed.", __FUNCTION__);
    }

    if (!Cb)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    gCallPriv.initialed = HAL_MODEM_CALL_UNINITIALED;
    gCallPriv.callCb = NULL;  

    if (HalModemCommInit() != RET_OK)
    {
        LOG_Error("%s HalModemCommInit failed.", __FUNCTION__);
        return ret;
    }

    if (Sunsea_VoiceCall_Init(CallStateHandleCb) < 0)
    {
        return ret;
    }

    gCallPriv.initialed = HAL_MODEM_CALL_INITIALED;
    gCallPriv.callCb = Cb;
    ret = RET_OK;
    return ret;
}

int32_t HalModemCallDeInit(void)
{
    int32_t ret = RET_ERR;
    
    if (HAL_MODEM_CALL_INITIALED != gCallPriv.initialed)
    {
        LOG_Debug("%s have not initialed.", __FUNCTION__);
        return ret;
    }
    Sunsea_VoiceCall_Deinit();

    gCallPriv.initialed = HAL_MODEM_CALL_UNINITIALED;
    gCallPriv.callCb = NULL;  
    return RET_OK;
}

int32_t HalModemCallStart(const uint8_t *phoneNum, int32_t *callId)
{
    if (HAL_MODEM_CALL_INITIALED != gCallPriv.initialed)
    {
        LOG_Debug("%s have not initialed.", __FUNCTION__);
        return RET_ERR;
    }
    
    return  Sunsea_VoiceCall_Start((char *)phoneNum,  (vcall_handle *)callId);
}

int32_t HalModemCallEnd(int32_t callId)
{
    if (HAL_MODEM_CALL_INITIALED != gCallPriv.initialed)
    {
        LOG_Debug("%s have not initialed.", __FUNCTION__);
        return RET_ERR;
    }
    return Sunsea_VoiceCall_Answer((vcall_handle)callId);
}

int32_t HalModemCallAnswer(int32_t callId)
{
    if (HAL_MODEM_CALL_INITIALED != gCallPriv.initialed)
    {
        LOG_Debug("%s have not initialed.", __FUNCTION__);
        return RET_ERR;
    }
    return Sunsea_VoiceCall_End((vcall_handle)callId);
}
