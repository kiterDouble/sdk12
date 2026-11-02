#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ql_type.h"
#include "ql_voice.h"
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

/**
 * @name CallStateHandle
 * @brief 处理呼叫状态的函数。
 * @param callId 呼叫ID。
 * @param phoneNum 电话号码。
 * @param state 呼叫状态。
 * @param contextPtr 上下文指针，用于传递额外的数据。
 */

static void voice_call_cb(ql_voice_record_array_t *p_arr)
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
    memset(&list, 0, sizeof(HalModemCallInfoList_t));
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
/**
 * @name HalModemCallInit
 * @brief modem通话功能初始化。
 * @param Cb 通话状态回调函数的指针。
 * @return 成功返回RET_OK，失败返回RET_ERR。
 */
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
    retVal = ql_voice_init();
    if(QL_ERR_OK != retVal)
    {
        LOG_Error("%s init fail. retVal:%d", __FUNCTION__, retVal);
        return ret;
    }

    retVal = ql_voice_set_call_cb(voice_call_cb);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s set voice call cb failed: %d\n", __FUNCTION__, retVal);
        ql_voice_deinit();
        return ret;
    }

    gCallPriv.callCb = Cb;
    gCallPriv.initialed = HAL_MODEM_CALL_INITIALED;
    ret = RET_OK;
    return ret;    
}

/**
 * @name HalModemCallDeInit
 * @brief modem通话模块反初始化。
 * @param null
 * @return 成功返回RET_OK，失败返回RET_ERR。
 */
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
    LOG_Debug("%s call ql_voice_deinit, retVal:%d", __FUNCTION__, retVal);  
    gCallPriv.initialed = HAL_MODEM_CALL_UNINITIALED;
    gCallPriv.callCb = NULL;
    gCallPriv.callId = -1;
    ret = RET_OK;
    return ret;
}

/**
 * @name HalModemCallStart
 * @brief 拨打电话
 * @param phoneNum 指向要拨打的电话号码的指针。
 * @param callId 指向接收呼叫ID的指针，呼叫启动成功时会填充呼叫ID。
 * @return 成功返回RET_OK，失败返回RET_ERR。
 */
int32_t HalModemCallStart(const uint8_t *phoneNum, int32_t *callId)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_CALL_INITIALED != gCallPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    retVal = ql_voice_dial((char*)phoneNum, strlen((char*)phoneNum), (uint32_t *)callId);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_voice_dial failed.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}

/**
 * @name HalModemCallEnd
 * @brief 挂断电话。
 * @param callId 通话的ID，用于标识要挂断的通话。
 * @return 成功返回RET_OK，失败返回RET_ERR。
 */
int32_t HalModemCallEnd(int32_t callId)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_CALL_INITIALED != gCallPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    retVal = ret = ql_voice_hangup((uint32_t)callId);
    if (QL_ERR_OK != retVal)
    {
        LOG_Error("%s ql_voice_hangup failed.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;    
}

/**
 * @name HalModemCallAnswer
 * @brief 接听电话。
 * @param callId 电话呼叫的ID。
 * @return 返回操作结果，RET_OK表示成功，RET_ERR表示失败。
 */
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
        LOG_Error("%s ql_voice_answer failed.", __FUNCTION__);
        return ret;
    }

    ret = RET_OK;
    return ret;
}
