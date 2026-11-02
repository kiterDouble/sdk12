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
#include "ota_upgrade_mcu.h"
#include "sr_ipc.h"
#include "ota_srec_parser.h"
#include "mcu_api.h"


#define   OTA_RECV_MCU_DATA_MAX_SIZE        (1024)
#define   OTA_SEND_MCU_FLASH_DATA_LEN       (512 + 4) // 512为数据长度，4 是地址长度
#define   OTA_RECV_MCU_DATA_TIME_OUT        (3)

#define   OTA_MCU_APP1_NAME            "mcu_1.bin"
#define   OTA_MCU_APP2_NAME            "mcu_2.bin"

#define   RET_OTA_RECV_MCU_MSG_TIMEOUT      (RET_ERR - 1)


typedef struct
{
    GMutex lock;
    GCond cond;
    uint32_t msgType;
    uint32_t dataLen;
    uint16_t sendCount;
    uint8_t seed[4];
    uint8_t data[OTA_RECV_MCU_DATA_MAX_SIZE];
}McuMsgArg_t;

typedef int32_t (*UpgradeMcuCallback)();
typedef struct McuUpgradeProcess
{
    uint8_t step;
    UpgradeMcuCallback pCallback;
} UpgradeMcuProcess_t;


static McuMsgArg_t g_mcuArgs;

/**
 * @name SetRecvMcuDataToUpgradeMcuProcess
 * @brief 向MCU发送升级数据
 * 
 * @param type 消息类型，用于标识发送的数据的种类。
 * @param recvData 指向要发送数据缓冲区的指针。
 * @param dataLen 要发送数据的长度，以字节为单位。
 * @return 函数始终返回RET_OK，表示操作成功。
 */
int32_t SendUpgradeDataToMcu(SYSQueMsgType_e type , const uint8_t *recvData, uint32_t dataLen)
{
    McuMesg_t msg;
    msg.type = type;
    msg.payloadLen = dataLen;
    msg.payload = (uint8_t *)recvData;
    McuSrvSendMesg(&msg);
    return RET_OK;
}
/**
 * @name SetRecvMcuDataToUpgradeMcuProcess
 * @brief 将接收到的MCU数据设置为升级MCU进程的状态。
 * 
 * @param recvData 指向接收到的MCU数据的指针。
 * @param dataLen 接收到的MCU数据的长度。
 * @param type 消息类型，指示如何处理这些数据。
 * @return 若成功，则返回RET_OK；若出错，则返回RET_ERR。
 */
