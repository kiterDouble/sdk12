#include <glib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "fty_control.h"
#include "fty_info.h"
#include "mcu_api.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "err_code.h"
#include "pm_server_api.h"
#include "modem_ipc_msg.h"
#include "modem_api.h"
#include "sr_wifi_api.h"
#include "modem_api.h"

#define FTY_CONTROL_LENGTH (3)
static gchar g_wifiSsid[16] = {0};
static gchar g_wifiPsk[16] = {0};
static guchar g_iparray[8] = {0};
static guchar g_phoneNum[17] = {0};

static gboolean g_IsTested = FALSE;
static MPUPmStateChange_t g_FtyPmState = {0};
static ModemSrvNotify_t g_FtyModemSrvNotify = {0};

typedef enum FTYControlType
{
    enumControlTestMode = 1,   // 测试模式控制
    enumControlApnPing,        // APN Ping 控制
    enumControlWifi,           // WiFi 控制
    enumControlBT,             // 蓝牙控制
    enumControlCall,           // 通话控制
    enumControlend,            // 控制类型结束标记
} FTYControlType_e;            // FTY 控制类型枚举

gpointer FtyThreadCallback(gpointer pArg)
{
    pthread_detach(pthread_self());
    GFunc ThreadCb = (GFunc) pArg;
    LOG_Error("FtyThreadCallback!");
    ThreadCb(NULL, NULL);
    return NULL;
}

gboolean CreateThread(gpointer data)
{
    pthread_t threadId;
    gboolean ret = TRUE;
    if (pthread_create(&threadId, NULL, FtyThreadCallback, data) < 0)
    {
        LOG_Error("pthread_create fail!");
        ret = FALSE;
    }

    return ret;
}
void FtyGetPmPowerState(const uint8_t* data, const uint16_t len, void* userdata)
{    
    if(sizeof(MPUPmStateChange_t) == len && NULL != data)
    {
        memcpy(&g_FtyPmState, data, len);
    }
    else
    {
        LOG_Debug("%s len error!",__FUNCTION__);
    }
    
}
void FtyGetModemSrvInfo(const uint8_t *data, const uint16_t len, void* userdata)
{
    if(sizeof(ModemSrvNotify_t) == len && NULL != data)
    {
        memcpy(&g_FtyModemSrvNotify,data,len);
    }
    else
    {
        LOG_Error("%s recv modem data leng is wrong: %d .",__FUNCTION__,len);
    }
    
}

static void GetNetInfo(void)
{
    uint8_t operator[64] = {0}; 
    int32_t ret = RET_ERR;
    ModemSrvExtSigInfo_t modemSrvExtSigInfo = {0};
    ret = ModemSrvGetOperator(operator);
    LOG_Debug("%s ModemSrvGetOperator : %s", operator,__FUNCTION__); 
    if (RET_OK == ret)
    {
        uint32_t operatorValue = atoi((const char *)operator);
        if (46000 == operatorValue || 46007 == operatorValue)
        {
            SetProviderName("CMCC");
        }
        else if (46001 == operatorValue || 46006 == operatorValue || 46002 == operatorValue)
        {
            SetProviderName("CUCC");
        }
        else if (46003 == operatorValue || 46005 == operatorValue || 46011 == operatorValue)
        {
            SetProviderName("CTCC");
        }
    }
    ret = ModemSrvGetExtSignalInfo(&modemSrvExtSigInfo);
    LOG_Debug("%s ModemSrvGetExtSignalInfo : type - %d ",__FUNCTION__,modemSrvExtSigInfo.lteSiginfo.sigType); 
    if (RET_OK == ret)
    {
        uint32_t sigType = modemSrvExtSigInfo.lteSiginfo.sigType;
        if (1 == sigType) // LTE
        {
             SetNetType(0);
        }
        else if (0 == sigType) // GSM
        {
             SetNetType(2);
        }
        else //other
        {
             SetNetType(1);
        }
    }  
}

gpointer TestTimeout(gpointer pArg)
{

    LOG_Debug("TestTimeout");
    while ( 1 )
    {
        if (PM_STATE_SLEEP == g_FtyPmState.PmState)
        {
            LOG_Debug("%s pm sleep",__FUNCTION__);
            sleep(2);
            continue;
        }
        SetCsq(g_FtyModemSrvNotify.csq);
        SetNetRegStatus(g_FtyModemSrvNotify.dataRegStatus);
        if(g_IsTested == TRUE)
        {
            GetNetInfo();
        }
        sleep(2);
    }
}


/**
* @brief 发送ECALL状态同步
*
* @param ecallStatus EnuEcallStatus中的状态类型
* @return int32_t 返回值，如果发送成功则返回RET_OK，否则返回RET_ERR
*/

