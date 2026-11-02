#include <stdint.h>
#include <glib.h>
#include <iostream>
#include <queue>
#include "sr_log.h"
#include "err_code.h"
#include "audio_playback.h"
#include "ql-sdk/ql_type.h"
#include "ql_audio_cfg.h"
#include "ql_ms_audio_pcm.h"
#include "ql_ms_audio_cfg.h"
#include "audio_common.h"
#include "audio_control.h"
#include "audio_get_times.h"

using namespace std;

#define OPEN_TTS_PLAY_BACK          (1)
#define OPEN_SHOUT_PLAY_BACK        (2)
#define OPEN_FILE_PLAY_BACK         (3)

static ql_audio_handle_t g_audioPlaybackHdl = QL_AUDIO_INVALID_HANDLE;

static pthread_mutex_t g_ttsOrShoutLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_shoutLock = PTHREAD_MUTEX_INITIALIZER;
static uint8_t g_ttsOrShoutFlag = 0;
static uint8_t g_awFileStatus = 0;
static uint8_t g_ttsIdle = 0;
static uint8_t g_notStopShoutFlag = 0;
static char g_playFileName[128] = {0};
static queue<AudioTTSPcm_t>  g_audioTTsQue; 
static int32_t g_playbackBufferSize = 0;
static queue<AudioTTSPcm_t>  g_audioShoutQue; 

int32_t AudioApiInit(void)
{
    int ret = QL_ERR_OK;

    ret = ql_ms_audio_init();
    if (QL_ERR_OK != ret)
    {
        LOG_Error("%s Failed, ret = %d", __FUNCTION__, ret);
    }
    else
    {
        LOG_Debug("%s Successful", __FUNCTION__);
    }
}

int32_t AudioApiDeinit(void)
{
    int ret = QL_ERR_OK;

    ret = ql_ms_audio_deinit();
    if (QL_ERR_OK != ret)
    {
        LOG_Error("%s Failed, ret = %d", __FUNCTION__, ret);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s Successful", __FUNCTION__);
        return RET_OK;
    }
}


int32_t AudioApiPlaybackOpen(void)
{
    uint32_t beDaiMask = 0;
    ql_audio_handle_t hdl = QL_AUDIO_INVALID_HANDLE;

    LOG_Debug("(0-primary i2s/1-secondary pcm/2-tertiary i2s/3-voice uplink) and 0");

    beDaiMask = QL_AUDIO_BE_DAI_MASK_PLAYBACK_PRI_I2S;

    if(QL_AUDIO_INVALID_HANDLE != g_audioPlaybackHdl)
    {
        LOG_Debug("%s Successful", __FUNCTION__);
        return RET_OK;
    }

    hdl = ql_ms_audio_playback_open(QL_AUDIO_FE_PCM_DEV_MULTIMEDIA1, beDaiMask);
    if (QL_AUDIO_INVALID_HANDLE == hdl)
    {
        LOG_Error("%s Failed", __FUNCTION__);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s Successful", __FUNCTION__);
        g_audioPlaybackHdl = hdl;
        return RET_OK;
    }
}

int32_t AudioApiPlaybackClose(void)
{
    int ret = QL_ERR_OK;

    if(QL_AUDIO_INVALID_HANDLE == g_audioPlaybackHdl)
    {
        LOG_Debug("%s Successful,  Audio Api Playback is Closed", __FUNCTION__);
        return RET_ERR;
    }

    ret = ql_ms_audio_playback_close(g_audioPlaybackHdl);
    if (QL_ERR_OK != ret)
    {
        LOG_Error("%s Failed, ret = %d", __FUNCTION__, ret);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s Successful", __FUNCTION__);
        g_audioPlaybackHdl = QL_AUDIO_INVALID_HANDLE;
        return RET_OK;
    }
}


