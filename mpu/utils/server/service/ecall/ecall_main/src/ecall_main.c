#include <glib.h>

#include "ecall_main.h"
#include "sr_log.h"
#include "sr_ipc.h"
#include "err_code.h"
#include "ecall_timer.h"
#include "ecall_mcu_manager.h"
#include "ecall_nad_manager.h"

#include "sr_sdk.h"


int main(int argc, char **argv)
{
    GMainLoop *loop = NULL;

    LOG_INIT("ecallServer");
    
    InitSRSdk();

    StartQueueMsgRcvThread("ecall_thread");
    
    loop = g_main_loop_new(NULL, FALSE);
       
    GThread *mcuServerThread = g_thread_new("McuServer", McuServer, (gpointer)loop);
    GThread *ecallServerThread = g_thread_new("EcallServer", EcallServer, (gpointer)loop);
    
    g_main_loop_run(loop);
    
    g_thread_unref(mcuServerThread);
    g_thread_unref(ecallServerThread);

    g_main_loop_unref(loop);

    return RET_ERR;
}

