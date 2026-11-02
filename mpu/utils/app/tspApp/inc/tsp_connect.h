#ifndef __TSP_CONNECT_H__
#define __TSP_CONNECT_H__

#include <glib.h>
#include "tsp_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define E50_DOWNLINK_TOPIC "/xCNzCoLiLME/"
#define E50_UPLINK_TOPIC "/xCNzCoLiLME/"
#define E50_FOTA_TOPIC "/fota/"
#define E50_QOS0 (0)
#define E50_QOS1 (1)
#define E50_QOS2 (2)

#define TSP_DATA_MAX_LEN (1024)


/**
 * @brief Get the Device ID object
 * 
 * @param deviceID 
 * @return gint 
 */
gint GetDeviceID(gchar *deviceID);

gboolean IsConnected(void);
/**
 * @brief Get the Package Data object
 * 
 * @param pData 
 * @return gint 
 */
gint GetTspPackageData(gpointer *pData);
/**
 * @brief Cache the Package Data object
 * 
 * @param pData 
 * @return gint 
 */
gint CachePackageData(gpointer pData);
/**
 * @brief Connect Thread Entry
 * 
 * @param pArg 
 * @return gpointer 
 */
void *ConnectTaskEntry(void *pArg);
/**
 * @brief Send Package Data To Tsp Server
 * 
 * @param[in]   u8Data 
 * @param[in]   u32length 
 * @retval      RET_OK  Success
 *              RET_ERR Failed
 */
gint SendPackageData(const gchar *topic, gint qos, gpointer u8Data, guint u32length);

#ifdef __cplusplus
}
#endif
#endif

