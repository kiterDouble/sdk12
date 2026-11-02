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

#include "era_nad_manager.h"
#include "ecall_server_api.h"
#include "ECallMessage.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "err_code.h"
#include "era_gps.h"
#include "hal_modem_era.h"
#include "era_timer.h"
#include "era_mcu_manager.h"
#include "pm_server_api.h"
#include "modem_api.h"
#include "era_config.h"

#define VIN_LENGTH (17)
#define MAX_ERA_MSD (140)
#define ECALL_MSG_SUBCMD (0x8007)
#define ABNORMAL_TIMES_INTERVAL (120*1000) //2min

static EraConfigMode_e ECALL_NET_MODE = E_VOICE_ECALL_MODE_NOMAL;

static EnuEcallStatus g_EnuEraStatus = 0;
static timer_t abnormalTimerId = {0};
static timer_t normalEndTimerId = {0};
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

struct ECallSyncData {
    uint8_t selfCheckStatus;
    uint8_t faultStatus[4];
};
int8_t ECallKLine15 = 2;        /* 1为off, 2为on */

#define ECALL_KL30_ST_MSK   (0xf<<4)
#define ECALL_MIC_ST_MSK    (0xf)
#define ECALL_SOS_ST_MSK    (0xf<<4)
#define ECALL_CAN_ST_MSK    (0xf)
#define ECALL_AIR_ST_MSK    (0xf<<0)
#define ECALL_SPK_ST_MSK    (0xf)
#define ECALL_BAT_ST_MSK    (0xf<<4)
#define ECALL_COM_ST_MSK    (0xf)
#define ECALL_MCU_ST_OK     0x1
#define ECALL_MCU_ST_NO     0x0

