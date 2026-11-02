#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ql_util_common.h"
#include "ql-sdk/ql_type.h"
#include "ql-sdk/ql_ms_voice.h"
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_call.h"

#define HAL_MODEM_CALL_INITIALED (0)
#define HAL_MODEM_CALL_UNINITIALED (-1)

typedef struct
{
    CallStatusHandleCb callCb;
    int32_t initialed;
    int32_t callId;
}HalModemCallPriv_t;

static HalModemCallPriv_t gCallPriv = 
{
    .callCb = NULL,
    .initialed = HAL_MODEM_CALL_UNINITIALED,
    .callId = -1
};

typedef struct
{
    QL_VOICE_STATE_E qlState;
    HalModemCallStatus_e srState;
}HalModemCallStatusConvert_t;

static HalModemCallStatusConvert_t gConvert[] = 
{
    {QL_VOICE_STATE_INCOMING, HAL_MODEM_CALL_STATUS_INCOMING},
    {QL_VOICE_STATE_DIALING, HAL_MODEM_CALL_STATUS_DIALING},
    {QL_VOICE_STATE_ALERTING, HAL_MODEM_CALL_STATUS_ALERTING},
    {QL_VOICE_STATE_ACTIVE, HAL_MODEM_CALL_STATUS_ACTIVE},
    {QL_VOICE_STATE_HOLDING, HAL_MODEM_CALL_STATUS_HOLDING},
    {QL_VOICE_STATE_END, HAL_MODEM_CALL_STATUS_DISCONNECTING},
    {QL_VOICE_STATE_WAITING, HAL_MODEM_CALL_STATUS_WAITING},
    {QL_VOICE_STATE_SETUP, HAL_MODEM_CALL_STATUS_OFFERING}
};


#define SIM_ID (1)

static char *voice_state_str[] = {
        "coming",
        "dialing",
        "alterting",
        "active",
        "holding",
        "end",
        "waiting",
        "setup",
        "unknown",
};

static void voice_call_cb(int32_t sim_id, ql_voice_record_array_t *p_arr)
{
    HalModemCallInfoList_t list;
    int32_t i = 0;

    if (NULL == p_arr)
    {
        return;
    }

    if (HAL_MODEM_CALL_INITIALED != gCallPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return;
    }

    memset(&list, 0x0, sizeof(list));
    for (i = 0; i < p_arr->len; i++)
    {

        memcpy(list.callInfo[list.infoNum].phoneNum,
            p_arr->records[i].number,
            strlen( p_arr->records[i].number));

        list.callInfo[list.infoNum].callId = p_arr->records[i].id;

        if (p_arr->records[i].state <= QL_VOICE_STATE_WAITING 
            && p_arr->records[i].state >= QL_VOICE_STATE_INCOMING)
        {
            LOG_Debug(" state: %s\n", voice_state_str[p_arr->records[i].state]);
            list.callInfo[list.infoNum].status = gConvert[i].srState;
        }
        else
        {
            continue;
        }

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
    int32_t retVal;

    if (HAL_MODEM_CALL_INITIALED == gCallPriv.initialed)
    {
        //have initialed, return.
        LOG_Debug("%s have initialed.", __FUNCTION__);
    }

    gCallPriv.initialed = HAL_MODEM_CALL_UNINITIALED;
    gCallPriv.callCb = NULL;
    gCallPriv.callId = -1;
    retVal = ql_ms_voice_init();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s init voice failed: %d\n", __FUNCTION__, ret);
        return ret;
    }
    retVal  = ql_ms_voice_set_call_cb(SIM_ID, voice_call_cb);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s set voice call cb failed: %d\n", __FUNCTION__, ret);
        ql_ms_voice_deinit();
        return ret;
    }

    gCallPriv.callCb = Cb;
    gCallPriv.initialed = HAL_MODEM_CALL_INITIALED;
    ret = RET_OK;
    return ret;
}

int32_t HalModemCallDeInit(void)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_CALL_INITIALED != gCallPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    retVal = ql_ms_voice_deinit();
    LOG_Debug("%s call ql_ms_voice_deinit.retVal:%d", __FUNCTION__, retVal);  
    gCallPriv.initialed = HAL_MODEM_CALL_UNINITIALED;
    gCallPriv.callCb = NULL;
    gCallPriv.callId = -1;
    ret = RET_OK;
    return ret;
}

int32_t HalModemCallStart(const uint8_t *phoneNum, int32_t *callId)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_CALL_INITIALED != gCallPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    ret = ql_ms_voice_dial(SIM_ID, (char*)phoneNum, strlen((const char*)phoneNum), (uint32_t *)callId);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_voice_dial failed.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

int32_t HalModemCallEnd(int32_t callId)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_CALL_INITIALED != gCallPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    retVal = ql_ms_voice_hangup(SIM_ID, callId);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_voice_answer failed.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

int32_t HalModemCallAnswer(int32_t callId)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_CALL_INITIALED != gCallPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    retVal = ql_ms_voice_answer(SIM_ID, callId);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_ms_voice_answer failed.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}