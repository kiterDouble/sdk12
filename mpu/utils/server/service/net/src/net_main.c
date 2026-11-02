#include <glib.h>

#include "net_config.h"
#include "net_rpc.h"
#include "net_service.h"
#include "sr_log.h"
#include "err_code.h"

int main()
{
    LOG_INIT("netServer");
    LOG_Debug("net server start !.");

    InitIpcRpcModules();

    StartNetService();

    GThread * netMonitorThread = g_thread_new("NetMonitor", NetMonitor, NULL);
    GThread * netSyncThread = g_thread_new("netSync", NetStatusSyncMcu, NULL);
    GThread * modemAliveThread = g_thread_new("modemAlive", SyncModemAliveStatus, NULL);
    GThread * networkCheckThread = g_thread_new("networkCheck", ModulePeriodicNetworkCheck, NULL);
    //GThread * dnsCheckThread = g_thread_new("DnsCheck", DnsCheckMonitor, NULL);

    //g_thread_join(dnsCheckThread);
    g_thread_join(networkCheckThread);
    g_thread_join(modemAliveThread);
    g_thread_join(netSyncThread);
    g_thread_join(netMonitorThread);
    
}
