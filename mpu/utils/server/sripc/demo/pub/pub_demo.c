
#include "sr_ipc.h"
#include "sr_log.h"
#include "err_code.h"
#include <glib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
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

    //启动ipc服务
    StartQueueMsgRcvThread("pubdemo_thread");

    while (1)
    {
        static int count = 0;

        urcBoardcast_t msg1;
        urcBoardcast_t msg2;
        memset(&msg1, 0x00, sizeof(urcBoardcast_t));
        memset(&msg2, 0x00, sizeof(urcBoardcast_t));

        count ++;
        snprintf(msg1.unslicitedData, sizeof(msg1.unslicitedData), "testmsg1.unslicitedData!:%d", count);
        snprintf(msg1.smsData, sizeof(msg1.smsData), "testmsg1.smsData!:%d", count);

        snprintf(msg2.unslicitedData, sizeof(msg2.unslicitedData), "testmsg2.unslicitedData!:%d", count);
        snprintf(msg2.smsData, sizeof(msg2.smsData), "testmsg2.smsData!:%d", count);

        SendMsgToAllQueue(SYS_QUE_TYPE_TEST1, (void *)&msg1, sizeof(urcBoardcast_t));
        // g_usleep(10000);
        SendMsgToAllQueue(SYS_QUE_TYPE_TEST2, (void *)&msg2, sizeof(urcBoardcast_t));
        g_usleep(200000);
    }
    return RET_OK;
}