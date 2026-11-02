#include  "tsp_monitor.h"
#include "common_functions.h"
#include "net_monitor_ipc_msg.h"
#include "sr_ipc.h"

#define MAX_MONITOR_NUM  16    //最大监控数量

static pthread_mutex_t g_monitor_result_lock = PTHREAD_MUTEX_INITIALIZER;
static AccessMonitorStatus g_monitor_result[MAX_MONITOR_NUM] = { MONITOR_STATUS_UNKNOWN };

//定义一个结构体数组，存放九识需要监控的外部服务器 和 内部网络节点
Access_Monitor_Config_t g_access_monitor_list[] = {
    { .pURL = "eos-wuxi-5.cmecloud.cn", .Port = 443, .Flag = ACCESS_MONITOR_CONNECT_TCP },
    { .pURL = "relay.zelostech.com.cn", .Port = 7999, .Flag = ACCESS_MONITOR_CONNECT_TCP },
    { .pURL = "video1.zelostech.com.cn", .Port = 20000, .Flag = ACCESS_MONITOR_CONNECT_TCP },

    { .pIPAddress = "192.168.1.1", .Flag = ACCESS_MONITOR_CONNECT_ICMP },
    { .pIPAddress = "192.168.1.2", .Flag = ACCESS_MONITOR_CONNECT_ICMP },
    { .pIPAddress = "192.168.1.3", .Flag = ACCESS_MONITOR_CONNECT_ICMP },
    { .pIPAddress = "192.168.1.8", .Flag = ACCESS_MONITOR_CONNECT_ICMP },
    { .pIPAddress = "192.168.1.20", .Flag = ACCESS_MONITOR_CONNECT_ICMP },
    { .pIPAddress = "192.168.1.201", .Flag = ACCESS_MONITOR_CONNECT_ICMP },
    { .pIPAddress = "192.168.1.202", .Flag = ACCESS_MONITOR_CONNECT_ICMP },

    { NULL, NULL, 0, ACCESS_MONITOR_CONNECT_NONE} // 结束标志
};



/**
  * 函数功能: 监控结果设置
  * 输入参数: index: 序号
  *          status：监控状态类型
  * 返 回 值: 无
  * 说    明: 无
  */
void SetMonitorResult(int index, AccessMonitorStatus status)
{
      //参数检查
      if (index < 0 || index >= MAX_MONITOR_NUM)
      {
         return;
      } 
      pthread_mutex_lock(&g_monitor_result_lock);
      //监控结果设置
      g_monitor_result[index] = status;
      pthread_mutex_unlock(&g_monitor_result_lock);
}

/**
  * 函数功能: 监控结果获取
  * 输入参数: index: 序号
  * 返 回 值: AccessMonitorStatus枚举类型结果
  * 说    明: 无
  */
AccessMonitorStatus GetMonitorResult(int index)
{
     //参数检查
      if (index < 0 || index >= MAX_MONITOR_NUM)
      {
         return MONITOR_STATUS_UNKNOWN; 
      } 
      pthread_mutex_lock(&g_monitor_result_lock);
      //监控结果获取
      AccessMonitorStatus result = g_monitor_result[index];
      pthread_mutex_unlock(&g_monitor_result_lock);
      return result;
}


/**
  * 函数功能: 监控线程
  * 输入参数: pArg
  * 返 回 值: 
  * 说    明: 无
  */
void * AccessMonitorEntry(void *pArg)
{
    int sock;
    int ret;
    char cmd[128];
    NetState_t dataToPost;
    while (1)
    {
        // 遍历结构体数组
        for (int i = 0; g_access_monitor_list[i].Flag != ACCESS_MONITOR_CONNECT_NONE; ++i)
        {
            Access_Monitor_Config_t *cfg = &g_access_monitor_list[i]; // 定义一个结构体指针
            // 如果是远端服务器 先DNS解析
            if (cfg->Flag == ACCESS_MONITOR_CONNECT_TCP)
            {
                struct hostent *host = gethostbyname(cfg->pURL);
                if (!host)
                {
                    LOG_Error("DNS resolve failed: %s", cfg->pURL);
                    continue;
                }             
                sock = socket(AF_INET, SOCK_STREAM, 0);
                if (sock < 0)
                {
                    LOG_Error("Failed to create socket for %s", cfg->pURL);
                    continue;
                }

                // 2.设置 socket结构体
                struct sockaddr_in sockaddr = {0};    
                sockaddr.sin_family = AF_INET;
                sockaddr.sin_port = htons(cfg->Port);
                memcpy(&sockaddr.sin_addr, host->h_addr, host->h_length);

                // 3.连接
                ret = connect(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
                if (ret == 0)
                {
                    LOG_Debug("TCP connect OK: %s:%d", cfg->pURL, cfg->Port);
                    SetMonitorResult(i, MONITOR_STATUS_OK);
                }
                else
                {
                    LOG_Error("TCP  connect FAIL: %s:%d", cfg->pURL, cfg->Port);
                     SetMonitorResult(i, MONITOR_STATUS_FAIL);
                }
                close(sock);
            }
            // 如果是内部网络节点
            else if (cfg->Flag == ACCESS_MONITOR_CONNECT_ICMP)
            {
                // 构造 PING 命令
                snprintf(cmd, sizeof(cmd), "ping -c 1 -W 1 %s > /dev/null", cfg->pIPAddress);
                ret = GetSystemRetValue(cmd);
                if (ret == 0)
                {
                    LOG_Debug("PING OK: %s", cfg->pIPAddress);
                     SetMonitorResult(i, MONITOR_STATUS_OK);

                }
                else
                {
                    LOG_Error("PING FAIL: %s", cfg->pIPAddress);
                    SetMonitorResult(i, MONITOR_STATUS_FAIL);
                }
            }
        }
        dataToPost.url1 = g_monitor_result[0] == MONITOR_STATUS_OK ? 1:0;
        dataToPost.url2 = g_monitor_result[1] == MONITOR_STATUS_OK ? 1:0;
        dataToPost.url3 = g_monitor_result[2] == MONITOR_STATUS_OK ? 1:0;
        dataToPost.gateway = g_monitor_result[3] == MONITOR_STATUS_OK ? 1:0;
        dataToPost.xxka = g_monitor_result[4] == MONITOR_STATUS_OK ? 1:0;
        dataToPost.xxkb = g_monitor_result[5] == MONITOR_STATUS_OK ? 1:0;
        dataToPost.onlineLanguageModule = g_monitor_result[6] == MONITOR_STATUS_OK ? 1:0;
        dataToPost.bluetoothToEthernetModule = g_monitor_result[7] == MONITOR_STATUS_OK ? 1:0;
        dataToPost.module1 = g_monitor_result[8] == MONITOR_STATUS_OK ? 1:0;
        dataToPost.module2 = g_monitor_result[9] == MONITOR_STATUS_OK ? 1:0;
        // 广播ping网状态
        SendMsgToAllQueue(SYS_QUE_TYPE_PING_NET, (void *)&dataToPost, sizeof(NetState_t));        
        sleep(10);
    }
    return NULL;
}