int32_t SetRecvMcuDataToUpgradeMcuProcess(const uint8_t *recvData, uint32_t dataLen, SYSQueMsgType_e type)
{
    if(NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    if(dataLen > OTA_RECV_MCU_DATA_MAX_SIZE || dataLen == 0)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    g_mutex_lock(&g_mcuArgs.lock);
    if(g_mcuArgs.msgType == type)
    {
        memcpy(g_mcuArgs.data, recvData, dataLen);
        g_mcuArgs.dataLen = dataLen;
        LOG_DebugHex(g_mcuArgs.data, g_mcuArgs.dataLen, "SetRecvMcuDataToUpgradeMcuProcess Data");
        g_cond_signal(&g_mcuArgs.cond);
    }
    g_mutex_unlock(&g_mcuArgs.lock);

    return RET_OK;
}
/**
 * @name WaitUpgradeMcuMsgTimeOut
 * @brief 等待MCU升级消息，设定超时时间。
 *  
 * @param secTimeOut 超时时间，单位为秒。指定等待消息的最大时间限制。
 * @return 返回操作结果状态码。如果在超时时间内接收到消息，则返回RET_OK；否则返回RET_ERR。
 */
int32_t WaitUpgradeMcuMsgTimeOut(uint32_t secTimeOut)
{
    int32_t retValue = RET_ERR;
    gint64 endTIme = 0;

    // g_mutex_lock(&g_mcuArgs.lock);
    
    endTIme = g_get_monotonic_time() + G_TIME_SPAN_SECOND * secTimeOut;
    if (g_cond_wait_until(&g_mcuArgs.cond, &g_mcuArgs.lock, endTIme))
    {
        retValue = RET_OK;
    }

    // g_mutex_unlock(&g_mcuArgs.lock);
    return retValue;
}

/**
 * @name UpgradeMcuGetVernReq
 * @brief 向MCU发送获取版本号的请求。
 * 
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，其他值表示不同类型的错误。
 */
static int32_t UpgradeMcuGetVernReq()
{
    int32_t retValue = RET_ERR;

    g_mutex_lock(&g_mcuArgs.lock);
    g_mcuArgs.msgType = SYS_QUE_TYPE_MCU_UPGRADE_GET_VER;
    g_mcuArgs.sendCount++;
    g_mcuArgs.data[0] = (g_mcuArgs.sendCount >> 8) & 0xFF;
    g_mcuArgs.data[1] = (g_mcuArgs.sendCount) & 0xFF;
    g_mcuArgs.dataLen = 2;
    SendUpgradeDataToMcu(g_mcuArgs.msgType, g_mcuArgs.data, g_mcuArgs.dataLen);
    memset(g_mcuArgs.data, 0, OTA_RECV_MCU_DATA_MAX_SIZE);
    g_mcuArgs.dataLen = 0;
    
    retValue = WaitUpgradeMcuMsgTimeOut(OTA_RECV_MCU_DATA_TIME_OUT);
    g_mutex_unlock(&g_mcuArgs.lock);
    if(retValue < RET_OK)
    {
        LOG_Error("wait mcu msg error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    if(g_mcuArgs.dataLen < 2)
    {
        LOG_Error(" mcu msg  len error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    LOG_DebugHex(g_mcuArgs.data, g_mcuArgs.dataLen, "MCU Data");
    if(g_mcuArgs.data[2] != 0x00)
    {
        LOG_Error("mcu upgrade msg status error: 0x%02x", g_mcuArgs.data[2]);
        return RET_ERR;
    }

    return RET_OK;
}

/**
 * @name UpgradeMcuGetSeedReq
 * @brief 向MCU发送获取Seed的请求。
 * 
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，其他值表示不同类型的错误。
 */
static int32_t UpgradeMcuGetSeedReq()
{
    int32_t retValue = RET_ERR;

    g_mutex_lock(&g_mcuArgs.lock);
    g_mcuArgs.msgType = SYS_QUE_TYPE_MCU_UPGRADE_GET_SEED;
    g_mcuArgs.sendCount++;
    g_mcuArgs.data[0] = (g_mcuArgs.sendCount >> 8) & 0xFF;
    g_mcuArgs.data[1] = (g_mcuArgs.sendCount) & 0xFF;
    g_mcuArgs.dataLen = 2;
    SendUpgradeDataToMcu(g_mcuArgs.msgType, g_mcuArgs.data, g_mcuArgs.dataLen);
    memset(g_mcuArgs.data, 0, OTA_RECV_MCU_DATA_MAX_SIZE);
    g_mcuArgs.dataLen = 0;

    retValue = WaitUpgradeMcuMsgTimeOut(OTA_RECV_MCU_DATA_TIME_OUT);
    g_mutex_unlock(&g_mcuArgs.lock);
    if(retValue < RET_OK)
    {
        LOG_Error("wait mcu msg error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    if(g_mcuArgs.dataLen < 2)
    {
        LOG_Error(" mcu msg  len error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    LOG_DebugHex(g_mcuArgs.data, g_mcuArgs.dataLen, "MCU Data");
    if(g_mcuArgs.data[2] != 0x00)
    {
        LOG_Error("mcu upgrade msg status error: 0x%02x", g_mcuArgs.data[2]);
        return RET_ERR;
    }

    g_mcuArgs.seed[0] = 0x01;
    g_mcuArgs.seed[1] = 0x02;
    g_mcuArgs.seed[2] = 0x03;
    g_mcuArgs.seed[3] = 0x04;

    return RET_OK;
}

/**
 * @name UpgradeMcuUnlockFlashReq
 * @brief 向MCU发送解锁flash的请求。
 * 
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，其他值表示不同类型的错误。
 */
static int32_t UpgradeMcuUnlockFlashReq()
{
    int32_t retValue = RET_ERR;

    g_mutex_lock(&g_mcuArgs.lock);
    g_mcuArgs.msgType = SYS_QUE_TYPE_MCU_UPGRADE_UNLOCK_FLASH;
    g_mcuArgs.sendCount++;
    g_mcuArgs.data[0] = (g_mcuArgs.sendCount >> 8) & 0xFF;
    g_mcuArgs.data[1] = (g_mcuArgs.sendCount) & 0xFF;
    memcpy(&g_mcuArgs.data[2], g_mcuArgs.seed, 4);
    g_mcuArgs.dataLen = 6;
    SendUpgradeDataToMcu(g_mcuArgs.msgType, g_mcuArgs.data, g_mcuArgs.dataLen);
    memset(g_mcuArgs.data, 0, OTA_RECV_MCU_DATA_MAX_SIZE);
    g_mcuArgs.dataLen = 0;

    retValue = WaitUpgradeMcuMsgTimeOut(OTA_RECV_MCU_DATA_TIME_OUT);
    g_mutex_unlock(&g_mcuArgs.lock);
    if(retValue < RET_OK)
    {
        LOG_Error("wait mcu msg error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    if(g_mcuArgs.dataLen < 2)
    {
        LOG_Error(" mcu msg  len error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    LOG_DebugHex(g_mcuArgs.data, g_mcuArgs.dataLen, "MCU Data");
    if(g_mcuArgs.data[2] != 0x00)
    {
        LOG_Error("mcu upgrade msg status error: 0x%02x", g_mcuArgs.data[2]);
        return RET_ERR;
    }
    return RET_OK;
}

/**
 * @name UpgradeMcuPutFlashReq
 * @brief 向MCU发送进入刷写模式的请求。
 * 
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，其他值表示不同类型的错误。
 */
static int32_t UpgradeMcuPutFlashReq()
{
    int32_t retValue = RET_ERR;

    g_mutex_lock(&g_mcuArgs.lock);
    g_mcuArgs.msgType = SYS_QUE_TYPE_MCU_UPGRADE_PUT_FLASHING;
    g_mcuArgs.sendCount++;
    g_mcuArgs.data[0] = (g_mcuArgs.sendCount >> 8) & 0xFF;
    g_mcuArgs.data[1] = (g_mcuArgs.sendCount) & 0xFF;
    g_mcuArgs.data[2] = 0x01;
    g_mcuArgs.data[3] = 0x02;
    g_mcuArgs.data[4] = 0x03;
    g_mcuArgs.data[5] = 0x04;
    g_mcuArgs.dataLen = 6;
    SendUpgradeDataToMcu(g_mcuArgs.msgType, g_mcuArgs.data, g_mcuArgs.dataLen);
    memset(g_mcuArgs.data, 0, OTA_RECV_MCU_DATA_MAX_SIZE);
    g_mcuArgs.dataLen = 0;

    retValue = WaitUpgradeMcuMsgTimeOut(OTA_RECV_MCU_DATA_TIME_OUT);
    g_mutex_unlock(&g_mcuArgs.lock);
    if(retValue < RET_OK)
    {
        LOG_Error("wait mcu msg error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    if(g_mcuArgs.dataLen < 2)
    {
        LOG_Error(" mcu msg  len error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    LOG_DebugHex(g_mcuArgs.data, g_mcuArgs.dataLen, "MCU Data");
    if(g_mcuArgs.data[2] != 0x00)
    {
        LOG_Error("mcu upgrade msg status error: 0x%02x", g_mcuArgs.data[2]);
        return RET_ERR;
    }

    return RET_OK;
}

/**
 * @name UpgradeMcuEraseFlashReq
 * @brief 向MCU发送擦除Flash的请求。
 * 
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，其他值表示不同类型的错误。
 */
static int32_t UpgradeMcuEraseFlashReq()
{
    int32_t  retValue  = RET_ERR;
    uint32_t startAddr = 0x00;
    int32_t  fileLen   = 0x00;

    retValue = GetUpgradeSrecFileStartAddr(&startAddr);
    if(retValue < RET_OK)
    {
        LOG_Error("%s GetUpgradeSrecFileStartAddr error", __FUNCTION__);
        return RET_ERR;
    }

    fileLen = GetUpgradeSrecFileContenLen();
    if(fileLen <= 0)
    {
        LOG_Error("%s  GetUpgradeSrecFileContenLen error", __FUNCTION__);
        return RET_ERR;
    }

    g_mutex_lock(&g_mcuArgs.lock);
    g_mcuArgs.msgType = SYS_QUE_TYPE_MCU_UPGRADE_ERASE_FLASH;
    g_mcuArgs.sendCount++;
    g_mcuArgs.data[0] = (g_mcuArgs.sendCount >> 8) & 0xFF;
    g_mcuArgs.data[1] = (g_mcuArgs.sendCount) & 0xFF;
    g_mcuArgs.data[2] = 0x01;
    g_mcuArgs.data[3] = 0x02;
    g_mcuArgs.data[4] = 0x03;
    g_mcuArgs.data[5] = 0x04;

    g_mcuArgs.data[6] = (startAddr >> 24) & 0xff;
    g_mcuArgs.data[7] = (startAddr >> 16) & 0xff;
    g_mcuArgs.data[8] = (startAddr >> 8) & 0xff;
    g_mcuArgs.data[9] = startAddr & 0xff;
    g_mcuArgs.data[10] = (fileLen >> 24) & 0xff;
    g_mcuArgs.data[11] = (fileLen >> 16) & 0xff;
    g_mcuArgs.data[12] = (fileLen >> 8) & 0xff;
    g_mcuArgs.data[13] = fileLen & 0xff;
    
    g_mcuArgs.dataLen = 14;
    SendUpgradeDataToMcu(g_mcuArgs.msgType, g_mcuArgs.data, g_mcuArgs.dataLen);
    memset(g_mcuArgs.data, 0, OTA_RECV_MCU_DATA_MAX_SIZE);
    g_mcuArgs.dataLen = 0;

    retValue = WaitUpgradeMcuMsgTimeOut(OTA_RECV_MCU_DATA_TIME_OUT);
    g_mutex_unlock(&g_mcuArgs.lock);
    if(retValue < RET_OK)
    {
        LOG_Error("wait mcu msg error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    if(g_mcuArgs.dataLen < 2)
    {
        LOG_Error(" mcu msg  len error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    LOG_DebugHex(g_mcuArgs.data, g_mcuArgs.dataLen, "MCU Data");
    if(g_mcuArgs.data[2] != 0x00)
    {
        LOG_Error("mcu upgrade msg status error: 0x%02x", g_mcuArgs.data[2]);
        return RET_ERR;
    }

    return RET_OK;
}
/**
 * @name UpgradeMcuFlashingReq
 * @brief 向MCU发送刷写文件的请求。
 * 
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，其他值表示不同类型的错误。
 */
static int32_t UpgradeMcuFlashingReq()
{
    int32_t readLen = 0;
    int32_t retValue = 0;
    
    g_mutex_lock(&g_mcuArgs.lock);
    g_mcuArgs.msgType = SYS_QUE_TYPE_MCU_UPGRADE_FLASH_MCU;
    g_mcuArgs.sendCount++;
    g_mcuArgs.data[0] = (g_mcuArgs.sendCount >> 8) & 0xFF;
    g_mcuArgs.data[1] = (g_mcuArgs.sendCount) & 0xFF;

    readLen = GetSrecFileContenLenFormList(&g_mcuArgs.data[6], 
                                            OTA_RECV_MCU_DATA_MAX_SIZE - 6, 
                                            OTA_SEND_MCU_FLASH_DATA_LEN);
    if(readLen < 0)
    {
        g_mutex_unlock(&g_mcuArgs.lock);
        return RET_ERR;
    }

    if(readLen == 0)
    {
        g_mutex_unlock(&g_mcuArgs.lock);
        return RET_ERR;
    }
    
    g_mcuArgs.data[2] = g_mcuArgs.data[6];
    g_mcuArgs.data[3] = g_mcuArgs.data[7];
    g_mcuArgs.data[4] = g_mcuArgs.data[8];
    g_mcuArgs.data[5] = g_mcuArgs.data[9];
    int tmpcount = 3;
    retValue = RET_ERR;
    while(tmpcount-- > 0 && retValue < RET_OK)
    {
        g_mcuArgs.dataLen = readLen + 6;
        LOG_DebugHex(g_mcuArgs.data, g_mcuArgs.dataLen, "Send Mcu Data");
        SendUpgradeDataToMcu(g_mcuArgs.msgType, g_mcuArgs.data, g_mcuArgs.dataLen);
        g_mcuArgs.dataLen = 0;
        retValue = WaitUpgradeMcuMsgTimeOut(2);
        LOG_Debug("tmpcount:%d", tmpcount);
    }
    LOG_Debug("Send Mcu Data OK, retValue:%d", retValue);
    //SendUpgradeDataToMcu(g_mcuArgs.msgType, g_mcuArgs.data, g_mcuArgs.dataLen);
    
    g_mutex_unlock(&g_mcuArgs.lock);
    if(retValue < RET_OK)
    {
        LOG_Error("wait mcu msg error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    if(g_mcuArgs.dataLen < 2)
    {
        LOG_Error(" mcu msg  len error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    LOG_DebugHex(g_mcuArgs.data, g_mcuArgs.dataLen, "MCU Data");
    if(g_mcuArgs.data[2] != 0x00)
    {
        LOG_Error("mcu upgrade msg status error: 0x%02x", g_mcuArgs.data[2]);
        return RET_ERR;
    }
    usleep(1000 * 10); // 等待MCU准备好(临时)
    return RET_OK;
}

/**
 * @name UpgradeFlashCompleteReq
 * @brief 向MCU发送升级完成的请求。
 * 
 * @return int32_t 返回操作的结果状态码。RET_OK表示成功，其他值表示不同类型的错误。
 */
static int32_t UpgradeFlashCompleteReq()
{
    int32_t  retValue  = RET_ERR;
    uint32_t crc32Value = 0x00;

    retValue = GetUpgradeSrecFileCrc(&crc32Value);
    if(retValue < RET_OK)
    {
        LOG_Error("%s GetUpgradeSrecFileCrc error", __FUNCTION__);
        return RET_ERR;
    }

    g_mutex_lock(&g_mcuArgs.lock);
    g_mcuArgs.msgType = SYS_QUE_TYPE_MCU_UPGRADE_FLASH_COMPLETE;
    g_mcuArgs.sendCount++;
    g_mcuArgs.data[0] = (g_mcuArgs.sendCount >> 8) & 0xFF;
    g_mcuArgs.data[1] = (g_mcuArgs.sendCount) & 0xFF;


    g_mcuArgs.data[2] = (crc32Value >> 24) & 0xff;
    g_mcuArgs.data[3] = (crc32Value >> 16) & 0xff;
    g_mcuArgs.data[4] = (crc32Value >> 8) & 0xff;
    g_mcuArgs.data[5] = crc32Value & 0xff;
    
    g_mcuArgs.dataLen = 6;
    LOG_DebugHex(g_mcuArgs.data, g_mcuArgs.dataLen, "Send Mcu Data");
    SendUpgradeDataToMcu(g_mcuArgs.msgType, g_mcuArgs.data, g_mcuArgs.dataLen);
    memset(g_mcuArgs.data, 0, OTA_RECV_MCU_DATA_MAX_SIZE);
    g_mcuArgs.dataLen = 0;
    
    retValue = WaitUpgradeMcuMsgTimeOut(OTA_RECV_MCU_DATA_TIME_OUT);
    g_mutex_unlock(&g_mcuArgs.lock);
    if(retValue < RET_OK)
    {
        LOG_Error("wait mcu msg error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    if(g_mcuArgs.dataLen < 2)
    {
        LOG_Error(" mcu msg  len error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    LOG_DebugHex(g_mcuArgs.data, g_mcuArgs.dataLen, "MCU Data");
    if(g_mcuArgs.data[2] != 0x00)
    {
        LOG_Error("mcu upgrade msg status error: 0x%02x", g_mcuArgs.data[2]);
        return RET_ERR;
    }

    return RET_OK;
}

/**
 * @name UpgradeMcuRestMcuReq
 * @brief  请求MCU复位请求。
 * 
 * @param 空
 * @return 函数返回一个整数。成功时返回RET_OK，否则返回RET_ERR或其他错误代码。
 */
static int32_t UpgradeMcuRestMcuReq()
{
    int32_t retValue = RET_ERR;

    g_mutex_lock(&g_mcuArgs.lock);
    g_mcuArgs.msgType = SYS_QUE_TYPE_MCU_UPGRADE_RESET_MCU;
    g_mcuArgs.sendCount++;
    g_mcuArgs.data[0] = 0x00;
    g_mcuArgs.data[1] = 0x01;
    g_mcuArgs.data[2] = 0x01;
    g_mcuArgs.data[3] = 0x02;
    g_mcuArgs.data[4] = 0x03;
    g_mcuArgs.data[5] = 0x04;
    g_mcuArgs.dataLen = 6;
    SendUpgradeDataToMcu(g_mcuArgs.msgType, g_mcuArgs.data, g_mcuArgs.dataLen);
    memset(g_mcuArgs.data, 0, OTA_RECV_MCU_DATA_MAX_SIZE);
    g_mcuArgs.dataLen = 0;

    retValue = WaitUpgradeMcuMsgTimeOut(OTA_RECV_MCU_DATA_TIME_OUT);
    g_mutex_unlock(&g_mcuArgs.lock);
    if(retValue < RET_OK)
    {
        LOG_Error("wait mcu msg error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    if(g_mcuArgs.dataLen < 2)
    {
        LOG_Error(" mcu msg  len error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    LOG_DebugHex(g_mcuArgs.data, g_mcuArgs.dataLen, "MCU Data");
    if(g_mcuArgs.data[2] != 0x00)
    {
        LOG_Error("mcu upgrade msg status error: 0x%02x", g_mcuArgs.data[2]);
        return RET_ERR;
    }

    return RET_OK;
}

/**
 * @name UpgradeMcuRunAppReq
 * @brief  请求MCU升级运行应用程序。
 * 
 * @param runStatus 指向一个字节的指针，用于接收MCU运行状态的响应。
 * @return 函数返回一个整数。成功时返回RET_OK，否则返回RET_ERR或其他错误代码。
 */
static int32_t UpgradeMcuRunAppReq(uint8_t *runStatus)
{
    int32_t retValue = RET_ERR;

    g_mutex_lock(&g_mcuArgs.lock);
    g_mcuArgs.msgType = SYS_QUE_TYPE_MCU_UPGRADE_GET_RUN_APP;
    g_mcuArgs.sendCount++;
    g_mcuArgs.data[0] = 0x00;
    g_mcuArgs.data[1] = 0x01;
    g_mcuArgs.data[2] = 0x01;
    g_mcuArgs.data[3] = 0x02;
    g_mcuArgs.data[4] = 0x03;
    g_mcuArgs.data[5] = 0x04;
    g_mcuArgs.dataLen = 6;
    SendUpgradeDataToMcu(g_mcuArgs.msgType, g_mcuArgs.data, g_mcuArgs.dataLen);
    memset(g_mcuArgs.data, 0, OTA_RECV_MCU_DATA_MAX_SIZE);
    g_mcuArgs.dataLen = 0;

    retValue = WaitUpgradeMcuMsgTimeOut(OTA_RECV_MCU_DATA_TIME_OUT);
    g_mutex_unlock(&g_mcuArgs.lock);
    if(retValue < RET_OK)
    {
        LOG_Error("wait mcu msg error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }

    if(g_mcuArgs.dataLen < 3)
    {
        LOG_Error(" mcu msg  len error");
        return RET_OTA_RECV_MCU_MSG_TIMEOUT;
    }
    LOG_DebugHex(g_mcuArgs.data, g_mcuArgs.dataLen, "MCU Data");
    *runStatus = g_mcuArgs.data[3];

    return RET_OK;
}


/**
 * @name InitUpgradeMcuResource
 * @brief 初始化升级MCU资源
 *  
 * @参数: 无
 * @返回值: 返回RET_OK表示初始化成功，其他值表示初始化失败。
 */
int32_t InitUpgradeMcuResource()
{
    memset(&g_mcuArgs, 0x0, sizeof(McuMsgArg_t));
    g_mutex_init(&g_mcuArgs.lock);
    g_cond_init(&g_mcuArgs.cond);

    return RET_OK;
}
/**
 * @name UpgradeMcuGetVerCallback
 * @brief 用于处理MCU升级时获取版本信息的回调函数
 * 
 * @param recvData 接收到的数据的指针
 * @param dataLen 接收到的数据的长度
 * @param userdata 用户自定义数据
 */
void UpgradeMcuGetVerCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
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
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_GET_VER);
    SetRecvMcuDataToUpgradeMcuProcess(recvData, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_GET_VER);
}

/**
 * @name UpgradeMcuGetSeedCallback
 * @brief 用于处理MCU升级获取种子数据的回调函数
 * 
 * @param recvData 接收到的数据的指针
 * @param dataLen 接收到的数据的长度
 * @param userdata 用户自定义数据
 */
void UpgradeMcuGetSeedCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
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
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_GET_SEED);
    SetRecvMcuDataToUpgradeMcuProcess(recvData, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_GET_SEED);
}

/**
 * @name UpgradeMcuUnlockFlashCallback
 * @brief 升级MCU解锁Flash回调函数
 * 
 * 
 * @param recvData 指向接收到的数据缓冲区的指针。
 * @param dataLen 接收到的数据长度。
 * @param userdata 用户自定义数据，本函数中未使用。
 */
void UpgradeMcuUnlockFlashCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
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
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_UNLOCK_FLASH);
    SetRecvMcuDataToUpgradeMcuProcess(recvData, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_UNLOCK_FLASH);
}

/**
 * @name UpgradeMcuPutFlashingCallback
 * @brief 处理MCU升级过程中接收到的刷写结果数据的回调函数。
 * 
 * @param recvData 指向接收到的数据的指针。不可为NULL。
 * @param dataLen 接收到的数据长度。不可为0。
 * @param userdata 用户自定义数据，本函数中未使用。
 */
void UpgradeMcuPutFlashingCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
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
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_PUT_FLASHING);
    SetRecvMcuDataToUpgradeMcuProcess(recvData, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_PUT_FLASHING);
}

/**
 * @name UpgradeMcuEaseFlashCallback 
 * @brief 升级MCU时，擦除Flash回调函数
 * 
 * @param recvData 指向接收到的数据的指针。该数据包含了擦除Flash的指令。
 * @param dataLen 接收到的数据长度。确保了处理过程不会误操作非指令数据。
 * @param userdata 用户自定义数据，可用于在回调中访问额外的上下文信息。
 */
void UpgradeMcuEaseFlashCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
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
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_ERASE_FLASH);
    SetRecvMcuDataToUpgradeMcuProcess(recvData, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_ERASE_FLASH);
}
/**
 * @name UpgradeMcuFlashMcuCallback 
 * @brief 升级MCU固件的回调函数
 * 
 * @param recvData 指向接收到的数据缓冲区的指针。
 * @param dataLen 接收到的数据长度。
 * @param userdata 用户自定义数据，可用于传递额外的上下文信息。
 */
void UpgradeMcuFlashMcuCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
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
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_FLASH_MCU);
    SetRecvMcuDataToUpgradeMcuProcess(recvData, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_FLASH_MCU);
}
/**
 * @name UpgradeMcuFlashCompleteCallback 
 * @brief MCU固件升级完成后的回调函数
 * 
 * @param recvData 指向接收到的数据缓冲区的指针
 * @param dataLen 接收到的数据长度
 * @param userdata 用户自定义数据，可用于传递额外的上下文信息
 */
void UpgradeMcuFlashCompleteCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
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
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_FLASH_COMPLETE);
    SetRecvMcuDataToUpgradeMcuProcess(recvData, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_FLASH_COMPLETE);
}

/**
 * @name UpgradeMcuResetMcuCallback 
 * @brief 用于处理MCU升级过程中重置MCU的回调函数
 * 
 * @param recvData 接收到的数据的指针
 * @param dataLen 接收到的数据的长度
 * @param userdata 用户自定义数据
 */
void UpgradeMcuResetMcuCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
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
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_RESET_MCU);
    SetRecvMcuDataToUpgradeMcuProcess(recvData, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_RESET_MCU);
}
/**
 *  @name UpgradeMcuGetRunAppCallback 
 *  @brief 升级MCU获取运行应用程序的回调函数
 * 
 * @param recvData 指向接收到的数据的指针。不可为NULL。
 * @param dataLen 接收到的数据的长度。不可为0。
 * @param userdata 用户自定义数据，该函数中未使用。
 */
void UpgradeMcuGetRunAppCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
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
    LOG_Debug("%s, recv DataLen:%d, type%08x", __FUNCTION__, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_GET_RUN_APP);
    SetRecvMcuDataToUpgradeMcuProcess(recvData, dataLen, SYS_QUE_TYPE_MCU_UPGRADE_GET_RUN_APP);
}

static UpgradeMcuProcess_t g_upgradeMcuProcess[] = 
{
    {0, UpgradeMcuGetVernReq},
    {1, UpgradeMcuGetSeedReq},
    {2, UpgradeMcuUnlockFlashReq},
    {3, UpgradeMcuPutFlashReq},
    {4, UpgradeMcuEraseFlashReq},
    {5, UpgradeMcuFlashingReq},
    {6, UpgradeFlashCompleteReq},
    {7, UpgradeMcuRestMcuReq}
};

/**
 *  @name InitUpgradeMcuIpcCallback 
 *  @brief 初始化MCU升级的IPC回调函数
 * 
 * 该函数用于初始化MCU升级过程中的各个阶段的回调函数，通过向订阅队列中添加特定类型的消息，
 * 来指明在收到对应消息时应该调用哪个回调函数进行处理。
 * 
 * @return int32_t 返回操作的结果状态码，成功时返回RET_OK。
 */
static int32_t InitUpgradeMcuIpcCallback()
{
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_GET_VER, UpgradeMcuGetVerCallback, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_GET_SEED, UpgradeMcuGetSeedCallback, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_UNLOCK_FLASH, UpgradeMcuUnlockFlashCallback, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_PUT_FLASHING, UpgradeMcuPutFlashingCallback, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_ERASE_FLASH, UpgradeMcuEaseFlashCallback, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_FLASH_MCU, UpgradeMcuFlashMcuCallback, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_FLASH_COMPLETE, UpgradeMcuFlashCompleteCallback, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_RESET_MCU, UpgradeMcuResetMcuCallback, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_GET_RUN_APP, UpgradeMcuGetRunAppCallback, NULL);
    return RET_OK;
}
/**
 *  @name DestoryUpgradeMcuIpcCallback 
 *  @brief 销毁升级MCU IPC回调
 * 
 * 该函数用于取消订阅一系列与MCU升级相关的消息队列消息，
 * 以确保在升级过程中或升级完成后，相关消息不再被处理。
 * 
 * @return int32_t 返回操作结果，若成功则为RET_OK。
 */
static int32_t DestoryUpgradeMcuIpcCallback()
{
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_GET_VER);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_GET_SEED);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_UNLOCK_FLASH);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_PUT_FLASHING);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_ERASE_FLASH);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_FLASH_MCU);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_FLASH_COMPLETE);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_RESET_MCU);
    UnSubscriptionQueueMsg(SYS_QUE_TYPE_MCU_UPGRADE_GET_RUN_APP);
    return RET_OK;
}

/**
 * @name UpgradeMcuProcess
 * @brief 升级MCU处理函数
 * 
 * @param filePath 文件路径，用于指定升级包的位置
 * @return int32_t 返回RET_OK表示成功，RET_ERR表示失败
 */
int32_t UpgradeMcuProcess(const char *filePath)
{
    int32_t retValue = RET_ERR;
    int32_t whileCount = 3; 
    int32_t callBackSize = sizeof(g_upgradeMcuProcess) / sizeof(UpgradeMcuProcess_t);
    int32_t callBackIndex = 0;
    uint8_t mcuRunStatus = 0;
    char upgradeFileName[256] = {0};

    if(NULL == filePath)
    {
        LOG_Error("%s arg is invalid", __FUNCTION__);
        return retValue;
    }

    LOG_Debug("start upgrade mcu ");

    InitUpgradeMcuResource();

    InitUpgradeMcuIpcCallback();

    //升级MCU
    do
    {
        LOG_Debug("upgrade get mcu run app : %d", whileCount);
        whileCount--;
        retValue = UpgradeMcuRunAppReq(&mcuRunStatus);
        if(retValue < 0)
        {
            LOG_Error("mcu upgrade msg status error: 0x%02x", retValue);
            retValue = RET_ERR;
            continue;
        }
        LOG_Debug("mcu run app : %d", mcuRunStatus);

        if(0x01 == mcuRunStatus)
        {
            snprintf(upgradeFileName, 256, "%s%s",filePath, OTA_MCU_APP2_NAME);
            retValue = InitSrecFileToList(upgradeFileName);
            if(retValue < 0)
            {
                DestoryUpgradeMcuIpcCallback();
                LOG_Error("Init Srec File To List errror");
                LOG_Debug("stop upgrade mcu ");
                return retValue;
            }
            whileCount = 3;
            retValue = RET_OK;
        }
        else if(0x00 == mcuRunStatus || 0x02 == mcuRunStatus)
        {
            snprintf(upgradeFileName, 256, "%s%s",filePath, OTA_MCU_APP1_NAME);
            retValue = InitSrecFileToList(upgradeFileName);
            if(retValue < 0)
            {
                DestoryUpgradeMcuIpcCallback();
                LOG_Error("Init Srec File To List errror");
                LOG_Debug("stop upgrade mcu ");
                return retValue;
            }
            whileCount = 3;
            retValue = RET_OK;
        }
    }
    while(whileCount > 0 && retValue < RET_OK);

    for(callBackIndex = 0; callBackIndex < callBackSize; callBackIndex++)
    {
        LOG_Debug("upgrade  mcu %d  setp", callBackIndex);
        while(whileCount > 0)
        {
            retValue = g_upgradeMcuProcess[callBackIndex].pCallback();
            LOG_Debug("callBackIndex: %d, retValue: %d, whileCount:%d ", callBackIndex, retValue, whileCount);
            if(5 == callBackIndex && RET_OK == retValue)
            {
                //此处只要大于等于2即可，每次返回成功继续赋值
                whileCount = 2;
            }
            else if(RET_OK == retValue)
            {
                whileCount = 3;
                break;
            }
            else if(5 == callBackIndex)
            {
                //此处返回失败直接退出，无充重新传输
                whileCount = 1;
                break;
            }
            LOG_Debug("callBackIndex: %d, retValue: %d, whileCount:%d ", callBackIndex, retValue, whileCount);
            whileCount--;
        }
    }

    DestroyListAndFree();
    DestoryUpgradeMcuIpcCallback();
    
    LOG_Debug("stop upgrade mcu ");

    return retValue;
}
