#ifndef __MQTT_CRC32_H__
#define __MQTT_CRC32_H__
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif
#define IN
#define OUT
#define INOUT
 
guint Crc32Init(void);
guint Crc32Update(IN guint crc, IN gpointer input, IN gint len);
guint Crc32Final(IN guint crc);
guint StrCalCrc(gchar *pStr);
#ifdef __cplusplus
}
#endif
#endif