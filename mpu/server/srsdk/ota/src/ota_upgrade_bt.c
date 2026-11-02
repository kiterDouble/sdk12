/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-09-04 10:32:19
 * @LastEditTime: 2019-09-04 10:40:17
 * @LastEditors: Please set LastEditors
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sr_log.h"
#include "err_code.h"
#include "ota_upgrade_bt.h"
#include "sr_ipc.h"
#include "ota_bin_parser.h"
#include "mcu_api.h"


#define   OTA_RECV_BT_DATA_MAX_SIZE        (1024)
#define   OTA_SEND_BT_FLASH_DATA_LEN       (240)
#define   OTA_RECV_BT_DATA_TIME_OUT        (2)

#define   OTA_BT_APP1_NAME            "bt_1.bin"
#define   OTA_BT_APP2_NAME            "bt_2.bin"

#define   RET_OTA_RECV_BT_MSG_TIMEOUT      (RET_ERR - 1)


typedef struct
{
    GMutex lock;
    GCond cond;
    uint32_t msgType;
    uint32_t dataLen;
    uint16_t sendCount;
    uint8_t seed[4];
    uint8_t data[OTA_RECV_BT_DATA_MAX_SIZE];
}BtMsgArg_t;

typedef int32_t (*UpgradeBtCallback)();
typedef struct BtUpgradeProcess
{
    uint8_t step;
    UpgradeBtCallback pCallback;
} UpgradeBtProcess_t;


static BtMsgArg_t g_btArgs;
static uint32_t g_flashPacketTotalCount = 0;  // 升级包发送总计数

/**
 * @name SetRecvBtDataToUpgradeBtProcess
 * @brief 向BT发送升级数据
 * 
 * @param type 消息类型，用于标识发送的数据的种类。
 * @param recvData 指向要发送数据缓冲区的指针。
 * @param dataLen 要发送数据的长度，以字节为单位。
 * @return 函数始终返回RET_OK，表示操作成功。
 */
int32_t SendUpgradeDataToBt(SYSQueMsgType_e type , const uint8_t *recvData, uint32_t dataLen)
{
    McuMesg_t msg;
    msg.type = type;
    msg.payloadLen = dataLen;
    msg.payload = (uint8_t *)recvData;
    McuSrvSendMesg(&msg);
    return RET_OK;
}
/**
 * @name SetRecvBtDataToUpgradeBtProcess
 * @brief 将接收到的BT数据设置为升级BT进程的状态。
 * 
 * @param recvData 指向接收到的BT数据的指针。
 * @param dataLen 接收到的BT数据的长度。
 * @param type 消息类型，指示如何处理这些数据。
 * @return 若成功，则返回RET_OK；若出错，则返回RET_ERR。
 */
