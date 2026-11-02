#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err_code.h"
#include "sr_log.h"
#include "hal_modem_ecall.h"


typedef struct
{
    int32_t inited;
    EcallVoiceCallbak ecallVoiceCbk;
} strHalEcallVoiceSet;

/**
* @brief 通用状态处理函数
*
* @param msgId: 消息 ID
* @param msgData: 消息数据
* @param msgDataLen: 消息数据长度
*
* @return 无返回值
*/
void CommonStateHandler(int ind,void *userdata)
{
}


/**
* @brief 初始化 eCall 资源
*
* @param cbFunc: 回调函数，用于处理 eCall 事件
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEcallVoiceInit(EcallVoiceCallbak cbFunc)
{
    return RET_OK;
}

/**
* @brief 释放 eCall 资源
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEcallVoiceDeinit() {
    return RET_OK;
}

/**
* @brief 使能ECALL语音功能
*
* @return RET_OK表示ECALL语音功能使能成功，RET_ERR表示失败。
*/
int32_t HalEcallVoiceEnable(void)
{
    return RET_OK;
}
/**
* @brief 禁用语音功能
*
* @return 成功返回0，失败返回非0
*/

int32_t HalEcallVoiceDisable(void)
{
    return RET_OK;
}

/**
* @brief 设置语音配置信息
*
* @param ecallConfig: 语音配置信息结构体指针
*
* @return 成功返回0，失败返回非0
*/
int32_t HalEcallVoiceSetConfigInfo(StrEcallConfig *ecallConfig)
{
    return RET_OK;
}
/**
* @brief 启动语音功能
*
* @param EcallVoiceInfo: 语音信息结构体指针
*
* @return 成功返回0，失败返回非0
*/
int32_t HalEcallVoiceStart(StrEcallVoiceInfo *const EcallVoiceInfo, EcallMsd_t *ecallMsdData) {

    return RET_OK;
}

/**
* @brief 挂断 eCall 通话
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEcallVoiceHangup(void)
{
    return RET_OK;
}

/**
* @brief 更新 MSD
*
* @param msd: 要更新的 MSD 数据
* @param msdLen: MSD 数据长度
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEcallVoiceUpdateMsd(const int8_t *msd, uint32_t msdLen, EcallMsd_t *ecallMsdData)
{
    return RET_OK;
}

/**
* @brief 发送 MSD 以 PUSH 方式
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEcallVoicePushMsd(void)
{
    return RET_OK;
}

/**
* @brief 获取 eCall 配置信息
*
* @param EcallConfigInfo: 用于存储 eCall 配置信息的结构体指针
*
* @return 成功返回 0，失败返回非 0
*/
int32_t HalEcallVoiceGetConfigInfo(EcallConfigInfo_t *ecallConfigInfo)
{
    return RET_OK; 
}


void HalEcallAudioPrepare(void)
{

}