struct ECallSyncData g_ECallMCUSyncData = 
{
    .selfCheckStatus =  0, 
    .faultStatus[0] = (ECALL_MCU_ST_OK<<4)|ECALL_MCU_ST_OK,
    .faultStatus[1] = (ECALL_MCU_ST_OK<<4)|ECALL_MCU_ST_OK,
    .faultStatus[2] = (ECALL_MCU_ST_OK<<4)|ECALL_MCU_ST_OK,
    .faultStatus[3] = (ECALL_MCU_ST_OK<<4)|ECALL_MCU_ST_OK,
};

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
            LOG_Info("ModemSrvSetAirplaneMode is success,nad into AIRPLANE MODE .");
        }
        else
        {
            LOG_Error("ModemSrvSetAirplaneMode is faild.");
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
            LOG_Info("ModemSrvSetAirplaneMode is success,nad into NORMAL MODE .");
        }
        else
        {
            LOG_Error("ModemSrvSetAirplaneMode is faild.");
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
static int32_t UpdataMsd(char *uperMsd, size_t msdLen, StrMsdInfo *msdInfo)
{
    ecallMsg *msd;
    char isowmi[8]={"ECA"};
    char isovds[8]={"LLEXAM"};
    char isovisModelyear[8]={"P"};
    char isovisSeqPlant[8]={"LE02013"};
    int len = 0;
    StrGPSInfo *gpsInfo = &msdInfo->gpsInfo;
    struct ECallSyncData *opData = &g_ECallMCUSyncData;
    
    msd = (ecallMsg *)calloc(1, sizeof(ecallMsg));
    if(NULL == msd)
        return 0;

    //msgcount 消息计数
    msd->messageIdentifier = MSDIdentifier;
    //ControlType
    //激活模式 1-自动激活 0-手动激活
    msd->control.automaticActivation = msdInfo->ecallTriggerModel;
    //呼叫模式 1-测试呼叫 0-紧急呼叫
    LOG_Info("eCallCmd->eCall_test_call : %d ",msdInfo->ecallType);
    msd->control.testCall = msdInfo->ecallType;

    //位置可信任度 1-可信度高 0-可信度低
    msd->control.positionCanBeTrusted = gpsInfo->trusted;
    msd->vehicleDirection = gpsInfo->direction;
    msd->position.latitude = gpsInfo->latitude;
    msd->position.longitude = gpsInfo->longitude;
    msd->recentVehicleLocationN1.latitudeDelta = gpsInfo->latitudeDeltaN1;
    msd->recentVehicleLocationN1.longitudeDelta = gpsInfo->longitudeDeltaN1;
    msd->recentVehicleLocationN2.latitudeDelta = gpsInfo->latitudeDeltaN2;
    msd->recentVehicleLocationN2.longitudeDelta = gpsInfo->longitudeDeltaN2;
    msd->timestamp = gpsInfo->timestamp;

    //车型 固定为M1
    EConfigVehicleTypeFill(&msd->control.vehType);

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

    // VehiclePropulsionStorageType 固定为储能电池
    VehiclePropulsionStorageTypeFill(&msd->vehPropulsionStorageType);
    msd->numberOfPassengers = 255;

    msd->optionalDataPres = 1;
    msd->optionalData.diagMicConnection = (opData->faultStatus[0] & ECALL_MIC_ST_MSK)? 1: 0;
    msd->optionalData.diagSpeakerConnection = (opData->faultStatus[2] & ECALL_SPK_ST_MSK)? 1: 0;
    msd->optionalData.diagIgnition = (1==ECallKLine15)? 0: 1;       /* ECallKLine15==1? KL15下电 */
    msd->optionalData.diagUIMTest = (opData->faultStatus[1] & ECALL_SOS_ST_MSK)? 1: 0;
    msd->optionalData.diagBattery = (opData->faultStatus[3] & ECALL_BAT_ST_MSK)? 1: 0;
    msd->optionalData.diagRTADetector = (opData->faultStatus[2] & ECALL_AIR_ST_MSK)? 1: 0;
    msd->optionalData.diagImageOk = 1;
    msd->optionalData.diagGNSSReceiver = 1;
    len = Encode_Msg(msd, uperMsd, msdLen);  /* uperMsd消息体 */
    LOG_Info("encoded msd length =[%d] ", len);
    free(msd);

    return len;
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
    const char *filepath = "/tmp/eraKey";
    StrEcallEventControlCmd_t localEcallEventControlCmd = {0};
    memcpy(&localEcallEventControlCmd,ecallEventControlCmd,sizeof(StrEcallEventControlCmd_t));

    int fd = open(filepath, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        LOG_Error("open error : %s ",strerror(errno));
        return RET_ERR;
    }
    close(fd);
    key_t msgSendKey = ftok(filepath, 'A');
    if (msgSendKey == -1) {
        LOG_Error("ftok error : %s ",strerror(errno));
        return RET_ERR;
    }

    msgID = msgget(msgSendKey, 0666 | IPC_CREAT);
    if (msgID < 0)
    {
        LOG_Error("SendEventToEcallServer msgget error : %s ",strerror(errno));
        return RET_ERR;
    }
    else
    {
        LOG_Info("SendEventToEcallServer msgget successfull");
    }
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
    const char *filepath = "/tmp/eraKey";

    int fd = open(filepath, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        LOG_Error("open error : %s ",strerror(errno));
        return RET_ERR;
    }
    close(fd);

    key_t msgMonitorKey = ftok(filepath, 'A');
    if (msgMonitorKey == -1) {
        LOG_Error("ftok error : %s ",strerror(errno));
        return RET_ERR;
    }
    msgID = msgget(msgMonitorKey, 0666 | IPC_CREAT);
    if (msgID < 0)
    {
        LOG_Error("EcallEventMonitor msgget error : %s ",strerror(errno));
        return RET_ERR;
    }
    else
    {
        LOG_Debug("SendEventToEcallServer msgget successfull.");
    }

    LOG_Debug("monitor ecall event.");
    ret = msgrcv(msgID, ecallEventControlCmd, sizeof(StrEcallEventControlCmd_t), ECALL_MSG_SUBCMD, 0);
    if (ret < 0)
    {
        LOG_Error("SendEventToEcallServer msgrcv failed %s ", strerror(errno));
    }
    LOG_Debug("new event = ecallEventType : [%d] ,ecallTriggerType : [%d] ",
        ecallEventControlCmd->ecallEventType,ecallEventControlCmd->ecallTriggerType);

    return RET_OK;
    
}
/**
* @brief 处理ECALL事件
*
* @param[in] event ECALL事件类型
* @return 无
*/
static void EcallEventStateCallBack(EnuEraEventId_e event)
{
    StrEcallEventControlCmd_t ecallEventControlCmd = {0};
    switch (event) 
    {
        case ECALL_STATUS_INCOMING:
            SendMsgToAllQueue(SYS_QUE_TYPE_ECALL_INCOMING_NOTIFY, NULL, 0);
            PmSetSleepEvent(PM_SLEEP_EVENT_ECALL);
            break;
        case ECALL_STATUS_CALLING:
            StopTimer(&abnormalTimerId);
            StopTimer(&normalEndTimerId);
            PmSetSleepEvent(PM_SLEEP_EVENT_ECALL);
            break;
        case ECALL_STATUS_END:
//          PmDestroySleepAlarm(CPU_ECALL_T9_TIMER);
//          PmCreateSleepAlarm(CPU_ECALL_T9_TIMER,3600,0);
            TimerXSecTimer(&normalEndTimerId,2,0);
            break;
        case ECALL_STATUS_ABNORMAL_END:
            TimerXSecTimer(&abnormalTimerId,60,0);
            break;
        case ECALL_STATUS_WAIT_PSAP:
//          PmDestroySleepAlarm(CPU_ECALL_T10_TIMER);
//          PmCreateSleepAlarm(CPU_ECALL_T10_TIMER,3600*10,0);
            PmCleanSleepEvent(PM_SLEEP_EVENT_ECALL);
            break;
        case ECALL_STATUS_SNED_MSD:
            break;
        case ECALL_STATUS_PUSH_MSD:     // 推动状态
            ecallEventControlCmd.ecallEventType = ECALL_TYPE_PUSH_MSD;
            SendEventToEcallServer(&ecallEventControlCmd);
            break;
        case ECALL_STATUS_UPDATE_MSD: // 推动状态
            ecallEventControlCmd.ecallEventType = ECALL_TYPE_UPDATE_MSD;
            SendEventToEcallServer(&ecallEventControlCmd);
            break;
        case ECALL_STATUS_MSD_SNED_SUCCESS:
            break;
        case ECALL_STATUS_MSD_SNED_FAULT:
            break;
        case ECALL_STATUS_ACTIVE:       /* ecall conversation : push msd */
            ecallEventControlCmd.ecallEventType = ECALL_TYPE_PUSH_MSD;
            SendEventToEcallServer(&ecallEventControlCmd);
            break;
        default:
            break;
    }
    
    g_EnuEraStatus = (EnuEcallStatus)event;
    SendMcuEcallStatusSync(g_EnuEraStatus);
    LOG_Info("Ecall status :  %d ",g_EnuEraStatus);
}
/**
* @brief 异常超时处理函数
*
* @param[in] val 信号值
* @return 无
*/
static void AbnormalTimeout(union sigval val)
{
    g_EnuEraStatus = ECALL_API_STATUS_INIT;
    SendMcuEcallStatusSync(g_EnuEraStatus);
    LOG_Info("%s : Ecall abnormal end timeout .",__FUNCTION__);
    PmCleanSleepEvent(PM_SLEEP_EVENT_ECALL);
}
static void NormalEndTimeout(union sigval val)
{
    LOG_Info("%s : Ecall normal end 2s timer.",__FUNCTION__);
    PmCleanSleepEvent(PM_SLEEP_EVENT_ECALL);
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
    
    if(EConfigECallTestMode())
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
    msdInfo.gpsInfo = *GetGpsInfo();

    EConfigVINFill(msdInfo.VIN, sizeof(msdInfo.VIN));

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
static StrEraVoiceInfo AssembleEcallVoiceInfo(StrEcallEventControlCmd_t *ecallEventControlCmd,char *msdInfo,uint8_t msdLen)
{
    StrEraVoiceInfo ecallVoiceInfo = {0};

    memcpy((char *)ecallVoiceInfo.PhoneNumber,"112",ERA_PHONE_NUM_LEN);
    if(EConfigECallTestMode())
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

    if(msdLen > MAX_ERA_MSD)
    {
        memcpy(ecallVoiceInfo.ecall_msd, msdInfo, MAX_ERA_MSD);
        ecallVoiceInfo.msd_len = MAX_ERA_MSD;
    }
    else
    {
        memcpy(ecallVoiceInfo.ecall_msd,msdInfo,msdLen);
        ecallVoiceInfo.msd_len = msdLen;
    }
    
    EConfigECallTestNumberFill((char *)ecallVoiceInfo.PhoneNumber, ERA_PHONE_NUM_LEN); /* 测试号码填充，非测试模式，模组内部写死，无法更改*/

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
    *respLen = sizeof(g_EnuEraStatus);
    memcpy(respData,&g_EnuEraStatus,sizeof(g_EnuEraStatus));
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
        LOG_Debug("len error!");
        return;
    }

    if(MPUPmStateChange.SleepAlarmEventId == CPU_ECALL_T9_TIMER) //t9 timer
    {   
//        PmSetSleepEvent(PM_SLEEP_EVENT_ECALL);
        g_EnuEraStatus = ECALL_API_STATUS_INIT;
//        SendMcuEcallStatusSync(g_EnuEraStatus);
//        PmDestroySleepAlarm(CPU_ECALL_T9_TIMER);
        system("rm /oemdata/log/sirunlog*");
        EcallNetAirPlaneModeConfig();
        LOG_Info("Ecall t9 timer, ECALL TIMER ID : %d ,ECALL STATUS : %d ",CPU_ECALL_T9_TIMER,g_EnuEraStatus);
//        PmCleanSleepEvent(PM_SLEEP_EVENT_ECALL);
    }

    if(MPUPmStateChange.SleepAlarmEventId == CPU_ECALL_T10_TIMER)//t10 timer
    {
//        PmSetSleepEvent(PM_SLEEP_EVENT_ECALL);
        g_EnuEraStatus = ECALL_API_STATUS_INIT;
//        SendMcuEcallStatusSync(g_EnuEraStatus);
//        PmDestroySleepAlarm(CPU_ECALL_T10_TIMER);
        system("rm /oemdata/log/sirunlog*");
        EcallNetAirPlaneModeConfig();
        LOG_Info("Ecall t10 timer, ECALL TIMER ID : %d ,ECALL STATUS : %d ",CPU_ECALL_T10_TIMER,g_EnuEraStatus);
//        PmCleanSleepEvent(PM_SLEEP_EVENT_ECALL);
    }
    if(MPUPmStateChange.WakeupSrc == PM_WAKEUP_SOURCE_RING)
    {
        LOG_Debug("yaosheng :%s : before SendMsgToAllQueue.",__FUNCTION__);
        SendMsgToAllQueue(SYS_QUE_TYPE_ECALL_INCOMING_NOTIFY, NULL, 0);
        LOG_Debug("yaosheng :%s : after SendMsgToAllQueue.",__FUNCTION__);
        LOG_Debug("yaosheng :%s : before set incoming sleep event.",__FUNCTION__);
        PmSetSleepEvent(PM_SLEEP_EVENT_ECALL);
        LOG_Debug("yaosheng :%s : after set incoming sleep event.",__FUNCTION__);
        g_EnuEraStatus = ECALL_API_STATUS_INCOMING;
        SendMcuEcallStatusSync(g_EnuEraStatus);
    }
}


void ECallMCUSync(uint32_t type, const uint8_t *data, int len)
{
    struct ECallSyncData *sync = NULL;

    if(SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_CON_SERVICE == type)
    {
        sync = (struct ECallSyncData *)data;
        if(5 != len)
            return;

        if(sync->selfCheckStatus > 2)
            return;
        memcpy(&g_ECallMCUSyncData, data, len);
    }
    else if(SYS_QUE_TYPE_MCU_SYNC_KLINE == type)
    {
        if(len > 0)
            ECallKLine15 = data[0];
    }
}


void ECallInfoDump(long subCmd);
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
    char msdEncode[MAX_ERA_MSD + 512] = {0};
    uint8_t msdLen = 0;
    StrEraVoiceInfo ecallVoiceInfo = {0};
    int32_t ReturnValue = 0;

    LOG_Debug("init start");

    EcallGnssInit();
    SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, PmMsgSubCallBack, NULL);
    RpcStartMsgServer(RPCSERVER_TYPE_ECALLSERVER, EcallServerCallBack,2, NULL);
    HalEraVoiceInit(EcallEventStateCallBack);
    EConfigInit();  //加载配置文件配置
    EcallNetAirPlaneModeConfig(); //ecall only 模式配置
    
    //注册初始状态定时器，电话异常结束后2min之内恢复到初始状态
    InitTimer(&abnormalTimerId, AbnormalTimeout);

    //注册恢复通话结束状态定时器
    InitTimer(&normalEndTimerId, NormalEndTimeout);

    while(1)
    {
        LOG_Debug("------- event start -------");
        memset(&ecallEventControlCmd, 0, sizeof(StrEcallEventControlCmd_t));
        if(RET_OK != EcallEventMonitor(&ecallEventControlCmd))
        {
            continue;
            usleep(50*1000);
        }

        LOG_Warn("ecallEventType = %s(%d)",  ECallEventTypeToString(ecallEventControlCmd.ecallEventType), ecallEventControlCmd.ecallEventType);
        //状态机
        switch(ecallEventControlCmd.ecallEventType)
        {
            case ECALL_TYPE_START_ECALL:
                EcallNetAirPlaneModeDeconfig();
                ecallEventControlCmdSave = ecallEventControlCmd;
                memset(msdEncode, 0, sizeof(msdEncode));
                //初始化msd数据
                msdInfo = AssembleMsdInfo(&ecallEventControlCmdSave);   /* 获取ERA当前基本信息 */
                //组包MSD
                MSDIdentifier = 1;
                msdLen = UpdataMsd(msdEncode, sizeof(msdEncode), &msdInfo);        /* 组装MSD帧格式原文 */
                //初始化StrEraVoiceInfo数据
                ecallVoiceInfo = AssembleEcallVoiceInfo(&ecallEventControlCmdSave,msdEncode,msdLen);    /* 组装模组参数结构 */
                //数据拨号
                ReturnValue = HalEraVoiceStart(&ecallVoiceInfo);
                LOG_Info("ecall do dialing(%s), return value : %d", EConfigECallTestMode()? EConfigECallTestNumber(): "112", ReturnValue);
                break;
            case ECALL_TYPE_HUANGUP:
                HalEraVoiceHangup();
                LOG_Info("ecall do hangup");
                break;
            case ECALL_TYPE_UPDATE_MSD:         /* 救援中心获取MSD请求 */
                memset(msdEncode, 0, sizeof(msdEncode));
                msdInfo = AssembleMsdInfo(&ecallEventControlCmdSave);
                MSDIdentifier++;
                msdLen = UpdataMsd(msdEncode, msdLen, &msdInfo);
                HalEraVoiceUpdateMsd((int8_t*)msdEncode, msdLen);
                LOG_Info("ecall do msd update");
                break;
            case ECALL_TYPE_PUSH_MSD:
                HalEraVoicePushMsd();
                LOG_Info("ecall do msd push");
                break;
            case ECALL_TYPE_DUMP_INFO:
                ECallInfoDump(ecallEventControlCmd.SubCmd);
                break;
            default:
                LOG_Warn("ecallEventType error  ");
                break;
        }
    }
    TimerDeleteTimer(abnormalTimerId);
    //去激活
    HalEraVoiceDeinit();
    return NULL;
}


