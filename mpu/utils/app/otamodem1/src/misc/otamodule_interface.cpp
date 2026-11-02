#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "completion_timeout.h"
#include "https.h"
#include "otamodule_interface.h"
#include "file_interface.h"
#include "sr_log.h"

void CloseSocketFd(int sockfd)
{
    close(sockfd);
    sockfd = -1;
}

int ConnectToServer() 
{
    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        LOG_Error("socket failed");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    //if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
    if (inet_pton(AF_INET, SERVERIP, &server_addr.sin_addr) <= 0) {
        LOG_Error("inet_pton failed");
        CloseSocketFd(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        LOG_Error("connect fail \n");
        CloseSocketFd(sockfd);
        return -1;
    }

    return sockfd;
}

void delay_ms() 
{
    struct timespec req = {
        .tv_sec  = 0,          // 秒
        .tv_nsec = 1 * 1000000 // 纳秒 (10ms = 10,000,000ns)
    };
    struct timespec rem;
    
    while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
        req = rem; // 被信号中断后继续剩余延时
    }
}

                 

void * OtaModule2(void *arg)
{
    int sock = 0;
    unsigned char buffer[BUFFER_SIZE];
    size_t bytes_to_send;
    double filesize = 0;
    char bufferdouble[10] = {0};
    char buf[128] = {0};
    FILE *file;
    int ret = 0;
    long total_bytes_received = 0;
    while(1)
    {

        if(WaitForCompletionTimeout(GetOtaComp(), -1))
        {
            LOG_Debug("Start Connect To Server\n");
           
            
                sock = ConnectToServer();
                if(sock < 0)
                {
                    LOG_Error("sock = %d \n",sock);
                    continue;
                }

                if(access(DOWNLOADUPGRADEPACKAGEFILEPATH,F_OK) != 0)
                {
                    CloseSocketFd(sock);
                    LOG_Debug("There is no local upgrade package available %s\n",DOWNLOADUPGRADEPACKAGEFILEPATH);
                    continue;
                }

                filesize =  GetFileSize(DOWNLOADUPGRADEPACKAGEFILEPATH); 
                if(filesize < 0)
                {
                    LOG_Error("Get File Size Error ");
                    CloseSocketFd(sock);
                    LOG_Error("Error in obtaining file size %s\n",DOWNLOADUPGRADEPACKAGEFILEPATH);
                    continue;
                }else{
                    
                    memcpy(bufferdouble, &filesize, sizeof(double));
                    ret = send(sock, bufferdouble, sizeof(double), 0);
                    if(ret < 0)
                    {
                        LOG_Error("send file size error %s ret = %d",DOWNLOADUPGRADEPACKAGEFILEPATH,ret);
                        CloseSocketFd(sock);
                        continue;
                    }
                }
                memset(bufferdouble,0,sizeof(bufferdouble));
                //send(sock,UPGRADESTART, strlen(UPGRADESTART), 0);
                //sleep(5);
                file = fopen(DOWNLOADUPGRADEPACKAGEFILEPATH, "rb");
                if (file == NULL) {
                    LOG_Error("Failed to open file");
                    CloseSocketFd(sock);
                    continue;
                }
                total_bytes_received = 0;
                //发送升级包
                LOG_Debug("Sending upgrade package...\n");
                while ((bytes_to_send = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
                    delay_ms();
                    ret = send(sock, buffer, bytes_to_send, 0);
                    if(ret < 0 )
                    {
                        LOG_Error("send data fail");
                        break;
                    }

                    total_bytes_received = bytes_to_send + total_bytes_received;
                    memset(buffer, 0, BUFFER_SIZE);
                    LOG_Debug("Bytes sent: %ld\n", total_bytes_received);
                }
                sleep(2);
                if(ret < 0 )
                {
                    CloseSocketFd(sock);
                    fclose(file);
                    continue;
                }
                //发送结束标志
                const char* end_signal = "end";
                ret = send(sock, end_signal, strlen(end_signal), 0);
                if(ret < 0 )
                {
                    CloseSocketFd(sock);
                    fclose(file);
                    LOG_Error("send data fail");
                    continue;
                }

                fclose(file);
                memset(buffer, 0, BUFFER_SIZE);
                memset(buf, 0, 128);
                int valread = read(sock, (char *)buf, 128);
                if(valread <= 0 )
                {
                    LOG_Error("Connection closed or error occurred\n");
                    CloseSocketFd(sock);
                    continue;
                }

                if(strncmp((char *)buf,UPGRADESUCCESS,strlen(UPGRADESUCCESS)) == 0)
                {
                    Complete(GetComp());
                    
                }else{
                    LOG_Debug("upgrades status %s",buf);
                }
                CloseSocketFd(sock);
            
        }
    }

    return NULL;
}
