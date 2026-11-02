#ifndef _AUDIO_CAPTURE_H_
#define _AUDIO_CAPTURE_H_

#include <stdint.h>
#include "sr_log.h"
#include "err_code.h"
#include "ql-sdk/ql_type.h"
#include "ql_ms_audio_pcm.h"
#include "ql_ms_audio_cfg.h"

#define  AUDIO_CAPTURE_FILE_PATH_NAME       "/oemdata/tts/audio_test.pcm"

int32_t AudioApiCaptureOpen(void);

int32_t AudioApiCaptureFilePrepare(const char *fileName, uint32_t sampleRate, uint8_t channels);

int32_t AudioApiCaptureRecord(void);

int32_t AudioApiCaptureStop(void);

int32_t AudioApiCaptureClose(void);

int32_t item_ql_ms_audio_capture_pause(void);

int32_t item_ql_ms_audio_capture_resume(void);


int32_t AudioApiCaptureStreamPrepare(int32_t channels, int32_t format, int32_t sampleRate);


int32_t AudioApiCaptureGetBufferSize(void);

int32_t AudioApiCapturePullStream(char *buffer, int32_t bufferSize);

void *AudioApiCaptureThread(void *argv);

int32_t AudioOpenCaptureMs(int32_t msTime);

int32_t AudioCloseCapture();

int32_t AudioOpenFileCaptureMs(int32_t msTime);

void * AudioApiCaptureDataSendThread(void *argv);

int32_t AudioCaptureGetStatus();

#endif