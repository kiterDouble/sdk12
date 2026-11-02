#ifndef DB_BACKUP_SERVICE_H
#define DB_BACKUP_SERVICE_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct
{
    const char *dbFilePath;
    const char *backupFilePath;
} DbBackupConfig_t;

gboolean DbBackupServiceInit(const DbBackupConfig_t *cfg);
void     DbBackupServiceDeInit(void);

gboolean DbBackupServiceStart(void);
void     DbBackupServiceStop(void);

void     DbBackupNotifyBackup(void);

gboolean DbBackupCheckAndRestore(void);

G_END_DECLS

#endif /* DB_BACKUP_SERVICE_H */