int32_t AudioApiPlaybackPlay(void)
{
    int ret = QL_ERR_OK;

    if(QL_AUDIO_INVALID_HANDLE == g_audioPlaybackHdl)
    {
        LOG_Error("%s Failed,  Audio Api Playback not open", __FUNCTION__);
        return RET_ERR;
    }

    ret = ql_ms_audio_playback_play(g_audioPlaybackHdl);
    if (QL_ERR_OK != ret)
    {
        LOG_Error("%s Failed, ret = %d", __FUNCTION__, ret);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s Successful", __FUNCTION__);
        return RET_OK;
    }
}

int32_t AudioApiPlaybackStop(void)
{
    int ret = QL_ERR_OK;

    if(QL_AUDIO_INVALID_HANDLE == g_audioPlaybackHdl)
    {
        LOG_Error("%s Failed,  Audio Api Playback not open", __FUNCTION__);
        return RET_ERR;
    }

    ret = ql_ms_audio_playback_stop(g_audioPlaybackHdl);
    if (QL_ERR_OK != ret)
    {
        LOG_Error("%s Failed, ret = %d", __FUNCTION__, ret);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s Successful", __FUNCTION__);
        return RET_OK;
    }
}

int32_t AudioApiPlaybackPause(void)
{
    int ret = QL_ERR_OK;

    if(QL_AUDIO_INVALID_HANDLE == g_audioPlaybackHdl)
    {
        LOG_Error("%s Failed,  Audio Api Playback not open", __FUNCTION__);
        return RET_ERR;
    }

    ret = ql_ms_audio_playback_pause(g_audioPlaybackHdl);
    if (QL_ERR_OK != ret)
    {
        LOG_Error("%s Failed, ret = %d", __FUNCTION__, ret);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s Successful", __FUNCTION__);
        return RET_OK;
    }
}

int32_t AudioApiPlaybackResume(void)
{
    int ret = QL_ERR_OK;

    if(QL_AUDIO_INVALID_HANDLE == g_audioPlaybackHdl)
    {
        LOG_Error("%s Failed,  Audio Api Playback not open", __FUNCTION__);
        return RET_ERR;
    }

    ret = ql_ms_audio_playback_resume(g_audioPlaybackHdl);
    if (QL_ERR_OK != ret)
    {
        LOG_Error("%s Failed, ret = %d", __FUNCTION__, ret);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s Successful", __FUNCTION__);
        return RET_OK;
    }
}

int32_t AudioApiPlaybackStreamPrepare(int32_t channels, int32_t format, int32_t sampleRate)
{
    int ret = QL_ERR_OK;
    ql_audio_pcm_config_t pcmConfig;

    if(QL_AUDIO_INVALID_HANDLE == g_audioPlaybackHdl)
    {
        LOG_Error("%s Failed,  Audio Api Playback not open", __FUNCTION__);
        return RET_ERR;
    }

    memset(&pcmConfig, 0, sizeof(pcmConfig));
    pcmConfig.num_channels = channels;
    pcmConfig.pcm_format = format;
    pcmConfig.sample_rate = sampleRate;

    ret = ql_ms_audio_playback_stream_prepare(g_audioPlaybackHdl, &pcmConfig, NULL, NULL);
    if (QL_ERR_OK != ret)
    {
        LOG_Error("%s Failed, ret = %d", __FUNCTION__, ret);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s Successful", __FUNCTION__);
        return RET_OK;
    }
}

int32_t AudioApiPlaybackGetBufferSize(void)
{
    int32_t ret = QL_ERR_OK;
    uint32_t playbackBufferSize = 0;

    if(QL_AUDIO_INVALID_HANDLE == g_audioPlaybackHdl)
    {
        LOG_Error("%s Failed,  Audio Api Playback not open", __FUNCTION__);
        return RET_ERR;
    }

    ret = ql_ms_audio_playback_buffer_size_get(g_audioPlaybackHdl, &playbackBufferSize);
    if (QL_ERR_OK != ret)
    {
        LOG_Error("%s Failed, ret = %d", __FUNCTION__, ret);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s Successful", __FUNCTION__);
        return playbackBufferSize;
    }
}

