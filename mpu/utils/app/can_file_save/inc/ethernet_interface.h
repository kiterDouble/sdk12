#ifndef __ETHERNET_INTERFACE_H__
#define __ETHERNET_INTERFACE_H__


#include <iostream>
#include <vector>
#include <queue>
#include <mutex>
#include <cstring>


#ifdef __cplusplus
extern "C" {
#endif




#define SERVER_PORT 5000
#define DATA_LENGTH 1500 // 固定长度的数据

// 线程安全的队列类
class SafeQueue {
public:
    void enqueue(const std::vector<unsigned char>& data);
    bool dequeue(std::vector<unsigned char>& data); 
    void canenqueue(const std::vector<unsigned char>& candata) ;
    bool candequeue(std::vector<unsigned char>& candata); 
    bool getcandequeue();
private:
    std::queue<std::vector<unsigned char>> queue_;
    std::mutex mutex_;
    std::queue<std::vector<unsigned char>> canqueue_;
    std::mutex canmutex_;
};

int EthernetInit();




#ifdef __cplusplus
}
#endif
#endif
