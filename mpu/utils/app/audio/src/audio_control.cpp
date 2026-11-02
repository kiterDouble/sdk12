#include <stdint.h>
#include <iostream>
#include <queue>
#include "audio_control.h"
#include "include/iconv.h"
#include "audio_config_file.h"
#include "sr_log.h"
#include "err_code.h"
#include "audio_common.h"
#include "audio_aikit.h"
#include "audio_playback.h"
#include "audio_capture.h"

#define  JSON_FEIL_PATH_NAME            "/oemdata/log/audio.json"
#define  AUDIO_CONTROL_BIND_PORT        (7889)

using namespace std;

static pthread_mutex_t g_controlLock = PTHREAD_MUTEX_INITIALIZER;
JsonFileHandler *g_jsonFileHandler = NULL;
Json::Value g_rootValue;
static queue<AudioControl_t>  g_audioControlQue; 
static queue<AudioControl_t>  g_audioNotTtsQue; 

GHashTable *g_hashTable = NULL;
static char g_configBuff[1024] = {0};
static char g_statusBuff[1024] = {0};
static char g_ttsBuff[1024] = {0};
static char g_devicesBuff[1024] = {0};
static char g_awFileBuff[128] = {0};
static uint32_t g_ttsBuffIdex = 0;
static int32_t g_cmdTimes = 0;
static char g_awCmdFlag = 0;
static uint32_t g_micDuration = 0;
static uint32_t g_shoutDuration = 0;
static uint64_t g_micStartTimes = 0;
static uint64_t g_playFileStartTimes = 0;
static uint64_t g_shoutStartTimes = 0;

const AudioCmdHandle_t g_AudioCmd[] = {
    {.cmdName = "#[GET]test", .callBack = NULL},\
    {.cmdName = "#[GET]get_config", .callBack = GetGetConfig},\
    {.cmdName = "#[GET]get_device_info", .callBack = GetGetDeviceInfo},\
    {.cmdName = "#[GET]get_status", .callBack = GetGetStatus},\
    {.cmdName = "#[GET]get_tts", .callBack = GetGetTts},\
    {.cmdName = "#[CONFIG]device_addr", .callBack = ConfigDeviceAddr},\
    {.cmdName = "#[CONFIG]control_port", .callBack = ConfigControlPort},\
    {.cmdName = "#[CONFIG]shout_port", .callBack = ConfigShoutPort},\
    {.cmdName = "#[CONFIG]shout_audio_codec", .callBack = ConfigShoutAudioCodec},\
    {.cmdName = "#[CONFIG]shout_audio_sample", .callBack = ConfigShoutAudioSample},\
    {.cmdName = "#[CONFIG]shout_audio_bit", .callBack = ConfigShoutAudioBit},\
    {.cmdName = "#[CONFIG]shout_volum", .callBack = ConfigShoutVolum},\
    {.cmdName = "#[CONFIG]mic_send_ip", .callBack = ConfigMicSendIp},\
    {.cmdName = "#[CONFIG]mic_send_port", .callBack = ConfigMicSendPort},\
    {.cmdName = "#[CONFIG]mic_audio_codec", .callBack = ConfigMicAudioCodec},\
    {.cmdName = "#[CONFIG]mic_audio_sample", .callBack = ConfigMicAudioSample},\
    {.cmdName = "#[CONFIG]mic_audio_bit", .callBack = ConfigMicAudioBit},\
    {.cmdName = "#[CONFIG]mic_frame_size", .callBack = ConfigMicFrameSize},\
    {.cmdName = "#[CONFIG]launch_tts", .callBack = ConfigLaunchTts},\
    {.cmdName = "#[CONFIG]upload_port", .callBack = ConfigUploadPort},\
    {.cmdName = "#[CONFIG]power_on_tts", .callBack = ConfigPowerOnTts},\
    {.cmdName = "#[CMD]open_shout", .callBack = CmdOpenShout},\
    {.cmdName = "#[CMD]close_shout", .callBack = CmdCloseShout},\
    {.cmdName = "#[CMD]open_mic", .callBack = CmdOpenMic},\
    {.cmdName = "#[CMD]close_mic", .callBack = CmdCloseMic},\
    {.cmdName = "#[CMD]reset", .callBack = CmdReset},\
    {.cmdName = "#[CMD]volum", .callBack = CmdVolum},\
    {.cmdName = "#[CMD]delete", .callBack = CmdDelete},\
    {.cmdName = "#[TTS]times", .callBack = TTSTimes},\
    {.cmdName = "#[TTS]interval", .callBack = TTSinterval},\
    {.cmdName = "#[TTS]codec", .callBack = TTSCodec},\
    {.cmdName = "#[TTS]m", .callBack = TTSM},\
    {.cmdName = "#[TTS]n", .callBack = TTSN},\
    {.cmdName = "#[TTS]p", .callBack = TTSP},\
    {.cmdName = "#[TTS]s", .callBack = TTSS},\
    {.cmdName = "#[TTS]v", .callBack = TTSV},\
    {.cmdName = "#[TTS]y", .callBack = TTSY},\
    {.cmdName = "#[TTS]=", .callBack = TTSDD},\
    {.cmdName = "#[TTS]b", .callBack = TTSB},\
    {.cmdName = "#[TTS]AW", .callBack = TTSAW},\
    {.cmdName = "#[TTS]tts", .callBack = TTSTts},\
    {.cmdName = "#[SRCMD]open_capture", .callBack = SrCmdStartCapyureFile},\
    {.cmdName = "#[SRCMD]play_capture", .callBack = SrCmdPlayCapyureFile}};


int gbk_to_utf8(const char *gbk_str, char *utf8_str, size_t utf8_len) 
{
    iconv_t cd;
    char *in_buf = (char *)gbk_str;
    char *out_buf = utf8_str;
    size_t in_len = strlen(gbk_str);
    size_t out_len = utf8_len;

    cd = iconv_open("UTF-8", "GBK");
    if (cd == (iconv_t)-1) {
        perror("iconv_open failed");
        return -1;
    }

    if (iconv(cd, &in_buf, &in_len, &out_buf, &out_len) == (size_t)-1) {
        perror("iconv failed");
        iconv_close(cd);
        return -1;
    }

    iconv_close(cd);
    *out_buf = '\0';
    return 0;
}

int32_t AudioCmdHashInit()
{
    int32_t arraySize = sizeof(g_AudioCmd) / sizeof(AudioCmdHandle_t);
    int32_t count = 0;

    if(NULL == g_hashTable)
    {
        g_hashTable = g_hash_table_new(g_str_hash, g_str_equal);
    }

    for(count = 0; count < arraySize; count++) 
    {
        g_hash_table_insert(g_hashTable, (gpointer)g_AudioCmd[count].cmdName, GINT_TO_POINTER(count));
    }
}

int32_t AudioCmdHashDestroy()
{
    if(NULL != g_hashTable)
    {
        g_hash_table_destroy(g_hashTable);
    }
}

int32_t GetCmdIndexFromHash(const char *cmdStr)
{
    int32_t arrayIndex = -1;
    gpointer result = NULL;

    if(NULL == g_hashTable)
    {
        LOG_Error("can upload cmdType is error");
        return RET_ERR;
    }

    result = g_hash_table_lookup(g_hashTable, cmdStr);
    if (result != NULL) 
    {
        arrayIndex = GPOINTER_TO_INT(result);
    } 
    else 
    {
        LOG_Error("未找到CMD: %s\n", cmdStr);
    }

    return arrayIndex;
}

int32_t GetAudioCmdHandleFromArray(int32_t index, AudioCmdHandle_t *audioCmdHandle)
{
    if(NULL == audioCmdHandle)
    {
        LOG_Error("%s argv is NULL", __FUNCTION__);
        return RET_ERR;
    }

    if(index >= (sizeof(g_AudioCmd) / sizeof(AudioCmdHandle_t)) || index < 0)
    {
        LOG_Error("%s index error", __FUNCTION__);
        return RET_ERR;
    }

    memcpy(audioCmdHandle->cmdName, g_AudioCmd[index].cmdName, 32);
    audioCmdHandle->callBack = g_AudioCmd[index].callBack;

    return RET_OK;
}

int32_t ConfigDeviceAddr(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return SetConfigJsonValueAsStr(cmdStr, cmdValue);
}
int32_t ConfigControlPort(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return SetConfigJsonValueAsInt(cmdStr, atoi(cmdValue));
}

