#ifndef __UCU_UPGRADE_H__
#define __UCU_UPGRADE_H__
#include <glib.h>
#ifdef __cplusplus
extern "C"
{
#endif

static inline int timespec_compare(const struct timespec *lhs, const struct timespec *rhs)
{
    if (lhs->tv_sec < rhs->tv_sec)
        return -1;
    if (lhs->tv_sec > rhs->tv_sec)
        return 1;
    return lhs->tv_nsec - rhs->tv_nsec;
}
gint32 UcuUpgradeStart(const gchar filepath[]);
gint32 UcuUpgradeGetResult(gchar errlog[]);
gint32 UcuUpgradeinit(void);
gint32 checkUpgradeStatus();
uint8_t GetVehMisModStatus(void);
gint StartUpgradePkg_ota2(const gchar *realpath, gchar uflag, gchar *errlog);
#ifdef __cplusplus
}
#endif
#endif