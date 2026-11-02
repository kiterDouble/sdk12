#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "ecall_nad_manager.h"
#include "ecall_server_api.h"
#include "ecall_gps.h"
#include "ECallMessage.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "err_code.h"
#include "ecall_gps.h"
#include "hal_modem_ecall.h"
#include "ecall_timer.h"
#include "ecall_mcu_manager.h"
#include "pm_server_api.h"
#include "modem_api.h"
#include "sr_param.h"

#define VIN_LENGTH (17)
#define ECALL_MSD_MAX_LEN (140)
#define ECALL_MSG_SUBCMD (0x8007)
#define ABNORMAL_TIMES_INTERVAL (120*1000) //2min

#define CALL_NUM_LENGTH (20)
#define TEST_CALL_OPEN (1)
#define TEST_CALL_CLOSE (0)

char TEST_CALL_NUM [ECALL_PHONE_NUM_LEN] = {0};
static int8_t TEST_CALL_MODE = TEST_CALL_CLOSE;
static EcallConfigMode_e ECALL_NET_MODE = E_VOICE_ECALL_MODE_NOMAL;

static EnuEcallStatus g_EnuEcallStatus = 0;
static timer_t abnormalTimerId = {0};
static uint8_t MSDIdentifier = 1;

typedef struct{
    uint8_t ecallTriggerModel;  //激活模式 1-自动激活 0-手动激活
    uint8_t ecallType;          //呼叫模式 1-测试呼叫 0-紧急呼叫
    StrGPSInfo gpsInfo;         //gps相关信息
    char VIN[VIN_LENGTH + 1];       //vin码
}StrMsdInfo;

// 定义配置项结构体
typedef struct {
    uint32_t paramId;
    const char *key;
    uint16_t *value;
} ConfigMapping_t;

/**
* @brief ECALL ONLY 下的飞行模式设置
*
* @param[in] 无
* @return 无
*/
static void EcallNetAirPlaneModeConfig(void)
{
    if(E_VOICE_ECALL_MODE_ONLY == ECALL_NET_MODE)
    {
        if(RET_OK == ModemSrvSetAirplaneMode(E_MODEM_SRV_DMS_AIRPLANE_ON))
        {
            LOG_Info("%s:  ModemSrvSetAirplaneMode is success,nad into AIRPLANE MODE .",__FUNCTION__);
        }
        else
        {
            LOG_Error("%s:  ModemSrvSetAirplaneMode is faild.",__FUNCTION__);
        }
    }
}
/**
* @brief 取消飞行模式
*
* @param[in] 无
* @return 无
*/
static void EcallNetAirPlaneModeDeconfig(void)
{
    if(E_VOICE_ECALL_MODE_ONLY == ECALL_NET_MODE)
    {
        if(RET_OK == ModemSrvSetAirplaneMode(E_MODEM_SRV_DMS_AIRPLANE_OFF))
        {
            LOG_Info("%s:  ModemSrvSetAirplaneMode is success,nad into NORMAL MODE .",__FUNCTION__);
        }
        else
        {
            LOG_Error("%s:  ModemSrvSetAirplaneMode is faild.",__FUNCTION__);
        }
    }
}