static void ApnTestCb(gpointer data, gpointer userData)
{
    system("echo \"#!/bin/sh\" > /oemdata/apntest.sh");
	system("echo \"IP1=\\$1\" >> /oemdata/apntest.sh");
	system("echo \"IP2=\\$2\" >> /oemdata/apntest.sh");
	system("echo \"TR1=1\" >> /oemdata/apntest.sh");
	system("echo \"TR2=1\" >> /oemdata/apntest.sh");
	//system("echo \"i=0\" >> /oemdata/apntest.sh");
	//system("echo \"while [ \\\"\\$i\\\" -le \\\"40\\\" ]\" >> /oemdata/apntest.sh");
	//system("echo \"do\" >> /oemdata/apntest.sh");
	system("echo \"if [ \\\"\\$TR1\\\" -ne \\\"0\\\" ]; then\" >> /oemdata/apntest.sh");
	system("echo \"ping \\$IP1 -c 1 -w 1\" >> /oemdata/apntest.sh");
	system("echo \"TR1=\\$?\" >> /oemdata/apntest.sh");
	system("echo \"fi\" >> /oemdata/apntest.sh");
	system("echo \"if [ \\\"\\$TR2\\\" -ne \\\"0\\\" ]; then\" >> /oemdata/apntest.sh");
	system("echo \"ping \\$IP2 -c 1 -w 1\" >> /oemdata/apntest.sh");
	system("echo \"TR2=\\$?\" >> /oemdata/apntest.sh");
	system("echo \"fi\" >> /oemdata/apntest.sh");
	system("echo \"if [[ \\\"\\$TR2\\\" -eq \\\"0\\\" && \\\"\\$TR1\\\" -eq \\\"0\\\" ]]; then\" >> /oemdata/apntest.sh");    
	system("echo \"echo \\\"0\\\" > /oemdata/apnResult\" >> /oemdata/apntest.sh");
	system("echo \"exit\" >> /oemdata/apntest.sh");
	system("echo \"fi\" >> /oemdata/apntest.sh");
	//system("echo \"let i++\" >> /oemdata/apntest.sh");
	//system("echo \"done\" >> /oemdata/apntest.sh");
	system("sync");
    gchar apnIp1[16] = {0x0,};
    gchar apnIp2[16] = {0x0,};
    gchar cmd[128] = {0x0,}; 
    
    snprintf(apnIp1, 16, "%d.%d.%d.%d",g_iparray[0],g_iparray[1],g_iparray[2],g_iparray[3]);
    snprintf(apnIp2, 16, "%d.%d.%d.%d",g_iparray[4],g_iparray[5],g_iparray[6],g_iparray[7]);
    snprintf(cmd, sizeof(cmd), "sh /oemdata/apntest.sh %s %s", apnIp1, apnIp2);

    guint endtime = time(NULL) + 40;
    while (time(NULL) < endtime)
    {
        system(cmd);
        LOG_Debug("ApnTestCb, cmd:%s", cmd);
        //system("sh /oemdata/apntest.sh 127.0.0.2 192.168.100.1");
        if((access("/oemdata/apnResult",F_OK)) != -1)   
        {   
            LOG_Debug("APN TEST success"); 
            system("rm -f /oemdata/apnResult");
            SetApnTestResult(2 | (1 << 8) | (1<< 16));	
            break;	
        } 
        else
        {
            SetApnTestResult(1);
            LOG_Debug("APN TEST fail"); 
        }
        usleep(100000);
    }
    if((access("/oemdata/apnResult",F_OK)) == -1)
    {
        SetApnTestResult(2);
    }
	system("rm -f /oemdata/apntest.sh");
    system("sync");
}

