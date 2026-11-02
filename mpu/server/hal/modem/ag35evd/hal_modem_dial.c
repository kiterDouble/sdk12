#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "ql_oe.h"
#include "ql_wwan_v2.h"
#include "sr_log.h"
#include "hal_modem_dial.h"

#define HAL_MODEM_DIAL_INITIALED (0)
#define HAL_MODEM_DIAL_UNINITIALED (-1)
#define HAL_MODEM_DIAL_STARTED (0)
#define HAL_MODEM_DIAL_UNSTARTED (-1)

typedef struct
{
    int32_t initialed;
    int32_t isStarted;
    ModemCallbak dialCb;
    HalModemDialParam_t dialParam;
}HalModemDialPriv_t;


static HalModemDialPriv_t gDialPriv = 
{
    .initialed = HAL_MODEM_DIAL_UNINITIALED,
    .isStarted = HAL_MODEM_DIAL_UNSTARTED,
    .dialCb = NULL,
};

/**
 * @name DialCallback
 * @brief 拨号回调函数，用于处理拨号状态的更新并调用注册的回调函数。
 * @param state 指向ql_data_call_state_s结构体的指针，包含当前拨号状态和相关网络信息。
 * @return 函数无返回值
 */
void DialCallback(ql_data_call_state_s *state)
{
    HalModemNetInfo_t netInfo;


    if (!state)
    {
        LOG_Error("%s args invalid.", __FUNCTION__);
        return;       
    }

    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return;
    }

    memset(&netInfo, 0x0, sizeof(HalModemNetInfo_t));
    netInfo.dialStatus = state->state;
    strcpy((char *)netInfo.name, state->name);
    strcpy((char *)netInfo.ip, inet_ntoa(state->v4.ip));
    strcpy((char *)netInfo.gateway, inet_ntoa(state->v4.gateway));
    strcpy((char *)netInfo.priDns, inet_ntoa(state->v4.pri_dns));
    strcpy((char *)netInfo.secDns, inet_ntoa(state->v4.sec_dns));

    if (gDialPriv.dialCb)
    {
        gDialPriv.dialCb(state->profile_idx, &netInfo);
    }
}

/**
 * @name HalModemDialInit
 * @brief Modem拨号初始化。
 *        该函数负责在Modem上初始化拨号设置，如果已经初始化过，则直接返回成功。如果未初始化，则进行初始化流程，
 *        包括重置私有拨号状态、调用底层API进行初始化并设置状态回调。
 * @param null
 * @return 返回操作结果，RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDialInit(void)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;

    if (HAL_MODEM_DIAL_INITIALED == gDialPriv.initialed)
    {
        LOG_Debug("%s have initialed.", __FUNCTION__);
        ret = RET_OK;        
        return ret;
    }

    memset(&gDialPriv, 0x0, sizeof(HalModemDialPriv_t));
    gDialPriv.initialed = HAL_MODEM_DIAL_UNINITIALED;
    gDialPriv.isStarted = HAL_MODEM_DIAL_UNSTARTED;
    retVal = QL_Data_Call_Init(DialCallback);
    if (0 != retVal)
    {
        LOG_Error("%s QL_Data_Call_Init failed.", __FUNCTION__);
        return ret;
    }
    gDialPriv.initialed = HAL_MODEM_DIAL_INITIALED;
    ret = RET_OK;
    return ret;
}

/**
 * @name HalModemDialDeinit
 * @brief modem拨号反初始化
 * @param null
 * @return 返回操作结果，RET_OK表示成功，RET_ERR表示失败。 
 */
int32_t HalModemDialDeinit(void)
{
    int32_t ret = RET_ERR;

    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    memset(&gDialPriv, 0x0, sizeof(HalModemDialPriv_t));
    gDialPriv.initialed = HAL_MODEM_DIAL_UNINITIALED;
    gDialPriv.isStarted = HAL_MODEM_DIAL_UNSTARTED;
    QL_Data_Call_Destroy();
    ret = RET_OK;
    return ret;
}

/**
 * @name HalModemDialSetParam
 * @brief 设置modem拨号参数
 *        本函数用于配置调制解调器的拨号参数，包括APN等。在调用此函数之前，必须先初始化拨号模块。
 * @param dialParam 指向HalModemDialParam_t结构体的指针，包含了拨号所需的参数。
 *                  如果此参数为NULL或者其中的apnNum为0，则视为参数无效。
 * @return 函数执行状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDialSetParam(const HalModemDialParam_t *dialParam)
{
    int32_t ret = RET_ERR;

    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    if (!dialParam || (dialParam->apnNum == 0))
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;        
    }

    memcpy(&gDialPriv.dialParam, dialParam, sizeof(HalModemDialParam_t));
    ret = RET_OK;
    return ret;
}

/**
 * @name HalModemDialSetStatusCallback
 * @brief 设置Modem拨号状态回调函数
 *        本函数用于在Modem初始化完成后，设置一个回调函数，以便于接收Modem拨号的相关状态更新。
 * @param cb 指向回调函数的指针。该回调函数应当由用户实现，用于处理Modem拨号状态的改变。
 * @return 返回操作结果。如果操作成功，返回RET_OK；如果Modem未初始化，则返回RET_ERR。
 */
