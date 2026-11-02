#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include "udp_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sr_log.h"
#include "mqtt.h"
#include "cjson_interface.h"

#define SERVER_PORT 5010
#define DATA_LENGTH 1500 // 固定长度的数据

static  struct sockaddr_in clientAddr = {0};
static  int serversock;
int UDPInit(int &serverSock)
{
    
    struct sockaddr_in serverAddr;
    int opt = 1;
    // 创建套接字
    if ((serverSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        LOG_Error("Socket creation failed");
        return -1;
    }

    if (setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        LOG_Error("setsockopt failed");
        close(serverSock);
        return -1;
    }
    // 设置服务器地址
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    //serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(SERVER_PORT);
     

    // 绑定套接字
    if (bind(serverSock, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        LOG_Error("Bind failed");
        close(serverSock);
        return -1;
    }
    return 0;
}

// 接收固定长度数据并将其放入队列的函数
void * UdphandleServer(void *arg)
{
    char buffer[DATA_LENGTH] = {0};
    ssize_t bytesReceived = 0;
    
    int ret = 0;
    int retryCount = 0;
   
    socklen_t clientAddrLen = sizeof(clientAddr);
    mqtt_data data = {0};

    do 
    {
        ret = UDPInit(serversock);
        if(ret < 0)
        {
            LOG_Error("UDP Init Fail");
        }
        retryCount++;
        if (retryCount >= 5) {
            LOG_Error("Exceeded maximum retry attempts. Exiting.");
            break;
        }

    }while (ret != 0);
   

    while(1)
    {
        LOG_Debug("Waiting for client connection..."); 
        while (true) 
        {
            memset(buffer, 0, sizeof(buffer));
            bytesReceived = recvfrom(serversock, buffer, DATA_LENGTH, 0,(struct sockaddr*)&clientAddr, &clientAddrLen);
            if (bytesReceived < 0) {
                
                LOG_Error("recv failed bytesReceived = %d",bytesReceived);
                break;
            }
            
            LOG_Debug("bytesReceived = %lld  buffer : %s",bytesReceived,(char *)buffer);
            memset(&data,0,sizeof(mqtt_data));
            data.payload  = SyncOtaFieldVehicleInfoCreatJSONIntetface((char* )buffer);
            data.topic = MqttHandle::getInstance().topic_handle.tbox_ota_reply_topic;
            data.payloadlen = strlen(data.payload);
            mqtt_publish(data,1);
            if(data.payload)
            {
                free(data.payload);
            }
            

            
        }
    }

    close(serversock);
    return NULL;
}


int UdpServerSend(char *result)
{
 
    ssize_t bytes_sent = sendto(serversock, result, strlen(result), 0, (const struct sockaddr *) &clientAddr, sizeof(clientAddr));
     if (bytes_sent < 0) {
        LOG_Error("sendto failed");
        // 根据 errno 进行具体错误处理
        switch (errno) {
            case EACCES:
                LOG_Error("Permission denied.");
                break;
            case EDESTADDRREQ:
                LOG_Error("Destination address required.");
                break;
            case ENETUNREACH:
                LOG_Error("Network unreachable.");
                break;
            case ENOBUFS:
                LOG_Error("Insufficient buffer space.");
                break;
            case ECONNRESET:
                LOG_Error("Connection reset by peer.");
                break;
            case EPIPE:
                LOG_Error("Broken pipe.");
                break;
            default:
                LOG_Error( "Unknown error: %d", errno);
                break;
        }
    } else {
        LOG_Debug("Sent %zd bytes to server.", bytes_sent);
    }

    return bytes_sent;
}

