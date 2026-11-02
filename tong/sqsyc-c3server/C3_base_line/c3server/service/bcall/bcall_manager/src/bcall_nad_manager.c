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

#include "bcall_nad_manager.h"
#include "bcall_server_api.h"
#include "bcall_gps.h"
#include "sr_ipc.h"
#include "sr_log.h"
#include "err_code.h"
#include "bcall_gps.h"
#include "hal_modem_bcall.h"
#include "bcall_timer.h"
#include "bcall_mcu_manager.h"
#include "pm_server_api.h"
#include "modem_api.h"
#include "sr_param.h"

#define BCALL_FIRST_CONFIG_FILE_NAME ("/oemapp/config/bcall.ini")
#define BCALL_SECOND_CONFIG_FILE_NAME ("/oemdata/config/bcall.ini")

#define VIN_LENGTH (17)
#define BCALL_MSG_SUBCMD (0x9007)

#define CALL_NUM_LENGTH (20)

char TEST_CALL_NUM [BCALL_PHONE_NUM_LEN] = {0};

static EnuBcallStatus g_EnuBcallStatus = 0;

/**
* @brief 将事件控制命令发送到BCALL SERVER处理线程
*
* @param[in] BcallEventControlCmd 存储事件控制命令的结构体
* @return 返回发送消息的ID
*/
int32_t SendEventToBcallServer(StrBcallEventControlCmd_t *bcallEventControlCmd)
{
    int ret = 0;
    int32_t msgID = 0;
    const char *filepath = "/tmp/bcallKey";
    StrBcallEventControlCmd_t localBcallEventControlCmd = {0};
    memcpy(&localBcallEventControlCmd,bcallEventControlCmd,sizeof(StrBcallEventControlCmd_t));

    int fd = open(filepath, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        LOG_Error("%s : open error : %s ",__func__,strerror(errno));
        return RET_ERR;
    }
    close(fd);
    key_t msgSendKey = ftok(filepath, 'B');
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
    LOG_Info("msg : %s",__FUNCTION__);
    localBcallEventControlCmd.SubCmd = BCALL_MSG_SUBCMD;
    ret = msgsnd(msgID,&localBcallEventControlCmd, sizeof(StrBcallEventControlCmd_t),0);
    return ret;
}
/**
* @brief 监听BCALL事件控制命令
*
* @param[in] bcallEventControlCmd 存储事件控制命令的结构体
* @return 返回发送消息的ID
*/
static int32_t BcallEventMonitor(StrBcallEventControlCmd_t *bcallEventControlCmd)
{   
    int32_t msgID = 0;
    int ret = 0;
    const char *filepath = "/tmp/bcallKey";

    int fd = open(filepath, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        LOG_Error("%s : open error : %s ",__func__,strerror(errno));
        return RET_ERR;
    }
    close(fd);
    key_t msgMonitorKey = ftok(filepath, 'B');
    if (msgMonitorKey == -1) {
        LOG_Error("%s : ftok error : %s ",__func__,strerror(errno));
        return RET_ERR;
    }
    msgID = msgget(msgMonitorKey, 0666 | IPC_CREAT);
    if (msgID < 0)
    {
        LOG_Error("%s :  msgget error : %s ", __func__,strerror(errno));
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s :  msgget successfull.", __func__);
    }

    LOG_Debug("wait mcu event.");
    ret = msgrcv(msgID,bcallEventControlCmd, sizeof(StrBcallEventControlCmd_t) ,BCALL_MSG_SUBCMD,0);
    if (ret < 0)
    {
        LOG_Error("%s : SendEventToBcallServer msgrcv failed %s ", __func__ ,strerror(errno));
    }
    LOG_Debug("%s : Rec bcallEventControlCmd ,bcallEventType : [%d] ,bcallTriggerType : [%d] ",
    __FUNCTION__,bcallEventControlCmd->bcallEventType,bcallEventControlCmd->bcallTriggerType);
    return RET_OK;    
}
/**
* @brief 处理BCALL事件
*
* @param[in] event BCALL事件类型
* @return 无
*/