void ECallInfoDump(long subCmd)
{
    int len = 0;
    char buff[2048 * 2];  // 确保此数组足够大

    LOG_Debug("%s", __FUNCTION__);

    // 用更安全的方式处理 snprintf 返回值，避免缓冲区溢出
    int remaining = sizeof(buff) - len;
    int written = snprintf(buff + len, remaining, "\tECall information summary - ");
    if (written < 0 || written >= remaining) {
        LOG_Error("Error: Buffer overflow or snprintf failure after summary\n");
        return;  // 退出或其他错误处理
    }
    len += written;

    written = snprintf(buff + len, sizeof(buff) - len, "test mode          : %s, ", EConfigECallTestMode() ? "open" : "close");
    if (written < 0 || written >= remaining) {
        LOG_Error("Error: Buffer overflow or snprintf failure after test mode\n");
        return;
    }
    len += written;

    written = snprintf(buff + len, sizeof(buff) - len, "test number        : %s, ", EConfigECallTestNumber());
    if (written < 0 || written >= remaining) {
        LOG_Error("Error: Buffer overflow or snprintf failure after test number\n");
        return;
    }
    len += written;

    written = snprintf(buff + len, sizeof(buff) - len, "g_EnuEraStatus   : %s, ", GetEcallStatusString(g_EnuEraStatus));
    if (written < 0 || written >= remaining) {
        LOG_Error("Error: Buffer overflow or snprintf failure after ecall status\n");
        return;
    }
    len += written;

    written = snprintf(buff + len, sizeof(buff) - len, "MSDIdentifier      : %d", MSDIdentifier);
    if (written < 0 || written >= remaining) {
        LOG_Error("Error: Buffer overflow or snprintf failure after MSDIdentifier\n");
        return;
    }
    len += written;

    written = snprintf(buff + len, sizeof(buff) - len, "\n");
    if (written < 0 || written >= remaining) {
        LOG_Error("Error: Buffer overflow or snprintf failure after new line\n");
        return;
    }
    len += written;

    // 最终输出完整的字符串
    LOG_Debug("%s", buff);

    // 输出 GNSS 和 GPS 信息
    EcallGnssDump();

    LOG_Debug("\n");

    if (GetGpsInfo() != NULL) {
        EcallGPSInfoDump(GetGpsInfo());
    } else {
        LOG_Debug("Error: GPS info is NULL\n");
    }

    LOG_Debug("\n");
    
    return;
}

const char *ECallEventTypeToString(EnumEcallEventType type)
{
    if(ECALL_TYPE_START_INIT == type)
        return "init";
    else if(ECALL_TYPE_START_ECALL == type)
        return "start_ecall";
    else if(ECALL_TYPE_HUANGUP == type)
        return "hangup";
    else if(ECALL_TYPE_UPDATE_MSD == type)
        return "update_msd";
    else if(ECALL_TYPE_PUSH_MSD == type)
        return "push_msd";
    else if(ECALL_TYPE_DUMP_INFO == type)
        return "dump_info";
    
    return "unknown";
}

