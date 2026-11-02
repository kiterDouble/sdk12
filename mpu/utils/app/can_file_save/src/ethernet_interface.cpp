
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include "crcTool.h"
#include "can_file_save.h"
#include "file_interface.h"
#include "ethernet_interface.h"
#include "sr_log.h"
#include "tmier_interface.h"
using namespace std;

// 线程安全的队列类
 

void SafeQueue::enqueue(const std::vector<unsigned char>& data) 
{
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(data);
}

bool SafeQueue::dequeue(std::vector<unsigned char>& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        data = queue_.front();
        queue_.pop();
        return true;
    }

void SafeQueue::canenqueue(const std::vector<unsigned char>& candata) {
        std::lock_guard<std::mutex> lock(canmutex_);
        canqueue_.push(candata);
    }

bool SafeQueue::candequeue(std::vector<unsigned char>& candata) {
        std::lock_guard<std::mutex> lock(canmutex_);
        if (canqueue_.empty()) {
            return false;
        }
        candata = canqueue_.front();
        canqueue_.pop();
        return true;
    }


bool SafeQueue::getcandequeue() 
{
       
	if (canqueue_.empty()) 
	{
            return false;
        }
       
        return true;
}


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
    serverAddr.sin_addr.s_addr = INADDR_ANY;
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
void handleServer(SafeQueue& queue) {
    unsigned char buffer[DATA_LENGTH] = {0};
    ssize_t bytesReceived = 0;
    int serverSock = 0;
    int ret = 0;
    int retryCount = 0;
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    do 
    {
        ret = UDPInit(serverSock);
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
            bytesReceived = recvfrom(serverSock, buffer, DATA_LENGTH, 0,(struct sockaddr*)&clientAddr, &clientAddrLen);
            if (bytesReceived < 0) {
                LOG_Error("recvfrom failed with error: %s  bytesReceived = %d", strerror(errno),bytesReceived);
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) 
                {
                    continue;
                }
                close(serverSock);
                UDPInit(serverSock);
                break;
            }
            //LOG_Debug("recv bytesReceived = %d",bytesReceived);
            
            std::vector<unsigned char> data(buffer, buffer + bytesReceived);
            queue.enqueue(data);
            
        }
    }

    close(serverSock);
}

void SubcontractingHandleQueue(SafeQueue& queue) 
{

    while (1)
    {
        // 主线程读取队列中的数据并打印
        DelayMS(1);
        std::vector<unsigned char> dataBuffer;
        if (queue.dequeue(dataBuffer)) 
        {
#if 0
            LOG_DebugHex(dataBuffer, dataBuffer.size(), "MCU send DATA");
#endif
            if(dataBuffer[0] == 0x55 && dataBuffer[1] == 0xaa)
            {
                uint16_t srvLen = ((dataBuffer[5] &0xff) << 8) + (dataBuffer[6] &0xff);
                //uint16_t crc16Val = ((dataBuffer[srvLen + 8] & 0xff ) << 8) + (dataBuffer[srvLen + 9] & 0xff);
                uint16_t crc16Val = ((dataBuffer[dataBuffer.size() -2] & 0xff ) << 8) + (dataBuffer[dataBuffer.size() -1] & 0xff);
                uint16_t crcval =  Crc16((uint8_t *)&dataBuffer[0],srvLen + 8, Crc16Ccitt);

                if(crc16Val == Crc16((uint8_t *)&dataBuffer[0],srvLen + 8, Crc16Ccitt))
                {
                    //LOG_Debug("CRC Ok");

                }else
                {
                    LOG_Error("CRC FAIL mcu crc:%x mpu crc:%x ",crc16Val,crcval);
                    continue;
                }

                if(srvLen % 15 != 0)
                {
                    LOG_Error("srvLen % 15 != 0");
                }

                for(int i = 0;i< srvLen /15 ; i++)
                {
                    std::vector<unsigned char> Buffer;
                    Buffer.resize(50);
                    long long time = GetCurrentTime(); 
                    memcpy(&(Buffer[0]),&time,8);
                    memcpy(&(Buffer[8]),&(dataBuffer[(i*15 + 8)]) ,15);
                    queue.canenqueue(Buffer);
                }


            }else{

                LOG_Error("data head incorrect...");
            }

        }

    }
  
    

}

void WriteHandle(SafeQueue& queue) 
{
   
    CAN_Frame canframe[10001] = {0};
    int ret = 0;
    int count = 0;
    while (1)
    {
        DelayMS(700);
        
        memset(canframe,0,sizeof(canframe));
        for(int i = 0;i<10000;i++)
        {
            std::vector<unsigned char> dataBuffer;
            if (queue.candequeue(dataBuffer)) 
            {
                canframe[i].timestamp = 0;
                memcpy(&(canframe[i].timestamp),&(dataBuffer[0]),8);
                canframe[i].channel = dataBuffer[8];
                if(dataBuffer[9] == 0)
                {
                    canframe[i].direction = 'R';
                }else{
                    canframe[i].direction = 'T';
                }
                
                canframe[i].id = (dataBuffer[10] << 24) | (dataBuffer[11] << 16) | (dataBuffer[12] << 8) | dataBuffer[13];
                canframe[i].dlc = dataBuffer[14];
                
                memcpy(canframe[i].data,&dataBuffer[15],8);
                canframe[i].dataflag = 'd';
                count = i;
            
            }else{
                break;
            }
        }
        //LOG_Debug("WriteHandle 50ms count: %d",count);
        if(count > 0)
        {
            ret = WriteDataWithRotation(canframe,count);
            if(ret < 0 )
            {
                LOG_Error("Write Data With Rotation fail");
            }
            count = 0;
        }
       
    }
    

}


int EthernetInit() 
{
    SafeQueue queue;
    std::vector<std::thread> threads;

    // 接受客户端连接
    
    threads.emplace_back(handleServer,std::ref(queue));
    threads.emplace_back(SubcontractingHandleQueue,std::ref(queue));
    threads.emplace_back(WriteHandle,std::ref(queue));
    while (1)
    {
        sleep(10);
    }
   
    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    return 0;
}
