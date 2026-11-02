#include <glib.h>

#include "net_config.h"
#include "net_rpc.h"
#include "net_service.h"
#include "sr_log.h"
#include "err_code.h"

static ModemSrvNotify_t modemSrvNotify = {0,0,1,0,0,0};
static StrNetInfo_t strNetInfo[NETWORK_APN_NUM] = {0};
static MPUPmStateChange_t netPmState = {0};
static NetUserCfg_t netUserCfg = {0};
static DialRecordDbCtx_t dialRecordDb = {0};

NetSrvCtx_t g_srvCtx = {
    .modemSrvNotify = &modemSrvNotify,
    .strNetInfo = strNetInfo,
    .pmState = &netPmState,
    .userCfg = &netUserCfg,
    .dialRecordDb = &dialRecordDb,
};

// 加载配置
static void LoadAndInitNetConfig(NetSrvCtx_t* ctx)
{
    if (LoadNetConfig(ctx->userCfg, DNS_PATH_NAME_MAX_LEN) != RET_OK)
    {
        LOG_Error("%s : LoadNetConfig is err.",__FUNCTION__);
    }
    else
    {
        memset(ctx->strNetInfo, 0, sizeof(StrNetInfo_t)*NETWORK_APN_NUM);
        for (uint8_t i = 0; i < ctx->userCfg->apnNum; ++i)
        {
            ctx->strNetInfo[i].profileIndex   = ctx->userCfg->apnInfoCfg[i].profileIndex;
            ctx->strNetInfo[i].pdpType        = ctx->userCfg->apnInfoCfg[i].pdpType;
            ctx->strNetInfo[i].authProto      = ctx->userCfg->apnInfoCfg[i].authProto;
            ctx->strNetInfo[i].nameserverPri  = ctx->userCfg->apnInfoCfg[i].nameserverPri;
            ctx->strNetInfo[i].dnsRouteSet    = ctx->userCfg->apnInfoCfg[i].dnsRouteSet;
            ctx->strNetInfo[i].privateNet     = ctx->userCfg->apnInfoCfg[i].privateNet;
            snprintf(ctx->strNetInfo[i].apnName, sizeof(ctx->strNetInfo[i].apnName)-1, "%s", ctx->userCfg->apnInfoCfg[i].apnName);
            snprintf(ctx->strNetInfo[i].usrName, sizeof(ctx->strNetInfo[i].usrName)-1, "%s", ctx->userCfg->apnInfoCfg[i].usrName);
            snprintf(ctx->strNetInfo[i].password, sizeof(ctx->strNetInfo[i].password)-1, "%s", ctx->userCfg->apnInfoCfg[i].password);
            snprintf(ctx->strNetInfo[i].privateRoute, sizeof(ctx->strNetInfo[i].privateRoute)-1, "%s", ctx->userCfg->apnInfoCfg[i].privateRoute);
            snprintf(ctx->strNetInfo[i].domain, sizeof(ctx->strNetInfo[i].domain)-1, "%s", ctx->userCfg->apnInfoCfg[i].domain);
         }
    }
}

int main()
{
    LOG_INIT("netServer");
    LOG_Debug("net server start !.");

    LoadAndInitNetConfig(&g_srvCtx);

    /** 北纬SIM卡切换任务 **/
    if (g_srvCtx.userCfg->supportBeWinnerSimCard){
        BeWinnerSimTaskStart();
    }

    InitIpcRpcModules(&g_srvCtx);

    StartNetService(&g_srvCtx);

    GThread * netMonitorThread = g_thread_new("NetMonitor", NetMonitor, &g_srvCtx);
    GThread * netSyncThread = g_thread_new("netSync", NetStatusSyncMcu, &g_srvCtx);
    //GThread * dnsCheckThread = g_thread_new("DnsCheck", DnsCheckMonitor, &g_srvCtx);

    //g_thread_join(dnsCheckThread);
    g_thread_join(netSyncThread);
    g_thread_join(netMonitorThread);
    
}