int32_t SetRecvBtDataToUpgradeBtProcess(const uint8_t *recvData, uint32_t dataLen, SYSQueMsgType_e type)
{
    if(NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    if(dataLen > OTA_RECV_BT_DATA_MAX_SIZE || dataLen == 0)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    g_mutex_lock(&g_btArgs.lock);
    g_btArgs.msgType = type;
    memcpy(g_btArgs.data, recvData, dataLen);
    g_btArgs.dataLen = dataLen;
    g_cond_signal(&g_btArgs.cond);
    g_mutex_unlock(&g_btArgs.lock);

    return RET_OK;
}
/**
 * @name WaitUpgradeBtMsgTimeOut
 * @brief 等待BT升级消息，设定超时时间。
 *  
 * @param secTimeOut 超时时间，单位为秒。指定等待消息的最大时间限制。
 * @return 返回操作结果状态码。如果在超时时间内接收到消息，则返回RET_OK；否则返回RET_ERR。
 */
int32_t WaitUpgradeBtMsgTimeOut(uint32_t secTimeOut)
{
    int32_t retValue = RET_ERR;
    gint64 endTIme = 0;

    //g_mutex_lock(&g_btArgs.lock);
    
    endTIme = g_get_monotonic_time() + G_TIME_SPAN_SECOND * secTimeOut;
    if (g_cond_wait_until(&g_btArgs.cond, &g_btArgs.lock, endTIme))
    {
        retValue = RET_OK;
    }

    //g_mutex_unlock(&g_btArgs.lock);
    return retValue;
}

/**
 * @name UpgradeBtEraseFlashReq
 * @brief 向BT发送擦除Flash的请求。
 * 
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，其他值表示不同类型的错误。
 */
static int32_t UpgradeBtEraseFlashReq()
{
    int32_t  retValue  = RET_ERR;
    int32_t  fileLen   = 0x00;

    fileLen = GetUpgradeBinFileContenLen();
    if(fileLen <= 0)
    {
        LOG_Error("%s  GetUpgradeBinFileContenLen error", __FUNCTION__);
        return RET_ERR;
    }

    g_mutex_lock(&g_btArgs.lock);
    g_btArgs.msgType = SYS_QUE_TYPE_BT_UPGRADE_ERASE_FLASH;
    g_btArgs.sendCount++;
    g_btArgs.data[0] = (g_btArgs.sendCount >> 8) & 0xFF;
    g_btArgs.data[1] = (g_btArgs.sendCount) & 0xFF;
    g_btArgs.data[2] = 0x01;
    g_btArgs.data[3] = 0x02;
    g_btArgs.data[4] = 0x03;
    g_btArgs.data[5] = 0x04;
    g_btArgs.dataLen = 6;
    SendUpgradeDataToBt(g_btArgs.msgType, g_btArgs.data, g_btArgs.dataLen);
    LOG_DebugHex(g_btArgs.data, g_btArgs.dataLen, "BT Data");
    memset(g_btArgs.data, 0, OTA_RECV_BT_DATA_MAX_SIZE);
    g_btArgs.dataLen = 0;
   

    retValue = WaitUpgradeBtMsgTimeOut(OTA_RECV_BT_DATA_TIME_OUT);
    g_mutex_unlock(&g_btArgs.lock);
    if(retValue < RET_OK)
    {
        LOG_Error("wait bt msg error");
        return RET_OTA_RECV_BT_MSG_TIMEOUT;
    }
    if(g_btArgs.dataLen < 2)
    {
        LOG_Error(" bt msg  len error");
        return RET_OTA_RECV_BT_MSG_TIMEOUT;
    }
    LOG_DebugHex(g_btArgs.data, g_btArgs.dataLen, "Bt Data");
    if(g_btArgs.data[2] != 0x00)
    {
        LOG_Error("bt upgrade msg status error: 0x%02x", g_btArgs.data[2]);
        return RET_ERR;
    }

    return RET_OK;
}
/**
 * @name UpgradeBtFlashingReq
 * @brief 向BT发送刷写文件的请求。
 * 
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，其他值表示不同类型的错误。
 */

static int32_t UpgradeBtFlashingReq()
{
    int32_t readLen = 0;
    int32_t retValue = 0;

    g_mutex_lock(&g_btArgs.lock);
    g_btArgs.msgType = SYS_QUE_TYPE_BT_UPGRADE_FLASHING_BT;
    g_btArgs.sendCount++;
    g_btArgs.data[0] = (g_btArgs.sendCount >> 8) & 0xFF;
    g_btArgs.data[1] = (g_btArgs.sendCount) & 0xFF;

    // 累加计数并打印
    g_flashPacketTotalCount++;
    // LOG_Debug("[BT Upgrade] Flash packet count: %u", g_flashPacketTotalCount);

    readLen = GetBinFileContenLenFormList(&g_btArgs.data[2],
                                            OTA_RECV_BT_DATA_MAX_SIZE - 6,
                                            OTA_SEND_BT_FLASH_DATA_LEN);
    if(readLen < 0)
    {
        g_mutex_unlock(&g_btArgs.lock);
        return RET_ERR;
    }

    if(readLen == 0)
    {
        g_mutex_unlock(&g_btArgs.lock);
        return RET_ERR;
    }
    int tmpcount = 5;
    retValue = RET_ERR;
    while(tmpcount-- > 0 && retValue < RET_OK)
    {
        g_btArgs.dataLen = readLen + 2;
        LOG_DebugHex(g_btArgs.data, g_btArgs.dataLen, "Send Bt Data");
        SendUpgradeDataToBt(g_btArgs.msgType, g_btArgs.data, g_btArgs.dataLen);
        g_btArgs.dataLen = 0;
        
        retValue = WaitUpgradeBtMsgTimeOut(OTA_RECV_BT_DATA_TIME_OUT);
        LOG_Debug("tmpcount:%d", tmpcount);
    }

    LOG_Debug("Send Mcu Data OK, retValue:%d", retValue);
    
    g_mutex_unlock(&g_btArgs.lock);
    if(retValue < RET_OK)
    {
        LOG_Error("wait bt msg error");
        return RET_OTA_RECV_BT_MSG_TIMEOUT;
    }
    if(g_btArgs.dataLen < 2)
    {
        LOG_Error(" bt msg  len error");
        return RET_OTA_RECV_BT_MSG_TIMEOUT;
    }
    LOG_DebugHex(g_btArgs.data, g_btArgs.dataLen, "Bt Data");
    if(g_btArgs.data[2] != 0x00)
    {
        LOG_Error("bt upgrade msg status error: 0x%02x", g_btArgs.data[2]);
        return RET_ERR;
    }
    return RET_OK;
}


/**
 * @name UpgradeFlashCompleteReq
 * @brief 向BT发送升级完成的请求。
 * 
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，其他值表示不同类型的错误。
 */
static int32_t UpgradeFlashCompleteReq()
{
    int32_t  retValue  = RET_ERR;
    uint32_t crc32Value = 0x00;

    retValue = GetUpgradeBinFileCrc(&crc32Value);
    if(retValue < RET_OK)
    {
        LOG_Error("%s GetUpgradeBtFileCrc error", __FUNCTION__);
        return RET_ERR;
    }

    g_mutex_lock(&g_btArgs.lock);
    g_btArgs.msgType = SYS_QUE_TYPE_BT_UPGRADE_FLASH_COMPLETE;
    g_btArgs.sendCount++;
    g_btArgs.data[0] = (g_btArgs.sendCount >> 8) & 0xFF;
    g_btArgs.data[1] = (g_btArgs.sendCount) & 0xFF;


    g_btArgs.data[2] = (crc32Value >> 24) & 0xff;
    g_btArgs.data[3] = (crc32Value >> 16) & 0xff;
    g_btArgs.data[4] = (crc32Value >> 8) & 0xff;
    g_btArgs.data[5] = crc32Value & 0xff;
    
    g_btArgs.dataLen = 6;
    LOG_DebugHex(g_btArgs.data, g_btArgs.dataLen, "Send Bt Data");
    SendUpgradeDataToBt(g_btArgs.msgType, g_btArgs.data, g_btArgs.dataLen);
    memset(g_btArgs.data, 0, OTA_RECV_BT_DATA_MAX_SIZE);
    g_btArgs.dataLen = 0;
    

    retValue = WaitUpgradeBtMsgTimeOut(OTA_RECV_BT_DATA_TIME_OUT);
    g_mutex_unlock(&g_btArgs.lock);
    if(retValue < RET_OK)
    {
        LOG_Error("wait bt msg error");
        return RET_OTA_RECV_BT_MSG_TIMEOUT;
    }
    if(g_btArgs.dataLen < 2)
    {
        LOG_Error(" bt msg  len error");
        return RET_OTA_RECV_BT_MSG_TIMEOUT;
    }
    LOG_DebugHex(g_btArgs.data, g_btArgs.dataLen, "Bt Data");
    if(g_btArgs.data[2] != 0x00)
    {
        LOG_Error("bt upgrade msg status error: 0x%02x", g_btArgs.data[2]);
        return RET_ERR;
    }

    return RET_OK;
}

/**
 * @name UpgradeBtRestBtReq
 * @brief  请求BT复位请求。
 * 
 * @param 空
 * @return 函数返回一个整数。成功时返回RET_OK，否则返回RET_ERR或其他错误代码。
 */
static int32_t UpgradeBtRestBtReq()
{
    int32_t retValue = RET_ERR;

    g_mutex_lock(&g_btArgs.lock);
    g_btArgs.msgType = SYS_QUE_TYPE_BT_UPGRADE_RESET_BT;
    g_btArgs.sendCount++;
    g_btArgs.data[0] = 0x00;
    g_btArgs.data[1] = 0x01;
    g_btArgs.data[2] = 0x01;
    g_btArgs.data[3] = 0x02;
    g_btArgs.data[4] = 0x03;
    g_btArgs.data[5] = 0x04;
    g_btArgs.dataLen = 6;
    LOG_DebugHex(g_btArgs.data, g_btArgs.dataLen, "BT Data");
    SendUpgradeDataToBt(g_btArgs.msgType, g_btArgs.data, g_btArgs.dataLen);
    memset(g_btArgs.data, 0, OTA_RECV_BT_DATA_MAX_SIZE);
    g_btArgs.dataLen = 0;

    retValue = WaitUpgradeBtMsgTimeOut(OTA_RECV_BT_DATA_TIME_OUT);
    g_mutex_unlock(&g_btArgs.lock);
    if(retValue < RET_OK)
    {
        LOG_Error("wait bt msg error");
        return RET_OTA_RECV_BT_MSG_TIMEOUT;
    }
    if(g_btArgs.dataLen < 2)
    {
        LOG_Error(" bt msg  len error");
        return RET_OTA_RECV_BT_MSG_TIMEOUT;
    }
    LOG_DebugHex(g_btArgs.data, g_btArgs.dataLen, "Bt Data");
    if(g_btArgs.data[2] != 0x00)
    {
        LOG_Error("bt upgrade msg status error: 0x%02x", g_btArgs.data[2]);
        return RET_ERR;
    }

    return RET_OK;
}

/**
 * @name UpgradeBtRunAppReq
 * @brief  请求BT升级运行应用程序。
 * 
 * @param runStatus 指向一个字节的指针，用于接收BT运行状态的响应。
 * @return 函数返回一个整数。成功时返回RET_OK，否则返回RET_ERR或其他错误代码。
 */
static int32_t UpgradeBtRunAppReq(uint8_t *runStatus)
{
    int32_t retValue = RET_ERR;

    g_mutex_lock(&g_btArgs.lock);
    g_btArgs.msgType = SYS_QUE_TYPE_BT_UPGRADE_READY;
    g_btArgs.sendCount++;
    g_btArgs.data[0] = 0x00;
    g_btArgs.data[1] = 0x01;
    g_btArgs.data[2] = 0x01;
    g_btArgs.data[3] = 0x02;
    g_btArgs.data[4] = 0x03;
    g_btArgs.data[5] = 0x04;
    g_btArgs.dataLen = 6;
    LOG_DebugHex(g_btArgs.data, g_btArgs.dataLen, "BT Data");
    SendUpgradeDataToBt(g_btArgs.msgType, g_btArgs.data, g_btArgs.dataLen);
    memset(g_btArgs.data, 0, OTA_RECV_BT_DATA_MAX_SIZE);
    g_btArgs.dataLen = 0;

    retValue = WaitUpgradeBtMsgTimeOut(OTA_RECV_BT_DATA_TIME_OUT);
    g_mutex_unlock(&g_btArgs.lock);
    if(retValue < RET_OK)
    {
        LOG_Error("wait bt msg error");
        return RET_OTA_RECV_BT_MSG_TIMEOUT;
    }

    if(g_btArgs.dataLen < 3)
    {
        LOG_Error(" bt msg  len error");
        return RET_OTA_RECV_BT_MSG_TIMEOUT;
    }
    
    *runStatus = g_btArgs.data[2];

    return RET_OK;
}


/**
 * @name InitUpgradeBtResource
 * @brief 初始化升级BT资源
 *  
 * @参数: 无
 * @返回值: 返回RET_OK表示初始化成功，其他值表示初始化失败。
 */
int32_t InitUpgradeBtResource()
{
    memset(&g_btArgs, 0x0, sizeof(BtMsgArg_t));
    g_mutex_init(&g_btArgs.lock);
    g_cond_init(&g_btArgs.cond);

    return RET_OK;
}

/**
 * @name UpgradeBtEaseFlashCallback 
 * @brief 升级BT时，擦除Flash回调函数
 * 
 * @param recvData 指向接收到的数据的指针。该数据包含了擦除Flash的指令。
 * @param dataLen 接收到的数据长度。确保了处理过程不会误操作非指令数据。
 * @param userdata 用户自定义数据，可用于在回调中访问额外的上下文信息。
 */
void UpgradeBtEaseFlashCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    if(NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if(dataLen == 0)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_BT_UPGRADE_ERASE_FLASH);
    SetRecvBtDataToUpgradeBtProcess(recvData, dataLen, SYS_QUE_TYPE_BT_UPGRADE_ERASE_FLASH);
}
/**
 * @name UpgradeBtFlashBtCallback 
 * @brief 升级BT固件的回调函数
 * 
 * @param recvData 指向接收到的数据缓冲区的指针。
 * @param dataLen 接收到的数据长度。
 * @param userdata 用户自定义数据，可用于传递额外的上下文信息。
 */
void UpgradeBtFlashBtCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    if(NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if(dataLen == 0)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_BT_UPGRADE_FLASHING_BT);
    SetRecvBtDataToUpgradeBtProcess(recvData, dataLen, SYS_QUE_TYPE_BT_UPGRADE_FLASHING_BT);
}
/**
 * @name UpgradeBtFlashCompleteCallback 
 * @brief BT固件升级完成后的回调函数
 * 
 * @param recvData 指向接收到的数据缓冲区的指针
 * @param dataLen 接收到的数据长度
 * @param userdata 用户自定义数据，可用于传递额外的上下文信息
 */
void UpgradeBtFlashCompleteCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    if(NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if(dataLen == 0)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_BT_UPGRADE_FLASH_COMPLETE);
    SetRecvBtDataToUpgradeBtProcess(recvData, dataLen, SYS_QUE_TYPE_BT_UPGRADE_FLASH_COMPLETE);
}

/**
 * @name UpgradeBtResetBtCallback 
 * @brief 用于处理BT升级过程中重置BT的回调函数
 * 
 * @param recvData 接收到的数据的指针
 * @param dataLen 接收到的数据的长度
 * @param userdata 用户自定义数据
 */
void UpgradeBtResetBtCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    if(NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if(dataLen == 0)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_BT_UPGRADE_RESET_BT);
    SetRecvBtDataToUpgradeBtProcess(recvData, dataLen, SYS_QUE_TYPE_BT_UPGRADE_RESET_BT);
}
/**
 *  @name UpgradeBtGetRunAppCallback 
 *  @brief 升级BT获取运行应用程序的回调函数
 * 
 * @param recvData 指向接收到的数据的指针。不可为NULL。
 * @param dataLen 接收到的数据的长度。不可为0。
 * @param userdata 用户自定义数据，该函数中未使用。
 */
void UpgradeBtGetRunAppCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    if(NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if(dataLen == 0)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_BT_UPGRADE_READY);
    SetRecvBtDataToUpgradeBtProcess(recvData, dataLen, SYS_QUE_TYPE_BT_UPGRADE_READY);
}

static UpgradeBtProcess_t g_upgradeBtProcess[] = 
{
    {0, UpgradeBtEraseFlashReq},
    {1, UpgradeBtFlashingReq},
    {2, UpgradeFlashCompleteReq},
    {3, UpgradeBtRestBtReq}
};

/**
 *  @name InitUpgradeBtIpcCallback 
 *  @brief 初始化BT升级的IPC回调函数
 * 
 * 该函数用于初始化BT升级过程中的各个阶段的回调函数，通过向订阅队列中添加特定类型的消息，
 * 来指明在收到对应消息时应该调用哪个回调函数进行处理。
 * 
 * @return int32_t 返回操作的结果状态码，成功时返回RET_OK。
 */
static int32_t InitUpgradeBtIpcCallback()
{
    SubscriptionQueueMsg(SYS_QUE_TYPE_BT_UPGRADE_ERASE_FLASH, UpgradeBtEaseFlashCallback, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_BT_UPGRADE_FLASHING_BT, UpgradeBtFlashBtCallback, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_BT_UPGRADE_FLASH_COMPLETE, UpgradeBtFlashCompleteCallback, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_BT_UPGRADE_RESET_BT, UpgradeBtResetBtCallback, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_BT_UPGRADE_READY, UpgradeBtGetRunAppCallback, NULL);
    return RET_OK;
}
/**
 *  @name DestoryUpgradeBtIpcCallback 
 *  @brief 销毁升级BT IPC回调
 * 
 * 该函数用于取消订阅一系列与BT升级相关的消息队列消息，
 * 以确保在升级过程中或升级完成后，相关消息不再被处理。
 * 
 * @return int32_t 返回操作结果，若成功则为RET_OK。
 */