static void BcallEventStateCallBack(EnuBcallEventId_e event)
{
    StrBcallEventControlCmd_t bcallEventControlCmd = {0};
    switch (event) 
    {
        case BCALL_STATUS_INCOMING:
            SendMsgToAllQueue(SYS_QUE_TYPE_BCALL_INCOMING_NOTIFY, NULL, 0);
            usleep(500000);
            PmSetSleepEvent(PM_SLEEP_EVENT_ECALL);
            break;
        case BCALL_STATUS_CALLING:
            PmSetSleepEvent(PM_SLEEP_EVENT_ECALL);
            break;
        case BCALL_STATUS_END:
            PmCleanSleepEvent(PM_SLEEP_EVENT_ECALL);
            break;
        case BCALL_STATUS_ABNORMAL_END:
            break;
        case BCALL_STATUS_WAIT_PSAP:
            g_EnuBcallStatus = event;
            SendMcuBcallStatusSync(g_EnuBcallStatus);
            PmCleanSleepEvent(PM_SLEEP_EVENT_ECALL);
            break;
        case BCALL_STATUS_SNED_MSD:
            break;
        case BCALL_STATUS_PUSH_MSD:
            // 推动状态
            bcallEventControlCmd.bcallEventType = BCALL_TYPE_PUSH_MSD;
            SendEventToBcallServer(&bcallEventControlCmd);
            break;
        case BCALL_STATUS_UPDATE_MSD:
            // 推动状态
            bcallEventControlCmd.bcallEventType = BCALL_TYPE_UPDATE_MSD;
            SendEventToBcallServer(&bcallEventControlCmd);
            break;
        case BCALL_STATUS_MSD_SNED_SUCCESS:
            break;
        case BCALL_STATUS_MSD_SNED_FAULT:
            break;
        case BCALL_STATUS_ACTIVE:
            break; 
        default:
            break;
    }
    
    g_EnuBcallStatus = (EnuBcallStatus)event;
    SendMcuBcallStatusSync(g_EnuBcallStatus);
    LOG_Info("Ecall status :  %d ",g_EnuBcallStatus);
}
/**
* @brief 初始化BCALL配置
*
* @return 无
*/
static int8_t LoadBcallConfigFromfile(const char * firstFileName,const char * secondFileName)
{
    GKeyFile *keyfile;
    GError *error = NULL;
    gboolean success = FALSE;
    int8_t retValue = RET_OK;
    
    if(NULL == firstFileName || NULL == secondFileName)
    {
        LOG_Error("%s : parameter is NULL.",__FUNCTION__);
        return RET_ERR;
    }

    keyfile = g_key_file_new();

    // 先从首选配置文件中加载配置
    if (!g_key_file_load_from_file(keyfile, firstFileName, G_KEY_FILE_NONE, &error))
    {
        LOG_Warn("Error loading first file: %s", error->message);
        g_error_free(error);
        error = NULL;
        
        // 如果首选配置文件加载失败，再尝试从第二个默认配置文件中加载配置
        if (!g_key_file_load_from_file(keyfile, secondFileName, G_KEY_FILE_NONE, &error))
        {
            LOG_Error("Error loading second file: %s", error->message);
            g_error_free(error);
            g_key_file_free(keyfile);
            return RET_ERR;
        }
        else 
        {
            success = TRUE;
            LOG_Debug("load second file %s", secondFileName);
        }
    } 
    else 
    {
        success = TRUE;
        LOG_Debug("load first file %s", firstFileName);
    }

    if (success) {
        // 读取基本配置
        gchar *value = NULL;
        value = g_key_file_get_string(keyfile, "default","bcallNum" , NULL);
        if(value)
        {
            snprintf(TEST_CALL_NUM, BCALL_PHONE_NUM_LEN, value);
            LOG_Debug("bcall config : bcallNumber(%s) get success!",TEST_CALL_NUM);
        }
        else
        {
            LOG_Warn("bcall config : bcallNumber(NULL) get failed!");
            retValue = RET_ERR;
        }
    }

    // 释放 GKeyFile 结构
    g_key_file_free(keyfile);

    return retValue;
}
static void BcallConfigInit(void)    
{
    int8_t retValue = RET_OK;
    //配置方式待定

    retValue = LoadBcallConfigFromfile(BCALL_FIRST_CONFIG_FILE_NAME,BCALL_SECOND_CONFIG_FILE_NAME);
    if(RET_ERR == retValue)
    {
        //没有配置或是配置不成功设置为112
        memcpy(TEST_CALL_NUM,"112",strlen("112"));
        LOG_Notice("%s : no config file set.give default bcall num : [%s].",__FUNCTION__,TEST_CALL_NUM);
    }
    
}

