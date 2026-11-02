#include "modem_data.h"
#include "modem_call.h"
#include "modem_sms.h"
#include "modem_ant.h"
#include "modem_rpc.h"
#include "sr_ipc.h"
#include "pm_server_api.h"
#define MODEM_KEYFILE_INIT "/oemapp/config/modem.ini"


static MPUPmStateChange_t g_ModemPmState = {0};
/**
 * @name  ModemSrvParserIni
 * @brief 解析初始化配置文件的函数
 * @param srvData 指向ModemSrvData_t结构体的指针，用于存储解析后的配置数据
 * @return 返回操作的结果状态码。RET_ERR表示错误，RET_OK表示成功。
 */
static int32_t ModemSrvParserIni(ModemSrvData_t *srvData)
{
    int32_t ret = RET_ERR;

    if (!srvData)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }
    GError *error = NULL;
    GKeyFile *keyFile = g_key_file_new();

    if (!keyFile)
    {
        LOG_Error("%s g_key_file_new fail.", __FUNCTION__);
        return ret;        
    }
    // Load the key file
    if (!g_key_file_load_from_file(keyFile, MODEM_KEYFILE_INIT, G_KEY_FILE_NONE, &error))
    {
        LOG_Error("%s Failed to load key file: %s", __FUNCTION__, error->message);
        g_clear_error(&error);
        g_key_file_free(keyFile);
        return ret;
    }

    // Get an integer value from the key file
    int value = g_key_file_get_integer(keyFile, "ant", "short", &error);
    if (error != NULL)
    {
        LOG_Error("%s Failed to get value for key 'short': %s", __FUNCTION__, error->message);
        g_clear_error(&error);
    }
    else
    {
        srvData->cfg.antShort = value;
    }

    value = g_key_file_get_integer(keyFile, "ant", "normal", &error);
    if (error != NULL)
    {
        LOG_Error("%s Failed to get value for key 'normal': %s", __FUNCTION__, error->message);
        g_clear_error(&error);
    } 
    else
    {
        srvData->cfg.antNormal = value;
    }

    value = g_key_file_get_integer(keyFile, "ant", "normalMax", &error);
    if (error != NULL)
    {
        LOG_Error("%s Failed to get value for key 'normalMax': %s", __FUNCTION__, error->message);
        g_clear_error(&error);
    } 
    else
    {
        srvData->cfg.antNormalMax = value;
    }

    value = g_key_file_get_integer(keyFile, "ant", "open", &error);
    if (error != NULL)
    {
        LOG_Error("%s Failed to get value for key 'open': %s", __FUNCTION__, error->message);
        g_clear_error(&error);
    }
    else
    {
        srvData->cfg.antOpen = value;
    }

    value = g_key_file_get_integer(keyFile, "ant", "secShort", &error);
    if (error != NULL)
    {
        LOG_Error("%s Failed to get value for key 'secShort': %s", __FUNCTION__, error->message);
        g_clear_error(&error);
    }
    else
    {
        srvData->cfg.secAntShort = value;
    }

    value = g_key_file_get_integer(keyFile, "ant", "secNormal", &error);
    if (error != NULL)
    {
        LOG_Error("%s Failed to get value for key 'secNormal': %s", __FUNCTION__, error->message);
        g_clear_error(&error);
    } 
    else
    {
        srvData->cfg.secAntNormal = value;
    }

    value = g_key_file_get_integer(keyFile, "ant", "secNormalMax", &error);
    if (error != NULL)
    {
        LOG_Error("%s Failed to get value for key 'secNormalMax': %s", __FUNCTION__, error->message);
        g_clear_error(&error);
    } 
    else
    {
        srvData->cfg.secAntNormalMax = value;
    }

    value = g_key_file_get_integer(keyFile, "ant", "secOpen", &error);
    if (error != NULL)
    {
        LOG_Error("%s Failed to get value for key 'secOpen': %s", __FUNCTION__, error->message);
        g_clear_error(&error);
    }
    else
    {
        srvData->cfg.secAntOpen = value;
    }

    g_key_file_free(keyFile);
    ret = RET_OK;
    return ret; 
}

/**
 * @name  ModemSrvJobsInit
 * @brief 初始化ModemSrvData_t结构体中存储的ModemSrvJob_t结构体数组的函数
 * @param srvData 指向ModemSrvData_t结构体的指针，用于存储初始化后的数据
 * @return 返回操作的结果状态码。RET_ERR表示错误，RET_OK表示成功。
 */
static int32_t ModemSrvJobsInit(ModemSrvData_t *srvData)
{
    int32_t ret = RET_ERR;

    if (!srvData)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    uint32_t i = E_MODEM_SRV_JOB_TYPE_SIM;

    for (; i < E_MODEM_SRV_JOB_TYPE_MAX; i++)
    {
        if (srvData->jobs[i].status == E_MODEM_SRV_JOB_STATUS_UNINITIALED)
        {
            int32_t retVal = srvData->jobs[i].initCb();
            if (retVal == RET_ERR)
            {
                continue;
            }
            pthread_mutex_lock(&srvData->mutex);
            srvData->jobs[i].status = E_MODEM_SRV_JOB_STATUS_INITIALED;
            pthread_mutex_unlock(&srvData->mutex);
        }
    }
    ret = RET_OK;
    return ret;
}

/**
 * @name  ModemSrvCheckNotify
 * @brief 检查ModemSrvData_t结构体中存储的ModemSrvNotify_t结构体中的数据，并更新ModemSrvData_t结构体中存储的ModemSrvNotify_t结构体中的数据
 * @param srvData 指向ModemSrvData_t结构体的指针，用于存储需要检查的数据
 * @return 返回操作的结果状态码。RET_ERR表示错误，RET_OK表示成功
 */
