#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "http.h"
#include "sr_log.h"

#define SERVER_IP "192.168.1.8"  // 服务器IP地址
#define SERVER_PORT 7889        // 服务器端口


int SetVolumeLevel(unsigned char level,int flag) 
{
    int sockfd;
    struct sockaddr_in server_addr;
    char volum_buffer[50] = {0};
    char buffer[10] = {0};
    int ret = 0;
    int opt =1;
    if(flag == 1)
    {
        sprintf(volum_buffer,"#[CMD]volum=%d",level);
    }else if(flag == 2)
    {
        sprintf(volum_buffer,"#[TTS][v%d]",level);
    }else{
        LOG_Error("flag error flag:%d level:%d",flag,level);
    }
    
    
    // 创建UDP套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        LOG_Error("socket creation failed");
        return -1;
    }


    // 设置服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_port = htons(SERVER_PORT); // 端口
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP); // IP地址
    LOG_Debug("Sent message: %s\n", volum_buffer);
    // 发送消息到服务器
    ssize_t sent_bytes = sendto(sockfd, volum_buffer, strlen(volum_buffer), 0,
                                 (const struct sockaddr *)&server_addr, sizeof(server_addr));
    if (sent_bytes < 0) {
        LOG_Error("sendto failed");
        close(sockfd);
        return -1;
        
    }

    
    // 设置接收超时时间
    struct timeval timeout;
    timeout.tv_sec = 5;  // 设置超时为 5 秒
    timeout.tv_usec = 0; // 设置微秒部分为 0

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
        LOG_Error("设置接收超时失败");
        close(sockfd);
        return -1;
    }

    LOG_Debug("等待接收数据...\n");
    // 接收服务器响应
    socklen_t addr_len = sizeof(server_addr);
    ssize_t recv_bytes = recvfrom(sockfd, buffer, 29, 0,
                                   (struct sockaddr *)&server_addr, &addr_len);
    if (recv_bytes < 0) 
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            LOG_Error("接收超时\n");
        } else {
            LOG_Error("接收数据失败");
        }
        LOG_Error("recvfrom failed");
        close(sockfd);
        return -1;
    }

    LOG_Debug("Received response: %s", buffer);

    if(strncmp(buffer,"OK",strlen("OK")) == 0)
    {
        close(sockfd);
        return 0;
    }else if(strncmp(buffer,"NG",strlen("NG")) == 0)
    {
        close(sockfd);
        return -1;
    }else{
        LOG_Error(" Unknow Received response...");
        close(sockfd);
        return -1;
    }

    return 0;
}



