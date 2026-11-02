#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <glib.h>
#include "sr_ipc.h"
#include "err_code.h"
#include "sr_log.h"
#include "sr_ipc.h"
#include "fty_control.h"
#include "fty_info.h"
#include "fty_crt.h"
#include "sr_sdk.h"

gint main(gint argc, gchar *argv[])
{
    GMainLoop *loop = NULL;
    LOG_INIT("ftyServer");    
    LOG_Debug("fty server start !.");
    InitSRSdk();
    
    loop = g_main_loop_new(NULL, FALSE);

    StartQueueMsgRcvThread("fty_thread");
    SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY,FtyGetPmPowerState,NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_GNSS_LOCATIONINFO, FtyGnssMsgSubCallBack, NULL); 
    SubscriptionQueueMsg(SYS_QUE_TYPE_MODEM_SIGNAL_NOFIFY, FtyGetModemSrvInfo, NULL); 
    SubscriptionQueueMsg(SYS_QUE_TYPE_MCU_SYNC_FTY_REQ,ProcessRequest, NULL);

    GThread *pCrtThread = g_thread_new("fty crt", FtyCrtHandle, (gpointer)loop);
    GThread *pTestTimeout = g_thread_new("TestTimeout", TestTimeout, (gpointer)loop);
    
    
    g_main_loop_run(loop);
    g_thread_unref(pCrtThread);
    g_thread_unref(pTestTimeout);
    g_main_loop_unref(loop);
    return 0;
}