static int32_t ModemSrvCheckNotify(ModemSrvData_t *srvData)
{
    int32_t ret = RET_ERR;
    static time_t lastTime = 0;
    time_t curr;

    if (!srvData)
    {
        LOG_Error("%s param invalid.", __FUNCTION__);
        return ret;
    }

    if (0 == lastTime)
    {
       lastTime = time(NULL);
    }
    if (PM_STATE_SLEEP == g_ModemPmState.PmState)
    {
        //LOG_Error("%s modem is sleep.", __FUNCTION__);
        return ret;
    }
    HalModemSimCardStatus_e cardStat = srvData->notify.simCardStatus;
    HalModemSimAppStatus_e appStat = srvData->notify.simAppStatus;
    HalModemNwRegStatus_e retStatus = srvData->notify.dataRegStatus;
    HalModemAntStatus_e status = srvData->notify.antennaStatus;
    HalModemAntStatus_e secondaryStatus = srvData->notify.secondaryAntennaStatus;

    HalModemSimGetCardStatus(&cardStat, &appStat);
    HalModemNwGetRegStatus(&retStatus);
    ModemSrvAntGetMainStatus(srvData, &status);
    ModemSrvAntGetSecondaryStatus(srvData, &secondaryStatus);
    HalModemNwGetCsq(&srvData->notify.csq);
    srvData->notify.simCardStatus = cardStat;
    srvData->notify.simAppStatus = appStat;
    srvData->notify.dataRegStatus = retStatus;
    srvData->notify.antennaStatus = status;
    srvData->notify.secondaryAntennaStatus = secondaryStatus;
    ret = RET_OK;

    curr = time(NULL);
    if (curr > lastTime)
    {
        SendMsgToAllQueue(SYS_QUE_TYPE_MODEM_SIGNAL_NOFIFY, &srvData->notify, sizeof(ModemSrvNotify_t));
        LOG_Debug("cardStat:%d,appStat:%d,retStatus:%d,antstatus:%d,secAntStatus:%d,csq:%d,datalen:%d", 
                srvData->notify.simCardStatus,
                srvData->notify.simAppStatus,
                srvData->notify.dataRegStatus,
                srvData->notify.antennaStatus,
                srvData->notify.secondaryAntennaStatus,
                srvData->notify.csq,sizeof(ModemSrvNotify_t));
        lastTime = curr;
    }
    
    return ret;
}

/**
 * @name ModemGetPmPowerState
 * @brief 获取电源状态
 * @param data 指向接收到的数据的指针。
 * @param len 接收到的数据长度。
 * @param userdata 用户自定义数据，本函数未使用此参数。
 * @return 无返回值
 */
void ModemGetPmPowerState(const uint8_t* data, const uint16_t len, void* userdata)
{
    if(sizeof(MPUPmStateChange_t) == len && NULL != data)
    {
        memcpy(&g_ModemPmState, data, len);
    }
    else
    {
        LOG_Debug("%s len error!",__FUNCTION__);
    }
}
/**
 * @name  ModemSrvMain
 * @brief ModemSrv主函数
 * @return 无返回值
 */
int main(int argc, char *argv[])
{
    LOG_INIT("modemServer");
    int32_t count = 0;
    static ModemSrvData_t srvData = 
    {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .notify = {E_MODEM_SRV_SIM_CARD_STATUS_ABSENT, 
                E_MODEM_SRV_SIM_APP_STATUS_ERROR, 
                E_MODEM_SRV_NW_REG_STATUS_UNREGISTERED, 
                99, E_MODEM_SRV_ANT_STATUS_CIRCUIT_NORMAL},
        .cfg = {
            .antShort = 300,
            .antNormal = 1350,
            .antNormalMax = 1750,
            .antOpen = 2200,
            .secAntShort = 300,
            .secAntNormal = 1350,
            .secAntNormalMax = 1750,
            .secAntOpen = 2200,
        },
        .jobs = {
            {E_MODEM_SRV_JOB_STATUS_UNINITIALED, HalModemSimInit, HalModemSimDeinit},
            {E_MODEM_SRV_JOB_STATUS_UNINITIALED, HalModemNwInit, HalModemNwDeinit},
            {E_MODEM_SRV_JOB_STATUS_UNINITIALED, HalModemDmsInit, HalModemDmsDeinit},
            {E_MODEM_SRV_JOB_STATUS_UNINITIALED, ModemSrvCallInit, HalModemCallDeInit},
            {E_MODEM_SRV_JOB_STATUS_UNINITIALED, ModemSrvSmsInit, HalModemSmsDeinit},
            {E_MODEM_SRV_JOB_STATUS_UNINITIALED, HalModemAntInit, HalModemAntDeinit},
        }
    };

    StartQueueMsgRcvThread("modemServer_ipc");
    if (RET_ERR == ModemSrvParserIni(&srvData))
    {
        LOG_Error("%s ModemSrvParserIni fail.", __FUNCTION__);
        return -1;
    }

    SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY,ModemGetPmPowerState,NULL);
    RpcStartMsgServer(RPCSERVER_TYPE_MODEMSERVER, ModemSrvRpcProcCallback, 1, (void*)&srvData);

    while (1)
    {
        ModemSrvJobsInit(&srvData);
        ModemSrvAntCheck(&srvData);
        ModemSrvCheckNotify(&srvData);
        usleep(500000);
    }
    return 0;
}
