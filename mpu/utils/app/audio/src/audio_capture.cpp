#include <stdint.h>
#include <iostream>
#include <queue>
#include "sr_log.h"
#include "err_code.h"
#include "ql-sdk/ql_type.h"
#include "ql_ms_audio_pcm.h"
#include "ql_ms_audio_cfg.h"
#include "audio_common.h"
#include "audio_capture.h"
#include "audio_playback.h"

#define AUDIO_CAPTURE_BIND_PORT (7992)
#define AUDIO_CAPTURE_FILE_FLAG             (2)
#define AUDIO_CAPTURE_OPEN_FLAG             (1)
#define AUDIO_CAPTURE_CLOSE_FLAG            (0)

using namespace std;
static ql_audio_handle_t g_audioCaptureHdl = QL_AUDIO_INVALID_HANDLE;
static pthread_mutex_t g_captureLock = PTHREAD_MUTEX_INITIALIZER;
static uint8_t g_captureFlag = 0;
static uint8_t g_notCloseCaptureFlag = 0;
static queue<AudioTTSPcm_t>  g_audioCaptureQue; 
static int32_t g_socketFd = -1;


int32_t AudioApiCaptureOpen(void)
{
    uint32_t beDaiMask = 0;
    ql_audio_handle_t hdl = QL_AUDIO_INVALID_HANDLE;

    LOG_Debug("(0-primary i2s/1-secondary pcm/2-tertiary i2s/3-voice downlink)");
    
    if(QL_AUDIO_INVALID_HANDLE != g_audioCaptureHdl)
    {
        LOG_Debug("%s Successful", __FUNCTION__);
        return RET_OK;
    }

    beDaiMask = QL_AUDIO_BE_DAI_MASK_CAPTURE_PRI_I2S;
    hdl= ql_ms_audio_capture_open(QL_AUDIO_FE_PCM_DEV_MULTIMEDIA1, beDaiMask);
    if (QL_AUDIO_INVALID_HANDLE == hdl)
    {
        LOG_Error("%s Failed", __FUNCTION__);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s Successful", __FUNCTION__);
        g_audioCaptureHdl = hdl;
        return RET_OK;
    }
}

