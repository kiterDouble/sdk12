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
#include "audio_common.h"
#include "audio_control.h"


/**
 * UDP 发送函数
 * @param dest_ip   目标IP地址（字符串形式，如 "127.0.0.1"）
 * @param dest_port 目标端口号
 * @param message   要发送的消息
 * @param msg_len   消息长度
 * @return 0 成功，-1 失败
 */
int32_t AudioUdpSend(const char *destIp, int destPort, const char *message, int32_t msgLen) 
{
    int32_t sockFd;
    struct sockaddr_in destAddr;

    sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFd < 0) 
    {
        LOG_Error("Socket 创建失败");
        return RET_ERR;
    }

    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(destPort);
    if (inet_aton(destIp, &destAddr.sin_addr) == 0) 
    {
        LOG_Error("无效的IP地址: %s", destIp);
        close(sockFd);
        return RET_ERR;
    }

    ssize_t sentBytes = sendto(sockFd, message, msgLen, 0,
                    (struct sockaddr*)&destAddr, sizeof(destAddr));
    if (sentBytes < 0) 
    {
        LOG_Error("发送失败");
        close(sockFd);
        return RET_ERR;
    }

    LOG_Debug("已发送 %zd 字节到 %s:%d\n", sentBytes, destIp, destPort);
    close(sockFd);  // 发送完即可关闭
    return RET_OK;
}

/**
 * UDP 发送函数
 * @param dest_ip   目标IP地址（字符串形式，如 "127.0.0.1"）
 * @param dest_port 目标端口号
 * @param message   要发送的消息
 * @param msg_len   消息长度
 * @return 0 成功，-1 失败
 */
int32_t AudioUdpSendWithFd(int32_t socketFd, const char *destIp, int destPort, const char *message, int32_t msgLen) 
{
    struct sockaddr_in destAddr;
    if(message && msgLen > 0){
        //LOG_DebugHex(message,msgLen, "Send udpData:");
        LOG_Debug("AudioUdpSendWithFd destIp:%s destPort:%d message %02X %02X msgLen:%d \n",destIp, destPort,message[0],message[msgLen-1],msgLen);
    }else{
        LOG_Error("error message:%p msgLen:%d",message,msgLen);
        return RET_ERR;
    }
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(destPort);
    if (inet_aton(destIp, &destAddr.sin_addr) == 0) 
    {
        LOG_Error("无效的IP地址: %s", destIp);
        return RET_ERR;
    }

    ssize_t sentBytes = sendto(socketFd, message, msgLen, 0,
                    (struct sockaddr*)&destAddr, sizeof(destAddr));
    if (sentBytes < 0) 
    {
        LOG_Error("发送失败");
        return RET_ERR;
    }

    LOG_Debug("udp sebd msg %zd bytes to %s:%d[%s]\n", sentBytes, destIp, destPort, message);
    return RET_OK;
}


int32_t AudioUdpBindPort(int32_t listenPort) 
{
    int32_t sockFd;
    struct sockaddr_in localAddr;
    int reuse = 1;

    sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFd < 0) 
    {
        LOG_Error("Socket 创建失败");
        return RET_ERR;
    }

    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = inet_addr(UDP_AUDIO_BAND_ADDR);  // 监听所有网卡
    localAddr.sin_port = htons(listenPort);

    setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    if (bind(sockFd, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0) 
    {
        LOG_Error("Bind 失败");
        close(sockFd);
        return RET_ERR;
    }
    LOG_Debug("band port[%d] success", listenPort);
    return sockFd;
}


int32_t AudioUdpRecFromFd(int32_t sockFd, char *buffer, int32_t bufLen) 
{
    ssize_t recvLen;
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    recvLen = recvfrom(sockFd, buffer, bufLen - 1, 0,
                        (struct sockaddr*)&clientAddr, &clientLen);
    if (recvLen < 0) 
    {
        LOG_Error("接收失败");
        return RET_ERR;
    }

    buffer[recvLen] = '\0';  // 确保字符串结束

    LOG_Debug("来自 %s:%d 的消息: %d\n", inet_ntoa(clientAddr.sin_addr),
                        ntohs(clientAddr.sin_port), recvLen);

    return recvLen;
}


