#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sr_wifi_api.h"
#include "err_code.h"
#include "ql_type.h"
#include "ql_wifi_common.h"
#include "ql_wifi.h"
#include "sr_log.h"

typedef struct
{
    GMutex lock;
    GCond cond;
    uint8_t initFlag;
}WifiCond_t;

static WifiCond_t g_wifiCond;
static SrWifiScanResult_t g_srWifiScanResultInfo;

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
    QL_WIFI_WORK_MODE_E runMode;

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
    ql_wifi_ap_auth_t apAuth;
    QL_WIFI_AP_MODE_TYPE_E modeType;

    retValue = ql_wifi_ap_ssid_set(wifiInfo.index - 1, wifiInfo.ssid);
    if(retValue < 0)
    {
        LOG_Error("%s, set ap ssid err:%d, ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    if(SR_WIFI_CHANNEL_MODE_2_4G == wifiInfo.mode)
    {
        modeType = QL_WIFI_AP_MODE_80211BGN;
    }
    else
    {
        LOG_Error("AG35CET NO 5G");
        return RET_ERR;
    }

    retValue = ql_wifi_ap_mode_set(wifiInfo.index - 1, modeType);
    if(retValue < 0)
    {
        LOG_Error("%s, set ap mode err:%d, ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    // retValue = ql_wifi_ap_bandwidth_set(wifiInfo.index - 1, QL_WIFI_BANDWIDTH_20MHZ);
    // if(retValue < 0)
    // {
    //     LOG_Error("%s, set ap bandwidth err:%d, ", __FUNCTION__, retValue);
    //     return RET_ERR;
    // }

    // if(SR_WIFI_CHANNEL_MODE_2_4G == wifiInfo.mode)
    // {
    //     retValue = ql_wifi_ap_channel_set(wifiInfo.index - 1, 11);
    // }
    // else
    // {
    //     retValue = ql_wifi_ap_channel_set(wifiInfo.index - 1, 36);
    // }

    // if(retValue < 0)
    // {
    //     LOG_Error("%s, set ap channel err:%d, ", __FUNCTION__, retValue);
    //     return RET_ERR;
    // }

    if(SR_WIFI_PASSWD_TYPE_WPA == wifiInfo.type || SR_WIFI_PASSWD_TYPE_WPA2 == wifiInfo.type)
    {
        apAuth.auth = wifiInfo.type + 1;
    }
    else if(SR_WIFI_PASSWD_TYPE_WPA_WPA2 == wifiInfo.type)
    {
        apAuth.auth = wifiInfo.type + 2;
    }
    else
    {
        apAuth.auth = wifiInfo.type;
    }

    apAuth.info.wpa_psk.pairwise = QL_WIFI_AUTH_WPA_PAIRWISE_AUTO;
    // apAuth.info.wpa_psk.group_rekey = 3600; /* one hour */
    strcpy(apAuth.info.wpa_psk.passwd, wifiInfo.passwd);

    // retValue = ql_wifi_ap_ssid_hidden_set(wifiInfo.index - 1, wifiInfo.hide);
    // if(retValue < 0)
    // {
    //     LOG_Error("%s, set ap ssid err:%d, ", __FUNCTION__, retValue);
    //     return RET_ERR;
    // }

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
    QL_WIFI_WORK_MODE_E runMode;
    int32_t strLen = 0;
    int32_t retValue = 0;

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

    retValue = ql_wifi_ap_max_sta_num_get(index - 1, num);
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

    retValue = ql_wifi_ap_stop(index - 1);
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

    retValue = ql_wifi_ap_start(index - 1);
    if(retValue < 0)
    {
        LOG_Error("%s,  ap  mode start error ", __FUNCTION__);
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
    QL_WIFI_WORK_MODE_E runMode;
    int32_t strLen = 0;
    int32_t retValue = 0;
    ql_wifi_sta_auth_t staAuth;

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

    if(SR_WIFI_PASSWD_TYPE_WPA == wifiInfo.type || SR_WIFI_PASSWD_TYPE_WPA2 == wifiInfo.type)
    {
        staAuth.auth = wifiInfo.type + 1;
    }
    else if(SR_WIFI_PASSWD_TYPE_WPA_WPA2 == wifiInfo.type)
    {
        staAuth.auth = wifiInfo.type + 2;
    }
    else
    {
        staAuth.auth = wifiInfo.type;
    }
    
    staAuth.info.wpa_psk.pairwise = QL_WIFI_AUTH_WPA_PAIRWISE_AUTO;
    strcpy(staAuth.info.wpa_psk.passwd, wifiInfo.passwd);

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
    QL_WIFI_WORK_MODE_E runMode;
    ql_wifi_sta_status_t staStatus;
    int32_t retValue = 0;

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

    retValue = ql_wifi_sta_get_status(&staStatus);
    if(retValue < 0)
    {
        LOG_Error("%s,  get sta status is error : %d", __FUNCTION__, retValue);
        return RET_ERR;
    }

    if( QL_WIFI_STA_STATUS_CONNECTED == staStatus.status || \
        QL_WIFI_STA_STATUS_ASSOCIATED == staStatus.status )
    {
        LOG_Error("%s,  sta status is connect and disconnect  ", __FUNCTION__);
        ql_wifi_sta_stop();
    }

    retValue = ql_wifi_sta_start();
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
    QL_WIFI_WORK_MODE_E runMode;
    ql_wifi_sta_status_t staStatus;
    int32_t retValue = 0;

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

    retValue = ql_wifi_sta_get_status(&staStatus);
    if(retValue < 0)
    {
        LOG_Error("%s,  get sta status is error : %d", __FUNCTION__, retValue);
        return RET_ERR;
    }

    if(QL_WIFI_STA_STATUS_DISCONNECTED == staStatus.status)
    {
        LOG_Error("%s,  sta status is error : %d, %d ", __FUNCTION__, retValue, staStatus.status);
        return RET_ERR;
    }

    retValue = ql_wifi_sta_stop();
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
    ql_wifi_sta_status_t staStatus;

    if(NULL == status)
    {
        LOG_Error("%s,  arg is invalid ", __FUNCTION__);
        return RET_ERR;
    }

    retValue = ql_wifi_sta_get_status(&staStatus);
    if(retValue < 0)
    {
        LOG_Error("%s,  get sta status is error : %d", __FUNCTION__, retValue);
        return RET_ERR;
    }

    switch (staStatus.status)
    {
    case QL_WIFI_STA_STATUS_NONE:
    case QL_WIFI_STA_STATUS_IDLE:
        *status = SR_WIFI_STA_STATUS_NO_OPEN;
        break;
    case QL_WIFI_STA_STATUS_CONNECTED:
    case QL_WIFI_STA_STATUS_ASSOCIATED:
        *status = SR_WIFI_STA_STATUS_CONNECTED;
        break;
    case QL_WIFI_STA_STATUS_CONNECTING:
    case QL_WIFI_STA_STATUS_DISCONNECTED:
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

static void WifiScanResultCallback(ql_wifi_sta_scan_result_t *pMsg)
{
    int32_t count = 0;

    if (0 != pMsg->scan_is_busy)
    {
        printf("Scan is busy, Please try again\n");
        return;
    }

    if(1 != g_wifiCond.initFlag)
    {
        g_mutex_init(&g_wifiCond.lock);
        g_cond_init(&g_wifiCond.cond);
        g_wifiCond.initFlag = 1;
    }


    LOG_Debug("cnt = %d\n", pMsg->scan_list.cnt);
    g_mutex_lock(&g_wifiCond.lock);
    g_srWifiScanResultInfo.infoSize = pMsg->scan_list.cnt;

    for (count = 0; count < pMsg->scan_list.cnt; count++)
    {
        memcpy(g_srWifiScanResultInfo.scanInfo[count].bssid, pMsg->scan_list.info[count].bssid, 13);
        memcpy(g_srWifiScanResultInfo.scanInfo[count].essid, pMsg->scan_list.info[count].essid, 33);
        g_srWifiScanResultInfo.scanInfo[count].frequency = pMsg->scan_list.info[count].frequency;

        if(QL_WIFI_AUTH_WPA_PSK == pMsg->scan_list.info[count].auth || \
           QL_WIFI_AUTH_WPA2_PSK == pMsg->scan_list.info[count].auth)
        {
            g_srWifiScanResultInfo.scanInfo[count].type = pMsg->scan_list.info[count].auth - 1;
        }
        else if(QL_WIFI_AUTH_WPA_WPA2_PSK_BOTH == pMsg->scan_list.info[count].auth)
        {
            g_srWifiScanResultInfo.scanInfo[count].type = pMsg->scan_list.info[count].auth - 2;
        }
        else
        {
            g_srWifiScanResultInfo.scanInfo[count].type = pMsg->scan_list.info[count].auth;
        }
        g_srWifiScanResultInfo.scanInfo[count].signalRssi = pMsg->scan_list.info[count].signal;
        LOG_Debug("bssid: %s, frequency: %d, signal: %d, authentication: %d, essid: %s\n",
               pMsg->scan_list.info[count].bssid,
               pMsg->scan_list.info[count].frequency,
               pMsg->scan_list.info[count].signal,
               pMsg->scan_list.info[count].auth,
               pMsg->scan_list.info[count].essid);
    }
    g_cond_signal(&g_wifiCond.cond);
    g_mutex_unlock(&g_wifiCond.lock);
}


/**
 * @name WifiStaModeScan
 * @brief 执行Wi-Fi station模式下的扫描操作
 * 
 * @return int32_t 成功执行返回0或正数，错误执行返回负数。
 */
int32_t WifiStaModeScan(uint32_t secTimeOut)
{
    QL_WIFI_WORK_MODE_E runMode;
    int32_t retValue = RET_ERR;
    gint64 endTIme = 0;

    if(1 != g_wifiCond.initFlag)
    {
        g_mutex_init(&g_wifiCond.lock);
        g_cond_init(&g_wifiCond.cond);
        g_wifiCond.initFlag = 1;
    }

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

    retValue = ql_wifi_sta_set_scan_result_ind_cb(WifiScanResultCallback);
    if(0 != retValue)
    {
        LOG_Error("%s set callback error: %d", __FUNCTION__, retValue);
        return RET_ERR;
    }

    retValue = ql_wifi_sta_start_scan();
    if(0 != retValue)
    {
        LOG_Error("%s start scan error: %d", __FUNCTION__, retValue);
        return RET_ERR;
    }

    g_mutex_lock(&g_wifiCond.lock);
    endTIme = g_get_monotonic_time() + G_TIME_SPAN_SECOND * secTimeOut;
    if (g_cond_wait_until(&g_wifiCond.cond, &g_wifiCond.lock, endTIme))
    {
        retValue = RET_OK;
    }
    g_mutex_unlock(&g_wifiCond.lock);

    return retValue;
}


/**
 * @name WifiScanResultGet
 * @brief 获取Wi-Fi扫描结果的函数
 * 
 * 
 * @return int32_t 成功执行返回0或正数，错误执行返回负数。
 */
int32_t WifiScanResultGet(SrWifiScanResult_t *scanResult)
{
    if(NULL == scanResult)
    {
        return RET_ERR;
    }

    if(1 != g_wifiCond.initFlag)
    {
        g_mutex_init(&g_wifiCond.lock);
        g_cond_init(&g_wifiCond.cond);
        g_wifiCond.initFlag = 1;
    }

    g_mutex_lock(&g_wifiCond.lock);
    memcpy(scanResult, &g_srWifiScanResultInfo, sizeof(SrWifiScanResult_t));
    g_mutex_unlock(&g_wifiCond.lock);
    
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
    retValue = ql_wifi_init();
    if(0 != retValue)
    {
        LOG_Error("%s, wifi init err:%d, ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    if(1 != g_wifiCond.initFlag)
    {
        g_mutex_init(&g_wifiCond.lock);
        g_cond_init(&g_wifiCond.cond);
        g_wifiCond.initFlag = 1;
    }

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
    
    retValue = ql_wifi_disable();
    if(retValue < 0)
    {
        LOG_Error("%s, wifi disables err:%d, ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    //看是否需要卸载驱动 
    retValue = ql_wifi_deinit();
    if(0 != retValue)
    {
        LOG_Error("%s, wifi deinit err:%d, ", __FUNCTION__, retValue);
        return RET_ERR;
    }

    return RET_OK;
}
