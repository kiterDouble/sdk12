#include "cycle_get_mcu_version.h"
#include "err_code.h"
#include "sr_log.h"
#include "mcu_api.h"
#include "cycle_common.h"
#include "common_functions.h"
#include "cycle_get_modem_info.h"

static char g_tboxVersion[256] = {0};
static char g_mcuVersion[4] = {0};
static pthread_mutex_t g_getMcuVerLock = PTHREAD_MUTEX_INITIALIZER;

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
    memcpy(g_tboxVersion, &recvData[5], recvData[4]);
    pthread_mutex_unlock(&g_getMcuVerLock);
    LOG_Debug("%s get mcu version:%s ", __FUNCTION__, g_mcuVersion);
    LOG_Debug("%s get tbox version:%s ", __FUNCTION__, g_tboxVersion);

    retValue = GetFileContent("/oemapp/upgrade/version.txt", fileContent, 16);
    if(retValue < 0 || retValue < 11)
    {
        LOG_Error("%s GetFileContent error: %d", __FUNCTION__, retValue);
        return ;
    }
    SendVersionBuffToMcu(fileContent, 11);

    SetTboxSoftVersion((uint8_t *)g_tboxVersion, strlen(g_tboxVersion));

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