int32_t AudioUdpRecFromFd_1(int32_t sockFd, char *buffer, int32_t bufLen, UdpInfo_t *udpInfo) 
{
    ssize_t recvLen;
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    recvLen = recvfrom(sockFd, buffer, bufLen - 1, 0,
                        (struct sockaddr*)&clientAddr, &clientLen);
    if (recvLen < 0) 
    {
        LOG_Error("接收失败");
        return RET_ERR;
    }

    buffer[recvLen] = '\0';  // 确保字符串结束

    LOG_Debug("来自 %s:%d 的消息: %d\n", inet_ntoa(clientAddr.sin_addr),
                        ntohs(clientAddr.sin_port), recvLen);
    memcpy(udpInfo->ipAddr, inet_ntoa(clientAddr.sin_addr), strlen(inet_ntoa(clientAddr.sin_addr)));
    udpInfo->udpPort = ntohs(clientAddr.sin_port);

    return recvLen;
}

/**
 * @name SrGetSystemRetValue
 * @brief  执行shell 命令，并获取返回值
 * @param 
 * @return -1 执行命令失败，其他值为命令的返回值
 */
int32_t SrGetSystemRetValue(const char *cmd)
{
	char cmdBuff[128] = {0};
	FILE *fileFp;				// 文件指针  
    int32_t cmdStatus;			// 用于存储命令的退出状态
	
	if(NULL == cmd)
	{
		LOG_Error("%s cmd is NULL", __FUNCTION__);
		return RET_ERR;
	}
    
    LOG_Debug("start exec cmd :%s", cmd);

	fileFp = popen(cmd, "r");
	if(NULL == fileFp)
	{
		LOG_Error("%s popen error :%s", __FUNCTION__, strerror(errno));
		return RET_ERR;
	}
	
	while(fgets(cmdBuff, 128, fileFp) != NULL)
	{
		LOG_Debug("get cmd ret: %s", cmdBuff);
	}
	
	cmdStatus = pclose(fileFp);
	if(-1 == cmdStatus)
	{
		LOG_Error("%s pclose error :%s", __FUNCTION__, strerror(errno));
		return RET_ERR;
	}
	
	if(WIFEXITED(cmdStatus))
	{  
        LOG_Debug("Command exited with status %d", WEXITSTATUS(cmdStatus));
		return WEXITSTATUS(cmdStatus);
    }
	else if(WIFSIGNALED(cmdStatus))
	{  
        LOG_Error("Command was terminated by signal %d", WTERMSIG(cmdStatus));
		return RET_ERR;
    }
	
	LOG_Error("Command was terminated not find signal" );
	return RET_ERR;
}

int32_t AudioCreateThread(pthread_t *threadId, threadFun fun, void *threadArg)
{
	pthread_attr_t attr;
	int32_t retValue = -1;

	if(NULL == threadId )
	{
		LOG_Error("%s argv threadId  is NULL", __FUNCTION__);
		return RET_ERR;
	}

	pthread_attr_init(&attr);

	//创建线程
	retValue = pthread_create(threadId, &attr, fun,(void *)threadArg);
	if(retValue < 0)
	{
		LOG_Error("pthread_create %s\n",strerror(errno));
		return RET_ERR;
	}

	LOG_Debug("national  thread success");
	return RET_OK;
}

char* AudioFindFileByNumber(int fileNum, char *outputPath, size_t outputPathSize)
{
    if(outputPath == NULL || outputPathSize == 0) {
        return NULL;
    }

    char baseDir[128] = "/oemdata/tts/";
    char tempPath[128];
    bool found = false;
    
    // 尝试.wav格式
    snprintf(tempPath, sizeof(tempPath), "%s%03d.wav", baseDir, fileNum);
    if (access(tempPath, F_OK) == 0) {
        strncpy(outputPath, tempPath, outputPathSize - 1);
        outputPath[outputPathSize - 1] = '\0';
        found = true;
    }
    
    // 尝试.pcm格式 
    if (!found) {
        snprintf(tempPath, sizeof(tempPath), "%s%03d.pcm", baseDir, fileNum);
        if (access(tempPath, F_OK) == 0) {
            strncpy(outputPath, tempPath, outputPathSize - 1);
            outputPath[outputPathSize - 1] = '\0';
            found = true;
        }
    }
    
    // 尝试.mp3格式
    if (!found) {
        snprintf(tempPath, sizeof(tempPath), "%s%03d.mp3", baseDir, fileNum);
        if (access(tempPath, F_OK) == 0) {
            strncpy(outputPath, tempPath, outputPathSize - 1);
            outputPath[outputPathSize - 1] = '\0';
            found = true;
        }
    }
    
    if (!found) {
        LOG_Error("No audio file found with number %03d", fileNum);
        return NULL;
    }
    
    return outputPath;
}