static void WifiTestCb(gpointer data, gpointer user_data)
{
    SrWifiInfo_t srWifiInfo = {0};
    sleep(10);
    WifiSetRunMode(SR_WIFI_STA_MODE);
    WifiDisable();
    sleep(2);
    WifiEnable();
    sleep(5);
    srWifiInfo.type = SR_WIFI_PASSWD_TYPE_WPA; //api 实现可能有问题，总是执行失败
    memcpy(srWifiInfo.ssid,g_wifiSsid,16);
    memcpy(srWifiInfo.passwd,g_wifiPsk,16);
    WifiStaModeSetInfo(srWifiInfo);    
    WifiStaConnect();
    //
    LOG_Debug("WifiTestCb, ssid:%s, psk:%s", g_wifiSsid, g_wifiPsk);
    system("echo \"#!/bin/sh\" > /oemdata/wifitest.sh");
	system("echo \"IP1=\\$1\" >> /oemdata/wifitest.sh");
	system("echo \"TR1=1\" >> /oemdata/wifitest.sh");
	//system("echo \"i=0\" >> /oemdata/wifitest.sh");
	//system("echo \"while [ \\\"\\$i\\\" -le \\\"40\\\" ]\" >> /oemdata/wifitest.sh");
	//system("echo \"do\" >> /oemdata/wifitest.sh");
	system("echo \"if [ \\\"\\$TR1\\\" -ne \\\"0\\\" ]; then\" >> /oemdata/wifitest.sh");
	system("echo \"ping \\$IP1 -c 1 -w 1\" >> /oemdata/wifitest.sh");
	system("echo \"TR1=\\$?\" >> /oemdata/wifitest.sh");
	system("echo \"fi\" >> /oemdata/wifitest.sh");
	system("echo \"if [ \\\"\\$TR1\\\" -eq \\\"0\\\" ]; then\" >> /oemdata/wifitest.sh");
	system("echo \"echo \\\"0\\\" > /oemdata/wifiResult\" >> /oemdata/wifitest.sh");
	system("echo \"exit\" >> /oemdata/wifitest.sh");
	system("echo \"fi\" >> /oemdata/wifitest.sh");
    system("sync");
	//system("echo \"let i++\" >> /oemdata/wifitest.sh");
	//system("echo \"done\" >> /oemdata/wifitest.sh");

    guint endtime = time(NULL) + 40;
    while (time(NULL) < endtime)
    {
        system("sh /oemdata/wifitest.sh 192.168.88.1");

        if((access("/oemdata/wifiResult",F_OK)) != -1)   
        {   
            LOG_Debug("WIFI TEST success"); 
            system("rm -f /oemdata/wifiResult");
            SetWifiTestResult(2 | (1 << 8));
            break;
        } 
        else
        {
            SetWifiTestResult(1);
            LOG_Debug("WIFI TEST fail"); 
        }
        usleep(100000);
    }
    if((access("/oemdata/wifiResult",F_OK)) == -1)
    {
        SetWifiTestResult(2);
    }
	system("rm -f /oemdata/wifitest.sh");
    system("sync");
    WifiSetRunMode(SR_WIFI_AP_MODE);
    sleep(5);
    WifiDisable();
    sleep(5);
    
    WifiEnable();
}
static void CallTestCb(gpointer data, gpointer user_data)
{
    int32_t callId = 0;
    int32_t ret = 0;

    SetCallTestResult(1);
    ret = ModemSrvStartCall(g_phoneNum, &callId);
    if(RET_OK == ret)
    {
        SetCallTestResult(2 | (1 << 8));
        ModemSrvStopCall(callId);
        LOG_Debug("CALL TEST success"); 
    }
    else
    {
        SetCallTestResult(2);
        LOG_Debug("CALL TEST fail"); 
    }
    
    return;
}

void HandleTestMode(const uint8_t *data, const uint16_t len)
{
    if (data[0] == 0) {
        LOG_Debug("< test end>"); 
        g_IsTested = FALSE;
    } else if (data[0] == 1) {
        LOG_Debug("< test start"); 
        g_IsTested = TRUE;
    }
    SendFtyInfoAck(OperationResultSuccess, E_TEST_MODE,0 , NULL);
}

void HandleApnPing(const uint8_t *data, const uint16_t len)
{
    LOG_Debug(">>> APN test");
    if(len > 8)
    {
        LOG_Error("%s : data len from mcu more than 8,error.",__FUNCTION__);        
        SendFtyInfoAck(OperationResultFailure, E_APN_PING_START,0 , NULL);
        return;
    }
    memcpy(g_iparray, data, len);
    CreateThread((gpointer)ApnTestCb);
    SendFtyInfoAck(OperationResultSuccess, E_APN_PING_START,0 , NULL);
}

void HandleWifi(const uint8_t *data, const uint16_t len)
{
    LOG_Debug(">>> wifi test");
    
    if(len > 32)
    {
        LOG_Error("%s : data len from mcu more than 32,error.",__FUNCTION__);        
        SendFtyInfoAck(OperationResultFailure, E_WIFI_CONNECT_SPECIFIC_NETWORK,0 , NULL);
        return;
    }
    memcpy(g_wifiSsid, data, 16);
    memcpy(g_wifiPsk, &data[16], 16);
    CreateThread((gpointer)WifiTestCb);
    SendFtyInfoAck(OperationResultSuccess, E_WIFI_CONNECT_SPECIFIC_NETWORK,0 , NULL);
}

void HandleBTConnect(const uint8_t *data, const uint16_t len)
{
    LOG_Debug(">>> bt test");
    // BtTest(data); // Uncomment when BtTest function is implemented
    SendFtyInfoAck(OperationResultSuccess, E_BLUETOOTH_CONNECT_COMMAND,0 , NULL);
}

void HandleCallDail(const uint8_t *data, const uint16_t len)
{   
    LOG_Debug(">>> call test");
    if(len > sizeof(g_phoneNum) || len < 0)
    {
        LOG_Error("%s : data len from mcu more than %d,error.",__FUNCTION__,sizeof(g_phoneNum));        
        SendFtyInfoAck(OperationResultFailure, E_DIAL_TEST_COMMAND,0 , NULL);
        return;
    }
    memset(g_phoneNum,0,sizeof(g_phoneNum));
    memcpy(g_phoneNum, data, len);
    LOG_Debug("> call num:%s.",g_phoneNum);
    CreateThread((gpointer)CallTestCb);
    SendFtyInfoAck(OperationResultSuccess, E_DIAL_TEST_COMMAND,0 , NULL);
}

