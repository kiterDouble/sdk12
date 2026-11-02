#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ql_type.h"
#include "err_code.h"
#include "ql_voice.h"
#include "sr_log.h"
#include "hal_modem_call.h"

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

static HalModemCallStatus_e gConvert[] = 
{
    HAL_MODEM_CALL_STATUS_INCOMING,
    HAL_MODEM_CALL_STATUS_DIALING,
    HAL_MODEM_CALL_STATUS_ALERTING,
    HAL_MODEM_CALL_STATUS_ACTIVE,
    HAL_MODEM_CALL_STATUS_HOLDING,
    HAL_MODEM_CALL_STATUS_DISCONNECTING,
    HAL_MODEM_CALL_STATUS_WAITING
};

static void CallStateHandle(ql_voice_record_array_t *recordArr)
{
    HalModemCallInfoList_t list;
    int32_t i = 0;

    if (HAL_MODEM_CALL_INITIALED != gCallPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return;
    }

    if (!recordArr)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;        
    }

    (void)memset(&list, 0x0, sizeof(list));

    for (; i < recordArr->len; i++)
    {
        if (recordArr->records[i].state > QL_VOICE_STATE_WAITING)
        {
            LOG_Error("%s state invalid, state:%d.", __FUNCTION__, recordArr->records[i].state);
            continue;
        }

        list.callInfo[list.infoNum].status = gConvert[recordArr->records[i].state];
        list.callInfo[list.infoNum].callId = (int32_t)recordArr->records[i].id;
        uint8_t *tmpPhoneNumLen = (uint8_t *)recordArr->records[i].number;
        (void)memcpy(list.callInfo[list.infoNum].phoneNum, tmpPhoneNumLen, strlen(recordArr->records[i].number));
        list.infoNum++;
    }
    if (gCallPriv.callCb != NULL)
    {
        gCallPriv.callCb(&list);
    }
    return;
}

int32_t HalModemCallInit(CallStatusHandleCb Cb)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_CALL_INITIALED == gCallPriv.initialed)
    {
        //have initialed, return.
        LOG_Debug("%s have initialed.", __FUNCTION__);
    }

    gCallPriv.initialed = HAL_MODEM_CALL_UNINITIALED;
    gCallPriv.callCb = NULL;
    retVal = ql_voice_init();
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s call ql_voice_init failed.retVal:%d", __FUNCTION__, retVal);
        return ret;
    }
    retVal = ql_voice_set_call_cb(CallStateHandle);
    if (0 != retVal)
    {
        LOG_Error("%s call QL_Voice_Call_AddStateHandler failed.retVal:%d", __FUNCTION__, retVal);
        (void)HalModemCallDeInit();
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

    retVal = ql_voice_deinit();
    if (0 != retVal)
    {
        LOG_Error("%s call ql_voice_deinit failed.retVal:%d", __FUNCTION__, retVal);
        return ret;
    }
    LOG_Debug("%s call ql_voice_deinit.retVal:%d", __FUNCTION__, retVal);  
    gCallPriv.initialed = HAL_MODEM_CALL_UNINITIALED;
    gCallPriv.callCb = NULL;
    ret = RET_OK;
    return ret;
}

int32_t HalModemCallStart(const uint8_t *phoneNum, int32_t *callId)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;
    int32_t id = 0;

    if (HAL_MODEM_CALL_INITIALED != gCallPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    retVal = (int32_t)ql_voice_dial((const char *)phoneNum, strlen((const char *)phoneNum), &id);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s call ql_voice_dial failed.retVal:%d", __FUNCTION__, retVal);
        return ret;        
    }

    *callId = id;
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

    retVal = ql_voice_hangup((uint32_t)callId);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s call ql_voice_hangup failed,retVal:%d.", __FUNCTION__, retVal);
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

    retVal = ql_voice_answer((uint32_t)callId);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s call ql_voice_answer failed,retVal:%d.", __FUNCTION__, retVal);
        return ret;
    }
    ret = RET_OK;
    return ret;
}
