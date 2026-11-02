#include "mqtt_get_mcu_version.h"
#include "err_code.h"
#include "sr_log.h"
#include "mcu_api.h"
#include "mqtt_common.h"
#include "common_functions.h"
#include "mqtt_get_modem_info.h"
#include "sr_param.h"

#define TBOX_VIN_LENGTH  (18)
#define TBOX_SN_LENGTH   (28)

static char g_tboxVersion[256] = {0};
static char g_mcuVersion[4] = {0};
static pthread_mutex_t g_getMcuVerLock = PTHREAD_MUTEX_INITIALIZER;
static int g_Initialized = 0;

static uint8_t g_carNum[8] = {0};
static char g_mqttAddrBuff[128] = {0};
static char g_vin[TBOX_VIN_LENGTH] = {0};
static char g_sn[TBOX_SN_LENGTH] = {0};

//添加数据库接口功能

/**
 * @brief 从数据库获取参数数据
 * 
 * @param paramId 参数 ID（如 E_ParamId_VIN、E_ParamId_SN 等）
 * @param buffer 存储数据的缓冲区
 * @param bufferSize 缓冲区大小
 * @param dataLen 实际获取的数据长度（输出参数）
 * @return int32_t 执行结果：RET_OK 成功，RET_ERR 失败
 */
static int32_t GetDatabaseParam(uint32_t paramId, uint8_t *buffer, uint32_t bufferSize, uint32_t *dataLen)
{
    ParamInfo_t paramInfo;
    
    // 参数验证
    if (NULL == buffer || NULL == dataLen || 0 == bufferSize) {
        return RET_ERR;
    }
    
    // 初始化结构体
    memset(&paramInfo, 0, sizeof(ParamInfo_t));
    paramInfo.id = paramId;
    
    // 从数据库获取参数
    if (GetParam(&paramInfo) < 0) {
        return RET_ERR;
    }
    
    // 计算实际数据长度（字符串长度）
    uint32_t len = 0;
    while (len < sizeof(paramInfo.value) && 
           paramInfo.value[len] != '\0') {
        len++;
    }
    
    // 复制数据到输出缓冲区
    if (len >= bufferSize) {
        len = bufferSize - 1;  // 保留一个字节给结束符
    }
    
    memcpy(buffer, paramInfo.value, len);
    buffer[len] = '\0';  // 确保字符串结束
    
    if (dataLen != NULL) {
        *dataLen = len;
    }
    
    return RET_OK;
}

// 数据库关键数据缓存初始化接口
// 从数据库初始化 carNum、storedVersion、mqttAddrBuff、vin、sn
int InitTboxDataBaseCache()
{
    int ret = -1;
    uint8_t tempBuffer[256] = {0};
    uint32_t dataLen = 0;
        
    // 防止重复初始化
    if (!g_Initialized) {
        // 初始化车辆编号 (使用数据库索引)
        if (GetDatabaseParam(CAR_NUM_SQLITE_INDEX, tempBuffer, sizeof(tempBuffer), &dataLen) < 0) {
            LOG_Error("%s Get CAR_NUM error",__FUNCTION__);
            //return ret;
        } else {
            memcpy(g_carNum, tempBuffer, sizeof(g_carNum));
            LOG_Debug("%s initialized g_carNum: %s", __FUNCTION__, g_carNum);
        }
        
        // 初始化 MQTT 地址 (使用数据库索引)
        if (GetDatabaseParam(MQTT_ADDR_SQLITE_INDEX, tempBuffer, sizeof(tempBuffer), &dataLen) < 0) {
            LOG_Error("%s Get MQTT_ADDR error",__FUNCTION__);
            //return ret;
        } else {
            strncpy(g_mqttAddrBuff, (char *)tempBuffer, sizeof(g_mqttAddrBuff) - 1);
            g_mqttAddrBuff[sizeof(g_mqttAddrBuff) - 1] = '\0';
            LOG_Debug("%s initialized g_mqttAddrBuff: [%s]", __FUNCTION__, g_mqttAddrBuff);
        }
        
        // 初始化 TBOX 版本 (使用数据库索引)
        if (GetDatabaseParam(E_ParamId_SW_Version, tempBuffer, sizeof(tempBuffer), &dataLen) < 0) {
            LOG_Error("%s Get TBOX_VERSION error",__FUNCTION__);
            //return ret;
        } else {
            strncpy(g_tboxVersion, (char *)tempBuffer, sizeof(g_tboxVersion) - 1);
            g_tboxVersion[sizeof(g_tboxVersion) - 1] = '\0';
            LOG_Debug("%s initialized g_tboxVersion from database: [%s]", __FUNCTION__, g_tboxVersion);
        }
        
        // 初始化 VIN
        if (GetDatabaseParam(E_ParamId_VIN, tempBuffer, sizeof(tempBuffer), &dataLen) < 0) {
            LOG_Error("%s Get VIN error",__FUNCTION__);
            //return ret;
        } else {
            strncpy(g_vin, (char *)tempBuffer, TBOX_VIN_LENGTH - 1);
            g_vin[TBOX_VIN_LENGTH - 1] = '\0';
            LOG_Debug("%s initialized g_vin: [%s]", __FUNCTION__, g_vin);
        }
        
        // 初始化 SN
        if (GetDatabaseParam(E_ParamId_SN, tempBuffer, sizeof(tempBuffer), &dataLen) < 0) {
            LOG_Error("%s Get SN error",__FUNCTION__);
            //return ret;
        } else {
            strncpy(g_sn, (char *)tempBuffer, TBOX_SN_LENGTH - 1);
            g_sn[TBOX_SN_LENGTH - 1] = '\0';
            LOG_Debug("%s initialized g_sn: [%s]", __FUNCTION__, g_sn);
        }
        
        // 标记初始化完成
        g_Initialized = 1;
        ret = 0;
    } else {
        ret = 1;  // 已经初始化，返回成功
    }
    
    return ret;
}
// 强制刷新数据库缓存
void RefreshTboxDataBaseCache()
{
    g_Initialized = 0;
    int ret = InitTboxDataBaseCache();
    LOG_Debug("%s database cache refresh requested ret:%d", __FUNCTION__,ret);
}

