#ifndef _AUDIO_PLAYBACK_H_
#define _AUDIO_PLAYBACK_H_

#include <stdint.h>
#include "sr_log.h"
#include "err_code.h"
#include "ql-sdk/ql_type.h"
#include "ql_ms_audio_pcm.h"
#include "ql_ms_audio_cfg.h"

#define PCM_DATA_MAX_LEN  (2048)

typedef struct 
{
	uint32_t pcmDataLen;
	uint8_t	 pcmData[PCM_DATA_MAX_LEN];
}AudioTTSPcm_t;


int32_t AudioApiInit(void);
int32_t AudioApiDeinit(void);

int32_t AudioApiPlaybackOpen(void);
int32_t AudioApiPlaybackClose(void);

int32_t AudioApiPlaybackPlay(void);

int32_t AudioApiPlaybackStop(void);

int32_t AudioApiPlaybackPause(void);

int32_t AudioApiPlaybackResume(void);
int32_t AudioApiPlaybackStreamPrepare(int32_t channels, int32_t format, int32_t sampleRate);

int32_t AudioApiPlaybackGetBufferSize(void);

int32_t AudioApiPlaybackPushStream(const char *buffer, int32_t bufferSize);

void *AudioPlayBackThread(void *argv);
    

int32_t AudioOpenShoutMs(int32_t msTime);

int32_t AudioCloseShout();

int32_t AudioPlayAwFile(const char *fileName);

int32_t AudioTTSIdleStatus();

int32_t AudioPushPcmDataToQue(uint8_t *pcmData, uint32_t pcmDataLen);

int32_t AudioInterruptTts();

int32_t AudioGetPlayAWFileStatus();

int32_t AudioApiSetVolum(int32_t volum);

int32_t AudioIsTTSQueEmpty();

int32_t AudioGetPlayShoutStatus();

void *AudioRecvPlayBackDataThread(void *argv);

int32_t AudioGetPlayShoutStatus();

int32_t AudioGetPlayTtsStatus();

int32_t AudioGetPlayFileStatus();

uint64_t AudioGetFileTimes(void);

#endif