int32_t AudioApiPlaybackPushStream(const char *buffer, int32_t bufferSize)
{
    uint32_t ret = QL_ERR_OK;

    if(QL_AUDIO_INVALID_HANDLE == g_audioPlaybackHdl)
    {
        LOG_Error("%s Failed,  Audio Api Playback not open", __FUNCTION__);
        return RET_ERR;
    }

    ret = ql_ms_audio_playback_push_stream(g_audioPlaybackHdl, (void *)buffer, bufferSize);
    if (QL_ERR_OK != ret)
    {
        LOG_Error("%s Failed, ret = %d", __FUNCTION__, ret);
        return RET_ERR;
    }
    else
    {
        // LOG_Debug("%s Successful", __FUNCTION__);
        return RET_OK;
    }
}

static int AudioPlaybackStateCallBack(ql_audio_handle_t hdl, void *params, QL_AUDIO_PLAYBACK_STATE_E state)
{
    switch (state)
    {
        case QL_AUDIO_PLAYBACK_STATE_OPEN:
            LOG_Debug("State: open play file Success");
            break;
        case QL_AUDIO_PLAYBACK_STATE_PREPARE:
            LOG_Debug("State: play file prepare");
            break;
        case QL_AUDIO_PLAYBACK_STATE_PLAYING:
            LOG_Debug("State: play file playing");
            break;
        case QL_AUDIO_PLAYBACK_STATE_FINISHED:
            pthread_mutex_lock(&g_ttsOrShoutLock);
            g_ttsOrShoutFlag = OPEN_TTS_PLAY_BACK;
            pthread_mutex_unlock(&g_ttsOrShoutLock);
            LOG_Debug("State: play file FINISHED");
            break;
        case QL_AUDIO_PLAYBACK_STATE_ERROR:
        case QL_AUDIO_PLAYBACK_STATE_PAUSE:
            LOG_Debug("State: %d", state);
            // pthread_mutex_lock(&g_ttsOrShoutLock);
            // g_ttsOrShoutFlag = OPEN_TTS_PLAY_BACK;
            // pthread_mutex_unlock(&g_ttsOrShoutLock);
            break;
        case QL_AUDIO_PLAYBACK_STATE_CLOSE:
            break;
    }

    return RET_OK;
}

static void AudioApiPlaybackSetStateIndCallBack(void)
{
    ql_ms_audio_playback_state_ind_cb_set(AudioPlaybackStateCallBack, NULL);
}



int32_t AudioApiPlaybackPlayFile(const char *fileName)
{
    uint32_t ret = QL_ERR_OK;

    ql_ms_audio_playback_file_prepare(g_audioPlaybackHdl, fileName, NULL, NULL, NULL);
    if (QL_ERR_OK != ret)
    {
        LOG_Error("%s Failed, ret = %d", __FUNCTION__, ret);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s Successful", __FUNCTION__);
        return RET_OK;
    }
}


int32_t AudioApiSetVolum(int32_t volum)
{
    uint32_t ret = QL_ERR_OK;
    int32_t tmpVolum = 0xBF;
    char cmdBuff1[128] = {0};
    char cmdBuff2[128] = {0};

    if(volum >= 50)
    {
       tmpVolum = tmpVolum + (volum - 50) / 2; 
    }
    else
    {
        tmpVolum = tmpVolum - (50 - volum); 
    }

    snprintf(cmdBuff1, 128, "ql_cmd at at+QIIC=0,0x10,0x46,1,0x%x", tmpVolum);
    snprintf(cmdBuff2, 128, "ql_cmd at at+QIIC=0,0x10,0x47,1,0x%x", tmpVolum);
    ret = SrGetSystemRetValue(cmdBuff1);
    if (RET_OK != ret)
    {
        LOG_Error("%s Failed, ret = %d", __FUNCTION__, ret);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s Successful", __FUNCTION__);
    }

    ret = SrGetSystemRetValue(cmdBuff2);
    if (RET_OK != ret)
    {
        LOG_Error("%s Failed, ret = %d", __FUNCTION__, ret);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s Successful", __FUNCTION__);
    }
    return RET_OK;
}

