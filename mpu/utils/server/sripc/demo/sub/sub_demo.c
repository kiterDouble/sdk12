
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
#include <stdint.h>

#include "ipcTestStructs.h"

typedef struct
{
    uint8_t a;
    uint16_t b;
} Userdata_t, *P_Userdata_t;

static void signal_handler(int signum)
{
    g_print("recv signal:%d\n", signum);
    if (signum == SIGINT)
    {
        kill(getpid(), SIGKILL); 
    }
}

static void CallBack1(const uint8_t* data, const uint16_t len, void* userdata)
{
    urcBoardcast_t recvmsg;
    memset(&recvmsg, 0x00, sizeof(urcBoardcast_t));
    if(sizeof(urcBoardcast_t) == len)
    {
        memmove(&recvmsg, data, len);
    }
    else
    {
        g_print("CallBack1 len error!\n");
    }

    g_print("\n\n");
    g_print("[CallBack1]recv unslicitedData:[%s]\n", recvmsg.unslicitedData);
    g_print("[CallBack1]recv smsData:[%s]\n", recvmsg.smsData);

    /********************************获取用户数据***********************************/
    if(userdata != NULL)
    {
        P_Userdata_t ud = (P_Userdata_t)userdata;
        g_print("[CallBack1]userdata:[%d],[%d]\n", ud->a, ud->b);
    }
}

static void CallBack2(const uint8_t* data, const uint16_t len, void* userdata)
{
    urcBoardcast_t recvmsg;
    memset(&recvmsg, 0x00, sizeof(urcBoardcast_t));
    if(sizeof(urcBoardcast_t) == len)
    {
        memmove(&recvmsg, data, len);
    }
    else
    {
        g_print("CallBack2 len error!\n");
    }

    g_print("[CallBack2]recv unslicitedData:[%s]\n", recvmsg.unslicitedData);
    g_print("[CallBack2]recv smsData:[%s]\n", recvmsg.smsData);
}

int main(int argc, char *argv[])
{

    LOG_INIT("demo");
    LOG_Debug("start transfer");

    signal(SIGINT, signal_handler);

    Userdata_t *userdata = g_try_malloc0(sizeof(Userdata_t));
    if (userdata == NULL)
    {
        LOG_Error("malloc recv data error");
        return -1;
    }
    userdata->a = 100;
    userdata->b = 200;

    StartQueueMsgRcvThread("subdemo_thread");
    SubscriptionQueueMsg(SYS_QUE_TYPE_TEST1, CallBack1, (void*)userdata);//将用户参数传入
    SubscriptionQueueMsg(SYS_QUE_TYPE_TEST2, CallBack2, NULL);//没有用户数据的话传入NULL即可

    //可以订阅多条消息：
    // SubscriptionQueueMsg(10001, CallBack3);
    // SubscriptionQueueMsg(10002, CallBack4);
    // SubscriptionQueueMsg(10003, CallBack5);
    // SubscriptionQueueMsg(10004, CallBack6);
    // SubscriptionQueueMsg(10005, CallBack7);
    // SubscriptionQueueMsg(10006, CallBack8);
    // SubscriptionQueueMsg(10007, CallBack9);
    // SubscriptionQueueMsg(10008, CallBack10);
    // SubscriptionQueueMsg(10009, CallBack11);

    while(1)
    {
        sleep(1);
    }

    g_free(userdata);

    return RET_OK;
}