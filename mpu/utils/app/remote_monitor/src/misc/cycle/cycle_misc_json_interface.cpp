#include "cycle_misc_json_interface.h"
#include "err_code.h"
#include "cycle_evet_handle.h"
#include "sr_log.h"
#include "../../../inc/json/json.h"
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <math.h>
#include "modem_api.h"
#include "cycle_get_modem_info.h"
#include "gnss_ipc_msg.h"
#include "common_functions.h"
#include "cycle_common.h"
#include "cjson_interface.h"
#include "timer_interface.h"
#include "system_interface.h"
#include "sr_gnss.h"


using namespace std;

#define  RET_ERR  (-1)
#define  RET_OK  (0)
static char g_sendMqttFlag = 1;

// #define ERROR_REISSUE

static char g_sendErrorFlag = 0;
static time_t oldTime = 0;
uint8_t g_localnetMonitorStatus[1] = {0};  //存放内部网络节点监控结

char SetSendMqttFlag(char sendFlag)
{
    g_sendMqttFlag = sendFlag;
    LOG_Debug("SetSendMqttFlag");
    return g_sendMqttFlag;
}

char SetsendErrorFlag(char sendErrorFlag)
{
    g_sendErrorFlag = sendErrorFlag;
    if(1 == g_sendErrorFlag)
    {
        oldTime = time(NULL) + 1 * 60;
    }
    LOG_Debug("g_sendErrorFlag:%d",g_sendErrorFlag);
    return g_sendErrorFlag;
}

static int32_t BytesToInt32(const uint8_t *data)
{
    if (data)
        return (int32_t)(
            ((uint32_t)data[3] << 0) |
            ((uint32_t)data[2] << 8) |
            ((uint32_t)data[1] << 16) |
            ((uint32_t)data[0] << 24));
}

/**
  * 函数功能: 获取系统运行时间
  * 输入参数: 
  * 返 回 值: 
  * 说    明: 无
  */
static long getRunTime(void)
{
    struct sysinfo info;
    if (sysinfo(&info)) {
        LOG_Error("sysinfo error");
        return 0;
    }

    long timenum=info.uptime;
    LOG_Debug("system run time: %ld",timenum);
    return timenum;
}


/**
  * 函数功能: 通过ICCID分析运营商
  * 输入参数: ICCID号
  * 返 回 值: OperatorType类型参数
  * 说    明: 无
  */
OperatorType GetOperatorFromIccid(const char *iccid)
{
    //参数检查
    if (iccid == NULL || strlen(iccid) < 6) {
        return OP_UNKNOWN;
    }

    if (strncmp(iccid, "898600", 6) == 0 || strncmp(iccid, "898602", 6) == 0 ||
        strncmp(iccid, "898604", 6) == 0 || strncmp(iccid, "898608", 6) == 0)
    {
        return OP_CMCC;  //中国移动
    }

    if (strncmp(iccid, "898601", 6) == 0 || strncmp(iccid, "898606", 6) == 0) 
    {
        return OP_CUCC;  //中国联通
    }

    if (strncmp(iccid, "898603", 6) == 0 || strncmp(iccid, "898611", 6) == 0)
    {
        return OP_CTCC;  //中国电信
    }
    return OP_UNKNOWN;
}


/**
  * 函数功能: 获取内部网络节点监控结果回调函数
  * 输入参数: recvData：获取的数据
  *          dataLen： 获取的速度长度
  *          userdata：用户自定义数据的指针，本函数中未使用。
  * 返 回 值: 
  * 说    明: 监控结果通过MQTT上报
  */
void GetNetMonitorstatuscallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    LocalNetState_t tmpBuf;

    if((NULL == recvData) || (sizeof(LocalNetState_t) != dataLen))
    {
        LOG_Error("%s argv is invalid, dataLen = %d", __FUNCTION__, dataLen);
        return;
    }

    if(dataLen < sizeof(LocalNetState_t))
    {
        LOG_Error("%s argv is invalid, dataLen[%d] <= sizeof(LocalNetState_t) ", __FUNCTION__, dataLen);
        return;
    }

    memcpy(&tmpBuf, recvData, dataLen);
     //8位 存放各个内部网络节点监控结果存放
     g_localnetMonitorStatus[0] =   tmpBuf.gateway << 7 | \
                                    tmpBuf.xxka << 6 | \
                                    tmpBuf.xxkb << 5 | \
                                    tmpBuf.onlineLanguageModule << 4 |\
                                    tmpBuf.bluetoothToEthernetModule << 3 |\
                                    tmpBuf.module1 << 2 |\
                                    tmpBuf.module2 << 1;

    LOG_Debug("g_netMonitorStatus:%#x %#x",g_localnetMonitorStatus[0]);
}


