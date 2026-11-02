
#include "sr_ipc.h"
#include "sr_log.h"
#include "err_code.h"
#include <glib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <math.h>
#include "glib.h"
#include <errno.h>
#include <stdint.h>

#include "ipcTestStructs.h"

static void signal_handler(int signum)
{
    g_print("recv signal:%d\n", signum);
    if (signum == SIGINT)
    {
        kill(getpid(), SIGKILL); 
    }
}

void rpcServerCallBack(const uint16_t subcmd, const uint8_t *const reqData, const uint16_t reqLen, uint8_t *respData, uint16_t *respLen, void* userdata)
{
    //回调处理函数中分别处理不同类型的请求
    if(RPCSERVERDEMO_MSGTYPE_SUBCMD_ID1 == subcmd)
    {
        g_print("\n\n");
        SendAtCmd_req_t msgreq;
        memset(&msgreq, 0x00, sizeof(SendAtCmd_req_t));
        //取出请求消息
        memmove(&msgreq, reqData, reqLen);

        g_print("[SUBCMD_ID1]recv request from Client: cmd:%s\n", msgreq.cmd);
        g_print("[SUBCMD_ID1]recv request from Client: prefix:%s\n", msgreq.prefix);
        g_print("[SUBCMD_ID1]recv request from Client: responseLines:%d\n", msgreq.responseLines);

        //------------------------------------------------------------------------------------------
        //--------------根据接收到的请求消息进行处理，处理过程需要用户根据具体业务实现---------------
        //-------------------------假设处理后已经得到了cmdResult与response值------------------------
        //------------------------------------------------------------------------------------------

        SendAtCmd_resp_t msgresp;
        memset(&msgresp, 0x00, sizeof(SendAtCmd_resp_t));
        msgresp.cmdResult = msgreq.responseLines + 10000;
        snprintf(msgresp.response, sizeof(msgresp.response), "at cmd ok!%d", msgresp.cmdResult);

        g_print("[SUBCMD_ID1]response to Client: cmdResult:%d\n", msgresp.cmdResult);
        g_print("[SUBCMD_ID1]response to Client: response:%s\n", msgresp.response);

        // 将序列化后的回复消息通过入参传给rpc库 rpc库会自动转发给请求的app
        *respLen = sizeof(SendAtCmd_resp_t);
        memmove(respData, (uint8_t *)&msgresp, sizeof(SendAtCmd_resp_t));
    }
    else if(RPCSERVERDEMO_MSGTYPE_SUBCMD_ID2 == subcmd)
    {
        SendAtCmd_req_t msgreq;
        memset(&msgreq, 0x00, sizeof(SendAtCmd_req_t));
        //取出请求消息
        memmove(&msgreq, reqData, reqLen);

        g_print("[SUBCMD_ID2]recv request from Client: cmd:%s\n", msgreq.cmd);
        g_print("[SUBCMD_ID2]recv request from Client: prefix:%s\n", msgreq.prefix);
        g_print("[SUBCMD_ID2]recv request from Client: responseLines:%d\n", msgreq.responseLines);

        //------------------------------------------------------------------------------------------
        //--------------根据接收到的请求消息进行处理，处理过程需要用户根据具体业务实现---------------
        //-------------------------假设处理后已经得到了cmdResult与response值------------------------
        //------------------------------------------------------------------------------------------

        SendAtCmd_resp_t msgresp;
        memset(&msgresp, 0x00, sizeof(SendAtCmd_resp_t));
        msgresp.cmdResult = msgreq.responseLines + 10000;
        snprintf(msgresp.response, sizeof(msgresp.response), "at cmd ok!%d", msgresp.cmdResult);

        g_print("[SUBCMD_ID2]response to Client: cmdResult:%d\n", msgresp.cmdResult);
        g_print("[SUBCMD_ID2]response to Client: response:%s\n", msgresp.response);

        // 将序列化后的回复消息通过入参传给rpc库 rpc库会自动转发给请求的app
        *respLen = sizeof(SendAtCmd_resp_t);
        memmove(respData, (uint8_t *)&msgresp, sizeof(SendAtCmd_resp_t));
    }
    else
    {

    }
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler);
    LOG_INIT("demo");
    LOG_Debug("start transfer");


    //2. 创建类型为RPCSERVER_TYPE_RPCSERVERDEMO的rpcserver，使用4个线程处理RPC请求，处理函数为rpcServerCallBack 处理函数需要由用户提供
    RpcStartMsgServer(RPCSERVER_TYPE_RPCSERVERDEMO, rpcServerCallBack, 4, NULL);
    while(1)
    {
        g_usleep(1000000);
    }
    return RET_OK;
}