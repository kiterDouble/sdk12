
#include "sr_ipc.h"
#include "sr_log.h"
#include "err_code.h"
#include <glib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
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

int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler);
    LOG_INIT("demo");//从pubdemo.conf中初始化zlog日志
    LOG_Debug("start transfer");

    // 1.初始化rpc的两个队列（在main函数中进行）
    CreatRpcQueue();
    // 2.准备请求携带的参数
    SendAtCmd_req_t msgreq;
    memset(&msgreq, 0x00, sizeof(SendAtCmd_req_t));
    msgreq.responseLines = 10000;
    snprintf(msgreq.cmd, sizeof(msgreq.cmd), "at+cgsn?");
    snprintf(msgreq.prefix, sizeof(msgreq.prefix), "whatisprefix");

    // 3.准备回复保存的位置buffer
    SendAtCmd_resp_t msgresp;
    memset(&msgresp, 0x00, sizeof(SendAtCmd_resp_t));

    // 4.使用用户准备的接口 通过rpc库向server获取消息
    while (1)
    {
        g_print("\n\n");

        //1.1.准备接收buffer
        uint16_t recvlen = 0;
        uint8_t *recvdata = (uint8_t *)g_try_malloc0(SYSQUE_MSG_DATA_MAX_LEN);
        memset(&msgresp, 0x00, sizeof(SendAtCmd_resp_t));
        if (recvdata == NULL)
        {
            LOG_Error("malloc error[%s]", g_strerror(errno));
            return -1;
        }
        //1.2 使用rpc库获取rpcserverdemo的SUBCMD_ID1类型消息，无超时，请求消息结构为msgreq，消息回复保存到buffer中
        RpcGetMsgSync(RPCSERVER_TYPE_RPCSERVERDEMO, RPCSERVERDEMO_MSGTYPE_SUBCMD_ID1, 0, 
                        (uint8_t *)&msgreq, sizeof(SendAtCmd_req_t),
                        recvdata, &recvlen);
        //1.3 如果接收到的消息长度和接收的结构体长度相同 说明接收正常，解析即可
        if(recvlen == sizeof(SendAtCmd_resp_t))
        {
            memmove(&msgresp, recvdata, recvlen);
            g_print("[SUBCMD_ID1]request to Server: responseLines:[%d], cmd:[%s], prefix[%s]\n", msgreq.responseLines, msgreq.cmd, msgreq.prefix);
            g_print("[SUBCMD_ID1]recv response from Server: response:[%s], cmdResult:[%d]\n", msgresp.response, msgresp.cmdResult);
        }
        else
        {
            g_print("recv len error!\n");
        }
        recvlen = 0;
        memset(recvdata, 0x00, SYSQUE_MSG_DATA_MAX_LEN);
        memset(&msgresp, 0x00, sizeof(SendAtCmd_resp_t));
        // 1.4 使用rpc库获取rpcserverdemo的SUBCMD_ID2类型消息，无超时，请求消息结构为msgreq，消息回复保存到buffer中
        RpcGetMsgSync(RPCSERVER_TYPE_RPCSERVERDEMO, RPCSERVERDEMO_MSGTYPE_SUBCMD_ID2, 0,
                      (uint8_t *)&msgreq, sizeof(SendAtCmd_req_t),
                      recvdata, &recvlen);
        // 1.5 如果接收到的消息长度和接收的结构体长度相同 说明接收正常，解析即可
        if (recvlen == sizeof(SendAtCmd_resp_t))
        {
            memmove(&msgresp, recvdata, recvlen);
            g_print("[SUBCMD_ID2]request to Server: responseLines:[%d], cmd:[%s], prefix[%s]\n", msgreq.responseLines, msgreq.cmd, msgreq.prefix);
            g_print("[SUBCMD_ID2]recv response from Server: response:[%s], cmdResult:[%d]\n", msgresp.response, msgresp.cmdResult);
        }
        else
        {
            g_print("recv len error!\n");
        }

        msgreq.responseLines ++;
        g_free(recvdata);
        recvdata = NULL;
        g_usleep(100000);
    }
    return RET_OK;
}