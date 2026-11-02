#include <glib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/statvfs.h>

#include "err_code.h"
#include "sr_log.h"
#include "fty_info.h"
#include "ecall_server_api.h"
#include "net_service_api.h"
#include "mcu_api.h"
#include "queue_msg.h"
#include "gnss_ipc_msg.h"
#include "fty_control.h"

#define FTY_INFO_LEGTH (3)

// 定义一个结构体 FtyInfo_t，用于存储不同类型的信息
typedef struct
{
    uint8_t csq;
    uint8_t netType;
    uint8_t providerName[128];
    uint8_t btName[128];
    uint8_t ecallStatus;
    uint8_t apnNum;
    uint32_t flashSpace;
    int8_t gnssInfo[10];
    uint32_t apnTestResult;
    uint32_t wifiTestResult;
    uint32_t btTestResult;
    uint32_t callTestResult;
    uint8_t netRegStatus;
} FtyInfo_t;

static FtyInfo_t g_ftyInfo = {0};

// 定义处理函数原型
typedef void (*CommandHandler)(const uint8_t* data, uint16_t len);

// 定义结构体，包含命令ID和对应的处理函数指针
typedef struct {
    CommandId_e commandId;
    CommandHandler handler;
} CommandEntry_t;

// 定义命令处理函数的具体实现
void HandleGetFlashInfo(const uint8_t *data, const uint16_t len) {
    LOG_Debug(" >>> get flash info ");
    FILE *fp;
    char buffer[1024];
    char command[] = "df -k";

    fp = popen(command, "r");
    if (fp == NULL) {
        LOG_Error("Failed to run df -k");
        return;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        char *mountPoint = strstr(buffer, "/mnt/sdcard");
        if (mountPoint != NULL) {
            char *token = strtok(buffer, " ");
            int count = 0;
            long availableSpaceKb = 0;
            while (token != NULL) {
                if (count == 3) {
                    // 第四个字段是可用空间（以 KB 为单位）
                    availableSpaceKb = atol(token);
                    g_ftyInfo.flashSpace = availableSpaceKb;
                    break;
                }
                token = strtok(NULL, " ");
                count++;
            }
            LOG_Debug("Available space on flash: %ld KB", availableSpaceKb);
            break;
        }
    }
   
    pclose(fp);
    uint8_t cpuStatus[4] = {
        (uint8_t)(g_ftyInfo.flashSpace >> 24),
        (uint8_t)(g_ftyInfo.flashSpace >> 16),
        (uint8_t)(g_ftyInfo.flashSpace >> 8),
        (uint8_t)(g_ftyInfo.flashSpace)
    };
    SendFtyInfoAck(OperationResultSuccess, E_GET_FLASH_STORAGE_SPACE, sizeof(cpuStatus), cpuStatus);
}

void HandleGetCsq(const uint8_t *data, const uint16_t len) {
    LOG_Debug(" >>> get csq ");
    uint8_t cpuStatus = g_ftyInfo.csq;
    LOG_Debug("> csq:%d.",cpuStatus);
    SendFtyInfoAck(OperationResultSuccess, E_GET_4G_MODULE_SIGNAL_STRENGTH, sizeof(cpuStatus), &cpuStatus);
}

void HandleGetEcallStatus(const uint8_t *data, const uint16_t len) {
    LOG_Debug(" >>> get ecall status ");
    EnuEcallStatus enuEcallStatus;
    GetEcallStatusInfo(&enuEcallStatus, 0);
    LOG_Debug(" >>> ecall status ：%d.", enuEcallStatus);
    g_ftyInfo.ecallStatus = enuEcallStatus;
    if (enuEcallStatus != ECALL_API_STATUS_INIT) {
        g_ftyInfo.ecallStatus = 1;
    }
    uint8_t cpuStatus = g_ftyInfo.ecallStatus;
    SendFtyInfoAck(OperationResultSuccess, E_RESERVED_START, sizeof(cpuStatus), &cpuStatus);
}

