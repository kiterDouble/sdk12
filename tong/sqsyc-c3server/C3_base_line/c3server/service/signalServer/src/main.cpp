#include <stdio.h>
#include <stdlib.h>
#include "err_code.h"
#include "data_pool.h"
#include "can_parser.h"
#include "sr_log.h"
#include "sr_ipc.h"
#include "queue_msg.h"
#include "can_storage.h"


#include <iostream>
#include <string>

using namespace std;


// void GB_read_file(const std::string& file_name, unsigned char* data_read, size_t data_size)
// {
//     std::ifstream infile(file_name.c_str(), std::ios::binary | std::ios::ate);

//     if (!infile.is_open()) 
//     {LOG_Debug("open encode file error");
//         return;
//     }
//     infile.seekg(0, std::ios::end);
//     size_t file_size = infile.tellg();
//     infile.seekg(0, std::ios::beg);
//     LOG_Debug("read file size %d success!",file_size);
//     infile.read(reinterpret_cast<char*>(data_read) , file_size);
//     LOG_Debug("get file success");infile.close();
//     LOG_Debug("close file success");sleep(1);
//     LOG_Debug("close file success");
// }


// int main()
// {
// 	char  buff[1024] = {0};
	
// 	GB_read_file("/oemdata/test", buff, 1024);
	
	
// 	return 0;
// }

int main()
{
    /**
     * @brief 初始化zlog
     * 
     */
    LOG_INIT("signalServer");

    // InitCanDataMemMap();
    /**
     * @brief 主线程循环 定时器广播信号依赖主线程
     * 
     */
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
	
    /**
     * @brief 开启ipc服务
     * 
     */
    StartQueueMsgRcvThread("signalserver_ipc");

    /**
     * @brief Construct a new Creat Rpc Queue object
     * 
     */
    CreatRpcQueue();

    /**
     * @brief 获取dbcjson
     * 
     */
    while (RET_OK != DpLoadConfig())
    {
        LOG_Error("DpLoadConfig failed");
        sleep(1);
    }


    /**
     * @brief Construct a new Subscription Queue Msg object:广播CAN数据
     * 
     */
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_VEHICLE_CAN_DATA, RecvMcuCANMsgCallBack, NULL);

    /**
     * @brief 读取structjson并初始化发送定时器
     * 
     */
    dealSendCanSignalMsgs();

    GThread *canParserPro = g_thread_new("CanParserProcess", CanParserProcess, (gpointer)loop);
    GThread *recvCanTimeoutPro = g_thread_new("RecvCanDataTimeoutProcess", RecvCanDataTimeoutProcess, (gpointer)loop);
    GThread *recvCanRealUpload = g_thread_new("RecvCanDataRealUploadProcess", RealDataUploadHandle, (gpointer)loop);

    g_main_loop_run(loop);
    g_thread_unref(recvCanRealUpload);
    g_thread_unref(recvCanTimeoutPro);
    g_thread_unref(canParserPro);
    g_main_loop_unref(loop);
    return 0;
}
