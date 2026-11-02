#ifndef NET_MONITOR_IPC_MSG_H__
#define NET_MONITOR_IPC_MSG_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MONITOR_CONNECT_SUCCESS 1
#define MONITOR_CONNECT_ERROR 0

typedef struct
{
    uint8_t url1;   //eos-wuxi-5.cmecloud.cn
    uint8_t url2;   //relay.zelostech.com.cn
    uint8_t url3;   //video1.zelostech.com.cn
    uint8_t gateway;    //192.168.1.1
    uint8_t xxka;   //192.168.1.2
    uint8_t xxkb;   //192.168.1.3
    uint8_t onlineLanguageModule;   //192.168.1.8
    uint8_t bluetoothToEthernetModule;  //192.168.1.20
    uint8_t module1;    //192.168.1.201
    uint8_t module2;    //192.168.1.202      
}NetState_t;

#ifdef __cplusplus
}
#endif

#endif