void HandleGetNetType(const uint8_t *data, const uint16_t len) {
    LOG_Debug(" >>> get net type  ");
    uint8_t cpuStatus = g_ftyInfo.netType;
    LOG_Debug("> net type:%d.",cpuStatus);
    SendFtyInfoAck(OperationResultSuccess, E_GET_NETWORK_TYPE, sizeof(cpuStatus), &cpuStatus);
}

void HandleGetApnCount(const uint8_t *data, const uint16_t len) {
    LOG_Debug(" >>> get apn count  ");
    StrNetStatus_t netStatus = {0};
    GetNetStatusInfo(&netStatus, 50);
    SetApnNumber(netStatus.apnNum);
    uint8_t cpuStatus = g_ftyInfo.apnNum;
    LOG_Debug("> apn count:%d.",cpuStatus);
    SendFtyInfoAck(OperationResultSuccess, E_NETWORK_APN_COUNT, sizeof(cpuStatus), &cpuStatus);
}

void HandleGetNetProvider(const uint8_t *data, const uint16_t len) {
    LOG_Debug(" >>> get net provider info ");
    LOG_Debug("> provider info:%s.",g_ftyInfo.providerName);
    SendFtyInfoAck(OperationResultSuccess, E_NETWORK_PROVIDER, ((uint8_t)strlen((char *)g_ftyInfo.providerName) + 1), (uint8_t *)g_ftyInfo.providerName);
}

void HandleGetGnssInfo(const uint8_t *data, const uint16_t len) {
    LOG_Debug(" >>> get gnss info ");
    SendFtyInfoAck(OperationResultSuccess, E_GET_GNSS_INFO, sizeof(g_ftyInfo.gnssInfo), (uint8_t *)g_ftyInfo.gnssInfo);
}

void HandleGetApnTestResult(const uint8_t *data, const uint16_t len) {
    LOG_Debug(" >>> get apn test result  ");
    LOG_Debug("> test result:%d.",g_ftyInfo.apnTestResult);
    uint8_t cpuStatus[3] = {
        (uint8_t)(g_ftyInfo.apnTestResult & 0xff),
        (uint8_t)((g_ftyInfo.apnTestResult >> 8) & 0xff),
        (uint8_t)((g_ftyInfo.apnTestResult >> 16) & 0xff)
    };
    SendFtyInfoAck(OperationResultSuccess, E_APN_IP_PING_RESULT, sizeof(cpuStatus), cpuStatus);
}

void HandleGetWifiTestResult(const uint8_t *data, const uint16_t len) {
    LOG_Debug(" >>> get wifi test result ");
    LOG_Debug("> wifi test result:%d.",g_ftyInfo.wifiTestResult);
    uint8_t cpuStatus[2] = {
        (uint8_t)(g_ftyInfo.wifiTestResult & 0xff),
        (uint8_t)((g_ftyInfo.wifiTestResult >> 8) & 0xff)
    };
    SendFtyInfoAck(OperationResultSuccess, E_WIFI_CONNECT_TEST_RESULT, sizeof(cpuStatus), cpuStatus);
}

void HandleGetBtTestResult(const uint8_t *data, const uint16_t len) {
    LOG_Debug(" >>> get bt test result ");
    LOG_Debug("> bt test result:%d.",g_ftyInfo.btTestResult);
    uint8_t cpuStatus[2] = {
        (uint8_t)(g_ftyInfo.btTestResult & 0xff),
        (uint8_t)((g_ftyInfo.btTestResult >> 8) & 0xff)
    };
    SendFtyInfoAck(OperationResultSuccess, E_BLUETOOTH_CONNECT_TEST_QUERY, sizeof(cpuStatus), cpuStatus);
}

void HandleGetCallTestResult(const uint8_t *data, const uint16_t len) {
    LOG_Debug(" >>> get call test result ");
    LOG_Debug("> call test result:%d.",g_ftyInfo.callTestResult);
    uint8_t cpuStatus[2] = {
        (uint8_t)(g_ftyInfo.callTestResult & 0xff),
        (uint8_t)((g_ftyInfo.callTestResult >> 8) & 0xff)
    };
    SendFtyInfoAck(OperationResultSuccess, E_DIAL_TEST_RESULT_QUERY, sizeof(cpuStatus), cpuStatus);
}