int32_t AudioPlayAwFile(const char *fileName)
{
    if(NULL == fileName)
    {
        LOG_Error("%s fileName is NULL", __FUNCTION__);
        return RET_ERR;
    }

    if(strlen(fileName) >= 128)
    {
        LOG_Error("%s fileName is too long", __FUNCTION__);
        return RET_ERR;
    }

    if(0 != access(fileName, F_OK))
    {
        LOG_Error("%s fileName not exists", __FUNCTION__);
        return RET_ERR;
    }

    pthread_mutex_lock(&g_ttsOrShoutLock);
    memset(g_playFileName, 0 , 128);
    memcpy(g_playFileName, fileName , strlen(fileName));
    
    

    if(OPEN_SHOUT_PLAY_BACK != g_ttsOrShoutFlag)
    {
        g_ttsOrShoutFlag = OPEN_FILE_PLAY_BACK;
        // AudioUdpSend(UDP_AUDIO_BAND_ADDR, AUDIO_PLAY_BACK_TTS_BIND_PORT,"1", 1);
        LOG_Debug("%s set play file %s", __FUNCTION__, g_playFileName);
    }
    pthread_mutex_unlock(&g_ttsOrShoutLock);

    return RET_OK;
}


gboolean AudioTimertimeout(gpointer user_data) 
{
    pthread_mutex_lock(&g_ttsOrShoutLock);
    LOG_Debug("%s g_notStopShoutFlag= %d success", __FUNCTION__, g_notStopShoutFlag);
    if(1 == g_notStopShoutFlag)
    {
        g_ttsOrShoutFlag = OPEN_TTS_PLAY_BACK;
        AudioUdpSend(UDP_AUDIO_BAND_ADDR, AUDIO_PLAY_BACK_SHOUT_BIND_PORT,"1", 1);
    }
    g_notStopShoutFlag--; 
    pthread_mutex_unlock(&g_ttsOrShoutLock);
    return FALSE;
}

int32_t AudioClearShoutQue()
{
    AudioTTSPcm_t shoutPcmData;

    LOG_Debug("%s", __FUNCTION__);
    pthread_mutex_lock(&g_shoutLock);
    while(true != g_audioShoutQue.empty())
    {
        shoutPcmData = g_audioShoutQue.front(); 
	    g_audioShoutQue.pop();
    }
    pthread_mutex_unlock(&g_shoutLock);
    
    return RET_OK;
}

int32_t AudioOpenShoutMs(int32_t msTime) 
{

    if(OPEN_FILE_PLAY_BACK == g_ttsOrShoutFlag)
    {
        g_awFileStatus = 1;
        AudioApiPlaybackPause();
        pthread_mutex_lock(&g_ttsOrShoutLock);
        g_ttsOrShoutFlag = OPEN_TTS_PLAY_BACK;
        pthread_mutex_unlock(&g_ttsOrShoutLock);
        usleep(30000);
    }

    if(msTime > 0)
    {
        pthread_mutex_lock(&g_ttsOrShoutLock);
        g_ttsOrShoutFlag = OPEN_SHOUT_PLAY_BACK;
        g_notStopShoutFlag++;
        g_timeout_add(msTime, AudioTimertimeout, NULL);
        pthread_mutex_unlock(&g_ttsOrShoutLock);
        LOG_Debug("%s= %d success", __FUNCTION__, msTime);
        AudioClearShoutQue();
        // AudioUdpSend(UDP_AUDIO_BAND_ADDR, AUDIO_PLAY_BACK_TTS_BIND_PORT,"1", 1);
        return RET_OK;
    }
    else if (msTime == 0)
    {
        pthread_mutex_lock(&g_ttsOrShoutLock);
        g_ttsOrShoutFlag = OPEN_SHOUT_PLAY_BACK;
        g_notStopShoutFlag = 0;
        pthread_mutex_unlock(&g_ttsOrShoutLock);
        LOG_Debug("%s = 0 success", __FUNCTION__);
        AudioClearShoutQue();
        // AudioUdpSend(UDP_AUDIO_BAND_ADDR, AUDIO_PLAY_BACK_TTS_BIND_PORT,"1", 1);
        return RET_OK;
    }
    else
    {
        LOG_Error("%s error, msTime = %d", __FUNCTION__, msTime);
        return RET_ERR;
    }
}

