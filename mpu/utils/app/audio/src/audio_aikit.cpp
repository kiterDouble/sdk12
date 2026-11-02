/*
 * @Description: 
 * @version: 
 * @Author: rfge
 * @Date: 2020-10-28 23:32:41
 * @LastEditors: rfge
 * @LastEditTime: 2020-12-29 10:10:30
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fstream>
#include <assert.h>
#include <cstring>
#include <atomic>
#include <unistd.h>

#include "aikit_biz_api.h"
#include "aikit_constant.h"
#include "aikit_biz_config.h"
#include "audio_common.h"
#include "sr_log.h"
#include "audio_playback.h"

using namespace std;
using namespace AIKIT;

static std::atomic_bool ttsFinished(false);

FILE *fin = nullptr;
static const char *ABILITY = "e2e44feff";
int32_t socketFd = 0;
uint64_t g_pcmDataLen = 0;
static time_t g_audioTimes = 0;
static int32_t g_errorNum = -1;

// 假设 N 的值为 16，你可以根据需要修改
#define BUFFER_THRESHOLD  4096




int accumulate_and_send(const uint8_t *inputData, uint32_t inputLen,uint32_t sendLen)
{
    // 全局缓冲区，用于累积数据
    static uint8_t globalBuffer[BUFFER_THRESHOLD];
    // 当前缓冲区中已有的数据长度
    static uint32_t bufferLength = 0;

    // 参数检查
    if (inputData == NULL) 
    {
        return -1;
    }

    // 参数检查
    if (sendLen >=  BUFFER_THRESHOLD) 
    {
        return -1;
    }

    // 将新数据追加到全局缓冲区
    uint32_t byteToSend = 0;
    while (inputLen > 0) 
    {
        // 计算本次可以追加到缓冲区的字节数
        byteToSend = (inputLen < (BUFFER_THRESHOLD - bufferLength)) ? 
                         inputLen : (BUFFER_THRESHOLD - bufferLength);

        // 将数据复制到缓冲区
        memcpy(globalBuffer + bufferLength, inputData, byteToSend);
        bufferLength += byteToSend;
        inputData += byteToSend;
        inputLen -= byteToSend;

        // 检查是否达到拷贝阈值
        if (bufferLength >= sendLen) {
            //调用PCM流push接口
            // memcpy(sendBuffer, globalBuffer, sendLen);
            // AudioUdpSendWithFd(socketFd, UDP_AUDIO_BAND_ADDR, AUDIO_PLAY_BACK_TTS_BIND_PORT, (const char *)globalBuffer, sendLen);
            AudioPushPcmDataToQue(globalBuffer, sendLen);

            memmove(globalBuffer, globalBuffer + sendLen, bufferLength - sendLen);
            bufferLength -= sendLen;
        }
        // 如果还没达到阈值，继续循环接收剩余输入数据
    }

    return 0;
}

int32_t GetAudioAikitStatus()
{
    if(0 == g_errorNum)
    {
        return RET_OK;
    }
    else
    {
        return RET_ERR;
    }
}


void OnOutput(AIKIT_HANDLE* handle, const AIKIT_OutputData* output){
    // printf("OnOutput key:%s\n",(char*)output->node->key);
    // printf("OnOutput status:%d\n",output->node->status);
    if((output->node->value) && (output->node->len > 0))
    {

        accumulate_and_send((uint8_t *)output->node->value, output->node->len, AUDIO_PLAY_BACK_TTS_PCM_LEN);
        // fwrite(output->node->value, sizeof(char), output->node->len, fin);
    }
    g_pcmDataLen += output->node->len;
    g_errorNum = 0;
}

void OnEvent(AIKIT_HANDLE* handle, AIKIT_EVENT eventType, const AIKIT_OutputEvent* eventValue){
    LOG_Debug("OnEvent:%d\n",eventType);
    if(eventType == AIKIT_Event_End){
        ttsFinished = true;
    }
}

void OnError(AIKIT_HANDLE* handle, int32_t err, const char* desc){
    LOG_Debug("OnError:%d\n",err);
    g_errorNum = err;
}

int32_t GetAudioPcmTimes(time_t *pcmTime)
{
    time_t newtime = 0;
    if(NULL == pcmTime)
    {
        return RET_ERR;
    }

    time(&newtime);

    *pcmTime = (g_pcmDataLen / 32000) - (newtime - g_audioTimes);

    if(*pcmTime <= 0)
    {
        *pcmTime = 0;
    }

    LOG_Debug("pcmTime:%d\n", *pcmTime);
    return RET_OK;
}

int32_t GetAudioTotalPcmTimes(time_t *pcmTime)
{
    if(NULL == pcmTime)
    {
        return RET_ERR;
    }

    *pcmTime = (g_pcmDataLen / 32000);

    return RET_OK;
}


void AudioGenerateXTTS(int32_t speed, int32_t volume, const char *ttsStr, const char *encoding, uint8_t fayin)
{
    AIKIT_ParamBuilder* paramBuilder = nullptr;
    AIKIT_DataBuilder* dataBuilder = nullptr;   
    string entext = "hello everyone, this is catherine speeking";
    // char tmpStr[1024] = {0};
    string cntext = ttsStr;
    AIKIT_HANDLE* handle = nullptr;
    AiText* aiText_raw = nullptr;
    ttsFinished = false;
    const char *role = "xiaofeng";
    const char *role1 = "xiaoyan";
    int32_t tmpSpeed = 0;
    int32_t tmpVolume = 0;

    if(NULL == ttsStr || NULL == encoding)
    {
        LOG_Debug("audio_control arg is NULL ");
        return;
    }

    // if(strlen(encoding) < 3)
    // {
    //     LOG_Debug("audio_control encoding: %s", encoding);
    //     return;
    // }
    g_pcmDataLen = 0;
    time(&g_audioTimes);
    //const char *role = "catherine"; 
    paramBuilder = AIKIT_ParamBuilder::create();
    paramBuilder->clear();
    if(3 == fayin)
    {
        paramBuilder->param("vcn", role1, strlen(role1));//必选参数，60020：xiaoyan发音人60030：xiaofeng发音人69010：john英文发音人69020：catherine英文发音人
    }
    else
    {
        paramBuilder->param("vcn", role, strlen(role));//必选参数，60020：xiaoyan发音人60030：xiaofeng发音人69010：john英文发音人69020：catherine英文发音人
    }
    
    paramBuilder->param("vcnModel", role, strlen(role));//必选参数，取值与vcn保持一致
    paramBuilder->param("language",1);//必须参数，1是中文，2是英文，是英文时，对应文本也得是英文

    if(speed > 10 || speed < 0)
    {
        tmpSpeed = 100;
    }
    else
    {
        tmpSpeed = speed * 10;
    }

    paramBuilder->param("speed", tmpSpeed); //可选参数，默认为0
    //paramBuilder->param("pitch",100); //可选参数，默认为0
    if(volume > 10 || volume < 0)
    {
        tmpVolume = 100;
    }
    else
    {
        tmpVolume = volume * 10;
    }
    paramBuilder->param("volume",tmpVolume);//可选参数，默认为0
    // if(0 == strncmp(encoding, "gbk", 3))
    // {
    //     paramBuilder->param("textEncoding","GBK", strlen("GBK")); //可选参数，文本编码格式，默认为65001，UTF8格式
    //     // LOG_Debug("audio_control encoding: %s", encoding);
    //     snprintf(tmpStr, 32, "%s", ttsStr);
    // }
    // else if(0 == strncmp(encoding, "utf", 3))
    // {
        paramBuilder->param("textEncoding","UTF-8", strlen("UTF-8")); //可选参数，文本编码格式，默认为65001，UTF8格式
        // snprintf(tmpStr, 1024, "%s", ttsStr);
    // }
    // string cntext = tmpStr;
    // else
    // {
        // paramBuilder->param("textEncoding","GBK", strlen("GBK")); //可选参数，文本编码格式，默认为65001，UTF8格式
        // LOG_Debug("AIKIT_Start encoding:%s", encoding);
    // }
    
    int ret = AIKIT_Start(ABILITY,AIKIT_Builder::build(paramBuilder),nullptr,&handle);
    LOG_Debug("AIKIT_Start:%d\n",ret);
    LOG_Debug("%s\n", cntext.c_str());
    if(ret != 0){
        goto exit;
    }
    dataBuilder = AIKIT_DataBuilder::create();
    dataBuilder->clear();
    aiText_raw = AiText::get("text")->data(cntext.c_str(),cntext.length())->once()->valid();
    dataBuilder->payload(aiText_raw);

    
    // fin = fopen("./output.pcm", "wb");
    // if (fin == nullptr) {
    //     printf("fopen output.pcm fail.\n");
    //     goto exit;
    // }
    ret = AIKIT_Write(handle,AIKIT_Builder::build(dataBuilder));
    LOG_Debug("AIKIT_Write:%d\n",ret);
    if(ret != 0){
        fclose(fin);
        goto exit;
    }

    while(ttsFinished != true){
        usleep(1000);
    }
    
    ret = AIKIT_End(handle);

    exit:
        // if (fin != nullptr) {
        //     fclose(fin);
        //     fin = nullptr;
        // }   
        if(paramBuilder != nullptr){
            delete paramBuilder;
            paramBuilder = nullptr;
        }

        if(dataBuilder != nullptr){
            delete dataBuilder;
            dataBuilder = nullptr;
        }
        
}




void AudioAikitSdkInt(){

    AIKIT_SetLogInfo(LOG_LVL_VERBOSE, LOG_FILE, "/oemdata/tts/aeeLog.txt");

    AIKIT_Configurator::builder()
        .app()
			.appID("c060fe8f")
			.apiKey("8104f478b165750432869034e66b60b0")
			.apiSecret("MmIxOGM1ZjM3NTgxMDhjZjI3YTk0Yjgw")
			.workDir("/oemdata/tts/")
		.auth()
            .licenseFile("/oemdata/tts/aee.license")
            .UDID("1111111202511030000000000000001")
			.authType(0);
		//.log()
		//	.logLevel(LOG_LVL_INFO)
		//	.logPath("./");
    int ret = AIKIT_Init();
    if(ret != 0){
        LOG_Debug("AIKIT_Init failed:%d\n",ret);
        return;
    }
    AIKIT_Callbacks cbs = {OnOutput,OnEvent,OnError};
    AIKIT_RegisterAbilityCallback(ABILITY,cbs);
    // AIKIT_RegisterCallback(cbs);      

    // if(socketFd < 2)
    // {
    //     socketFd = AudioUdpBindPort(9987);
    // }

}



int testTtts(int32_t speed, int32_t volume) {
    // AIKIT_SetLogInfo(LOG_LVL_VERBOSE, LOG_FILE, "/oemdata/tts/aeeLog.txt");
    // if(socketFd < 2)
    // {
    //     socketFd = AudioUdpBindPort(9987);
    // }
    // AudioAikitSdkInt();
    AudioGenerateXTTS(speed, volume, "抱歉，音频测试，谢谢", "utf", 4);
	// AIKIT_UnInit();
    return 0;
}
