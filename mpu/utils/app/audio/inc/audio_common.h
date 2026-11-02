#ifndef _AUDIO_COMMON_H_
#define _AUDIO_COMMON_H_

#include <glib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "sr_log.h"
#include "err_code.h"


#define UDP_AUDIO_BAND_ADDR     "192.168.1.8"
#define UDP_CMD_SUCCESS_RET     "OK"
#define UDP_CMD_ERROR_RET       "NG"
#define UDP_CMD_RET_ADDR        "192.168.1.2"
#define UDP_CMD_RET_PORT        (7889)

#define  AUDIO_PLAY_BACK_SHOUT_BIND_PORT    (7890)
#define  AUDIO_PLAY_BACK_TTS_BIND_PORT      (9990)
#define  AUDIO_PLAY_BACK_TTS_PCM_LEN        (640)


typedef void *(threadFun)(void *);

typedef struct 
{
    char ipAddr[64];
    uint32_t udpPort;
}UdpInfo_t;

int32_t AudioUdpSend(const char *destIp, int destPort, const char *message, int32_t msgLen);
int32_t AudioUdpSendWithFd(int32_t socketFd, const char *destIp, int destPort, const char *message, int32_t msgLen);
int32_t AudioUdpBindPort(int32_t listenPort);
int32_t AudioUdpRecFromFd(int32_t sockfd, char *buffer, int32_t bufLen);
int32_t SrGetSystemRetValue(const char *cmd);
int32_t AudioCreateThread(pthread_t *threadId, threadFun fun, void *threadArg);

int32_t AudioUdpRecFromFd_1(int32_t sockFd, char *buffer, int32_t bufLen, UdpInfo_t *udpInfo);

// 根据文件编号查找音频文件
char* AudioFindFileByNumber(int fileNum, char *outputPath, size_t outputPathSize);

#endif