int32_t AudioCloseShout() 
{
    pthread_mutex_lock(&g_ttsOrShoutLock);
    g_ttsOrShoutFlag = OPEN_TTS_PLAY_BACK;
    pthread_mutex_unlock(&g_ttsOrShoutLock);
    LOG_Debug("%s success", __FUNCTION__);
    AudioUdpSend(UDP_AUDIO_BAND_ADDR, AUDIO_PLAY_BACK_SHOUT_BIND_PORT,"1", 1);
    return RET_OK;
}

int32_t AudioTTSIdleStatus()
{
    // if(1 == g_ttsIdle)
    // {
    //     return RET_OK;
    // }
    // else
    // {
    //     return RET_ERR;
    // }
}

int32_t AudioPushPcmDataToQue(uint8_t *pcmData, uint32_t pcmDataLen)
{
    AudioTTSPcm_t tmpPcmData;

    if(NULL == pcmData)
    {
        LOG_Error("%s pcmData is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(pcmDataLen > PCM_DATA_MAX_LEN)
    {
        LOG_Error("%s pcmDataLen[%d] >  PCM_DATA_MAX_LEN", __FUNCTION__, pcmDataLen);
        return RET_ERR;
    }

    if(pcmDataLen <= 0)
    {
        LOG_Error("%s pcmDataLen[%d] <= 0", __FUNCTION__, pcmDataLen);
        return RET_ERR;
    }

    tmpPcmData.pcmDataLen = pcmDataLen;
    memcpy(tmpPcmData.pcmData, pcmData, pcmDataLen);
    pthread_mutex_lock(&g_ttsOrShoutLock);
    g_audioTTsQue.push(tmpPcmData);
    pthread_mutex_unlock(&g_ttsOrShoutLock);

    return RET_OK;
}

int32_t AudioGetPlayAWFileStatus()
{
    int32_t retValue = RET_ERR;
    pthread_mutex_lock(&g_ttsOrShoutLock);

    if(OPEN_FILE_PLAY_BACK == g_ttsOrShoutFlag)
    {
        retValue = RET_ERR;
    }
    else
    {
        retValue = RET_OK;
    }
    pthread_mutex_unlock(&g_ttsOrShoutLock);

    return retValue;
}

int32_t AudioGetPlayShoutStatus()
{
    int32_t retValue = RET_ERR;
    pthread_mutex_lock(&g_ttsOrShoutLock);

    if(OPEN_SHOUT_PLAY_BACK == g_ttsOrShoutFlag)
    {
        retValue = RET_ERR;
    }
    else
    {
        retValue = RET_OK;
    }
    pthread_mutex_unlock(&g_ttsOrShoutLock);

    return retValue;
}

int32_t AudioGetPlayTtsStatus()
{
    int32_t retValue = RET_ERR;
    pthread_mutex_lock(&g_ttsOrShoutLock);

    if(OPEN_TTS_PLAY_BACK == g_ttsOrShoutFlag)
    {
        retValue = RET_OK;
    }
    else
    {
        retValue = RET_ERR;
    }
    pthread_mutex_unlock(&g_ttsOrShoutLock);

    return retValue;
}

int32_t AudioGetPlayFileStatus()
{
    int32_t retValue = RET_ERR;
    pthread_mutex_lock(&g_ttsOrShoutLock);

    if(OPEN_FILE_PLAY_BACK == g_ttsOrShoutFlag)
    {
        retValue = RET_OK;
    }
    else
    {
        retValue = RET_ERR;
    }
    pthread_mutex_unlock(&g_ttsOrShoutLock);

    return retValue;
}

uint64_t AudioGetFileTimes(void)
{
    if(strlen(g_playFileName) <= 0)
    {
        return RET_OK;
    }

    if(NULL != strstr(g_playFileName, ".wav"))
    {
        return get_wav_duration(g_playFileName) * 1000;
    }
    else if(NULL != strstr(g_playFileName, ".pcm"))
    {
        return get_pcm_duration(g_playFileName, 16000, 1, 16) * 1000;
    }
    else if(NULL != strstr(g_playFileName, ".mp3"))
    {
        return get_mp3_duration(g_playFileName) * 1000;
    }
    else
    {
        return RET_OK;
    }
}

int32_t AudioInterruptTts()
{
    AudioTTSPcm_t ttsPcmData;
    uint8_t sleepFlag = 0;

    LOG_Debug("%s %d", __FUNCTION__, g_ttsOrShoutFlag);
    pthread_mutex_lock(&g_ttsOrShoutLock);
    if(OPEN_FILE_PLAY_BACK == g_ttsOrShoutFlag)
    {
        g_awFileStatus = 1;
        AudioApiPlaybackPause();
        g_ttsOrShoutFlag = OPEN_TTS_PLAY_BACK;
        sleepFlag = 1;
    }

    while(true != g_audioTTsQue.empty())
    {
        ttsPcmData = g_audioTTsQue.front(); 
	    g_audioTTsQue.pop();
    }
    pthread_mutex_unlock(&g_ttsOrShoutLock);

    if(1 == sleepFlag)
    {
        usleep(30000);
    }

    return RET_OK;
}

int32_t AudioIsTTSQueEmpty()
{
    int32_t retValue = RET_ERR;

    pthread_mutex_lock(&g_ttsOrShoutLock);
    if(g_audioTTsQue.empty())
    {
        retValue = RET_OK;
    }
    else
    {
        retValue = RET_ERR;
    }
    pthread_mutex_unlock(&g_ttsOrShoutLock);

    return retValue;
}


int32_t AudioPlayBackTTS()
{
    int32_t socketFd = -1;
    char audioBuff[2048] = {0};
    char playbackBuffer[2048] = {0};
    int32_t playbackBufferSize = 0;
    int32_t playbackSize = 0;
    int32_t recvLen = 0;
    AudioTTSPcm_t ttsPcmData;

    AudioApiPlaybackOpen();

    // AudioApiPlaybackStreamPrepare(2, 2, 16000);
    AudioApiPlaybackStreamPrepare(1, 2, 16000);

    AudioApiPlaybackPlay();

    playbackBufferSize = AudioApiPlaybackGetBufferSize();

    // socketFd = AudioUdpBindPort(AUDIO_PLAY_BACK_TTS_BIND_PORT);
    LOG_Debug("%s playbackBufferSize : %d Successful", __FUNCTION__, playbackBufferSize);

    while(1)
    {
        if(OPEN_TTS_PLAY_BACK != g_ttsOrShoutFlag)
        {
            LOG_Debug("%s exit", __FUNCTION__);
            break;
        }
        
        if(g_audioTTsQue.empty())
        {
            usleep(10000);
            continue;
        }

        pthread_mutex_lock(&g_ttsOrShoutLock);
        if(!g_audioTTsQue.empty())
        {
            ttsPcmData = g_audioTTsQue.front(); 
            g_audioTTsQue.pop();
        }
        pthread_mutex_unlock(&g_ttsOrShoutLock);
        // recvLen = AudioUdpRecFromFd(socketFd, audioBuff, 2048);
        
        if(ttsPcmData.pcmDataLen <= 0)
        {
            LOG_Debug("%s ttsPcmData.pcmDataLen <= 0: %d", __FUNCTION__, ttsPcmData.pcmDataLen);
            continue;
        }
        if((playbackSize + ttsPcmData.pcmDataLen) >= PCM_DATA_MAX_LEN)
        {
            LOG_Debug("%s ttsPcmData.pcmDataLen <= 0: %d", __FUNCTION__, ttsPcmData.pcmDataLen);
            continue;
        }

        memcpy(&playbackBuffer[playbackSize], ttsPcmData.pcmData, ttsPcmData.pcmDataLen);
        playbackSize += ttsPcmData.pcmDataLen;
        if(playbackSize >= playbackBufferSize)
        {
            AudioApiPlaybackPushStream(playbackBuffer, playbackBufferSize);
            playbackSize = 0;
        }
    }
    // close(socketFd);
    AudioApiPlaybackStop();
    AudioApiPlaybackClose();

    return RET_OK;
}



int32_t AudioPlayBackShout()
{
    int32_t playbackBufferSize = 0;
    int32_t playbackSize = 0;
    int32_t recvLen = 0;
    AudioTTSPcm_t shoutPcmData;

    AudioApiPlaybackOpen();

    AudioApiPlaybackStreamPrepare(2, 2, 16000);
    // AudioApiPlaybackStreamPrepare(1, 2, 16000);

    AudioApiPlaybackPlay();

    playbackBufferSize = AudioApiPlaybackGetBufferSize();

    pthread_mutex_lock(&g_shoutLock);
    g_playbackBufferSize = playbackBufferSize;
    pthread_mutex_unlock(&g_shoutLock);
    LOG_Debug("%s playbackBufferSize : %d Successful", __FUNCTION__, playbackBufferSize);

    while(1)
    {
        if(OPEN_SHOUT_PLAY_BACK != g_ttsOrShoutFlag)
        {
            LOG_Debug("%s exit", __FUNCTION__);
            break;
        }
        pthread_mutex_lock(&g_shoutLock);
        if(g_audioShoutQue.empty())
        {
            pthread_mutex_unlock(&g_shoutLock);
            usleep(10000);
            continue;
        }

        shoutPcmData = g_audioShoutQue.front(); 
        g_audioShoutQue.pop();
        pthread_mutex_unlock(&g_shoutLock);
        // LOG_Debug("%s shoutPcmData.pcmDataLen : %d Successful", __FUNCTION__, shoutPcmData.pcmDataLen);
        if(shoutPcmData.pcmDataLen == playbackBufferSize)
        {
            AudioApiPlaybackPushStream((char *)(shoutPcmData.pcmData), playbackBufferSize);
        }
    }
    pthread_mutex_lock(&g_shoutLock);
    g_playbackBufferSize = 0;
    pthread_mutex_unlock(&g_shoutLock);
    AudioSendTTSCmdNotDoing();
    usleep(200000);
    AudioApiPlaybackStop();
    AudioApiPlaybackClose();
    
    return RET_OK;
}


int32_t AudioPlayFile()
{
    int32_t socketFd = -1;
    char audioBuff[2048] = {0};
    char playbackBuffer[2048] = {0};
    int32_t playbackBufferSize = 0;
    int32_t playbackSize = 0;
    int32_t recvLen = 0;

    AudioApiPlaybackOpen();

    AudioApiPlaybackPlayFile(g_playFileName);

    AudioApiPlaybackPlay();


    g_awFileStatus = 0;
    while(1)
    {
        if(OPEN_FILE_PLAY_BACK != g_ttsOrShoutFlag)
        {
            LOG_Debug("%s exit", __FUNCTION__);
            break;
        }
        // LOG_Debug("%s ", __FUNCTION__);
        usleep(10000);
    }

    if(0 != g_awFileStatus)
    {
        AudioApiPlaybackResume();
    }
    usleep(50000);
    AudioApiPlaybackStop();
    AudioApiPlaybackClose();
    LOG_Debug("%s while exit", __FUNCTION__);
    return RET_OK;
}


void *AudioRecvPlayBackDataThread(void *argv)
{
    int32_t socketFd = -1;
    char audioBuff[2048] = {0};
    char playbackBuffer[PCM_DATA_MAX_LEN] = {0};
    int32_t playbackSize = 0;
    int32_t recvLen = 0;
    AudioTTSPcm_t audioShoutData;
    LOG_Debug("%s  tid: %d", __FUNCTION__, gettid());
    socketFd = AudioUdpBindPort(AUDIO_PLAY_BACK_SHOUT_BIND_PORT);

    while(1)
    {
        recvLen = AudioUdpRecFromFd(socketFd, audioBuff, 2048);
        
        if(recvLen <= 0)
        {
            LOG_Error("%s AudioUdpRecFromFd error ", __FUNCTION__);
            continue;
        }

        if(g_playbackBufferSize <= 0)
        {
            LOG_Error("%s g_playbackBufferSize:%d ", __FUNCTION__, g_playbackBufferSize);
            continue;
        }

        memcpy(&playbackBuffer[playbackSize], audioBuff, recvLen);
        playbackSize += recvLen;
        if(playbackSize >= g_playbackBufferSize)
        {
            LOG_Debug("%s shoutPcmData.pcmDataLen : %d Successful", __FUNCTION__, g_playbackBufferSize);
            memset(audioShoutData.pcmData, 0, PCM_DATA_MAX_LEN);
            memcpy(audioShoutData.pcmData, playbackBuffer, g_playbackBufferSize);
            audioShoutData.pcmDataLen = g_playbackBufferSize;
            pthread_mutex_lock(&g_shoutLock);  
            g_audioShoutQue.push(audioShoutData);
            pthread_mutex_unlock(&g_shoutLock);
            playbackSize = 0;
        }        
    }

    return RET_OK;
}


void *AudioPlayBackThread(void *argv)
{
    LOG_Debug("%s  tid: %d", __FUNCTION__, gettid());
    g_ttsOrShoutFlag = OPEN_TTS_PLAY_BACK;
    AudioApiPlaybackSetStateIndCallBack();

    while(1)
    {
        if(OPEN_SHOUT_PLAY_BACK != g_ttsOrShoutFlag && OPEN_FILE_PLAY_BACK != g_ttsOrShoutFlag)
        {
            AudioPlayBackTTS();
        }
        else if(OPEN_FILE_PLAY_BACK == g_ttsOrShoutFlag)
        {
            AudioPlayFile();
        }
        else
        {
            AudioPlayBackShout();
        }
    }
    

    // int32_t socketFd = -1;
    // char audioBuff[2048] = {0};
    // char playbackBuffer[2048] = {0};
    // int32_t playbackBufferSize = 0;
    // int32_t playbackSize = 0;
    // int32_t recvLen = 0;

    // AudioApiPlaybackOpen();

    // // AudioApiPlaybackStreamPrepare(2, 2, 16000);
    // AudioApiPlaybackStreamPrepare(1, 2, 16000);

    // AudioApiPlaybackPlay();

    // playbackBufferSize = AudioApiPlaybackGetBufferSize();

    // socketFd = AudioUdpBindPort(AUDIO_PLAY_BACK_BIND_PORT);
    // LOG_Debug("%s playbackBufferSize : %d Successful", __FUNCTION__, playbackBufferSize);

    // while(1)
    // {
    //     recvLen = AudioUdpRecFromFd(socketFd, audioBuff, 2048);
        
    //     while(recvLen <= 0)
    //     {
    //         // AudioApiPlaybackPushStream(audioBuff, playbackBufferSize);
    //         // recvLen -= playbackBufferSize;
    //         continue;
    //     }
    //     memcpy(&playbackBuffer[playbackSize], audioBuff, recvLen);
    //     playbackSize += recvLen;
    //     if(playbackSize >= playbackBufferSize)
    //     {
    //         AudioApiPlaybackPushStream(playbackBuffer, playbackBufferSize);
    //         playbackSize = 0;
    //     }
    // }

    // AudioApiPlaybackStop();
    // AudioApiPlaybackClose();
}



    
