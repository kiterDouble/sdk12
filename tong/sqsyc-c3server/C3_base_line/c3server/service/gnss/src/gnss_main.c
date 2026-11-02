/*
 * @Description: In User Settings Edit
 * @Author: your name
 * @Date: 2019-09-09 10:31:08
 * @LastEditTime: 2019-09-18 13:59:26
 * @LastEditors: Please set LastEditors
 */
#include <glib.h>
#include <gio/gio.h>
#include "err_code.h"
#include "sr_log.h"
#include "sr_ipc.h"
#include "gnss_process.h"
#include "sr_hal_gnss.h"

gint main(gint argc, gchar *argv[])
{
    GMainLoop *loop = NULL;

    LOG_INIT("gnssServer");
    LOG_Debug("start transfer");
    StartQueueMsgRcvThread("gnssServer_ipc");
    GpsProcessInit();
    RpcStartMsgServer(RPCSERVER_TYPE_GNSSSERVER, gnssServerCallBack, 1, NULL);
    SubscriptionQueueMsg(SYS_QUE_TYPE_PM_POWER_STATE_NOTIFY, HandlePmSleepCmd, NULL);
    g_timeout_add(1000, (GSourceFunc)CheckGnssCBTimer, NULL);
    if (GpsLedIOInit() != 0)
    {
        g_timeout_add(1000, (GSourceFunc)CheckStatusOfGnssLocation, NULL);
    }
    StartInjectAgnssDataThread();
    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);
    g_main_loop_unref(loop);
    return 0;
}
