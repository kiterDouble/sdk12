#include <string.h>
#include <unistd.h>

#include <glib.h>

#include "sr_log.h"
#include "net_service.h"
#include "net_shell_utils.h"
#include "net_route_utils.h"

static void PollApnStatus(NetSrvCtx_t *ctx)
{
    static int currentApnIndex = -1;      // 当前使用的 APN 索引
    static int previousApnSwitched = 0;   // 用来标记是否切换过 APN

    int currentDefaultApn = -1;
    int backupApn = -1;

    for (int i = 0; i < ctx->userCfg->apnNum; i++)
    {
        if (strstr(ctx->strNetInfo[i].privateRoute, "default") != NULL)
        {
            currentDefaultApn = i;
            break;
        }
    }

    if (currentDefaultApn == -1)
    {
        LOG_Error("No default APN found.");
        return;
    }


    if (CheckNetworkConnectivity(ctx->strNetInfo[currentDefaultApn].priDns, ctx->strNetInfo[currentDefaultApn].ethName))
    {
        if (1 == previousApnSwitched)
        {
            LOG_Info("Restoring original default APN: %s", ctx->strNetInfo[currentDefaultApn].apnName);
            SetDefaultRoute(ctx->strNetInfo[currentDefaultApn].gateway, ctx->strNetInfo[currentDefaultApn].ethName);
            previousApnSwitched = 0;
            currentApnIndex = currentDefaultApn;
        }
        else
        {
            LOG_Debug("Default APN is still connected, no action needed.");
            return;
        }
    }
    else
    {
        if (1 == previousApnSwitched && currentApnIndex != -1)
        {
            if (CheckNetworkConnectivity(ctx->strNetInfo[currentApnIndex].priDns, ctx->strNetInfo[currentApnIndex].ethName))
            {
                LOG_Debug("Current switched APN is still connected, no need to switch.");
                return;
            }
        }

        for (int i = 0; i < ctx->userCfg->apnNum; i++)
        {
            if (i != currentDefaultApn && CheckNetworkConnectivity(ctx->strNetInfo[i].priDns, ctx->strNetInfo[i].ethName))
            {
                if (i == currentApnIndex)
                {
                    LOG_Debug("Backup APN is the same as the currently switched APN. No need to switch.");
                }
                else
                {
                    backupApn = i;
                    break;
                }
            }
        }

        if (backupApn != -1)
        {
            LOG_Info("Switching to backup APN: %s", ctx->strNetInfo[backupApn].apnName);
            SetDefaultRoute(ctx->strNetInfo[backupApn].gateway, ctx->strNetInfo[backupApn].ethName);
            previousApnSwitched = 1;
            currentApnIndex = backupApn;
        }
        else
        {
            LOG_Error("No APNs are currently available.");
        }
    }
}



gpointer DnsCheckMonitor(gpointer pArg)
{
    NetSrvCtx_t *ctx = (NetSrvCtx_t *)pArg;

    usleep(60000000);
    while (1)
    {
        PollApnStatus(ctx);
        sleep(10);
    }
}
