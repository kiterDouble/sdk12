#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <glib.h>

#include "sr_ipc.h"
#include "sr_log.h"
#include "err_code.h"
#include "mcu_api.h"
#include "era_nad_manager.h"
#include "era_mcu_manager.h"

#define MCU_ACK_LENGTH (2)

#define MCU_ECALL_EVENT_TRIGGER_RESULT (1)
#define MCU_ECALL_STATUS_SYNC (2)       /* 同步状态 */

int32_t SendMcuEcallAmpGain(int gain)
{
    int32_t retValue = RET_OK;
    uint8_t payLoad[1] = {0};
    McuMesg_t McuMsg = {0};

    if(gain <= 0)
        gain = 0;
    else if(gain >= 4)
        gain = 4;

    /* ecall AMP增益调节 */
    McuMsg.type = SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_AMP_GAIN;
    McuMsg.payloadLen = 1;
    payLoad[0] = (uint8_t)gain;
    McuMsg.payload = payLoad;
    if(RET_ERR == McuSrvSendMesg(&McuMsg))
    {
        retValue = RET_ERR;
        LOG_Error(" %s:McuSrvSendMesg faild, gain=%d", __func__, gain);
    }
    LOG_Debug(" %s:McuSrvSendMesg gain = %d, scucess!", __func__, gain);

    return retValue;

}
/**
* @brief 发送ECALL状态同步
*
* @param ecallStatus EnuEcallStatus中的状态类型
* @return int32_t 返回值，如果发送成功则返回RET_OK，否则返回RET_ERR
*/
int32_t SendMcuEcallStatusSync(EnuEcallStatus eraStatus)
{
    int32_t retValue = RET_OK;
    uint8_t payLoad[MCU_ACK_LENGTH] = {0};
    McuMesg_t McuMsg = {0};
    
                    /* ecall状态同步 */
    McuMsg.type = SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_RES;
    McuMsg.payloadLen = MCU_ACK_LENGTH;
    payLoad[0] = MCU_ECALL_STATUS_SYNC;
    payLoad[1] = eraStatus;   //EnuEcallStatus 中状态类型
    McuMsg.payload = payLoad;
    if(RET_ERR == McuSrvSendMesg(&McuMsg))
    {
        retValue = RET_ERR;
        LOG_Error(" %s:McuSrvSendMesg faild!", __func__);
    }
    LOG_Debug(" %s:McuSrvSendMesg scucess!", __func__);

    return retValue;
}
/**
* @brief 发送ECALL事件确认
*
* @param ack 确认状态（1：成功，0：失败）
* @return int32_t 返回值，如果发送成功则返回RET_OK，否则返回RET_ERR
*/
int32_t SendMcuEcallEventAck(uint8_t ack)
{   
    int32_t retValue = RET_OK;
    uint8_t payLoad[MCU_ACK_LENGTH] = {0};
    McuMesg_t McuMsg = {0};
    
    McuMsg.type = SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_RES;
    McuMsg.payloadLen = MCU_ACK_LENGTH;
    payLoad[0] = MCU_ECALL_EVENT_TRIGGER_RESULT;
    payLoad[1] = ack; //1:success,0;faild
    McuMsg.payload = payLoad;
    if(RET_ERR == McuSrvSendMesg(&McuMsg))
    {
        retValue = RET_ERR;
        LOG_Error(" %s:McuSrvSendMesg faild!", __func__);
    }
    LOG_Debug(" %s:McuSrvSendMesg scucess!", __func__);

    return retValue;
}
/**
* @brief 接收ECALL事件控制回调
*
* @param data 接收到的ECALL事件控制命令数据
* @param len ECALL事件控制命令数据的长度
* @param userdata 回调函数传递的用户数据
* @return void
*/
static void McuMsgSubCallBack(const uint8_t* data, const uint16_t len, void* userdata)
{
    StrEcallEventControlCmd_t ecallEventControlCmd;
    
    memset(&ecallEventControlCmd, 0x00, sizeof(StrEcallEventControlCmd_t));
    ecallEventControlCmd.ecallEventType = ECALL_TYPE_START_ECALL;
    if(sizeof(uint8_t) == len)
    {
        
        ecallEventControlCmd.ecallTriggerType = *data;
        if(ecallEventControlCmd.ecallTriggerType < 0 || ecallEventControlCmd.ecallTriggerType > 4)
        {
            LOG_Error("%s: SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_REQ, trigger type :[%d]",__func__,ecallEventControlCmd.ecallTriggerType);
        }
        else
        {
            SendMcuEcallEventAck(1);
            SendEventToEcallServer(&ecallEventControlCmd);
            LOG_Info("%s: SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_REQ, trigger type :[%d]",__func__,ecallEventControlCmd.ecallTriggerType);
        }
    }
    else
    {
        LOG_Error("%s: SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_REQ sub msg len error!",__func__);
    }
}

static void McuAlarmECallCallBack(const uint8_t* data, const uint16_t len, void* userdata)
{
//    LOG_DebugHex(data, len, "recvmsg : SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_CON_SERVICE"); 

    ECallMCUSync(SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_CON_SERVICE, data, len);
}

static void McuSyncKLineCallBack(const uint8_t* data, const uint16_t len, void* userdata)
{
//    LOG_DebugHex(data, len, "recvmsg : SYS_QUE_TYPE_MCU_SYNC_KLINE"); 

    ECallMCUSync(SYS_QUE_TYPE_MCU_SYNC_KLINE, data, len);
}

static void McuAmpGainECallCallBack(const uint8_t* data, const uint16_t len, void* userdata)
{
    LOG_DebugHex(data, len, "recvmsg : SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_AMP_GAIN"); 
}

/**
 * @brief MCU服务器的主函数。
 *
 * 该函数初始化了服务器所需的组件，启动了一个消息接收线程，并订阅了MCU的消息队列。
 * 
 *
 * @param pArg 传递的额外参数指针（在本例中未使用）。
 * @return 函数的返回值（在本例中始终为RET_OK）。
 */
gpointer McuServer(gpointer pArg)
{  
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_REQ, McuMsgSubCallBack, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_AMP_GAIN, McuAmpGainECallCallBack, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_KLINE, McuSyncKLineCallBack, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_ALARM_ECALL_EVENT_CON_SERVICE, McuAlarmECallCallBack, NULL);

    return NULL;
}