void HandleGetBtName(const uint8_t *data, const uint16_t len) {
    LOG_Debug(" >>> get bt name ");
    LOG_Debug("> bt name:%s.",g_ftyInfo.btName);
    uint8_t cpuStatusLen = strlen((const char *)g_ftyInfo.btName);
    SendFtyInfoAck(OperationResultSuccess, E_GET_BLUETOOTH_NAME, cpuStatusLen, (uint8_t *)g_ftyInfo.btName);
}
void HandleSetBluetoothName(const uint8_t *data, const uint16_t len)
{
    LOG_Debug(" >>> set bt name ");
    memset(g_ftyInfo.btName,0,sizeof(g_ftyInfo.btName));
    if(len > 32)
    {
        LOG_Error("%s : data len from mcu more than 32,error.",__FUNCTION__);        
        SendFtyInfoAck(OperationResultFailure, E_APN_PING_START,0 , NULL);
        return;
    }
    memcpy(g_ftyInfo.btName,data,len);
   //SetBtName();
    LOG_Debug("> set name:%s.",g_ftyInfo.btName);
    SendFtyInfoAck(OperationResultSuccess, E_SET_BLUETOOTH_NAME, 0, NULL);
}
void HandleSpeakerTest(const uint8_t *data, const uint16_t len)
{
    LOG_Debug(" >>> speaker test ");
    // not apply
    SendFtyInfoAck(OperationResultSuccess, E_SPEAKER_TEST, 0, NULL);
}

// 定义处理函数数组
CommandEntry_t commandHandlers[] = {
    {E_TEST_MODE, HandleTestMode},
    {E_APN_PING_START, HandleApnPing},
    {E_APN_IP_PING_RESULT, HandleGetApnTestResult},
    {E_WIFI_CONNECT_SPECIFIC_NETWORK, HandleWifi},
    {E_WIFI_CONNECT_TEST_RESULT, HandleGetWifiTestResult},
    {E_BLUETOOTH_CONNECT_COMMAND, HandleBTConnect},
    {E_BLUETOOTH_CONNECT_TEST_QUERY, HandleGetBtTestResult},
    {E_DIAL_TEST_COMMAND, HandleCallDail},
    {E_DIAL_TEST_RESULT_QUERY, HandleGetCallTestResult},
    {E_SET_BLUETOOTH_NAME, HandleSetBluetoothName},
    {E_GET_BLUETOOTH_NAME, HandleGetBtName},
    {E_SPEAKER_TEST, HandleSpeakerTest},
    {E_GET_4G_MODULE_SIGNAL_STRENGTH, HandleGetCsq},
    {E_GET_NETWORK_TYPE, HandleGetNetType},
    {E_NETWORK_APN_COUNT, HandleGetApnCount},
    {E_NETWORK_PROVIDER, HandleGetNetProvider},
    {E_GET_GNSS_INFO, HandleGetGnssInfo},
    {E_GET_FLASH_STORAGE_SPACE, HandleGetFlashInfo}
};
    
int32_t SendFtyInfoAck(uint8_t status, uint8_t commandId, uint8_t comResultLen, uint8_t *comResultValue)
{   
    McuMesg_t *McuMsg = NULL;

    // 计算payload所需的总长度
    size_t payloadSize = sizeof(uint8_t) * (3 + comResultLen); // 3个字节用于status、commandId、comResultLen
    
    // 分配足够的内存
    McuMsg = (McuMesg_t *)calloc(1, sizeof(McuMesg_t) + payloadSize);
    if(NULL == McuMsg)
    {
        return RET_ERR;
    }
    
    McuMsg->type = SYS_QUE_TYPE_MCU_SYNC_FTY_RES;
    McuMsg->payloadLen = payloadSize;

    // 初始化payload指针
    McuMsg->payload = ((uint8_t*)McuMsg) + sizeof(McuMesg_t);

    // 写入数据
    McuMsg->payload[0] = status;
    McuMsg->payload[1] = commandId; 
    McuMsg->payload[2] = comResultLen; 
    if (comResultLen > 0 && comResultValue != NULL) {
        memcpy(&McuMsg->payload[3], comResultValue, comResultLen);
    }
    
    if(RET_ERR == McuSrvSendMesg(McuMsg))
    {
        LOG_Error(" %s:SendFtyInfoAck failed!", __func__);
        free(McuMsg);
        return RET_ERR;
    }
    
    LOG_Debug(" %s:SendFtyInfoAck success!", __func__);
    free(McuMsg);
    return RET_OK;
}

