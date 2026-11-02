#ifndef _AUDIO_CONTROL_CONFIG_H_
#define _AUDIO_CONTROL_CONFIG_H_

#include "audio_config_file.h"
#include "audio_common.h"

typedef  int32_t (*AudioCmdHandleCallBack)(const char *cmdStr, const char *cmdValue, const char *ttsCode);

#define CONTROL_DATA_MAX_LEN  (1024)

typedef struct 
{
    int32_t socketFd;
    UdpInfo_t udpInfo;
	uint32_t controlLen;
	uint8_t	 controlData[CONTROL_DATA_MAX_LEN];
}AudioControl_t;

typedef struct 
{
    char cmdName[32];
    AudioCmdHandleCallBack callBack;
}AudioCmdHandle_t;

typedef struct 
{
    char cmdName[32];
    char cmdKey[32];
    char cmdValue[1024];
    char ttsCode[1024];
    int32_t socketFd;
}AudioCmdInfo_t;

int32_t AudioCmdHashInit();
int32_t AudioCmdHashDestroy();
int32_t GetCmdIndexFromHash(const char *cmdStr);
int32_t ConfigDeviceAddr(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t ConfigControlPort(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t ConfigShoutPort(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t ConfigShoutAudioCodec(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t ConfigShoutAudioSample(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t ConfigShoutAudioBit(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t ConfigShoutVolum(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t ConfigMicSendIp(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t ConfigMicSendPort(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t ConfigMicAudioCodec(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t ConfigMicAudioSample(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t ConfigMicAudioBit(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t ConfigMicFrameSize(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t ConfigLaunchTts(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t ConfigUploadPort(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t ConfigPowerOnTts(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t CmdOpenShout(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t CmdCloseShout(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t CmdOpenMic(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t CmdCloseMic(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t CmdReset(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t CmdVolum(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t CmdDelete(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t GetGetConfig(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t GetGetDeviceInfo(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t GetGetStatus(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t GetGetTts(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t TTSTimes(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t TTSinterval(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t TTSCodec(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t TTSM(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t TTSN(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t TTSP(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t TTSS(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t TTSV(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t TTSY(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t TTSDD(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t TTSB(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t TTSAW(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t TTSTts(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t SrCmdPlayCapyureFile(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t SrCmdStartCapyureFile(const char *cmdStr, const char *cmdValue, const char *ttsCode);
int32_t GetAudioCmdHandleFromArray(int32_t index, AudioCmdHandle_t *audioCmdHandle);
int32_t GetCmdIndexFromHash(const char *cmdStr);
int32_t GetIndexCmdRetStr(int32_t index, char *retBuff, int32_t buffLen);
int32_t InitJsonFileHandler();

int32_t GetConfigJsonKeyValue(char *configBuff, int32_t buffLen);
int32_t GetTTSJsonKeyValue(char *ttsBuff, int32_t buffLen);
int32_t GetStatusJsonKeyValue(char *statusBuff, int32_t buffLen);
int32_t GetDeviceInfoJsonKeyValue(char *deviceInfo, int32_t buffLen);
int32_t SetConfigJsonValueAsStr(const char *jsonKey, const char *jsonValue);
int32_t SetConfigJsonValueAsInt(const char *jsonKey, int64_t jsonValue);
int32_t SetTTSJsonValueAsStr(const char *jsonKey, const char *jsonValue);

int32_t SetTTSJsonValueAsInt(const char *jsonKey, int64_t jsonValue);

int32_t SetStatusJsonValueAsStr(const char *jsonKey, const char *jsonValue);

int32_t SetStatusJsonValueAsInt(const char *jsonKey, int64_t jsonValue);

int32_t GetTTSJsonValueAsStr(const char *jsonKey, char *jsonValue, int32_t valueLen);

int32_t GetTTSJsonValueAsInt(const char *jsonKey, int64_t *jsonValue);

void *AudioControlThread(void *argv);

void *AudioControlHandleThread(void *argv);

void *AudioControlNotTtsThread(void *argv);

int32_t AudioSendVehicleStartTTS();

int32_t AudioSendTTSCmdNotDoing();

#endif