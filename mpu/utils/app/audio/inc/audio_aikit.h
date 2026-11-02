#ifndef _AUDIO_AIKIT_H_
#define _AUDIO_AIKIT_H_

#include <stdint.h>
#include "sr_log.h"
#include "err_code.h"

void testTtts(int32_t speed, int32_t volume);
void AudioAikitSdkInt();
void AudioGenerateXTTS(int32_t speed, int32_t volume, const char *ttsStr, const char *encoding, uint8_t fayin);
int32_t GetAudioPcmTimes(time_t *pcmTime);
int32_t GetAudioTotalPcmTimes(time_t *pcmTime);
int32_t GetAudioAikitStatus();
#endif