int32_t SendVersionBuffToMcu(const char *recvData, const uint16_t dataLen)
{
    McuMesg_t sendToMcu;

    if(NULL == recvData || dataLen <= 0)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return RET_ERR;
    }

    sendToMcu.type = (SYSQueMsgType_e)(SYS_QUE_TYPE_MCU_VEHICLE_CTROL_VERSION_SYNC_TO_MCU);
    sendToMcu.payloadLen = dataLen;
    sendToMcu.payload = (uint8_t *)recvData;

    LOG_Debug("sendToMcu.type : %8x", sendToMcu.type);
    return McuSrvSendMesg(&sendToMcu);
}

void McuSyncVersionToMpuCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    char fileContent[16] = {0};
    char *hwVersion = NULL;
    int32_t retValue = -1;
    char newVersion[256] = {0};

    if(NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    if(dataLen < 5)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }


    pthread_mutex_lock(&g_getMcuVerLock);
    memcpy(g_mcuVersion, &recvData[1], 3);
    memcpy(newVersion, &recvData[5], recvData[4]);
    newVersion[recvData[4]] = '\0';  // 确保字符串结尾
    pthread_mutex_unlock(&g_getMcuVerLock);
    
    LOG_Debug("%s get mcu version:%s ", __FUNCTION__, g_mcuVersion);
    LOG_Debug("%s get tbox version:%s ", __FUNCTION__, newVersion);

    retValue = GetFileContent("/oemapp/upgrade/version.txt", fileContent, 16);
    if(retValue < 0 || retValue < 11)
    {
        LOG_Error("%s GetFileContent error: %d", __FUNCTION__, retValue);
        return ;
    }
    SendVersionBuffToMcu(fileContent, 11);

    // 直接与全局变量g_tboxVersion比较，避免重复读取数据库
    if (strncmp(g_tboxVersion, newVersion, sizeof(g_tboxVersion)) != 0) {
        // 版本不同才进行设置
        LOG_Debug("%s version changed from [%s] to [%s], updating...", 
                 __FUNCTION__, g_tboxVersion, newVersion);
        strncpy(g_tboxVersion, newVersion, sizeof(g_tboxVersion) - 1);
        g_tboxVersion[sizeof(g_tboxVersion) - 1] = '\0';
        SetTboxSoftVersion((uint8_t *)newVersion, strlen(newVersion));
    } else {
        // 版本相同，跳过设置
        LOG_Debug("%s version unchanged [%s], skip setting", __FUNCTION__, g_tboxVersion);
    }

    if(dataLen < (recvData[4] + 6 + strlen("HV001.002")))
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }

    hwVersion = (char *)&(recvData[recvData[4] + 6]);

    if(0 == strncmp(hwVersion, "HV001.002", strlen("HV001.002")))
    {
        SrGetSystemRetValue("touch  /oemdata/tts/audio_switch");
    }
    
    LOG_Debug("%s get tbox hwVersion:%s ", __FUNCTION__, hwVersion);
}
//数据库内容同步
void McuSyncDatabaseToMpuCallback(const uint8_t *recvData, const uint16_t dataLen, void* userdata)
{
    uint8_t CarNumLebgth = 0;
    uint8_t carNum[8] = {0};
    uint8_t MQTTAddressLebgth = 0;
    char mqttAddrBuff[128] = {0};
    uint8_t VINLength = 0;
    char vin[TBOX_VIN_LENGTH] = {0};
    uint8_t SNLength = 0;
    char sn[TBOX_SN_LENGTH] = {0};
    ParamInfo_t paranInfo;

    if(NULL == recvData)
    {
        LOG_Error("%s argv is invalid", __FUNCTION__);
        return;
    }else{
        LOG_DebugHex(recvData,dataLen, "DataSync");
    }

    // 参数验证：至少需要 2 字节（CarNumLebgth + MQTTAddressLebgth）
    if(dataLen < 2)
    {
        LOG_Error("%s data length too small: %d", __FUNCTION__, dataLen);
        return;
    }

    // 解析协议数据
    CarNumLebgth = recvData[0];
    
    // 验证 CarNumLebgth 范围（最大 8 字节）
    if(CarNumLebgth > sizeof(carNum))
    {
        LOG_Error("%s CarNumLebgth too large: %d", __FUNCTION__, CarNumLebgth);
        return;
    }
    
    // 计算 MQTTAddressLebgth 的偏移位置：CarNumLebgth + 1
    if(dataLen < (CarNumLebgth + 2))  // 至少需要 CarNumLebgth + 2 字节
    {
        LOG_Error("%s data length too small for CarNumData: %d", __FUNCTION__, dataLen);
        return;
    }
    
    MQTTAddressLebgth = recvData[CarNumLebgth + 1];
    
    // 验证 MQTTAddressLebgth 范围（最大 128 字节）
    if(MQTTAddressLebgth > sizeof(mqttAddrBuff))
    {
        LOG_Error("%s MQTTAddressLebgth too large: %d", __FUNCTION__, MQTTAddressLebgth);
        return;
    }
    
    // 检查总数据长度是否足够
    if(dataLen < (CarNumLebgth + 2 + MQTTAddressLebgth))
    {
        LOG_Error("%s data length too small for MQTTAddressData: %d", __FUNCTION__, dataLen);
        return;
    }

    // 处理车辆编号
    if(CarNumLebgth != 0 && CarNumLebgth <= sizeof(carNum)){
        memcpy(carNum, &recvData[1], CarNumLebgth);
        LOG_Debug("%s get carNum:%s len:%d", __FUNCTION__, carNum,CarNumLebgth);
        if(carNum[0] != '\0' && memcmp(carNum, g_carNum, sizeof(g_carNum)) != 0){  // 与缓存不一致则更新数据库
            LOG_Debug("%s carNum %s:%s diff ", __FUNCTION__, carNum,g_carNum);
            if(UpdateParam(CAR_NUM_SQLITE_INDEX, carNum, CarNumLebgth) < 0)
            {
                LOG_Error("%s Set CAR_NUM_SQLITE_INDEX error", __FUNCTION__);
            }else{
                RefreshTboxDataBaseCache();
            }
        }else{
            LOG_Debug("%s carNum %s:%s OK skip ", __FUNCTION__, carNum,g_carNum);
        }
    }else{
        LOG_Error("%s CarNumLebgth: %d ERR", __FUNCTION__, CarNumLebgth);
    }

    // 处理 MQTT 地址
    if(MQTTAddressLebgth != 0 && MQTTAddressLebgth <= sizeof(mqttAddrBuff)){
        memcpy(mqttAddrBuff, &recvData[CarNumLebgth + 2], MQTTAddressLebgth);
        LOG_Debug("%s get mqttAddr:%s len:%d", __FUNCTION__, mqttAddrBuff,MQTTAddressLebgth);
        if(mqttAddrBuff[0] != '\0' && memcmp(mqttAddrBuff, g_mqttAddrBuff, sizeof(g_mqttAddrBuff)) != 0){  // 与缓存不一致则更新数据库
            LOG_Debug("%s mqttAddrBuff %s:%s diff ", __FUNCTION__, mqttAddrBuff,g_mqttAddrBuff);
            if(UpdateParam(MQTT_ADDR_SQLITE_INDEX, (uint8_t *)mqttAddrBuff, MQTTAddressLebgth) < 0)
            {
                LOG_Error("%s Set MQTT_ADDR_SQLITE_INDEX error", __FUNCTION__);
            }else{
                RefreshTboxDataBaseCache();
            }
        }else{
            LOG_Debug("%s mqttAddrBuff %s:%s OK skip ", __FUNCTION__, mqttAddrBuff,g_mqttAddrBuff);
        }
    }else{
        LOG_Error("%s MQTTAddressLebgth: %d ERR", __FUNCTION__, MQTTAddressLebgth);
    }

    // 计算 VIN 长度字段的偏移位置
    uint16_t vinOffset = CarNumLebgth + 2 + MQTTAddressLebgth;
    
    // 验证是否有 VIN 长度字段
    if(dataLen < (vinOffset + 1))
    {
        LOG_Error("%s data length too small for VINLength: %d", __FUNCTION__, dataLen);
        return;
    }
    
    VINLength = recvData[vinOffset];
    
    // 验证 VINLength 范围（最大 17 字节，加 1 字节结束符）
    if(VINLength > TBOX_VIN_LENGTH - 1)
    {
        LOG_Error("%s VINLength too large: %d", __FUNCTION__, VINLength);
        return;
    }
    
    // 检查总数据长度是否足够接收 VIN 数据
    if(dataLen < (vinOffset + 1 + VINLength))
    {
        LOG_Error("%s data length too small for VINData: %d", __FUNCTION__, dataLen);
        return;
    }

    // 处理 VIN
    if(VINLength != 0 && VINLength <= TBOX_VIN_LENGTH - 1){
        memcpy(vin, &recvData[vinOffset + 1], VINLength);
        vin[VINLength] = '\0';  // 添加字符串结束符
        LOG_Debug("%s get vin:%s len:%d", __FUNCTION__, vin, VINLength);
        if(vin[0] != '\0' && (memcmp(vin, g_vin, VINLength) != 0 || vin[VINLength] != g_vin[VINLength])){  // 与缓存不一致则更新数据库
            LOG_Debug("%s vin %s:%s diff ", __FUNCTION__, vin, g_vin);
            if(UpdateParam(E_ParamId_VIN, (uint8_t *)vin, VINLength) < 0)
            {
                LOG_Error("%s Set VIN error", __FUNCTION__);
            }else{
                RefreshTboxDataBaseCache();
            }
        }else{
            LOG_Debug("%s vin %s:%s OK skip ", __FUNCTION__, vin, g_vin);
        }
    }else{
        LOG_Error("%s VINLength: %d ERR", __FUNCTION__, VINLength);
    }

    // 计算 SN 长度字段的偏移位置
    uint16_t snOffset = vinOffset + 1 + VINLength;
    
    // 验证是否有 SN 长度字段
    if(dataLen < (snOffset + 1))
    {
        LOG_Error("%s data length too small for SNLength: %d", __FUNCTION__, dataLen);
        return;
    }
    
    SNLength = recvData[snOffset];
    
    // 验证 SNLength 范围（最大 27 字节，加 1 字节结束符）
    if(SNLength > TBOX_SN_LENGTH - 1)
    {
        LOG_Error("%s SNLength too large: %d", __FUNCTION__, SNLength);
        return;
    }
    
    // 检查总数据长度是否足够接收 SN 数据
    if(dataLen < (snOffset + 1 + SNLength))
    {
        LOG_Error("%s data length too small for SNData: %d", __FUNCTION__, dataLen);
        return;
    }

    // 处理 SN
    if(SNLength != 0 && SNLength <= TBOX_SN_LENGTH - 1){
        memcpy(sn, &recvData[snOffset + 1], SNLength);
        sn[SNLength] = '\0';  // 添加字符串结束符
        LOG_Debug("%s get sn:%s len:%d", __FUNCTION__, sn, SNLength);
        if(sn[0] != '\0' && (memcmp(sn, g_sn, SNLength) != 0 || sn[SNLength] != g_sn[SNLength])){  // 与缓存不一致则更新数据库
            LOG_Debug("%s sn %s:%s diff ", __FUNCTION__, sn, g_sn);
            if(UpdateParam(E_ParamId_SN, (uint8_t *)sn, SNLength) < 0)
            {
                LOG_Error("%s Set SN error", __FUNCTION__);
            }else{
                RefreshTboxDataBaseCache();
            }
        }else{
            LOG_Debug("%s sn %s:%s OK skip ", __FUNCTION__, sn, g_sn);
        }
    }else{
        LOG_Error("%s SNLength: %d ERR", __FUNCTION__, SNLength);
    }
    
    return;

}
