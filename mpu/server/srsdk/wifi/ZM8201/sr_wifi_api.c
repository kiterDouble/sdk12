#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sr_wifi_api.h"
#include "err_code.h"
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
  
}

int32_t WifiSetApModeInfoNoInspect(SrWifiInfo_t wifiInfo)
{
    
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

}


/**
 * @name WifiStaConnect
 * @brief WIFI STA 发起连接 (使新的配置信息生效)
 * @param 
 * @return 成功返回0, 失败返回小于0
 */
int32_t WifiStaConnect()
{
   
}


/**
 * @name WifiStaDisConnect
 * @brief 关闭STA模式下和热点连接
 * @param 
 * @return 成功返回0, 失败返回小于0
 */
int32_t WifiStaDisConnect()
{
   
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
   
}

// static void WifiScanResultCallback(ql_wifi_sta_scan_result_t *pMsg)
// {
    
// }


/**
 * @name WifiStaModeScan
 * @brief 执行Wi-Fi station模式下的扫描操作
 * 
 * @return int32_t 成功执行返回0或正数，错误执行返回负数。
 */
int32_t WifiStaModeScan(uint32_t secTimeOut)
{
   
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
    
}