int32_t HalModemDialSetStatusCallback(ModemCallbak cb)
{
    int32_t ret = RET_ERR;
    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    gDialPriv.dialCb = cb;
    ret = RET_OK;
    return ret;
}

/**
 * @name HalModemDialStart
 * @brief 启动拨号功能，配置APN参数，并尝试建立数据呼叫。
 *        该函数首先检查拨号功能是否已初始化，是否已设置拨号参数，以及是否已经开始拨号。
 *        如果以上检查失败，则返回错误。如果检查通过，函数将为每个提供的APN配置参数，
 *        尝试创建数据呼叫并启动它。
 * @param null
 * @return 使用自定义错误代码表示成功或失败。
 */
int32_t HalModemDialStart(void)
{
    int32_t ret = RET_ERR;
    int32_t retVal = -1;
    int32_t i = 0;
    int32_t len = gDialPriv.dialParam.apnNum;
    ql_data_call_s dataCall;
    ql_data_call_error_e err = QL_DATA_CALL_ERROR_NONE;
    ql_apn_info_s apnInfo;

    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }

    if (len <= 0)
    {
        LOG_Error("%s have not set dialParam.", __FUNCTION__);
        return ret;
    }

    if (HAL_MODEM_DIAL_STARTED == gDialPriv.isStarted)
    {
        LOG_Error("%s have started.", __FUNCTION__);
        ret = RET_OK;
        return ret;
    }

    for (; i < len; i++)
    {
        memset(&dataCall, 0x0, sizeof(ql_data_call_s));
        dataCall.profile_idx = gDialPriv.dialParam.apnInfo[i].callId;//call id start with 1.
        dataCall.reconnect = 1;
        dataCall.ip_family = QL_APN_PDP_TYPE_IPV4;
  
        apnInfo.profile_idx = gDialPriv.dialParam.apnInfo[i].callId;
        apnInfo.pdp_type = 0;
        apnInfo.auth_proto = 0;
        memcpy(apnInfo.apn_name, gDialPriv.dialParam.apnInfo[i].apnName, strlen((char *)gDialPriv.dialParam.apnInfo[i].apnName));
        apnInfo.apn_name[strlen((char *)gDialPriv.dialParam.apnInfo[i].apnName)] = '\0';
        memcpy(apnInfo.username, gDialPriv.dialParam.apnInfo[i].userName, strlen((char *)gDialPriv.dialParam.apnInfo[i].userName));
        apnInfo.username[strlen((char *)gDialPriv.dialParam.apnInfo[i].userName)] = '\0';
        memcpy(apnInfo.password, gDialPriv.dialParam.apnInfo[i].userPasswd, strlen((char *)gDialPriv.dialParam.apnInfo[i].userPasswd));
        apnInfo.password[strlen((char *)gDialPriv.dialParam.apnInfo[i].userPasswd)] = '\0';
        retVal = QL_APN_Set(&apnInfo);
        if(retVal < 0)
        {
        LOG_Error("%s QL_APN_Set fail, idx:%d.", __FUNCTION__, i);
        return ret;
        }
    
        
        retVal = QL_Data_Call_Start(&dataCall, &err);

        if ((0 != retVal) || (err != QL_DATA_CALL_ERROR_NONE))
        {
            int32_t j = 0;
            for (; j < i; j++)
            {
                QL_Data_Call_Stop(gDialPriv.dialParam.apnInfo[i].callId, 
                                QL_APN_PDP_TYPE_IPV4, 
                                &err);
            }
            LOG_Error("%s call start fail, idx:%d.", __FUNCTION__, i);
            return ret;
        }
        
    }
    gDialPriv.isStarted = HAL_MODEM_DIAL_STARTED;
    ret = RET_OK;

    return ret;
}

/**
 * @name HalModemDialStop
 * @brief 停止拨号操作
 *        该函数用于停止当前正在进行的拨号操作。它会终止所有通过HalModemDialStart启动的电话连接。
 * @param null
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，RET_ERR表示失败。
 */
int32_t HalModemDialStop(void)
{
    int32_t ret = RET_ERR;
    int32_t i = 0;
    int32_t len = gDialPriv.dialParam.apnNum;
    ql_data_call_error_e err = QL_DATA_CALL_ERROR_NONE;

    if (HAL_MODEM_DIAL_INITIALED != gDialPriv.initialed)
    {
        LOG_Error("%s have not initialed.", __FUNCTION__);
        return ret;
    }
    
    if (HAL_MODEM_DIAL_STARTED != gDialPriv.isStarted)
    {
        LOG_Error("%s have not started.", __FUNCTION__);
        return ret;
    }

    if (len <= 0)
    {
        LOG_Error("%s have not set dialParam.", __FUNCTION__);
        return ret;
    }

    for (; i < len; i++)
    {
        QL_Data_Call_Stop(gDialPriv.dialParam.apnInfo[i].callId, 
                        QL_APN_PDP_TYPE_IPV4, 
                        &err);
    }

    gDialPriv.isStarted = HAL_MODEM_DIAL_UNSTARTED;
    ret = RET_OK;
    return ret;
}