/**
* @brief 更新MSD消息
*
* @param[in] uperMsd 存储MSD消息的缓冲区
* @param[in] msdLen 输入的缓冲区长度
* @param[in] msdInfo 存储GPS信息的结构体
* @return 返回编码后的MSD消息长度
*/
static int32_t UpdataMsd(char *uperMsd,uint8_t msdLen,StrMsdInfo *msdInfo,ecallMsg *msd)
{
    
 //   ecallMsg *msd;
    char isowmi[8]={"ECA"};
    char isovds[8]={"LLEXAM"};
    char isovisModelyear[8]={"P"};
    char isovisSeqPlant[8]={"LE02013"};
    int hour,min;
    double sec;
    int len = 0;
    double min_tmp;
    double longtitude_value,latitude_value,direction;
    double locationDiff = 0;
    
 //   msd = (ecallMsg*)calloc(1, sizeof(ecallMsg));

    //msgcount 消息计数
    msd->messageIdentifier = MSDIdentifier;
    LOG_Info("msdtest messageIdentifier : %d ", msd->messageIdentifier);
    //ControlType
    //激活模式 1-自动激活 0-手动激活
    msd->control.automaticActivation = msdInfo->ecallTriggerModel;
    LOG_Info("msdtest automaticActivation : %d ",msdInfo->ecallTriggerModel);
    //呼叫模式 1-测试呼叫 0-紧急呼叫
    LOG_Info("msdtest eCallCmd->eCall_test_call : %d ",msdInfo->ecallType);
    msd->control.testCall = msdInfo->ecallType;

    //位置可信任度 1-可信度高 0-可信度低
    if(msdInfo->gpsInfo.status != 0)
        msd->control.positionCanBeTrusted = 1;
    else
        msd->control.positionCanBeTrusted = 0;
    LOG_Info("msdtest positionCanBeTrusted : %d ", msd->control.positionCanBeTrusted);
    //车型 固定为M1
    msd->control.vehType = EU_MSD_VEHICLE_COMMERCIAL_N1;
    LOG_Info("msdtest vehType : %d ", msd->control.vehType);

    //VIN 
    if(strlen(msdInfo->VIN) == 17)
    {
        memcpy(msd->VIN.isowmi,&msdInfo->VIN[0],3);
        memcpy(msd->VIN.isovds,&msdInfo->VIN[3],6);
        memcpy(msd->VIN.isovisModelyear,&msdInfo->VIN[9],1);
        memcpy(msd->VIN.isovisSeqPlant,&msdInfo->VIN[10],7);
    }
    else
    {
        memcpy(msd->VIN.isowmi,isowmi,3);
        memcpy(msd->VIN.isovds,isovds,6);
        memcpy(msd->VIN.isovisModelyear,isovisModelyear,1);
        memcpy(msd->VIN.isovisSeqPlant,isovisSeqPlant,7);
    }
    LOG_Info("msdtest VIN: %s ", msdInfo->VIN);

    // VehiclePropulsionStorageType 固定为储能电池
    msd->vehPropulsionStorageType.gasolineTankPresent = 0;
    msd->vehPropulsionStorageType.dieselTankPresent = 0;
    msd->vehPropulsionStorageType.compressedNaturalGas = 0;
    msd->vehPropulsionStorageType.liquidPropaneGas = 0;
    msd->vehPropulsionStorageType.electricEnergyStorage = 1;
    msd->vehPropulsionStorageType.hydrogenStorage = 0;
    msd->vehPropulsionStorageType.otherStorage = 0;
    LOG_Info("msdtest gasolineTankPresent: %d ", msd->vehPropulsionStorageType.gasolineTankPresent);
    LOG_Info("msdtest dieselTankPresent: %d ", msd->vehPropulsionStorageType.dieselTankPresent);
    LOG_Info("msdtest compressedNaturalGas: %d ", msd->vehPropulsionStorageType.compressedNaturalGas);
    LOG_Info("msdtest liquidPropaneGas: %d ", msd->vehPropulsionStorageType.liquidPropaneGas);
    LOG_Info("msdtest electricEnergyStorage: %d ", msd->vehPropulsionStorageType.electricEnergyStorage);
    LOG_Info("msdtest hydrogenStorage: %d ", msd->vehPropulsionStorageType.hydrogenStorage);
    LOG_Info("msdtest otherStorage: %d ", msd->vehPropulsionStorageType.otherStorage);
    //TIME 时间点
    msd->timestamp = msdInfo->gpsInfo.timestamp;
    LOG_Info("msdtest timestamp: %d ", msdInfo->gpsInfo.timestamp);

    //GPS数据获取
    longtitude_value = msdInfo->gpsInfo.longtitude_value;
    latitude_value = msdInfo->gpsInfo.latitude_value;
    direction = msdInfo->gpsInfo.direction;
    LOG_Info("msdtest GNSS_INFO: longtitude_value :%f  |latitude_value :%f ",longtitude_value,latitude_value);
    hour = (longtitude_value);
    min_tmp = (longtitude_value) - hour;
    min = (min_tmp*60);
    sec = ((min_tmp*60) - (double)min)*60;

    //经度
    msd->position.longitude = (hour*60*60+min*60+sec)*1000;

    hour = (latitude_value);
    min_tmp = (latitude_value) - hour;
    min = (min_tmp*60);
    sec = ((min_tmp*60) - (double)min)*60 ;
    //纬度
    msd->position.latitude = (hour*60*60+min*60+sec)*1000;
    LOG_Info("msdtest GNSS_MSD_INFO: longtitude_value :%d  |latitude_value :%d ",msd->position.longitude,msd->position.latitude);
    //车辆方向
    msd->vehicleDirection = direction/20; //以2度为单位
    LOG_Info("msdtest : msd->vehicleDirection: %d, %f", msd->vehicleDirection, direction);

    locationDiff = msdInfo->gpsInfo.latitude_value - msdInfo->gpsInfo.latitude_value_N1;
    hour = (locationDiff);
    min_tmp = (locationDiff) - hour;
    min = (min_tmp*60);
    sec = ((min_tmp*60) - (double)min)*60;
    msd->recentVehicleLocationN1.latitudeDelta  = (sec * 10);
    if(msd->recentVehicleLocationN1.latitudeDelta > 511)
    {
        msd->recentVehicleLocationN1.latitudeDelta = 511;
    }
    else if(msd->recentVehicleLocationN1.latitudeDelta < -512)
    {
        msd->recentVehicleLocationN1.latitudeDelta = -512;
    }
    
    locationDiff = msdInfo->gpsInfo.longtitude_value - msdInfo->gpsInfo.longtitude_value_N1;
    hour = (locationDiff);
    min_tmp = (locationDiff) - hour;
    min = (min_tmp*60);
    sec = ((min_tmp*60) - (double)min)*60;
    msd->recentVehicleLocationN1.longitudeDelta  = (sec * 10);
    if(msd->recentVehicleLocationN1.longitudeDelta > 511)
    {
        msd->recentVehicleLocationN1.longitudeDelta = 511;
    }
    else if(msd->recentVehicleLocationN1.longitudeDelta < -512)
    {
        msd->recentVehicleLocationN1.longitudeDelta = -512;
    }
    
    locationDiff = msdInfo->gpsInfo.latitude_value_N1 - msdInfo->gpsInfo.latitude_value_N2;
    hour = (locationDiff);
    min_tmp = (locationDiff) - hour;
    min = (min_tmp*60);
    sec = ((min_tmp*60) - (double)min)*60;
    msd->recentVehicleLocationN2.latitudeDelta  = (sec * 10);
    if(locationDiff > 511)
    {
        msd->recentVehicleLocationN2.latitudeDelta = 511;
    }
    else if(locationDiff < -512)
    {
        msd->recentVehicleLocationN2.latitudeDelta = -512;
    }
    
    locationDiff = msdInfo->gpsInfo.longtitude_value_N1 - msdInfo->gpsInfo.longtitude_value_N2;
    hour = (locationDiff);
    min_tmp = (locationDiff) - hour;
    min = (min_tmp*60);
    sec = ((min_tmp*60) - (double)min)*60;
    msd->recentVehicleLocationN2.longitudeDelta  = (sec * 10);
    if(locationDiff > 511)
    {
        msd->recentVehicleLocationN2.longitudeDelta = 511;
    }
    else if(locationDiff < -512)
    {
        msd->recentVehicleLocationN2.longitudeDelta = -512;
    }

    msd->numberOfPassengers = 255;
    LOG_Info("msdtest vehicleDirection: %d ", msd->vehicleDirection);
    LOG_Info("msdtest recentVehicleLocationN1.latitudeDelta: %d ", msd->recentVehicleLocationN1.latitudeDelta);
    LOG_Info("msdtest recentVehicleLocationN1.longitudeDelta: %d ", msd->recentVehicleLocationN1.longitudeDelta);
    LOG_Info("msdtest recentVehicleLocationN2.latitudeDelta: %d ", msd->recentVehicleLocationN2.latitudeDelta);
    LOG_Info("msdtest recentVehicleLocationN2.longitudeDelta: %d ", msd->recentVehicleLocationN2.longitudeDelta);
    LOG_Info("msdtest numberOfPassengers: %d ", msd->numberOfPassengers);
    msdLen = Encode_Msg(msd,uperMsd,len);
    LOG_Info("msdtest msd_len=[%d] ",msdLen);
    return msdLen;
}
/**
* @brief 将事件控制命令发送到ECALL SERVER处理线程
*
* @param[in] ecallEventControlCmd 存储事件控制命令的结构体
* @return 返回发送消息的ID
*/
int32_t SendEventToEcallServer(StrEcallEventControlCmd_t *ecallEventControlCmd)
{
    int ret = 0;
    int32_t msgID = 0;
    const char *filepath = "/tmp/ecallKey";
    StrEcallEventControlCmd_t localEcallEventControlCmd = {0};
    memcpy(&localEcallEventControlCmd,ecallEventControlCmd,sizeof(StrEcallEventControlCmd_t));

    int fd = open(filepath, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        LOG_Error("%s : open error : %s ",__func__,strerror(errno));
        return RET_ERR;
    }
    close(fd);
    key_t msgSendKey = ftok(filepath, 'E');
    if (msgSendKey == -1) {
        LOG_Error("%s : ftok error : %s ",__func__,strerror(errno));
        return RET_ERR;
    }

    msgID = msgget(msgSendKey, 0666 | IPC_CREAT);
    if (msgID < 0)
    {
        LOG_Error("%s : SendEventToEcallServer msgget error : %s ", __func__,strerror(errno));
        return RET_ERR;
    }
    else
    {
        LOG_Info("%s : SendEventToEcallServer msgget successfull %s ", __func__ ,strerror(errno));
    }
    LOG_Info("msg SendEventToEcallServer ");
    localEcallEventControlCmd.SubCmd = ECALL_MSG_SUBCMD;
    ret = msgsnd(msgID,&localEcallEventControlCmd, sizeof(StrEcallEventControlCmd_t),0);
    return ret;
}
/**
* @brief 监听ECALL事件控制命令
*
* @param[in] ecallEventControlCmd 存储事件控制命令的结构体
* @return 返回发送消息的ID
*/
static int32_t EcallEventMonitor(StrEcallEventControlCmd_t *ecallEventControlCmd)
{   
    int32_t msgID = 0;
    int ret = 0;
    const char *filepath = "/tmp/ecallKey";

    int fd = open(filepath, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        LOG_Error("%s : open error : %s ",__func__,strerror(errno));
        return RET_ERR;
    }
    close(fd);
    key_t msgMonitorKey = ftok(filepath, 'E');
    if (msgMonitorKey == -1) {
        LOG_Error("%s : ftok error : %s ",__func__,strerror(errno));
        return RET_ERR;
    }
    msgID = msgget(msgMonitorKey, 0666 | IPC_CREAT);
    if (msgID < 0)
    {
        LOG_Error("%s : EcallEventMonitor msgget error : %s ", __func__,strerror(errno));
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s : SendEventToEcallServer msgget successfull.", __func__);
    }

    LOG_Debug("wait mcu event.");
    ret = msgrcv(msgID,ecallEventControlCmd, sizeof(StrEcallEventControlCmd_t) ,ECALL_MSG_SUBCMD,0);
    if (ret < 0)
    {
        LOG_Error("%s : SendEventToEcallServer msgrcv failed %s ", __func__ ,strerror(errno));
    }
    LOG_Debug("EcallEventMonitor Rec ecallEventControlCmd ,ecallEventType : [%d] ,ecallTriggerType : [%d] ",
    ecallEventControlCmd->ecallEventType,ecallEventControlCmd->ecallTriggerType);
    return RET_OK;
    
}
/**
* @brief 处理ECALL事件
*
* @param[in] event ECALL事件类型
* @return 无
*/
static void EcallEventStateCallBack(EnuEcallEventId_e event)
{
    StrEcallEventControlCmd_t ecallEventControlCmd = {0};
    switch (event) 
    {
        case ECALL_STATUS_INCOMING:
            SendMsgToAllQueue(SYS_QUE_TYPE_ECALL_INCOMING_NOTIFY, NULL, 0);
            usleep(500000);
            PmSetSleepEvent(PM_SLEEP_EVENT_ECALL);
            break;
        case ECALL_STATUS_CALLING:
            StopTimer(&abnormalTimerId);
            PmSetSleepEvent(PM_SLEEP_EVENT_ECALL);
            break;
        case ECALL_STATUS_END:
            PmDestroySleepAlarm(CPU_ECALL_T9_TIMER);
            PmCreateSleepAlarm(CPU_ECALL_T9_TIMER,3600,0);
            break;
        case ECALL_STATUS_ABNORMAL_END:
            TimerXSecTimer(&abnormalTimerId,120,0);
            break;
        case ECALL_STATUS_WAIT_PSAP:
            PmDestroySleepAlarm(CPU_ECALL_T10_TIMER);
            PmCreateSleepAlarm(CPU_ECALL_T10_TIMER,3600*10,0);
            g_EnuEcallStatus = event;
            SendMcuEcallStatusSync(g_EnuEcallStatus);
            PmCleanSleepEvent(PM_SLEEP_EVENT_ECALL);
            break;
        case ECALL_STATUS_SNED_MSD:
            break;
        case ECALL_STATUS_PUSH_MSD:
            // 推动状态
            ecallEventControlCmd.ecallEventType = ECALL_TYPE_PUSH_MSD;
            SendEventToEcallServer(&ecallEventControlCmd);
            break;
        case ECALL_STATUS_UPDATE_MSD:
            // 推动状态
            ecallEventControlCmd.ecallEventType = ECALL_TYPE_UPDATE_MSD;
            SendEventToEcallServer(&ecallEventControlCmd);
            break;
        case ECALL_STATUS_MSD_SNED_SUCCESS:
            break;
        case ECALL_STATUS_MSD_SNED_FAULT:
            break;
        case ECALL_STATUS_ACTIVE:
            break; 
        default:
            break;
    }
    
    g_EnuEcallStatus = (EnuEcallStatus)event;
    SendMcuEcallStatusSync(g_EnuEcallStatus);
    LOG_Info("Ecall status :  %d ",g_EnuEcallStatus);
}
/**
* @brief 异常超时处理函数
*
* @param[in] val 信号值
* @return 无
*/
static void AbnormalTimeout(union sigval val)

