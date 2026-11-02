#ifndef __TSP_MONITOR_H__
#define __TSP_MONITOR_H__

#include <glib.h>
#include "tsp_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef enum {
    MONITOR_STATUS_UNKNOWN = -1,            //未知结果
    MONITOR_STATUS_FAIL = 0,                //执行失败
    MONITOR_STATUS_OK = 1                   //执行成功
} AccessMonitorStatus;                      //监控状态类型结果枚举


typedef enum {
    ACCESS_MONITOR_CONNECT_NONE = 0,        //监控类型：无
    ACCESS_MONITOR_CONNECT_TCP,             //监控类型：TCP
    ACCESS_MONITOR_CONNECT_ICMP             //监控类型：ICMP
} AccessMonitorType;                        //监控类型枚举



typedef struct {
    const char *pURL;                       //URL
    const char *pIPAddress;                 //地址
    int Port;                               //端口
    AccessMonitorType Flag;                 //监控类型枚举
} Access_Monitor_Config_t;                  //监控配置结构体



/**
 * @brief Connect Thread Entry
 * 
 * @param pArg 
 * @return gpointer 
 */
void *AccessMonitorEntry(void *pArg);


void SetMonitorResult(int index, AccessMonitorStatus status);
AccessMonitorStatus GetMonitorResult(int index);

#ifdef __cplusplus
}
#endif
#endif