int32_t PackRegStatusJsonValue(Json::Value &jsonValue, SimCardInfoToMcu_t currentRegSta, uint8_t flag, const char *keyValue)
{
    char tmpBuff[64] = {0};
    uint32_t tmpCid = 0;
    uint16_t tmpLac = 0;
    uint16_t tmpPcid5g = 0;
    uint64_t tmpCid5g = 0;

    tmpCid |= currentRegSta.cid[0] << 24;
    tmpCid |= currentRegSta.cid[1] << 16;
    tmpCid |= currentRegSta.cid[2] << 8;
    tmpCid |= currentRegSta.cid[3] << 0;

    tmpLac |= currentRegSta.lac[0] << 8;
    tmpLac |= currentRegSta.lac[1];

    tmpPcid5g |=  currentRegSta.pcid5g[0] << 8;
    tmpPcid5g |=  currentRegSta.pcid5g[1] << 0;

    tmpCid5g |= currentRegSta.cid5g[0] << 56;
    tmpCid5g |= currentRegSta.cid5g[1] << 48;
    tmpCid5g |= currentRegSta.cid5g[2] << 40;
    tmpCid5g |= currentRegSta.cid5g[3] << 32;
    tmpCid5g |= currentRegSta.cid5g[4] << 24;
    tmpCid5g |= currentRegSta.cid5g[5] << 16;
    tmpCid5g |= currentRegSta.cid5g[6] << 8;
    tmpCid5g |= currentRegSta.cid5g[7] << 0;

    if(1 == flag)
    {
        snprintf(tmpBuff, 64, "%s,%s,%X,%X", currentRegSta.mcc, currentRegSta.mnc, tmpCid5g, tmpPcid5g);
    }
    else
    {
        snprintf(tmpBuff, 64, "%s,%s,%X,%X", currentRegSta.mcc, currentRegSta.mnc, tmpCid, tmpLac);
    }
    
    jsonValue[keyValue] = tmpBuff;

    return RET_OK;
}


int32_t PackRegStatusJsonValue(Json::Value &jsonValue, SimCardInfoToMpu_t fromMcuRegSta, uint8_t flag, const char *keyValue)
{
    char tmpBuff[64] = {0};
    uint32_t tmpCid = 0;
    uint16_t tmpLac = 0;
    uint16_t tmpPcid5g = 0;
    uint64_t tmpCid5g = 0;

    tmpCid |= fromMcuRegSta.cid[0] << 24;
    tmpCid |= fromMcuRegSta.cid[1] << 16;
    tmpCid |= fromMcuRegSta.cid[2] << 8;
    tmpCid |= fromMcuRegSta.cid[3] << 0;

    tmpLac |= fromMcuRegSta.lac[0] << 8;
    tmpLac |= fromMcuRegSta.lac[1];

    tmpPcid5g |=  fromMcuRegSta.pcid5g[0] << 8;
    tmpPcid5g |=  fromMcuRegSta.pcid5g[1] << 0;

    tmpCid5g |= fromMcuRegSta.cid5g[0] << 56;
    tmpCid5g |= fromMcuRegSta.cid5g[1] << 48;
    tmpCid5g |= fromMcuRegSta.cid5g[2] << 40;
    tmpCid5g |= fromMcuRegSta.cid5g[3] << 32;
    tmpCid5g |= fromMcuRegSta.cid5g[4] << 24;
    tmpCid5g |= fromMcuRegSta.cid5g[5] << 16;
    tmpCid5g |= fromMcuRegSta.cid5g[6] << 8;
    tmpCid5g |= fromMcuRegSta.cid5g[7] << 0;

    if(1 == flag)
    {
        snprintf(tmpBuff, 64, "%s,%s,%X,%X", fromMcuRegSta.mcc, fromMcuRegSta.mnc, tmpCid5g, tmpPcid5g);
    }
    else
    {
        snprintf(tmpBuff, 64, "%s,%s,%X,%X", fromMcuRegSta.mcc, fromMcuRegSta.mnc, tmpCid, tmpLac);
    }
    
    jsonValue[keyValue] = tmpBuff;

    return RET_OK;
}