int32_t AudioApiCaptureFilePrepare(const char *fileName, uint32_t sampleRate, uint8_t channels)
{
    int ret = QL_ERR_OK;
    ql_audio_pcm_config_t config;

    if(NULL == fileName)
    {
        LOG_Error("%s fileName is NULL", __FUNCTION__);
        return RET_ERR;
    }

    if(QL_AUDIO_INVALID_HANDLE == g_audioCaptureHdl)
    {
        LOG_Error("%s error,  Audio Api Capture not Open", __FUNCTION__);
        return RET_ERR;
    }

    memset(&config, 0, sizeof(config));
    config.sample_rate = sampleRate;
    config.num_channels = channels;
    config.pcm_format = QL_AUDIO_FORMAT_PCM_S16_LE;

    ret = ql_ms_audio_capture_file_prepare(g_audioCaptureHdl, fileName,
                                        QL_AUDIO_STREAM_FORMAT_PCM, &config, NULL, NULL);
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

int32_t AudioApiCaptureRecord(void)
{
    int ret = QL_ERR_OK;

    if(QL_AUDIO_INVALID_HANDLE == g_audioCaptureHdl)
    {
        LOG_Error("%s error,  Audio Api Capture not Open", __FUNCTION__);
        return RET_ERR;
    }

    ret = ql_ms_audio_capture_record(g_audioCaptureHdl);
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

int32_t AudioApiCaptureStop(void)
{
    int ret = QL_ERR_OK;

    if(QL_AUDIO_INVALID_HANDLE == g_audioCaptureHdl)
    {
        LOG_Error("%s error,  Audio Api Capture not Open", __FUNCTION__);
        return RET_ERR;
    }

    ret = ql_ms_audio_capture_stop(g_audioCaptureHdl);
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

int32_t AudioApiCaptureClose(void)
{
    int ret = QL_ERR_OK;

    if(QL_AUDIO_INVALID_HANDLE == g_audioCaptureHdl)
    {
        LOG_Debug("%s Successful,  Audio Api Capture Closed", __FUNCTION__);
        return RET_OK;
    }

    ret = ql_ms_audio_capture_close(g_audioCaptureHdl);
    if (QL_ERR_OK != ret)
    {
        LOG_Error("%s Failed, ret = %d", __FUNCTION__, ret);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s Successful", __FUNCTION__);
        g_audioCaptureHdl = QL_AUDIO_INVALID_HANDLE;
        return RET_OK;
    }
}

int32_t item_ql_ms_audio_capture_pause(void)
{
    int ret = QL_ERR_OK;

    if(QL_AUDIO_INVALID_HANDLE == g_audioCaptureHdl)
    {
        LOG_Error("%s error,  Audio Api Capture not Open", __FUNCTION__);
        return RET_ERR;
    }

    ret = ql_ms_audio_capture_pause(g_audioCaptureHdl);
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

int32_t item_ql_ms_audio_capture_resume(void)
{
    int ret = QL_ERR_OK;

    if(QL_AUDIO_INVALID_HANDLE == g_audioCaptureHdl)
    {
        LOG_Error("%s error,  Audio Api Capture not Open", __FUNCTION__);
        return RET_ERR;
    }

    ret = ql_ms_audio_capture_resume(g_audioCaptureHdl);
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


int32_t AudioApiCaptureStreamPrepare(int32_t channels, int32_t format, int32_t sampleRate)
{
    int ret = QL_ERR_OK;
    ql_audio_pcm_config_t pcmConfig;

    if(QL_AUDIO_INVALID_HANDLE == g_audioCaptureHdl)
    {
        LOG_Error("%s error,  Audio Api Capture not Open", __FUNCTION__);
        return RET_ERR;
    }

    memset(&pcmConfig, 0, sizeof(pcmConfig));
    pcmConfig.num_channels = channels;
    pcmConfig.pcm_format = format;
    pcmConfig.sample_rate = sampleRate;

    ret = ql_ms_audio_capture_stream_prepare(g_audioCaptureHdl, &pcmConfig, NULL, NULL);
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


int32_t AudioApiCaptureGetBufferSize(void)
{
    int ret = QL_ERR_OK;
    uint32_t captureBufferSize = 0;

    if(QL_AUDIO_INVALID_HANDLE == g_audioCaptureHdl)
    {
        LOG_Error("%s error,  Audio Api Capture not Open", __FUNCTION__);
        return RET_ERR;
    }

    ret = ql_ms_audio_capture_buffer_size_get(g_audioCaptureHdl, &captureBufferSize);
    if (QL_ERR_OK != ret)
    {
        LOG_Error("%s Failed, ret = %d", __FUNCTION__, ret);
        return RET_ERR;
    }
    else
    {
        LOG_Debug("%s Successful", __FUNCTION__);
        return captureBufferSize;
    }
}

int32_t AudioApiCapturePullStream(char *buffer, int32_t bufferSize)
{
    uint32_t ret = QL_ERR_OK;

    if(NULL == buffer)
    {
        LOG_Error("%s buffer is NULL", __FUNCTION__);
        return RET_ERR;
    }

    if(QL_AUDIO_INVALID_HANDLE == g_audioCaptureHdl)
    {
        LOG_Error("%s Failed,  Audio Api Playback not open", __FUNCTION__);
        return RET_ERR;
    }

    ret = ql_ms_audio_capture_pull_stream(g_audioCaptureHdl, buffer, bufferSize);
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

static int audio_capture_state_cb(ql_audio_handle_t hdl, void *params, QL_AUDIO_CAPTURE_STATE_E state)
{
    switch (state)
    {
        case QL_AUDIO_CAPTURE_STATE_OPEN:
        case QL_AUDIO_CAPTURE_STATE_PREPARE:
        case QL_AUDIO_CAPTURE_STATE_CAPTURING:
        case QL_AUDIO_CAPTURE_STATE_FINISHED:
        case QL_AUDIO_CAPTURE_STATE_PAUSE:
        case QL_AUDIO_CAPTURE_STATE_ERROR:
        case QL_AUDIO_CAPTURE_STATE_CLOSE:
        default:
            LOG_Debug("State: %d", state);
            break;
    }

    return 0;
}

int32_t AudioApiSetCaptureCallBack()
{
    ql_ms_audio_capture_state_ind_cb_set(audio_capture_state_cb, NULL);
    LOG_Debug("%s Successful", __FUNCTION__);
    return RET_OK;
}



gboolean AudioCaptureTimertimeout(gpointer user_data) 
{
    pthread_mutex_lock(&g_captureLock);
    LOG_Debug("%s g_notCloseCaptureFlag= %d success", __FUNCTION__, g_notCloseCaptureFlag);
    if(1 == g_notCloseCaptureFlag)
    {
        g_captureFlag = AUDIO_CAPTURE_CLOSE_FLAG;
    }
    g_notCloseCaptureFlag--; 
    pthread_mutex_unlock(&g_captureLock);
    return FALSE;
}

gboolean AudioFileCaptureTimertimeout(gpointer user_data) 
{
    pthread_mutex_lock(&g_captureLock);
    LOG_Debug("%s  success", __FUNCTION__);
    g_captureFlag = AUDIO_CAPTURE_CLOSE_FLAG;
    pthread_mutex_unlock(&g_captureLock);
    return FALSE;
}

int32_t AudioOpenFileCaptureMs(int32_t msTime) 
{
    if(msTime > 0)
    {
        pthread_mutex_lock(&g_captureLock);
        g_captureFlag = AUDIO_CAPTURE_FILE_FLAG;
        g_timeout_add(msTime, AudioFileCaptureTimertimeout, NULL);
        pthread_mutex_unlock(&g_captureLock);
        LOG_Debug("%s= %d success", __FUNCTION__, msTime);
        return RET_OK;
    }
    else if (msTime == 0)
    {
        pthread_mutex_lock(&g_captureLock);
        g_captureFlag = AUDIO_CAPTURE_FILE_FLAG;
        g_timeout_add(10000, AudioFileCaptureTimertimeout, NULL);
        pthread_mutex_unlock(&g_captureLock);
        LOG_Debug("%s = 0 success", __FUNCTION__);
        return RET_OK;
    }
    else
    {
        LOG_Error("%s error, msTime = %d", __FUNCTION__, msTime);
        return RET_ERR;
    }
}

int32_t AudioOpenCaptureMs(int32_t msTime) 
{
    if(msTime > 0)
    {
        pthread_mutex_lock(&g_captureLock);
        g_captureFlag = AUDIO_CAPTURE_OPEN_FLAG;
        g_notCloseCaptureFlag++;
        g_timeout_add(msTime, AudioCaptureTimertimeout, NULL);
        pthread_mutex_unlock(&g_captureLock);
        LOG_Debug("%s= %d success", __FUNCTION__, msTime);
        return RET_OK;
    }
    else if (msTime == 0)
    {
        pthread_mutex_lock(&g_captureLock);
        g_captureFlag = AUDIO_CAPTURE_OPEN_FLAG;
        g_notCloseCaptureFlag = 0;
        pthread_mutex_unlock(&g_captureLock);
        LOG_Debug("%s = 0 success", __FUNCTION__);
        return RET_OK;
    }
    else
    {
        LOG_Error("%s error, msTime = %d", __FUNCTION__, msTime);
        return RET_ERR;
    }
}

int32_t AudioCloseCapture() 
{
    pthread_mutex_lock(&g_captureLock);
    g_captureFlag = AUDIO_CAPTURE_CLOSE_FLAG;
    pthread_mutex_unlock(&g_captureLock);
    LOG_Debug("%s success", __FUNCTION__);
    return RET_OK;
}


int32_t AudioApiOpenCapture(void)
{
    int32_t socketFd = -1;
    char audioBuff[1024] = {0};
    int32_t captureBufferSize = 0;
    int32_t recvLen = 0;
    AudioTTSPcm_t tmpPcmData;

    AudioApiCaptureOpen();

    AudioApiCaptureStreamPrepare(1, 2, 16000);

    AudioApiCaptureRecord();

    captureBufferSize = AudioApiCaptureGetBufferSize();

    socketFd = AudioUdpBindPort(AUDIO_CAPTURE_BIND_PORT);
    pthread_mutex_lock(&g_captureLock);
    g_socketFd = socketFd;
    pthread_mutex_unlock(&g_captureLock);

    while(1)
    {
        if(AUDIO_CAPTURE_CLOSE_FLAG == g_captureFlag)
        {
            break;
        }

        if(RET_OK == AudioApiCapturePullStream(audioBuff, captureBufferSize))
        {
            tmpPcmData.pcmDataLen = captureBufferSize;
            memcpy(tmpPcmData.pcmData, audioBuff, captureBufferSize);
            g_audioCaptureQue.push(tmpPcmData);
            // AudioUdpSendWithFd(socketFd, "192.168.1.2", AUDIO_CAPTURE_BIND_PORT, audioBuff, captureBufferSize);
        }
    }

    AudioApiCaptureStop();
    AudioApiCaptureClose();

    pthread_mutex_lock(&g_captureLock);
    close(socketFd);
    g_socketFd = -1;
    pthread_mutex_unlock(&g_captureLock);

    return RET_OK;
}

void *AudioApiCaptureDataSendThread(void *argv)
{
    LOG_Debug("%s  tid: %d", __FUNCTION__, gettid());
    AudioTTSPcm_t tmpPcmData;

    while(1)
    {
        if(g_audioCaptureQue.empty())
        {
            usleep(10000);
            continue;
        }

        if(AUDIO_CAPTURE_CLOSE_FLAG == g_captureFlag)
        {
            tmpPcmData = g_audioCaptureQue.front(); 
	        g_audioCaptureQue.pop();
            usleep(10000);
            continue;
        }

        tmpPcmData = g_audioCaptureQue.front(); 
	    g_audioCaptureQue.pop();
        pthread_mutex_lock(&g_captureLock);
        if(g_socketFd > 2)
        {
            AudioUdpSendWithFd(g_socketFd, "192.168.1.2", AUDIO_CAPTURE_BIND_PORT, (char *)tmpPcmData.pcmData, tmpPcmData.pcmDataLen);
        }
        else
        {
            LOG_Error("%s  g_socketFd <= 2 %d", __FUNCTION__, g_socketFd);
        }
        pthread_mutex_unlock(&g_captureLock);
        
    }

    return RET_OK;
}

int32_t AudioApiOpenCaptureForSaveFile(void)
{
    int32_t socketFd = -1;
    char audioBuff[1024] = {0};
    int32_t captureBufferSize = 0;
    int32_t recvLen = 0;

    AudioApiCaptureOpen();

    AudioApiCaptureFilePrepare(AUDIO_CAPTURE_FILE_PATH_NAME, 16000, 2);

    AudioApiCaptureRecord();
    while(1)
    {
        if(AUDIO_CAPTURE_FILE_FLAG != g_captureFlag)
        {
            break;
        }
        usleep(100000);
        LOG_Debug("%s is start", __FUNCTION__);
    }

    AudioApiCaptureStop();
    AudioApiCaptureClose();

    return RET_OK;
}


int32_t AudioCaptureGetStatus()
{
    if(AUDIO_CAPTURE_OPEN_FLAG == g_captureFlag)
    {
        return RET_OK;
    }
    else
    {
        return RET_ERR;
    }
}

void *AudioApiCaptureThread(void *argv)
{
    LOG_Debug("%s  tid: %d", __FUNCTION__, gettid());
    AudioApiSetCaptureCallBack();

    while(1)
    {
        if(AUDIO_CAPTURE_OPEN_FLAG == g_captureFlag)
        {
            AudioApiOpenCapture();
        }
        else if(AUDIO_CAPTURE_FILE_FLAG == g_captureFlag)
        {
            system("rm /oemdata/tts/audio_test.pcm");
            AudioApiOpenCaptureForSaveFile();
        }
        else
        {
            usleep(100000);
        }
    }
}