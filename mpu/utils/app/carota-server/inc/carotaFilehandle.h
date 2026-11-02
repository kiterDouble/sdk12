#ifndef __CAROTA_FILEHANDLE_H__
#define __CAROTA_FILEHANDLE_H__
 
#include <glib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif



int read_status(void);
gint32 UcuStatusFileGetStatus(int *status);
gint32 UcuStatusFileSetStatus(int status);
gint32 create_directory_if_not_exists(const char *path);
int ReadKl15StatusFromFile(uint8_t *kl15_value);
int SaveKl15StatusToFile(uint8_t kl15_value);
gint32 CarotaFileVersionGetVersion(gchar *version_buffer, gsize buffer_size);
int ReadSMainModemUpgradeStatus(void);
gint32 WriteSMainModemUpgradeStatus(int status);
int ReadMqttUpgradeStatus(void);
#ifdef __cplusplus
}
#endif


#endif