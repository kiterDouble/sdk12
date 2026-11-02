#include <glib.h>

#include "era_main.h"
#include "sr_log.h"
#include "sr_ipc.h"
#include "err_code.h"
#include "era_timer.h"
#include "era_mcu_manager.h"
#include "era_nad_manager.h"

#include "sr_sdk.h"
#include "asn_application.h"

#include <NativeInteger.h>
#include <OCTET_STRING.h>
#include <constr_SEQUENCE.h>
#include <BOOLEAN.h>

void ASNTest(void)
{
#if 0
    struct ERAAdditionalData data;

    memset(&data, 0, sizeof(data));
#endif



}



int main(int argc, char **argv)
{
    GMainLoop *loop = NULL;

    LOG_INIT("eraServer");
    InitSRSdk();
    StartQueueMsgRcvThread("era_thread");
    
    loop = g_main_loop_new(NULL, FALSE);
       
    GThread *mcuServerThread = g_thread_new("McuServer", McuServer, (gpointer)loop);
    
    GThread *ecallServerThread = g_thread_new("eraServer", EcallServer, (gpointer)loop);
    g_main_loop_run(loop);
    
    g_thread_unref(mcuServerThread);
    g_thread_unref(ecallServerThread);

    g_main_loop_unref(loop);

    return RET_ERR;
}