{
    g_EnuEcallStatus = ECALL_API_STATUS_INIT;
    SendMcuEcallStatusSync(g_EnuEcallStatus);
    LOG_Info("%s: Ecall abnormal end timeout .",__FUNCTION__);
    PmCleanSleepEvent(PM_SLEEP_EVENT_ECALL);
}
/**
* @brief 初始化ECALL配置
*
* @return 无
*/
static void EcallConfigInit(void)    
{
    
    // 创建配置结构体实例并初始化
    StrEcallConfig ecallConfig = {
            .ecallConfigEnable = 1,
            .ecallConfigVoiceConf = 0,
            .ecallConfigMode = 0,
            .ecallConfigProcessinfo = 1,
            .ecallConfigStartTimer = 5,
            .ecallConfigHackTimer = 5,
            .ecallConfigMsdTransmission = 20,
            .ecallConfigMoFailRedial = 1,
            .ecallConfigDropRedial = 1,
            .ecallConfigCleardownTimer = 3600,  // 默认1小时
            .ecallConfigInitiationTimer = 2,    // 默认2秒
            .ecallConfigNadRegistrationTimer = 3600,  // 默认1小时
            .ecallConfigNadUnregistrationTimer = 43200, // 默认12小时
            .ecallConfigSystem = 0,  // 默认European eCall
            .ecallConfigEraGlonassRedialTimer = 3,  // 默认3秒
            .ecallConfigEraGlonassAutoAnswerTimer = 2,  // 默认2秒
            .ecallConfigRedialDurationTimer = 120  // 默认2分钟
        };
    
    // 定义配置项数组          注释项目前采用默认配置
    ConfigMapping_t configMapping[] = {
        {21,"E_QL_MCM_VOICE_ECALL_CONFIG_ENABLE", &ecallConfig.ecallConfigEnable},
        {22,"E_QL_MCM_VOICE_ECALL_CONFIG_VIOCECONF", &ecallConfig.ecallConfigVoiceConf},
        {23,"E_QL_MCM_VOICE_ECALL_CONFIG_MODE", &ecallConfig.ecallConfigMode},
        {24,"E_QL_MCM_VOICE_ECALL_CONFIG_PROCESSINFO", &ecallConfig.ecallConfigProcessinfo},
        {25,"E_QL_MCM_VOICE_ECALL_CONFIG_START_TIMER", &ecallConfig.ecallConfigStartTimer},
        {26,"E_QL_MCM_VOICE_ECALL_CONFIG_HACK_TIMER", &ecallConfig.ecallConfigHackTimer},
        {27,"E_QL_MCM_VOICE_ECALL_CONFIG_MSD_TRANSMISSION", &ecallConfig.ecallConfigMsdTransmission},
        {28,"E_QL_MCM_VOICE_ECALL_CONFIG_MO_FAILR_REDIAL", &ecallConfig.ecallConfigMoFailRedial},
        {29,"E_QL_MCM_VOICE_ECALL_CONFIG_DROP_REDIAL", &ecallConfig.ecallConfigDropRedial},
        {30,"E_QL_MCM_VOICE_ECALL_CONFIG_CLEARDOWN_TIMER", &ecallConfig.ecallConfigCleardownTimer},
        {31,"E_QL_MCM_VOICE_ECALL_CONFIG_INITIATION_TIMER", &ecallConfig.ecallConfigInitiationTimer},
        {32,"E_QL_MCM_VOICE_ECALL_CONFIG_NAD_REGISTRATION_TIMER", &ecallConfig.ecallConfigNadRegistrationTimer},
        {33,"E_QL_MCM_VOICE_ECALL_CONFIG_NAD_UNREGISTRATION_TIMER", &ecallConfig.ecallConfigNadUnregistrationTimer},
        {34,"E_QL_MCM_VOICE_ECALL_CONFIG_ECALL_SYSTEM", &ecallConfig.ecallConfigSystem},
        {35,"E_QL_MCM_VOICE_ECALL_CONFIG_ERA_GLONASS_REDIAL_TIMER", &ecallConfig.ecallConfigEraGlonassRedialTimer},
        {36,"E_QL_MCM_VOICE_ECALL_CONFIG_ERA_GLONASS_AUTO_ANSWER_TIMER", &ecallConfig.ecallConfigEraGlonassAutoAnswerTimer},
        {37,"E_QL_MCM_VOICE_ECALL_CONFIG_REDIAL_DURATION_TIMER", &ecallConfig.ecallConfigRedialDurationTimer},
    };
        
    size_t num_mappings = sizeof(configMapping) / sizeof(configMapping[0]);
    
    for (size_t i = 0; i < num_mappings; ++i) 
    {
        ParamInfo_t paramInfo = {0};
        paramInfo.id = configMapping[i].paramId;
        
        int32_t value = GetParam(&paramInfo);
        if (value == RET_OK) 
        {
            uint16_t paramValue = atoi((char *)paramInfo.value);
            *configMapping[i].value = paramValue;
            LOG_Debug("paramId : %d, value : %d",configMapping[i].paramId,*configMapping[i].value);
            if(0 == strcmp(configMapping[i].key, "E_QL_MCM_VOICE_ECALL_CONFIG_MODE"))
            {
                if(paramValue > E_VOICE_ECALL_MODE_DEFAULT || paramValue < E_VOICE_ECALL_MODE_NOMAL)
                {
                    //参数超出范围默认设置为normal模式
                    ECALL_NET_MODE = E_VOICE_ECALL_MODE_NOMAL;
                }
                else
                {
                    ECALL_NET_MODE = paramValue;
                }
                LOG_Info("%s:  ECALL_NET_MODE is :%d  .",__FUNCTION__,ECALL_NET_MODE);
            }
        } 
        else 
        {
            LOG_Error("ecall param config : [%u : %s ] is not config,use default.", configMapping[i].paramId,configMapping[i].key);
        }
    }
    
    // 将配置结构体传递给设置函数
    uint8_t setResult = HalEcallVoiceSetConfigInfo(&ecallConfig);
    if (setResult != 0) {
        LOG_Error("Failed to set eCall configuration!");
    }

    //读取 其他参数
    ParamInfo_t pInfo = {0};
    pInfo.id = 39; //测试电话参数ID
    uint32_t configRead = GetParam(&pInfo);
    if(configRead == RET_ERR)
	{
        memcpy(TEST_CALL_NUM,"112",strlen("112"));
        LOG_Info("MCM_VOICE_ECALL_TEST_NUM is not config. set defult value : [%s]",TEST_CALL_NUM);
	}
    else
    {
        if(strlen((char *)pInfo.value) <= CALL_NUM_LENGTH)
        {
            memcpy(TEST_CALL_NUM,pInfo.value,strlen((char *)pInfo.value));
            LOG_Info("MCM_VOICE_ECALL_TEST_NUM read : [%s]",TEST_CALL_NUM);
        }
        else
        {
            memcpy(TEST_CALL_NUM,"112",strlen("112"));
            LOG_Info("MCM_VOICE_ECALL_TEST_NUM is bigger than default config. set defult value : [%s]",TEST_CALL_NUM);
        }
    }

    memset(&pInfo,0,sizeof(ParamInfo_t));
    pInfo.id = 38; //测试模式ID
    configRead = GetParam(&pInfo);
    if(configRead == RET_ERR)
	{
        TEST_CALL_MODE = TEST_CALL_CLOSE;
        LOG_Info("MCM_VOICE_ECALL_TEST_NUM is not config. set defult value : [%d]",TEST_CALL_MODE);
	}
    else
    {
        TEST_CALL_MODE = atoi((char *)pInfo.value);
		LOG_Info("MCM_VOICE_ECALL_TEST_MODE read : [%d] ",TEST_CALL_MODE);
    }
    
}
/**
* @brief 将事件控制命令转换为MSD消息
*
* @param[in] ecallEventControlCmd 存储事件控制命令的结构体
* @return 返回MSD消息结构体
*/
static StrMsdInfo AssembleMsdInfo(StrEcallEventControlCmd_t *ecallEventControlCmd)
{
    StrMsdInfo msdInfo = {0};
    
    if(TEST_CALL_MODE == TEST_CALL_OPEN )
    {
        msdInfo.ecallType = TEST_CALL_OPEN;
    }
    else
    {
        if(ECALL_TRIGGER_TEST == ecallEventControlCmd->ecallTriggerType)
        {
            msdInfo.ecallType = TEST_CALL_OPEN;
        }
        else
        {
            msdInfo.ecallType = TEST_CALL_CLOSE;
        }
    }
    
    if(ecallEventControlCmd->ecallTriggerType == ECALL_TRIGGER_SOS || ecallEventControlCmd->ecallTriggerType == ECALL_TRIGGER_TEST)
        msdInfo.ecallTriggerModel = TRIGGER_MANUAL;
    else
        msdInfo.ecallTriggerModel = TRIGGER_AUTO;
    msdInfo.gpsInfo = GetGpsInfo();

    ParamInfo_t param = {0};
    param.id = 4; //vin 码的参数ID
    int32_t ret = GetParam(&param);
    if(ret == RET_OK)
    {
        memcpy(msdInfo.VIN, param.value, VIN_LENGTH);
    }
    else
    {
        memset(msdInfo.VIN, 0, VIN_LENGTH); 
    }

    return msdInfo;
}
/**
* @brief 将事件控制命令转换为MSD消息
*
* @param[in] ecallEventControlCmd 存储事件控制命令的结构体
* @param[in] msdInfo MSD消息结构体
* @param[in] msdLen MSD消息长度
* @return 返回MSD消息结构体
*/
static StrEcallVoiceInfo AssembleEcallVoiceInfo(StrEcallEventControlCmd_t *ecallEventControlCmd,char *msdInfo,uint8_t msdLen)
{
    StrEcallVoiceInfo ecallVoiceInfo = {0};
    
    if(TEST_CALL_MODE == TEST_CALL_OPEN )
    {
        ecallVoiceInfo.eCallModeType = ECALL_TEST;
    }
    else
    {
        if(ECALL_TRIGGER_TEST == ecallEventControlCmd->ecallTriggerType)
        {
            ecallVoiceInfo.eCallModeType = ECALL_TEST;
        }
        else
        {
            ecallVoiceInfo.eCallModeType = ECALL_EMERGENCY;
        }
    }
    if(ecallEventControlCmd->ecallTriggerType == ECALL_TRIGGER_SOS || ecallEventControlCmd->ecallTriggerType == ECALL_TRIGGER_TEST)
        ecallVoiceInfo.manual = TRIGGER_MANUAL;
    else
        ecallVoiceInfo.manual = TRIGGER_AUTO;

    if(msdLen > ECALL_MSD_MAX_LEN)
    {
        memcpy(ecallVoiceInfo.ecall_msd,msdInfo,ECALL_MSD_MAX_LEN);
        ecallVoiceInfo.msd_len = ECALL_MSD_MAX_LEN;
    }
    else
    {
        memcpy(ecallVoiceInfo.ecall_msd,msdInfo,msdLen);
        ecallVoiceInfo.msd_len = msdLen;
    }
    
    memcpy(ecallVoiceInfo.PhoneNumber,TEST_CALL_NUM,ECALL_PHONE_NUM_LEN);

    ecallVoiceInfo.simId = U_SIM_1;

    return ecallVoiceInfo;
}
/**
* @brief 处理ECALL服务器回调
*
* @param[in] subcmd 子命令
* @param[in] reqData 请求数据
* @param[in] reqLen 请求数据长度
* @param[out] respData 响应数据
* @param[out] respLen 响应数据长度
* @param[in] userdata 用户数据
* @return 无
*/
static void EcallServerCallBack(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
    *respLen = sizeof(g_EnuEcallStatus);
    memcpy(respData,&g_EnuEcallStatus,sizeof(g_EnuEcallStatus));
}
/**
* @brief PM消息订阅回调函数
*
* @param[in] data PM消息数据
* @param[in] len PM消息长度
* @param[in] userdata 用户数据
* @return 无
*/
static void PmMsgSubCallBack(const uint8_t* data, const uint16_t len, void* userdata)
{
	MPUPmStateChange_t MPUPmStateChange;
	memset(&MPUPmStateChange, 0x00, sizeof(MPUPmStateChange_t));
    
    if(sizeof(MPUPmStateChange_t) == len && NULL != data)
    {
        memcpy(&MPUPmStateChange, data, len);
    }
    else
    {
        LOG_Debug("%s len error!",__FUNCTION__);
        return;
    }

    if(MPUPmStateChange.SleepAlarmEventId == CPU_ECALL_T9_TIMER) //t9 timer
    {   
        PmSetSleepEvent(PM_SLEEP_EVENT_ECALL);
        g_EnuEcallStatus = ECALL_API_STATUS_INIT;
        SendMcuEcallStatusSync(g_EnuEcallStatus);
        PmDestroySleepAlarm(CPU_ECALL_T9_TIMER);
        system("rm /oemdata/log/sirunlog*");
        EcallNetAirPlaneModeConfig();
        LOG_Info("Ecall t9 timer, ECALL TIMER ID : %d ,ECALL STATUS : %d ",CPU_ECALL_T9_TIMER,g_EnuEcallStatus);
        PmCleanSleepEvent(PM_SLEEP_EVENT_ECALL);
    }

    if(MPUPmStateChange.SleepAlarmEventId == CPU_ECALL_T10_TIMER)//t10 timer
    {
        PmSetSleepEvent(PM_SLEEP_EVENT_ECALL);
        g_EnuEcallStatus = ECALL_API_STATUS_INIT;
        SendMcuEcallStatusSync(g_EnuEcallStatus);
        PmDestroySleepAlarm(CPU_ECALL_T10_TIMER);
        system("rm /oemdata/log/sirunlog*");
        EcallNetAirPlaneModeConfig();
        LOG_Info("Ecall t10 timer, ECALL TIMER ID : %d ,ECALL STATUS : %d ",CPU_ECALL_T10_TIMER,g_EnuEcallStatus);
        PmCleanSleepEvent(PM_SLEEP_EVENT_ECALL);
    }
}

