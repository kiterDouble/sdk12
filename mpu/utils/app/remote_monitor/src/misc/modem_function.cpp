#include<iostream>
#include "modem_function.h"
#include <cstring> // 在C++11及以后的标准中
#include<stdlib.h>
#include "modem_api.h"
#include "err_code.h"
#include "sr_log.h"

static ModemSrvNotify_t g_ModemSrvNotify = {0};

/**
 * @brief modem广播信息回调，可获取sim卡状态
 * @return
*/
void GetModemSrvInfo(const uint8_t *data, const uint16_t len, void* userdata)
{
    TBoxSimMode mode = SIM_AIRPLANE_OFF;
    TBoxNetType netType = TYPE_GSM;
    int32_t ret = RET_OK;

    if(sizeof(ModemSrvNotify_t) != len)
    {
        LOG_Error("%s recv modem data leng is wrong.", __FUNCTION__);
        return;
    }
    else
    {
        memcpy(&g_ModemSrvNotify, data, len);
    }

    return;
}

int32_t GetSimMode(TBoxSimMode *mode)
{   
    if(!mode)
    {
        LOG_Error("%s param invalid", __FUNCTION__);
        return RET_ERR;
    }

    return ModemSrvGetAirplaneMode((uint8_t *)mode);
}

int32_t GetNetworkProvider(TBoxNetWorkProvider_t *netProvider)
{
    uint8_t info[64] = {'\0',};
    uint8_t mcc[3] = {0x0,};
    uint8_t mnc[2] = {0x0};
    int32_t ret = RET_OK;

    if(!netProvider)
    {
        LOG_Error("%s param invalid", __FUNCTION__);
        return RET_ERR;
    }

    ret = ModemSrvGetOperator(info);
    if(ret)
    {
        LOG_Error("%s Modem get operator failed", __FUNCTION__);
        return ret;
    }

    memcpy(mcc, info, 3);
    memcpy(mnc, info+3, 2);
    netProvider->mcc = atoi((char *)mcc);
    netProvider->mnc = atoi((char *)mnc);
    switch(netProvider->mnc)
    {
        case 0:
        case 2:
        case 4:
        case 7:
        case 8:
        case 20:
        case 99:
            netProvider->providerLen = strlen("CHINA MOBILE");
            memcpy(netProvider->networkProvider, "CHINA MOBILE", strlen("CHINA MOBILE"));
            break;
        case 1:
        case 6:
        case 9:
            netProvider->providerLen = strlen("CHN-CUGSM");
            memcpy(netProvider->networkProvider, "CHN-CUGSM", strlen("CHN-CUGSM"));
            break; 
        case 3:
        case 5:
        case 11:
        case 12:
        case 13:  
            netProvider->providerLen = strlen("CHINA TELECOM");
            memcpy(netProvider->networkProvider, "CHINA TELECOM", strlen("CHINA TELECOM"));
            break;    
        default:
            netProvider->providerLen = strlen("unknow provider");
            memcpy(netProvider->networkProvider, "unknow provider", strlen("unknow provider"));
            break; 
    }

    return ret;
}

int32_t GetSimInfo(TBoxSIMInfo_t *simInfo)
{
    int32_t ret = RET_OK;
    uint8_t buffer[64] = {0x0, };

    if(!simInfo)
    {
        printf("%s param invald", __FUNCTION__);
        return RET_ERR;
    }
    memset(buffer, 0, 15);
    ret = ModemSrvGetImsi(buffer);
    if(ret)
    {
         printf("buffer imsi = %s  error\n",buffer);
        LOG_Error("%s get imsi failed", __FUNCTION__);
        //return ret;
    }
    printf("buffer imsi = %s\n",buffer);
    memcpy(simInfo->imsi, buffer, 16);
    printf("buffer imsi = %s\n",simInfo->imsi);
    memset(buffer, 0, 16);
    ret = ModemSrvGetImei(buffer);
    if(ret)
    {
        LOG_Error("%s get imei failed", __FUNCTION__);
        return ret;
    }
    printf("buffer imei = %s\n",buffer);
    memcpy(simInfo->imei, buffer, 16);
    memset(buffer, 0, 20);
    ret = ModemSrvGetIccid(buffer);
    if(ret)
    {
        LOG_Error("%s get iccid failed", __FUNCTION__);
        return ret;
    }
    memcpy(simInfo->iccid, buffer, 21);
    printf("buffer iccid = %s",buffer);
    return RET_OK;
}

int32_t GetSimNetType(TBoxNetType *netType)
{
    int32_t ret = RET_OK;
    ModemSrvExtSigInfo_t sigInfo;

    if(!netType)
    {
        LOG_Error("%s invalid param", __FUNCTION__);
        return RET_ERR;
    }

    memset(&sigInfo, 0, sizeof(ModemSrvExtSigInfo_t));
    ret = ModemSrvGetExtSignalInfo(&sigInfo);
    if(ret)
    {
        LOG_Error("%s get signal info failed", __FUNCTION__);
        return ret;
    }

    if(sigInfo.lteSiginfo.sigType == 1)
    {
        *netType = TYPE_LTE;
    }
    else 
    {
        *netType = TYPE_GSM;
    }

    return ret;
}

int32_t GetSimStatus(TBoxSimState *status)
{
    if(!status)
    {
        LOG_Error("%s get sim status invalid param", __FUNCTION__);
        return RET_ERR;
    }

    *status = (TBoxSimState)g_ModemSrvNotify.simCardStatus;

    return RET_OK;
}

int32_t GetSignalStrehgth(uint8_t *strength)
{
    if(!strength)
    {
        LOG_Error("%s get sim signal strength invalid param", __FUNCTION__);
        return RET_ERR;
    }

    *strength = g_ModemSrvNotify.csq;

    return RET_OK;
}