static int32_t DestoryUpgradeBtIpcCallback()
{
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_BT_UPGRADE_ERASE_FLASH);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_BT_UPGRADE_FLASHING_BT);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_BT_UPGRADE_FLASH_COMPLETE);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_BT_UPGRADE_RESET_BT);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_BT_UPGRADE_READY);
    return RET_OK;
}

/**
 * @name UpgradeBtProcess
 * @brief 升级BT处理函数
 * 
 * @param filePath 文件路径，用于指定升级包的位置
 * @return int32_t 返回RET_OK表示成功，RET_ERR表示失败
 */
int32_t UpgradeBtProcess(const char *filePath)
{
    //int32_t retValue = RET_ERR;
     int32_t retValue = 0;
    int32_t whileCount = 3; 
    int32_t callBackSize = sizeof(g_upgradeBtProcess) / sizeof(UpgradeBtProcess_t);
    int32_t callBackIndex = 0;
    uint8_t btRunStatus = 0;
    char upgradeFileName[256] = {0};

    if(NULL == filePath)
    {
        LOG_Error("%s arg is invalid", __FUNCTION__);
        return retValue;
    }

    LOG_Debug("start upgrade bt ");
    
    // 重置升级包计数器和文件读取位置
    g_flashPacketTotalCount = 0;
    ResetBinFileReadPosition();

    InitUpgradeBtResource();

    InitUpgradeBtIpcCallback();

    //升级BT
    do
    {
        LOG_Debug("upgrade get bt run app : %d", whileCount);
        whileCount--;
        retValue = UpgradeBtRunAppReq(&btRunStatus);
        if(retValue < 0)
        {
            LOG_Error("bt upgrade msg status error: 0x%02x", retValue);
            retValue = RET_ERR;
            continue;
        }
        LOG_Debug("bt run app : %d", btRunStatus);

        if(0x00 == btRunStatus)
        {
            snprintf(upgradeFileName, 256, "%s%s",filePath, OTA_BT_APP1_NAME);
            retValue = InitBinFileToList(upgradeFileName);
            if(retValue < 0)
            {
                DestoryUpgradeBtIpcCallback();
                LOG_Error("Init Bin File To List errror");
                LOG_Debug("stop upgrade bt ");
                return retValue;
            }
            whileCount = 3;
            retValue = RET_OK;
        }
    }
    while(whileCount > 0 && retValue < RET_OK);

    for(callBackIndex = 0; callBackIndex < callBackSize; callBackIndex++)
    {
        LOG_Debug("upgrade  bt %d  setp", callBackIndex);
        while(whileCount > 0)
        {
            retValue = g_upgradeBtProcess[callBackIndex].pCallback();
            LOG_Debug("callBackIndex: %d, retValue: %d, whileCount:%d ", callBackIndex, retValue, whileCount);
            if(1 == callBackIndex && RET_OK == retValue)
            {
                //此处只要大于等于2即可，每次返回成功继续赋值
                whileCount = 2;
            }
            else if(RET_OK == retValue)
            {
                whileCount = 3;
                break;
            }
            else if(1 == callBackIndex)
            {
                //此处返回失败直接退出，无充重新传输
                whileCount = 1;
                break;
            }
            LOG_Debug("callBackIndex: %d, retValue: %d, whileCount:%d ", callBackIndex, retValue, whileCount);
            whileCount--;
        }
    }

    DestroyBinListAndFree();
    DestoryUpgradeBtIpcCallback();
    LOG_Debug("stop upgrade bt ");
    LOG_Debug("[BT Upgrade] Total flash packets sent: %u", g_flashPacketTotalCount);

    return retValue;
}
