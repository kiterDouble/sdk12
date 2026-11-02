#include "modem_sms.h"
#include "modem_sms_record_db_api.h"
#include "msg_queue.h"

static MsgQueue_t g_modemSmsRecordMsgQueue = {0};
static volatile int g_modemSmsStoreTaskExit = 0;    /**< 短信存储线程退出标志，1=请求退出 */
static GThread *g_modemSmsStoreThread = NULL;       /**< 短信存储线程句柄，用于反初始化时 join */

void* ModemSrvSmsStoreTaskdFunc(void* arg)
{
    (void)arg;
    SmsRecordDbCtx_t smsRecordDbCtx = {0};
    Msg_t smsMsg = {0};
    ModemSrvSms_t* sms = NULL;
    int32_t smsStoredNum = 0;
    int ret = 0;

    /* 初始化短信存储数据库；收到退出请求时立即结束循环 */
    while(!g_modemSmsStoreTaskExit)
    {
        if(g_smsRecordDbApi.init(&smsRecordDbCtx, MODEM_SMS_RECORD_DB_PATH) == 0)
        {
            LOG_Info("[%s] smsRecord database init success", __FUNCTION__);
            break;
        }
        else
        {
            LOG_Error("[%s] smsRecord database init failed", __FUNCTION__);
        }
        sleep(2);
    }

    /* 初始化短信存储事件队列；收到退出请求时立即结束循环 */
    while(!g_modemSmsStoreTaskExit)
    {
        if(MsgQueueInit(&g_modemSmsRecordMsgQueue, 2) == 0)
        {
            LOG_Info("[%s] smsRecord MsgQueue init success", __FUNCTION__);
            break;
        }
        else
        {
            LOG_Error("[%s] smsRecord MsgQueue init failed", __FUNCTION__);
        }
        sleep(2);
    }

    /* 主循环：接收短信存储事件并写入数据库，直到收到退出请求 */
    while(!g_modemSmsStoreTaskExit)
    {
        /* 接收短信存储事件 */
        ret = MsgQueueRecv(&g_modemSmsRecordMsgQueue, &smsMsg, 1000);
        if(ret == 0)
        {
            if (smsMsg.len == sizeof(ModemSrvSms_t))
            {
                sms = (ModemSrvSms_t*)smsMsg.data;
                switch(smsMsg.type){
                    case E_SMS_RECORD_TYPE_RECEIVED:
                    case E_SMS_RECORD_TYPE_SENT:
                        {
                            LOG_Warn("[%s] sms %s", __FUNCTION__, (smsMsg.type == E_SMS_RECORD_TYPE_RECEIVED) ? "receive" : "send");
                            LOG_Warn("[%s] number:%s msgLen:%d", __FUNCTION__, sms->sourceAddr, sms->msgLen);

                            ret = g_smsRecordDbApi.insert(&smsRecordDbCtx, smsMsg.type, sms->sourceAddr, sms->msgContent, sms->msgLen);
                            LOG_Warn("[%s] sms stored ret:%d", __FUNCTION__, ret);
                            if (ret == 0)
                            {
                                smsStoredNum = g_smsRecordDbApi.queryCount(&smsRecordDbCtx, E_SMS_RECORD_TYPE_ALL);
                                LOG_Warn("[%s] sms stored num:%d", __FUNCTION__, smsStoredNum);

                                if (smsStoredNum >= (MODEM_SMS_RECORD_KEEP_NUM+20))
                                {
                                    LOG_Warn("[%s] trim sms stored to:%d", __FUNCTION__, MODEM_SMS_RECORD_KEEP_NUM);
                                    ret = g_smsRecordDbApi.trim(&smsRecordDbCtx, MODEM_SMS_RECORD_KEEP_NUM);
                                    LOG_Warn("[%s] trim sms stored ret:%d", __FUNCTION__, ret);
                                }
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
            MsgFree(&smsMsg);
        }
    }

    /* 线程退出前释放资源：关闭 sqlite3 句柄并销毁消息队列 */
    (void)g_smsRecordDbApi.deinit(&smsRecordDbCtx);
    MsgQueueDeinit(&g_modemSmsRecordMsgQueue);
    g_modemSmsRecordMsgQueue.inited = 0;
    return NULL;
}

/**
 * @name ModemSrvSmsRecvHandle
 * @brief 接收到短信处理函数
 *        该函数负责处理来自Modem的短信接收事件，将短信内容和来源信息封装成消息，
 *        然后将该消息发送到所有相关队列中。
 * @param mesg 指向包含短信信息的结构体的指针。该结构体包括短信内容、长度和来源地址。
 * @return 函数无返回值
 */
void ModemSrvSmsRecvHandle(HalModemSmsInfo_t *mesg)
{
    ModemSrvSms_t sms;
    if (!mesg)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return;
    }

    (void)memset(&sms, 0x0, sizeof(ModemSrvSms_t));
    sms.msgLen = mesg->msgLen;
    (void)memcpy(sms.msgContent, mesg->msgContent, (unsigned int)mesg->msgLen);
    (void)memcpy(sms.sourceAddr, mesg->sourceAddr, sizeof(mesg->sourceAddr));
    (void)SendMsgToAllQueue(SYS_QUE_TYPE_MODEM_SMS_NOTIFY, &sms, sizeof(ModemSrvSms_t));
    /** 接收短信存储 **/
     LOG_Warn("%s addr:%s msgLen:%d", __FUNCTION__, sms.sourceAddr, sms.msgLen);
    (void)MsgQueueSend(&g_modemSmsRecordMsgQueue, E_SMS_RECORD_TYPE_RECEIVED, (void*)&sms, sizeof(ModemSrvSms_t));
}

/**
 * @name ModemSrvSmsInit
 * @brief 初始化Modem的SMS服务。
 *        该函数负责初始化Modem的SMS处理机制，确保能够接收和处理SMS消息。
 * @param 无
 * @return int32_t 返回初始化操作的结果状态码。成功时返回0，失败时返回错误码。
 */
int32_t ModemSrvSmsInit(void)
{
    /* 创建短信存储线程；保留线程句柄以便反初始化时 join */
    g_modemSmsStoreTaskExit = 0;
    g_modemSmsStoreThread = g_thread_new("SmsStoreTask", ModemSrvSmsStoreTaskdFunc, NULL);
    if (g_modemSmsStoreThread == NULL)
    {
        LOG_Error("[%s] Failed to create SmsStoreTask thread", __FUNCTION__);
    }

    return HalModemSmsInit(ModemSrvSmsRecvHandle);
}

/**
 * @name ModemSrvSendSmsMesgProc
 * @brief 发送SMS消息处理函数。
 * @param reqData 指向请求数据的指针，包含要发送的SMS消息的信息。
 * @param reqLen 请求数据的长度。
 * @param respData 指向响应数据的指针，用于存储操作结果和响应数据的长度。
 * @param respLen 指向响应数据长度的指针，用于输出实际的响应数据长度。
 * @param userdata 用户数据，通常用于传递额外的上下文信息，本函数中用于访问Modem服务数据。
 * @return 操作结果，RET_OK表示成功，其他值表示失败。
 */
int32_t ModemSrvSendSmsMesgProc(const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
    ModemSrvData_t *srvData = (ModemSrvData_t *) userdata;
    ModemSrvRpcRes_t *res = (ModemSrvRpcRes_t *)((void*)respData);
    int32_t ret = RET_ERR;

    if (!srvData || ((reqLen > 0U) && !reqData) || !res || !respLen)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    res->result = RET_ERR;
    res->dataLen = 0;
    *respLen = (uint16_t)sizeof(ModemSrvRpcRes_t);

    if (srvData->jobs[E_MODEM_SRV_JOB_TYPE_SMS].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
    {
        LOG_Error("%s not initialed.", __FUNCTION__);
        return ret;
    }

    const ModemSrvSms_t *const sms = (const ModemSrvSms_t *const)((const void *const)reqData);

    res->result = HalModemSmsSend(sms->sourceAddr, sms->msgContent, (uint8_t)sms->msgLen);

    if (RET_OK != res->result)
    {
        LOG_Error("%s HalModemSmsSend fail.", __FUNCTION__);
        return ret;
    }

    /** 发送短信存储 **/
     LOG_Warn("%s addr:%s msgLen:%d", __FUNCTION__, sms->sourceAddr, sms->msgLen);
    (void)MsgQueueSend(&g_modemSmsRecordMsgQueue, E_SMS_RECORD_TYPE_SENT, (void*)sms, sizeof(ModemSrvSms_t));

    ret = RET_OK;
    return ret;
}

/**
 * @name ModemSrvSmsDeinit
 * @brief 反初始化 Modem 短信服务。
 *        停止短信存储线程并释放相关资源。
 * @return 操作结果，RET_OK 表示成功，RET_ERR 表示失败。
 */
int32_t ModemSrvSmsDeinit(void)
{
    int32_t ret = HalModemSmsDeinit();
    if (ret != RET_OK)
    {
        LOG_Error("[%s] HalModemSmsDeinit fail.", __FUNCTION__);
    }

    /* 设置退出标志并等待存储线程结束；线程内会自行释放 DB 与队列资源 */
    g_modemSmsStoreTaskExit = 1;
    if (g_modemSmsStoreThread != NULL)
    {
        g_thread_join(g_modemSmsStoreThread);
        g_modemSmsStoreThread = NULL;
    }

    return ret;
}
