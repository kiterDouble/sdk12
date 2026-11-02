#ifndef __TSP_GETTOKEN_H__
#define __TSP_GETTOKEN_H__
#include "tsp_common.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_BUFFER_SIZE (4096)

void GetTokenInfo(gchar *buf);
gint TaskRequestToken(void);

gint GetFileFromTokenCheck();
gint GetHostIpAddr(const char *domainName, gint domainNameLen,
                   char       *ipAddrBuff, gint ipAddrBuffLen);
gint PostFotaMsg(char type, guchar *msgBuff, int msgLen);

#ifdef __cplusplus
}
#endif
#endif