int32_t PackRegStatusJsonValue(Json::Value &jsonValue,GnssLocation_t &currentGnssInfo)
{
    if(0 == (currentGnssInfo.gpsState & 0x1F))
    {
        // GPS未定位，但仍需设置基本字段
        jsonValue["WE"] = "-";
        jsonValue["NS"] = "-";
        jsonValue["Lon"] = 0.0;
        jsonValue["Lat"] = 0.0;
        jsonValue["Hop"] = 0.0;
        jsonValue["Speed"] = 0.0;
        return RET_ERR;
    }

    if(0 == (currentGnssInfo.gpsState & 0x40))
    {
        jsonValue["WE"] = "W";
    }
    else
    {
        jsonValue["WE"] = "E";
    }

    if(0 == (currentGnssInfo.gpsState & 0x80))
    {
        jsonValue["NS"] = "S";
    }
    else
    {
        jsonValue["NS"] = "N";
    }

    jsonValue["Lon"] = currentGnssInfo.flongitude;
    jsonValue["Lat"] = currentGnssInfo.flatitude;
    jsonValue["Hop"] = currentGnssInfo.seaHeight * 0.1f;
    jsonValue["Speed"] = currentGnssInfo.speed * 0.1f;

    return RET_OK;
}





int MiscCycleData(char *vehicleData, int dataLen)
{
    LOG_Debug("PackageCycleData is start");
    char tboxICCID[21] = {0};           //存放ICCID
    char versionStr[32] = {0};          //存放软件版本号
    char tboxSN[32] = {0};               //存放tboxSN号
    long sysRunTime = 0;                   //存放系统运行时间
    OperatorType OpType;                   //运营商厂商
    char tmpBuff[64] = {0};
    uint32_t tmpFlow = 0;
    uint32_t tmpVersionPro = 0;
    uint32_t tmpVersionMcu = 0;
    uint32_t tmpVersionMpu = 0;
    uint32_t tmpmodemDialNum = 0;
    uint32_t tmpmodemResetNum = 0;
    uint32_t tmpmodemHQnum = 0;
    uint32_t tmpMile = 0;
    uint32_t tmpRunTimes = 0;
    uint16_t tmpMResetNum = 0;
    uint16_t tmpSResetNum = 0;
    char carid[30] = {0};
    char vinBuff[32] = {0};
    GnssLocation_t currentGnssInfo = {0};
    long long timestamp = 0;
    ModemSrvExtSigInfo_t modemSrvExtSigInfo;
    SimCardInfoToMcu_t currentSimCardInfo;
    static SimCardInfoToMpu_t simCardInfoFromMcu;

    Json::Value tmpJsonValue;             
    Json::Value operatorValue;
    Json::Value operatorValueFromMcu;
    Json::Value payload;
    static Json::Value tmpLocation;
    int32_t size = 0;

    if (vehicleData == NULL || dataLen <= 0)
    {
        LOG_Error("PackageCycleData error");
        return RET_ERR;
    }

    if(GetVehicleName(carid) == -1)
    {
        LOG_Error("GetCarNum fail");
        return RET_ERR;
    }

    if (GetVIN((uint8_t *)vinBuff) == -1)
    {
        LOG_Error("get vin Error");
        return RET_ERR;
    }
   
    payload["VIN"] =(vinBuff);
    payload["VehicleNo"] = carid;
    
    
    timestamp =  GetCurrentTime();
    payload["TimeStamp"] = static_cast<unsigned int>((0));
    payload["ProtocolVer"] = GPSPROTOCOLVER;
    payload["ServiceId"] = GPSSERVICEID;
    payload["SubServiceId"] = GPSSUBSERVICEID;

    GetGnssLocation(&currentGnssInfo);
    
    
    if(0 == (currentGnssInfo.gpsState & 0x1F))
    {
        tmpJsonValue["FixStatus"] = 0;
        tmpJsonValue["WE"] = '-';
        tmpJsonValue["NS"] = '-';
        tmpJsonValue["Lat"] = 0;
        tmpJsonValue["Hop"] = 0;
        tmpJsonValue["Speed"] = 0;
        tmpJsonValue["HDOP"] = 0;
        tmpJsonValue["VDOP"] = 0;
        tmpJsonValue["PDOP"] = 0;
        tmpJsonValue["SNR"] = 0;
        tmpJsonValue["SatellitesInView"] = currentGnssInfo.moonLinkNum;
    }else{

        tmpJsonValue["FixStatus"] = 1;
        if(0 == (currentGnssInfo.gpsState & 0x40))
        {
            tmpJsonValue["WE"] = 'W';
        }
        else
        {
            tmpJsonValue["WE"]  = 'E';
        }

        if(0 == (currentGnssInfo.gpsState & 0x80))
        {
            tmpJsonValue["NS"] = 'S';
        }
        else
        {
            tmpJsonValue["NS"] = 'N';
        }
        tmpJsonValue["Lon"] = currentGnssInfo.flongitude;
        tmpJsonValue["Lat"] = currentGnssInfo.flatitude;
        tmpJsonValue["Hop"] = currentGnssInfo.seaHeight * 0.1f;
        tmpJsonValue["Speed"] = currentGnssInfo.speed * 0.1f;
        tmpJsonValue["HDOP"] = 0.01;
        tmpJsonValue["VDOP"] = 0.01;
        tmpJsonValue["PDOP"] = 0.01;
        tmpJsonValue["SNR"] = 1;
        tmpJsonValue["SatellitesInView"] = currentGnssInfo.moonLinkNum;
    }

   

    VehicleCycleData ZelosvehicleCycleData;

    if (GetVehicleCycleData(&ZelosvehicleCycleData) != RET_OK)
    {
        LOG_Error("GetZelosVehicleCycleData failed");
        return RET_ERR;
    }
    LOG_Debug("GetVehicleCycleData success");

//     //车辆剩余电量
//     if(ZelosvehicleCycleData.VehicleSOC != 0xff)  
//     {
//         tmpJsonValue["VehicleSOC"] = ZelosvehicleCycleData.VehicleSOC;
//     }
//     else
//     {
//         tmpJsonValue["VehicleSOC"] = -1; // 默认值0
//     }
//     //充电状态
//     if(ZelosvehicleCycleData.ChargeStatus != 0xff)
//     {
//         tmpJsonValue["ChargeStatus"] = ZelosvehicleCycleData.ChargeStatus;
//     }
//     else
//     {
//         tmpJsonValue["ChargeStatus"] = -1; // 默认值0
//     }
//     //上下电状态
//     if(ZelosvehicleCycleData.VehicleStatus != 0xff)
//     {
//         tmpJsonValue["VehicleState"] = ZelosvehicleCycleData.VehicleStatus;
//     }
//     else
//     {
//         tmpJsonValue["VehicleState"] = -1; // 默认值3
//     }

//     tmpMile = ZelosvehicleCycleData.VCU_VehODO_km[0] << 24;
//     tmpMile |= ZelosvehicleCycleData.VCU_VehODO_km[1] << 16;
//     tmpMile |= ZelosvehicleCycleData.VCU_VehODO_km[2] << 8;
//     tmpMile |= ZelosvehicleCycleData.VCU_VehODO_km[3];
//     if(tmpMile != 0xFFFFFFFF)
//     {
//         tmpJsonValue["Mile"] = tmpMile; // 默认值3
//     }
//     else
//     {
//         tmpJsonValue["Mile"] = -1; // 默认值3
//     }

//     //仪表校准SOC
//    if(ZelosvehicleCycleData.VehicleMeterSOC != 0xff)
//    {
//        tmpJsonValue["VehicleMeterSOC"] = ZelosvehicleCycleData.VehicleMeterSOC;
//    }
//    else
//    { 
//          tmpJsonValue["VehicleMeterSOC"] = -1; //默认值 255 
//    }
//    //下电倒计时(0-63, Unit: 秒) 此字段会影响平台下发远程下电，先去掉，后续等协议变更。
//    if(ZelosvehicleCycleData.PowerOffCountdown != 0xff)
//    {
//     //    tmpJsonValue["PowerOffCountdown"] = ZelosvehicleCycleData.PowerOffCountdown;
//    }
//    else
//    { 
//         //  tmpJsonValue["PowerOffCountdown"] = 0; //默认值 0
//    }
   // TBOX供电电压
   uint16_t pwrVal = (ZelosvehicleCycleData.pwr[0] << 8) | ZelosvehicleCycleData.pwr[1];
//    printf("pwr[0] = 0x%02X, pwr[1] = 0x%02X, pwrVal = %u (0x%04X)\n",
//           ZelosvehicleCycleData.pwr[0],
//           ZelosvehicleCycleData.pwr[1],
//           pwrVal, pwrVal);
   float pwrFloat = roundf(pwrVal * 0.1f);
   tmpJsonValue["Pwr"] = pwrFloat;

   // 温度
   int16_t tempVal = (ZelosvehicleCycleData.temp[0] << 8) | ZelosvehicleCycleData.temp[1];
    //   printf("temp[0] = 0x%02X, temp[1] = 0x%02X, tempVal = %u (0x%04X)\n",
    //       ZelosvehicleCycleData.temp[0],
    //       ZelosvehicleCycleData.temp[1],
    //       tempVal, tempVal);
   float tempFloat = tempVal * 0.1f;
   //printf("tempFloat = %.2f\n", tempFloat);
   tmpJsonValue["Tmp"] = tempFloat;

   // 获取TBOX 设备SN号 减1的目的：给'\0'留位置
   if (GetSN((uint8_t *)tboxSN, sizeof(tboxSN) - 1))
   {
       tmpJsonValue["SN"] = tboxSN;
   }
   else
   { 
        tmpJsonValue["SN"] = "2507261245";
   }

    // 获取TBOX软件版本号
    if (GetTboxSoftVersion((uint8_t *)versionStr, sizeof(versionStr)))
    {
        tmpJsonValue["VER"] = (char *)versionStr;
    }
    else
    {
        tmpJsonValue["VER"] = "VTBOX_5G.202507A.4";
    }

     //CAN 发送消息数目
    int32_t u32CanTxMsgCnt = BytesToInt32(ZelosvehicleCycleData.g_u32CanTxMsgCnt); 
    tmpJsonValue["VCANTx"] = u32CanTxMsgCnt;

    //CAN 接收消息数目
    int32_t u32CanRxMsgCnt = BytesToInt32(ZelosvehicleCycleData.g_u32CanRxMsgCnt);
    tmpJsonValue["VCANRx"] = u32CanRxMsgCnt;  

    //内网连接状态
    memset(tmpBuff, 0, 64);
    snprintf(tmpBuff, 64, "%02X",  ZelosvehicleCycleData.lanStatus);
    tmpJsonValue["LAN"] = tmpBuff;    
    //获取设备运行时间
    // sysRunTime = getRunTime();
    //AG591系统是64位的，在64位的情况下Json::Value可能无法解析long类型数据 需要强制转换。
    tmpRunTimes = ZelosvehicleCycleData.mcuRunTimes[0] << 24;
    tmpRunTimes |= ZelosvehicleCycleData.mcuRunTimes[1] << 16;
    tmpRunTimes |= ZelosvehicleCycleData.mcuRunTimes[2] << 8;
    tmpRunTimes |= ZelosvehicleCycleData.mcuRunTimes[3];
    tmpJsonValue["RunSec"] = tmpRunTimes;          
    // 获取当前系统时间（单位：秒）
    //time_t currentTime = time(NULL);   
    
    //tmpJsonValue["Time"] = static_cast<int>(timestamp);

    if (RET_OK == GetCurrentSimCardInfo(&currentSimCardInfo))
    {

        memcpy(tboxICCID, currentSimCardInfo.ICCID, 20);
        LOG_Debug("tboxICCID (raw): [%s]\n", tboxICCID);
        operatorValue["ICCID"] = tboxICCID; // ICCID

        // operatorValue["SNR"] = currentSimCardInfo.signalSnr;        // 信噪比
        // operatorValue["SSI"] = currentSimCardInfo.signalSsi;        // 信号强度
        snprintf(tmpBuff, 64, "%d,%d",  currentSimCardInfo.signalSsi, currentSimCardInfo.signalSnr);
        operatorValue["SIG"] = tmpBuff;

        memset(tmpBuff, 0, 64);
       
        tmpFlow = (currentSimCardInfo.flow[3] << 24) | \
                  (currentSimCardInfo.flow[2] << 16) | \
                  (currentSimCardInfo.flow[1] << 8) | \
                  (currentSimCardInfo.flow[0]);
        //operatorValue["Flow"] = tmpFlow;          // 流量消耗
        
        tmpVersionPro = (currentSimCardInfo.modemVer[6] << 16) | \
                        (currentSimCardInfo.modemVer[5] << 8) | \
                        (currentSimCardInfo.modemVer[4]);
         LOG_Debug("tmpValue :%d, %d,%d, %d", tmpVersionPro, currentSimCardInfo.modemVer[6], currentSimCardInfo.modemVer[5], currentSimCardInfo.modemVer[4]);
        tmpVersionMcu = (currentSimCardInfo.modemVer[3] << 8) | \
                        (currentSimCardInfo.modemVer[2]);
        tmpVersionMpu = (currentSimCardInfo.modemVer[1] << 8) | \
                        (currentSimCardInfo.modemVer[0]);

        snprintf(tmpBuff, 64, "%02d,%02d,%02d", 0, tmpVersionPro % 100, tmpVersionMpu % 100);

        operatorValue["VER"] = tmpBuff;  // 模块软件版本

        memset(tmpBuff, 0, 64);

        tmpmodemDialNum = (currentSimCardInfo.modemDialNum[3] << 24) | \
                          (currentSimCardInfo.modemDialNum[2] << 16) | \
                          (currentSimCardInfo.modemDialNum[1] << 8) | \
                          (currentSimCardInfo.modemDialNum[0]);

        tmpmodemResetNum = (currentSimCardInfo.modemResetNum[3] << 24) | \
                           (currentSimCardInfo.modemResetNum[2] << 16) | \
                           (currentSimCardInfo.modemResetNum[1] << 8) | \
                           (currentSimCardInfo.modemResetNum[0]);
        
        tmpmodemHQnum = (currentSimCardInfo.modemHQnum[3] << 24) | \
                          (currentSimCardInfo.modemHQnum[2] << 16) | \
                          (currentSimCardInfo.modemHQnum[1] << 8) | \
                          (currentSimCardInfo.modemHQnum[0]);
        
        if(1 == simCardInfoFromMcu.modemMasterOrSlave)
        {
            snprintf(tmpBuff, 64, "%x,%u,%u,%u", currentSimCardInfo.modemSta & 0xFF,\
                                               tmpmodemDialNum,\
                                               tmpMResetNum,\
                                               tmpmodemHQnum);
        }
        else if(2 == simCardInfoFromMcu.modemMasterOrSlave)
        {
            snprintf(tmpBuff, 64, "%x,%u,%u,%u", currentSimCardInfo.modemSta & 0xFF,\
                                               tmpmodemDialNum,\
                                               tmpSResetNum,\
                                               tmpmodemHQnum);
        }
        else
        {
            snprintf(tmpBuff, 64, "%x,%u,%u,%u", currentSimCardInfo.modemSta & 0xFF,\
                                               tmpmodemDialNum,\
                                               0,\
                                               tmpmodemHQnum);
        }
        
        operatorValue["St"] = tmpBuff; //"St": "模组状态，模块重新拨号次数，模块重启次数，高质量时间"

        switch(currentSimCardInfo.modemDialType)
        {
            case 1:
                // operatorValue["NonCDMA"] = "MCC.MNC.CI.LAC";
                PackRegStatusJsonValue(operatorValue, currentSimCardInfo, 0, "NonCDMA");
                break;
            case 2:
                // operatorValue["CDMA"] = "MCC.MNC.CI.LAC";
                PackRegStatusJsonValue(operatorValue, currentSimCardInfo, 0, "CDMA");
                break;
            case 3:
                // operatorValue["NR"] = "MCC.MNC.CI.PCID";
                PackRegStatusJsonValue(operatorValue, currentSimCardInfo, 1, "NR");
                break;
            case 4:
                // operatorValue["LTE"] = "MCC.MNC.CI.PCID";
                PackRegStatusJsonValue(operatorValue, currentSimCardInfo, 1, "LTE");
                break;
            case 5:
                // operatorValue["ENDC"] = "MCC.MNC.CI.PCID";
                PackRegStatusJsonValue(operatorValue, currentSimCardInfo, 1, "ENDC");
                break;
            case 6:
                // operatorValue["WCDMA"] = "MCC.MNC.CI.LAC";
                PackRegStatusJsonValue(operatorValue, currentSimCardInfo, 1, "WCDMA");
                break;
        }

        // 通过ICCID判断运营商厂商  （无论是移动还是电信的厂商都需要上报）
        OperatorType OpType = GetOperatorFromIccid((char *)tboxICCID);
        switch (OpType)
        {
        case OP_CMCC: // 移动
            LOG_Info("OperatorType is CMCC!");
            tmpJsonValue["CMCC"] = operatorValue;
            break;
        case OP_CTCC: // 电信
            LOG_Info("OperatorType is CTCC");
            tmpJsonValue["CT"] = operatorValue;
            break;
        case OP_CUCC: // 联通
            LOG_Info("OperatorType is CUCC");
            tmpJsonValue["CU"] = operatorValue;
            break;
        default:
            LOG_Warn("OperatorType is unknown");
            break;
        }
    }

    if (RET_OK == GetSimCardInfoFromMcu(&simCardInfoFromMcu))
    {
        memset(tmpBuff, 0, 64);
        snprintf(tmpBuff, 64, "ff%02x", simCardInfoFromMcu.funcSwitch);
        tmpJsonValue["FunC"] = tmpBuff; 

        if(simCardInfoFromMcu.modemMasterOrSlave == 1 && (currentSimCardInfo.modemSta & 0x40) == 0)
        {
            tmpJsonValue["SIM"]  = 2;                    //SIM卡
            tmpJsonValue["MQTT"] = 1;                     //当前报文MQTT实例ID
        }
        else if(simCardInfoFromMcu.modemMasterOrSlave == 1 && (currentSimCardInfo.modemSta & 0x40) != 0)
        {
            tmpJsonValue["SIM"]  = 1;                    //SIM卡
            tmpJsonValue["MQTT"] = 1;                     //当前报文MQTT实例ID
        }

        if(simCardInfoFromMcu.modemMasterOrSlave == 2 && (currentSimCardInfo.modemSta & 0x40) == 0)
        {
            tmpJsonValue["SIM"]  = 1;                    //SIM卡
            tmpJsonValue["MQTT"] = 2;                     //当前报文MQTT实例ID
        }
        else if(simCardInfoFromMcu.modemMasterOrSlave == 2 && (currentSimCardInfo.modemSta & 0x40) != 0)
        {
            tmpJsonValue["SIM"]  = 2;                    //SIM卡
            tmpJsonValue["MQTT"] = 2;                     //当前报文MQTT实例ID
        }
        LOG_Debug("sim: [%d] %d\n", simCardInfoFromMcu.modemMasterOrSlave, currentSimCardInfo.modemSta);

        if(simCardInfoFromMcu.modemMasterOrSlave == 2)
        {
            tmpSResetNum |= ZelosvehicleCycleData.slaveModemResetNum[2] << 8;
            tmpSResetNum |= ZelosvehicleCycleData.slaveModemResetNum[3];
        }
        else
        {
            tmpMResetNum |= ZelosvehicleCycleData.masterModemResetNum[2] << 8;
            tmpMResetNum |= ZelosvehicleCycleData.masterModemResetNum[3];
        }

        memcpy(tboxICCID, simCardInfoFromMcu.ICCID, 20);
        LOG_Debug("tboxICCID (raw): [%s]\n", tboxICCID);
        operatorValueFromMcu["ICCID"] = tboxICCID; // ICCID

        // operatorValueFromMcu["SNR"] = simCardInfoFromMcu.signalSnr;        // 信噪比
        // operatorValueFromMcu["SSI"] = simCardInfoFromMcu.signalSsi;        // 信号强度
        snprintf(tmpBuff, 64, "%d,%d",  simCardInfoFromMcu.signalSsi, simCardInfoFromMcu.signalSnr);
        operatorValueFromMcu["SIG"] = tmpBuff;

        memset(tmpBuff, 0, 64);

        tmpFlow = (simCardInfoFromMcu.flow[3] << 24) | \
                  (simCardInfoFromMcu.flow[2] << 16) | \
                  (simCardInfoFromMcu.flow[1] << 8) | \
                  (simCardInfoFromMcu.flow[0]);
        //operatorValueFromMcu["Flow"] = tmpFlow;          // 流量消耗

        tmpVersionPro = (simCardInfoFromMcu.modemVer[6] << 16) | \
                        (simCardInfoFromMcu.modemVer[5] << 8) | \
                        (simCardInfoFromMcu.modemVer[4]);
        tmpVersionMcu = (simCardInfoFromMcu.modemVer[3] << 8) | \
                        (simCardInfoFromMcu.modemVer[2]);
        tmpVersionMpu = (simCardInfoFromMcu.modemVer[1] << 8) | \
                        (simCardInfoFromMcu.modemVer[0]);

        snprintf(tmpBuff, 64, "%02d,%02d,%02d", 0, tmpVersionPro % 100, tmpVersionMpu % 100);

        operatorValueFromMcu["VER"] = tmpBuff;  // 模块软件版本

        memset(tmpBuff, 0, 64);

        tmpmodemDialNum = (simCardInfoFromMcu.modemDialNum[3] << 24) | \
                          (simCardInfoFromMcu.modemDialNum[2] << 16) | \
                          (simCardInfoFromMcu.modemDialNum[1] << 8) | \
                          (simCardInfoFromMcu.modemDialNum[0]);

        tmpmodemResetNum = (simCardInfoFromMcu.modemResetNum[3] << 24) | \
                           (simCardInfoFromMcu.modemResetNum[2] << 16) | \
                           (simCardInfoFromMcu.modemResetNum[1] << 8) | \
                           (simCardInfoFromMcu.modemResetNum[0]);
        
        tmpmodemHQnum = (simCardInfoFromMcu.modemHQnum[3] << 24) | \
                          (simCardInfoFromMcu.modemHQnum[2] << 16) | \
                          (simCardInfoFromMcu.modemHQnum[1] << 8) | \
                          (simCardInfoFromMcu.modemHQnum[0]);

        if(1 == simCardInfoFromMcu.modemMasterOrSlave)
        {
            snprintf(tmpBuff, 64, "%x,%u,%u,%u", simCardInfoFromMcu.modemSta & 0xFF,\
                                               tmpmodemDialNum,\
                                               tmpMResetNum,\
                                               tmpmodemHQnum);
        }
        else if(2 == simCardInfoFromMcu.modemMasterOrSlave)
        {
            snprintf(tmpBuff, 64, "%x,%u,%u,%u", simCardInfoFromMcu.modemSta & 0xFF,\
                                               tmpmodemDialNum,\
                                               tmpSResetNum,\
                                               tmpmodemHQnum);
        }
        else
        {
            snprintf(tmpBuff, 64, "%x,%u,%u,%u", simCardInfoFromMcu.modemSta & 0xFF,\
                                               tmpmodemDialNum,\
                                               0,\
                                               tmpmodemHQnum);
        }
        
        operatorValueFromMcu["St"] = tmpBuff; //"St": "模组状态，模块重新拨号次数，模块重启次数，高质量时间"

        switch(simCardInfoFromMcu.modemDialType)
        {
            case 1:
                // operatorValueFromMcu["NonCDMA"] = "MCC.MNC.CI.LAC";
                PackRegStatusJsonValue(operatorValueFromMcu, simCardInfoFromMcu, 0, "NonCDMA");
                break;
            case 2:
                // operatorValueFromMcu["CDMA"] = "MCC.MNC.CI.LAC";
                PackRegStatusJsonValue(operatorValueFromMcu, simCardInfoFromMcu, 0, "CDMA");
                break;
            case 3:
                // operatorValueFromMcu["NR"] = "MCC.MNC.CI.PCID";
                PackRegStatusJsonValue(operatorValueFromMcu, simCardInfoFromMcu, 1, "NR");
                break;
            case 4:
                // operatorValueFromMcu["LTE"] = "MCC.MNC.CI.PCID";
                PackRegStatusJsonValue(operatorValueFromMcu, simCardInfoFromMcu, 1, "LTE");
                break;
            case 5:
                // operatorValueFromMcu["ENDC"] = "MCC.MNC.CI.PCID";
                PackRegStatusJsonValue(operatorValueFromMcu, simCardInfoFromMcu, 1, "ENDC");
                break;
            case 6:
                // operatorValueFromMcu["WCDMA"] = "MCC.MNC.CI.LAC";
                PackRegStatusJsonValue(operatorValueFromMcu, simCardInfoFromMcu, 1, "WCDMA");
                break;
        }

        // 通过ICCID判断运营商厂商  （无论是移动还是电信的厂商都需要上报）
        OperatorType OpType = GetOperatorFromIccid((char *)tboxICCID);
        switch (OpType)
        {
        case OP_CMCC: // 移动
            LOG_Info("OperatorType is CMCC!");
            // 避免重复添加，只在没有CMCC信息时添加
            if (!tmpJsonValue.isMember("CMCC")) {
                tmpJsonValue["CMCC"] = operatorValueFromMcu;
            }
            break;
        case OP_CTCC: // 电信
            LOG_Info("OperatorType is CTCC");
            // 避免重复添加，只在没有CT信息时添加
            if (!tmpJsonValue.isMember("CT")) {
                tmpJsonValue["CT"] = operatorValueFromMcu;
            }
            break;
        case OP_CUCC: // 联通
            LOG_Info("OperatorType is CUCC");
            if (!tmpJsonValue.isMember("CU")) {
                tmpJsonValue["CU"] = operatorValueFromMcu;
            }
            break;
        default:
            LOG_Warn("OperatorType is unknown");
            break;
        }
    }

    if(0 != (simCardInfoFromMcu.modemSta & 0x80) || 0 != (currentSimCardInfo.modemSta & 0x80))
    {
        tmpJsonValue["Mode"] = 3;
    }
    else if(SGMW_PM_STATE_NORMAL == GetPmState())
    {
        tmpJsonValue["Mode"] = 0;
    }
    else
    {
        tmpJsonValue["Mode"] = 1;
    }

    // 处理GNSS信息
    PackRegStatusJsonValue(tmpLocation,currentGnssInfo);
    tmpJsonValue["GNSS"] = tmpLocation;
    payload["Payload"] = tmpJsonValue;

    // 将 JSON 转换为字符串
    Json::FastWriter fastWriter;
    //std::string jsonStr = fastWriter.write(tmpJsonValue);
    std::string jsonStr = fastWriter.write(payload);
    //清除最后的换行符 保证格式正确
    if (!jsonStr.empty() && jsonStr.back() == '\n')
    {
        jsonStr.pop_back();
    }
    // 打印出完整的JSON报文
    //LOG_Debug(" Final JSON string:%s", jsonStr.c_str());
    // 判断缓冲区是否足够大
    if (jsonStr.size() >= static_cast<size_t>(dataLen))
    {
        LOG_Error("Buffer too small to hold JSON datalast char");
        return RET_ERR;
    }

    char* json_str =  TimeStampChanged(jsonStr.c_str(),timestamp);
    if(json_str)
    {
        size = strlen(json_str);
        if(size >= dataLen)
        {
            size = -1;
            LOG_Error("Buffer too small size:%d",size);
        }else{
            strncpy(vehicleData,json_str, strlen(json_str));
        }
        
        free(json_str);
        json_str = NULL;
    }

    
    return size;
}
