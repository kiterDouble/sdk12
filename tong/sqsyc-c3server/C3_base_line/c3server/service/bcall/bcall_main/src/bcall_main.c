#include <glib.h>

#include "bcall_main.h"
#include "sr_log.h"
#include "sr_ipc.h"
#include "err_code.h"
#include "bcall_timer.h"
#include "bcall_mcu_manager.h"
#include "bcall_nad_manager.h"

#include "sr_sdk.h"


int main(int argc, char **argv)
{
    GMainLoop *loop = NULL;

    LOG_INIT("bcallServer");
    
    InitSRSdk();

    StartQueueMsgRcvThread("bcall_thread");
    
    loop = g_main_loop_new(NULL, FALSE);
       
    GThread *mcuServerThread = g_thread_new("McuServer", McuServer, (gpointer)loop);
    GThread *bcallServerThread = g_thread_new("BcallServer", BcallServer, (gpointer)loop);
    
    g_main_loop_run(loop);
    
    g_thread_unref(mcuServerThread);
    g_thread_unref(bcallServerThread);

    g_main_loop_unref(loop);

    return RET_ERR;
}

