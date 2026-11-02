#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "ql_oe.h"
#include "ql_mcm_voice.h"
#include "sr_log.h"
#include "hal_modem_call.h"

#define HAL_MODEM_CALL_INITIALED (0)
#define HAL_MODEM_CALL_UNINITIALED (-1)

typedef struct
{
    voice_client_handle_type callHdl;
    CallStatusHandleCb callCb;
    int32_t initialed;
    int32_t callId;
}HalModemCallPriv_t;

static HalModemCallPriv_t gCallPriv = 
{
    .callHdl = 0x0,
    .callCb = NULL,
    .initialed = HAL_MODEM_CALL_UNINITIALED,
    .callId = -1
};

typedef struct
{
    E_QL_VOICE_CALL_STATE_T qlState;
    HalModemCallStatus_e srState;
}HalModemCallStatusConvert_t;

static HalModemCallStatusConvert_t gConvert[] = 
{
    {E_QL_MCM_VOICE_CALL_STATE_INCOMING, HAL_MODEM_CALL_STATUS_INCOMING},
    {E_QL_MCM_VOICE_CALL_STATE_DIALING, HAL_MODEM_CALL_STATUS_DIALING},
    {E_QL_MCM_VOICE_CALL_STATE_ALERTING, HAL_MODEM_CALL_STATUS_ALERTING},
    {E_QL_MCM_VOICE_CALL_STATE_ACTIVE, HAL_MODEM_CALL_STATUS_ACTIVE},
    {E_QL_MCM_VOICE_CALL_STATE_HOLDING, HAL_MODEM_CALL_STATUS_HOLDING},
    {E_QL_MCM_VOICE_CALL_STATE_END, HAL_MODEM_CALL_STATUS_DISCONNECTING},
    {E_QL_MCM_VOICE_CALL_STATE_WAITING, HAL_MODEM_CALL_STATUS_WAITING}
};

/**
 * @name CallStateHandle
 * @brief 处理呼叫状态的函数。
 * @param callId 呼叫ID。
 * @param phoneNum 电话号码。
 * @param state 呼叫状态。
 * @param contextPtr 上下文指针，用于传递额外的数据。
 */
static void CallStateHandle(int callId,
                            char* phoneNum,
                            E_QL_VOICE_CALL_STATE_T state,
                            void *contextPtr)
{
    HalModemCallInfoList_t list;
    int32_t i = 0;
    int32_t len = 0;
    int32_t *id = (int32_t *)contextPtr;

    len = sizeof(gConvert) / sizeof(HalModemCallStatusConvert_t);

    if (HAL_MODEM_CALL_INITIALED != gCallPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return;
    }

    memset(&list, 0x0, sizeof(list));
    list.infoNum = 1;
    memcpy(list.callInfo[0].phoneNum, phoneNum, strlen(phoneNum));
    list.callInfo[0].callId = callId;
    for (; i < len; i++)
    {
        if (state == gConvert[i].qlState)
        {
            list.callInfo[0].status = gConvert[i].srState;
            break;
        }
    }

    if (gCallPriv.callCb)
    {
        gCallPriv.callCb(&list);
    }
    *id = callId;
    return;
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
    int32_t retVal = -1;

    if (HAL_MODEM_CALL_INITIALED == gCallPriv.initialed)
    {
        //have initialed, return.
        LOG_Debug("%s have initialed.", __FUNCTION__);
    }

    gCallPriv.callHdl = 0x0;
    gCallPriv.initialed = HAL_MODEM_CALL_UNINITIALED;
    gCallPriv.callCb = NULL;
    gCallPriv.callId = -1;
    retVal = QL_Voice_Call_Client_Init(&gCallPriv.callHdl);
    if (MCM_SUCCESS_V01 != retVal)
    {
        LOG_Error("%s call QL_Voice_Call_Client_Init failed.retVal:%d", __FUNCTION__, retVal);
        return ret;
    }
    retVal = QL_Voice_Call_AddStateHandler(gCallPriv.callHdl, CallStateHandle, &gCallPriv.callId);
    if (0 != retVal)
    {
        LOG_Error("%s call QL_Voice_Call_AddStateHandler failed.retVal:%d", __FUNCTION__, retVal);
        HalModemCallDeInit();
        gCallPriv.callHdl = 0x0;
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

    retVal = QL_Voice_Call_RemoveStateHandler(gCallPriv.callHdl);
    LOG_Debug("%s call QL_Voice_Call_RemoveStateHandler.retVal:%d", __FUNCTION__, retVal);    
    retVal = QL_Voice_Call_Client_Deinit(gCallPriv.callHdl);
    LOG_Debug("%s call QL_Voice_Call_Client_Deinit.retVal:%d", __FUNCTION__, retVal);  
    gCallPriv.callHdl = 0x0;
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

    retVal = QL_Voice_Call_Start(gCallPriv.callHdl, 0, (char *)phoneNum, callId);
    if (0 != retVal)
    {
        LOG_Error("%s call QL_Voice_Call_Start failed.retVal:%d", __FUNCTION__, retVal);
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

    retVal = QL_Voice_Call_End(gCallPriv.callHdl, callId);
    if (0 != retVal)
    {
        LOG_Error("%s call QL_Voice_Call_End failed,retVal:%d.", __FUNCTION__, retVal);
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

    retVal = QL_Voice_Call_Answer(gCallPriv.callHdl, callId);
    if (0 != retVal)
    {
        LOG_Error("%s call QL_Voice_Call_Answer failed,retVal:%d.", __FUNCTION__, retVal);
        return ret;
    }
    ret = RET_OK;
    return ret;
}