void FillEcallMsdData(EcallMsd_t *ecallMsdData, ecallMsg *msdInfo)
{
    if(!ecallMsdData || !msdInfo)
    {
        LOG_Error("%s invalid param", __FUNCTION__);
        return;
    }

    memcpy(ecallMsdData->vin, msdInfo->VIN.isowmi, 3);
    memcpy(&ecallMsdData->vin[3], msdInfo->VIN.isovds, 6);
    memcpy(&ecallMsdData->vin[9], msdInfo->VIN.isovisModelyear, 1);
    memcpy(&ecallMsdData->vin[10], msdInfo->VIN.isovisSeqPlant, 7);
    ecallMsdData->msdVersion = 3;
    ecallMsdData->ecallPostion.latitude = msdInfo->position.latitude;
    ecallMsdData->ecallPostion.longitude = msdInfo->position.longitude;
    ecallMsdData->ecallPostion.isTrusted = msdInfo->control.positionCanBeTrusted;
    ecallMsdData->ecallPostion.direction = msdInfo->vehicleDirection;
    ecallMsdData->ecallPositionDeltaN1.latitudeDelta = msdInfo->recentVehicleLocationN1.latitudeDelta;
    ecallMsdData->ecallPositionDeltaN1.longitudeDelta = msdInfo->recentVehicleLocationN1.longitudeDelta;
    ecallMsdData->ecallPositionDeltaN2.latitudeDelta = msdInfo->recentVehicleLocationN2.latitudeDelta;
    ecallMsdData->ecallPositionDeltaN2.longitudeDelta = msdInfo->recentVehicleLocationN2.longitudeDelta;
    ecallMsdData->vehicleType = msdInfo->control.vehType;
    ecallMsdData->numberOfPassengers = msdInfo->numberOfPassengers;
    ecallMsdData->PropulsionType = 0x10; //固定为电力驱动
    if(TEST_CALL_MODE == TEST_CALL_OPEN)
    {
        ecallMsdData->testCall = 1;
    }
    else
    {
        ecallMsdData->testCall = 0;
    } 
}