static int32_t SendFtyPeriodStatusInfo(char *status, uint8_t statusLen)
{   
    McuMesg_t McuMsg = {0};

    if(NULL == status || 0 == statusLen)
    {
        return RET_OK;
    }
    
    McuMsg.payload = (uint8_t *)calloc(1, statusLen);
    if(NULL == McuMsg.payload)
    {
        return RET_ERR;
    }
    
    McuMsg.type = SYS_QUE_TYPE_MCU_SYNC_FTY_PERIOD_STATUS_INFO;
    McuMsg.payloadLen = statusLen;
    memcpy(McuMsg.payload, status, statusLen);
    if(RET_ERR == McuSrvSendMesg(&McuMsg))
    {
        LOG_Error(" %s:SendFtyInfoAck failed!", __func__);
        free(McuMsg.payload);
        return RET_ERR;
    }
    
    LOG_Debug(" %s:SendFtyInfoAck success!", __func__);
    free(McuMsg.payload);
    return RET_OK;
}

int GetAlarmInfomation()
{
    gint fileFd = -1;
    gchar alarm[4] = {0};

    fileFd = open("/usrdata/status/alarm",O_RDONLY);
    if(-1 == fileFd)
    {
        LOG_Debug("open .speed failed");
        return 0;
    }
    else
    {
        if(read(fileFd,alarm,1) > 0)
        {
            close(fileFd);
            return atoi(alarm);
        }
        else
        {
            close(fileFd);
            return 0;
        }
    }
}

static void SetGnss(GnssLocation_t *gnssLocation)  
{
    //LOG_Debug("SetGnss, status:%d, linkNum:%d", gnssLocation->status, gnssLocation->moonLinkNum);
    int32_t longtitudeValue = gnssLocation->flongitude;
    int32_t latitudeValue = gnssLocation->flatitude;
    
    g_ftyInfo.gnssInfo[0] = gnssLocation->gpsState;
    g_ftyInfo.gnssInfo[1] = gnssLocation->moonLinkNum;
    g_ftyInfo.gnssInfo[2] = longtitudeValue >> 24;
    g_ftyInfo.gnssInfo[3] = longtitudeValue >> 16;
    g_ftyInfo.gnssInfo[4] = longtitudeValue >> 8;
    g_ftyInfo.gnssInfo[5] = longtitudeValue;

    g_ftyInfo.gnssInfo[6] = latitudeValue >> 24;
    g_ftyInfo.gnssInfo[7] = latitudeValue >> 16;
    g_ftyInfo.gnssInfo[8] = latitudeValue >> 8;
    g_ftyInfo.gnssInfo[9] = latitudeValue;
}
void SetCsq(guint value)
{
    LOG_Debug("SetCsq, value:%d", value);
    g_ftyInfo.csq = value;
}

void SetEcalStatus(guint value)
{
    g_ftyInfo.ecallStatus = value;
}

void SetNetType(guint value)
{
    g_ftyInfo.netType = value;
}
void SetNetRegStatus(uint8_t value)
{
    g_ftyInfo.netRegStatus = value;
}
void  SetProviderName(gchar value[])
{
    LOG_Debug("SetProviderName, value:%s", value);
    memset(g_ftyInfo.providerName,0,sizeof(g_ftyInfo.providerName));
    g_stpcpy((gchar *)g_ftyInfo.providerName, value);
}

void SetFlashSpace(guint space)
{
    g_ftyInfo.flashSpace = space;
}

void SetApnNumber(uint8_t value)
{
    g_ftyInfo.apnNum = value;
}

void SetApnTestResult(guint value)
{
    g_ftyInfo.apnTestResult = value;
}

