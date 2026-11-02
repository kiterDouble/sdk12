#include <stdio.h>
#include <string.h>
#include "sr_wifi_api.h"
#include "err_code.h"
#include "ql_wifi.h"
#include "sr_log.h"


/**
 * @name WifiSetRunMode
 * @brief 设置WIFI运行模式
 * @param 
 *      mode:输入参数，
 *          SR_WIFI_STA_MODE：  WIFI STA 模式
 *          SR_WIFI_AP_MODE：   WIFI AP 模式
 *          SR_WIFI_AP_STA_MODE WIFI AP+STA 模式
 *          SR_WIFI_AP_AP_MODE  WIFI AP+AP 模式
 * @return 成功返回0, 失败返回-1
 */
int32_t WifiSetRunMode(int32_t mode)
{
    if(SR_WIFI_STA_MODE > mode || SR_WIFI_AP_AP_MODE < mode)
    {
        LOG_Error("%s arg is invalid", __FUNCTION__);
        return RET_ERR;
    }

    return ql_wifi_work_mode_set(mode - 1);
}


/**
 * @name WifiGetRunMode
 * @brief 获取WIFI当前运行模式
 * @param 
 *      mode:输出参数，
 *          SR_WIFI_STA_MODE：  WIFI STA 模式
 *          SR_WIFI_AP_MODE：   WIFI AP 模式
 *          SR_WIFI_AP_STA_MODE WIFI AP+STA 模式
 *          SR_WIFI_AP_AP_MODE  WIFI AP+AP 模式
 * @return 成功返回0, 失败返回小于0
 */
int32_t WifiGetRunMode(int32_t *mode)
{
    int32_t retValue = RET_ERR;
    ql_wifi_work_mode_e runMode;

    if(NULL == mode)
    {
        LOG_Error("%s arg is invalid", __FUNCTION__);
        return RET_ERR;
    }


    retValue = ql_wifi_work_mode_get(&runMode);
    if(retValue < 0)
    {
        LOG_Error("%s : get weok mode err: %d", __FUNCTION__, retValue);
        return RET_ERR;
    }

    *mode = runMode + 1;

    return RET_OK;
}