/**
* @brief 将事件控制命令转换为MSD消息
*
* @param[in] bcallEventControlCmd 存储事件控制命令的结构体
* @param[in] msdInfo MSD消息结构体
* @param[in] msdLen MSD消息长度
* @return 返回MSD消息结构体
*/

static StrBcallVoiceInfo AssembleBcallVoiceInfo(StrBcallEventControlCmd_t *bcallEventControlCmd)
{
    StrBcallVoiceInfo bcallVoiceInfo = {0};
    
    memcpy(bcallVoiceInfo.PhoneNumber,TEST_CALL_NUM,BCALL_PHONE_NUM_LEN);

    return bcallVoiceInfo;
}

/**
* @brief 处理BCALL服务器回调
*
* @param[in] subcmd 子命令
* @param[in] reqData 请求数据
* @param[in] reqLen 请求数据长度
* @param[out] respData 响应数据
* @param[out] respLen 响应数据长度
* @param[in] userdata 用户数据
* @return 无
*/
static void BcallServerCallBack(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
    *respLen = sizeof(g_EnuBcallStatus);
    memcpy(respData,&g_EnuBcallStatus,sizeof(g_EnuBcallStatus));
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

    //休眠的相关逻辑
    
}


/**
* @brief BCALL SERVER 线程
*
* @param[in] pArg 用户数据
* @return 无
*/
gpointer BcallServer(gpointer pArg)
{
    StrBcallEventControlCmd_t bcallEventControlCmd = {0};
    StrBcallVoiceInfo bcallVoiceInfo = {0};
    int32_t ReturnValue = 0;

    LOG_Debug("%s init.",__FUNCTION__);
    SubscriptionQueueMsg(SYS_QUE_TYPE_GNSS_LOCATIONINFO, GnssMsgSubCallBack, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, PmMsgSubCallBack, NULL);
    RpcStartMsgServer(RPCSERVER_TYPE_BCALLSERVER, BcallServerCallBack,2, NULL);
    HalBcallVoiceInit(BcallEventStateCallBack);
    BcallConfigInit();  //加载配置文件配置
    
    HalBcallAudioPrepare();
    
    while(1)
    {
        memset(&bcallEventControlCmd, 0, sizeof(StrBcallEventControlCmd_t));
        if(RET_OK != BcallEventMonitor(&bcallEventControlCmd))
        {
            continue;
            usleep(50*1000);
        }
        
        //状态机
        switch(bcallEventControlCmd.bcallEventType)
        {
            case BCALL_TYPE_START_ECALL:
                bcallVoiceInfo = AssembleBcallVoiceInfo(&bcallEventControlCmd);
                //数据拨号
                ReturnValue = HalBcallVoiceStart(&bcallVoiceInfo);
                LOG_Info("%s:  HalEcallVoiceStart is action,return value : %d .",__FUNCTION__,ReturnValue);
                break;
            case BCALL_TYPE_HUANGUP:
                HalBcallVoiceHangup();
                break;
            case BCALL_TYPE_UPDATE_MSD:
                bcallVoiceInfo = AssembleBcallVoiceInfo(&bcallEventControlCmd);
                HalBcallVoiceUpdateMsd(&bcallVoiceInfo);
                break;
            case BCALL_TYPE_PUSH_MSD:
                HalBcallVoicePushMsd();
                break;
            default:
                LOG_Warn("%s : bcallEventType error  ", __func__);
                break;
        }
    }

    HalBcallVoiceDeinit();
    return NULL;
}