int32_t ConfigShoutPort(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return SetConfigJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t ConfigShoutAudioCodec(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return SetConfigJsonValueAsStr(cmdStr, cmdValue);
}
int32_t ConfigShoutAudioSample(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return SetConfigJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t ConfigShoutAudioBit(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return SetConfigJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t ConfigShoutVolum(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    int32_t volum = 0;

    volum = atoi(cmdValue);
    if(volum > 10)
    {
        volum = 10;
    }

    AudioApiSetVolum(volum * 10);
    return SetConfigJsonValueAsInt(cmdStr, volum);
}
int32_t ConfigMicSendIp(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return SetConfigJsonValueAsStr(cmdStr, cmdValue);
}
int32_t ConfigMicSendPort(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return SetConfigJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t ConfigMicAudioCodec(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return SetConfigJsonValueAsStr(cmdStr, cmdValue);
}
int32_t ConfigMicAudioSample(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return SetConfigJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t ConfigMicAudioBit(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return SetConfigJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t ConfigMicFrameSize(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return SetConfigJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t ConfigLaunchTts(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return SetConfigJsonValueAsStr(cmdStr, cmdValue);
}
int32_t ConfigUploadPort(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return SetConfigJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t ConfigPowerOnTts(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    char ttsBuff[1024] = {0};
    char utf8Buff[1024] = {0};

    if(0 == strncmp(cmdValue, "[codec:gbk]", strlen("[codec:gbk]")) || \
    0 != strncmp(cmdValue, "[codec", strlen("[codec")))
    {
        gbk_to_utf8(cmdValue, utf8Buff, 1024);
    }
    else
    {
        memcpy(utf8Buff, cmdValue, strlen(cmdValue));
    }
    SetTTSJsonValueAsStr(cmdStr, utf8Buff);
    snprintf(ttsBuff, 1024, "#[TTS][codec:utf]%s", utf8Buff);

    LOG_Debug("ttsBuff:%s", ttsBuff);

    return SetConfigJsonValueAsStr(cmdStr, ttsBuff);
}
int32_t CmdOpenShout(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    if(0 == strlen(cmdValue))
    {
        SetStatusJsonValueAsInt("shout_status", 0);
        SetStatusJsonValueAsInt("shout_start", 0);
        SetStatusJsonValueAsInt("shout_remaining", 0);
        AudioOpenShoutMs(0);
        g_shoutDuration = 0;
        g_shoutStartTimes = g_get_monotonic_time() / 1000;
        return SetStatusJsonValueAsInt("shout_duration", 0);
    }
    else
    {
        g_shoutDuration = atoi(cmdValue);
        g_shoutStartTimes = g_get_monotonic_time() / 1000;
        SetStatusJsonValueAsInt("shout_status", 1);
        SetStatusJsonValueAsInt("shout_start", 0);
        SetStatusJsonValueAsInt("shout_remaining", atoi(cmdValue));
        AudioOpenShoutMs(atoi(cmdValue));
        return SetStatusJsonValueAsInt("shout_duration", atoi(cmdValue));
    }
}
int32_t CmdCloseShout(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    SetStatusJsonValueAsInt("shout_status", 0);
    SetStatusJsonValueAsInt("shout_start", 0);
    SetStatusJsonValueAsInt("shout_remaining", 0);
    AudioCloseShout();
    return SetStatusJsonValueAsInt("shout_duration", 0);
}
int32_t CmdOpenMic(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    if(0 == strlen(cmdValue))
    {
        SetStatusJsonValueAsInt("mic_status", 0);
        SetStatusJsonValueAsInt("mic_start", 0);
        SetStatusJsonValueAsInt("mic_remaining", 0);
        g_micDuration = 0;
        g_micStartTimes = g_get_monotonic_time() / 1000;
        AudioOpenCaptureMs(0);
        return SetStatusJsonValueAsInt("mic_duration", 0);
    }
    else
    {
        g_micDuration = atoi(cmdValue);
        g_micStartTimes = g_get_monotonic_time() / 1000;
        SetStatusJsonValueAsInt("mic_status", 1);
        SetStatusJsonValueAsInt("mic_start", 0);
        SetStatusJsonValueAsInt("mic_remaining", atoi(cmdValue));
        AudioOpenCaptureMs(atoi(cmdValue));
        return SetStatusJsonValueAsInt("mic_duration", atoi(cmdValue));
    }
}
int32_t CmdCloseMic(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    SetStatusJsonValueAsInt("mic_status", 0);
    SetStatusJsonValueAsInt("mic_start", 0);
    SetStatusJsonValueAsInt("mic_remaining", 0);
    AudioCloseCapture();
    return SetStatusJsonValueAsInt("mic_duration", 0);
}
int32_t CmdReset(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return SetStatusJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t CmdVolum(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    int32_t volum  = 0;

    volum = atoi(cmdValue);

    if(volum > 10)
    {
        volum = 10;
    }

    AudioApiSetVolum(volum * 10);

    return SetStatusJsonValueAsInt(cmdStr, atoi(cmdValue));
}
// 添加删除命令处理函数
int32_t CmdDelete(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    uint32_t fileNum = 0;
    char fileName[64] = {0};

    if (NULL == cmdValue || strlen(cmdValue) < 1)
    {
        LOG_Error("Invalid delete command value");
        return RET_ERR;
    }

    // 解析文件编号
    if (sscanf(cmdValue, "%u", &fileNum) != 1)
    {
        LOG_Error("Failed to parse file number from delete command");
        return RET_ERR;
    }

    // 构造文件路径
    snprintf(fileName, sizeof(fileName), "/oemdata/tts/%03u.wav", fileNum);

    // 检查文件是否存在
    if (access(fileName, F_OK) != 0)
    {
        LOG_Error("File does not exist: %s", fileName);
        return RET_ERR;
    }

    // 执行删除操作
    if (unlink(fileName) == 0)
    {
        LOG_Info("Successfully deleted audio file: %s", fileName);
        return RET_OK;
    }
    else
    {
        LOG_Error("Failed to delete audio file: %s, error: %s", fileName, strerror(errno));
        return RET_ERR;
    }
}
int32_t GetGetConfig(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return GetConfigJsonKeyValue(g_configBuff, 1024);
}
int32_t GetGetDeviceInfo(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return GetDeviceInfoJsonKeyValue(g_devicesBuff, 1024);
}
int32_t GetGetStatus(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    uint32_t micStatus = 0;
    uint32_t micStart = 0;
    uint32_t micRemaining = 0;
    uint32_t shoutStatus = 0;
    uint32_t shoutStart = 0;
    uint32_t shoutRemaining = 0;
    uint32_t ttsStatus = 0;
    // uint32_t tts_model=idle
    uint32_t ttsPlayed = 0;
    uint32_t ttsTotal = 0;
    uint32_t fileStatus = 0;
    uint32_t filePlayed = 0;
    uint32_t fileTotal = 0;
    time_t ttsPlayTimes = 0;
    time_t ttsTotalPcmTimes = 0;
    char ttsStatusBuff[16] = {0};

    if(RET_OK == AudioCaptureGetStatus())
    {
        micStatus = 1;
        micStart = (g_get_monotonic_time() / 1000) - g_micStartTimes;
        micRemaining = g_micDuration - micStart;
    }
    else
    {
        micStatus = 0;
        micStart = g_micDuration;
        micRemaining = 0;
    }

    if(RET_ERR == AudioGetPlayShoutStatus())
    {
        shoutStatus = 1;
        shoutStart = (g_get_monotonic_time() / 1000) - g_shoutStartTimes;
        shoutRemaining = g_shoutDuration - micStart;
    }
    else
    {
        shoutStatus = 0;
        shoutStart = g_shoutDuration;
        shoutRemaining = 0;
    }

    if(RET_OK == AudioGetPlayTtsStatus())
    {
        ttsStatus = 1;
        GetAudioTotalPcmTimes(&ttsTotalPcmTimes);
        GetAudioPcmTimes(&ttsPlayTimes);
        snprintf(ttsStatusBuff, 16, "idle");
        ttsPlayed = (ttsTotalPcmTimes - ttsPlayTimes) * 1000;
        ttsTotal = ttsTotalPcmTimes * 1000;
        if((0 != ttsPlayed || 0 != ttsTotal) && ttsTotal != ttsPlayed)
        {
            snprintf(ttsStatusBuff, 16, "announcing");
        }
    }
    else
    {
        snprintf(ttsStatusBuff, 16, "close");
        ttsStatus = 0;
        ttsPlayed = 0;
        ttsTotal = 0;
    }

    if(RET_OK == AudioGetPlayFileStatus())
    {
        fileStatus = 1;
        filePlayed = (g_get_monotonic_time() / 1000) - g_playFileStartTimes;
        fileTotal = AudioGetFileTimes();
    }
    else
    {
        fileStatus = 0;
        filePlayed = 0;
        fileTotal = 0;
    }

    snprintf(g_statusBuff, 1024, "mic_status=%d&mic_duration=%d&mic_start=%d&mic_remaining=%d&"\
                                 "shout_status=%d&shout_duration=%d&shout_start=%d&shout_remaining=%d&"\
                                 "tts_status=%d&tts_model=%s&tts_played=%d&tts_total=%d&"\
                                 "file_status=%d&file_played=%d&file_total=%d", \
                                  micStatus, g_micDuration, micStart, micRemaining, \
                                  shoutStatus, g_shoutDuration, shoutStart, shoutRemaining,\
                                  ttsStatus, ttsStatusBuff, ttsPlayed, ttsTotal,\
                                  fileStatus, filePlayed, fileTotal);

    return RET_OK;
}
int32_t GetGetTts(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    if(g_rootValue["tts"]["power_on_tts"].isString())
    {
        snprintf(g_ttsBuff, 1024, "power_on_tts=%s", g_rootValue["tts"]["power_on_tts"].asString().c_str());
    }
    else
    {
        snprintf(g_ttsBuff, 1024, "power_on_tts=%s", "[codec:utf][times1][interval100][m3][s5][v5]车辆已上电，请注意安全");
    }

    return RET_OK;
}

int32_t GetIndexCmdRetStr(int32_t index, char *retBuff, int32_t buffLen)
{
    if(NULL == retBuff || buffLen < 1024)
    {
        LOG_Error("%s cmd is NULL", __FUNCTION__);
        return RET_ERR;
    }

    switch(index)
    {
        case 1:
            memcpy(retBuff, g_configBuff, 1024);
            break;
        case 2:
            memcpy(retBuff, g_devicesBuff, 1024);
            break;
        case 3:
            memcpy(retBuff, g_statusBuff, 1024);
            break;
        case 4:
            memcpy(retBuff, g_ttsBuff, 1024);
            break;
    }
    
    return RET_OK;
}

int32_t TTSTimes(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    int32_t retValue = 0;

    retValue = snprintf(&g_ttsBuff[g_ttsBuffIdex], 1024 - g_ttsBuffIdex - 1, "%s", ttsCode);

    g_ttsBuffIdex += retValue;
    return SetTTSJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t TTSinterval(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    int32_t retValue = 0;

    retValue = snprintf(&g_ttsBuff[g_ttsBuffIdex], 1024 - g_ttsBuffIdex - 1, "%s", ttsCode);

    g_ttsBuffIdex += retValue;
    return SetTTSJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t TTSCodec(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    int32_t retValue = 0;

    retValue = snprintf(&g_ttsBuff[g_ttsBuffIdex], 1024 - g_ttsBuffIdex - 1, "%s", ttsCode);

    g_ttsBuffIdex += retValue;
    return RET_OK;
    // return SetTTSJsonValueAsStr("codec:", cmdValue);
}
int32_t TTSM(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    int32_t retValue = 0;

    retValue = snprintf(&g_ttsBuff[g_ttsBuffIdex], 1024 - g_ttsBuffIdex - 1, "%s", ttsCode);

    g_ttsBuffIdex += retValue;
    return SetTTSJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t TTSN(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    int32_t retValue = 0;

    retValue = snprintf(&g_ttsBuff[g_ttsBuffIdex], 1024 - g_ttsBuffIdex - 1, \
    "[%s%s]%s", cmdStr, cmdValue, ttsCode);

    g_ttsBuffIdex += retValue;
    return SetTTSJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t TTSP(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    int32_t retValue = 0;

    retValue = snprintf(&g_ttsBuff[g_ttsBuffIdex], 1024 - g_ttsBuffIdex - 1, \
    "[%s%s]%s", cmdStr, cmdValue, ttsCode);

    g_ttsBuffIdex += retValue;
    return SetTTSJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t TTSS(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    int32_t retValue = 0;
    int32_t cmdValueNum = 0;


    if(0 == g_cmdTimes && (g_rootValue["tts"]["times"].asInt() > 1))
    {
        g_cmdTimes = g_rootValue["tts"]["times"].asInt();
    }

    if(g_ttsBuffIdex > 0)
    {
        AudioGenerateXTTS(g_rootValue["tts"]["s"].asInt(), \
                            g_rootValue["tts"]["v"].asInt(), \
                            g_ttsBuff, \
                            g_rootValue["tts"]["codec:"].asString().c_str(), \
                            g_rootValue["tts"]["m"].asInt());
    }
    
    memset(g_ttsBuff, 0, 1024);
    g_ttsBuffIdex = 0;
    retValue = snprintf(&g_ttsBuff[g_ttsBuffIdex], 1024 - g_ttsBuffIdex - 1, "%s", ttsCode);

    g_ttsBuffIdex += retValue;
    
    cmdValueNum = atoi(cmdValue);

    return SetTTSJsonValueAsInt(cmdStr, cmdValueNum);
}
int32_t TTSV(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    int32_t retValue = 0;
    int32_t cmdValueNum = 0;

    if(0 == g_cmdTimes && (g_rootValue["tts"]["times"].asInt() > 1))
    {
        g_cmdTimes = g_rootValue["tts"]["times"].asInt();
    }

    if(g_ttsBuffIdex > 0)
    {
        AudioGenerateXTTS(g_rootValue["tts"]["s"].asInt(), \
                            g_rootValue["tts"]["v"].asInt(), \
                            g_ttsBuff, \
                            g_rootValue["tts"]["codec:"].asString().c_str(),\
                            g_rootValue["tts"]["m"].asInt());
    }

    memset(g_ttsBuff, 0, 1024);
    g_ttsBuffIdex = 0;
    retValue = snprintf(&g_ttsBuff[g_ttsBuffIdex], 1024 - g_ttsBuffIdex - 1, "%s", ttsCode);

    g_ttsBuffIdex += retValue;
    
    cmdValueNum = atoi(cmdValue);

    return SetTTSJsonValueAsInt(cmdStr, cmdValueNum);
}
int32_t TTSY(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    int32_t retValue = 0;

    retValue = snprintf(&g_ttsBuff[g_ttsBuffIdex], 1024 - g_ttsBuffIdex - 1, \
    "[%s%s]%s", cmdStr, cmdValue, ttsCode);

    g_ttsBuffIdex += retValue;
    return SetTTSJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t TTSDD(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    int32_t retValue = 0;

    retValue = snprintf(&g_ttsBuff[g_ttsBuffIdex], 1024 - g_ttsBuffIdex - 1, \
    "[%s%s]%s", cmdStr, cmdValue, ttsCode);

    g_ttsBuffIdex += retValue;
    return SetTTSJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t TTSB(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    int32_t retValue = 0;

    retValue = snprintf(&g_ttsBuff[g_ttsBuffIdex], 1024 - g_ttsBuffIdex - 1, \
    "[%s%s]%s", cmdStr, cmdValue,ttsCode);

    g_ttsBuffIdex += retValue;
    return SetTTSJsonValueAsInt(cmdStr, atoi(cmdValue));
}
int32_t TTSAW(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    char fileName[128] = {0};
    int32_t retValue = 0;
    time_t sleepTimes;
    int32_t sleepNum = 0;

    if(0 == g_cmdTimes && (g_rootValue["tts"]["times"].asInt() > 1))
    {
        g_cmdTimes = g_rootValue["tts"]["times"].asInt();
    }

    if(g_cmdTimes == g_rootValue["tts"]["times"].asInt() || 0 == g_cmdTimes)
    {
        AudioApiSetVolum(g_rootValue["tts"]["v"].asInt()*10);
    }

    if(g_ttsBuffIdex > 0)
    {
        AudioGenerateXTTS(g_rootValue["tts"]["s"].asInt(), \
                            g_rootValue["tts"]["v"].asInt(), \
                            g_ttsBuff, \
                            g_rootValue["tts"]["codec:"].asString().c_str(), \
                            g_rootValue["tts"]["m"].asInt());
    }
    
    while(1)
    {
        if(true != g_audioControlQue.empty())
        {
            // AudioInterruptTts();
            LOG_Debug(" if(true != g_audioControlQue.empty())");
            return RET_ERR;
        }

        if(RET_OK == AudioIsTTSQueEmpty())
        {
            LOG_Debug(" AudioIsTTSQueEmpty");
            break;
        }
        sleep(1);
        LOG_Debug("wait AudioIsTTSQueEmpty");
    }

    memset(g_ttsBuff, 0, 1024);
    g_ttsBuffIdex = 0;
    retValue = snprintf(&g_ttsBuff[g_ttsBuffIdex], 1024 - g_ttsBuffIdex - 1, "%s", ttsCode);

    g_ttsBuffIdex += retValue;

    if(0 != g_awCmdFlag)
    {
        while(1)
        {
            if(true != g_audioControlQue.empty())
            {
                LOG_Debug(" if(true != g_audioControlQue.empty())");
                return RET_ERR;
            }
            if(RET_OK == AudioGetPlayAWFileStatus())
            {
                LOG_Debug(" AudioGetPlayAWFileStatus");
                usleep(100000);
                break;
            }
            else
            {
                usleep(500000);
                LOG_Debug("wait AudioGetPlayAWFileStatus");
            }
        }
    }

    if (cmdValue != NULL && strlen(cmdValue) > 0) {
        char *endptr;
        long fileNum = strtol(cmdValue, &endptr, 10);
        
        if (*endptr == '\0' && fileNum >= 0 && fileNum <= 999) {
            if (AudioFindFileByNumber(fileNum, fileName, sizeof(fileName)) == NULL) {
                LOG_Error("%s No audio file found for number %s", __FUNCTION__, cmdValue);
                snprintf(fileName, 128, "/oemdata/tts/%s.wav", cmdValue);
            }else{
                 LOG_Debug("Find audio fileName:%s",fileName);
            }
        } else {
             LOG_Error("%s cmdValue:%s is invalid", __FUNCTION__,cmdValue);
             snprintf(fileName, 128, "/oemdata/tts/%s.wav", cmdValue);
        }
    } else {
        LOG_Error("%s cmdValue is invalid", __FUNCTION__);
        snprintf(fileName, 128, "/oemdata/tts/%s.wav", cmdValue);
    }

    SetTTSJsonValueAsStr(cmdStr, cmdValue);
    memset(g_awFileBuff, 0, 128);
    memcpy(g_awFileBuff, fileName, 128);
    g_awCmdFlag = 1;

    LOG_Debug("%s g_cmdTimes:%d, times :%d", __FUNCTION__, g_cmdTimes, g_rootValue["tts"]["times"].asInt());
    g_playFileStartTimes = (g_get_monotonic_time() / 1000);
    return AudioPlayAwFile(fileName);
    // return SetTTSJsonValueAsStr(cmdStr, cmdValue);
}

int32_t TTSTts(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    int32_t retValue = 0;

    retValue = snprintf(&g_ttsBuff[g_ttsBuffIdex], 1024 - g_ttsBuffIdex - 1, "%s", ttsCode);

    g_ttsBuffIdex += retValue;
    return RET_OK;
}

int32_t SrCmdPlayCapyureFile(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return AudioPlayAwFile(AUDIO_CAPTURE_FILE_PATH_NAME);
}

int32_t SrCmdStartCapyureFile(const char *cmdStr, const char *cmdValue, const char *ttsCode)
{
    return AudioOpenFileCaptureMs(atoi(cmdValue));
}

int32_t InitConfigJsonValue(Json::Value &rootValue)
{

    rootValue["config"]["device_addr"] = "192.168.1.8/24";
    rootValue["config"]["control_port"] = 7889;
    rootValue["config"]["shout_port"] = 7890;
    rootValue["config"]["shout_audio_codec"] = "pcm";
    rootValue["config"]["shout_audio_sample"] = 3;
    rootValue["config"]["shout_audio_bit"] = 16;
    rootValue["config"]["shout_frame_size"] = 1920;
    rootValue["config"]["shout_channel_mode"] = 2;
    rootValue["config"]["shout_vqe"] = 0;
    rootValue["config"]["shout_volum"] = 10;
    rootValue["config"]["mic_send_ip"] = "192.168.1.2";
    rootValue["config"]["mic_send_port"] = 7992;
    rootValue["config"]["mic_audio_codec"] = "pcm";
    rootValue["config"]["mic_audio_sample"] = 3;
    rootValue["config"]["mic_audio_bit"] = 16;
    rootValue["config"]["mic_frame_size"] = 960;
    rootValue["config"]["mic_channel_mode"] = 2;
    rootValue["config"]["mic_vqe"] = 1;
    rootValue["config"]["upload_port"] = 7991;
    rootValue["config"]["power_on_tts"] = "#[TTS][codec:utf][times1][interval100][m3][s5][v5]车辆已上电，请注意安全";
}

int32_t InitTTSJsonValue(Json::Value &rootValue)
{
    rootValue["tts"]["times"] = 1;
    rootValue["tts"]["interval"] = 100;
    rootValue["tts"]["codec:"] = "gbk";
    rootValue["tts"]["m"] = 3;
    rootValue["tts"]["n"] = 0;
    rootValue["tts"]["p"] = 0;
    rootValue["tts"]["s"] = 50;
    rootValue["tts"]["v"] = 50;
    rootValue["tts"]["y"] = 0;
    rootValue["tts"]["="] = 0;
    rootValue["tts"]["b"] = 0;
    rootValue["tts"]["AW"] = "001";
}

int32_t InitStatusJsonValue(Json::Value &rootValue)
{
    rootValue["status"]["mic_status"] = 1;
    rootValue["status"]["mic_duration"] = 0;
    rootValue["status"]["mic_start"] = 0;
    rootValue["status"]["mic_remaining"] = 0;
    rootValue["status"]["shout_status"] = 1;
    rootValue["status"]["shout_duration"] = 0;
    rootValue["status"]["shout_start"] = 0;
    rootValue["status"]["shout_remaining"] = 0;
    rootValue["status"]["tts_status"] = 0;
    rootValue["status"]["tts_model"] = "idle";
    rootValue["status"]["tts_played"] = 0;
    rootValue["status"]["tts_total"] = 0;
    rootValue["status"]["file_status"] = 0;
    rootValue["status"]["file_played"] = 0;
    rootValue["status"]["file_total"] = 0;
}

int32_t InitDeviceInfoJsonValue(Json::Value &rootValue)
{
    rootValue["info"]["device_id"] = "SR-TBOX-ES8389";
    rootValue["info"]["software_version"] = "1.0.0";
    rootValue["info"]["hardware_version"] = "1.0.0";
}


int32_t InitJsonFileHandler()
{       
    if(NULL != g_jsonFileHandler)
    {
        return RET_OK;
    }
    g_jsonFileHandler = new JsonFileHandler(JSON_FEIL_PATH_NAME);
    g_rootValue = g_jsonFileHandler->get_data();
    if(g_rootValue["config"].size() == 0)
    {
        InitConfigJsonValue(g_rootValue);
        g_jsonFileHandler->update_value(g_rootValue);
        LOG_Debug("%s InitConfigJsonValue", __FUNCTION__);
    }
    if(g_rootValue["status"].size() == 0)
    {
        InitStatusJsonValue(g_rootValue);
        g_jsonFileHandler->update_value(g_rootValue);
        LOG_Debug("%s InitStatusJsonValue", __FUNCTION__);
    }
    if(g_rootValue["tts"].size() == 0)
    {
        InitTTSJsonValue(g_rootValue);
        g_jsonFileHandler->update_value(g_rootValue);
        LOG_Debug("%s InitTTSJsonValue", __FUNCTION__);
    }

    if(g_rootValue["info"].size() == 0)
    {
        InitDeviceInfoJsonValue(g_rootValue);
        g_jsonFileHandler->update_value(g_rootValue);
        LOG_Debug("%s InitTTSJsonValue", __FUNCTION__);
    }

    if(g_rootValue.isObject() && g_rootValue.size() == 0)
    {
        InitConfigJsonValue(g_rootValue);
        InitStatusJsonValue(g_rootValue);
        InitTTSJsonValue(g_rootValue);
        InitDeviceInfoJsonValue(g_rootValue);
        g_jsonFileHandler->update_value(g_rootValue);
        LOG_Debug("%s init all", __FUNCTION__);
    }
    
    g_rootValue["info"]["device_id"] = "SR-TBOX-ES8389";

    return RET_OK;
}

int32_t SetConfigJsonValueAsStr(const char *jsonKey, const char *jsonValue)
{
    if(NULL == jsonKey || NULL == jsonValue)
    {
        LOG_Error("%s argv is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == g_jsonFileHandler)
    {
        LOG_Error("%s g_jsonFileHandler not init ", __FUNCTION__);
        return RET_ERR;
    }

    if(g_rootValue["config"].isObject())
    {
        g_rootValue["config"][jsonKey] = jsonValue;
        g_jsonFileHandler->update_value(g_rootValue);
        return RET_OK;
    }

    LOG_Error("%s g_rootValue not isObject", __FUNCTION__);
    return RET_ERR;
}

int32_t SetConfigJsonValueAsInt(const char *jsonKey, int64_t jsonValue)
{
    if(NULL == jsonKey)
    {
        LOG_Error("%s argv is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == g_jsonFileHandler)
    {
        LOG_Error("%s g_jsonFileHandler not init ", __FUNCTION__);
        return RET_ERR;
    }

    if(g_rootValue["config"].isObject())
    {
        g_rootValue["config"][jsonKey] = jsonValue;
        g_jsonFileHandler->update_value(g_rootValue);
        return RET_OK;
    }

    LOG_Error("%s g_rootValue not isObject", __FUNCTION__);
    return RET_ERR;
}
    
int32_t SetTTSJsonValueAsStr(const char *jsonKey, const char *jsonValue)
{
    if(NULL == jsonKey || NULL == jsonValue)
    {
        LOG_Error("%s argv is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == g_jsonFileHandler)
    {
        LOG_Error("%s g_jsonFileHandler not init ", __FUNCTION__);
        return RET_ERR;
    }

    if(g_rootValue["tts"].isObject())
    {
        g_rootValue["tts"][jsonKey] = jsonValue;
        g_jsonFileHandler->update_value(g_rootValue);
        return RET_OK;
    }

    LOG_Error("%s g_rootValue not isObject", __FUNCTION__);
    return RET_ERR;
}

int32_t SetTTSJsonValueAsInt(const char *jsonKey, int64_t jsonValue)
{
    if(NULL == jsonKey)
    {
        LOG_Error("%s argv is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == g_jsonFileHandler)
    {
        LOG_Error("%s g_jsonFileHandler not init ", __FUNCTION__);
        return RET_ERR;
    }

    if(g_rootValue["tts"].isObject())
    {
        g_rootValue["tts"][jsonKey] = jsonValue;
        g_jsonFileHandler->update_value(g_rootValue);
        return RET_OK;
    }

    LOG_Error("%s g_rootValue not isObject", __FUNCTION__);
    return RET_ERR;
}

int32_t SetStatusJsonValueAsStr(const char *jsonKey, const char *jsonValue)
{
    if(NULL == jsonKey || NULL == jsonValue)
    {
        LOG_Error("%s argv is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == g_jsonFileHandler)
    {
        LOG_Error("%s g_jsonFileHandler not init ", __FUNCTION__);
        return RET_ERR;
    }

    if(g_rootValue["status"].isObject())
    {
        g_rootValue["status"][jsonKey] = jsonValue;
        g_jsonFileHandler->update_value(g_rootValue);
        return RET_OK;
    }

    LOG_Error("%s g_rootValue not isObject", __FUNCTION__);
    return RET_ERR;
}

int32_t SetStatusJsonValueAsInt(const char *jsonKey, int64_t jsonValue)
{
    if(NULL == jsonKey)
    {
        LOG_Error("%s argv is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == g_jsonFileHandler)
    {
        LOG_Error("%s g_jsonFileHandler not init ", __FUNCTION__);
        return RET_ERR;
    }

    if(g_rootValue["status"].isObject())
    {
        g_rootValue["status"][jsonKey] = jsonValue;
        g_jsonFileHandler->update_value(g_rootValue);
        return RET_OK;
    }

    LOG_Error("%s g_rootValue not isObject", __FUNCTION__);
    return RET_ERR;
}

int32_t GetStatusJsonValueAsStr(const char *jsonKey, char *jsonValue, int32_t valueLen)
{
    if(NULL == jsonKey || NULL == jsonValue)
    {
        LOG_Error("%s argv is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == g_jsonFileHandler)
    {
        LOG_Error("%s g_jsonFileHandler not init ", __FUNCTION__);
        return RET_ERR;
    }

    if(g_rootValue["status"][jsonKey].isString())
    {
        if(valueLen <= g_rootValue["status"][jsonKey].asString().length())
        {
            LOG_Error("%s valueLen < str.length() ", __FUNCTION__);
            return RET_ERR;
        }
        memcpy(jsonValue, g_rootValue["status"][jsonKey].asString().c_str(), g_rootValue["status"][jsonKey].asString().length());
        return RET_OK;
    }

    LOG_Error("%s g_rootValue not isObject", __FUNCTION__);
    return RET_ERR;
}

int32_t GetStatusJsonValueAsInt(const char *jsonKey, int64_t *jsonValue)
{
    if(NULL == jsonKey || NULL == jsonValue)
    {
        LOG_Error("%s argv is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == g_jsonFileHandler)
    {
        LOG_Error("%s g_jsonFileHandler not init ", __FUNCTION__);
        return RET_ERR;
    }

    if(g_rootValue["status"][jsonKey].isInt())
    {
        *jsonValue = g_rootValue["status"][jsonKey].asInt();
        return RET_OK;
    }

    LOG_Error("%s g_rootValue not isObject", __FUNCTION__);
    return RET_ERR;
}

int32_t GetTTSJsonValueAsStr(const char *jsonKey, char *jsonValue, int32_t valueLen)
{
    if(NULL == jsonKey || NULL == jsonValue)
    {
        LOG_Error("%s argv is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == g_jsonFileHandler)
    {
        LOG_Error("%s g_jsonFileHandler not init ", __FUNCTION__);
        return RET_ERR;
    }

    if(g_rootValue["tts"][jsonKey].isString())
    {
        if(valueLen <= g_rootValue["tts"][jsonKey].asString().length())
        {
            LOG_Error("%s valueLen < str.length() ", __FUNCTION__);
            return RET_ERR;
        }
        memcpy(jsonValue, g_rootValue["tts"][jsonKey].asString().c_str(), g_rootValue["tts"][jsonKey].asString().length());
        return RET_OK;
    }

    LOG_Error("%s g_rootValue not isObject", __FUNCTION__);
    return RET_ERR;
}

int32_t GetTTSJsonValueAsInt(const char *jsonKey, int64_t *jsonValue)
{
    if(NULL == jsonKey || NULL == jsonValue)
    {
        LOG_Error("%s argv is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == g_jsonFileHandler)
    {
        LOG_Error("%s g_jsonFileHandler not init ", __FUNCTION__);
        return RET_ERR;
    }

    if(g_rootValue["tts"][jsonKey].isInt())
    {
        *jsonValue = g_rootValue["tts"][jsonKey].asInt();
        return RET_OK;
    }

    LOG_Error("%s g_rootValue not isObject", __FUNCTION__);
    return RET_ERR;
}

int32_t GetConfigJsonKeyValue(char *configBuff, int32_t buffLen)
{
    int32_t index = 0;
    int32_t writeLen = 0;
    int32_t retValue = 0;
    
    if(NULL == configBuff)
    {
        LOG_Error("%s argv is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if (g_rootValue["config"].isObject()) 
    {
        Json::Value::const_iterator it = g_rootValue["config"].begin();
        for (; it != g_rootValue["config"].end(); ++it) 
        {
            std::string key = it.memberName();     // 获取 key
            Json::Value value = *it;               // 获取 value

            if (value.isString()) 
            {
                std::string strVal = value.asString();
                retValue = snprintf(&configBuff[index], buffLen - writeLen, "%s=%s&", key.c_str(), strVal.c_str());
                writeLen += retValue;
                index = writeLen ;
            }
            else if (value.isInt()) 
            {
                int intVal = value.asInt();
                retValue = snprintf(&configBuff[index], buffLen - writeLen, "%s=%d&", key.c_str(), intVal);
                writeLen += retValue;
                index = writeLen;
            }
            else if (value.isDouble()) 
            {
                LOG_Debug("%s is  Double", __FUNCTION__);
            }
            else if (value.isBool()) 
            {
                LOG_Debug("%s is Bool all", __FUNCTION__);
            }
            else if (value.isNull()) 
            {
                LOG_Debug("%s is Null", __FUNCTION__);
            }
            else if (value.isArray()) 
            {
                LOG_Debug("%s init all", __FUNCTION__);
            }
            else if (value.isObject()) 
            {
                LOG_Debug("%s is Nested Object", __FUNCTION__);
            }
            else 
            {
                LOG_Debug("%s Unknown type", __FUNCTION__);
            }
        }
        configBuff[index - 1] = '\0';
    }
    return RET_OK;
}
int32_t GetTTSJsonKeyValue(char *ttsBuff, int32_t buffLen)
{
    int32_t index = 0;
    int32_t writeLen = 0;
    int32_t retValue = 0;

    if(NULL == ttsBuff)
    {
        LOG_Error("%s argv is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if (g_rootValue["tts"].isObject()) 
    {
        Json::Value::const_iterator it = g_rootValue["tts"].begin();
        for (; it != g_rootValue["tts"].end(); ++it) 
        {
            std::string key = it.memberName();     // 获取 key
            Json::Value value = *it;               // 获取 value

            if (value.isString()) 
            {
                std::string strVal = value.asString();
                retValue = snprintf(&ttsBuff[index], buffLen - writeLen, "[%s%s]", key.c_str(), strVal.c_str());
                writeLen += retValue;
                index = writeLen;
            }
            else if (value.isInt()) 
            {
                int intVal = value.asInt();
                retValue = snprintf(&ttsBuff[index], buffLen - writeLen, "[%s%d]", key.c_str(), intVal);
                writeLen += retValue;
                index = writeLen;
            }
            else if (value.isDouble()) 
            {
                LOG_Debug("%s is  Double", __FUNCTION__);
            }
            else if (value.isBool()) 
            {
                LOG_Debug("%s is Bool all", __FUNCTION__);
            }
            else if (value.isNull()) 
            {
                LOG_Debug("%s is Null", __FUNCTION__);
            }
            else if (value.isArray()) 
            {
                LOG_Debug("%s init all", __FUNCTION__);
            }
            else if (value.isObject()) 
            {
                LOG_Debug("%s is Nested Object", __FUNCTION__);
            }
            else 
            {
                LOG_Debug("%s Unknown type", __FUNCTION__);
            }
        }

    }
    return RET_OK;
}

int32_t GetDeviceInfoJsonKeyValue(char *deviceInfo, int32_t buffLen)
{
    int32_t index = 0;
    int32_t writeLen = 0;
    int32_t retValue = 0;

    if(NULL == deviceInfo)
    {
        LOG_Error("%s argv is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if (g_rootValue["info"].isObject()) 
    {
        Json::Value::const_iterator it = g_rootValue["info"].begin();
        for (; it != g_rootValue["info"].end(); ++it) 
        {
            std::string key = it.memberName();     // 获取 key
            Json::Value value = *it;               // 获取 value

            if (value.isString()) 
            {
                std::string strVal = value.asString();
                retValue = snprintf(&deviceInfo[index], buffLen - writeLen, "%s=%s&", key.c_str(), strVal.c_str());
                writeLen += retValue;
                index = writeLen;
            }
            else if (value.isInt()) 
            {
                int intVal = value.asInt();
                retValue = snprintf(&deviceInfo[index], buffLen - writeLen, "%s=%d&", key.c_str(), intVal);
                writeLen += retValue;
                index = writeLen;
            }
            else if (value.isDouble()) 
            {
                LOG_Debug("%s is  Double", __FUNCTION__);
            }
            else if (value.isBool()) 
            {
                LOG_Debug("%s is Bool all", __FUNCTION__);
            }
            else if (value.isNull()) 
            {
                LOG_Debug("%s is Null", __FUNCTION__);
            }
            else if (value.isArray()) 
            {
                LOG_Debug("%s init all", __FUNCTION__);
            }
            else if (value.isObject()) 
            {
                LOG_Debug("%s is Nested Object", __FUNCTION__);
            }
            else 
            {
                LOG_Debug("%s Unknown type", __FUNCTION__);
            }
        }
        deviceInfo[index - 1] = '\0';
    }
    return RET_OK;
}

int32_t GetStatusJsonKeyValue(char *statusBuff, int32_t buffLen)
{
    int32_t index = 0;
    int32_t writeLen = 0;
    int32_t retValue = 0;

    if(NULL == statusBuff)
    {
        LOG_Error("%s argv is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if (g_rootValue["status"].isObject()) 
    {
        Json::Value::const_iterator it = g_rootValue["status"].begin();
        for (; it != g_rootValue["status"].end(); ++it) 
        {
            std::string key = it.memberName();     // 获取 key
            Json::Value value = *it;               // 获取 value

            if (value.isString()) 
            {
                std::string strVal = value.asString();
                retValue = snprintf(&statusBuff[index], buffLen - writeLen, "%s=%s&", key.c_str(), strVal.c_str());
                writeLen += retValue;
                index = writeLen;
            }
            else if (value.isInt()) 
            {
                int intVal = value.asInt();
                retValue = snprintf(&statusBuff[index], buffLen - writeLen, "%s=%d&", key.c_str(), intVal);
                writeLen += retValue;
                index = writeLen;
            }
            else if (value.isDouble()) 
            {
                LOG_Debug("%s is  Double", __FUNCTION__);
            }
            else if (value.isBool()) 
            {
                LOG_Debug("%s is Bool all", __FUNCTION__);
            }
            else if (value.isNull()) 
            {
                LOG_Debug("%s is Null", __FUNCTION__);
            }
            else if (value.isArray()) 
            {
                LOG_Debug("%s init all", __FUNCTION__);
            }
            else if (value.isObject()) 
            {
                LOG_Debug("%s is Nested Object", __FUNCTION__);
            }
            else 
            {
                LOG_Debug("%s Unknown type", __FUNCTION__);
            }
        }
        statusBuff[index - 1] = '\0';
    }
    return RET_OK;
}


int32_t CutConfigCmdBuffToArray(const char *cmdBuff, int32_t recvLen, AudioCmdInfo_t *cmdArray, int32_t arraySize)
{
    char *cmdStr = NULL;
    char *cmdStr1 = NULL;
    const char delim[] = "&\r\n";
    const char delim2[] = "=\r\n";
    char *str1 = NULL;
    char *str2 = NULL;
    char *saveptr1 = NULL;
    char *saveptr2 = NULL;
    int32_t countStr1 = 0;
    int32_t countStr2 = 0;
    char tmpBuff[1024] = {0};


    if(NULL == cmdBuff)
    {
        LOG_Error("%s cmdBuff is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == cmdArray)
    {
        LOG_Error("%s cmdArray is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(recvLen < 2 || recvLen > 1024)
    {
        LOG_Error("%s recvLen < 2 || recvLen > 1024", __FUNCTION__);
        return RET_ERR;
    }

    memcpy(tmpBuff, cmdBuff, recvLen);

    for (str1 = tmpBuff; ; countStr1++, str1 = NULL) 
    {
        cmdStr = strtok_r(str1, delim, &saveptr1);
        if(NULL == cmdStr)
        {
            break;
        }
        if(countStr1 >= arraySize)
        {
            break;
        }

        for (str2 = cmdStr, countStr2 = 0; ; countStr2++, str2 = NULL) 
        {
            cmdStr1 = strtok_r(str2, delim2, &saveptr2);
            if(NULL == cmdStr1)
            {
                break;
            }
            if(0 == countStr2 && NULL != cmdStr1)
            {
                snprintf(cmdArray[countStr1].cmdName, 32, "#[CONFIG]%s", cmdStr1);
                snprintf(cmdArray[countStr1].cmdKey, 32, "%s", cmdStr1);
            }
            if(1 == countStr2 && NULL != cmdStr1)
            {
                snprintf(cmdArray[countStr1].cmdValue, 1024, "%s", cmdStr1);
            }
            LOG_Debug("countStr1 :%d cmdStr1 :%s", countStr1, cmdStr1);
        }
    }

    return countStr1;
}

int32_t CutCmdCmdBuffToArray(const char *cmdBuff, int32_t recvLen, AudioCmdInfo_t *cmdArray, int32_t arraySize)
{
    char *cmdStr = NULL;
    char *cmdStr1 = NULL;
    const char delim[] = "&\r\n";
    const char delim2[] = "=\r\n";
    char *str1 = NULL;
    char *str2 = NULL;
    char *saveptr1 = NULL;
    char *saveptr2 = NULL;
    int32_t countStr1 = 0;
    int32_t countStr2 = 0;
    char tmpBuff[1024] = {0};


    if(NULL == cmdBuff)
    {
        LOG_Error("%s cmdBuff is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == cmdArray)
    {
        LOG_Error("%s cmdArray is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(recvLen < 2 || recvLen > 1024)
    {
        LOG_Error("%s recvLen < 2 || recvLen > 1024", __FUNCTION__);
        return RET_ERR;
    }

    memcpy(tmpBuff, cmdBuff, recvLen);

    for (str1 = tmpBuff; ; countStr1++, str1 = NULL) 
    {
        cmdStr = strtok_r(str1, delim, &saveptr1);
        if(NULL == cmdStr)
        {
            break;
        }
        if(countStr1 >= arraySize)
        {
            break;
        }

        for (str2 = cmdStr, countStr2 = 0; ; countStr2++, str2 = NULL) 
        {
            cmdStr1 = strtok_r(str2, delim2, &saveptr2);
            if(NULL == cmdStr1)
            {
                break;
            }
            if(0 == countStr2 && NULL != cmdStr1)
            {
                snprintf(cmdArray[countStr1].cmdName, 32, "#[CMD]%s", cmdStr1);
                snprintf(cmdArray[countStr1].cmdKey, 32, "%s", cmdStr1);
            }
            if(1 == countStr2 && NULL != cmdStr1)
            {
                snprintf(cmdArray[countStr1].cmdValue, 1024, "%s", cmdStr1);
            }
            LOG_Debug("countStr1 :%d cmdStr1 :%s", countStr1, cmdStr1);
        }
    }

    return countStr1;
}

int32_t CutSrCmdCmdBuffToArray(const char *cmdBuff, int32_t recvLen, AudioCmdInfo_t *cmdArray, int32_t arraySize)
{
    char *cmdStr = NULL;
    char *cmdStr1 = NULL;
    const char delim[] = "&\r\n";
    const char delim2[] = "=\r\n";
    char *str1 = NULL;
    char *str2 = NULL;
    char *saveptr1 = NULL;
    char *saveptr2 = NULL;
    int32_t countStr1 = 0;
    int32_t countStr2 = 0;
    char tmpBuff[1024] = {0};


    if(NULL == cmdBuff)
    {
        LOG_Error("%s cmdBuff is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == cmdArray)
    {
        LOG_Error("%s cmdArray is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(recvLen < 2 || recvLen > 1024)
    {
        LOG_Error("%s recvLen < 2 || recvLen > 1024", __FUNCTION__);
        return RET_ERR;
    }

    memcpy(tmpBuff, cmdBuff, recvLen);

    for (str1 = tmpBuff; ; countStr1++, str1 = NULL) 
    {
        cmdStr = strtok_r(str1, delim, &saveptr1);
        if(NULL == cmdStr)
        {
            break;
        }
        if(countStr1 >= arraySize)
        {
            break;
        }

        for (str2 = cmdStr, countStr2 = 0; ; countStr2++, str2 = NULL) 
        {
            cmdStr1 = strtok_r(str2, delim2, &saveptr2);
            if(NULL == cmdStr1)
            {
                break;
            }
            if(0 == countStr2 && NULL != cmdStr1)
            {
                snprintf(cmdArray[countStr1].cmdName, 32, "#[SRCMD]%s", cmdStr1);
                snprintf(cmdArray[countStr1].cmdKey, 32, "%s", cmdStr1);
            }
            if(1 == countStr2 && NULL != cmdStr1)
            {
                snprintf(cmdArray[countStr1].cmdValue, 1024, "%s", cmdStr1);
            }
            LOG_Debug("countStr1 :%d cmdStr1 :%s", countStr1, cmdStr1);
        }
    }

    return countStr1;
}


int32_t CutGetCmdBuffToArray(const char *cmdBuff, int32_t recvLen, AudioCmdInfo_t *cmdArray, int32_t arraySize)
{
    char *cmdStr = NULL;
    char *cmdStr1 = NULL;
    const char delim[] = "[]\r\n";
    const char delim2[] = "=\r\n";
    char *str1 = NULL;
    char *str2 = NULL;
    char *saveptr1 = NULL;
    char *saveptr2 = NULL;
    int32_t countStr1 = 0;
    int32_t countStr2 = 0;
    char tmpBuff[1024] = {0};

    if(NULL == cmdBuff)
    {
        LOG_Error("%s cmdBuff is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == cmdArray)
    {
        LOG_Error("%s cmdArray is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(recvLen < 2 || recvLen > 1024)
    {
        LOG_Error("%s recvLen < 2 || recvLen > 1024", __FUNCTION__);
        return RET_ERR;
    }

    LOG_Debug("cmdBuff :%s", cmdBuff);
    memcpy(tmpBuff, cmdBuff, recvLen);
    cmdStr = strtok_r(tmpBuff, delim, &saveptr1);
    if(NULL == cmdStr)
    {
        LOG_Error("%s strtok_r ret NULL ", __FUNCTION__);
        return RET_ERR;
    }

    snprintf(cmdArray[countStr1++].cmdName, 32, "#[GET]%s", cmdStr);
    LOG_Debug("cmdStr:%s", cmdStr);
    return countStr1;
}

int32_t CutTtsCmdBuffToArray(const char *cmdBuff, int32_t recvLen, AudioCmdInfo_t *cmdArray, int32_t arraySize)
{
    char *cmdStr = NULL;
    char *cmdStr1 = NULL;
    char *cmdStr2 = NULL;
    const char delim[] = "[\r\n";
    const char delim2[] = ":1234567890\r\n";
    const char delim3[] = "timesnrvalcod:pybAW=]\r\n";
    char *str1 = NULL;
    char *str2 = NULL;
    char *saveptr1 = NULL;
    char *saveptr2 = NULL;
    char *saveptr3 = NULL;
    char *saveptr4 = NULL;
    char *saveptr5 = NULL;
    int32_t countStr1 = 0;
    int32_t countStr2 = 0;
    char tmpBuff[1024] = {0};
    char tmpBuff2[1024] = {0};
    char tmpBuff3[1024] = {0};
    uint8_t flag = 0;


    if(NULL == cmdBuff)
    {
        LOG_Error("%s cmdBuff is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == cmdArray)
    {
        LOG_Error("%s cmdArray is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(recvLen < 1 || recvLen > 1024)
    {
        LOG_Error("%s recvLen < 2 || recvLen > 1024", __FUNCTION__);
        return RET_ERR;
    }

    memcpy(tmpBuff, cmdBuff, recvLen);
    LOG_Debug("cmdBuff :%s", cmdBuff);
    if(cmdBuff[0] != '[')
    {
        flag = 2;
    }

    for (str1 = tmpBuff; ; countStr1++, str1 = NULL) 
    {
        cmdStr = strtok_r(str1, "[", &saveptr1);
        if(NULL == cmdStr)
        {
            break;
        }
        if(countStr1 >= arraySize)
        {
            break;
        }

        if(2 == flag && 0 == countStr1)
        {
            snprintf(cmdArray[countStr1].cmdName, 32, "#[TTS]%s", "tts");
            snprintf(cmdArray[countStr1].cmdKey, 32, "%s", "tts");
            snprintf(cmdArray[countStr1].cmdValue, 1024, "%s", "");
            snprintf(cmdArray[countStr1].ttsCode, 1024, "%s", cmdStr);
            continue;
        }

        cmdStr2 = cmdStr;
        memset(tmpBuff2, 0, 1024);
        memcpy(tmpBuff2, cmdStr, strlen(cmdStr));
        memset(tmpBuff3, 0, 1024);
        memcpy(tmpBuff3, cmdStr, strlen(cmdStr));

        if(tmpBuff2[0] == '=')
        {
            snprintf(cmdArray[countStr1].cmdName, 32, "#[TTS]%s", "=");
            snprintf(cmdArray[countStr1].cmdKey, 32, "%s", "=");
            for (str2 = cmdStr, countStr2 = 0; ; countStr2++, str2 = NULL) 
            {
                cmdStr1 = strtok_r(str2, "=]\r\n", &saveptr2);
                if(NULL == cmdStr1)
                {
                    break;
                }
                if(0 == countStr2)
                {
                    snprintf(cmdArray[countStr1].cmdValue, 1024, "%s", cmdStr1);
                }
                if(1 == countStr2)
                {
                    snprintf(cmdArray[countStr1].ttsCode, 1024, "%s", cmdStr1);
                }
            }
            continue;
        }

        for (str2 = cmdStr, countStr2 = 0; ; countStr2++, str2 = NULL) 
        {
            cmdStr1 = strtok_r(str2, delim2, &saveptr3);
            if(NULL == cmdStr1)
            {
                break;
            }
            snprintf(cmdArray[countStr1].cmdName, 32, "#[TTS]%s", cmdStr1);
            snprintf(cmdArray[countStr1].cmdKey, 32, "%s", cmdStr1);
            break;
        }
        
        flag = 0;
        for (str2 = tmpBuff2, countStr2 = 0; ; countStr2++, str2 = NULL) 
        {
            
            if(NULL != strstr(tmpBuff2, ":") || 1 == flag)
            {
                flag = 1;
                cmdStr1 = strtok_r(str2, ":]\r\n", &saveptr4);
            }
            else
            {
                flag = 3;
                cmdStr1 = strtok_r(str2, delim3, &saveptr4);
            }
            
            if(NULL == cmdStr1)
            {
                break;
            }
            if(1 == flag)
            {
                if(1 == countStr2)
                {
                    snprintf(cmdArray[countStr1].cmdValue, 1024, "%s", cmdStr1);
                }
                if(2 == countStr2)
                {
                    snprintf(cmdArray[countStr1].ttsCode, 1024, "%s", cmdStr1);
                }
            }
            else
            {
                if(0 == countStr2)
                {
                    snprintf(cmdArray[countStr1].cmdValue, 1024, "%s", cmdStr1);
                }
                if(1 == countStr2)
                {
                    snprintf(cmdArray[countStr1].ttsCode, 1024, "%s", cmdStr1);
                }
            }
        }
        if(3 == flag)
        {
            memset(cmdArray[countStr1].ttsCode, 0 , 1024);
            for (str2 = tmpBuff3, countStr2 = 0; ; countStr2++, str2 = NULL) 
            {
                cmdStr1 = strtok_r(str2, "]\r\n", &saveptr5); 
                if(NULL == cmdStr1)
                {
                    break;
                }
                if(1 == countStr2)
                {
                    snprintf(cmdArray[countStr1].ttsCode, 1024, "%s", cmdStr1);
                }
                // LOG_Debug("tmpBuff3[%d] :%s", countStr1, cmdStr1);
            }
        }
    }

    return countStr1;
}

int32_t CutCmdBuffToArray(const char *cmdBuff, int32_t recvLen, AudioCmdInfo_t *cmdArray, int32_t arraySize)
{
    char *cmdStr = NULL;
    const char delim[] = "[]";
    int32_t cmpLen = 0;
    char tmpBuff[1024] = {0};
    char *saveptr1 = NULL;

    if(NULL == cmdBuff)
    {
        LOG_Error("%s cmdBuff is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(NULL == cmdArray)
    {
        LOG_Error("%s cmdArray is NULL ", __FUNCTION__);
        return RET_ERR;
    }

    if(recvLen < 2 || recvLen > 1024)
    {
        LOG_Error("%s recvLen < 2 || recvLen > 1024", __FUNCTION__);
        return RET_ERR;
    }

    LOG_Debug("cmdStr:%s", cmdStr);

    if(cmdBuff[0] != '#')
    {
        LOG_Error("%s cmdBuff[0] != '#' ", __FUNCTION__);
        return RET_ERR;
    }
    LOG_Debug("cmdStr:%s", cmdBuff);
    memcpy(tmpBuff, cmdBuff, recvLen);
    LOG_Debug("tmpBuff:%s", tmpBuff);
    cmdStr = strtok_r(&tmpBuff[1], delim, &saveptr1);
    if(NULL == cmdStr)
    {
        LOG_Error("%s strtok_r return  NULL ", __FUNCTION__);
        return RET_ERR;
    }

    LOG_Debug("cmdStr:%s", cmdStr);
    
    cmpLen = strlen("CONFIG") > strlen(cmdStr) ? strlen(cmdStr) : strlen("CONFIG");
    if(0 == strncmp(cmdStr, "CONFIG", cmpLen))
    {
        return CutConfigCmdBuffToArray(&cmdBuff[cmpLen + 3], (recvLen - cmpLen - 3), cmdArray, arraySize);
    }
    
    cmpLen = strlen("GET") > strlen(cmdStr) ? strlen(cmdStr) : strlen("GET");
    if(0 == strncmp(cmdStr, "GET", cmpLen))
    {
        return CutGetCmdBuffToArray(&cmdBuff[cmpLen + 3], (recvLen - cmpLen - 3), cmdArray, arraySize);
    }
    
    cmpLen = strlen("CMD") > strlen(cmdStr) ? strlen(cmdStr) : strlen("CMD");
    if(0 == strncmp(cmdStr, "CMD", cmpLen))
    {
        return CutCmdCmdBuffToArray(&cmdBuff[cmpLen + 3], (recvLen - cmpLen - 3), cmdArray, arraySize);
    }
    
    cmpLen = strlen("TTS") > strlen(cmdStr) ? strlen(cmdStr) : strlen("TTS");
    if(0 == strncmp(cmdStr, "TTS", cmpLen))
    {
        return CutTtsCmdBuffToArray(&cmdBuff[cmpLen + 3], (recvLen - cmpLen - 3), cmdArray, arraySize);
    }

    cmpLen = strlen("SRCMD") > strlen(cmdStr) ? strlen(cmdStr) : strlen("SRCMD");
    if(0 == strncmp(cmdStr, "SRCMD", cmpLen))
    {
        return CutSrCmdCmdBuffToArray(&cmdBuff[cmpLen + 3], (recvLen - cmpLen - 3), cmdArray, arraySize);
    }

    LOG_Error("%s cmd  is error", __FUNCTION__);

    return RET_ERR;
}



int32_t AudioSendVehicleStartTTS()
{
    return AudioUdpSend(UDP_AUDIO_BAND_ADDR, UDP_CMD_RET_PORT, \
                        g_rootValue["config"]["power_on_tts"].asString().c_str(),\
                        g_rootValue["config"]["power_on_tts"].asString().length());
}


int32_t AudioSendTTSCmdNotDoing()
{
    return AudioUdpSend(UDP_AUDIO_BAND_ADDR, UDP_CMD_RET_PORT, \
                        "#[TTS][s1]", strlen("#[TTS][s1]"));
}

void *AudioControlHandleThread(void *argv)
{
    AudioCmdHandle_t audioCmdHandle;
    AudioCmdInfo_t cmdArray[32];
    int32_t socketFd = -1;
    char recvCmdBuff[1024] = {0};
    char cmdBuff[1024] = {0};
    char retBuff[1024] = {0};
    int32_t recvLen = 0;
    int32_t cmdNum  = 0;
    int32_t count = 0;
    int32_t index = 0;
    int32_t retValue = RET_ERR;
    int32_t timesNum = 0;
    int32_t countNum = 0;
    int32_t sleepNum = 0;
    time_t sleepTimes = 0;
    AudioControl_t controlData;
    LOG_Debug("%s  tid: %d", __FUNCTION__, gettid());
    while(1)
    {
        memset(controlData.udpInfo.ipAddr, 0, 64);
        g_awCmdFlag = 0;
        if(0 == g_cmdTimes)
        {
            if(g_audioControlQue.empty())
            {
                usleep(100000);
                continue;
            }

            memset(&cmdArray, 0, sizeof(AudioCmdInfo_t) * 32);
            memset(cmdBuff, 0, 1024);
            SetTTSJsonValueAsInt("times", 1);
            SetTTSJsonValueAsInt("s", 5);
            SetTTSJsonValueAsInt("m", 3);
            SetTTSJsonValueAsInt("interval", 50);
            
            pthread_mutex_lock(&g_controlLock);
            controlData = g_audioControlQue.front(); 
            g_audioControlQue.pop();
            pthread_mutex_unlock(&g_controlLock);

            if(RET_ERR == AudioGetPlayShoutStatus())
            {
                AudioUdpSendWithFd(controlData.socketFd, controlData.udpInfo.ipAddr, controlData.udpInfo.udpPort, UDP_CMD_ERROR_RET, strlen(UDP_CMD_ERROR_RET));
                continue;
            }

            if(0 == strncmp((char *)controlData.controlData, "#[TTS][codec:gbk]", strlen("#[TTS][codec:gbk]")) || \
            0 != strncmp((char *)controlData.controlData, "#[TTS][codec", strlen("#[TTS][codec")))
            {
                gbk_to_utf8((char *)controlData.controlData, cmdBuff, 1024);
            }
            else
            {
                memcpy(cmdBuff, controlData.controlData, controlData.controlLen);
            }

            if(0 == strncmp(cmdBuff, "#[TTS]", strlen("#[TTS]")))
            {
                AudioInterruptTts();
                AudioApiSetVolum(100);
                LOG_Debug("AudioInterruptTts: %s", cmdBuff);
            }
            else
            {
                LOG_Debug("AudioInterruptTts: %s", cmdBuff);
            }

            LOG_Debug("cmdBuff: %s", cmdBuff);
            cmdNum = CutCmdBuffToArray(cmdBuff, strlen(cmdBuff), cmdArray, 32);
            if(cmdNum <= 0)
            {
                retValue = RET_ERR;
                continue;
            }
        }
        
        for(count = 0; count < cmdNum; count++)
        {
            LOG_Debug("cmdArray[%d].cmdName:%s :cmdArray[%d].cmdValue :%s", count, cmdArray[count].cmdName, count, cmdArray[count].cmdValue);
            LOG_Debug("cmdArray[%d].cmdKey:%s :cmdArray[%d].ttsCode :%s", count, cmdArray[count].cmdKey, count, cmdArray[count].ttsCode);
            index = GetCmdIndexFromHash(cmdArray[count].cmdName);
            if(index < 0)
            {
                LOG_Error("%s index  is error", __FUNCTION__);
                continue;
            }
            GetAudioCmdHandleFromArray(index, &audioCmdHandle);
            if(NULL == audioCmdHandle.callBack)
            {
                LOG_Error("%s get cmd index callBack is NULL ", __FUNCTION__);
                continue;
            }
            
            retValue = audioCmdHandle.callBack(cmdArray[count].cmdKey, cmdArray[count].cmdValue, cmdArray[count].ttsCode);
            if(retValue == RET_OK && index > 4)
            {
                continue;
            }
            else
            {
                break;
            }

            audioCmdHandle.callBack = NULL;

        }

        if(retValue == RET_OK && index <= 4)
        {
            memset(retBuff, 0, 1024);
            GetIndexCmdRetStr(index, retBuff, 1024);
            AudioUdpSendWithFd(controlData.socketFd, controlData.udpInfo.ipAddr, controlData.udpInfo.udpPort, retBuff, strlen(retBuff));
        }
        else if(retValue == RET_OK && index > 4)
        {
            AudioUdpSendWithFd(controlData.socketFd, controlData.udpInfo.ipAddr, controlData.udpInfo.udpPort, UDP_CMD_SUCCESS_RET, strlen(UDP_CMD_SUCCESS_RET));
        }
        else
        {
            SetTTSJsonValueAsInt("times", 0);
            g_cmdTimes = 0;
            AudioUdpSendWithFd(controlData.socketFd, controlData.udpInfo.ipAddr, controlData.udpInfo.udpPort, UDP_CMD_ERROR_RET, strlen(UDP_CMD_ERROR_RET));
        }

        LOG_Debug("g_ttsBuff: %s", g_ttsBuff);
        timesNum = g_rootValue["tts"]["times"].asInt();
        LOG_Debug("index: %d g_ttsBuffIdex:%d timesNum:%d", index, g_ttsBuffIdex, timesNum);
        if(index >= 27 && g_ttsBuffIdex > 0)
        {
            for(countNum = 0; countNum < timesNum; countNum++)
            {
                AudioGenerateXTTS(g_rootValue["tts"]["s"].asInt(), \
                                    g_rootValue["tts"]["v"].asInt(), \
                                    g_ttsBuff, \
                                    g_rootValue["tts"]["codec:"].asString().c_str(),\
                                    g_rootValue["tts"]["m"].asInt());
                
                sleepTimes = 0;
                GetAudioPcmTimes(&sleepTimes);
                for(sleepNum = 0; sleepNum < sleepTimes && timesNum > 1; sleepNum++)
                {
                    if(true != g_audioControlQue.empty())
                    {
                        // AudioInterruptTts();
                        LOG_Debug(" if(true != g_audioControlQue.empty())");
                        break;
                    }
                    sleep(1);
                }
                if(timesNum > 1 && sleepNum < sleepTimes)
                {
                    LOG_Debug(" if(true != g_audioControlQue.empty())");
                    g_cmdTimes = 0;
                    break;
                }
                if(timesNum > 1 && sleepNum >= sleepTimes)
                {
                    usleep(g_rootValue["tts"]["interval"].asInt() * 1000);
                }
                if(g_cmdTimes > 0)
                {
                    break;
                }
            }
            memset(g_ttsBuff, 0, 1024);
            g_ttsBuffIdex = 0;
            if(g_cmdTimes > 0)
            {
                g_cmdTimes--;
                continue;
            }
        }
        LOG_Debug("g_cmdTimes: %d g_ttsBuffIdex:%d timesNum:%d", g_cmdTimes, g_ttsBuffIdex, timesNum);
        
        if(strlen(g_awFileBuff) > 0 && index == 38 && timesNum > 1)
        {
            for(countNum = 1; countNum < timesNum; countNum++)
            {
                while(1)
                {
                    if(true != g_audioControlQue.empty())
                    {
                        LOG_Debug(" if(true != g_audioControlQue.empty())");
                        break;
                    }
                    if(RET_OK == AudioGetPlayAWFileStatus())
                    {
                        LOG_Debug(" AudioGetPlayAWFileStatus");
                        break;
                    }
                    sleep(1);
                }
                if(true != g_audioControlQue.empty())
                {
                    LOG_Debug(" if(true != g_audioControlQue.empty())");
                    g_cmdTimes= 0;
                    break;
                }
                // if(timesNum > 1 && countNum >= timesNum)
                // {
                //     usleep(g_rootValue["tts"]["interval"].asInt() * 1000);
                // }
                if(g_cmdTimes > 0)
                {
                    usleep(g_rootValue["tts"]["interval"].asInt() * 1000);
                    g_cmdTimes--;
                    break;
                }
                g_playFileStartTimes = (g_get_monotonic_time() / 1000);
                AudioPlayAwFile(g_awFileBuff);
            }
        }
    }

}


void *AudioControlNotTtsThread(void *argv)
{
    AudioCmdHandle_t audioCmdHandle;
    AudioCmdInfo_t cmdArray[32];
    int32_t socketFd = -1;
    char recvCmdBuff[1024] = {0};
    char cmdBuff[1024] = {0};
    char retBuff[1024] = {0};
    int32_t recvLen = 0;
    int32_t cmdNum  = 0;
    int32_t count = 0;
    int32_t index = 0;
    int32_t retValue = RET_ERR;
    int32_t timesNum = 0;
    int32_t countNum = 0;
    time_t sleepTimes = 0;
    AudioControl_t controlData;
    LOG_Debug("%s  tid: %d", __FUNCTION__, gettid());
    while(1)
    {
        memset(controlData.udpInfo.ipAddr, 0, 64);
        memset(&cmdArray, 0, sizeof(AudioCmdInfo_t) * 32);
        memset(cmdBuff, 0, 1024);

        if(g_audioNotTtsQue.empty())
        {
            usleep(10000);
            continue;
        }
        pthread_mutex_lock(&g_controlLock);
        controlData = g_audioNotTtsQue.front(); 
        g_audioNotTtsQue.pop();
        pthread_mutex_unlock(&g_controlLock);

        memcpy(cmdBuff, controlData.controlData, controlData.controlLen);

        cmdNum = CutCmdBuffToArray(cmdBuff, strlen(cmdBuff), cmdArray, 32);
        if(cmdNum <= 0)
        {
            retValue = RET_ERR;
            continue;
        }
        
        for(count = 0; count < cmdNum; count++)
        {
            LOG_Debug("cmdArray[%d].cmdName:%s :cmdArray[%d].cmdValue :%s", count, cmdArray[count].cmdName, count, cmdArray[count].cmdValue);
            LOG_Debug("cmdArray[%d].cmdKey:%s :cmdArray[%d].ttsCode :%s", count, cmdArray[count].cmdKey, count, cmdArray[count].ttsCode);
            index = GetCmdIndexFromHash(cmdArray[count].cmdName);
            if(index < 0)
            {
                LOG_Error("%s index  is error", __FUNCTION__);
                continue;
            }
            GetAudioCmdHandleFromArray(index, &audioCmdHandle);
            if(NULL == audioCmdHandle.callBack)
            {
                LOG_Error("%s get cmd index callBack is NULL ", __FUNCTION__);
                continue;
            }
            
            retValue = audioCmdHandle.callBack(cmdArray[count].cmdKey, cmdArray[count].cmdValue, cmdArray[count].ttsCode);
            if(retValue == RET_OK && index > 4)
            {
                continue;
            }
            else
            {
                break;
            }

            audioCmdHandle.callBack = NULL;

        }

        if(retValue == RET_OK && index <= 4)
        {
            memset(retBuff, 0, 1024);
            GetIndexCmdRetStr(index, retBuff, 1024);
            AudioUdpSendWithFd(controlData.socketFd, controlData.udpInfo.ipAddr, controlData.udpInfo.udpPort, retBuff, strlen(retBuff));
        }
        else if(retValue == RET_OK && index > 4)
        {
            AudioUdpSendWithFd(controlData.socketFd, controlData.udpInfo.ipAddr, controlData.udpInfo.udpPort, UDP_CMD_SUCCESS_RET, strlen(UDP_CMD_SUCCESS_RET));
        }
        else
        {
            AudioUdpSendWithFd(controlData.socketFd, controlData.udpInfo.ipAddr, controlData.udpInfo.udpPort, UDP_CMD_ERROR_RET, strlen(UDP_CMD_ERROR_RET));
        }

    }

}



void *AudioControlThread(void *argv)
{
    AudioCmdHandle_t audioCmdHandle;
    AudioCmdInfo_t cmdArray[32];
    int32_t socketFd = -1;
    char recvCmdBuff[1024] = {0};
    char cmdBuff[1024] = {0};
    char retBuff[1024] = {0};
    int32_t recvLen = 0;
    int32_t cmdNum  = 0;
    int32_t count = 0;
    int32_t index = 0;
    int32_t retValue = RET_ERR;
    int32_t timesNum = 0;
    int32_t countNum = 0;
    time_t sleepTimes = 0;
    AudioControl_t controlData;
    
    LOG_Debug("%s  tid: %d", __FUNCTION__, gettid());
    AudioCmdHashInit();

    socketFd = AudioUdpBindPort(AUDIO_CONTROL_BIND_PORT);

    while(1)
    {
        memset(&cmdArray, 0, sizeof(AudioCmdInfo_t) * 32);
        memset(controlData.udpInfo.ipAddr, 0, 64);
        memset(cmdBuff, 0, 1024);
        // SetTTSJsonValueAsInt("times", 1);

        recvLen = AudioUdpRecFromFd_1(socketFd, recvCmdBuff, 1024, &(controlData.udpInfo));
        if(recvLen <= 0)
        {
            LOG_Error("%s AudioUdpRecFromFd_1 error ", __FUNCTION__);
            usleep(100000);
            continue;
        }


        memset(controlData.controlData, 0, CONTROL_DATA_MAX_LEN);
        controlData.controlLen = recvLen;
        memcpy(controlData.controlData, recvCmdBuff, recvLen);
        controlData.socketFd = socketFd;

        if(0 == strncmp((char *)controlData.controlData, "#[TTS]", strlen("#[TTS]")))
        {
            pthread_mutex_lock(&g_controlLock);
            g_audioControlQue.push(controlData); 
            pthread_mutex_unlock(&g_controlLock);
        }
        else
        {
            pthread_mutex_lock(&g_controlLock);
            g_audioNotTtsQue.push(controlData);
            pthread_mutex_unlock(&g_controlLock);
        }
        

        // if(0 == strncmp(recvCmdBuff, "#[TTS][codec:gbk]", strlen("#[TTS][codec:gbk]")) || \
        //    0 != strncmp(recvCmdBuff, "#[TTS][codec", strlen("#[TTS][codec")))
        // {
        //     gbk_to_utf8(recvCmdBuff, cmdBuff, 1024);
        // }
        // else
        // {
        //     memcpy(cmdBuff, recvCmdBuff, recvLen);
        // }

        // cmdNum = CutCmdBuffToArray(cmdBuff, strlen(cmdBuff), cmdArray, 32);
        // if(cmdNum <= 0)
        // {
        //     retValue = RET_ERR;
        //     continue;
        // }
        
        // for(count = 0; count < cmdNum; count++)
        // {
        //     LOG_Debug("cmdArray[%d].cmdName:%s :cmdArray[%d].cmdValue :%s", count, cmdArray[count].cmdName, count, cmdArray[count].cmdValue);
        //     LOG_Debug("cmdArray[%d].cmdKey:%s :cmdArray[%d].ttsCode :%s", count, cmdArray[count].cmdKey, count, cmdArray[count].ttsCode);
        //     index = GetCmdIndexFromHash(cmdArray[count].cmdName);
        //     if(index < 0)
        //     {
        //         LOG_Error("%s index  is error", __FUNCTION__);
        //         continue;
        //     }
        //     GetAudioCmdHandleFromArray(index, &audioCmdHandle);
        //     if(NULL == audioCmdHandle.callBack)
        //     {
        //         LOG_Error("%s get cmd index callBack is NULL ", __FUNCTION__);
        //         continue;
        //     }
            
        //     retValue = audioCmdHandle.callBack(cmdArray[count].cmdKey, cmdArray[count].cmdValue, cmdArray[count].ttsCode);
        //     if(retValue == RET_OK && index > 4)
        //     {
        //         continue;
        //     }
        //     else
        //     {
        //         break;
        //     }

        //     audioCmdHandle.callBack = NULL;

        // }

        // if(retValue == RET_OK && index <= 4)
        // {
        //     memset(retBuff, 0, 1024);
        //     GetIndexCmdRetStr(index, retBuff, 1024);
        //     AudioUdpSendWithFd(socketFd, udpInfo.ipAddr, udpInfo.udpPort, retBuff, strlen(retBuff));
        // }
        // else if(retValue == RET_OK && index > 4)
        // {
        //     AudioUdpSendWithFd(socketFd, udpInfo.ipAddr, udpInfo.udpPort, UDP_CMD_SUCCESS_RET, strlen(UDP_CMD_SUCCESS_RET));
        // }
        // else
        // {
        //     AudioUdpSendWithFd(socketFd, udpInfo.ipAddr, udpInfo.udpPort, UDP_CMD_ERROR_RET, strlen(UDP_CMD_ERROR_RET));
        // }

        // LOG_Debug("g_ttsBuff: %s", g_ttsBuff);
        // if(index > 27 && g_ttsBuffIdex > 0)
        // {
        //     timesNum = g_rootValue["tts"]["times"].asInt();
        //     for(countNum = 0; countNum < timesNum; countNum++)
        //     {
        //         AudioGenerateXTTS(g_rootValue["tts"]["s"].asInt(), \
        //                             g_rootValue["tts"]["v"].asInt(), \
        //                             g_ttsBuff, \
        //                             g_rootValue["tts"]["codec:"].asString().c_str(),\
        //                             g_rootValue["tts"]["m"].asInt());
                
        //         if(timesNum > 1)
        //         {
        //             sleepTimes = 0;
        //             GetAudioPcmTimes(&sleepTimes);
        //             sleep(sleepTimes);
        //             usleep(g_rootValue["tts"]["interval"].asInt() * 1000);
        //         }
        //     }
        //     memset(g_ttsBuff, 0, 1024);
        //     g_ttsBuffIdex = 0;
        // }
        
    }

}
