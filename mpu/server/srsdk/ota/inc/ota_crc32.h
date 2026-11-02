#ifndef __OTA_CRC32_H__
#define __OTA_CRC32_H__
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

guint Crc32Init(void);
guint Crc32Update(guint crc, gpointer input, gint len);
guint Crc32Final(guint crc);
#ifdef __cplusplus
}
#endif
#endif