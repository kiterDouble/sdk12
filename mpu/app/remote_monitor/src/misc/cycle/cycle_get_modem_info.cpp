#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "err_code.h"
#include "sr_log.h"
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include "ota_api.h"
#include "common_functions.h"
#include "cycle_evet_handle.h"
#include "net_service_api.h"
#include "hal_modem_sim.h"
#include "hal_modem_nw.h"
#include "modem_ipc_msg.h"
#include "modem_api.h"
#include "mqtt.h"
#include "mcu_api.h"
#include "sr_gnss.h"
#include "gnss_ipc_msg.h"
#include "common_functions.h"
#include "cycle_common.h"
static pthread_mutex_t g_simCarInfoLock = PTHREAD_MUTEX_INITIALIZER;

SimCardInfoToMpu_t g_SimCardInfoToMpu;
SimCardInfoToMcu_t g_SimCardInfoToMcu;
GnssLocation_t g_gnssInfo;

int32_t GetCurrentGnssInfo(GnssLocation_t *currentGnssInfo)
{
    if(NULL == currentGnssInfo)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    pthread_mutex_lock(&g_simCarInfoLock);
    memcpy(currentGnssInfo, &g_gnssInfo, sizeof(GnssLocation_t));
    pthread_mutex_unlock(&g_simCarInfoLock);

    return RET_OK;
}


int32_t GetCurrentSimCardInfo(SimCardInfoToMcu_t *currentSimCardInfo)
{
    if(NULL == currentSimCardInfo)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    pthread_mutex_lock(&g_simCarInfoLock);
    memcpy(currentSimCardInfo, &g_SimCardInfoToMcu, sizeof(SimCardInfoToMcu_t));
    pthread_mutex_unlock(&g_simCarInfoLock);

    return RET_OK;
}

int32_t GetSimCardInfoFromMcu(SimCardInfoToMpu_t *simCardInfoFromMcu)
{
    if(NULL == simCardInfoFromMcu)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    pthread_mutex_lock(&g_simCarInfoLock);
    memcpy(simCardInfoFromMcu, &g_SimCardInfoToMpu, sizeof(SimCardInfoToMpu_t));
    pthread_mutex_unlock(&g_simCarInfoLock);

    return RET_OK;
}


void SimCardInfoToMpuCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{

    if(NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if(dataLen != sizeof(SimCardInfoToMpu_t))
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    pthread_mutex_lock(&g_simCarInfoLock);
    memcpy(&g_SimCardInfoToMpu, recvData, dataLen);
    pthread_mutex_unlock(&g_simCarInfoLock);
}

/**
 * 获取指定网卡的 IPv4 地址
 * @param ifName 网卡名称，如 "eth0"
 * @param ipBuf  用于保存 IP 的缓冲区
 * @param bufLen 缓冲区大小
 * @return 成功返回 0，失败返回 -1
 */
int SrGetInterfaceIp(const char *ifName, char *ipBuf, size_t bufLen) 
{
    struct ifreq ifr;
    int sockfd;
    int ret = -1;

    if(NULL == ifName || strlen(ifName) > IFNAMSIZ)
    {
        LOG_Error("%s argv error", __FUNCTION__);
        return RET_ERR;
    }
    // 创建 UDP 套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
    {
        LOG_Error("create socket error");
        return RET_ERR;
    }

    // 设置网卡名称
    strncpy(ifr.ifr_name, ifName, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    // 获取 IP 地址
    if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) 
    {
        LOG_Error("ioctl(SIOCGIFADDR) on %s failed: %s\n", ifName, strerror(errno));
        close(sockfd);
        return RET_ERR;
    }

    // 将 IP 地址转换为字符串
    struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
    if (inet_ntop(AF_INET, &sin->sin_addr, ipBuf, bufLen) == NULL) 
    {
        LOG_Error("inet_ntop error");
        close(sockfd);
        return RET_ERR;
    }

    close(sockfd);
    return RET_OK;
}



/**
 * 获取指定网卡的 IPv4 地址
 * @param ifName 网卡名称，如 "eth0"
 * @param ipBuf  用于保存 IP 的缓冲区
 * @param bufLen 缓冲区大小
 * @return 成功返回 0，失败返回 -1
 */
int SrGetInterfaceDownUpStatus(const char *ifName) 
{
    struct ifreq ifr;
    int sockfd;
    int ret = -1;

    if(NULL == ifName || strlen(ifName) > IFNAMSIZ)
    {
        LOG_Error("%s argv error", __FUNCTION__);
        return RET_ERR;
    }
    // 创建 UDP 套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
    {
        LOG_Error("create socket error");
        return RET_ERR;
    }

    // 设置网卡名称
    strncpy(ifr.ifr_name, ifName, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    // 获取 IP 地址
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) 
    {
        LOG_Error("ioctl(SIOCGIFADDR) on %s failed: %s\n", ifName, strerror(errno));
        close(sockfd);
        return RET_ERR;
    }

    close(sockfd);
    return (ifr.ifr_flags & IFF_UP) ? RET_OK : RET_ERR;
}


/**
 * @name SrGetSystemRetValue
 * @brief  执行shell 命令，并获取返回值
 * @param 
 * @return -1 执行命令失败，其他值为命令的返回值
 */
int32_t SrGetSystemRetValue(const char *cmd)
{
	char cmdBuff[128] = {0};
	FILE *fileFp;				// 文件指针  
    int32_t cmdStatus;			// 用于存储命令的退出状态
	
	if(NULL == cmd)
	{
		LOG_Error("%s cmd is NULL", __FUNCTION__);
		return RET_ERR;
	}
    
    LOG_Debug("start exec cmd :%s", cmd);

	fileFp = popen(cmd, "r");
	if(NULL == fileFp)
	{
		LOG_Error("%s popen error :%s", __FUNCTION__, strerror(errno));
		return RET_ERR;
	}
	
	while(fgets(cmdBuff, 128, fileFp) != NULL)
	{
		LOG_Debug("get cmd ret: %s", cmdBuff);
	}
	
	cmdStatus = pclose(fileFp);
	if(-1 == cmdStatus)
	{
		LOG_Error("%s pclose error :%s", __FUNCTION__, strerror(errno));
		return RET_ERR;
	}
	
	if(WIFEXITED(cmdStatus))
	{  
        LOG_Debug("Command exited with status %d", WEXITSTATUS(cmdStatus));
		return WEXITSTATUS(cmdStatus);
    }
	else if(WIFSIGNALED(cmdStatus))
	{  
        LOG_Error("Command was terminated by signal %d", WTERMSIG(cmdStatus));
		return RET_ERR;
    }
	
	LOG_Error("Command was terminated not find signal" );
	return RET_ERR;
}


int32_t GeUpgradeStatusFromMasterOrSlave()
{
    int32_t retValue = RET_ERR;
    UpgradeStatus_E upgradeStatus;

    retValue = OtaGetUpgradeStatus(OTA_UPGRADE_PATH_NAME, &upgradeStatus);
    if(retValue < RET_OK)
    {
        LOG_Error("OtaGetUpgradeStatus error");
		return RET_ERR;
    }

    if(UPGRADE_INIT != upgradeStatus &&\
       UPGRADE_MPU_FAILED != upgradeStatus &&\
       UPGRADE_MPU_SUCCESS != upgradeStatus)
    {
        LOG_Debug("OtaGetUpgradeStatus is upgrade");
        return RET_OK;
    }

    // if(g_SimCardInfoToMpu.modemSta & 0x80) //另一个模组为升级状态时，返回升级状态。
    // {
    //     LOG_Debug("get upgrade status is upgrade");
    //     return RET_OK;
    // }
    // else
    // {
    //     return RET_ERR;
    // }
}

// 函数声明
uint64_t GetNetInterfaceBytes(const char *ifname)
{
	uint64_t rxBytes = 0;
	uint64_t txBytes = 0;
	char rxPath[256] = {0};
	char txPath[256] = {0};
	FILE *fpRx = NULL;
	FILE *fpTx = NULL;
	
	snprintf(rxPath, sizeof(rxPath), "/sys/class/net/%s/statistics/rx_bytes", ifname);
    snprintf(txPath, sizeof(txPath), "/sys/class/net/%s/statistics/tx_bytes", ifname);
	
	if((fpRx = fopen(rxPath, "r")) == NULL) 
	{
        LOG_Error("Failed to open rxPath file:%s\n", rxPath);
        return RET_ERR;
	}
	LOG_Debug("fopen rxPath %s success \n", rxPath);

    if (fscanf(fpRx, "%llu", &rxBytes) != 1) 
	{
        LOG_Error("Failed to read rxBytes\n");
        fclose(fpRx);
        return RET_ERR;
    }
    fclose(fpRx);
	LOG_Debug("read rxBytes success :%lld\n", rxBytes);

	if((fpTx = fopen(txPath, "r")) == NULL) 
	{
        LOG_Error("Failed to open txPath file:%s\n", txPath);
        return RET_ERR;
	}
    LOG_Debug("fopen txPath %s success \n", txPath);

    if (fscanf(fpTx, "%llu", &txBytes) != 1) 
	{
        LOG_Error("Failed to read txBytes\n");
        fclose(fpTx);
        return RET_ERR;
    }
    fclose(fpTx);
	LOG_Debug("read txBytes success :%lld\n", txBytes);
	return (rxBytes + txBytes) / 1048576; // 返回总和
}

int32_t GetModemCgregStatus(void)
{
    // SrGetSystemRetValue("echo \"#!/bin/bash\" > /oemdata/log/get_cgreg.sh");
    // SrGetSystemRetValue("echo \"retvalue=\\$(ql_cmd  at at+cgreg? | grep CGREG | awk -F, '{print \\$2}' | tr -d ' ')\" >> /oemdata/log/get_cgreg.sh");
    // SrGetSystemRetValue("echo \"num=\\$(( retvalue + 0 ))\" >> /oemdata/log/get_cgreg.sh");
    // SrGetSystemRetValue("echo \"exit \\$num\" >> /oemdata/log/get_cgreg.sh");

    //return SrGetSystemRetValue("sh /oemapp/scripts/get_cgreg.sh");
    HalModemNwRegStatus_e status;
    int32_t retValue = HalModemNwGetRegStatus(&status);
    
    if (retValue == RET_OK && status == E_HAL_MODEM_NW_REG_STATUS_REGISTERED)
    {
        return 1;  // 已注册（对应原脚本的 CGREG 返回值）
    }
    return 0;  // 未注册
}

int32_t GetModemDialStatus(void)
{
    int32_t retValue = -1;
    StrNetStatus_t netStatus;
    uint32_t modemDialNum;

    netStatus.apnNum = 1;
    retValue = GetNetStatusInfo(&netStatus, 1000);
    if(retValue < RET_ERR)
    {
        LOG_Error("%s GetNetStatusInfo is error", __FUNCTION__);
        return RET_ERR;
    }    
    
    modemDialNum = netStatus.netStatusInfo[0].dialTimes;
    g_SimCardInfoToMcu.modemDialNum[0] = modemDialNum & 0xFF;
    g_SimCardInfoToMcu.modemDialNum[1] = (modemDialNum >> 8) & 0xFF;
    g_SimCardInfoToMcu.modemDialNum[2] = (modemDialNum >> 16)& 0xFF;
    g_SimCardInfoToMcu.modemDialNum[3] = (modemDialNum >> 24)& 0xFF;


    if(0 == netStatus.netStatusInfo[0].dialStatus)
    {
        LOG_Error("%s netStatus.netStatusInfo.dialStatus is error", __FUNCTION__);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s netStatus.netStatusInfo.dialStatus is OK", __FUNCTION__);
        return RET_OK;
    }
}

int32_t GetSimCardStatus(void)
{
    int32_t retValue = -1;
    HalModemSimCardStatus_e cardStat;
    HalModemSimAppStatus_e appStat;

    retValue = HalModemSimGetCardStatus(&cardStat, &appStat);
    if(retValue < RET_ERR)
    {
        LOG_Error("%s HalModemSimGetCardStatus is error", __FUNCTION__);
        return RET_ERR;
    }

    if(E_HAL_MODEM_SIM_CARD_STATUS_PRESENT != cardStat)
    {
        LOG_Error("%s HalModemSimGetCardStatus cardStat is error", __FUNCTION__);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s HalModemSimGetCardStatus cardStat is OK", __FUNCTION__);
        return RET_OK;
    }
}

int32_t GetModemVersion(uint8_t *modemVersion, uint8_t versionLen)
{
    int32_t retValue = RET_ERR;
    uint8_t fileContent[16] = {0};
    uint8_t tmpBuff[6] = {0}; 
    uint32_t tmpValue = 0;

    if(NULL == modemVersion || versionLen < 7)
    {
        LOG_Error("%s argv error: %d", __FUNCTION__);
        return RET_ERR;
    }

    retValue = GetFileContent("/oemapp/upgrade/version.txt", (char *)fileContent, 16);
    if(retValue < 0 || retValue < 11)
    {
        LOG_Error("%s GetFileContent error: %d", __FUNCTION__, retValue);
        return RET_ERR;
    }

    tmpBuff[0] = fileContent[0];
    tmpBuff[1] = fileContent[1];
    tmpBuff[2] = fileContent[2];
    tmpBuff[3] = fileContent[3];
    tmpBuff[4] = fileContent[4];
    tmpValue = atoi((char *)tmpBuff);
   
    modemVersion[6] = (tmpValue >> 16) & 0xFF;
    modemVersion[5] = (tmpValue >> 8) & 0xFF;
    modemVersion[4] =  tmpValue & 0xFF;
     LOG_Debug("tmpValue :%d, %d,%d,%d", tmpValue, modemVersion[6], modemVersion[5],modemVersion[4]);
    memset(tmpBuff, 0, 6);
    tmpBuff[0] = fileContent[5];
    tmpBuff[1] = fileContent[6];
    tmpBuff[2] = fileContent[7];
    tmpValue = atoi((char *)tmpBuff);
    modemVersion[3] = (tmpValue >> 8) & 0xFF;
    modemVersion[2] =  tmpValue & 0xFF;

    tmpBuff[0] = fileContent[8];
    tmpBuff[1] = fileContent[9];
    tmpBuff[2] = fileContent[10];
    tmpValue = atoi((char *)tmpBuff);
    modemVersion[1] = (tmpValue >> 8) & 0xFF;
    modemVersion[0] =  tmpValue & 0xFF;

    return RET_OK;
}



int32_t GetFileLineCount(const char *filename) 
{
    int32_t lineCount = 0;
    int ch;
    uint8_t last_char_was_newline = 0;

    FILE *file = fopen(filename, "r");
    if (file == NULL) 
    {
        LOG_Error("Error opening file");
        return RET_OK;
    }

    while ((ch = fgetc(file)) != EOF) 
    {
        if (ch == '\n') 
        {
            lineCount++;
            last_char_was_newline = 1;
        } 
        else
        {
            last_char_was_newline = 0;
        }
    }

    // 处理最后一行没有换行符的情况
    if (!last_char_was_newline && lineCount > 0) 
    {
        lineCount++;
    } 
    else if (lineCount == 0) 
    {
        fseek(file, 0, SEEK_SET);
        if (fgetc(file) != EOF) 
        {
            lineCount = 1;
        }
    }

    fclose(file);
    return lineCount;
}

int32_t GetModemSignalQualityAndDialType(uint8_t *rssi, uint8_t *snr, uint8_t *dialType)
{
    ModemSrvExtSigInfo_t modemSrvExtSigInfo;
    int32_t retValue = -1;
    int16_t rsrpValue = -1;
    double sinrValue = -1;
    uint8_t dialValue = 0;

    if(NULL == rssi || NULL == snr)
    {
        LOG_Error("%s argv is error", __FUNCTION__);
        return RET_ERR;
    }

    retValue = ModemSrvGetExtSignalInfo(&modemSrvExtSigInfo);
    if(retValue < RET_OK)
    {
        *dialType = 255;//获取失败时，基站信息不存在，不需要组基站相关信息。
        LOG_Error("%s ModemSrvGetExtSignalInfos error", __FUNCTION__);
        return RET_ERR;
    }
    

    rsrpValue = modemSrvExtSigInfo.lteSiginfo.rsrp;
    sinrValue = 10.0 * log10(modemSrvExtSigInfo.lteSiginfo.sinr);
    dialValue = modemSrvExtSigInfo.lteSiginfo.sigType;

    LOG_Debug("rssi: %d : sinr:%f, dialValue:%d", rsrpValue, sinrValue, dialValue);

    if(rsrpValue >= -50)
    {
        *rssi = 31;
    }
    else if(rsrpValue <= -120)
    {
        *rssi = 1;
    }
    else
    {
        *rssi = 31 - int16_t((-50 - rsrpValue) / 2.5);
    }
    
    if(sinrValue >= 30.0)
    {
        *snr = 31;
    }
    else if(sinrValue <= 0.0)
    {
        *snr = 1;
    }
    else
    {
        *snr = 1 + ((sinrValue / 30.0) * 30);
    }

    switch(dialValue)
    {
        case 1:
            *dialType = 1;         //1:NonCDMA(4G 非CDMA)
            break;
        case 2:
            *dialType = 2;         //2:CDMA(4G CDMA)
            break;
        case 3:
            *dialType = 3;         //3:NR(5G-SA)
            break;
        case 4:
            *dialType = 4;         //4:LTE(5G-LTE)
            break;
        case 5:
            *dialType = 5;         //5:ENDC(5G-NSA)
            break;
        case 6:
            *dialType = 6;         //6: WCDMA(5G-CDMA)
            break;
    }

    return RET_OK;
}
  


void *GetModemInfoThread(void *pArg)
{
    int32_t retValue = RET_ERR;
    uint8_t tboxICCID[25] = {0};
    char pingCmd[128] = {0};
    uint8_t modemVersion[7] = {0};
    uint8_t whileCount = 3;
    gboolean boolRetValue = 0;
    char  ipaddrBuff[16] = {0};
    uint64_t flowValue = 0;
    uint32_t modemHQnum = 0;
    uint32_t modemHQnumCount = 0;
    uint8_t  sendBuff[128] = {0};
    McuMesg_t sendToMcu;
    HalModemNwRegStatusInfo_t regStaInfo;

    //snprintf(pingCmd, 128, "timeout 1 ping -c 1 %s", MQTT_CONNECT_IP_ADDR);

    HalModemSimInit();
    HalModemNwInit();

    memset(&g_SimCardInfoToMpu, 0, sizeof(SimCardInfoToMpu_t));
    memset(&g_SimCardInfoToMcu, 0, sizeof(SimCardInfoToMcu_t));

    do
    {
        boolRetValue = GetIccid(tboxICCID, 24);
        if(boolRetValue)
        {
            whileCount = 3;
            break;
        }
        whileCount--;
        sleep(1);
    } while (whileCount > 0);
    
    //ICCID 
    memcpy(g_SimCardInfoToMcu.ICCID, tboxICCID, 20);

    while (1)
    {
        pthread_mutex_lock(&g_simCarInfoLock);
        //模组状态
        retValue = GetSimCardStatus();
        if(retValue == RET_OK)
        {
            g_SimCardInfoToMcu.modemSta = 0x01;
        }
        else
        {
            g_SimCardInfoToMcu.modemSta = 0x00;
        }

        LOG_Debug("g_SimCardInfoToMcu.modemSta : %02x", g_SimCardInfoToMcu.modemSta);

        HalModemNwRegStatus_e status = E_HAL_MODEM_NW_REG_STATUS_UNREGISTERED;
        retValue = HalModemNwGetRegStatus(&status);
        if(retValue != RET_ERR){
            g_SimCardInfoToMcu.modemSta |= (!status << 1);
        }else{
            g_SimCardInfoToMcu.modemSta |= (0 << 1);
        }
        LOG_Debug("cgreg : %u", !status);
        /*
        retValue = GetModemCgregStatus();
        g_SimCardInfoToMcu.modemSta |= ((retValue & 0x0F) << 1);
        LOG_Debug("cgreg : %02x", retValue);
        */

        //模组拨号状态 1: 拨号成功，0:拨号不成功 拨号次数
        retValue = GetModemDialStatus();
        if(retValue == 0)
        {
            g_SimCardInfoToMcu.dialStatus = 1;
            g_SimCardInfoToMcu.modemSta |= 0x20;
        }
        else
        {
            g_SimCardInfoToMcu.dialStatus = 0;
        }
        LOG_Debug("g_SimCardInfoToMcu.dialStatus : %d", g_SimCardInfoToMcu.dialStatus);

        retValue = SrGetInterfaceIp("bridge1", ipaddrBuff, 16);
        if(0 == retValue && 0 == strncmp((char *)ipaddrBuff, "192.168.1.1", strlen("192.168.1.1")))
        {
            g_SimCardInfoToMcu.modemSta |= 0x40;
            LOG_Debug("SrGetInterfaceIp : %02x", g_SimCardInfoToMcu.modemSta);
        }
        retValue = GeUpgradeStatusFromMasterOrSlave();
        if(0 == retValue)
        {
            g_SimCardInfoToMcu.modemSta |= 0x80;
            LOG_Debug("GeUpgradeStatusFromMasterOrSlave : %02x", g_SimCardInfoToMcu.modemSta);
        }

        //信号强度 信噪比 拨号类型
        GetModemSignalQualityAndDialType(&g_SimCardInfoToMcu.signalSsi, &g_SimCardInfoToMcu.signalSnr, &g_SimCardInfoToMcu.modemDialType);

        //流量消耗单位MB
        flowValue = GetNetInterfaceBytes(APN1_INTERFACE);
        g_SimCardInfoToMcu.flow[0] = flowValue & 0xFF;
        g_SimCardInfoToMcu.flow[1] = ((flowValue >> 8) & 0xFF);
        g_SimCardInfoToMcu.flow[2] = ((flowValue >> 16) & 0xFF);
        g_SimCardInfoToMcu.flow[3] = ((flowValue >> 24) & 0xFF);

        // 重启次数
        retValue = GetFileLineCount("/oemdata/log/bootup.txt");
        g_SimCardInfoToMcu.modemResetNum[0] = retValue & 0xFF;
        g_SimCardInfoToMcu.modemResetNum[1] = ((retValue >> 8) & 0xFF);
        g_SimCardInfoToMcu.modemResetNum[2] = ((retValue >> 16) & 0xFF);
        g_SimCardInfoToMcu.modemResetNum[3] = ((retValue >> 24) & 0xFF);

        //拨号次数
        // g_SimCardInfoToMcu.modemDialNum[0] = retValue & 0xFF;
        // g_SimCardInfoToMcu.modemDialNum[1] = ((retValue >> 8) & 0xFF);
        // g_SimCardInfoToMcu.modemDialNum[2] = ((retValue >> 16) & 0xFF);
        // g_SimCardInfoToMcu.modemDialNum[3] = ((retValue >> 24) & 0xFF);

        //高质量次数
        if(modemHQnum == g_SimCardInfoToMcu.signalSsi)
        {
            modemHQnumCount++;
        }
        else
        {
            modemHQnum = g_SimCardInfoToMcu.signalSsi;
            modemHQnumCount = 0;
        }
        g_SimCardInfoToMcu.modemHQnum[0] = modemHQnumCount & 0xFF;
        g_SimCardInfoToMcu.modemHQnum[1] = ((modemHQnumCount >> 8) & 0xFF);
        g_SimCardInfoToMcu.modemHQnum[2] = ((modemHQnumCount >> 16) & 0xFF);
        g_SimCardInfoToMcu.modemHQnum[3] = ((modemHQnumCount >> 24) & 0xFF);

        //模组版本号
        GetModemVersion(modemVersion, 7);
        memcpy(g_SimCardInfoToMcu.modemVer, modemVersion, 7);
        

        //模组上网状态(判断MQTT 发送状态) 1：网络通， 2：网络不通 
        if(MqttHandle::getInstance().client_handle.is_connected)
        {
            g_SimCardInfoToMcu.pingStatus = 1;
        }
        else
        {
            g_SimCardInfoToMcu.pingStatus = 0;
        }
        LOG_Debug("g_SimCardInfoToMcu.pingStatus : %d", g_SimCardInfoToMcu.pingStatus);

        retValue = HalModemNwGetRegStatusInfo(&regStaInfo);
        if(retValue == RET_OK)
        {
            memcpy(g_SimCardInfoToMcu.mcc, regStaInfo.mcc, 4);
            memcpy(g_SimCardInfoToMcu.mnc, regStaInfo.mnc, 4);
            g_SimCardInfoToMcu.cid[0] = (regStaInfo.cid >> 24) & 0xFF;
            g_SimCardInfoToMcu.cid[1] = (regStaInfo.cid >> 16) & 0xFF;
            g_SimCardInfoToMcu.cid[2] = (regStaInfo.cid >> 8) & 0xFF;
            g_SimCardInfoToMcu.cid[3] = (regStaInfo.cid >> 0) & 0xFF;
            g_SimCardInfoToMcu.lac[0] = (regStaInfo.lac >> 8) & 0xFF;
            g_SimCardInfoToMcu.lac[1] = (regStaInfo.lac & 0xFF);
            g_SimCardInfoToMcu.pcid5g[0] = (regStaInfo.pcid5g >> 8) & 0xFF;
            g_SimCardInfoToMcu.pcid5g[1] = (regStaInfo.pcid5g & 0xFF);
            g_SimCardInfoToMcu.cid5g[0] = (regStaInfo.cid5g >> 56) & 0xFF;
            g_SimCardInfoToMcu.cid5g[1] = (regStaInfo.cid5g >> 48) & 0xFF;
            g_SimCardInfoToMcu.cid5g[2] = (regStaInfo.cid5g >> 40) & 0xFF;
            g_SimCardInfoToMcu.cid5g[3] = (regStaInfo.cid5g >> 32) & 0xFF;
            g_SimCardInfoToMcu.cid5g[4] = (regStaInfo.cid5g >> 24) & 0xFF;
            g_SimCardInfoToMcu.cid5g[5] = (regStaInfo.cid5g >> 16) & 0xFF;
            g_SimCardInfoToMcu.cid5g[6] = (regStaInfo.cid5g >> 8) & 0xFF;
            g_SimCardInfoToMcu.cid5g[7] = (regStaInfo.cid5g >> 0) & 0xFF;

        }

        GetGnssLocation(&g_gnssInfo);

        pthread_mutex_unlock(&g_simCarInfoLock);

        // memcpy(sendBuff, &g_SimCardInfoToMcu, sizeof(SimCardInfoToMcu_t));
        // sendToMcu.type = (SYSQueMsgType_e)(SYS_QUE_TYPE_MCU_VEHICLE_CTROL_SIMCARD_INFO_TO_MCU);
        // sendToMcu.payloadLen = sizeof(SimCardInfoToMcu_t);
        // sendToMcu.payload = sendBuff;

        // LOG_Debug("sendToMcu.type : %8x", sendToMcu.type);
        // McuSrvSendMesg(&sendToMcu);

        
        

        sleep(1);
    }

    return NULL;
}