void SetWifiTestResult(guint value)
{
    g_ftyInfo.wifiTestResult = value;
}

void SetBtTestResult(guint value)
{
    g_ftyInfo.btTestResult = value;
}

void SetCallTestResult(guint value)
{
    g_ftyInfo.callTestResult = value;
}

void FtyGnssMsgSubCallBack(const uint8_t* data, const uint16_t len, void* userdata)
{
    GnssLocation_t gnssLocation;
    
    memset(&gnssLocation, 0x00, sizeof(GnssLocation_t));
    if(sizeof(GnssLocation_t) == len)
    {
        memmove(&gnssLocation, data, len);
        SetGnss(&gnssLocation);
    }
    else
    {
        LOG_Debug("FtyGnssMsgSubCallBack len error!");
    }
}


/*
* @brief send information to mcu 
*
* @param pArg
*
* @return 
*/
gpointer SendInfoToMCU(gpointer pArg)
{
    char data[18];
    int tmpInt =  0;
    unsigned char degree = 0;
    unsigned char minute = 0;
    unsigned short  second = 0;
    int alarm = 0;
    while( 1 )
    {
        sleep(1);
        /*gnss location status */
        data[0] = ((g_ftyInfo.gnssInfo[0] & 0x3f) == 0 ? 1 : 0 );
        
        /*latitude*/
        tmpInt = g_ftyInfo.gnssInfo[2] << 24;
        tmpInt += g_ftyInfo.gnssInfo[3] << 16;
        tmpInt += g_ftyInfo.gnssInfo[4] << 8;
        tmpInt += g_ftyInfo.gnssInfo[5];

        /*charge north latitue or south latitude */
        if(tmpInt > 0)
        {
            /*0x4E ascii N*/
            data[5] = 0x4E;
        }
        else
        {
            /*0x53 ascii S*/
            data[5] = 0x53;
            tmpInt = abs(tmpInt);
        }
        degree = tmpInt/1000000;
        minute = (tmpInt/1000000.0 - degree ) * 60;
        second = ((tmpInt/1000000.0 - degree ) * 60 - minute)*6000;

        data[1] = degree;
        data[2] = minute;
        data[3] = second >> 8;
        data[4] = second;

        /*longitude*/
        tmpInt = 0;
        tmpInt = g_ftyInfo.gnssInfo[6] << 24;
        tmpInt += g_ftyInfo.gnssInfo[7] << 16;
        tmpInt += g_ftyInfo.gnssInfo[8] << 8;
        tmpInt += g_ftyInfo.gnssInfo[9];
        
        if(tmpInt > 0)
        {
            /*0x45 ascii E*/
            data[10] = 0x45;
        }
        else
        {
            /*0x57 ascii W*/
            data[10] = 0x57;
            tmpInt = abs(tmpInt);
        }

        degree = tmpInt/1000000;
        minute = (tmpInt/1000000.0 - degree) * 60;
        second = ((tmpInt/1000000.0 - degree) * 60 - minute)*6000;

        data[6] = degree;
        data[7] = minute;
        data[8] = second >> 8;
        data[9] = second;
        

        /*get the status with platform*/
        data[11] = g_ftyInfo.netRegStatus;
        alarm = GetAlarmInfomation(); //根据之后得协议定
        data[12] = alarm;
        
        data[17] =g_ftyInfo.csq;
        SendFtyPeriodStatusInfo(data,18);
    }
}
void ProcessRequest(const uint8_t* data, const uint16_t len,void* userdata)
{
    uint8_t commandId = data[0]; // 命令ID位于数据的第一个字节
    uint8_t comParaLen = data[1]; // 参数长度位于数据的第二个字节

    uint8_t comParaValueOffset = 2; // 如果有参数，数据中参数的起始位置为第三个字节

    // 遍历命令处理函数结构体数组，找到与命令ID匹配的处理函数并调用
    for (size_t i = 0; i < sizeof(commandHandlers) / sizeof(commandHandlers[0]); ++i) {
        if (commandHandlers[i].commandId == commandId) {
            commandHandlers[i].handler(data + comParaValueOffset, comParaLen);
            break;
        }
    }
}