int32_t WifiSetApModeInfoNoInspect(SrWifiInfo_t wifiInfo)
{
    int32_t retValue = 0;
    ql_wifi_ap_auth_s apAuth;
    ql_wifi_mode_type_e modeType;

    retValue = ql_wifi_ap_ssid_set(wifiInfo.index - 1, wifiInfo.ssid);
    if(retValue < 0)
    {
        LOG_Error("%s, set ap ssid err:%d, ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    if(SR_WIFI_CHANNEL_MODE_2_4G == wifiInfo.mode)
    {
        modeType = QL_WIFI_MODE_80211BGN;
    }
    else
    {
        modeType = QL_WIFI_MODE_80211AC;
    }

    retValue = ql_wifi_ap_mode_set(wifiInfo.index - 1, modeType);
    if(retValue < 0)
    {
        LOG_Error("%s, set ap mode err:%d, ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    retValue = ql_wifi_ap_bandwidth_set(wifiInfo.index - 1, QL_WIFI_BANDWIDTH_HT20);
    if(retValue < 0)
    {
        LOG_Error("%s, set ap bandwidth err:%d, ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    if(SR_WIFI_CHANNEL_MODE_2_4G == wifiInfo.mode)
    {
        retValue = ql_wifi_ap_channel_set(wifiInfo.index - 1, 11);
    }
    else
    {
        retValue = ql_wifi_ap_channel_set(wifiInfo.index - 1, 36);
    }

    if(retValue < 0)
    {
        LOG_Error("%s, set ap channel err:%d, ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    apAuth.auth = wifiInfo.type;
    apAuth.wpa_psk.pairwise = QL_WIFI_AUTH_WPA_PAIRWISE_AUTO;
    apAuth.wpa_psk.group_rekey = 3600; /* one hour */
    strcpy(apAuth.wpa_psk.passwd, wifiInfo.passwd);

    retValue = ql_wifi_ap_ssid_hide_set(wifiInfo.index - 1, wifiInfo.hide);
    if(retValue < 0)
    {
        LOG_Error("%s, set ap ssid err:%d, ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    retValue = ql_wifi_ap_auth_set(wifiInfo.index - 1, &apAuth);
    if(retValue < 0)
    {
        LOG_Error("%s, set ap auth err:%d, ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    return RET_OK;
}

/**
 * @name WifiSetApModeInfo
 * @brief 设置WIFI通道模式
 * @param 
 *      wifiInfo:
 *      index ： 1：通道1， 2:   通道2
 *      mode:    1: 2.4G， 2：5G
 *      ssid :       最大长度为32，不可为空   
 *      passwd:      最大长度为64，不可为空
 *      type:       1: WPA  2:WAP2  3:WPA3
 *      hide:       0:不隐藏SSID 1:隐藏SSID
 * @return 成功返回0, 失败返回小于0
 */
int32_t WifiSetApModeInfo(SrWifiInfo_t wifiInfo)
{
    ql_wifi_work_mode_e runMode;
    int32_t strLen = 0;
    int32_t retValue = 0;
    ql_wifi_ap_auth_s apAuth;
    ql_wifi_mode_type_e modeType;

    retValue = ql_wifi_work_mode_get(&runMode);
    if(retValue < 0 && QL_WIFI_WORK_MODE_STA != runMode)
    {
        LOG_Error("%s, get weok mode[%d]: %d, ", __FUNCTION__, retValue, runMode);
        return RET_ERR;
    }

    if(SR_WIFI_CHANNEL_ONE != wifiInfo.index && SR_WIFI_CHANNEL_TWO != wifiInfo.index)
    {
        LOG_Error("%s, index range error ", __FUNCTION__);
        return RET_ERR;
    }

    if(SR_WIFI_CHANNEL_MODE_2_4G != wifiInfo.mode && SR_WIFI_CHANNEL_MODE_5G != wifiInfo.mode)
    {
        LOG_Error("%s, mode error ", __FUNCTION__);
        return RET_ERR;
    }

    strLen = strlen(wifiInfo.ssid);
    if(0 >= strLen || SR_WIFI_SSID_MAX_LEN < strLen)
    {
        LOG_Error("%s, ssid len error:%d ", __FUNCTION__, strLen);
        return RET_ERR;
    }

    strLen = strlen(wifiInfo.passwd);
    if(0 >= strLen || SR_WIFI_PASSWD_MAX_LEN < strLen)
    {
        LOG_Error("%s, passwd len error:%d ", __FUNCTION__, strLen);
        return RET_ERR;
    }
    
    if(SR_WIFI_PASSWD_TYPE_WPA != wifiInfo.type && \
       SR_WIFI_PASSWD_TYPE_WPA2 != wifiInfo.type && \
       SR_WIFI_PASSWD_TYPE_WPA_WPA2 != wifiInfo.type && \
       SR_WIFI_PASSWD_TYPE_WPA3 != wifiInfo.type)
    {
        LOG_Error("%s, type error ", __FUNCTION__);
        return RET_ERR;
    }

    if(SR_WIFI_AP_HIDE_ON != wifiInfo.hide && SR_WIFI_AP_HIDE_OFF != wifiInfo.hide)
    {
        LOG_Error("%s, type error ", __FUNCTION__);
        return RET_ERR;
    }

    retValue = ql_wifi_ap_ssid_set(wifiInfo.index - 1, wifiInfo.ssid);
    if(retValue < 0)
    {
        LOG_Error("%s, set ap ssid err:%d, ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    if(SR_WIFI_CHANNEL_MODE_2_4G == wifiInfo.mode)
    {
        modeType = QL_WIFI_MODE_80211BGN;
    }
    else
    {
        modeType = QL_WIFI_MODE_80211AC;
    }

    retValue = WifiSetApModeInfoNoInspect(wifiInfo);

    return retValue;
}

/**
 * @name WifiGetMaxConNum
 * @brief 获取WIFI的最大连接个数
 * @param 
 *      index :  通道索引  1：通道1， 2:   通道2
 *      num   ： 最大连接个数 支持最大连接个数，不超过16
 * @return 成功返回0, 失败返回小于0
 */
int32_t WifiGetMaxConNum(int32_t index, int32_t *num)
{
    int32_t retValue = 0;

    if(NULL == num)
    {
        LOG_Error("%s,  arg is invalid ", __FUNCTION__);
        return RET_ERR;
    }

    retValue = ql_wifi_ap_max_sta_get(index, num);
    if(retValue < 0)
    {
        LOG_Error("%s, get ap max num[%d]: %d, ", __FUNCTION__, retValue, *num);
        return RET_ERR;
    }

    return RET_OK;
}


/**
 * @name WifiApModeStop
 * @brief 关闭WIFI AP模式
 * @param 
 *      index :  通道索引  1：通道1， 2:   通道2
 * @return 成功返回0, 失败返回小于0
 */
int32_t WifiApModeStop(int32_t index)
{
    int32_t retValue = 0;

    if(SR_WIFI_CHANNEL_ONE != index && SR_WIFI_CHANNEL_TWO != index)
    {
        LOG_Error("%s, index range error ", __FUNCTION__);
        return RET_ERR;
    }

    retValue = ql_wifi_ap_stop(index);
    if(retValue < 0)
    {
        LOG_Error("%s,  ap  mode stop error ", __FUNCTION__);
        return RET_ERR;
    }

    return RET_OK;
}


/**
 * @name WifiApModeStart
 * @brief 获取主天线状态
 * @param 
 *      index :  通道索引  1：通道1， 2:   通道2
 * @return 成功返回0, 失败返回小于0
 */
int32_t WifiApModeStart(int32_t index)
{
    int32_t retValue = 0;

    if(SR_WIFI_CHANNEL_ONE != index && SR_WIFI_CHANNEL_TWO != index)
    {
        LOG_Error("%s, index range error ", __FUNCTION__);
        return RET_ERR;
    }

    retValue = ql_wifi_ap_start(index);
    if(retValue < 0)
    {
        LOG_Error("%s,  ap  mode stop error ", __FUNCTION__);
        return RET_ERR;
    }

    return RET_OK;
}


/**
 * @name WifiStaModeSetInfo
 * @brief 设置WIFI STA 连接信息
 * @param 
 *      index ： 1：通道1， 2:   通道2  -STA 无效
 *      mode:    1: 2.4G， 2：5G       -STA 无效
 *      ssid :       最大长度为32，不可为空   passwd:  最大长度为64，不可为空
 *      type:       1: WPA  2:WAP2  3:WPA3
 *      hide:       0:不隐藏SSID 1:隐藏SSID -STA 无效
 * @return 成功返回0, 失败返回小于0
 */
int32_t WifiStaModeSetInfo(SrWifiInfo_t wifiInfo)
{
    ql_wifi_work_mode_e runMode;
    int32_t strLen = 0;
    int32_t retValue = 0;
    ql_wifi_sta_auth_s staAuth;

    retValue = ql_wifi_work_mode_get(&runMode);
    if(retValue < 0 )
    {
        LOG_Error("%s, get work mode error: %d ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    if(QL_WIFI_WORK_MODE_STA != runMode && QL_WIFI_WORK_MODE_AP0_STA != runMode)
    {
        LOG_Error("%s, get work mode error, [%d]: %d, ", __FUNCTION__, retValue, runMode);
        return RET_ERR;
    }

    strLen = strlen(wifiInfo.ssid);
    if(0 >= strLen || SR_WIFI_SSID_MAX_LEN < strLen)
    {
        LOG_Error("%s, ssid len error:%d ", __FUNCTION__, strLen);
        return RET_ERR;
    }

    strLen = strlen(wifiInfo.passwd);
    if(0 >= strLen || SR_WIFI_PASSWD_MAX_LEN < strLen)
    {
        LOG_Error("%s, passwd len error:%d ", __FUNCTION__, strLen);
        return RET_ERR;
    }
    
    if(SR_WIFI_PASSWD_TYPE_WPA != wifiInfo.type && \
       SR_WIFI_PASSWD_TYPE_WPA2 != wifiInfo.type && \
       SR_WIFI_PASSWD_TYPE_WPA_WPA2 != wifiInfo.type && \
       SR_WIFI_PASSWD_TYPE_WPA3 != wifiInfo.type)
    {
        LOG_Error("%s, type error ", __FUNCTION__);
        return RET_ERR;
    }

    staAuth.auth = wifiInfo.type + 1;
    staAuth.wpa_psk.pairwise = QL_WIFI_AUTH_WPA_PAIRWISE_AUTO;
    strcpy(staAuth.wpa_psk.passwd, wifiInfo.passwd);

    retValue = ql_wifi_sta_ssid_set(wifiInfo.ssid);
    if(retValue < 0)
    {
        LOG_Error("%s, set sta ssid err:%d, ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    retValue = ql_wifi_sta_auth_set(&staAuth);
    if(retValue < 0)
    {
        LOG_Error("%s, set ap auth err:%d, ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    return RET_OK;
}


/**
 * @name WifiStaConnect
 * @brief WIFI STA 发起连接 (使新的配置信息生效)
 * @param 
 * @return 成功返回0, 失败返回小于0
 */
int32_t WifiStaConnect()
{
    ql_wifi_work_mode_e runMode;
    ql_wifi_station_status_e staStatus;
    int32_t retValue = 0;

    retValue = ql_wifi_work_mode_get(&runMode);
    if(retValue < 0 )
    {
        LOG_Error("%s, get work mode error: %d ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    if(QL_WIFI_WORK_MODE_STA != runMode && QL_WIFI_WORK_MODE_AP0_STA == runMode)
    {
        LOG_Error("%s, get work mode error, [%d]: %d, ", __FUNCTION__, retValue, runMode);
        return RET_ERR;
    }

    retValue = ql_wifi_sta_status(&staStatus);
    if(retValue < 0)
    {
        LOG_Error("%s,  get sta status is error : %d", __FUNCTION__, retValue);
        return RET_ERR;
    }

    if(QL_WIFI_STATION_DISABLE == staStatus)
    {
        LOG_Error("%s,  sta status is error : %d, %d ", __FUNCTION__, retValue, staStatus);
        return RET_ERR;
    }

    if(QL_WIFI_STATION_CONNECTED == staStatus)
    {
        LOG_Error("%s,  sta status is connect and disconnect  ", __FUNCTION__);
        ql_wifi_sta_disconnect();
    }

    retValue = ql_wifi_sta_connect();
    if(retValue < 0)
    {
        LOG_Error("%s,  sta connect error : %d ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    return RET_OK;
}


/**
 * @name WifiStaDisConnect
 * @brief 关闭STA模式下和热点连接
 * @param 
 * @return 成功返回0, 失败返回小于0
 */
int32_t WifiStaDisConnect()
{
    ql_wifi_work_mode_e runMode;
    ql_wifi_station_status_e staStatus;
    int32_t retValue = 0;

    retValue = ql_wifi_work_mode_get(&runMode);
    if(retValue < 0 )
    {
        LOG_Error("%s, get work mode error: %d ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    if(QL_WIFI_WORK_MODE_STA != runMode && QL_WIFI_WORK_MODE_AP0_STA == runMode)
    {
        LOG_Error("%s, get work mode error, [%d]: %d, ", __FUNCTION__, retValue, runMode);
        return RET_ERR;
    }

    retValue = ql_wifi_sta_status(&staStatus);
    if(retValue < 0)
    {
        LOG_Error("%s,  get sta status is error : %d", __FUNCTION__, retValue);
        return RET_ERR;
    }

    if(QL_WIFI_STATION_DISABLE == staStatus)
    {
        LOG_Error("%s,  sta status is error : %d, %d ", __FUNCTION__, retValue, staStatus);
        return RET_ERR;
    }

    retValue = ql_wifi_sta_disconnect();
    if(retValue < 0)
    {
        LOG_Error("%s,  sta disconnect error : %d ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    return RET_OK;
}


/**
 * @name WifiStaGetSta
 * @brief 获取STA模式的连接状态
 * @param 
 *      status:输出参数
 *          1：  STA 模式没有打开
 *          2：  STA 模式已连接状态
 *          3：  STA 模式断开
 * @return 成功返回0, 失败返回小于0
 */
int32_t WifiStaGetSta(int32_t *status)
{
    int32_t retValue = RET_ERR;
    ql_wifi_station_status_e staStatus;

    if(NULL == status)
    {
        LOG_Error("%s,  arg is invalid ", __FUNCTION__);
        return RET_ERR;
    }

    retValue = ql_wifi_sta_status(&staStatus);
    if(retValue < 0)
    {
        LOG_Error("%s,  get sta status is error : %d", __FUNCTION__, retValue);
        return RET_ERR;
    }

    switch (staStatus)
    {
    case QL_WIFI_STATION_DISABLE:
        *status = SR_WIFI_STA_STATUS_NO_OPEN;
        break;
     case QL_WIFI_STATION_CONNECTED:
        *status = SR_WIFI_STA_STATUS_CONNECTED;
        break;
     case QL_WIFI_STATION_DISCONNECTED:
        *status = SR_WIFI_STA_STATUS_DISCONNECTED;
        break;
    default:
        *status = SR_WIFI_STA_STATUS_NO_OPEN;
        LOG_Error("%s,  get sta status is error : %d", __FUNCTION__, retValue);
        break;
    }

    return RET_OK;
}



/**
 * @name WifiGetStaInterface
 * @brief 获取WIFI STA模式网卡名称
 * @param 
 *      interface : 网卡名称最长16个字符
 * @return 成功返回0, 失败返回小于0
 */
int32_t WifiGetStaInterface(char *interface, int32_t len)
{
    if(NULL == interface || len < 16)
    {
        return RET_ERR;
    }
    memcpy(interface, "wlan1", strlen("wlan1"));

    return RET_OK;
}


/**
 * @name WifiEnable
 * @brief 打开WIFI
 * @param 
 * 
 * @return 成功返回0, 失败返回小于0
 */
int32_t WifiEnable()
{
    int32_t retValue = RET_ERR;
    //看是否需要加载驱动 

    retValue = ql_wifi_enable();
    if(retValue < 0)
    {
        LOG_Error("%s, wifi enables err:%d, ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    return RET_OK;
}


/**
 * @name WifiDisable
 * @brief 关闭WIFI
 * @param 
 * 
 * @return 成功返回0, 失败返回小于0
 */
int32_t WifiDisable()
{
    int32_t retValue = RET_ERR;
    //看是否需要卸载驱动 

    retValue = ql_wifi_disable();
    if(retValue < 0)
    {
        LOG_Error("%s, wifi disables err:%d, ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    return RET_OK;
}