/**
* @brief ECALL SERVER 线程
*
* @param[in] pArg 用户数据
* @return 无
*/
gpointer EcallServer(gpointer pArg)
{
    StrEcallEventControlCmd_t ecallEventControlCmd = {0};
    StrEcallEventControlCmd_t ecallEventControlCmdSave = {0};
    StrMsdInfo msdInfo = {0};
    char msdEncode[ECALL_MSD_MAX_LEN + 512] = {0};
    uint8_t msdLen = 0;
    StrEcallVoiceInfo ecallVoiceInfo = {0};
    int32_t ReturnValue = 0;
    ecallMsg msd; //获取的msd信息
    EcallMsd_t ecallHalMsdData; //用于hal层填充模组接口，使用ecallMsg msd填充

    LOG_Debug("%s init.",__FUNCTION__);
    SubscriptionQueueMsg(SYS_QUE_TYPE_GNSS_LOCATIONINFO, GnssMsgSubCallBack, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, PmMsgSubCallBack, NULL);
    RpcStartMsgServer(RPCSERVER_TYPE_ECALLSERVER, EcallServerCallBack,2, NULL);
    HalEcallVoiceInit(EcallEventStateCallBack);
    EcallConfigInit();  //加载配置文件配置

    EcallNetAirPlaneModeConfig(); //ecall only 模式配置
    
    //注册初始状态定时器，电话异常结束后2min之内恢复到初始状态
    InitTimer(&abnormalTimerId,AbnormalTimeout);

    HalEcallAudioPrepare();

    while(1)
    {
        memset(&ecallEventControlCmd, 0, sizeof(StrEcallEventControlCmd_t));
        if(RET_OK != EcallEventMonitor(&ecallEventControlCmd))
        {
            continue;
            usleep(50*1000);
        }
        
        //状态机
        switch(ecallEventControlCmd.ecallEventType)
        {
            case ECALL_TYPE_START_ECALL:
                EcallNetAirPlaneModeDeconfig();
                ecallEventControlCmdSave = ecallEventControlCmd;
                memset(msdEncode, 0, sizeof(msdEncode));
                //初始化msd数据
                msdInfo = AssembleMsdInfo(&ecallEventControlCmdSave);
                //组包MSD
                MSDIdentifier = 1;
                memset(&msd, 0, sizeof(ecallMsg));
                msdLen = UpdataMsd(msdEncode,msdLen,&msdInfo,&msd);
                //初始化StrEcallVoiceInfo数据
                ecallVoiceInfo = AssembleEcallVoiceInfo(&ecallEventControlCmdSave,msdEncode,msdLen);
                //数据拨号
                memset(&ecallHalMsdData, 0, sizeof(EcallMsd_t));
                FillEcallMsdData(&ecallHalMsdData, &msd);
                ReturnValue = HalEcallVoiceStart(&ecallVoiceInfo, &ecallHalMsdData);
                LOG_Info("%s:  HalEcallVoiceStart is action,return value : %d .",__FUNCTION__,ReturnValue);
                break;
            case ECALL_TYPE_HUANGUP:
                HalEcallVoiceHangup();
                break;
            case ECALL_TYPE_UPDATE_MSD:
                memset(msdEncode, 0, sizeof(msdEncode));
                msdInfo = AssembleMsdInfo(&ecallEventControlCmdSave);
                MSDIdentifier++;
                memset(&msd, 0, sizeof(ecallMsg));
                msdLen = UpdataMsd(msdEncode,msdLen,&msdInfo,&msd);
                memset(&ecallHalMsdData, 0, sizeof(EcallMsd_t));
                FillEcallMsdData(&ecallHalMsdData, &msd);
                HalEcallVoiceUpdateMsd((int8_t*)msdEncode,msdLen, &ecallHalMsdData);
                break;
            case ECALL_TYPE_PUSH_MSD:
                HalEcallVoicePushMsd();
                break;
            default:
                LOG_Warn("%s : ecallEventType error  ", __func__);
                break;
        }
    }
    TimerDeleteTimer(abnormalTimerId);
    //去激活
    HalEcallVoiceDeinit();
    return NULL;
}

