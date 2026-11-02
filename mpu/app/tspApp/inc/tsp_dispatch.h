#ifndef __TSP_DISPATCH_H__
#define __TSP_DISPATCH_H__

#include <glib.h>
#include "tsp_common.h"

#ifdef __cplusplus
extern "C" {
#endif
gint CacheDispatchData(IN gchar *pTopic, IN guchar *pData, IN guint dataLen);
gint DispatchCalTopicCrc(void);
/**
 * @brief 处理平台报文
 * 
 * @param pArg 
 * @return gpointer 
 */
gpointer DispatchTaskEntry(IN gpointer pArg);
#ifdef __cplusplus
}
#endif

#endif